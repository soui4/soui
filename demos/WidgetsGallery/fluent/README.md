# 13 demo Fluent 统一交接（2026-10-12；保留历史记录）

## 2026-10-14 阴影补充

按当前 dpidemo（不是历史配方）统一32张菜单/下拉明暗阴影，窗口/菜单共58张资源逐像素对照通过；已有主窗PNG一致，未改变原NC/non-layered模式及原生菜单。fun_test提供`--shadows-only`以保留私有palette/icons并同步加密ZIP。最新范围、实际构建/测试/截图与nativepixels未验证边界见[阴影验收记录](SHADOWS.md)。下方2026-10-12内容仍为历史批次。

## 2026-10-12 Chrome 最终验收

本轮实现与 Windows x64 Debug 适用验收已闭环。最新 521 注册/520 执行、55 定向、13 GUI、108 Python、构建及视觉证据见 [统一验收记录](GOAL_VERIFICATION_20261012.md)；[新 manifest](../../output/remaining-fluent/evidence/manifest-20261012-goal-final.json) 收录最终源码、15 EXE 和截图/日志 SHA-256。Native composed-pixel 仍 unavailable，不宣称全平台/全矩阵/全像素通过。下方正文及各目录保留的 **2026-10-11 数据、通过声明和旧 manifest 均属历史批次，不是最新结果**。最新最终数字、命令、证据与未覆盖范围只以主线程维护的 [统一验收记录](GOAL_VERIFICATION_20261012.md) 为准；本页仅列摘要；最终统计、实际命令与详细边界以统一记录为准。

- **主窗边界：** `WindowChrome` 仅适用于既有 layered 主窗：demo、wxDemo、flowlayout_demo、framelayout_demo、hellocharts_demo、mclistview_demo、propanimator_demo、wallpaper 选择器及 IPC client/server 两窗。demo2/gridctrl_demo/lightbutton_demo/sci_demo 保留原 NC/non-layered 模式，不为阴影强改窗口模式。
- **几何与单位：** 保留各 demo 原默认单位；原 padding 按真实 `SLayoutSize`（含 px/dp 混合）留存。主窗 gutter 为左/上/右/下 **18/12/18/24dp**，只在 content 外加一次，重复绑定/主题刷新不累计扩张；最大化撤除 gutter，还原恢复。
- **动态弹层：** 所有 `Theme::Bind(host)` 均接入共享 `PopupOwner` attribute factory，覆盖建树后动态创建的 Combo。已挂接 popup 清除旧 `ncSkin` / `uniformAlpha` 路径，使用 per-pixel 表面；这不改变上述四个主窗模式。
- **专有接线：** demo tray、demo2（经 C bridge）、wx emoji 的 MenuEx 按目标 scale 按原资源单位测量并保留固定 body 宽度，gutter 外加，不挤 body、不偏移 anchor。MC 搜索使用真实 buddy 的屏幕矩形，显式 `keyboard=true`，打字查询立即呈现，不逐次播放入场。
- **动效边界：** 共享层只拥有 popup 的 owned enter：**150ms、显式 4dp、cubic-bezier(0.23, 1, 0.32, 1)**；已有动画优先，键盘/减少动态效果路径跳过 owned enter。原主窗动画保留，popup 关闭/命令立即退出；不承诺额外 fade/hover，未迁移的 dateTimePicker/native popup 不纳入 Chrome 完成声明。
- **资源与回归：** 只追加 **44 PNG（18 window / 26 popup）** 及相应 shadow skin/PNG 索引，不覆盖原 tokens。共享 native 回归源码已覆盖 repeat、max/restore、DPI、handler、destruction、动态 Combo；主线程 fresh 验收已完成，详见统一记录。`nativepixels` 当前 **unavailable**，native 行为测试通过不等于 pixel verified，不写成像素 PASS。

Chrome 资源使用增量 generator，再按目录运行真实 builder（下面是复现命令，不是本 writer 已执行记录）：

~~~powershell
python -B demos/fluent/generate_chrome_resources.py --check
python -B demos/fluent/generate_chrome_resources.py --demo demo
python -B demos/fluent/build_resources.py --demo demo
~~~

