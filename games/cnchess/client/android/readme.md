# SOUI 中国象棋（cnchess）Android 版

本目录是 **cnchess 中国象棋** 的 Android 移植工程，基于 `soui-android-lib` 的 JNI 桥接，在 Android 上运行同一份 SOUI C++ 业务代码。

## 工程结构

```
games/cnchess/client/android/
├── app/
│   ├── build.gradle                    # Gradle 配置（namespace、minSdk、CMake 参数）
│   ├── src/main/
│   │   ├── AndroidManifest.xml         # 声明 GameActivity 为启动 Activity
│   │   ├── java/com/soui/cnchess/
│   │   │   ├── CnchessApplication.java  # Application
│   │   │   └── GameActivity.java        # 启动 Activity，承载 SouiScreen
│   │   ├── cpp/CMakeLists.txt          # native 构建（soui4_android.cmake，统一编译 soui-android-lib + client 源码）
│   │   └── assets/
│   │       ├── uires/                  # SOUI 应用资源（布局、皮肤、字符串、动画）
│   │       └── soui_sys_res/           # SOUI 系统资源（控件皮肤）
│   └── release/                        # 已生成 APK（cnchess-soui.apk）
├── build.gradle / settings.gradle      # 根 Gradle，引入 soui-android-lib 模块
└── gradlew.bat
```

C++ 业务层与鸿蒙、PC 共享，位于 `games/cnchess/client/`（`android_entry.cc` 为 Android 专属入口）。

## 架构与启动流程

1. **GameActivity.onCreate**（`app/src/main/java/com/soui/cnchess/GameActivity.java`）
   - 加载 SOUI native 库，创建 `SouiScreen` 承载 SOUI 渲染。
   - 通过 `SouiPlatformBridge` 初始化并启动 SOUI 应用。

2. **SouiScreen**
   - 尺寸就绪后触发 C++ 侧 `ScreenStartup`，创建并显示 `CMainDlg`（主窗口）。

3. **C++ 层（`android_entry.cc`）**
   - `CnchessAndroidApp::InitApp`：创建 `SApplication`，注册自定义皮肤类（`SSkinAni`/`SSkinPiece`/`SSkinBoard`）与窗口类（`SGifPlayer`/`CChessBoard`/`CChessPiece`），配置 `Render_Skia` + `ImgDecoder_Stb`。
   - 资源加载：通过 `AAssetManager` 从 **APK assets 直读**（`cfg.SetSysResAndroidAsset` / `SetAppResAndroidAsset`），无需把资源解压到文件系统。
   - `CnchessAndroidApp::ScreenStartup`：创建 `CMainDlg` 并显示。

## 资源加载

采用 **APK assets 直读** 方式：

- `assets/uires/`、`assets/soui_sys_res/` 在打包时进入 APK。
- C++ 通过 NDK `AAssetManager`（由 Java 侧传入）交给 `SAppCfg::SetSysResAndroidAsset` / `SetAppResAndroidAsset` 直接读取。

## 构建运行

### 前置条件

1. Android Studio（含 Android SDK）
2. NDK（与 `app/build.gradle` 中 `externalNativeBuild.cmake` 版本匹配，CMake 3.18.1）
3. `soui-android-lib` 模块（与 `games/` 同级，由 Gradle `settings.gradle` 引入）

### 构建步骤

1. 用 Android Studio 打开 `games/cnchess/client/android`。
2. Gradle 同步后会编译 `soui-android-lib` 与 cnchess native（经由 `app/src/main/cpp/CMakeLists.txt`，CMake 参数 `SOUI_ROOT_DIR` 指向 SOUI 根目录）。
3. 连接 Android 设备或启动模拟器（**minSdk 28**，即 Android 9）。
4. 点击 Run，或执行 `./gradlew assembleDebug` 生成 APK 安装运行。

### 说明

- 应用包名 `com.soui.cnchess`，`GameActivity` 为 LAUNCHER（见 `AndroidManifest.xml`）。
- `app/release/cnchess-soui.apk` 为已生成的发布包，可直接安装验证。

## 关键文件说明

| 文件 | 作用 |
|------|------|
| `app/build.gradle` | Gradle/CMake 配置，传递 `SOUI_ROOT_DIR`/`CNCHESS_CLIENT_DIR` 等给 native 构建 |
| `app/src/main/AndroidManifest.xml` | 声明 `GameActivity` 启动 Activity、`INTERNET` 权限 |
| `app/src/main/java/com/soui/cnchess/GameActivity.java` | 启动 Activity，承载 SOUI |
| `app/src/main/cpp/CMakeLists.txt` | native 构建脚本 |
| `android_entry.cc`（client 根目录） | 实现 `Soui4AndroidEntry` 四虚函数 + JNI 注册 |
| `app/src/main/assets/uires/` | SOUI 应用资源 |
| `app/src/main/assets/soui_sys_res/` | SOUI 系统资源 |

## 与鸿蒙（OHOS）版的对应关系

见 [鸿蒙版 readme](../ohos/readme.md) 末尾的对照表。两者共享 `games/cnchess/client` 下的 C++ 业务源码，仅入口与桥接层不同。
