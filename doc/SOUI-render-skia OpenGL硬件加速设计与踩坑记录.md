# render-skia 模块 OpenGL 硬件加速：设计与踩坑记录

本文记录 `components/render-skia` OpenGL 硬件加速渲染目标（GPU RT）的设计实现、关键决策依据，以及开发过程中踩到的坑。适用版本：SOUI 4 + skia m47。

## 1. 背景与目标

render-skia 原本只使用 skia 软件光栅化（`SRenderTarget_Skia`，绘制直接落在 CPU 位图上）。本次改造目标：

- 非透明窗口的渲染目标改用 skia 的 OpenGL 后端（`SkSurface::NewRenderTarget` + `GrContext`），绘制在 GPU FBO 上完成；
- **不改变 SOUI 内核的脏矩形渲染契约与呈现链路**；
- GPU 初始化失败时自动回退软件渲染，行为等价；
- 提供运行期开关，可禁用 GPU 回到原有路径。

约束：透明（分层）窗口继续走软件 RT（`CreateRenderTarget`），只有非透明窗口的 `CreateRenderTarget2` 走 GPU RT。

## 2. 总体架构

### 2.1 双面渲染目标：GPU surface + CPU 权威位图

`SRenderTarget_SkiaGL` 内部同时持有两份像素：

```
                 BeginDraw(增量上传)                EndDraw(脏区回读)
  CPU 位图(m_curBmp) ──────────────────► GPU SkSurface ──────────────────► CPU 位图
  （唯一权威画面）                        （仅脏区内是新内容）                 （呈现源）
```

- **CPU 位图是唯一权威画面**。呈现（`SHostPresenter::OnHostPresent` → `GetDC(1)` + `BitBlt`）、GDI 互操作、`BitBlt/GetPixel` 等所有旧接口仍然只读 CPU 位图，对内核与呈现层完全透明；
- GPU surface 只承担"脏区内的绘制加速"，每帧生命周期为：

```
BeginDraw：把上一会话改动区域(m_rgnDrawPrev)从 CPU 上传到 GPU（首帧/Resize 后整图上传）
PushClip*：canvas 裁剪生效，同时把 getTotalClip() 精确并入 m_rgnDraw
绘制阶段：ClearRect 清空脏区 → 控件重绘（全部被裁剪限制在脏区内）
EndDraw ：flush 后只回读 m_rgnDraw 覆盖的矩形到 CPU 位图；记录 m_rgnDrawPrev = m_rgnDraw
```

### 2.2 为什么必须"回读"，不能让 GPU surface 直接呈现

老版 skia（m47）在 macOS 上存在 stencil 裁剪泄漏缺陷（见踩坑一）：GPU surface 上会多出脏区边界外的杂散像素。只要呈现内容严格来自"按裁剪范围回读后的 CPU 位图"，这些杂散像素就永远进不了最终画面。这是整个设计的核心防御。

### 2.3 裁剪记录：`m_rgnDraw` 的语义

`m_rgnDraw` = 本绘制会话中所有裁剪状态覆盖区域的并集，即"GPU 上可能被改动的像素范围"。回读用它限定范围：

- `PushClipRect/PushClipRegion`：调 base（save + clipXxx）后取 `SkCanvas::getTotalClip()` 精确并入。**不能用 Push 的矩形直接做并集**——必须尊重裁剪模式语义（见踩坑三）；
- `ClearRect`：清空同样受当前裁剪约束，也并入 `getTotalClip()`；
- `SaveClip/RestoreClip/PopClip/ExcludeClipRect`：只缩小或恢复裁剪，绘制 ⊆ 已记录范围，无需处理。

该记录方式满足两条不变式：**实际绘制 ⊆ m_rgnDraw**（不漏回读）；**m_rgnDraw ⊆ 各裁剪状态并集**（不多回读）。

### 2.4 GL 环境单例：SGPURenderEnv

- 进程级单例，封装平台 GL 上下文（macOS: CGL 离屏上下文；Windows: WGL；Linux: GLX；移动端: EGL）与 `GrContext`；
- 惰性初始化：首次 `CreateRenderTarget2` 才创建上下文；
- 生命周期与 `SRenderFactory_Skia` 绑定（构造 Acquire / 析构 Release），归零时按"先 unref GrContext、后销毁 GL 上下文"的顺序释放；
- 隐含约束：GPU RT 的存活期必须短于 render factory（SOUI 中 factory 由 `SApplication` 持有至进程结束，天然满足）。

### 2.5 开关与回退

- 导出函数 `Render_Skia_EnableGPU(BOOL)` 运行期开关；`s_bSkiaEnableGPU` 默认 TRUE（若希望默认保持 CPU 渲染、显式开启，改默认值即可）；
- `CreateRenderTarget2` 中 GPU 创建失败自动回退 `CreateRenderTarget` 软件路径；
- `Resize` 中 GPU surface 重建失败回退光栅 `SkCanvas`（画在 CPU 位图上）。

