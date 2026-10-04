# ScriptModule-LUA 升级记录与踩坑备忘

> 记录时间：2026-10>   
> 范围：Lua 5.4 + lua_tinker（SOUI 定制版）脚本模块升级，对齐 soui4js（QuickJS）导出面。>   
> 目的：沉淀本次升级完成的功能与踩过的坑，**防止下次升级/移植再犯**。>   
> 相关记忆：`.workbuddy/memory/2026-10-01.md`、`2026-10-02.md`；对外文档：soui-docs `08-advanced-topics/script/index.md`。

---

## 一、升级概述

- 基线：`components/ScriptModule-LUA`（lua 5.4.4 内核 + lua_tinker 绑定层）。
- 目标：让 lua 脚本具备与 soui4js（QuickJS 导出）对等的核心能力，可在 demo 中用纯 lua 驱动完整游戏（消消乐、跑马机）。
- 涉及三处独立缺陷域（都真实存在，互不掩盖）：
  1. lua_tinker 绑定层 bug（事件字段错位、布尔参数、调用约定等）；
  2. `components/commgr2.h` 静态 COM 守卫 bug（lua 完全不执行层）；
  3. VS2010 x64 `/Og` 误编译 lua 内核（`lgc.c clearkey`，崩溃层）。

---

## 二、完成的功能

### 2.1 lua_tinker 层增强（`lua_tinker/`）

| # | 修复/增强                               | 说明                                                                                                                                                                                                                                                     |
| - | ----------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| 1 | 事件字段注册修正                            | `DEF_EVT` 生成 `EventXxx : SEvtArgs, StEventXxx` 多继承，SEvtArgs 有 5 成员 ⇒ StEventXxx 子对象偏移非零；裸指针 cast 读字段全部错位。改为注册**完整 EventXxx 类**走 `sobj_cast`（63 事件、118 处 class_mem），并提供 `toEventXxx`/`toStEventXxx` 双别名                                                 |
| 2 | `mem_var_derived<T,BASE,V>`         | lua_tinker `class_mem` 用 `read<T*>` 裸重解释，派生类注册基类成员指针错位；新增派生类成员变量绑定（`V BASE::*` 让编译器做偏移调整），向后兼容                                                                                                                                                         |
| 3 | `wchar_t`/`wchar_t*` 绑定             | 原本宽字符串返回是垃圾 userdata；补自包含 UTF-8↔UTF-16 编解码（read 侧用 `lua_newuserdata` 栈锚定缓冲）。`CreateChildrenFromXml(LPCWSTR)` 因此可直接传 lua string                                                                                                                         |
| 4 | **布尔参数 read 修复（最隐蔽）**               | `read<int/BOOL/long/float/double/...>` 全部基于 `lua_tonumber`，而 Lua C API 对布尔值 `lua_tonumber` 恒返 0.0 ⇒ **lua 传 `true` 到 C++ 的 BOOL/int 参数一律变 0**（`SetVisible(true)` 实际是隐藏！）。全部数值 read 特化补 `lua_isboolean` 分支；`long/long long` 原本布尔会走 userdata 解引用（UB）一并堵上 |
| 5 | `call<带参>` 错误路径修复                   | pcall 失败后 `lua_pop(1)` 弹 errmsg 再 `lua_remove(L,-2)`，偷掉调用方外层栈一个槽，随后 `pop<RVal>` 把错误对象当返回值读 ⇒ 任何 lua 处理器报错都会破坏宿主栈（tbc abort 只是下游症状）。5 个重载统一改为无条件 `lua_pcall` + `lua_remove(L, errfunc)`                                                                 |
| 6 | `class_set_cfun`（toobj.h）           | 带 `DEF_VAL` 默认参数的接口方法无法直绑，用 cfunction 包装统一支持                                                                                                                                                                                                           |
| 7 | `push_gcnew` / `constructor_lstate` | 生命周期完全归属 lua 的对象入栈助手：`push_gcnew<T>(L, args...)`（val2user 语义，`__gc` 直接 delete）；`constructor_lstate<T[, A1..A5]>` 支持 ctor 首参 `lua_State*` 的类走 `class_con` 直接构造（见 §3.9/§3.10）                                                                            |
| 8 | `lua_tinker` 无 `__stdcall` 死角的正确用法  | x86 下 UAPI（`__stdcall`）成员走 `invokeU` 重载链（lua_tinker.h ~1387 起 x86-only 块），注册 STDMETHOD 接口方法时必须显式写 `(UAPI RET (T::*)(args))`，见 §3.1                                                                                                                     |

