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

This README is available in the following languages:
- [English Version (README.md)](README.md)
- [Chinese Version (README.zh-CN.md)](README.zh-CN.md)

以下是指南的中文版本和英文版本：

##  **指南 / Guide** 

 SOUI4 是一套已经持续开发维护 **14年** 的轻量级Windows平台客户端开发框架,
（内核编译后体积1M左右），
框架设计吸收了`WTL`，`Android`，`QT`，`Flash`等流行框架的特点，
UI展示基于`xml`配置，性能高效（可选择GDI或[Skia](https://www.oschina.net/p/skia)渲染），
控件丰富，扩展灵活（可使用 xml + lua + js  ），真正做到UI界面与逻辑分离。

点击 [SOUI](https://www.soui.com.cn) 访问官网。

##  **demo基本界面** 
![demo1](./doc/demo_png/souidemo1.png)
![demo2](./doc/demo_png/souidemo2.png)

如果您初次了解此库，可以下载自动化静态编译的[demo.exe](https://github.com/soui4/soui/actions)进行尝鲜体验（无需下载源代码和其他依赖，直接下载运行）
具体方法
[![CMake Build](https://github.com/soui4/soui/actions/workflows/build.yml/badge.svg)](https://github.com/soui4/soui/actions/workflows/build.yml)

```
1.点击上方图标,进入页面
2 随便点击一个绿色圆圈后面的链接进入
3 下拉界面，找到`Artifacts`栏
4 点击栏中的`PC_Demo`进行下载解压运行即可。
```
如您想自己编译更多的界面案例，请拉取 DEMO仓库：

- https://github.com/soui4-demo

每一个Demo一个单独的仓库，自己分别clone就好了。

##  **开始** 

使用SOUI需要一定的c++与win32的基础，请您确保您掌握了上述技能。

除本页说明外，您还可以查看更详细的[官方文档](https://2926295173.github.io/SoUIDocs/)

### Git下载地址

- [国外Git下载站点](https://github.com/soui4/soui)

- [国内Gitee](https://gitee.com/setoutsoft/soui4)

- [国内gitcode](https://gitcode.com/setoutsoft/soui4)

### [编译](./doc/build.md)

[![CMake Build](https://github.com/soui4/soui/actions/workflows/build.yml/badge.svg)](https://github.com/soui4/soui/actions/workflows/build.yml).

使用cmake生成对应版本VS工程再编译，运动在cmake中统一修改配置。具体编译方法查看附带[编译文档](./doc/build.md)

### UI可视化设计

最新版本souieditor已经集成到这个代码库的demos中，选择编译demo就可以获得souieditor。
### SOUI4JS
- `SOUI4的JavaScript导出版本，实现使用js开发客户端程序的框架，完美解决性能和灵活性的矛盾`,仓库地址(https://github.com/soui4/soui4js)
- 目前已经支持windows,linux,macos三平台。
### SOUI4PY
- `SOUI4的Python3版本，实现使用python3开发客户端程序的框架，完美解决性能和灵活性的矛盾`,仓库地址(https://gitee.com/setoutsoft/soui4py)
- 目前已经支持windows,linux,macos三平台。

### SOUI教程

- 最新[教程](https://gitee.com/setoutsoft/soui-docs) ，自己在本地搭建http服务器,本地查看教程。发现教程有问题，欢迎随时提issue。
- 作者[博客](http://www.cnblogs.com/setoutsoft/)，能让你更快掌握如何使用SOUI快速开发Window桌面应用
- 作者录制的b站[视频教程](https://space.bilibili.com/110611388)

##  **QQ群** 

- SOUI讨论#1: [点击加入](http://shang.qq.com/wpa/qunwpa?idkey=9653a811a72365d798a5247d6ba6885a568bdcf51c624f906c8ce7b8fd9e4eda)

- SOUI讨论#2: [点击加入](http://shang.qq.com/wpa/qunwpa?idkey=03d3294a2551beb1b54b4012086cec14b3f66d5c253debaeed241d9c623966e0)

##  **编译**
###  **Windows编译**
- 直接使用cmake 生成工程文件，然后使用Visual Studio打开工程文件，编译即可。
- 使用mingw32/64编译：mkdir build && cd build && cmake .. && make -j8

###  **Linux编译** 
- 拉取子模块swinx：git submodule update --init
- 安装依赖：sudo apt install cmake git build-essential gdb libxcb1-dev libgl1-mesa-dev freeglut3-dev
- 编译：mkdir build && cd build && cmake .. && make -j8

###  **macos 编译**
- 拉取子模块swinx：git submodule update --init
- 安装brew,国内建议使用[中国镜像](https://gitee.com/cunkai/HomebrewCN)
- 安装homebrew-core 和 homebrew-cask: brew tap homebrew/core && brew tap homebrew/cask
- 安装编译工具及依赖: brew install cmake ninja pkgconf glfw3 glew
- 编译: mkdir build && cd build && cmake .. -G Ninja && ninja -j8

## **贡献名单**
参见[贡献名单](./contributors.md)

##  **LICENSE** 

请查看[`license.txt`](./license.txt)文档