## 3. 性能设计

| 环节 | 策略 | 依据 |
|---|---|---|
| BeginDraw 上传 | 增量：只上传 `m_rgnDrawPrev`（上一会话脏区）；首帧/Resize 后整图 | 脏区绘制前被 `ClearRect(kSrc)` 清空，不依赖 GPU 旧内容；脏区外 GPU 与 CPU 一致（上次同步结果） |
| EndDraw 回读 | 脏区子矩形逐块 `readPixels(info,dst,rowBytes,x,y)` | 回读范围 = 裁剪并集，杂散像素不进 CPU 位图 |
| GDI 互操作（DrawIconEx 等） | `GetDC(0)` 先 syncToBitmap，GDI 在共享内存上绘制，`ReleaseDC` 只回传 `m_rgnDraw` | base `GetDC` 把 canvas 裁剪同步到了 GDI DC，GDI 绘制范围 ⊆ `m_rgnDraw` |
| Push/ClearRect 记录成本 | `getTotalClip()` 返回 SkRegion 值拷贝，每会话调用次数 = Push 次数 | 每控件 1~2 次，可忽略 |

每帧固定开销为一次 flush + 脏区面积的 GPU↔CPU 拷贝；静止界面无脏区时几乎零开销。

## 4. 踩坑记录

### 坑一：m47 skia 在 macOS 上 stencil 裁剪把像素画到裁剪区外

**现象**：GPU 模式局部刷新后，脏区边界外出现填充色条带（横向 ≥8px），"脏区外画面丢失"。

**定位过程**（独立诊断程序，CGL + GrContext 直连 skia，逐帧排除）：

| 实验 | 内容 | 结果 |
|---|---|---|
| 1 | `writePixels` 上传 + 多矩形区域裁剪绘制 + 全图回读 | FAIL（泄漏像素与"裁剪区外应为基图"的期望不符） |
| 2 | 同上，上传改 `drawBitmap` | FAIL → 与上传方式无关 |
| 3 | 单矩形裁剪（scissor 路径） | PASS → 泄漏只发生在多矩形 stencil 路径 |
| 4 | 只上传+回读，不做裁剪绘制 | PASS → 上传/回读管线本身正确 |
| 5 | L 形区域 + 上传后 flush，统计泄漏像素 | FAIL → 泄漏沿脏矩形右边缘形成 ≥8px 条带 |
| 6 | 照常 stencil 绘制，但只对脏区矩形做子矩形回读 | 30 帧 PASS |

**结论**：m47 的 `GrGpuGL` 在多矩形 `SkRegion` 走 stencil 裁剪时存在缺陷，写入超出裁剪边界。与上传方式无关、加 flush 无效。**修复**：GPU 照常绘制，回读严格限定在裁剪并集内（2.3 节）。

### 坑二：m47 `SkBitmap` 拷贝后 `getPixels()` 返回 NULL

**现象**：修复过程中 demo 在 `GetDC(0)`（图标绘制路径）崩溃，SIGSEGV at `0x37a0`。

**根因**（m47 `SkBitmap` 语义，极易踩）：

- `setPixels()` 结束时会立即 `lockPixels()`，因此**原对象**的 `getPixels()` 永久有效；
- 但 `SkBitmap` 的**赋值拷贝**在 `memcpy` 后强制 `fPixelLockCount=0; fPixels=NULL`（`updatePixelsFromRef` 只在加锁后重算）；`isNull()` 只查 `fPixelRef`，拦不住这种情况；
- 于是 `SkBitmap bmp = xxx.GetSkBitmap()` 后 `bmp.getPixels() == NULL`，手工计算 `NULL + fTop*rowBytes + fLeft*4` 得到 `0x37a0`（崩溃地址 2×7064+28×4=14240，精确吻合）。

**为什么旧代码不崩**：全图 `readPixels(&bmp,0,0)` 变体内部先 `SkAutoLockPixels` 再取指针，遇到 NULL 安全返回（静默失败）。

**修复**：持有 `const SkBitmap &` 引用（与全仓库其他用法一致），并加 `getPixels()` 判空防御。

**启示**：m47 里凡是从 SkBitmap 拷贝取像素，要么先 `lockPixels()`，要么引用原对象。

### 坑三：回读范围记录必须尊重裁剪模式语义

**现象**：脏区子矩形回读上线后，局部刷新仍然"老样子"丢画面。

**根因**：初版 `PushClipRect` 无条件把 Push 的矩形并集进回读范围。但 SOUI 裁剪有三种模式：

- `RGN_AND`（交集）：控件裁剪 = 脏区 ∩ 控件矩形，记录整个控件矩形会多记控件超出脏区的部分；
- `RGN_DIFF`（挖除）：缓存绘制窗口（`IsDrawToCache`）和带 NC 边框的窗口每帧都会 `PushClipRect(rcClient, RGN_DIFF)` 挖掉客户区再画边框，把挖掉的矩形并进回读范围等于把"未重绘区域"纳入回读。

