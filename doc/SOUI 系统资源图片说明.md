# SOUI 系统资源图片说明文档

本文档基于 `soui-sys-resource/theme_sys_res/sys_xml_skin.xml` 配置文件，详细说明 SOUI 框架系统主题资源图片的功能和用途。

## 一、窗口与对话框相关图片

### 1. 窗口背景与边框
- **sys_wnd_bkgnd.png** (14x86) - 窗口背景皮肤
  - 用途：用作窗口默认背景皮肤（`_skin.sys.wnd.bkgnd`）
  - 特性：支持九宫格拉伸（left="5" right="5" top="5" bottom="5"）
  - 使用场景：主窗口、对话框的根容器背景

- **sys_border.png** (53x27) - 边框图片
  - 用途：通用边框绘制（`_skin.sys.border`）
  - 特性：2 种状态，支持 margin 拉伸
  - 使用场景：需要边框装饰的 UI 元素

### 2. 窗口控制按钮（标题栏按钮）
- **sys_btn_close.png** (91x31) - 关闭按钮
  - 用途：窗口关闭按钮皮肤（`_skin.sys.btn.close`）
  - 特性：3 种状态（正常/悬停/按下）
  - 使用场景：caption 中的关闭按钮

- **sys_btn_minimize.png** (91x31) - 最小化按钮
  - 用途：窗口最小化按钮皮肤（`_skin.sys.btn.minimize`）
  - 特性：3 种状态
  - 使用场景：caption 中的最小化按钮

- **sys_btn_maximize.png** (91x31) - 最大化按钮
  - 用途：窗口最大化按钮皮肤（`_skin.sys.btn.maximize`）
  - 特性：3 种状态
  - 使用场景：caption 中的最大化按钮

- **sys_btn_restore.png** (91x31) - 还原按钮
  - 用途：窗口还原按钮皮肤（`_skin.sys.btn.restore`）
  - 特性：3 种状态
  - 使用场景：caption 中的还原按钮（与最大化按钮切换使用）

### 3. 消息框图标
- **sys_msgicon.png** (241x49) - 消息框图标
  - 用途：消息框中的提示图标（`_skin.sys.msgicon`）
  - 特性：5 种状态（完成|done/警告|warning/信息|info/错误stop/问题确认|question）
  - 使用场景：MessageBox 等对话框中的图标显示

## 二、表单控件相关图片

### 1. 复选框与单选框
- **sys_checkbox.png** (129x17) - 复选框
  - 用途：标准复选框皮肤（`_skin.sys.checkbox`）
  - 特性：8 种状态，不支持按下即选中（checkAsPushdown="0"）
  - 状态包括：(正常/悬停/按下/禁用)*(未选中/选中)
  - 使用场景：表单中的多选控件

- **sys_radio.png** (129x17) - 单选框
  - 用途：标准单选框皮肤（`_skin.sys.radio`）
  - 特性：8 种状态，不支持按下即选中（checkAsPushdown="0"）
  - 状态包括：(正常/悬停/按下/禁用)*(未选中/选中)
  - 使用场景：表单中的单选控件组

- **sys_focus_checkbox.png** (17x17) - 焦点复选框
  - 用途：带焦点状态的复选框（`_skin.sys.focuscheckbox`）
  - 特性：1种状态, 叠加显示在复选框上
  - 使用场景：需要键盘导航的复选框

- **sys_focus_radio.png** (17x17) - 焦点单选框
  - 用途：带焦点状态的单选框（`_skin.sys.focusradio`）
  - 特性：1种状态, 叠加显示在单选框上
  - 使用场景：需要键盘导航的单选框

### 2. 下拉按钮
- **sys_dropbtn.png** (49x17) - 下拉按钮
  - 用途：下拉框的下拉箭头按钮（`_skin.sys.dropbtn`）
  - 特性：3 种状态（正常/悬停/按下）
  - 使用场景：ComboBox、下拉选择器等

### 3. 自旋按钮（上下调节按钮）
- **sys_spin_up.png** (17x48) - 向上按钮
  - 用途：数值调节的向上按钮（`_skin.sys.spin.up`）
  - 特性：3 种状态，垂直排列
  - 使用场景：NumericUpDown、滚动箭头等

