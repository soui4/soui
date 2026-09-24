# Scintilla 无窗口（headless）集成设计要点

> 本文档记录 SOUI 4 中"无真实 HWND 的 Scintilla 编辑器"（`SScintillaView`）的结构设计、
> 关键取舍与已知边界，供后续维护者理解与扩展。文中代码标识、消息/命令、路径保留原文。

## 1. 目标与边界

让 Scintilla 引擎能够在没有真实原生窗口（HWND）的前提下，作为 SOUI 的 DUI 控件渲染、接收输入、
收发通知。核心约束：

- **HWND 与渲染解耦在 `third-part/Scintilla` 内部完成**，借助官方的直调机制
  （`SCI_GETDIRECTFUNCTION` / `SCI_GETDIRECTPOINTER`）与一套新的 Host 抽象，
  业务层不再依赖 `SendMessage(SCI_*)` 走窗口消息循环。
- **不修改 `controls.extend/ScintillaWnd.h`**：原生 HWND 承载路径保持既有行为不变。
- 新增 DUI 控件参考 **`SRichEdit`** 的包装范式（用 SOUI 自身的 `SCaret`/虚拟滚动条）

## 2. 总体架构

```
┌───────────────────────────── SOUI 层 ─────────────────────────────┐
│  SScintillaView (DUI控件, 继承 SPanel 以复用虚拟滚动条)            │
│   ├─ SendEditor() → Scintilla_DirectFunction → ScintillaWin::WndProc│
│   ├─ OnPaint()  → Scintilla_PaintHeadless → OnPaint(HDC, rc)       │
│   ├─ OnTimer()  → Scintilla_TickHeadless → OnTick(reason)          │
│   ├─ OnSetCursor()→ Scintilla_CursorForPointHeadless → 光标决策     │
│   ├─ 通知回调(Notifier) / 计时器回调(TimerHandler) / 光标回调        │
│   └─ SyncScrollBars() ⇄ host_->Get/SetScrollInfo ⇄ 引擎             │
└──────────────┬──────────────────────────────────┬─────────────────┘
               │ ScintillaHost 抽象               │ headless 入口
┌──────────────▼──────────────────────────────────▼─────────────────┐
│                    third-part/Scintilla (ScintillaWin)             │
│   ScintillaNativeHost   : 真 HWND，保持原生行为                     │
│   ScintillaHeadlessHost : placeholder(HWND=0x1)，无真实窗口         │
│   引擎所有 OS 相关能力经 host_ 路由，IsNative()==true 分支不变       │
└────────────────────────────────────────────────────────────────────┘
```

### 2.1 Host 抽象（`third-part/Scintilla/include/ScintillaHost.h`，已从 win32/ 移入 include/）

`ScintillaHost` 接口把 ScintillaWin 中与窗口相关的能力全部抽出来：

- `IsNative()` / `MainHWND()` / `GetClientRectangle()` / `GetCtrlID()` / `SetCtrlID()`
- `InvalidateRectangle()` / `NotifyParent()` / `NotifyChange()` / `NotifyFocus()` / `NotifyDoubleClick()`
- `SetScrollInfo()` / `GetScrollInfo()` / `DefWndProc()`
- `RequestTimer()` / `NotifyCaret()`：默认 **no-op**，仅 `ScintillaHeadlessHost` 覆写后转调 listener。
  把"引擎 → 宿主"的计时器与光标上报收敛进单一 listener 接口，`ScintillaWin` 无需按宿主类型下转型。

两个实现：

- **`ScintillaNativeHost`**：直接映射 Win32 API，重构前后原生行为逐字节一致。
- **`ScintillaHeadlessHost`**：无真实窗口。`MainHWND()` 返回稳定非空占位值 `0x1`
  （仅用于让引擎的 surface / autocomplete 路径认为"存在窗口"，永不解引用）；
  滚动条只**记录**不绘制；通知/计时器/光标通过**回调**送达属主。

### 2.2 引擎侧路由（`third-part/Scintilla/win32/ScintillaWin.cxx`）

ScintillaWin 对所有 OS 相关调用改为 `host_->xxx()`，并用 `IsNative()` 守卫仅为原生路径保留的分支
（COM 初始化、`BeginPaint/GetUpdateRgn`、系统 caret、原生计时器 `SetTimer/KillTimer`、track mouse 等）。

headless 场景新增的一组入口（`include/ScintillaHeadless.h`，由 ScintillaWin.cxx 的 `extern "C"` 导出）：

