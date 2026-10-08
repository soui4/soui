# 1600 行 Lua 写成一个能玩的消消乐：SOUI 内置脚本模块能写到什么程度

*（这不是"脚本能挂个按钮回调"的演示。拆的是一个真能玩的 8×8 消消乐：动画流水线、浮层副本、抛物线飞币、LED 滚动计数、圆周洗牌 —— 全部由 Lua 编排，宿主侧一行 C++ 都没写。仓库里都能对上。）*

## 先看这篇想解决什么

GUI 框架说"内置脚本"，很多时候兑现成的是：

```lua
script.onClick = function() print("hi") end
```

你写三行，它就是一个更麻烦的按钮回调。用户真正想知道的其实是另一句：**玩法逻辑、原型验证、动画时序这类要反复改的东西，能不能不付 C++ 编译链接的代价？**

SOUI 内置脚本组件（`components/ScriptModule-LUA`）给的答案在当前仓库状态下是**能**。证据不是演示代码，是 `demos/demo` 里一个已经能玩的消消乐：8×8 棋盘、7 种棋子、消除 / 聚拢 / 下落 / 级联 / 连击计分、金币三段入账动画、花 5 金币的洗牌技能、死局检测、结束弹窗 —— 全部由 `uires/lua/test.lua` 一个文件驱动，共 **1,621 行**。

而且它**不是 Windows 专属**。这一点下面整节专门讲。

---

## 一、先看效果：它到底能干成什么

| 你做这件事 | 你看到的效果 | 最终是谁在管 |
| --- | --- | --- |
| XML 里写 `<script src="lua:lua_test">` | 整个对话框的脚本挂上了 | 构建期接线 |
| XML 里写 `on_command="xxl_on_restart"` | 按钮一点就进 Lua 函数，**零 C++** | 运行时事件路由 |
| 脚本里改一个 `duration="200"` | 手感立刻变，宿主程序不用重编 | 动画资源 |
| 脚本里调 `SelectPage(n, true)` | 棋子 / LED 数字带翻页动画切换 | SOUI 动画层 |
| 脚本里编排 `xxl_ani_update` / `xxl_ani_end` | 多段顺序动画串成一条结算链 | Lua 状态机 |
| 洗牌改一行 `dir = 1` | 所有棋子顺时针绕一圈归位 | Lua + C++ 动画 tick |

一句话：**XML 管界面，Lua 管时序与规则，C++ 管能力与帧。**

这套分工不是喊口号，而是有一个很明确的取舍 —— **不是把游戏全搬进 Lua，而是把每帧级调用留在 C++ 侧，Lua 只在回调里提交数据**。既是性能取向，也是它能撑起完整游戏的地基。

---

## 二、跨平台这条线：同一份脚本，不为平台改过一个字

这一节是全文最该看的部分。

"跨平台"在框架宣传里是最容易空转的词。所以这里不讲愿景，只列三处你可以自己打开仓库核对的事实。

### 2.1 底座：脚本下面垫的是 swinx，不是一个 Windows 壳

SOUI 整套东西要跨端，靠的是子模块 **swinx** —— 一个把 Win32 语义铺到别的系统上的兼容层。

| 项 | 现状 |
| --- | --- |
| 覆盖平台 | Windows（原生）、Linux、macOS、iOS、Android、鸿蒙 —— 六端 |
| 公开头文件 | 44 个 / 34,335 行声明 |
| swinx 自身实现（不含第三方 vendored） | 23,492 行 |
| 扫到的 Win32 API 声明 | 1,042 个 |
| 其中在 `src` 里有落地定义的 | 913 个（实现 884 / 半实现 6 / 桩 23） |

换句话说：**脚本模块导出的那些 SOUI 接口，落到 Linux / macOS / 鸿蒙 上时，底下垫的是 swinx 的实现，不是"Windows 专属的壳 + 空函数"**。

### 2.2 同一份 `.lua`，一个 `#ifdef` 都没有

这是最硬的一条证据。整个 `uires/lua/test.lua` —— 1,621 行、包含完整的消消乐逻辑 —— 里：

```
grep -n "#ifdef\|#if\|WIN32\|linux\|APPLE" test.lua
（无输出）
```

不是"用了跨平台库所以不用判断"，是**连判断的机会都没有留过**。同一份文件在 Windows 编出来的 exe 里跑，在别的端编出来的包里也是这份文件原样跑。


### 2.3 反过来讲清边界