### 2.2 导出补齐（`src/exports/`，核心优先范围）

- **动画体系**：IPropertyValuesHolder / IPropertyAnimator（table 读写 raw cfunction）、IAnimatorSet / IAnimatorGroup、工厂 `CreatePropertyAnimator` / `CreatePropertyValuesHolder` / `CreateAnimatorSet` / `CreateAnimatorGroup` / `LoadValueAnimator`。
- **LuaValueAnimator / LuaAnimatorGroup**（`exp_LuaValueAnimator.h`，对齐 js 侧 JsValueAnimator）：LoadAnimator / CopyFrom / SetRangeRect / Start / SetCtx / SetOnUpdate / SetOnEnd 等；回调按「全局函数名 + ctxId」路由回 lua。
- **XML 具体类**：SXmlDoc / SXmlNode / SXmlAttr（LoadStringU8/LoadFileA 可选参包装；IXmlNode 增删 DOM 方法 + Text/SetText；IXmlAttr.As* 走 class_set_cfun）。
- **IMatrix / SMatrix**（`exp_SMatrix.h`）：IWindow::SetMatrix/GetMatrix 原直绑因 IMatrix 未注册完全不可用，现加 lua 友好包装（收 SMatrix userdata 或 9 float 表 / 返回值）。
- **IBitmapS**（`exp_IBitmapS.h`）：Width/Height/Size/LoadFromFile/Clone（out 参数→返回值）。
- **事件类**：EventXxx 别名 + SwndCaptureChanged / SwndVisibleChanged / MenuCmd。
- **ISkinObj / ITranslator+ITranslatorMgr / ILogMgr**：tr/config/getFontInfo 等。
- **IApplication**：InstallTranslator / LoadTranslator(U8) / LoadImage(U8)。
- **IHostWnd**：InitFromXml / EnableDragDrop / EnablePrivateUiDef / ShowHostWnd / GetMsgLoop。
- **IWindow**：CreateChildrenFromResId / SetLayer；**IComboBase / IComboView / IListBox / IOsrPanel** 补方法；CRect::MoveToX/Y/XY；ISouiFactory::CreateAnimatorGroup。
- **IStackView**（`exp_ICtrl.h`）：SelectPage / GetSelPage / GetPage / SetAniStyle / SetAniDir + `DEF_QICTRL` 注册全局 `QiIStackView`（消消乐格子 7 态切换依赖）。
- 确认不移植：JsHostWnd/JsHostDialog 专属机制、RequireEven&#x74;*、setOutputFileBuilder、DrawBy*、IHttpClient、INcPainter、SDropTarget 等冷门项。

### 2.3 demo 实证（`demos/demo`）

- **消消乐（lua 版，移植 soxxl）**：8×8 棋盘、模板格子（`t:g.xxl_ele` stack + SelectPage 7 态 SVG 棋子）、LuaValueAnimator 交换/消除/下沉动画、级联消除、LED 计数器、提示/重开、消除爆发特效（cnchess ShowGameFx 模式）。端到端自测 PASS，长跑稳定。
- **跑马机**：卡片式重排 + GIF 马匹 + 金币 SVG 下注按钮，lua 驱动。
- 两游戏全部脚本在 `uires/lua/test.lua`，Debug 构建 RESTYPE_FILE 运行时直读目录，改 lua 无需重编。

---

## 三、踩过的坑（防再犯重点）

### 3.1 🚨 x86 调用约定：注册 STDMETHOD 接口方法绝不能剥 `__stdcall`（本次最重大根因）

