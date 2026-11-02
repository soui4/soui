# SOUI4 事件体系设计方案

> **文档定位**：本文是 SOUI4 **事件与消息体系**的实现级设计文档，承载事件定义形态、两张映射表、订阅容器与派发链的逐条源码证据，以及测试覆盖边界。总体评价与设计层价值见 `doc/SOUI4项目评估报告-2026-10-14.md` §4.3；分工是——**报告只回答"这套设计对开发者的价值是什么"，本文回答"它具体怎么工作、边界在哪里"**。
>
> **实测基线**：主仓 HEAD `889f2d6c8`；文中行号与计数均可用所列宏名 / 符号名复核。

---

## 一、两条互补的机制，各自解决一半问题

SOUI 的事件处理不是"一种模式"，而是**两种模式共用一份事件定义**：

| 模式                      | 解决什么                                      | 形态                                    | 何时决定             |
| ----------------------- | ----------------------------------------- | ------------------------------------- | ---------------- |
| **事件映射表（集中处理）**         | 一个窗口 / 控件到底响应哪些消息与事件——**读表即得，代码维护的第一现场** | 一个类体里一段 `EVENT_MAP_*` / `SOUI_MSG_MAP_*` 块，处理器是普通成员函数 | 编译期             |
| **事件订阅（动态处理）**          | 运行时才知道要挂谁：外部对象、脚本函数、XML 属性、跨窗口广播            | `SEventSet` + `IEvtSlot`，按事件 ID 挂/摘   | 运行期             |

两者的价值不重复：映射表让"静态职责可枚举"（维护性），订阅让"未知扩展点可插拔"（可扩展性）。缺任何一个都会退化——只有表，脚本与主题系统无处接入；只有订阅，事件处理散落到各处的 `connect`/`addEventListener`，读代码时无法回答"这个控件会响应什么"。

与常见框架对照，差别在于**默认把哪一半给足**：

| 框架                | 集中声明处                              | 动态挂接处                                     | 遗留问题                                       |
| ------------------ | ----------------------------------- | ----------------------------------------- | ------------------------------------------ |
| **SOUI4**          | `EVENT_MAP` / `SOUI_MSG_MAP`（编译期展开成函数体内的 if 链，与类定义同处） | `subscribeEvent(id, slot)` + XML `on_*` + Lua | 表不可运行期内省；订阅需自行解绑（§七）                        |
| Qt 5/6            | 无（`QObject` 无"我响应哪些信号"的声明块）         | `connect(sender, &T::sig, recv, &U::slot)`  | 关系只能运行时看，`connect` 散落在构造/初始化里；拼错字符串式 SIGNAL/SLOT 到运行期才炸 |
| WPF               | XAML 属性 `Click="Handler"` + code-behind | `AddHandler` / 路由事件                           | 声明与逻辑分处两文件，控件类的事件契约靠文档而非同一处代码              |
| Flutter           | `build()` 里的 `onPressed: () {…}`      | 闭包 / `ChangeNotifier` 监听                     | 处理器嵌在 widget 树构建代码中，无法枚举"这个控件响应什么"          |
| Electron / Web    | 无                                   | `addEventListener('click', fn)`           | 匿名监听器最常见的内存与行为泄漏点：注销需持有引用，且容易漏              |
| MFC / WTL（本设计的直接血缘） | `BEGIN_MESSAGE_MAP … END_MESSAGE_MAP` | 无内建（需自己加）                                 | 只有静态表，动态扩展要另写一套                             |

SOUI 的取舍是：**把 MFC/WTL 那种"集中一张表"的可维护性保留下来（且不用运行时消息映射查表），同时把 Qt 那种"任意对象可订阅"的动态性补上**，再让同一份事件定义同时喂给 C++、XML 与脚本。

---

## 二、事件定义：一条 `DEF_EVT` 同时产出 ID、结构体、事件类与属性名

事件的唯一声明形态是 `DEF_EVT(EventXxx, EVT_XXX, on_xxx, { 字段… })`（`SOUI/include/event/SEvents.h:387` 起），它展开为四层产物：