值得说清楚的是：**脚本层之所以能跨平台，不是因为脚本做了抽象，而是因为下面的底座本来就跨**。lua 自己不管窗口、不管消息循环、不管绘制；它编排的那些对象（`IWindow`、`IValueAnimator`、`IAnimation`）在六端都是同一套接口，具体行为由 swinx 那一层决定。

所以结论是实的：

1. **同一份 `.lua` 不改，六端可用**；中文用 `L()` 包一下即可。
2. **跨平台成本落在底座（swinx 913 个落地 API），不落在你改玩法逻辑的动作上**。
3. **它也不是"脚本层替你解决跨平台"** —— 你在脚本里绕开平台 API，照样只能在某端跑。

---

## 三、脚本在这里干什么：从回调胶水到动画编排

从"按钮回调"到"能玩的完整游戏"，中间隔着的其实是**动画编排能力**。

### 3.1 接线只要四处

| # | 位置 | 内容 |
| - | ---- | ---- |
| ① | `demos/demo/demo.cpp:107` | `cfg.EnableScript(TRUE)` —— 应用启用脚本 |
| ② | `demos/demo/uires/uires.idx:173` | `<file name="lua_test" path="lua\test.lua" />` —— 注册为资源 |
| ③ | `demos/demo/uires/xml/dlg_main.xml:23` | `<script src="lua:lua_test">` —— XML 挂载脚本 |
| ④ | `page_script.xml:42/48/55` | `on_command="xxl_on_restart"` 等 —— 事件直指脚本函数 |

从 XML 属性到 Lua 全局函数的完整路由链（逐段回源码核对过）：

| 环节 | 实现 | 位置 |
| ---- | --- | ---- |
| XML 解析 `on_` 前缀属性 | `SWindow::DefAttributeProc` 取值为脚本函数名 | `Swnd.cpp:4005`（`setEventScriptHandler` 于 `:4010`） |
| 包装成事件槽 | `SEventSet::setEventScriptHandler` | `SEventSet.cpp:201` |
| 事件触发查表 | `FireEvent` 命中脚本槽 | `SEventSet.cpp` |
| 进 Lua | `LuaFunctionSlot::Run` 以 pcall 调同名全局函数 | `exports/luaFunSlot.h:17` |

所以 `on_command="xxl_on_restart"` 是**零 C++ 代码**把按钮点击接到 Lua 的。

### 3.2 七种棋子，其实是 stack 的七页

模板文件 `values/template.xml` 里就这么几行：

```xml
<g.xxl_ele>
  <stack id="{{id}}" size="-1,-1" curSel="0" aniType="fade" cursor="hand" on_command="xxl_on_cmd">
    <img size="-1,-1" skin="svg_xxl_icons" iconIndex="0"/>
    <img size="-2,-2" skin="svg_xxl_icons" iconIndex="1"/>
    ... 共 7 页（iconIndex 0..6）
  </stack>
</g.xxl_ele>
```

七种棋子 = 一个 `stack` 的七个页，切换显示只要一行：

```lua
local stackApi = QiIStackView(ele);
stackApi:SelectPage(xxl.board[pos.y][pos.x], enableAni);
```

更省的是 `on_command="xxl_on_cmd"` 那行 XML：**一次声明，64 个格子的点击全路由到同一个 Lua 函数**，函数里用 `args:IdFrom()` 区分点了哪一格。

计分区是同款思路的十页版（LED 翻页计数器），`SelectPage(d, true)` 走 200ms 的 push 上翻，就是老式计数器的翻页效果。

### 3.3 三个绕不开的设计

- **整页动画浮层**：`wnd_xxl_aniframe`（`page_script.xml:82`）是 `pos="0,0,-0,-0"` 铺满整页的 float 层，只显示不挡交互。移动副本、消除星芒、飞行金币都画在它顶层。**必须 float** —— stack 会裁剪子窗口。
- **棋盘格是 gridlayout 的布局子项，不能直接自由位移**。解法是"隐藏原格 → 浮层建同状态副本 → 副本走动画 → 结束销毁副本恢复原格"。副本创建后必须补一次 `SelectPage(state)`，否则模板停在 0 号棋子，下沉期间所有副本长得一样。
- **动画资源全是 XML 声明**，改时长改曲线不碰代码：

