# SOUI 移动端（Android / OHOS / iOS）适配方案技术文档

## 1. 概述

SOUI 是一款面向 C++ 的轻量级 DirectUI 框架，核心围绕 Win32 HWND 句柄模型、SHostWnd（顶层窗口宿主）与消息循环构建。为了让一套基于 SWinx（SOUI 的跨平台 Win32 API 仿真层）编写的 SOUI C++ 业务代码以**几乎零改动**的方式运行在移动端，SOUI 提供了三套对等（peer）的原生适配方案：

- **SOUI for Android**：业务代码跑在 Android 主线程，通过 JNI 桥接 Java/Kotlin View 系统；渲染后端为离屏 Bitmap + Skia（SIMD BGRA→RGBA）。
- **SOUI for OHOS（鸿蒙）**：业务代码跑在 UIAbility 主线程（JS 线程），通过 N-API 桥接 ArkUI 组件；渲染后端为离屏 PixelMap + Skia，脏矩形回传局部上屏。
- **SOUI for iOS**：业务代码通过 Objective-C++ 直接接入 UIKit / Core Graphics，无桥接层；定时器由 C++ 消息循环直接驱动。

三套方案共享同一套 SWinx 跨平台抽象，仅在"原生窗口体系 / 桥接技术 / 渲染后端 / 线程模型"上存在平台差异；业务层 SWinx C++ 代码（`CreateWindowEx` / `SetTimer` / `SetCapture` / 消息循环 …）完全复用。同一份业务代码（如 `games/cnchess/client`）即可同时构建出 Android、OHOS、iOS 安装包，以及 Windows / Linux / macOS 桌面程序——只需切换入口函数与平台层。

本文档从**总体架构（三端共享机制）→ 各平台适配实现（Android / OHOS / iOS）→ 竞品对比（Qt/Flutter）→ 原生集成优势 → 编译与测试**五个维度完整介绍这套移动端适配技术。

---

## 2. 总体架构：三端共享的核心设计机制

尽管三个平台使用不同的原生 UI 体系与桥接技术，它们在底层设计上高度一致。理解以下六条共享机制，是阅读后续各平台章节的前提。

### 2.1 SWinx：Win32 API 跨平台仿真层

SWinx 是 SOUI 的跨平台抽象核心，把 Win32 的 `CreateWindowEx / SetTimer / SetCapture / GetMessage / SendMessage / 剪贴板 / RawInput` 等 API 抽象为与平台无关的接口。各平台在 `swinx/src/platform/<os>`（或各 `soui-*-lib`）下提供具体实现，业务层只依赖 SWinx 头文件，不感知平台差异。详见各平台章节的"平台层"小节。

### 2.2 HWND = 原生对象指针（零查表映射）

三端均采用同一思路：**HWND 即原生窗口对象的指针，不做 Handle → Object 的 Map 查表**。

- Android：HWND = `jobject` 的 `NewGlobalRef` 指针值（见 §3.2）。
- OHOS：HWND = `napi_ref` 指针值（见 §4.2）。
- iOS：HWND = `SUIView`（UIView 子类）对象指针（见 §5.2）。

任意窗口状态操作都直接对指针解引用调用原生接口，**无哈希开销、无类型转换**。

### 2.3 Windows 消息语义严格对齐

三端都保证 Win32 消息语义一致：`WM_LBUTTONDOWN/UP/DBLCLK/MOVE`、`WM_RBUTTONDOWN`、`WM_MOUSEWHEEL/WM_MOUSEHWHEEL`、`WM_XBUTTON*`、`WM_TIMER`、`WM_SETFOCUS/WM_KILLFOCUS`、`WM_KEYBOARD_HEIGHT` 等在不同平台上的行为、参数、返回值一致，业务代码无需"平台特判"。

### 2.4 线程与消息循环模型

- **iOS** 最彻底：C++ 业务跑在主线程 runloop（`CFRunLoopRunInMode`），定时器由 C++ 消息循环 tick 直接驱动，无独立调度层（见 §5.7）。
- **Android / OHOS**：C++ 业务跑在原生主线程（UI 线程 / JS 线程）。主线程调用走"快路径"直接调原生桥；**仅当调用发生在工作线程时**，才经跨线程机制（Android `Handler.post`、OHOS `napi_threadsafe_function`）投递回主线程执行——根因是 JNIEnv / napi_env 不能跨线程使用。

### 2.5 String Slot 字符串交换机制(Android,鸿蒙)

跨层字符串参数（如 `WM_SETTEXT`）通过 Slot 机制交换：C++ 侧 `AutoStringSlot`（RAII）分配 1~65535 环形复用的槽 ID（0 保留无效），原生层按槽 ID 读写字符串数据，避免每次 JNI/N-API 字符串拷贝与编码转换开销。

### 2.6 screenId 多窗口路由（激活栈）

当 App 存在多个 Activity / Ability / Window 时，创建顶层窗口（`hWndParent=HWND_DESKTOP`）需要知道挂到哪个"桌面容器"。三端均以 `screenId` 激活栈实现：`screenStartup` push、`screenShutdown` pop，创建窗口从栈顶取当前激活 screenId。详见各平台"多 Screen"小节。

### 2.7 三端横向对比总表

| 维度 | SOUI for Android | SOUI for OHOS | SOUI for iOS |
|---|---|---|---|
| 原生 UI 体系 | Android View/ViewGroup（Java/Kotlin） | ArkUI 组件（ArkTS） | UIKit UIView/UIWindow（Obj-C/Swift） |
| 桥接技术 | JNI（JavaVM 全局引用） | N-API（node_api，弱符号链接 OH NDK） | Objective-C++ 直接调用（无桥接层） |
| HWND 真身 | jobject GlobalRef 指针 | napi_ref 指针 | SUIView（UIView 子类）指针 |
| 渲染后端 | 离屏 Bitmap + Skia，SIMD BGRA→RGBA | 离屏 PixelMap + Skia，脏矩形回传局部上屏 | Core Graphics（CGContextRef） |
| 上屏方式 | canvas.drawBitmap | Canvas writePixelsSync + drawImage（局部脏矩形） | UIView drawRect → CGContext 提交 |
| 线程模型 | 共用主线程，JNI 零跨线程 | 主线程 callBridge 快路径；工作线程 napi_threadsafe_function 投递 | 主线程 CFRunLoop；C++ 消息循环直接驱动定时器 |
| 定时器 | Handler(Looper.getMainLooper) | setInterval/clearInterval | C++ TimerInfo 链表 + 消息循环 tick |
| 入口 | Soui4AndroidEntry（JNI_OnLoad） | Soui4OhosEntry（N-API 模块注册） | swinx_ios_entry() C API + AppDelegate |
| IME | EditText / InputConnection | inputMethod.InputMethodController | UIKeyInput / UIResponder |
| 剪贴板 | ClipboardManager | @ohos.pasteboard | UIPasteboard |
| 构建系统 | Gradle + CMake（AGP/NDK） | hvigor + CMake（OHOS SDK） | CMake（CMAKE_SYSTEM_NAME=iOS，定义 __IOS__） |
| 独立适配库 | soui-android-lib | soui-ohos-lib | swinx/src/platform/ios（平台层，业务经 swinx_ios_entry 直连） |

### 2.8 三端架构总览

- **Android（JNI）**：见 §3.1 架构图。C++ 与 Java 同线程，每个 `SouiSurface` 对应一个 Android `View`。
- **OHOS（N-API）**：见 §4.1 架构图。C++ 与 ArkTS 同线程，每个 `SouiSurface` 对应一个 ArkUI `Canvas` 组件。
- **iOS（Obj-C++）**：见 §5.1 架构图。C++ 经 `swinx_ios_entry` 接入 UIKit，每个 HWND 对应一个 `SUIView`。

三端共同保证：HWND 即原生窗口对象指针（零查表）、Windows 消息语义严格对齐、SetCapture/ReleaseCapture 一致、String Slot 字符串交换机制一致、多 Screen（screenId 激活栈）一致。

---

## 3. SOUI for Android 适配方案

Android 是三套方案中的参考实现：C++ 业务代码**不启独立渲染线程，完全跑在 Android 主线程**，与 Qt（独立 QtThread）、Flutter（独立 UI/Raster/IO 线程池）形成鲜明对比。其直接收益是 JNI 调用无需跨线程调度、View 操作无需 post、与原生生命周期天然对齐。

### 3.1 三层架构总览

```
┌──────────────────────────────────────────────────────────────────┐
│  Android Java / Kotlin 层 (UI 线程 = 主线程)                      │
│  ┌──────────────┐  ┌────────────┐  ┌──────────────────────────┐ │
│  │ Activity /   │  │ FrameLayout│  │ SouiPlatformBridge       │ │
│  │ Fragment     │  │ (任意原生   │  │  - 单例全局 JNI 桥       │ │
│  │              │  │  布局容器)  │  │  - createWindow/定时器/  │ │
│  │              │  │            │  │    SetCapture/View 工厂   │ │
│  └──────┬───────┘  └─────┬──────┘  └────────────┬─────────────┘ │
│         │                │                        │               │
│  ┌──────▼────────────────▼────────────────────────▼──────────┐   │
│  │ SouiAbsLayout 绝对布局引擎 (继承 ViewGroup)               │   │
│  │  ├─ SouiAbsWindow（单 HWND 容器抽象）                    │   │
│  │  │    └─ SouiWindow（单 HWND 具体容器）                  │   │
│  │  │         └─ SouiScreen（桌面容器 = GetDesktopWindow）   │   │
│  │  │              └─ SouiBaseSurface → SouiSurface          │   │
│  │  └─ SouiBaseSurface → SouiSurface (INativeWindow 接口,   │   │
│  │                              对应 1 个 C++ HWND GlobalRef) │   │
│  └───────────────────────────────────────────────────────────┘   │
├──────────────────────────────────────────────────────────────────┤
│  JNI 层（双向；方法 ID 在 init 时一次性缓存，避免运行时 FindClass）│
│  C++ → Java: AndroidPlatformAPI (CallVoidMethod/CallObjectMethod)│
│  Java → C++: SouiPlatformBridge native 方法 / INativeWindow      │
├──────────────────────────────────────────────────────────────────┤
│  C++ Native 层 (运行在 Android UI 线程，无独立 QtThread/RenderT) │
│  ┌─────────────┐  ┌───────────────┐  ┌────────────────────────┐ │
│  │   SWinx     │  │AndroidPlatform │  │    SOUI Core           │ │
│  │(Win32 API   │──│   API 实现     │──│  (SHostWnd/SNativeWnd/ │ │
│  │ 仿真抽象层) │  │               │  │   SWnd/Skin/Xml 资源)  │ │
│  └─────────────┘  └───────────────┘  └────────────────────────┘ │
│  SouiSurfaceProxy：每个 Surface 的输入渲染 native 实现（含 SIMD）    │
```

### 3.2 HWND = INativeWindow GlobalRef：零查表映射机制

传统跨平台 UI 框架通常维护一张 `Handle → Object` 的 Map（从整型句柄查表得到底层对象）。SOUI for Android 采用更直接的策略：

- Java 层定义接口 `INativeWindow`（见 [INativeWindow.java](../demos/android-demo/app/src/main/java/com/soui/android/INativeWindow.java)），声明窗口状态操作：`nativeDestroy / nativeInvalidate / nativeShow / nativeMove / nativeIsVisible / asView / nativeSendMessage` 等。
- 每个窗口对应的 Surface View（`SouiSurface` 或原生包装 View）实现该接口。
- C++ 层拿到这个 `jobject` 后，调用 `NewGlobalRef` 锁定；**GlobalRef 的指针值（`UINTPTR` / `jlong`）就是 SWinx 的 HWND**。
- 从此 C++ 层任何窗口状态操作，直接 `CallXxxMethod(hwnd, methodId, ...)` 即可，**无需 Map 查表、无需类型转换、无哈希开销**。`SouiPlatformBridge.nativeViewFromHwnd` 是唯一的"反向"小工具，几行代码就把 `jlong` 还原为本地引用 View。