| 产物                   | 生成方式                                                                                                              | 位置                              | 消费者                                                                    |
| -------------------- | ----------------------------------------------------------------------------------------------------------------- | ------------------------------- | ---------------------------------------------------------------------- |
| 数值 ID                | `EVT_*` 枚举段：`EVT_INIT=8000`、`EVT_CMD=10000`、`EVT_LV_SELCHANGING=15100`、`EVT_ITEM_SELCHANGED=15200`；`_SOUI_EVENTS` 实测 **111 个枚举项** | `SEvents.h:30-182`（`:32/:66/:123/:127`） | 映射表的 `if (uCode == …)`、`EventID` 常量                                      |
| 数据结构体 `St*`         | `DEF_EVT_STRUCT(n,x)` → `typedef struct n x n;`                                                                   | `SEvents.h:374`、`DEF_EVENT` 的 `:378/:381` 两分支 | 脚本绑定按成员偏移导出（`lua_tinker::class_mem<EventTimer>(L,"uID",&StEventTimer::uID)`） |
| 事件类 `Event*`        | `DEF_EVT_CLASS`：继承 `SEvtArgs` + `St*`，`DEF_SOBJECT(SEvtArgs, WIDESTR(evt_name))` 注册对象名，override `GetID/GetName/Data`，`enum { EventID = id }` | `SEvents.h:342-368`（`:346/:348-359/:360-363`） | C++ 侧 `pEvt->GetData()` 型别转换、引用计数与 COM 查询                                |
| 属性名 `on_*` / `name` | `DEF_EVT` 第三参数；`DEF_EVT_EXT(evt,id,x)` 自动命名 `on_##evt`                                                       | `SEvents.h:384`、`:385`          | XML 属性名与脚本函数名的匹配键（§五）                                                 |

关键设计是 `DEF_EVENT` 的**双分支**（`SEvents.h:376-382`）：C++ 编译时生成"结构体 + 类"，纯 C 编译时只生成 `St*` typedef——同一份事件头可同时被内核、C API 与绑定层包含，不需要第二份事件清单。事件载体 `SEvtArgs`（`SEvents.h:206-339`，类名注册 `L"event"` `:207`）自带 `handled` 计数、`bubbleUp`（默认 `TRUE`，`:211/:313`）、`idFrom`/`nameFrom`/`sender`（构造时 `AddRef` `:315-319`，析构 `Release` `:331-338`），即"发送者生命周期由事件参数自己持有"，派发途中 sender 析构不会悬垂。

规模实测：`SEvents.h` 878 行、`^DEF_EVT(` **109 条**；全仓 `^DEF_EVT` **145 条**，其余 36 条散布在扩展控件头（`controls.extend/hellocharts/event/SChartEvents.h` 11、`STreeBox.h`/`SGroupList.h`/`SChromeTabCtrl.h` 各 4、`gif/SGifPlayer.h` 3、`SHexEdit.h`/`gridctrl/SGridCtrl.h` 各 2 等）。**新增一个事件的成本是在控件头里加一行 `DEF_EVT_EXT`**，无集中注册表、无 ID 分配器、无生成代码。

---

## 三、两张编译期映射表：`ProcessSwndMessage` 与 `_HandleEvent` 都是宏展开的线性 if 链

框架有两套同构但互不相干的表，都用宏把处理器展开成一个虚函数的 if 链，而不是虚表数组或哈希表：

