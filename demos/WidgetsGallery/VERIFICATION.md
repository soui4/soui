# SOUIWidgetsGalleryDemo 验证记录

验证日期：2026-10-15（Asia/Shanghai）。本记录仅描述本地实际执行结果，不代表其他平台、线上 CI 或发布结果。

前半部分记录内置控件分类扩展的交付；文末「滚轮路由修复」记录后续输入缺陷修复，包含公共框架修改，不沿用前一阶段“未改框架”的范围口径。

## 目标与覆盖口径

在原生 SOUI C++ / XML Gallery 中，按用户要求整理内置控件分类，为各类型提供配套的统一 Fluent 样式、真实使用场景与可查看的 XML / C++ 初始化路径；不是截图容器、WebView 或只列名称。

- 以 `SOUI/src/SApp.cpp::SObjectDefaultRegister::RegisterWindows` 与 `DEF_SOBJECT` 交叉核对 **58 个 Window 类**：57 个无条件注册，另 1 个 `SActiveX` 仅 Windows 非 MinGW 默认注册。
- **20 个导航分类，包含 Overview**，不是 20 个独立控件家族。原 14 项顺序及 ID 保留，新增 ListView、TileView、Date & time、Menus & toolbars、Containers、Hosting & platform。
- 代码场景为 **46 live + 8 composite + 3 dynamic = 57 类**。复合根、标题栏、组合框编辑器、容器子页等不冒充独立控件演示；平台 recipe 另计。
- `SActiveX` 为 **1 个 platform_recipe**，不激活外部 COM；`SKeyboardSpacer` 的桌面高度为 0，移动端 IME 避让未验证。
- Layout、Skin、动画对象、内部辅助类及 `controls.extend` 不混入这 58 类分母。GalleryButton / GalleryTabCtrl / GalleryRebar 是 Demo 本地绘制子类，不重复计数。

清点、矩阵及实施计划分别见 `BUILTIN_CONTROLS.md`、`CONTROL_COVERAGE.md`、`CONTROL_COVERAGE_PLAN.md`。目录唯一手工来源是 `control_catalog.json`；`build_catalog.py` 从真实 XML 抽取用法并生成头文件与矩阵。

## 可观察验收与证据

| 验收项 | 实现 / 验证方式 |
|---|---|
| 类型清点和 20 页映射一致 | 注册 / 声明逐项核对；目录、唯一 class/tag/sample、ID / 页面 / 搜索映射回归；真实导航标题去重 |
| 每类使用真实控件及配套样式 | 命名 XML 实例、真实建树的具体类型；按独立 / 组合 / 动态 / recipe 公开矩阵 |
| Button 和不同列表类型不再混为一个示例 | SButton / SImageButton / SLink，以及 SListBox / SListCtrl / SListView / SMCListView / STileView / SHeaderCtrl 各自场景 |
| 虚拟视图不是空壳或静态占位 | 真实 adapter：ListCtrl 12 行、ListView 200 行、MCListView 80 行 × 3 列、TileView 30 卡片、TreeView 3 组 × 4 子项、ComboView 8 项、搜索建议 8 条本地数据 |
| 回收绑定、选择、展开和排序 | C++ 真实窗口检查滚动后的绑定、选择反馈、多列表头排序、TreeView 展开、ComboView 选择及浅深往返状态 |
| XML 可用于理解实际用法 | 从真实实例抽取模板和 buddy / 父组合框上下文；ComboEdit 的完整 dropdown=0 组合框回归；复杂绑定明确指向 GalleryAdapters.h / MainDlg.cpp |
| 一致浅深主题且不重建控件树 | 每主题 96 个 Skin、29 个语义 token、26 个图标；真实选择 / 编辑 / 排序 / 折叠状态在切换后保留 |
| 原有进度功能保持 | 初值 25 / 60 / 90 / 35，范围 20–80 的 35 对应 25%；900 ms ease-out、重入重定向、Reset 及真实离屏绘制回归 |
| 输入、数值及堆栈行为 | RadioBox 方向键 / 焦点、Spin 边界钳制、Switch、StackView 与反馈的真实窗口回归 |
| 模态会话不是视觉遮罩 | 初焦点、Tab 隔离、重复打开保护；确认 / 取消 / Escape / 背景退出均恢复原 enabled 状态与焦点；浅深居中画面复查 |
| 菜单 / Toolbar / Rebar | 真实菜单选择事件、New / Save / Pin、三项完整几何；Rebar 保留核心布局和折叠状态，仅本地覆盖硬编码绘制 |
| Native host 生命周期及画面 | 不透明宿主中延迟创建实际 SRealWnd 子 HWND；可见且有尺寸、关闭销毁与 owner 恢复；浅深 child / host 的 PrintWindow 像素回归及截图复查 |
| SVG 优先与改动隔离 | 新控件皮肤 / 图标均为 SVG；4 个已有 PNG 仅用于可复现的窗口 / 弹层阴影。未改框架、共享 Fluent 或系统资源，无新增运行时依赖 |
| 窗口几何 | 默认 1256×876（含阴影留白）、1040×700、最大化 / 还原；独立导航与内容滚动 |

