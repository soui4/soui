#pragma once

#include "protocol.h"
#include "WebSocketClient.h"
#include <helper/obj-ref-impl.hpp>

class LobbyHandler
    : public SOUI::TObjRefImpl<WebSocketClient::IHandler>
{
public:
    LobbyHandler();
    virtual ~LobbyHandler();

    void Init(SOUI::IWindow* pParent, WebSocketClient* pWsClient);

protected:
    // WebSocketClient::IHandler
    virtual BOOL OnMessage(GS_PKG *pkg) override;

    virtual void OnConnected() override;

    virtual void OnDisconnected() override;

protected:
    void OnBtnCreateRoom();
    void OnBtnJoinRoom();

protected:
    SOUI::IWindow* m_pParent;
    WebSocketClient* m_pWsClient;
};