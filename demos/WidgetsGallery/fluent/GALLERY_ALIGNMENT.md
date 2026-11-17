# Gallery 全项目样式对齐

## 目标、状态与边界

以 `SOUIWidgetsGalleryDemo` 当前资源为基准，对齐 `demos/` 下全部 **21 个 UI 项目目录**的标题栏／工具栏、普通菜单、窗口阴影和语义配色；`fluent` 为共享实现，不是独立应用。

截至 **2026-10-28（Asia/Shanghai）**：**资源与测试实现完成，Windows 自动验证通过；新截图和全项目视觉验收未完成，跨平台运行未验证。** 不把构建、静态契约或 GUI 烟测等同于全部画面已通过验收。

- 全程单代理，未使用子代理；不新增依赖、不改公共 API／共享系统资源，不执行 Git 提交。
- 保留 name/id、事件、文本、页签、编辑内容、DPI、多语言、外窗尺寸、单位和宿主窗口模式；不以重建业务树掩盖主题刷新问题。
- 改动限于各 demo 资源、对应契约／回归测试和本文。未重录 `fluent_contract.json`、`fluent_baseline.json`，也不通过更新冻结业务数据来消除失败。
- 21 个目录包括 19 个桌面项目目录及 Android/OHOS 两个移动项目。`ipcdemo` 分别产生 client/server 两个目标，因此本次 Windows 构建为 **20 个目标**，不是移动端构建也已通过。

## 验收与实施顺序

1. 固定全部项目资源布局中的 name/id、事件、文本、页签和宿主模式，建立独立的 Gallery 样式契约及冻结业务哈希。
2. 普通桌面主窗口采用 48dp 标题栏、46×40dp 窗口按钮、40×32dp 主题按钮；保持既有外窗大小和内容结构，同步调整绝对定位正文起点。最大化与还原共用同尺寸槽位。普通对话框采用 40dp 标题栏、44×36dp 关闭按钮。
3. 普通菜单行高 34dp，使用语义文字／面板色；保留 ID、快捷键、子菜单与原阴影留白。表情网格等非普通菜单不统一行高。
4. 沿用 Gallery 明暗语义色、跨平台字体和柔和阴影。分层／原生窗口模式不互换，不重复添加阴影。共享 WindowChrome 阴影 gutter 保持 18/12/18/24；Gallery/dpidemo 保持 26/20/26/32。UIEditor 保持 `ue_` 私有资源域，防止污染被编辑文档。
5. ScreenPin 对齐浅色面板、强调色、按钮状态和三个菜单，保留截图／标注交互、激活阴影及 popup 定位 gutter，不改变标注色的业务含义。
6. Android/OHOS 对齐浅色语义色、按钮与模态 surface；保留触控尺寸、RGB 示例块、主题按钮原有“切换文本颜色”行为，不添加桌面 chrome。
7. 精确更新受影响的旧几何断言，保留业务契约与负向变异测试；同步资源，再执行 Python 回归、Windows 构建及 CTest。视觉迭代须依据新截图，本轮截图不可用，不据此继续盲调样式。

## 范围矩阵

“GUI 烟测”只指其已覆盖的交互，不代表该行所有项目、页签及状态均有像素验收。

