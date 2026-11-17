# SoTool / uieditor / fun_test Fluent goal：2026-10-14 验证检查点

**Native goal：BLOCKED，未 complete。** 三个Demo的代码/资源已实装，最新构建、静态、无头与独立GUI回归通过；最终完整应用菜单/工程回合被Windows锁屏阻断。连续三轮goal均以实时foreground/进程映像确认LockApp未解除；2026-10-14已通过native工具标记blocked，不是complete。不可将历史成功或最新部分截图当作最终整套通过。本文文件名保留最初2026-10-12日期，内容已更新至2026-10-14（Asia/Shanghai）。本提交收录已实装的代码、资源和回归；goal仍为BLOCKED，未推送、未发布。

## 目标、范围与交付

以当前dpidemo及 `soui-fluent-demo` 为质量基准：成对浅/深主题、语义色、控件状态、清晰图标、菜单/下拉、合理留白与阴影、必要轻量动效、生命周期与编辑状态保持。遵从用户2026-10-14追加要求，旧UI位图不适合Fluent时用项目 `my-imagegen` 生成并接入真实消费者。

| Demo | 当前实现 | 主要文件/记录 |
|---|---|---|
| SoTool | 七页原树换主题，直接创建树/表头绑定，紧凑输入/组合焦点/选中与占比修正，原生菜单当前主题，7帧新导航 | `demos/SoTool/SoTool.cpp`、`generate_fluent.py`、`FLUENT.md` |
| uieditor | `ue_fluent_*` + `ueFluentShell` 私有壳层，设计/预览隔离，原factory与Scintilla状态保持，私有菜单/下拉/缓存子菜单/浮动主题，inplace生命周期与canonical XML写入修复 | `EditorTheme.h`、`AttrStorage.*`、`EditorPopup.h`、`EditorPropertyGrid.h`、`EditorPropertyValue.h`、`FLUENT.md` |
| fun_test | 五页/原native与GDI诊断保留，Scintilla显示主题、日期时间保持，菜单scale一致，真实12帧菜单新图，FILE/加密ZIP一致 | `test_soui.cpp`、`ScintillaWnd.*`、`generate_fluent_resources.py`、`FLUENT.md` |

只更改三个Demo及相关 `demos/fluent` 辅助/测试；`SOUI/`、`components/`、`controls.extend/`、`utilities/`、`swinx/`、`.github/` 无diff。未改框架公共API/共享系统皮肤，无新增运行时依赖，保留用户原有 `.codex/.omx` 与缓存。37布局、265既有name/id/type条目的兼容契约持续检查；两个 `ue_propgrid` 是明确限定的同基类壳层opt-in。

## 最新实跑证据

| Gate | 结果 | 日志/证据 |
|---|---|---|
| Windows Debug三目标build | SoTool、uieditor、fun_test成功 | `output/tools-fluent-final-assets-build4-20261014.log` |
| 无头CTest | 530注册：529通过、1既有disabled、0失败；476单元/1集成/53无头E2E（含disabled的注册数） | `output/tools-fluent-headless-assets-final-20261014.log` |
| 独立GUI CTest（真实宿主/renderer及失败路径） | 18/18通过 | `output/tools-fluent-gui-assets-final-20261014.log` |
| Demo-local Python静态 | 11脚本，65 tests通过 | `output/tools-fluent-static-assets-final-20261014.log` |
| GUI helper单测 | 81 tests通过；PID/foreground、模态交接、唯一popup及隐藏console | `output/tools-fluent-helper-assets-final-20261014.log` |
| 资源audit | SoTool97索引/40skins，uieditor128/39，fun_test85/34；配对浅深一致，全部0 errors/0 warnings | `output/tools-fluent-resource-assets-final-20261014.json` |
| FILE/ZIP parity | 101总条目/90加密文件，`souizip`解密后与FILE集合及每个文件字节一致 | 同上；真实7-Zip ZipCrypto打包 |
| 生成可复现 | SoTool101资源/生成文件，fun_test90资源文件重跑哈希相同 | `output/tools-fluent-assets-idempotence-20261014.json` |
| diff/AST/范围 | CRLF-aware diff check及三个GUI runner AST通过；框架/子模块无diff；没有遗留测试进程 | `output/tools-fluent-diff-check-assets-final-20261014.log` |

禁用项 `swinx_misc.is_dbcs_lead_byte_codepage_dependent` 不是本轮新增、没有计为通过。通用audit不识别UIEditor私有引用，本地private契约tests补足。生成可复现只排除真实uiresbuilder的idx-mtime `//stamp`，不排除生成表内容；ZIP随机加密头不要求整个archive字节相同。

构建需要本机既有ATL workaround，未修改源码/CMake工具链：

```powershell
$atl='C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Tools/MSVC/14.16.27023/atlmfc'
$env:_CL_="/I`"$atl/include`""
$env:_LINK_="/LIBPATH:`"$atl/lib/x64`""
cmake --build build --config Debug --target SoTool uieditor fun_test --parallel 4
ctest --test-dir build -C Debug -L '^soui-headless$' --output-on-failure
ctest --test-dir build -C Debug -L '^soui-gui$' --output-on-failure
```

