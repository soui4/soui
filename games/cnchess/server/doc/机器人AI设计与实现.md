# cnchess 机器人 AI：设计与实现

本文档详细说明 cnchess 项目的机器人（AI 对手）逻辑，覆盖从**接入方式**（服务器如何调度、如何与游戏主线程协作）到**算法实现细节**（搜索引擎、评估函数、置换表、开局库等）的完整链路。

相关代码位置：

- 引擎：`games/cnchess/algorithm/ChessAI.{h,cpp}`、`games/cnchess/algorithm/ChessBook.{h,cpp}`
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

---

## 2. 接入（服务器侧）

### 2.1 机器人标记与等级

```cpp
void CCnChess::SetupRobot(int seatId, int nLevel); // 标记某座位为机器人并记录等级
bool CCnChess::HasRobot() const;                   // 是否对局中存在机器人
```

- 等级来自客户端坐席时选择的按钮（`btn_lvl_beginner/medium/advanced`，见客户端 `LobbyHandler/EndgameHandler`）。
- 等级 → 搜索深度映射在 [ChessAI.h](`ROBOT_AI_DEPTH_*`)：

| 等级 | 值 | 搜索深度 |
|------|----|---------|
| 初级 ROBOT_LEVEL_BEGINNER | 1 | 4 |
| 中级 ROBOT_LEVEL_MEDIUM   | 2 | 5 |
| 高级 ROBOT_LEVEL_ADVANCED | 3 | 6 |

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

核心入口为 `CChessAI::SearchBestMove(layout, depth, timeMs)`，由三个层面构成：

### 3.1 迭代加深（Iterative Deepening）

```
for (d = 1; d <= depth; d++):
    用上一层的最佳着法把根着法列表提到最前（候选打头 → 剪枝更快）
    SearchRoot(根)  // 对每个合法着法做一次 alpha-beta
    若时间预算耗尽(g_time.stop) → break，放弃本层，采用上一完整层结果
随机取最终层的一组等优着法之一（增加棋风变化）
```

收益：

- 浅层结果**预热置换表**与**杀手着法表**，让深层搜索获得更好的着法排序与剪枝；
- 为**时间预算**提供"随时可取上一层的较优解"的回退能力。

### 3.2 负极大 + Alpha-Beta 剪枝（Negamax）

`Negamax(layout, depth, alpha, beta, ply)` 返回**相对当前行棋方**的估值（负极大形式）。

- `depth <= 0` 时转入**静态搜索** `Quiesce`。
- 无合法着法 → 返回 `-(CHECKMATE - ply)`（将死/困毙，距离随搜索深度递减，让 AI 倾向较快的将死解）。
- **着法排序**（剪枝效率的核心）：`置换表最佳着法 > 吃子(按被吃子力) > 杀手着法 > 其它`。
- 剪枝触发（`alpha >= beta`）时若该着为**安静着法**（非吃子），记为同层杀手着法。
- 结点边界：基于 `origAlpha` 与 `beta` 判定 `EXACT / LOWER / UPPER` 并写入置换表。

### 3.3 静态搜索（Quiescence）

在搜索深度用尽后，为避免"水平线效应"（对手紧接着的吃子/反吃子连锁被误判），只延展**吃子着法**：

1. 先对当前局面做静态评估（stand-pat），若已 `≥ beta` 直接返回；
2. 仅取吃子着法，按被吃子力降序；
3. 对每个吃子递归 `-Quiesce(layout, -beta, -alpha, ply+1)`；
4. 更新 `alpha`，`≥ beta` 即剪枝。

---

## 4. 评估函数 `Evaluate`

总评 = 我方各子力价值 + 位置分 − 对方相应之和（正值为当前行棋方占优）。

### 4.1 子力价值 `KR_VAL[7]`

| 棋子 | 将 | 车 | 马 | 炮 | 士 | 相 | 兵 |
|------|----|----|----|----|----|----|----|
| 价值 | 10000 | 900 | 400 | 450 | 200 | 200 | 100 |

### 4.2 位置评分表（PSQT）

为弥补纯子力对"站位与子力配合"感知的不足，为每类棋子配置 `10×9` 位置评分表，以"**本方王宫在第 0 行、向对方推进方向 r=9**"的规范坐标系设计（黑方做垂直镜像 `tr = 9 - y`）。

