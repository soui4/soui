# SOUI4 Android 原生控件适配 & 字符串槽（StringSlot）跨层传递机制 设计文档

## 1. 背景与目标

SOUI4 支持在 Android 平台把某些控件（如 `WC_EDIT` = `"Edit"`）**降级为系统原生控件**（Android `EditText`/`Button`）替换 DUI 自绘，从而获得系统级 IME、辅助功能（TalkBack）、Undo/Redo、光标/选择、剪贴板等能力，同时尽可能保持和 Windows 端同一套 Win32 消息（`WM_*` / `EM_*`）业务代码无需重写。

**架构边界**：C++ SOUI 业务层（swinx/SOUI4 标准窗口类）→ C++ 控件级 `Android_*.cpp` WndProc → C++ `AndroidPlatformAPI` 平台桥 → JNI → Java `INativeWindow`（Android `View` 子类，实现具体控件行为）。所有交互**以 Win32 消息为唯一契约**。

### 1.1 原先设计的问题

早期实现跨层字符串传递用了 **3 个 JNI 方法重载**：

- `long nativeSendMessage(int msg, long wParam, long lParam)` —— 纯数值消息
- `long nativeSendMessageStr(int msg, long wParam, String inStr)` —— 字符串入参
- `String nativeSendMessageStrOut(int msg, long wParam, long lParam)` —— 字符串出参

问题：
1. **接口冗余**：每加一种新消息类型需要加 1~3 个 JNI 重载；INativeWindow 表面积膨胀；
2. **C++ 调用方判断复杂**：需要根据消息号选择不同函数；参数语义分散；
3. **出参字符串签名污染**：Java 返回 `String` 无法同时返回 `LRESULT` 错误码 / 字符数，导致信息丢失；
4. **和 Win32 `SendMessage(Wparam, Lparam) : LRESULT` 模型严重不一致**，跨平台业务代码在 Android 侧需要额外 ifdef。

### 1.2 重构目标

- ✅ **统一单接口**：只保留 `nativeSendMessage(int msg, long wParam, long lParam) : long`，与 Win32 签名对齐；
- ✅ **字符串跨层通过槽（slot）交换**：参数和返回值全部是整数，槽里承载实际字符串；
- ✅ **C++ 提供 RAII**：`AutoStringSlot` 构造分配、析构释放，避免泄漏/UAF；
- ✅ **编码约定 SSOT**：槽编码（含 `EM_GETLINE` 特殊打包）只在 `AndroidPlatformAPI.h` + `INativeWindow.java` Javadoc 两处同步维护；不再依赖魔法位；
- ✅ **职责下沉**：控件相关槽编码/解码放在**控件 WndProc**（如 `Android_Edit.cpp`）里，`AndroidPlatformAPI.sendMessage` 只做通用转发；
- ✅ **Android 主线程保证**：Java 所有 View 操作强制切到主线程同步执行，避免 `CalledFromWrongThreadException`。

---

## 2. 总体架构