| 资源 | 参数 | 用途 |
| ---- | ---- | ---- |
| `animator:xxl_move` | `duration=200`，无插值器 ⇒ `GetFraction()` 就是线性进度 t | 位移模板 |
| `anim:xxl_scale_select` | 缩放 1→0.8，`repeatMode=reverse` 无限 | 选中脉冲 |
| `anim:xxl_fx_pop` | 0.2→1.15→1.0→0.05 三段缩放 + 停留 | 消除星芒 |
| `anim:xxl_fx_ring` | 380ms 0.25→1.35 + 300ms 淡出 | 提示冰环 |

### 3.4 ctxId 路由：回调里只带一个数字

有个反直觉的坑：异步回调到达时，**Lua 侧持有的 C++ 对象身份不可靠** —— 回调里对象指针每次都被重新 push 成新 userdata，而 Lua 的 `==` 是 userdata 裸身份比较，恒为 `false`；控件还可能已销毁重建。

解法是模板克隆 + ctxId：

```lua
local id = xxl_new_ctx("move", c)     -- 分配递增 id 并登记上下文
ani:SetCtx(id); ani:SetOnUpdate("xxl_ani_update"); ani:SetOnEnd("xxl_ani_end")
ani:Start(xxl.aniframe)               -- 每个动画都要单独 Start
```

回调只带一个数字，Lua 侧拿 `xxl.ani_ctx[ctxId]` 取回自己的上下文，不依赖任何可能悬空的对象。

配套是**对象生命周期三分法**（用错直接泄漏或双释放）：

| 类别 | 入栈方式 | 释放责任 |
| ---- | ---- | ---- |
| 单例 / 宿主 / 窗口树对象 | 普通 push（不拥有） | C++ 管，**脚本绝不可 Release** |
| 工厂 new 出的 `IObjRef` 产品 | `class_def` 直绑 | **脚本必须显式 `:Release()`**（`LoadAnimation`、`CreateTimer`） |
| 生命周期只属 Lua 的包装对象 | `constructor_lstate` | GC 直接 delete，**绝不可 Release**（`LuaValueAnimator`） |

### 3.5 动画组：只聚合回调，不启动子动画

`SAnimatorGroup` **只聚合回调、不启动子动画**，漏掉每个子动画的 `Start`，"组结束"永不触发、状态机直接卡死。

组结束回调里还有个很实在的坑：**不能用 `v == group` 摘除** —— C++ 回调把组指针重新 push 成新 userdata，`==` 恒 false，列表只增不减。得按创建时登记的 `ctxId` 摘除。整条玩法状态机就是这张分发表：`swap / clear / drop / samey / drop_y`。

### 3.6 金币三段入账，全是脚本串的

```
① 特效段  消除中心播短停留星芒(约 600ms)
   ↓ on_animation_stop
② 飞行段  把特效窗口沿抛物线飞到金币数字区(400ms)
   ↓ 落地
③ 计数段  LED 从当前值滚到数据值(400ms)
```

中间那段最有意思：抛物线不是动画资源给的，是脚本用线性 fraction 自己叠出来的 —— `t = GetFraction()`，再减一个 `arc` 项拱起来，起终点精确落在两格中心。

三段用 `ani_count` 单位串联：**段尾先启动下一段（+1）再释放本段（-1）**，中途不归零，结算链就不会被打断。

### 3.7 洗牌与稳定态自检

洗牌是主动技能（花 5 金币），数据层用受约束 Fisher-Yates：洗现有棋子位置、约束重试最多 50 次，候选盘必须无初始 3 连且保证有解。动画是两段式、每枚棋子并发：第一段借一个 rect 恒等的位移动画**只借它的 fraction 驱动圆周运动**，第二段从轨道终点直线飞到目标格。

`xxl_on_settle` 是常驻自检点：无 3 连且动画计数归零时逐格校验"存在 / 可见 / 数据非空"，异常打日志。它**正常时零输出**，专门盯"棋子缺失、消失不恢复"这类最难复现的问题。

---

## 四、它干得怎么样

**好的地方：**

- **改一行就能看效果**。`test.lua` 里改 `xxl_max_state`（棋子种类）或把 `xxl_move` 的 `duration` 从 200 调成 600，重新打一次资源就变了 —— demo 是 EXE 内嵌资源模式，但宿主 C++ 一行没动。
- **不是玩具量级**。1,621 行脚本、27 条无头测试、完整状态机与结算链，规模上已经能承载真实玩法。
- **有回归网**。`demos/fun_test/test_lua.cpp` 是它的专门测试套件，**27 条用例**（`soui_lua` 8 + `soui_lua_app` 19），占 `soui-headless` 门禁 400 条的 6.75%。几个做法值得提：纯 `IScriptModule` 接口驱动、零 lua 链接（测的是"脚本看到的 SOUI"而不是实现细节）；脚本末尾写 flag 文件供 C++ 检查（脚本中途报错则 flag 不出现）；泄漏与 UAF 在 MSVC Debug 0xDD 填充 + VLD 下核查。
- **文档是组件里最全的**。自带 3 篇（873 行）：导出技术与使用指南、模块升级与踩坑备忘、与 Qt-QML 的对比。
- **体积代价小**。组件自有源码 7,842 行，占 `components/` 85,082 行不到一成，按需链接。