- `Scintilla_CreateHeadless` / `Scintilla_DestroyHeadless`
- `Scintilla_PaintHeadless` → `OnPaint(HDC, rc)`
- `Scintilla_TickHeadless` → `OnTick(reason)`（驱动 caret 闪烁等细粒度 ticker）
- `Scintilla_DirectFunction` → `WndProc(msg, wp, lp)`（事件转发核心）
- `Scintilla_CursorForPointHeadless` → `CursorForPointHeadless(x, y)`
- `Scintilla_UpdateCaretHeadless` → `UpdateCaretHeadless()`（把引擎当前光标几何推给宿主）

这些入口**复用原生 `WndProc` / `WndPaint` 路径**，由各 `On*` 辅助函数把调用合成对应的 `WM_*`
消息投喂给引擎，从而平台无关的编辑逻辑完全复用、零改动。

headless 没有 `WM_CREATE`，原生只在该分支完成的一次性初始化必须在 `Scintilla_CreateHeadless`
里显式补齐，否则对应功能被静默禁用。当前补的是 `GetIntelliMouseParameters()`（滚轮每格滚动行数
`linesPerScroll`）：初值为 0，而引擎滚轮路径要求 `linesPerScroll > 0` 才会滚动，故 headless
下不初始化则滚轮完全无效。

### 2.3 渲染

`OnPaint` 构造一个合成的 `PAINTSTRUCT`（`hdc`/`rcPaint` 来自属主传入的 HDC 与客户矩形），
走 `WndPaint` 既有的 **OCX 分支**（`wParam != 0` 表示外部传入 `PAINTSTRUCT*`），不新增渲染管线。

`SScintillaView::OnPaint` 采用 SRichEdit 风格：直接在渲染目标自身的 HDC 上绘制，
`SetViewportOrgEx` 把原点移到控件左上角，再把控件本地客户矩形传给引擎
（零基点 `(0,0,w,h)`），不建离屏 RT，不引入 alpha 混合；绘制完成后还原 viewport。

**局部重绘（性能）**：只在确实需要重绘的区域驱动引擎——通过
`pRT->GetClipBox()` 取当前裁剪框（已是 invalidate 区域与客户矩形的交集），与客户矩形
求交后得到子矩形，转成局部坐标传给 `Scintilla_PaintHeadless`；裁剪框不可用或无交集时
回退整窗/跳过。这样引擎只对受影响的行做布局与重绘，而非每帧全文档重绘。

### 2.4 滚动条桥接

`SScintillaView` 继承 `SPanel`，复用 SOUI 的虚拟滚动条（skin 渲染、命中测试、拖动、翻页、滚轮淡出）。

- 引擎计算的滚动条状态经 `ScintillaHeadlessHost::SetScrollInfo` **按 `fMask` 合并字段**记录
  （引擎常用 `SIF_POS` 单独更新，整段覆盖会清掉已存的 range/page），
  再经 `SyncScrollBars()` 灌入 `SPanel`。
- SOUI 滚动条交互由 `OnScroll` 转发给引擎（`WM_VSCROLL/WM_HSCROLL`）。
- 拖动 `SB_THUMBTRACK` 时先把 `nTrackPos` 推进 host，完成后同步回引擎状态；
  `IsThumbTracking()` 期间不切换滚动条显隐、不覆写 `nPos`，避免拖动中滚动条消失。

### 2.5 光标（caret）桥接

无窗口模式没有系统 caret，改为由属主 DUI 控件驱动 **SOUI 的 `SCaret`**：

- `UpdateSystemCaret()` 的 headless 分支调用 `host_->NotifyCaret(x,y,h,shown)`（基类虚方法，
  headless 覆写为回调，native 用默认空实现）把光标几何/可见性**推**给属主（事件驱动，而非轮询）。
- `SScintillaView::OnScintillaCaret` 映射为 `SWindow::CreateCaret/ShowCaret(TRUE)/SetCaretPos`
  （位置要加上控件客户区左上角偏移，转换到 SOUI 绘图坐标系），闪烁由 `SCaret` 自身的 timeline 处理。
- 为避免引擎与 SOUI 各画一条光标（一条固定黑线 + 一条闪烁光标），在 `OnCreate` 里设置
  `SCI_SETCARETSTYLE = CARETSTYLE_INVISIBLE`，关掉引擎自身不闪烁的 caret 条，只保留 SOUI `SCaret`。
- **滚动时的光标同步**：引擎的 `ScrollText`（末尾 `UpdateSystemCaret()`）与 `Redraw` 覆写
  （headless 分支补 `UpdateSystemCaret()`）都会推送最新 caret 几何，宿主无需显式拉取；
  `SScintillaView::OnScroll` 转发 `WM_VSCROLL/WM_HSCROLL` 后只 `SyncScrollBars()` + `Invalidate()`。
