# SOUI 迁移 HarmonyOS/OpenHarmony 可行性评估与开发方案

生成日期：2026-06-17
评估对象：当前仓库 `SOUI4`，重点为 `swinx` 兼容层。

## 1. 结论

SOUI 迁移到 HarmonyOS/OpenHarmony 具有可行性，但不是简单重新编译。合理路线是保留 SOUI 控件、布局、资源、动画、组件机制，新增一个 `swinx` 的 HarmonyOS 后端，把现有 Win32 消息/窗口/GDI 兼容层映射到 HarmonyOS NDK 的 XComponent、NativeWindow、ArkUI/Node-API 和基础 POSIX 能力上。

建议采用“一个 ArkTS 应用壳 + 一个 Native C++ 动态库 + 一个 XComponent 渲染承载面”的集成方式。MVP 阶段把 SOUI 作为单个自绘视图运行在 XComponent 内，先支持主窗口、绘制、鼠标/触摸、键盘、定时器、资源和基础控件；多顶层窗口、拖放、系统托盘、复杂 IME、辅助功能和全量富文本放到后续阶段。

总体可行性判断：

| 项目 | 判断 |
| --- | --- |
| SOUI 控件/布局/动画层复用 | 高，可基本复用 |
| `swinx` 基础 Win32 API 复用 | 中高，公共层可复用，平台层需新增 |
| HarmonyOS 原生窗口承载 | 可行，依赖 XComponent/NativeWindow |
| GDI/Cairo 自绘路径 | 可行，建议 MVP 使用 cairo image surface + NativeWindow buffer 提交 |
| Skia 路径 | 可行但后置，先不要作为 MVP 阻塞项 |
| 多窗口/弹窗/菜单 | 中，MVP 内部模拟，后续再映射 ArkUI/窗口能力 |
| IME/剪贴板/拖放 | 中到高风险，需按 API level 分层实现 |
| 上架级稳定性 | 需要 3-4 个阶段迭代后评估 |

## 2. 当前代码结构与迁移边界

### 2.1 顶层构建逻辑

当前顶层 `CMakeLists.txt` 已经把平台分为两条路径：

- Windows：使用系统 Win32 API，`WCHAR_SIZE=2`。
- 非 Windows：引入 `swinx`，`WCHAR_SIZE=4`，并把 `swinx/include` 加入头文件路径。

关键文件：

- `CMakeLists.txt`：第 123-179 行附近决定 Windows/非 Windows 路径、引入 `swinx`、设置 `WCHAR_SIZE`。
- `SOUI/CMakeLists.txt`：非 Windows 时 `soui4` 链接 `swinx`。
- `components/CMakeLists.txt`：D2D/WIC/GDI+ 仅 Windows，`render-gdi` 和 `render-skia` 跨平台构建。
- `components/render-gdi/CMakeLists.txt`：非 Windows 版本链接 `swinx`。
- `components/render-skia/CMakeLists.txt`：非 Windows 间接依赖 `swinx` 提供 fontconfig/freetype 支持。

这说明 HarmonyOS 不应该走 Windows 分支，而应新增非 Windows 的第三类后端：`OHOS`。

### 2.2 SwinX 的职责

`swinx` 是 SOUI 跨平台的关键。它提供一组 Windows 头文件和 Win32 API 兼容实现，让 SOUI 继续使用熟悉的 `HWND/HDC/MSG/WNDPROC/GDI/IMM/OLE` 等接口。

主要结构：

- `swinx/include/*.h`：Win32/COM/GDI/用户态 API 兼容头文件。
- `swinx/src/*.cpp`：跨平台公共实现，例如窗口对象、消息分发、GDI 对象、资源、同步对象、COM 基础对象。
- `swinx/src/platform/linux/*`：Linux/XCB 后端。
- `swinx/src/platform/cocoa/*`：macOS/Cocoa 后端。
- `swinx/linux.cmake`：绑定 XCB、xkbcommon、dbus、ALSA、cairo-xcb。
- `swinx/macos.cmake`：绑定 Cocoa、QuartzCore、IOKit、Carbon、AudioToolbox。

HarmonyOS 必须新增：

- `swinx/ohos.cmake`
- `swinx/src/platform/ohos/SConnection.{h,cpp}`
- `swinx/src/platform/ohos/SOhosWindow.{h,cpp}`
- `swinx/src/platform/ohos/SClipboard.{h,cpp}` 或 ArkTS bridge
- `swinx/src/platform/ohos/keyboard.{h,cpp}`
- `swinx/src/platform/ohos/imm.cpp`
- 必要时增加 `swinx/src/platform/ohos/napi_bridge.{h,cpp}`

