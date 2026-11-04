# fun_test

SOUI 的 gtest 单元测试工程（链接 `gtest`（含 gtest_main）+ `soui4` + `Scintilla`；非 Windows 平台额外链接 `swinx`）。

## 基本自动验证

从仓库根目录配置并构建 `fun_test` 后，执行：

```sh
cmake --build build --target fun_test --config Debug
ctest --test-dir build -L '^soui-headless$' --output-on-failure
```

CTest 在**配置期**扫描测试源码，把每个 `TEST` / `TEST_F` 注册成独立用例，并按**文件名约定**分层打标签（分组的实现与理由见 `CMakeLists.txt` 文件头注释）：

| 源文件                            | 层级标签                                                      | 注册用例数（2026-11-05 实测） |
| ------------------------------ | --------------------------------------------------------- | --------------------- |
| 其余 `test_*.cpp`                | `soui-unit`                                               | 511                   |
| `test_integration_*.cpp`       | `soui-integration`                                        | 1                     |
| `test_e2e_*.cpp`               | `soui-e2e`                                                | 12                    |
| `test_soui_uitest_*.cpp`       | `soui-uitest`（SouiTest 框架，Tier A 框架级注入，无头）            | 37                    |
| **门禁合计**                       | **`soui-headless`**（= 上面四行；1 条 `DISABLED_` 不执行，实跑 560） | **561**               |
| `test_gui_*.cpp`               | `soui-gui`，需 `-DSOUI_ENABLE_GUI_SMOKE=ON`                 | 4                     |
| `test_gdi.cpp`、`test_soui.cpp` | `soui-interactive`，需 `SOUI_ENABLE_INTERACTIVE_TESTS=ON`   | 2                     |

> 用例数会随用例增删变化，以 `ctest --test-dir build -N -L '^soui-headless$' | tail -1` 为准；上表只用来一眼看出各层量级。

`soui-headless` 是门禁标签，`pr-core.yml` 与 `build.yml` 都是 `ctest -L '^soui-headless$'`；`soui-unit` / `soui-integration` / `soui-e2e` 用来单跑某一层。最后两层**不带** `soui-headless`，所以打开它们的开关也不会把"需要桌面"或"需要人工操作"的用例拖进门禁。

新增常规 `TEST`/`TEST_F` 用例只要文件按约定命名，重新构建就会进入相应标签，**不必改 CMakeLists**。三条限制：① 扫描发生在配置期，新增文件后需让 CMake 重跑一次（`CONFIGURE_DEPENDS` 一般能自动触发）；② 参数化用例（`TEST_P`）只能扫出模板、拆不出全部实例，须单独核验；③ 分组以文件为界，同一套件若跨文件可能分属不同层（`soui_component` 的用例就分别在 `test_module.cpp` 与 `test_integration_resprovider.cpp` 里），标签只是登记层级，不代表语义纯度。注册结果用 `ctest --test-dir build -N -L '^soui-headless$'` 核对，单组用例可用 `ctest --test-dir build -R '^soui_matrix\.' --output-on-failure` 复跑。

E2E 不打开交互窗口，暂不验证屏幕显示、鼠标输入或窗口容器中的事件派发。测试失败会输出用例日志并返回非零状态。

Linux/macOS 默认在构建 `fun_test` 后运行全套非交互测试；PR 快速构建可配置 `-DSOUI_FUN_TEST_POST_BUILD=OFF` 关掉该构建后动作，再显式运行上面的 CTest 门禁。两条路径覆盖面一致（都是"除 `window.*` 外的全部用例"），差别只在粒度：门禁逐用例各起一个进程、可 `-j` 并行、失败定位到用例；构建后动作是单进程一次跑完，快得多。全套也可手动运行 `fun_test '--gtest_filter=-window.*'`。

## 无头 UI 自动化测试（SouiTest）

`test_soui_uitest_*.cpp` 属于 `soui-uitest` 标签，并随 `soui-headless` 门禁一起跑（无需显示设备）。它基于设计文档 `doc/SOUI4-UI自动化测试框架设计.md` 的 **SouiTest** 支撑库（`components/souitest/souitest.h`），用框架级事件注入（`SWindow::SSendMessage`，即 Tier A）驱动控件并验证行为：

- **对象定位**：`SouiTest::findByName<T>` / `findByPath` / `findByClass` / `findIf`。
- **输入注入**：`SouiTest::mouseClick` / `keyClick`（Tier A，无头）；`SouiTest::nativeMouseClick` 为 Tier B 原生级（仅 `test_gui_uitest_*` 对照用例使用）。
- **行为验证**：`SouiTest::SignalSpy` 订阅任意事件（如 `EventCmd::EventID`、`EventCBSelChange::EventID`、`EventTabSelChanged::EventID`），等价 `QSignalSpy`。
- **无头基座**：复用本目录 `test_container.h` 的 `TestContainer`（继承 `SwndContainerImpl`，桩掉绘制/宿主纯虚函数）+ `HeadlessApp` + `CreateRootFromXml`，使控件树有 `ISwndContainer` 从而 `FireEvent` 能派发，且不需要原生窗口。

新增 UI 用例只需新建 `test_soui_uitest_*.cpp`、按上面的夹具写 `TEST_F(SouiTestFixture, ...)`，重新构建即自动纳入 `soui-uitest` / `soui-headless`，**不必改 CMakeLists**。用例内部只允许 Tier A 注入；需要完整输入管线保真（命中测试、焦点、hover、IME）的用例放 `test_gui_uitest_*.cpp`（受 `SOUI_ENABLE_GUI_SMOKE` 门控）。

## GUI 烟测

在有桌面显示的环境配置 `-DSOUI_ENABLE_GUI_SMOKE=ON`，构建后运行 `ctest --test-dir build -L '^soui-gui$' --output-on-failure`。Linux 可用 `xvfb-run -a` 包裹 CTest。`window.gui_smoke_dispatches_button_click` 创建并显示真实 SOUI 宿主窗口，通过窗口过程发送鼠标消息并检查按钮事件；它不会长期等待人工操作。GUI 组默认不注册，门禁仍是那批无头用例（见上表）；全套非交互 `fun_test` 的 `-window.*` 过滤器也排除 GUI 组。此烟测尚不比较渲染像素或验证真实设备鼠标。

