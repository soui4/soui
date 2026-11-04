// 浏览器原生 WebSocket 版 IWsClient（详见 WsClientWasm.h 的口径说明）。
//
// 这一版把"事件在哪条线程上交给监听器"对齐到桌面版：主线程的 JS 回调只做一次拷贝 +
// 入队，拆包头和 IConnListener 的四个回调都发生在 ITaskLoop 的 worker 线程上。

#include "WsClientWasm.h"
#include <commgr2.h>
#include <emscripten/atomic.h>
#include <emscripten/threading.h>
#include <emscripten/websocket.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <string>

SNSBEGIN

namespace
{
WsLogCallback g_wsLog = NULL;

// 每次构建 ws 客户端都从这里要一条独立队列，与桌面版"每个 WsClient 一条 std::thread"
// 同形态。SComMgr2 的静态分支直接落到 TASKLOOP::SCreateInstance（见 components/commgr2.h），
// 和 ThemeDownloadManager 拿解压队列用的是同一个入口。
SComMgr2 g_wsComMgr;

// 投递计数走原子加减：worker 后端由 worker 线程递增、UI 线程只读；不带 -pthread 的构建
// 里这两个调用退化成普通读写，本来也只有一条线程碰它们。
volatile unsigned int g_wsFramesDelivered = 0;
volatile unsigned int g_wsFramesDeliveredOffMain = 0;

unsigned int wsAtomicLoad(volatile unsigned int *p)
{
    return emscripten_atomic_load_u32((const void *)p);
}

void wsAtomicAdd(volatile unsigned int *p, unsigned int n)
{
    emscripten_atomic_add_u32((void *)p, n);
}

} // namespace

void WsWasmSetLogCallback(WsLogCallback cb)
{
    g_wsLog = cb;
}

void WsWasmLog(int level, const char *fmt, ...)
{
    if (!g_wsLog)
        return;
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    g_wsLog(level, buf);
}

long WS_WasmDeliveredFrames()
{
    return (long)wsAtomicLoad(&g_wsFramesDelivered);
}

long WS_WasmDeliveredFramesOffMain()
{
    return (long)wsAtomicLoad(&g_wsFramesDeliveredOffMain);
}

//=============================================================================
// 事件信箱
//=============================================================================

CWsWasmMailboxTask::CWsWasmMailboxTask(CWsWasmMailbox *pMail)
{
    m_pMail = pMail;
}

IRunnable *CWsWasmMailboxTask::clone() const
{
    return new CWsWasmMailboxTask(m_pMail);
}

void CWsWasmMailboxTask::run()
{
    m_pMail->runOne();
}

void *CWsWasmMailboxTask::getObject()
{
    return m_pMail;
}

CWsWasmMailbox::CWsWasmMailbox(IConnListener *pListener, ITaskLoop *pLoop)
    : m_pListener(pListener)
    , m_active(true)
    , m_bBlocked(false)
{
    m_pLoop = pLoop;
}

CWsWasmMailbox::~CWsWasmMailbox()
{
    detach();
}

void CWsWasmMailbox::enqueue(WsWasmEvent ev, const void *pData, int nBytes, bool bText,
                             unsigned short code, const char *pszReason)
{
    bool bPost = false;
    {
        SAutoLock lock(m_cs);
        if (!m_active)
            return;
        Msg msg;
        msg.ev = ev;
        msg.bText = bText;
        msg.code = code;
        if (nBytes > 0 && pData)
            msg.data.assign((const unsigned char *)pData, (const unsigned char *)pData + nBytes);
        if (pszReason)
            msg.reason = pszReason;
        m_msgs.push_back(msg);
        bPost = !m_bBlocked;
    }
    if (bPost)
        postOne();
}

void CWsWasmMailbox::postOne()
{
    if (!m_pLoop)
    {
        // 没有队列可投：就地交付，退回改动前的形态。丢事件比"在主线程上调一次回调"更糟。
        runOne();
        return;
    }
    CWsWasmMailboxTask task(this);
    if (m_pLoop->postTask(&task, FALSE, (int)Normal) < 0)
        runOne();
}

