# SOUI 消消乐游戏（Lua 实现）详细设计文档

> **版本**：2026-10-07
> **实现代码**：`demos/demo/uires/lua/test.lua`（消消乐部分自第 236 行注释块起，约 1,386 行；全文 1,621 行含跑马机）
> **配套资源**：`demos/demo/uires/` 下的 `xml/page_script.xml`、`values/template.xml`、`values/skin.xml`、`animator/xxl_move.xml`、`anim/xxl_*.xml`、`svg/xxl_*.svg`
> **运行载体**：`components/ScriptModule-LUA`（Lua 5.4 脚本组件；绑定面、事件路由与 ctxId 动画机制见《SOUI4 项目评估报告》第五章）
> **参考实现**：soxxl（基于 soui4js 的 `main.js`）——本实现为其 1:1 移植 + 市场常见玩法完善

---

## 1. 概述

### 1.1 定位

`demos/demo` 脚本页中的两个可玩游戏之一（另一个是跑马机）。它是一个 **8×8 的消消乐（Match-3）**，**全部逻辑用 Lua 编写、零 C++ 编译**：改玩法只需改 `test.lua`（随资源热更），不必重新编译链接宿主程序。

它同时是 `ScriptModule-LUA` 的端到端示范——把「XML 声明 UI + Lua 编排列 + C++ 能力层」这条免编译迭代路径，从「按钮回调」推进到「带动画状态机的完整游戏」：

| 能力 | 提供方 | 在游戏里的落点 |
|---|---|---|
| 窗口树 / 布局 / 绘制 | SOUI 内核 | 棋盘 `gridlayout`、浮层、按钮、LED |
| XML 模板工厂 | SOUI 内核 `SObjectFactory` | `t:g.xxl_ele` / `t:g.xxl_digit` 动态建格子/数字位 |
| 数值动画 / 动画组 | `IValueAnimator` / `IAnimatorGroup` | 交换、聚拢、下落、旋转、抛物线飞币 |
| 栈控件翻页 | `IStackView` | 棋子状态切换（fade）、LED 数字滚动（push） |
| 事件路由 | `SWindow::DefAttributeProc` + `LuaFunctionSlot` | `on_command` → Lua 全局函数 |
| 系统对话框 | `SMessageBox` | 游戏结束弹窗 |

### 1.2 三层结构

```
XML 声明（界面骨架 / 模板 / 动画资源）
   └─ Lua 编排（test.lua：状态机 + 动画链 + 事件回调）—— 本文重点
        └─ C++ 能力（SOUI 内核 + ScriptModule-LUA 绑定）
```

设计上刻意贯彻的原则：**能用数据描述的（布局、棋子外观、动画曲线）都放进 XML/资源；只有需要时序编排与运行时判断的才写 Lua**。

---

## 2. 运行环境与资源接线

### 2.1 页面与控件树

`xml/page_script.xml` 用一个 `tabctrl` 承载两个子页（消消乐 / 跑马机）。消消乐页结构：

```
tabctrl
└─ page "消消乐"
   ├─ window (hbox)
   │   ├─ window (vbox, 左信息卡片)
   │   │   ├─ 标题行（SVG 糖果图标 + 标题/副标题）
   │   │   ├─ button btn_xxl_restart  on_command="xxl_on_restart"
   │   │   ├─ button btn_xxl_hint     on_command="xxl_on_hint"
   │   │   ├─ button btn_xxl_shuffle  on_command="xxl_on_shuffle"
   │   │   ├─ LED 计分区：digit_coin_2/1/0、digit_score_2/1/0（t:g.xxl_digit）
   │   │   ├─ text txt_xxl_combo（连击 / 提示文本，字号 26）
   │   │   └─ 玩法说明文本
   │   └─ window wnd_xxl_board (gridlayout, columnCount=8, weight=1)
   └─ window wnd_xxl_aniframe (pos="0,0,-0,-0", msgTransparent="1")  <-- 整页动画浮层
```

两个关键设计：

- **`wnd_xxl_aniframe` 是整页浮层**：`pos="0,0,-0,-0"` 铺满消消乐页，盖住棋盘与左侧计分区，所有移动中的副本、星芒、飞币都画在这最顶层。**必须用 float 而非放进某个 stack**——stack 会裁剪子窗口。它常驻可见，靠 `msgTransparent="1"`（只显示不挡交互）而非切显隐来让位交互。
- **按钮 `on_command` 直接把点击接到 Lua 全局函数**：`xxl_on_restart` / `xxl_on_hint` / `xxl_on_shuffle`，零 C++ 代码。