- **现象**：vs2010_x86 Debug 全量重编后启动崩，栈为 `IMatrix::vcall{16}+0x4 ← lua_tinker::mem_functor<int,IStackView,int,int,...>::invoke:832 ← ... ← SHostWnd::InitFromXml(EventInit)`。
- **根因**：`exp_ICtrl.h` / `exp_IWindow.h` / `exp_SXml.h` 里 5 处 C 风格强转（SelectPage / GetPage / SetMatrix / AsInt~AsBool）把 STDMETHOD 成员指针的 **`__stdcall`（UAPI 宏）剥掉了** ⇒ `push_functor` 重载决议永远选普通 thiscall 版 `invoke`（this 放 ECX），而实际值是 **stdcall vcall thunk**（`mov eax,[esp+4]; mov eax,[eax]; jmp [eax+XX]`，this 从栈取）⇒ thunk 把第一个实参（如 iView=1）当对象指针 ⇒ AV 恰在 thunk+0x4。
- **误名字陷阱**：`IMatrix::vcall{16}` 是 linker 把相同字节码 thunk 折叠后的 PDB 命名，**slot 数与 IStackView 无关，勿被符号名误导**。
- **为什么 x64 从不出事**：x64 只有一种调用约定，「lua_tinker 的 32/64 位有很大区别」这一判断完全正确。
- **修法**（已落盘，勿回退）：

```cpp
// exp_ICtrl.h —— 强转必须保留 UAPI
lua_tinker::class_def<IStackView>(L, "SelectPage",
    (UAPI BOOL (IStackView::*)(int, BOOL)) & IStackView::SelectPage);
```

- **规则**：注册 `STDMETHOD` 接口方法（sxml-i.h、interface 头里的 I* 接口）一律显式补 `UAPI`；SWindow 等普通类方法（`__thiscall`）不受影响。**注册前先看接口声明里的 STDMETHOD/SOAOCMT 宏**。

### 3.2 🚨 commgr2.h 静态 COM 守卫 bug（lua 完全不执行层，勿再还原）

- `SComMgr2::CreateScrpit_Lua` 的守卫曾写成 `#if (SCOM_MASK & scom_mask_script_lua) && defined(DLL_SOUI_COM)` —— `LIB_SOUI_COM` 与 `DLL_SOUI_COM` 在 config.h 互斥 ⇒ **静态 COM 构建（vs2010_x86 / vs2008_x86，COM_LIB=ON）恒 FALSE**，`SAppCfg::Init` 拿不到 script factory，`SHostWnd::InitFromXml` 里 CreateScriptModule 失败，**且静默无任何日志**。同文件其余 Create* 方法均无 DLL 守卫，属复制粘贴错误。
- 已修复（删守卫 + 注释说明）。**修 commgr2 后，demo.cpp 所在 TU 必须同步重编**（`EnableScript` 门控链在 demo.cpp / SAppCfg.h：`EnableScript` 被 `LIB_CORE && LIB_SOUI_COM` 宏门控，编译命令缺 `/D` 行时脚本模块同样静默不创建）。
- **症状判别**：lua 一行都不执行、连脚本模块 ctor 都不进 ⇒ 查 commgr2/EnableScript 门控；lua 执行后才崩 ⇒ 查 lua_tinker 绑定层。两层 bug 独立共存时会互相掩盖。

### 3.3 🚨 VS2010 x64 `/Og` 误编译 lua 内核（lgc.c clearkey）

- VS2010 x64 SP1 `/Og`（含于 /O2）把 `lgc.c` 的 `clearkey` 中 `if(keyiscollectable(n)) setdeadkey(n)` 换成 cmov + **无条件写回** ⇒ GC 步进遍历 `ltable.c` 的 `dummynode_`（static const，落 .rdata 只读段）时写回也 AV ⇒ `luaL_openlibs` 内崩，栈恒为 clearkey+0x10。VS2022 / VS2008 x86 均无此变换。
- **修复**：`third-part/lua-54/src/lgc.c` clearkey 改 `*(volatile lu_byte*)&(n)->u.key_tt = LUA_TDEADKEY;` 强制条件写。**勿还原为 setdeadkey 宏**。

### 3.4 lua_tinker 绑定层使用坑（lua 侧写脚本时的天坑清单）