void CWsWasmMailbox::runOne()
{
    Msg msg;
    SAutoLock lock(m_cs);
    if (!m_active || m_bBlocked || m_msgs.empty())
        return;
    msg = m_msgs.front();
    m_msgs.pop_front();
    invokeLocked(msg);
}

void CWsWasmMailbox::invokeLocked(const Msg &msg)
{
    if (!m_pListener)
        return;
    switch (msg.ev)
    {
    case WSEV_OPEN:
        m_pListener->onConnected();
        return;
    case WSEV_ERROR:
        m_pListener->onConnError(msg.reason.c_str());
        return;
    case WSEV_CLOSE:
        m_pListener->onDisconnect();
        return;
    default:
        break;
    }
    const void *pData = msg.data.empty() ? NULL : (const void *)&msg.data[0];
    m_pListener->onDataRecv(pData, (int)msg.data.size(), msg.bText ? FALSE : TRUE);
    wsAtomicAdd(&g_wsFramesDelivered, 1);
    // 泵后端下这里读到的是主线程（STaskLoopPump 由宿主的帧循环调用），计数不动；
    // worker 后端下整条都在 worker 上，两个数因此始终相等。
    if (!emscripten_is_main_runtime_thread())
        wsAtomicAdd(&g_wsFramesDeliveredOffMain, 1);
}

void CWsWasmMailbox::setBlocked(BOOL bBlock)
{
    size_t nToPost = 0;
    {
        SAutoLock lock(m_cs);
        if (!m_active)
            return;
        m_bBlocked = (bBlock != FALSE);
        // 解除阻塞时给堆着的每条事件补一条任务：与到达时投递等价，顺序照旧
        if (!m_bBlocked)
            nToPost = m_msgs.size();
    }
    for (size_t i = 0; i < nToPost && m_pLoop; ++i)
    {
        CWsWasmMailboxTask task(this);
        if (m_pLoop->postTask(&task, FALSE, (int)Normal) < 0)
            break;
    }
    if (nToPost && !m_pLoop)
    {
        while (!m_msgs.empty())
            runOne();
    }
}

void CWsWasmMailbox::detach()
{
    {
        SAutoLock lock(m_cs);
        m_active = false;
        m_msgs.clear();
    }
    if (!m_pLoop)
        return;
    // stop() 会 join worker：与桌面版 quit() 里等 worker 退出同口径。worker 此时可能在
    // 最后一条 invokeLocked 里，而那条回调只做"拷一份 + PostTask 回 UI 消息队列"，
    // 不会反过来等 UI 线程，所以这里等得到。
    m_pLoop->stop();
    m_pLoop = NULL;
}

//=============================================================================
// IWsClient
//=============================================================================

CWasmWsClient::CWasmWsClient(IConnListener *pListener)
    : m_pListener(pListener)
    , m_socket(0)
    , m_connected(false)
    , m_closed(true)
{
}

CWasmWsClient::~CWasmWsClient()
{
    teardown();
}

void CWasmWsClient::ensureDelivery()
{
    if (m_pMail)
        return;

    SAutoRefPtr<ITaskLoop> pLoop;
    // 建不起队列不是失败：信箱会退回就地交付，页面照常收发，只是没有并行 ——
    // 与 worker 创建失败时 TaskLoop 自己退回泵后端是同一个态度。
    if (g_wsComMgr.CreateTaskLoop((IObjRef **)&pLoop) && pLoop)
        pLoop->start("ws_delivery", Normal);
    if (pLoop && !pLoop->isRunning())
        pLoop = NULL;

    CWsWasmMailbox *pMail = new CWsWasmMailbox(m_pListener, pLoop);
    m_pMail.Attach(pMail);
    WsWasmLog(2, "delivery loop %s", pLoop ? "started" : "unavailable");
}