### 2.2 模板（`values/template.xml`）

**棋子 `t:g.xxl_ele`**：

```xml
<g.xxl_ele>
  <stack id="{{id}}" size="-1,-1" curSel="0" aniType="fade" cursor="hand" on_command="xxl_on_cmd">
    <img size="-1,-1" skin="svg_xxl_icons" iconIndex="0"/>
    ... 共 7 页（iconIndex 0..6）
  </stack>
</g.xxl_ele>
```

- **七种棋子 = `stack` 的七个页**：Lua 用 `QiIStackView(ele):SelectPage(state, ani)` 切换显示哪一枚。
- `on_command="xxl_on_cmd"`：**一次 XML 声明，64 个格子的点击全部路由到同一个 Lua 函数**（函数内用 `args:IdFrom()` 区分点击了哪格）。
- `aniType="fade"`：翻页走淡入淡出。
- `<data id="..."/>` 在实例化时注入唯一 id。

**LED 数字位 `t:g.xxl_digit`**：

```xml
<g.xxl_digit>
  <stack name="{{name}}" size="22,32" curSel="0" aniType="push" vertical="1" duration="200">
    ... 共 10 页（0..9）
  </stack>
</g.xxl_digit>
```

- 十页 = 数字 0~9。`SelectPage(d, true)` 走 **push 上翻动画**（200ms），即七段 LED 计数器的「翻页」效果。
- `<data name="..."/>` 注入名字（如 `digit_coin_1`）。

### 2.3 动画资源

| 资源名 | 文件 | 内容 | 用途 |
|---|---|---|---|
| `animator:xxl_move` | `animator/xxl_move.xml` | `<RectAnimator duration="200" repeatCount="0"/>` | 位移模板（**无插值器** ⇒ `GetFraction()` 即线性进度 t） |
| `anim:xxl_scale_select` | `anim/xxl_scale_select.xml` | `scale 1→0.8, repeatMode=reverse, repeatCount=-1` | 选中脉冲（常驻循环） |
| `anim:xxl_fx_pop` | `anim/xxl_fx_pop.xml` | 三段 scale（0.2→1.15→1.0→0.05，带停留） | 消除星芒（长停留版） |
| `anim:xxl_fx_pop_fly` | `anim/xxl_fx_pop_fly.xml` | 两段 scale + alpha no-op 占位 | 金币入账链专用（短停留，播完转飞行） |
| `anim:xxl_fx_ring` | `anim/xxl_fx_ring.xml` | `scale 0.25→1.35` + alpha 淡出 | 提示按钮冰环 |

🚨 **`xxl_move` 无插值器** ⇒ `GetFraction()` 返回**线性** t。所有圆周 / 抛物线轨迹都在 Lua 里用这个 t 自己算，形状完全可控（见 §7.3、§6）。

### 2.4 SVG 皮肤（`values/skin.xml`）

| 名称 | 类型 | 说明 |
|---|---|---|
| `svg_xxl_icons` | imglist, 7 态 | 棋子（糖果风 SVG） |
| `svg_xxl_digits` | imglist, 10 态 | LED 数字 |
| `skin_xxl_fx` | imglist | 金色星芒（消除/重开） |
| `skin_xxl_fx2` | imglist | 青色冰环（提示） |
| `xxl_btn_primary` / `xxl_btn_secondary` | imgframe, 3 态 | 药丸形按钮底 |
| `svg_xxl_btn_restart` / `_hint` / `_shuffle` | imglist, 3 态 | 按钮图标 |

### 2.5 脚本入口与生命周期

```
on_init (test.lua:34)              宿主初始化
   ├─ 取宿主窗口、初始化跑马机
   └─ xxl_init(root) (1513)        消消乐初始化
         ├─ 取 wnd_xxl_board / wnd_xxl_aniframe（无则 return 0，demo 不含该页时跳过）
         ├─ 加载 ani_move 模板 + ani_sel 动画
         └─ xxl_restart_internal() (1434)  复位 coin/score/combo/gameover + 建盘

on_exit (71)                        退出
   └─ xxl_exit() (1504)             释放特效动画缓存（fx_ani 每项 Release）+ ani_sel
```

事件路由链（与评估报告 §5.2 一致）：XML `on_command="xxl_on_restart"` → `SWindow::DefAttributeProc` 识别 `on_` 前缀属性 → `setEventScriptHandler` 包装为事件槽 → 事件触发 → `LuaFunctionSlot` 以 pcall 调用同名全局函数。