| 范围 | 对齐点／保留项 | 已有证据层级 |
| --- | --- | --- |
| demo、demo2、dpidemo、flowlayout_demo、framelayout_demo、fun_test | 主窗／普通对话框、菜单、明暗色；纯拖动／尺寸演示 caption 保留 | Gallery 契约、相关回归、Windows 构建；已有 GUI 烟测覆盖部分交互 |
| gridctrl_demo、hellocharts_demo、lightbutton_demo、propanimator_demo、mclistview_demo、sci_demo | 标题栏及状态按钮；表格／编辑／动画演示和窗口模式保留 | Gallery 契约、相关项目回归、Windows 构建 |
| ipcdemo | client/server 同时覆盖 | Gallery 契约、两个 Windows 目标构建 |
| SoTool、uieditor、wallpaper | 工具主窗／菜单；UIEditor 私有域、紧凑对话框字段和按钮互不遮挡 | 工具契约与负向变异、Windows 构建；已有 GUI 烟测覆盖部分交互 |
| wxDemo | 三个业务页右上角窗口操作；保留 64px 侧栏、全页拖动 caption 及拖动／尺寸演示 | Gallery 契约及 DPI 留白断言、Windows 构建 |
| screenpin | toolbar/field/menu surface、三菜单行高、控件状态及 popup 阴影；保留标注颜色 | Gallery 契约、定向真实窗口菜单／键盘／工作区／DPI 烟测、Windows 构建 |
| android-demo、ohos-demo | 语义色、按钮状态、模态；不添加桌面 chrome | XML／索引、业务与触控尺寸契约；未构建移动 SDK／未运行设备 |
| SOUIWidgetsGalleryDemo | 作为基准，保留业务及视觉契约 | Gallery 与自身资源回归、Windows 构建；未取得本轮新截图 |

## 关键修改与回归保护

- 各项目的 `uires/xml/`、`uires/layout/`、`uires/smenu/` 以及移动端 assets/rawfile 资源中对齐 chrome 和菜单；局部调整 ScreenPin surface／状态及移动按钮配色。未统一外窗尺寸、强改演示色块或重写宿主实现。
- 新增 `demos/fluent/test_gallery_alignment.py`：16 项契约，覆盖 21 个项目目录、117 份布局／菜单的冻结业务哈希、主窗／对话框几何、UIEditor 紧凑窗字段、wx DPI、34dp 普通菜单、移动触控／按钮状态、ScreenPin 阴影、索引及业务负向变异。
- 新增 `demos/fluent/gallery_behavior.json`：冻结上述业务结构，**不得重新录制以掩盖业务回归**；视觉白名单变化由测试显式表达。
- 精确更新 demo2、SoTool、hellocharts、lightbutton、mclistview、sci、UIEditor、wallpaper 等现有契约对 Gallery chrome 的允许范围，继续保留原身份、业务和 hostile mutation 检查。
- `demos/fun_test/test_e2e_dpidemo_res.cpp`、`test_e2e_fluent_dock_states.cpp`、`test_e2e_fluent_editing.cpp`、`test_e2e_screenpin_resources.cpp` 只同步被本次样式变更影响的旧几何预期。
- `demos/fun_test/test_gui_screenpin.cpp` 两处普通菜单预期由旧 28dp 改为 Gallery 34dp；未跳过键盘、DPI、工作区或阴影检查。
- 资源通过 `demos/fluent/build_resources.py` 与 `demos/SOUIWidgetsGalleryDemo/build_resources.py` 的项目入口同步，记录见 `resource-sync.log`；未运行全局资源生成器或重生成阴影素材。`fun_test` 使用源码资源加载，无需伪造 PE 资源目录。

## 验证环境与最终结果

工作区：`F:\OpenSources\soui4_remaker\soui5-fluent`。以下证据文件统一位于工作区下的 `output/gallery-alignment/`（Git 忽略目录，本地保留，不随提交分发）。统一入口为 **`verification-summary-final.json`**；应随任务记录一起保留该目录，不能仅凭本文代替原始输出。

- Windows x64，Visual Studio 17 2022，多配置 Debug；沿用现有默认工具链。
- `SOUI_ENABLE_GUI_SMOKE=ON`、`SOUI_ENABLE_SVG=ON`、`SOUI_FUN_TEST_POST_BUILD=OFF`。
- 初次 UIEditor 链接未找到已有 ATL 库；demo/uieditor 的成功重试仅临时在 `_LINK_` 中加入 `/LIBPATH:"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.16.27023\atlmfc\lib\x64"`，并在 `finally` 恢复原环境。未安装新依赖，未为此修改源码／全局环境。