后果链：回读范围 ⊃ 实际绘制范围 → GPU 上"未重绘 + stencil 泄漏"的内容被读回 CPU 位图 → 呈现上屏。

**修复**：Push/ClearRect 后直接取 `SkCanvas::getTotalClip()`（canvas 维护的精确当前裁剪，COPY/AND/DIFF 语义自动正确）并入记录。`RGN_DIFF` 后新裁剪 ⊆ 原裁剪 ⊆ 已记录范围，并集不变，天然正确。

### 坑四：Mac 驱动对 FBO attached 纹理的 glTexImage2D 有缺陷

skia m47 上游（`GrContext.cpp` 注释）明确说明：Mac 平台禁用了对 FBO attached 纹理的直接写入路径。因此 GPU RT 的像素更新必须走 `writePixels`（纹理上传）+ `readPixels`（回读），不能假设直写纹理可用。这也是本设计采用"上传/回读"而非"直写"的客观背景。

### 坑五：BeginDraw/EndDraw 会嵌套——嵌套会话把"上一帧内容"回传上来，造成旧 item 状态残留

**现象**：`demo` 与 `uieditor` 里，鼠标从一个 item 移到另一个 item 后，**前一个 item 的 hover/选中状态不消失**（treeview、tileview 都能复现），越移越多。把 `s_bSkiaEnableGPU` 关掉（软件 RT）立刻正常。

**根因**：SOUI 的绘制不是严格"一帧一开一合"，绘制过程中可能再次进入 `_RedrawRegion`（动画回调里的 `Update()` → `::UpdateWindow` 同步触发 WM_PAINT 等），于是同一个 RT 上出现 `BeginDraw` 嵌套。而 `SRenderTarget_SkiaGL` 的会话状态是单份的：

- 嵌套的 `BeginDraw` 会 `m_rgnDraw.setEmpty()` → 外层此前已绘制的区域从脏区里消失，外层 `EndDraw` 不再回读它；
- 更致命的是嵌套的 `BeginDraw` 会 `uploadRegion(m_rgnDrawPrev)`，把**上一帧的 CPU 位图内容**覆盖到 GPU 上，正好把外层本次刚画好的"取消 hover"像素改回"仍是 hover"的旧像素。

于是屏幕（由 CPU 位图呈现）上旧 item 一直保持 hover。软件 RT 的 `BeginDraw/EndDraw` 是空实现，所以只有开 GPU 才暴露。

**定位方法**：临时在 `BeginDraw/EndDraw` 里全表面回读并与 CPU 位图逐像素比对。修复前日志显示 127 次 BeginDraw 中有 60 次嵌套，且 `preUpload/postDownload` 持续出现数千像素差异；修复后 70 次会话全部 `identical`。

**修复**：加嵌套深度计数 `m_nDrawDepth`：只有最外层 `BeginDraw` 才重置脏区并上传上一帧脏区；嵌套 `EndDraw` 只出栈，不下载、不改 `m_rgnDrawPrev`，由最外层统一回读与呈现。脏区在外层与嵌套会话之间自然累积，因此回读范围仍是并集，正确性不受影响。

**启示**：凡是"会话式"的 RT 状态（脏区、上传/下载、SaveClip 计数），都必须按嵌套深度管理，不能假设 BeginDraw/EndDraw 一一配对。

## 5. 验证方式

- **单元级诊断**：`build/diag/diag_skia_gpu.cpp`（临时诊断程序，未纳入仓库）以纯 skia 层面验证 frame1~11 各场景，用于坑一的定位与修复方案模拟；
- **无头回归**：`ctest --test-dir build -L '^soui-headless$'`，424 个用例（软件渲染路径）100% 通过，GPU 改动未影响 CPU 渲染；
- **GPU/CPU 一致性自检**：在 `BeginDraw/EndDraw` 处临时全表面回读并与 CPU 影子位图逐像素比对，可精确断言"呈现内容与绘制内容一致"（定位坑五即用此法，详见该节）；
- **人工验证**：macOS demo 启动、局部刷新、图标绘制路径（GDI 互操作）正常。

## 6. 已知限制

- stencil 泄漏是 m47 上游缺陷，本方案仅保证泄漏不进入呈现；GPU surface 内部残留像素会被后续增量上传逐步覆盖。升级 skia 版本后可重新评估是否可以放宽回读范围（例如整图回读）；
- Resize 后 GPU surface 重建失败的回退路径（光栅 canvas）未在真机覆盖；
- 多矩形脏区逐矩形 `readPixels` 每次有固定管线开销，正确性优先；
- 透明（分层）窗口仍走软件 RT，未纳入 GPU 加速。