int CWasmWsClient::connectTo(const char *pszSvr, const char *pszPath, uint16_t nPort,
                             const char *pszProtocol, ClientOption option)
{
    if (m_socket > 0)
        return -2; // 已经连过，与桌面版一致

    if (!emscripten_websocket_is_supported())
    {
        WsWasmLog(3, "connectTo: this environment has no WebSocket");
        if (m_pListener)
            m_pListener->onConnError("WebSocket is not supported");
        return -1;
    }

    ensureDelivery();

    // cnchess 传进来的 path 形如 "/cnchess/?"，端口单独给；wss 只由 option.bSecure 决定。
    std::string url;
    url.reserve(64);
    url += option.bSecure ? "wss://" : "ws://";
    url += pszSvr;
    if (nPort)
    {
        char szPort[16];
        snprintf(szPort, sizeof(szPort), ":%u", (unsigned)nPort);
        url += szPort;
    }
    if (pszPath && *pszPath)
        url += pszPath;

    EmscriptenWebSocketCreateAttributes attr;
    emscripten_websocket_init_create_attributes(&attr);
    attr.url = url.c_str();
    // 服务器（libwebsockets）按 "cnchess" 这个子协议注册；浏览器里空串表示不协商子协议，
    // 与桌面版 lws 客户端传 "" 的行为一致。
    attr.protocols = (pszProtocol && *pszProtocol) ? pszProtocol : NULL;
    // 这里必须留 true，而且它现在也是工具链的唯一选择：libwebsocket.js 里
    // createOnMainThread==false 还是 TODO，socket 对象只能建在主线程的 JS 上下文里。
    // 真正搬到后台线程的是回调之后的事（见 CWsWasmMailbox）。
    attr.createOnMainThread = true;

    EMSCRIPTEN_WEBSOCKET_T socket = emscripten_websocket_new(&attr);
    if (socket <= 0)
    {
        WsWasmLog(3, "connectTo: emscripten_websocket_new failed (%d), url=%s", (int)socket, url.c_str());
        if (m_pListener)
            m_pListener->onConnError("create websocket failed");
        return -1;
    }

    m_socket = socket;
    m_closed = false;
    m_connected = false;

    emscripten_websocket_set_onopen_callback(m_socket, this,
                                             [](int, const EmscriptenWebSocketOpenEvent *, void *user) -> bool {
                                                 static_cast<CWasmWsClient *>(user)->onOpen();
                                                 return true;
                                             });
    emscripten_websocket_set_onerror_callback(m_socket, this,
                                              [](int, const EmscriptenWebSocketErrorEvent *, void *user) -> bool {
                                                  static_cast<CWasmWsClient *>(user)->onError();
                                                  return true;
                                              });
    emscripten_websocket_set_onclose_callback(m_socket, this,
                                              [](int, const EmscriptenWebSocketCloseEvent *e, void *user) -> bool {
                                                  static_cast<CWasmWsClient *>(user)->onClose(e->code, e->reason);
                                                  return true;
                                              });
    emscripten_websocket_set_onmessage_callback(
        m_socket, this, [](int, const EmscriptenWebSocketMessageEvent *e, void *user) -> bool {
            static_cast<CWasmWsClient *>(user)->onMessage(e->data, e->numBytes, e->isText);
            return true;
        });

    WsWasmLog(2, "connectTo: %s", url.c_str());
    return 0;
}

void CWasmWsClient::teardown()
{
    // 顺序要紧：先摘信箱，再关 socket。摘完之后到 delete 之间到达的事件会被 enqueue
    // 直接丢掉（m_active 已 false），而监听器对象此刻还没被调用方释放。
    if (m_pMail)
    {
        m_pMail->detach();
        m_pMail = NULL;
    }
    if (m_socket > 0)
    {
        // emscripten_websocket_delete 只是把 JS 侧四个 handler 置空并释放句柄，
        // 不会主动 close；先 close（已经 close 过的就别再来一次）再 delete。
        if (!m_closed)
            emscripten_websocket_close(m_socket, 1000, "bye");
        emscripten_websocket_delete(m_socket);
        m_socket = 0;
    }
    m_connected = false;
    m_closed = true;
}

