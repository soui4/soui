# SOUI 列表类控件框选（Rubber Band）多选支持设计方案

> 版本：2026-09-28（第 4 版，随实现同步更新）
> 范围：`SPanel` 框选框架 + `SListCtrl`、`STreeCtrl`、`SListView`、`SMCListView`、`STileView`、`STreeView` 六个控件的接入
> 验证：`demos/fun_test/test_rubberband.cpp`（29 用例），门禁 `ctest -L '^soui-headless$'` 359/359 通过

## 1. 需求与约束

控件在支持多选时，按住左键拖动应画出"框选矩形"（marquee），松开后覆盖的条目全部选中，Ctrl+拖动为追加选择。

核心约束来自 `SPanel` 内建的 **drag scroll / fling 惯性滚动**：左键按下 + 拖动恰好也是 fling 的手势。两者共享"按下—移动—抬起"消息流，必须裁决优先级：

- **多选（框选）优先于 fling**；
- 未启用多选的控件，fling 行为保持原样，不受任何影响；
- **框选手势与多选能力解耦**（`bandEnable` 属性，见 §3.3）：多选但关闭框选时，拖动进入 fling 滚动。

## 2. 总体设计

### 2.1 分层

```
SPanel（框架层，不认识"条目"）
  ├─ 状态机：drag pending → (阈值 8px) → 框选 / drag scroll → fling
  ├─ band 矩形维护：起点、clamp、最小 1×1、坐标系（SPanel::GetClientRect，即扣除滚动条后的客户区）
  ├─ 绘制：DrawRubberBandSel()（唯一路径走 bandSkin：bandSkin 属性或内置 _skin.sys.selband（colorrect：alpha 填充+1px 边框+圆角，一次 DrawByIndex）；无皮肤（headless 无 sys 资源）不绘制，框选手势与选中逻辑照常）
  └─ 4 个回调（子控件实现"怎么选"）：
       IsRubberBandSelEnabled()   是否启用（多选开关 + bandEnable + 数据就绪）
       OnRubberBandStart()        拍摄选择快照
       OnRubberBandSelect(rc, bAdd) band → 条目集合映射（每控件各自实现，映射内部逐条发单项事件）
       OnRubberBandEnd(rc, bCancelled) 取消时还原选择快照（正常结束为空操作，不发事件）
6 个视图控件（接入层，只做"band 矩形 → 条目集合"的映射）
```

设计原则：**SPanel 只管手势与矩形，条目映射全部下沉到控件**。行式列表用定位器换算区间（O(区间长)），网格/树用矩形相交遍历（带提前 break）。

### 2.2 手势优先级（`SPanel::HandleMouseDrag` 分派顺序）

1. **框选激活**：消费 `WM_MOUSEMOVE`（更新 band）/ `WM_LBUTTONUP`（结束）；其它消息放行。
2. **drag scroll 激活**：原 fling 逻辑不变。
3. **fling 运行中**：带 `MK_LBUTTON` 的 move 先尝试启动框选，失败才转 drag scroll。
4. **drag pending（等待 8px 阈值）**：超过阈值时，`IsRubberBandSelEnabled()` 为真 → 启动框选（起点 = **按下点**）；否则走原 drag scroll。阈值那次 move 同时用于更新 band，不丢帧。

起点固定为按下点（`m_ptDragStart`），保证按下时所在的行参与框选；启动时以该点构造 1×1 初始 band 并立即执行一次 `OnRubberBandSelect`，因此按下点所在条目随 band 启动即进入多选集合。

### 2.3 与 fling 的冲突裁决

- **主动优先**：上述 3、4 两处，框选判定先于滚动分支。
- **被动拒绝**：`SPanel::CancelCaptureMode(CANCEL_REASON_SCROLL)` 在框选中返回 `FALSE`，外层容器开始滚动时无法抢走 capture。
- **ESC 取消**：框选启动时 `SetFocus()` 保证键盘可达；`SPanel::OnKeyDown` 收到 `VK_ESCAPE` 时经统一结束路径 `EndRubberBandSel(bCancelled)` 取消 band，控件侧以 band 开始时的**选择快照**还原（详见 3.1）。
- band 激活期间滚轮被吞（`OnMouseWheel` 直接返回 TRUE）：滚动会让 band 矩形与所覆盖条目错位。