## 环境与实际命令

- Windows x64，Visual Studio 2022，Debug，GDI + Stb，`SOUI_ENABLE_SVG=ON`、`SOUI_ENABLE_GUI_SMOKE=ON`。
- 目标 GUI 为 DPI-unaware / 96 逻辑 DPI。资源 E2E 覆盖 100 / 150 / 200 皮肤实例及真实离屏绘制，不能等同于跨显示器真机高 DPI 输入验收。
- Debug 使用 FILE 资源；Windows Release 保留 PE 内嵌资源；其他桌面平台保留 Skia / FILE 及系统 ZIP 复制分支，但本次未运行其他平台。

从仓库根实际执行：

~~~powershell
cmake --build build --config Debug --target SOUIWidgetsGalleryDemo fun_test --parallel 4
ctest --test-dir build -C Debug -R '^gallery[._]|^window\.gui_gallery_' --output-on-failure
ctest --test-dir build -C Debug -L '^soui-headless$' --output-on-failure
.\build\bin\Debug\fun_test.exe '--gtest_filter=gallery*:window.gui_gallery_*' --gtest_repeat=3
python -B -m unittest discover -s demos/SOUIWidgetsGalleryDemo -p 'test_*.py'
python -B -m ruff check --no-cache demos/SOUIWidgetsGalleryDemo
python -B demos/SOUIWidgetsGalleryDemo/generate_assets.py --check
python -B demos/SOUIWidgetsGalleryDemo/build_catalog.py --check
python -B demos/SOUIWidgetsGalleryDemo/build_resources.py --check
git diff --check
~~~

| 检查 | 实际结果 |
|---|---|
| Demo + fun_test Debug 构建 | 通过，包含最后的 ComboEdit 生成示例和 Native 字体修复 |
| Gallery 专项 | **28/28**：11 模型、8 真实资源无窗口 E2E、9 真实窗口 GUI |
| 单进程重复 | **28 × 3 全部通过**，同一进程中重复检查资源、应用、主题与 HWND 生命周期 |
| 完整无头门禁 | **549 注册，548 通过，1 个仓库既有禁用项未执行**；其中 487 单元注册、1 集成、61 无窗口 E2E |
| Python | **21/21**：7 资源 / 构建契约、5 GUI 检查器 / 动作文件、6 目录 / 用法、3 资源发布 |
| 资源静态审计 | 187 indexed 文件、183 XML/SVG 文档；引用、成对主题、状态切片几何及默认样式检查通过 |
| 生成物一致性 | 资源、目录、用法、覆盖矩阵三个 --check 通过；资源 check 调用真实 uiresbuilder 在临时目录比较，不改工程产物，仅忽略 stamp 差异 |
| 格式 / 静态检查 | 手写 C++ 的 clang-format --dry-run --Werror、Ruff、git diff --check、包含未跟踪新增文件的尾空格审计通过；不格式化生成的 GalleryCatalog.h |
| SVG-OFF 边界 | 独立 CMake 配置及工程 / CTest 检查通过：无 Gallery target、资源 E2E、GUI 源及注册，保留 11 个纯模型注册。未构建 / 执行该配置 |
| Release | 仅实际执行 Gallery 的 MSBuild /t:ResourceCompile，通过；没有完整 Release 编译链接或启动结论 |