```cpp
// C++ 层 INativeWindow 方法表初始化（一次性，见 AndroidPlatformAPI.cpp init）
jmethodID midDestroy     = env->GetMethodID(clsNativeWnd, "nativeDestroy", "()V");
jmethodID midInvalidate  = env->GetMethodID(clsNativeWnd, "nativeInvalidate","(IIII)V");
jmethodID midShow        = env->GetMethodID(clsNativeWnd, "nativeShow", "(I)Z");
// 之后任意 HWND 操作：
env->CallVoidMethod((jobject)hWnd, midShow, SW_SHOW);  // 零中间层
```

### 3.3 窗口层级与容器模型

SOUI 的"HWND 树"在 Android 上直接映射为标准的 Android `View / ViewGroup` 树（而非画到一张全屏位图里再合成）：

| C++ 语义 | Java 对应类 | 职责 |
|---|---|---|
| `GetDesktopWindow()` 桌面 | `SouiScreen`（继承 `SouiWindow`） | 桌面级容器；挂在 Activity 布局里，作为所有顶级窗口的父。创建自身的 `SouiBaseSurface`（不做绘制，仅作为 HWND 占位），只承载 `SouiWindow` 子节点与屏幕尺寸同步。详见 [SouiScreen.java](../demos/android-demo/app/src/main/java/com/soui/android/SouiScreen.java) |
| 一个 HWND 窗口（SHostWnd / SNativeWnd） | `SouiWindow`（继承 `SouiAbsWindow`） | 对应一个 C++ 窗口的 ViewGroup 容器：index=0 是该窗口的主 Surface，其余 index≥1 是子 HWND 子容器。见 [SouiWindow.java](../demos/android-demo/app/src/main/java/com/soui/android/SouiWindow.java) |
| HWND 的像素绘制面（客户区） | `SouiSurface`（继承 `SouiBaseSurface`） | 继承自 `View`，实现 `INativeWindow`；`onDraw` 内把 C++ 层用 Skia 画好的离屏 Bitmap 用 `canvas.drawBitmap` 贴到自己区域。每个 HWND 一张 Surface，独立 invalidate，独立 draw。 |
| 原生 EditText/Button 等控件作为 SOUI 子控件 | 由 `registerViewFactory("edit", NativeEditView::new)` 注册工厂 | className 命中即创建原生 View，同样包装 INativeWindow，获得同等 HWND 句柄，支持 `SetDlgItemText/EnableWindow` 等语义。 |
| 消息占位窗口（HWND_MESSAGE） | `SouiBaseSurface`（不挂视觉树） | 不可见的 0x0 占位窗口，用于消息处理（如定时器），不参与布局和绘制。 |

`SouiAbsLayout`（见 [SouiAbsLayout.java](../demos/android-demo/app/src/main/java/com/soui/android/SouiAbsLayout.java)）是 `SouiAbsWindow` 的父类，提供统一的绝对定位 measure/layout：子 View 的 `LayoutParams(x, y, width, height)` 直接使用 SWinx `RECT` 语义。`SouiScreen` 通过 `SouiWindow` → `SouiAbsWindow` 间接继承 `SouiAbsLayout`。关键细节：

- `SouiAbsLayout.LayoutParams extends ViewGroup.MarginLayoutParams`：**兼容被外层 FrameLayout/LinearLayout 等原生容器托管时的 `measureChildWithMargins` ClassCastException**。
- `SouiAbsWindow.measureChild` 重载：强制 index=0 的主 Surface MATCH_PARENT 填满容器（HWND 客户区与容器等价）。
- `SouiAbsLayout.updateChildFrame / setChildPosition / setChildSize`：提供原子性的子 View 位置和尺寸更新。
- `SouiScreen.createSurface` 重载：创建 `SouiBaseSurface`（而非 `SouiSurface`），因为 SouiScreen 本身不做绘制。

### 3.4 Surface 渲染流程（离屏 Bitmap + Skia + SIMD 颜色通道转换）

每个 `SouiSurface` 的 `onDraw(Canvas canvas)` 流程：

1. **懒初始化离屏 Bitmap**：首次有效尺寸或尺寸变化时，`createOffscreenBitmap(w, h)` 创建 `ARGB_8888` Bitmap，并调用 `nativeOnSizeChanged` 通知 C++ 更新 SWinx VirtualHWND Rect。尺寸匹配时复用现有 Bitmap，避免内存抖动。
2. **C++ Skia 渲染**：`nativeRender(nativeId, offscreenBitmap)` 将 `Bitmap` 的像素缓冲区（通过 `AndroidBitmap_lockPixels`）绑定为 `CreateDIBSectionEx` 的 DIB Section，SOUI 完成当前窗口的所有 DirectUI 控件绘制（Skin、文字、SVG、渐变等），通过 `SendMessage(hWnd, WM_PAINT)` 写入到这块 bitmap。
3. **SIMD 颜色通道转换**：Cairo/Skia 渲染输出为 BGRA 格式，而 Android Bitmap 期望 RGBA 格式。`SouiSurfaceProxy::render` 使用 SIMD 指令（NEON/AArch64/SSSE3）批量交换 R/B 通道，一次处理 4 像素，大幅提升转换效率。
4. **合成到屏幕**：`canvas.drawBitmap(offscreenBitmap, 0, 0, paint)`。Android 系统把该 View 的内容作为一个 Layer，交给 SurfaceFlinger 与状态栏、导航栏、其他原生 View 一起合成。

**SIMD 优化策略**：
- **AArch64 NEON**：使用 `vqtbl1q_u8` 查表指令，一次处理 16 字节（4 像素）
- **ARMv7-A NEON**：使用 `vtbl1_u8` 查表指令，双半区并行处理
- **SSSE3**：使用 `_mm_shuffle_epi8` 指令，一次处理 16 字节
- **标量兜底**：通用路径使用逐像素交换，确保兼容性

**和 Flutter/Qt 的本质不同**：SOUI 不独占一个全屏 Surface；每个 SOUI 窗口在 View 树里是一个独立节点，可与原生 Button、Toolbar、RecyclerView 交叉混合布局，Z-order 由父 ViewGroup addView 顺序天然决定。

### 3.5 输入事件分发：严格对齐 Win32 `SetCapture`

输入路径（见 [SouiBaseSurface.java](../demos/android-demo/app/src/main/java/com/soui/android/SouiBaseSurface.java) `dispatchTouchEventToNative` 与 C++ [SouiSurfaceProxy.cpp](../demos/android-demo/app/src/main/cpp/SouiSurfaceProxy.cpp) `onMotionEvent`）：

1. Android 系统按正常 `onInterceptTouchEvent`/`dispatchTouchEvent` 把事件派到被命中的 `SouiSurface` View。
2. Java 层先调用 `SouiPlatformBridge.tryDispatchCapturedMotion`：**如果当前有 SetCapture，并且事件来源 Surface ≠ 捕获目标 Surface，则做坐标变换（源 View 屏幕坐标 → 目标 View 客户区坐标）后，直接投递给捕获目标的 `nativeOnMotionEventEx`**，返回 Boolean.TRUE 表示"已消费重定向"。
3. 未被 Capture 拦截时，正常投递 `nativeOnMotionEventEx(action, x, y, pointerId, buttonState, vscroll, hscroll, metaState, timestamp)` 到当前 Surface。
4. C++ `onMotionEvent` 根据 action 派发到 `WM_LBUTTONDOWN / WM_LBUTTONDBLCLK / WM_MOUSEMOVE / WM_RBUTTONDOWN / WM_MOUSEWHEEL / WM_MOUSEHWHEEL / WM_XBUTTONDOWN / WM_POINTER...`，完全对齐 Win32 MSG 格式，`SendMessage(hHost, ...)` 进入 SOUI 消息循环。
5. 遇到 `ACTION_UP / ACTION_CANCEL`：投递完后自动 `ReleaseCapture`（对应 Win32 "最后一个鼠标按键弹起时系统自动释放捕获"语义），保证滚动条拖拽、标题栏拖拽等控件交互与 Win32 表现一致。

**扩展输入支持**：
- **双击检测**：记录上次按下时间和坐标，若两次按下时间间隔小于系统双击阈值（默认 500ms）且坐标距离在阈值内，触发 `WM_LBUTTONDBLCLK`
- **右键菜单**：长按事件转换为 `WM_RBUTTONDOWN`，支持上下文菜单弹出
- **滚轮事件**：支持垂直滚轮（`WM_MOUSEWHEEL`）和水平滚轮（`WM_MOUSEHWHEEL`），外接鼠标时正常工作
- **悬停事件**：支持 `TrackMouseEvent` 语义，`WM_MOUSEHOVER`/`WM_MOUSELEAVE` 消息正确投递
- **XButton 支持**：鼠标侧键转换为 `WM_XBUTTONDOWN`/`WM_XBUTTONUP`

`SouiPlatformBridge.setCapture / releaseCapture / getCapture` 三个方法（JNI 暴露给 C++ `AndroidPlatformAPI`）是该机制的状态总控。

### 3.6 焦点管理：SetFocus/GetFocus 双向同步

`SouiPlatformBridge` 维护全局焦点状态 `mFocusHwnd`（volatile long），实现 Win32 `SetFocus`/`GetFocus` 语义：

- **C++ → Java 同步**：`AndroidPlatformAPI::setFocus` 调用 Java `setFocus(hwnd)`，更新 `mFocusHwnd`，并同步 Android View 焦点和 IME 状态
- **Java → C++ 同步**：`SouiBaseSurface` 在获得焦点时调用 `nativeNotifyFocusGained(hwnd)`，触发 C++ 层 `::SetFocus`，投递 `WM_SETFOCUS`/`WM_KILLFOCUS` 消息

**IME 策略**：
- `EditText` 子类（`NativeEditView` → className="edit"）自动弹出 IME，支持光标位置查询（`EM_GETSEL`）和文本设置（`WM_SETTEXT`）
- 其他 `SouiBaseSurface`：SOUI 自己实现了 SEdit 等控件，用离屏渲染绘制文本，但仍依赖 Android IME 把按键输入转成字符，也弹出 IME
- **输入连接管理**：`SouiBaseSurface.onCreateInputConnection` 创建 `SouiInputConnection`，处理字符提交（`commitText` → `sendImeString` → `WM_IME_CHAR`）
- **IME 选项**：设置 `IME_FLAG_NO_FULLSCREEN`，避免横屏时全屏输入法遮挡内容

### 3.7 定时器：`SetTimer/KillTimer` 语义 100% 对齐 Win32

`SouiPlatformBridge` 内部用 `Handler(Looper.getMainLooper())` + 嵌套 Map `Map<hWnd, Map<timerId, TimerRunnable>>` 实现：

- `(hWnd, timerId)` 二元组唯一标识一个定时器；不同 hWnd 之间相同数值的 timerId **互不冲突**。
- `hWnd != 0 && nIDEvent == 0`：系统在该 hWnd 下自动分配一个未使用的 id。
- `hWnd == 0`（非窗口定时器）：Win32 文档语义"忽略 nIDEvent，系统生成全局唯一 id"，严格复现。
- `TimerRunnable.run`：一次触发后，若仍注册且 active，则 `postDelayed(self, interval)`，完成"默认循环"行为；同时 JNI 调用 `nativeOnTimerExpired(hWnd, timerId)` → C++ `AndroidPlatformAPI::onTimerExpired` → `PostMessage WM_TIMER` 或直接调 `TIMERPROC`。
- C++ 层 `m_timerEntries` 镜像 Java 层的定时器状态，仅记录 `TIMERPROC` 回调与元数据，真正调度由 Java Handler 完成。

### 3.8 多 Activity / 多 Screen（screenId 机制）

一个 Android App 里往往有多个 Activity，每个 Activity 都可能承载自己的 SouiScreen；C++ 需要知道"创建一个顶级窗口（WS_POPUP/WS_OVERLAPPED）时，应该把它加到哪个 SouiScreen 的 View 树里"。方案：