| 验证项 | 最终实测结果 | 记录／原始输出 |
| --- | --- | --- |
| Windows Debug 构建 | **20/20 目标成功** | `desktop-build-results-final.json`；每个目标保留原始 `source_record`、命令及 log |
| ScreenPin 菜单定向 GUI | **2/2 通过** | `ctest-gui-menu-targeted-results.json` 及同名前缀 `.log`／`.xml` |
| 完整 Windows GUI 烟测 | **103/103 通过，0 失败** | `ctest-gui-post-alignment-final-results.json` 及同名前缀 `.log`／`.xml` |
| 无头 CTest | **772 项注册，实际执行 771 项通过，1 项既有 Disabled，0 失败** | `ctest-headless-post-gui-final-results.json` 及同名前缀 `.log`／`.xml` |
| Python 项目回归 | **19 组命令全部 returncode 0，合计 238 次 unittest 执行** | `project-regressions-post-sync.json`／`.log`；238 不是去重后的独立用例数 |
| Gallery 新契约最后复跑 | **16/16 通过**，2026-10-28 05:00:00–05:00:01 +08:00 | `gallery-alignment-contracts-final-results.json`／`.log`；已在上述 discovery 中覆盖，不再累加为独立总数 |
| 共享原业务契约 | **13 demos、57 original layouts/menus，0 errors** | 包含在上述 19 组命令内的 `demos/fluent/check_contracts.py` 输出 |
| CRLF 感知差异检查 | **exit 0**；保留尾空白检查 | `diff-check-crlf-final-results.json`／`.log`／`-warnings.log` |

无头 CTest 最后执行于 **2026-10-28 04:48:09–04:49:34 +08:00**。唯一未执行项为既有 Disabled 的 `swinx_misc.is_dbcs_lead_byte_codepage_dependent`，不能写成 772 项全部执行通过。GUI 的 103 项不是对全部 21 个项目的全组合截图验证。

### 构建、CTest 与资源入口

以下从工作区根执行；20 个目标按目标分别构建，实际逐条命令、退出码和重试来源保存在构建 JSON 中：

```powershell
Set-Location -LiteralPath 'F:\OpenSources\soui4_remaker\soui5-fluent'
$evidence = Join-Path (Get-Location) 'output\gallery-alignment'
$env:PYTHONIOENCODING = 'utf-8'

# 各项目资源入口（已完成同步）
python -B demos/fluent/build_resources.py
python -B demos/SOUIWidgetsGalleryDemo/build_resources.py

# 单目标命令形状；<target> 取下列目标，不是额外 CMake target
# cmake --build build --config Debug --target <target> --parallel 4
cmake --build build --config Debug --target fun_test --parallel 4

ctest --test-dir build -C Debug -R '^window\.gui_screenpin_popup_(menu_private_surface_keyboard_and_workarea|menus_follow_monitor_dpi_on_every_open)$' --output-on-failure --output-junit "$evidence\ctest-gui-menu-targeted.xml"
ctest --test-dir build -C Debug -L '^soui-gui$' --output-on-failure --output-junit "$evidence\ctest-gui-post-alignment-final.xml"
ctest --test-dir build -C Debug -L '^soui-headless$' --output-on-failure --output-junit "$evidence\ctest-headless-post-gui-final.xml"
python -B demos/fluent/test_gallery_alignment.py -v
```

20 个成功目标：

```text
demo demo2 dpidemo flowlayout_demo framelayout_demo fun_test
gridctrl_demo hellocharts_demo ipc_client ipc_server lightbutton_demo
mclistview_demo propanimator_demo sci_demo screenpin SoTool
SOUIWidgetsGalleryDemo uieditor wallpaper wxDemo
```

最后一次 `fun_test` 构建亦已于 2026-10-28 04:59:59–05:00:00 +08:00 成功复查，证据为 `build-fun_test-evidence-final-results.json`／`.log`。构建成功只证明对应目标可构建，测试结论来自单独运行的 CTest。

### 19 组 Python 回归命令

