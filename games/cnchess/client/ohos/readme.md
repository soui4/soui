# SOUI 中国象棋（cnchess）鸿蒙（OHOS）版

本目录是 **cnchess 中国象棋** 的鸿蒙（HarmonyOS NEXT）移植工程，基于 `soui-ohos-lib` 的 N-API 桥接，在鸿蒙上运行同一份 SOUI C++ 业务代码。

> 与 `soui-ohos-demo` 不同，本工程**不**把 UI 业务代码放在 `entry/src/main/cpp` 内，而是复用 `games/cnchess/client` 下的共享 C++ 源码（`MainDlg.cpp`、`ChessGame.cpp` 等），仅在此处的 `cpp/CMakeLists.txt` 中通过 GLOB 引入，并附加鸿蒙专属入口 `ohos_entry.cc`。

## 工程结构

```
games/cnchess/client/ohos/
├── AppScope/                              # 应用级配置（包名、图标）
├── entry/                                # 主模块
│   ├── src/main/
│   │   ├── cpp/
│   │   │   ├── CMakeLists.txt            # 构建脚本（计算 SOUI 根目录，引入 soui-ohos-lib + client 源码）
│   │   │   └── types/                     # N-API 类型声明（libcnchess / libsoui4ohos）
│   │   ├── ets/
│   │   │   ├── entryability/EntryAbility.ets   # UIAbility 入口，初始化 SOUI 并启动
│   │   │   ├── pages/Index.ets                 # 主页面，放置 SouiScreen
│   │   │   └── soui/                           # soui-ohos-lib 的 ArkTS 适配层
│   │   │       ├── SouiPlatformBridge.ets      # 全局桥接单例（生命周期/窗口/定时器/IME/剪切板…）
│   │   │       ├── SouiScreen.ets              # SOUI 渲染宿主组件
│   │   │       ├── SouiSurface.ets / SouiSurfaceView.ets
│   │   │       ├── NativeEditView.ets          # 输入法编辑框
│   │   │       └── AudioPlayer.ets             # 音频播放
│   │   └── resources/rawfile/
│   │       ├── uires/                     # SOUI 应用资源（布局、皮肤、字符串、动画）
│   │       └── soui_sys_res/              # SOUI 系统资源（控件皮肤）
│   └── build-profile.json5
├── build-profile.json5
└── hvigorfile.ts
```

C++ 业务层（`games/cnchess/client`）：

```
games/cnchess/client/
├── ohos_entry.cc        # 鸿蒙入口：实现 Soui4OhosEntry 四虚函数 + N-API 模块注册
├── MainDlg.cpp/.h       # 主窗口（SHostWnd 子类）
├── ChessGame.cpp/.h     # 棋局逻辑
├── ChessBoard.cpp/.h    # 棋盘自定义窗口
├── ...（与 android / PC 共享的业务源码）
```

## 架构与启动流程

1. **EntryAbility.onCreate**（`entry/src/main/ets/entryability/EntryAbility.ets`）
   - `import cnchess from 'libcnchess.so'` 触发 N-API 模块加载（静态构造 `theApp` → `InitSoui4OhosEntry` 注册业务入口）。
   - 调用 `cnchess.init()` 完成模块注册。
   - `SouiPlatformBridge.instance().init(this.context)` 加载 `libsoui4ohos.so` 并注册平台 API。
   - `SouiPlatformBridge.instance().startSouiApp()` 启动 SOUI 应用。

2. **Index.ets**（`entry/src/main/ets/pages/Index.ets`）
   - 放置 `SouiScreen({ screenId: 1, layoutName: 'layout:dlg_main' })`。
   - SouiScreen 尺寸就绪后通过 `SouiPlatformBridge.registerScreen` → C++ `ScreenStartup` 创建并显示 `CMainDlg`。

