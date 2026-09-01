# svgcvt

将 SVG 渲染为位图，并导出 **PNG / ICO / ICNS** 三种格式的命令行工具。


## 功能

- 用 SOUI 自身的渲染管线把 SVG 光栅化为位图，再封装成目标格式。
- **PNG**：单尺寸位图，可指定宽度，高度按原始宽高比自动计算。
- **ICO**：Windows 图标，一次性内嵌 7 个标准尺寸的 PNG。
- **ICNS**：macOS 图标，一次性内嵌 6 个标准尺寸的 PNG。
- ICO / ICNS 内部**直接内嵌 PNG**（不使用 BMP 路径），完整保留 alpha 透明通道，无需额外依赖。

## 构建

本工具随 SOUI4 主工程一起构建，受两个开关控制（**默认均为 ON**）：

| 选项 | 位置 | 默认值 | 说明 |
| --- | --- | --- | --- |
| `SOUI_BUILD_TOOLS` | 根 `CMakeLists.txt:92` | `ON` | 是否构建 tools 目录 |
| `SOUI_ENABLE_SVG` | 根 `CMakeLists.txt:100` | `ON` | 是否构建本工具 |

Windows 下：

```bat
mkdir build && cd build
cmake .. -A x64
cmake --build . --config Release
```

产物输出目录由 `__cmake/global.cmake:16` 统一指定：

```
${CMAKE_BINARY_DIR}/bin/svgcvt.exe
```

依赖：链接 `soui4`；运行时使用 `Render_Gdi` 渲染工厂 + `ImgDecoder_Stb` 图像解码器（在 `main.cpp` 中固定配置），PNG 编码使用 `stb_image_write`。

## 用法

```
svgcvt -i <input.svg> [-o <output>] [-s <size>] [-f png|ico|icns]
```

### 参数

| 参数 | 必填 | 说明 |
| --- | --- | --- |
| `-i` | 是 | 输入 SVG 路径。必须以 `.svg` 结尾（大小写不敏感），否则直接打印用法并返回 `-1`。 |
| `-o` | 否 | 输出文件路径。**扩展名不参与格式判断**，详见下方说明。 |
| `-s` | 否 | 目标尺寸，仅对 `png` 生效（指定的宽度，高度按比例计算）。用于 `ico`/`icns` 时会被忽略并打印提示。 |
| `-f` | 否 | 输出格式：`png`（默认）/ `ico` / `icns`。大小写不敏感。 |

> **重要**：输出格式由 **`-f`** 决定，**不是**由 `-o` 的扩展名决定。
> `-o` 仅作为输出文件路径使用，即使写成 `.png` 而 `-f ico`，产出的仍是 ICO 文件。

### 省略 `-o` 时的默认输出名

| 格式 | 默认输出名 | 示例（`logo.svg`，`-s 64`） |
| --- | --- | --- |
| png | `<输入名>_<宽度>.png` | `logo_64.png` |
| ico / icns | `<输入名>.<格式>` | `logo.ico` |

png 且未指定 `-s` 时，宽度取 SVG 自身的原始宽度。

## 输出格式细节

### PNG

- 宽度 = `-s` 指定值（缺省为 SVG 原始宽度），高度 = 宽度 × (SVG 原始高 / 原始宽)，**保持宽高比**。
- 像素在内存中解预乘（BGRA 预乘 → RGBA 直显）后编码，透明通道正确。

### ICO

内嵌标准尺寸集（共 7 个）：`16, 24, 32, 48, 64, 128, 256`

结构：小端 `ICONDIR`(6B) + N×`ICONDIRENTRY`(16B) + 各尺寸 PNG 数据。
`ICONDIRENTRY` 中 `bWidth`/`bHeight` 按正方形写入，尺寸 256 按 ICO 规范记为 `0`。

### ICNS

内嵌标准尺寸集（共 6 个）：`32, 64, 128, 256, 512, 1024`

OSType 映射：`ic11`=32、`ic12`=64、`ic07`=128、`ic13`=256、`ic09`=512、`ic10`=1024

结构：`'icns'` 魔数 + 大端总长度 + 若干（OSType + 大端长度 + PNG 数据）。

## 示例

```bat
:: 导出单张 PNG，宽 256（高按比例）
svgcvt -i logo.svg -o logo.png -s 256

:: 不指定 -o，输出 logo_256.png
svgcvt -i logo.svg -s 256

:: 导出 Windows 图标（内嵌 7 个尺寸）
svgcvt -i logo.svg -o app.ico -f ico

:: 导出 macOS 图标（内嵌 6 个尺寸）
svgcvt -i logo.svg -o app.icns -f icns
```

## 实现要点

1. `SAppCfg` 配置 `Render_Gdi` + `ImgDecoder_Stb`，初始化 `SApplication`。
2. `CreateSvgFromFile` 加载 SVG（`SOUI/include/core/Svg.h`），读取原始宽高。
3. 创建离屏 `IRenderTarget`，`DrawSVG` 绘制到目标尺寸。
4. `LockPixelBits` 取像素，**解预乘**（BGRA → RGBA 直显），保证半透明区域颜色正确。
5. `stbi_write_png_to_mem` 在内存中编码 PNG（**无中间临时文件**）。
6. 按目标格式封装：PNG 直接落盘；ICO / ICNS 逐个尺寸渲染后写入容器。

## 退出码

| 返回值 | 含义 |
| --- | --- |
| `0` | 成功 |
| `-1` | 用法错误（未给 `-i`，或输入不是 `.svg`） |
| `-2` | SVG 加载失败 |
| `-3` | SVG 尺寸非法（宽或高 ≤ 0） |
| `-4` | 不支持的输出格式（仅支持 png/ico/icns） |
| `-5` | 未生成任何图标图像（ico/icns 分支） |
| `-6` | 写出文件失败 |

## 注意

- **非正方形 SVG 导出 ICO 时**：`ICONDIRENTRY` 的宽高字段按正方形写入（等于该档尺寸），而内嵌 PNG 仍保持原始宽高比。如需严格的方形图标，建议先准备正方形 SVG。
- ICO / ICNS 的多尺寸是**固定档位**，`-s` 无法自定义；需要单档自定义尺寸请用 `png` 格式。
- 本工具的渲染依赖 SOUI 的 GDI 渲染工厂，实际使用场景以 Windows 为主。
