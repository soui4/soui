#ifndef _WS_CLIENT_LISTENER_H_
#define _WS_CLIENT_LISTENER_H_

#include "WebSocketClient.h"
#include <list>

class WsClientListener : public WebSocketClient::IListener{

public:
    WsClientListener();
    ~WsClientListener();

    void BLockReceive(BOOL bBlock);
protected:
    virtual BOOL _OnMessage(DWORD dwType, WebSocketClient::MSGDATA data) = 0;
public:
    BOOL OnMessage(DWORD dwType, WebSocketClient::MSGDATA data) override;
private:
    void PumpMessage();

    std::list < std::pair<DWORD, WebSocketClient::MSGDATA> > m_lstPendingMsg;
    BOOL m_bMsgBlock;
};

#endif//_WS_CLIENT_LISTENER_H_