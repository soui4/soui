#ifndef __WS_CLIENT_WASM_H__
#define __WS_CLIENT_WASM_H__

#include <interface/ws-i.h>
#include <interface/STaskLoop-i.h>
#include <helper/obj-ref-impl.hpp>
#include <helper/SCriticalSection.h>
#include <emscripten/websocket.h>
#include <deque>
#include <string>
#include <vector>

SNSBEGIN

/**
 * @brief 要交给 IConnListener 的事件种类，与桌面版那几个 lws 回调一一对应
 */
enum WsWasmEvent
{
    WSEV_OPEN,  ///< onConnected()
    WSEV_ERROR, ///< onConnError(reason)
    WSEV_CLOSE, ///< onDisconnect()
    WSEV_DATA   ///< onDataRecv(data, len, bBinary)
};

class CWsWasmMailbox;

/**
 * @brief 信箱任务：ITaskLoop 队列里的一项，一条事件对应一条任务
 *
 * clone() 只复制信箱引用，队列里的任务因此不额外持有字节拷贝；真正的内容在信箱
 * 的 deque 里排队。getObject() 回指信箱，cancelTasksForObject 才有得可匹配。
 */
class CWsWasmMailboxTask : public TObjRefImpl<IRunnable>
{
  public:
    CWsWasmMailboxTask(CWsWasmMailbox *pMail);

    STDMETHOD_(IRunnable *, clone)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(void, run)(THIS) OVERRIDE;
    STDMETHOD_(void *, getObject)(THIS) OVERRIDE;
    STDMETHOD_(const char *, getClassInfo)(CTHIS) SCONST OVERRIDE
    {
        return "CWsWasmMailboxTask";
    }

  private:
    SAutoRefPtr<CWsWasmMailbox> m_pMail;
};

/**
 * @brief 事件信箱：主线程只负责入队，交付由 ITaskLoop 的执行线程做
 *
 * 为什么要在中间插一个信箱：浏览器侧的 socket 只能活在页面主线程（见
 * CWasmWsClient::connectTo 的注释），而监听器回调要在后台线程跑才有桌面版的口径。
 * 信箱就是这两者之间唯一的一次拷贝 + 一次排队。
 *
 * 三条不变量：
 *   - m_active 为 false 之后绝不再碰 m_pListener。detach() 与交付都在同一把锁里改
 *     这个标志、调回调，所以调用方 detach 完就能安全释放监听器 —— 与桌面版
 *     WsClient::quit() 里 join 工作线程给的是同一个保证。
 *   - 队列里的事件严格按到达顺序交付：postTask 同优先级是 FIFO，一条事件一条任务。
 *   - 没有可用队列（组件没链进来、或 postTask 失败）时就地交付，退回改动前的
 *     "主线程同步调用"形态，不静默丢事件。
 */
class CWsWasmMailbox : public TObjRefImpl<IObjRef>
{
  public:
    /** pLoop 可以为 NULL：那时所有交付都在调用 enqueue 的线程上就地完成 */
    CWsWasmMailbox(IConnListener *pListener, ITaskLoop *pLoop);
    ~CWsWasmMailbox();

    /** 页面主线程（JS 事件回调）调用：拷进队列并按队列形态决定就地交付还是投一条任务 */
    void enqueue(WsWasmEvent ev, const void *pData, int nBytes, bool bText, unsigned short code,
                 const char *pszReason);

    /** 执行线程调用：取出队首一条交给监听器（队空、已摘除或处于阻塞态时什么都不做） */
    void runOne();

    /** 与桌面版 blockReceive 同义：TRUE 只堆不交付，FALSE 给堆着的每条补投一条任务 */
    void setBlocked(BOOL bBlock);

    /** UI 线程调用：摘掉监听器、丢掉排队中的事件、停掉并释放后台队列；可重复调用 */
    void detach();

    /** 后台队列是否可用（观测与注释用；不可用时交付就地发生在主线程） */
    bool hasLoop()
    {
        return m_pLoop != NULL;
    }

  private:
    struct Msg
    {
        Msg()
            : ev(WSEV_DATA)
            , bText(false)
            , code(0)
        {
        }
        WsWasmEvent ev;
        std::vector<unsigned char> data;
        bool bText;
        unsigned short code;
        std::string reason;
    };

