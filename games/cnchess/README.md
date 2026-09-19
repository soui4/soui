# 中国象棋游戏（cnchess，SOUI4 跨平台示例）

将传统中国象棋游戏移植到 **SOUI4** 框架的项目，是 SOUI4 **一套 C++ 代码覆盖六端** 的官方示例：
Windows / Linux / macOS / iOS / Android / OpenHarmony（OHOS）共用同一份 C++ 业务与 UI 逻辑，
各端均已测试通过。

## 项目结构

```
cnchess/
├── algorithm/              # 象棋算法与 AI 引擎（纯标准 C++，不依赖 windows.h）
│   ├── ChsAIEngine.h/.cpp  # 迭代加深 + PVS + 置换表 + 杀手/历史启发（C++ 移植，MIT）
│   ├── Chessman.h/.cpp     # 棋子定义与走子规则
│   ├── ChessLayout.h/.cpp  # 棋局布局与状态管理
│   └── ChessBook.h/.cpp    # 开局/残局棋谱
├── client/                 # 客户端共享 C++ 业务代码 + 桌面/iOS 入口
│   ├── main.cc             # 统一入口：桌面（Win/Linux/macOS）与 iOS 经 swinx 归一为 WinMain
│   ├── android_entry.cc    # Android 专属入口（实现 Soui4AndroidEntry 四虚函数 + JNI 注册）
│   ├── ohos_entry.cc       # 鸿蒙专属入口（实现 Soui4OhosEntry 四虚函数 + N-API 注册）
│   ├── MainDlg.cpp/.h      # 主窗口（SHostWnd 子类）
│   ├── ChessGame.cpp/.h    # 棋局逻辑
│   ├── ChessBoard.cpp/.h   # 棋盘自定义窗口
│   ├── ChessPiece.cpp/.h   # 棋子自定义窗口
│   ├── LoginDlg / PeaceReqDlg / PeaceAckDlg / myprofile / CnchessSkin …  # 对话框与皮肤
│   ├── LobbyHandler / EndgameHandler / SGameTheme / WebSocketClient / WsClientListener …  # 网络与大厅
│   ├── ThemeDownloadManager.h/.cpp  # 主题热换（WebSocket 分块下载 + MD5 校验）
│   ├── uires/              # 桌面版 UI 资源（布局/皮肤/字符串/动画，XML 描述）
│   ├── pc_theme/           # 桌面端本地皮肤资产（时钟/数字等局部皮肤）
│   ├── mobile_theme/       # 移动端本地皮肤资产（时钟/数字等局部皮肤）
│   └── CMakeLists.txt      # 生成 cnchess_client 目标
├── client/android/         # Android 工程（JNI 桥接，见 android/readme.md）
├── client/ohos/            # 鸿蒙工程（N-API 桥接，见 ohos/readme.md）
├── server/                 # 服务器程序（C++，详见下）
└── CMakeLists.txt          # 顶层 CMake 配置
```

> 各平台详细结构、构建与运行步骤见：
> - `client/readme.md`（桌面 / iOS）
> - `client/android/readme.md`
> - `client/ohos/readme.md`

## 跨平台机制（一套 C++，六端运行）

同一份 `MainDlg` / `ChessGame` / `ChessBoard` / `algorithm` 经**三个薄入口**接入不同宿主：

- **桌面 / iOS**：`client/main.cc` 由 `swinx` 提供事件泵与窗口系统，归一为 `WinMain`；
  iOS 经 `swinx_ios_entry` 托管进 `UIApplicationMain`。
- **Android**：`client/android/` 经 `soui-android-lib`（JNI）把 `SApplication` 托管进 `GameActivity`。
- **OHOS**：`client/ohos/` 经 `soui-ohos-lib`（N-API）托管进 `EntryAbility`。

平台差异被 `swinx` + 宿主 lib 吸收；核心绘制/逻辑文件（`ChessBoard.cpp`、`ChessPiece.cpp`、
`myprofile.cpp`、`CnchessSkin.cpp`、`algorithm/*`）**完全无平台宏**，`__ANDROID__` 在业务层出现 0 次，
仅以 `__MOBILE__` 一处开关区分移动触屏与桌面弹窗交互。

## 主题（皮肤）机制

- 本地 `pc_theme/` 与 `mobile_theme/` 仅承载**时钟/数字等局部皮肤资产**，随包发布。
- **完整主题包在运行时经 `ThemeDownloadManager`（WebSocket）分块下载、MD5 校验后解压到
  `theme_cache/`**，由服务端 `server/ThemeResourceProvider.cpp` 按平台（`dwOSId`）分发达下发。
  换肤不改动任何 C++ 代码。

## 编译说明

| 目标 | 构建方式 | 入口 |
|------|----------|------|
| Windows / Linux / macOS / iOS | SOUI 根工程 CMake 生成 `cnchess_client` | `client/main.cc` |
| Android | Android Studio 打开 `client/android`（Gradle + NDK） | `android_entry.cc` |
| OpenHarmony | DevEco Studio 打开 `client/ohos`（hvigor + NDK） | `ohos_entry.cc` |

桌面端构建产物支持资源内嵌（`ENABLE_BUILD_RESOURCE` 把系统/应用资源打进 PE，单文件部署）；
Linux 提供 `client/build_deb.sh` 打包为 `.deb`。

## 服务器

`server/` 同为 C++，提供联机对弈、人机对战（机器人复用 `algorithm/` 引擎）与残局练习：

- `Game.cpp`：牌桌/对局管理
- `CnChess.cpp` / `WebSocketGame.cpp`：棋局规则与 WebSocket 协议
- `RobotAIPool.cpp`：机器人 AI（复用 `algorithm/` 引擎）
- `ThemeResourceProvider.cpp`：按平台分发主题资源
- `config.xml` / `endgames.json`：配置与残局库

运行：`./chess_server [config_file]`。

## 使用的 SOUI 特性

- XML 布局系统（UI 与逻辑分离）
- 事件映射机制（Win32/WTL 式消息路由）
- 网格 / 锚点 / 流式 / 框架布局
- 自定义窗口与皮肤（`SWindow` 子类、`SSkinAni`/`SSkinPiece`/`SSkinBoard`）
- 资源管理系统（ZIP / PE / Android assets / OHOS rawfile 抽象）
- WebSocket 网络通信
- 类 COM 引用计数（`SAutoRefPtr`）管理对象生命周期
- Google Test 单元测试框架（算法层）
