# SOUI4 Android 方案 vs Flutter vs Qt — 架构对比、优缺点与选型建议

> 目标读者：需要从 0 到 1 选型跨平台移动端 UI 框架的团队 / 手上已有 SOUI（Windows DirectUI）存量代码想上 Android 的团队。
> 基准版本：
> - SOUI4：仓库 `d:\work\soui4\` HEAD（含 `soui-android-lib` + `swinx` + Skia 后端）
> - Flutter 3.19 / Dart 3.x
> - Qt 6.6（Qt Quick + Qt Widgets 两条产品线）

---

## 0. TL;DR（结论先行）

| 你面对的场景 | 推荐方案 | 理由一句话 |
|---|---|---|
| 团队已经有几十万行 SOUI（Windows DirectUI）C++ 业务代码，需要低成本跑上 Android | **✅ SOUI4 soui-android-lib** | **同一套 SHostWnd 子类 + EVT 事件 + SWindow 树，90%+ 源码复用**，改 10% 平台相关就能编译上 Android |
| 新团队、新项目、没有任何 C++/Win32 存量，追求开发效率 + 热重载 + 插件生态 | **✅ Flutter** | 纯 Dart + 一套代码多端，社区最大，坑最少 |
| 需要跨 Win/Linux/Android/iOS/嵌入式 Linux，**同时有重型工业 UI 大量 Qt C++ 历史业务** | **✅ Qt** | QPA 抽象层成熟，QML + WebEngine 能力完备，B2B/工业界认可度最高 |
| 要做抖音/小红书级流畅动效、手势导航、Lottie 动画、拍照相机 Picker 完美体验 | **✅ Flutter / 原生 Kotlin** | SOUI/Qt 在这些方面坑都较多，不值得花团队精力填 |

---

## 1. 三者 Android 端架构一句话定义

- **SOUI4 for Android**：在 Android View/Canvas/Handler 之上，用 `swinx` 模拟了一整套 **Win32 子系统（HWND / SendMessage / SetTimer / InvalidateRect / SetCapture）**，让原 Windows 上的 `SHostWnd + SWindow 树` 源码级直接跑起来。
- **Flutter**：Dart 语言自带 UI 框架（Widget → Element → RenderObject → Layer），用自家 C++ Engine 调 Skia 直接画到 Surface / SurfaceTexture，**完全不经过 Android 原生 View 绘制系统**，Android 侧只是一个 Embedder（容器 + 事件分发入口 + Platform Channel）。
- **Qt for Android**：用 **QPA（Qt Platform Abstraction）** 抽象层替换 Windows/Wayland/X11 后端，Android QPA 插件把 `QGuiApplication / QWindow / QOpenGLContext` 映射到 `ANativeWindow / Surface / JNI Activity / InputDevice`，再在之上跑 Qt Widgets（CPU 栅格化）或 Qt Quick Scene Graph（OpenGL ES GPU 渲染）。

三者从架构路线上就不是一类东西：SOUI 是 **"Win32 on Android" 的操作系统 API 模拟层方案**；Qt 是 **"GUI 类库跨平台" 的 QPA 平台抽象方案**；Flutter 是 **"自绘渲染引擎+自带UI框架" 的自绘方案**。路线差异决定了后面所有优缺点。

---

## 2. 架构分层对比图

```
┌───────────────────────────────────────────────────────────────────────────────┐
│ SOUI4 for Android（Win32 模拟层方案）                                          │
├───────────────────────────────────────────────────────────────────────────────┤
│  ┌─────────────────────────────────────────┐   ┌────────────────────────────┐ │
│  │ 你的业务（C++ SHostWnd 子类）           │   │  SOUI 资源（layout/skin）  │ │
│  │  CMainDlg::OnInitDialog/OnOK/OnTimer    │   │  直接复用 Windows 端 uires │ │
│  └─────────────┬───────────────────────────┘   └────────────────────────────┘ │
│                │ SHostWnd::DoFrameEvent / EVENT_MAP                            │
│  ┌─────────────▼───────────────────────────┐                                  │
│  │ SOUI 框架（SWindow 树 / RenderTarget ） │  ← Skia 后端 Win/Android 共用    │
│  └─────────────┬───────────────────────────┘                                  │
│                │ ::SendMessage / ::SetTimer / ::InvalidateRect                │
│  ┌─────────────▼───────────────────────────┐                                  │
│  │ swinx（跨平台 Win32 抽象层）              │  ← 本项目核心差异化：WindowProc │
│  │ WndMgr::fromHwnd(nativeId) → WndObj     │    thunk + 内部消息队列          │
│  └─────────────┬───────────────────────────┘                                  │
│                │ JNI 双向回调                                                  │
│  ┌─────────────▼───────────────────────────┐                                  │
│  │ SouiPlatformBridge / SouiView (Java)    │  Handler.postDelayed = SetTimer  │
│  │ View.invalidate = InvalidateRect        │  Bitmap + Canvas.drawBitmap      │
│  │ MotionEvent → WM_LBUTTONDOWN / WM_MOVE  │  KeyEvent → WM_KEYDOWN/WM_CHAR   │
│  └─────────────────────────────────────────┘                                  │
└───────────────────────────────────────────────────────────────────────────────┘

