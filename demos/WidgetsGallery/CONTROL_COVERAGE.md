# Gallery 内置控件覆盖矩阵

由 `build_catalog.py` 从 `control_catalog.json` 与实际 XML 生成。

## 统计口径

- 框架默认注册 **58 类**：57 类无条件注册 + Windows 非 MinGW 的 SActiveX。
- **57 类有真实运行场景**（独立实例、复合子控件或按需创建的模态会话）；SActiveX 仅提供集成用法，不激活外部 COM。
- 同一控件的配色/方向/禁用状态不重复计数。GalleryButton / GalleryTabCtrl / GalleryRebar 为本 Demo 的绘制子类，不算新增内置类。
- Layout、Skin、动画对象以及 controls.extend 不混入此分母。分类依据见 `BUILTIN_CONTROLS.md`。

## 查看与运行

选择分类后点 **View XML**，查看从真实示例抽取的 XML；模板和 buddy 引用一起保留。
虚拟视图的完整数据绑定及排序示例在 `GalleryAdapters.h`，事件与会话初始化在 `MainDlg.cpp`。
`live`=独立运行实例；`composite`=真实复合子节点；`dynamic`=按钮启动后实际创建；`platform_recipe`=有前提的用法，不算 live。

## Overview

| C++ 类 | XML 标签 | 样式 / 资源 | 命名实例 | 覆盖 |
|---|---|---|---|---|

## Buttons

| C++ 类 | XML 标签 | 样式 / 资源 | 命名实例 | 覆盖 |
|---|---|---|---|---|
| `SButton` | `button` | fluent_btn_primary | `sample_native_button` | live |
| `SImageButton` | `imgbtn` | gallery_image_button | `sample_image_button` | live |
| `SLink` | `link` | fluent_link | `sample_link` | live |

## Selection

| C++ 类 | XML 标签 | 样式 / 资源 | 命名实例 | 覆盖 |
|---|---|---|---|---|
| `SCheckBox` | `check` | _skin.sys.checkbox / focuscheckbox | `sample_check` | live |
| `SRadioBox` | `radio` | _skin.sys.radio / focusradio | `sample_radio_light` | live |
| `SRadioGroup` | `radioGroup` | themed radio children | `sample_radiogroup` | composite |
| `SToggle` | `toggle` | gallery_toggle / gallery_focus | `sample_toggle` | live |
| `SSwitch` | `switch` | gallery_switch_track / thumb | `sample_switch` | live |

## Text input

| C++ 类 | XML 标签 | 样式 / 资源 | 命名实例 | 覆盖 |
|---|---|---|---|---|
| `SHotKeyCtrl` | `hotkey` | fluent_edit | `sample_hotkey` | live |
| `SSpinButtonCtrl` | `spinButton` | gallery_arrow_up / down | `sample_spin` | live |
| `SRichEdit` | `richedit` | fluent_edit | `sample_richedit` | live |
| `SEdit` | `edit` | fluent_edit | `sample_input` | live |
| `SKeyboardSpacer` | `keyboardSpacer` | IME-dependent layout (desktop height 0) | `sample_keyboardspacer` | composite |

## ComboBox

| C++ 类 | XML 标签 | 样式 / 资源 | 命名实例 | 覆盖 |
|---|---|---|---|---|
| `SComboEdit` | `ComboEdit` | transparent nested edit / Fluent text | `sample_comboedit` | composite |
| `SComboBox` | `combobox` | fluent_edit / _skin.sys.dropbtn | `sample_combo` | live |
| `SComboView` | `comboview` | fluent_edit / virtual item template | `sample_comboview` | live |
| `SSearchDropdownList` | `searchdropdownlist` | virtual listStyle / dropdownStyle | `sample_suggestions` | live |

## Progress

| C++ 类 | XML 标签 | 样式 / 资源 | 命名实例 | 覆盖 |
|---|---|---|---|---|
| `SProgress` | `progress` | gallery_progress_track / blue | `progress_standard` | live |

## Sliders & scrollbars

| C++ 类 | XML 标签 | 样式 / 资源 | 命名实例 | 覆盖 |
|---|---|---|---|---|
| `SSliderBar` | `sliderbar` | gallery_progress_track / slider_thumb | `sample_slider` | live |
| `SScrollBar` | `scrollbar` | _skin.sys.scrollbar | `sample_scrollbar` | live |

## ListBox

| C++ 类 | XML 标签 | 样式 / 资源 | 命名实例 | 覆盖 |
|---|---|---|---|---|
| `SListBox` | `listbox` | _skin.sys.list.item | `sample_list` | live |

## TreeView

| C++ 类 | XML 标签 | 样式 / 资源 | 命名实例 | 覆盖 |
|---|---|---|---|---|
| `STreeCtrl` | `treectrl` | gallery_tree_toggle / themed item | `sample_tree` | live |
| `STreeView` | `treeview` | gallery_tree_expander / item template | `sample_treeview` | live |

## Tabs & stacks