## 交互式窗口用例

`test_gdi.cpp`（`window.gdi`）与 `test_soui.cpp`（`window.soui`）会开窗并**等待人工操作**、没有时间上界，因此默认不注册（`TIMEOUT 0` = 不设超时）。需要人工验证时显式打开：

```sh
cmake -S . -B build -DSOUI_ENABLE_INTERACTIVE_TESTS=ON
cmake --build build --target fun_test
ctest --test-dir build -L '^soui-interactive$' --output-on-failure
```

不要把它加进门禁：它既需要桌面，也需要人。


## wasm 腿（swinx GDI 后端的 A/B）

`swinx` 在 wasm 上有两个 GDI 后端：`canvas2d`（默认，光栅化交给浏览器 canvas）和 `cairo`（光栅化在 wasm 模块内）。桌面构建只能跑到其中一个，后端差异要靠同一批用例在两个后端各跑一遍才能暴露。`-DSOUI_BUILD_WASM_FUN_TEST=ON` 把 `test_dc.cpp` / `test_region.cpp` / `test_text.cpp` / `test_swinx_handle_type_unit.cpp` 四个纯 Win32 兼容面的 TU 编成一个 wasm 可执行文件（源集与不注册 CTest 的理由写在 `CMakeLists.txt` 第 0 节）。

```sh
source ~/emsdk/emsdk_env.sh
EMC=~/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
# 后端由 -DSWINX_WASM_GDI_BACKEND 选择，产物落在 demos/fun_test/out.<backend>/
cmake -S . -B build-ft-c2d -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=$EMC \
      -DSOUI_BUILD_WASM_FUN_TEST=ON -DSWINX_WASM_GDI_BACKEND=canvas2d
cmake -S . -B build-ft-cai -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=$EMC \
      -DSOUI_BUILD_WASM_FUN_TEST=ON -DSWINX_WASM_GDI_BACKEND=cairo
cmake --build build-ft-c2d --target fun_test -j8
cmake --build build-ft-cai --target fun_test -j8
```

两条运行载体，证明范围不同（细节见两份脚本的文件头注释）：

```sh
# node：假 DOM + 假 canvas，绘制调用是 no-op、getImageData 全 0
node demos/fun_test/wasm/run-node.js demos/fun_test/out.canvas2d/fun_test-wasm.js \
     [--gtest_filter=SwinxDcTest.TextPath*]
node demos/fun_test/wasm/run-node.js demos/fun_test/out.cairo/fun_test-wasm.js

# 浏览器：真 canvas、真字体。服务器根 = demos/fun_test
python3 -m http.server 8931 --directory demos/fun_test
# http://127.0.0.1:8931/wasm/browser-runner.html?backend=out.canvas2d
# 取结果：window.__ftSummary() -> {done, failed[], okCount, lines}；__ftFailureText() 给断言原文
```

- **node 腿**证明：句柄与对象生命周期、参数编组、区域布尔运算、返回值与 `GetLastError` 口径、wasm↔JS 边界不崩不泄。**像素往返、依赖真实字形/光栅的断言在这一腿不可判定** —— 桩件不给像素，不是实现有错。
- **浏览器腿**证明：canvas2d 的实际光栅结果与文本度量；cairo 腿在这一腿只是复认，它的光栅在模块内，node 已可信。
- 以 cairo 为已知正确的基线做 A/B：两个后端跑同一批用例（当前 88 条），差集就是 canvas2d 的缺陷清单。
  第二轮修复后的实测（2026-11-06，当时 87 条）：cairo node 87/87、cairo 浏览器 87/87、canvas2d 浏览器 87/87、canvas2d node 84/87（3 条属上面那类不可判定：`compatible_bitmap_and_pixels`、`dib_section_setpixel_updates_memory`、`patblt_blackness_whiteness_24bpp`）。
  同一批用例跑在修复前的 swinx（子模块 `9f5484b1`，其余不变）上，canvas2d 浏览器腿失败 5 条，就是这一轮修的缺陷：`stock_objects`（库存对象被 `DeleteObject` 释放后成为悬垂句柄）、`selectcliprgn_replaces_previous_clip` / `extselectcliprgn_copy_recovers_from_leaked_clip` / `selectcliprgn_null_removes_clip`（canvas 的裁剪只会求交，替换/移除裁不掉旧裁剪，`GetClipBox` 回报 NULLREGION）、`moving_text_after_leaked_clip_leaves_no_residue`（漏出的窄裁剪把之后每一帧的清屏困住，飘字残影留在画布上）。cairo 两条腿在修复前后都是满通过，说明这批断言测的是后端差异而不是用例本身。