### 2.3 关键抽象点：SConnection

`swinx/src/wnd.cpp` 是公共 Win32 窗口语义实现，它大量调用 `WndObj::mConnection`。平台差异被集中在 `SConnection`。

需要优先实现的 `SConnection` 能力：

| 能力 | Linux 实现参考 | macOS 实现参考 | HarmonyOS 实现目标 |
| --- | --- | --- | --- |
| 连接/全局状态 | `SConnection::SConnection` | `SConnection::SConnection` | 初始化 NAPI/XComponent 全局上下文 |
| 消息循环 | `peekMsg/getMsg/TranslateMessage` | 同名函数 | 从 ArkUI 回调、定时器、内部队列生成 `MSG` |
| 创建窗口 | `OnWindowCreate` | `createNsWindow` 间接实现 | 绑定或虚拟化 XComponent surface |
| 显示/隐藏 | `SetWindowVisible` | `showNsWindow` | MVP 内部状态，根窗口始终绑定 XComponent |
| 重绘请求 | `SendExposeEvent/updateWindow/commitCanvas` | 同名函数 | 触发 XComponent 重绘，提交 NativeWindow buffer |
| 绘制 surface | `CreateWindowSurface/ResizeSurface` | 同名函数 | cairo image surface 或 NativeWindow-backed surface |
| 鼠标/触摸 | XCB event | NSView event | XComponent touch/mouse callback 转 Win32 消息 |
| 键盘/文本 | xkb/XIM | Cocoa key/input | ArkUI/NativeXComponent key + IME bridge |
| 光标 | X cursor | NSCursor | PC 形态支持，移动端降级 |
| 剪贴板 | X selection | NSPasteboard | Pasteboard NDK 或 ArkTS bridge |
| 拖放 | XDND | NSPasteboard dragging | 后置，UDMF/ArkUI drag |
| DPI/屏幕 | X screen | NSScreen | ArkUI display/window density |

### 2.4 SOUI 层修改量

SOUI 本体不需要大规模重写。主要原因：

- 控件绘制、布局、事件分发集中在 `SOUI/src/core` 和 `SOUI/include/core`，外部系统窗口只通过 `SNativeWnd` 和 Win32 消息进入。
- `SHostWnd` 使用 `SNativeWnd::CreateNative` 创建宿主窗口，后续绘制通过 `WM_PAINT`、`GetDC`、`UpdateWindow` 等路径完成。
- 资源加载已支持文件资源、ZIP/7z provider 和跨平台 `.rc` 资源兼容。
- 非 Windows 已经关闭或替代部分 Windows-only 功能，例如 D2D/WIC/GDI+。

需要谨慎处理的 SOUI 直接依赖：

- `SOUI/src/core/SNativeWnd.cpp`：thunk 机制支持 x86/x64/ARM/ARM64。HarmonyOS ARM64 理论可复用，但建议 MVP 关闭 `ENABLE_THUNK`，减少可执行内存和 W^X 策略风险。
- `SOUI/src/core/shostwnd.cpp`：IME、透明窗口、dummy window、弹窗位置需要验证。
- `SOUI/src/control/SRichEdit.cpp`：非 Windows 依赖 `libmsftedit`。HarmonyOS 早期可先禁用富文本或只覆盖基础输入框。
- `SOUI/src/res.mgr/SResProvider.cpp`：文件资源路径、`FindFirstFile` 兼容由 `swinx` 提供，OHOS 沙箱路径需要适配。
- `SOUI/include/helper/souidlgs.h`：传统 Windows common dialog 语义不适合 MVP，建议用 ArkTS picker bridge 替换。

## 3. HarmonyOS/OpenHarmony 平台依据

外部资料显示，HarmonyOS/OpenHarmony 支持 C/C++ native 动态库、CMake 工具链、XComponent surface、NativeWindow buffer 和 NAPI/ArkTS 桥接，足以承载 SOUI 的自绘 UI。

参考资料：