### 2.4 边缘自动滚动

band 激活期间，鼠标进入/超出客户区边缘 24px 区域时自动向该方向滚动：

- `UpdateRubberBandSel` 每次都保存**原始（未 clamp）鼠标点**并重算滚动速度（每 tick 1~4 行，按深入边缘带的深度分档）；
- 用 `Timer_BandAutoScroll`（ID=103，100~102 被滚动条占用）以 40ms 驱动；`SWindow::SetTimer` 转发 Win32 会重置已存在的定时器，因此仅在 off→on 跳变时 Set（`m_bBandTimerOn` 标志），否则鼠标持续移动会让定时器永远无法触发；
- 定时器回调 `OnBandAutoScroll`：按轴向循环 `OnScroll(SB_LINEUP/DOWN)`（返回 FALSE 即到端点），滚动后用最后鼠标点重跑 `UpdateRubberBandSel`；
- 鼠标离开边缘带或 band 结束时停定时器。

**内容锚定（滚动中不丢选择的关键）**：band 起点在 `StartRubberBandSel` 时以**内容坐标**（client + nPos）记录为 `m_ptBandAnchorContent`；`UpdateRubberBandSel` 始终用 `min/max(锚点, 当前点 + 当前 nPos)` 构造 band，再减回当前 nPos 得到传给 `OnRubberBandSelect` 的矩形。效果：

- 自动滚动时鼠标不动但 nPos 增长，band 在内容空间持续向滚动方向延伸，且**始终覆盖 band 开始以来扫过的全部条目**——六个控件的映射都是"从 band 全量重推导"，因此滚动中已选条目不会丢失；
- 传给控件的 band 矩形**允许超出客户区**（视口外部分代表已滚过的内容），各控件映射天然支持（线性控件按 index 区间 clamp 到条目数，树控件全量遍历）；
- 鼠标拖回视口内时 band 收缩，超出锚点区间的条目随之取消，语义与原生列表一致。

### 2.5 统一结束路径

band 的四种结束方式（鼠标抬起 / ESC / 外部 `CancelCaptureMode` / 窗口隐藏禁用 `ClearDragState`）全部收敛到 `EndRubberBandSel(bCancelled)`：清 band 与 drag-pending 状态、停自动滚动定时器、回调 `OnRubberBandEnd(rc, bCancelled)`、`ReleaseCapture`。

### 2.6 矩形不变量

- band 始终 **≥1×1 像素**（`UpdateRubberBandSel` 与 `StartRubberBandSel` 中 `right = max(right, left+1)`，bottom 同理）。否则垂直列表水平拖动（X 不动）产生零宽矩形，`IntersectRect` 永远失败，一行都选不中。
- 坐标系统一为 **SPanel::GetClientRect**（已扣除滚动条），band 不会盖住滚动条。

### 2.7 坐标系约定（重要）

SOUI 中**鼠标消息坐标、`GetClientRect`、绘制 RT 的原点全部是 host 窗口坐标**，控件没有自己的局部坐标空间；而 `Position2Item/Item2Position` 是**内容坐标**（内容 0 映射到控件的"行区起点"）。band 矩形来自鼠标消息，因此控件在把 band 换算为条目区间/条目矩形时，必须先减去行区起点在 host 中的偏移，再加滚动量：

```
内容位置 = hostY - 行区起点(host) + nPos
行区起点 = rcClient.top（+ headerHeight，若该控件条目从 header 下方开始）
```

各控件行区起点与绘制公式（必须与映射完全一致，否则控件不在 host 原点或列表滚动后选中错位）：

| 控件 | 行区起点 | 对应绘制公式 |
|---|---|---|
| `SListView` | `rcClient.top` | `rcItem = rcClient; top += Item2Position - nPos` |
| `SMCListView` | `rcClient.top + GetHeaderHeight()` | `_OnItemGetRect: top = rcClient.top + header + Item2Position - nPos` |
| `SListCtrl` | `GetListRect().top`（含 header） | `HitTest: y -= rcList.top - m_ptOrigin.y` |
| `STileView` | `rcClient.top + GetMarginSize()`（`CalcItemDrawRect` 内已含） | band 与 `CalcItemDrawRect`（host 坐标）直接相交，天然自洽 |
| `STreeView` | `rcClient.top` | `OnItemGetRect: top = rcClient.top + Item2Position - nPos` |
| `STreeCtrl` | `rcClient.top` | `rcClient.top - nPos + iVisible*nItemHei` |