- **光标同步采用 push 而非 pull**：引擎在三个光标落位点——`SetSelection(SelectionPosition,
  SelectionPosition)`、`SetSelection(SelectionPosition)`、`SetEmptySelection(SelectionPosition)`
  末尾统一调用 `UpdateSystemCaret()`（native 为空实现、无副作用；headless 走 `NotifyCaret`
  推给属主）。所有光标移动路径（方向键、点击、输入、Undo/Redo、`AutoCompleteInsert` 补全等）
  都汇入这三个落位点，因此光标位置一旦变化即自动推送，不再依赖 `InvalidateCaret`/滚动等
  偶发触发点，也删除了此前 DUI 侧的主动拉取（`Scintilla_UpdateCaretHeadless`）。

### 2.6 计时器桥接

`FineTickerStart/Cancel` 的 headless 分支经 `host_->RequestTimer(reason, millis)`（基类虚方法）
映射到属主的 SOUI 定时器；`millis>0` 启动/复位，`millis<=0` 取消。
`SScintillaView::OnTimer` 再调 `Scintilla_TickHeadless → OnTick(reason)` 驱动引擎内部 ticker，
并触发 `Invalidate()` 让滚动/光标可见。

### 2.7 鼠标光标（cursor）职责

- 引擎在 `WM_MOUSEMOVE` 内会按位置设光标（文本 ibeam / margin 反向箭头 / 热点 hand / 选区 arrow），
  但 `WM_SETCURSOR`（SOUI 容器）也会设一遍。若两者不一致会在同一输入周期互相覆盖造成闪烁。
- 结论：**引擎决策、DUI 落地**。`CursorForPointHeadless(x,y)` 只返回引擎会用的光标枚举、不打平台光标，
  `SScintillaView::OnSetCursor` 用 `GetCursorPos + ScreenToClient(GetHostHwnd())` 取真实位置，
  问引擎取光标类型，再映射到 swinx 标准光标（margin 反向箭头映射到 macOS 最接近的水平双向箭头
  `IDC_SIZEWE`），返回 `TRUE` 阻止容器再次覆盖。
- 删除此前 `OnCreate` 里错误的 `SCI_SETCURSOR, 3`（会把 `cursorMode` 强制为 up-arrow、丢失上下文光标）。

### 2.8 常用配置属性与内置自动完成下拉框

`SScintillaView` 通过 SWindow 属性系统暴露下列常用配置（均可在 XML 上直接写，加载期先存值，
`OnCreate` 时 m_sci 就绪后再落到引擎）：

- `font`：把窗口自身的字体（族 + 字号）下推到引擎的 `STYLE_DEFAULT`。
- `showLineNumber`：切换行号 margin（margin 1）。开启时设 `SC_MARGIN_NUMBER` + 宽度，
  并显式 `SCI_SETMARGINCURSORN(1, SC_CURSORARROW)`，避免 margin 默认的反向箭头被
  OnSetCursor 映射成 `IDC_SIZEWE（<->）`；关闭时宽度置 0（无 margin 区，自然无 `<->`）。
- `autocompleteList`：空格分隔的候选词。

### 2.9 自动完成的 Host 型 ListBox（弹出窗口方案）

用户要求：自动完成不能用"控件客户区自绘 listbox"，必须用**弹出窗口**；并**与 HWND 模式共享引擎的
listbox 弹出方案**。引擎对自动完成的所有逻辑（候选加载、选中移动、Up/Down 键盘、完成动作）都经由
`AutoComplete::lb`（一个 `ListBox*`，见 `src/AutoComplete.h` 的 `AutoComplete` 类）驱动。因此满足需求
的落点是：headless 时让 `AutoComplete::lb` 指向一个**不创建真实窗口、但把数据推给上层**的 `ListBox`
实现（Host 回调型 ListBox），键盘 Up/Down 等仍由引擎 `ScintillaBase::KeyCommand → AutoCompleteMove`
处理，与 HWND 共用同一状态机；DUI 层收到数据后用 SOUI 真实弹出窗口展示，不在客户区绘制。

**"内核先问上层"**：

- `ScintillaHost` 新增虚方法，默认 no-op / false：
  - `virtual bool HandlesAutoComplete() const { return false; }`：内核问上层是否接管 listbox 弹出。
  - `virtual void AutoCompleteNotify(const ScintillaAutoCompleteInfo &) const {}`：把候选/选中/可见/
    弹窗矩形推给上层（与 `NotifyCaret` 同一范式，事件驱动而非轮询）。