---

## 3. 数据模型

### 3.1 棋盘数据

`xxl.board[y][x]`：行主序（先 y 行后 x 列），取值 `0..xxl_max_state-1`（共 7 种棋子）。

| 常量 | 值 | 含义 |
|---|---|---|
| `xxl_row` / `xxl_col` | 8 / 8 | 棋盘尺寸 |
| `xxl_max_state` | 7 | 棋子种类数 |
| `xxl_min_same` | 3 | 消除阈值 |
| `xxl_base_id` | 30000 | 棋盘格 id 基址 |

坐标 ↔ id：

```lua
xxl_pos2id(pos) = xxl_base_id + pos.y * xxl_col + pos.x
xxl_id2pos(id) = { x = (id-xxl_base_id) % xxl_col,
                   y = floor((id-xxl_base_id) / xxl_col) }
```

### 3.2 id 空间隔离（三个不重叠段）

| 空间 | 基址 | 增量 | 用途 | 查找方式 |
|---|---|---|---|---|
| 棋盘格 | `xxl_base_id` (30000) | `y*8+x`（0..63） | 64 个常驻棋子格 | `FindChildByID` |
| 浮层副本 | `xxl_base_id+100000` (130000) | `xxl.copy_seq` | 动画替身（`xxl_build_ani_widget`） | `FindChildByID` |
| 特效窗口 | `xxl_fx_base_id` (250000) | `xxl.fx_seq` | 星芒 / 冰环（`xxl_pop_fx`） | `FindChildByID` |

🚨 **三者必须隔离**（文件头「刻意偏离 1」）：级联时同一格可能同时存在多个存活副本，若复用格子 id，`FindChildByID` 会撞回旧副本；旧副本销毁后新动画仍 tick ⇒ **段错误**。

### 3.3 游戏状态表 `xxl`

| 字段 | 类型 | 含义 |
|---|---|---|
| `board` | table | `board[y][x]` = 棋子状态 |
| `click_id` | int | 当前选中格 id（-1 = 未选中） |
| `coin` / `score` | int | 金币 / 得分 |
| `coin_shown` | int | 金币 LED 当前显示值（滚动计数期间落后于 `coin`） |
| `combo` | int | 连击数（第 n 波得分 ×n） |
| `gameover` | bool | 结束弹窗只出一次 |
| `ani_count` | int | 存活动画链计数（全局门禁，见 §4.2） |
| `ani_ctx` | table | `ctxId → 上下文`（kind / element / ani_widget / …） |
| `ani_list` | table | `ctxId → 动画组`（保活用） |
| `ani_orphan` | table | 上一次重建时存活的动画组（GC 悬挂防护，见 §4.3） |
| `ani_seq` / `copy_seq` / `fx_seq` | int | 三类 id 序号 |
| `fx_ani` | table | `"anim:xxx" → 懒加载的动画缓存` |
| `hint_ids` | table | 提示脉冲涉及的格子 id |
| `shuffle_pending` / `shuffle_newboard` | int / table | 洗牌进行中计数 / 新盘数据 |
| `wndBoard` / `aniframe` / `ani_move` / `ani_sel` / `root` | ref | 关键窗口与动画引用 |

### 3.4 坐标模型

**SOUI4 全树共享宿主窗口坐标系**（`Swnd.cpp DispatchPaint` 无逐级平移）。所以 `GetWindowRect2()` 返回的 rect 在棋盘格、浮层副本、按钮、特效之间**可直接互用，无需任何换算**。

定位一律用**显式 `Move(CRect)`**（浮动模式立即生效）；用 `pos` 属性要等下一次 relayout，曾导致「特效时有时无 / 错位」。

---

## 4. 动画运行时框架

### 4.1 数值动画 wrapper 与 ctx 路由

Lua 侧 `LuaValueAnimator()` 包装一个 `IValueAnimator`。回调以 **ctxId 为键**回到 Lua：

```lua
local c  = { ... }                              -- 上下文
local id = xxl_new_ctx("move", c)              -- 分配递增 ctxId 并登记（316）
ani:SetCtx(id)
ani:SetOnUpdate("xxl_ani_update")              -- 回调 fn(luaAni, ctxId)
ani:SetOnEnd("xxl_ani_end")
ani:Start(xxl.aniframe)
-- 回调里： local c = xxl.ani_ctx[ctxId]
```

