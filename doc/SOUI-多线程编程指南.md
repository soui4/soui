# SOUI 多线程编程指南

- **适用版本**：SOUI 4/5（dev 分支，2026-09 起含完整多 UI 线程加固）
- **读者**：使用 SOUI 开发应用的开发者
- **配套文档**：框架内部实现与风险分析见《[SOUI-多UI线程安全报告-2026-09-29](SOUI-多UI线程安全报告-2026-09-29.md)》

---

## 1. 总原则

SOUI 的线程安全建立在一条核心原则之上：

> **同一个窗口树（SHostWnd 及其全部 SWindow 子孙）在任一时刻只允许一个线程访问。**

SOUI 支持三种并发形态，规则各不相同：

| 线程形态 | 允许的操作 |
|---|---|
| **UI 线程**（运行 SOUI 消息循环） | 创建窗口、操作自己创建的窗口树、使用全部全局管理器 |
| **工作线程**（std::thread / 任务池） | 只做数据计算、IO、解码；**不得直接触碰任何 SWindow**；通过 §4 的通道把结果交回 UI 线程 |
| **多 UI 线程**（每个线程各自的消息循环） | 各自拥有独立的窗口树；共享资源（skin、样式、资源、渲染后端）由框架保证并发安全 |

### 1.1 一页速查（黄金法则）

1. **窗口对象永远只在创建它的线程里访问**——包括 `FindChildByName`、`SetVisible`、`GetWindowText` 等看似只读的调用。
2. **SStringW/SStringA 是单线程类型**。跨线程传递字符串一律用 `std::string`/`std::wstring`（见 §3）。
3. 工作线程 → UI 线程只能走三条通道：**窗口消息、`STaskHelper::post`、`FireEventAsync`**（见 §4）。
4. **初始化与注册只在启动期单线程做**：工厂注册（`SApplication::RegisterFactory` 等宏）、`SNamedID`、资源包加载、皮肤 XML 加载。
5. skin/样式/字体等共享对象**初始化完成后可以多线程并发读**（绘制、查找），框架内部已加锁。
6. 染色（`OnColorize`）、换 skin 图源（`SetImage/SetSvg`）是写操作，可以在任意 UI 线程调用，效果对所有线程可见（见 §5.2）。

---

## 2. UI 线程内：窗口树亲和

每个 `SHostWnd` 及其控件树属于创建它的线程。SOUI **不在 SWindow 内部加锁**，线程间互斥完全靠这条亲和性约定：

```cpp
// ❌ 错误：工作线程直接操作窗口树
void WorkerThread(SWindow *pWnd) {
    pWnd->SetVisible(TRUE, TRUE);   // 未定义行为：崩溃或状态损坏
}

// ✅ 正确：把操作封送回窗口所在线程（见 §4）
void WorkerThread(SHostWnd *pHost) {
    STaskHelper::post(pHost->GetMsgLoop(), []() {
        // 此代码运行在 UI 线程，可安全操作窗口树
    });
}
```

**跨线程"看起来只读"的调用同样禁止**：布局查询、文本读取、遍历子窗口都可能触发内部状态修改（EnsureVisible、延迟布局等）。

多 UI 线程的典型结构：每个 UI 线程各自 `SApplication::AddMsgLoop()` 注册消息循环、各自创建 `SHostWnd`。窗口 A 所在线程的代码不要持有窗口 B 的指针直接调用；如需跨窗口通信，用 host HWND 的 Win32 消息（操作系统负责封送到目标线程）。

---

## 3. 字符串：SStringW/SStringA 只在单线程使用

> **`SStringW`/`SStringA` 的设计目标是单线程内使用。跨线程传递字符串，一律使用 `std::string`/`std::wstring`。**

原因：`SStringT` 采用写时复制（COW）+ **非原子**引用计数（`TStringData::nRefs` 为裸 `int`）。同一个字符串缓冲区被两个线程同时拷贝/析构时，引用计数会错乱，最终双重释放崩溃。`std::string`（MSVC 实现，SSO + 无共享引用计数）没有这个问题。