| 维度 | 消息映射表                                                     | 事件映射表                                                                                     |
| ---- | ------------------------------------------------------------ | -------------------------------------------------------------------------------------------- |
| 宏定义 | `SOUI/include/helper/SwndMsgCracker.h`（15 个 `#define`＝4 表框架 + 10 处理器 + 包含守卫） | `SOUI/include/event/SEventCrack.h`（22 个 `#define`＝7 表框架 + 3 链式 + 12 处理器） |
| 展开成的函数 | `virtual BOOL ProcessSwndMessage(UINT uMsg, WPARAM, LPARAM, LRESULT&)`（`:4-8`，根类用 `SOUI_MSG_MAP_BEGIN_ROOT` `:10`） | `virtual BOOL _HandleEvent(SNS::IEvtArgs *pEvt)`（`:14-20`，覆写形态 `_BEGIN_OV` `:21`、分离定义 `_BEGIN2` `:32`） |
| 匹配依据 | `uMsg == WM_*`                                                | `uCode = pEvt->GetID()` 数值匹配；`EVENT_NAME_HANDLER` 系列改按 `pEvt->NameFrom()` 字符串匹配（`:121-130`） |
| 未命中时的去向 | `SOUI_MSG_MAP_END`：`if (!IsMsgHandled()) return __baseCls::ProcessSwndMessage(…)`（`:15-20`）——沿**基类链**上抛 | `EVENT_MAP_END`：`if (pEvt->IsBubbleUp()) … __super::_HandleEvent(pEvt)`，收尾 `return nCount > 0`（`:38-45`；`EVENT_MAP_END2(baseCls)` `:47-54` 显式指定基类） |
| 处理器形态 | `MSG_WM_ERASEBKGND_EX`/`_PAINT_EX`/`_NCPAINT_EX`/`_SETFONT_EX`/`_SETFOCUS_EX`/`_SETFOCUS_EX2`/`_KILLFOCUS_EX`/`_NCMOUSEHOVER`/`_NCMOUSELEAVE`/`_TIMER_EX` | `EVENT_HANDLER:88`、`EVENT_ID_HANDLER:99`、`EVENT_ID_RANGE_HANDLER:110`、`EVENT_NAME_HANDLER:121`、`EVENT_COMMAND:132`、`EVENT_ID_COMMAND:143`、`EVENT_ID_COMMAND_MAGIC:154`、`EVENT_ID_COMMAND_RANGE:165`、`EVENT_NAME_COMMAND:176`、`EVENT_NAME_COMMAND_MAGIC:187`、`EVENT_ID_CONTEXTMENU:198`、`EVENT_NAME_CONTEXTMENU:210` |
| 组合子 | —（只有 `__baseCls` 链）                                       | `CHAIN_EVENT_MAP:60`、`CHAIN_EVENT_MAP_MEMBER:69`（委托给另一个对象的表）、`EVENT_CHECK_SENDER_ROOT:78`（限定 sender 为根窗口）、`EVENT_MAP_BREAK:56`（提前收尾） |
| 实例数（实测，已扣除头文件内的宏定义行） | **114 处** `SOUI_MSG_MAP_BEGIN()` + 1 处 `_BEGIN_ROOT` | **55 处** `EVENT_MAP_BEGIN()` + 2 处 `_BEGIN_OV`；收尾 44 处 `EVENT_MAP_END2` + 1 处 `EVENT_MAP_END` |
| 负载分布 | 覆盖内核与全部控件的消息处理                                      | `EVENT_ID_COMMAND` 使用 **53 处 / 21 个文件**，`EVENT_ID_COMMAND_RANGE` 5 处、`EVENT_ID_RANGE_HANDLER` 3 处——命令族是主用法 |

调用入口对称：消息侧 `SWindow::SwndProc`（`SOUI/src/core/Swnd.cpp:4611`）→ `:4623 ProcessSwndMessage`；事件侧 `SHostWnd::OnFireEvent`（`SOUI/src/core/shostwnd.cpp:1129`）→ `:1161 _HandleEvent`，宿主自身实现 `SHostWnd::_HandleEvent`（`:1962`）。

> **取舍**：编译期 if 链的代价是每处表都展开成一段线性比较、`nCount` 计数与内联的处理器调用，换来的是**零运行期注册开销、零分配、无查表哈希**，且处理器与普通成员函数同形（可直接写 `BOOL OnCmd(IEvtArgs*)`）。它同时放弃了运行期内省——"某个控件支持哪些消息/事件"只能读源码，框架不提供枚举接口。这与资源侧的取向相反：消息与事件走编译期常量比较（`doc/SOUI-资源管理设计方案.md` §五），资源走运行期字符串查找，一个偏吞吐、一个偏可扩展。

---

## 四、运行期订阅：先"开槽"，再挂 `IEvtSlot`

**a. 事件必须先在该窗口上登记（能力面是显式的）**