🚨 **包装 `ani` 必须存进 ctx 保活**：包装对象把自己注册为 `IValueAnimator` 的 update/end 监听器，而 C++ 动画器只 AddRef 了 `IValueAnimator` 本体、不持有包装。若包装被 GC（`~LuaValueAnimator` → Detach 摘监听），`onUpdate` / `onEnd` 不再回调 ⇒ 副本冻结在浮层上冒充棋子、原格子永久隐藏。故必须存入 ctx 直到 `ani_end` 清掉。**这一条是整套机制的根基，散落在 `xxl_begin_move` / `xxl_begin_shuffle_rot` / 金币三段里。**

### 4.2 ani_count 全局门禁

`xxl.ani_count` 既是「还有没有动画在跑」的计数，也是**点击门禁**：

- 每条动画链启动时 +1（动画组启动 / 洗牌链 / 金币入账每段）。
- 组结束 / 段结束时 -1。
- `xxl_on_click` 开头：

```lua
if xxl.ani_count ~= 0 then
    xxl_slog("on_click BUSY drop id=" .. idFrom .. " ani_count=" .. xxl.ani_count);
    return 0;   -- 动画链进行中禁止新点击
end
```

**为什么必须禁**：交换 / 消除 / 下沉链都在组结束回调里才提交数据（`xxl_swap`）与补位（`free_same_x/y`）。并发链捕获的 pos / board 是**中间态**，前后链互相踩踏后数据与显示分叉且无法自愈。

- 归零时补跑 `xxl_on_settle()`（稳定态收尾，见 §9.1）。

### 4.3 保活与生命周期（ani_list / ani_orphan）

- `xxl.ani_list[ctxId] = group` 持有动画组引用，防止被 GC。
- 组结束分发 `xxl_group_end`（1076）里**必须按 ctxId 摘除**：

```lua
if ctxId and xxl.ani_list[ctxId] then
    xxl.ani_list[ctxId] = nil;
    xxl.ani_count = xxl.ani_count - 1;
end
```

  🚨 **不能用 `v == group` 摘除**：C++ 回调把组指针重新 push 成新 userdata，Lua 的 `==` 是 userdata 裸身份比较，恒为 `false` ⇒ `ani_list` 只增不减。ctxId 是组创建时登记的键、回调原样带回，按它摘除才可靠。

- `xxl_init_board` 重建时把 `ani_list` 挪入 `ani_orphan` 保活到下次重建：`LuaAnimatorGroup` 是子动画器的监听者（`SAnimatorGroup::AddAnimator` 存裸指针），若让它被 GC 而子动画器还在跑，子动画器结束回调会**踩悬挂指针**。

### 4.4 浮层副本（动画替身）机制

棋盘格是 `gridlayout` 的布局子项，不能直接做自由位移动画。做法：

```
1. 隐藏原格        ele:SetVisible(false,false)
2. 在浮层建同状态副本  xxl_build_ani_widget(aniframe, state)   (1006)
      CreateChildrenFromXml("<t:g.xxl_ele><data id=cid/></t:g.xxl_ele>")
      + SelectPage(state)            <-- 见下
3. 副本走位移动画    xxl_ani_update 里 ani_widget:Move(luaAni:GetRectValue())
4. 收尾            xxl_ani_end 销毁副本 + 恢复原格可见（带失效）
```

🚨 副本创建后**必须 `SelectPage(state)`**：模板 `curSel="0"` 默认停在 0 号棋子，漏掉会让下沉 / 交换期间所有副本**显示同一枚棋子**。

🚨 恢复可见必须**补失效** `SetVisible(true,true)`（第二个参数 = 失效重绘）；动画期间无副本覆盖此区域。

### 4.5 动画组：只聚合回调、不启动子动画

`LuaAnimatorGroup()` 把多个子动画打包成一个「组结束」回调：

```lua
local group = LuaAnimatorGroup();
group:SetOnGroupEnd("xxl_group_end");
group:SetCtx(ctxId);
group:AddAnimator(ani:GetIValueAnimator());
-- 子动画仍需逐个 Start：
ani:Start(xxl.aniframe);
```

🚨 **`SAnimatorGroup` 只聚合回调，不启动子动画** ⇒ 每个子动画必须单独 `Start`（js 同款）。

组结束分发按 `kind` 分派（`swap` / `clear` / `drop` / `samey` / `drop_y`）。

### 4.6 lua_tinker 的 BOOL 返回约定