资源入口现在向 fresh 临时路径调用真实 builder，绕过未计入 idx 的旧 header 缓存；两轮26产物字节相同、13份完整 `_R` 定义文本保持。builder失败/缺输出不替换既有产物，不承诺写入失败时跨文件事务回滚。

**不要重跑全局 `generate_resources.py` 套模板**，以免覆盖各 demo 的定制 palette/tokens。以下历史正文原样保留，不替代本次统一验收记录。

本轮将下列 13 个 demo 目录的展示层统一为默认浅色、运行时可往返深色的 Fluent 外观；IPC 目录有两个 executable，因此是 **14 个 GUI target**。保留原演示内容、事件、name/id、adapter、资源入口、尺寸单位及业务边界，不新增运行时依赖、不修改框架公共 API。各目录的 `FLUENT.md` 保留架构、实施决策和历史阶段记录；其中旧 slice 的“未构建/未跑 GUI”不能当作本轮最新状态。

本页只记主线程已确认且有对应记录的事实。**照片修复后的 fresh 构建、CTest、Fluent 定向测试、静态检查及 GUI 工具单测均已通过。** [统一 manifest](../../output/remaining-fluent/evidence/manifest.json) 已生成，收录 12 个单目标 canonical、IPC 双窗口、8 个代表性附加 run，以及实际截图/日志 SHA-256 索引。 既有 GUI 批次与 fresh 构建/测试是不同证据，不以同名日志、截图目录名或构建成功推断某页面已重新实测。

## 1. 范围与原单位

| 目录 / 详细记录 | GUI target | 原默认单位 | 保留边界 |
| --- | --- | --- | --- |
| [demo](../demo/FLUENT.md) | `demo` | dp | 主演示页面、背景选择、HSL 派生及原持久化；明暗选择不持久化 |
| [demo2](../demo2/FLUENT.md) | `demo2` | px | 纯 C COM、C 控件/adapter、原 NC、Menu/MenuEx 和 modal |
| [wxDemo](../wxDemo/FLUENT.md) | `wxDemo` | px | SOUI 聊天示例，非 wxWidgets；会话/草稿/媒体及原语言资源入口 |
| [flowlayout_demo](../flowlayout_demo/FLUENT.md) | `flowlayout_demo` | dp | 三页流式布局、勾选/选择和 SVG 示例 |
| [framelayout_demo](../framelayout_demo/FLUENT.md) | `framelayout_demo` | dp | frame/dock、拖动、缩放及原 dock 状态 |
| [gridctrl_demo](../gridctrl_demo/FLUENT.md) | `gridctrl_demo` | px | 原网格数据、编辑器、选择/合并/排序及行列尺寸 |
| [hellocharts_demo](../hellocharts_demo/FLUENT.md) | `hellocharts_demo` | px | 六页图表、数值/数据色、算法及原动画配置 |
| [lightbutton_demo](../lightbutton_demo/FLUENT.md) | `lightbutton_demo` | px | 五个原发光控件、预设/形状/高光/报警；不补不存在的 lightbutton5 |
| [mclistview_demo](../mclistview_demo/FLUENT.md) | `mclistview_demo` | dp | 分组/勾选/搜索/记录 overlay、原显式缩放；不新增排序或短信发送 |
| [propanimator_demo](../propanimator_demo/FLUENT.md) | `propanimator_demo` | px | 三页锚点/布局/动画集及原参数、数据色 |
| [sci_demo](../sci_demo/FLUENT.md) | `sci_demo` | dp | 无 HWND 的 Scintilla DUI；纯文本缺省 lexer、文档及编辑状态 |
| [wallpaper](../wallpaper/FLUENT.md) | `wallpaper` | px | 原分类、异步下载/解码、虚拟行和缓存；不新增预览/设置壁纸 |
| [ipcdemo](../ipcdemo/FLUENT.md) | `ipc_client` / `ipc_server` | px | 原协议身份 `sipc_demo_svr`、真实调用、日志和 Sum 上限 |

**主 demo、flow、frame、MC、SCI 保持 dp，其余保持 px。** 共享资源生成器只物化 palette/SVG/索引，不把原布局整体改为 dp；原显式单位、内容几何与数据颜色仍按各目录契约保留。XML 内容声明、窗口外框和 OS 缩放后的屏幕像素不能混为一个尺寸。

## 2. 主题接线与刷新契约