- **sys_spin_down.png** (17x48) - 向下按钮
  - 用途：数值调节的向下按钮（`_skin.sys.spin.down`）
  - 特性：3 种状态，垂直排列
  - 使用场景：NumericUpDown、滚动箭头等

- **sys_btn_prev.png** (49x16) - 前一个按钮
  - 用途：向前/向上导航按钮（`_skin.sys.btn.prev`）
  - 特性：3 种状态
  - 使用场景：分页器、列表导航等

- **sys_btn_next.png** (49x17) - 后一个按钮
  - 用途：向后/向下导航按钮（`_skin.sys.btn.next`）
  - 特性：3 种状态
  - 使用场景：分页器、列表导航等

## 三、列表与树形控件相关图片

### 1. 树形控件专用
- **sys_tree_toggle.png** (103x18) - 树节点展开/折叠切换器
  - 用途：树节点的展开/折叠按钮（`_skin.sys.tree.toggle`）
  - 特性：6 种状态（正常/悬停/按下）×（展开/折叠）
  - 使用场景：TreeCtrl 中的节点展开/折叠指示器

- **sys_tree_checkbox.png** (145x16) - 树节点复选框
  - 用途：树节点的复选框（`_skin.sys.tree.checkbox`）
  - 特性：9 种状态 (正常/悬停/按下)*(未选中/选中/半选) 
  - 使用场景：支持复选的树形控件

### 2. 列表头
- **sys_header.png** (200x20) - 列表头
  - 用途：列表控件的列头皮肤（`_skin.sys.header`）
  - 特性：4 种状态 (正常/悬停/按下/禁用)
  - 使用场景：ListCtrl、TableCtrl 的列头

## 四、滚动与进度条相关图片

### 1. 滚动条
- **sys_scrollbar.png** (145x49) - 滚动条
  - 用途：标准滚动条皮肤（`_skin.sys.scrollbar`）
  - 特性：支持 margin 设置, 支持九宫格拉伸, 滚动条图片是一个复合图片,它包含4行9列,每一列包含一种滚动条元素的4种状态: 正常/悬停/按下/禁用, 第一列代表一个滚动条元素,前面8列分别为: 垂直滚动条lineup, linedown, thumbtrack, rail, 水平滚动条lineleft, lineright, thumbtrack, rail. 最后一列只有上面3个是有效状态: 控件有横竖两种滚动条时,显示在右下角的交点的代表拉伸块图片, 垂直滚动条的thumbtrack上的grip块图片, 水平滚动条的thumbtrack上的grip块图片.
  - 使用场景：ScrollView、ListBox 等可滚动控件

### 2. 滑块
- **sys_slider_thumb.png** (80x20) - 滑块拇指
  - 用途：滑动条的拖动块（`_skin.sys.slider.thumb`）
  - 特性：4 种状态
  - 使用场景：Slider 控件的拖动部分

### 3. 分割线
- **sys_split_horz.png** (86x7) - 水平分割线
  - 用途：水平方向的分隔线（`_skin.sys.split.horz`）
  - 特性：支持 margin 拉伸
  - 使用场景：水平布局分隔

- **sys_split_vert.png** (7x86) - 垂直分割线
  - 用途：垂直方向的分隔线（`_skin.sys.split.vert`）
  - 特性：支持 margin 拉伸
  - 使用场景：垂直布局分隔

### 4. 进度条
- **sys_prog_bkgnd.png** (17x17) - 进度条背景（水平）
  - 用途：水平进度条的背景槽（`_skin.sys.prog.bkgnd`）
  - 特性：支持九宫格拉伸
  - 使用场景：ProgressBar 水平模式

- **sys_prog_bar.png** (13x13) - 进度条前景（水平）
  - 用途：水平进度条的填充部分（`_skin.sys.prog.bar`）
  - 特性：支持九宫格拉伸
  - 使用场景：ProgressBar 水平模式的进度指示

- **sys_vert_prog_bkgnd.png** (17x17) - 进度条背景（垂直）
  - 用途：垂直进度条的背景槽（`_skin.sys.vert.prog.bkgnd`）
  - 特性：支持九宫格拉伸
  - 使用场景：ProgressBar 垂直模式

