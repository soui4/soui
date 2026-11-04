# swinx canvas2d 后端"飘字残影"缺陷修复说明

日期：2026-11-06
影响面：`swinx` 的 wasm **canvas2d** GDI 后端（`swinx/src/gdi/canvas2d/gdi.cpp`）
基线：同一份 SOUI 代码 + 同一份 demo 资源，在 wasm **cairo** 后端、Windows 真 GDI、macOS apple 后端上均无此现象
过程记录：`doc/soui-wasm-port-plan.md` §31–§36；本文是结案说明，回答**为什么这次修复有效**

---

## 0. 一句话结论

俄罗斯方块消行后那串"永远擦不干净的金色抛物线"不是控件、动画或脚本的问题，而是 canvas2d 后端在**三条 Win32 GDI 语义**上偏离了参考实现：

1. 上屏那次 `BitBlt(SRCCOPY)` 做成了**叠加**而不是**替换**；
2. 裁剪的**替换/移除**撤不掉旧裁剪（canvas 只能求交）；
3. `DrawText` **不裁格式化矩形**，会往控件矩形外面落墨。

三条各自都足以造成"某格墨没有任何一帧会覆盖它"。它们合成一条完整因果链之后，才有现象；三条都补齐之后，这条渲染链上不再存在"画得出去、擦不回来"的路径。修复只把 canvas2d 拉回参考实现，没有改 SOUI 层、没有改另外三个后端 —— 这正是"只有 canvas2d 出问题、只需要改 canvas2d"的自洽解释。

---

## 1. 现象与判据

- 截图：`demos/wasm-demo/bug.png`、`demos/wasm-demo/bug2.png` —— 金色"+分数"沿抛物线飞过后，轨迹上留下一格格窄条，棋盘与得分区都不重绘它。
- 后端维度：canvas2d 出、cairo 不出（同一份 `.data` 资源，只差 swinx 二进制）。
- 形态维度：把捡分特效换成图片（`SGraphic` + 星芒 SVG，走 `DrawSVG → AlphaBlend`，完全不经文字绘制与文字测量）后，1–10 关连测两遍无残影；换回文字复现。这一步把范围从"通用擦除链"收敛到"文字链 + 覆盖范围"。
- 偶发性：需要特定帧序（某帧有窄裁剪泄漏，且某次消行触发越界墨或宽脏区），所以不是每局都出现。

---

## 2. 一帧画面是怎么来的（不讲清这条链，就无法解释"为什么擦不掉"）

| 步骤 | 代码 | 干了什么 |
|---|---|---|
| 1 | `SOUI/src/core/shostwnd.cpp:678` `_RedrawRegion` | 取本轮脏区并交上窗口矩形 |
| 2 | `shostwnd.cpp:690` `PushClipRegion(pRgnUpdate, RGN_COPY)` | 把裁剪区**设成**脏区 |
| 3 | `components/render-gdi/linux/render-gdi.cpp:660` | 实现是 `SaveDC` + `ExtSelectClipRgn(hdc, rgn, mode)` —— 第 2 步走的是 **RGN_COPY 替换** |
| 4 | `shostwnd.cpp:699` `ClearRect(rcInvalid, 0)` | 整块脏区清成**全透明** |
| 5 | `GetRoot()->RedrawRegion(...)` → `SWindow::DispatchPaint`（`SOUI/src/core/Swnd.cpp:1977`） | 只重画与脏区相交的控件 |
| 6 | `SOUI/src/core/SHostPresenter.cpp:156/162` | 上屏只有一次拷贝：`BitBlt(窗口DC, rcInvalid, memRT, rcInvalid, SRCCOPY)` |

两个结构性事实，是本次全部推理的地基：

- **旧内容消失只能靠第 6 步。** 第 4 步把内存 DC 的脏区擦成全透明，第 6 步要把这层"透明"落到窗口画布上，语义必须是**连 alpha 一起替换**。只要它是叠加，透明位置就等于"什么都不做"，窗口画布上的旧墨原样保留。
- **脏矩形只按控件矩形失效。** 第 5 步的 `DispatchPaint` 只应用显式的 `m_clipRgn`/`m_clipPath`，从不把子窗口裁进它自己的窗口矩形；失效范围（第 2/4 步的那个框）来自控件矩形。所以**画到控件矩形外面的墨，此后再没有任何一帧的脏区会覆盖它** —— 它一旦被画出来就是永久的。

---

## 3. 三个缺陷如何合成那串抛物线

### 缺陷一：`SRCCOPY` 被实现成叠加（present 那一跳）

canvas 2D 的默认 `globalCompositeOperation` 是 `source-over`：源像素透明的位置**保留目标原有像素**。Win32 的 32bpp `BitBlt(SRCCOPY)` 和 cairo 的 `CAIRO_OPERATOR_SOURCE` 都是连 alpha 一起替换。canvas 里没有 `SOURCE` 这个名字，等价物是 `copy`。