`SEventSet::subscribeEvent` / `unsubscribeEvent` 都以 `isEventPresent(dwEventID)` 为前提，未登记直接返回 `FALSE`（`SOUI/src/event/SEventSet.cpp:177-182`、`:184-189`）——拼错事件 ID 不会静默成功。登记形态统一是 `GetEventSet()->addEvent(EVENTID(EventXxx))`，`EVENTID(x)` 一次给出 ID 与类名（`SEvents.h:370`），因此 `SEvent` 同时持有 `m_dwEventID` 与 `m_strEventName`（`SEventSet.cpp:8-12`）。实测全仓 **148 处** `addEvent(EVENTID(…))`：窗口基类 `Swnd.cpp:618` 起 27 条、`SRootWindow.cpp:15-19` 5 条（`EventInit`/`EventExit`/`EventMenuCmd`/`EventUpdateCmdUI`/`EventUpdateCmdTip`）、`SItemPanel.cpp` 7 条、`SPanel.cpp` 3 条、`SModalViewSession.cpp:36` 1 条；控件层各自登记（`STreeCtrl.cpp` 9、`SHeaderCtrl.cpp` 5、`SListbox.cpp`/`SCmnCtrl.cpp`/`SComboBase.cpp`/`SRealWnd.cpp` 各 4）。**结论：控件"支持哪些事件"由源码里的 addEvent 清单决定**，这是能力面而不是实现细节。

**b. 容器、去重与触发顺序**

- 数据结构：每个事件一个 `SEvent`（`SEventSet.h:12-82`），槽数组 `SArray<IEvtSlot*> m_evtSlots`（`:82`）；`SEventSet` 用 `SArray<SEvent*> m_evtArr`（`:300`）+ `GetEventObject` **线性查找**（`SEventSet.cpp:119-127`）。单窗口事件量在个位数到 27 之间，框架没有为它上哈希。
- 去重：`SEvent::subscribe` 先 `findSlotFunctor`（逐个 `Equal` 比较，`:42-53`）再 `slot->Clone()` 存副本（`:23-30`）；`unsubscribe` 按同一 `Equal` 定位后 `Release` + `RemoveAt`（`:31-41`）。**订阅语义完全取决于槽的 `Equal` 实现**（见 §六的两个已知坑）。
- 触发：`SEvent::fire`（`:54-76`）先把槽指针快照进 `SAutoRefPtr<IEvtSlot>[]`，随后**倒序**执行（源码注释 "the latest event handler handles the event first."）——因此回调里增删订阅不会破坏本轮迭代，但**后注册者先执行**，与直觉相反；`Run` 返回 `TRUE` 才 `IncreaseHandleCount()`，且一旦 `!IsBubbleUp()` 立即 break。`SEventSet::FireEvent`（`:129-139`）外层再套 `m_nMuted` 引用式静音计数（`setMutedState` `:110-117`，成对增减 + `SASSERT(m_nMuted >= 0)`），用于"批量改动期间不要触发重排/重绘"。
- 窗口级 API：`SWindow::SubscribeEvent` / `UnsubscribeEvent` 只是 `GetEventSet()->subscribeEvent/unsubscribeEvent` 的转发（`Swnd.cpp:4581`、`:4586`）。

**c. 一次 `FireEvent` 的四段派发**

`SWindow::FireEvent`（`SOUI/src/core/Swnd.cpp:2246-2292`）前置 `ASSERT_UI_THREAD()` 与 `m_evtSet.isMuted() || !GetContainer()` 短路，`AddRef()/Release()` 包住整段（防止处理器在派发途中销毁自己）：

| 段 | 动作                                                                                                             | 位置                    |
| -- | ---------------------------------------------------------------------------------------------------------------- | --------------------- |
| ①  | `m_evtSet.FireEvent(evt)`——本窗口的订阅槽倒序执行                                                                     | `:2256-2257`          |
| ②  | 脚本处理器：按 `evt->GetName()` 取 `getEventScriptHandler`，命中则 `GetScriptModule()->executeScriptedEventHandler` | `:2264-2278`          |
| ③  | `GetOwner()->FireEvent(evt)`——向父窗口上抛（订阅槽 + 脚本处理器同样生效）                                                     | `:2280-2283`          |
| ④  | `GetContainer()->OnFireEvent(evt)`——交给宿主窗口的事件表（`shostwnd.cpp:1129 → :1161 _HandleEvent`）                    | `:2285-2290`          |

