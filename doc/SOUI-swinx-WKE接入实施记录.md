# swinx 承载 WKE 浏览器引擎：接入与适配实施记录

> **文档定位**：本文件保存 SOUI4 评估报告在 2026-10-27 收敛到设计层时剥离下来的 **实施细节**（控件接入方式、契约用例清单、cairo 字体后端机制、世界变换故障定位、句柄类型形状、API 面门禁、逐项缺口）。评估报告 `doc/SOUI4项目评估报告-2026-10-14.md` §2.10 只保留设计层结论。
>
> **归属建议**：引擎本体的编译与逐 API 缺口记录的权威口径属于 `wkex` 仓库（`wkex/build-cross-platform.md`、`wkex/swinx-api-gap.md`）与 `swinx` 子模块文档；本文件是 **soui4 侧的接入记录**，可在把内容并入 wkex 后删除。迁移时请保留行号证据与用例文件名。
>
> **实测基线**：主仓库 HEAD `889f2d6c8`（2026-10-27「调整 wke 浏览器支持，支持 WKE 跨平台」）、`swinx` 子模块 `7d91784`（swinx 1.8 / 2026-10-26）。数字与行号为该检出上的实测。

## 一、本仓口径（不夸大）

- **WKE 不是 soui4 的子模块**。`.gitmodules` 只有 5 项（`swinx`、`third-part/{libcurl,libwebsockets,openssl-cmake}`、`soui-freertos`）。本仓受控的 WKE 相关资产只有 9 条路径：`third-part/wke/include/wke.h`（418 行 API 头）、`third-part/wke/wke.7z`（2,962,328 B，Windows 32 位预编包）、`third-part/wke/readme.txt`，`controls.extend/SWkeWebkit.{h,cpp}`（114 + 393 行），以及 5 个测试文件（见本文三）。
- **引擎本体在独立仓库 `wkex`（WebKit 分支）中编译**，其交叉编译产物（`libwke.so` / `libwke.dylib`）不入库，本仓 CI 也不构建它——仓内无任何 target 链接 wke 库。
- 因此本记录只覆盖**两件事**：①SOUI 侧浏览器控件的接入面已跨平台化；②swinx 面向 WKE 的 Win32 API 契约面已建立，并有双侧对照的无头用例。引擎自身在非 Windows 上的可运行性属于 `wkex` 仓库的构建记录，不在本记录可验证范围内（评估报告 §9.2 风险 12）。

## 二、SOUI 侧：控件脱离 Windows 条件编译

| 变更                                                            | 位置                                              | 含义                                                                                                                       |
| ------------------------------------------------------------- | ----------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------ |
| `#include "SWkeWebkit.h"` 与 `RegisterWindowClass<SWkeWebkit>()` 移出 `#ifdef _WIN32` | `controls.extend/SCtrlsRegister.cpp:44`、`:89`   | `SWkeWebkit` 在 Linux / macOS 也进入窗口类表，XML 可直接声明浏览器控件；`controls.extend/CMakeLists.txt:17` 的 GLOB 本就全平台编译该文件           |
| **运行期绑定**而非链接期依赖：`LoadLibrary(pszDll)` + 6 个 `GetProcAddress`（`wkeInit`/`wkeShutdown`/`wkeCreateWebView`/`wkeDestroyWebView`/`wkeStringW`/`wkeString`） | `controls.extend/SWkeWebkit.cpp:74-83`          | 引擎缺失时 `Init` 返回 `FALSE`、页面退化为占位提示（`demos/demo/uires/xml/page_webkit.xml:9`），SOUI 与 demo 照常构建运行——**缺引擎不影响主程序可交付性**  |
| 加载名由 `"wke.dll"` 改为 `"wke"`                                    | `demos/demo/demo.cpp:164`                        | 交给 swinx 的 `LoadLibraryA` 候选序列（原名 → `wke.so`/`wke.dylib` → `libwke.so`，`swinx/src/platform/posix/sysapi.cpp:1037-1099`），一份调用代码六端同一写法 |
| 卸载前先调 `wkeShutdown()` 再 `FreeLibrary`                          | `controls.extend/SWkeWebkit.cpp:57-70`（`SWkeLoader::~SWkeLoader`） | WKE 内部有常驻后台线程（JavaScriptCore 堆 block-free 线程），不先停线就 unmap 镜像会执行到已卸载代码段而崩溃。这是 Win32 语义之外的**生命周期契约**，源码注释已就地写明        |
| 仅 Windows x86 自动解包 `wke.7z` 到运行目录                               | `demos/demo/CMakeLists.txt:125-134`             | 其余平台的 wke 库需由 `wkex` 侧产出，本仓不带产物                                                                                              |

