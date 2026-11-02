// 浏览器原生 WebSocket 版 IWsClient（详见 WsClientWasm.h 的口径说明）。

#include "WsClientWasm.h"
#include <emscripten/websocket.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <string>

SNSBEGIN

namespace
{
WsLogCallback g_wsLog = NULL;
}

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

CWasmWsClient::CWasmWsClient(IConnListener *pListener)
    : m_pListener(pListener)
    , m_socket(0)
    , m_connected(false)
    , m_closed(true)
    , m_bBlockReceive(FALSE)
{
}

CWasmWsClient::~CWasmWsClient()
{
    teardown();
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
    m_pendingRecv.clear();
}

void CWasmWsClient::disconnect()
{
    teardown();
}

BOOL CWasmWsClient::wait(int timeoutMs)
{
    (void)timeoutMs;
    // 事件由 JS 事件循环投递，不在 C 调用栈上；这里没有可等的队列。
    return FALSE;
}

void CWasmWsClient::blockReceive(BOOL bBlock)
{
    m_bBlockReceive = bBlock;
    if (!bBlock)
        flushPending();
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
    if (m_pListener)
        m_pListener->onConnected();
}

void CWasmWsClient::onError()
{
    m_connected = false;
    WsWasmLog(3, "onError");
    if (m_pListener)
        m_pListener->onConnError("websocket error");
}

void CWasmWsClient::onClose(unsigned short code, const char *reason)
{
    m_connected = false;
    m_closed = true;
    WsWasmLog(2, "onClose code=%u reason=%s", (unsigned)code, reason ? reason : "");
    if (m_pListener)
        m_pListener->onDisconnect();
}

void CWasmWsClient::onMessage(const unsigned char *pData, unsigned int nBytes, bool bText)
{
    if (!m_pListener)
        return;
    // 回调返回后 data 就失效（底下是 JS 的 ByteArray 视图），必须立刻拷出来
    std::vector<unsigned char> buf(pData, pData + nBytes);
    deliver(buf, bText);
}

void CWasmWsClient::deliver(const std::vector<unsigned char> &buf, bool bText)
{
    if (m_bBlockReceive)
    {
        PendingMsg msg;
        msg.data = buf;
        msg.bText = bText;
        m_pendingRecv.push_back(msg);
        return;
    }
    if (!m_pListener)
        return;
    const void *pData = buf.empty() ? NULL : (const void *)&buf[0];
    m_pListener->onDataRecv(pData, (int)buf.size(), bText ? FALSE : TRUE);
}

void CWasmWsClient::flushPending()
{
    while (!m_pendingRecv.empty())
    {
        PendingMsg msg = m_pendingRecv.front();
        m_pendingRecv.pop_front();
        deliver(msg.data, msg.bText);
    }
}

SNSEND