每段之后重查 `IsBubbleUp()`：处理器把事件标为"不再冒泡"即终止上抛，`bRet = evt->HandleCount() > 0`。这就是"事件冒泡 + 可打断"的完整实现，也是列表控件"框选期间选择集实时逐条反馈、但锚点事件族不发"（评估报告 §4.2）能够成立的机制。与之对照，**系统消息**走 `SwndProc → ProcessSwndMessage → __baseCls` 链（`:4611/:4623`），两条链不互通：消息表不认事件名，事件表不认 `WM_*`。

---

## 五、XML 属性即订阅（声明式入口，零 C++）

`SWindow::DefAttributeProc`（`Swnd.cpp:4006-4015`）把**任意属性名**交给 `m_evtSet.setEventScriptHandler(strAttribName, S_CW2A(strValue, CP_UTF8))`（`:4011`）；`SEventSet::setEventScriptHandler`（`SEventSet.cpp:201-212`）在 `m_evtArr` 里按**事件名**匹配，命中则把处理器字符串存进该 `SEvent` 并返回 `true`，`DefAttributeProc` 随即把 HRESULT 置为 `S_FALSE`——即"该属性已被当作事件消费，不作为普通属性存储"。

需要澄清两点常见误解：① 这里**不检查 `on_` 前缀**，`on_init`/`on_command` 之所以能用，只是因为 `DEF_EVT` 的第三参数按此约定命名（§二）；② 匹配前提是该窗口已 `addEvent` 这条事件（见 §四 a），否则属性落回普通属性、事件也不会触发。

因此 `demos/demo/uires/xml/dlg_main.xml:58` 的 `on_init="on_init" on_exit="on_exit"`、`:42` 动态创建控件里的 `on_command="onEvtTest2"` 都无需 C++ 代码。脚本侧的完整路由链与 Lua 装载细节见评估报告 §5.2。

---

## 六、槽形态：四种 `IEvtSlot`，各有明确适用面与坑

`IEvtSlot`（`SOUI/include/interface/SEvtArgs-i.h:89-90`）四槽：`Run`（`:115`）、`Clone`（`:121`）、`Equal`（`:128`）、`GetSlotType`（`:134`），继承 `IObjRef`——**槽是引用计数对象**，`SEvent` 存的是 `Clone` 副本。

| 槽实现                                   | 位置                                                                                                        | 创建方式                                             | 已知行为与坑                                                                 |
| -------------------------------------- | ----------------------------------------------------------------------------------------------------------- | -------------------------------------------------- | --------------------------------------------------------------------- |
| `FreeFunctionSlot`                     | `SEventSlot.h:35`（`Equal` `:72`）                                                                            | `Subscriber(FunCallback, void* ctx)`（`:257`）        | 比较函数指针 + 上下文，可安全重复订阅/退订                                                    |
| `StdFunctionSlot`                      | `SEventSlot.h:102`（`Equal` `:137-145`）                                                                       | `#if ENABLE_STDFUNCTOR` 的 `subscribeEvent` 重载（`SEventSet.cpp:192-196`） | **`Equal` 恒返回 `TRUE`**，源码注释直接写明"impossible to subscribe an event with two different slots"：同事件上的第二个不同 `std::function` 会被去重逻辑静默拒绝 |
| `MemberFunctionSlot<T, A>`             | `SEventSlot.h:168`（`Equal` `:210-219` 比较成员指针 + 对象指针；`d_object` 为裸 `T*` `:232`）                              | `Subscriber(&T::OnX, &obj)`（`:245-246`）             | 对象销毁后框架**不会自动解绑**，必须显式 `unsubscribeEvent`；框架自身用 RAII 包装规避（见 §七） |
| `LuaFunctionSlot`                      | `components/ScriptModule-LUA/src/exports/luaFunSlot.h:5`                                                      | `CreateEventSlot(pszLuaFun)`（`exports/exp_global.h:101-104`）、`LuaConnect(wnd,idEvt,"fn")`（`:153-157`） | 以 Lua 全局函数名字符串为键；`IWindow::SubscribeEvent` 亦导出给脚本（`exports/exp_IWindow.h:166`），`exp_eventArgs.h` 474 行 / 66 个 `toStEvent*` 负责事件参数装箱 |