| C++ 类 | XML 标签 | 样式 / 资源 | 命名实例 | 覆盖 |
|---|---|---|---|---|
| `STabCtrl` | `tabctrl` | _skin.sys.tab.page / gallery_focus | `sample_tabs` | live |
| `STabPage` | `page` | gallery_subtle | `sample_tabpage` | composite |
| `SStackView` | `stack` | gallery_subtle | `sample_stack` | live |
| `SStackPage` | `stackPage` | gallery_subtle | `sample_stackpage` | composite |

## Dialogs

| C++ 类 | XML 标签 | 样式 / 资源 | 命名实例 | 覆盖 |
|---|---|---|---|---|
| `SModalRoot` | `modalroot` | 40% scrim / modal session | `sample_modalroot` | dynamic |
| `SModalView` | `modalview` | gallery_card | `sample_modalview` | dynamic |

## Icons & images

| C++ 类 | XML 标签 | 样式 / 资源 | 命名实例 | 覆盖 |
|---|---|---|---|---|
| `SImageWnd` | `img` | gallery_icon_layout | `sample_image` | live |
| `SIconWnd` | `icon` | SVG-to-HICON / gallery_icon_layout | `sample_icon` | live |
| `SAnimateImgWnd` | `animateimg` | gallery_spinner | `sample_animateimg` | live |

## Typography

| C++ 类 | XML 标签 | 样式 / 资源 | 命名实例 | 覆盖 |
|---|---|---|---|---|
| `SStatic` | `text` | fluent_text | `sample_static` | live |
| `SHrLine` | `hr` | gallery_stroke | `sample_hr` | live |

## Layout

| C++ 类 | XML 标签 | 样式 / 资源 | 命名实例 | 覆盖 |
|---|---|---|---|---|
| `SWindow` | `window` | gallery_subtle | `sample_window` | live |
| `SPanel` | `div` | _skin.sys.scrollbar | `sample_panel` | live |
| `SScrollView` | `scrollview` | _skin.sys.scrollbar | `sample_scrollview` | live |

## ListView

| C++ 类 | XML 标签 | 样式 / 资源 | 命名实例 | 覆盖 |
|---|---|---|---|---|
| `SHeaderCtrl` | `header` | _skin.sys.header / gallery_sort | `sample_header` | live |
| `SListCtrl` | `listctrl` | _skin.sys.list.item / header | `sample_listctrl` | live |
| `SListView` | `listview` | _skin.sys.list.item / two-line template | `sample_listview` | live |
| `SMCListView` | `mclistview` | _skin.sys.list.item / header / gallery_sort | `sample_mclistview` | live |

## TileView

| C++ 类 | XML 标签 | 样式 / 资源 | 命名实例 | 覆盖 |
|---|---|---|---|---|
| `STileView` | `tileview` | gallery_card / SVG icon | `sample_tileview` | live |

## Date & time

| C++ 类 | XML 标签 | 样式 / 资源 | 命名实例 | 覆盖 |
|---|---|---|---|---|
| `SCalendar` | `calendar` | gallery_arrow_prev / next / day colors | `sample_calendar` | live |
| `SDateTimePicker` | `dateTimePicker` | fluent_edit / themed calstyle | `sample_datetime` | live |

## Menus & toolbars

| C++ 类 | XML 标签 | 样式 / 资源 | 命名实例 | 覆盖 |
|---|---|---|---|---|
| `SMenuBar` | `menubar` | gallery_quiet / MenuEx items | `sample_menubar` | live |
| `SToolBar` | `toolbar` | gallery_toolbar_icons / quiet | `sample_toolbar` | live |
| `SRebar` | `rebar` | gallery_stroke / fluent_text_secondary | `sample_rebar` | live |

## Containers

| C++ 类 | XML 标签 | 样式 / 资源 | 命名实例 | 覆盖 |
|---|---|---|---|---|
| `SGroup` | `group` | gallery_stroke | `sample_group` | live |
| `SSplitPane` | `pane` | gallery_subtle | `sample_pane` | composite |
| `SSplitWnd` | `splitwnd` | gallery_separator | `sample_splitwnd` | live |
| `SSplitWnd_Col` | `splitcol` | gallery_separator | `sample_splitcol` | live |
| `SSplitWnd_Row` | `splitrow` | gallery_separator | `sample_splitrow` | live |
| `SFrame` | `frame` | gallery_subtle | `sample_frame` | live |
| `SDockBar` | `dockbar` | gallery_subtle / mini.close | `sample_dockbar` | live |

## Hosting & platform

| C++ 类 | XML 标签 | 样式 / 资源 | 命名实例 | 覆盖 |
|---|---|---|---|---|
| `SRootWindow` | `root` | _skin.sys.wnd.bkgnd | `sample_rootwindow` | composite |
| `SRealWnd` | `realwnd` | native owner-draw + Fluent colors | `sample_realwnd` | dynamic |
| `SCaption` | `caption` | Fluent caption / system-button SVGs | `sample_caption` | composite |
| `SActiveX` | `activex` | external COM server owns its internal UI | `sample_activex_usage` | platform_recipe |
