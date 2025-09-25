# SOUI GridCtrl Demo - Enhanced Cell Types

这是一个演示SOUI GridCtrl控件增强单元格类型和类型感知复制粘贴功能的示例程序。

## 新增功能特性

### 增强的单元格类型
- **颜色选择单元格** (第4列): 点击打开颜色选择对话框
- **日期时间选择单元格** (第5列): 点击选择日期和时间
- **复选框单元格** (第6列): 切换复选框状态
- **URL链接单元格** (第7列): 可点击的超链接

### 类型感知的复制粘贴
- 使用 `Ctrl+C` 复制单元格 - 同时保存文本和单元格类型信息
- 使用 `Ctrl+V` 粘贴 - 只有源单元格和目标单元格类型匹配时才执行粘贴
- 通过防止类型不匹配来保护数据完整性
- 保持与外部剪贴板数据的兼容性

## 功能特性

本示例展示了从MFC-GridCtrl移植到SOUI框架的网格控件的增强功能：

### 已实现的功能

1. **基础网格显示**
   - 可配置的行列数量
   - 固定行和固定列支持
   - 网格线绘制（水平、垂直、或两者）
   - 可自定义的网格线颜色和背景色
   - 优化的绘制性能（只绘制可见单元格）

2. **单元格管理**
   - 单元格文本设置和获取
   - 单元格数据存储和状态管理
   - 单元格创建、销毁和属性设置
   - 焦点管理和可见性检查
   - 行列操作（插入、删除、自动调整大小）

3. **鼠标交互**
   - 鼠标点击选择和焦点设置
   - 双击编辑支持
   - 右键菜单支持
   - 鼠标悬停效果
   - 固定单元格特殊处理

4. **键盘导航**
   - 方向键导航（上下左右）
   - Home/End键快速定位
   - Page Up/Page Down翻页
   - Tab键单元格切换
   - Ctrl组合键快速移动

5. **单元格编辑**
   - 就地编辑功能
   - 编辑状态管理
   - 输入验证支持
   - 编辑控件自动创建
   - 编辑完成事件处理

6. **选择和高亮**
   - 单选、多选、区域选择
   - 选择状态视觉反馈
   - 行选择和列选择模式
   - 选择区域高亮显示
   - 焦点单元格特殊标识

7. **滚动和视图管理**
   - 智能滚动条管理
   - 固定区域和滚动区域分离
   - 确保单元格可见功能
   - 可见区域计算优化
   - 精确的坐标转换

8. **排序功能**
   - 列头点击排序
   - 文本和数值排序支持
   - 升序、降序、无序切换
   - 排序状态视觉指示
   - 自定义排序函数支持

9. **剪贴板操作**
   - 复制、剪切、粘贴功能
   - 键盘快捷键支持（Ctrl+C/X/V）
   - 多单元格批量操作
   - 文本格式自动解析
   - 跨应用程序数据交换

### 可扩展的功能

以下功能可以根据需要进一步扩展：

1. **高级编辑**
   - 下拉列表编辑器
   - 日期时间选择器
   - 数值范围验证
   - 自定义编辑控件

2. **数据绑定**
   - 数据源绑定
   - 虚拟模式支持
   - 大数据量优化
   - 数据变更通知

3. **外观增强**
   - 主题支持
   - 自定义绘制
   - 图标显示
   - 条件格式化

4. **交互增强**
   - 拖拽重排
   - 列宽调整
   - 右键菜单
   - 工具提示

5. **导入导出**
   - Excel文件支持
   - CSV格式支持
   - 打印功能
   - 数据过滤

## 使用方法

### XML配置

在SOUI的XML布局文件中使用GridCtrl：

```xml
<gridctrl name="grid_test" pos="10,40,-10,-40" 
          rowCount="10" 
          colCount="5" 
          fixedRows="1" 
          fixedCols="1" 
          defRowHeight="25" 
          defColWidth="100" 
          gridLines="3" 
          gridLineColor="#C8C8C8" 
          gridBkColor="#FFFFFF" 
          editable="1"/>
```

### 代码操作

```cpp
// 获取网格控件
SGridCtrl* pGrid = FindChildByName2<SGridCtrl>(L"grid_test");

// 设置网格大小
pGrid->SetRowCount(10);
pGrid->SetColumnCount(5);

// 设置单元格文本
pGrid->SetItemText(0, 1, _T("Column 1"));
pGrid->SetItemText(1, 0, _T("Row 1"));
pGrid->SetItemText(1, 1, _T("Cell Data"));

// 设置外观
pGrid->SetGridLines(SGVL_BOTH);
pGrid->SetGridLineColor(RGB(200, 200, 200));

// 启用功能
pGrid->SetEditable(TRUE);
pGrid->SetListMode(TRUE);
pGrid->SetSortOnClick(TRUE);

// 设置焦点和选择
pGrid->SetFocusCell(1, 1);
pGrid->SetSelectedRange(1, 1, 1, 7, TRUE);
```

### 键盘快捷键

| 快捷键 | 功能 |
|--------|------|
| 方向键 | 移动焦点单元格 |
| Ctrl + 方向键 | 快速移动到边界 |
| Home/End | 移动到行首/行尾 |
| Ctrl + Home/End | 移动到表格首/尾 |
| Page Up/Down | 翻页 |
| Tab/Shift+Tab | 切换到下一个/上一个单元格 |
| Enter/F2 | 开始编辑 |
| Escape | 取消编辑或清除选择 |
| Delete | 删除选中单元格内容 |
| Ctrl + C | 复制 |
| Ctrl + X | 剪切 |
| Ctrl + V | 粘贴 |
| Ctrl + A | 全选 |

### 鼠标操作

| 操作 | 功能 |
|------|------|
| 单击 | 选择单元格 |
| 双击 | 开始编辑 |
| 右键 | 显示上下文菜单（可扩展） |
| 点击列头 | 排序（如果启用） |
| 拖拽 | 选择区域（可扩展） |

## 编译和运行

1. 确保已正确配置SOUI开发环境
2. 使用CMake生成项目文件
3. 编译并运行

## 技术说明

### 架构设计

- **SGridCtrl**: 主控件类，继承自SWindow
- **IGridCellBase**: 单元格基类，定义单元格接口
- **SGridCell**: 默认单元格实现
- **SCellID**: 单元格标识结构
- **SCellRange**: 单元格范围结构

### 与MFC-GridCtrl的对应关系

| MFC-GridCtrl | SOUI GridCtrl | 说明 |
|--------------|---------------|------|
| CGridCtrl | SGridCtrl | 主控件类 |
| CGridCellBase | IGridCellBase | 单元格基类 |
| CGridCell | SGridCell | 默认单元格实现 |
| CCellID | SCellID | 单元格标识 |
| CCellRange | SCellRange | 单元格范围 |

### 绘制机制

使用SOUI的IRenderTarget接口替代MFC的CDC，实现跨平台的绘制功能。

## 许可证

本项目遵循SOUI项目的许可证。