```
┌───────────────────────────────────────────────────────────────────┐
│  C++ SOUI 业务层（SEdit / SWindow / swinx SendMessage 标准实现）   │
└──────────────────────┬────────────────────────────────────────────┘
                       │ Win32 消息 (带 char* buf / LPCSTR 指针)
                       ▼
┌───────────────────────────────────────────────────────────────────┐
│  C++ 控件级 WndProc（Android_Edit.cpp / Android_*.cpp）           │
│  ◀ 这里发生"字符串 ↔ slotId"编码 / 解码 + buf 拷贝 ▶              │
│   • 入参：AutoStringSlot(pszText) → lParam = slot.id()            │
│   • 出参：AutoStringSlot(NULL)    → lParam = encode(slot.id(), M) │
│   • EM_GETLINE 特殊：lParam = (maxCch << 16) | slotId            │
└──────────────────────┬────────────────────────────────────────────┘
                       │ 纯整数：nativeSendMessage(msg, wp, lp)
                       ▼
┌───────────────────────────────────────────────────────────────────┐
│  C++ AndroidPlatformAPI (平台层)                                  │
│   • unordered_map<int, string> m_slotStrings  + mutex            │
│   • stringSlotAlloc / WriteString / ReadString / stringSlotFree  │
│   • sendMessage → 只做 JNI CallLongMethod(INativeWindow) 转发    │
└──────────────────────┬────────────────────────────────────────────┘
                       │ JNI: CallLongMethod(nativeSendMessage)
                       ▼
┌───────────────────────────────────────────────────────────────────┐
│  Java INativeWindow 实现（如 NativeEditView 继承 EditText）       │
│   • 入口：nativeSendMessage(msg, wp, lp)                          │
│   • runOnUiSync 切主线程同步执行 handleMessageImpl                │
│   • 根据 msg 查表：数值类直接处理；字符串类再走槽读写 JNI           │
│     (SouiPlatformBridge.nativeReadCxxStringSlot / Write)          │
│   • 调用 setText / setSelection / getLayout / scrollTo 等 View API│
│   • 返回 long LRESULT                                             │
└──────────────────────┬────────────────────────────────────────────┘
                       │ SouiPlatformBridge 静态 JNI
                       ▼
┌───────────────────────────────────────────────────────────────────┐
│  JNI Soui4Android.cpp：槽读写桥                                    │
│   Java_com_soui_SouiPlatformBridge_nativeReadCxxStringSlot        │
│   Java_com_soui_SouiPlatformBridge_nativeWriteCxxStringSlot       │
└───────────────────────────────────────────────────────────────────┘
```

---

## 3. 关键机制详解

### 3.1 字符串槽（StringSlot）C++ 实现

- **存储**：`std::unordered_map<int, std::string> m_slotStrings` + 独立 `std::mutex m_slotMtx`，不与窗口/屏幕等主锁竞争；
- **分配**：`std::atomic<int> m_slotNextId` 原子自增；
  - **快速路径**（99.9%）：`fetch_add(1)` 返回值落在 `[1, 65535]` → 立即 `insert_or_assign` 入槽；
  - **慢速 / wrap 路径**：id 超界或异常 → 持 `m_slotMtx` 串行化重置；重置起点固定为 `1`，**永远跳过 `id=0`**（Read/Write/Free 对 `id<=0` 直接短路返回，`id=0` 是无效值）；
  - 下一个 id 形成环形：`id==65535 → after=1`；
- **创建/更新一律用 `insert_or_assign`**：即使 id 已存在（wrap 前一轮未释放、或调用方对未知 id 直接 `WriteString`），也不会残留旧值；
- **释放**：`stringSlotFree(id)` → `m_slotStrings.erase(id)`；
- **RAII**：`AutoStringSlot`（头文件内联类）
  - 构造：`stringSlotAlloc(pszStr)` 拿到 id；`pszStr == NULL` 内部安全转 `""`；
  - 析构（`noexcept`）：若 id>0 则 `stringSlotFree(id)` 并清零；
  - 辅助方法：`id()` / `explicit operator int()` / `read()` / `write(std::string)`；
  - **生命周期 = 单次消息往返周期**：出作用域即释放，不会被 Java 继续引用（Java 已在 `nativeSendMessage` 同步返回前完成 `nativeWriteCxxStringSlot`）。

### 3.2 SSOT 消息编码约定（Single Source of Truth）

> **同时维护在**：`AndroidPlatformAPI.h` 顶部注释 + `INativeWindow.java nativeSendMessage` Javadoc + NativeEditView 常量。