构建与测试分开证明，`-C Debug` 对当前VS多配置树是必要参数。新回归曾先因丢失仍需的Option类include构建失败、以及同一row的SetCurSel返回FALSE而失败，均读取日志后修正，再取得表中最终证据；未把失败当通过。

## my-imagegen 生成与实装

使用指定skill直接调用SDK，模型 `gpt-image-2`，不增加CLI/runner，不改凭证，不记录密钥。原始大图/prompt/response保留于忽略的 `output/imagegen/tools-fluent-20261014/`，交付母版和无密钥provenance在demo-local `fluent_assets/`。

| 使用点 | 母版 | 交付图集 | 原逻辑geometry/顺序 |
|---|---|---|---|
| SoTool `tab_main → fluent_page_icons` | 1484×192，4× | 配对742×96，scale200 | 7帧，每帧53×48；图片/代码/编码/目录/MD5/Spy++/关于 |
| fun_test原生及扩展 `skin_small_icons` | 864×72，4× | 配对432×36，scale200 | 12帧，每帧18×18；网站/帮助/退出/关于等原帧索引保持 |

以实际返回尺寸切格、native alpha裁切/等比居中；去掉微弱透明噪点与缩小ringing，透明间隔避免跨帧污染。原图RGB中透明区域的光晕没有被装成不透明背景。浅深背景、100/200%尺寸的归一化预览及新SoTool实际窗口均已看图；fun_test真实原生菜单对象在重复主题/scale100/150/200下解析到新图集，源scale不会撑大菜单。

**未安装最初误选的9帧fun_test参考图**：它不被实际菜单消费。UIEditor已适配的私有SVG/功能图标、品牌、技术诊断/文档图保留，不为三个demo都“有生图”而添加无用插画。离线generators从选定母版重建，不会回退为旧tint。

素材契约真实红→绿：`output/tools-fluent-assets-red-20261014.log` → `tools-fluent-assets-green-20261014.log`。独立Kepler只读Review确认真实消费者、7/12帧/尺寸、provenance及90文件ZIP parity，无确证P1/P2；不将静态Review当作GUI运行。

## UIEditor 生命周期/文档安全闭环

旧dump `uieditor.exe.52020.dmp` 的RVA0x2cfc4c定位至 `SWindow::OnContainerChanged:4435/GetCapture`；rax为freed-memory哨兵。异常寄存器和候选返回地址符号化，加上源码生命周期验证支持：focused inplace失焦同步销毁，外层SetContainer仍继续遍历。不是正规的完整dump stack unwind，也不是旧ExitMenu假设。

- demo-local `ue_propgrid` 在旧容器子树遍历前结束属于当前inplace的焦点；保留真实文本提交/option取消、选中行及销毁行为，不保活子节点掩盖UAF。AV红日志 `output/tools-fluent-inplace-red2.log`。
- option显示label仅在真实inplace失焦链内规范化。插入对话框共用canonical XML writer，不再对值二次Option2Value；真实事件消费者序列化/重解析 `1:0|0:1` 两方向选择与失焦结果。
- XML no-op guard阻止focus-only触发保存/dirty/undo/选区变化。
- screenshot写入open/write/flush/close及重读字节检查；invalid path、locked stale file、short overwrite、NUL失败用例通过。失效/旧图不能作为当次证据。
- Sagan独立Review提出的两项P2已逐项修复并复核关闭，最终限定复核未发现新的确证P1/P2。最终18/18 GUI覆盖上述回归。

## 当前真实GUI：最新成功、部分失败、历史成功分开

### 最新主窗三态成功（已看图）

- SoTool：`output/remaining-fluent/SoTool_assets-main-20261014_20261014_031550_9785e68b/evidence.json`，新导航实装、浅深浅及几何。
- UIEditor：`output/remaining-fluent/uieditor_assets-shell-20261014_20261014_031458_7544c2fb/evidence.json`，独立runtime/Config中的空壳，125%/120DPI、浅深浅及几何；不当作loaded-project验证。
- fun_test：`output/remaining-fluent/fun_test_assets-main-20261014_20261014_031555_9e9c0550/evidence.json`，浅深浅及几何；不当作menu验证。
- 当前GUI用例Skia dropdown/float bitmap已落盘并看图，汇总 `output/tools-fluent-fresh-implemented-20261014.png`；renderer cache不证明桌面合成阴影。

### 最新完整应用回合没有通过

- SoTool `output/sotool-assets-gui-20261014.log`：`sotool-business-79432503/` 中MD5/scan/焦点图可用，但async代码统计前的激活被拒绝，未到最终menu/restore gate。
- fun_test `output/funtest-assets-gui-20261014.log`、`funtest-editing-f3673320/failure.json`：Scintilla6472→6493、选区5..15、dirty/undo三态相同且undo/redo已检查；native menu之前激活被拒绝，总体failure。
- UIEditor `output/uieditor-final-gui-20261014.log` 先前File menu visible[]；最新诊断解释了同一桌面环境后续native-menu运行失败的前台限制，**未宣称已证明每次旧visible[]失败的唯一根因**。增强脚本增加redock-state与失败证据，尚待最终完整通过。

