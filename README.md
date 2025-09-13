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

SOUI4 is a lightweight Windows client development framework that has been continuously developed and maintained for **14 years**,
(The core compiled size is about 1M),
The framework design absorbs the characteristics of popular frameworks such as `WTL`, `Android`, `QT`, and `Flash`,
The UI presentation is based on `xml` configuration, with efficient performance (you can choose GDI or [Skia](https://www.oschina.net/p/skia) rendering),
Rich controls and flexible extensions (can use xml + lua + js), truly separating the UI interface from logic.

Click [SOUI](https://www.soui.com.cn) to visit the official website.

## **Basic demo interface** 
![demo1](./doc/demo_png/souidemo1.png)
![demo2](./doc/demo_png/souidemo2.png)

If you are new to this library, you can download the automatically statically compiled [demo.exe](https://github.com/soui4/soui/actions) for a fresh experience (no need to download source code and other dependencies, download and run directly)
Specific method
[![CMake Build](https://github.com/soui4/soui/actions/workflows/build.yml/badge.svg)](https://github.com/soui4/soui/actions/workflows/build.yml)

```
1. Click the icon above to enter the page
2. Click any link behind the green circle to enter
3. Pull down the interface and find the `Artifacts` column
4. Click `PC_Demo` in the column to download, decompress and run.
```
If you want to compile more interface cases yourself, please pull the DEMO repository:

- https://github.com/soui4-demo

Each Demo has a separate repository, just clone them separately.

## **Getting Started** 

Using SOUI requires a certain foundation in c++ and win32, please make sure you have mastered the above skills.

In addition to the instructions on this page, you can also view more detailed [official documentation](https://2926295173.github.io/SoUIDocs/)

### Git download address

- [Foreign Git download site](https://github.com/soui4/soui)

- [Domestic Gitee](https://gitee.com/setoutsoft/soui4)

- [Domestic gitcode](https://gitcode.com/setoutsoft/soui4)

### [Compile](./doc/build.md)

[![CMake Build](https://github.com/soui4/soui/actions/workflows/build.yml/badge.svg)](https://github.com/soui4/soui/actions/workflows/build.yml).

Use cmake to generate the corresponding version of the VS project and then compile, modify the configuration uniformly in cmake. For specific compilation methods, see the attached [compilation documentation](./doc/build.md)

### UI visual design

The latest version of souieditor has been integrated into the demos in this code base. Select to compile demo to get souieditor.

### SOUI4JS
- `SOUI4 JavaScript export version, a framework that implements client program development using js, perfectly solving the contradiction between performance and flexibility`, repository address(https://github.com/soui4/soui4js)
- Currently supports windows, linux and macos platforms.

### SOUI4PY
- `SOUI4 Python3 version, a framework that implements client program development using python3, perfectly solving the contradiction between performance and flexibility`, repository address(https://gitee.com/setoutsoft/soui4py)
- Currently supports windows， linux and macOs platforms.

### SOUI Tutorial

- Latest [Tutorial](https://gitee.com/setoutsoft/soui-docs), set up an http server locally to view the tutorial locally. If you find any problems with the tutorial, please feel free to submit an issue.
- Author's [Blog](http://www.cnblogs.com/setoutsoft/), which will help you master how to quickly develop Window desktop applications using SOUI
- Author's Bilibili [Video Tutorial](https://space.bilibili.com/110611388)

## **QQ Group** 

- SOUI Discussion #1: [Click to join](http://shang.qq.com/wpa/qunwpa?idkey=9653a811a72365d798a5247d6ba6885a568bdcf51c624f906c8ce7b8fd9e4eda)

- SOUI Discussion #2: [Click to join](http://shang.qq.com/wpa/qunwpa?idkey=03d3294a2551beb1b54b4012086cec14b3f66d5c253debaeed241d9c623966e0)

## **Compile**
### **Windows Compile**
- Directly use cmake to generate project files, then use Visual Studio to open the project file and compile.
- Compile using mingw32/64: mkdir build && cd build && cmake .. && make -j8

### **Linux Compile** 
- Pull submodule swinx: git submodule update --init
- Install dependencies: sudo apt install cmake git build-essential gdb libxcb1-dev libgl1-mesa-dev freeglut3-dev
- Compile: mkdir build && cd build && cmake .. && make -j8

### **macos Compile**
- Pull submodule swinx: git submodule update --init
- Install brew, domestic users are advised to use [Chinese mirror](https://gitee.com/cunkai/HomebrewCN)
- Install homebrew-core and homebrew-cask: brew tap homebrew/core && brew tap homebrew/cask
- Install compilation tools and dependencies: brew install cmake ninja pkgconf glfw3 glew
- Compile: mkdir build && cd build && cmake .. -G Ninja && ninja -j8

## **Contributor List**
See [Contributor List](./contributors.md)

## **LICENSE** 

Please refer to [`license.txt`](./license.txt) document