共享实现是 [FluentTheme.h](FluentTheme.h)，成对资源来自各 demo 自有的 `values/fluent_light.xml` / `fluent_dark.xml` 与 `theme_color_light.xml` / `theme_color_dark.xml`，不修改共享系统皮肤。

1. 在建树前 `Theme.Load()` 加载并安装浅色颜色域/skin pool；建树后、消息循环前 `Theme.Bind(root)` 绑定 `fluent_theme_toggle`。主题按钮使用独立 ID 65000，不挤占原自动 ID。`Theme.Switch()` 原位换色，不重建控件树或换 adapter。
2. attribute factory **组合原 factory/delegate**，保留原属性副作用、读取优先级和失败契约；记录 `@color/...` 与皮肤名，字面量覆盖后移除旧 token，不按旧 RGB 全树替换。
3. 独立颜色域与 skin pool 按生命周期配对 push/pop；按名重取皮肤、重放 token、派发同尺度 `UM_SETSCALE`，刷新 cache、表头和脱离普通子树的可见虚拟 item panel。窗口销毁解除订阅；析构仅在仍持有当前 hook 时恢复原 factory。
4. Fluent 表单编辑器按自身焦点事件刷新 focus skin；无边框聊天编辑器保留其输入外观。既有 UI tint 在 skin 更换后重放；照片/头像不据正文或 accent 颜色重色，相关 image skin 的 `enableColorize` 边界单独回归。
5. 特例由 demo-local 接线处理：demo2 用 opaque extern-C bridge 与 NC 刷新；主 demo 保留 HSL/背景语义域；图表只改 presentation 与本目标 renderer 色彩覆盖；Grid 用本地派生控件；SCI 更新原 engine 样式/真实 caret；wallpaper 在同尺度刷新时不重绑图片数据。详见各目录文档，不推断所有特殊控件自动获全量覆盖。

明暗选择默认不持久化。主 demo 原 HSL/背景的保存策略是另一条保留契约；“非默认 saved HSL 恢复通过”不等于 GUI Save 全链路已经测完。

## 3. 复现入口（PowerShell）

以下命令供复现使用；本轮已执行的构建和 GUI 结果见第 4 节及对应证据。当前 Windows 构建树为 Visual Studio 17 2022 / x64 / Debug，运行文件在 `build/bin/Debug`。从仓库根开始：

```powershell
$repo = 'F:/OpenSources/soui4_remaker/soui5-fluent'
Set-Location -LiteralPath $repo
$demos = @('demo','demo2','wxDemo','flowlayout_demo','framelayout_demo',
    'gridctrl_demo','hellocharts_demo','lightbutton_demo','mclistview_demo',
    'propanimator_demo','sci_demo','wallpaper','ipcdemo')
$targets = @('demo','demo2','wxDemo','flowlayout_demo','framelayout_demo',
    'gridctrl_demo','hellocharts_demo','lightbutton_demo','mclistview_demo',
    'propanimator_demo','sci_demo','wallpaper','ipc_client','ipc_server')
```

### 3.1 资源重生与真实 builder

日常同步使用 `build_resources.py`：它以每个 demo 的工作目录调用**真正的 uiresbuilder**，再规范生成物尾部空白，保留 BOM/行尾，不手写 generated ID。

```powershell
python -B demos/fluent/build_resources.py
python -B demos/fluent/check_contracts.py
# 单目录同步/检查；--demo 可重复，省略则按 contracts.json 覆盖本轮13目录。
python -B demos/fluent/build_resources.py --demo wxDemo
python -B demos/fluent/check_contracts.py --demo wxDemo
```

**不要把共享 `generate_resources.py` 对所有 demo 批量运行当作无损重生。** 它是起始公共模板 primitive，会覆盖各 demo 定制 palette。确需重生 palette 时，先按该目录文档核实自有 generator 的范围，例如 [demo generator](../demo/generate_fluent_resources.py)、[demo2 generator](../demo2/generate_demo2_skins.py)、[wx generator](../wxDemo/generate_fluent_resources.py)；Grid/MC 的 `generate_fluent_extras.py` 只负责局部 extras，不将其当完整 palette 重建器。没有完整自有 generator 的目录，编辑 demo-local 资源，再运行真实 builder 和契约检查；保留历史初始化命令不代表日常建议重新套模板。

