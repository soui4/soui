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

---

## Guide

SOUI4 is a lightweight Windows client development framework, continuously developed and maintained for **14 years**. The compiled core is about 1MB. The framework design draws inspiration from popular frameworks such as WTL, Android, QT, and Flash. UI presentation is based on XML configuration, with high performance (choose GDI or [Skia](https://www.oschina.net/p/skia) rendering), rich controls, and flexible extension (supports xml + lua + js), truly separating UI and logic.

Visit the [SOUI Official Website](https://www.soui.com.cn).

## Demo Screenshots
![demo1](./doc/demo_png/souidemo1.png)
![demo2](./doc/demo_png/souidemo2.png)

If you are new to SOUI, you can download the statically compiled [demo.exe](https://github.com/soui4/soui/actions) for a quick try (no need for source code or dependencies, just download and run).

[![CMake Build](https://github.com/soui4/soui/actions/workflows/build.yml/badge.svg)](https://github.com/soui4/soui/actions/workflows/build.yml)

Steps:
1. Click the badge above to enter the page
2. Click any link after the green circle
3. Scroll down to find the `Artifacts` section
4. Download and extract `PC_Demo` to run

For more demo cases, visit the DEMO repository:
- https://github.com/soui4-demo
Each demo is in a separate repository, clone as needed.

## Getting Started

SOUI requires basic knowledge of C++ and Win32. Please ensure you are familiar with these skills.
For more details, see the [official documentation](http://doc.soui.com.cn/).

### Git Repositories
- [GitHub](https://github.com/soui4/soui)
- [Gitee](https://gitee.com/setoutsoft/soui4)
- [GitCode](https://gitcode.com/setoutsoft/soui4)

### [Build Instructions](./doc/build.md)

[![CMake Build](https://github.com/soui4/soui/actions/workflows/build.yml/badge.svg)](https://github.com/soui4/soui/actions/workflows/build.yml)

Use CMake to generate Visual Studio projects, then compile. All configuration is managed in CMake. For details, see [build documentation](./doc/build.md).

### UI Visual Designer
The latest souieditor is integrated in the demos folder. Build the demo to get souieditor.

### SOUI4JS
- JavaScript export version for client development. Repository: [soui4js](https://github.com/soui4/soui4js)
- Supports Windows, Linux, macOS.

### SOUI4PY
- Python3 version for client development. Repository: [soui4py](https://gitee.com/setoutsoft/soui4py)
- Supports Windows, Linux, macOS.

### Tutorials
- Latest [Online Tutorial](http://doc.soui.com.cn) (most complete, updated regularly)
- [Tutorial Source Code](https://gitee.com/setoutsoft/soui-docs) (host locally with http server)
- Author's [Blog](http://www.cnblogs.com/setoutsoft/)
- Author's Bilibili [Video Tutorials](https://space.bilibili.com/110611388)

## QQ Groups
- SOUI Discussion #1: [Join](http://shang.qq.com/wpa/qunwpa?idkey=9653a811a72365d798a5247d6ba6885a568bdcf51c624f906c8ce7b8fd9e4eda)
- SOUI Discussion #2: [Join](http://shang.qq.com/wpa/qunwpa?idkey=03d3294a2551beb1b54b4012086cec14b3f66d5c253debaeed241d9c623966e0)

## Build Instructions
### Windows
- Use CMake to generate project files, open with Visual Studio, and compile
- Or use mingw32/64: `mkdir build && cd build && cmake .. && make -j8`

### Linux
- Pull submodule swinx: `git submodule update --init`
- Install dependencies: `sudo apt install cmake git build-essential gdb libxcb1-dev libgl1-mesa-dev freeglut3-dev`
- Build: `mkdir build && cd build && cmake .. && make -j8`

### macOS
- Pull submodule swinx: `git submodule update --init`
- Install Homebrew ([China mirror](https://gitee.com/cunkai/HomebrewCN) recommended)
- Install homebrew-core and homebrew-cask: `brew tap homebrew/core && brew tap homebrew/cask`
- Install tools and dependencies: `brew install cmake ninja pkgconf glfw3 glew`
- Build: `mkdir build && cd build && cmake .. -G Ninja && ninja -j8`

## Contributors
See [contributors.md](./contributors.md)

## License
See [`license.txt`](./license.txt)