```powershell
python -B -m unittest discover -s demos/fluent -p 'test*.py'
python -B demos/fun_test/test_tools_fluent_contracts.py
python -B demos/fun_test/test_tools_fluent_menu_dpi.py
python -B demos/fun_test/test_fluent_shadow_resources.py
python -B demos/fun_test/test_fluent_generated_assets.py
python -B -m unittest discover -s demos/uieditor -p 'test_fluent*.py'
python -B demos/SoTool/test_fluent.py
python -B demos/wallpaper/test_fluent_static.py
python -B demos/wallpaper/verify_fluent.py
python -B demos/gridctrl_demo/verify_fluent.py --self-test
python -B demos/mclistview_demo/verify_fluent.py --self-test
python -B demos/sci_demo/verify_fluent.py --self-test
python -B demos/hellocharts_demo/check_fluent_contract.py
python -B demos/lightbutton_demo/check_fluent_contract.py
python -B demos/demo2/check_fluent_contract.py --self-test
python -B demos/SOUIWidgetsGalleryDemo/test_resources.py
python -B demos/SOUIWidgetsGalleryDemo/test_build_resources.py
python -B demos/dpidemo/test_popup_shadow.py
python -B demos/fluent/check_contracts.py
```

### 失败、修复与替代证据

保留早期失败日志，不将它们删除或改写成成功记录：

| 早期记录 | 原因与处理 | 应采用的最终证据 |
| --- | --- | --- |
| `desktop-build-results.json` | 初次 UIEditor ATL 链接失败；既有库的临时路径重试成功，另补齐 flowlayout／fun_test 构建证据 | `desktop-build-results-final.json`，逐项追溯原始 source_record |
| `ctest-headless-results.json` | 四处测试仍断言旧 chrome 几何；只更新受影响的尺寸预期，未重录业务 fixture | `ctest-headless-post-gui-final-results.json` |
| `ctest-gui-results-final.json` | 两项 ScreenPin GUI 仍预期旧 28dp 普通菜单 | 先运行 `ctest-gui-menu-targeted-results.json` 的 2 项，再完整复跑 `ctest-gui-post-alignment-final-results.json` 的 103 项；随后再次运行无头测试 |
| `diff-check-final-results.json` | 裸 `git diff --check` 返回 2，26 份 XML 的 102 处提示均为 HEAD 和工作副本既有 CRLF 的行尾 CR | `diff-check-crlf-final-results.json`；未批量归一化换行或修改 Git 配置 |

按仓库既有 CRLF 规则的差异检查命令如下，仅本次调用认可 CR 行尾，仍启用真实尾空白、文件尾空行和空格在 tab 前的检查：

```powershell
git -c core.whitespace=blank-at-eol,blank-at-eof,space-before-tab,cr-at-eol diff --check
```

新增未跟踪的三个交付文件另行检查尾空白和文件末尾；不能以 `git diff` 没有显示新增文件来视为已检查。LF→CRLF 通知与真正空白错误分别保存，不把通知当失败，也不声称裸 `git diff --check` 通过。

## 单代理自审结论

已核对改动范围、冻结业务哈希／负向变异、上述五份 C++ 回归文件、UIEditor 三份契约测试以及各项目的显式几何白名单。未发现尚待修复的自动化失败；未通过放宽 name/id、事件、原文本等业务约束消除失败。未改框架公共 API、子模块指针或新增依赖。

本轮没有独立 Reviewer 或子代理；以上是实施者自审，不宣称独立 AI Review 已通过。资源头文件存在无字节差异的工作区状态时保留原状，不为清理状态执行 restore/reset，也未清理既有 `.codex/`、`.omx/` 或 `__pycache__`。

## 尚未覆盖与停止边界

1. **本轮没有成功取得新截图，视觉验收未通过。** 支持的 `@oai/sky` 窗口捕获在窗口枚举／选择／激活恢复后仍两次超时（`window capture timed out: timed out waiting on channel`）。不绕过支持的工具另造截图路径，不以无图的推断继续样式迭代。
2. 103 项真实窗口烟测只证明其覆盖的运行时交互，不证明 21 个项目所有明暗×语言×DPI、页签、hover/pressed/disabled/focus、长文本及阴影边角的视觉像素正确。
3. Linux/macOS 桌面、Android/OHOS SDK 与设备未构建／运行；移动端目前仅有资源／业务契约证据。Windows 结果不能扩展为跨平台通过。
4. 尚需在支持的窗口捕获可用后，取得 Gallery 与各类窗口的明暗、语言、DPI 代表性新图，检查标题／按钮留白、菜单翻转及工作区边界、阴影四边四角，再决定是否需要进一步修改。此项仍为待验收，不记录为完成。

