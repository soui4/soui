<div align= "center">
<img align="center" width=200 src="./doc/logo.png" />
</div>

<div align= "center">
 <h1>SOUI 4</h1>

 <a>
    <img alt="DirectUI" src="https://img.shields.io/badge/C%2B%2B-DirectUI%E5%BA%93-brightgreen?style=for-the-badge" />
 </a>
 <a href="https://github.com/soui4/soui/releases">
    <img alt="releases" src="https://img.shields.io/github/v/release/soui4/soui?color=blueviolet&include_prereleases&style=for-the-badge" />
 </a>
<a href="https://github.com/soui4/soui/">
    <img alt="star" src="https://img.shields.io/github/stars/soui4/soui?style=for-the-badge">
</a>

 <a href="https://2926295173.github.io/SoUIDocs">
    <img alt="usage" src="https://img.shields.io/badge/%E4%BD%BF%E7%94%A8-%E6%96%87%E6%A1%A3-yellow?style=for-the-badge"/>
</a>

 <a href="https://deepwiki.com/soui4/soui">
    <img alt="Ask DeepWiki" src="https://deepwiki.com/badge.svg"/>
</a>
</div>

## Language Versions / 语言版本

以下是指南的中文版本和英文版本：
- [英文版 (README.md)](README.md)
- [中文版 (README.zh-CN.md)](README.zh-CN.md)

---

##  指南 / Guide

