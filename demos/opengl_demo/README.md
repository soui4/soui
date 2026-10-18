# opengl_demo：SRealWnd + SNativeWnd + OpenGL 子窗口

演示如何在 SOUI 里用 `<realwnd/>` 把一个 **原生子窗口**嵌进 DUI 布局，并在这个子窗口里
使用 **OpenGL** 渲染。支持 Windows / macOS / Linux（不需要 GLEW/GLFW）。

上下文管理按 WGL 语义书写（`ChoosePixelFormat`/`wglCreateContext`/`wglMakeCurrent`/`SwapBuffers`），
源码三平台统一写 `#include <windows.h>` + `<GL/gl.h>`，开发人员感知不到平台差异：
Windows 直接用系统头；macOS/Linux 命中 swinx 的同名兼容头（`swinx/include/GL/gl.h` 转发平台
GL 并拉入 `swinx/include/wgl.h` 的 WGL 声明）——macOS 用 `NSOpenGLContext` 绑到子窗口的
NSView，Linux 用 GLX（运行期 `dlopen` libGL，按窗口 Visual 匹配 FBConfig，无 X11/GLX
链接期依赖）。

```
SOUI 宿主窗口 (SHostWnd / SHostDialog)
 ├─ DUI 控件：标题栏、按钮、状态文本
 └─ <realwnd wndclass="SOUI_GLWnd"/>          <- SRealWnd 控件
       └─ CGLWnd : SNativeWnd                 <- 真正的原生子窗口（父窗口是宿主 HWND）
             └─ HGLRC                          <- OpenGL 渲染上下文
```

## 文件

| 文件 | 作用 |
|------|------|
| `main.cpp` | 入口：初始化 `SApplication`/`SAppCfg`，注册自定义 `IRealWndHandler`，解析 `--selftest` |
| `GLRealWndHandler.h/.cpp` | `IRealWndHandler` 实现：把 `<realwnd>` 变成 `CGLWnd` |
| `GLWnd.h/.cpp` | `CGLWnd`：SNativeWnd 派生，建 GL 上下文、两条渲染路径、鼠标交互、截图 |
| `GLProc.h/.cpp` | GL 1.5/2.0 入口的运行期加载（Windows：`wglGetProcAddress` + `opengl32` 兜底；macOS/Linux：swinx 兼容层内置 dlsym/glxGetProcAddress 回退） |
| `GLWglTest.cpp` | WGL 入口契约测试：扩展入口（extensions string / swap control / create_context_attribs / make_current_read / share lists / swap multiple buffers / default proc address）与降级入口（图层平面 / GDI 字形 / copy context） |
| `MainDlg.h/.cpp` | 主对话框：按钮、状态文本、自检逻辑 |
| `uires/` | 布局、图标、`uires.idx` |

## 调用链

1. `main.cpp` 里 `app.SetRealWndHandler(new CGLRealWndHandler())`——`SApplication` 全局只存一个
   `IRealWndHandler`，`SRealWnd` 构造时通过 `GETREALWNDHANDLER` 取它。
2. 布局里的 `<realwnd name="glwnd" wndclass="SOUI_GLWnd"/>` 创建后，`SRealWnd` 在需要真窗口时
   调用 `IRealWndHandler::OnRealWndCreate`。
3. 处理器比对 `wndclass` 后 `new CGLWnd`，调 `SNativeWnd::CreateNative` 建出子窗口
   （复用 SOUI 注册的“简单窗口类”，对象/HWND 绑定由框架 thunk 完成），把对象指针挂到
   `IRealWnd::SetData`，返回 HWND。
4. `OnRealWndPosition` 返回 `FALSE`，位置/大小交给 `SRealWnd` 用 `SetWindowPos` 处理——
   子窗口坐标就是宿主客户区坐标，两者始终重合（可用 `--selftest` 的 `[几何]` 段核对）。
5. 控件销毁时 `OnRealWndDestroy` 从 `GetData()` 取回对象，先 `DestroyWindow` 再 `delete`。

## 运行

```bat
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target opengl_demo
build\bin\opengl_demo.exe
```

```sh
# macOS / Linux
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target opengl_demo
./build/bin/opengl_demo
```