后果直白：上一帧的字还在窗口画布上，这一帧内存 DC 对应位置是透明的，`source-over` 让这次上屏"什么都没做"，于是移动的文字在身后拖出残影。

修复（`swinx/src/gdi/canvas2d/gdi.cpp`）：

- `ropComposite()`（:164）把 `SRCCOPY` 映射为 `"copy"`；
- `blitRop()`（:2232）逐次下发 rop，并用 `save → clip(目标矩形) → drawImage → restore` 限制作用范围 —— **`copy` 不裁就等于把矩形外的整块画布一起清空**；
- `rop` 不再是 DC 的持久状态（Win32 里它是逐次调用的参数；留在 ctx 上会让下一次 `TextOut`/`FillRect` 沿用 `SRCAND` 的 `source-in`）；
- `StretchDIBits`（:2335）同一口径 —— 光标/图标加载正是走这个入口的 `SRCCOPY`；
- `ClearRect()`（:2126）同样用 `copy` + 裁剪，"透明底"才真的是透明而不是黑底。

回归用例：`SwinxDcTest.bitblt_srccopy_replaces_destination`（`demos/fun_test/test_dc.cpp:848`）、`SwinxDcTest.stretchdibits_srccopy_replaces_destination`（:921）。两条都先在目标上留一枚金色标记，再用**整块透明**的源以 `SRCCOPY` 盖过去，要求旧标记消失、源里那一枚不透明点到达、矩形外不动 —— 只断言"旧标记消失"会放过"把整块画布擦掉"的实现，所以断言必须成对。

### 缺陷二：裁剪只会累加，替换撤不掉

canvas 的 `clip()` 与既有裁剪**求交**，撤不掉；而第 2 节第 3 行那条每帧路径要的正是**替换**（能放宽）。`SelectClipRgn`、`ExtSelectClipRgn(RGN_COPY)`、`SelectClipRgn(hdc, NULL)` 三个入口在修复前都退化成求交，`GetClipBox` 还会把已经空掉的情况回报成 NULLREGION。

后果：只要有任何一次裁剪泄漏在 `SaveDC/RestoreDC` 配对之外（组件里 `IntersectClipRect` 的写法很常见），累加语义就把裁剪区**永久困在窄框里**，之后每一帧的 `ClearRect` 和上屏拷贝都只能在窄框内起作用 —— 即使 rop 已经是替换也擦不到窄框外面。静态画面看不出这条差别，每帧重画的地方就是致命的。

修复：引入**未裁剪锚点快照**。在"绑定绘制目标且 `SaveDC` 栈为空"时压一次锚点（那一刻 ctx 必然未裁剪：新建 canvas 默认未裁剪，复用同一块 canvas 时先 restore 回上一轮锚点再压新的）；替换语义由 `clipClearCanvas()`（:981）实现 —— 弹掉本 DC 压过的全部 save（锚点 + 每档 SaveDC 各一层），再按同样层数压回，于是栈深不变、与 `SaveDC` 的配对关系不变，而每档快照都回到未裁剪状态。

明确**不能用 `ctx.reset()`**：规范里它除了恢复默认状态还会把画布内容清成透明黑（浏览器实测：填白后 `reset`，`getImageData` 返回 `0,0,0,0`），拿它撤裁剪等于把已画好的内容整幅抹掉，比残影更糟。`canvas.width/height` 被改写时是同一套清空语义，所以 `SConnection::ResizeSurface` 要把锚点标记作废；没有锚点（在 `SaveDC` 嵌套里换绘制目标）时退回累加，与修复前一致、不会更差。

回归用例：`selectcliprgn_replaces_previous_clip`（:1269）、`extselectcliprgn_copy_recovers_from_leaked_clip`（:1308）、`selectcliprgn_null_removes_clip`（:1350），以及应用级复现 `moving_text_after_leaked_clip_leaves_no_residue`（:1604）—— 这条就是"漏出的窄裁剪把之后每帧清屏困住 → 飘字残影留在画布上"。

### 缺陷三：`DrawText` 不裁格式化矩形（墨从哪儿来）

Win32 的 `DrawText` 始终把字形裁进格式化矩形，只有 `DT_NOCLIP` 放开。cairo 有这条判断（`swinx/src/gdi/cairo/drawtext.cpp:183`），apple 也有（`swinx/src/gdi/apple/gdi.cpp:3402`），canvas2d 的 `drawTextLines()` 修复前没有 —— 于是一次 `DrawText` 能把墨画到格式化矩形外，而按第 2 节的结构性事实，那部分墨落在**所有脏矩形都不会覆盖的地方**。