既有禁用项：`swinx_misc.is_dbcs_lead_byte_codepage_dependent`。历史一次门禁曾遇到未改动的 `soui_rubberband.listview_multisel_normal_click_collapses_selection` 偶发失败；原因未在本任务确定，保留旧失败 / 复跑日志，没有修改不相关框架或测试来规避。本轮最后的 548 个可执行无头用例全部通过。

最后的构建 / 测试日志位于忽略目录 `output/gallery-work/`：

- `build-acceptance.log`、`gallery-acceptance.log`、`headless-acceptance.log`、`repeat-acceptance.log`
- `python-acceptance.log`、`ruff-acceptance.log`
- `generate_assets-acceptance.log`、`build_catalog-acceptance.log`、`build_resources-acceptance.log`
- `svg-off-acceptance.log`、`svg-off-tests.txt`、`release-resource-acceptance.log`
- Native 绘制回归的 red / green：`native-print-red.log`、`native-rgb.log`

## GUI 与视觉复查

自动化仅操作本次启动 PID 所属的窗口。`gui_smoke.py` 复用既有所有权 / PrintWindow / 有界退出验证器，在本 Demo 内增加排队点击、受限光标同步和标题检查；没有修改共享验证器。前台被系统拒绝则失败，不绕过系统策略或操作其他应用。

- 全 20 分类 × 浅深两主题：**40 张无激活 PrintWindow 捕获**，已检查浅深总览图；它证明采集与代表性页面视觉，不证明每种状态和输入行为。
- 目录交互最终运行：**74 个动作、17 张捕获**，真实选择 / 虚拟视图、菜单、工具栏、浅深菜单弹层与原生子 HWND 宿主；已查看实际菜单反馈和浅深原生子窗口内容。非黑图门槛不充当行为断言。
- 20 页实际导航的最后运行通过：**145 个动作、23 张捕获、21 项完整性 / 标题去重检查**，并完成主题往返。标题检查用于防止导航脚本漏页，不替代视觉复查。
- `gui_geometry.py` 最后运行：Progress / Overview / ListView / Commands 在 1040×700，另有最大化与还原，**6 张捕获通过**；已复查窄 ListView / Commands。无激活、无桌面键鼠输入。
- 实际窗口内模态的浅深截图已复查，位于内容中心；9 个 `window.gui_gallery_*` 用例覆盖真实宿主输入、选择、数据回收、排序、模态输入隔离、Native 生命周期与 Toolbar / Rebar 状态，而不是伪造控件事件。

本地证据（忽略目录，不随 Demo 分发）：

- `output/gallery-snapshots/20261014_214030_b4402723/evidence.json`（40 图；同目录 montage_light.jpg / montage_dark.jpg）
- `output/gallery-gui/SOUIWidgetsGalleryDemo_catalog_final_20261015_055812_51522e8b/evidence.json`（74 动作、17 捕获）
- `output/gallery-snapshots/20261014_213830_c9aa5ed5/dialog_dark_modal.png`
- `output/gallery-snapshots/20261014_213832_fa09af36/dialog_light_modal.png`
- `output/gallery-geometry/evidence.json`
- `output/gallery-gui/SOUIWidgetsGalleryDemo_pages_acceptance_20261015_060547_872a9841/evidence.json`（全 20 页导航与主题往返）
- `output/gallery-snapshots/20261014_220631_6cca6588/combo_dark.png`（真实 View XML 面板）
- `output/gallery-snapshots/20261014_220931_6240ae97/listview_dark.png`（多列虚拟视图与存储列表）
- `output/gallery-snapshots/20261014_220858_780422be/hosting_light.png`、`output/gallery-snapshots/20261014_220900_ca54d7c4/hosting_dark.png`（最后的平台边界文案）

截图目录使用 UTC 时间戳，GUI 运行目录使用本地时间戳；上述均为本地 2026-10-15 的证据。最终视觉检查结论写入 `.omx/state/gallery/ralph-progress.json`。本次使用真实窗口图像检查，不将不可用的 visual-verdict skill 写成已调用。

