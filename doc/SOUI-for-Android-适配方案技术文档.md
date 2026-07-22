# SOUI for Android 适配方案技术文档

## 1. 概述

SOUI 是一款面向 C++/Win32 的轻量级 DirectUI 框架，核心围绕 Win32 HWND 句柄模型、SHostWnd（顶层窗口宿主）与消息循环构建。**SOUI for Android 适配方案** 的目标，是让一套基于 SWinx（SOUI 的跨平台 Win32 API 仿真层）编写的 SOUI C++ 业务代码，以 **几乎零改动** 的方式运行在 Android 设备上，并同时实现：

- 与 **Android 原生 Java/Kotlin View 系统无缝互嵌**：原生 View 可作为 SOUI 的"子窗口"，SOUI 的桌面容器（SouiScreen）也可以是任意原生布局（FrameLayout/LinearLayout/CoordinatorLayout...）的普通子 View。
- **严格对齐 Win32 语义**：`CreateWindowEx`、`SetCapture`/`ReleaseCapture`、`SetTimer`/`KillTimer`、`moveWindow`、`showWindow`、剪贴板操作、RawInput 设备枚举等核心 API 在 C++ 层的行为、参数、返回值与 Win32 一致，业务代码无需"Android 特判"。
- **单 SurfaceView 粒度的渲染**：每个 HWND 对应一个独立的 Android View（SouiSurface），独立测量、独立失效、独立合成，走系统 SurfaceFlinger 正常合成管线，而不是把所有 UI 画到一张全屏 Texture 上。
- **多 Activity / 多 Screen 支持**：通过 screenId 机制，支持在多个 Activity 中独立承载不同的 SOUI 窗口系统。
- **SIMD 优化的颜色通道转换**：使用 NEON/AArch64/SSSE3 指令加速 BGRA→RGBA 转换，大幅提升渲染性能。
- **完整的 IME 输入支持**：支持软键盘字符输入、键盘高度同步、输入连接管理。
- **HWND_MESSAGE 支持**：创建不可见的消息占位窗口，用于定时器消息处理等场景。

本文档从实现机制、竞品对比（Qt/Flutter）、原生集成优势、Demo 编译与测试四个维度完整介绍该方案。

---

## 2. 核心架构与实现机制

### 2.1 三层架构总览

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

**关键设计决策：C++ 业务代码不启独立渲染线程，完全跑在 Android 主线程。** 这一点和 Qt（独立 QtThread）、Flutter（独立 UI/Raster/IO 线程池）形成鲜明对比，带来的直接收益是 **JNI 调用无需跨线程调度、View 操作无需 post、与原生生命周期天然对齐**。

### 2.2 HWND = INativeWindow GlobalRef：零查表映射机制

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

### 2.3 窗口层级与容器模型

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

### 2.4 Surface 渲染流程（离屏 Bitmap + Skia + SIMD 颜色通道转换）

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

### 2.5 输入事件分发：严格对齐 Win32 `SetCapture`

输入路径（见 [SouiBaseSurface.java](../demos/android-demo/app/src/main/java/com/soui/android/SouiBaseSurface.java) `dispatchTouchEventToNative` 与 C++ [SouiSurfaceProxy.cpp](../demos/android-demo/app/src/main/cpp/SouiSurfaceProxy.cpp) `onMotionEventEx`）：

