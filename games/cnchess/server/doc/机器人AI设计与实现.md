# cnchess 机器人 AI：设计与实现

本文档详细说明 cnchess 项目的机器人（AI 对手）逻辑，覆盖从**接入方式**（服务器如何调度、如何与游戏主线程协作）到**算法实现细节**（搜索引擎、评估函数、置换表、开局库等）的完整链路。

> **2026-09 更新**：搜索引擎已整体替换为开源项目 [jie65535/ChineseChess](https://github.com/jie65535/ChineseChess)（MIT License, Copyright (c) 2021）C# 引擎的 C++ 移植版（`ChsAIEngine.{h,cpp}`），算法升级为 **PVS + 空着裁剪 + 将军延伸 + 置换表 + 杀手/历史启发 + 静态搜索**。对外接口 `CChessAI::SearchBestMove` 与服务器调度链路保持不变。

相关代码位置：

- 引擎：`games/cnchess/algorithm/ChsAIEngine.{h,cpp}`（搜索算法本体）、`games/cnchess/algorithm/ChessAI.{h,cpp}`（适配层）、`games/cnchess/algorithm/ChessBook.{h,cpp}`（开局库）
- 线程池：`games/cnchess/server/RobotAIPool.{h,cpp}`、`games/cnchess/server/RobotDispatch.h`
- 游戏流程：`games/cnchess/server/CnChess.{h,cpp}`、`games/cnchess/server/WebSocketGame.cpp`
- 配置：`games/cnchess/server/PropBag.{h,cpp}`、`games/cnchess/server/config/config.xml`

---

## 1. 总体架构

机器人走子采用"**搜索在线程池、落子在主线程**"的模式分离：

```
┌───────────────────────── 游戏主线程 (LWS 线程) ─────────────────────────┐
│  TriggerIfRobotTurn()                                                  │
│        │  轮到机器人                                        │  返回结果  │
│        ▼                                                     ▲         │
│  RobotMakeMove(seatId)                                      │         │
│        1. 算出搜索深度 depth 与时间预算 timeMs               │         │
│        2. 深拷贝棋盘快照 task.layout.Copy(&m_layout)         │         │
│        │  Dispatch(SRobotTask)                              │         │
└────────┼────────────────────────────────────────────────────┼─────────┘
         ▼                                                    ▲
┌──────────────── 机器人线程池 CRobotAIPool (1..16 线程) ──────┤─────────┐
│  RunTask(task)   ← 派发到任务最少的 ITaskLoop               │         │
│    best = CChessAI::SearchBestMove(layout, depth, timeMs)   │         │
│    service(λ{ apply(tableId,seatId,generation,best) })  ────┼─────────┘
│        │ （经 SOUI 服务队列回到主线程）                              │
└────────┼──────────────────────────────────────────────────────────────┘
         ▼
  ApplyRobotMove(best, seatId, generation)
    校验 状态/当前走棋方/代数 → 落子 → 广播 MSG_REQ_MOVE → TriggerIfRobotTurn()
```

**关键点**：线程池线程**只对棋盘快照做纯计算**，绝不触碰实时游戏状态；计算结果通过 SOUI 的 `postServiceTask` 服务队列**串行**回到游戏主线程去落子，从而保证逻辑一致性，同时让耗时的搜索不阻塞其他玩家的对局。

`CChessAI::SearchBestMove` 内部分两层：**适配层**（`ChessAI.cpp`，负责坐标映射与结果转换）+ **搜索引擎**（`ChsAIEngine.cpp`，纯算法，只依赖标准 C++ 头，MSVC / MinGW / NDK / OHOS clang 均可编译）。

---

## 2. 接入（服务器侧）

### 2.1 机器人标记与等级

```cpp
void CCnChess::SetupRobot(int seatId, int nLevel); // 标记某座位为机器人并记录等级
bool CCnChess::HasRobot() const;                   // 是否对局中存在机器人
```

- 等级来自客户端坐席时选择的按钮（`btn_lvl_beginner/medium/advanced`，见客户端 `LobbyHandler/EndgameHandler`）。
- 等级 → 搜索深度映射在 `ChessAI.h`（`ROBOT_AI_DEPTH_*`）：

| 等级 | 值 | 搜索深度 |
|------|----|---------|
| 初级 ROBOT_LEVEL_BEGINNER | 1 | 3 |
| 中级 ROBOT_LEVEL_MEDIUM   | 2 | 5 |
| 高级 ROBOT_LEVEL_ADVANCED | 3 | 7 |

### 2.2 触发：轮到机器人走棋

每次对局落子/开局后都会调用 `TriggerIfRobotTurn()`：若当前活跃座位是机器人且对局进行中，则调用 `RobotMakeMove(seatId)`。

`RobotMakeMove()` 负责**组装一个搜索任务**：

```cpp
int nDepth = CChessAI::LevelToDepth(m_nRobotLevel[seatId]);               // 深度上限
task.depth   = nDepth;
task.timeMs  = PropBag->GetRobotAITimeMs(m_nRobotLevel[seatId]);          // 时间预算
task.generation = m_nChessMsg;                                            // 用于丢弃过期结果
task.layout.Copy(&m_layout);   // 深拷贝棋盘快照，线程池只操作副本
CRobotAIPool::getSingletonPtr()->Dispatch(task);
```

### 2.3 任务结构 `SRobotTask`

```cpp
struct SRobotTask
{
    int        tableId;    // 回表定位用
    int        seatId;     // 机器人座位
    int        depth;      // 搜索深度上限（由等级决定）
    int        generation; // 派发时的走棋序号，用于丢弃过期结果
    int        timeMs;     // 思考时间预算（毫秒，0=只用固定深度）
    CChessLayout layout;   // 棋盘深拷贝快照（只有快照，不共享实时状态）
};
```

### 2.4 线程池 `CRobotAIPool`

基于 SOUI `ITaskLoop`（每线程一条任务队列）实现，单例。

- **初始化** `Init(nThreads)`：线程数夹在 `[1,16]`，幂等（已初始化则直接返回）。线程名为 `RobotAI-0..N`。
- **负载均衡派发** `Dispatch(task)`：选择当前排队任务**最少**的 `ITaskLoop` 派发副本，均衡各线程负载。
- **计算** `RunTask(task)`：在线程池线程调用
  `CChessAI::SearchBestMove(task.layout, task.depth, task.timeMs)`，
  然后通过 `service` 回调把结果投递回主线程。
- **回主线程**：`SetSink(serviceSink, applySink)` 在启动时由 `WebSocketGame` 接线：
  - `serviceSink` 用 `StdRunnable` 包一层后 `postServiceTask` 投递到 LWS 主线程；
  - `applySink` 定位 `tableId` 对应棋盘并调用 `pChess->ApplyRobotMove(best, seatId, generation)`。
- **关停** `Shutdown()`：交换出队列、清空回调、`stop()` 所有任务循环。须在 `m_stopRequested` 置位后由主线程调用，避免阻塞信号处理上下文。（见项目约束：信号处理器只置标志，不执行阻塞关停。）

### 2.5 回到主线程落子 `ApplyRobotMove`

在主线程串行执行，逐项防御：

1. 对局仍在 `PLAYING`、当前活跃座位仍是该机器人、仍是机器人；
2. `generation == m_nChessMsg`，否则说明状态已推进（悔棋/新局），**丢弃过期结果**；
3. 若 `best.pt1.x < 0`（无合法着法）→ 判负广播；
4. 否则 `m_layout.Move(best.pt1, best.pt2)`，累计时间、记录对局，广播 `MSG_REQ_MOVE` 给所有玩家（机器人无连接不会收到）；
5. 更新未吃子步数等判和/判负条件；
6. 再次 `TriggerIfRobotTurn()` 判定是否继续轮到机器人。

### 2.6 配置项

`config.xml` 的 `<prop ...>`：

| 键 | 默认 | 含义 |
|----|------|------|
| `robot_ai_pool` | 4 | 机器人线程池线程数（夹在 [1,16]） |
| `robot_ai_begin_time_ms` | 400 | 初级思考时间预算（毫秒） |
| `robot_ai_medium_time_ms` | 800 | 中级思考时间预算（毫秒） |
| `robot_ai_advanced_time_ms` | 1500 | 高级思考时间预算（毫秒） |

任一级预算设 `0` 表示**不回退到固定深度**（该等级只用深度上限，不做时间截断）。深度上限与时间预算共同决定棋力：深度是"顶"（最大搜索层数），时间是"闸"（到点即停，采纳当前已完成最深层的结果）。

---

## 3. 搜索引擎

核心入口 `CChessAI::SearchBestMove(layout, depth, timeMs)` 分为**适配层**与**搜索引擎**两层。

### 3.1 适配层（`ChessAI.cpp`）

适配层对搜索引擎屏蔽 `CChessLayout` 的坐标系细节，公开接口与旧版完全一致：

1. **开局库探测**：先调 `CChessOpeningBook::Probe(layout, book1, book2)`，命中直接返回开局着法（见第 7 节）。
2. **朝向判定 `DetectRedTop`**：搜索引擎内部恒用"红方宫在低行（row 0..2）、红兵向高行进攻"的规范坐标系；`CChessLayout` 经 `InitLayout/ConvertLayout` 后红方可能在高 y 侧，故按**红帅实际位置**自动判定——红帅在低 y 侧直接映射 `row=y`，否则垂直翻转 `row=9-y`；无红帅（残局构造场景）按黑将位置反推。**列方向不翻转**（棋规左右对称，恒取 `col=x`）。
3. **棋盘转换** `LayoutToSearchBoard`：`m_chesses[y][x]` 的 `CHESSMAN` 枚举 → 90 格一维 `int8_t`（正红负黑，类型 1..7 = 将士象马车炮卒），同时记录红帅/黑将所在格、行棋方，并从头计算 Zobrist 键。搜索全程在快照上运行，不修改调用方的 layout。
4. **根节点合法着法普查**：`MoveGenerator::Generate` 全量生成后逐个 `Make → IsSideInCheck → Unmake` 数合法着法；**0 个合法**（将死/困毙/无子）直接返回无效走法 `pt1=pt2={-1,-1}`，上层 `ApplyRobotMove` 据此判负。
5. **结果回转** `SearchMoveToStep`：压缩着法编码（起点高 8 位 + 终点低 8 位）还原为 `MOVESTEP`；`enemy/nEnemyID` 取自走子前的盘面快照。
6. **兜底保证**：若时间预算过小导致引擎连第 1 层都没算完（`BestMove==0`），不计时间补算 1 层（`Search(sb, 1, 0)`），**保证永远返回合法着法**。

> ⚠️ **严格采用引擎最佳着法（不要做根节点"等分随机挑选"）**：根节点子搜窗口随 alpha 收窄后，fail-high 返回的边界值会与最佳分"伪同分"——实测自对弈中 64% 的等分候选为伪等分，真实分值可差数百，随机/优先吃子挑选会系统性劣化棋力。此行为与 C# 原版一致。

### 3.2 迭代加深（Iterative Deepening）

```
for (d = 1; d <= maxDepth; d++):
    iterBest = 上一层的 bestMove（经根节点着法排序置顶 → 剪枝更快）
    score = AlphaBetaRoot(d, iterBest)
    若本层被时间硬停(m_stop) → 本层结果作废，break，采用上一个完整层结果
    记录 bestMove/bestScore/completedDepth
    若已见绝杀（|score| >= MateInMaxPly）→ 提前结束
    若耗时超过预算的 60%（软停）→ break，避免下一层吞超
```

收益：浅层结果**预热置换表**与**杀手/历史表**，让深层搜索获得更好的着法排序与剪枝；同时为时间预算提供"随时可取上一层完整解"的回退能力。每次 `Search` 开启置换表**新代次**并清零杀手/历史表。

### 3.3 PVS（Principal Variation Search + Alpha-Beta）

`AlphaBeta(depth, alpha, beta, ply, allowNull)` 返回相对当前行棋方的估值：

- **将军延伸**：当前行棋方被将军时 `depth++`（将军链不计入深度预算）；
- **ply 上限保护**：`ply >= MaxPly-2`（64 层）强制转入静态搜索，防止延伸链导致深度不再递减而栈溢出/杀手表越界；
- **置换表截断**：命中且 `ttDepth >= depth` 时，按 `EXACT / LOWER(≥beta) / UPPER(≤alpha)` 三种边界直接返回（见第 5 节）；
- **空着裁剪（Null-Move Pruning）**：`allowNull && 不被将军 && depth>=3 && 己方仍有车马炮卒大子 && beta 不是绝杀边界` 时，走一步空着并以 `R = depth>=6 ? 3 : 2` 的减层做 null-window 验证，`nullScore >= beta` 即剪枝（null 搜索返回的绝杀分钳制到 beta 防止假杀分传播）；
- **PVS 搜索窗**：第一个合法着法全窗 `(-beta,-alpha)`；后续先以 null-window `(-alpha-1,-alpha)` 试探，`alpha < score < beta` 时才重搜全窗——对排序良好的着法序列可省去大量全窗搜索；
- **剪枝反馈**：`alpha >= beta` 且导致剪枝的是**安静着法**（非吃子）时，记入杀手表并按 `depth*depth` 加权累计历史启发（见第 6 节）；
- 结点边界：基于本结点是否提升过 alpha 判定 `TT_EXACT / TT_LOWER / TT_UPPER` 写入置换表；
- **无合法着法** → 返回 `-(MateValue - ply)`（将死/困毙，中国象棋无和棋判定），分值随 ply 递减使引擎倾向**较快**的将死解。

### 3.4 静态搜索（Quiescence）

在搜索深度用尽（或 ply 达上限）后，为避免"水平线效应"，只延展**吃子着法**（fail-hard 风格）：

1. 先做静态评估（stand-pat），`>= beta` 直接返回 `beta`，否则把 `alpha` 提升到 stand-pat；
2. 仅生成吃子着法，按 **MVV-LVA**（被吃子价值×16 − 攻方价值）降序；
3. 每个吃子 `Make` 后同样用 `IsSideInCheck` 排除自将着法，递归 `-Quiescence(-beta,-alpha)`；
4. `score >= beta` 返回 `beta`，否则提升 `alpha`。

### 3.5 着法排序（剪枝效率的核心）

`OrderMoves` 为每个着法打分后插入排序（着法数通常 30~80，插入排序足够）：

| 优先级 | 类别 | 分值 |
|--------|------|------|
| 1 | 置换表最佳着法 | `10,000,000` |
| 2 | 吃子（MVV-LVA） | `1,000,000 + victim*16 − attacker` |
| 3 | 杀手着法[ply][0] | `900,000` |
| 4 | 杀手着法[ply][1] | `800,000` |
| 5 | 其它 | 历史启发累计值 |

---

## 4. 评估函数 `Evaluation::Evaluate`

全盘单遍扫描：`score = Σ ±(PieceValue[type] + kPSTByType[type][sq])`，黑方格子按行镜像 `MirrorSq`（`row → 9-row`）取红方视角的位置分，最后按行棋方取正负——**返回值相对当前行棋方**，越大越有利。

### 4.1 子力价值 `PieceValue[8]`

| 棋子 | 将/帅 | 士 | 象 | 马 | 车 | 炮 | 卒/兵 |
|------|-------|-----|-----|-----|-----|-----|-------|
| 价值 | 10000¹ | 120 | 120 | 270 | 600 | 285 | 30 |

¹ 将的分值仅参与绝杀量级计算（`MateValue=30000` 与之保持量级隔离），常规评估中双方将帅恒在。

### 4.2 分兵种位置表（PST）

每类棋子一张 90 项表（红方视角 `[row][col]`，黑方按行镜像），弥补纯子力对站位与配合的感知不足：

- **兵 `kPawnPST`**：起始线基本为 0，**过河（row 5）起大幅升值**（14~28），越深入越高，近九宫（row 8）达到峰值 32~58，敌方底线略回落（18~32）；
- **马 `kKnightPST`**：中心强（最高 34），被困边角弱（-4）；
- **车 `kRookPST`**：到处都强，向前推进与中路、肋道略优（最高 28），己方底线角落 -2；
- **炮 `kCannonPST`**：中路与宫顶炮位为正（如空头炮位 +16），边路与后退为负（-12~-14）；
- **将 `kKingPST`**：鼓励窝在底线中位（+18/+10），上宫顶线为负（-6/-8）；
- **士 `kMandarinPST`**：仅宫心格 +4，其余 0；
- **象 `kElephantPST`**：典型三七象位 +4/+6，其余 0。

> 与旧版差异：旧版"兵过河独立加分"逻辑已并入兵的 PST，不再有独立代码路径。

---

## 5. 置换表（Transposition Table）+ Zobrist 哈希

- **每线程独立一份**：`thread_local SearchEngine`（`ChessAI.cpp` 的 `GetEngine()`），构造参数 `ttSizeBits=18`，即 **2^18 = 262,144 条**；每条 `Entry` 16 字节（Key 8 + Score int16 2 + BestMove 2 + Depth/Flag/Generation/Pad 各 1），**约 4MB/线程**，16 线程上限约 64MB。同池线程连续走子时复用表内容，每次 `Search` 开启新代次。
- Zobrist 键在棋盘上**增量维护**（`Make/Unmake/MakeNullMove` 同步异或），转换快照时由 `ComputeZobristFromScratch` 从头计算一次。
- **替换策略（深度优先 + 代次老化）**：同 key 且已存深度更深且同代 → 保留；否则替换。`Generation` 字段让跨搜索的旧条目自然老化。
- **mate 分 ply 校正**：`|score| >= MateInMaxPly`（30000−1024=28976，达到该量级视为已见绝杀）的分数在 **Store 时 `±ply`、Probe 时 `∓ply`**，保证存储的是"距根的绝对距离"，取出时还原为"距当前结点的相对距离"——跨结点复用不会导致将死距离失真。
- 命中条件与截断：`ttDepth >= depth` 时按 `EXACT / LOWER(≥beta) / UPPER(≤alpha)` 返回；未达截断条件时仍给出 **最佳着法** 用于着法排序置顶。
- **线程安全**：表随引擎实例 thread_local，**无锁**——各池线程互不干扰。

---

## 6. 杀手着法（Killer Moves）与历史启发（History Heuristic）

- **杀手表** `m_killers[MaxPly][2]`：每 ply 记录两个**导致 beta 剪枝的安静着法**（出车、进马等在非吃子情况下却关键着法的启发），新着法挤入 [0]、原 [0] 顺移到 [1]（同着法不重复记）。排序分值仅次于吃子。
- **历史表** `m_history[14][90]`：按"被移棋子类型 × 落点格"累计 `depth*depth` 加权的剪枝次数——深层剪枝的着法获得远高于浅层的权重。排序时作为普通着法的分值。
- **生命周期**：两者均为引擎成员，每次 `Search` 开始时清零——启发知识只在单次搜索内有效，跨对局不携带（避免上一局的启发污染本局）。

---

## 7. 开局经验库（Opening Book）

### 7.1 结构

- `ChessBook.cpp` 内置 8 条常见开局变例：中炮对屏风马（缓开车/黑先进右马）、顺手炮、仙人指路、起马、过宫炮、反宫马、五七炮。以"红在上"坐标系（`kDefaultLayout`）存储。
- 构建期 `BuildBook()`（文件级静态 `BBoot`，进入 `main` 前单线程执行，无并发）：逐变例走子并**校验合法性**，不可行即截断该变例；对每个可达局面把推荐着法写入 `std::map<hash, vector<(p1,p2)>>`。

### 7.2 哈希规范化 `Hash()`

- FNV-1a 64 位哈希，遍历全棋盘（含空位占位）+ 行棋方。
- **垂直翻转规范化**：若红方将在下半区，则垂直翻转棋盘再哈希。这样红方在上/在下两种坐标系下，同一局面命中同一书条目。

### 7.3 探测 `Probe()`

- **开局阶段判定**：全场棋子数 `>= 26` 才查书（棋子明显减少即进入中/残局，交给搜索）。
- 命中则在该局面多条推荐着法中随机取一，并把规范化坐标还原为实际棋盘坐标返回。
- `SearchBestMove` 在搜索前先 `Probe`，命中即直接采用，**补充开局经验、避免开局浪费搜索**。

---

## 8. 着法合法性与搜索健壮性

- **伪合法生成 + Make 后过滤**：`MoveGenerator::Generate` 生成伪合法走法（**不预过滤自将**），搜索在 `Make` 之后用 `IsSideInCheck` 排除把己方将送入将军的着法——比生成期过滤更简单，且静态搜索/根普查共用同一套逻辑。`capturesOnly=true` 时仅生成吃子（静态搜索用）。
- **将帅不可被吃**：引擎不做"吃将"着法（将死由"无合法着法"判定）；`IsSquareAttacked` 覆盖车/将**对面飞将照面**/炮的直线扫描、马的骑士步（含蹩马腿）、兵的向前+过河后左右，将帅不在盘上视为被将。
- **无合法着法的行棋方即判负**（将死/困毙），返回 `-(MateValue - ply)`；适配层在根节点额外做一次合法普查，0 合法时返回无效走法，由服务器 `ApplyRobotMove` 发出判负广播。
- **栈/表边界**：`ply >= MaxPly-2` 强制转静态搜索；单节点着法缓冲 `MaxBuffer=128`（车最多 17 步，全盘 44 子实测远小于该值）。

---

## 9. 时间预算与并发安全汇总

| 方面 | 机制 |
|------|------|
| 计时基准 | `clock()` 进程 CPU 时间（不引 `<chrono>`，最小依赖；单线程阻塞搜索场景与墙钟等价） |
| 硬停（搜内） | `CheckTime()` 每 4096 个节点（`m_nodes & 4095 == 0`）查一次，到 100% 预算置 `m_stop`，AlphaBeta/Quiesce 即时 `return 0` 快速放弃，本层结果作废 |
| 软停（层间） | 每层完整算完后查一次，超过预算 **60%** 即停——下一层大概率吞超，不如保住已完成的层 |
| 兜底 | 预算过小连第 1 层都没算完时，不计时间补算 1 层，保证总返回合法着法 |
| 线程隔离 | 搜索引擎实例（含置换表、杀手/历史表）`thread_local`，**无锁**；各线程互不干扰 |
| 快照隔离 | 线程池只操作 `task.layout` 深拷贝，搜索又在转换出的 `SearchBoard` 快照上运行，绝不触碰实时 `m_layout` |
| 串行落子 | 结果经 `postServiceTask` 回到主线程，`ApplyRobotMove` 串行应用并校验剧本代 |
| 过期结果 | `generation`（`m_nChessMsg`）不匹配即丢弃 |

---

## 10. 移植说明与如何调整/扩展

### 10.1 移植来源与边界

- 引擎移植自 [jie65535/ChineseChess](https://github.com/jie65535/ChineseChess) 的 `ChineseChess.Core/AI` 模块（C#），**MIT License**，按许可要求在 `ChsAIEngine.h` 文件头保留版权与许可声明。
- 移植保持算法语义不变（PVS/空着裁剪/置换表替换策略/mate 分校正等均与原版一致）；两处 C++ 化差异：① 原版时间到时抛 `OperationCanceledException` 中断本层，C++ 版以 `m_stop` 标记等价实现；② 引擎按线程独立实例（原版单线程 UI 场景无此需求）。
- `ChsAIEngine.h/.cpp` 只依赖标准 C++ 头（`stdint.h`、`string.h`、`time.h`），**不依赖 stdafx/windows.h**，可在服务端与各移动端（NDK/OHOS clang）直接复用。

### 10.2 调整与扩展

- **提棋力**：增大各级 `robot_ai_medium/advanced_time_ms`（时间预算对 PVS 引擎的增益显著）或提升 `ROBOT_AI_DEPTH_*`；给高级"只按时间、不设深度硬上限"时，可在 `RobotMakeMove` 为高级传入足够大的 `nDepth`，让迭代加深由时间自然主导。
- **置换表容量**：`GetEngine()` 的 `SearchEngine(18)` 可调大（如 20 = 16MB/线程），内存换命中率；线程池越大越要留意总内存。
- **补充开局库**：在 `ChessBook.cpp` 的 `kLineA..H` 追加新变例，保证坐标与合法性正确（以"红在上"坐标系书写），构建期会自动校验并注册。
- **细化预算**：`PropBag` 的预算字段是 `[3]` 数组，若想加"很弱"等更多档位，同步扩展数组与 `ROBOT_*` 宏即可。
- **进一步优化方向**：TT 结点直接返回最佳着法以改善 PVS 截断质量、空着裁剪的自适应 R、残局专用评估/残局库（配合 `server/config/endgames.json` 残局玩法）、静态权重（PST/子力价值）调优。改动需在确保正确性与并发安全的前提下引入。
- ⚠️ **再次强调**：不要在根节点做"等分候选随机挑选"（历史教训，见 3.1 节末尾的警告）。