### 2026-10-28 05:18 +08:00 续接检查

通过支持的 `@oai/sky` 启动 Gallery 后，新一轮捕获及重新枚举／选择后的恢复捕获仍各超时一次；未取得新截图，也未重复构建或改动实现。两次捕获的精确时间未保留，不把本节记录时间当作捕获时间。原始错误及恢复顺序另记于 `output/gallery-alignment/visual-capture-recheck-20261028T0518.json`。只对本轮自行启动的 Gallery 发送一次 `Alt+F4`，随后重新枚举确认该窗口已退出（05:18:29 +08:00），未强制终止进程或关闭其他应用。视觉验收仍未通过；不重复失败的捕获循环，不在无图条件下继续改样式。

视觉状态保存在 `.omx/state/gallery-alignment/ralph-progress.json`：`implementation_status=complete`、`automated_verification_status=passed`、`visual_pass=false`。该 JSON 记录证据和缺口，不表示启用了 OMX runtime 工作流。总体目标不因自动验证通过而标记为全量视觉验收完成。


## 2026-10-29 demo2 主窗口遗漏修正

用户截图显示主窗口标题紧凑、工具栏/滑块贴边、列表与右侧按钮无间隔。此前 Gallery 检查仅覆盖 caption 型主窗，漏掉 demo2 的 ncpainter；此前自动检查通过不代表该窗口视觉通过。

验收与实现：保留纯 C COM、原 name/id/事件/adapter、600×400 内容初始尺寸、非客户区机制及明暗主题；NC 标题改为 48dp、窗口按钮显式 46×40dp；内容留白 16/12/16/16dp、区块间隔 12dp；工具栏四按钮按剩余宽度平分，hello c 显式 100dp 避免 desired-width=200 挤占；右侧操作按钮高 32dp。标题增高会增加初始窗口总高度，未通过缩小内容区抵消。5dp 原 NC border 和原皮肤 intrinsic 尺寸不变，本次不声称修复窗口外阴影。

精确布局差异列入 check_fluent_contract.py 白名单；未重录 fluent_contract.json 或业务指纹。新增独立 NC 主窗测试，先在旧 30dp 标题上观察失败，修改后通过。

实际验证（Windows）：
- `python -B demos/demo2/check_fluent_contract.py --self-test`：通过，15 项异常变异均被拒绝。
- `python -B demos/fluent/test_gallery_alignment.py -v`：17 项通过。
- `python -B demos/fluent/check_contracts.py --demo demo2`：4 个原布局/菜单，0 errors。
- `python -B demos/fluent/build_resources.py --demo demo2`：资源生成成功。
- `cmake --build build --config Debug --target demo2 --parallel 4`：成功。
- `ctest --test-dir build -C Debug -R 'soui_fluent_(resources_e2e.demo2|controls_e2e.demo2)' --output-on-failure`：2/2 通过（已有测试程序，运行时读取资源）。
- `git diff --check`（仓库 CRLF 容忍设置）：通过。

未完成验收：本轮没有可调用的 node_repl/sky 截图入口，未获得修改后实际画面。需进一步验证明暗主题、100/125/150/200% DPI、缩放窗口、窗口按钮及弹层；自动检查不代表视觉像素或实时交互通过。Linux/macOS 未运行。此前保持旧标题/旧布局密度的说明为历史约束，以本节授权修正为准。


## 2026-10-29 demo2 第二次截图反馈：重复标题按钮与内角黑边