- OpenHarmony NDK 概述说明 NDK 提供 C/C++ 工具链、CMake、libc、图形、窗口、多媒体、压缩库、Node-API、XComponent、Drawing、OpenGL 等能力：<https://raw.githubusercontent.com/openharmony/docs/master/en/application-dev/napi/ndk-development-overview.md>
- NativeWindow 文档说明可通过 NativeWindow 请求图形 buffer、写入内容并 flush 到图形队列，XComponent 回调可取得 `OHNativeWindow`：<https://raw.githubusercontent.com/openharmony/docs/master/en/application-dev/graphics/native-window-guidelines.md>
- XComponent 文档说明 `XComponent` 有独立 NativeWindow，可用于 native C/C++ 创建 EGL/OpenGL ES 环境和自定义渲染：<https://gitee.com/openharmony/docs/blob/73f242a3d024f6ee18c48b22242c2aa458ae3839/en/application-dev/ui/arkts-common-components-xcomponent.md>
- OpenHarmony 5.0.0 release note 提到 API level 12，NDK C API 在 ArkUI、图形渲染、输入法、pasteboard 等方面增强，适合第三方框架适配：<https://raw.githubusercontent.com/openharmony/docs/master/en/release-notes/OpenHarmony-v5.0.0-release.md>
- Qt 官方 2026-04-10 文章说明 Qt 正在移植 HarmonyOS，并通过 vcpkg 支持 `arm64-ohos/arm-ohos/x64-ohos` triplet、HarmonyOS SDK native toolchain、API 12+、CMake/Ninja：<https://www.qt.io/blog/building-libraries-for-harmonyos-with-vcpkg>
- Qt Wiki 的 OpenHarmony 构建文档显示 Qt 5.12/5.15 的 OHOS 构建也采用 DevEco Studio + OpenHarmony SDK native + C/C++ 交叉编译 + 应用壳集成：<https://wiki.qt.io/Building_Qt_for_OpenHarmony>
- Qt for HarmonyOS 页面把 HarmonyOS NEXT、PC 形态和 Qt for HarmonyOS 文档入口集中列出，可作为平台演进参考：<https://wiki.qt.io/Qt_for_HarmonyOS>

对 SOUI 的启发：

- 不要把 SOUI 直接改成 ArkUI 控件库；应像 Qt 一样保持 C++ 框架主体，由平台插件/后端适配窗口、输入和渲染。
- 第三方库交叉编译是主要工程成本之一。Qt 已把 libpng、fontconfig、ICU 等作为 OHOS port 的重点，这与 SOUI/swinx 的 cairo/fontconfig/freetype/libpng 依赖高度相似。
- 应用交付形态应遵循 HarmonyOS：DevEco/HAP 工程负责 Ability、页面、权限和生命周期；SOUI 作为 native library 被 ArkTS/XComponent 加载。

## 4. 目标架构

### 4.1 运行时结构

```text
HarmonyOS App / HAP
  src/main/ets
    EntryAbility.ets
    pages/SouiPage.ets
      XComponent(id: "soui_root", type: "surface" 或 "texture", libraryname: "soui_ohos")
  src/main/cpp
    libsoui_ohos.so
      NAPI module init
      SOUI app bootstrap
      swinx platform/ohos
      soui4
      components
      cairo/freetype/fontconfig/libpng/zlib
```

ArkTS 侧职责：

- 创建 `XComponent`。
- 管理 Ability/page 生命周期。
- 把 `OH_NativeXComponent` 传给 native。
- 提供少量平台能力桥接：文件选择、权限申请、剪贴板授权、软键盘、系统主题/字体变更等。

Native 侧职责：

- 初始化 `SApplication`、资源 provider、渲染组件。
- 在 `swinx` 中把 XComponent/NativeWindow 封装成根 `HWND`。
- 将 XComponent 输入事件转成 `WM_*` 消息。
- 通过 cairo/GDI 兼容层完成 SOUI 绘制。
- 把脏区提交到 NativeWindow buffer。

### 4.2 窗口模型

MVP 不做真正的多顶层窗口。建议窗口模型如下：

- 根 `SHostWnd` 绑定唯一 XComponent。
- `WS_CHILD`、控件、菜单、下拉框继续在 SOUI 内部管理。
- `WS_POPUP` 弹窗先作为 SOUI 内部 overlay 或虚拟窗口处理，限制不越出 XComponent。
- `MessageBox`、文件选择器等系统交互后续通过 ArkTS bridge。
- PC 形态下如需多顶层窗口，再研究 HarmonyOS WindowStage/subWindow 或多 XComponent 方案。

这样可以避免 HarmonyOS 应用模型与 Win32 任意 HWND 模型之间的一次性全量对齐。

### 4.3 渲染模型

推荐两步走。