默认 builder 位于仓库 `tools/uiresbuilder.exe`，也可显式传 `--builder <路径>`。本轮 13 目录共 **26 个 rc2/resource.h** 的真实 builder 重复生成后逐字节一致，见 [generated JSON](../../output/remaining-fluent-generated.json) 的 `file_count=26`、`same_after_repeat=true` 和 SHA-256；这不是只比较 palette。

### 3.2 Windows Debug 构建和 CTest

本地主机没有完整 v143 ATL。实际构建仅在 shell 的 `_CL_` / `_LINK_` 环境中补入已安装 **14.16.27023 atlmfc** 路径，不改源码、CMake、工具集或仓库配置；这是本地兼容验证条件，不是跨机器保证。在已有 build 上重新配置会刷新新增文件的 GLOB：

```powershell
$atl = 'C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Tools/MSVC/14.16.27023/atlmfc'
$savedCL = $env:_CL_
$savedLINK = $env:_LINK_
if (!(Test-Path -LiteralPath "$atl/include/atlbase.h")) { throw "ATL header missing: $atl" }
try {
    $env:_CL_ = "$savedCL /I`"$atl/include`""
    $env:_LINK_ = "$savedLINK /LIBPATH:`"$atl/lib/x64`""
    cmake -S . -B build -DSOUI_BUILD_DEMOS=ON -DSOUI_FUN_TEST_POST_BUILD=OFF
    if ($LASTEXITCODE -ne 0) { throw "configure failed" }
    cmake --build build --config Debug --target @targets fun_test --parallel 4
    if ($LASTEXITCODE -ne 0) { throw "build failed" }
} finally {
    if ($null -eq $savedCL) { Remove-Item Env:_CL_ -ErrorAction SilentlyContinue }
    else { $env:_CL_ = $savedCL }
    if ($null -eq $savedLINK) { Remove-Item Env:_LINK_ -ErrorAction SilentlyContinue }
    else { $env:_LINK_ = $savedLINK }
}
ctest --test-dir build -C Debug -N -L '^soui-headless$'
ctest --test-dir build -C Debug -R '^soui_fluent_' --output-on-failure
ctest --test-dir build -C Debug -L '^soui-headless$' --output-on-failure
# 单进程复跑这23项，补查跨用例的全局状态恢复；不是额外23项新用例。
Push-Location build/bin/Debug
try { ./fun_test.exe '--gtest_filter=soui_fluent*' } finally { Pop-Location }
```

上述配置保留已有生成器/架构；新建构建树时另按本机安装指定 `-G "Visual Studio 17 2022" -A x64`。本页没有给出 Linux/macOS、Release 或移动端通过承诺。

### 3.3 shared、8 个 leaf checker、strict audit 与工具单测

```powershell
python -B demos/fluent/check_contracts.py
python -B demos/demo2/check_fluent_contract.py
python -B demos/gridctrl_demo/verify_fluent.py --self-test
python -B demos/hellocharts_demo/check_fluent_contract.py
python -B demos/lightbutton_demo/check_fluent_contract.py
python -B demos/mclistview_demo/verify_fluent.py --self-test
python -B demos/sci_demo/verify_fluent.py --self-test
python -B demos/wallpaper/verify_fluent.py
python -B demos/wallpaper/test_fluent_static.py
$audit = 'F:/OpenSource/ide_setting_sync/scripts/UserData/.codex/skills/soui-fluent-demo/scripts/audit_uires.py'
foreach ($name in $demos) {
    python -B $audit --uires "$repo/demos/$name/uires" --strict-system-skins
    if ($LASTEXITCODE -ne 0) { throw "audit failed: $name" }
}
python -B -m unittest discover -s demos/fluent -p tests_gui_smoke.py -v
```

工具单测验证 ownership、坐标/几何、黑图拒绝、ROI、动作和 cleanup 等纯逻辑/mock 路径；41 项工具单测不是 41 个真实 GUI 业务用例。静态契约也不替代 renderer、输入、线程时序或屏幕验收。

### 3.4 实际 GUI

[gui_smoke.py](gui_smoke.py) 只捕获本次启动的 owned PID/HWND，以 PrintWindow 保存完整目标窗口；默认生成 `light` / `dark` / `light_return` 三帧。`--toggle`/动作坐标是目标 HWND DPI context 下的逻辑客户区像素，`--protect-roi` 是整窗截图像素。必须核对真实窗口和 evidence 中的 `options`、`actions`、`dpi_context`，不能复制其它窗口的全局坐标。当前环境的基础例子：