🚨 `ani:Start(...)` 等 BOOL 返回在 Lua 侧是**数字**，判失败要用 `ok == 0 or ok == false`（**勿用 `not ok`**）。

---

## 5. 核心玩法流程

### 5.1 点击与选中

入口 `xxl_on_cmd(args)`（1429）→ `xxl_on_click(args:IdFrom(), toSWindow(args:Sender()))`（1353）：

1. 前置校验：`coin>0`、id 落在棋盘段、`ani_count==0`。
2. `xxl_clear_hint()`：任意棋盘点击都收掉上次提示脉冲。
3. **无选中格**：给该格挂选中脉冲动画（`ani_sel:clone()` + `SetAnimation` + `Release`），记 `click_id`。
4. **已有选中格**：
   - 相邻（`xxl_can_swap`，同列差 1 或同行差 1）→ 走交换动画组。
   - 不相邻 → 取消旧选中（`ClearAnimation`），以新点击重新走流程。

### 5.2 交换

两个副本交叉飞行（`xxl_begin_move` 各一次，加入同一组），组结束回调 `xxl_on_swap_end`（1107）：

```lua
xxl_swap(g.pos1, g.pos2);   -- 提交数据：交换 board + 刷新两格 + 全盘检查
xxl.coin = xxl.coin - 1;    -- 每次交换扣 1 金币
xxl_show_coin();
```

**不消除也不回退**（js 同款）：即使交换后不成 3 连也提交交换（代价即那 1 金币）。

### 5.3 消除检测

`xxl_check_board`（587）：先查横（`xxl_check_board_row`，从右往左扫行，621），再查纵（`xxl_check_board_col`，从底往上扫列，595）。

- **一次只处理「一组 3 连」**，补位后再触发下一轮（级联由此形成）。
- 命中 → `xxl_on_get_same_x` / `xxl_on_get_same_y`。
- 都不命中 → `xxl_on_settle`。

### 5.4 消除 → 聚拢 → 下落 → 补位 → 级联

**横向**（`xxl_on_get_same_x`, 1234 → `xxl_on_clear_end`, 1114 → `xxl_on_drop_end`, 1157）：

```
1. aniframe 显示，combo+1，显示连击文本
2. len 个棋子做「聚拢到中心」动画组（副本从各格移到中心格 rcCenter）
3. 组结束 clear_end：
     · 金币数据 +1，启动金币入账链 ① 特效段（见 §6）
     · 若消除行不在顶行（samex.y > 0）：启动「上方每格下移一格」动画组
         （xxl_begin_move，rc2 = rc1 纵向 offset Height）
4. 组结束 drop_end：
     · score += len * combo；显示得分
     · xxl_free_same_x 补位（上方整段下移、顶行随机、带翻页动画）
     · xxl_check_board() 级联
```

**纵向**同构：`xxl_on_get_same_y`（1270）→ `xxl_on_clear_end_y`（1306）→ `xxl_on_drop_end_y`（1345）→ `xxl_free_same_y`（656），下移 **len 格**。

### 5.5 计分与连击

- `combo` 每次 3 连命中时 +1；**新一次交换时清零**。
- 单波得分 = `len × combo`（第 n 波消除 n 倍）。
- 连击文本 `combo ≥ 2` 才显示（`xxl_show_combo`, 351）。
- `settle` 时 combo 清零并清显示。

---

## 6. 金币入账链（三段顺序动画）

设计目标：**金币数据在消除瞬间即 +1，显示层延迟到第三段才滚动**。三段各占一个 `ani_count` 单位；**段尾先启动下一段（+1）再释放本段（-1）**，`ani_count` 不中途归零 ⇒ `settle` 链不会被打断。

```
① 特效段  xxl_coin_fx (388)
     在消除中心播 anim:xxl_fx_pop_fly（短停留版，约 600ms）
     on_animation_stop 事件 → xxl_fx_fly_start (401)

② 飞行段  xxl_coin_fly_begin (423)
     不销毁特效窗口，用数值动画把它「本身」直线移到金币数字区
     · x / 宽 / 高 线性插值
     · y 在线性插值上叠加向上弓起的抛物线：y -= arc·4t(1-t)
       （t=0/1 时为 0，中点最高；起终点精确落两格中心）
     · 弓高 arc = clamp(距离×35%, 60, 150)
     · 终点缩为 22×22
     · xxl_fly_update (461) 逐帧 Move

③ 计数段  xxl_coin_count_begin (489)
     LED 从 coin_shown 滚到 coin（400ms），逐帧 xxl_coin_count_update (515)
     结束 xxl_coin_count_end (526)：对齐终值 + 释放本段
```

