# SOUI 内置公开控件清点

## 口径

- 以框架默认注册入口 `SObjectDefaultRegister::RegisterWindows`（`SOUI/src/SApp.cpp:134-199`）与各类 `DEF_SOBJECT` 交叉核对；仅声明名称、存在头文件或被 include，不等于已默认注册。
- 默认入口使用 `TplRegisterFactory<T>`；`RegisterWindowClass<T>`（`SOUI/include/SApp.h:662-667`）最终也调用该工厂注册方法。
- **共 58 个独立 Window 类：57 个无条件注册 + 1 个平台条件注册。** `SActiveX` 仅在 `defined(_WIN32) && !defined(__MINGW32__)` 时注册（`SOUI/src/SApp.cpp:196-198`）；其余配置为 57 项。注册声明不证明各平台运行依赖或视觉行为一致。
- 本文件仅提供框架清单与分类边界，**不代表 Gallery 已实现、已覆盖或已通过运行／视觉验收**；Gallery 分类映射与覆盖状态由独立矩阵维护。
- 下列定义／注册路径均相对仓库根目录；定义行是 `DEF_SOBJECT` 所在行。分类仅用于清点，不绑定 Gallery 的分类结构。

## 内置 Window 分类表（58 项）

### Button、链接与选择（8）

| C++ class | XML tag | 定义 | 默认注册 | 用途／边界 |
|---|---|---|---|---|
| `SButton` | `button` | `SOUI/include/control/SCmnCtrl.h:205` | `SOUI/src/SApp.cpp:140` | 普通按钮 |
| `SImageButton` | `imgbtn` | `SOUI/include/control/SCmnCtrl.h:390` | `SOUI/src/SApp.cpp:143` | 图像按钮 |
| `SCheckBox` | `check` | `SOUI/include/control/SCmnCtrl.h:972` | `SOUI/src/SApp.cpp:145` | 复选框 |
| `SRadioBox` | `radio` | `SOUI/include/control/SCmnCtrl.h:1195` | `SOUI/src/SApp.cpp:147` | 单选框 |
| `SRadioGroup` | `radioGroup` | `SOUI/include/control/SCmnCtrl.h:1392` | `SOUI/src/SApp.cpp:148` | 单选组容器 |
| `SToggle` | `toggle` | `SOUI/include/control/SCmnCtrl.h:1447` | `SOUI/src/SApp.cpp:154` | 独立派生类，非 check 别名 |
| `SSwitch` | `switch` | `SOUI/include/control/SSwitch.h:29` | `SOUI/src/SApp.cpp:186` | 开关 |
| `SLink` | `link` | `SOUI/include/control/SCmnCtrl.h:109` | `SOUI/src/SApp.cpp:149` | 链接 |

### 文本输入、组合框与搜索下拉（8）

| C++ class | XML tag | 定义 | 默认注册 | 用途／边界 |
|---|---|---|---|---|
| `SEdit` | `edit` | `SOUI/include/control/SEdit.h:14` | `SOUI/src/SApp.cpp:180` | 文本编辑 |
| `SRichEdit` | `richedit` | `SOUI/include/control/SRichEdit.h:36` | `SOUI/src/SApp.cpp:179` | 富文本编辑 |
| `SHotKeyCtrl` | `hotkey` | `SOUI/include/control/SHotKeyCtrl.h:19` | `SOUI/src/SApp.cpp:168` | 快捷键输入 |
| `SSpinButtonCtrl` | `spinButton` | `SOUI/include/control/SSpinButtonCtrl.h:25` | `SOUI/src/SApp.cpp:169` | 步进按钮 |
| `SComboBox` | `combobox` | `SOUI/include/control/SComboBox.h:27` | `SOUI/src/SApp.cpp:182` | 组合框 |
| `SComboView` | `comboview` | `SOUI/include/control/SComboView.h:15` | `SOUI/src/SApp.cpp:183` | 组合视图 |
| `SComboEdit` | `ComboEdit` | `SOUI/include/control/SComboBase.h:25` | `SOUI/src/SApp.cpp:181` | 已注册的组合框嵌入编辑器；配套覆盖 |
| `SSearchDropdownList` | `searchdropdownlist` | `SOUI/include/control/SSearchDropdownList.h:26` | `SOUI/src/SApp.cpp:187` | 零尺寸搜索下拉入口；非独立编辑框 |

