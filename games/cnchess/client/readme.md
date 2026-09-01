# SOUI 中国象棋（cnchess）PC 端（Windows / Linux / macOS / iOS）

本目录（`games/cnchess/client`）是 **cnchess 中国象棋** 的桌面与 iOS 版本共享的 C++ 业务代码。它通过 SOUI 根工程的 CMake 统一编译，是“其它版本”（相对 Android / 鸿蒙 移动端）的实现。

> Android 与鸿蒙 各自的工程与 readme 在子目录 `android/`、`ohos/`；本文件描述的是直接由 `main.cc` 入口、在桌面（Windows/Linux/macOS）与 iOS 上运行的版本。

## 入口与平台分流

统一入口 `main.cc`：

- **Windows**：直接 `WinMain`（`_tWinMain`），由系统 CRT 拉起消息循环（`app.Run`）。
- **Linux / macOS**：`main()` 转发回 `_tWinMain`，进程事件循环由 swinx 内部实现（Linux 用 xcb + poll，macOS 用 `SwinXApplication`）。
- **iOS**：`main()` 调用 `swinx_ios_entry(argc, argv, _tWinMain)`，由 `UIApplicationMain` 托管。

各平台通过宏区分资源与字体路径（`__APPLE__`、`__IOS__`、`_WIN32` 等，均为 SOUI 预设宏）。

## 构建方式

cnchess 作为 SOUI 解决方案的一部分编译：

1. 在 SOUI 源码根目录用 CMake 配置工程（`games/CMakeLists.txt` 会 `add_subdirectory(cnchess)`，生成目标 `cnchess_client`）。
2. 构建目标 `cnchess_client` 即可（依赖 `soui4` / `utilities4` / `swinx`（非 Windows）/ `resprovider-zip` / `ExtendCtrls` / `cnchess_algorithm` 等）。
3. **Windows 资源预处理**：`build_rc2.bat` 调用 `uiresbuilder.exe`（依赖环境变量 `SOUI4_INSTALL_64`）把 `uires/uires.idx` 编译为 `res/soui_res.rc2` 与 `res/resource.h`，再经 `cnchess.rc` 打入 PE。

```bat
REM build_rc2.bat（Windows，需先设置 SOUI4_INSTALL_64 指向 SOUI 安装根）
uiresbuilder.exe -i uires\uires.idx -p uires -r res\soui_res.rc2 -h res\resource.h idtable
```

## 资源加载

| 平台 | 系统资源 | 应用资源 |
|------|----------|----------|
| Windows | PE 文件 `soui-sys-resource.dll`（`SetSysResPeFile`） | 当前模块 PE 句柄（`SetAppResPeHandle`） |
| Linux / macOS | `soui-sys-resource.zip` 或 `soui-sys-resource/` 目录 | `chess_uires.zip` 或 `uires/` 目录 |
| iOS | 同 macOS，资源位于 App Bundle（`soui-sys-resource`） | 复用 Android 的 `assets/uires`（`add_macos_res_folder` 拷入 Bundle） |

所有平台统一使用 `Render_Skia` + `ImgDecoder_Stb`。

## 平台差异要点

- **Windows**：PE 内嵌资源；链接 `win32_audio`；DPI 感知 manifest。
- **Linux**：通过 swinx 的 xcb 事件泵运行；字体 `simsun.ttc` 位于源码树上三级目录，运行期 `AddFontResource`。
- **macOS**：资源与框架库（`swinx`/`soui4`/…）拷入 `.app` Bundle；`MACOSX_BUNDLE` 配置；自动签名。
- **iOS**：`SW_MAXIMIZE` 全屏；UI 资源与 Android 共享（`assets/uires`）；通过 `swinx_ios_entry` 接入 `UIApplicationMain`。

## 关键文件说明

| 文件 | 作用 |
|------|------|
| `main.cc` | 统一入口，按平台分流，创建 `SApplication`、注册皮肤/窗口类、加载资源、显示 `CMainDlg` |
| `CMakeLists.txt` | cnchess_client 构建脚本（被 SOUI 根工程 `add_subdirectory` 引入） |
| `build_rc2.bat` | Windows 下用 uiresbuilder 预处理 UI 资源 |
| `MainDlg.cpp/.h` | 主窗口（`SHostWnd` 子类） |
| `ChessGame.cpp/.h` | 棋局逻辑 |
| `uires/` | 桌面版 UI 资源（布局、皮肤、字符串） |
| `pc_theme/` / `mobile_theme/` | 桌面 / 移动两套主题资源 |

## 与其它版本的关系

- **Android**：见 [android/readme.md](android/readme.md)
- **鸿蒙（OHOS）**：见 [ohos/readme.md](ohos/readme.md)

三者共享 `MainDlg` / `ChessGame` / `ChessBoard` 等业务源码，仅入口（`main.cc` / `android_entry.cc` / `ohos_entry.cc`）与资源加载方式不同。