- 所有 host → 属主的交互统一收敛为**单一 listener 接口** `ScintillaHeadlessListener`
  （`ScintillaHostListener.h`，轻量头，不拉入 Platform.h/Scintilla.h，避免与 SOUI::Window 冲突）。
  它同时承载上行的通知回调（`RequestTimer`/`NotifyCaret`/`AutoCompleteNotify`/
  `InvalidateRectangle`/`Notify`）与下行的几何查询（`GetClientRectangle`/`GetIMEWindowOffset`）。
  `ScintillaHeadlessHost` 只持有一个 `ScintillaHeadlessListener*`，由 `SetListener()` 安装
  （`SScintillaView` 在 cpp 里用一个不透明的适配器 `SScintillaHeadlessListenerImpl`
  （仅在 `SScintillaView.cpp` 定义、`SScintillaView.h` 只前向声明）实现该接口并传入 host；
  头文件完全不拉入 `ScintillaHostListener.h`，其它依赖 `SScintillaView.h` 的模块无需该 include 路径）；
  所有通知与几何一律转调 listener，不再维护
  多组 `fn+ctx` 回调，也不再 push 缓存 `rcClient`/IMe 原点——属主需要时通过 listener 从 host 侧
  pull 返回。`HandlesAutoComplete()` 在已安装 listener 时返回 `true`，否则返回 `false`
  （退回引擎内部默认创建原生 listbox）。native 走默认空实现，行为不变。

**Host 回调型 ListBox**（不创建真实窗口，仅 headless 构造时由 ScintillaWin 换入 `AutoComplete::lb`）：

- 实现 `ListBox` 接口（`include/Platform.h` 的 `class ListBox`）。关键改动是绕开
  `SetList`（解析候选、填充 `ScintillaAutoCompleteInfo` 后 `host_->AutoCompleteNotify()`）、
  `Select(n)` / `Show(b)` / `Clear()`（同步内存中的当前选中/可见并回发回调）、
  `GetSelection`/`GetValue(n)`/`Length`/`GetDesiredRect`（返回内存态供引擎完成逻辑读取）。
- 弹窗矩形：引擎在 `AutoCompleteStart` 里对返回的 `ScintillaAutoCompleteInfo.rcPopup` 负责；
  Host 型 ListBox 计算当前候选 n 行、结合默认宽高给出弹窗矩形，一并随回调交给上层。
- **内存态回读是必须的**：`AutoCompleteCompleted` 会 `ac.GetSelection()` / `ac.GetValue(item)` 取选中词
  做替换；`AutoCompleteMove` 会 `lb->GetSelection()`。所以 Host 型 ListBox 必须在内存里维护
  "候选数组 + 当前选中 index"，不真正建窗口。

**DUI 侧（`SScintillaView`）**：

- 通过 `ScintillaHeadlessListener::AutoCompleteNotify` 收到候选状态（`SScintillaView` 在 cpp 里以
  `SScintillaHeadlessListenerImpl` 适配器实现该接口并当作 listener 传给 host）；
- `SScintillaView` 实现 `ISDropDownOwner`，事先用 `CreateAccListBox()`（同 `SComboBox::CreateListBox`）
  创建好一个 `SListBox`（ID 为 `IDC_DROPDOWN_LIST`、`SetOwner(this)` 把事件回链给编辑器）。
  `OnCreateDropDown` 把该 `SListBox` 插入下拉根 `pDropDown->GetRoot()` 并 `SetFocus`；
  `OnDestroyDropDown` 移出并恢复其容器，同时把焦点还给编辑器。
- 下拉窗口按需 `new SAutoCompleteDropDown(this)`（`SAutoCompleteDropDown : SDropDownWnd`，
  `OnFinalMessage` 自析构）。**刻意不采用 `ShowWindow`（激活、抢焦点）**，而是仿
  SPropertyGrid 文本输入框自动完成的 `SSearchDropdownList`：用
  `SetWindowPos(HWND_TOPMOST, x, y, cx, cy, SWP_SHOWWINDOW | SWP_NOACTIVATE)` + `SNativeWnd::SetCapture()`。
  `SWP_NOACTIVATE` 是关键——下拉**从不夺走编辑器焦点**，避免 `OnKillFocus` 让引擎把 pending 的
  autocomplete `cancel` 掉（下拉刚弹就关）。`OnKillFocus` 里再加一道守卫 `if (m_sci && !m_pAccDropDown)`
  才转发 `WM_KILLFOCUS` 给引擎，作为安全兜底。
- `ApplyAutoComplete` 首次 `Create(rect,0)` 建窗，之后每次收到回调就刷新候选行、`SetCurSel(sel, FALSE)`
  并统一经 `Adjust()` 用 `SWP_NOACTIVATE` 弹窗/重定位。行高取 `SCI_TEXTHEIGHT`，可见行数最多 10 行
  （超出由 `SListBox` 内部滚动），弹窗矩形经 `CalcAccPopupRect` 锚定 caret 下方：在 frame 坐标系里
  用 caret（控件局部坐标 + 客户区偏移）构造 caret 行矩形，按
  `SComboBase::CalcPopupRect`/`SSearchDropdownList::AdjustDropdownList` 的同一链路
  `FrameToHost → ClientToScreen` 转屏幕坐标，并把 x 钳制在监视器内；靠屏底时翻到上方。
  下拉列表的默认视觉与 `SSearchDropdownList::CreateListBox` 一致（主题背景/边框、1px margin、
  `hotTrack=1` 行悬停高亮）。