### 列表、表头与虚拟视图（6）

| C++ class | XML tag | 定义 | 默认注册 | 用途／边界 |
|---|---|---|---|---|
| `SListBox` | `listbox` | `SOUI/include/control/SListbox.h:54` | `SOUI/src/SApp.cpp:167` | 传统列表框 |
| `SListCtrl` | `listctrl` | `SOUI/include/control/SListCtrl.h:84` | `SOUI/src/SApp.cpp:166` | 传统列表控件 |
| `SHeaderCtrl` | `header` | `SOUI/include/control/SHeaderCtrl.h:32` | `SOUI/src/SApp.cpp:165` | 独立注册的表头 |
| `SListView` | `listview` | `SOUI/include/control/SListView.h:25` | `SOUI/src/SApp.cpp:170` | Adapter 虚拟列表 |
| `SMCListView` | `mclistview` | `SOUI/include/control/SMCListView.h:28` | `SOUI/src/SApp.cpp:171` | Adapter 多列虚拟列表 |
| `STileView` | `tileview` | `SOUI/include/control/STileView.h:41` | `SOUI/src/SApp.cpp:172` | Adapter 平铺视图 |

### 树（2）

| C++ class | XML tag | 定义 | 默认注册 | 用途／边界 |
|---|---|---|---|---|
| `STreeCtrl` | `treectrl` | `SOUI/include/control/STreeCtrl.h:105` | `SOUI/src/SApp.cpp:163` | 传统树 |
| `STreeView` | `treeview` | `SOUI/include/control/STreeView.h:281` | `SOUI/src/SApp.cpp:173` | Adapter 虚拟树 |

### 容器、分页、分割与模态（17）

| C++ class | XML tag | 定义 | 默认注册 | 用途／边界 |
|---|---|---|---|---|
| `SWindow` | `window` | `SOUI/include/core/SWnd.h:355` | `SOUI/src/SApp.cpp:136` | 基础窗口／容器 |
| `SRootWindow` | `root` | `SOUI/include/core/SRootWindow.h:32` | `SOUI/src/SApp.cpp:137` | 宿主根；配套场景 |
| `SPanel` | `div` | `SOUI/include/core/SPanel.h:39` | `SOUI/src/SApp.cpp:138` | 滚动面板基础类 |
| `SScrollView` | `scrollview` | `SOUI/include/core/SPanel.h:697` | `SOUI/src/SApp.cpp:152` | 滚动视图 |
| `SGroup` | `group` | `SOUI/include/control/SCmnCtrl.h:1506` | `SOUI/src/SApp.cpp:150` | 分组容器 |
| `SFrame` | `frame` | `SOUI/include/control/SFrame.h:14` | `SOUI/src/SApp.cpp:176` | 框架容器，非同名布局对象 |
| `STabCtrl` | `tabctrl` | `SOUI/include/control/STabCtrl.h:124` | `SOUI/src/SApp.cpp:155` | 分页容器 |
| `STabPage` | `page` | `SOUI/include/control/STabCtrl.h:29` | `SOUI/src/SApp.cpp:156` | Tab 子页；随容器覆盖 |
| `SStackView` | `stack` | `SOUI/include/control/SStackView.h:233` | `SOUI/src/SApp.cpp:177` | 堆栈分页容器 |
| `SStackPage` | `stackPage` | `SOUI/include/control/SStackView.h:176` | `SOUI/src/SApp.cpp:178` | Stack 子页；随容器覆盖 |
| `SSplitWnd` | `splitwnd` | `SOUI/include/control/SSplitWnd.h:69` | `SOUI/src/SApp.cpp:159` | 通用分割容器 |
| `SSplitWnd_Col` | `splitcol` | `SOUI/include/control/SSplitWnd.h:286` | `SOUI/src/SApp.cpp:160` | 独立方向派生类，非 splitwnd 别名 |
| `SSplitWnd_Row` | `splitrow` | `SOUI/include/control/SSplitWnd.h:306` | `SOUI/src/SApp.cpp:161` | 独立方向派生类，非 splitwnd 别名 |
| `SSplitPane` | `pane` | `SOUI/include/control/SSplitWnd.h:27` | `SOUI/src/SApp.cpp:158` | 分割窗格子项；随容器覆盖 |
| `SModalRoot` | `modalroot` | `SOUI/include/core/SModalViewSession.h:73` | `SOUI/src/SApp.cpp:193` | 模态会话根／遮罩；配套场景 |
| `SModalView` | `modalview` | `SOUI/include/core/SModalViewSession.h:168` | `SOUI/src/SApp.cpp:194` | 模态内容区 |
| `SKeyboardSpacer` | `keyboardSpacer` | `SOUI/include/control/SCmnCtrl.h:1564` | `SOUI/src/SApp.cpp:191` | 键盘避让辅助场景 |