- **激活栈机制**：`AndroidPlatformAPI` 维护 `m_activeScreenStack`（vector<jlong>），`screenStartup` 时 push，`screenShutdown` 时 pop。创建窗口遇到 `hWndParent=HWND_DESKTOP(0)` 时，从栈顶取当前激活的 screenId。
- **Screen Context 映射**：`m_screenContexts`（map<jlong, HWND>）存储 screenId 到其 Surface HWND 的映射。
- **原子启动**：Java 层 `SouiPlatformBridge.screenStartup(screenId, screenHwnd, layout)` 一次 JNI 调用完成四件事：
  1. `RegisterVirtualHWND` 注册虚拟 HWND
  2. `setScreenHwnd` 写入 screenId → HWND 映射
  3. `pushActiveScreen` 压入激活栈
  4. 调用 `s_entry->ScreenStartup(screenId, layout)` 创建主窗口（通过 `Soui4AndroidEntry` 虚函数接口）
- **原子销毁**：`screenShutdown(screenId)` 调用 `s_entry->ScreenShutdown(screenId)` → `popActiveScreen` → `eraseScreen` → `UnregisterVirtualHWND`。
- **启动流程优化**：`SouiScreen.onAttachedToWindow` 中，若 `mScreenId != 0` 才调用 `screenStartup`，避免未配置 screenId 时的误启动。

**启动参数传递**：
- `screenId`：唯一标识当前 Screen，用于多 Activity 场景下的窗口路由
- `screenHwnd`：Screen 对应的 Surface HWND，作为桌面窗口的 HWND
- `layout`：SOUI 布局文件名（如 `"layout:dlg_main"`），指定启动时加载的 XML 布局

**创建窗口时的路由逻辑**：
1. 若 `hWndParent == HWND_DESKTOP(0)`：从激活栈顶获取当前 screenId，查找对应的 Screen HWND 作为实际父窗口
2. 若 `hWndParent == HWND_MESSAGE(-1)`：创建消息占位窗口，不挂视觉树
3. 其他情况：直接使用指定的父窗口 HWND

### 3.9 屏幕旋转与生命周期管理

Android 的配置变更（如屏幕旋转）会触发 Activity 重建，导致所有 View 被销毁后重新创建。由于 SOUI 的所有 HWND 都映射到 Java 层的 View 对象（`SouiSurface` / `SouiWindow`），这些 HWND 也会随之销毁和重建。

**生命周期绑定**：
- `SouiScreen.onAttachedToWindow()`：调用 `screenStartup` 创建 C++ 窗口系统
- `SouiScreen.onDetachedFromWindow()`：调用 `screenShutdown` 销毁 C++ 窗口系统
- `SouiScreen.onSizeChanged()`：调用 `moveWindow` 和 `updateScreenSize` 更新窗口尺寸

**旋转行为**：
1. Activity 销毁 → `onDetachedFromWindow` → `screenShutdown` → 所有 C++ HWND 销毁
2. Activity 重建 → `onAttachedToWindow` → `screenStartup` → 重新创建 C++ 窗口系统

**注意事项**：
- C++ 层窗口状态（如控件内容、滚动位置）在旋转时会丢失，需要业务层自行保存和恢复
- `mSouiMainHwnd` 在旋转后会变为新值，需要重新获取
- 尺寸同步通过 `onSizeChanged` 自动处理，确保 C++ 窗口尺寸与新屏幕尺寸一致

### 3.10 String Slot 机制：JNI 字符串参数交换

由于 JNI 字符串传递涉及内存管理和编码转换，采用 Slot 机制进行字符串交换：

**设计原则**：
- C++ 层创建字符串槽（`AutoStringSlot`），分配唯一 ID（范围 1~65535，0 保留为无效）
- 字符串数据存储在 `AndroidPlatformAPI` 的 `m_slotStrings` 映射中
- C++ 通过 Slot ID 传递字符串引用到 Java
- Java 通过 Slot ID 读写字符串数据
- `AutoStringSlot` 采用 RAII 模式，作用域结束自动释放槽
- 槽 ID 超出范围时自动环形重置，保证无限复用

**使用流程**（以 `WM_SETTEXT` 为例）：

1. C++ 端（`Android_Edit.cpp`）：
   ```cpp
   case WM_SETTEXT: {
       const char* str = reinterpret_cast<const char*>(lParam);
       AutoStringSlot slot(str);  // 创建槽，自动分配 ID
       return api.sendMessage((UINT_PTR)hWnd, message, wParam, (LPARAM)slot.id());
   }  // slot 析构，自动释放槽
   ```

2. Java 端（`NativeEditView.java`）：
   ```java
   case WM_SETTEXT: {
       final int slotId = (int) lp;
       final CharSequence text = SouiPlatformBridge.nativeReadCxxStringSlot(slotId);
       setText(text == null ? "" : text);
       return 1;
   }
   ```

**槽 ID 编码**：
- 范围：`[1..65535]`，0 保留为无效
- `EM_GETLINE` 特殊编码：`lParam = (maxChars << 16) | (slotId & 0xFFFF)`，高 16 位为缓冲区容量，低 16 位为槽 ID

**关键类**：
- `AutoStringSlot`（[AndroidPlatformAPI.h](../demos/android-demo/app/src/main/cpp/AndroidPlatformAPI.h)）：RAII 包装类，构造时分配槽，析构时释放槽
- `stringSlotAlloc / WriteString / ReadString / stringSlotFree`：槽管理方法

### 3.11 消息转发机制：统一消息路由

`INativeWindow.nativeSendMessage(int msg, long wParam, long lParam)` 提供了统一的消息转发接口：

- **字符串参数**：通过 String Slot 机制交换
- **纯数值消息**：按 Win32 语义直接传递
- **特殊处理**：`EM_GETSEL` 返回值编码为 `(start << 32) | end`，C++ 层解码后写入用户缓冲区

### 3.12 SouiSurfaceProxy：输入事件转换与渲染代理

每个 `SouiSurface` 在 C++ 层对应一个 `SouiSurfaceProxy` 对象：

- **生命周期管理**：通过 `std::shared_ptr` 管理，`nativeViewInsert`/`nativeViewLookup`/`nativeViewErase` 提供线程安全的访问。`NativeDestroy` 时先从 map 移除，再让局部 `shared_ptr` 出作用域析构，确保其他线程正在持有的引用安全完成。
- **输入事件转换**：`onMotionEvent` 将 Android MotionEvent 转换为 Win32 风格消息（`WM_LBUTTONDOWN`、`WM_MOUSEMOVE`、`WM_MOUSEWHEEL`、`WM_XBUTTONDOWN` 等），支持双击检测、右键菜单、滚轮（垂直+水平）、悬停事件。
- **键盘事件转换**：`onKeyEvent` 将 Android KeyEvent 转换为 Win32 键盘消息，支持 `repeatCount`、`scanCode`、`unicodeChar`、`metaState` 等参数，正确处理 ALT 修饰键生成 `WM_SYSKEYDOWN/SYSKEYUP`。
- **渲染**：`render` 方法将 C++ 层绘制结果绑定到 Java Bitmap，包含 SIMD 优化的 BGRA→RGBA 颜色通道转换。
- **鼠标状态跟踪**：维护 `m_buttonsDown`、`m_lastDownTime/X/Y`、`m_hoverTracked` 等状态，支持 `TrackMouseEvent` 语义。

### 3.13 剪贴板 API：Win32 语义完整实现

剪贴板操作在 Java 层通过 `ClipboardManager` 实现，C++ 层通过 JNI 桥接调用，**直接使用 String Object 交换数据，不再使用 string slot 技术**：

**C++ 层 API**（见 [AndroidPlatformAPI.h](../demos/android-demo/app/src/main/cpp/AndroidPlatformAPI.h)）：
- `clipboardOpen(hWndNewOwner)`：打开剪贴板，设置所有者
- `clipboardClose()`：关闭剪贴板
- `clipboardEmpty()`：清空剪贴板
- `clipboardGetData(uFormat)`：获取剪贴板数据，支持 `CF_TEXT` 和 `CF_UNICODETEXT`，直接从 Java 返回 `jstring`
- `clipboardSetData(uFormat, hMem)`：设置剪贴板数据，直接传递 `jstring` 到 Java
- `clipboardIsFormatAvailable(format)`：检查指定格式是否可用
- `clipboardRegisterFormat(pszName)`：注册自定义格式
- `clipboardGetOwner()`：获取剪贴板所有者
- `clipboardHasFormat(format)`：检查剪贴板是否包含指定格式

**数据交换方式**：
- `clipboardGetData`：Java 返回 `String`，C++ 使用 `GetStringUTFChars` 读取，`ReleaseStringUTFChars` 释放
- `clipboardSetData`：C++ 使用 `NewStringUTF` 创建 `jstring`，Java 直接使用参数

### 3.14 RawInput 设备枚举：跨平台输入设备检测

Android 平台通过 JNI 调用 `SouiPlatformBridge.getInputDevices()` 获取系统输入设备列表：

**实现流程**：
1. Java 层枚举 `InputDevice.getDeviceIds()`，获取每个设备的名称和类型
2. 设备类型映射：`SOURCE_KEYBOARD`/`SOURCE_DPAD` → `RIM_TYPEKEYBOARD`，`SOURCE_MOUSE`/`SOURCE_TOUCHPAD` → `RIM_TYPEMOUSE`，其他 → `RIM_TYPEHID`
3. C++ 层 `getRawInputDeviceList` 接收设备数组，填充 `RAWINPUTDEVICELIST` 结构
4. `getRawInputDeviceInfoA/W` 提供设备名称和设备信息查询

**设备信息结构**（`RID_DEVICE_INFO`）：
- 鼠标：按钮数量、采样率、是否支持水平滚轮
- 键盘：类型、功能键数量、指示灯数量、总键数
- HID：厂商 ID、产品 ID、版本号、使用页、使用值

### 3.15 键盘高度同步：IME 弹出时自动调整布局

`SouiScreen` 通过 `WindowInsetsAnimation.Callback` 监听软键盘弹出/收起事件：

```java
// API ≥ 30 时注册监听器
rootView.setWindowInsetsAnimationCallback(new WindowInsetsAnimation.Callback(DISPATCH_MODE_CONTINUE_ON_SUBTREE) {
    @Override
    public WindowInsets onProgress(@NonNull WindowInsets insets, ...) {
        int imeHeight = insets.getInsets(WindowInsets.Type.ime()).bottom;
        SouiPlatformBridge.nativeSetKeyboardHeight(imeHeight);
        return insets;
    }
});
```

C++ 层接收到键盘高度后，通过 `WM_KEYBOARD_HEIGHT` 消息通知根窗口，业务层可据此调整布局。

### 3.16 IME 输入支持：软键盘字符输入

`SouiBaseSurface` 实现 `onCreateInputConnection`，支持 Android 输入法：

- **输入类型**：`TYPE_CLASS_TEXT | TYPE_TEXT_VARIATION_NORMAL`
- **IME 选项**：`IME_FLAG_NO_FULLSCREEN`，避免横屏时全屏输入法遮挡内容
- **字符提交**：`SouiInputConnection.commitText` 通过 `sendImeString` 将字符投递到 C++ 层，转换为 `WM_IME_CHAR` 消息

### 3.17 HWND_MESSAGE 支持：消息占位窗口

支持 Win32 的 `HWND_MESSAGE` 语义，创建不可见的消息占位窗口：

- **创建流程**：`createWindow` 检测到 `hWndParent == HWND_MESSAGE` 时，创建 `SouiBaseSurface`（不挂视觉树），尺寸强制为 0x0，可见性为 `GONE`
- **存储机制**：单独存储在 `mMessageOnlyWindows` 集合中，与普通窗口分离
- **用途**：用于定时器消息处理等不需要视觉呈现的场景

---

## 4. SOUI for OHOS（鸿蒙）适配方案

### 4.1 三层架构总览（ArkTS + N-API）