## Review 与缺陷闭环

独立覆盖审计确认：20 分类、58 注册类、46 live / 8 composite / 3 dynamic / 1 platform_recipe 的清点和矩阵对应，无遗漏或重复计数。

已处理的可定位问题：

1. MenuBar 绑定真实 `EventSelectMenu::nMenuId`，不是普通菜单的 EventMenuCmd。
2. 窗口内模态禁止 Tab 逃入背景，统一处理确认 / 取消 / Esc / 背景退出并恢复原状态；按 anchor 契约修正居中定位。
3. ComboEdit 的 View XML 原先只有孤立 editStyle：现在保留真实可编辑父组合框；新增回归在旧实现失败、修复后通过。
4. Native 初始尺寸为 0：不透明宿主完成布局后延迟创建；真实生命周期回归锁定浅深两轮。
5. Native 黑块：SOUI RGBA 颜色传给原生 GDI 前清除 alpha 字节，补入 child / host 两层 PrintWindow 的实际主题像素断言；同一配置字体和测量后的两行居中排版已看图确认。
6. TreeView 的状态型展开按钮与 TreeCtrl 的索引型皮肤分离；Toolbar 使用此分支实际支持的横向尺寸契约；Rebar 仅本地覆盖硬编码浅色绘制。
7. Toggle 文字、Tile 换行、MC 单元格省略、Spin 非循环边界、隐藏 RichEdit 初始化通知、真实 Sender 类名及单选树 / RadioBox 键盘反馈均已修正。

## 未覆盖范围

- **57 是具有代码场景的类数，不是 57 个独立 live 实例，也不是 57 类的所有状态都完成运行或像素验收。**
- ActiveX 仅 recipe；未实例化 / 激活外部 COM，外部组件内部 UI 不计入 Fluent 换肤覆盖。KeyboardSpacer 的移动端键盘避让未验证。
- 没有实际 Linux / macOS / Android / OHOS 构建运行；SVG-OFF 只检查配置边界；Release 只编译资源。
- ComboBox 分层下拉在本机 PrintWindow 的像素捕获仍有限制；已验证实际展开、定向键盘选择及选中值，**不声称全部下拉弹层像素通过**。已捕获并复查的 MenuEx 弹层不代表所有弹层。
- 没有覆盖屏幕阅读器、所有 IME 组合、跨显示器真实 DPI、减少动态效果、真实触控或每个控件的全部指针 / 键盘状态。
- 模态“保存”、链接与菜单是本地交互示例，不保存用户文件、不发起网络导航；主题不持久化。
- 本记录为本地提交前的验证证据；未新增运行时依赖，不代表远端推送、PR、线上 CI 或发布结果。


## 2026-10-15：滚轮路由修复

### 用户问题、验收与实施边界

用户反馈：键盘焦点留在左侧导航时，即使鼠标位于中央内容区，滚轮仍滚动侧栏。
验收要求是按本次鼠标位置命中最上层控件；嵌套滚动优先内层，到边界仅向命中分支的祖先冒泡；不改变键盘焦点，不破坏 hover / enter / leave。

先补回归并确认失败，再修复实现，最后执行专项、完整无头、完整 GUI 和单进程重复验证。涉及公共框架是因为宿主坐标处理与滚动后鼠标状态刷新属于共享输入路径，不能只在 Gallery 吞事件或强制改焦点。

### 根因与修改

1. Gallery 主 XML 未启用已有的 `sendWheel2Hover`。主窗现显式设置为 `1`，所有页面共用鼠标命中策略；框架默认值及显式 `0` 的 legacy 焦点策略保持兼容。
2. `SHostWnd::OnMouseWheel` 忽略本次 wheel 的屏幕坐标，沿用上次鼠标移动的缓存。现将本次坐标转换为客户区坐标，再送入容器；不依赖预先收到 `WM_MOUSEMOVE`。
3. `SwndContainerImpl` 使用局部命中结果，不再直接替换由 mousemove 管理的 `m_hHover`，以免跳过旧目标的 leave / 新目标的 enter。捕获优先保持，目标消息坐标与命中使用相同逆变换。
4. 独立代码审阅指出：真实 `SPanel` 原先把收到的目标坐标当作宿主坐标，回放 `WM_MOUSEMOVE`；经过矩阵变换的控件会再次错误命中。已将滚动后的 move / cursor 刷新集中到原生宿主，保留原始客户区坐标，在整个滚轮派发及嵌套冒泡后刷新一次。直接给逻辑控件发送 wheel 不再额外合成原生鼠标移动。
5. 嵌套列表边界回归另外暴露 `SListView::UpdateScrollBar` 的 inclusive range 少算末端：外层布局会让已到底的内层回退 1px。竖向、横向两处钳制统一为 `nMax - nPage + 1`；未削弱 GUI 的“内层不移动”断言，也未扩散修改其他列表实现。