3. **C++ 层（`ohos_entry.cc`）**
   - `CnchessOhosApp::InitApp`：创建 `SApplication`，注册自定义皮肤类（`SSkinAni`/`SSkinPiece`/`SSkinBoard`）与窗口类（`SGifPlayer`/`CChessBoard`/`CChessPiece`），配置 `Render_Skia` + `ImgDecoder_Stb`。
   - 资源加载：通过 `NativeResourceManager` 从 **HAP rawfile 直读**（`cfg.SetSysResOhosRawFile` / `SetAppResOhosRawFile`），**不再需要把 uires / soui_sys_res 复制到 filesDir**。
   - `CnchessOhosApp::ScreenStartup`：创建 `CMainDlg` 并显示主窗口。

## 资源加载

采用 **rawfile 直读** 方式（与早期 soui-ohos-demo 复制资源到 filesDir 的方案不同）：

- `rawfile/uires/`、`rawfile/soui_sys_res/` 在构建时打入 HAP。
- C++ 通过 `OH_ResourceManager_InitNativeResourceManager` 取得 `NativeResourceManager`，交给 `SAppCfg::SetSysResOhosRawFile` / `SetAppResOhosRawFile` 直接读取，省去运行期复制。

## 构建运行

### 前置条件

1. DevEco Studio（HarmonyOS NEXT 版本）
2. HarmonyOS NEXT SDK（API 版本以 `entry/build-profile.json5` 中 `compatibleSdkVersion` 为准）
3. 工程依赖 `soui-ohos-lib`，位于 SOUI 源码根目录 `soui-ohos-lib/`（与 `games/` 同级）

### 构建步骤

1. 用 DevEco Studio 打开 `games/cnchess/client/ohos`。
2. 等待 hvigor 同步；`cpp/CMakeLists.txt` 会自动定位 SOUI 根目录并编译 SOUI 核心 + `soui-ohos-lib` + cnchess 业务层（首次耗时较长）。
3. 连接鸿蒙真机或启动模拟器（phone / tablet / 2in1）。
4. 点击 Run，或执行 `hvigorw assembleHap` 构建 HAP 并安装。

### 签名配置

默认未配置签名。请在 DevEco Studio 的 **File → Project Structure → Signing Configs** 中配置自动签名（需鸿蒙开发者账号）。

## 关键文件说明

| 文件 | 作用 |
|------|------|
| `entry/src/main/cpp/CMakeLists.txt` | 计算 SOUI 根目录，引入 `soui-ohos-lib` + `games/cnchess/client` 共享源码，定义宏（`SOUI_OHOS`/`__OHOS__`/`__MOBILE__`/`ENABLE_VIRTUAL_HWND` 等） |
| `ohos_entry.cc`（client 根目录） | 实现 `Soui4OhosEntry` 四虚函数 + `cnchess` N-API 模块注册 |
| `entry/src/main/ets/entryability/EntryAbility.ets` | 应用入口，初始化并启动 SOUI |
| `entry/src/main/ets/pages/Index.ets` | 主页面，放置 `SouiScreen` |
| `entry/src/main/ets/soui/SouiPlatformBridge.ets` | ArkTS 桥接单例（对应 `soui-ohos-lib` 的 N-API 层） |
| `entry/src/main/ets/soui/SouiScreen.ets` | SOUI 渲染宿主组件 |
| `entry/src/main/resources/rawfile/uires/` | SOUI 应用资源 |
| `entry/src/main/resources/rawfile/soui_sys_res/` | SOUI 系统资源 |

## 与 Android 版的对应关系

| 鸿蒙（OHOS） | Android |
|------|---------|
| `ohos_entry.cc`（`CnchessOhosApp`） | `android_entry.cc`（`CnchessAndroidApp`） |
| `EntryAbility.ets` | `GameActivity.java` |
| `Index.ets`（`SouiScreen`） | `SouiScreen`（Java 侧） |
| `SouiPlatformBridge.ets` | `SouiPlatformBridge.java` |
| `SouiSurface.ets` / `SouiSurfaceView.ets` | `SouiSurface.java` |
| rawfile 直读（`SetSysResOhosRawFile`） | APK assets 直读（`SetSysResAndroidAsset`） |