- **sys_vert_prog_bar.png** (13x13) - 进度条前景（垂直）
  - 用途：垂直进度条的填充部分（`_skin.sys.vert.prog.bar`）
  - 特性：支持九宫格拉伸
  - 使用场景：ProgressBar 垂直模式的进度指示

## 五、菜单相关图片

### 1. 菜单项装饰
- **sys_menu_check.png** (49x17) - 菜单勾选标记
  - 用途：菜单项的勾选标记（`_skin.sys.menu.check`）
  - 特性：3 种状态: checked, radio unchecked, radio checked
  - 使用场景：可勾选菜单项的左侧标记

- **sys_menu_arrow.png** (33x17) - 菜单箭头
  - 用途：子菜单指示箭头（`_skin.sys.menu.arrow`）
  - 特性：2 种状态: normal, hover
  - 使用场景：有子菜单的菜单项右侧箭头

- **sys_menu_sep.png** (50x2) - 菜单分隔线
  - 用途：菜单项之间的分隔线（`_skin.sys.menu.sep`）
  - 特性：左侧留白 26 像素用于对齐菜单项
  - 使用场景：菜单分组分隔

### 2. 菜单背景
- **sys_menu_border.png** (27x27) - 菜单边框
  - 用途：菜单弹出框的边框（`_skin.sys.menu.border`）
  - 特性：支持 margin 拉伸
  - 使用场景：ContextMenu、PopupMenu 的边框

- **sys_menu_skin.png** (200x26) - 菜单皮肤
  - 用途：菜单项的背景皮肤（`_skin.sys.menu.skin`）
  - 特性：2 种状态: normal, hover.  其中normal左侧留白 26 像素用于放置勾选标记
  - 使用场景：菜单项的背景（包含勾选区域）

## 六、标签页与选项卡

- **sys_tab_page.png** (200x30) - 标签页背景
  - 用途：TabCtrl 的页面背景（`_skin.sys.tab.page`）
  - 特性：4 种状态 (正常/悬停/按下/禁用), 使用九宫格拉伸,注意表达出选中状态和未选中状态的区别
  - 使用场景：TabControl 的内容区域背景

## 七、开关控件

- **sys_switch.png** (304x19) - 开关前景
  - 用途：开关控件的滑动按钮（`_skin.sys.switch`）
  - 特性：8 种状态: (normal, hover, pressed, disabled)*(ON, OFF)
  - 使用场景：Switch  Toggle 控件的前景按钮

- **sys_switch_bg.png** (60x19) - 开关背景
  - 用途：开关控件的背景轨道（`_skin.sys.switch_bg`）
  - 特性：1 种状态
  - 使用场景：Switch 控件的背景轨道

## 八、图标集合

- **sys_icons.png** (192x16) - 系统图标集合
  - 用途：通用系统图标集（`_skin.sys.icons`）
  - 特性：12 种状态: 文件新建, 文件打开, 文件保存, CUT, COPY, PASTE, PRINT, 大图标模式, 小图标模式, 列表模式,详细模式, 帮助图标
  - 使用场景：编辑菜单、上下文菜单中的功能图标（剪切/复制/粘贴等）

## 九、其他辅助图片

- **sys_tree_lines.png** - 树形控件连线
  - 用途：树形控件的节点连线（`_skin.sys.tree.lines`）
  - 特性：9 种状态, 每种状态显示参考SSkinTreeLines::_DrawByIndex实现.
  - 使用场景：TreeCtrl 的层级关系连线显示

- **sys_settting.png** - 设置按钮
  - 用途：设置按钮（`_skin.sys.setting`）
  - 特性：4种状态: normal, hover, pressed, disabled
  - 使用场景：设置按钮的显示状态

- **sys_mini_close.png** - 工具类窗口的关闭按钮
  - 用途：工具类窗口的关闭按钮（`_skin.sys.mini.close`）
  - 特性：4种状态: normal, hover, pressed, disabled
  - 使用场景：工具类窗口的关闭按钮的显示状态


## 十、使用说明

### 图片命名规范
所有系统图片遵循以下命名规则：
- 前缀：`sys_` 表示系统资源
- 功能描述：如 `btn_` 表示按钮，`menu_` 表示菜单
- 状态描述：如 `close` 关闭，`maximize` 最大化