兜底：任一环节创建 / 启动失败都**跳过该段直接计分**（不给 settle 链留悬挂单位）。`xxl_coin_fx` / `xxl_fx_fly_start` / `xxl_coin_fly_begin` 三处都有 fallback 分支。

---

## 7. 洗牌（主动技能）

### 7.1 触发与费用

`xxl_on_shuffle`（1605）：点击洗牌按钮 → 若 `coin < xxl_shuffle_cost`（= 5）则提示「金币不足,不能洗牌!」、不扣币不结算；否则扣 5 金币 → `xxl_redeal`（929）。

洗牌**无死局门，随时可用**（死局时玩家必须靠它自救）。

### 7.2 受约束置换（数据层）

`xxl_gen_permutation`（758）：用 **Fisher-Yates** 洗现有棋子位置（**不重新发牌**），约束重试最多 50 次：

- 候选盘**无初始 3 连**（`xxl_board_has_match`, 745）
- 且**保证有解**（`xxl_has_valid_move`, 721）
- 50 次仍不满足（概率极低）接受最后一版，避免死循环。

返回 `nb`（新盘数据）与 `perm`（位置置换表：`perm[i+1]` = 第 i 个棋子 `(x=i%col, y=i/col)` 的目标格 0 基索引）。判定期间临时把 `xxl.board` 指向候选盘。

新盘数据**随置换即时提交**（数据先行）：飞行期间被 `ani_count` 门禁挡住无读取方，棋子到达时 `xxl_on_grid_changed(tPos)` 读到的即该棋子状态，可立即落位显示。

### 7.3 两段式洗牌动画

每枚棋子并发启动，逐枚归位：

**第一段：绕棋盘中心旋转**（`xxl_begin_shuffle_rot`, 789）

```
起点 = 棋子当前 rect 中心，圆心 = 棋盘中心 (cx, cy)
radius = 起点到圆心的距离
ang0   = atan2(dy, dx)                       -- 起始角
turns  = 1.0 + random()×2.0                  -- 1.0 ~ 3.0 圈
duration = turns × 400ms                     -- 约 400ms/圈
SetRangeRect(rcFrom, rcFrom)                 -- rect 恒等不用，仅借 fraction 驱动圆周
a   = ang0 + angDelta × GetFraction()        -- angDelta = dir × turns × 2π
pos = 圆心 + radius × (cos a, sin a)         -- xxl_shuffle_rot_update (834)
末帧位置存入 c.rcEnd
```

**第二段：直线飞往目标格**（`xxl_shuffle_rot_end`, 847 → `xxl_shuffle_move_end`, 880）

```
从轨道终点 rcEnd 直线飞到目标格 rcTo（250ms）
到达 → 销毁副本 → 目标格立即落位显示（xxl_shuffle_arrive, 890）
每枚完成 → xxl_shuffle_piece_done (898) 递减 shuffle_pending
pending 归零 → xxl_shuffle_finalize
```

> 注：这里**没有用 `SAnimatorGroup`**：洗牌是「每枚棋子旋转完立即接力第二段」的逐个衔接，而 `SAnimatorGroup` 的「等全部子动画结束再回调」语义不适合，故在旋转的 `onEnd` 里直接链式启动第二段。

### 7.4 旋转方向：统一顺时针

屏幕坐标系 **Y 轴向下**（金币飞行的「向上弓起」`y -= arc·4t(1-t)` 即证）。在 `x=cos(a), y=sin(a)` 下：

```
a = 0        → 右侧  (cx+r, cy)
a = π/2      → 下方  (cx, cy+r)          <-- 因为 Y 向下，sin>0 即向下
```

即角度**递增** = 右 → 下 → 左 → 上 = **视觉顺时针**。因此 `angDelta > 0`（`dir = +1`）即顺时针。

当前实现为**统一顺时针**：

```lua
local dir = 1;   -- 统一顺时针（屏幕坐标 Y 向下，角度递增即顺时针）
```

> 早期实现为 `local dir = math.random() < 0.5 and -1 or 1`（随机方向），现已统一为顺时针。

### 7.5 全盘归位与收尾