| 坑                                | 后果                                                                                                         | 正确做法                                                |
| -------------------------------- | ---------------------------------------------------------------------------------------------------------- | --------------------------------------------------- |
| lua 布尔传 BOOL/int 参数              | 升级前恒变 0（已修 read 层）；但**自己写包装 cfunction 时仍需注意**                                                              | 包装层记得 `lua_isboolean` 分支                            |
| lua 里比较 C++ 回调推回的对象身份            | `v == group` 恒 false：C++ 侧 `lua_tinker::call(..., this, ...)` 把指针重新 push 成新 userdata，`==` 是 userdata 裸身份比较 | 回调对象用创建时的 **ctxId 键控**管理，勿用身份比较                     |
| `IWindow::GetWindowRect(LPRECT)` | 要传 RECT 参数；无参调用报 "no class at first argument"                                                              | 无参返回 CRect 的版本注册名是 **`GetWindowRect2`**（SWindow 上）  |
| `lua_tinker` 指针推入不拥有对象           | ptr2user 无 `__gc`，普通 push 的对象永不析构（泄漏但无 UAF），知悉即可                                                           | 长生命周期包装对象由业务层 ctxId 表管理                             |
| lua 处理器报错后不能信其返回值                | call 错误路径已修为无条件 pcall，但业务上仍应假设失败即无返回值                                                                      | 关键流程在 C++ 侧校验                                       |
| 未注册的 IWindow 方法直接调               | 如 `GetParent` 未绑定 ⇒ "can't find 'GetParent' class variable"                                                | **用之前必须 grep `exports/exp_*.h` 确认绑定面**，绑定面窄于 C++ 接口 |
| `Root()` 语义（pugi）                | `Root()` 是文档节点                                                                                             | 文档元素要 `Root():Child('root', false)`                 |
| BOOL 返回值                         | 是数字                                                                                                        | `==0` 判空，勿用 `not`                                   |
| 类表即全局名                           | lua_tinker 类表 = Lua 全局名                                                                                    | 可 `type(IXmlAttr.AsInt)=='function'` 断言绑定存在         |

### 3.5 SOUI 框架机制坑（lua 游戏实证踩出，写脚本前先看）

1. **模板引用 `t:NAME` 剥前缀后原样查模板池**，池 key = template.xml 原始节点名（约定 `g.` 前缀）⇒ 引用必须写 `t:g.xxx`；查不到 Debug 断言 `Swnd.cpp SASSERT(!strXmlTemp.IsEmpty())`。移植 js 项目时最易写错。
2. **SAnimatorGroup 不启动子动画**（SValueAnimator.cpp：AddAnimator 只 addListener 聚合回调）⇒ 每个 IValueAnimator 必须**单独 `start(container)`**，漏 Start 则动画组 end 永不触发、状态机卡死。
3. **模板实例 stack 初始停在默认页** ⇒ 随机布局后必须逐格 SelectPage。
4. **动画浮层副本 id 必须全局唯一**：级联动画下同一格有多个存活副本，复用棋盘格 id 时 FindChildByID 撞回旧副本 ⇒ 旧副本 Destroy 后新动画仍 tick ⇒ Move 打已释放窗口 ⇒ 段错误（js 靠 GC 掩盖，lua 必炸）。**销毁窗口前先清动画回调上下文表**，存活动画回调全部变 no-op。
5. **SetVisible(bVisible, bUpdate)** 第二参数是 invalidate 开关：`SetVisible(true, false)` 不失效 ⇒ 恢复可见后仍是空白格。终态点要传 true 或补 `Invalidate()`。不可见态下 `SelectPage` 直切不产生动画且 invalidate 被吞。
6. **全树共享宿主坐标系**（Swnd.cpp DispatchPaint 无逐级平移）：任何窗口的 `GetWindowRect2` 结果直接互用，**禁做父相对/累加换算**；动态建窗定位必须用 `Move(CRect)`（pos 属性要等 relayout 才生效，特效窗口时有时无就是 pos 依赖 relayout）。
7. `SStringT` 是单线程类型（COW + 非原子引用计数），跨线程传一律 `std::string/std::wstring`；lambda 禁按值捕获 SStringT。
8. GUI 崩溃诊断：log4z 异步缓冲会丢尾部日志 ⇒ 用 `SetUnhandledExceptionFilter` + dbghelp(StackWalk64+SymFromAddr) 写崩溃栈文件；lua 错误默认被吞 ⇒ 用 `set_print_callback` 接日志（这是本次取证的关键手段）。
9. 事件绑定：XML 里 `on_xxx` 属性走 `DefAttributeProc → setEventScriptHandler`，事件名 = 属性名；dlg_main.xml 的 `<script>` 节点有 src 时 **CDATA 不执行**（if/else 互斥）⇒ lua 函数放 `uires/lua/test.lua`。

### 3.6 工程与构建坑（复现/验证时参考）