资源从源码目录读取（`__FILE__` 推导），不要求把可执行文件放到 `uires` 旁边。
渲染器默认 Skia，若组件缺失自动回落 GDI（DUI 部分只有标题栏/按钮/文本，两者都够用）。

平台要求：

- Windows：系统自带 `opengl32`。
- macOS：系统自带 OpenGL framework（桌面 GL 上限 2.1）。
- Linux：X11 会话（或 XWayland），运行期需有 `libGL.so.1` 与 `libX11.so.6`，
  编译期需要 GL 开发头（如 `libgl1-mesa-dev`）。窗口 Visual 不支持 GLX 时
  `wglCreateContext` 失败，探针报告会列出失败点。

### 交互

- 左键拖动：旋转立方体
- 滚轮：缩放
- 双击：暂停/继续动画
- 按钮：暂停动画 / 切换渲染路径 / 重建 GL 上下文 / 弹出 API 探针报告 / 跑 WGL 入口测试

## 两条渲染路径

| 模式 | 用到的 API |
|------|-----------|
| 固定管线 | `glFrustum/glRotatef/glBegin/glEnd/glColor3fv`、`glEnable(GL_DEPTH_TEST)`、`glOrtho` + `glBlendFunc`（左下角半透明色块） |
| GLSL 着色器 | `glCreateShader/glCompileShader/glLinkProgram`、VBO（`glGenBuffers/glBufferData/glVertexAttribPointer`）、`glUniform1f` 传角度/缩放/宽高比，旋转与透视除法都在顶点着色器里完成 |

着色器路径是“探针”的一部分：入口缺失或编译/链接失败时自动退回固定管线，失败原因写进报告。

## API 探针

「API 探针报告」按钮（或 `--selftest` 输出）会列出：

- 像素格式（`ChoosePixelFormat/SetPixelFormat` + `DescribePixelFormat` 的实际值）
- `wglCreateContext` / `wglMakeCurrent`
- `GL_VENDOR` / `GL_RENDERER` / `GL_VERSION` / `GLSL`
- `GL_MAX_TEXTURE_SIZE`、`GL_DEPTH_BITS`、`GL_DOUBLEBUFFER`、`wglSwapIntervalEXT`
- GL 1.1 立即模式、1.5 VBO 入口、2.0 着色器入口是否可用
- GLSL 程序编译链接结果

报告同时写到当前工作目录的 `opengl_probe.txt`。

## WGL 入口测试

「API 测试」按钮（或 `--selftest`）会跑一遍 `GLWglTest.cpp` 里的**契约测试**，覆盖 swinx
WGL 兼容层新增的那批入口。测的不是"某个驱动一定支持某个扩展"，而是"三个平台的行为契约一致"：

| 分组 | 断言 |
| --- | --- |
| 扩展入口取址 | `wglGetExtensionsStringARB/EXT`、`wglSwapIntervalEXT/Get`、`wglCreateContextAttribsARB`、`wglMakeContextCurrentARB`、`wglGetCurrentReadDCARB` 都能经 `wglGetProcAddress` 取到 |
| 扩展串自洽 | 串里声明了 `WGL_ARB_extensions_string` / `WGL_EXT_swap_control`，就必须能取到对应入口，反之亦然 |
| `wglSwapIntervalEXT` | `Set(1)` 后回读为 1；负间隔按规范应被拒（驱动不校验时记 SKIP）；结束后恢复原值 |
| `wglCreateContextAttribsARB` | 3.2 core 能创建并 `MakeCurrent`，新上下文 `GL_VERSION >= 3.2`；请求 9.9 必须返回 NULL（不能悄悄降级成 2.1）；core + compatibility 同置必须返回 NULL |
| `wglShareLists` | 共享后，源上下文里 `glGenTextures` 的纹理名在目标上下文里 `glIsTexture` 可见 |
| `wglMakeContextCurrentARB` | 读写同一窗口返回 TRUE，`wglGetCurrentReadDCARB` 回读的正是窗口 DC |
| `wglSwapMultipleBuffers` | `(0, NULL)` 返回 0；真实交换一项后当前上下文与 `glGetError` 都没被扰动 |
| `wglGetDefaultProcAddress` | 未知名字返回 NULL |
| 降级入口 | 图层平面 6 个 + `wglCopyContext` + GDI 字形 4 个：swinx 上必须返回文档写明的失败值，Windows 上交驱动 |
| 收尾 | 以上过程不得改变线程级的当前上下文 / 当前 DC |