- 洗牌全程 `wndBoard:EnableWindow(0, 1)` **禁用棋盘防误操作**：点击经 hover 链路派发，`IsDisabled(TRUE)` 查父链，禁用棋盘窗口即可挡掉全部格子交互（浮层副本是兄弟窗口，不受影响）。整个洗牌链算一条动画链（`ani_count += 1`）期间也禁止点击。
- `xxl_shuffle_finalize`（909）：逐格**兜底**恢复可见 / 显示（补动画失败棋子的目标格），`ani_count -= 1`，恢复棋盘交互 `EnableWindow(1,1)`，再 `xxl_check_board()` 检查消除行列。
- 恢复交互的**双入口**：`finalize`（正常结束）与 `xxl_init_board`（中途重开兜底）。

---

## 8. 死局检测 / 提示 / 游戏结束

| 功能 | 函数 | 说明 |
|---|---|---|
| 单点是否成 3 连 | `xxl_match_at` (670) | 纯数据，横向 + 纵向 |
| 试交换是否成 3 连 | `xxl_try_swap_creates_match` (695) | 试完**立即还原** |
| 找一个可行交换 | `xxl_find_move` (707) | 扫右邻、下邻，返回 `{p1, p2}` 或 nil |
| 是否有解 | `xxl_has_valid_move` (721) | `find_move() ~= nil` |
| 提示 | `xxl_on_hint` (970) | 提示按钮播青色冰环；两枚棋子复用选中脉冲动画，点击即清除（`xxl_clear_hint`, 993） |
| 结束 | `xxl_game_over` (1587) | `coin ≤ 0` 或 **死局且金币不足洗牌费** → `SMessageBox` 展示得分 → 重开 |

**死局策略**：`settle` 判定解不开时，**不自动洗牌**，只提示「无路可走,请洗牌!」，等玩家花金币手动洗；若金币连洗牌费都不够，则直接游戏结束（死局永远解不开）。

---

## 9. 稳定性设计（防御式）

### 9.1 稳定态自检 `xxl_on_settle`（1539）

常驻、**正常时零输出**。触发条件：无 3 连且无动画在跑（`ani_count == 0` 且无孤儿 ctx）⇒ 判定棋盘稳定，逐格校验：

- 格子存在（`FindChildByID`）
- 格子可见（`IsVisible(FALSE) ~= 0`）
- 数据完整（`board[y][x] ~= nil`）

异常即打日志定位。**直接盯住「棋子缺失 / 消失不恢复」类问题**。稳定后再做市场玩法收尾：连击清零 / 金币耗尽判负 / 死局等待洗牌。

### 9.2 副本 id 全局唯一（见 §3.2）

### 9.3 越界保护：`xxl_free_same_y`（656）

```lua
for i = 0, len-1 do
    local src = y-1-i;
    if src < 0 then break end   -- 清除段贴顶：其余目标格由下面的随机填充覆盖
    ...
```

js 原版的 `this.board[x][y-1-i]` 是**转置索引笔误**，贴顶消除时会越界（js 靠 `setGridState` 的 try/catch 掩盖）。

### 9.4 可见性回滚

副本创建失败时**回滚格子可见性**（`xxl_begin_move` / 交换分支两处），绝不让格子凭空消失。

### 9.5 点击门禁（见 §4.2）

---

## 10. 与 js 参考实现（soxxl）的差异

函数与 js 的 `Board` / `MainDialog` 方法一一对应，**动画衔接顺序与 js 完全一致**。三处刻意偏离（均为修复，**勿改回**）：

| # | 差异 | 原因 |
|---|---|---|
| 1 | 浮层副本 id **全局唯一**（+100000 段） | 级联时同格可能多个存活副本；复用格子 id 会让 `FindChildByID` 撞回旧副本，旧副本销毁后新动画仍 tick ⇒ **段错误** |
| 2 | `freeSameY` **源行越界保护** | js 的 `board[x][y-1-i]` 转置索引笔误，贴顶消除越界（js 靠 try/catch 掩盖） |
| 3 | 副本创建失败时**回滚格子可见性** | 绝不让格子凭空消失 |

**js 版基础上新增**（市场常见玩法）：受约束发牌、连击计分、LED 翻页计数、金币入账链（飞币 + 滚动）、主动洗牌、提示、游戏结束弹窗。

---

## 附录 A：关键函数索引