```
┌──────────────────────────────────────────────────────────────┐
│  ArkTS 层（UIAbility 主线程 = JS 线程）                       │
│  ┌──────────────┐  ┌────────────┐  ┌──────────────────────┐  │
│  │ UIAbility /  │  │ ArkUI 组件 │  │ SouiPlatformBridge    │  │
│  │ Page         │  │ (SouiScreen│  │  - 单例 N-API 桥      │  │
│  │              │  │  SouiSurface│ │  - createWindow/定时器/│ │
│  │              │  │  NativeEdit│  │    SetCapture/焦点工厂 │  │
│  └──────┬───────┘  └─────┬──────┘  └──────────┬───────────┘  │
│  ┌──────▼────────────────▼─────────────────────▼──────────┐  │
│  │ ArkUI 组件树（SouiAbsLayout → SouiWindow → SouiScreen → │  │
│  │   SouiSurface / NativeEditView，均实现 INativeWindow）  │  │
│  └─────────────────────────────────────────────────────────┘ │
├──────────────────────────────────────────────────────────────┤
│  N-API 层（双向；方法句柄在 init 时一次性缓存，弱符号链接 OH NDK）│
│  C++ → ArkTS: OhosPlatformAPI（callBridge / invokeBridge）   │
│  ArkTS → C++: Soui4Ohos_NAPI（surf_render / surf_onMotion…） │
├──────────────────────────────────────────────────────────────┤
│  C++ Native 层（运行在 UIAbility 主线程；工作线程经 tsfn 回投）  │
│  ┌─────────────┐  ┌───────────────┐  ┌────────────────────┐ │
│  │   SWinx     │  │ OhosPlatformAPI│  │    SOUI Core        │ │
│  │(Win32 API   │──│  实现（N-API） │──│  (SHostWnd/SNativeWnd│ │
│  │ 仿真抽象层) │  │               │  │   /SWnd/Skin/Xml)   │ │
│  └─────────────┘  └───────────────┘  └────────────────────┘ │
│  SouiSurfaceProxy：每个 Surface 的输入渲染 native 实现         │
└──────────────────────────────────────────────────────────────┘
```

与 Android 方案完全同构：仅把 JNI 替换为 N-API、jobject GlobalRef 替换为 napi_ref、Handler 替换为 setInterval、Android Bitmap 替换为 ArkUI PixelMap/Canvas。业务层 SWinx C++ 代码（`CreateWindowEx` / `SetTimer` / `SetCapture` …）语义不变。

### 4.2 HWND = napi_ref 指针：零查表映射

- ArkTS 定义接口 `INativeWindow`（见 [INativeWindow.ets](../soui-ohos-lib/src/main/ets/INativeWindow.ets)）：`nativeDestroy / nativeInvalidate / nativeShow / nativeMove / nativeIsVisible / nativeSendMessage` 等。
- 每个窗口对应的 Surface 组件（`SouiSurface` 或原生包装 `NativeEditView`）实现该接口。
- C++ 层对 ArkTS 对象调用 `napi_create_reference` 锁定；**引用指针值（int64 / napi_ref）就是 SWinx 的 HWND**。
- 任意窗口操作：`napi_get_reference_value` 还原 local ref → `callBridge(methodRef, …)` 直接调用，**无需 Map 查表、无哈希开销**（与 Android 一致）。

### 4.3 窗口层级与容器模型

SOUI 的"HWND 树"直接映射为 ArkUI 组件树（而非画到一张全屏位图）：

| C++ 语义 | ArkTS 对应 | 职责 |
|---|---|---|
| `GetDesktopWindow()` 桌面 | `SouiScreen`（自定义组件） | 桌面级容器；挂在某 Ability 的页面布局里，作为所有顶级窗口的父；承载 screenId 启动参数与生命周期。见 [SouiScreen.ets](../soui-ohos-lib/src/main/ets/SouiScreen.ets) |
| 一个 HWND 窗口 | `SouiWindow`（基于 `SouiAbsLayout`） | 单 HWND 容器；index=0 是主 Surface，其余是子 HWND 子容器。 |
| HWND 的像素绘制面 | `SouiSurface`（基于 Canvas 自绘） | 实现 `INativeWindow`；`doRender` 调 C++ 渲染并把结果 writePixelsSync 到自身 PixelMap 再 drawImage 上屏。独立失效、独立绘制。见 [SouiSurface.ets](../soui-ohos-lib/src/main/ets/SouiSurface.ets) |
| 原生输入控件作为子控件 | `NativeEditView`（包装 ArkUI TextInput） | className 命中即创建原生输入控件，包装 INativeWindow，支持 SetDlgItemText/EnableWindow 等语义。 |
| 消息占位窗口（HWND_MESSAGE） | 占位 Surface | 不可见 0×0 占位窗口，用于定时器消息处理等。 |

`SouiAbsLayout`（见 [SouiAbsLayout.ets](../soui-ohos-lib/src/main/ets/SouiAbsLayout.ets)）提供绝对定位布局引擎，子组件位置/尺寸直接使用 SWinx RECT 语义，与 Android `SouiAbsLayout` 对应。

### 4.4 Surface 渲染（离屏 PixelMap + Skia + 脏矩形回传）

每个 `SouiSurface` 的 `doRender` 流程：

1. **尺寸同步**：Component 尺寸变化时，ArkTS 侧创建/复用离屏 `PixelMap`（RGBA_8888），并通知 C++ 更新 SWinx VirtualHWND Rect。
2. **C++ Skia 渲染**：`surf_render(pixelBuffer, w, h)` → `SouiSurfaceProxy::render` 把 PixelMap 的像素缓冲区绑定为 `CreateDIBSectionEx` 的 DIB Section，SOUI 完成当前窗口所有 DirectUI 控件绘制（Skin/文字/SVG/渐变），通过 `SendMessage(hWnd, WM_PAINT)` 写入缓冲。
3. **脏矩形回传（单一数据源）**：`render` 从 `GetUpdateRgn` 的包围盒（`GetRgnBox`，客户区物理像素坐标，WM_PAINT 之前读取）得到 `rcUpdate` 返回 ArkTS；`doRender` 仅对该矩形 `writePixelsSync` + `drawImage` 局部上屏，**不再由 TS 层维护脏矩形列表**。空矩形（l>=r）表示本次无需上屏。
4. **合成**：ArkUI 把该组件作为正常节点交给系统合成，可与原生 Toolbar/List 等组件交叉混合。


### 4.5 输入事件分发（对齐 SetCapture）

输入路径（见 [SouiSurface.ets](../soui-ohos-lib/src/main/ets/SouiSurface.ets) 触摸回调与 C++ [SouiSurfaceProxy.cpp](../soui-ohos-lib/src/main/cpp/src/SouiSurfaceProxy.cpp) `onMotionEvent`）：

1. ArkUI 按正常事件派发命中 `SouiSurface` 组件。
2. `SouiPlatformBridge` 先判断 `getCapture`：若已 SetCapture 且事件来源 Surface ≠ 捕获目标，则做坐标变换后重定向投递（与 Android 一致）。
3. 未命中 Capture 时，正常 `surf_onMotionEvent(action, x, y, pointerId, buttonState, vscroll, hscroll, metaState, ts)`。
4. C++ `onMotionEvent` 派发 `WM_LBUTTONDOWN/UP/DBLCLK/MOVE`、`WM_RBUTTONDOWN`、`WM_MOUSEWHEEL/WM_MOUSEHWHEEL`、`WM_XBUTTON*` 等，完全对齐 Win32 MSG。
5. `ACTION_UP/CANCEL` 后自动 `releaseCapture`（与 Win32 语义一致）。

键盘：`surf_onKeyEvent` → `onKeyEvent`，多模输入 `KeyCode`（HarmonyOS 值 2000+，区别于 Android 7-67）经 `convertKeyCode` 表映射到 Win32 `VK_*`，正确处理 ALT 修饰键生成 `WM_SYSKEYDOWN/UP`。

### 4.6 焦点与 IME

- `setFocus/getFocus` 双向同步（C++→ArkTS 焦点+IME；ArkTS→C++ `WM_SETFOCUS/KILLFOCUS`），与 Android 一致。
- IME 经 `inputMethod.InputMethodController`；原生 `NativeEditView` 自动弹键盘，SOUI 自绘输入控件依赖 ArkTS 把按键转字符，亦弹键盘。
- IME 字符经 **String Slot** 机制跨层传递（见 4.10）。

### 4.7 定时器：主线程快路径 + 工作线程跨线程回退

`SouiPlatformBridge` 用 `setInterval/clearInterval`（`TimerEntry`，repeat 语义对齐 Win32 `SetTimer`）。C++ 侧 `OhosPlatformAPI`：

- **先判线程**：`setTimer/killTimer/killWindowTimers` 开头 `if (isJsThread())` 走主线程快路径——直接 `callBridge` 调 ArkTS（与 `createWindow` 等同构），省去跨线程 `BridgeArg`/`BridgeTask` 构造与结果回传开销。
- **工作线程才跨线程**：`else` 分支走 `invokeBridge`，经 `napi_threadsafe_function` 把调用投递回 JS 线程执行（**根因：`napi_env` 只属于创建它的 JS 线程，工作线程直接 `napi_call_function` 会崩溃**）。`invokeBridge` 带同步超时（默认 `kBridgeWaitMs=2000ms`），避免 JS 线程阻塞时永久挂起工作线程。
- C++ 层 `m_timerEntries` 镜像 ArkTS 定时器状态，仅记录 `TIMERPROC` 回调；加锁记录保持在调用之后，避免持锁等待 JS 线程造成 `onTimerExpired` 死锁。

### 4.8 多 Ability / 多 Screen（screenId 机制）

与 Android 激活栈同构：`OhosPlatformAPI` 维护 `m_activeScreenStack`，`screenStartup` push / `screenShutdown` pop；创建窗口遇 `hWndParent=HWND_DESKTOP(0)` 从栈顶取当前 screenId。`SouiScreen` 在 `aboutToAppear` 调 `screenStartup`，`aboutToDisappear` 调 `screenShutdown`。

### 4.9 生命周期管理

- `UIAbility.onCreate` → `SouiPlatformBridge.init`：加载 `libsoui4ohos.so` 并 N-API 注册（对应 Android `JNI_OnLoad`）。
- `SouiScreen` 组件 `aboutToAppear` → `screenStartup` 创建 C++ 窗口系统；`aboutToDisappear` → `screenShutdown` 销毁。
- 尺寸变化：`onSizeChanged` → `moveWindow` + 更新屏幕尺寸。

### 4.10 String Slot / 剪贴板 / RawInput / 键盘高度

- **String Slot**：ID 范围 1~65535 环形复用（0 保留无效），RAII `AutoStringSlot`；与 Android 完全一致，用于 WM_SETTEXT 等字符串参数交换。
- **剪贴板**：`@ohos.pasteboard`（ClipboardManager 等价），`clipboardOpen/Close/GetData/SetData` 对齐 Win32。
- **RawInput**：OHOS 暂不枚举物理设备（返回 0），保持接口一致以兼容业务层。
- **键盘高度**：`inputMethod` 监听键盘高度变化 → `WM_KEYBOARD_HEIGHT` 通知根窗口调整布局。

### 4.11 附录：OHOS 核心文件索引

ArkTS 层（`soui-ohos-lib/src/main/ets/`）：

| 文件 | 角色 |
|---|---|
| [SouiPlatformBridge.ets](../soui-ohos-lib/src/main/ets/SouiPlatformBridge.ets) | 全局 N-API 桥单例：createWindow/View 工厂/SetCapture/焦点/定时器/screen 生命周期/剪贴板/键盘高度/字符串槽。 |
| [INativeWindow.ets](../soui-ohos-lib/src/main/ets/INativeWindow.ets) | HWND 真身接口；HWND = 实现该接口的 ArkTS 对象 napi_ref 指针。 |
| [SouiScreen.ets](../soui-ohos-lib/src/main/ets/SouiScreen.ets) | 桌面容器（GetDesktopWindow），screenId 启动、生命周期、尺寸同步。 |
| [SouiAbsLayout.ets](../soui-ohos-lib/src/main/ets/SouiAbsLayout.ets) | 绝对布局基类，提供 measure/layout + 子组件位置/尺寸更新。 |
| [SouiSurface.ets](../soui-ohos-lib/src/main/ets/SouiSurface.ets) | Canvas 自绘 Surface：doRender → surf_render → writePixelsSync + drawImage（局部脏矩形）。 |
| [SouiSurfaceView.ets](../soui-ohos-lib/src/main/ets/SouiSurfaceView.ets) | Surface 视图组件包装。 |
| [NativeEditView.ets](../soui-ohos-lib/src/main/ets/NativeEditView.ets) / [NativeEditViewView.ets](../soui-ohos-lib/src/main/ets/NativeEditViewView.ets) | 原生输入控件实现，处理 Win32 编辑控件消息，支持 IME。 |
| [NativeWindowDelegate.ets](../soui-ohos-lib/src/main/ets/NativeWindowDelegate.ets) | INativeWindow 通用委托实现。 |
| [AudioPlayer.ets](../soui-ohos-lib/src/main/ets/AudioPlayer.ets) | playSound 委托 ArkTS AudioPlayer。 |
| [UiThreadUtils.ets](../soui-ohos-lib/src/main/ets/UiThreadUtils.ets) | UI 线程工具类（对应 Android UiThreadUtils）。 |