┌───────────────────────────────────────────────────────────────────────────────┐
│ Flutter（自绘引擎方案）                                                        │
├───────────────────────────────────────────────────────────────────────────────┤
│  ┌─────────────────────────────────────────┐                                  │
│  │ 你的业务（Dart Widget / Riverpod/Bloc） │  ← 与 Win/Android/Linux 共用代码  │
│  └─────────────┬───────────────────────────┘                                  │
│                │ Dart: Build → Layout → Paint → Layer Tree                    │
│  ┌─────────────▼───────────────────────────┐                                  │
│  │ Flutter Framework（Dart）               │  ← Widget/RenderObject/Layer      │
│  └─────────────┬───────────────────────────┘                                  │
│                │ Dart<->C++ binding                                          │
│  ┌─────────────▼───────────────────────────┐                                  │
│  │ Flutter Engine（C++）                    │  Skia / Impeller → OpenGL/Vulkan  │
│  │ Animator/Scheduler/Compositor/PlatformView│                                │
│  └─────────────┬───────────────────────────┘                                  │
│                │ Embedder API (C)                                             │
│  ┌─────────────▼───────────────────────────┐                                  │
│  │ Android Embedder (Java/Kotlin)          │  FlutterSurfaceView / FlutterView │
│  │ Platform Channel                         │  Activity/Application 生命周期管理│
│  └─────────────────────────────────────────┘                                  │
└───────────────────────────────────────────────────────────────────────────────┘