回归测试用"控件放在 host (100,50) + 预滚动 + host 坐标模拟消息"覆盖该换算（`listctrl_band_offset_within_host`、`listview_band_offset_within_host`、`mclistview_band_scrolled_offset_within_host`）。

## 3. 各控件映射实现要点

| 控件 | 启用条件 | band → 条目 | 复杂度 |
|---|---|---|---|
| `SListView` / `SMCListView` | `GetMultiSel() && adapter && locator` | 滚动轴区间：`Position2Item(nBandBegin..nBandEnd)`，**区间端点为排他坐标需 -1px** | O(区间) |
| `STileView` | 同上（tile locator） | `CalcItemDrawRect(i)` 与 band 相交，`rcItem.top > band.bottom` 提前 break | O(可见) |
| `STreeView` | `GetMultiSel() && adapter && locator` | `Item2Position - nPos` 逐可见项相交，`GetNextVisibleItem` 遍历 | O(可见) |
| `SListCtrl` | `m_bMultiSelection` | `(band.top - listTop + origin) / itemHeight` 行区间，写 `DXLVITEM::checked` | O(区间) |
| `STreeCtrl` | `m_bMultiSel`（多选能力） | `bVisible` 计数 + `iVisible*nItemHei` 定位，collapsed 子树跳过 | O(可见) |

公共语义：

- `bAdd=FALSE` 时先清空多选集合再应用 band；`bAdd=TRUE`（Ctrl）保留集合外已有选择。
- **单项选中状态事件（多选实时反馈）**：`SViewBase` / `STreeView` / `STreeCtrl` 的 `AddSelItem` / `RemoveSelItem` 在条目选中状态真实翻转时触发 `EventItemSelChanged`（`iItem` + `bSelected`，索引类四控件）或 `EventTreeItemSelChanged`（`hItem` + `bSelected`，两个树控件）；`ClearSelItems` 经由 `RemoveSelItem` 路由，取消还原、空白清除、单选替换等所有集合变化路径都会逐条发出。Add/Remove 内部按 map 成员资格去重（重复添加/移除同一项不发事件），无需调用方做前后集合对比。band 的 `!bAdd` 路径采用"先清后填"，每次 select 调用会按清空数 + 填入数逐条发事件（无 diff 压缩）。
- **主选中项事件（单选协议）**：`SelChanged`（`EventLVSelChanged` / `EventLCSelChanged` / `EventTVSelChanged` / `EventTCSelChanged`）及其 `SelChanging` 前置**只在单选状态下发出**——单选点击路径即时触发；多选态下一律不发（点击路径、band 全程、取消还原均不发），多选反馈一律走单项事件。
- **编程选中 API 的统一语义**（`STreeView::SetSel`、`SViewBase::SetSel` 及三个子类包装、`STreeCtrl::SelectItem`、`SListCtrl::NotifySelChange` 普通分支）：① 单选态先经 `IsItemSelected` 判重，**同项重选直接返回、不发任何事件**；② 记录只在通过取消检查后才更新（`SelChanging` 被取消时选中记录保持原状，不做"先改后回滚"）；③ `hOldSel` 在任何修改前捕获，`SelChanged` 携带正确的旧/新值，在视觉更新之后发出；④ 单选态只写锚点、不发单项事件；多选态 = 清空集合（逐条单项事件）+ 光标跟随 + 记录新条目，不发锚点事件；清空入参（-1 / `ITEM_NULL`）等价于"选中空"。
- 主选中项随 band 移动指向最后命中的条目（六个控件一致；多选态下锚点仅承担光标职责、不是选中记录，见 §3.2）。
- 取消路径（`bCancelled=TRUE`，ESC 或外部取消）**还原 band 开始时的选择快照**并还原主选中项。注意快照拍摄于 `OnRubberBandStart`，即按下点的 click 选择已生效之后，"按下即选中"的行属于快照、ESC 后保留。
- **多选 ↔ 单选切换的选中迁移**：运行时关闭多选时，若多选集合恰有一条选中项，该选中项迁移到单选标记（`SViewBase` 系迁移到 `m_iSelItem`、`STreeCtrl` 迁移到 `m_hSelItem`、`STreeView` 迁移到 `m_hSelected`、`SListCtrl` 将 `m_nSelectItem` 指向仅存的 checked 项），选中项得以保留；多条选中则整组清除（锚点一并复位）；零条不动。重新开启多选时反向迁移：锚点条目**直接写入 map**（无事件、无重绘——条目本就处于选中态），锚点自身保留**光标职责**不归零；`SListCtrl` 单选态 checked 本就与锚点同步、无需迁移。相关 API：`SViewBase::SetMultiSel`、`STreeView::SetMultiSel`、`STreeCtrl::EnableMultiSelection`、`SListCtrl::EnableMultiSelection`——四处的迁移语义已统一。
- **双轨互斥与锚点的角色分化**：每种模式只有一个选中记录——单选 = 锚点，多选 = map（`SListCtrl` 为 checked 数组）。选中查询（`IsItemSelected` / `GetSelItemCount` / `GetSelItems`）严格按模式分支，只读各自记录，且是**唯一的选中判定入口**（内部代码与调用方一律经它判断，不直接比较锚点或查 map）。六控件全部提供 `GetSelItemCount` / `GetSelItems`（`SListCtrl` 本轮补齐）：多选态返回集合大小/枚举集合，单选态返回锚点 1/0 / 输出锚点。全部控件的锚点在多选态都保留**光标职责**（键盘导航基点、Shift 范围基点、`GetSelectedItem` 返回值），但不再作为选中记录被任何查询读取。
- **空白点击清除选择**：普通左键点击（无 Ctrl/Shift）落在无条目区域时清除全部选择并触发 `SelChanged`（新选中项为空）。Ctrl/Shift+点击空白不清除（保护多选结果）；右键/中键不改变选择。六个控件语义一致：adapter 视图在各自 `OnMouseEvent` 的 `HitTest()==NULL` 分支处理，`SPanel::Ex` 系控件（`SListCtrl` 经 `NotifySelChange(old,-1)` 原生支持、`STreeCtrl::OnLButtonDownEx` 显式处理）。

