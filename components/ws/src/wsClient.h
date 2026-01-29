#ifndef WsClient_H
#define WsClient_H

#include "Connection.h"
#include <helper/obj-ref-impl.hpp>
#include <string>
#include <thread>
#include <condition_variable>
#include <sstream>
#include <atomic>
#include <mutex>
#include <list>

SNSBEGIN

class WsClient : public TObjRefImpl<IWsClient> {
  public:
    WsClient(IConnListener *pListener);

    ~WsClient();

  public:
    // IConnection methods
    STDMETHODIMP_(int) isValid(THIS) SCONST OVERRIDE
    {
        return m_connected && !m_finished;
    }

    STDMETHODIMP_(int) sendText(THIS_ const char *text, int nLen DEF_VAL(-1)) OVERRIDE
    {
        if (nLen < 0)
            nLen = strlen(text);
        return send(std::string(text, nLen), false);
    }
    
    STDMETHODIMP_(int) sendBinary(THIS_ const void *data, int nLen) OVERRIDE
    {
        return send(std::string((const char *)data, nLen), true);
    }

    STDMETHODIMP_(int) sendBinary2(THIS_ DWORD dwType, const void *data, int nLen) OVERRIDE;
    
    // IWsClient methods
    STDMETHODIMP_(int) connectTo(THIS_ const char *server_, const char *path_, uint16_t port, const char *protocolName_, ClientOption option) OVERRIDE;

    STDMETHODIMP_(BOOL) wait(THIS_ int timeoutMs) OVERRIDE;

    STDMETHODIMP_(void) disconnect(THIS) OVERRIDE;

    STDMETHOD_(void, blockReceive)(THIS_ BOOL bBlock) OVERRIDE;
  private:
    void quit();

    int send(const std::string &text, bool bBinary);

    int genMsgId()
    {
        m_msgId++;
        if (m_msgId < 0)
            m_msgId = 0;
        return m_msgId;
    }

    void run();
    int handler(lws_callback_reasons reasons, void *user, const void *data, std::size_t len);

    IConnListener *m_pListener;
    std::thread m_worker;
    std::atomic_bool m_connected;

    std::string m_server, m_path;
    uint16_t m_port;

    std::string m_protocolName;
    lws_context *m_context;
    lws *m_wsi;

    std::mutex m_mutex;
    std::condition_variable m_cvQuit;
    std::atomic_bool m_finished;

    std::stringstream m_receiveStream;

    struct MsgData
    {
        MsgData(const std::string &buf_, bool bBinary_, int msgId_)
            : buf(buf_)
            , bBinary(bBinary_)
            , msgId(msgId_)
        {
        }
        std::string buf;
        bool bBinary;
        int msgId;
    };
    std::list<MsgData> m_sendingBuf;
    int m_msgId;

    std::list<std::shared_ptr<MsgData> > m_receivingBuf;
    BOOL m_bBlockReceive;
    
    static int cb_lws(lws *websocket, lws_callback_reasons reasons, void *, void *data, std::size_t len);
};

SNSEND

#endif // WsClient_H