- 键盘/滚轮导航由 **`SAutoCompleteDropDown::PreTranslateMessage`** 承担（仿 `SSearchDropdownList` 内部
  `SDropdownList`）：在消息循环预翻译阶段拦截 `WM_MOUSEWHEEL`（让滚轮滚动列表而不是编辑区）以及
  Up/Down/Return/Escape，经 `SNativeWnd::SendMessage`
  回发到原生下拉窗，路由给其焦点 `SListBox`。选中移动 → `EventLBSelChanged` → 编辑器发
  `SCI_AUTOCSELECT(选中词)` 做实时预览；Enter/Esc → `EndDropDown(IDOK)` / 取消；
  `OnDestroyDropDown` 按退出码提交（`IDOK` → `CompleteAutoComplete`，内部 `SCI_AUTOCSELECT`+
  `SCI_AUTOCCOMPLETE`）或 `SCI_AUTOCCANCEL`。双击行经 `FireEvent` 的 `EventLBDbClick` →
  `EndDropDown(IDOK)` 提交。`OnCreateDropDown` 调 `GetContainer()->OnDropdownState(pDropDown, TRUE)`
  以对齐 SOUI 下拉状态机。
- 实时过滤：下拉打开后，用户输入由 `OnChar`（以及引擎回发的 `SCN_CHARADDED`）触发
  `TriggerAutoComplete()` → `SCI_AUTOCSHOW`。
  `SCI_AUTOCSHOW` 的 `wParam`（`lenEntered`）是**当前 token 已输入的字符数**，不是候选列表长度：
  用 `SCI_GETCURRENTPOS` 取 caret、`SCI_WORDSTARTPOSITION`（该版本没有
  `SCI_GETWORDSTARTPOSITION`）取词首，二者差为 `lenEntered`；否则引擎用超大 startLen 把词范围
  拉回整篇文档，匹配不上而 cancel。
  **必须先发 `SCI_AUTOCSETORDER(SC_ORDER_PERFORMSORT)`**：`AutoComplete::Select` 对 `sortMatrix`
  做二分，而 `SC_ORDER_PRESORTED` 假定调用方传的候选串已排序；demo 的 `autocompleteList` 未按前缀
  排序时，二分会错过 'p' 组等前缀并返回 `location=-1` → `Cancel()`。`PERFORMSORT` 让引擎在
  `SetList` 里先排序再二分，前缀匹配才能命中。引擎过滤后经 host 回调型 ListBox 回发状态，DUI 侧
  据此原地刷新下拉。
  在 `SCN_CHARADDED` 内触发须以 `PERFORMSORT` + 正确 `lenEntered` 为前提——早前正是在这两个 bug 下
  （未排序 + 超大 startLen）使 `Select` 在瞬态 caret/词上匹配不到而 cancel。`autocompleteList` 的
  空格分隔语义通过在 `OnCreate` 里 `SCI_AUTOCSETSEPARATOR(0x20)` 保留。
  为避免把整份 `autocompleteList` 全量传给引擎，候选词在 XML 属性加载时预解析为
  **大小写不敏感排序的向量**（`m_vAccWords`，`OnAttrAutocompleteList` 里按空格切分 +
  `std::sort(CompareNoCase)`）。`TriggerAutoComplete` 只在当前 token **至少已输入一个字符**
  时才工作：用 `SCI_GETTEXTRANGE([wordStart, caret))` 取前缀，对排序向量做
  `std::lower_bound` 二分定位前缀区间，收集连续的前缀匹配项（与引擎 `AutoComplete` 默认匹配
  规则一致），把匹配列表经 `SCI_AUTOCSHOW` 交给引擎；**无输入或前缀无匹配时不发**
  `SCI_AUTOCSHOW`（弹窗保持关闭）。