### 3.1 取消还原快照

| 控件组 | 快照内容 | 实现 |
|---|---|---|
| `SListView`/`SMCListView`/`STileView` | 多选集合索引数组（`SViewBase::SnapshotSelItems/RestoreSelItems`，共用 `m_arrBandSnapshot`） | `ClearSelItems` + 逐项 `AddSelItem` |
| `STreeView` | `SArray<HSTREEITEM>`（遍历 `m_mapSelItems`）+ 光标快照 `m_hBandOldSel` | 同上；band 期间光标 `m_hSelected` 跟随最后命中项，ESC 还原到 band 前位置 |
| `STreeCtrl` | `SArray<HSTREEITEM>`（遍历 `m_mapSelItems`）+ 光标快照 `m_hBandOldSel` | 同上；band 期间光标 `m_hSelItem` 跟随最后命中项，ESC 还原到 band 前位置 |
| `SListCtrl` | 全部条目的 `checked` 数组 + `m_nSelectItem` | 逐行还原并只重绘变化行 |

### 3.2 STreeCtrl 多选与键盘交互

`STreeCtrl` 的选择模型为**双轨互斥**：单选模式只用锚点 `m_hSelItem` 记录，多选模式只用 `m_mapSelItems` 记录，锚点在多选态仅承担**键盘光标**职责（不参与选中记录）：

