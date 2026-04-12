# SVG 图标转换工具 v2（使用 svg2png.exe）

## 功能说明

`svg_to_icon_v2.py` 是一个专门使用 `svg2png.exe` 工具从 SVG 矢量图生成 Windows (.ico) 和 macOS (.icns) 格式图标的 Python 脚本。

## 依赖库

```bash
pip install Pillow
```

**无需安装其他软件！** `svg2png.exe` 已包含在 tools 目录中。

## 使用方法

### 1. 单个文件转换

```bash
# 基本用法（输出到同目录，PNG 尺寸 1024px）
python svg_to_icon_v2.py icon.svg

# 指定输出文件名（不含扩展名）
python svg_to_icon_v2.py icon.svg -o myapp_icon

# 指定 PNG 渲染尺寸
python svg_to_icon_v2.py icon.svg --size 512
```

**输出：**
- `icon.ico` - Windows 图标（包含 16x16, 32x32, 48x48, 64x64, 128x128, 256x256）
- `icon.icns` - macOS 图标集（包含 16x16 到 1024x1024 及 @2x 变体）

### 2. 批量转换

```bash
# 处理目录下所有 SVG 文件
python svg_to_icon_v2.py --batch ./icons/

# 批量转换并指定输出目录
python svg_to_icon_v2.py --batch ./src_icons/ -o ./dist_icons/

# 批量转换并自定义渲染尺寸
python svg_to_icon_v2.py --batch ./icons/ --size 2048
```

### 3. 帮助信息

```bash
python svg_to_icon_v2.py --help
```

## 技术实现

### 工作流程（优化版）

**关键改进：直接从 SVG 渲染每个目标尺寸，而非缩放单个 PNG**

1. **ICO 生成**: 
   - 从 SVG 直接渲染 6 个尺寸：16x16, 32x32, 48x48, 64x64, 128x128, 256x256
   - 使用 `svg2png.exe` 为每个尺寸单独渲染
   - 组合所有尺寸的 PNG 为单个 ICO 文件

2. **ICNS 生成**: 
   - 从 SVG 直接渲染 10 个尺寸：16, 32, 64, 128, 256, 512 + @2x 变体
   - 使用 `svg2png.exe` 为每个尺寸单独渲染
   - 使用 `iconutil` 打包为 ICNS

3. **优势**:
   - ✅ **矢量图优势**：SVG 在任意尺寸下重新渲染，保持清晰锐利
   - ✅ **避免缩放失真**：不使用 PNG 缩放算法，消除模糊和锯齿
   - ✅ **最佳质量**：小尺寸图标（如 16x16）细节更清晰
   - ✅ **专业效果**：与专业图标编辑软件（如 IconJar）相同的处理方式

### svg2png.exe 调用示例

``bash
# 为不同尺寸分别调用
svg2png.exe -i icon.svg -s 16 -o icon.16.png
svg2png.exe -i icon.svg -s 32 -o icon.32.png
svg2png.exe -i icon.svg -s 256 -o icon.256.png
```

**参数说明：**
- `-i`: 输入 SVG 文件路径
- `-s`: PNG 输出尺寸（宽和高相同）
- `-o`: 输出 PNG 文件路径


### 图标尺寸规范

**Windows ICO:**
- 16x16 (任务栏小图标)
- 32x32 (桌面图标)
- 48x48 (文件资源管理器)
- 64x64 (大视图)
- 128x128 (超大视图)
- 256x256 (Vista+ 高清图标)

**macOS ICNS:**
- 标准尺寸: 16, 32, 64, 128, 256, 512, 1024
- Retina (@2x): 32, 64, 128, 256, 512


## 注意事项

### SVG 文件要求

1. **viewBox 属性**: 确保 SVG 包含正确的 viewBox，例如：
   ```xml
   <svg viewBox="0 0 1024 1024" xmlns="http://www.w3.org/2000/svg">
   ```

2. **透明背景**: 建议在透明背景上设计，以便在不同背景下显示良好

3. **简洁图形**: 避免过于复杂的渐变和滤镜，以确保在各尺寸下清晰可辨

### 平台限制

- **ICO 生成**: 所有平台均可生成
- **ICNS 生成**: 
  - ✅ macOS: 完整支持
  - ⚠️ Windows/Linux: 仅生成提示，需转移到 Mac 完成转换


**版本**: 2.0  
**创建日期**: 2026-04-11  
**作者**: SOUI Team  
**依赖工具**: svg2png.exe (已内置)