## 三、swinx 侧：面向 WKE 的契约用例（实测 71 条 / 2,131 行 / 12 套件）

| 文件                                             | 行   | 用例 | 覆盖的 Win32 面                                                                                                                                                                                                                                            |
| ---------------------------------------------- | --- | -- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `demos/fun_test/test_swinx_wke_font_unit.cpp`  | 496 | 21 | 按**字形**维度的字体 API：`GetGlyphIndices{W,A}`、`GetCharWidthI`、`GetCharWidth32{W,A}`、`GetGlyphOutline{W,A}`、`GetFontData`（两步：先取尺寸再取字节，覆盖两种 sfnt tag 字节序）、`GetFontUnicodeRanges`、`GetOutlineTextMetrics{W,A}`、`EnumFontFamiliesEx{W,A}`（含回调返回 FALSE 提前终止、A/W 家族数关系）                     |
| `demos/fun_test/test_swinx_wke_script_unit.cpp` | 559 | 17 | Uniscribe + EMF 文本回放：`ScriptItemize`/`ScriptShape`/`ScriptPlace`/`ScriptGetFontProperties`/`ScriptXtoCP`、`ScriptStringAnalyse`/`ScriptStringOut`/`ScriptStringFree`（`SSA_FALLBACK\|SSA_METAFILE`）、`CreateEnhMetaFile*`/`CloseEnhMetaFile`/`EnumEnhMetaFile`/`DeleteEnhMetaFile`、`SelectObject` 记录 `EMR_EXTCREATEFONTINDIRECTW` |
| `demos/fun_test/test_swinx_wke_registry_unit.cpp` | 565 | 15 | `PathCombine`/`PathAppend`/`PathRemoveFileSpec`/`PathRemoveExtension`/`PathRenameExtension`/`PathGetCharType`（A+W）、注册表往返 `Reg{Create,Open,QueryValueEx,EnumKeyEx,QueryInfoKey,Close}Key`、`SHGetValue{A,W}`、`GetFileVersionInfo*` + `VerQueryValue{A,W}`                                             |
| `demos/fun_test/test_swinx_wke_sysapi_unit.cpp` | 461 | 17 | `ExpandEnvironmentStringsW`、`GetFileAttributesExW`、`GetThreadTimes`、`InterlockedExchange`、`VirtualProtect`、`LoadLibraryExW`（同一镜像两次返回同一 `HMODULE`）、`RegisterWaitForSingleObject`（`WT_EXECUTEONLYONCE` 回调一次）、`CryptAcquireContextW`/`CryptGenRandom`/`CryptReleaseContext`、`GetAdaptersAddresses`（严格两遍 `ERROR_BUFFER_OVERFLOW` + `Next` 链遍历）、`EnumDisplaySettingsW`、标准光标共享规则 |
| `demos/fun_test/test_swinx_handle_type_unit.cpp` | 50  | 1  | `HANDLE`/`HGDIOBJ`/`void*` 三者同型的形状断言（见本文六）                                                                                                                                                                                                    |

两点口径值得单独记录：

1. **同一批用例在两侧都跑**：这些用例断言的是 Win32 的**共享契约**，Windows 上直接对系统 API 跑同一份断言——为此 `demos/fun_test/CMakeLists.txt` 的 Windows 分支显式补链 `usp10 shlwapi version crypt32 iphlpapi`（这批入口在 Windows 上归属系统模块，不链接则用例编不过）。swinx 侧的刻意偏差（`GetThreadTimes` 不记创建时间、`EnumDisplaySettingsW` 不保存模式表、注册表缺失键与失效句柄的报错口径）一律关在 `#ifndef _WIN32` 内，不与系统行为混成一条断言。
2. **用例即契约边界**：swinx "面向 WKE 的改动以 wke 的实际引用面为契约边界，**不为未被引用的 Win32 API 主动补实现**"（`swinx/AGENTS.md:106`）。这解释了为什么补齐是**定向**的（字体按字形、EMF 回放、注册表/路径），而不是把 SDK 抄一遍。

## 四、机制：cairo 后端的"win32 字体后端"路线

WKE 的字体代码按 Windows 语义书写，落到 cairo 后端需要三条精确配合——`swinx/src/gdi/cairo/fontapi.cpp:22-45` 的文件头注释直接点名 WebKit 的 `FontCacheWin` / `GlyphPageTreeNodeCairoWin` / `SimpleFontData*Win` / `FontPlatformDataWin`：