- XML 属性 `multiSel`；API：`EnableMultiSelection` / `GetMultiSel` / `IsItemSelected` / `GetSelItemCount` / `GetSelItems`；多选态下 `GetSelectedItem()` 返回键盘光标（不是选中记录），取选中集合用 `GetSelItems` / `GetSelItemCount`；
- 多选集合 `SMap<HSTREEITEM,BOOL> m_mapSelItems`；多选态下普通选中（`SelectItem`）将集合**整组替换**为该条目（map 记录）并把光标移到该条目，单选态下 `SelectItem` 只写锚点（map 仅在多选态使用，模式切换时已清理）；
- 选中判定 `IsItemSelected(hItem)`：单选查锚点、多选查 map（严格模式分支——多选态光标不参与判定）；
- 删除条目时级联清理其全部后代的多选标记；`RemoveAllItems` 全清；
- **模式切换迁移**（`EnableMultiSelection`，与 `SViewBase::SetMultiSel` 等统一）：开启时把锚点迁入 map，锚点留作光标（状态不变：无事件、不重绘）；关闭时若 map 恰一条则把该条迁移到锚点（同样无事件、不重绘），多条则 `ClearSelItems` 整组清除（逐条发单项事件）并把锚点归零，零条不动；
- 点击路径的修饰键语义与 `STreeView` 对齐（在 `OnLButtonDownEx` 分流）：Ctrl+点击条目为 toggle（`IsItemSelected ? RemoveSelItem : AddSelItem`）并把光标移到被点击条目；Ctrl/Shift+点击空白、Shift+点击条目均不动选择；无修饰键点击条目走 `SelectItem`（多选分支 = 整组替换 + 光标跟随；点击唯一选中项只移光标、不动集合）；无修饰键点击空白清除全部选择——多选态只走 `ClearSelItems`（逐条单项事件），锚点式 `TCSelChanging`/`TCSelChanged` 是单选协议、多选态不发；
- **band 与光标**（与其他五控件统一）：band 启动时快照光标（`m_hBandOldSel`）；band 期间光标跟随最后命中的条目；正常结束光标留在最后命中项（键盘导航从 band 末端继续）；ESC 取消时光标随选择快照一起还原到 band 前位置；
- **键盘交互**（`OnKeyDown` + `OnGetDlgCode` = `SC_WANTARROWS | SC_WANTSYSKEY`，语义对齐 `STreeView`/`SListView`）：
  - **单选态**：`↑`/`↓` 移动选中项（走 `SelectItem`，锚点事件正常发出）；`←`/`→` 在展开/折叠与移动间切换（有子且已展开 → 折叠，否则移到上/下一可见项）；`Home`/`End`/`PgUp`/`PgDn` 滚动并选中首/末/页首/页尾可见项；
  - **多选态**：普通 `↑`/`↓` = 整组替换为相邻可见项（同 `SelectItem` 多选分支）；`Ctrl+↑/↓` 只移光标不动集合（锚点跟随新光标）；`Shift+↑/↓`/`Shift+PgUp/PgDn/Home/End` = **锚点区间**（Explorer 语义，2026-09-29 第 28 轮定稿）：光标移动、固定锚点 `m_hSelAnchor` 不动，集合变为锚点..光标的**可见序 span**（区间替换，可扩大也可缩小）；`SPACE` toggle 光标条目；`Ctrl+A` 全选可见项；
  - **锚点区间模型**（六控件统一）：每控件有独立范围锚点（`SViewBase::m_iSelAnchor` / `STreeCtrl::m_hSelAnchor` / `STreeView::m_hSelAnchor` / `SListCtrl::m_nSelAnchor`），多选态下 Shift 导航（键盘方向键、翻页键、Shift+点击）以"锚点..光标"为准，其余一切非 Shift 变更（普通点击/方向键、Ctrl+点击、Ctrl+方向键、`SPACE`、`Ctrl+A`、API `SetSel`、band 结束、空白点击、模式切换）都让锚点跟随光标；索引类控件的区间为连续索引（`SViewBase::SetSelRange`），树类为可见序 span（`STreeCtrl::SetSelRange` / `STreeView::SetSelRange`），`SListCtrl` 的 Shift+点击在 `NotifySelChange` 内按锚点区间替换 checked；
  - 可见项遍历辅助：`GetNextVisibleItem` / `GetPrevVisibleItem` / `GetVisibleItemByRow` / `GetLastVisibleItem`（基于 `TVITEM::bVisible`，折叠子树自动跳过）；`ESC` 不拦截（`SetMsgHandled(FALSE)` 回落 `SPanel::OnKeyDown` 走 band 取消）；
  - 修饰键经 `GetKeyState` 读取（与 `SListView`/`STreeView` 一致），headless 测试只能覆盖无修饰键路径，`Ctrl/Shift` 组合键需 GUI 环境人工验证。

### 3.3 框选手势与多选解耦（bandEnable）

框选是**手势**，多选是**能力**，二者独立开关：