- 车 `kJuPos`：鼓励占据/沿第 4 行通路，底线两头略低；
- 马 `kMaPos`：偏向中心，避免边角；
- 炮 `kPaoPos`：开局偏后（如中炮/自家人马未动时不急于推进），中盘前压；
- 将 `kJiangPos`：鼓励镇中与补士安全，避免暴露；
- 士 `kShiPos`：鼓励归位守宫；
- 相 `kXiangPos`：鼓励回到各相位；
- 兵 `kBingPos`：过河（r≥5）后价值陡增，越深入越高。

### 4.3 兵过河加分

在位置表之外，兵过河额外加分增强其战斗价值（红：`y>=5 +40`，`y>=3 +(y-2)*10`；黑镜像）。

---

## 5. 置换表（Transposition Table）+ Zobrist 哈希

- 大小 `1<<16 = 65536` 项，Zobrist 键（固定种子 LCG 生成，`call_once` 初始化）。
- 键 = 全棋盘棋子异或 + 黑方行棋时再异或 `g_zobSide`（区分行棋方）。
- 命中条件：`e.key == key` 且 `e.depth >= 需要的深度`；按 `EXACT / LOWER / UPPER` 三种边界决定是否截断。
- 命中时给出**最佳着法**，用于着法排序置顶。
- **将死/困毙分值随 ply 变化**，不写入 TT，避免跨深度复用导致距离失真（以 `best ∈ (-(CHECKMATE-200), (CHECKMATE-200))` 过滤）。
- **线程安全**：机器人并发走子，共享表用 `std::mutex` 保护读写。

---

## 6. 杀手着法（Killer Moves）

- 表 `g_killers[64][2]`，每层记录两个**导致 beta 剪枝的安静着法**（出车、进马等在非吃子情况下却关键着法的启发）。
- 在着法排序中位于"吃子之后、其它之前"，进一步提高剪枝率。
- **thread_local 隔离**：每次搜索在各自线程内进行，既避免跨线程竞争，也保证不同对局的杀手学习互不干扰。

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

- `GenerateLegalMoves`：仅收集**己方棋子**的合法落点；模拟走子后再用 `IsJiangInCheck` 校验**不把自己送进将军**；**禁止吃掉对方将**（吃将/飞将不构成着法，将死由"无合法着法"判定），避免出现无效将位置（-1）引发的崩溃。
- `IsSquareAttacked` 覆盖车/将对面/炮的直线扫描、马的骑士步（含蹩马腿）、兵的向前+过河后左右。
- 无合法着法的行棋方即判负（将死/困毙），同时由 `CChsLytState` 在服务器判胜/判和路径中兜底。

---

## 9. 时间预算与并发安全汇总

| 方面 | 机制 |
|------|------|
| 思考时间 | `thread_local STimeCtrl`（`deadlineMs`、`stop`、`nodeCount`）；`CheckTime()` 每 256 个节点看一次 `steady_clock` 降低开销；到点置 `stop`，Negamax/Quiesce 即时 `return 0` 快速放弃，`SearchRoot` 丢弃未完成迭代 |
| 线程隔离 | 时间控制、杀手着法均 `thread_local`；置换表全局但互斥量保护 |
| 快照隔离 | 线程池只操作 `task.layout` 深拷贝，绝不触碰实时 `m_layout` |
| 串行落子 | 结果经 `postServiceTask` 回到主线程，`ApplyRobotMove` 串行应用并校验剧本代 |
| 过期结果 | `generation`（`m_nChessMsg`）不匹配即丢弃 |

---

## 10. 如何调整与扩展

- **提棋力**：增大各级 `robot_ai_medium/advanced_time_ms` 或提升 `ROBOT_AI_DEPTH_*`；给高级"只按时间、不设深度硬上限"时，可在 `RobotMakeMove` 为高级传入足够大的 `nDepth`，让迭代加深由时间自然主导。
- **补充开局库**：在 `ChessBook.cpp` 的 `kLineA..H` 追加新变例，保证坐标与合法性正确（以"红在上"坐标系书写），构建期会自动校验并注册。
- **细化预算**：`PropBag` 的预算字段是 `[3]` 数组，若想加"很弱"等更多档位，同步扩展数组与 `ROBOT_*` 宏即可。
- **进一步优化**：可考虑“历史启发（History）”“约束搜索/迭代加深结合时间动态调层”等，但这些**不属于当前必需**，需在确保正确性与并发安全的前提下引入。