- **按字形工作**：`fontconfig` 把 (family, weight, slant) 解析成字体文件，`FreeType` 读 glyph 索引 / 推进量 / 字形度量，HDC 侧只读 `_SDC::hfont`。
- **`ETO_GLYPH_INDEX` 直通**：WKE 自带的 cairo（`wkex/3rd_src/cairo` 的 win32 字体后端）把每个字形经 `ExtTextOutW(..., ETO_GLYPH_INDEX, ...)`（`cairo-win32-font.c` 的 `_flush_glyphs()`）栅格化，传的是它用 `GetGlyphIndicesW` 取得的原生 TrueType ID；swinx 的 `ApplyFont()` 用同一个 LOGFONT 解析出同一个 FreeType face，其 cairo glyph ID 恰是原生 ID，索引因此直通 `cairo_show_glyphs()`（`swinx/src/gdi/cairo/gdi.cpp:4833-4842`）。
- **DC 可用性判定**：cairo 的 win32 后端把 `SetGraphicsMode` 失败视为"此 DC 不可用"（`_get_global_font_dc` 会删除缓存 DC），并在保存 surface 前用 `GetGraphicsMode` 读回模式；swinx 因此必须**真的存并取得**该值，同时对 `NULL` HDC 复刻 Windows 的 `0 + ERROR_INVALID_HANDLE`（WebKit 的 `syncContext()` 会对 ImageBuffer 调 `SetGraphicsMode(NULL, GM_ADVANCED)`）——见 `swinx/src/gdi/cairo/gdi.cpp:1236-1248`。
- **后端不对称是刻意的**：apple 后端不共用 fontconfig/FreeType 路径，`swinx/src/gdi/apple/fontapi.cpp` 用 CoreText 独立实现同名入口，使"取字形索引"与"实际绘制"落在同一个字体上；两份实现只在各自后端目录里编译——评估报告 §2.2 的"平台 × 渲染后端二维正交"在这里直接兑现。

## 五、机制：世界变换健壮性（一次真实故障的定位）

非 Windows 上 WKE 的 tooltip 文本测量返回 `0×0`，链条是：`GetDC(NULL)` 取共享桌面 DC → WKE 的 cairo win32 字体后端对零缩放字体算出 `1/0` → NaN 世界变换被 `SetWorldTransform` 写到**共享** DC 上 → cairo `cairo_set_matrix` 记下 **sticky** 的 `CAIRO_STATUS_INVALID_MATRIX`（`cairo_save/restore` 清不掉）→ 此后该 DC 上每次 `DrawText(DT_CALCRECT)` 均返回 `0×0`，tooltip 塌成只剩边距。修复口径是**在入口拒绝，而不是把脏状态留给 DC**：

- `is_invertible_matrix()`（`swinx/src/gdi/cairo/gdi.cpp:4120`）逐条检查六个成员 `isfinite` 且行列式非零；
- 在 `SetWorldTransform`（`:4211`，检查点 `:4217`）与 `ModifyWorldTransform`（`:4224`，检查点 `:4241`/`:4255`）处返回 `FALSE`；
- 回归用例：`demos/fun_test/test_text.cpp:469`（奇异矩阵被拒绝后，桌面 DC 上 `DrawTextA(DT_CALCRECT)` 两维仍 >0）与 `:501`（NaN 矩阵不断言返回值，只断言可用 `MWT_IDENTITY` 恢复、后续测量与合法 2× 缩放变换均正常）。

## 六、类型形状：让"按 Windows 头写的代码"少一处平台分支

`swinx` 的 `HANDLE` 与 `HGDIOBJ` 现与 Windows SDK **完全同型**——都是 `void*`（`swinx/include/ctypes.h:255`、`:304`；`INVALID_HANDLE_VALUE` 同口径 `:520`），具体 GDI 句柄保持 `DECLARE_HANDLE` 式独立结构体指针；内部非空句柄仍指向 `_Handle` 包装，需要读写包装成员处经 `AsHandle()` / `ToGdiHandle()` 取回真实类型（`swinx/src/handle.h:36`、`:44`）。

直接收益在绑定层：`components/ScriptModule-LUA/lua_tinker/` 原先为非 Windows 的 `HGDIOBJ` 写的 `read<>` / `push<>` 两条模板特化已删除（同型之后它们变成 `void*` 的**重复特化**，反而编译失败），`demos/demo`（含 `scriptmodule-lua`）全量构建通过即为该结论的实证；形状本身由 `test_swinx_handle_type_unit.cpp` 钉住。

## 七、API 面与门禁（2026-10-27 实测）