- `SPanel` 提供 XML 属性 `bandEnable`（默认 `1`）与运行时 API `EnableBandSel(BOOL)` / `IsBandSelEnabled()`；
- 六控件的 `IsRubberBandSelEnabled()` 统一为 `多选开关 && IsBandSelEnabled() && 数据就绪`：
  - 两个都开 → 拖动优先进入框选；
  - 多选开、框选关（`bandEnable="0"`）→ 拖动走 drag scroll → fling，多选的点击路径（Ctrl+click toggle、Shift range、空白清除）不受影响；
  - 多选关 → 框选自动失效，行为与不支持框选的控件一致；
- `bandEnable` 挂在 `SPanel` 上，所有面板子类（含非列表控件）均可配置，只是对未接入框选回调的控件无效果。

### 3.4 STreeCtrl 选中外观与整行高亮

STreeCtrl 是自绘控件，选中高亮由 `itemSkin` 皮肤配置，`fullRowSel` 控制高亮是否覆盖整行：

- **选中背景走皮肤**：`itemSkin` 属性（与既有 `itemSelSkin` 等价，均指向选中背景皮肤成员）。绘制时按 `SState2Index::GetDefIndex(WndState_Check, checkAsPushdown=true)` 取状态索引并 clamp 到皮肤状态数——多状态皮肤（如内置 `_skin.sys.list.item`，3 态）取选中态，单状态专用皮肤取 0。**未配置时默认使用内置 `_skin.sys.list.item`**（与列表条目观感一致）；只有显式配置 `itemSkin=""` 或 `itemSkin="none"` 才禁用皮肤、回退 `colorItemSelBkgnd` 颜色值。`colorItemSelText` 默认值为 `CR_INVALID`（皮肤模式下不强制改文字颜色，避免浅色底配白字不可读）。
- **整行高亮 `fullRowSel`**（默认 `1`）：开启时选中高亮覆盖整个可见条目行——**包括条目内容前的缩进（树线）空间**（绘制原点在 `rc.left + level*indent`，故左缘取 `-level*indent`、右缘扩到客户区宽 + nPos）；关闭时只高亮文字区域，且 `HitTest` 对文字右缘以外的点击返回空——点击条目文字后方不触发该项选中，而是按空白点击语义清除选择（见 §3.1）。默认取 `1`，保持"整行可点击"的交互。
- **文字矩形独立计算**：`DrawItem` 中文字矩形从 `m_nItemOffset`（toggle/checkbox/icon 占位总宽）起算，**不得复用背景矩形**——整行模式会把背景左缘移到缩进区，复用会导致文字画进树线/图标区域。

## 4. 实现关键约束

以下约束是当前实现正确性的前提，修改框架或控件映射时必须保持。

### 4.1 框架层（SPanel）

1. **状态机清理**。启动框选时同时清除 `m_bDragPending`/`m_bDragStarted`——band 完全取代 drag-pending 状态；残留标志会让松开按键后的鼠标移动重新触发框选且等不到 UP。`ClearDragState`（窗口隐藏/禁用）与 `CancelCaptureMode` 一致地走 `EndRubberBandSel(TRUE)` 并 `ReleaseCapture`。
2. **capture 归属**。`StartRubberBandSel` 统一 `SetCapture()`（容器实现为单槽幂等）：drag-pending 路径按下时已捕获（重复设置无害），fling 路径的按下被消费时未捕获（必须补上），否则鼠标移出窗口后 band 失控。
3. **阈值 move 不丢帧**。跨过 8px 阈值的那个 move 事件在启动 band 后立即用于更新 band，快速拖动 + 松开不会得到停在按下点的空 band。
4. **映射按需重跑**。`UpdateRubberBandSel` 仅在 band 矩形变化时调用 `OnRubberBandSelect`（鼠标移动但覆盖集合不变不触发映射）；边缘自动滚动的速度评估独立于矩形变化，每次 move 都执行。
5. **排他坐标端点**。`rcBand.bottom` 是排他边界，用 `Position2Item(bottom)` 换算会多选一行；必须 `-1` 后换算（`SListCtrl` 的 `(bottom-1-listTop)/h` 与 `SListView`/`SMCListView` 的 `nBandEnd-1` 已统一该语义）。
6. **修饰键状态来源统一**。band 路径与 `SListCtrl` 的点击路径（`NotifySelChange` 的 Ctrl toggle / Shift range，`nFlags` 参数，调用方为 `OnLButtonDownEx`/`OnLButtonDbClick` 等，编程式调用传 0）都从消息 `wParam` 取修饰键，不查 `GetKeyState()`——两个来源真实交互下等价，但 wParam 可测试且路径一致。