未新增公共 API、运行时依赖或图片资源，没有改变布局、主题或为滚轮抢占键盘焦点。

### 回归证据

- `test_mousewheel.cpp` 新增 **6 个无头单元回归**：顶层 / 隐藏 / 透传命中、hover 生命周期、变换后的 hit / capture / focus 坐标、capture 与 legacy 策略、禁用 / 未命中不回退到无关焦点、ListView 竖横两方向底边在父级移位布局后保持。
- `test_gui_gallery.cpp` 新增 **3 个真实 HWND 回归**：并排区域双向滚动（浅 / 深主题，旧 mousemove 与 wheel 点冲突，以及 leave 后不先 move）、真实 200 行虚拟列表优先滚动及底边向外冒泡、带 400px 平移矩阵的真实 ScrollView 滚动后 hover / enter / leave 配对。
- GUI 刺激统一把客户区点经 `ClientToScreen` 转为原生 `WM_MOUSEWHEEL` 的屏幕坐标；不使用全局 `SendInput`，不在 fixture 内覆盖发布 XML 的滚轮策略。
- 首轮 **7 项中 4 项失败**，包含错误滚动侧栏、hover 遗留及变换坐标错误；ListView 底边独立回归在旧实现竖 / 横均出现 `600 -> 599`；变换 ScrollView 独立回归先证明滚动后 hover 跳到其他分支及多发 leave。修复后 **9/9 专项通过**。

### 实际执行结果

Windows x64 / VS 2022 / Debug；GDI 渲染，`SOUI_ENABLE_GUI_SMOKE=ON`、`SOUI_ENABLE_SVG=ON`。

~~~powershell
cmake --build build --config Debug --target SOUIWidgetsGalleryDemo fun_test --parallel 4
ctest --test-dir build -C Debug -N -L '^soui-headless$'
ctest --test-dir build -C Debug -R '^soui_mousewheel\.|^window\.gui_gallery_.*wheel' --output-on-failure
ctest --test-dir build -C Debug -L '^soui-headless$' --output-on-failure
ctest --test-dir build -C Debug -L '^soui-gui$' --output-on-failure
.\build\bin\Debug\fun_test.exe '--gtest_filter=soui_mousewheel.*:gallery*:window.gui_gallery_*' --gtest_repeat=3
python -B -m unittest discover -s demos/SOUIWidgetsGalleryDemo -p 'test_*.py'
python -B -m ruff check --no-cache demos/SOUIWidgetsGalleryDemo
python -B demos/SOUIWidgetsGalleryDemo/generate_assets.py --check
python -B demos/SOUIWidgetsGalleryDemo/build_catalog.py --check
python -B demos/SOUIWidgetsGalleryDemo/build_resources.py --check
git diff --check
~~~

| 检查 | 本次结果 |
|---|---|
| Demo + fun_test Debug 构建 | 通过 |
| 滚轮专项 | **9/9**：6 个无头单元、3 个真实窗口 GUI |
| 完整无头 | **555 注册，554 执行并通过，1 个原有 disabled 未执行**；493 单元注册、1 集成、61 无窗口 E2E |
| 完整 GUI | **30/30**，包括既有菜单、弹层、模态、Native host、Dock 与 UIEditor 回归 |
| 单进程重复 | **37 × 3 全部通过**：19 个 Gallery 模型 / 资源、12 个 Gallery GUI、6 个滚轮单元 |
| Python / 静态资源 | **21/21**、Ruff、187 文件资源审计及三个生成物 `--check` 通过 |
| 格式 / diff | 两个新增 / 修改测试文件及公共实现修改行的 clang-format 检查、`git diff --check` 通过 |