### Skin 引用方式
在 XML 中通过 `_skin.sys.xxx` 名称引用对应的皮肤定义，例如：
```xml
<imgbtn skin="_skin.sys.btn.close" pos="-45,0" tip="close"/>
```

### 多状态支持
大部分交互性图片支持多种状态（states），常见的状态包括：
- 正常（normal）
- 悬停（hover）
- 按下（pushdown）
- 禁用（disable）
- 选中（checked）
- 组合状态（如 hover&checked）

### 九宫格拉伸
对于需要适应不同尺寸的背景图片（如窗口背景、进度条背景），采用九宫格拉伸技术，通过 margin 属性定义不变形的区域。

### SVG 支持

从 SOUI4 开始，系统移除了独立的 `svg` skin 类型，改为在 `imglist`、`imgframe`、`imgcenter` 等图片类 skin 中直接支持 SVG。

#### 使用方式

**1. 使用位图图片（默认方式）**
```xml
<imglist name="mySkin" src="PNG:MY_IMAGE" states="3"/>
```

**2. 使用 SVG 图片**
```xml
<!-- 方式 1：使用 svg 属性 -->
<imglist name="mySkin" src="FILE:icon.svg" svg="true"/>

<!-- 方式 2：直接使用 svgSrc 属性 -->
<imglist name="mySkin" svgSrc="FILE:icon.svg"/>
```

#### 支持的 Skin 类型
以下 skin 类型都支持 SVG：
- `imglist` - 图片列表皮肤
- `imgframe` - 图片帧皮肤（支持九宫格）
- `imgcenter` - 居中图片皮肤
- `imgbtnex` - 扩展图片按钮皮肤

#### SVG 属性说明
- **svg** - 布尔值，设置为 `true` 或 `1` 时启用 SVG 模式
- **svgSrc** - SVG 文件路径，当设置了此属性时自动启用 SVG 模式
- **src** - 资源路径，配合 `svg` 属性使用

#### 注意事项
1. SVG 文件必须使用 `FILE:` 前缀指定文件路径
2. SVG 不支持多状态（states 属性对 SVG 无效）
3. SVG 会自动缩放到绘制区域大小
4. SVG 不支持平铺（tile 属性对 SVG 无效）
5. 需要确保编译时启用了 `SOUI_ENABLE_SVG` 宏

#### 示例代码

```xml
<skin>
  <!-- 普通位图按钮 -->
  <imglist name="skin.btn.normal" src="PNG:BTN_NORMAL" states="3"/>
  
  <!-- SVG 图标按钮 -->
  <imglist name="skin.btn.svg" src="FILE:icons/save.svg" svg="true"/>
  
  <!-- SVG 窗口背景 -->
  <imgframe name="skin.wnd.bg" src="FILE:background.svg" svg="true" margin="10,10,10,10"/>
  
  <!-- SVG 居中图标 -->
  <imgcenter name="skin.logo" src="FILE:logo.svg" svg="true"/>
</skin>
```

## 十一、典型应用场景

### 1. 主窗口框架
```xml
<SOUI>
  <root skin="_skin.sys.wnd.bkgnd">
    <caption>
      <imgbtn skin="_skin.sys.btn.close"/>
      <imgbtn skin="_skin.sys.btn.minimize"/>
      <imgbtn skin="_skin.sys.btn.maximize"/>
    </caption>
  </root>
</SOUI>
```

### 2. 消息对话框
```xml
<window>
  <img skin="_skin.sys.msgicon"/>
  <text name="msgtext"/>
  <button skin="_skin.sys.btn.normal"/>
</window>
```

### 3. 表单界面
```xml
<check skin="_skin.sys.checkbox"/>
<radio skin="_skin.sys.radio"/>
<combobox dropBtnSkin="_skin.sys.dropbtn"/>
```

### 4. 数据列表
```xml
<listctrl headerSkin="_skin.sys.header"/>
<scrollbar skin="_skin.sys.scrollbar"/>
```

### 5. SVG 图标应用
```xml
<button skin="_skin.sys.btn.normal">
  <img src="FILE:icons/edit.svg" svg="true"/>
</button>
```

---

**文档版本**: 1.1  
**更新日期**: 2026-03-24  
**适用版本**: SOUI4  
**SVG 支持**: 需要启用 SOUI_ENABLE_SVG 宏