### 文本、图像、数值、日期与分隔（10）

| C++ class | XML tag | 定义 | 默认注册 | 用途／边界 |
|---|---|---|---|---|
| `SStatic` | `text` | `SOUI/include/control/SCmnCtrl.h:30` | `SOUI/src/SApp.cpp:139` | 静态文本 |
| `SImageWnd` | `img` | `SOUI/include/control/SCmnCtrl.h:415` | `SOUI/src/SApp.cpp:141` | 图像 |
| `SIconWnd` | `icon` | `SOUI/include/control/SCmnCtrl.h:1136` | `SOUI/src/SApp.cpp:146` | 图标 |
| `SAnimateImgWnd` | `animateimg` | `SOUI/include/control/SCmnCtrl.h:571` | `SOUI/src/SApp.cpp:151` | 动画图像 |
| `SProgress` | `progress` | `SOUI/include/control/SCmnCtrl.h:720` | `SOUI/src/SApp.cpp:142` | 进度条 |
| `SSliderBar` | `sliderbar` | `SOUI/include/control/SSliderBar.h:96` | `SOUI/src/SApp.cpp:162` | 滑块 |
| `SScrollBar` | `scrollbar` | `SOUI/include/control/SScrollbar.h:30` | `SOUI/src/SApp.cpp:164` | 独立滚动条 |
| `SHrLine` | `hr` | `SOUI/include/control/SCmnCtrl.h:902` | `SOUI/src/SApp.cpp:144` | 分隔线 |
| `SCalendar` | `calendar` | `SOUI/include/control/SCalendar.h:136` | `SOUI/src/SApp.cpp:174` | 日历 |
| `SDateTimePicker` | `dateTimePicker` | `SOUI/include/control/SDateTimePicker.h:20` | `SOUI/src/SApp.cpp:175` | 日期时间选择 |

### 命令栏、标题与原生宿主（7）

| C++ class | XML tag | 定义 | 默认注册 | 用途／边界 |
|---|---|---|---|---|
| `SMenuBar` | `menubar` | `SOUI/include/control/SMenuBar.h:24` | `SOUI/src/SApp.cpp:185` | 菜单栏 |
| `SToolBar` | `toolbar` | `SOUI/include/control/SToolBar.h:56` | `SOUI/src/SApp.cpp:188` | 工具栏 |
| `SDockBar` | `dockbar` | `SOUI/include/control/SDockBar.h:12` | `SOUI/src/SApp.cpp:189` | 停靠栏 |
| `SRebar` | `rebar` | `SOUI/include/control/SRebar.h:35` | `SOUI/src/SApp.cpp:190` | Rebar 容器 |
| `SCaption` | `caption` | `SOUI/include/control/SCaption.h:24` | `SOUI/src/SApp.cpp:184` | 标题栏 |
| `SRealWnd` | `realwnd` | `SOUI/include/control/SRealWnd.h:23` | `SOUI/src/SApp.cpp:153` | 原生窗口嵌入宿主 |
| `SActiveX` | `activex` | `SOUI/include/control/SActiveX.h:19` | `SOUI/src/SApp.cpp:197` | 仅 _WIN32 且非 __MINGW32__ |