期望值只有一处与平台相关（`GLWGL_STRICT_SPEC`）：swinx 是我们自己实现的，必须严格按规范校验和
降级；Windows 上的返回值一律交给驱动，只校验"可调用且不改变状态"。所以同一份断言在三个平台上
都应该通过 —— Windows 的真实现与 macOS/Linux 的兼容实现正好互相对照。

报告写成 `opengl_apitest.txt`。Windows（Intel Iris Xe / GL 4.6）实测：

```
WGL 入口契约测试（扩展 / 降级入口）

[WGL 扩展]
  [OK]   wglGetExtensionsStringARB/EXT 取址: ARB 与 EXT 都可取址
  [OK]   扩展串与取址一致: 串内 ARB_extensions_string=1 / EXT_swap_control=1，取址 ARB=1 / swap=1
  [OK]   wglSwapIntervalEXT(1) 生效: 原值=1, Set(1) 返回 1, 回读=1
  [SKIP] wglSwapIntervalEXT(-1) 被拒: Set(-1) 返回 1, 回读=-1（合法实现应保持 1）
  [OK]   wglSwapIntervalEXT 恢复原值: 恢复为 1: 返回 1
  [OK]   wglMakeContextCurrentARB(读写同窗口): 返回=1, 读 DC 就是窗口 DC, 当前上下文未变
  [OK]   wglCreateContextAttribsARB(3.2 core): MakeCurrent=1, 新上下文 GL_VERSION=3.2
  [OK]   wglCreateContextAttribsARB(9.9) 被拒: 返回 NULL
  [OK]   core+compatibility 同置被拒: 返回 NULL
  [OK]   wglShareLists + 纹理共享: 纹理名 1：源上下文可见=1，共享上下文可见=1
  [OK]   wglSwapMultipleBuffers(0, NULL): 返回 0
  [OK]   wglSwapMultipleBuffers(1, {窗口 DC}): 返回 1, 当前上下文未变, glGetError=0x0
  [OK]   wglGetDefaultProcAddress(未知名字): 返回 NULL

[降级入口]
       Windows：真实 GDI 能力，返回值交给驱动，只校验可调用且不改状态
  [OK]   图层平面入口(6 个): createLayer=0 describe=0 setPal=0 getPal=0 realize=0 swapLayer=1
  [OK]   wglCopyContext: 返回 FALSE
  [OK]   GDI 字形入口(4 个): bitmapsA=1 bitmapsW=1 outlinesA=0 outlinesW=0

[收尾]
  [OK]   线程级状态未被测试改变: 当前上下文=主上下文，当前 DC=主窗口 DC

汇总: PASS 16 / FAIL 0 / SKIP 1
```

两条与驱动有关的实测差异，都不记为失败：

- Intel 驱动接受 `wglSwapIntervalEXT(-1)` 并返回 TRUE（`WGL_EXT_swap_control` 要求拒绝），所以这条
  在 Windows 上即使不成立也只记 SKIP；swinx 上必须拒绝。
- `wglSwapMultipleBuffers` 的返回值 MS 没有文档化，Windows 实测与 swinx 一样返回"成功交换的个数"，
  因此断言只校验"调用后状态没被破坏"，不校验具体数值。

## 无人值守自检

```bat
opengl_demo.exe --selftest
```

启动后等约 1.2 秒（让 GL 出若干帧），然后：

- 先跑一遍 **WGL 入口测试**（见上一节）：放在抓图之前，它若把上下文状态弄坏，后面三张截图会立刻失败
- 保存固定管线画面到 `opengl_selftest.bmp`
- 切到 GLSL 路径再保存 `opengl_selftest_shader.bmp`
- 重建一次 GL 上下文（销毁 `HGLRC`、重建上下文与 VBO/着色器程序）再保存 `opengl_selftest_recreated.bmp`
- 写 `opengl_selftest.txt`：三条截图 + WGL 入口测试的结果、状态摘要、探针报告、入口测试明细、
  `[几何]`（realwnd 控件矩形 vs 原生子窗口矩形，用于确认两者重合）
