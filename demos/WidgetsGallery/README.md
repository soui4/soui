# SOUI Widgets Gallery / Fluent Demo

独立 C++ / XML 控件画廊，目标名 **SOUIWidgetsGalleryDemo**。使用原生 SOUI 控件与事件，不依赖 YogaUI、WebView，也不增加 Demo 运行时依赖。

## 目录与覆盖边界

- **20 分类**（含 Overview）：Overview、Buttons、Selection、Text input、ComboBox、Progress、Sliders & scrollbars、ListBox、TreeView、Tabs & stacks、Dialogs、Icons & images、Typography、Layout、ListView、TileView、Date & time、Menus & toolbars、Containers、Hosting & platform。
- **58 个框架注册类型**：57 个无条件注册 + 仅 Windows 非 MinGW 注册的 `SActiveX`。57 类具有 `live` / `composite` / `dynamic` 场景，分别指独立实例、真实复合子控件、按需创建的会话／宿主。
- `SActiveX` 仅为 **`platform_recipe`**：需匹配平台及已安装的 COM 服务；Demo 不激活外部 COM，**不能宣称已完成 ActiveX 激活验证**。
- `SKeyboardSpacer/keyboardSpacer` 是复合布局场景，**桌面高度为 0，移动端键盘避让未验证**。Layout、Skin、动画对象和 `controls.extend` 不混入控件分母；GalleryButton / GalleryTabCtrl / GalleryRebar 是 Demo 本地绘制子类，不另计内置类型。

目录的单一手工来源为 [control_catalog.json](control_catalog.json)；[build_catalog.py](build_catalog.py) 结合真实布局 XML，统一生成 [GalleryCatalog.h](GalleryCatalog.h) 和 [CONTROL_COVERAGE.md](CONTROL_COVERAGE.md)，不要手改生成产物。

- [BUILTIN_CONTROLS.md](BUILTIN_CONTROLS.md)：框架 class、XML tag、定义／注册及内部、扩展边界的事实清单。
- [CONTROL_COVERAGE.md](CONTROL_COVERAGE.md)：配套样式／资源、命名 XML 实例与场景范围。界面 **View XML** 展示从真实示例抽取的 XML，保留模板和 buddy 引用；完整 C++ 数据绑定／排序见 [GalleryAdapters.h](GalleryAdapters.h)，事件、对话框与宿主初始化见 [MainDlg.cpp](MainDlg.cpp)。XML 片段不能替代 C++ 初始化。
- [VERIFICATION.md](VERIFICATION.md)：实际测试结果、最终用例数、截图、Review 与未覆盖范围的唯一记录；本 README 不记“已通过”的最终测试数字。

## 主要功能

- 默认打开 Progress：25% 标准、60% 带文字、90% 圆角、范围 20–80／值 35；Animate / Reset 可操作，重复点击从当前值平滑重定向。
- 搜索匹配分类名、key、SOUI API 和 XML tag；空结果有提示，不丢失当前页面状态。按钮、输入、选择、列表增删、树、分页、堆栈、菜单、日期及模态会话使用原生事件。
- 浅／深色切换不重建控件树，保留输入、选择与数据状态；统一字体、间距、圆角、描边与状态皮肤。图标／主视觉为 SVG，窗口／弹层阴影使用可复现的九宫格资源，不修改框架或系统皮肤。
- 本地绘制适配见 [GalleryButton.h](GalleryButton.h)：GalleryButton / GalleryTabCtrl 统一焦点外观；`GalleryRebar : SRebar` 仅覆盖 `OnPaint`，用语义 token 替代核心硬编码浅色绘制，保留核心 layout、drag 和 children 行为。
- SRealWnd 使用独立的不透明宿主，布局完成后延迟创建真实子 HWND；原生绘制复用语义颜色及配置字体，不在分层主窗口中强行嵌入子窗口。
- 窗口可调整尺寸，导航与内容独立滚动；使用 dp、hbox / vbox / weight 布局。

真实数据不是静态占位文字，由 `Gallery::InitializeViews` 在 XML 初始化后安装：

| 控件 | 初始数据 | 绑定方式 |
|---|---|---|
| SListCtrl | 12 行 | InsertItem / SetSubItemText 的真实存储行与表头 |
| SListView | 200 行 | SAdapterBase，虚拟行绑定与回收 |
| SMCListView | 80 行 | SMcAdapterBase，多列数据及表头排序 |
| STileView | 30 项 | SAdapterBase，平铺模板绑定 |
| STreeView | 3 组 × 每组 4 个子项 | STreeAdapterBase，真实父子节点与展开状态 |
| SComboView／搜索建议 | 8 个本地候选 | 组合视图 Adapter；搜索建议按关键字过滤同一候选模型 |

## 构建与运行

在仓库根目录执行：

~~~powershell
cmake -S . -B build -DSOUI_BUILD_DEMOS=ON -DSOUI_ENABLE_SVG=ON
cmake --build build --config Debug --target SOUIWidgetsGalleryDemo fun_test --parallel 4
./build/bin/Debug/SOUIWidgetsGalleryDemo.exe
~~~

