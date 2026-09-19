# fun_test

SOUI 的 gtest 单元测试工程（链接 `gtest`（含 gtest_main）+ `soui4` + `Scintilla`；非 Windows 平台额外链接 `swinx`）。

## 基本自动验证

从仓库根目录配置并构建 `fun_test` 后，执行：

```sh
cmake --build build --target fun_test --config Debug
ctest --test-dir build -L '^soui-headless$' --output-on-failure
```

CTest 在**配置期**扫描测试源码，把每个 `TEST` / `TEST_F` 注册成独立用例，并按**文件名约定**分层打标签（分组的实现与理由见 `CMakeLists.txt` 文件头注释）：

| 源文件 | 层级标签 | 注册用例数 |
|---|---|---|
| 其余 `test_*.cpp` | `soui-unit` | 323 |
| `test_integration_*.cpp` | `soui-integration` | 1 |
| `test_e2e_*.cpp` | `soui-e2e` | 1 |
| **门禁合计** | **`soui-headless`**（= 上面三行） | **325** |
| `test_gui_*.cpp` | `soui-gui`，需 `-DSOUI_ENABLE_GUI_SMOKE=ON` | 1 |
| `test_gdi.cpp`、`test_soui.cpp` | `soui-interactive`，需 `-DSOUI_ENABLE_INTERACTIVE_TESTS=ON` | 2 |

`soui-headless` 是门禁标签，`pr-core.yml` 与 `build.yml` 都是 `ctest -L '^soui-headless$'`；`soui-unit` / `soui-integration` / `soui-e2e` 用来单跑某一层。最后两层**不带** `soui-headless`，所以打开它们的开关也不会把"需要桌面"或"需要人工操作"的用例拖进门禁。

新增常规 `TEST`/`TEST_F` 用例只要文件按约定命名，重新构建就会进入相应标签，**不必改 CMakeLists**。三条限制：① 扫描发生在配置期，新增文件后需让 CMake 重跑一次（`CONFIGURE_DEPENDS` 一般能自动触发）；② 参数化用例（`TEST_P`）只能扫出模板、拆不出全部实例，须单独核验；③ 分组以文件为界，同一套件若跨文件可能分属不同层（`soui_component` 的用例就分别在 `test_module.cpp` 与 `test_integration_resprovider.cpp` 里），标签只是登记层级，不代表语义纯度。注册结果用 `ctest --test-dir build -N -L '^soui-headless$'` 核对，单组用例可用 `ctest --test-dir build -R '^soui_matrix\.' --output-on-failure` 复跑。

E2E 不打开交互窗口，暂不验证屏幕显示、鼠标输入或窗口容器中的事件派发。测试失败会输出用例日志并返回非零状态。

Linux/macOS 默认在构建 `fun_test` 后运行全套非交互测试；PR 快速构建可配置 `-DSOUI_FUN_TEST_POST_BUILD=OFF` 关掉该构建后动作，再显式运行上面的 CTest 门禁。两条路径覆盖面一致（都是"除 `window.*` 外的全部用例"），差别只在粒度：门禁逐用例各起一个进程、可 `-j` 并行、失败定位到用例；构建后动作是单进程一次跑完，快得多。全套也可手动运行 `fun_test '--gtest_filter=-window.*'`。

## GUI 烟测

在有桌面显示的环境配置 `-DSOUI_ENABLE_GUI_SMOKE=ON`，构建后运行 `ctest --test-dir build -L '^soui-gui$' --output-on-failure`。Linux 可用 `xvfb-run -a` 包裹 CTest。`window.gui_smoke_dispatches_button_click` 创建并显示真实 SOUI 宿主窗口，通过窗口过程发送鼠标消息并检查按钮事件；它不会长期等待人工操作。GUI 组默认不注册，门禁仍是 325 个无头用例；全套非交互 `fun_test` 的 `-window.*` 过滤器也排除 GUI 组。此烟测尚不比较渲染像素或验证真实设备鼠标。

## 交互式窗口用例

`test_gdi.cpp`（`window.gdi`）与 `test_soui.cpp`（`window.soui`）会开窗并**等待人工操作**、没有时间上界，因此默认不注册（`TIMEOUT 0` = 不设超时）。需要人工验证时显式打开：

```sh
cmake -S . -B build -DSOUI_ENABLE_INTERACTIVE_TESTS=ON
cmake --build build --target fun_test
ctest --test-dir build -L '^soui-interactive$' --output-on-failure
```

不要把它加进门禁：它既需要桌面，也需要人。

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
│                    # 命名子窗口按层级查找
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
    任何用例时返回 0）。所以某个平台上"325 个全过"里可能含若干空条目，不能当覆盖证据。
  - **连注释掉的 `TEST` 也会被注册**：注册是纯文本正则匹配（`GoogleTest.cmake` 的
    `(TYPED_TEST|TEST)_?[FP]?` 用的是**未锚定行首**的 `REGEX MATCH`），所以
    `//TEST(suite, name)` 同样产出一条空跑条目——`test_misc.cpp:51` 的
    `//TEST(swinx_misc, is_bad_read_ptr)` 正是如此（325 条里有 1 条来自它，源码实际只有
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
- swinx 已知语义偏差（测试已按实际行为断言）：互斥锁为管道字节实现，**不支持 Win32 的属主线程递归等待**。
- 运行方式：构建后在 `uires` 同级目录执行 `fun_test`，可用 `--gtest_filter=swinx_*` 只跑 swinx 用例。