void CWasmWsClient::disconnect()
{
    teardown();
}

BOOL CWasmWsClient::wait(int timeoutMs)
{
    (void)timeoutMs;
    // 事件先到页面主线程、再排进信箱，没有一条队列是调用方能等的；把 UI 线程睡下去
    // 在这个平台上等于冻住整个标签页。要缓存就用 blockReceive，要即时处理就在监听器里排队。
    return FALSE;
}

void CWasmWsClient::blockReceive(BOOL bBlock)
{
    if (m_pMail)
        m_pMail->setBlocked(bBlock);
}

int CWasmWsClient::sendText(const char *text, int nLen)
{
    if (nLen < 0)
        nLen = text ? (int)strlen(text) : 0;
    return sendFrame(text, nLen, true);
}

int CWasmWsClient::sendBinary(const void *data, int nLen)
{
    return sendFrame(data, nLen, false);
}

int CWasmWsClient::sendBinary2(DWORD dwType, const void *data, int nLen)
{
    // 与桌面版一致的 8 字节头：dwType + nLen
    std::vector<unsigned char> frame(8 + (size_t)nLen);
    memcpy(&frame[0], &dwType, sizeof(dwType));
    memcpy(&frame[4], &nLen, sizeof(nLen));
    if (nLen > 0 && data)
        memcpy(&frame[8], data, (size_t)nLen);
    return sendFrame(&frame[0], (int)frame.size(), false);
}

int CWasmWsClient::sendFrame(const void *data, int nLen, bool bText)
{
    if (m_socket <= 0 || m_closed)
        return -1;
    if (nLen < 0)
        return -1;

    EMSCRIPTEN_RESULT ret;
    if (bText)
    {
        // send_utf8_text 要 NUL 结尾的串，调用方只保证长度
        std::string text((const char *)data, (size_t)nLen);
        ret = emscripten_websocket_send_utf8_text(m_socket, text.c_str());
    }
    else
    {
        ret = emscripten_websocket_send_binary(m_socket, const_cast<void *>(data), (uint32_t)nLen);
    }
    if (ret != EMSCRIPTEN_RESULT_SUCCESS)
    {
        WsWasmLog(3, "sendFrame: send failed, ret=%d", (int)ret);
        return -1;
    }
    return 0;
}

void CWasmWsClient::onOpen()
{
    m_connected = true;
    m_closed = false;
    WsWasmLog(2, "onOpen");
    if (m_pMail)
        m_pMail->enqueue(WSEV_OPEN, NULL, 0, false, 0, NULL);
    else if (m_pListener)
        m_pListener->onConnected();
}

void CWasmWsClient::onError()
{
    m_connected = false;
    WsWasmLog(3, "onError");
    if (m_pMail)
        m_pMail->enqueue(WSEV_ERROR, NULL, 0, false, 0, "websocket error");
    else if (m_pListener)
        m_pListener->onConnError("websocket error");
}

void CWasmWsClient::onClose(unsigned short code, const char *reason)
{
    m_connected = false;
    m_closed = true;
    WsWasmLog(2, "onClose code=%u reason=%s", (unsigned)code, reason ? reason : "");
    if (m_pMail)
        m_pMail->enqueue(WSEV_CLOSE, NULL, 0, false, code, reason);
    else if (m_pListener)
        m_pListener->onDisconnect();
}

void CWasmWsClient::onMessage(const unsigned char *pData, unsigned int nBytes, bool bText)
{
    // 回调返回后 data 就失效（底下是 JS 的 ByteArray 视图），所以拷贝只能留在这里；
    // 拷完就入队，剩下的拆包与监听器调用都在后台线程的 runOne() 里。
    if (m_pMail)
        m_pMail->enqueue(WSEV_DATA, pData, (int)nBytes, bText, 0, NULL);
    else if (m_pListener)
        m_pListener->onDataRecv(pData, (int)nBytes, bText ? FALSE : TRUE);
}

SNSEND
