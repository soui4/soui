#ifndef __WS_CLIENT_WASM_H__
#define __WS_CLIENT_WASM_H__

#include <interface/ws-i.h>
#include <helper/obj-ref-impl.hpp>
#include <emscripten/websocket.h>
#include <deque>
#include <vector>

SNSBEGIN

/**
 * @brief 浏览器原生 WebSocket 版的 IWsClient
 *
 * 与桌面版（lws + 工作线程）的差别：
 *   - 没有工作线程，也没有 lws 事件循环。连接、收发全部走 Emscripten 的
 *     emscripten_websocket_*（底层就是页面的 WebSocket 对象），回调由 JS 事件
 *     循环送到"创建 socket 的线程"，也就是主线程——正好是 SOUI 的 UI 线程；
 *   - 消息数据只在回调期间有效，因此必须立刻拷出来再交给 IConnListener；
 *   - wait() 没有可等的队列（事件在 JS 事件循环里到达，不在 C 栈上），恒返回 FALSE。
 *     调用方要么用 blockReceive 缓存、要么像 cnchess 那样在自己的监听器里排队；
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

    void onOpen();
    void onError();
    void onClose(unsigned short code, const char *reason);
    void onMessage(const unsigned char *pData, unsigned int nBytes, bool bText);

    void flushPending();
    void deliver(const std::vector<unsigned char> &buf, bool bText);

    IConnListener *m_pListener;
    EMSCRIPTEN_WEBSOCKET_T m_socket;
    bool m_connected;
    bool m_closed;
    BOOL m_bBlockReceive;

    struct PendingMsg
    {
        std::vector<unsigned char> data;
        bool bText;
    };
    std::deque<PendingMsg> m_pendingRecv;
};

/** 日志回调（与桌面版 IWebsocket::SetLogCallback 同义） */
void WsWasmSetLogCallback(WsLogCallback cb);
void WsWasmLog(int level, const char *fmt, ...);

SNSEND

#endif // __WS_CLIENT_WASM_H__