- **手工 cl+lib+link 管线**（MSBuild 被沙箱拦时的替代）：
  - cl.rsp 里 `/I` 不认分号引号串（须一行一目录）；`/D` 内嵌引号吞后续参数（如 `/D CMAKE_INTDIR=\"Debug\"`，别抄）。
  - 复用 cmake_pch.pch 需 `/FI + /Yu + /Fp` 且 `/Fd` 指回原 vc100.pdb（否则 C2859）。
  - `VC\bin\mspdb100.dll` 可能被 x64 版覆盖 ⇒ 0xC000007B；PATH 把 `Common7\IDE` 放最前。
  - **obj 清单必须来自 `link.command.1.tlog` 的 ^ 行**——demo.dir 里混有历史孤儿 obj，按 *.obj 通配必 LNK2001。
- **ASLR**：每次运行地址不同，崩溃日志地址必须与**同一次链接**的 map 配对解析，跨 run 对比地址差会得出错误结论（实际误判过"虚表在 exe 外"）。
- VS2010 不支持 lambda→函数指针转换（探针里用静态函数）。
- **探针卫生**：条件编译宏门控（如 `X86_PROBE`），IDE 构建零影响；验证完**全部还原**（demo.cpp 的 VEH、lua_tinker.h/exp_ICtrl.h 的 QI/调用日志均已还原），只留正式修复。
- 编辑 CRLF 源文件（如 exp_SXml.h）后必须字节级复核 BOM/CRLF，编辑工具可能把 CRLF 写坏成 LF。
- 诊断定位优先级：无头环境 printf 结果在 segfault 时全丢 ⇒ `setvbuf(stdout, NULL, _IONBF, 0)` + 结果文件残留判断；GUI 子系统进程 stderr 不可见 ⇒ 日志文件才是可靠观察点。

### 3.7 🚨 工厂返回值泄漏：脚本必须显式 Release（VLD/valgrind 实测驱动）

**规则**：lua_tinker `push<T*>` 一律是**非托管指针**（ptr2user，无 `__gc`，GC 永不  
回收）。`class_def` 直绑的工厂方法（`LoadAnimation`、`CreateTimer`、  
`CreateSouiFactory` 等）把初始引用=1 的 IObjRef 对象交给脚本——**脚本必须显式  
`:Release()`，漏调即泄漏**，且泄漏的是整条成员链（对象本体 + 字符串成员 +  
容器缓冲 + 插值器）。

**典型事故**：消消乐把 `LoadAnimation` 结果缓存在 `xxl.fx_ani`/`xxl.ani_sel` 但从不  
Release → VLD 报十几个对象的整链泄漏。正确模式 = **master 缓存 + clone 挂载**：

- master 懒加载入缓存（脚本持 1 引），进程退出钩子（`xxl_exit`）逐项 Release；
- 挂载走 `clone → SetAnimation(ani) → ani:Release()`（clone 新对象 1 引 → 窗口    
  AddRef→2 → 脚本 Release→1 → 窗口 stop 时释放→0）；
- 🚨 **缓存 master 绝不能直接 SetAnimation 给窗口**：`SWindow::m_animation` 是    
  `SAutoRefPtr`，窗口 stop 时会 Release 它，缓存里就成了悬空指针。


**监听器**：IAnimatorListener 等是**非持有的裸指针**，框架不 AddRef、不备份迭代；  
回调中删除监听器对象必崩，属脚本契约，框架层不兜底（备份迭代方案试过，救不了）。

**回调窗口契约**：动画运行期 lua 侧经 `ani_ctx[ctxId]` 持有 userdata；脚本若在  
动画回调里主动丢弃全部引用并触发 GC，属契约违规（提前回收/监听器悬挂，框架不兜底）。

**教训**：

- 🚨 判据看继承链（TObjRefImpl/SObjectImpl = IObjRef 系），不是"有没有叫    
  Release 的方法"——普通对象自带无关 `Release()` 时绑定能编译但语义错。
- 🚨 脚本侧对每个 [Rel] 对象建立"谁创建谁 Release"的收支账：创建/缓存/退出    
  三处对齐，用完即 Release、持有期绝不提前 Release。
- 🚨 无头/宿主环境验泄漏：MinGW release 靠残留字节"侥幸不崩"会掩盖 UAF，    
  **MSVC Debug（0xDD 填充）+ VLD/valgrind 才是有效探测器**。