第一步，CPU buffer MVP：

```text
SOUI 控件绘制
  -> render-gdi
  -> swinx GDI/HDC
  -> cairo image surface
  -> commitCanvas
  -> OH_NativeWindow_NativeWindowRequestBuffer
  -> memcpy/convert dirty rect
  -> OH_NativeWindow_NativeWindowFlushBuffer
```

优点：

- 复用现有 `render-gdi/linux` 和 `swinx` GDI 兼容层。
- 不要求立即打通 OpenGL/EGL/Skia。
- 方便做首屏、事件和控件正确性验证。

缺点：

- 大窗口高频动画下 CPU 拷贝成本高。
- 像素格式、stride、premultiplied alpha 需要认真处理。

第二步，GPU/Skia 优化：

- 研究 `render-skia` 在 OHOS 上链接 OpenGL ES/Vulkan/Skia 的路径。
- 将 XComponent NativeWindow 用作 EGL surface 或 Vulkan surface。
- 保留 CPU path 作为兼容后备。

## 5. 构建系统改造

### 5.1 顶层 CMake

新增 OHOS 识别：

```cmake
if(CMAKE_SYSTEM_NAME MATCHES "OHOS|OpenHarmony|HarmonyOS")
    set(IS_OHOS ON)
endif()
```

调整逻辑：

- Windows 分支保持不变。
- 非 Windows 分支中，OHOS 判断不应进入 Linux/XCB 细节；C++ 代码统一使用 `__OHOS__`。
- OHOS 仍设置 `WCHAR_SIZE=4`，除非后续明确需要 Windows 兼容 UTF-16 ABI。
- 默认关闭高风险模块：
  - `SOUI_BUILD_DEMOS=OFF`
  - `SOUI_BUILD_GAMES=OFF`
  - `SOUI_BUILD_TOOLS=OFF`
  - `SOUI_BUILD_WS=OFF`，网络后续单独验证
  - `SOUI_BUILD_RICHEDIT=OFF` 或只在 `msftedit` OHOS 构建通过后打开
  - `SOUI_ENABLE_ACC=OFF`
  - `SOUI_ENABLE_SPY=OFF`
  - `ENABLE_THUNK=OFF`

### 5.2 swinx/CMakeLists.txt

当前逻辑只有 Darwin 与 Linux：

```cmake
if(CMAKE_SYSTEM_NAME MATCHES Darwin)
include(macos.cmake)
else()
include(linux.cmake)
endif()
```

应改为：

```cmake
if(CMAKE_SYSTEM_NAME MATCHES "OHOS|OpenHarmony|HarmonyOS")
    include(ohos.cmake)
elseif(CMAKE_SYSTEM_NAME MATCHES Darwin)
    include(macos.cmake)
else()
    include(linux.cmake)
endif()
```

### 5.3 swinx/ohos.cmake

初版目标：

- 不引入 XCB、dbus、ALSA、xkbcommon、xcb-imdkit。
- 构建 `src/*.cpp`、`src/cmnctl32/*.cpp`、`src/platform/ohos/*.cpp`。
- 链接 NDK 库：
  - `libace_napi.z.so`
  - `libace_ndk.z.so`
  - `libnative_window.so`
  - `libhilog_ndk.z.so`
  - 视具体使用加入 `libEGL.so`、`libGLESv3.so`、`libnative_drawing.so`
- cairo/freetype/fontconfig/libpng/zlib 先通过内置 thirdparty 或 vcpkg OHOS triplet 解决。

建议优先评估 vcpkg OHOS triplet，因为 Qt 迁移中已经暴露并解决了一部分 fontconfig/libpng/ICU 的 OHOS 交叉编译问题。

### 5.4 资源编译

当前 `__cmake/windres.cmake` 已支持非 Windows 通过 MinGW `windres` 将 `.rc` 编译成 COFF，再转 ELF/Mach-O。OHOS 产物本质是 ELF 动态库，理论上可复用 Linux 的 COFF->ELF 路线。

建议：

- 第一阶段不要依赖 `.rc` 内嵌资源，先使用文件资源 provider 或 ZIP provider。
- 第二阶段验证 `target_compile_resources` 在 OHOS toolchain 下能否链接 ELF object。
- 如果 OHOS linker 对 host 生成 ELF object 有格式/架构限制，改用 `res2cdata` 生成 C 数组资源，避免 COFF/ELF 转换问题。

## 6. SwinX OHOS 后端开发任务