**代价（这部分决定你要不要选它）：**

- **脚本逻辑跑在 UI 线程**。`SStringT` 是单线程类型（COW + 非原子引用计数），非 UI 线程触达 SOUI 字符串/控件就是数据竞争。密集计算必须走 C++ + 工作线程，脚本只做调度与呈现。
- **绑定面即能力边界**。Lua 只能调已注册的类与方法，调未绑定成员是**运行时报错而不是静默**（43 个文件、约 1,470 处注册）。绑定面会演变，但不是你随手加个方法就能用的。
- **生命周期红线要记住**。工厂产品必须显式 `:Release()`，包装对象绝不可 Release，同一指针禁止既托管又普通入栈 —— 判据看继承链，不是看"有没有叫 Release 的方法"。
- **默认参数要显式传满**。绑定层读不到默认值，缺参会被读成 `0`/空串。`ani:Start(...)` 的 BOOL 返回在 Lua 侧是数字，判失败要写 `ok == 0 or ok == false`，别用 `not ok`。
- **动画视觉无头测不到**，只能人工观察；app 未装 `LogMgr` 时 lua 错误文本会被静默丢弃。
- **组件经 COM 装载，依赖宿主**：要求 `SApplication` 已初始化，脱离宿主进程单独调会装载失败。

---

## 五、想验一下，三步

1. 克隆仓库，编出 `demos/demo`，打开脚本页的"消消乐"子页。两个小游戏（跑马机 + 消消乐）都在 `demos/demo/uires/lua/` 下。
2. 跑无头门禁（含 27 条脚本模块用例）：

```
ctest --test-dir build -L '^soui-headless$'
```


---

## 六、适合谁

如果你的痛点是：

- 玩法、原型、动画这类**高频改动**的东西，每次都拖着整个 C++ 宿主重编一遍；
- 希望**策划或同学能直接改逻辑**而不碰编译环境；
- 已经有一套 Windows 桌面代码和 Win32 知识，不想为了跨端重选语言；
- 想让脚本层从"回调胶水"升级成**能承载完整流程编排**（状态机、多段顺序链、并发特效、自定义轨迹）。

那这套值得花两天看看。

反过来，如果你的场景是帧时间极度敏感的渲染、或者团队完全没有 C++ 底子，那它现在还不是答案。

---

**SOUI4 项目主页：**

- Gitee：<https://gitee.com/setoutsoft/soui4>
- GitHub：<https://github.com/soui4/soui>
- 官网：<https://www.soui.com.cn>
- 在线教程：<https://soui.com.cn/doc>

### 脚注：本文数字与口径

- swinx 规模：44 个公开头 / 34,335 行头文件声明；自身实现 23,492 行（**不含第三方 vendored**）；六端指 Windows、Linux、macOS、iOS、Android、鸿蒙。
- API 覆盖：以 `swinx/doc/tools/api_scan.py` 扫描结果为准 —— 声明 1,042 个，`src` 内找到定义的 913 个（实现 884 / 半实现 6 / 桩 23），其余 129 个未见定义。
- 组件规模（7,842 行 / `components/` 85,082 行 / 3 篇文档 873 行 / 43 个导出文件约 1,470 处注册）来自 `components/ScriptModule-LUA` 当前源码统计，不含 vendored 的 Lua 5.4。
- `test.lua` 1,621 行（消消乐部分自第 236 行起，约 1,386 行）、`test_lua.cpp` 27 条用例（占 400 条门禁 6.75%）为当前基线实测值。
- 行号（`demo.cpp:107`、`uires.idx:173`、`dlg_main.xml:23`、`Swnd.cpp:4005/4010`、`SEventSet.cpp:201`、`luaFunSlot.h:17`、`page_script.xml:42/48/55/:82`）均逐处回源码核对；改动代码后需重新核对。
- 无头门禁条数口径为 `ctest -L '^soui-headless$'` 的用例总数。