```powershell
python -B demos/fluent/gui_smoke.py --demo flowlayout_demo --phase roundtrip --toggle 434 24
# 本地草稿 action JSON 属于该批次的忽略目录证据；存在时可复现头像样本。
python -B demos/fluent/gui_smoke.py --demo wxDemo --phase avatar_recheck --toggle 725 24 --action-file output/remaining-fluent/wx_draft.json --protect-roi 90 147 16 18
```

具体页面、网格编辑、MC scale_200 与按钮动作见下表对应 `evidence.json`。wallpaper 先按 [本目录 smoke 说明](../wallpaper/FLUENT.md) 运行 `prepare_fluent_smoke.py` 并仅向该进程传入 `SOUI_WALLPAPER_SMOKE_DIR`；测试后恢复环境，不回退真实 HTTP。忽略目录里的 action JSON、截图及历史 IPC 脚本不是随源码分发的运行时依赖，缺失时不能宣称原批次已重新复现。

IPC 需要同时运行 `ipc_server` / `ipc_client`，依次连接、Add(100,200)、Concat(100,200)、Sum(100)、两个窗口独立切主题、断开重连和关闭；预期分别为 300、100200、输入夹到 10 后结果 55。Hello 以 owned-PID 调试日志验证。历史记录保留 `reproduce`/`script` 字段及旧 5050 expectation mismatch；应按原 clamp 基线解释，不靠改业务代码迎合错误断言，也不把单进程主题截图当作真实 IPC 调用证明。

## 4. 已确认验证表

### 4.1 照片修复后的 fresh 非 GUI 验证

| 验证 | 已确认结果 | 日志 / 证据 |
| --- | --- | --- |
| Windows x64 Debug | 14 GUI target + fun_test 全量 fresh 构建通过 | [final-build](../../output/remaining-fluent-final-build.log) |
| CTest 注册 | 443 项 | [final-registered](../../output/remaining-fluent-final-registered.log) |
| headless | 442 项实际执行全部通过；1 项既有 Disabled 未执行 | [final-headless](../../output/remaining-fluent-final-headless.log) |
| Fluent 定向 | CTest 23 项通过；同一进程连续运行这23项也通过 | [final-targeted](../../output/remaining-fluent-final-targeted.log) / [single-process](../../output/remaining-fluent-final-single-process.log) |
| shared 契约 | 13 demo / 57 layout-menu / 0 error | [static](../../output/remaining-fluent-static.log) |
| strict audit | 13 目录全量，0 error / 0 warning | [static](../../output/remaining-fluent-static.log) |
| 8 个 leaf checker | 全量 fresh 通过（含适用 self-test） | [static](../../output/remaining-fluent-static.log) |
| 真实资源 builder 重复生成 | 26 个文件完全一致 | [generated](../../output/remaining-fluent-generated.json) |
| GUI 工具 unit | 41 项 fresh 通过 | [gui-tool-tests](../../output/remaining-fluent-gui-tool-tests.log) |
| 范围 / UTF-8 / 源 BOM / diff whitespace | 主线程检查通过 | [scope](../../output/remaining-fluent-scope.json) |

未执行项是既有 `swinx_misc.is_dbcs_lead_byte_codepage_dependent (Disabled)`；不能写成 443 项实际全部运行。Fluent 23 项属于定向复跑，与 headless 数量重叠，不额外相加。

### 4.2 已有真实 GUI 往返批次

12 个非 IPC demo 均有 `output/remaining-fluent/*_roundtrip_*/evidence.json`：owned PID/HWND、窗口/客户区几何、非黑图、明暗变化和返回浅色检查通过。整窗返回比较允许正常像素差；只有显式受保护 ROI 才声明像素恒等。下表按 manifest 的实际 canonical 批次索引；其中主 demo 已用照片修复后的最终二进制重新实测并视读，wx 使用修复后的 avatar_final，Grid 使用 edited 样本；不是完整页面或状态矩阵。wx contacts 的追加往返也已通过并视读，统一收录在 manifest 的 8 个代表性附加 run 中：