1. Android 系统按正常 `onInterceptTouchEvent`/`dispatchTouchEvent` 把事件派到被命中的 `SouiSurface` View。
2. Java 层先调用 `SouiPlatformBridge.tryDispatchCapturedMotion`：**如果当前有 SetCapture，并且事件来源 Surface ≠ 捕获目标 Surface，则做坐标变换（源 View 屏幕坐标 → 目标 View 客户区坐标）后，直接投递给捕获目标的 `nativeOnMotionEventEx`**，返回 Boolean.TRUE 表示"已消费重定向"。
3. 未被 Capture 拦截时，正常投递 `nativeOnMotionEventEx(action, x, y, pointerId, buttonState, vscroll, hscroll, metaState, timestamp)` 到当前 Surface。
4. C++ `onMotionEventEx` 根据 action 派发到 `WM_LBUTTONDOWN / WM_LBUTTONDBLCLK / WM_MOUSEMOVE / WM_RBUTTONDOWN / WM_MOUSEWHEEL / WM_MOUSEHWHEEL / WM_XBUTTONDOWN / WM_POINTER...`，完全对齐 Win32 MSG 格式，`SendMessage(hHost, ...)` 进入 SOUI 消息循环。
5. 遇到 `ACTION_UP / ACTION_CANCEL`：投递完后自动 `ReleaseCapture`（对应 Win32 "最后一个鼠标按键弹起时系统自动释放捕获"语义），保证滚动条拖拽、标题栏拖拽等控件交互与 Win32 表现一致。

**扩展输入支持**：
- **双击检测**：记录上次按下时间和坐标，若两次按下时间间隔小于系统双击阈值（默认 500ms）且坐标距离在阈值内，触发 `WM_LBUTTONDBLCLK`
- **右键菜单**：长按事件转换为 `WM_RBUTTONDOWN`，支持上下文菜单弹出
- **滚轮事件**：支持垂直滚轮（`WM_MOUSEWHEEL`）和水平滚轮（`WM_MOUSEHWHEEL`），外接鼠标时正常工作
- **悬停事件**：支持 `TrackMouseEvent` 语义，`WM_MOUSEHOVER`/`WM_MOUSELEAVE` 消息正确投递
- **XButton 支持**：鼠标侧键转换为 `WM_XBUTTONDOWN`/`WM_XBUTTONUP`

`SouiPlatformBridge.setCapture / releaseCapture / getCapture` 三个方法（JNI 暴露给 C++ `AndroidPlatformAPI`）是该机制的状态总控。

### 2.6 焦点管理：SetFocus/GetFocus 双向同步

`SouiPlatformBridge` 维护全局焦点状态 `mFocusHwnd`（volatile long），实现 Win32 `SetFocus`/`GetFocus` 语义：

- **C++ → Java 同步**：`AndroidPlatformAPI::setFocus` 调用 Java `setFocus(hwnd)`，更新 `mFocusHwnd`，并同步 Android View 焦点和 IME 状态
- **Java → C++ 同步**：`SouiBaseSurface` 在获得焦点时调用 `nativeNotifyFocusGained(hwnd)`，触发 C++ 层 `::SetFocus`，投递 `WM_SETFOCUS`/`WM_KILLFOCUS` 消息

**IME 策略**：
- `EditText` 子类（`NativeEditView` → className="edit"）自动弹出 IME，支持光标位置查询（`EM_GETSEL`）和文本设置（`WM_SETTEXT`）
- 其他 `SouiBaseSurface`：SOUI 自己实现了 SEdit 等控件，用离屏渲染绘制文本，但仍依赖 Android IME 把按键输入转成字符，也弹出 IME
- **输入连接管理**：`SouiBaseSurface.onCreateInputConnection` 创建 `SouiInputConnection`，处理字符提交（`commitText` → `sendImeString` → `WM_IME_CHAR`）
- **IME 选项**：设置 `IME_FLAG_NO_FULLSCREEN`，避免横屏时全屏输入法遮挡内容

### 2.7 定时器：`SetTimer/KillTimer` 语义 100% 对齐 Win32

`SouiPlatformBridge` 内部用 `Handler(Looper.getMainLooper())` + 嵌套 Map `Map<hWnd, Map<timerId, TimerRunnable>>` 实现：

