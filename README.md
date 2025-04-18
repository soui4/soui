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


</div>


 _SOUI4_ 是一套已经持续开发维护 **14年** 的轻量级Windows平台客户端开发框架,
（内核编译后体积1M左右），
框架设计吸收了`WTL`，`Android`，`QT`，`Flash`等流行框架的特点，
UI展示基于`xml`配置，性能高效（可选择GDI或[Skia](https://www.oschina.net/p/skia)渲染），
控件丰富，扩展灵活（可使用 xml + lua + jx  ），真正做到UI界面与逻辑分离。



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

- [国内Git下载站点](https://gitee.com/setoutsoft/soui4)(作为镜像使用)


### [编译](./doc/build.md)

[![CMake Build](https://github.com/soui4/soui/actions/workflows/build.yml/badge.svg)](https://github.com/soui4/soui/actions/workflows/build.yml.

使用cmake生成对应版本VS工程再编译，运动在cmake中统一修改配置。具体编译方法查看附带[编译文档](./doc/build.md)

### UI可视化设计

最新版本souieditor已经集成到这个代码库的demos中，选择编译demo就可以获得souieditor。
### SOUI4JS
`SOUI4的JavaScript导出版本，实现使用js开发客户端程序的框架，完美解决性能和灵活性的矛盾`,仓库地址(https://github.com/soui4/soui4js)

使用教程见文档

### 其他教程

- 阅读作者[博客](http://www.cnblogs.com/setoutsoft/)，能让你更快掌握如何使用SOUI快速开发Window桌面应用

- 作者录制的b站[视频教程](https://space.bilibili.com/110611388)

## 使用soui的软件展示

请点击[链接](https://2926295173.github.io/SoUIDocs/Show/show/)查看，持续更新

##  **QQ群** 

- SOUI讨论#1: [点击加入](http://shang.qq.com/wpa/qunwpa?idkey=9653a811a72365d798a5247d6ba6885a568bdcf51c624f906c8ce7b8fd9e4eda)

- SOUI讨论#2: [点击加入](http://shang.qq.com/wpa/qunwpa?idkey=03d3294a2551beb1b54b4012086cec14b3f66d5c253debaeed241d9c623966e0)


##  **Linux编译** 
soui从5.0开始支持linux平台，soui通过链接swinx(https://github.com/soui4/swinx)项目提供在linux平台下的windows api实现跨平台支持。
linux平台编译参见swinx中的build.md，安装后必须的依赖后，使用cmake即可完成编译。

##  **使用vs远程调试linux** 
linux机器上安装
sudo apt install -y openssh-server build-essential gdb rsync make zip ninja-build
本机安装
CMake, Vcxsvr(https://sourceforge.net/projects/vcxsrv/files/vcxsrv/21.1.10/vcxsrv-64.21.1.10.0.installer.exe/download)
在VS菜单：
工具\选项\跨平台 页面的列表中添加linux主机IP，注意配置好登陆启用名，密码
然后就可以在VS里运行远程调试了。
具体参考：https://learn.microsoft.com/zh-cn/cpp/build/get-started-linux-cmake?view=msvc-170

##  **linux 内存泄漏检测** 
valgrind --leak-check=full ./your_program

## **贡献名单**
参见[贡献名单](./contributors.md)

##  **LICENSE** 

请查看[`license.txt`](./license.txt)文档