| demo | 已存实际证据 | 已确认范围 / 边界 |
| --- | --- | --- |
| demo | [canonical final](../../output/remaining-fluent/demo_final_20261011_223509_44bc2fb5/evidence.json) | PASS；最终二进制 light/dark/return，已视读 |
| demo2 | [roundtrip](../../output/remaining-fluent/demo2_roundtrip_20261011_214814_d84d08f1/evidence.json) | PASS；记录中的 light/dark/return |
| wxDemo | [canonical avatar_final](../../output/remaining-fluent/wxDemo_avatar_final_20261011_222055_9557ad38/evidence.json) | PASS；修复后头像圆内 ROI 恒等 |
| flowlayout_demo | [roundtrip](../../output/remaining-fluent/flowlayout_demo_roundtrip_20261011_214825_c58c26f2/evidence.json) | PASS；记录中的 light/dark/return |
| framelayout_demo | [roundtrip](../../output/remaining-fluent/framelayout_demo_roundtrip_20261011_214828_338cd14e/evidence.json) | PASS；记录中的 light/dark/return |
| gridctrl_demo | [canonical edited](../../output/remaining-fluent/gridctrl_demo_edited_20261011_222912_b9edd9fb/evidence.json) | PASS；Fluent_QA 提交后实际文字与往返 |
| hellocharts_demo | [roundtrip](../../output/remaining-fluent/hellocharts_demo_roundtrip_20261011_215244_b6fd8cc9/evidence.json) | PASS；记录中的 light/dark/return |
| lightbutton_demo | [roundtrip](../../output/remaining-fluent/lightbutton_demo_roundtrip_20261011_215248_fd98141f/evidence.json) | PASS；不是报警帧时序验证 |
| mclistview_demo | [roundtrip](../../output/remaining-fluent/mclistview_demo_roundtrip_20261011_215252_5fa1aec2/evidence.json) | PASS；记录中的 light/dark/return |
| propanimator_demo | [roundtrip](../../output/remaining-fluent/propanimator_demo_roundtrip_20261011_214832_0c507f24/evidence.json) | PASS；实际按钮追加证据与仅切页证据分开记录 |
| sci_demo | [roundtrip](../../output/remaining-fluent/sci_demo_roundtrip_20261011_215257_9dbbe6eb/evidence.json) | PASS；记录中的 light/dark/return |
| wallpaper | [roundtrip](../../output/remaining-fluent/wallpaper_roundtrip_20261011_215309_f7b4b938/evidence.json) | PASS；实际展示 offline fixture，图像 ROI 恒等，不代表真实 HTTP |
| ipcdemo | [32 张双窗口截图](../../output/remaining-fluent/ipc-20261011-215014-017029/evidence.json) / [visual verdict](../../output/remaining-fluent/ipc-20261011-215014-017029/visual_verdict.json) | 两窗口独立往返，真实连接/Add/Concat/Sum10/Hello/断开重连；旧 Sum100=5050 预期错误 |

主 demo 另有 [非默认 saved HSL 恢复](../../output/remaining-fluent/demo_saved_accent_20261011_215356_690728f2/evidence.json) 往返通过，**不宣称 GUI Save 完整路径已测**。34 个背景缩略图 `enableColorize` 修复有实际像素回归 [before 失败](../../output/remaining-fluent-photo-before.log) → [after 通过](../../output/remaining-fluent-photo-after.log)。wx 透明占位头像白色圆板修复保留原 PNG；[wxDemo_avatar_final_20261011_222055_9557ad38](../../output/remaining-fluent/wxDemo_avatar_final_20261011_222055_9557ad38/evidence.json) 往返通过，个人头像圆内 ROI 在 dark/return 像素恒等；不把 UI glyph 的重色套到媒体素材。

### 4.3 已确认并视读的追加页面、状态与按钮