- 最新依据：`codex-clipboard-18da2155-9cd5-464d-98d5-c71a0d91bf80.png`，判定未通过：原生窗口按钮覆盖自绘按钮，客户区圆角露黑。
- 验收目标：仅保留 NC 自绘标题按钮；客户区与 NC 边框使用全覆盖不透明背景；明暗切换仍刷新 NC，保留原 ID、C 接口、布局和可调整大小行为。
- 最小修复：demo2.c 创建窗口不再带 WS_CAPTION；dlg_main.xml 的 root/skinBorder 复用两套主题中已有的纯色 skin_active；FluentBridge.cpp 的 NC 主题刷新同步使用该皮肤。未改框架或资源生成器。
- 回归：新增单标题栏/不透明背景配置检查，修改前失败于 WS_POPUP|WS_CAPTION，修改后通过；检查双主题背景与 NC 刷新引用一致。该检查是静态约束，不证明 Windows 实际绘制结果。
- 实际验证：`python -B demos/fluent/test_gallery_alignment.py -v` 18/18；`python -B demos/demo2/check_fluent_contract.py --self-test` 15 个恶意变更拒绝、契约通过；`python -B demos/fluent/check_contracts.py --demo demo2` 0 错误；`python -B demos/fluent/build_resources.py --demo demo2` 成功；`cmake --build build --config Debug --target demo2 --parallel 4` 成功；`ctest --test-dir build -C Debug -R 'soui_fluent_(resources_e2e.demo2|controls_e2e.demo2)' --output-on-failure` 2/2；diff 空白检查通过。
- 自检：修改限定 demo2 和回归检查，保留 NC 命中测试路径、窗口功能 ID、resizable/appWnd 设置及 C-COM 生命周期；未更新冻结基线。没有使用子代理。
- 限制：当前没有可调用的受支持 node_repl 截图入口，未获取修复后画面，也未验证实际拖动/缩放/最小化/最大化/关闭。待 GUI 验收：浅/深主题及 100%/125%/150% DPI 下检查单套按钮、四角无黑边、拖动/缩放/最大化还原；未运行其他平台。


## 2026-10-29 demo2 第三次反馈：原生外框残留

- 依据：codex-clipboard-5246472e-4bd7-433b-aeb0-2af33f08f24b.png；视觉判定失败，顶部白条和外沿仍呈系统边框。此前移除 WS_CAPTION 不足以处理外框：SHostWnd 因 resizable 自动加入 WS_THICKFRAME。
- 验收/方案：不移除 WS_THICKFRAME、不关闭 resizable，不改共享框架；仅在 demo2 的 Windows 主题桥接中设置 DWMWA_NCRENDERING_POLICY=DWMNCRP_DISABLED，在帧刷新前关闭 DWM 非客户区绘制，继续让 SNcPainter 绘制外沿及处理八方向缩放命中。首次绑定及主题切换均应用；失败写调试日志。CMake 仅 Windows 链接系统 dwmapi 库，无第三方依赖。
- 回归先红后绿：新增保留缩放配置/禁用原生绘制/刷新顺序/Windows 链接检查，旧实现失败，新实现通过。静态检查不代表真实窗口绘制或交互通过。
- 实际验证：`python -B demos/fluent/test_gallery_alignment.py` 19/19；`python -B demos/demo2/check_fluent_contract.py --self-test` 15 个异常变异被拒绝；`python -B demos/fluent/check_contracts.py --demo demo2` 4 布局、0 错误；`cmake --build build --config Debug --target demo2 --parallel 4` 成功；`ctest --test-dir build -C Debug -R 'soui_fluent_(resources_e2e.demo2|controls_e2e.demo2)' --output-on-failure` 2/2。未改 XML，无须重新生成资源。
- 自检：修改为 demo2 局部 Windows 分支，未改变 C-COM 接口、功能 ID、事件或冻结基线；本轮没有子代理。
- 待验收：当前无可调用的 node_repl/sky，尚无修改后截图；未实测浅/深主题、激活/失焦、100/125/150/200% DPI、八方向缩放、拖动、最大化还原和 Snap。关闭 DWM 非客户区绘制也可能改变系统阴影，尚未视觉确认。非 Windows 未构建。不把上述静态/无头通过记为视觉通过。
