#include "stdafx.h"
#include "WebSocketClient.h"
#include "protocol.h"
#include <helper/slog.h>
#define kLogTag "WebSocketClient"

WebSocketClient::WebSocketClient()
    : m_pListener(nullptr)
{
}

WebSocketClient::~WebSocketClient()
{
    Close();
}

BOOL WebSocketClient::ConnectToServer(LPCSTR strUrl, LPCSTR strProtocol)
{
    SLOGI() << "connecting to " << strUrl;
    Close();
    SComMgr2* pComMgr = SApplication::getSingletonPtr()->GetComMgr2();
    SASSERT(pComMgr);
    ISvrConnection* pConn = nullptr;
    pComMgr->CreateWebsocketConnection(&pConn);
    if (!pConn)
    {
        SLOGE() << "create websocket connection failed!";
        return FALSE;
    }
    m_pConn.Attach(pConn);
    BOOL bRet = m_pConn->connect(strUrl, strProtocol, this);
    if (!bRet)
    {
        SLOGE() << "connect to server failed!";
        m_pConn = nullptr;
    }
    return bRet;
}

void WebSocketClient::Close()
{
    if (m_pConn)
    {
        m_pConn->disconnect();
        m_pConn = nullptr;
    }
}

void WebSocketClient::SetMessageHandler(IHandler* pHandler)
{
    m_pHandler = pHandler;
}

void WebSocketClient::SetMessageHandler(IListener* pListener)
{
    m_pListener = pListener;
}

BOOL WebSocketClient::SendBinary(DWORD dwType, const void* pData, DWORD dwLen)
{
    if (!m_pConn) return FALSE;
    return m_pConn->sendBinary2(dwType, pData, dwLen);
}

BOOL WebSocketClient::SendText(LPCSTR lpText)
{
    if (!m_pConn) return FALSE;
    return m_pConn->sendText(lpText);
}