- **Blt 族的 rop 是"替换"不是"叠加"**（第二轮，85 → 87）：`bitblt_srccopy_replaces_destination` 与 `stretchdibits_srccopy_replaces_destination` 先在目标上留一枚金色标记，再用**整块透明**的源（`ClearRect(mem, rc, 0)` 的内存 DC / 32bpp 全 0 的 DIB）以 `SRCCOPY` 盖过去，要求旧标记消失、源里那一枚不透明点到达、矩形外的蓝色标记活着。基线（子模块 `172b3ceb`，只把对应函数换回修复前）上这两条各红一次，红的都是"旧标记还在"那一条：`test_dc.cpp:898`（BitBlt）、`test_dc.cpp:973`（StretchDIBits）。canvas2d 把 rop 映成 `source-over`，透明源擦不掉任何东西；Win32 的 32bpp `SRCCOPY` 与 cairo 的 `CAIRO_OPERATOR_SOURCE` 都是连 alpha 一起替换。SOUI 的 present 只有这一次拷贝（`SHostPresenter::OnHostPresent`），而 `_RedrawRegion` 每帧先把脏矩形 `ClearRect(…, 0)` 清成全透明 —— 于是移动的文字会在身后拖出残影，就是 `demos/wasm-demo/bug.png` 那串金色抛物线。
- **DrawText 自己要把字形裁进格式化矩形**（第三轮，87 → 88）：`drawtext_clips_to_format_rect` 是这一条的第一份证据。上面所有裁剪用例都是调用方先 `SelectClipRgn` 再画，而 Win32 的 `DrawText` 不看调用方的裁剪、自己裁 `RECT`（只有 `DT_NOCLIP` 放开）。SOUI 侧 `SWindow::DispatchPaint` 只应用控件显式的 `m_clipRgn`/`m_clipPath`，**不把孩子窗口裁到自己的窗口矩形**，脏矩形又只按控件矩形失效 —— 于是越出矩形的字墨此后再没有任何一次重绘会覆盖它，飘字动画就在轨迹上留下永久残影。cairo 后端 `src/gdi/cairo/drawtext.cpp` 一直是裁的，canvas2d 的 `drawTextLines()` 漏了，`DT_NOCLIP` 也被忽略。
  用例三条断言配对：16x16 矩形里画 6 个 20px 粗体 `W` 时框内必须有墨（否则"整体不画"也能过）且框外必须全白；同一矩形带 `DT_NOCLIP` 重画时 `x>=32` 必须有墨（否则"永远不画"也能过）；随后 `x=40..56` 的 `FillRect` 必须到达（证明裁剪是 `SaveDC` 级作用域，没漏到调用方 DC）。
  **证伪是在已知正确的基线上做的**：把 cairo 那侧的 `if (!(m_uFormat & DT_NOCLIP))` 临时改成 `if (false && …)` 重编原生 `fun_test`，用例红在"墨越界"这一条（`test_dc.cpp:1481: gold ink at 20,9 escaped the formatting rect`），恢复后转绿 —— 断言钉的就是矩形裁剪这件事，canvas2d 修复前缺的也是同一件。
  第三轮实测（2026-11-06）：cairo node 88/88（cairo 的光栅在模块内，这一腿能判像素，新用例的证据来自这里）、canvas2d node 85/88（新用例被 `pixelsReadable()` 早退，本腿不作证据）、原生 `fun_test` 的 cairo 腿全绿；canvas2d 浏览器腿由人工开页复跑并通过 —— 产物做两份：`out.canvas2d`（含修复）与 `out.canvas2d.red`（`git show HEAD:` 的修复前 swinx 重编，`.wasm` 差 819 字节），红构建只在"墨越界"这一条断言上失败、绿构建通过。当时这条 A/B 只证明**后端不对称**，还不等于演示里 `bug.png` 那串残影结案；随后用户在 canvas2d 后端把俄罗斯方块 1–10 关连测 4 次（文字特效形态）全部无残影，应用腿才一并结案。因果链与"为什么这三条修齐才有效"见 `doc/soui-wasm-canvas2d-残影修复说明.md`，过程记录见 `doc/soui-wasm-port-plan.md` §31–§37。
  结案后 `out.canvas2d.red/` 与 `/tmp/wasm-demo-red/` 这两份红产物已删除（本来就在 `.gitignore` 的 `/out.*/` 里）；要复跑红/绿 A/B 按 §36.4 记录的命令重建，注意主仓库历史里有两个 swinx 指针在本地和远端都取不到（`da85cbe1`、`56b70c82`），红基线要用能取到的修复前提交（`31179e6a`）。
- **像素断言要成对写**：只断言"清屏后那个点是黑/白"分不清"裁剪放宽了"和"画布被整体清空"（`ctx.reset()` 就是把画布清成透明黑，读回来也是 0,0,0）。所以裁剪类用例同时在"旧裁剪内、新裁剪动作覆盖不到的地方"留一枚红色标记，要求它还在；`moving_text_after_leaked_clip_leaves_no_residue` 则要求整幅回到白色。把 `clipClearCanvas` 临时换成 `ctx.reset()` 版本重跑，这三条的"画布被清空"断言和残影用例全数失败，证明它们能挡住这种"以破坏换通过"的改法。Blt 族两条同理：单看"旧标记消失"会放过"把整块画布擦掉"的实现，必须同时要求不透明源点到达、矩形外不动。
- 产物目录 `out.*/` 已在 `demos/fun_test/.gitignore` 里排除，不入库；`out_pt/` 之类的临时目录同理。

## 目录结构


