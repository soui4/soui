# 编译

## `cmake`

从cmake官网下载cmake的最新Release版本,这里以cmake-3.15.4-win64-x64.zip举例。

1.将cmake-3.15.4-win64-x64.zip解压后运行bin目录下的cmake-gui.exe。

2。选择soui源码目录和cmake临时文件生成目录，点击Configure。

![image](./cmake/01.png)

3。弹出临时文件目录不存在，是否创建？选择Yes。   

![image](./cmake/02.png)

4.选择需要生成的VS版本(如果未安装会生成失败,根据实际情况选择)和需要生成的编译版本(Win32\x64)，点击Finish。   

![image](./cmake/04.png)

5。根据需要选择生成方式：内核和组件模块的(静态链接或动态链接)；是否使用CRT；Unicode或多字节；点击Generate开始生成。   

![image](./cmake/05.png)

6。生成完毕,显示"Generating done"，此时点击Open Project打开项目(也可以双击cmake临时文件生成目录中找到sln文件打开)

![image](./cmake/06.png)

7。在VS中编译，编译完成后,在soui源码的bin目录下可以找到生成出来的dll(或lib)和exe。

![image](./cmake/07.png)

## 老版本中64位的问题的说明

新版本32 64分离，方便同时编译32+64方式，向导已更新支持

要同时支持32+64目前使用相对麻烦一点,方法如下：

1 分别运行bat生成，32位和64位，如果要向导正常工作请保持两项配置保持一致。

2 分别编译soui.sln和soui64.sln。

3 使用向导生成项目。

其它说明：如果生成的是DLL版本。请注意64位可以会提示找不到DLL。因为64位版本输出目录bin64并没有加入PATH。