- **Windows headless 平台差异**（`ScintillaWin`/`SScintillaView`）：
  - **双击选词**：引擎无 `WM_LBUTTONDBLCLK` 分支，双击选词由内部对连续 `WM_LBUTTONDOWN` 的计时判定。
    SOUI 把第二次按下吞进 `OnLButtonDblClk`，`SScintillaView::OnLButtonDblClk` 重新以
    `WM_LBUTTONDOWN` 转发给引擎。
  - **剪贴板宿主句柄**：headless 的占位 `MainHWND()=0x1` 在 Windows 上使 `::OpenClipboard(0x1)` 失败
    （macOS 走 swinx 的 `SClipboard` 接受任意 owner 故正常）。`Paste`/`CopyToClipboard` 改用
    `host_->ClipboardOwnerHwnd()`：基类多态新增虚方法（native 返回 `MainHWND()`），headless host
    由 DUI 层在 `OnCreate` 时用 `GetContainer()->GetHostHwnd()` 外部注入真实宿主 HWND
    （`SetClipboardOwner`），不再直接用 NULL。`Ctrl+C/V` 恢复，`Ctrl+Z` 本就不依赖剪贴板。
  - **光标职责划分**：headless 下引擎**只告知光标位置**（`UpdateSystemCaret → NotifyCaret →
    OnScintillaCaret`），**闪烁完全由 SOUI 的 `SCaret` 负责**，内核不管理非 native 的光标闪烁。
    `FineTickerStart` 的 headless 分支仅保留通用宿主 tick（wrap/延迟处理等），与闪烁职责无关。
  - **滚动后光标推送**：`Editor::ScrollTo`（垂直）与 `Editor::HorizontalScrollTo`（水平）滚动生效后
    直接调 `UpdateSystemCaret()`。水平滚动走 `RedrawRect`（headless 只 `InvalidateRectangle` 不推），
    垂直滚动的小幅度 blit 路径也可能绕过 `Redraw()` 的推送，故不在滚动函数内兜底推送（基类空实现，
    native 端无副作用）。
- **销毁路径防重入**：`OnDestroyDropDown` 在调引擎（`SCI_AUTOCCANCEL` / 完成路径）**之前**先把
  `m_pAccDropDown`/`m_bAccPopup` 置空。否则 `SCI_AUTOCCANCEL` 会同步经 host 回调型 ListBox 的
  `Clear() → AutoCompleteNotify(visible=false)` 重入 `ApplyAutoComplete`，而此时弹窗指针仍非空，
  会再次 `EndDropDown(IDCANCEL)` → 第二次 `WM_DESTROY` → 第二次 `OnDestroyDropDown`（日志表现为
  同窗口 249ms 内 `OnDestroyDropDown` → `EndDropDown(code=2)` → `OnDestroyDropDown` 连发，随后
  swinx 侧还会补一条延迟的无效句柄销毁）。先置空指针后，重入路径看到弹窗已注销即跳过再次销毁。
- **补全后的光标同步**：`SCI_AUTOCCOMPLETE` 把引擎 caret 移到插入词尾（`AutoCompleteInsert` 里
  `SetEmptySelection(startPos + lengthInserted)`）。该路径经 §2.5 的 push 机制（`SetEmptySelection`
  末尾 `UpdateSystemCaret()`）自动把新 caret 几何推给属主，`CompleteAutoComplete` 不再显式拉取。
- **重绘条件（防 WM_PAINT 刷屏）**：`Notifier` 不再无条件 `Invalidate()`（那会让消息循环持续处理
  `WM_PAINT`，饿死 `WM_TIMER`）；仅 `SCN_MODIFIED`（输入/删除/autocomplete 完成替换文本）与
  `SCN_PAINTED` 时重绘，保证 autocomplete 完成的文本能刷新出来。
- **局部刷新（dirty rect 上报）**：引擎的失效路径统一经 `host_->InvalidateRectangle(rc)` 上报
  （`ScintillaWin::RedrawRect` 局部、`ScintillaWin::Redraw` 全量补发整 client rect，`Editor::Redraw()`
  内部的 `wMain.InvalidateRectangle` 走 PlatWin native 对占位句柄无效）。headless host 经
  `ScintillaHeadlessListener::InvalidateRectangle` 把矩形回调给 SOUI
  （`SScintillaView` 作为 host 安装的 listener → `OnScintillaInvalidate` → `SWindow::InvalidateRect`），
  `OnPaint` 按 clip 区域只排版/重绘受影响行。SOUI 侧不再对输入/滚动/焦点事件全窗 `Invalidate()`（仅
  `OnMouseMove` 拖选预览保留），重绘完全由引擎上报驱动。
- **仅字符输入触发 autocomplete**：autocomplete 需"键盘事件 **且** 内容变化"同时满足才启动。根因最终
  定位在**引擎侧**：caret 移动存在一个会启动 autocomplete 的入口（方向键移到 `for` 上自动弹
  `false` 等候选即此触发，SSCintillaView 层的移动前后 cancel 无法根治），已删除该引擎入口修复。
  DUI 侧维持最简：只有 `SCN_CHARADDED`（`OnScintillaNotify`）才 `TriggerAutoComplete`，`OnChar`/
  `OnImeChar` 不再手动触发；点击/方向键一律不做 autocomplete 调度。
- **弹窗高度按 listbox 实际行高**：`ApplyAutoComplete` 用 `m_pAccListBox->GetItemHeight()` 计算
  `nHeight = rowH * min(count,10) + 2`，不再用引擎 `SCI_TEXTHEIGHT`（文本行高，与 SOUI 列表行含
  内边距的尺寸可能不一致，导致弹窗偏高）。