具体规则：

```cpp
// ❌ 错误 1：lambda 按值捕获 SStringW 跨线程投递
SStringW strName = L"avatar.gif";
STaskHelper::post(pTaskLoop, [strName]() {   // 拷贝构造在投递线程，
    // ...                                   // 析构在执行线程 → 引用计数竞争
}, ...);

// ❌ 错误 2：跨线程窗口消息携带 SStringT 指针
::PostMessage(hwnd, WM_USER + 1, 0, (LPARAM)(LPCTSTR)strName);

// ✅ 正确：跨线程边界用 std::string/std::wstring
std::wstring strName = L"avatar.gif";
STaskHelper::post(pTaskLoop, [strName]() {   // std::wstring 拷贝线程安全
    SStringW strSoui = strName.c_str();      // 进入 UI 线程后再转回 SStringW
    // ... 使用 strSoui
});
```

边界判别方法：**只要一个对象的生命周期"开始于线程 A、结束于线程 B"，它的类型就必须是无共享状态的**——`std::string`、POD、堆独占指针（`std::unique_ptr`/裸指针+约定所有权）都满足；`SStringT` 不满足。

---

## 4. 跨线程通信的三条通道

### 4.1 任务投递：`STaskHelper::post`

`SOUI/include/helper/SFunctor.hpp` 提供 lambda 支持的重载：

```cpp
#include <helper/SFunctor.hpp>

// ① 投递到 UI 线程（消息循环空闲时执行）
IMessageLoop *pMsgLoop = pHostWnd->GetMsgLoop();       // 或 SApplication::getSingleton().GetMsgLoop(tid)
STaskHelper::post(pMsgLoop, [this, nResult]() {
    m_pProgress->SetProgress(nResult);                  // UI 线程内安全
});

// ② 投递到工作线程池（ITaskLoop）
ITaskLoop *pTaskLoop = ...;
STaskHelper::post(pTaskLoop, this, &CFoo::_DoWork, strUrl, nRetry, false, 0);
//                                                    ↑ waitUntilDone   ↑ priority
```

**捕获纪律**：lambda 只捕获 `this`、POD 和 `std::string` 等无共享对象；禁止按值捕获 `SStringT`、`SAutoRefPtr` 之外的引用计数对象跨线程复制（`SAutoRefPtr` 的 `operator=` 本身也非原子，跨线程持有目标对象没问题，但同一份 `SAutoRefPtr` 变量不能被两个线程同时读写）。

### 4.2 窗口消息

对 host 窗口用 Win32 原生消息，操作系统负责跨线程封送：

```cpp
// 工作线程
::PostMessage(pHost->GetHostHwnd(), WM_USER + 1, nProgress, 0);

// host 窗口内
void OnUserMsg(UINT uMsg, WPARAM wp, LPARAM lp) { ... }   // 已在 UI 线程
```

### 4.3 异步事件：`FireEventAsync`

```cpp
// 任意线程可调；事件对象必须堆分配，调用方 Release
SNotifyCenter::getSingleton().FireEventAsync(evt);   // evt 为 new 出来的 IEvtArgs
```

事件由主 UI 线程的 `SNotifyReceiver` 定时器统一派发，处理函数运行在主线程。**同步版 `FireEventSync` 只允许主 UI 线程调用**（内部有断言），工作线程误用会直接断言失败。

三条通道怎么选：需要**带大量数据/高频**回传用窗口消息或任务投递；需要**广播给多个订阅者**用 `FireEventAsync`。

---

## 5. 共享资源的并发规则

### 5.1 只读使用：随时、任意 UI 线程

以下操作框架内部已同步，多 UI 线程可放心并发：

- `GETSKIN(name, scale)` 取 skin、skin 绘制（`DrawByIndex`/`_DrawByState`）；
- 字体查找（`GetFont`）、样式/模板查找（`GetStyle`/`GetTemplateString`）；
- 资源加载（`LOADIMAGE2` 等，内部串行化解码，注意这是全局瓶颈，高频大图加载建议自行在工作线程解码后 `SetImage`）。