```
fun_test/
├── CMakeLists.txt   # 编译 + CTest 分组注册（分层约定与理由见文件头注释）
├── common.h         # 用例共用：源码目录定位（getSourceDir）、LoadPng
├── ScintillaWnd.cpp # Scintilla 控件封装
├── test_gui_smoke.cpp / test_gdi.cpp / test_soui.cpp
│                    # 窗口用例：前者需桌面（soui-gui），后两者需人工操作
│                    # （soui-interactive），三者默认都不注册
├── test_soui_core_unit.cpp
│                    # SOUI 内核无窗口单测：SLayoutSize 语义、SMatrix 逆变换、
│                    # 命名子窗口按层级查找、链式成员属性的命名空间前缀（常量集中在
│                    # SAttrCracker.h；前缀只路由给声明它的成员、不匹配的成员根本不被
│                    # 问到、未声明前缀不得当裸名、同名属性靠前缀区分、大小写不敏感；
│                    # 名字在 SOUI_ATTRS_BEGIN 里只切一次，成员读剥后名，而对象自己的
│                    # 属性表始终比较完整名字，所以带前缀的名字不会落到自己表里）
├── test_lua.cpp      # script_lua 模块导出 SOUI 对象的创建/释放调试测试（见"说明"）：
│                     # SComMgr2 加载模块、零 lua 链接、脚本经
│                     # IScriptModule::executeScriptBuffer 创建对象；每个导出对象
│                     # 一个用例只做"创建+释放"，按释放路径分三类（GC / 显式
│                     # Release / 无释放路径预期泄漏），供人工断点跟踪 C++ 生命周期
├── test_sync.cpp     # swinx（Win32 兼容层）模块专项测试（下列 test_*.cpp 均属此类，
│                      # 仅在非 Windows 平台链接 swinx；Windows 宿主按真实 Win32 API
│                      # 编译运行并应同样通过）：
│                      # 内核同步对象：事件（手动/自动复位、命名、跨线程）、
│                      # WaitForMultipleObjects、互斥锁（含跨线程互斥）、信号量、Interlocked、
│                      # SOUI SSemaphore 助手（自 test.cpp 迁入）；
│                      # 跨线程持有/释放同样用事件握手，不依赖 Sleep 时间差
├── test_thread.cpp   # 线程：CreateThread/join、多线程并行、SuspendThread/ResumeThread、tid 唯一性
├── test_beep.cpp     # MessageBeep 系统提示音（平台实现：Linux=X11 Bell 请求、macOS=NSBeep()、
│                     # iOS=AudioServicesPlayAlertSound、移动端经 g_platformAPI.audio.messageBeep
│                     # 回调宿主应用发声）：
│                     # 提示音是否可闻取决于运行环境（声卡、有无 X 连接、X server 的 xset b），
│                     # 故不断言"一定成功"——只验证调用安全 + 返回值是合法 BOOL（同时回归
│                     # helper 跨 TU 的 int/BOOL ABI：声明在 swinx/src/SwinxUtils.h、实现在
│                     # 平台目录）与同一环境下多次调用结果稳定
├── test_file.cpp     # 文件：CreateFileA 访问模式（含只读文件 GENERIC_READ 回归用例）、
│                     # 读写/seek/size、目录操作、CopyFile/MoveFile、
│                     # MoveFile 边界（目标存在必败、源缺失必败、W 版中文文件名）、
│                     # FindFirstFile/FindNextFile/FindClose 通配符枚举、FindFirstFileExW
│                     # 宽字符文件名枚举、_open_osfhandle/_get_osfhandle 与句柄共享
│                     # 文件位置往返、Get/SetFileAttributes 只读往返、GetTempPath/GetTempFileName、
│                     # SetEndOfFile 截断、GetFileSizeEx、
│                     # GetCurrentDirectory/SetCurrentDirectory、Get/SetFileTime 往返、
│                     # GetStdHandle 写出（invalid selector → INVALID_HANDLE_VALUE）、
│                     # 打开既有 uires.zip 并读取（自 test.cpp 迁入）
├── test_string.cpp   # 字符串：UTF-8/CP936 转换回环、GetStringTypeExW/A（ctype1 回归用例）、
│                     # CharLower/lstrlen
├── test_memory.cpp   # 内存：Global/Local/Heap 分配器、LocalHandle、GlobalReAlloc 内容保持
├── test_gdi.cpp      # GDI：对象管理与 DC 状态、GDI Path 全套（自 test.cpp 迁入）、贝塞尔曲线
│                     # （DC 附加用例在 test_dc.cpp：CreateDIBSection 元数据、GetDIBits 往返、
│                     #  SetPixel 落盘、PatBlt BLACKNESS/WHITENESS、BitBlt SRCCOPY；
│                     #  文本测量：DrawText DT_CALCRECT、GetTextFace 真机截断语义、GetCharWidth、
│                     #  GetTabbedTextExtent 默认 8 字符制表位；世界变换：Set/Modify/
│                     #  GetWorldTransform 平移与缩放、LPtoDP/DPtoLP 往返、MWT_IDENTITY 重置）├── test_hook.cpp     # 钩子：Set/Unhook/CallHook 分发与类型隔离、
│                     # 并发 unhook vs CallHook 回归用例（引用计数生命周期修复）
├── test_time.cpp     # 时间：GetTickCount 单调性、QueryPerformanceCounter/Frequency、
│                     # GetLocalTime/GetSystemTime 合法性、FILETIME 转换族
│                     # （CompareFileTime 排序、LocalFileTime 双向往返、DOS 日期时间往返）
├── test_oleauto.cpp  # OLE 自动化数据 API：BSTR（SysAllocString/AllocLen/AllocByteLen、
│                     # SysReAllocString/ReAllocLen 换长与 NULL 源语义、
│                     # 嵌入 NUL 的 ByteLen 语义、NULL 安全）、VARIANT（Init/Clear/Copy、
│                     # VT_BSTR 深拷贝独立存储、Clear 对 VT_DISPATCH/VT_UNKNOWN 走
│                     # Release 与 VT_ARRAY 走 SafeArrayDestroy 的引用计数闭环、
│                     # VT_BYREF 点属调用者不被释放、VT_VARIANT|VT_BYREF 解引用拷贝
│                     # （SOUI accValidateNavStart 依赖）、Copy 数组分支回填 vt）、
│                     # CoTaskMemAlloc/Realloc/Free（前缀保持）、
│                     # CoGetMalloc/IMalloc（Alloc/Realloc/Free、与 CoTaskMem 同分配器、
│                     # 非 MEMCTX_TASK 拒绝）、IIDFromString（知名 GUID、大小写不敏感、
│                     # CoCreateGuid 格式化往返、畸形串拒绝）、CoCreateInstance
│                     # DragDropHelper 支持 IID_IUnknown 请求（仅 swinx 断言）
├── test_oleacc.cpp   # MSAA（双平台同一套用例：Windows 走真实 oleacc.dll/user32，
│                     # 其余走 swinx；断言不一致即 swinx 未对齐 Windows 语义）：
│                     # LresultFromObject/ObjectFromLresult 往返（功能等价断言，不假设
│                     # 指针同一性；每次调用新句柄、两个句柄均可独立解析；对象不支持
│                     # riid 返回 0；非法参数失败）；
│                     # AccessibleObjectFromWindow 端到端闭环（测试窗口 WM_GETOBJECT
│                     # 应答 LresultFromObject，与 SHostWnd::OnGetObject 同模式）；
│                     # AccessibleObjectFromEvent（WM_GETOBJECT 解析 + pvarChild 回带
│                     # child id）；AccessibleChildren 枚举（child id 从 iChildStart+1
│                     # 开始、不含 CHILDID_SELF；部分返回 S_FALSE）；
│                     # GetRoleText/GetStateText A/W、参数校验；
│                     # OUTOFCONTEXT 钩子经消息循环异步投递（NotifyWinEvent 返回时
│                     # 回调未执行）、多钩子区间过滤、SKIPOWNPROCESS/SKIPOWNTHREAD
│                     # 注册成功但收不到本进程/本线程事件、注销语义；
│                     # swinx 内部不变量回归（#ifndef _WIN32）：IAccessible::GetIID
│                     # 自解析（DECLARE_CLASS_SIID）、弱表不续命（最后一次 Release
│                     # 返回 0，胶水架构不持有 IAccessible）
├── test_cs.cpp       # 临界区（递归进入、跨线程 TryEnter）、SRWLock（独占/共享、并发读者）；
│                     # 跨线程用例一律用事件握手定序（先持锁再起线程、释放后用事件通知），
│                     # 不靠 Sleep 估算时间点
├── test_profile.cpp  # INI 配置：WritePrivateProfile*/GetPrivateProfile* 读写回环、
│                     # 缺省值、覆盖写、多 section、GetPrivateProfileIntA、
│                     # GetPrivateProfileSection 全键值遍历（空 section → 空）、
│                     # GetPrivateProfileSectionNames、GetPrivateProfileStruct 校验和往返
│                     # （篡改 hex 后必败）
├── test_module.cpp   # 模块与进程：GetModuleFileNameA(NULL)、LoadLibrary/GetProcAddress/
│                     # FreeLibrary（libm 实调 sqrt，按平台选库）、进程/线程 ID、GetModuleHandleA(NULL)、
│                     # CreateProcess/GetExitCodeProcess/GetProcessId（双平台分叉：
│                     # POSIX=swinx 约定（lpCommandLine 只含参数、hProcess 为"子进程已启动"事件、
│                     # 收尸后 GetExitCodeProcess 才 TRUE）；Windows=真 Win32（cmd.exe /c、
│                     # hProcess 退出时 signaled、STILL_ACTIVE 轮询；probe 锁定
│                     # GetExitCodeProcess(INVALID_HANDLE_VALUE)=伪当前进程句柄→TRUE+259）、
│                     # 多子进程并发退出（POSIX：SIGCHLD 合并/self-pipe 排水线程回归；
│                     # Windows：6 并发子进程退出码隔离；
│                     # 6 个子进程各以不同 code 退出，必须全部对应）；
│                     # SOUI 组件（soui_component 套件，自 test.cpp 迁入）：
│                     # ITaskLoop 创建/post/stop、ZIP 资源提供器加载与 uires.idx 解析/图标加载
├── test_flock.cpp    # swinx 文件锁回归（仅非 Windows 编译，Windows 无此失败模式）：
│                     # 全局句柄表锁由命名信号量改为 fcntl 记录锁后，"持锁进程崩溃
│                     # →下次启动卡死"必须根除。三用例：
│                     # crash_holder_releases_flock（持锁者存活时排他、SIGKILL 后内核
│                     # 自动释放、新进程经 swinx 锁类无阻塞重入）、
│                     # named_objects_usable_after_abrupt_exit（进程 _exit 猝死后
│                     # 全局句柄表仍可用：命名对象可重开、新对象可创建）、
│                     # startup_survives_crashed_lock_holder + startup_probe
│                     # （原失败场景复现：fork 持有全局表锁→SIGKILL→CreateProcessA
│                     # exec 全新进程重跑静态构造期并建命名对象，看门狗把卡死转为用例失败）
├── test_multimon.cpp # 多显示器 API（Windows 对真实 Win32 API 跑作基准，非 Windows 对
│                     # swinx 兼容层跑；无显示连接自动跳过，旧版 gtest 无 GTEST_SKIP
│                     # 用 MULTIMON_SKIP=SUCCEED+return 兼容）：
│                     # EnumDisplayMonitors（枚举≥1、裁剪矩形语义、回调矩形非空）、
│                     # GetMonitorInfo（主屏几何有效、MONITORINFOF_PRIMARY、工作区不越界、
│                     # cbSize 校验）、MonitorFromPoint（桌面内命中/桌面外按
│                     # NULL/NEAREST/PRIMARY 标志）、MonitorFromRect（中心命中、
│                     # 远处 NULL、跨屏大矩形命中）、MonitorFromWindow（NULL 窗口
│                     # 标志语义）、EnumDisplayDevices（设备枚举与越界索引）
├── test_wnd_rect.cpp # 窗口几何（AdjustWindowRectEx）。仅非 Windows 断言 swinx 的非客户区
│                     # 模型，Windows 宿主跑真机、只保留那条跨平台用例：swinx 把**自绘的
│                     # WS_BORDER 一圈**算进窗口矩形（厚度 SM_CXEDGE/SM_CYEDGE），标题栏与
│                     # 调整边框由原生窗口管理器画在窗口矩形**之外**、菜单栏由 SOUI 自绘，
│                     # 故这三项对矩形的贡献为 0；WS_CAPTION 的定义含 WS_BORDER 位，
│                     # 必须先判标题栏再把这圈边框归一化清掉（与 WIN_CreateWindowEx 一致）；
│                     # NULL 矩形返回失败、menu 与扩展样式标志不改动矩形；
│                     # 跨平台用例断言"与 GetClientRect 互逆"
├── test_region.cpp   # 区域：CreateRectRgn/Indirect、GetRgnBox、RectInRegion、EqualRgn、
│                     # CombineRgn（OR/AND/DIFF/XOR 返回类型与边界，含包含关系 OR 盒不变、
│                     # 整列 DIFF 剩余矩形——自 test.cpp 迁入）、OffsetRgn、
│                     # PtInRegion（左上闭/右下开边界）、GetRegionData（尺寸/头/单矩形）
├── test_text.cpp     # GDI 文本：CreateFontA/SelectObject、GetTextExtentPoint32A、
│                     # GetTextMetricsA（结构性断言，字体无关）
├── test_shell.cpp    # Shell/路径（路径字面量按平台分叉：_WIN32 用盘符+反斜杠、POSIX 用 '/'，
│                     # 期望值也随原生形式不同，如 CP("C:\\dir\\*")=6 vs CP("/dir/*")=4）：
│                     # PathFileExists/PathIsDirectory（INVALID 属性回归）、
│                     # PathIsRelative（Windows: 盘符/盘相对形式；POSIX: '/' 开头即绝对）、
│                     # PathMatchSpec（多模式：真机默认支持、swinx 需 PMSF_MULTIPLE）、
│                     # PathCommonPrefix、PathFindFileName、PathQuoteSpaces（已带引号输入两端不同）、
│                     # PathCanonicalize（断言仅限去 ".." 语义）、GetFullPathName（绝对/相对解析）、
│                     # PathUnquoteSpaces（配对引号才动作，与 QuoteSpaces 往返）、PathIsPrefix
│                     # （前缀后须跟分隔符，等路径也算，盘根 "C:\" 例外）、PathFindExtension
│                     # （'.' 在分隔符/空格处重置）、PathCommonPrefix
│                     # （组件级语义：盘根含分隔符、部分组件不算、大小写不敏感；
│                     #   POSIX 侧 '/' 根非组件边界，CP("/foobar","/foo")=0），
│                     # CoCreateGuid 唯一性、SafeArray 全生命周期
├── test_misc.cpp     # 杂项：环境变量、MulDiv（四舍五入语义）、IsBadReadPtr（注释保留）、
│                     # InterlockedCompareExchange/Decrement、Sleep 最短时长、
│                     # Sleep(1000) 真的睡满（tv_usec 越界回归）
│                     # GetSystemInfo/GetVersionEx(NT 平台位)、GetCommandLineA 非空、
│                     # GetComputerName/GetUserName（nSize 语义：前者不含 NUL、后者含）、
│                     # IsBadStringPtrA、IsDBCSLeadByte（swinx=UTF-8 首字节判定，仅非 Windows）、
│                     # GetSystemMetrics（屏幕/边框正值）、SMatrix 缩放矩阵乘法组合
│                     # （自 test.cpp 迁入）
├── test_strapi.cpp   # 字符串辅助：_splitpath/_wsplitpath（绝对/相对/混合分隔符/多点扩展名、
│                     # 盘符仅真机）、CharNextA（ASCII 步进；UTF-8 多字节步进仅非 Windows）、
│                     # CharToOem/OemToChar（A/W 往返、Buff 按长度转换不终止）、
│                     # StrToIntExA/StrToInt64ExA（符号、前导零、截断于首个非数字、
│                     # 0x 十六进制需 STIF_SUPPORT_HEX、int64 全幅、非数字拒绝）、
│                     # GetACP（swinx=CP_UTF8）/IsValidCodePage
├── test_kernel.cpp   # 内核对象：TLS、Atom（本地/全局表）、SRWLock TryAcquire、
│                     # 可等待定时器（一次性/周期/取消/重设 due/命名 Open/TimerQueue，全工程
│                     # 定时器用例唯一所在文件）、CreateDIBSection/GetDIBits 往返、
│                     # CreateFileMapping/MapViewOfFile（匿名+命名 OpenFileMapping）、
│                     # VirtualAlloc/VirtualFree
├── test_utilities.cpp # utilities 库 helper（`helper/STls.h` 线程局部存储）：
│                     # ID 懒分配（首次 Set 才分配全局槽位、Get 不分配）、
│                     # 按线程隔离（子线程看不到主线程的值）、析构回调 + 手动 Cleanup
│                     # （含 NULL 值槽位同样回调、Cleanup 后存储仍可复用）、
│                     # 多 ID 独立 + 超出 TLS_ALLOC_CHUNKSIZE=4 的扩容路径、
│                     # id 为 NULL 的参数校验与空 Cleanup 幂等
├── test_dc.cpp 补充  # CreateIconIndirect/GetIconInfo 往返（fIcon/hotspot/位图尺寸与内容、
│                     # GetIconInfo 返回新拷贝需 DeleteObject；真机语义：icon 的 hotspot
│                     # 强制取位图中心、mask 高度须等于 color 高度，2x 反报 87）
│                     # 与裁剪/飘字一组：SelectClipRgn 是替换不是求交、ExtSelectClipRgn
│                     # (RGN_COPY) 要能放宽回整幅、SelectClipRgn(hdc,NULL) 移除裁剪、
│                     # 裁剪必须困住文字墨迹、飘字动画逐帧清屏后不留残影，以及"先漏出
│                     # 窄裁剪再让帧清屏放宽"的俄罗斯方块"+分数"重现（bug.png）；
│                     # 每条都带"旧裁剪内的标记必须活着"的配对断言
│                     # 与 Blt 族一组：BitBlt / StretchDIBits 的 SRCCOPY 是替换不是叠加
│                     # （透明源必须擦掉目标旧墨迹、不透明源点要到达、矩形外不动）
├── wasm/             # wasm 腿的两条运行载体：run-node.js（node + 假 DOM/canvas 桩）、
│                     # browser-runner.html（真 canvas，服务器根 = 本目录）。证明范围与
│                     # 桩件边界见上面的"wasm 腿"一节
└── ...
```