## 复合、辅助与别名边界

- **已默认注册的配套类仍计入 58 项**：`SComboEdit` 是组合框内嵌编辑器（构造入口：`SOUI/src/control/SComboBase.cpp:103-127`）；`SRootWindow`、`SModalRoot` 是宿主／模态根；`STabPage`、`SStackPage`、`SSplitPane` 是容器子项。它们可在复合场景核验，不要求每类独立展示卡片。
- `SSearchDropdownList` 是已注册的零尺寸弹层入口，不是另一个文本编辑框；尺寸实现见 `SOUI/include/control/SSearchDropdownList.h:53-76`。
- **58 个 Window 类均无注册别名**。`SStatic/text`、`SPanel/div` 是正式名称，不另含 `static`、`panel` 别名；`SToggle`、`SSplitWnd_Col`、`SSplitWnd_Row` 等派生类不是父类别名。
- 工厂名称统一小写，大小写变体不另计类；显式别名由 `DEF_SOBJECT_EX` 提供并按对象类型注册（`SOUI/src/core/SObjectFactory.cpp:6-14,36-58`）。
- `button.ok` 是创建 `button` 后设置 `class="button.ok"` 的语法，不是新控件／别名（`SOUI/src/SApp.cpp:846-857`）。

下列显式名称声明**未进入默认 Window 注册表**，不增加内置分母；菜单标签仅在专用上下文中有效。

| C++ class | 声明名称 | 定义 | 边界 |
|---|---|---|---|
| `SComboBase` | `combobase` | `SOUI/include/control/SComboBase.h:102` | 抽象组合框基类 |
| `SOsrPanel` | `osrPanel` | `SOUI/include/core/SItemPanel.h:93` | 离屏面板基础类 |
| `SItemPanel` | `ItemPanel` | `SOUI/include/core/SItemPanel.h:262` | 虚拟视图项面板 |
| `SNcPanel` | `ncpanel` | `SOUI/include/core/SNcPainter.h:373` | 非客户区绘制面板 |
| `STabSlider` | `tabslider` | `SOUI/src/control/STabCtrl.cpp:11` | Tab 切换内部辅助类 |
| `SMenuBarItem` | `menuItem` | `SOUI/src/control/SMenuBar.cpp:37` | 菜单栏局部构造 |
| `SMenuExItem` | `menuItem` | `SOUI/include/helper/SMenuEx.h:17` | 弹出菜单专用项 |
| `SMenuExRoot` | `menuRoot` | `SOUI/src/helper/SMenuEx.cpp:17` | 弹出菜单专用根 |
| `SMenuExSep` | `sep` | `SOUI/src/helper/SMenuEx.cpp:472` | 弹出菜单分隔项 |

两个 `menuItem` 对应不同上下文的不同 C++ 类，不是彼此别名，也不是默认通用窗口 tag；菜单局部工厂见 `SOUI/src/helper/SMenuEx.cpp:532-547`。`SDropDownWnd`（`SOUI/include/control/SDropDown.h:43`）、`SDockFloatWnd`（`SOUI/include/control/SDockFloatWnd.h:18`）是宿主窗口设施，亦不在上述注册表内。

## 默认注册的非控件对象（63 类，不计入控件分母）

| 对象类型 | 类数 | 示例（class / 注册名） | 定义示例 | 默认注册 |
|---|---|---|---|---|
| Skin | 13 | `SSkinButton/button`、`SSkinScrollbar/scrollbar` | `SOUI/include/core/SSkin.h:388,707` | `SOUI/src/SApp.cpp:203-215` |
| Layout | 10 | `SVBox/vbox`、`SGridLayout/gridLayout` | `SOUI/include/layout/SLinearLayout.h:211`；`SOUI/include/layout/SGridLayout.h:144` | `SOUI/src/SApp.cpp:220-229` |
| Interpolator | 27 | `SLinearInterpolator/Linear`、`SBounceInterpolator/Bounce` | `SOUI/include/animation/SInterpolatorImpl.h:17,213` | `SOUI/src/SApp.cpp:234-260` |
| Animation | 6 | `SAlphaAnimation/alpha`、`SAnimationSet/set` | `SOUI/include/animation/SAlphaAnimation.h:15`；`SOUI/include/animation/SAnimationSet.h:32` | `SOUI/src/SApp.cpp:265-270` |
| ValueAnimator | 7 | `SIntAnimator/IntAnimator`、`SColorAnimator/ColorAnimator` | `SOUI/include/valueAnimator/SValueAnimator.h:609,765` | `SOUI/src/SApp.cpp:274-280` |