- 全部成功（含 WGL 入口测试无 FAIL）退出码 0，否则 1

这条路径把“窗口能不能建、上下文能不能建、两条渲染路径能不能出图、上下文重建后还能不能画、
截图链路对不对”都串成了可自动检查的形式，实测：

- Windows（Intel Iris Xe / GL 4.6）：

```
selftest: PASS
snapshot(fixed pipeline): ...\opengl_selftest.bmp -> OK
snapshot(GLSL shader)   : ...\opengl_selftest_shader.bmp -> OK
snapshot(recreate ctx) : ...\opengl_selftest_recreated.bmp -> OK
WGL 入口测试 (PASS/FAIL/SKIP): 16/0/1
模式: GLSL 着色器    动画: 运行中    FPS: 41.3    窗口: 906x476  客户区: 906x476

[几何]
  realwnd 控件 (宿主坐标): 7,85,913,561 (906x476)
  GL 子窗口   (宿主坐标): 7,85,913,561 (906x476)
```

- macOS（Intel Iris Pro / GL 2.1，NSOpenGLContext 落地）：

```
selftest: PASS
snapshot(fixed pipeline): /tmp/ogl_selftest/opengl_selftest.bmp -> OK
snapshot(GLSL shader)   : /tmp/ogl_selftest/opengl_selftest_shader.bmp -> OK
snapshot(recreate ctx) : /tmp/ogl_selftest/opengl_selftest_recreated.bmp -> OK
GL_VERSION: 2.1 INTEL-18.8.16    GLSL: 1.20
GL_RENDERER: Intel Iris Pro OpenGL Engine    GL_VENDOR: Intel Inc.
[API 路径]
  GL 1.1 立即模式(glBegin/glEnd) : OK
  GL 1.5 VBO 入口              : OK
  GL 2.0 着色器入口            : OK
  GLSL 程序                    : 编译链接 OK

[几何]
  realwnd 控件 (宿主坐标): 7,85,913,561 (906x476)
  GL 子窗口   (宿主坐标): 7,85,913,561 (906x476)
```

macOS/Linux 的 `opengl_selftest.txt` 格式与上面一致，只是会多出 `WGL 入口测试 (PASS/FAIL/SKIP)` 一行，
以及末尾的入口测试明细（按各自驱动/swinx 兼容层的实际结果给出）。

## 注意事项

- 窗口类不是 `CS_OWNDC`，所以 `SetPixelFormat` 只做一次，并把 `GetDC()` 的 DC 一直持有到窗口销毁，
  保证 `wglMakeCurrent`/`SwapBuffers` 始终用同一个 DC（macOS/Linux 兼容层保持同一语义）。
- `SetPixelFormat` 对一个窗口只能成功设置一次，因此“重建 GL 上下文”只重建 `HGLRC` 与挂在上下文上的
  VBO/着色器程序，不动像素格式；重建前后用户的旋转/缩放视角会被保留。
- 截图用 `glReadPixels` 读后缓冲（先渲染不交换），BMP 存盘时把 RGB 交换成 BMP 要求的 BGR。
- 本 demo 走 legacy 2.1 上下文与 `#version 120`：固定管线与 GLSL 都能覆盖，macOS 桌面 GL
  上限正好是 2.1。swinx 的 WGL 兼容层已经提供 `wglCreateContextAttribsARB`（3.2/4.1 core）
  与 `wglSwapIntervalEXT`，需要现代上下文的代码按 Windows 写法即可使用；但 core profile 下
  固定管线和 `#version 120` 都不可用，所以本 demo 不切换。
- 本 demo 没有做 MSAA：需要 `wglChoosePixelFormatARB`（WGL_ARB_pixel_format），swinx 目前
  未提供该扩展，`wglGetProcAddress` 返回 NULL，调用方按"不支持"降级即可。