### 4.2 测试基建约束

1. **headless 容器**。`SwndContainerImpl` 为抽象类（绘制/宿主纯虚留给 `SHostWnd`）；测试派生 `TestContainer` 补齐约 20 个纯虚（重绘 no-op、`PostTask` 内联执行保证确定性）；`SPanel::OnCreate` 需要内置 `_skin.sys.scrollbar`，向 `GETUIDEF->GetBuiltinSkinPool()` 注入无图 `SSkinScrollbar` 满足消息级测试。
2. **定时器依赖宿主消息循环**。`SWindow::SetTimer` 在 headless 下不会触发，边缘自动滚动的定时器路径靠实现审阅 + GUI 人工验证；测试用 `SetScrollPos` + 重发 move 等价驱动滚动后的映射。
3. **`SetScrollPos` 的语义**。public 的 `SPanel::SetScrollPos` 直接设置 `nPos` 不触发物化，适合测试预滚动；`SetTimer`/`EnsureVisible` 的落点依赖内部 clamp，不可用于需要精确断言的场景。
4. **header 的创建约束**。`SListCtrl` 创建 header 时必须提供 data，XML 缺省会触发自动析构——测试 XML 需带 `<headerStyle wndclass="header"/>`（该约束由 `SListCtrl::CreateChildren` 保证）。

## 5. 已知边界与后续改进

- **fling 中的框选启动分支**实际基本不可达（fling 运行中 `WM_LBUTTONDOWN` 会先停 fling），作为防御逻辑保留。
- **band 期间单项事件粒度**：`!bAdd` 的"先清后填"使每次 select 调用按"清空数 + 填入数"逐条发 `EventItemSelChanged` / `EventTreeItemSelChanged`（不做前后集合对比），大集合下事件量与选中数成正比；需要更低粒度的场景可在子类重载 `OnRubberBandSelect` 自行压缩。
- **自动滚动以垂直轴为主**：水平轴（`STreeCtrl`/`STileView` 的横向内容）同样支持，但速度分档共用同一边缘常量 24px，未按控件方向差异化。
- 不等高 item（`SListViewItemLocatorFlex`）下 band 底边恰压条目边界的映射依赖 locator 的 floor 语义，已在测试用例中等高场景覆盖，不等高场景建议后续补充用例。
- **六控件家族一致性统一**（2026-09-29 第 27 轮：第 26 轮 review 记录的 5 条次要不一致已全部在代码中统一，方向均取多数派）：
  1. band 取消时光标还原：`STreeView` 由条件式改为无条件还原（与其余 5 控件一致）；
  2. Shift+点击区间语义：~~`SListCtrl` 由"替换区间"改为"追加区间"~~（第 28 轮升级为**锚点区间**模型：六控件 Shift 导航统一为"锚点..光标"区间替换，见 §3.2 锚点区间模型）；
  3. 空清除事件：单选清除一个本就为空的选择不再发 `SelChanging/SelChanged` 空事件对（`SViewBase::SetSel`、`SListView/SMCListView/STileView/STreeView::SetSel`、`SListCtrl::NotifySelChange` 全部加守卫）；
  4. SetSel 双实现合并：`SViewBase::SetSel` 虚化，`SListView/SMCListView/STileView` 的 COM 接口 SetSel 同时成为其 override——鼠标点击（`SViewBase::OnItemClick/OnItemClickUp`）与键盘导航走同一实现，`accNotifyEvent` 两条路径都有。多选分支改为复用基类逻辑，含"集合已恰为 {X} 时仅移光标"的 no-op 守卫（保持第 24 轮"同项重选零事件"语义，且不破坏 click-up 折叠）；
  5. Shift+PgUp/PgDn/Home/End：`STreeView` 补齐——滚动后落点项参与选中（单选移动 / 多选 Shift 追加、Ctrl 只移光标），与 `STreeCtrl` 对齐；
  6. `SListCtrl` 键盘支持补齐（第 29 轮）：`OnKeyDown` + `OnGetDlgCode`（`SC_WANTARROWS|SC_WANTSYSKEY`）——↑↓/PgUp/PgDn/Home/End 导航（无光标时从首/末项起）、多选态 SPACE toggle 光标项、多选态锚点区间（Shift 走 `NotifySelChange(..., MK_SHIFT)`，与点击路径共用）、Ctrl+方向只移光标并重置锚点（Ctrl/Shift 组合读 `GetKeyState`，GUI 可用）；`EnsureVisible` 跟随光标滚动；`DrawItem` 对持焦点的多选光标项画 `DrawDefFocusRect` 虚线框（`OnKillFocus` 时重绘消框）。单选态边界按键零事件（复用 `NotifySelChange` 同项守卫）、SPACE 留给对话框处理。