修复（`swinx/src/gdi/canvas2d/gdi.cpp:2811`）：`bool bClip = !(format & DT_NOCLIP)`，为真时在绘制前后成对做 `Op(SWX_SAVE)` / `Op(SWX_RESTORE)` 并缓存/还原 `C2DStateCache` 与 `hdc->rcClip`，用 `clipRectDevice()` 下发裁剪（它负责变换与记账，与同一函数里 `fillRect`/`fillText` 的逻辑坐标口径一致）；`DT_CALCRECT` 分支在裁剪之前就返回，测量不受影响。

回归用例：`drawtext_clips_to_format_rect`（:1438），三条成对断言 —— 窄矩形外的墨必须不存在；换成 `DT_NOCLIP` 后矩形外**必须**有墨（证明窄裁剪不是靠"根本没画出来"蒙过去）；随后一次 `FillRect` 必须还能命中矩形外的区域（证明 `DrawText` 的裁剪没有漏到调用方的 DC 上）。

### 三条的合式（这一节才是"为什么有效"的答案）

> 残影 = 有一格墨落在"以后没人会重画的地方"。

- 缺陷三决定这种墨**会不会产生**（往控件矩形外画）；
- 缺陷二决定每帧的清屏**能不能覆盖到**它（裁剪困在窄框，清屏与上屏都被截断）；
- 缺陷一决定上屏那一跳**能不能把它替换掉**（叠加 = 透明源等于不擦）。

三者是串联关系，缺任意一环都可能继续看到残影 —— 这解释了为什么前几轮修了其中一条仍然被报"还在"，也解释了为什么补齐三条之后应用腿才彻底干净。而"偶发"也顺理成章：需要窄裁剪泄漏与越界墨（或宽脏区）在同一局里按序命中。

---

## 4. 为什么 cairo / Windows / macOS 从来没有这个问题

| 语义 | Win32 | cairo | apple | canvas2d（修复前） |
|---|---|---|---|---|
| `SRCCOPY` | 连 alpha 替换 | `CAIRO_OPERATOR_SOURCE` | — | `source-over`（叠加）**✗** |
| 裁剪替换/移除 | 原生支持 | `ApplyRegion` 先 `cairo_reset_clip` 再裁，天然能放宽 | `CGContextClip` + 状态栈 | 只能求交，撤不掉 **✗** |
| `DrawText` 裁格式化矩形 | 是 | `drawtext.cpp:183` | `gdi.cpp:3402` | 无 **✗** |

三个后端的三条语义都正确，所以同一份 SOUI 代码在它们上面不会出现"擦不回来的墨"。这也是本次判据的来源：**同一批用例跑两个后端，差集就是 canvas2d 的缺陷清单**；修复面因此可以严格限制在 canvas2d，不需要在 SOUI 侧加补丁，也不需要给某个控件特殊处理。

---

## 5. 有效性证据（每条都可复跑）

1. **用例腿 A/B（真 canvas 判像素）**：修复前产物 `demos/fun_test/out.canvas2d.red` 在 `drawtext_clips_to_format_rect` 的"墨越界"断言上失败，修复后 `out.canvas2d` 通过；同一腿里 `DT_NOCLIP` 与"裁剪不漏到调用方 DC"两条断言也成立。更早那批（`stock_objects`、三条 clip 替换、飘字残影）在修复前的 canvas2d 浏览器腿上是 5 条红、修复后全绿，而 cairo 腿修复前后都满通过 —— 说明这批断言测的是**后端差异**，不是用例自身。
2. **证伪（断言不是摆设）**：把 cairo 参考实现的裁剪临时改成 `if (false && ...)`，同一条用例在同一行 `test_dc.cpp:1481` 失败；把 `clipClearCanvas` 换成 `ctx.reset()` 版本重跑，"画布被清空"断言与残影用例全数失败。新用例能在回归引入时失败，不是空跑。
3. **各腿实测**：原生 `ctest -L '^soui-headless$'` 注册 570、实跑 569 全绿；cairo node 腿 88/88；canvas2d node 腿 85/88（3 条失败是 node 里 canvas 被桩件替换、像素不可读的既有登记项：`compatible_bitmap_and_pixels`、`dib_section_setpixel_updates_memory`、`patblt_blackness_whiteness_24bpp`，canvas2d 的像素判定只认浏览器腿）。
4. **应用腿（用户人工，2026-11-06）**：canvas2d 后端俄罗斯方块 1–10 关连测 4 次，得分与升级飘字特效均为**文字形态**，全部无残影。
5. **差分佐证**：同一次消行里"图片形态干净、文字形态残"的历史观察，与缺陷三"只有文字链会往矩形外落墨"完全吻合。

---

## 6. 代价与边界