## 说明

- `test_*.cpp` 中的 swinx 用例断言全部按标准 Win32 行为编写：非 Windows 平台跑 swinx 兼容实现，    
  Windows 宿主（真实 Win32 API）参与编译且应同样通过（已用 MSVC + 真机验证）。
- **CTest 注册的两个坑**（都源于"配置期扫源码文本"这一机制）：
  - **平台条件分支下的用例名必须互不相同。** `gtest_add_tests` 扫的是源码文本而非预处理结果，      
    `#ifdef` 的每个分支都会被扫到；而 CTest 会把 `DISABLED_` 前缀从用例名里剥掉。于是      
    `#ifdef _WIN32` 写 `DISABLED_foo`、`#else` 写 `foo` 会让两条记录重名，CMake 在**配置期**      
    直接报 `add_test given test NAME ... which already exists in this directory`。      
    `test_misc.cpp` 的 `is_dbcs_lead_byte` 正是这个坑，Windows 分支现名      
    `DISABLED_is_dbcs_lead_byte_codepage_dependent`。
  - 被平台条件编译掉的用例，其 CTest 记录在其他平台上**仍然存在并"空跑通过"**（gtest 过滤不到      
    任何用例时返回 0）。所以某个平台上"400 个全过"里可能含若干空条目，不能当覆盖证据。
  - **连注释掉的 `TEST` 也会被注册**：注册是纯文本正则匹配（`GoogleTest.cmake` 的      
    `(TYPED_TEST|TEST)_?[FP]?` 用的是**未锚定行首**的 `REGEX MATCH`），所以      
    `//TEST(suite, name)` 同样产出一条空跑条目——`test_misc.cpp:51` 的      
    `//TEST(swinx_misc, is_bad_read_ptr)` 正是如此（400 条里有 1 条来自它，源码实际只有      
    19 个 TEST 却注册出 20 条）。要让某用例彻底退出门禁，必须整行删除，不能只注释掉。