C++ 层（`soui-ohos-lib/src/main/cpp/`）：

| 文件 | 角色 |
|---|---|
| [OhosPlatformAPI.h](../soui-ohos-lib/src/main/cpp/include/OhosPlatformAPI.h) / [.cpp](../soui-ohos-lib/src/main/cpp/src/OhosPlatformAPI.cpp) | SWinx OHOS 平台实现（N-API）；screen 生命周期激活栈；SetCapture/焦点/定时器（主线程快路径 + invokeBridge 跨线程）/窗口操作/字符串槽/剪贴板/RawInput/键盘高度/HWND_MESSAGE。 |
| [OhosPlatformAPIReg.cpp](../soui-ohos-lib/src/main/cpp/src/OhosPlatformAPIReg.cpp) | PlatformAPI 注册（RegisterOhosPlatformAPI）。 |
| [Soui4Ohos_NAPI.cpp](../soui-ohos-lib/src/main/cpp/src/Soui4Ohos_NAPI.cpp) | N-API 入口：surf_render / surf_onMotionEvent / surf_onKeyEvent 等；napi_module_register 替代 JNI_OnLoad。 |
| [SouiSurfaceProxy.h](../soui-ohos-lib/src/main/cpp/src/SouiSurfaceProxy.h) / [.cpp](../soui-ohos-lib/src/main/cpp/src/SouiSurfaceProxy.cpp) | 每个 Surface 的 native 对应：Motion 事件 → Win32 MSG（双击/右键/滚轮/悬停）；键盘事件（KeyCode→VK_*）；render 离屏 PixelMap + Skia + BGRA→RGBA + rcUpdate 脏矩形。 |
| [OhosBridge.cpp](../soui-ohos-lib/src/main/cpp/src/OhosBridge.cpp) | N-API 桥接辅助（弱符号链接 OH NDK）。 |
| [ohos_napi_bridge.h](../soui-ohos-lib/src/main/cpp/ohos_napi_bridge.h) / [ohos_ime_bridge.h](../soui-ohos-lib/src/main/cpp/ohos_ime_bridge.h) | N-API / IME 桥接头文件。 |
| [CMakeLists.txt](../soui-ohos-lib/src/main/cpp/CMakeLists.txt) | 单体 so 构建脚本。 |

---

## 5. SOUI for iOS 适配方案

### 5.1 架构总览（UIKit + Core Graphics + Objective-C++）

iOS 适配位于 `swinx/src/platform/ios`，与 macOS cocoa 分支同构（UIKit 取代 AppKit）。它不依赖 JNI/N-API 桥，而是用 **Objective-C++（.mm）** 直接调用 UIKit/Foundation，HWND 即 `SUIView`（UIView 子类）的桥接指针。

```
┌──────────────────────────────────────────────────────────────┐
│  iOS 原生（UIApplication / UIWindow / UIView）                │
│  ┌──────────────┐  ┌──────────────────────────────────────┐  │
│  │ UIApplication │  │ SUIView（UIView 子类，1 HWND = 1 View） │ │
│  │ Delegate      │  │  drawRect → CGContextRef → SOUI 绘制   │ │
│  └──────┬───────┘  └───────────────┬──────────────────────┘  │
├──────────┼────────────────────────────────────────────────────┤
│  swinx SConnection（Objective-C++）：消息循环 + 窗口管理        │
│  ├─ OnDrawRect(HWND, RECT, CGContextRef) 渲染提交              │
│  ├─ OnNsEvent → WM_LBUTTON*/WM_TIMER/WM_SETFOCUS…              │
│  ├─ TimerInfo 链表（消息循环 tick 驱动 SetTimer）             │
│  ├─ CFRunLoopSourceRef 唤醒（postMsg → signal wake source）   │
│  └─ SClipboard(UIPasteboard) / STrayIconMgr / SetCapture      │
├──────────────────────────────────────────────────────────────┤
│  SOUI Core（SWindow/SNativeWnd/SWnd/Skin/Xml 资源）—— 业务层   │
│  C++ 代码通过 swinx_ios_entry() 接入，几乎零改动               │
└──────────────────────────────────────────────────────────────┘
```

**入口机制**（`ios_main.mm`）：`UIApplicationMain` 是阻塞调用且永不返回，因此 `SwinxAppDelegate.didFinishLaunchingWithOptions` 用 `dispatch_async(dispatch_get_main_queue(), …)` 异步调用宿主 `_tWinMain`；`_tWinMain` 内的 `GetMessage` 循环通过 `CFRunLoopRunInMode` 运行主线程 runloop，从而接收 UIKit 触摸事件。对外暴露 C API `swinx_ios_entry(argc, argv, funIosMain)`，应用层纯 C++ 通过 `ios_entry.h` 调用。

### 5.2 HWND = SUIView 指针：零查表映射

- `createUiWindow` 创建 `SUIView`（UIView 子类）并登记；**其对象指针即 HWND**（与 Android jobject GlobalRef、OHOS napi_ref 同思路：零 Map 查表）。
- `getHwndFromUiView(view)` 反向还原 HWND。
- 窗口状态操作（`showUiWindow / setUiWindowPos / setUiWindowSize / invalidateUiWindow / setUiWindowCapture …`）直接操作 UIView，见 [SUIWindow.h](../swinx/src/platform/ios/SUIWindow.h)。

### 5.3 窗口层级（UIView/UIWindow 树）

每个 HWND = 一个 `SUIView`，作为 iOS 原生视图层级的节点，独立 measure/layout/失效/合成（与 Android View 树思路一致）。系统把各 UIView 作为独立 Layer 交给渲染合成，可与原生 UITableView/UIButton 等交叉混合。

### 5.4 渲染（Core Graphics）

- `SConnection::OnDrawRect(HWND, const RECT &rc, CGContextRef ctx)` 由 UIView 的 `drawRect:` 触发；SOUI 把当前窗口所有 DirectUI 控件绘制进该 `CGContextRef`（Core Graphics 渲染后端），`commitCanvas/updateWindow` 提交。
- 颜色空间：iOS 使用设备 RGBA/广色域，SOUI 绘制后端做相应映射。
- 每个 HWND 独立 UIView、独立失效、独立 `setNeedsDisplay`，系统正常合成。

### 5.5 消息循环与 UIKit 集成

- `GetMessage` 循环经由 `CFRunLoopRunInMode` 运行在主线程 runloop；触摸/键盘由 UIResponder 体系转发到 `SUIView`，再经 `OnNsEvent` 投递 Win32 MSG。
- **唤醒机制**：`SConnection` 持有 `CFRunLoopSourceRef m_wakeSource` 与 `m_wakeRunLoop`；`postMsg` 后 signal 该 source 唤醒主 runloop（跨线程/异步消息驱动），见 [SConnection.h](../swinx/src/platform/ios/SConnection.h)。

### 5.6 输入与 SetCapture / IME

- `SUIView` 触摸事件 → `OnNsEvent` → `WM_LBUTTONDOWN/UP/MOVE/DBLCLK`、`WM_RBUTTONDOWN`、`WM_MOUSEWHEEL/HWHEEL`、`WM_XBUTTON*`；`SetCapture/ReleaseCapture` 经 `setUiWindowCapture/releaseUiWindowCapture`。
- 键盘/IME：`keyboard.mm` / `imm.mm` 处理 `UIKeyInput`/`UIResponder` 转发与软键盘：`showUiSoftKeyboard(bShow)`、`isUiSoftKeyboardVisible()`、`getUiSoftKeyboardHeight()`（iOS 无直接 API，靠全局标记 + 通知维护）。

### 5.7 定时器（消息循环直接驱动）

- `SConnection` 内部 `std::list<TimerInfo> m_lstTimer`，每条含 `id/hWnd/elapse/fireRemain/proc`；随消息循环 tick 递减 `fireRemain`，到 0 触发 `SetTimer` 回调（repeat 语义，与 Win32 一致）。
- 与 Android（`Handler`）、OHOS（`setInterval`）不同：**iOS 定时器由 C++ 消息循环直接驱动，无独立 JS/ArkTS 调度层**，因此天然无跨线程桥接开销（见 §2.4）。

### 5.8 剪贴板 / 拖放 / 托盘 / 焦点

- `SClipboard`（见 [SClipboard.h](../swinx/src/platform/ios/SClipboard.h)）封装 `UIPasteboard`，采用 Win32 OLE `IDataObject/IDropSource` 风格接口（`GetData/SetData/EnumFormatEtc`）。
- `DoDragDrop` 经 UIKit 拖放（`EnableDragDrop`）；`STrayIconMgr` 管理（iOS 无系统托盘，降级/忽略）。
- 焦点/激活窗口：`SetActiveWindow/SetForegroundWindow/BringWindowToTop` 映射到 `UIWindow` 层级（`setUiActiveWindow/getUiActiveWindow`）。

### 5.9 生命周期

- `UIApplicationDelegate` 接收前后台切换（`OnNsActive`）；与 SOUI screen 创建/销毁绑定。
- 窗口创建/销毁：`OnWindowCreate/OnWindowDestroy` 维护 HWND 与 `SUIView` 映射。

### 5.10 构建与示例客户端

iOS 与桌面（Windows / macOS）共用**同一份业务代码**，由 CMake 按目标平台切换入口：

- **构建**：主 `CMakeLists.txt` 在 `CMAKE_SYSTEM_NAME MATCHES "iOS"` 时设置`add_definitions(-D__IOS__)`；swinx 平台层 `swinx/src/platform/ios` 全部以 Objective-C++ 编入。iOS / macOS 走 Apple bundle，资源经 `add_macos_res_folder` 拷入 `.app`（iOS 与 Android 共享同一套移动版 `uires`，见 `games/cnchess/client/CMakeLists.txt`）。
- **示例客户端 `games/cnchess/client`**：这是一份同时支持 **Windows / macOS / iOS / Android / OHOS** 的跨端业务工程，无需为 iOS 单独建壳：
  - `main.cc` 的统一入口：`_tWinMain` 是业务主函数；`#if defined(__IOS__)` 分支下 `int main` 直接调用 `swinx_ios_entry(argc, argv, _tWinMain)` 接入 iOS 平台层；非 Windows 桌面（macOS）则 `int main` → `_tWinMain`。
  - `android_entry.cc` / `ohos_entry.cc` 仅因 Android（JNI）、OHOS（N-API）需要在桥接模块内注册入口而单独存在；**iOS 不需要独立的 `ios_entry.cc`——`main.cc` 已通过 `swinx_ios_entry()` 直接接管**。
  - 资源路径：iOS 下 `getResourceDir()` 走 `GetAppleBundlePath` 取 `.app` 内资源；`MainDlg.cpp` 对 `pane_ios_header` 等 iOS 专属 UI 做平台可见性控制。
- 业务层通过 `swinx_ios_entry()` 接入 `swinx/src/platform/ios`，复用同一套 SWinx C++ 业务代码。

### 5.11 附录：iOS 核心文件索引

`swinx/src/platform/ios/`（Objective-C++）：

