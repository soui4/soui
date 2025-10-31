#ifndef WS_CONNECTION_H
#define WS_CONNECTION_H

#include <mutex>
#include <libwebsockets.h>
#include <sstream>
#include <string>
#include <list>
#include <helper/obj-ref-impl.hpp>
#include <interface/ws-i.h>

SNSBEGIN

class SvrConnection : public TObjRefImpl<ISvrConnection> {
  public:
    SvrConnection(lws_context *ctx, lws *socket, ISvrListener *pSvrListener);
    ~SvrConnection();

    // IConnection methods
    STDMETHODIMP_(int) isValid(THIS) SCONST OVERRIDE;
    STDMETHODIMP_(int) sendText(THIS_ const char *text, int nLen DEF_VAL(-1)) OVERRIDE;
    STDMETHODIMP_(int) sendBinary(THIS_ const void *data, int nLen) OVERRIDE;
    
    // ISvrConnection methods
    STDMETHODIMP_(void) close(THIS_ const char* reason) OVERRIDE;

    STDMETHODIMP_(void) setId(THIS_ int id) OVERRIDE;
    STDMETHODIMP_(int) getId(THIS) SCONST OVERRIDE;
    STDMETHODIMP_(void) setGroupId(THIS_ int id) OVERRIDE;
    STDMETHODIMP_(int) getGroupId(THIS) SCONST OVERRIDE;
    
    int send(const std::string &text, bool bBinary);
    void onRecv(const std::string &message, bool isLastMessage, bool bBinary);
    void sendBuf();

  private:
    int genMsgId()
    {
        m_msgId++;
        if (m_msgId < 0)
            m_msgId = 0;
        return m_msgId;
    }
    int m_id,m_groupId;
    lws *m_socket;
    lws_context * m_context;
    std::mutex m_mutex;

    struct MsgData
    {
        std::string buf;
        bool bBinary;
        int msgId;
    };

    std::list<MsgData> sendingBuf;
    int m_msgId;
    std::stringstream receiveStream;
    SAutoRefPtr< ISvrListener> m_svrListener;

};
SNSEND
#endif // WS_CONNECTION_H