skin 的内部语义保证**共享不崩溃**：懒加载、SVG 光栅缓存、缩放副本都在锁内完成，绘制方拿到的是持有引用的对象。允许的代价是极小概率观察到半染色像素——不影响正确性。

### 5.2 写操作：合法但全局可见

| 写操作 | 线程要求 | 效果 |
|---|---|---|
| `skin->OnColorize(cr)` | 任意 UI 线程 | 染色同步影响**所有**引用该 skin 的窗口（含缩放副本，SVG 副本共享同一 SVG 源） |
| `skin->SetImage/SetSvg` | 任意 UI 线程 | 换源全局可见；在途绘制因持有引用不受影响 |
| `AddSkin/RemoveSkin/RemoveAll` | **约定单线程**（启动期或换肤时序） | 运行期并发增删有 UAF 窗口（见报告 §5.3） |

### 5.3 仅限启动期单线程的清单

- 对象工厂注册（`SOUI_COM_*` 宏、`RegisterFactory`、`SetRenderFactory` 等）
- `SNamedID` 命名 ID 表填充
- 资源包初始化（`LoadResProvider(0, ...)`）
- 皮肤 XML 加载（`LoadSkins`）
- `SLog::setLogCallback`

这些数据结构无锁，启动期单线程填充后运行期只读，并发读是安全的；**运行期重注册/重填充会与并发读者竞争**。

---

## 6. 禁止事项清单

- ❌ 工作线程直接调用任何 `SWindow`/控件方法（包括"只读"方法）
- ❌ 跨线程按值传递 `SStringW`/`SStringA`（lambda 捕获、消息 LPARAM、任务参数）
- ❌ 非主线程调用 `SNotifyCenter::FireEventSync`
- ❌ 运行期（多线程运行中）注册/反注册对象工厂、增删内置 skin 池内容
- ❌ 两个 UI 线程各自启动时同时执行"初始化清单"（§5.3）里的操作——初始化必须先在单一主线程完成
- ❌ 在窗口析构后投递绑定该窗口的任务（`STaskHelper::post` 捕获 `this` 的对象须保证消息处理完前存活，或在析构时清理队列）

---

## 7. 常见场景配方

### 7.1 工作线程下载/计算 → 更新 UI

```cpp
void CMainDlg::OnBtnStart() {
    std::wstring strUrl = m_editUrl->GetText().GetString();  // UI 线程取值
    std::thread([this, strUrl]() {
        std::string result = Download(strUrl);               // 工作线程干活
        // 回传：std::string 捕获安全
        STaskHelper::post(GetMsgLoop(), [this, result]() {
            m_txtResult->SetWindowText(S_CW2T(S_CA2W(result.c_str())));
        });
    }).detach();
}
```

### 7.2 多 UI 线程各自建窗口

```cpp
// 线程入口（SApplication 已在主线程完成全部初始化）
DWORD WINAPI SecondUiThread(LPVOID) {
    SApplication::getSingleton().AddMsgLoop(new SMessageLoop());
    SHostWnd wnd(L"xml:second_dlg");
    wnd.Create(...);
    wnd.ShowWindow(SW_SHOW);
    /* Run 内部消息循环，直到窗口关闭 */
    SApplication::getSingleton().RemoveMsgLoop();
    return 0;
}
```

前提：所有工厂注册、资源包、皮肤加载已在主线程完成；两个线程只共享**只读**资源，各自操作自己的窗口树。

---

## 8. 参考

- 框架内部同步机制、锁序约束与遗留风险逐条分析：《[SOUI-多UI线程安全报告-2026-09-29](SOUI-多UI线程安全报告-2026-09-29.md)》
- 异步任务原语：`SOUI/include/helper/SFunctor.hpp`（`STaskHelper`）、`STaskLoop-i.h`（`ITaskLoop::postTask`）
- 事件机制：`SOUI/include/event/SNotifyCenter.h`