| 文件 | 角色 |
|---|---|
| [SConnection.h](../swinx/src/platform/ios/SConnection.h) / [SConnection.mm](../swinx/src/platform/ios/SConnection.mm) | 平台连接：消息循环（CFRunLoop 唤醒）、窗口管理、定时器、SetCapture、剪贴板、RawInput、Caret。 |
| [SUIWindow.h](../swinx/src/platform/ios/SUIWindow.h) / [SUIWindow.mm](../swinx/src/platform/ios/SUIWindow.mm) | UIView/UIWindow 封装，对应 macOS SNsWindow；HWND = SUIView 指针。 |
| [ios_main.mm](../swinx/src/platform/ios/ios_main.mm) | iOS 应用入口：AppDelegate + dispatch_async 调 _tWinMain + CFRunLoopRunInMode 驱动消息循环；暴露 swinx_ios_entry()。 |
| [SClipboard.h](../swinx/src/platform/ios/SClipboard.h) / [SClipboard.mm](../swinx/src/platform/ios/SClipboard.mm) | 剪贴板：UIPasteboard + IDataObject OLE 风格接口。 |
| [keyboard.h](../swinx/src/platform/ios/keyboard.h) / [keyboard.mm](../swinx/src/platform/ios/keyboard.mm) | 软键盘显隐与高度维护。 |
| [imm.mm](../swinx/src/platform/ios/imm.mm) | IME 输入法集成（UIKeyInput/UIResponder）。 |
| [ole2.mm](../swinx/src/platform/ios/ole2.mm) | OLE/拖放数据对象支持。 |
| [os_state.h](../swinx/src/platform/ios/os_state.h) / [os_state.mm](../swinx/src/platform/ios/os_state.mm) | 系统状态（DPI/屏幕/工作区）。 |
| [STrayIconMgr.h](../swinx/src/platform/ios/STrayIconMgr.h) / [STrayIconMgr.mm](../swinx/src/platform/ios/STrayIconMgr.mm) | 托盘图标管理（iOS 降级）。 |
| [SUIDataObjectProxy.h](../swinx/src/platform/ios/SUIDataObjectProxy.h) / [SUIDataObjectProxy.mm](../swinx/src/platform/ios/SUIDataObjectProxy.mm) | 拖放数据对象代理。 |
| [atoms.h](../swinx/src/platform/ios/atoms.h) / [atoms.mm](../swinx/src/platform/ios/atoms.mm) | 原子/注册消息。 |
| [utils.mm](../swinx/src/platform/ios/utils.mm) / [dlghelper.mm](../swinx/src/platform/ios/dlghelper.mm) | 平台工具与对话框辅助。 |
| [ios_entry.h](../swinx/include/ios_entry.h) | 应用层 iOS 入口函数原型（与 Win32 WinMain 一致）。 |

---

## 6. 与 Qt / Flutter 方案对比

### 6.1 Qt for Android 架构要点