- `swinx/include/` 公共头 **57 个**（评估报告 §2.1 的"44 个"是 2026-10-14 基线口径）；`swinx/src` + `swinx/include` 合计 **191,708 行**（不含 `thirdparty/`）。
- `swinx/doc/tools/api_scan.json`：**声明 1,272 / 有定义 1,142**（完整 1,094 · 部分 8 · 空实现或语义桩 40），仅声明未定义 130 条（含扫描器收录的宏）；`api_baseline.json` 锁 **1,121** 条能力下限；`.github/workflows/swinx-api-gate.yml` 持续门禁（评估报告 §1.4），面向 WKE 补的入口因此**不得回退**。
- 版本记录：`swinx/src/swinx_version.h:39` — **1.8 / 2026-10-26「初步完成 freeRTOS 适配，补充完成 WKE 浏览器适配需要的 API」**；主仓库侧的接入落地在提交 `889f2d6c8`（2026-10-27「调整 wke 浏览器支持，支持 WKE 跨平台」）。

## 八、边界与未覆盖面（客观记录）

| 缺口                                   | 现状                                                                                                                                                     | 证据                                                                    |
| ------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------ | --------------------------------------------------------------------- |
| 内存字体加载仅 apple 后端                    | `AddFontMemResourceEx` / `RemoveFontMemResourceEx` **只有 apple 实现**，cairo 后端无同名实现 → Linux / Android / OHOS 上引用即**链接期失败**（显式失败，不静默）                       | `swinx/src/gdi/apple/gdi.cpp:5982`、`:6020`；`swinx/doc/swinx-API实现清单.md:268` |
| `GetGlyphOutline` 仅实现 `GGO_METRICS` | 两个后端一致；`GGO_BITMAP` / `GGO_NATIVE` / `GGO_BEZIER` / `GGO_GRAY*` 返回 `GDI_ERROR`（源码注释："只实现 GGO_METRICS（wkex 只用它）"）                                      | `swinx/src/gdi/cairo/fontapi.cpp:356-357`、`swinx/src/gdi/apple/fontapi.cpp:336-337` |
| USP10 不做真实整形                        | 每字符一 glyph、identity clusters、`GOOFFSET` 为零——复杂文种（连字/整形）不能依赖它；`GetCharacterPlacement` 有实现但无对应用例                                                  | `swinx/src/platform/posix/usp10.cpp`；`wkex/swinx-api-gap.md` §USP10      |
| 少量等待 / 定时器入口未补                      | `CreateWaitableTimerExW` **不存在**（仅 `CreateWaitableTimer{A,W}`）；`UnregisterWaitEx` 有实现但无对应用例                                                             | `swinx/src/platform/posix/sysobjs.cpp:871`、`:897`、`:2177`             |
| 剪贴板 / IME 无无头用例                     | `demos/fun_test/` 中 `OpenClipboard` / `GetClipboardData` / `IMM_*` 命中数为 **0**，而这两块 WKE 实际会用；当前只能靠 GUI 人工验证                                          | 全文 grep（排除 build 目录）                                                  |
| 引擎侧与本仓的 swinx 检出不同源                 | `wkex` 自带 `swinx` 子模块，与本仓子模块指针（`7d91784`）**不是同一提交**，两边历史已分叉——同一份 API 契约存在**两份工作副本**，需人为同步                                           | `git ls-tree HEAD swinx`；`wkex/.gitmodules`                           |
| FreeRTOS 端口不带字体 / GDI 面             | `swinx/freeRTOS.cmake:74-80` 只 glob `src/`、`src/cmnctl32/`、`src/platform/freertos/`，**不编译 `src/gdi/**`**：面向 WKE 的字形与字体入口不进入该归档                     | `swinx/freeRTOS.cmake`                                                |

**与评估报告的关系**：报告 §2.7 证明"只要组件存在 Windows 实现，swinx 就足以让它跨平台"；WKE 把这条论证推到最重的一端，且接入方式是**运行期绑定 + 契约用例双侧对照**——本仓不为尚未存在的引擎产物承担构建风险，也不为未被引用的 API 扩张实现面。对使用者的直接含义是：SOUI 可以在**不引入 Chromium / Node 运行时**的前提下嵌入 Web 渲染能力（对照报告 §8 的 Electron 一列）。

---

### 迁移检查表（并入 wkex 时逐项确认）

1. 本文四 / 本文五的机制描述应归入 `wkex/swinx-api-gap.md` 的"后端机制"章节，因为其中的 `ETO_GLYPH_INDEX` 直通与 sticky 错误码结论只有对照 WebKit 源码才成立。
2. 本文三的用例表应随用例文件一起说明"Windows 侧同份断言"的链接依赖（`usp10 shlwapi version crypt32 iphlpapi`）。
3. 本文八的缺口表需要与 `api_scan.json` / `api_baseline.json` 的实际数字对齐后再搬，避免带过来一份过期快照。