| 分类 | 函数（行号） | 职责 |
|---|---|---|
| 工具 | `xxl_slog`(303) `xxl_id2pos`(307) `xxl_pos2id`(312) `xxl_new_ctx`(316) | 日志 / 坐标换算 / ctx 分配 |
| 显示 | `xxl_set_digits`(325) `xxl_show_score`(340) `xxl_show_coin`(344) `xxl_show_combo`(351) | LED 与文本刷新 |
| 网格 | `xxl_on_grid_changed`(536) `xxl_set_grid_state`(547) `xxl_get_grid_state`(553) | 格子显示/数据 |
| 交换 | `xxl_can_swap`(558) `xxl_swap`(568) | 相邻判定 / 提交交换 |
| 消除 | `xxl_check_board`(587) `xxl_check_board_col`(595) `xxl_check_board_row`(621) `xxl_free_same_x`(643) `xxl_free_same_y`(656) | 检测与补位 |
| 死局 | `xxl_match_at`(670) `xxl_try_swap_creates_match`(695) `xxl_find_move`(707) `xxl_has_valid_move`(721) `xxl_board_has_match`(745) | 可行性判定 |
| 发牌 | `xxl_gen_board`(726) `xxl_gen_permutation`(758) | 受约束随机 / 置换 |
| 洗牌动画 | `xxl_begin_shuffle_rot`(789) `xxl_shuffle_rot_update`(834) `xxl_shuffle_rot_end`(847) `xxl_shuffle_move_end`(880) `xxl_shuffle_arrive`(890) `xxl_shuffle_piece_done`(898) `xxl_shuffle_finalize`(909) `xxl_redeal`(929) | 两段式洗牌 |
| 提示 | `xxl_on_hint`(970) `xxl_clear_hint`(993) | 可行走法提示 |
| 动画底座 | `xxl_build_ani_widget`(1006) `xxl_begin_move`(1029) `xxl_ani_update`(1050) `xxl_ani_end`(1058) `xxl_group_end`(1076) | 副本 / 位移 / 组分发 |
| 组回调 | `xxl_on_swap_end`(1107) `xxl_on_clear_end`(1114) `xxl_on_drop_end`(1157) `xxl_on_clear_end_y`(1306) `xxl_on_drop_end_y`(1345) | 各阶段数据提交 |
| 特效 | `xxw_fx_stop`(1175) `xxl_pop_fx`(1190) | 星芒 / 冰环 |
| 聚拢 | `xxl_on_get_same_x`(1234) `xxl_on_get_same_y`(1270) | 3 连聚拢动画组 |
| 输入 | `xxl_on_click`(1353) `xxl_on_cmd`(1429) | 点击状态机 |
| 金币链 | `xxl_coin_fx`(388) `xxl_fx_fly_start`(401) `xxl_coin_fly_begin`(423) `xxl_fly_update`(461) `xxl_fly_end`(475) `xxl_coin_count_begin`(489) `xxl_coin_count_update`(515) `xxl_coin_count_end`(526) `xxl_ani_count_dec`(376) | 三段入账链 |
| 生命周期 | `xxl_restart_internal`(1434) `xxl_on_restart`(1445) `xxl_init_board`(1456) `xxl_exit`(1504) `xxl_init`(1513) | 初始化 / 重开 |
| 收尾 | `xxl_on_settle`(1539) `xxl_game_over`(1587) `xxl_show_deadboard_tip`(1598) `xxl_on_shuffle`(1605) | 稳定态与结束 |

> 行号对应当前 `test.lua`（1,621 行）；改动代码后需重新核对。

## 附录 B：资源清单

| 类别 | 资源 | 文件 |
|---|---|---|
| 页面 | `page_script` | `xml/page_script.xml` |
| 模板 | `g.xxl_ele` / `g.xxl_digit` | `values/template.xml` |
| 皮肤 | `svg_xxl_icons` / `svg_xxl_digits` / `skin_xxl_fx` / `skin_xxl_fx2` / `xxl_btn_primary` / `xxl_btn_secondary` / `svg_xxl_btn_*` | `values/skin.xml` + `svg/xxl_*.svg` |
| 动画器 | `xxl_move`（RectAnimator） | `animator/xxl_move.xml` |
| 动画 | `xxl_scale_select` / `xxl_fx_pop` / `xxl_fx_pop_fly` / `xxl_fx_ring` | `anim/xxl_*.xml` |
| 脚本 | `lua_test` | `lua/test.lua` |

> 资源索引登记见 `demos/demo/uires/uires.idx`（`<lua>` / `<animator>` / `<anim>` / `<svg>` 段）。🚨 `uires.idx` 是手工维护的**源文件**，改动资源必须同步它。