---

## 七、两条旁路通道

**a. 全局通知中心（无需 sender 句柄）**

`SNotifyCenter`（`SOUI/include/event/SNotifyCenter.h:100`，头 223 行 / `SOUI/src/event/SNotifyCenter.cpp` 222 行）解决"跨窗口广播、发送方不知道接收方"的形态：`RegisterEventMap`/`UnregisterEventMap`（`SNotifyCenter.cpp:153`、`:165`）、`FireEventSync`/`FireEventAsync`（`:85`、`:92`）、`OnFireEvent`（`:104`）与 `OnFireEvts`（`:123`，异步队列批量冲刷；冲刷节拍来自 `SNotifyCenter(int nInterval)` `:60` 与 `SNotifyReceiver::OnTimer` `:50`）。`TAutoEventMapReg<T>`（`SNotifyCenter.h:33`）用构造注册（`:38`）/ 析构注销（`:46`）包住 `MemberFunctionSlot`（`Subscriber(&_thisClass::OnEvent, this)` `:211`），是当前代码库里对 §六"裸对象指针不自动解绑"这一坑的标准规避姿势。同一组件还承担**线程投递**：`RunOnUI(IRunnable*, bSync)`（`:156`）、`RunOnUI2`（WPARAM/LPARAM 形态 `:165`）、`RunOnUISync`/`RunOnUIAsync`（`:173`、`:179`）与宏 `SRUNONUISYNC`/`SRUNONUI`（`:18`、`:21`，`:20` 注释固定了 `[=]` 值捕获语义），接收端 `SNotifyReceiver::OnRunOnUISync`（`:43`）。跨线程 UI 任务的协作与代价见评估报告 §7.2。

**b. Adapter 观察者（数据变更，不走 `IEvtSlot`）**

`ILvDataSetObserver`（`SOUI/include/interface/SAdapter-i.h:11-12`）经 `registerDataSetObserver`/`unregisterDataSetObserver`（`:79`、`:86`；多列与树另有 `:185/:192`、`:406/:413` 的 `ITv` 形态）挂在适配器上，实现见 `SOUI/include/helper/SAdapterBase.h:145`/`:154`，触发端 `notifyDataSetChanged()`（`:119`），视图侧回调 `onDataSetChanged()`（`SOUI/include/control/SViewBase.h:84`）。这条通道处理的是"数据变了，但没有具体窗口是 sender"的情形，与 MVC 虚拟列表配套；它刻意不复用 `IEvtSlot`，因此也**不受** §六 `Equal` 去重语义的影响。

---

## 八、测试覆盖与缺口

现有覆盖（按 CTest 文件名分层，约定见根 `AGENTS.md`）：

| 用例文件                                     | 层                       | 与事件体系直接相关的断言                                                                                                             |
| ---------------------------------------- | ----------------------- | ----------------------------------------------------------------------------------------------------------------------------- |
| `demos/fun_test/test_rubberband.cpp`     | 单元（`soui-headless`）     | 37 条 / 8 处 `GetEventSet()->subscribeEvent(…)`（`:1501`、`:1681`、`:1714-1715` 等）；断言单项事件逐条触发、不发与不重复发——"订阅→触发→计数"闭环的主要证据                  |
| `demos/fun_test/test_check_sync.cpp`     | 单元（`soui-headless`）     | 6 条 / 5 处订阅（`:162/:164/:334/:404/:463`）；复选状态与选择集事件的双轨同步                                                                    |
| `demos/fun_test/test_e2e_inplace_edit.cpp` | 无窗口核心 E2E               | 7 条 / 6 处订阅（`:145/:195/:236/:294/:340/:357`）；记录 `Event*EndEditLabel` 载荷、在 `BeginEditLabel` 里写 `bCancel = TRUE` 验证**事件可取消**语义 |
| `demos/fun_test/test_gui_smoke.cpp`      | `soui-gui`（默认关闭）         | 唯一直接订阅命令事件的用例：`ASSERT_TRUE(action->SubscribeEvent(EventCmd::EventID, &slot))`（`:80`）                                             |
| `demos/fun_test/test_soui.cpp`           | `soui-interactive`（默认不注册） | `EVENT_MAP_BEGIN() … EVENT_MAP_END2(SHostWnd)`（`:320-341`）完整事件表样例                                                                 |