### 3.8 🚨 导出对象生命周期调试测试（fun_test/test_lua.cpp）抓出的坑（2026-10-03 晚）

`demos/fun_test/test_lua.cpp`（27 用例，SComMgr2 加载模块、零 lua 链接、纯  
IScriptModule 接口驱动）落地过程中实锤的三个缺陷与两个工程坑：

1. 🚨 **`CreateTranslatorMgr`（exp_global.h）栈上 SComMgr2 → FreeLibrary 崩溃**：     
   原实现 `SComMgr2 comMgr; comMgr.CreateTranslator(...); return pRet;` —— 函数返回时     
   `~SComLoader` 会 `FreeLibrary(libtranslator.dll)`，返回的 ITranslatorMgr 对象属于     
   该模块，调用方任何虚调用（`o:Release()`）都跳进已卸载代码段 → AV。与"持     
   ITaskLoop 的 SComMgr2 必须 leak-on-purpose"同一规则：改为**函数局部静态指针**     
   `static SComMgr2 *s_comMgr = new SComMgr2();`，随进程存活绝不析构。     
   ⚠️ 静态 COM 构建（LIB_SOUI_COM）下走 `TRANSLATOR::SCreateInstance` 直链无此问题，     
   所以这个 bug 只在 DLL COM 下暴露——新导出工厂凡经 SComMgr2 加载 DLL 的都要按此模式审。
2. 🚨 **SOUI `SUiDefInfo::Init`（res.mgr/SUiDef.cpp）空/非法 pszUidef 越界**：     
   `ParseResID` 失败只打 warning 不 return，继续用空 `SStringTList` 取     
   `strUiDef[0]/[1]` → `SArray::operator[]` 的 SASSERT abort（release 下未定义行为）。     
   已修：解析失败直接 `return bRet;`。
3. 🚨 **lua 侧调带 `DEF_VAL` 默认参的接口方法必须传满参数**：`class_def` 直接绑接口     
   方法（非 class_set_cfun 包装）不做默认参填充——`AddResProvider(rp)` 只传 1 参，     
   第二参被 `read<wchar_t const*>` 读成**空串 L""（不是 NULL）**，恰好绕过     
   `if (pszUidef)` 判空走进第 2 条的越界。经验：脚本里调用二参以上接口方法一律显式     
   传参；宿主基建（如资源挂载）优先 C++ 侧做，可传真 NULL。
4. **成败探针**：`executeScriptBuffer` 返回 void、`executeScriptFile` 恒 TRUE、     
   `executeScriptedEventHandler` 会在 handler 返回真值时解引用 pEvt（传 NULL evt     
   的 handler 不能返回真值）——纯 IScriptModule 接口下唯一可靠的成败观测 =     
   **脚本末尾写 flag 文件**，C++ 侧检查文件是否出现（脚本中途 error 则 dobuffer     
   弹栈、flag 不会出现）。
5. **lua 错误输出进 SOUI 日志而非 stdout**：SScriptModule_Lua 把 lua_tinker 的     
   print_error 重定向到 SLog —— app 未 SetLogManager 时脚本错误**静默丢弃**，只能     
   靠 flag 判成败。调试期给 app 装上 LogMgr（SComMgr2::CreateLog4z）才能看到错误文本。

### 3.9 对象生命周期模型（现行）：三条路径与判据

LuaValueAnimator / LuaAnimatorGroup 这类"属性在 lua 栈上的对象"，生命周期完全  
归属 lua 一个 userdata：`LuaValueAnimator` 是**纯 C++ 包装类**（无 IObjRef 基类，  
IAnimatorListener/IAnimatorUpdateListener 均非 IObjRef 系、addListener 只存裸  
指针）；`LuaAnimatorGroup` 保留 `SAnimatorGroup` 基类，但其初始引用永不  
Release，GC 直接 delete、不经 OnFinalRelease。

**导出写法**：常规走 `class_con<T>(L, lua_tinker::constructor_lstate<T>)`（§3.10）；  
需要运行时逻辑的工厂用 `push_gcnew<T>(L, args...)`（val2user 语义，`__gc` 直接  
delete，不经引用计数）。

**判据（三条路径）**：