| 消息类别 | 代表消息 | wParam | lParam | 返回值（long/LRESULT） |
|----------|---------|--------|--------|----------------------|
| **纯数值类**（不涉及字符串） | `WM_GETTEXTLENGTH` / `EM_SETSEL` / `EM_GETSEL` / `EM_GETLINECOUNT` / `EM_SETMODIFY` / `EM_GETMODIFY` / `EM_CANUNDO` / `EM_UNDO` / `EM_EMPTYUNDOBUFFER` / `EM_LIMITTEXT` / `EM_GETLIMITTEXT` / `EM_SETREADONLY` / `EM_SETPASSWORDCHAR` / `EM_GETPASSWORDCHAR` / `EM_SETMARGINS` / `EM_GETMARGINS` / `EM_LINEINDEX` / `EM_LINELENGTH` / `EM_LINEFROMCHAR` / `EM_SCROLLCARET` | 按 Win32 原义整数 | 按 Win32 原义整数 | 按 Win32 语义；<br>`EM_GETSEL` = `((end & 0xFFFFFFFFL) << 32) | (start & 0xFFFFFFFFL)`，C++ WndProc 解码回 `*(LPDWORD)wParam=start / *(LPDWORD)lParam=end` |
| **字符串入参**（C++ → Java） | `WM_SETTEXT` / `EM_REPLACESEL` | `WM_SETTEXT`: unused / `EM_REPLACESEL`: `BOOL fCanUndo` | `lParam = inSlotId`（正整数 ∈[1..65535]；C++ 侧 `AutoStringSlot` 已把字符串写入槽；Java 调 `nativeReadCxxStringSlot(slotId)` 读取） | `WM_SETTEXT`: `TRUE(1)` / `FALSE`；<br>`EM_REPLACESEL`: `0` |
| **字符串出参**（Java → C++） | `WM_GETTEXT` | `wParam = cchTextMax`（含 NUL 终止符位） | `lParam = outSlotId`（C++ 侧已分配空槽；Java 计算结果后调 `nativeWriteCxxStringSlot(slotId, result)` 写回） | 返回**实际字符数**（不含 NUL，按 Win32）；C++ WndProc 再 `ReadString(slotId)` 拷贝到 buf + 补 NUL |
| **字符串出参（Win32 兼容打包）** | **`EM_GETLINE`**（对齐 Win32：buf 首 `WORD` = buf 最大字符数） | `wParam = lineNo`（0 基行号，Win32 原义） | **`lParam = ((long)maxCch << 16) | (slotId & 0xFFFFL)`**<br>解码：`slotId = (int)(lp & 0xFFFF)`，`maxCch = (int)(lp >>> 16)` | 返回拷贝到槽的**实际字符数**；C++ 侧 Read 后按 min(maxCch, n) 字节 memcpy 到调用方 buf |

**通用规则（重要）**：
1. SlotId ∈ `[1, 65535]`，**`id=0` 永远无效**，Read/Write/Free 直接短路；
2. **无 bit63 / SLOT_MARK / isSlot 判断**："是否字符串参数由 msg 号本身查表决定"，参数永远是干净整数；
3. JNI `jStr == nullptr` → 写空串 `"\"` 到槽，不崩溃；
4. Java 所有 View 操作必须通过 `runOnUiSync` 切到 Android 主线程（见 3.4）；
5. 未知消息 `default: return 0`，不抛异常。

### 3.3 C++ 控件级 WndProc 编码/解码下沉

`AndroidPlatformAPI.sendMessage` 只做 JNI 转发，**不处理控件级字符串 slot 化**。槽编码/解码下沉到每个控件独立的 WndProc 源文件里。例如 **EDIT 控件**：