- `(hWnd, timerId)` 二元组唯一标识一个定时器；不同 hWnd 之间相同数值的 timerId **互不冲突**。
- `hWnd != 0 && nIDEvent == 0`：系统在该 hWnd 下自动分配一个未使用的 id。
- `hWnd == 0`（非窗口定时器）：Win32 文档语义"忽略 nIDEvent，系统生成全局唯一 id"，严格复现。
- `TimerRunnable.run`：一次触发后，若仍注册且 active，则 `postDelayed(self, interval)`，完成"默认循环"行为；同时 JNI 调用 `nativeOnTimerExpired(hWnd, timerId)` → C++ `AndroidPlatformAPI::onTimerExpired` → `PostMessage WM_TIMER` 或直接调 `TIMERPROC`。
- C++ 层 `m_timerEntries` 镜像 Java 层的定时器状态，仅记录 `TIMERPROC` 回调与元数据，真正调度由 Java Handler 完成。

### 2.8 多 Activity / 多 Screen（screenId 机制）

一个 Android App 里往往有多个 Activity，每个 Activity 都可能承载自己的 SouiScreen；C++ 需要知道"创建一个顶级窗口（WS_POPUP/WS_OVERLAPPED）时，应该把它加到哪个 SouiScreen 的 View 树里"。方案：

- **激活栈机制**：`AndroidPlatformAPI` 维护 `m_activeScreenStack`（vector<jlong>），`souiStartup` 时 push，`souiShutdown` 时 pop。创建窗口遇到 `hWndParent=HWND_DESKTOP(0)` 时，从栈顶取当前激活的 screenId。
- **Screen Context 映射**：`m_screenContexts`（map<jlong, HWND>）存储 screenId 到其 Surface HWND 的映射。
- **原子启动**：Java 层 `SouiPlatformBridge.souiStartup(screenId, screenHwnd, layout)` 一次 JNI 调用完成四件事：
  1. `RegisterVirtualHWND` 注册虚拟 HWND
  2. `setScreenHwnd` 写入 screenId → HWND 映射
  3. `pushActiveScreen` 压入激活栈
  4. 调用 `s_entry->Startup(screenId, layout)` 创建主窗口（通过 `Soui4AndroidEntry` 虚函数接口）
- **原子销毁**：`souiShutdown(screenId)` 调用 `s_entry->Shutdown(screenId)` → `popActiveScreen` → `eraseScreen` → `UnregisterVirtualHWND`。
- **启动流程优化**：`SouiScreen.onAttachedToWindow` 中，若 `mScreenId != 0` 才调用 `souiStartup`，避免未配置 screenId 时的误启动。

**启动参数传递**：
- `screenId`：唯一标识当前 Screen，用于多 Activity 场景下的窗口路由
- `screenHwnd`：Screen 对应的 Surface HWND，作为桌面窗口的 HWND
- `layout`：SOUI 布局文件名（如 `"layout:dlg_main"`），指定启动时加载的 XML 布局

**创建窗口时的路由逻辑**：
1. 若 `hWndParent == HWND_DESKTOP(0)`：从激活栈顶获取当前 screenId，查找对应的 Screen HWND 作为实际父窗口
2. 若 `hWndParent == HWND_MESSAGE(-1)`：创建消息占位窗口，不挂视觉树
3. 其他情况：直接使用指定的父窗口 HWND

### 2.9 屏幕旋转与生命周期管理

Android 的配置变更（如屏幕旋转）会触发 Activity 重建，导致所有 View 被销毁后重新创建。由于 SOUI 的所有 HWND 都映射到 Java 层的 View 对象（`SouiSurface` / `SouiWindow`），这些 HWND 也会随之销毁和重建。

**生命周期绑定**：
- `SouiScreen.onAttachedToWindow()`：调用 `souiStartup` 创建 C++ 窗口系统
- `SouiScreen.onDetachedFromWindow()`：调用 `souiShutdown` 销毁 C++ 窗口系统
- `SouiScreen.onSizeChanged()`：调用 `moveWindow` 和 `updateScreenSize` 更新窗口尺寸