- **Esc 退出下拉**：`SScintillaView::OnKeyDown` 在弹窗打开时拦截 `VK_ESCAPE` 直接
  `EndDropDown(IDCANCEL)`，不依赖 `SAutoCompleteDropDown::PreTranslateMessage` 的转发回路
  （该回路的拦截顺序与消息是否到达焦点编辑器都不可靠）。
- **macOS 快捷键（Cmd+c/v/a/z…）**：macOS 上 swinx 把 Command 映射为 `VK_LWIN`/`VK_RWIN`，而引擎
  的编辑命令以 `VK_CONTROL` 判定 → Cmd 组合键不触发。在 `ScintillaWin::WndProc` 的
  `WM_KEYDOWN` 分支，headless（macOS）下把 `VK_LWIN/VK_RWIN` 并入 `KeyDown` 的 ctrl 参数；
  native 路径保持不变。
- **macOS 输入法旁路 Esc**：swinx `SNsWindow::keyDown:/keyUp:` 在 IME 开启时会先把事件交给
  `NSTextInputContext handleEvent:`；该路径会**消费 Esc 并触发 `cancelOperation:` 默认实现
  `NSBeep()`**（可听到系统 beep），且不再产生 `WM_KEYDOWN` → Esc 到不了引擎。修复：事件为 Esc
  （keyCode==53）且当前**无组合文本**（`hasMarkedText==NO`）时，在 consult inputContext 之前
  直接走 `onKeyDown:/onKeyUp:`，把 `VK_ESCAPE` 正常送达引擎（引擎经 KeyMap `SCK_ESCAPE→SCI_CANCEL`
  取消自动完成）。`SScintillaView::OnKeyDown` 的 `MSG_WM_KEYDOWN` 默认 `SetMsgHandled(TRUE)`，
  Esc 不会继续冒泡触发 `SHostDialog` 的 `IDCANCEL` 关闭对话框。
- **headless 剪贴板（Cmd+c/v）**：Scintilla 的 `CopyToClipboard` 用 Win32 延迟渲染惯用法
  `SetClipboardData(fmt, 0)`（NULL 句柄），swinx `SClipboard`（NSPasteboard 后端）不支持延迟渲染；
  且 `SMimeData::GetData` 的 `CF_UNICODETEXT` 路径 `GlobalReAlloc` 参数顺序写反
  （`dwBytes`/`uFlags` 互换）导致 `Cmd+v` 读回失败。修复：① `GetData` 中
  `GlobalReAlloc(hGlobal, (len+1)*sizeof(wchar_t), GMEM_MOVEABLE)`；② `SClipboard::setClipboardData`
  对 `hMem==NULL` 直接返回（丢弃延迟渲染格式，避免向 NSPasteboard 写出空自定义类型、避免
  `IsClipboardFormatAvailable(cfLineSelect)` 误判行粘贴模式）；③ `SMimeData::flush()` 对
  `data==NULL` 防御跳过。RichEdit 走 OLE 数据对象路径不受影响。
- 已删除原客户区自绘路径（`DrawAutoComplete`/`GetAccRect`/`AccItemAt`/`UpdateAutoComplete`/`InsertAutoComplete`
  及 `OnKeyDown` 对 Up/Down/Enter/Tab/Esc 的拦截，以及早先基于 `SAutoCompletePopup`（`SNativeWnd`
  `WS_POPUP`）的轻量自绘原生弹窗——统一收口到 `SDropDownWnd` + `SListBox`）。
- **语法高亮属性（`lexer=`）**：`SScintillaView` 暴露 `lexer` 属性，值可为 `xml`/`cpp`（`text`/`none`/空 = 关闭）。
  `OnAttrLexer` 存值；`OnCreate` 在 `ApplyEditorFont()` 之后调 `ApplyLexer()`（保证 lexer 样式继承编辑器字体）。
  `ApplyLexer()` 按属性调 `SCI_SETLEXERLANGUAGE`（配 `SCI_SETKEYWORDS` / `SCI_STYLESET*`）。具体：
  ① 关闭时 `SCI_SETLEXER(SCLEX_NULL)`；
  ② 打开时先确保 `SCI_SETSTYLEBITS(5)`（XML 样式 `SCE_H_* ≤ 29`、cpp 样式 `SCE_C_* ≤ 24` 均落在 32 内），
  再按语言设 lexer 与关键字列表——cpp 用 `SCI_SETKEYWORDS(0, …)`（语言关键字）与 `SCI_SETKEYWORDS(3, …)`
  （预处理分支），连续地为关键样式 `SCI_STYLESETFORE`/`SCI_STYLESETBOLD` 配可读配色（关键字蓝色加粗、注释绿色、
  字面量暗红、预处理紫、数字棕）。headless 下 Scintilla 静态库 `-DSCI_LEXER` + `GLOB_RECURSE *.cxx` 已编入
  `lexers/LexCPP.cxx`、`lexers/LexHTML.cxx`（含 `SCLEX_XML` 的 `ColouriseXMLDoc`），`SCI_SETLEXERLANGUAGE`
  可用。该路径仅创建/属性应用时执行一次，不参与逐键路径。
