# SOUI for OpenHarmony Demo

基于 `soui-ohos-lib` 的 HarmonyOS NEXT 测试工程，演示如何使用 SOUI 跨平台 UI 框架开发鸿蒙应用。

## 工程结构

```
soui-ohos-demo/
├── AppScope/                     # 应用级配置
├── entry/                        # 主模块
│   ├── src/main/
│   │   ├── cpp/                  # C++ 业务层
│   │   │   ├── CMakeLists.txt    # 构建脚本（链接 soui-ohos-lib）
│   │   │   ├── demo_native.cpp   # Soui4OhosEntry 实现 + N-API 注册
│   │   │   ├── MainDlg.h/cpp     # 主对话框（SHostWnd 子类）
│   │   │   ├── SouiRealWndHandler.h/cpp  # RealWnd 处理器
│   │   │   └── types/             # N-API 类型声明
│   │   ├── ets/
│   │   │   ├── entryability/     # EntryAbility（初始化 SOUI）
│   │   │   ├── pages/Index.ets   # 主页面（放置 SouiScreen）
│   │   │   └── soui/             # soui-ohos-lib 的 ArkTS 适配层
│   │   └── resources/
│   │       ├── rawfile/          # SOUI 资源（uires + soui_sys_res）
│   │       └── base/             # ArkUI 资源
│   └── build-profile.json5
├── build-profile.json5
└── hvigorfile.ts
```

## 架构说明

### 启动流程

1. **EntryAbility.onCreate**
   - 注入 `soui4ohos` 到 `globalThis.soui4ohosNative`
   - 调用 `souidemo.init()` 触发 Soui4OhosEntry 注册
   - 调用 `SouiPlatformBridge.init(context)` 注册平台 API
   - 异步复制 rawfile 资源到 filesDir，然后调用 `SouiPlatformBridge.startSouiApp()`

2. **Index.ets**
   - 放置 `SouiScreen({ screenId: 1, layoutName: 'layout:dlg_main' })`
   - SouiScreen 首次尺寸就绪后调用 `registerScreen` → C++ `ScreenStartup`
   - C++ 创建 `CMainDlg`（SHostWnd 子类）并显示

3. **C++ 层 (demo_native.cpp)**
   - `SouiOhosApp::InitApp` 创建 SApplication，配置 Skia 渲染 + Stb 图像解码
   - 资源加载方式：文件模式（ArkTS 从 rawfile 复制到 filesDir，C++ 通过 SetAppResFile 加载）
   - `SouiOhosApp::ScreenStartup` 创建 CMainDlg 并显示主窗口

### 资源加载

当前采用**文件方式**加载 SOUI 资源：
- rawfile 中的 `uires/` 和 `soui_sys_res/` 在启动时复制到 `filesDir/`
- C++ 通过 `SetAppResFile` / `SetSysResFile` 指向 filesDir 路径

未来集成 OHOS Rawfile 资源管理器后，可直接从 rawfile 读取，无需复制。

## 构建运行

### 前置条件

1. DevEco Studio（HarmonyOS NEXT 版本）
2. HarmonyOS SDK（API 26+）
3. 工程依赖 `soui-ohos-lib`（位于 `d:\work\soui4\soui-ohos-lib`）

### 构建步骤

1. 用 DevEco Studio 打开 `d:\work\soui4\demos\soui-ohos-demo`
2. 等待 hvigor 同步完成（首次会编译 SOUI 核心 + soui-ohos-lib，耗时较长）
3. 连接鸿蒙设备或启动模拟器
4. 点击 Run 或 `hvigorw assembleHap` 构建并安装

### 签名配置

默认未配置签名。请在 DevEco Studio 的 File → Project Structure → Signing Configs 中配置自动签名。

## 关键文件说明

| 文件 | 作用 |
|------|------|
| `entry/src/main/cpp/demo_native.cpp` | 实现 Soui4OhosEntry 四个虚函数 + N-API 模块注册 |
| `entry/src/main/cpp/MainDlg.h/cpp` | 主对话框，演示按钮事件、定时器 |
| `entry/src/main/ets/entryability/EntryAbility.ets` | 应用入口，初始化 SOUI |
| `entry/src/main/ets/pages/Index.ets` | 主页面，放置 SouiScreen |
| `entry/src/main/ets/soui/` | soui-ohos-lib 的 ArkTS 适配层（从 soui-ohos-lib 复制） |
| `entry/src/main/resources/rawfile/uires/` | SOUI 应用资源（布局、皮肤、字符串） |
| `entry/src/main/resources/rawfile/soui_sys_res/` | SOUI 系统资源（控件皮肤） |

## 与 Android Demo 的对应关系

| OHOS | Android |
|------|---------|
| `demo_native.cpp` (SouiOhosApp) | `demo_native.cpp` (SouiAndroidApp) |
| `MainDlg.h/cpp` | `MainDlg.h/cpp` |
| `SouiRealWndHandler.h/cpp` | `SouiRealWndHandler.h/cpp` |
| `EntryAbility.ets` | `HomeActivity.java` |
| `Index.ets` (SouiScreen) | `activity_home.xml` (SouiScreen) |
| `SouiPlatformBridge.ets` | `SouiPlatformBridge.java` |
| `SouiSurface.ets` | `SouiSurface.java` |
| `SouiScreen.ets` | `SouiScreen.java` |