缺口（逐条可复核）：

- **两张映射表本身无自动化测试**：`fun_test` 目录内 `_HandleEvent`、`FireEvent` 出现 **0 次**，`EVENT_MAP` 仅 2 处且都在默认不注册的 `test_soui.cpp`。§三 的"线性 if 链 + `__baseCls`/`__super` 上抛 + `nCount>0` 返回值"语义，目前 114 处消息表与 57 处事件表都**没有回归网**；改坏 `EVENT_MAP_END2` 不会被现有门禁发现。
- **冒泡 / 静音 / 句柄计数无语义测试**：`IsBubbleUp`、`m_nMuted` 与 `setMutedState`、`HandleCount()`、`SWindow::FireEvent` 的四段派发（含 ③ `GetOwner()` 上抛与 ④ `OnFireEvent` 两分支）均无无头用例。
- **XML→脚本订阅无测试**：`setEventScriptHandler`（`SEventSet.cpp:201`）/`getEventScriptHandler`（`:214`）与 `DefAttributeProc`（`Swnd.cpp:4011`）零用例；`test_lua.cpp` 的 24 条覆盖脚本对象模型但不覆盖"属性名即事件名"这条绑定。
- **`SNotifyCenter` 零测试**：`fun_test` 内 `SNotifyCenter`、`RunOnUI` 出现 0 次。注册/注销的成对性、异步队列的定时器冲刷、`TAutoEventMapReg` 析构注销——正是最容易留下悬垂槽与漏注销的区域——属未验证面。
- **去重语义与生命周期是"源码契约"**：`StdFunctionSlot::Equal` 恒 `TRUE`（`SEventSlot.h:137-145`）与 `MemberFunctionSlot` 裸 `T*`（`:232`）都有明确注释，但没有用例把这两条钉住；使用者若依赖"同一事件可挂多个 `std::function`"，会在升级时无感失败。
- **事件能力面无枚举接口**：145 条 `DEF_EVT` 与 148 处 `addEvent` 只能靠源码扫描得到，框架不回答"这个控件支持哪些事件"，文档与工具链无法自动生成控件—事件矩阵。

---

## 九、如果要改这块，需要知道的约束

1. **事件定义的入口只有一个**：新增事件请在所属控件头里用 `DEF_EVT_EXT` / `DEF_EVT` 声明，并让 ID 落进该控件已用的枚举段（如 15xxx 为列表族），**不要**在 cpp 里手写裸 ID 比较——否则映射表与 XML 属性名两处会失配。
2. **能力先登记**：任何希望被订阅或被 `on_*` 属性命中的事件，必须在该窗口/控件构造路径上 `addEvent(EVENTID(...))`；忘记登记的表现是"订阅返回 FALSE、属性被当普通属性存下"，而不是编译错误。
3. **改收尾宏等于改全库语义**：`SOUI_MSG_MAP_END` 与 `EVENT_MAP_END2` 分别决定 114 / 44 处表的上抛行为，动它们必须先补 §八 缺的那批回归用例。
4. **倒序触发是既定行为**：不要在依赖"先注册先执行"的代码上叠逻辑；若确需顺序控制，用不同的事件 ID 或显式分层（`CHAIN_EVENT_MAP`）。
5. **槽的生命周期归调用方**：优先用 `TAutoEventMapReg<T>` 或 `Subscriber(&T::On, obj)` + 析构前 `unsubscribeEvent`；用 `std::function` 重载时记住"每事件只能挂一个"。
6. **补测试的优先顺序**：映射表上抛/打断 → `SNotifyCenter` 注册注销成对性 → XML `on_*` → `setEventScriptHandler` 绑定（四者都可在无窗口环境用现有 `subscribeEvent` 间谍模式测）。