- 某一层分组为空会让 CMake **配置期直接失败**（`soui_assert_group_not_empty`），    
  避免该层悄悄退化成 0 个用例、而门禁依旧全绿。
- 运行方式：构建后在 `uires` 同级目录执行 `fun_test`，可用 `--gtest_filter=swinx_*` 只跑 swinx 用例。
- 部分用例是近期修复的回归测试，标注 `Regression`：
  - `test_file.cpp` `readonly_file_generic_read`：CreateFileA 权限位 `|`/`&` 恒真 bug。
  - `test_string.cpp` `get_string_type_*`：GetStringTypeEx 字符类型判定永真 bug。
  - `test_hook.cpp` `concurrent_unhook_vs_call` / `churn_while_calling`：钩子对象引用计数生命周期（UAF）修复。
  - `test_memory.cpp` `local_alloc_lock_handle`：LocalHandle 的 HeapLock 配对修复。
  - `test_flock.cpp` `swinx_flock.*`：全局表锁命名信号量→fcntl 文件锁回归（持锁崩溃后启动不再卡死；      
    阻塞点均有 SIGALRM 看门狗，锁泄漏时判失败而非挂死）。
  - `test_misc.cpp` `sleep_over_one_second`：swinx `Sleep` 把 ms*1000 直接写进      
    `timeval::tv_usec`，>=1000ms 时越界（POSIX 未定义；musl/macOS、glibc 2.32 直接      
    返回 EINVAL），`Sleep(1000)` 完全不睡；且忽略 `select` 的 EINTR，睡眠会被信号截断。      
    现在按截止时间重试并拆成 tv_sec+tv_usec。
  - `test_cs.cpp` `try_enter_cross_thread` / `test_sync.cpp` `mutex_cross_thread`：      
    原实现靠三个 `Sleep` 时间点给两个线程定序（只留 150ms 余量），机器繁忙或 Sleep      
    被截断时会随机失败（`tryAfterRelease` 读到 0）；已改为事件握手，无时序假设。
  - `test_multimon.cpp` `swinx_multimon.*`：多显示器 API 回归（EnumDisplayMonitors/GetMonitorInfo/      
    MonitorFrom* 的 Win32 语义；Windows 跑真实 Win32 作基准，headless 环境自动跳过）。      
    其中 `monitor_rect_pixel_convention` 为 macOS 专用：用 CoreGraphics 锁定      
    "显示器矩形 = 屏 frame × 该屏 scale、y 以主屏高度 H1 翻转"的像素坐标约定，      
    与窗口放置（ConvertNSRect）共用同一约定。