- 入口在 [Android_Edit.cpp](file:///d:/work/soui4/demos/android-demo/app/src/main/cpp/Android_Edit.cpp)：`extern "C" LRESULT CALLBACK Android_EditWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)`；
- 每个字符串类消息（`WM_SETTEXT` / `EM_REPLACESEL` / `WM_GETTEXT` / `EM_GETLINE`）对应一个 case：
  - **入参**：`AutoStringSlot slot(str ? str : ""); return api.sendMessage(hWnd, msg, wParam, (LPARAM)slot.id());`
  - **出参**：`AutoStringSlot slot(NULL); LRESULT n = api.sendMessage(...); string s = slot.read(); /* memcpy 到用户 buf + NUL */ return n;`
  - **EM_GETLINE 打包**：先从 buf 首 WORD 读 maxCch → `((LPARAM)(UINT)maxCch << 16) | (WORD)slot.id()`；
- 优点：新控件（NativeButton / NativeList / NativeStatic）加对应 WndProc 文件即可，平台桥零改动；
- RAII 保护：即使消息 early-return / 异常，析构函数仍会 `stringSlotFree`。

### 3.4 Java 层主线程安全（UiThreadUtils.runOnUiSync）

> Android 所有 `View` 方法（setText/setSelection/getLayout/scrollTo/setPadding 等）必须在 UI 主线程调用，否则 Android 9+ 直接抛 `CalledFromWrongThreadException`。

`NativeEditView.nativeSendMessage` 可以被 C++ **任意线程**调用（通常是 swinx 业务线程 ≠ 主线程）。为此实现了公共工具类 **[UiThreadUtils](file:///d:/work/soui4/demos/android-demo/app/src/main/java/com/soui/UiThreadUtils.java)**（final class，纯静态）：

- 各 NativeXxxView 的实现模式统一：
  ```java
  @Override
  public long nativeSendMessage(final int msg, final long wp, final long lp) {
      return UiThreadUtils.runOnUiSync(() -> handleMessageImpl(msg, wp, lp));
  }
  // 私有实现主体 —— 进入时已保证在主线程
  private long handleMessageImpl(int msg, long wp, long lp) { ... }
  ```
- 拆分：`nativeSendMessage` 作为入口包装，实际消息体放入私有 `handleMessageImpl`；
- `UiThreadUtils.runOnUiSync` 关键点：
  1. **快速路径**：`Looper.myLooper() == Looper.getMainLooper()` → 直接 `block.get()`；避免"在主线程自投递然后 CountDownLatch 等自己"造成死锁；
  2. **其它线程**：通过 `Handler(Looper.getMainLooper())` 静态 `MAIN_HANDLER.post(runnable)` 投递（View detach 也可运行，比 `View.post` 更可靠）；
  3. 用 `CountDownLatch(1)` + `AtomicReference<T> result` + `AtomicReference<Throwable> thrown` 同步；
  4. **超时**：`latch.await(UiThreadUtils.UI_SYNC_TIMEOUT_SEC, TimeUnit.SECONDS)`（当前 10s，常量公开可读取，后续可改 `volatile` 可配置），超时抛出 `RuntimeException` 避免永久挂起；
  5. **中断**：`InterruptedException` → 恢复 `Thread.currentThread().interrupt()` 后再包装抛出；
  6. **异常透传**：`RuntimeException` / `Error` 原样抛出；其他 `Throwable` 包装成 `RuntimeException`；
  7. **Null 防护**：`block == null` 立刻抛 `NullPointerException`，避免静默空操作。

> 配套机制 `inTextOp(Supplier<T>/Runnable)`（文本修改计数 + EN_CHANGE 合并）因为 **始终在主线程** 运行，成员字段 `mInNativeTextOp / mNativeTextDirty` 的可见性天然正确；内部 `postDelayed` 也合法。

### UiThreadUtils vs NativeWindowDelegate 分工

- **NativeWindowDelegate** — 负责 `INativeWindow` 12 个窗口方法（nativeDestroy/nativeMove/nativeShow 等）的公共实现 + `resolveTarget` 容器提升逻辑，**与具体窗口操作绑定**；
- **UiThreadUtils** — 纯工具类，**不与任何 View/接口绑定**，任何地方（JNI 回调、BroadcastReceiver、非 View 代码）都能用来切主线程同步执行代码；与 NativeWindowDelegate 职责正交。

---

## 4. 设计优点

1. **接口最小化 & 对齐 Win32 模型**：INativeWindow 只剩一个 `nativeSendMessage(int,long,long):long`，完全对应 Win32 `SendMessage`；跨平台业务代码不需按平台选择不同 JNI 重载；
2. **新增消息零 JNI 签名改动**：未来 `EM_FORMATRANGE` / `WM_SETFONT` / 控件自定义消息等，只需更新控件 WndProc + SSOT 注释；Java/C++ JNI 层完全不动；
3. **C++/Java 边界干净**：双方只传基本类型（int/long/jstring），不跨层持有对象引用；彻底避免跨层生命周期、JNI 局部/全局引用溢出、GC 图污染；
4. **槽容量无上限**：任意大小字符串（base64 图片、JSON、长文档）都能通过；不受 jlong 位宽限制；
5. **EM_GETLINE 编码近似 Win32 语义**：lParam 里携带"buf 容量 + 槽 id"两个字段，和 Win32 把"buf 首 WORD 填最大字节数"的心智模型一致，C++ 侧 buf 填充代码几乎一行不改；
6. **无魔法位**：移除 bit63 判断，不占用参数高位、不与 `EM_LINEINDEX(wp=-1 当前行)` 这类负数参数冲突；Java/C++ 两侧位运算错配不会再造成难排查乱码；
7. **RAII 防泄漏/UAF**：AutoStringSlot 构造/析构配对，异常/early return 也释放；C++ 调用方几乎无法写出泄漏槽的代码；
8. **性能 / GC 友好**：slot 分配 99.9% 是 lock-free `fetch_add`；每个字符串消息比原来"NewStringUTF(GetStringUTFChars × 2 次)"少 1 次跨层拷贝，降低 young gc 压力；
9. **主线程强约束模板已落地为独立工具类**：`UiThreadUtils.runOnUiSync(Supplier<T>) / runOnUiSyncVoid(Runnable)` 已抽出为与 View/接口无关的 final 工具类；后续 NativeButton/NativeList/NativeStatic 等任何代码（含非 View 回调）都可直接调用，无需再各自写一套 Handler + CountDownLatch；并与 NativeWindowDelegate 职责正交（前者管线程切换、后者管窗口操作）；
10. **平台层与控件解耦**：AndroidPlatformAPI 永远不知道 EM_GETLINE/WM_SETTEXT 的语义；控件 WndProc 独立存在，可单测、可插拔。

---

## 5. 设计缺点 & 已知风险

1. **SlotId 环形复用无代际（ABA 风险）**：
   - id 是纯 `[1..65535]` 环；若出现**多线程并发密集 sendMessage**（业务场景罕见，swinx 通常单线程消息循环），线程 A 持有 id=N 还在 JNI 往返，线程 B wrap 后也分配到 N → 覆盖前者字符串；
   - **当前缓解**：注释 + `insert_or_assign` 覆盖语义（避免读旧值）；
   - **改进**见 §6.1。
2. **全局槽表的隔离性**：
   - 任何拿到 slotId 的代码都能 Read/Write；没有 HWND/owner 绑定；
   - **现状**：只在可信的 swinx 内部使用，风险可控；若未来槽 API 暴露给业务层需加 owner 校验。
3. **16-bit SlotId 上限**：
   - SSOT 固定 slotId 在 0~65535（实际 1..65535）；同时飞行 >65535 个字符串槽必然碰撞；
   - **现状**：同时飞行槽数 ≈ "当前处理消息数 × 每消息 1~2 槽" << 65535；实际不受限；
   - 若未来批量异步发送，可以扩展为 32-bit（`slotId | metadata`）或直接走 `NewStringUTF` 路径。
4. **runOnUiSync 的 ABBA 死锁风险**：
   - 典型：C++ 线程持锁 `L` → 调用 `sendMessage` 等主线程；主线程的另一个 JNI 回调尝试拿 `L` → 相互等；虽然有 10s 超时，但仍会长时间 block；
   - **团队约定**：sendMessage 调用点不要持有和 Java 回调共享的锁；或提供异步 `nativeSendMessage(msg,wp,lp,Callback)`。
5. **10 秒超时是 public static final 硬编码**：
   - 极复杂布局 / 大文本 setText（如 10MB 文本）+ 低端机可能超过 10s → 误报死锁；
   - **改进**：`UiThreadUtils.UI_SYNC_TIMEOUT_SEC` 改为 `volatile` 非 final，可在 Application 初始化时覆盖（如 Debug 放宽到 60s 方便断点），或设置 0 = infinite + 依赖系统 ANR 监控。
6. **EM_GETSEL 非 Win32 标准返回**：
   - Win32 `SendMessage(EM_GETSEL)` 返回值本身是 `MAKELRESULT(start,end)`；我们返回 jlong 打包值，再在 C++ WndProc 手动拆到指针；
   - 不是 bug，但属于"Android 特定约定"，必须写进 SSOT；新来者可能漏拆或误拆。
7. **JNI 长名注册的脆弱性**：
   - 目前 `Soui4Android.cpp` 全部用 `Java_com_soui_SouiPlatformBridge_*` 长名；包名/类名改动、方法签名改动、ProGuard/R8 都可能静默导致 `UnsatisfiedLinkError`；
   - **改进**§6.2：`JNI_OnLoad` + `RegisterNatives`。
8. **AutoStringSlot 默认复制/移动语义**：
   - 当前没显式 `= delete` 复制构造/赋值，编译器默认 shallow copy → 两个对象持同 id：先析构者 free，后者 `read()` 读到无效/复用中槽（ABA）；
   - 现状：代码中总是作为"栈局部变量"用、没有复制发生；
   - 改进 §6.3：`= delete` 复制 + 显式移动。

---

## 6. 改进建议（按优先级）

### 6.1【高】SlotId 加代际（epoch），消除 ABA
把 slotId 扩展为 32-bit：
```
slotId (uint32_t) = (epoch << 16) | (ringIdx & 0xFFFF)
```
- 每次 wrap 到 1 时 `epoch++`；
- `ReadString(id)` 先拆 epoch 与表中存的 epoch 比对，不匹配返回空串（或 assert）；
- Java 侧 `lParam` 保持 64-bit，代际编码到高位（例如 EM_GETLINE 调整为 `(maxCch << 32) | slotId`），64-bit 完全塞得下。

### 6.2【高】JNI 改为 RegisterNatives
在 `Soui4Android.cpp` 里实现 `JNI_OnLoad`：
```cpp
jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = nullptr;
    vm->GetEnv((void**)&env, JNI_VERSION_1_6);
    // register SouiPlatformBridge natives, NativeEditView.nativeNotifyEditEvent, etc.
    return JNI_VERSION_1_6;
}
```
- 方法名/包名改了不担心找不到；
- ProGuard/R8 可以自由 rename Java 类；
- 启动时一次性校验签名，不匹配立刻 crash（比"运行期某消息才 UnsatisfiedLink"好）。

### 6.3【中】提炼 NativeControlBase 基类 + 显式禁止复制 AutoStringSlot
> （子项"runOnUiSync 公共工具"已落地为 [UiThreadUtils](file:///d:/work/soui4/demos/android-demo/app/src/main/java/com/soui/UiThreadUtils.java)，✅ 已完成；以下两项仍待推进）

Java 侧公共基类（继承链用，避免每个 NativeXxxView 再写 12 个 delegate 方法）：
```java
public abstract class NativeControlBase extends View implements INativeWindow {
    protected final NativeWindowDelegate mDelegate = new NativeWindowDelegate(this);
    @Override public void   nativeDestroy()                              { mDelegate.nativeDestroy(); }
    @Override public void   nativeInvalidate(int l, int t, int r, int b) { mDelegate.nativeInvalidate(l,t,r,b); }
    @Override public boolean nativeShow(int cmdShow)                      { return mDelegate.nativeShow(cmdShow); }
    @Override public boolean nativeMove(int x, int y, int w, int h)      { return mDelegate.nativeMove(x,y,w,h); }
    // ... 其余 INativeWindow 基础方法一劳永逸；runOnUiSync 统一调 UiThreadUtils
}
```
NativeXxxView 的入口模式更短（1 行）：
```java
@Override
public long nativeSendMessage(final int msg, final long wp, final long lp) {
    return UiThreadUtils.runOnUiSync(() -> handleMessageImpl(msg, wp, lp));
}
```

C++ 侧 `AutoStringSlot` 增加移动语义 + 禁止复制：
```cpp
class AutoStringSlot {
public:
    AutoStringSlot(const AutoStringSlot&) = delete;
    AutoStringSlot& operator=(const AutoStringSlot&) = delete;
    AutoStringSlot(AutoStringSlot&& o) noexcept : m_id(o.m_id) { o.m_id = 0; }
    AutoStringSlot& operator=(AutoStringSlot&& o) noexcept {
        if (this != &o) { this->~AutoStringSlot(); m_id = o.m_id; o.m_id = 0; }
        return *this;
    }
    // ...
};
```

### 6.4【中】运行时诊断 & 告警
- C++ 侧维护 `msg → meta` 元信息表（`入参槽位/出参槽位/允许范围`）；
- `ReadString(id<=0)` / `WriteString(id 不在表内)` / `lp 高 16bit 越界` 等异常路径打印 `ALOGE/ATALOGE`；目前静默 return 0 非常难排查；
- `AndroidPlatformAPI` 可以开一个 `dumpStringSlotTable()` debug API 辅助排查泄漏。

### 6.5【中】runOnUiSync 超时可配置
把 `UiThreadUtils` 中的常量改为"volatile 非 final"，支持 Application 初始化时覆盖：
```java
public final class UiThreadUtils {
    public static volatile long UI_SYNC_TIMEOUT_SEC =
            BuildConfig.DEBUG ? 60L : 10L;  // Debug 宽松 60s 方便断点；Release 10s
    // ... 其余不变
}
```
测试或特殊设备可以覆盖：
```java
public class DemoApplication extends Application {
    @Override public void onCreate() {
        super.onCreate();
        UiThreadUtils.UI_SYNC_TIMEOUT_SEC = 0L;  // 0 = 无限等待（不推荐，仅开发调试用）
    }
}
```

### 6.6【低】Free List 加速分配 & 避免 wrap
`stringSlotFree(id)` 把 id 压入 `std::vector<int> m_freeList`；下次 alloc 先从 free list `pop_back`；free list 空了再 fetch_add ring；
- 减少 wrap 频率，ABA 概率进一步下降；
- 典型工作模式槽复用率非常高（N 条消息之后反复用同一批 id）。

---

## 7. 附录

### 附录 A. WM_SETTEXT（入参字符串）时序

```
C++ SEdit::SetWindowText(L"A")
  → swinx SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)L"A")
  → Android_Edit.cpp case WM_SETTEXT:
       TO_UTF8 → "A"
       AutoStringSlot slot("A");        // id = N
       api.sendMessage(hEdit, WM_SETTEXT, 0, slot.id())
         ──JNI──▶
         Java NativeEditView.nativeSendMessage
           runOnUiSync(() -> handleMessageImpl(WM_SETTEXT, 0, N))
             case WM_SETTEXT:
               text = nativeReadCxxStringSlot(N)
                 ──JNI──▶ ReadString(N) → jstring "A"
               inTextOp(() -> setText(text))   // 主线程
               return 1L
         ◀──JNI 返回 1
       return 1
     AutoStringSlot 析构 → stringSlotFree(N)   ✅
```

### 附录 B. WM_GETTEXT（出参字符串）时序

```
C++ GetWindowTextLength → WM_GETTEXTLENGTH → len
  → buf = new TCHAR[len+1]
  → SendMessage(hEdit, WM_GETTEXT, len+1, buf)
  → Android_Edit.cpp case WM_GETTEXT:
       AutoStringSlot slot(NULL);          // id = M
       LRESULT n = api.sendMessage(WM_GETTEXT, cchTextMax, slot.id())
         ──JNI──▶
         Java handleMessageImpl:
           s = getText().toString();
           if (cchTextMax > 0) s = s.substring(0, min(s.length(), cchTextMax-1));
           nativeWriteCxxStringSlot(M, s);   // 写入槽
           return s.length();
       string result = slot.read();          // ✅ 刚写入
       memcpy(buf, result.data(), min(cchTextMax-1, result.size()));
       buf[...] = '\0';
       return n;
     AutoStringSlot 析构 → stringSlotFree(M)   ✅
```

### 附录 C. EM_GETLINE（buf 首 WORD + 打包）时序

```
Win32 语义：首 WORD(buf) = 最大可写字节数；
C++ WORD buf[128]; buf[0] = 128;
  → SendMessage(hEdit, EM_GETLINE, lineNo, (LPARAM)buf)
  → Android_Edit.cpp case EM_GETLINE:
       const int lineNo = (int)wParam;
       const WORD maxCch = *reinterpret_cast<WORD*>(buf);   // 128
       AutoStringSlot slot(NULL);                            // id = K
       LPARAM lp = ((LPARAM)(UINT)maxCch << 16) | (WORD)slot.id();
       LRESULT actual = api.sendMessage(EM_GETLINE, (WPARAM)lineNo, lp)
         ──JNI──▶
         Java handleMessageImpl:
           slotId   = lp & 0xFFFF;                       // K
           maxChars = lp >>> 16;                         // 128
           lineStr = layout.getLineVisibleEnd(lineNo).substring(so, min(eo,so+maxChars));
           nativeWriteCxxStringSlot(slotId, lineStr);
           return lineStr.length();
       string line = slot.read();
       memcpy(buf, line.data(), min(maxCch, (int)line.size()));  // ✅ 和 Win32 同样写回 buf
       return min(maxCch, (int)line.size());
     AutoStringSlot 析构 → stringSlotFree(K)   ✅
```

---

> 文档版本：2026-07；对应代码位置：
> - C++ 平台桥：[AndroidPlatformAPI.h](file:///d:/work/soui4/demos/android-demo/app/src/main/cpp/AndroidPlatformAPI.h) / [.cpp](file:///d:/work/soui4/demos/android-demo/app/src/main/cpp/AndroidPlatformAPI.cpp)
> - C++ EDIT WndProc：[Android_Edit.cpp](file:///d:/work/soui4/demos/android-demo/app/src/main/cpp/Android_Edit.cpp)
> - C++ JNI 桥：[Soui4Android.cpp](file:///d:/work/soui4/demos/android-demo/app/src/main/cpp/Soui4Android.cpp)
> - Java 接口：[INativeWindow.java](file:///d:/work/soui4/demos/android-demo/app/src/main/java/com/soui/INativeWindow.java)
> - Java 原生 EDIT 实现：[NativeEditView.java](file:///d:/work/soui4/demos/android-demo/app/src/main/java/com/soui/NativeEditView.java)
> - Java 静态桥：[SouiPlatformBridge.java](file:///d:/work/soui4/demos/android-demo/app/src/main/java/com/soui/SouiPlatformBridge.java)
> - Java 公共委托（INativeWindow 12 方法）：[NativeWindowDelegate.java](file:///d:/work/soui4/demos/android-demo/app/src/main/java/com/soui/NativeWindowDelegate.java)
> - Java 主线程同步工具（独立模块）：[UiThreadUtils.java](file:///d:/work/soui4/demos/android-demo/app/src/main/java/com/soui/UiThreadUtils.java)