- `hbox/vbox/hflow/vflow` 是 Layout 对象名，不是默认 Window tag；布局通过 `layout="..."` 创建（`SOUI/src/core/SWnd.cpp:4172-4179`）。
- 默认非控件中的真别名：`SGridLayout` 的 `gridLayout → grid`（`SOUI/include/layout/SGridLayout.h:144`）、`SFlowLayout` 的 `flowLayout → flow`（`SOUI/include/layout/SFlowLayout.h:149`）、`SFrameLayout` 的 `frameLayout → frame`（`SOUI/include/layout/SFrameLayout.h:86`）；这 3 个别名不增加类数。
- `SFrame/frame`（Window）与布局别名 `frame`（Layout）不同；Skin 中的 `button/scrollbar/group` 也与同名 Window 分属不同对象类型。
- 不要把名称声明误当默认注册：`SViewSwitchAnimator/ViewSwitchAnimator` 虽位于 `SOUI/include/control/SStackView.h:30`，仍是动画器且未默认注册；`SValueAnimator`、`SPropertyAnimator`、`SAnimatorSet` 的声明（分别见 `SOUI/include/valueAnimator/SValueAnimator.h:32`、`SOUI/include/valueAnimator/SPropertyAnimator.h:108`、`SOUI/include/valueAnimator/SAnimatorSet.h:28`）亦不属于上表的默认注册项。

## controls.extend 边界（全部排除于内置 58 项）

- `controls.extend/SCtrlsRegister.cpp:52-106` 的显式入口包含 **37 条 Window + 5 条 Skin 注册声明**；第 98 行另调用 `controls.extend/hellocharts/SHelloCharts.cpp:6-15` 的 **6 条图表 Window 注册声明**。扩展链共 43 条窗口注册声明，含平台条件，不表示任一平台默认注册 43 项。
- 扩展示例：`SGifPlayer/gifplayer`（定义 `controls.extend/gif/SGifPlayer.h:33`，注册 `controls.extend/SCtrlsRegister.cpp:59`）；`SPropertyGrid/propgrid`（定义 `controls.extend/propgrid/SPropertyGrid.h:61`，注册 `controls.extend/SCtrlsRegister.cpp:67`）；`SGridCtrl/gridctrl`（定义 `controls.extend/gridctrl/SGridCtrl.h:512`，注册 `controls.extend/SCtrlsRegister.cpp:68`）。它们以及图表、扩展编辑器等均不可混入内置分母。
- 扩展中的非控件：`SSkinVScrollbar/vscrollbar`、`SSkinHScrollbar/hscrollbar` 是 Skin（定义 `controls.extend/SVscrollbar.h:11`、`controls.extend/SHscrollbar.h:11`；注册 `controls.extend/SCtrlsRegister.cpp:102-103`），不是两个滚动条控件。
- `SSkinAni/ani` 的真别名为 `gif|apng`（定义 `controls.extend/gif/SSkinAni.h:8`，注册 `controls.extend/SCtrlsRegister.cpp:105`）；`gif`、`apng` 都是扩展 Skin 别名，不是内置控件。

## 清点核验

分类表 58 行与默认注册的 58 个独立 class、58 个唯一 Window tag 一一对应，定义／注册行已交叉核对，无漏项或重复，Window 别名为 0。此结论限于静态源码清点；不证明控件运行、跨平台行为、Gallery 实现或视觉覆盖。
