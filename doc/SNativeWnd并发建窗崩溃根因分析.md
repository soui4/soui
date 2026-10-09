# SNativeWnd 并发建窗偶发崩溃根因分析

## 1. 背景与现象

- **工程**：isserver3（输入法后台带 UI 服务），通过 soui4 的 SIpcObject 模块基于窗口消息为 sinstar3_ime / sinstar3_tsf 提供 UI 支持；每个输入法链接会创建独立的 UI 线程。
- **崩溃点**：soui4（5.3 版，源码位于 `D:\work\soui4`）的 `SNativeWnd::WindowProc` 第 291 行断言
  `SASSERT(pThis->m_pCurrentMsg == &msg)`。
- **触发链**：291 行断言 → 断言 MessageBox 进入模态循环 → 期间 SWindow 收到跨线程消息 → `SWindow::TestMainThread` 断言（int 3 中断）。
- **复现概率**：原版本启动 **3-5 次即可复现**（偶发）；修改后重试 **10 次未复现**。

## 2. 根因

### 2.1 原机制：用全局共享指针传递 this

原版 `SNativeWnd::CreateNative` 的建窗流程：

1. `LockSharePtr(this)`：`m_cs.Enter()` 后把对象指针写入 `SNativeWndHelper` 全局单例的 `m_sharePtr`；
2. `CreateWindowEx` 同步分发 `WM_NCCREATE`；
3. `StartWindowProc` **无锁读取** `GetSharePtr()`（inline 直读 `m_sharePtr`）取回对象，初始化 thunk 并把对象指针注入新窗口的 wndproc；
4. `CreateWindowEx` 返回后 `UnlockSharePtr()`。

### 2.2 根本原因：一次 Enter 匹配了两次 Leave

原版代码里 `UnlockSharePtr()` 被调用了**两次**——`StartWindowProc` 内一次（`CreateWindowEx` 分发 `WM_NCCREATE` 期间），`CreateNative` 在 `CreateWindowEx` 返回后又一次：

```cpp
// CreateNative                                        // StartWindowProc（CreateWindowEx 同步分发时执行）
LockSharePtr(this);   // Enter，计数 1                {
    ...                                                   SNativeWnd *pThis = GetSharePtr(); // 无锁读
CreateWindowEx(...);  // 分发 WM_NCCREATE                  UnlockSharePtr();  // Leave，计数 0（提前释放）
    ...                                               }
UnlockSharePtr();     // Leave，计数 -1（非拥有者 Leave，UB）
```

即每个建窗周期 **Enter 1 次、Leave 2 次**，临界区递归计数每次净减 1。第二次 Leave 由非拥有线程执行，属未定义行为——Windows 实现不校验调用者是否拥有锁，直接递减内部计数，锁状态逐步损坏。

**若 Enter/Leave 严格配对（Leave 只留在 `CreateWindowEx` 返回后），共享指针方案在并发下也是正确的**：

- `CreateWindowEx` 是同步分发，`StartWindowProc` 必然在锁持有期间执行，无锁读 `GetSharePtr()` 读到的一定是当前线程刚写入的对象；
- 嵌套建窗（同线程在 `WM_NCCREATE` 处理中再建子窗口）命中 CRITICAL_SECTION 的同线程重入，计数对称配对；子窗口在重入后的锁内读到子对象，父窗口的读取发生在重入之前，互不干扰。

因此**真正的根因是锁配对错乱**，而非"共享指针并发写"本身。

### 2.3 锁损坏后的后果：互斥失效 → 串对象

临界区计数被反复错乱后（多建几个窗口即可），内部"拥有者 + 递归计数"状态被破坏，可能出现**两个线程同时判定锁空闲并都 Enter 成功**——互斥真正失效，`m_sharePtr` 才会被两个线程同时读写、互相覆盖。即便在锁损坏之前，`StartWindowProc` 的无锁读也完全依赖"读发生在锁内"这一前提，一旦锁提前释放或损坏，读到的就可能是其他线程写入的陈旧值。

windbg 转储证实了串对象：线程 14 新建窗口的 thunk 指向了线程 12 的对象 `0x056e80a4`——即线程 14 的 `StartWindowProc` 读到了线程 12 写入的 `m_sharePtr`。结果窗口 A 的 thunk 被绑定到对象 B，该窗口所有消息直接经 thunk 进入 `WindowProc`（第一参数被替换为对象 B 的指针），与线程 B 真正操作的窗口共用同一个对象实例。

### 2.4 崩溃链

`m_pCurrentMsg` 是**对象级**状态，其设计前提是"一个窗口对象同一时刻只由创建它的一个线程处理消息"。并发建窗把两个线程的消息塞进同一个对象后，该前提被破坏：

1. 线程 1 把消息写入对象 B 的 `m_pCurrentMsg`；
2. 线程 2 也在处理同一对象 B 的消息，再次覆盖 `m_pCurrentMsg`；
3. 线程 1 处理完返回时校验 `m_pCurrentMsg == &msg` 失败 → 291 行断言；
4. Debug 断言弹窗为模态 MessageBox，进入其消息循环后 SWindow 收到跨线程消息 → `TestMainThread` 断言 → int 3。

### 2.5 为什么是偶发