- **业务层候选回调（`EventScintillaAccQuery`）**：当 `autocompleteList` 属性未配置（`m_vAccWords` 为空）时，
  `TriggerAutoComplete` 不再直接返回，而是经 `FireEvent` 同步派发 SOUI 事件 `EventScintillaAccQuery`
  （ID `24000`，数据体含只读 `strPrefix`、`nLenEntered` 与**入参/出参 `strCandidates`**，业务层处理者填
  空格分隔候选串）。事件在控制器构造期经 `m_evtSet.addEvent(EVENTID(EventScintillaAccQuery))` 注册；
  `FireEvent` 同步返回后控件立即把 `strCandidates` 回填并 `SCI_AUTOCSHOW`。事件类定义在
  `SScintillaView.h`（`DEF_EVT` + 固定字面量 ID，`EVT_EXTERNAL_BEGIN(10000000)` 之前，避开 SOUI 核心枚举），
  业务层包含该头文件即可订阅。这与"控件内部未提供时，由业务层提供候选"的需求一致；二者互斥：内部
  `autocompleteList` 命中时走二分前缀匹配，否则走业务层事件，二者都无候选则弹窗保持关闭。
  `OnScintillaNotify` 现仅以 `m_sci` 为条件调 `TriggerAutoComplete`（不再以 `m_strAccAll` 非空为前提）。
- **滚动条光标**：`OnSetCursor` 先按 `HasScrollBar` + `GetScrollBarRect` 命中 SPanel 虚拟滚动条条带时直接设
  箭头光标并返回 TRUE（与 `SPanel::OnNcHitTest` 同一规则），再回退到引擎的 `Scintilla_CursorForPointHeadless`
  判定；引擎客户区含滚动条条带，否则会在此处返回文本 I-beam。注意两个坐标空间：`GetScrollBarRect` 返回
  **父相对坐标**，与容器发给 `OnNcHitTest` 的宿主客户区坐标同空间，滚动条命中检测须用 `ScreenToClient`
  (宿主) 后**未减** `GetClientRect().TopLeft()` 的点；而引擎查询需要**控件本地坐标**，须减
  `GetClientRect().TopLeft()`（与 `OnMouseWheel` 一致）。两处必须分开计算，否则控件不在父原点时（如 demo
  的 caption 下方偏移）水平条因纵向偏移整条错位、只剩 beam。

## 3. 关键文件

| 文件 | 职责 |
| --- | --- |
| `third-part/Scintilla/include/ScintillaHost.h` | `ScintillaHost` 抽象 + Native/Headless 两实现 |
| `third-part/Scintilla/win32/ScintillaWin.cxx` | 引擎侧 host_ 路由、headless `On*` 入口与导出函数 |
| `third-part/Scintilla/include/ScintillaHeadless.h` | headless C 入口声明 |
| `controls.extend/SScintillaView.h/.cpp` | SOUI DUI 控件：渲染/输入/光标/滚动条/计时器桥接 |
| `SOUI/src/control/SRichEdit.cpp` | 参考范式（disabled caret、offscreen 渲染、滚动条） |
| `SOUI/include/control/SComboBase.h` | 参考范式（dropdown popup 生命周期） |

## 4. 已知边界与未覆盖场景

- **headless 下 IME 组合态光标未主动驱动**：无原生 IME 组合窗口，组合期间光标闪烁未由属主驱动。
- 无窗口核心 E2E 只验证"资源 → 控件树 → 状态"，不含画面像素/鼠标输入/窗口事件派发；
  字体/行号 margin、自动完成下拉框、鼠标滚轮、滚动条拖动等仍需在 `sci_demo` 中人工核对。
- `MainHWND()` 占用 `0x1` 占位值依赖"不改动即安全"的约定；若未来引擎对 window handle 有更严格要求需重审。

## 5. 构建与验证

- 改动后构建受影响目标：`cmake --build build --target Scintilla ExtendCtrls sci_demo fun_test --parallel`。
- 无头门禁：构建 `fun_test` 后运行
  `ctest --test-dir build -L '^soui-headless$' --output-on-failure`（三层：单元/集成/无窗口核心 E2E）。
- 光标闪烁/黑线、鼠标滚轮、滚动条拖动等 GUI 行为需在 `sci_demo` 中人工核对。