SOUI4 是一套已经持续开发维护 **14年** 的轻量级**跨平台** C++ DirectUI 框架（内核编译后体积 1M 左右）。框架设计吸收了 `WTL`、`Android`、`QT`、`Flash` 等流行框架的特点，UI 展示基于 `xml` 配置，性能高效（可选择 GDI 或 [Skia](https://www.oschina.net/p/skia) 渲染），控件丰富，扩展灵活（可使用 xml + lua + js），真正做到 UI 界面与逻辑分离。

借助内置的 **swinx** 层（在各平台重新实现 Windows API：`windows.h` / `winuser.h` / `gdi.h` …），**同一份 C++ UI 与业务代码**即可运行在 Windows、Linux、macOS、iOS、Android 与鸿蒙（OHOS）六大平台上。点击 [SOUI 官网](https://www.soui.com.cn) 了解更多。

##  跨平台支持 / Cross-Platform

SOUI4 通过 swinx 覆盖六大平台：

| 平台 | 状态 | 窗口系统 / 启动器 | 2D 后端 |
| --- | --- | --- | --- |
| **Windows** | 原生 | 系统 CRT / Win32 | GDI / Direct2D |
| **Linux** | 已支持 | xcb + `poll()` 事件循环 | Cairo |
| **macOS** | 已支持 | `NSApplication`（`SwinXApplication`） | Core Graphics |
| **iOS** | 已支持 | `UIApplicationMain`（由 `swinx_ios_entry` 托管） | Core Graphics |
| **Android** | 已支持 | JVM 启动器，`PlatformAPI` 桥接 | Cairo |
| **鸿蒙（OHOS）** | 已支持 | ArkTS 运行时，`PlatformAPI` 桥接 | Cairo |

swinx 在不同平台的分层方式：

- **Windows** 不编译 swinx，直接使用系统的 Windows SDK。
- **Linux / macOS / iOS** 在 swinx 内部自实现平台 API，且各自拥有自己的消息循环 / 启动器，无需额外桥接。
- **Android / 鸿蒙** 由 JVM / ArkTS 运行时启动，没有自己的启动器；其平台桥接（`clipboard / window / ime / audio / path`）被填入 C 结构体 `PlatformAPI` 后，通过 `PlatformAPI_Init` 注册。

SOUI 的渲染工厂（`Render_Gdi` / `Render_Skia` / `Render_D2d`）在各平台统一；跨平台工程通常统一使用 **`Render_Skia`** + `ImgDecoder_Stb`。移动端还额外提供 `SModalView`（替代 `SHostDialog` 的模态对话框实现，因为移动端运行时无法自行拉起消息循环），以及 `SPanel` 上的惯性 **fling** 滑动支持。

##  demo 基本界面

![demo1](./doc/demo_png/souidemo1.png)
![demo2](./doc/demo_png/souidemo2.png)

如果您初次了解此库，可以下载自动化静态编译的 [demo.exe](https://github.com/soui4/soui/actions) 进行尝鲜体验（无需下载源代码和其他依赖，直接下载运行）。

具体方法：

```
1. 点击上方图标，进入页面
2. 随便点击一个绿色圆圈后面的链接进入
3. 下拉界面，找到 `Artifacts` 栏
4. 点击栏中的 `PC_Demo` 进行下载解压运行即可。
```

如您想自己编译更多的界面案例，请拉取 DEMO 仓库：
- https://github.com/soui4-demo

每一个 Demo 一个单独的仓库，自己分别 clone 就好了。

##  开始

使用 SOUI 需要一定的 C++ 与 Win32 的基础，请您确保您掌握了上述技能。

除本页说明外，您还可以查看更详细的 [官方文档](http://124.222.50.176:4043/)。

### Git 下载地址

- [国外 Git 下载站点](https://github.com/soui4/soui)
- [国内 Gitee](https://gitee.com/setoutsoft/soui4)
- [国内 gitcode](https://gitcode.com/setoutsoft/soui4)

### UI 可视化设计

最新版本 souieditor 已经集成到这个代码库的 demos 中，选择编译 demo 就可以获得 souieditor。

### SOUI4JS
- `SOUI4 的 JavaScript 导出版本，实现使用 js 开发客户端程序的框架，完美解决性能和灵活性的矛盾`，仓库地址 [soui4js](https://github.com/soui4/soui4js)
- 目前已经支持 Windows、Linux、macOS 三平台。

### SOUI4PY
- `SOUI4 的 Python3 版本，实现使用 python3 开发客户端程序的框架，完美解决性能和灵活性的矛盾`，仓库地址 [soui4py](https://gitee.com/setoutsoft/soui4py)
- 目前已经支持 Windows、Linux、macOS 三平台。

### SOUI 教程

- 最新 [教程](http://doc.soui.com.cn)，目前最完整的 SOUI 在线教程，不断更新，请持续关注。
- 最新 [教程源代码](https://gitee.com/setoutsoft/soui-docs)，自己在本地搭建 http 服务器，本地查看教程。发现教程有问题，欢迎随时提 issue。
- 作者 [博客](http://www.cnblogs.com/setoutsoft/)，能让你更快掌握如何使用 SOUI 快速开发客户端应用。
- 作者录制的 B 站 [视频教程](https://space.bilibili.com/110611388)

##  QQ 群

- SOUI 讨论 #1: [点击加入](http://shang.qq.com/wpa/qunwpa?idkey=9653a811a72365d798a5247d6ba6885a568bdcf51c624f906c8ce7b8fd9e4eda)
- SOUI 讨论 #2: [点击加入](http://shang.qq.com/wpa/qunwpa?idkey=03d3294a2551beb1b54b4012086cec14b3f66d5c253debaeed241d9c623966e0)

##  [![编译](https://github.com/soui4/soui/actions/workflows/build.yml/badge.svg)](https://github.com/soui4/soui/actions/workflows/build.yml)

###  Windows 编译
- 直接使用 cmake 生成工程文件，然后使用 Visual Studio 打开工程文件，编译即可。
- 使用 mingw32/64 编译：`mkdir build && cd build && cmake .. && make -j8`

###  Linux 编译
- 拉取子模块 swinx：`git submodule update --init`
- 安装依赖：`sudo apt install cmake git build-essential gdb libxcb1-dev libgl1-mesa-dev freeglut3-dev`
- 编译：`mkdir build && cd build && cmake .. && make -j8`

###  macOS 编译
- 拉取子模块 swinx：`git submodule update --init`
- 安装 brew，国内建议使用 [中国镜像](https://gitee.com/cunkai/HomebrewCN)
- 安装 homebrew-core 和 homebrew-cask: `brew tap homebrew/core && brew tap homebrew/cask`
- 安装编译工具及依赖: `brew install cmake ninja pkgconf glfw3 glew`
- 编译: `mkdir build && cd build && cmake .. -G Ninja && ninja -j8`

###  iOS 编译
- 拉取子模块 swinx：`git submodule update --init`
- 用 CMake 生成 Xcode 工程并通过 Xcode 编译：
  `mkdir build && cd build && cmake .. -G Xcode -DCMAKE_SYSTEM_NAME=iOS`
- 应用入口由 `swinx_ios_entry()` 托管（内部封装 `UIApplicationMain`），C++ 的 `_tWinMain` 在启动后于主线程派发执行。

###  鸿蒙（OHOS）编译
- 需要 HarmonyOS SDK 与 DevEco Studio。
- 原生层位于 `entry/src/main/cpp`；UI 资源通过 `NativeResourceManager` 直接从 HAP 的 `rawfile` 目录读取（无需拷贝到沙箱）。
- 在 DevEco Studio（或 `hvigor` 命令行）中编译 OHOS 模块。

##  贡献名单
参见 [贡献名单](./contributors.md)

##  LICENSE

请查看 [`license.txt`](./license.txt) 文档
