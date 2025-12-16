#pragma once

#include <interface/ws-i.h>
#include <helper/obj-ref-impl.hpp>
#include <string>
#include <vector>
#include <memory>

struct GS_PKG;

class WebSocketClient
{
public:
    struct IHandler : IObjRef
    {
        virtual BOOL OnMessage(GS_PKG *pkg) = 0;
        virtual void OnConnected() = 0;
        virtual void OnDisconnected() = 0;
    };

    struct IListener
    {
        virtual BOOL OnMessage(DWORD dwType, std::shared_ptr<std::vector<BYTE> > data) = 0;
    };

public:
    WebSocketClient();
    ~WebSocketClient();

    BOOL ConnectToServer(LPCSTR strUrl, LPCSTR strProtocol);
    void Close();

    void SetMessageHandler(IHandler* pHandler);
    void SetMessageHandler(IListener* pListener);

    BOOL SendBinary(DWORD dwType, const void* pData, DWORD dwLen);
    BOOL SendText(LPCSTR lpText);

protected:
    SAutoRefPtr<ISvrConnection> m_pConn;
    SAutoRefPtr<IHandler>       m_pHandler;
    IListener*                  m_pListener;
};