日志保存于仓库忽略目录 `output/wheel-routing/`：`regression-red.log`、`list-boundary-red.log`、`transform-red.log`、`regression-green.log`、`headless.log`、`gui.log`、`repeat.log`、`resources.log` 与构建日志。
既有 disabled 仍是 `swinx_misc.is_dbcs_lead_byte_codepage_dependent`，本轮没有新增禁用项。

独立 AI 代码审阅首轮提出 1 项 P2（变换后的目标坐标被当作宿主坐标回放）；已通过上述真实 ScrollView 先失败、后通过的回归修复。复查结论为本轮范围内无新增阻塞性缺陷；审阅本身不替代测试，最终版完整无头与 30 项 GUI 已由主代理实际重跑并通过。

### 验证限制

- 实际窗口测试证明本进程 HWND 的原生消息入口、命中、滚动及事件状态，不等同于真实鼠标硬件、系统跨应用滚轮路由或全部视觉像素验收。
- 未执行 Linux / macOS / Android / OHOS、完整 Release、多显示器负坐标及分数 DPI 真机输入测试。
- 没有全仓强制切换默认策略；其他应用需要按鼠标命中分发时仍应设置 `sendWheel2Hover="1"`。本次仅 Gallery 主窗改变该配置。


## 2026-10-19：滚动边界不再传递到父容器

本节取代 2026-10-15 中“内层到边界向祖先冒泡”的验收要求；旧记录仅描述当时行为。

- 用户要求：鼠标所在滚动视窗到顶或到底后继续滚轮，父容器保持不动。
- 计划与范围：先更新真实 Gallery 边界测试并新增最低层回归，再修改共享 `SPanel::OnMouseWheel`，最后运行专项、完整无头、GUI 与重复测试。共享实现需修改，因为边界转发发生在框架，而非 Gallery 页面。
- 实现：有滚动条的面板消费滚轮及越界剩余量，返回已处理；没有滚动条的容器不累积滚轮，原样转发到父级。普通文字 / div 仍可驱动所属滚动视窗；不改变焦点、鼠标命中、宿主刷新和主题布局。
- 回归：新单元覆盖竖向 / 水平滚动条、上下边界、一次多格越界、连续滚轮、半格累积及无条容器的原样转发。真实 HWND 测试在浅 / 深主题下检查 200 行虚拟列表上下两端连续滚轮，父级、侧栏和焦点保持不变。
- RED：旧实现上新增单元及更新后的 Gallery GUI 均失败，观测到父级收到边界滚轮 / 父级位置变化；GREEN：修复后滚轮专项 10/10 通过。

实际验证环境：Windows x64 / VS 2022 / Debug。执行：

~~~powershell
cmake --build build --config Debug --target SOUIWidgetsGalleryDemo fun_test --parallel 4
ctest --test-dir build -C Debug -R '^soui_mousewheel\.|^window\.gui_gallery_.*wheel' --output-on-failure
ctest --test-dir build -C Debug -L '^soui-headless$' --output-on-failure
ctest --test-dir build -C Debug -L '^soui-gui$' --output-on-failure
.\build\bin\Debug\fun_test.exe '--gtest_filter=soui_mousewheel.*:window.gui_gallery_*wheel*' --gtest_repeat=3
git diff --check
~~~

结果：Demo 与测试构建通过；专项 **10/10**；完整无头 **556 注册、555 通过、1 个既有 disabled**；完整 GUI **30/30**；滚轮专项单进程 **10 × 3 全部通过**；两份测试文件 clang-format 检查与 diff 空白检查通过。
日志位于忽略目录 `output/wheel-boundary-*.log`。

限制：此次改变所有使用 SPanel 默认滚轮处理的控件边界策略，不再提供旧的边界向父级传递行为。未验证其他平台、Release、真实鼠标硬件或全量视觉像素；窗口回归使用本进程 HWND 原生消息，不将无头测试冒充 GUI 验收。