| 样本 | 实际结果 | 不应扩大为 |
| --- | --- | --- |
| [flow page1](../../output/remaining-fluent/flowlayout_demo_page1_20261011_222602_1de9ac0c/evidence.json) | page1 light/dark/return 通过 | 全三页输入/DPI 矩阵 |
| [Chart Line](../../output/remaining-fluent/hellocharts_demo_line_20261011_222607_613f2f4f/evidence.json) / [Pie](../../output/remaining-fluent/hellocharts_demo_pie_20261011_222612_e4cab4d4/evidence.json) | 两个真实页面往返通过 | 六页全部交互、视口与动画时序 |
| [Grid edited](../../output/remaining-fluent/gridctrl_demo_edited_20261011_222912_b9edd9fb/evidence.json) | row10 F2 → `Fluent_QA` → Enter 提交；dark 实际显示该文字，往返通过 | 全部类型编辑器/网格业务 |
| [MC scale200](../../output/remaining-fluent/mclistview_demo_scale200_20261011_223143_1762be26/evidence.json) | 原 `scale_200` 令 `550×466` → `1100×932`，同 PID/HWND；1 recipient 和 15 字符 `Fluent QA draft` 在 dark/return 保留 | OS target-DPI 全矩阵、短信发送或新增排序 |
| [Prop layout page](../../output/remaining-fluent/propanimator_demo_layout_animation_20261011_222617_a70d61d2/evidence.json) / [set page](../../output/remaining-fluent/propanimator_demo_property_animation_20261011_222624_19c9d337/evidence.json) | 仅切页和主题往返通过 | 按钮实际触发 |
| [Prop width button](../../output/remaining-fluent/propanimator_demo_layout_animated_20261011_222919_1ad20fb9/evidence.json) | 点击宽度动画，中块变宽；终态 dark/return 不重启 | 连续帧时序、取消/重入 |
| [Prop parallel button](../../output/remaining-fluent/propanimator_demo_set_animated_20261011_222926_43a5c553/evidence.json) | 点击并行执行，色块位置交换；终态 dark/return 不重启 | 连续帧时序、取消/重入 |

以上追加样本均为真实 PrintWindow，主线程已视读；它们补充而不替代各目录历史 GUI 检查清单。IPC 原 `OnSum` 的 `n>10` clamp10 与 Sum100 输入变为10、返回55同样属于保留基线；旧证据的 `EXPECTATION_MISMATCH` / `PASS_WITH_EXPECTATION_MISMATCH` 是旧子代理错误预期的历史标记，不是产品未完成或 Sum100=5050 成功。

## 5. 未覆盖范围与使用边界

- **实际桌面为 125%。** 主 demo / SCI 的 target window DPI 为 120；其余保留原 96 DPI、不感知 DPI 窗口由 OS 缩放。MC 的显式 scale_200 是应用内缩放证据，不等于 target window DPI=192，也不构成完整 100/125/150/200% DPI × 语言 × 页面组合矩阵。
- **语言 UI 暂未实测。** wx `btn_menu` 在原基线没有事件绑定；保留中日语言资源/菜单入口，不虚构日文切换通过，不借此补业务功能。其它翻译资源及平台字体分支也不据静态检查宣称组合验收。
- Linux/macOS、移动平台、Windows Release、实际 HTTP、完整 DPI/语言组合及所有业务页面未测。Windows Debug 成功不证明脱离源码树的资源部署或其它平台构建/运行。
- 仅确认所记录动画按钮触发、终态和主题往返；连续帧采样、帧时序、快速重入、取消与压力行为未验证。LightButton 报警帧、所有图表交互及异步下载生命周期也不能由主题截图推断通过。
- saved HSL 恢复与原持久化契约保留，不等于已操作 GUI Save 的完整写入/重启链。wallpaper 的离线展示/ROI 不证明真实服务可用或原磁盘缓存写入已修复。
- [最终 manifest](../../output/remaining-fluent/evidence/manifest.json) 已生成，IPC 基线期望纠正保留原 evidence/verdict，不改历史证据。日志与真实截图按 SHA-256 关联；[scope 检查](../../output/remaining-fluent-scope.json) 允许资源 XML 序列化规范化 UTF-8 BOM，**不改源 C++ BOM**，不能扩大为全源码 BOM 清理。
- manifest 的最终 bounded media read-only review 已收录：未发现确认的 P1/P2，检查范围包括照片 opt-out、UI glyph、头像底板、生成器及组件生命周期；这是有限范围的代码复查，**不是人工最终 Review 或发布验收**。目前已提供的确认事实均已纳入；如后续另有已完成的 DPI/语言或页面证据，由主线程按实际覆盖增量更新本页与索引，不据新索引消除上述未测边界。