### 6.1 NAPI/XComponent bootstrap

新增 native 模块入口：

- 在 `src/main/cpp` 或 `swinx/src/platform/ohos` 中实现 NAPI module。
- 从 exports 中取得 `OH_NATIVE_XCOMPONENT_OBJ`。
- 调用 `OH_NativeXComponent_GetXComponentId` 识别 XComponent。
- 注册 `OH_NativeXComponent_Callback`：
  - `OnSurfaceCreated`
  - `OnSurfaceChanged`
  - `OnSurfaceDestroyed`
  - `DispatchTouchEvent`
- 按平台 API 能力注册 mouse/key/focus 回调。

内部保存：

```cpp
struct SOhosXComponentContext {
    OH_NativeXComponent *component;
    OHNativeWindow *nativeWindow;
    int width;
    int height;
    float density;
    HWND rootHwnd;
};
```

### 6.2 `SOhosWindow`

参考 macOS 的 `SNsWindow`，不要把 OHOS 细节散落到 `SConnection`。

建议接口：

```cpp
HWND createOhosWindow(HWND parent, DWORD style, DWORD exStyle,
                      LPCSTR title, int x, int y, int cx, int cy,
                      SConnBase *listener);
BOOL showOhosWindow(HWND hwnd, int nCmdShow);
BOOL setOhosWindowPos(HWND hwnd, int x, int y);
BOOL setOhosWindowSize(HWND hwnd, int cx, int cy);
void invalidateOhosWindow(HWND hwnd, LPCRECT rc);
BOOL getOhosWindowRect(HWND hwnd, RECT *rc);
BOOL getOhosCursorPos(LPPOINT pt);
int getOhosDpi(BOOL bx);
void commitOhosWindow(HWND hwnd, cairo_surface_t *surface, const RECT &dirty);
```

MVP 中这些接口可以只管理一个根窗口和若干虚拟子窗口，不必创建真实系统窗口。

### 6.3 消息队列

`SConnection` 需要维护内部消息队列，与 Linux/macOS 的 `Msg` 结构保持一致。

输入来源：

- XComponent touch/mouse callback。
- ArkUI key callback 或 NAPI bridge。
- NativeWindow surface created/changed/destroyed。
- 定时器线程或 FFRT task。
- `PostMessage/SendMessage` 内部调用。
- 重绘请求。

必须优先覆盖的消息：

- `WM_CREATE`
- `WM_DESTROY`
- `WM_SIZE`
- `WM_MOVE`
- `WM_SHOWWINDOW`
- `WM_SETFOCUS`
- `WM_KILLFOCUS`
- `WM_PAINT`
- `WM_ERASEBKGND`
- `WM_TIMER`
- `WM_MOUSEMOVE`
- `WM_LBUTTONDOWN/UP/DBLCLK`
- `WM_RBUTTONDOWN/UP`
- `WM_MOUSEWHEEL`
- `WM_KEYDOWN/UP`
- `WM_CHAR`
- `WM_IME_CHAR` 或等价文本输入消息

### 6.4 绘制和 buffer 提交

`CreateWindowSurface` 初版返回 `cairo_image_surface_create(CAIRO_FORMAT_ARGB32, cx, cy)`。

`ResizeSurface`：

- 尺寸变化时销毁旧 image surface。
- 新建 surface。
- 投递 `WM_SIZE`。
- 全量 invalidate。

`commitCanvas`：

- 从 `WndObj` 取当前 cairo surface。
- 调用 `OH_NativeWindow_NativeWindowHandleOpt` 设置 buffer geometry。
- `OH_NativeWindow_NativeWindowRequestBuffer` 获取 buffer。
- 等待 fence。
- 获取 buffer handle、stride、format。
- 将 cairo surface 的 dirty rect 转换/拷贝到 buffer。
- `OH_NativeWindow_NativeWindowFlushBuffer` 提交。
- 失败时 abort buffer。

需要确认的像素细节：

- cairo image surface 默认 ARGB32 是本机端序的 premultiplied ARGB。
- OHOS NativeWindow buffer 常见格式可能为 RGBA_8888/BGRA_8888/RGBA_1010102，必须显式设置或做转换。
- dirty rect 提交前要考虑 stride、裁剪和 buffer 尺寸变化。

### 6.5 输入映射

触摸到鼠标：

- 单指 down/move/up 映射到左键消息。
- 多指先忽略或转换为滚动/手势，后续扩展。
- 坐标从 XComponent 局部坐标转 client 坐标。