崩溃概率由两个叠加因素决定：

1. 锁未损坏时，只有两个线程恰好在同一时段重叠建窗才会踩中读窗口（竞争窗口极小）；
2. 锁损坏是**累积**的（每次建窗计数净减 1），启动/运行次数越多损坏越重，互斥失效后概率急剧上升。

启动初期多链接并发初始化时竞争概率最高，故表现为"3-5 次必现、平时偶发"。

## 3. 修复方案（根治）

### 3.1 SNativeWnd 修改（`D:\work\soui4\SOUI\src\core\SNativeWnd.cpp`）

改用线程局部存储（TLS，实现见 `D:\work\soui4\utilities\include\helper\STls.h`）在建窗同步分发期间传递对象指针：

- 新增静态槽位 `STlsId SNativeWnd::s_tlsWndCreate = 0;`（`STlsId` 为 `volatile LONG`，首次使用懒分配，线程安全）；
- `CreateNative` 在 `CreateWindowEx` 前把 `this` 写入 TLS 槽，创建返回后立即清空：

  ```cpp
  STls::Set(&s_tlsWndCreate, this, NULL);
  HWND hWnd = ::CreateWindowEx(dwExStyle, /* 窗口类 */, lpWindowName, dwStyle,
      x, y, nWidth, nHeight, hWndParent, (HMENU)(UINT_PTR)nID,
      SNativeWndHelper::instance()->GetAppInstance(), lpParam); // lpParam 原样传递
  STls::Set(&s_tlsWndCreate, NULL, NULL);
  ```

- `StartWindowProc` 对**任意**首个消息从 TLS 取回对象（TLS 在 `CreateWindowEx` 同步分发期间对当前线程始终可用），初始化 thunk 并绑定 wndproc；TLS 为 NULL（窗口非经 `CreateNative` 创建）时走 `DefWindowProc` 兜底：
- **彻底删除** `SNativeWndHelper::m_sharePtr` 及 `LockSharePtr / UnlockSharePtr / GetSharePtr` 全部实现。

要点：对象指针存于线程局部存储，各线程读自己的槽，互不覆盖，从根上消除跨线程注入。

#### 为什么不用 lpCreateParams 方案

初版曾尝试用 `struct LPARAMWRAP { SNativeWnd *pThis; LPVOID pParam; }` 经 `CreateWindowEx` 的 `lpCreateParams` 传递，存在一个不足：**顶层窗口创建时第一个到达的消息是 `WM_GETMINMAXINFO` 而非 `WM_NCCREATE`**，此时 `CREATESTRUCT::lpCreateParams` 尚不可用，无法取回对象，只能 `DefWindowProc` 兜底，导致 `WM_GETMINMAXINFO` 得不到 `SNativeWnd` 的处理。TLS 对任何首个消息都可用，且不占用/不干扰调用方 `lpParam`，故最终采用 TLS。

#### 为什么保持 TLS 而非退回"修正配对"的共享指针

修正 Enter/Leave 配对后，共享指针方案并发下虽正确，但每次建窗仍含一次全局临界区 Enter/Leave，多线程并发建窗时该临界区成为争用热点（锁竞争）。TLS 方案（`STls::Set/Get`）是线程局部读写，**全程无锁**——仅首次懒分配槽位时有一次全局同步，之后建窗不再有任何锁操作，无锁竞争、性能更好；同时消除了"所有创建路径必须经 `CreateNative`"的全局隐式前提，任何线程、任何时机都可安全取回对象。


## 4. 验证

- VS2008 Debug 编译通过（soui4.dll / soui4.lib）；TLS 改造后再次编译通过（exit 0）；
- 用户实测：新版本连续启动 **10 次未复现**崩溃；原版本 3-5 次即可复现。

## 5. 经验教训

1. **临界区 Enter/Leave 必须严格配对**。一次 Enter 配两次 Leave（尤其由非拥有线程 Leave）属未定义行为，锁计数净漂移、最终互斥失效，表现为难以定位的偶发崩溃。
2. **共享可变状态 + 无锁读 + 多线程 = 不可用的对象传递通道**。窗口创建期的对象传递应走线程隔离或 per-window 通道（线程局部存储 TLS / 结构体 `lCustData`），而不是进程级共享状态——共享指针方案即便锁配对正确，仍隐含"所有创建路径必须经 `CreateNative`"的全局前提，过于脆弱。
3. 窗口对象级成员（如 `m_pCurrentMsg`）隐含"单线程处理"假设；并发场景下必须保证对象与线程的绑定关系唯一，否则表现为难以定位的偶发断言。
4. 修复窗口创建机制后，需排查所有依赖旧机制的调用方（如对话框 hook），避免留下静默失效的代码路径。

## 6. 涉及文件

- `D:\work\soui4\SOUI\src\core\SNativeWnd.cpp`：`CreateNative` / `StartWindowProc` / TLS 槽 `s_tlsWndCreate`
- `D:\work\soui4\SOUI\include\core\SNativeWnd.h`：删除共享指针 API、新增 `s_tlsWndCreate` 声明与 `STls.h` 引用
- `D:\work\soui4\utilities\include\helper\STls.h`：TLS 工具实现（懒分配槽位）