**旋转行为**：
1. Activity 销毁 → `onDetachedFromWindow` → `souiShutdown` → 所有 C++ HWND 销毁
2. Activity 重建 → `onAttachedToWindow` → `souiStartup` → 重新创建 C++ 窗口系统

**注意事项**：
- C++ 层窗口状态（如控件内容、滚动位置）在旋转时会丢失，需要业务层自行保存和恢复
- `mSouiMainHwnd` 在旋转后会变为新值，需要重新获取
- 尺寸同步通过 `onSizeChanged` 自动处理，确保 C++ 窗口尺寸与新屏幕尺寸一致

### 2.10 String Slot 机制：JNI 字符串参数交换

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

### 2.11 消息转发机制：统一消息路由

`INativeWindow.nativeSendMessage(int msg, long wParam, long lParam)` 提供了统一的消息转发接口：

- **字符串参数**：通过 String Slot 机制交换
- **纯数值消息**：按 Win32 语义直接传递
- **特殊处理**：`EM_GETSEL` 返回值编码为 `(start << 32) | end`，C++ 层解码后写入用户缓冲区

### 2.12 SouiSurfaceProxy：输入事件转换与渲染代理

每个 `SouiSurface` 在 C++ 层对应一个 `SouiSurfaceProxy` 对象：

- **生命周期管理**：通过 `std::shared_ptr` 管理，`nativeViewInsert`/`nativeViewLookup`/`nativeViewErase` 提供线程安全的访问。`NativeDestroy` 时先从 map 移除，再让局部 `shared_ptr` 出作用域析构，确保其他线程正在持有的引用安全完成。
- **输入事件转换**：`onMotionEventEx` 将 Android MotionEvent 转换为 Win32 风格消息（`WM_LBUTTONDOWN`、`WM_MOUSEMOVE`、`WM_MOUSEWHEEL`、`WM_XBUTTONDOWN` 等），支持双击检测、右键菜单、滚轮（垂直+水平）、悬停事件。
- **键盘事件转换**：`onKeyEventEx` 将 Android KeyEvent 转换为 Win32 键盘消息，支持 `repeatCount`、`scanCode`、`unicodeChar`、`metaState` 等参数，正确处理 ALT 修饰键生成 `WM_SYSKEYDOWN/SYSKEYUP`。
- **渲染**：`render` 方法将 C++ 层绘制结果绑定到 Java Bitmap，包含 SIMD 优化的 BGRA→RGBA 颜色通道转换。
- **鼠标状态跟踪**：维护 `m_buttonsDown`、`m_lastDownTime/X/Y`、`m_hoverTracked` 等状态，支持 `TrackMouseEvent` 语义。

### 2.13 剪贴板 API：Win32 语义完整实现

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

### 2.14 RawInput 设备枚举：跨平台输入设备检测

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

### 2.15 键盘高度同步：IME 弹出时自动调整布局

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

### 2.16 IME 输入支持：软键盘字符输入

`SouiBaseSurface` 实现 `onCreateInputConnection`，支持 Android 输入法：

- **输入类型**：`TYPE_CLASS_TEXT | TYPE_TEXT_VARIATION_NORMAL`
- **IME 选项**：`IME_FLAG_NO_FULLSCREEN`，避免横屏时全屏输入法遮挡内容
- **字符提交**：`SouiInputConnection.commitText` 通过 `sendImeString` 将字符投递到 C++ 层，转换为 `WM_IME_CHAR` 消息

### 2.17 HWND_MESSAGE 支持：消息占位窗口

支持 Win32 的 `HWND_MESSAGE` 语义，创建不可见的消息占位窗口：

- **创建流程**：`createWindow` 检测到 `hWndParent == HWND_MESSAGE` 时，创建 `SouiBaseSurface`（不挂视觉树），尺寸强制为 0x0，可见性为 `GONE`
- **存储机制**：单独存储在 `mMessageOnlyWindows` 集合中，与普通窗口分离
- **用途**：用于定时器消息处理等不需要视觉呈现的场景