Windows Debug 使用源目录 FILE 资源，Release 将资源嵌入 PE；其他桌面平台使用构建时复制到可执行文件旁的 `SOUIWidgetsGalleryDemo-res`。框架、渲染器及系统资源依赖与其他 SOUI Demo 相同，不能据此宣称非 Windows 或移动端已验证。

## 资源与目录维护

当前资源索引含 **187 个 indexed 文件**；浅／深色**每套**各有 **96 个 Skin、29 个颜色 token、26 个图标**。这些是静态资源规模，不是 GUI 验收结果。

修改目录 JSON、真实 XML 或资源生成器后，按顺序同步 SVG／主题、目录／用法、真实 uiresbuilder 资源产物；不要手写生成的资源头或 PE 索引：

~~~powershell
python -B demos/SOUIWidgetsGalleryDemo/generate_assets.py
python -B demos/SOUIWidgetsGalleryDemo/build_catalog.py
python -B demos/SOUIWidgetsGalleryDemo/build_resources.py

# 检查现有产物
python -B demos/SOUIWidgetsGalleryDemo/generate_assets.py --check
python -B demos/SOUIWidgetsGalleryDemo/build_catalog.py --check
python -B demos/SOUIWidgetsGalleryDemo/build_resources.py --check
~~~

`build_resources.py --check` 使用真实 uiresbuilder 生成到临时目录后比较有效内容，不写入工程生成物；仅时间戳变化不会造成陈旧误报。

布局入口为 `uires/xml/gallery.xml`；会话／宿主示例见 `uires/xml/dialog.xml`、`uires/xml/gallery_modal.xml`、`uires/xml/gallery_native.xml`。主题和图标在 Demo 自有资源池内覆盖，沿用 `demos/fluent/FluentTheme.h` 管理资源及属性回放。

## 检查入口

以下是可执行入口，不表示本次已经运行或通过；实际证据统一维护在 [VERIFICATION.md](VERIFICATION.md)。

~~~powershell
python -B -m unittest discover -s demos/SOUIWidgetsGalleryDemo -p 'test_*.py'
python -B -m ruff check --no-cache demos/SOUIWidgetsGalleryDemo
ctest --test-dir build -C Debug -R '^gallery[._]' --output-on-failure
ctest --test-dir build -C Debug -L '^soui-headless$' --output-on-failure
~~~

### 无激活截图

`snapshot_catalog.py` 使用 `--snapshot` 启动并以 PrintWindow 采集本次进程窗口，**不激活前台、不发送桌面键鼠输入**：`--all` 顺序采集全部分类的浅／深色；`--page` 指定目录 key，`--section` 定位命名实例，`--dark` 使用深色，`--source` 打开 View XML，`--modal` 打开真实窗口内模态会话。

~~~powershell
python -B demos/SOUIWidgetsGalleryDemo/snapshot_catalog.py --all
python -B demos/SOUIWidgetsGalleryDemo/snapshot_catalog.py --page listview --section sample_mclistview --dark --source
python -B demos/SOUIWidgetsGalleryDemo/snapshot_catalog.py --page dialog --modal --dark
~~~

结果写入 `output/gallery-snapshots/`。截图与非黑图检查不证明点击、焦点、键盘或 ActiveX 激活行为。

### 导航、交互与 C++ 真实 GUI 专项

推荐 `tests/gui_pages.json` 的 **20 页导航**和 `tests/gui_catalog_actions.json` 的目录交互脚本；先启用 GUI 测试并构建，再运行 C++ 真实窗口专项（选择、虚拟视图回收、多列表头排序等）与脚本：

~~~powershell
cmake -S . -B build -DSOUI_ENABLE_GUI_SMOKE=ON -DSOUI_FUN_TEST_POST_BUILD=OFF
cmake --build build --config Debug --target SOUIWidgetsGalleryDemo fun_test --parallel 4
ctest --test-dir build -C Debug -R '^window[.]gui_gallery_' --output-on-failure
python -B demos/SOUIWidgetsGalleryDemo/gui_smoke.py --phase pages --toggle 1050 36 --dwell 0.3 --action-file demos/SOUIWidgetsGalleryDemo/tests/gui_pages.json
python -B demos/SOUIWidgetsGalleryDemo/gui_smoke.py --phase catalog --toggle 1050 36 --dwell 0.3 --action-file demos/SOUIWidgetsGalleryDemo/tests/gui_catalog_actions.json
python -B demos/SOUIWidgetsGalleryDemo/tests/gui_geometry.py
~~~

Python GUI 验证沿用工程已有 Pillow / pywin32 测试依赖；输入仅针对本次启动进程所属窗口，前台激活被拒则失败，不绕过系统策略或操作其他应用。坐标脚本按默认窗口布局运行；20 页标题去重用于防止漏导航，不替代布局、缩放、浅／深色画面和输入行为的专项验收。C++ 用例见 `demos/fun_test/test_gui_gallery.cpp`，注册为 `soui-gui`，不是无头门禁。