鼠标：

- PC/2-in-1 设备需要原生 mouse event。
- 映射 hover、move、button、wheel。
- cursor shape 初期只支持默认箭头，后续映射常见 IDC。

键盘：

- key down/up 转 `WM_KEYDOWN/WM_KEYUP`。
- 文本输入走 `WM_CHAR`，不要只靠 keycode 推断字符。
- 功能键映射：Tab、Enter、Esc、Backspace、Delete、方向键、Home/End、PageUp/PageDown、Ctrl/Shift/Alt。

IME：

- API 12+ 文档显示 NDK C API 增强了自绘 UI 的输入法能力，应优先调查并接入。
- 若 NDK IME API 不足，MVP 可使用隐藏 ArkTS `TextInput` 接收输入，再通过 NAPI 回调 native 生成 `WM_CHAR/WM_IME_CHAR`。
- `SRichEdit` 和复杂候选窗位置是高风险项，第一阶段只保证 `SEdit` 可输入。

### 6.6 剪贴板

实现顺序：

1. 内部进程剪贴板：支持 `CF_TEXT/CF_UNICODETEXT`，满足 SOUI 控件内复制粘贴。
2. 系统 pasteboard bridge：通过 API 12+ pasteboard NDK 或 ArkTS pasteboard 模块同步文本。
3. 扩展 HTML、图片、自定义格式。

注意：OpenHarmony 5.0.0 之后 pasteboard 读取增加权限和授权要求，自定义控件读取系统剪贴板时需要按平台安全规则处理。

### 6.7 文件与路径

需要新增/验证：

- `GetModuleFileName`
- `GetCurrentDirectory/SetCurrentDirectory`
- `SHGetFolderPath` 或 SOUI 等价应用目录
- `FindFirstFile/FindNextFile`
- `LoadLibrary/dlopen`
- 沙箱路径、`resfile/rawfile`、应用私有目录映射

建议策略：

- SOUI 资源放入 HAP 的 `resources/rawfile` 或安装后复制到应用沙箱。
- `SResProviderFiles` 初期读取解压后的真实文件路径。
- ZIP/7z provider 后续验证。

### 6.8 动态库与组件加载

当前非 Windows 组件名使用 `libxxx` 前缀，例如 `librender-gdi`、`libimgdecoder-stb`。OHOS 上动态库通常打包到 HAP native libs 目录。

建议：

- MVP 使用 `SOUI_ENABLE_COM_LIB=ON`，把必要组件静态合入 `libsoui_ohos.so`，减少动态加载和打包复杂度。
- 后续再恢复动态组件加载，适配 OHOS 的 `dlopen` 路径。

MVP 必选组件：

- `imgdecoder-stb`
- `render-gdi`
- `translator`
- `TaskLoop`
- `resprovider-zip` 可选

暂缓：

- `render-skia`
- `ws`
- `network`
- `ScriptModule-LUA`
- `resprovider-7zip`
- `richedit41/msftedit`

## 7. 分阶段开发计划

### 阶段 0：基线与裁剪

目标：明确最小构建集，避免一开始被非核心模块阻塞。

任务：

- 新增 OHOS CMake preset/toolchain 说明。
- 顶层识别 `OHOS`。
- 新增 `swinx/ohos.cmake` 空后端骨架。
- 关闭 demos/games/tools/websocket/richedit/skia。
- 静态合并核心组件。
- 建立 DevEco 示例 HAP 工程。

完成标准：

- OHOS toolchain 能配置 CMake。
- `utilities4`、`soui4`、基础 components 能进入编译阶段。
- 编译错误按模块归类，有明确阻塞清单。

预计工期：1-2 周。

### 阶段 1：首屏绘制 MVP

目标：在 HarmonyOS 设备或模拟器上显示一个 SOUI 静态界面。

任务：

- NAPI 模块初始化。
- XComponent surface 生命周期接入。
- `SConnection` 最小消息队列。
- 根 `HWND` 创建与 `SHostWnd` 绑定。
- cairo image surface 创建/resize。
- NativeWindow buffer 提交。
- 文件资源 provider 路径打通。
- `WM_PAINT/WM_SIZE/WM_DESTROY` 可用。

完成标准：

- 能启动 HAP。
- XComponent 内显示 SOUI 主界面。
- 旋转或 resize 后能重新布局和重绘。
- 无明显崩溃和黑屏。

预计工期：3-5 周。

### 阶段 2：基础交互