    void postOne();
    /** 必须带着 m_cs 调用：与 detach() 互斥就是这里的立身之本 */
    void invokeLocked(const Msg &msg);

    SCriticalSection m_cs;
    std::deque<Msg> m_msgs;
    IConnListener *m_pListener;
    SAutoRefPtr<ITaskLoop> m_pLoop;
    bool m_active;
    bool m_bBlocked;
};

/**
 * @brief 浏览器原生 WebSocket 版的 IWsClient
 *
 * 与桌面版（lws + 工作线程）的差别：
 *   - socket 对象本身只能在页面主线程上。Emscripten 的 libwebsocket.js 里
 *     createOnMainThread==false 还是 TODO，四个 *_on_thread 接口把 thread 参数直接忽略、
 *     回调一律在主线程的 JS 事件循环里 makeDynCall —— 所以"把 socket 搬进 worker"这条路
 *     在当前工具链上不通，只能把搬运的边界放在回调之后。
 *   - 边界之后与桌面版同口径：主线程的 C 回调只做一次拷贝 + 入队，剩下的（拆包头、
 *     调 IConnListener）交给 ITaskLoop 的 worker 线程。消息数据只在回调期间有效，
 *     所以拷贝必须留在回调里。
 *   - wait() 没有可等的队列（事件不由调用线程驱动，UI 线程一睡整个标签页就冻住），
 *     恒返回 FALSE。调用方要么用 blockReceive 缓存、要么像 cnchess 那样在自己的监听器里排队。
 *   - 浏览器不能监听端口，CreateWsServer 返回 NULL。
 */
class CWasmWsClient : public TObjRefImpl<IWsClient>
{
  public:
    CWasmWsClient(IConnListener *pListener);
    ~CWasmWsClient();

    STDMETHODIMP_(int) isValid(THIS) SCONST OVERRIDE
    {
        return (m_socket > 0 && m_connected && !m_closed) ? 1 : 0;
    }

    STDMETHODIMP_(int) sendText(THIS_ const char *text, int nLen DEF_VAL(-1)) OVERRIDE;
    STDMETHODIMP_(int) sendBinary(THIS_ const void *data, int nLen) OVERRIDE;
    STDMETHODIMP_(int) sendBinary2(THIS_ DWORD dwType, const void *data, int nLen) OVERRIDE;

    STDMETHODIMP_(int) connectTo(THIS_ const char *pszSvr, const char *pszPath, uint16_t nPort,
                                 const char *pszProtocol, ClientOption option) OVERRIDE;
    STDMETHODIMP_(void) disconnect(THIS) OVERRIDE;
    STDMETHODIMP_(BOOL) wait(THIS_ int timeoutMs) OVERRIDE;
    STDMETHOD_(void, blockReceive)(THIS_ BOOL bBlock) OVERRIDE;

    /** 关闭底层 socket 并摘掉回调，析构与 disconnect 共用 */
    void teardown();

  private:
    int sendFrame(const void *data, int nLen, bool bText);

    /** 建信箱与后台队列（连接入口调用；重复调用只建一次） */
    void ensureDelivery();

    void onOpen();
    void onError();
    void onClose(unsigned short code, const char *reason);
    void onMessage(const unsigned char *pData, unsigned int nBytes, bool bText);

    IConnListener *m_pListener;
    SAutoRefPtr<CWsWasmMailbox> m_pMail;
    EMSCRIPTEN_WEBSOCKET_T m_socket;
    bool m_connected;
    bool m_closed;
};

/** 日志回调（与桌面版 IWebsocket::SetLogCallback 同义） */
void WsWasmSetLogCallback(WsLogCallback cb);
void WsWasmLog(int level, const char *fmt, ...);

/**
 * 投递计数：无头烟测用来取证"收包整条离开了 UI 线程"的三口之一。
 *
 * 只数交给 onDataRecv 的二进制/文本帧（连接建立与断开是单发事件，数它们没有区分度）。
 * 两个数在 worker 后端下相等，在泵后端下第二个恒为 0 —— 只看第一个数的话两条后端
 * 都给得出，区分不了这次改动的目标，与 TaskLoopWasm 里 6/19 那对计数同一个道理。
 */
long WS_WasmDeliveredFrames();
long WS_WasmDeliveredFramesOffMain();

SNSEND

#endif // __WS_CLIENT_WASM_H__