前台实际为 `LockApp`（PID18612），正常目标host SetForegroundWindow返回False。QA只请求一次且确认HWND/PID；不使用全局SendInput/clipboard，不AttachThreadInput、不关闭LockApp、不解锁或绕过安全桌面，不删除唯一popup gate、不盲目重试。

### 历史完整成功（只证明对应版本）

- SoTool `sotool-business-6ddd17ee/evidence.json`：MD5正确、非空scan、代码统计、filetype menu、max/restore。
- UIEditor `uieditor-project-26cb73f1/state.json`：223保存字节（含BOM）只含授权编辑，三态编辑状态相同，作者ROI `[821,317,1181,557]` 完全一致，menu/option/float/redock/restore/max成功。早于本轮消费者/截图失败回归及增强state脚本。
- fun_test `funtest-editing-ba634bdc/evidence.json`：编辑/undo/redo及native/extended menus成功，早于本轮12帧素材。

## Completion audit / 剩余必要步骤

| 验收项 | 状态 |
|---|---|
| 三目标原功能ID、内容/布局尺寸、资源提供器及编译 | 已有静态/动态/构建证据 |
| 成对主题、延迟对象、private editor隔离、编辑值/状态/生命周期 | 静态、E2E、18实际GUI回归与对应版本完整窗口证据 |
| 新生成素材进真实索引/皮肤/消费对象，可复现与FILE/ZIP | 已有实装、红绿、runtime绑定、看图与打包证据 |
| 最新主窗浅深浅、当前renderer dropdown/float | 通过，实图已看；覆盖范围如上 |
| **最终完整native/extended菜单与加载工程增强脚本复跑** | **等待桌面解锁；不可宣称goal complete** |

解锁桌面并由用户恢复goal后的下一步（agent自行执行，无需再请求普通操作许可）：依次运行下面三个脚本，检查新的JSON/截图并作visual verdict；重点fun_test新12帧菜单、UIEditorredock状态/插入option实际交互。不要并行操作前台窗口。若均通过，更新三份FLUENT.md/此记录和goal状态；若发现确证产品问题，修复后重新跑受影响的验证。

```powershell
python -B demos/SoTool/verify_fluent_business_gui.py
python -B demos/uieditor/verify_fluent_state_gui.py
python -B demos/fun_test/verify_fluent_editing_gui.py
```

UIEditor固定坐标脚本限定1920×1080/125%，在独立project/Config/runtime中运行。当前SoTool/fun_test沿用DPI-unaware：125%桌面采图仍为96DPI/逻辑100%，内部scale100/150/200不等于真实设备多屏。非Windows、所有拖放/错误/键盘/语言切换/辅助技术、桌面合成阴影和全部业务未穷尽；退出fade/hover渐变未实现，不作完成声明。

## 最终阻塞交接（2026-10-14）

实时证据 `output/tools-fluent-locked-desktop-audit-turn3-20261014.json`，前台LockApp/PID18612仍在；没有遗留Demo/test进程或待轮询的构建任务。当前唯一必须完成的最后门禁是三份完整GUI脚本的新菜单/加载项目/增强redock状态回合。Native goal工具已返回blocked；不伪装为verified wait，不缩小目标、不重复无效运行。用户解锁Windows桌面并恢复goal后继续原目标，恢复时若仍被阻塞则按规则重新开始三轮审计。

## 2026-10-14 本地提交前复验

用户已明确要求本地提交；仅提交三个 Demo 与四个相关 `demos/fluent` 交付文件，不推送远端。Native goal 仍为 BLOCKED，不以本地提交替代最终完整 GUI 门禁。

- 精确暂存 330 个交付文件；未收录 `.codex/`、`.omx/`、`__pycache__/`、构建制品、截图或运行日志。
- 暂存检查发现三份 UIEditor Python 脚本的末尾空行/行尾空格，已仅清理空白；CRLF-aware `git diff --cached --check` 通过。
- 复跑 `ctest --test-dir build -C Debug -L '^soui-headless$' --output-on-failure`：530 注册，529 通过、1 既有 disabled、0 失败。证据：`output/tools-fluent-commit-headless-20261014.log`。
- 复跑 11 个 Demo-local Python 脚本的 65 项测试及 GUI helper 的 81 项测试，全部通过。证据：`output/tools-fluent-commit-python-20261014.log`。
- 三份 Debug EXE、两个 my-imagegen 母版与 fun_test ZIP 的 SHA-256 均与最终已验证制品一致；两个无密钥 provenance 的母版哈希匹配。
- 再次验证 fun_test ZIP：101 个条目、90 个加密文件，全部与 FILE 资源逐字节一致；暂存文件未发现私钥/API-token 格式内容。
- 本次提交未重复运行最终完整应用菜单/加载项目/增强 redock GUI 回合；其锁屏验证缺口保持不变。既有独立 GUI 回归 18/18 证据仍适用于哈希一致的制品。