- **macOS 多显示器真机回归清单**（外接显示器，自动化覆盖不到的部分，升级显示器配置后手跑一遍）：
  1. 混合 scale（Retina 内屏 + 1080p 外接）与混合分辨率（两屏高度不同）各测一次；
  2. 外接屏上新建 SOUI 窗口：位置/尺寸应与请求一致（验证 ConvertNSRect 的 H1 翻转）；
  3. 窗口跨屏拖动：拖放后 SOUI 收到 WM_SIZE/WM_MOVE 且布局正确、鼠标 hittest 正常；
  4. 拔掉外接屏：窗口自动迁移到剩余屏，SOUI 收到 WM_MOVE/WM_SIZE；重新插回再测；
  5. 外接屏上全屏（fullscreen 按钮）：进入/退出动画单段、退出后窗口回原位原尺寸；
  6. `fun_test --gtest_filter=swinx_multimon.*` 在多屏配置下全过（含 pixel_convention 用例）。
- `swinx_beep.*`（MessageBeep）只断言"调用安全 + 返回值合法"，不断言"一定响"：是否可闻取决于    
  运行环境（声卡、有无 X 连接、X server 的 `xset b` 设置）。需要人工确认听感时执行    
  `fun_test --gtest_filter=swinx_beep.*`（会连续响 6 次 + 3 次）。移动端（Android/OHOS）    
  该 API 依赖宿主应用注册的 `g_platformAPI.audio.messageBeep` 回调：应用未注册时恒为 `FALSE`，    
  因此这两个平台上的期望值同样是"合法 BOOL"而非 `TRUE`。
- `swinx_wnd_rect.*`（`AdjustWindowRectEx`）断言的是 **swinx 的非客户区模型**，不是 Win32 的    
  `NONCLIENTMETRICS` 算法：swinx 只把**自绘的 `WS_BORDER` 一圈**算进窗口矩形（四周各    
  `SM_CXEDGE`/`SM_CYEDGE`），标题栏与调整边框由原生窗口管理器画在窗口矩形**之外**（Linux    
  `_MOTIF_WM_HINTS` 的 `MWM_DECOR_*`、macOS `NSWindowStyleMask`，cocoa 的 `GetWindowRect`    
  还显式做 `contentRectForFrameRect`），菜单栏由 SOUI 自己的菜单控件自绘，因此这三项对矩形的    
  贡献为 0（`GetSystemMetrics` 对相应度量返回 0，不再刷 "unknown index" 日志）。`WS_CAPTION`    
  的定义本身含 `WS_BORDER` 位，所以函数内先按**原始 style** 判定标题栏、再把这圈边框归一化    
  清掉——与 `WIN_CreateWindowEx` 创建标题栏窗口时的处理一致。这些断言在 Windows 宿主上不参与    
  （`#ifndef _WIN32`）：真机的标题栏与调整边框确实占窗口矩形，两边语义本就不同；跨平台那条    
  用真实窗口验证"`AdjustWindowRectEx` 与 `GetClientRect` 互逆"。
- `utilities_tls.*` 与 `test_kernel.cpp` 的 `swinx_tls` 是两件事：前者测 utilities 自己实现的    
  `helper/STls.h`（对标 SDL3 的 `SDL_TLS*`），后者测 Win32 API `TlsAlloc/TlsSetValue/
  TlsGetValue/TlsFree` 的 swinx 兼容实现。