- Linux 上 GLX 上下文必须与窗口 Visual 匹配：swinx 用建窗时记录的 Visual ID 去
  `glXChooseFBConfig` 挑配置，挑不到时创建失败（多发生在该 Visual 无 GLX 支持的老式驱动上）。
  挑配置时会按 `24+8(模板) -> 24 -> 16 -> 不要求` 逐级放宽深度缓冲（不指定 `GLX_DEPTH_SIZE`
  时 GLX 按"至少 0 位"处理，容易选到没有深度缓冲的配置，`GL_DEPTH_TEST` 就形同虚设）。
- Linux 上 GLX 的 drawable 必须已经映射到 server。swinx 的窗口要等 `ShowWindow` 才 `map`，
  而本 demo 是"建窗后立刻建上下文"，所以 `wglMakeCurrent` 失败不会让上下文作废：
  swinx 记下这个状态，在首次 `SwapBuffers` 时补一次 make-current。
- Linux 上 swinx 给 GLX 装了自己的 Xlib 错误回调（只打印不退出）。Xlib 默认处理是直接
  `exit()`，窗口销毁/重排期间的 `BadWindow`/`BadMatch` 会直接把 UI 打死。
- macOS 上 AppKit 给一张 NSView 只有一张"当前 GL 表面"：另一个 `NSOpenGLContext` 对同一
  视图 `setView:` 会把它顶掉，而旧上下文的 `view` 属性仍指回该视图（比较指针发现不了）。
  swinx 因此在桥接层按视图记账：创建时不绑、`wglMakeCurrent` 时抢占、销毁后把剩余栈顶重绑。
  多个上下文混用一个窗口的代码（例如本 demo 的 WGL 入口测试）依赖这层记账，否则窗口需要
  重画时 GL 画面会整块变空白。

## swinx WGL 兼容层的覆盖范围（macOS / Linux）

> 实现细节与两平台方案见 `swinx/doc/swinx-OpenGL(WGL)跨平台实现.md`。

上下文管理按 Windows 的 WGL 语义提供，源码不需要平台分支：

| 分类 | 入口 | 说明 |
| --- | --- | --- |
| 核心 | `ChoosePixelFormat` / `DescribePixelFormat` / `SetPixelFormat` | 固定 32/24/8 + 双缓冲，恒选 1 号格式 |
| 核心 | `wglCreateContext` / `wglMakeCurrent` / `wglDeleteContext` / `wglGetCurrentDC` / `wglGetCurrentContext` / `SwapBuffers` / `wglSwapMultipleBuffers` | 上下文与创建它的窗口绑定；批量交换逐项等价 `SwapBuffers` |
| 核心 | `wglShareLists` | 支持"先创建、后共享、再 MakeCurrent"的常见顺序；上下文已创建后再共享返回 FALSE（平台只允许创建时指定 share group） |
| 扩展 | `wglCreateContextAttribsARB` | 按 `WGL_ARB_create_context` 语义解析版本/profile/flags；macOS 只提供 3.2 core 与 4.1 core，超出范围返回 NULL 由调用方降级 |
| 扩展 | `wglSwapIntervalEXT` / `wglGetSwapIntervalEXT` | Linux 依次尝试 `glXSwapIntervalEXT/MESA/SGI`；macOS 用 `NSOpenGLCPSwapInterval` |
| 扩展 | `wglGetExtensionsStringARB` / `wglGetExtensionsStringEXT` | 按平台实际能力拼装扩展串 |
| 扩展 | `wglMakeContextCurrentARB` / `wglGetCurrentReadDCARB` | 上下文只有一个 drawable：读写同一窗口可用，传入不同的读/写 DC 返回 FALSE |
| 降级 | `wglCopyContext` / `wglCreateLayerContext` / `wglDescribeLayerPlane` / `wgl*LayerPaletteEntries` / `wglRealizeLayerPalette` / `wglSwapLayerBuffers` / `wglUseFontBitmaps*` / `wglUseFontOutlines*` | 依赖 Windows 的图层平面与 GDI 字形，macOS/Linux 无对应能力，恒返回失败 |

扩展入口一律用 `wglGetProcAddress("wgl...")` 获取，与 Windows 上的写法一致；
`<GL/wglext.h>` 由 swinx 提供（扩展常量与 `PFNWGL...PROC` 类型），三平台都能解析。