---

## 3. 与 Qt / Flutter 方案对比

### 3.1 Qt for Android 架构要点

Qt 传统 Android 适配（Android QPA 插件）：<sup>[\[1\]](https://doc-snapshot.qt-project.org/qt6-dev/android-how-it-works.html)</sup>

- **双线程模型**：`QtThread`（执行 `main()`、加载 Qt 库、跑 Qt 事件循环、执行所有 QML/C++ 渲染）独立于 Android UI Thread。任何需要改 View 的操作都需要跨线程 post，`runOnAndroidMainThread` 封装调度。
- **Qt 掌控 Activity 生命周期**：默认 Activity 必须继承 `QtActivity`；`onCreate` 内加载所有 Qt .so 并启动 QtThread，之后 Qt 通过内部 `QtLayout`、渲染 Surface、输入代理把 QML/Qt Widget 结果呈现。
- **单 Activity 单顶层窗口** 传统模式；Qt 6.7+ "Novel Approach" 才提供把 QML 作为 `QtQuickView`（本质一个 `FrameLayout + Surface + JNI 双向绑定`）嵌入到已有 Android App 的能力，但仍要求 QML 场景由 Qt Quick 管理，控件与原生控件不混排。<sup>[\[2\]](https://www.qt.io/blog/qt-for-android-a-novel-approach)</sup>
- **渲染路径**：Qt Quick 场景栅格化到 OpenGL FBO 或 `ANativeWindow`（QAndroidSurfaceRenderEngine 方式：headless QQuickWindow → 纹理转发到多个 Surface View）<sup>[\[3\]](https://doc.qt.io/QtAndroidAutomotive/qandroidsurfacerenderengine.html)</sup>，不是原生 View 粒度。

### 3.2 Flutter for Android 架构要点

- **三层架构**：Dart Framework（Widget/Element/RenderObject）、Dart VM（AOT 编译 Release）、Engine（C++：Skia 2024→Impeller 图形、HarfBuzz 文本、Dart Runtime）<sup>[\[4\]](https://blog.csdn.net/2302_80693444/article/details/155980501)</sup>。
- **自绘引擎**：Flutter 不把 Widget 映射成原生 View，而是通过 Skia/Impeller 把整棵 RenderObject 树画到**一个或少数几个全屏 FlutterView Surface** 上，SurfaceFlinger 只把它当一个 Layer 合成<sup>[\[5\]](https://juejin.cn/post/7572480736362266630)</sup>。
- **与原生互操作靠 Platform View**：`AndroidView` / `PlatformViewLink`（Surface 级别）把原生控件"贴"到 Flutter 渲染结果上；本质是两个独立 Surface 的合成，Z-order、裁剪、手势消歧都有历史兼容性坑，且列表内复用成本高。
- **独立线程池**：UI 线程（Dart 构建 Widget/Layout）、Raster 线程（Skia/Impeller 提交 GL/Vulkan）、IO 线程（图片解码、字体加载）都独立跑，与主线程通过 Port Channel 通信。

### 3.3 三维对比总表

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

## 4. 核心优势：与原生 Java/Kotlin 开发无缝集成

"**无缝**"在这里不是宣传语，而是三个具体事实的总和：

### 4.1 原生 View 可直接成为 SOUI 的一个"子控件"

`SouiPlatformBridge.registerViewFactory(className, Function<String,View>)`：C++ 层 `CreateWindow("edit")` 会走到 Java 查表 → 创建 `NativeEditView`（继承 `AppCompatEditText` + 包装 `INativeWindow`）→ 返回 HWND。从此：

- C++ 层 `SetDlgItemText(hDlg, ID_EDIT, L"xxx")` → Java `SetText`；
- `EnableWindow(GetDlgItem(hDlg, ID_EDIT), FALSE)` → Java `setEnabled(false)`；
- 用户在 EditText 里的触摸/输入，Android 系统按原生焦点/输入法正常派；如需把事件再交 SOUI，也只需要该包装 View 的监听器里做 JNI 回调。

这是 Qt/Flutter 做不到或做起来代价极高的点：**SOUI 的子控件是"原生控件真身"，不是画出来的外观仿真**。对需要调用 Android 系统原生键盘、文字选择、辅助服务（TalkBack）、厂商对 Edit 的特殊优化等场景，天然 100% 兼容。

### 4.2 SouiScreen 能嵌入**任意**原生 ViewGroup

`SouiScreen extends SouiWindow extends SouiAbsWindow extends SouiAbsLayout extends ViewGroup`。你可以：

- 在 `FrameLayout` 里一半放 `Toolbar + SouiScreen`，另一半放原生 `BottomNavigationView`；
- 在 `CoordinatorLayout` 里让 SouiScreen 配合 `AppBarLayout` 折叠；
- 在 `RecyclerView` item 里嵌一个小型 SouiScreen（当然通常不建议，但技术上完全可行）。

`SouiAbsLayout.LayoutParams extends MarginLayoutParams` 这一行保证被外层标准容器 `measureChildWithMargins` 调用时无 ClassCastException。

### 4.3 XML 一行声明即可启动 SOUI

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

`SouiScreen.Init` 在构造阶段读取属性 → `onAttachedToWindow` 自动调用 `souiStartup`。Activity 的 `onCreate` 只需一行 `setContentView(R.layout.activity_xml_host)`（见 [XmlHostActivity.java](../demos/android-demo/app/src/main/java/com/soui/demo/XmlHostActivity.java)），**零 SOUI 相关样板代码**。

### 4.4 JNI 双向调用天然友好：C++ 直接调 Android 原生 API 示例

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

### 4.5 NativeWindowDelegate：窗口状态操作的集中委托

`NativeWindowDelegate` 提供了 `INativeWindow` 接口的公共实现，让不同基类的 Surface View 共享同一套行为：

- **目标解析**：自动识别父容器是 `SouiAbsLayout` 还是普通 `ViewGroup`，选择正确的操作目标
- **操作委托**：将 `nativeDestroy`、`nativeShow`、`nativeMove` 等操作委托给父容器或 Surface 自身
- **特殊情况处理**：支持 message-only 窗口（无父容器的 0x0 占位窗口）的优雅处理

---

## 5. Demo 编译与测试

Demo 工程位于：`demos/android-demo/`。

### 5.1 环境要求

| 组件 | 版本要求 | 说明 |
|---|---|---|
| Android Studio | Hedgehog (2023.1) 及以上 | AGP 7.4 / 8.x 均可。 |
| Android SDK | compileSdk 34，minSdk 28 | 见 [app/build.gradle](../demos/android-demo/app/build.gradle)。 |
| Android NDK | 与 AGP 匹配的任意 LTS（如 25.x / 26.x） | Gradle 会通过 `local.properties` 里的 `ndk.dir` 或 SDK Manager 默认安装目录自动定位。 |
| CMake | ≥ 3.18.1 | SDK Manager → SDK Tools 安装。 |
| JDK | 17（运行 Gradle）；代码兼容 Java 8。 | Gradle 项目属性配置。 |
| 硬件/模拟器 | armeabi-v7a / arm64-v8a / x86 / x86_64 任一种 | `ndk.abiFilters` 四者都编；按需裁剪。 |
| Git 子模块（如有） | 确保 `soui4/` 下所有子模块已初始化 | 若 SOUI 目录缺 `third-part/` 或 `components/` 内容，先 `git submodule update --init --recursive`。 |

### 5.2 编译步骤

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

**C++ 入口方式**：采用 `Soui4AndroidEntry` 抽象类 + 虚函数调用模式。用户实现 `Soui4AndroidEntry` 的四个虚函数（`InitApp`、`UninitApp`、`Startup`、`Shutdown`），通过全局静态变量注册到系统。`AndroidPlatformAPI` 内部通过 `s_entry` 指针调用这些虚函数，实现平台层与业务层的解耦。见 [demo_native.cpp](../demos/android-demo/app/src/main/cpp/demo_native.cpp)。

### 5.3 Demo 功能结构

启动 App 后进入 `HomeActivity`（见 [HomeActivity.java](../demos/android-demo/app/src/main/java/com/soui/demo/HomeActivity.java)），提供两个按钮对应两种启动方式：

**入口 A：DefaultHostActivity（代码方式）**
- `SCREEN_ID = 0x0000_D001L`，`LAYOUT_DEMO = "layout:dlg_main"`（对应 assets/uires/xml/dlg_main.xml）。
- `mScreen = new SouiScreen(this)` → `setScreenId / setSouiLayout` → `FrameLayout.addView(mScreen, MATCH_PARENT)` → `onAttachedToWindow` 自动调用 `souiStartup` → C++ 调 `SHostWnd::Create(NULL)` 创建主窗口。
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

### 5.4 测试要点清单

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

---

## 6. 附录：核心文件索引

Java 层（`app/src/main/java/com/soui/`）：

| 文件 | 角色 |
|---|---|
| [INativeWindow.java](../demos/android-demo/app/src/main/java/com/soui/android/INativeWindow.java) | HWND 真身接口；HWND=实现该接口的 jobject GlobalRef。 |
| [SouiPlatformBridge.java](../demos/android-demo/app/src/main/java/com/soui/android/SouiPlatformBridge.java) | 全局 JNI 桥单例：createWindow、View 工厂、SetCapture、焦点管理、定时器、screen 生命周期、剪贴板、RawInput、键盘高度同步。 |
| [SouiAbsLayout.java](../demos/android-demo/app/src/main/java/com/soui/android/SouiAbsLayout.java) | 绝对布局基类；提供 measure/layout 引擎 + MarginLayoutParams 兼容 + 子 View 位置/尺寸更新方法。 |
| [SouiAbsWindow.java](../demos/android-demo/app/src/main/java/com/soui/android/SouiAbsWindow.java) | 单 HWND 容器的抽象：主 Surface MATCH_PARENT 规则。 |
| [SouiWindow.java](../demos/android-demo/app/src/main/java/com/soui/android/SouiWindow.java) | 单 HWND 具体容器：`newSurface` 创建主 Surface、设置自身 LayoutParams、可见性/禁用初始值。 |
| [SouiScreen.java](../demos/android-demo/app/src/main/java/com/soui/android/SouiScreen.java) | 桌面容器（GetDesktopWindow），继承自 SouiWindow：screenId 启动参数、生命周期管理（onAttached→souiStartup/onDetached→souiShutdown）、尺寸同步、空状态监听、键盘高度监听。 |
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
| [demo_native.cpp](../demos/android-demo/app/src/main/cpp/demo_native.cpp) | Demo 启动实现：`Soui4AndroidEntry` 抽象类实现，通过 `InitSoui4AndroidEntry` 注册入口，提供 `InitApp/UninitApp/Startup/Shutdown` 虚函数接口。 |
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

---

**参考**：
- [1] Qt 6.9 文档《How Qt for Android Works》: https://doc-snapshot.qt-project.org/qt6-dev/android-how-it-works.html
- [2] Qt Blog《Qt for Android: a Novel Approach》(2024.07): https://www.qt.io/blog/qt-for-android-a-novel-approach
- [3] Qt Docs《QAndroidSurfaceRenderEngine Class》: https://doc.qt.io/QtAndroidAutomotive/qandroidsurfacerenderengine.html
- [4] 掘金《Flutter,Compose,Web 在Android平台上从布局到屏幕的机制探究》: https://juejin.cn/post/7572480736362266630
- [5] CSDN《Flutter 跨平台开发全解析》: https://blog.csdn.net/2302_80693444/article/details/155980501