- **性能**：每次 `SRCCOPY`/`ClearRect` 多一组 `save/clip/restore` 跨语言调用，同机 A/B 记录的帧间隔代价在 +1% 量级（`doc/soui-wasm-port-plan.md` §34）；这是保守裁剪换来的必要开销，不做"能省则省"的优化。
- **`DT_NOCLIP` 仍是放开裁剪的口子**：全仓库只有 `swinx/src/uitools.cpp:1266` 显式使用；SOUI 的文字调用面没有一处传它，所以应用层默认全部受裁剪保护。
- **裁剪只影响"画到控件矩形外"的字**：如果某个控件的窗口矩形本来就小于字形覆盖框，Windows 上同样会缺边 —— 那是 SOUI 侧的布局问题，应该按布局问题处理，**不要把这条裁剪再关掉来掩盖**。
- **后端对称性要求**：canvas2d 与 cairo 必须实现同一 API 面与同一语义；`python swinx/doc/tools/api_scan.py --check` 是本轮的最后一道闸（只动实现，未动声明，清单无需刷新）。

---

## 7. 一次性诊断代码的清理（2026-11-06）

结案后按"留证据、不留噪音"处理，删除的都是为了本次定位临时搭建的对照物：

| 曾加入 | 为什么可以删 | 处理 |
|---|---|---|
| `games.lua` 的 `TETRIS_FLY_IMG` 开关与图片分支 | 文字/图片差分已完成使命，结论落在文字链 | 删除；`tetris_fly_text` 恢复原始签名，文件与诊断前的版本逐字节一致 |
| 特效台页（`page_game.xml` 第 4 页 + `games.lua` 的 `flybench_*` 段） | 静态页在修复前后两个构建上都不复现本 bug，长期维护一个"取键盘焦点 + 舞台"的诊断页没有价值 | 删除；`page_game.xml` 与 `games.lua` 回到只有三个游戏的状态 |
| `demos/wasm-demo/tools/diag/{prescmp,tetris_frames,tetris_play,cliptrace}.cjs` 及 README 对应章节 | 本轮残影专用取证脚本，判据已并入用例与本文 | 删除；`tools/diag/` 回到 11-03 的状态，其余长期验收脚本（`snap_tabs`/`shot_dpr`/`bench_repaint` 等）保留 |
| `demos/fun_test/out.canvas2d.red/`、`/tmp/wasm-demo-red/` | A/B 数据已取完，红构建只是修复前 swinx 的重编产物 | 删除；需要复跑 A/B 时按 §36.4 记录的命令重建（`.gitignore` 已排除 `/out.*/`） |

**刻意保留**的东西：`demos/fun_test/test_dc.cpp` 的 8 条相关用例（其中 5 条就是这三条缺陷的回归网，删掉就等于把修复退回"没有证据"）、wasm `fun_test` 的构建链与 node/浏览器两条运行载体、`bug.png`/`bug2.png` 作为现象证据、`doc/soui-wasm-port-plan.md` §31–§36 的过程记录，以及本文。

清理后复跑（2026-11-06，全部实测）：`luac -p games.lua` 通过；`page_game.xml` 的 `<page>` 回到 3；demo 重链后 `.data` 从 813253 缩到 **801375** 字节，按字符串核对已不含 `TETRIS_FLY_IMG` / `flybench` / `wnd_flybench` / `特效台`；浏览器里 demo 正常启动，控制台依次 `XXL init done` → `tetris: init done` → `tank: init done`，没有 `flybench` 相关行也没有 lua 报错；原生 `ctest -L '^soui-headless$'` 注册 570、实跑 **569 全绿**；`node wasm/run-node.js out.cairo/fun_test-wasm.js` **88/88**、`... out.canvas2d/fun_test-wasm.js` **85/88**（3 条为 node 桩件下像素不可判定的既有登记项）；`python3 swinx/doc/tools/api_scan.py --check` 通过。

---

## 8. 还没有覆盖的

- **Windows / macOS 腿需要人工复跑**：`SwinxDcTest.*clip*` / `*drawtext*` / `*bitblt*` / `*stretchdibits*` 与两条飘字用例。本修复没动这两条链的实现，预期全绿，但按仓库约定这类"平台专属"通过证据只能来自那两个平台本身，不能拿 Linux 的绿色代替。
- **归因粒度**：`bug.png` 那串抛物线最终归到哪一条缺陷，没有做逐条单独的回归实验（三条在同一段周期里依次补齐，且每条都有各自的红→绿用例）。可确定的是：三条补齐后这条链上不存在"擦不回来"的路径，且三条在用例腿上修复前都红过。
- **提交状态**：见 `doc/soui-wasm-port-plan.md` §36.6 与 §37.4 —— 其中记录了主仓库历史里两个本地/远端都不可解析的 swinx 指针（`da85cbe1`、`56b70c82`），检出那两个提交时 `git submodule update` 会失败；要复现修复前的红构建，用能取到的 `swinx 31179e6a`，不要照搬 §36.4 里当时的 `git show HEAD:`。