目标：常用控件可操作。

任务：

- 触摸/鼠标映射。
- 基础键盘输入。
- 定时器和动画 tick。
- focus/capture。
- `SEdit` 基础文本输入。
- `SComboBox`、菜单、弹窗在 XComponent 内虚拟显示。
- 简单剪贴板。

完成标准：

- button/list/tree/edit/combo/tab/scrollbar 基本可用。
- demo 页面可点击、滚动、输入。
- 动画和 invalidation 不丢帧到不可接受程度。

预计工期：4-6 周。

### 阶段 3：平台能力补齐

目标：从 demo 可用进入应用可用。

任务：

- IME 正式接入，候选窗位置和组合文本。
- 系统 pasteboard。
- 文件选择器 ArkTS bridge。
- 网络组件或 libwebsockets OHOS 构建。
- 富文本 `msftedit` 编译与验证。
- 字体 fallback/fontconfig 配置。
- HiLog 日志接入。
- 生命周期 pause/resume/background/foreground。
- 多 DPI、深色模式、字体缩放。

完成标准：

- 真实业务界面可运行。
- 输入法、复制粘贴、文件选择能满足常规使用。
- 应用前后台切换稳定。

预计工期：6-10 周。

### 阶段 4：性能与产品化

目标：可交付版本。

任务：

- dirty rect 精细提交。
- buffer 格式和拷贝优化。
- Skia/GPU 路径评估。
- 动态库体积裁剪。
- crash dump、日志、符号表。
- 自动化 UI smoke test。
- 多设备验证：手机、平板、2-in-1、PC 形态。
- 上架权限、隐私、包体审计。

完成标准：

- 常规界面达到可接受帧率。
- 长时间运行无明显泄漏。
- 关键业务路径通过回归测试。

预计工期：4-8 周。

## 8. 关键风险与缓解

| 风险 | 影响 | 缓解 |
| --- | --- | --- |
| HarmonyOS 窗口模型与 Win32 多 HWND 差异大 | 弹窗、菜单、多窗口行为不一致 | MVP 单 XComponent，内部虚拟 popup；后续按 PC 形态扩展 |
| NativeWindow CPU buffer 性能不足 | 动画、大窗口、高 DPI 卡顿 | dirty rect、stride 优化、后续 Skia/GPU |
| IME API 与 Win32 IMM 语义差异 | 中文输入、候选窗、富文本受影响 | 先 ArkTS TextInput bridge，再接 NDK IME |
| fontconfig/freetype 交叉编译和字体配置复杂 | 文本渲染异常、fallback 缺字 | 参考 Qt/vcpkg OHOS triplet；内置默认字体配置 |
| `.rc` 资源内嵌在 OHOS 链接失败 | 系统资源加载失败 | MVP 文件资源；后续 res2cdata 或修正 ELF resource 流程 |
| 动态组件加载路径不稳定 | 运行时找不到组件 so | MVP 静态组件；产品化再动态化 |
| `ENABLE_THUNK` 可执行内存策略 | 安全策略或崩溃 | OHOS 默认关闭 thunk |
| `libmsftedit` 依赖面大 | 富文本控件不可用 | MVP 禁用富文本；阶段 3 专项移植 |
| 剪贴板/文件 picker 权限 | 上架或用户授权问题 | 通过 ArkTS bridge 遵守平台权限模型 |
| 第三方库 license/体积 | 合规和包体风险 | 建立 thirdparty 清单，优先使用系统/NDK/vcpkg 可审计版本 |

## 9. 建议的目录与代码改动清单

新增文件：

```text
swinx/ohos.cmake
swinx/src/platform/ohos/SConnection.h
swinx/src/platform/ohos/SConnection.cpp
swinx/src/platform/ohos/SOhosWindow.h
swinx/src/platform/ohos/SOhosWindow.cpp
swinx/src/platform/ohos/SClipboard.h
swinx/src/platform/ohos/SClipboard.cpp
swinx/src/platform/ohos/keyboard.h
swinx/src/platform/ohos/keyboard.cpp
swinx/src/platform/ohos/imm.cpp
swinx/src/platform/ohos/napi_bridge.h
swinx/src/platform/ohos/napi_bridge.cpp
demos/ohos 或 examples/ohos/DevEco 工程
```

修改文件：

```text
CMakeLists.txt
swinx/CMakeLists.txt
SOUI/CMakeLists.txt
components/CMakeLists.txt
components/render-gdi/CMakeLists.txt
components/render-skia/CMakeLists.txt
__cmake/windres.cmake
SOUI/src/helper/SAppDir.cpp
SOUI/src/res.mgr/SResProvider.cpp
```