| 对象                                               | 入栈方式                                | 释放                                  |
| ------------------------------------------------ | ----------------------------------- | ----------------------------------- |
| 单例/宿主/窗口树对象（GetApp()、事件参数等）                      | 普通 push（ptr2user）                   | C++ 侧管理，**脚本绝不可 Release**           |
| 工厂 new 的 IObjRef 产品（LoadAnimation、CreateTimer 等） | `class_def` 直绑                      | **脚本必须显式 `:Release()`**             |
| 生命周期只归属 lua 的纯包装对象（LuaValueAnimator 等）           | `constructor_lstate` / `push_gcnew` | GC 直接 delete，**脚本绝不可 Release**（双释放） |

**红线**：同一指针绝不能既托管入栈、又以普通 `push` 方式二次入栈（两条路径的  
`__gc` 语义不同，混用必然双释放或泄漏）；动画运行期 lua 必须经 `ani_ctx[ctxId]`  
持有 userdata 的脚本契约不变（提前回收/监听器悬挂，框架不兜底）。

### 3.10 构造器重载 constructor_lstate：ctor 首参 lua_State* 的类走 class_con

LuaValueAnimator/LuaAnimatorGroup 的导出与 CRect 等可构造类风格一致：  
`lua_tinker.h` 提供 `constructor_lstate<T[, A1..A5]>` 重载族（与 `constructor`  
并列，同 6 档参数）：

- **语义**：C++ ctor 首参 `lua_State*` 由 lua_tinker 自动注入当前 L，其余参数仍    
  按 `__call` 约定从栈索引 2 起读取（索引 1 是 `__call` 传入的类表本身）。对象    
  同为 `val2user`（GC 直接 delete），与 `push_gcnew` 同语义。
- **导出写法归一**：`class_con<T>(L, lua_tinker::constructor_lstate<T>)`；带参    
  构造 `constructor_lstate<T, int>`。缺参安全：`read<int>` 对 nil 返回 0    
  （`LuaAnimatorGroup()` 不传参 → nID=0）。
- **无别名**：脚本一律 `LuaValueAnimator()` / `LuaAnimatorGroup(nID)`，没有    
  全局工厂名。
- **手工工厂退役**：`push_gcnew` 保留作底层工具（需要运行时逻辑的工厂仍可    
  用），常规导出一律走 `class_con + constructor_lstate`。

**注意**：`class_con` 挂 `__call` 时，`constructor_lstate` 内部取元表用  
`class_type<T>::type`（只剥 ptr/ref/const，不沿继承链走），与 `class_add<T>`  
注册名天然一致，无需担心挂错元表。

---

## 四、提交/回归检查清单

下次动 lua 模块（或移植到新编译配置）时按此过一遍：

- [ ] 新注册的 **STDMETHOD 接口方法**强转是否带 `UAPI`？（x86 必崩项）
- [ ] 新增事件类是否注册**完整 EventXxx**（非 StEventXxx 裸数据类）？
- [ ] 新增带 `DEF_VAL` 默认参的方法是否走 `class_set_cfun` 包装？
- [ ] 派生类成员变量绑定是否用 `mem_var_derived`？
- [ ] 静态 COM 构建（COM_LIB=ON）是否真跑过 lua？—— commgr2.h 守卫 + `EnableScript` 宏门控两处都要核。
- [ ] VS2010 x64 /O2 下 lua GC 是否过了压力测试？（clearkey volatile 修复还在）
- [ ] 探针/日志代码是否全部还原（git diff 只剩正式修复）？
- [ ] 改过的 CRLF/带 BOM 文件是否字节级复核？
- [ ] lua 脚本用到的每个 C++ 方法是否都 grep 过 `exports/exp_*.h` 确认已绑定？
- [ ] 新增"工厂 new 出来交给脚本"的对象：**IObjRef 系产品**→ `class_def` 直绑，脚本显式 `:Release()`（判据看继承链，恰有同名 `Release()` 的普通对象绑定能编译但语义错）；**生命周期只归属 lua 的普通对象**→ `class_con + constructor_lstate` 或 `push_gcnew`（见 §3.9 判据表）。
- [ ] ctor 首参为 `lua_State*` 的类：构造器是否走 `class_con + constructor_lstate`（勿再手写 lua_CFunction 工厂，见 §3.10）？

---

*本文档由 2026-10 的升级与调试过程整理而成；证据链、完整排查过程见 `.workbuddy/memory/2026-10-01.md` 与 `2026-10-02.md`。*