- **跨线程用例不要用 `Sleep` 给两个线程定序**：`Sleep` 只保证"不早于"，机器繁忙时会过冲，    
  信号也会打断它（swinx 进程装着 SIGCHLD 处理器），只留几十~几百 ms 余量的断言必然随机失败。    
  统一做法：把"等待对方进入某状态"交给事件（`SetEvent`/`WaitForSingleObject`），    
  让 worker 阻塞在事件上而不是"睡够时间"。参考 `test_cs.cpp` 的 `try_enter_cross_thread`。
- **X11 事件一律经 `xcb_send_event32()` 发送**（见 `swinx/src/platform/linux/xcb_event.h`）：    
  libxcb 的 `xcb_send_event()` 无条件 `memcpy` 32 字节，而 `xcb_expose_event_t`(20B)、    
  `xcb_unmap_notify_event_t`(16B)、`xcb_selection_notify_event_t`(24B) 都不足 32 字节，    
  直接传结构体地址会让 libxcb 读过对象尾部、把未初始化栈字节发给 X server    
  （valgrind: `Syscall param writev(vector[...]) points to uninitialised byte(s)`；报错点    
  指向最近的 `xcb_flush`，容易被误判到无关的 cairo 绘制路径）。事件结构体自身还须 `= {}`，    
  因为 padding 与未填的 `data.data32[]` 同样会被 memcpy 出去。
- **用 valgrind 跑本工程请加 `--child-silent-after-fork=yes`**：`test_flock.cpp` 用 `fork()`    
  制造并发锁场景，子进程不 exec 且以 `_exit()` 结束，会继承父进程整片堆后直接退出；于是    
  fontconfig/cairo（`FcPatternDuplicate`/`FcFontMatch`）与 swinx 单例的线程对象在每个子进程里    
  都被报成 `definitely lost`（每子进程约 95 KB，3 个子进程数字雷同）。这是 fork 假阳性而非真泄漏    
  ——同一份代码在主进程里是 `definitely lost: 0`。
- **主进程泄漏基线（2026-10-26 实测，valgrind 3.18.1）**：`valgrind --leak-check=full --child-silent-after-fork=yes --suppressions=../swinx/valgrind-linux.supp ./fun_test --gtest_filter=-window.*` 只剩一份 heap summary，其中 `definitely lost: 0`、`indirectly lost: 0`、`possibly lost: 336`（glibc 线程栈缓存留下的 `_dl_allocate_tls` dtv 块——`CloseHandle` 已 `pthread_join`，是 glibc 复用缓存的假阳性，不是泄漏）、`ERROR SUMMARY: 1`。此前主进程另有两处真问题，均已修复：`~CursorMgr` 的 `DestroyIcon` 被自己的 `isStdCursor` 守卫短路，清表后遗留 7 个标准光标（280 B 直接 + 27,552 B 间接）；`SPanel::m_dwUpdateTime` 未在构造初始化，`ScrollUpdate()` 读到未初始化值。守卫语义由 `swinx_wke_cursor.standard_cursor_survives_unconditional_destroy`（标准光标销毁失败且句柄仍共享）与 `swinx_wke_cursor.caller_owned_icon_is_destroyable`（自己建的对象必须能释放，防止守卫退化成"一概不释放"）钉住；退出时的实际释放只能靠上面那条 valgrind 基线复现。看到主进程 nonzero `definitely lost` 时先按这两处对号，再判读新条目。
- swinx 已知语义偏差（测试已按实际行为断言）：互斥锁为管道字节实现，**不支持 Win32 的属主线程递归等待**。
- `test_lua.cpp`（script_lua 模块导出对象生命周期调试，套件 `soui_lua` + `soui_lua_app`，
  27 个用例）**零 lua 链接、无编译条件**：SComMgr2 加载 scriptmodule-lua（静态 COM
  直链 / DLL COM 运行时 LoadLibrary，代码零差异），对象全部由脚本经
  `IScriptModule::executeScriptBuffer` 创建；每个导出对象一个用例只做"创建+释放"，
  供人工在调试器里跟踪 C++ 生命周期（源文件头部有断点速查表）。成败探针 = 脚本
  末尾写 flag 文件（`executeScriptBuffer` 返回 void，纯模块接口下唯一可观测通道）。
  释放路径三类，用例注释逐对象标注：
  - **[GC]** 生命周期完全归属 lua 的纯包装对象（LuaValueAnimator/LuaAnimatorGroup
    经 `push_gcnew`/`constructor_lstate` → `__gc` 直接 `delete`，不经引用计数，
    见模块文档 §3.9）→ `o=nil; collectgarbage()` 触发；**绝不能**再调
    `:Release()`（双释放）。
  - **[Rel]** 非托管 push 的 IObjRef 系（CreateStringA/W、CreateXmlDoc、
    LoadAnimation、LoadValueAnimator、CreateMenu、CreateTimer、
    CreateAnimatorGroup、CreateEventSlot、三个 LuaAdapter 等）→ 必须显式
    `:Release()`，忘调即泄漏——人工排查主战场。
  - **[None]** 非托管 push 且无 Release 绑定（CreateAnimationListener 等 3 个纯接口
    监听器）→ 预期泄漏，待人工确认补救方案。
  本测试抓出的真实缺陷（均已修）：`CreateTranslatorMgr` 栈上 SComMgr2 析构时
  FreeLibrary(libtranslator.dll)，返回对象一用即 AV（改 leak-on-purpose 静态指针）；
  `SUiDefInfo::Init` 空/非法 pszUidef 越界（warning 后未 return）；以及脚本侧调带
  DEF_VAL 默认参的接口方法必须传满参数（`AddResProvider(rp)` 二参被 read 成空串
  触发上述 SUiDef 越界）——资源挂载改在夹具 C++ 侧传真 NULL。app 夹具
  （`soui_lua_app`）由 C++ 创建/释放 SApplication 并挂 FILE resprovider
  （demo uires，CMake §3.5 注入 `SOUI_FUN_TEST_DEMO_UIRES`）与 log4z。
- 运行方式：构建后在 `uires` 同级目录执行 `fun_test`，可用 `--gtest_filter=swinx_*` 只跑 swinx 用例。