Qt 传统 Android 适配（Android QPA 插件）：<sup>[\[1\]](https://doc-snapshot.qt-project.org/qt6-dev/android-how-it-works.html)</sup>

- **双线程模型**：`QtThread`（执行 `main()`、加载 Qt 库、跑 Qt 事件循环、执行所有 QML/C++ 渲染）独立于 Android UI Thread。任何需要改 View 的操作都需要跨线程 post，`runOnAndroidMainThread` 封装调度。
- **Qt 掌控 Activity 生命周期**：默认 Activity 必须继承 `QtActivity`；`onCreate` 内加载所有 Qt .so 并启动 QtThread，之后 Qt 通过内部 `QtLayout`、渲染 Surface、输入代理把 QML/Qt Widget 结果呈现。
- **单 Activity 单顶层窗口** 传统模式；Qt 6.7+ "Novel Approach" 才提供把 QML 作为 `QtQuickView`（本质一个 `FrameLayout + Surface + JNI 双向绑定`）嵌入到已有 Android App 的能力，但仍要求 QML 场景由 Qt Quick 管理，控件与原生控件不混排。<sup>[\[2\]](https://www.qt.io/blog/qt-for-android-a-novel-approach)</sup>
- **渲染路径**：Qt Quick 场景栅格化到 OpenGL FBO 或 `ANativeWindow`（QAndroidSurfaceRenderEngine 方式：headless QQuickWindow → 纹理转发到多个 Surface View）<sup>[\[3\]](https://doc.qt.io/QtAndroidAutomotive/qandroidsurfacerenderengine.html)</sup>，不是原生 View 粒度。

### 6.2 Flutter for Android 架构要点

- **三层架构**：Dart Framework（Widget/Element/RenderObject）、Dart VM（AOT 编译 Release）、Engine（C++：Skia 2024→Impeller 图形、HarfBuzz 文本、Dart Runtime）<sup>[\[4\]](https://blog.csdn.net/2302_80693444/article/details/155980501)</sup>。
- **自绘引擎**：Flutter 不把 Widget 映射成原生 View，而是通过 Skia/Impeller 把整棵 RenderObject 树画到**一个或少数几个全屏 FlutterView Surface** 上，SurfaceFlinger 只把它当一个 Layer 合成<sup>[\[5\]](https://juejin.cn/post/7572480736362266630)</sup>。
- **与原生互操作靠 Platform View**：`AndroidView` / `PlatformViewLink`（Surface 级别）把原生控件"贴"到 Flutter 渲染结果上；本质是两个独立 Surface 的合成，Z-order、裁剪、手势消歧都有历史兼容性坑，且列表内复用成本高。
- **独立线程池**：UI 线程（Dart 构建 Widget/Layout）、Raster 线程（Skia/Impeller 提交 GL/Vulkan）、IO 线程（图片解码、字体加载）都独立跑，与主线程通过 Port Channel 通信。

### 6.3 三维对比总表

| 维度 | SOUI for Android | Qt for Android (QPA) | Flutter for Android |
|---|---|---|---|
| **运行线程** | 完全共用 Android 主线程；JNI 零跨线程调度。 | QtThread（事件循环+渲染）独立于 UI Thread；需 `runOnAndroidMainThread` 桥。 | UI/Raster/IO 三线程池 + Platform Channel。 |
| **窗口/渲染粒度** | 每个 HWND = 一个独立 Android View；独立测量、独立绘制、独立失效；系统合成。 | 整 QML 场景 → FBO 纹理，再贴到若干 SurfaceView/ANativeWindow 上。 | 整个 App UI 画到 1~几张 FlutterView Surface 上；原生控件需 Platform View 特例。 |
| **HWND 模型** | HWND 就是 jobject GlobalRef；C++ 直接调 Java 接口；零 Map 查表。 | QWindow/WId 抽象；内部通过 QAndroidJniObject 反射访问。 | 无原生句柄概念；Dart Widget 与底层 View 无一一对应。 |
| **原生 View 嵌入 SOUI** | `registerViewFactory("edit", NativeEditView::new)`：原生 View 作为 SOUI 子控件，获同等 HWND 句柄，moveWindow/enableWindow/SetDlgItemText 语义齐全。 | QWidget/Qt Quick Item 内嵌入原生 View 需自定义 QQuickPaintedItem + JNI；非一等公民。 | 需写 Platform Channel + 自定义 PlatformViewFactory；两个 Surface 合成。 |
| **SOUI 嵌入原生布局** | SouiScreen 是标准 ViewGroup 子类，可写在任意 XML（FrameLayout/LinearLayout/CoordinatorLayout…）里，XML 属性即可启动。 | 传统 Qt：App 必须是 QtActivity；Novel Approach：提供 QtQuickView 嵌入但需额外 Gradle 插件 + AAR 部署步骤。 | FlutterFragment / FlutterView 嵌入；预热引擎/引擎缓存等机制较繁琐。 |
| **业务代码移植** | SWinx C++ 代码几乎零改动；SetCapture/SetTimer/CreateWindowEx 语义一致。 | 需要把 UI 改写为 QWidget 或 QML；Win32 UI 逻辑不兼容。 | 整 UI 需用 Dart+Widget 重写；C++ 仅能作 FFI 逻辑层。 |
| **包体积增量** | 仅 SOUI Core + SWinx + Skia 依赖（≈3-5MB/ABI，视启用 SVG/RichEdit 等组件）。 | QtCore/QtGui/QtQml/QtQuick + 插件（单 ABI≈15-30MB+）。 | Flutter Engine AOT 运行时（单 ABI≈6-10MB）。 |
| **语言栈** | C++(业务+UI)+Java/Kotlin(原生宿主+部分控件) | C++/QML + Java/Kotlin(Activity 壳) | Dart + Kotlin/Swift(宿主+插件互操作) |
| **SetCapture/拖拽语义** | Java 层显式实现 + ACTION_UP 自动释放；与 Win32 行为一致。 | Qt Quick DragHandler 自定义实现；非 Win32 语义对齐。 | GestureRecognizer 自实现；无 Win32 映射。 |
| **焦点管理** | 双向同步（C++→Java 焦点+IME；Java→C++ WM_SETFOCUS）；Win32 语义一致。 | Qt 自有焦点系统；与 Android View 焦点独立。 | Flutter 自有焦点系统；与 Android View 焦点独立。 |
| **字符串参数传递** | String Slot 机制（RAII 管理，槽 ID 环形复用）；编码转换统一处理。 | JNI 字符串直接传递；需手动管理引用。 | Platform Channel 序列化；JSON/MessageCodec 开销。 |

---

## 7. 核心优势：与原生开发无缝集成

"**无缝**"在这里不是宣传语，而是三个具体事实的总和（以下以 Android 为例说明，OHOS / iOS 同理）。

### 7.1 原生 View 可直接成为 SOUI 的一个"子控件"

`SouiPlatformBridge.registerViewFactory(className, Function<String,View>)`：C++ 层 `CreateWindow("edit")` 会走到 Java 查表 → 创建 `NativeEditView`（继承 `AppCompatEditText` + 包装 `INativeWindow`）→ 返回 HWND。从此：

- C++ 层 `SetDlgItemText(hDlg, ID_EDIT, L"xxx")` → Java `SetText`；
- `EnableWindow(GetDlgItem(hDlg, ID_EDIT), FALSE)` → Java `setEnabled(false)`；
- 用户在 EditText 里的触摸/输入，Android 系统按原生焦点/输入法正常派；如需把事件再交 SOUI，也只需要该包装 View 的监听器里做 JNI 回调。

这是 Qt/Flutter 做不到或做起来代价极高的点：**SOUI 的子控件是"原生控件真身"，不是画出来的外观仿真**。对需要调用 Android 系统原生键盘、文字选择、辅助服务（TalkBack）、厂商对 Edit 的特殊优化等场景，天然 100% 兼容。

### 7.2 SouiScreen 能嵌入**任意**原生 ViewGroup

`SouiScreen extends SouiWindow extends SouiAbsWindow extends SouiAbsLayout extends ViewGroup`。你可以：

- 在 `FrameLayout` 里一半放 `Toolbar + SouiScreen`，另一半放原生 `BottomNavigationView`；
- 在 `CoordinatorLayout` 里让 SouiScreen 配合 `AppBarLayout` 折叠；
- 在 `RecyclerView` item 里嵌一个小型 SouiScreen（当然通常不建议，但技术上完全可行）。

`SouiAbsLayout.LayoutParams extends MarginLayoutParams` 这一行保证被外层标准容器 `measureChildWithMargins` 调用时无 ClassCastException。

### 7.3 XML 一行声明即可启动 SOUI

支持通过 `attrs.xml` 自定义属性（见 [attrs.xml](../demos/android-demo/app/src/main/res/values/attrs.xml) 的 `SouiScreen_screenId` 与 `SouiScreen_souiLayout`），在 XML 中直接：

```xml
<FrameLayout ...>
  <com.soui.SouiScreen
    xmlns:app="http://schemas.android.com/apk/res-auto"
    android:id="@+id/soui_screen"
    android:layout_width="match_parent"
    android:layout_height="match_parent"
    app:screenId="0x0000D002"
    app:souiLayout="@string/layout_demo" />
</FrameLayout>
```

`SouiScreen.Init` 在构造阶段读取属性 → `onAttachedToWindow` 自动调用 `screenStartup`。Activity 的 `onCreate` 只需一行 `setContentView(R.layout.activity_xml_host)`（见 [XmlHostActivity.java](../demos/android-demo/app/src/main/java/com/soui/demo/XmlHostActivity.java)），**零 SOUI 相关样板代码**。

### 7.4 JNI 双向调用天然友好：C++ 直接调 Android 原生 API 示例

Demo 的 `CMainDlg::ShowToastAndroid`（见 [MainDlg.cpp](../demos/android-demo/app/src/main/cpp/MainDlg.cpp#L115-L157)）是典型例子：

```cpp
void CMainDlg::ShowToastAndroid(const std::wstring& text) {
    JNIEnv* env = AndroidPlatformAPI::instance().getJNIEnv();
    jobject ctx = AndroidPlatformAPI::instance().getApplicationContext();
    // 把 wstring 转 UTF-8 → NewStringUTF
    SStringA utf8 = S_CW2A(text.c_str(), CP_UTF8);
    jstring jText = env->NewStringUTF(utf8.c_str());
    // FindClass + GetStaticMethodID("android/widget/Toast", "makeText", ...) + CallObjectMethod + show()
    // 标准 JNI 调用，无需 Channel / QAndroidJniObject 封装；运行在主线程零线程切换
}
```

因为 C++ 和 Java UI **共用同一线程**，任何 JNI 回调、任何系统服务（WindowManager / NotificationManager / MediaCodec / ContentResolver）都可以像普通 Android App 一样同步调用，不需要"跨线程消息队列 + Future 等待"模式。

### 7.5 NativeWindowDelegate：窗口状态操作的集中委托

`NativeWindowDelegate` 提供了 `INativeWindow` 接口的公共实现，让不同基类的 Surface View 共享同一套行为：

- **目标解析**：自动识别父容器是 `SouiAbsLayout` 还是普通 `ViewGroup`，选择正确的操作目标
- **操作委托**：将 `nativeDestroy`、`nativeShow`、`nativeMove` 等操作委托给父容器或 Surface 自身
- **特殊情况处理**：支持 message-only 窗口（无父容器的 0x0 占位窗口）的优雅处理

---

## 8. Demo 编译与测试

### 8.1 Android（demos/android-demo）

Demo 工程位于：`demos/android-demo/`。

**环境要求**：

| 组件 | 版本要求 | 说明 |
|---|---|---|
| Android Studio | Hedgehog (2023.1) 及以上 | AGP 7.4 / 8.x 均可。 |
| Android SDK | compileSdk 34，minSdk 28 | 见 [app/build.gradle](../demos/android-demo/app/build.gradle)。 |
| Android NDK | 与 AGP 匹配的任意 LTS（如 25.x / 26.x） | Gradle 会通过 `local.properties` 里的 `ndk.dir` 或 SDK Manager 默认安装目录自动定位。 |
| CMake | ≥ 3.18.1 | SDK Manager → SDK Tools 安装。 |
| JDK | 17（运行 Gradle）；代码兼容 Java 8。 | Gradle 项目属性配置。 |
| 硬件/模拟器 | armeabi-v7a / arm64-v8a / x86 / x86_64 任一种 | `ndk.abiFilters` 四者都编；按需裁剪。 |
| Git 子模块（如有） | 确保 `soui4/` 下所有子模块已初始化 | 若 SOUI 目录缺 `third-part/` 或 `components/` 内容，先 `git submodule update --init --recursive`。 |

**编译步骤**

**方式 A：Android Studio（推荐）**

1. 菜单 File → Open，选择目录 `d:\work\soui4\demos\android-demo`（含根 `settings.gradle`）。
2. 首次打开 Gradle Sync 会自动：
   - 下载 `androidx.appcompat`、`material`、`constraintlayout` 等 Maven 依赖；
   - 调用 `app:configureCMakeDebug` → CMake 配置阶段输出 `SOUI_ROOT_DIR`、`SOUI Source`、`swinx` 等摘要（见 CMakeLists.txt message）。
3. Gradle Sync 成功后，Build → Make Project（或 Ctrl+F9）。
4. 运行（Shift+F10）：选择连接的 Android 设备或启动一个模拟器（API ≥ 28）。
5. 产生的 APK：`demos/android-demo/app/build/outputs/apk/debug/app-debug.apk`，内含 4 个 ABI 的 `libsoui-android-demo.so` + assets 中的 SOUI uires 资源。

**方式 B：命令行**（适用于 CI/CD）

```powershell
# Windows PowerShell 下
cd d:\work\soui4\demos\android-demo
.\gradlew.bat assembleDebug                 # 产出 debug APK
# .\gradlew.bat assembleRelease             # 如果签名配置好了（默认 release 未启用 minify）
# .\gradlew.bat :app:externalNativeBuildDebug  # 仅重编 C++
```

**C++ 编译说明（重要）**：Demo 的 CMake 不把 SOUI 当预建 AAR 依赖，而是**直接 `add_subdirectory`** 了 `SWINX_DIR`、`UTILITIES_DIR`、`SOUI_SOURCE_DIR`、`THIRD_PART_DIR`、`COMPONENTS_DIR` 五个子目录（见 [CMakeLists.txt](../demos/android-demo/app/src/main/cpp/CMakeLists.txt#L101-L112)），全部与 demo 业务代码 (`AndroidPlatformAPI.cpp / SouiSurfaceProxy.cpp / MainDlg.cpp / demo_native.cpp`) 编译进单体 `libsoui-android-demo.so`。好处是便于在 Android Studio 里单步调试 C++ 层。生产 SDK 形式可以拆分：SOUI 核心 → AAR（`libsoui-android.so`），业务层单独 link。

**C++ 入口方式**：采用 `Soui4AndroidEntry` 抽象类 + 虚函数调用模式。用户实现 `Soui4AndroidEntry` 的四个虚函数（`InitApp`、`UninitApp`、`ScreenStartup`、`ScreenShutdown`），通过全局静态变量注册到系统。`AndroidPlatformAPI` 内部通过 `s_entry` 指针调用这些虚函数，实现平台层与业务层的解耦。见 [demo_native.cpp](../demos/android-demo/app/src/main/cpp/demo_native.cpp)。

**Demo 功能结构**

启动 App 后进入 `HomeActivity`（见 [HomeActivity.java](../demos/android-demo/app/src/main/java/com/soui/demo/HomeActivity.java)），提供两个按钮对应两种启动方式：

**入口 A：DefaultHostActivity（代码方式）**
- `SCREEN_ID = 0x0000_D001L`，`LAYOUT_DEMO = "layout:dlg_main"`（对应 assets/uires/xml/dlg_main.xml）。
- `mScreen = new SouiScreen(this)` → `setScreenId / setSouiLayout` → `FrameLayout.addView(mScreen, MATCH_PARENT)` → `onAttachedToWindow` 自动调用 `screenStartup` → C++ 调 `SHostWnd::Create(NULL)` 创建主窗口。
- `setOnEmptyListener`：当所有子 SOUI 窗口都被销毁（只剩 Screen 自己的 Surface）时 `finish()` Activity，对应"关闭最后一个窗口自动退出"语义。

**入口 B：XmlHostActivity（XML 声明方式）**
- `setContentView(R.layout.activity_xml_host)` 仅此一行；
- 布局 XML 内 `<com.soui.SouiScreen app:screenId=... app:souiLayout=... />` 已声明参数，Attach 时自动启动。

两个入口最终渲染的都是同一个 C++ `CMainDlg`（基于 dlg_main.xml）。CMainDlg 界面与功能：

| 控件 (ID) | 功能 |
|---|---|
| 标题栏 (默认 SOUI sys 按钮) | 关闭按钮触发 `OnClose` → `DestroyWindow` → Screen onEmptyListener → finish |
| 1001 按钮（"点击计数 + Toast"） | `m_clickCount++`；更新 info 文本；JNI 调 Android 原生 Toast.makeText().show()（中文提示）。 |
| 1002 按钮（"切换主题色"） | info 文本颜色在 5 套 COLORREF 中循环：深灰 / 品红 / 绿 / 蓝 / 橙。 |
| 1003 按钮（"关闭窗口"） | 直接 `OnClose`。 |
| info 文本（`kInfoName` 命名 SStatic） | 每秒由 `WM_TIMER(kTimerIdTick)` 更新：`点击次数: N | 运行时长: Ms`。 |

**测试要点清单**

以下是回归该适配方案核心功能的最小必过测试集合：

1. **布局零崩溃**：两种入口启动后，按返回键退出，再重新进入 10 次 → 无 ClassCastException / 零尺寸 / Surface nativeId=0。
2. **尺寸同步正确性**：
   - 旋转屏幕或折叠屏开合，Activity 重建后重新创建 SOUI 窗口系统，主窗口跟随新尺寸；
   - info 文本不被裁切；Toast 出现无黑边。
3. **点击计数 + 原生 Toast**：
   - 点按钮 1001，N 递增；Android Toast 正常弹，中文不乱码（验证 S_CW2A UTF-8 + NewStringUTF 链路）。
4. **主题色切换**：点 1002 五次，颜色循环回到首色；每次切换立即生效（验证 `SetAttribute("colorText")` → 局部 Invalidate → Skia 局部绘制）。
5. **定时器**：info 文本里的"运行时长"每秒 +1，不跳秒不暂停（验证 `SetTimer → Handler.postDelayed → nativeOnTimerExpired → WM_TIMER` 全链路）。
6. **关闭 / 窗口销毁**：
   - 点 1003 或标题栏"×"，Activity 立即 finish（验证 Screen 下子窗口全销毁 → onEmpty → finish 流程）。
   - 再返回 Home，按"最近任务"卡片划掉 App → 无残留 GlobalRef（logcat 搜索 `SouiPlatformBridge.destroy`：定时器/SetCapture/mViewMap 全部清空）。
7. **SetCapture / 拖拽模拟**：若后续接入滚动条或标题栏拖拽控件，验证：
   - DOWN 后手指滑出控件范围，该控件仍能收到 MOVE；
   - UP 后下次 DOWN 在其他控件，事件正确命中新控件（自动 ReleaseCapture 生效）。
8. **多 Activity 独立**：在启动 DefaultHostActivity 之后按 Home，再从桌面图标重新进入 → 新 Activity 用新 screenId；之前 Activity 返回键退出不影响当前（验证 `pushActiveScreen / popActiveScreen` 不串台）。
9. **焦点与 IME**：
   - 点击 NativeEditView，自动弹出软键盘；
   - 点击其他控件，键盘自动收起；
   - 输入文本正确显示；
   - 键盘高度变化时布局正确调整（API ≥ 30）。
10. **字符串 Slot 机制**：
    - `SetDlgItemText` 正常设置文本；
    - `GetDlgItemText` 正常读取文本；
    - 多次操作后无内存泄漏。
11. **剪贴板操作**：
    - `SetClipboardText` 设置文本后，系统剪贴板能读取到；
    - `GetClipboardText` 能读取系统剪贴板内容；
    - 多次操作后无内存泄漏。
12. **RawInput 设备枚举**：
    - 连接外部键盘/鼠标后，`getRawInputDeviceList` 能正确返回设备列表；
    - 设备类型映射正确（键盘→RIM_TYPEKEYBOARD，鼠标→RIM_TYPEMOUSE）。
13. **HWND_MESSAGE 窗口**：
    - 创建消息占位窗口成功，不影响其他窗口布局；
    - 定时器消息能正确投递到消息窗口。
14. **双击检测**：快速连续点击同一位置，触发 `WM_LBUTTONDBLCLK` 消息。
15. **右键菜单**：长按弹出上下文菜单，触发 `WM_RBUTTONDOWN` 消息。
16. **滚轮事件**：外接鼠标滚轮操作，触发 `WM_MOUSEWHEEL`/`WM_MOUSEHWHEEL` 消息。

### 8.2 OHOS（games/cnchess/client/ohos）

适配模块：`soui-ohos-lib/`（ArkTS + C++ N-API，被业务工程 `add_subdirectory` 编入单体 `libsoui4ohos.so`）。示例工程：`games/cnchess/client/ohos`（基于 `ohos_entry.cc` 实现 `Soui4OhosEntry` 的 `InitApp/UninitApp/ScreenStartup/ScreenShutdown`）。

**环境要求**：

- DevEco Studio + OHOS SDK（API 匹配）；hvigor 构建。
- CMake 交叉编译（OHOS clang），启用 SVG/RichEdit。
- 联网权限：`module.json5` 声明 `ohos.permission.INTERNET`（normal/system_grant，安装时授予；变更需卸载重装生效）。
- 命令行构建示例（本机）：
  ```powershell
  env -u NODE_OPTIONS DEVECO_SDK_HOME="C:/Program Files/Huawei/DevEco Studio/sdk" `
    "C:/Program Files/Huawei/DevEco Studio/tools/node/node.exe" hvigorw.js assembleHap
  ```

**测试要点清单**（核心回归集合）：

1. 两种入口启动后退出/重进多次无崩溃、无零尺寸、无 nativeId=0。
2. 尺寸同步：页面旋转/分屏后主窗口跟随新尺寸。
3. 点击计数 + 原生 Toast（通过 C++→ArkTS 调用链）。
4. 主题色切换局部重绘生效（`SetAttribute` → Invalidate → 脏矩形局部上屏）。
5. 定时器：运行时长每秒 +1（验证 `SetTimer → setInterval → onTimerExpired → WM_TIMER`）。
6. 关闭/窗口销毁：子窗口全销毁 → finish Ability。
7. SetCapture/拖拽：DOWN 后滑出控件仍收 MOVE；UP 后自动释放。
8. 跨线程：在工作线程创建/销毁定时器不崩溃（验证 invokeBridge tsfn 路径）。
9. 焦点/IME：点 NativeEditView 弹键盘、输入正确、键盘高度调整布局。
10. 剪贴板：SetClipboardText/GetClipboardText 正常。
11. 多 Ability：返回桌面再进新 Ability 用新 screenId 不串台。

### 8.3 iOS / 桌面（games/cnchess/client）

`games/cnchess/client` 是同一份业务代码，覆盖 **iOS / macOS / Windows / Android / OHOS** 五端（见 §5.10）。iOS 与桌面（Windows/macOS）共用 `main.cc` 入口，无需独立壳工程：

- **iOS**：`CMAKE_SYSTEM_NAME=iOS` 时定义 `__IOS__`，`main.cc` 的 `int main` 调 `swinx_ios_entry(argc, argv, _tWinMain)`（见 [main.cc](../games/cnchess/client/main.cc)）。资源经 CMake `add_macos_res_folder` 拷入 `.app`；iOS 与 Android 共享同一套移动版 `uires`（见 [CMakeLists.txt](../games/cnchess/client/CMakeLists.txt)）。
- **Windows / macOS**：非 iOS 时 `int main` → `_tWinMain`；Windows 额外链接 `win32_audio`，Apple 平台走 bundle + 签名。

**构建要点**：

- iOS：主 `CMakeLists.txt` 在 `CMAKE_SYSTEM_NAME MATCHES "iOS"` 设 `SOUI_BUILD_TOOLS OFF` 并 `add_definitions(-D__IOS__)`；swinx iOS 平台层以 Obj-C++ 编入。需配置 Apple 团队/签名（`MACOSX_TEAM_ID`）。
- macOS：与 iOS 共享 `getResourceDir()` 的 `__APPLE__` 分支，资源落在 `.app` 内；`icon.icns` 作为 bundle 图标。
- 资源：iOS 用 `android/app/src/main/assets/uires`（移动版布局），桌面用 `client/uires`。

**测试要点清单**（iOS / 桌面回归集合）：

1. 启动后 `CMainDlg` 正常显示，iOS 下 `ShowWindow(SW_MAXIMIZE)` 全屏、桌面下 `SW_SHOWNORMAL`。
2. 点击计数 / 主题色切换 / 定时器（每秒 +1）全链路（验证 iOS TimerInfo 链表由消息循环驱动）。
3. SetCapture / 拖拽：DOWN 滑出控件仍收 MOVE；UP 自动释放。
4. 输入 / IME：iOS 软键盘显隐与高度（`getUiSoftKeyboardHeight`）正确；桌面直接键入。
5. 焦点 / 剪贴板：UIPasteboard 读写正常。
6. 资源路径：iOS / macOS 经 `GetAppleBundlePath` 正确定位 `soui-sys-resource` 与 `uires`，无缺失资源崩溃。
7. 旋转 / 前后台切换：iOS 前后台切换（`OnNsActive`）窗口不丢失、不崩溃。

---

## 9. 附录：核心文件索引

### 9.1 Android 核心文件索引（demos/android-demo）

Java 层（`app/src/main/java/com/soui/`）：

| 文件 | 角色 |
|---|---|
| [INativeWindow.java](../demos/android-demo/app/src/main/java/com/soui/android/INativeWindow.java) | HWND 真身接口；HWND=实现该接口的 jobject GlobalRef。 |
| [SouiPlatformBridge.java](../demos/android-demo/app/src/main/java/com/soui/android/SouiPlatformBridge.java) | 全局 JNI 桥单例：createWindow、View 工厂、SetCapture、焦点管理、定时器、screen 生命周期、剪贴板、RawInput、键盘高度同步。 |
| [SouiAbsLayout.java](../demos/android-demo/app/src/main/java/com/soui/android/SouiAbsLayout.java) | 绝对布局基类；提供 measure/layout 引擎 + MarginLayoutParams 兼容 + 子 View 位置/尺寸更新方法。 |
| [SouiAbsWindow.java](../demos/android-demo/app/src/main/java/com/soui/android/SouiAbsWindow.java) | 单 HWND 容器的抽象：主 Surface MATCH_PARENT 规则。 |
| [SouiWindow.java](../demos/android-demo/app/src/main/java/com/soui/android/SouiWindow.java) | 单 HWND 具体容器：`newSurface` 创建主 Surface、设置自身 LayoutParams、可见性/禁用初始值。 |
| [SouiScreen.java](../demos/android-demo/app/src/main/java/com/soui/android/SouiScreen.java) | 桌面容器（GetDesktopWindow），继承自 SouiWindow：screenId 启动参数、生命周期管理（onAttached→screenStartup/onDetached→screenShutdown）、尺寸同步、空状态监听、键盘高度监听。 |
| [SouiBaseSurface.java](../demos/android-demo/app/src/main/java/com/soui/android/SouiBaseSurface.java) | Surface 基类：输入事件分发 → Capture 重定向 → nativeOnMotionEventEx，IME 输入支持。 |
| [SouiSurface.java](../demos/android-demo/app/src/main/java/com/soui/android/SouiSurface.java) | 具体渲染 Surface：离屏 Bitmap + nativeRender → canvas.drawBitmap，SIMD 颜色通道转换。 |
| [NativeWindowDelegate.java](../demos/android-demo/app/src/main/java/com/soui/android/NativeWindowDelegate.java) | INativeWindow 的通用委托实现：窗口状态操作集中处理，支持 message-only 窗口。 |
| [SouiNativeHandle.java](../demos/android-demo/app/src/main/java/com/soui/android/SouiNativeHandle.java) | JNI 句柄包装工具（NativeCreate/NativeDestroy）。 |
| [NativeEditView.java](../demos/android-demo/app/src/main/java/com/soui/android/NativeEditView.java) | 原生 EditText 控件实现，处理 Win32 编辑控件消息（WM_SETTEXT/EM_GETSEL 等），支持 IME 输入。 |
| [UiThreadUtils.java](../demos/android-demo/app/src/main/java/com/soui/android/UiThreadUtils.java) | UI 线程工具类，提供线程安全的 UI 操作方法。 |
| [SouiInputConnection.java](../demos/android-demo/app/src/main/java/com/soui/android/SouiInputConnection.java) | IME 输入连接实现，处理字符提交和输入状态管理。 |

C++ 层（`app/src/main/cpp/`）：

| 文件 | 角色 |
|---|---|
| [AndroidPlatformAPI.h](../demos/android-demo/app/src/main/cpp/AndroidPlatformAPI.h) / [.cpp](../demos/android-demo/app/src/main/cpp/AndroidPlatformAPI.cpp) | SWinx Android 平台实现；JNI 调 Java 桥；screen 生命周期（激活栈+Context映射）；SetCapture/焦点/定时器/窗口操作；字符串槽管理；剪贴板 API；RawInput 设备枚举；键盘高度同步；HWND_MESSAGE 支持。 |
| [AndroidPlatformAPIReg.cpp](../demos/android-demo/app/src/main/cpp/AndroidPlatformAPIReg.cpp) | PlatformAPI 注册（RegisterAndroidPlatformAPI）。 |
| [SouiSurfaceProxy.h](../demos/android-demo/app/src/main/cpp/SouiSurfaceProxy.h) / [.cpp](../demos/android-demo/app/src/main/cpp/SouiSurfaceProxy.cpp) | 每个 Surface 的 native 侧对应：Motion 事件 → Win32 MSG 派发（支持双击、右键、滚轮、悬停）；Bitmap 像素锁 + Skia 渲染；键盘事件转换（支持 ALT 修饰键）；SIMD BGRA→RGBA 颜色通道转换。 |
| [Soui4Android.cpp](../demos/android-demo/app/src/main/cpp/Soui4Android.cpp) | JNI 方法实现：NativeCreate/NativeDestroy/nativeOnMotionEventEx/nativeOnKeyEventEx/nativeSouiStartup/nativeSouiShutdown/字符串槽读写等。 |
| [demo_native.cpp](../demos/android-demo/app/src/main/cpp/demo_native.cpp) | Demo 启动实现：`Soui4AndroidEntry` 抽象类实现，通过 `InitSoui4AndroidEntry` 注册入口，提供 `InitApp/UninitApp/ScreenStartup/ScreenShutdown` 虚函数接口。 |
| [MainDlg.h](../demos/android-demo/app/src/main/cpp/MainDlg.h) / [.cpp](../demos/android-demo/app/src/main/cpp/MainDlg.cpp) | 顶层 SHostWnd 业务：按钮点击计数、主题色、定时器、ShowToastAndroid。 |
| [SouiRealWndHandler.h](../demos/android-demo/app/src/main/cpp/SouiRealWndHandler.h) / [.cpp](../demos/android-demo/app/src/main/cpp/SouiRealWndHandler.cpp) | SOUI 真实窗口处理器：OnRealWndCreate/OnRealWndDestroy。 |
| [Android_Edit.cpp](../demos/android-demo/app/src/main/cpp/Android_Edit.cpp) | 原生 EDIT 控件 WNDPROC，处理字符串消息的 Slot 化转换，支持 IME 输入。 |
| [CMemDC_Android.cpp](../demos/android-demo/app/src/main/cpp/CMemDC_Android.cpp) | Android 平台的 CMemDC 实现，支持离屏渲染和 DIB Section 管理。 |
| [CMakeLists.txt](../demos/android-demo/app/src/main/cpp/CMakeLists.txt) | 单体 so 构建脚本；五个大目录 add_subdirectory；cxx flags、include dirs、link libs。 |

Java Demo 入口（`app/src/main/java/com/soui/demo/`）：

- [DemoApplication.java](../demos/android-demo/app/src/main/java/com/soui/demo/DemoApplication.java)：`onCreate` 中 loadLibrary → `SouiPlatformBridge.init` → `nativeInitSouiApp`。
- [HomeActivity.java](../demos/android-demo/app/src/main/java/com/soui/demo/HomeActivity.java)：首页两个按钮入口。
- [DefaultHostActivity.java](../demos/android-demo/app/src/main/java/com/soui/demo/DefaultHostActivity.java)：代码方式启动 SouiScreen。
- [XmlHostActivity.java](../demos/android-demo/app/src/main/java/com/soui/demo/XmlHostActivity.java)：XML 声明方式启动。

资源：

- `app/src/main/assets/soui_sys_res/`：SOUI 系统皮肤资源（sys_btn_*.svg、skin.xml、msgbox.xml），对应 Win32 端的 soui-sys-resource。
- `app/src/main/assets/uires/`：Demo 业务资源（image/soui.ico、values/{color,skin,string}.xml、xml/{demo_layout,dlg_main}.xml、uires.idx 索引）。

### 9.2 OHOS 核心文件索引（soui-ohos-lib）

见 §4.11。

### 9.3 iOS 核心文件索引（swinx/src/platform/ios）

见 §5.11。

---

**参考**：
- [1] Qt 6.9 文档《How Qt for Android Works》: https://doc-snapshot.qt-project.org/qt6-dev/android-how-it-works.html
- [2] Qt Blog《Qt for Android: a Novel Approach》(2024.07): https://www.qt.io/blog/qt-for-android-a-novel-approach
- [3] Qt Docs《QAndroidSurfaceRenderEngine Class》: https://doc.qt.io/QtAndroidAutomotive/qandroidsurfacerenderengine.html
- [4] 掘金《Flutter,Compose,Web 在Android平台上从布局到屏幕的机制探究》: https://juejin.cn/post/7572480736362266630
- [5] CSDN《Flutter 跨平台开发全解析》: https://blog.csdn.net/2302_80693444/article/details/155980501