┌───────────────────────────────────────────────────────────────────────────────┐
│ Qt for Android（QPA 平台抽象方案）                                              │
├───────────────────────────────────────────────────────────────────────────────┤
│  ┌─────────────────────────────────────────┐  ┌────────────────────────────┐ │
│  │ 你的业务 C++ (QWidget) 或 QML + JS      │  │ Qt Resource (.qrc)        │ │
│  │  QMainWindow::on_pushButton_clicked     │  │ Win/Android/Linux 共用     │ │
│  └─────────────┬───────────────────────────┘  └────────────────────────────┘ │
│                │ QWidget::event() / QQuickItem::mousePressEvent               │
│  ┌─────────────▼───────────────────────────┐                                  │
│  │ Qt GUI / Qt Quick Scene Graph           │  raster 或 OpenGL ES             │
│  └─────────────┬───────────────────────────┘                                  │
│                │ QPlatform* 抽象接口                                          │
│  ┌─────────────▼───────────────────────────┐                                  │
│  │ Qt Android QPA Plugin (C++)             │  ← 核心差异化：把 QWindow 等     │
│  │ QAndroidPlatformWindow / QAndroidPlatformBackingStore / QAndroidPlatformOpenGLContext│
│  └─────────────┬───────────────────────────┘                                  │
│                │ JNI                                                           │
│  ┌─────────────▼───────────────────────────┐                                  │
│  │ QtActivity / QtApplication (Java)       │  加载 .so / plugins / assets     │
│  │ Input 事件 / Surface / 生命周期分发      │  自带一堆 Android 服务管理代码  │
│  └─────────────────────────────────────────┘                                  │
└───────────────────────────────────────────────────────────────────────────────┘
```

---

## 3. 8 大维度系统对比

### 3.1 已有存量代码复用成本（SOUI 最大强项）

| 维度 | SOUI4 for Android | Flutter | Qt |
|---|---|---|---|
| **Windows 端同名 SOUI 业务** | ⭐⭐⭐⭐⭐ **近乎零成本复用**：`CMainDlg.cpp`、MainDlg.h、EVENT_MAP、资源 uires.zip 直接 copy。实测我们 demo 里 CMainDlg 代码 99% 与 Win 端一致，只改了 postAndroidToast 一个平台函数。 | ⭐ **完全不能复用**：Widget 体系完全不同，必须全部重写为 Dart + Flutter Widget。 | ⭐⭐ **可复用 C++ 非 UI 逻辑**；UI 如果是 Qt 跨端写的（Qt Widgets/QML）就能复用，**但是如果原业务用的是 MFC/WTL/SOUI/WinForm，则 UI 依然要重写**。 |
| **跨 Android/iOS/Linux/macOS 复用** | ⭐⭐ **仅 Win + Android 路线成熟**：iOS/macOS 需要再写一套 swinx + 平台 View 映射（工程量巨大，暂无官方实现）。 | ⭐⭐⭐⭐⭐ **官方 6 端均一等公民**，一套 Dart 全端运行。 | ⭐⭐⭐⭐⭐ **官方 6 端 + 嵌入式 Linux + QNX/INTEGRITY 全支持**，工业软件首选。 |
| **非 UI 业务（C/C++ 核心算法/网络）** | ⭐⭐⭐⭐⭐ **直接链接进 libsoui-android.so**，CMake target_link_libraries 一把梭。 | ⭐⭐⭐⭐ 用 `dart:ffi` 调用 `.so`，但类型映射/内存管理需要手写，比 C++ 原生链接麻烦一些。 | ⭐⭐⭐⭐⭐ **Qt 本身就是 C++**，算法代码直接 #include 编译。 |

### 3.2 渲染管线与渲染性能

| 维度 | SOUI4 for Android | Flutter | Qt |
|---|---|---|---|
| **渲染方式** | Skia 画到 ARGB_8888 Bitmap → Java Canvas `drawBitmap(offscreenBitmap, 0,0, paint)` 上屏。 | Skia/Impeller 直接画到 Surface / SurfaceTexture → Android SurfaceFlinger 合成上屏。**无中间 Bitmap 拷贝**。 | Qt Widgets：raster engine 软件栅格化到 QImage/BackingStore → QPainter；Qt Quick：Scene Graph 用 OpenGL ES / Vulkan 直接画到 ANativeWindow。 |
| **GPU 加速** | ⚠️ 当前实现是 **CPU 栅格化 → Bitmap → GPU 合成**（因为 Bitmap 在 GL 线程是一张纹理上传）；Skia 本身支持 GPU 但要接入 OpenGL/Vulkan 上下文后才能启用。 | ✅ **默认 GPU 加速**（3.19 默认 Skia GL、Impeller 正在替代），动画帧率原生可达 90/120Hz。 | ✅ Qt Quick 默认 OpenGL ES；Qt Widgets 软件（可改 OpenGL 后端但默认没开）。 |
| **复杂 60FPS 动画** | ⭐⭐ 长列表/复杂动效需要自己优化。受限于每次重绘都要把整个 offscreenBitmap 重新 drawBitmap；局部刷新（`invalidate(left,top,right,bottom)`）可显著降低开销。 | ⭐⭐⭐⭐⭐ **强项**：Scheduler + Raster Cache + Layer 合成天生适合动画，Lottie/Flare 生态成熟。 | ⭐⭐⭐⭐ Qt Quick Scene Graph 表现优秀；Qt Widgets 路线动画性能一般。 |
| **首帧 vs 增量绘制** | SOUI 的 InvalidateRect 已经桥接到 Java `view.invalidate(left,top,right,bottom)`，增量重绘只画脏区后 Canvas 合成脏矩形上屏（链路正确）。 | Flutter Scheduler 每帧按 vsync 驱动，自动层离屏缓存 + Raster 增量。 | Qt Quick 同样 Scene Graph 节点级缓存。 |
| **文本渲染** | Skia HarfBuzz（与 Flutter/Qt 共用），中/英/复杂脚本都 OK。 | Skia + Minikin/HarfBuzz。 | Qt 文本引擎自有 + HarfBuzz。 |

> 🧭 **补充：SOUI UI 全 C++ 实现 vs 原生 Java/Kotlin/Compose UI 的性能差异（容易被忽视的非渲染管线性能）**
>
> 1. **零 ART GC 压力**：Java 原生每一个 View/Compose Node/LazyList Item 都是 ART 堆对象（ViewHolder 里 1 个 ConstraintLayout + 1 个 ImageView + 3 个 TextView = 5 个对象，长列表一次滑过 = 成百上千次分配/回收），直接触发 GC 卡顿（`ART : Background young concurrent copying GC freed 123840(6MB) AllocSpace objects, 2(128KB) LOS objects, 33% free`）。**SOUI 的 SWindow/SHostWnd 是 C++ `new/delete` / `std::unique_ptr`，不走 ART 堆，永远不会因为 UI 对象分配触发 Java GC。** 这在工业 App 长时间运行（7x24 小时开机不杀进程）、列表高频滑动场景是决定性的稳定性优势。
> 2. **跨语言调用边界收敛、往返次数少**：一次触摸事件从 `MotionEvent` → JNI `onTouchEvent` 一次进入 C++，之后在 C++ 里跑完 HitTest + EventMap + InvalidateRect，最后通过一次 `view.invalidate()` 回调 Java 上屏。全程**一次进 + 一次出 = 2 次 JNI**。原生 Java 写一个列表 Item 点击，事件路径是：DecorView → ViewGroup dispatch → ItemView onTouch → clickListener → Adapter.notifyItemChanged → ViewHolder bind → 多次 getter/setter 访问跨模块 → 触发 Choreographer 下一帧重绘，跨方法/跨类调用几十次以上，且伴随大量临时对象分配。
> 3. **数据内存布局 CPU cache 友好**：SOUI SWindow 的状态（位置/大小/颜色/字体/可见性）是 C++ POD 字段连续布局，遍历 SWindow 树做 Layout/Paint 时 CPU L1/L2 cache hit 率高。Java View 每个对象都有对象头 + klass 指针 + 字段分散在堆上，做一次 requestLayout 遍历树 cache miss 率非常高。
> 4. **DEX 方法数压力小**：SOUI 所有控件（SButton/SEdit/SList/SGrid/STabCtrl）都是 C++ 方法，不占 Android 65536 DEX 方法数配额，不需要配置 multidex 对 5.0 以下机型做分包。原生 Java 项目稍微写大一点就要 Multidex + App ScreenStartup 优化类加载。

### 3.3 启动速度 & 内存占用

| 指标 | SOUI4（预估，基于 C++ 启动路径） | Flutter（release AOT 典型值） | Qt（Qt Quick release 典型值） |
|---|---|---|---|
| **冷启动时间** | 🏆 **最快**：纯 C++ JNI_OnLoad → SouiPlatformBridge.init → SHostWnd 构造；无需 Dart VM AOT 代码加载、也不用加载 Qt 一堆 plugins。典型中低端机 150-250ms 可交互。 | 次之：Dart VM 启动 + AOT snapshot 加载 + Dart Isolate 初始化。典型 250-450ms。 | 最慢：`QtActivity.java` 解压 Qt plugins、加载 `libQt6Core.so / libQt6Gui.so / libQt6Quick.so` + QML 引擎初始化。典型 400-800ms（工程化加了缓存可优化到 300ms+）。 |
| **空包 PSS 内存** | 🏆 **最小**：Skia + SOUI 框架 + 适配层总共约 15-25MB。 | 中间：Dart VM + Engine 约 40-60MB。 | 最大：Qt Core+GUI+QML+QPA plugins 约 60-100MB 起步。 |
| **包体积增量（单个 ABI arm64-v8a）** | 🏆 **最小**：`libsoui-android.so`（含 Skia/stb/等三方可裁剪到 ~8-15MB） | 中间：Flutter release AOT + libflutter.so 约 ~10-18MB | 最大：Qt 基础库 + QPA + platform plugins 往往 ~25-50MB 起步 |

> 以上数值为经验典型值，实际随 Skia 裁剪、Qt 模块选择、Flutter 页面数浮动。但排名基本稳定：**SOUI < Flutter < Qt**。
>
> 🧭 **补充：vs 原生 Java/Kotlin（含 Jetpack Compose）**
>
> - **冷启动**：原生「空 Activity + 纯 XML View」启动时间确实可以做到 100-180ms（Android 12+  SplashScreen 加持更快），但真实业务 App 一上 Jetpack Compose、Hilt/Koin 依赖注入、Room、WorkManager、3 个 ContentProvider 初始化 + 30 个类的 dex 加载，冷启动立刻飙升到 300ms-600ms 是普遍情况。SOUI 启动开销是「加载 libsoui-android.so（一次性 mmap） + JNI_OnLoad 调几个 C++ 初始化函数」，不依赖 ClassLoader 类加载、不需要反射实例化 Fragment/ViewModel，**真实业务场景首帧通常比 Compose 项目快 30%-50%**。
> - **内存**：空包情况下原生 Java 最小（ART 已在运行），但复杂 UI 场景（一个界面 100+ 个 View、长列表常驻、Compose Slot Table 开销）SOUI PSS 通常比原生低 15%-30%：① SWindow 是 C++ 对象（约 64-128 字节/个），Java View 对象头 + 字段 + 阴影资源引用往往 >512 字节/个；② 没有 ART 为了 GC 预留的空闲堆空间（堆占用率默认 50% 左右才触发 GC，空闲内存是"被占但不能用"的 PSS）。
> - **安装体积**：原生 Java/Kotlin 代码会被 D8/R8 编译成 dex → 再压缩进 apk，SOUI UI 代码编译在 `.so` 里用 LZMA/xz 压缩后通常比相同功能的 dex 小 20%-40%，同时 dex 方法数不超限意味着不需要 multidex 的 secondary dex 体积开销。

### 3.4 平台特性接入（系统能力/原生 View 嵌入）

| 能力 | SOUI4 for Android | Flutter | Qt |
|---|---|---|---|
| **调用原生 API（Toast/Camera/蓝牙/Share）** | 自己写 JNI：`CMainDlg::OnOK` 里 `env->FindClass` 调 Java 方法，或者封装成 C++ helper。**没有标准化通道**，自己写。 | ✅ Platform Channel + Method Channel 官方标准化，pub.dev 上 99% 常用能力插件已有人写好。 | ✅ `QAndroidJniObject`（C++ JNI 封装）+ `QCoreApplication::nativeInterface()`；常用蓝牙/位置等也有 Qt Connectivity 等模块。 |
| **嵌入 Android 原生 View（WebView / Map / VideoView）** | ⚠️ 需要在 SouiViewGroup 里手动 `addView` 到指定坐标、并用 swinx `createWindow` 返回的 HWND 对应起来。需要自己补一层 View/SWindow 同步逻辑（可实现，工程量大）。 | ✅ **PlatformView / AndroidView / TextureLayer** 官方支持，WebView、地图、相机预览直接用现成插件。 | ✅ Qt Quick `QQuickPaintedItem` + QAndroidJniObject 手动嵌入、或 Scene Graph 纹理导入。 |
| **输入法（软键盘中文/表情）** | ⚠️ 当前 `SouiView.onKeyEvent` 能把硬键盘 / commitText 里的单字符变成 WM_CHAR 送 SEdit；**但是中文输入法真正的 commitText(composing) 需要自定义 InputConnection** → 再调 JNI 送文本给 SEdit，这块要自己写。 | ✅ **官方 `EditableText` + Android InputConnection 深度对接**，中文/表情/光标/选词/复制粘贴全部原生级体验。 | ✅ `QInputMethod` + Android 平台输入法对接成熟，QLineEdit / QML TextInput 开箱即用。 |
| **权限管理 / 生命周期 / 前后台切换** | Activity/Fragment 自己在 Java 侧管理；C++ 侧用 `::PostMessage` 发自定义 WM_USER 事件回 SHostWnd 处理。体系自由但无标准化封装。 | ✅ `WidgetsBindingObserver` + `permission_handler` 插件生态成熟。 | ✅ `QGuiApplication::applicationState` + 权限 requestPermissions 封装都有。 |
| **手势 / 多点触控** | 当前映射：第 1 指=左键（WM_LBUTTON*）、第 2 指=右键（WM_RBUTTON*）；真正的捏合/双指缩放/滑动惯性需要在 `SouiView.onTouchEvent` 写手势识别器再映射消息。 | ✅ Flutter `GestureDetector` 一套跨端，所有手势官方实现完善。 | Qt Quick `PinchArea`/`Flickable`/`MultiPointTouchArea` 完善。 |

### 3.5 开发生态、工具链、热更新

| 维度 | SOUI4 for Android | Flutter | Qt |
|---|---|---|---|
| **热重载/热更新** | ❌ **没有**：改 C++ / XML 都需要重新编译 CMake + 打包 apk。 | ✅🔥 **最大卖点之一**：`r` 键秒级 Hot Reload，Widget 改完立刻看效果；热更新用 shorebird 等方案。 | ⚠️ QML 可以用 `QQmlEngine::clearComponentCache` + 重新加载远程 QML 做热更新（工程实现）；C++ 部分依然要重编。 |
| **IDE 调试体验** | Android Studio + LLDB C++ 断点能断；但没有专门的 UI Inspector，只能看 SOUI 自己在 Windows 端的 UIEDitor XML 结果。 | ✅🔥 Flutter DevTools：Widget Inspector / Performance Overlay / Timeline / Memory / 布局约束调试一应俱全。 | ✅ Qt Creator + QML Profiler / GammaRay（重量级 Qt 对象/信号/事件槽分析器）。 |
| **第三方组件生态** | ❌ 几乎为 0：SOUI 只有 Windows 端社区生态，移动端没有任何 pub/devstore 聚合地；需要的组件都得自己从 SOUI 控件改。 | ✅🔥 **pub.dev 上 4 万+ 包**，UI/网络/数据库/蓝牙/图片缓存/Lottie/推送 基本不用自己写。 | ✅ Qt Marketplace + inqlude.org + KDE Frameworks，**B 端/GUI 控件库非常厚**（Qt Charts、Qt Data Visualization、Qt 3D 等），但 C 端 App 生活服务类插件数量不如 Flutter。 |
| **CI/CD & 构建系统** | Gradle + CMake（Android Studio 标准做法），打包流程与普通 NDK 项目无差。 | `flutter build apk / ipa` 一条命令搞定；GitHub Actions / CodeMagic 官方模板多。 | CMake + Qt Creator + androiddeployqt 工具链比较"重"，CI 配置需要熟悉 Qt 交叉编译环境。 |

### 3.6 团队学习门槛 / 招人

| 角色 | SOUI4 for Android | Flutter | Qt |
|---|---|---|---|
| **有 SOUI/Win32 C++ 老员工的团队** | ⭐⭐⭐⭐⭐ 零门槛：MainDlg 消息映射 / EVENT_MAP / SWindow 都和 Windows 一样。 | 学 Dart + 响应式 Widget，对 C++ Win32 老程序员挑战大。 | 学 Qt 信号槽 / QWidget 或 QML，学习曲线中等。 |
| **纯 Android Kotlin/Java 团队（没 C++ 经验）** | ⭐ 门槛高：需要同时掌握 NDK/JNI、Win32 消息机制、SOUI DirectUI 架构三件套。 | ⭐⭐⭐⭐ 低门槛：Dart 语法像 Java+JS 混合，Widget 模型新人上手快。 | ⭐⭐ 门槛中：QML 易学、但涉及 C++ JNI 交互时需要 Qt 架构经验。 |
| **招人来源** | 几乎招不到现成 Android SOUI 工程师；只能内部从 Windows SOUI 或 C++ 工程师培养。 | 移动端最火的跨平台，人好招。 | Qt 工业圈人才不少，但互联网 C 端 Qt 工程师少。 |

### 3.7 维护风险与许可证

| 维度 | SOUI4 for Android | Flutter | Qt |
|---|---|---|---|
| **许可证** | SOUI MIT；swinx MIT；Skia BSD-3；Android NDK/STL Apache 2.0 — **全部宽松商业友好免费闭源**。 | Flutter BSD-3；Dart BSD-3；Engine BSD-3。**商业免费**。 | Qt 分**开源 (GPL/LGPL)** 和 **商业付费**。商业闭源用 Qt 要么遵守 LGPL 动态链接 + 提供重新链接方式，要么付年费（每人/年万元级人民币）。**对商业软件最不友好**。 |
| **维护主体** | 国内 SOUI 社区（小团队）。遇到 Android 平台的坑基本要自己填。 | Google 官方全力维护。 | The Qt Company 商业公司维护。 |
| **弃坑风险** | swinx 上的 Android 平台抽象层要自维护；但 SHostWnd 代码和 Windows 共用，所以即使未来改架构，业务代码不用动。 | 几乎无弃坑风险。 | 商业版长期支持承诺；开源版跟随 Qt 公司策略。 |

### 3.8 线程模型与跨语言/跨线程交互复杂度（SOUI 对比 Qt 的核心差异化优势）

> 一句话结论：**SOUI = 1 条 Java 主线程跑完整 UI 消息循环；Qt for Android = 2 条 UI 线程（Qt GUI 线程 + Java 主线程）天然不同步，交互要切线程 + 加锁，代码和心智负担翻倍。**

| 维度 | SOUI4 for Android | 原生 Java/Kotlin | Flutter | Qt for Android |
|---|---|---|---|---|
| **UI 线程数量** | 🏆 **1 条 = Java 主线程（Main Looper）**：`::SendMessage` / `::PostMessage` / `SWindow::Invalidate` / SHostWnd 回调全在同一条线程；swinx 消息循环直接挂在主线程 Looper 的 MessageQueue 上，不需要起额外线程。 | ✅ **1 条 = Java 主线程**：Android 标准模型，View/Compose 操作都在主线程。 | ✅ **逻辑上 1 条主 UI 线程（Dart Isolate 主事件循环）**，但和 Android Platform 线程（Java 主线程）是两条，需要 Channel 通信。 | ❌ **2 条 UI 线程双轨并行**：① Java 主线程（管 Activity/View/Surface/生命周期分发）② Qt GUI 线程（独立 `QEventLoop`，管 QGuiApplication/QWindow/QWidget/QQuickItem 的事件分发与渲染）。**两条线程天然不同步，没有公共锁。** |
| **跨线程同步成本（原生平台 ↔ UI 代码）** | 🏆 **几乎为 0**：要调 Java API（Toast/startActivity/蓝牙回调），直接在当前（同一条）线程 JNI `CallVoidMethod` / `FindClass`，不需要 `runOnUiThread`、不需要 `QMetaObject::invokeMethod`、不需要 `std::mutex`。反过来 Java 回调 C++ 也只需要直接 JNI Call → C++ SHostWnd ::PostMessage。 | ✅ 同线程 0 成本；如果是 C++ NDK 异步回调到 UI，需要 `Activity.runOnUiThread` 或 `Handler.post`。 | ⚠️ Dart Isolate → Platform Channel 切换线程 + 参数序列化/反序列化；Platform → Dart 同样要排队，不能立即取返回值。 | ❌ **巨痛，跨 2 条 UI 线程必排队程**：Java 主线程想改 QLabel 文字/调 QML 函数 **必须** `QMetaObject::invokeMethod(qmlRoot, "setText", Qt::QueuedConnection, Q_ARG(QString, s))`；反过来 Qt GUI 线程想 `startActivity` / 弹 AlertDialog **必须** `activity.runOnUiThread(...)` + 先 `AttachCurrentThread` 把 JNI Env 绑到当前 Java 线程。两边共享状态还要再加 `QMutex`/`std::mutex`，时序错一次就死锁/ANR/崩。 |
| **生命周期事件分发（onResume/onPause/onDestroy）** | 生命周期在 Java 主线程触发 → 直接 JNI `nativeOnResume` → C++ 层 `::PostMessage(WM_USER+1)` 回 SHostWnd，**分发路径全同线程，延迟 <1ms，时序确定**。 | 主线程直接分发到 Activity/Fragment/Composable，延迟 <1ms。 | 生命周期到 Java 主线程 → Embedder → 切 Platform Channel → 发到 Dart Isolate → `WidgetsBindingObserver.didChangeAppLifecycleState`，有排队延迟。 | ❌ **延迟最高、时序最容易出 bug**：onResume 先到 Java 主线程 → JNI 入 Qt → 投递到 Qt GUI 线程的 QEventLoop 队列排队（前面可能有 N 个 QTimer/QPaint 事件）→ 等下一次 QEventLoop 才派发；用户肉眼可见切后台后 200ms 才把动画停掉、或者切回来画面闪白。 |
| **定时器/延时任务管理** | 🏆 **一套 Looper 管到底**：`SNS::SetTimer(m_hWnd, ID, ms, nullptr)` → swinx → `SouiPlatformBridge.setTimer` → Java 主线程 `Handler.postDelayed` → 到期 C++ 发送 `WM_TIMER`；和原生 View 的 Choreographer/Handler 节拍一致，不会出现"我定时器触发但 View 还没 draw"的时序漂移。 | ✅ 主线程 Handler / Kotlin Coroutines Main Dispatcher / WorkManager 一套。 | Dart `Timer.periodic` + 原生 `AlarmManager` / WorkManager 两套。 | ❌ **两套定时器体系，跨线程取消/同步很痛**：Qt 侧 `QTimer`（跑在 Qt GUI 线程）+ Java 侧 `Handler.postDelayed`（跑在 Java 主线程），要做"倒计时到 0 后两个线程同时停止刷新 + 启动 Activity"这类业务，需要两把锁 + 两条取消路径，漏一个就内存泄漏/ANR。 |
| **典型痛点举例** | 「点击按钮弹 Toast」代码一行写完：`JNIEnv* env = AndroidPlatformAPI::getJNIEnv(); env->CallStaticVoidMethod(bridgeClass, methodToast, jstr);` — 无需切线程。 | 对应：`Toast.makeText(this, "ok", LENGTH_SHORT).show();` 一行。 | 对应：写一个 `MethodChannel.invokeMethod("showToast")` + 原生注册 + Dart 侧 await，三端代码配合。 | ❌ **对应需要 3 处**：① 按钮槽函数（在 Qt GUI 线程）里先 `QMetaObject::invokeMethod` 切到主线程执行 JNI；② JNI 里要 `AttachCurrentThread`（因为当前是 Qt 线程不是主线程，Env 不绑定）；③ `runOnUiThread` 真正执行 Toast 代码。写错一处要么 Toast 不弹、要么崩 `JNI DETECTED ERROR IN APPLICATION: JNI FindClass called with pending exception`，要么出现竞态条件。 |

---

## 4. SOUI4 Android 方案的 9 大优点（相对 Flutter/Qt/原生 Java）

1. **已有 SOUI（Windows DirectUI）业务代码 90%+ 源码级复用，移植成本为三者最低。**  
   这是唯一的、决定性的优点——如果你手上有一个运行了 10 年的 SOUI Windows 客户端要上 Android，用 Flutter 或 Qt 都等于 **把 UI + 业务交互全部重写一遍**，用 SOUI 则是：MainDlg 直接编、布局 XML 直接拷、资源 zip 直接用、EVT 回调都能跑。移植项目级 3-6 周可产出可交互 demo，Flutter/Qt 路线往往需要 3-6 个月。

2. **C++ 业务 + SOUI UI 层全部编译进同一个 .so，天然零 IPC、零数据拷贝。**  
   常见于工业控制、金融终端、音视频工具类 C++ 项目：核心算法/Camera 数据流都是 C++ buffer，直接传给 SImage / SPanel 指针就能画，不需要像 Flutter 走 ffi 再做 TypedData 内存拷贝。

3. **启动最快、内存最小、包体积最小。**  
   三者（加上原生 Compose 项目的真实业务场景）中 SOUI 启动速度和安装包增量都是第一名，适合做：工具类 APK、工厂预装、嵌入式终端、运营商定制机 App（对首次启动 <300ms、包大小硬性要求严的场景）。

4. **Win32 消息 + SHostWnd 双端一致性极高，bug 少。**  
   一旦 Android 端验证通过的业务流程，Windows 端基本不会出现不同 bug；反过来 Windows 端修过的 SOUI 控件 bug 自动同步。Flutter/Qt 往往双端 UI 都要各自 QA 回归。

5. **许可证全宽松 MIT/BSD，商业永久免费无 LGPL 动态链接义务。**  
   对需要闭源商业分发、又不想付 Qt 年费、也不想踩 LGPL 条款雷区的公司，是巨大优势。

6. **不引入 Dart VM / QML 引擎，技术栈"纯 C++ + Kotlin/Java 两个世界"，边界清晰。**  
   很多传统客户端团队不愿意再引入一门新语言（Dart/QML）来"增加一层技术债"，SOUI 方案的认知负担是三者中对 Win32 C++ 团队最低的。

7. **可以按页面/模块逐步"蚕食式"接入现有 Android App。**  
   因为本质就是一个 `com.soui.SouiViewGroup`，可以嵌在任何原生 Fragment / Activity / RecyclerView Item 里；一个页面用 SOUI、另外 9 个页面用原生 Jetpack Compose，完全没有问题。Flutter/Qt 往往要么整个 App 用它、要么 hybrid 混合栈（Flutter Boost 等）引入额外复杂度。

8. **UI 全链路 C++ 实现，对比纯 Java/Kotlin/Compose：零 ART GC 停顿、CPU cache 友好、DEX 方法数压力更小。**  
   详见 3.2/3.3 详细对比：① SOUI SWindow 全部是 C++ 堆对象，**不会触发 ART GC 回收停顿**（7x24 小时长运行 App 稳定性的杀手级优势）；② 复杂界面/长列表场景 SWindow 内存布局连续，cache hit 更高，PSS 通常比原生 UI 低 15-30%；③ 所有 SOUI 控件方法不占 65536 DEX 方法数配额，不用 multidex，冷启动更快。对于长时间运行、不允许卡顿的工业/金融自助终端 App，这个特性直接拍板。

9. **线程模型最简：单 UI 线程 = Java 主线程，跨语言/跨平台 API 调用几乎零同步成本。**  
   相比 **Qt for Android 的双 UI 线程（Qt GUI + Java 主线程）**，SOUI 把 Win32 消息循环和原生 View 的事件分发放在同一条线程执行：调 Toast、startActivity、弹 Dialog 直接 JNI 调，不需要 `QMetaObject::invokeMethod` 排队切线程、不需要 `AttachCurrentThread` 绑 Env、不需要两把 QMutex/std::mutex 保护共享状态、也不会出现 onResume 切前台 Qt GUI 线程还没收到事件的时序漂移。**相同功能 Qt 需要 3 处切线程+加锁的代码，SOUI 一行 JNI 调用就搞定，bug 率显著下降。**

---

## 5. SOUI4 Android 方案的 7 大缺点（相对 Flutter/Qt/原生 Java）

1. **没有移动端生态、几乎所有插件需要自己写。**  
   这是最大的坑：微信登录、支付宝支付、微信分享、扫码、推送、Lottie 动画、WebView、高德地图这些 C 端 App 标配能力，Flutter 上 pub.dev 搜一下 5 分钟接入，SOUI 上每个都需要自己写 JNI 桥接 + 在 C++ SHostWnd 里封装一层事件接口。对 C 端 App（非工具/非工业）这个缺点可能一票否决。  
   ⚠️ 折中提示：**即便同样要自写 JNI 桥接，SOUI 单 UI 线程同域的设计也比 Qt 双 UI 线程好写 3 倍**——不需要 `QMetaObject::invokeMethod` + `AttachCurrentThread` + `runOnUiThread` 三处配合，直接在当前线程 JNI 调用原生方法即可。

2. **没有热重载，开发效率明显低于 Flutter。**  
   改一个按钮颜色 → 跑 `gradlew assembleDebug` → 重新安装 APK。Flutter 是一个热重载快捷键，0.5 秒看到效果。在 UI 需要大量反复打磨的产品里，SOUI 团队产出速度至少慢 2-3 倍。

3. **输入法、手势、Accessibility（无障碍）这些平台交互细节需要自己填坑。**  
   中文输入法 composing、双指捏合、复制粘贴悬浮菜单、屏幕阅读器 TalkBack、横屏分屏、折叠屏、系统字体缩放……Flutter/Qt 花了 10 年把这些平台细节打磨到 95 分；SOUI 当前框架只打通了基础 WM_KEYDOWN/WM_CHAR 管线，这些细节要团队用 Android 经验补。

4. **性能在"复杂动画 + 长列表"场景落后于 Flutter/Qt Quick。**  
   当前实现是 "Skia → Bitmap → Canvas.drawBitmap"，一帧完整刷新等于要把全屏像素从 CPU 内存传到 GPU 纹理。Flutter 和 Qt Quick 都是直接在 GPU Surface 上画，没有这份额外的 Bitmap 拷贝。做 feed 流、Lottie 动画、120Hz 高刷屏体验，SOUI 会吃亏。（可优化方向：接 OpenGL Skia 后端 + SurfaceTexture 渲染，工程量大）

5. **招人难、维护成本高。**  
   市面上找不到现成的 "Android SOUI" 工程师；必须由懂 Windows SOUI + 懂 Android NDK/JNI 的复合型人才维护。一旦核心成员离职，项目接手难度远高于 Flutter 项目。

6. **非 Win + Android 双端场景，劣势立刻放大。**  
   老板说"我们下季度还要上个 iOS 版"——Flutter/Qt 可以基本一套代码直接上，SOUI 方案你得再写一整个 iOS swinx 后端（UIView/CALayer 做映射、GCD 做 SetTimer、UIEvent 转 WM_LBUTTONDOWN 等），工程量 ≈ 再做一遍本库。

7. **没有成熟 UI 调试工具。**  
   Flutter 有 Widget Inspector 点击任意像素就能看到对应的 Widget 源码、布局约束、render 时间。SOUI 需要用 Windows 端 UIEDitor 离线看 XML，真机端没有"选中-高亮-改属性-实时预览"的开发手段。UI 还原/调试效率显著偏低。

---

## 6. 选型决策矩阵（4 类推荐 vs 4 类不推荐）

### ✅ 4 类强烈推荐采用 SOUI4 Android 方案的场景

1. **你已经有 5 万行以上 SOUI（Windows DirectUI）桌面端业务代码，现在要做一个 Android 版本**  
   业务代码复用率碾压 Flutter/Qt，ROI 最高。

2. **B 端工具/工业终端/金融自助机/运营商定制机 App（非 C 端消费级）**  
   对包体积、冷启动、内存敏感，但不需要微信/支付宝登录、Lottie 动画等生态能力——这正是 SOUI 方案舒适区。

3. **App 核心是 C/C++（音视频编解码、机器视觉、测量仪器、加密认证）的"算法壳 UI"**  
   C++ 代码直接链接、数据零拷贝、交互用 SHostWnd 足够表达，SOUI 最顺手。

4. **商业闭源软件，对许可证条款敏感，不想付 Qt 年费也不想踩 LGPL 动态链接限制**  
   SOUI + swinx + Skia 全部 MIT/BSD 宽松，商业无负担。

### ❌ 4 类不推荐 SOUI4 Android 方案的场景

1. **从 0 开始的新 C 端 App（社交/电商/生活服务/内容型产品）**  
   生态、热重载、动效、输入法、微信登录推送这些你后面 100% 会遇到，Flutter 用半年的坑 SOUI 可能要填 2 年。

2. **同时需要 Android + iOS + Web + macOS + Windows 多端，且没有 SOUI 历史包袱**  
   Flutter 或者 Qt 在"真·多端"方面是正式支持的，SOUI 目前官方只有 Win + Android 两条线。

3. **团队没有 Windows SOUI 经验，也没有 C++/NDK 背景**  
   技术门槛过高，容易死在第 1 个里程碑前。

4. **要求 120Hz 流畅滚动 + Lottie 动效 + 原生手势/捏合双指 + 中文字形富文本**  
   不是不能做，而是需要投入的工程量（Skia GPU 管线、InputConnection、手势识别、Lottie 移植）已经远大于直接选 Flutter 的代价。

---

## 7. 折中路线（给纠结的团队）

如果你的场景介于两者之间，可以考虑 **"SOUI 做核心业务页 + 原生 Kotlin/Jetpack Compose 做 C 端生态页"的混合栈**：

```
┌─────────────────────────────────────────────────────────────┐
│  Android 原生 Activity / Jetpack Compose 做：               │
│  - 登录页（微信/手机号验证码/推送授权）                       │
│  - WebView 页、协议页                                       │
│  - 高德地图 + 定位页                                        │
│  - 设置页 / 个人中心（这些页面用 Compose 写得快，复用生态）   │
├─────────────────────────────────────────────────────────────┤
│  点击"进入核心工作台" → 启动一个 SouiHostActivity：          │
│  里面一个全屏 SouiViewGroup → initSouiLayout("layout:main")  │
│  = 核心 C++ SHostWnd 业务全部复用 Windows 代码              │
│  （例如：订单办理 / 设备调试 / 交易界面）                    │
└─────────────────────────────────────────────────────────────┘
```

这种混合架构在"既有 SOUI 历史包袱，又必须接入微信登录/推送/地图等生态"的场景下，通常是性价比最高的：**只让 SOUI 承担它最擅长的"老业务代码移植"那部分，剩下的交给原生/Kotlin/Compose**。目前来看，这是大多数"传统 Windows 客户端厂商 + 现在要做 Android 版"的真实最优解。