## 6. 验证

| 验证项 | 命令 | 结果 |
|---|---|---|
| 框选与键盘行为 | `fun_test.exe --gtest_filter='soui_rubberband.*'` | 29/29 通过 |
| 回归门禁 | `ctest --test-dir build/demos/fun_test -C Debug -L '^soui-headless$' --output-on-failure` | 364/364 通过 |

测试覆盖：listctrl band 基础选择与事件、未开多选不启用（fling 路径不受影响）、Ctrl 追加选择保留旧选中、treectrl 多选 + 单选互斥、treectrl Ctrl+band 保留原有选择（`treectrl_ctrl_band_keeps_selection`）、**bandEnable 关闭框选手势但保留多选点击**（treectrl / listview 各一条，含 `EnableBandSel(TRUE)` 运行时恢复）、listctrl/treectrl 的 ESC 取消还原、**删除节点不误清无关选中**（`treectrl_removeitem_keeps_unrelated_selection`：band 选中后删除首项其余保持选中、删除含选中后代的子树后代从 map 消失且不波及树尾其余选中）、**清空本就为空的单选不发事件对**（`listview_clear_empty_selection_no_events`：真选中/真清除各一对、空清除零事件）、**锚点区间可扩大可缩小**（`listctrl_shift_click_anchored_range`：普通点击定锚 → Shift+点击 2..5 扩大 → Shift+点击行 3 缩小到 2..3 → 普通点击重置锚点；SListCtrl 的 Shift 路径读消息 nFlags，无头可测；键盘 Shift 组合读 GetKeyState 仍需 GUI 验证）、**band 期间单项选中状态事件**（listctrl / listview / treectrl 各一条：翻转即发、重复添加/移除不发、band 结束与取消均不发锚点 SelChanged、取消还原逐条发）、**host 偏移 + header + 预滚动三重叠加下的坐标换算**（listctrl / listview / mclistview 各一条回归）、**滚动中选择累积不丢失**（`mclistview_band_union_during_scroll`，以 SetScrollPos + 重发 move 等价驱动自动滚动 tick）、空白点击清除选择（listview / treectrl）、**多选 ↔ 单选切换的选中迁移**（listview / treectrl / listctrl 各一条：单条迁移、多条清除、重开多选后锚点迁入 map 并留作光标、Ctrl 取消已迁移条目）、**STreeCtrl 双轨互斥模型**（多选态选中记录只在 map：普通点击记 map + 光标跟随、band 不写锚点、ESC 还原含按下行、事件计数）、**多选态不发锚点式 SelChanging/SelChanged**（listctrl / listview SelSpy：多选态点击只发单项事件）、**STreeCtrl 键盘交互**（单选态 ↑↓ 移动选中、Home/End 首末项、←→ 折叠展开后再移动；多选态 ↑↓ 整组替换、SPACE toggle 光标条目；键盘 × 模式切换组合：迁移后的光标可直接导航、关闭多选迁回锚点）、**SListCtrl 键盘导航**（`listctrl_keyboard_multisel_navigation`：无光标时 ↓ 选中首项、普通 ↓ 整组替换、SPACE toggle 光标项且不移光标、End/Home 跳末/首项、边界 ↑ 不动；`listctrl_keyboard_single_selection`：单选态 ↓/↑ 移动选中、边界键零事件、SPACE 留给对话框）。定时器真实触发路径与 fling 实际滚动效果依赖宿主消息循环，无头测试无法覆盖，需 GUI 环境人工验证；band 视觉表现同属 `soui-gui` 层级；键盘 Ctrl/Shift 组合（Ctrl+↑↓ 只移光标、Shift+↑↓/翻页键 锚点区间、Ctrl+A 全选）依赖 `GetKeyState`，headless 下不可模拟。