尽量不改：

```text
SOUI/include/core/*
SOUI/src/core/Swnd.cpp
SOUI/src/control/*
SOUI/src/valueAnimator/*
```

## 10. MVP 验证用例

### 10.1 构建验证

- `arm64-v8a` Release 构建通过。
- `x86_64` 模拟器构建通过，如 SDK 支持。
- `libsoui_ohos.so` 能被 HAP 加载。
- 所有必要 symbols 可解析，无 `dlopen` 缺库。

### 10.2 首屏验证

- XComponent 创建后 native 收到 surface created。
- `SApplication` 初始化成功。
- SOUI 资源加载成功。
- `SHostWnd` 创建成功。
- 第一帧显示非黑屏。
- resize/surface changed 后不崩溃。

### 10.3 交互验证

- 点击按钮触发事件。
- 列表滚动。
- edit 输入 ASCII。
- edit 输入中文。
- combo 下拉与选择。
- 菜单弹出与关闭。
- 定时器动画运行。
- 前后台切换恢复。

### 10.4 稳定性验证

- 连续 resize 100 次。
- 连续点击/滚动 5 分钟。
- 前后台切换 50 次。
- 销毁页面再创建 50 次。
- 内存增长小于设定阈值。

## 11. 开发优先级

强制优先：

1. OHOS 构建链。
2. NAPI/XComponent 生命周期。
3. 根窗口和消息队列。
4. cairo image surface 和 NativeWindow 提交。
5. 文件资源加载。
6. 触摸/鼠标。
7. 定时器与动画。
8. 基础键盘和文本。

明确后置：

1. Skia/GPU。
2. 多顶层窗口。
3. 拖放。
4. 系统托盘。
5. ActiveX。
6. 辅助功能 MSAA。
7. Spy 工具。
8. 完整 RichEdit。

## 12. 工作量估算

以 2 名熟悉 SOUI/SwinX 的 C++ 工程师 + 1 名 HarmonyOS/DevEco 工程师估算：

| 阶段 | 人周 | 说明 |
| --- | --- | --- |
| 阶段 0 | 2-4 | 构建和裁剪 |
| 阶段 1 | 6-10 | 首屏绘制 |
| 阶段 2 | 8-12 | 基础交互 |
| 阶段 3 | 12-20 | 平台能力补齐 |
| 阶段 4 | 8-16 | 性能与产品化 |

总计：36-62 人周。
若已有 Qt/OpenHarmony、NativeWindow、输入法经验，可压缩到约 28-45 人周。若要求完整 Win32 多窗口语义和富文本，工作量会明显增加。

## 13. 推荐落地策略

第一条原则：不要一开始追求“完整 Win32”。SOUI 已经是 DirectUI，自绘能力强，HarmonyOS 上应优先把它作为一个高性能自绘 view 跑起来。

第二条原则：MVP 使用 `render-gdi + cairo image surface`，把 Skia/GPU 放到首屏和交互稳定之后。图形后端过早复杂化会掩盖窗口和消息问题。

第三条原则：组件先静态合并。OHOS HAP 的 native so 打包、路径和权限与桌面不同，动态组件可作为产品化优化，不应影响 MVP。

第四条原则：IME、剪贴板、文件选择这类受权限和系统 UX 约束的能力，用 ArkTS bridge 比纯 C++ 模拟更稳。

第五条原则：复用 Qt 适配经验，尤其是 OHOS toolchain、vcpkg triplet、fontconfig/libpng/freetype 交叉编译问题，但不要引入 Qt 作为运行时依赖。

## 14. 下一步建议

建议马上开一个 `feature/ohos-port` 分支，按以下顺序推进：

1. 建立 DevEco 空工程，放入一个 XComponent 和 native module。
2. 新增 `swinx/ohos.cmake`，让 `swinx` 能在 OHOS toolchain 下开始编译。
3. 先用假的 `SConnection` 打通 `SApplication` 初始化和根窗口创建。
4. 完成 cairo image surface 到 NativeWindow 的整帧提交。
5. 移植一个最小 SOUI demo 页面，只包含按钮、文本、图片。
6. 再补触摸、键盘、定时器、资源和组件。

只有当这 6 步跑通后，再评估 Skia、RichEdit、WebSocket、动态组件和复杂平台能力。
