#include "stdafx.h"
#include "LobbyHandler.h"

LobbyHandler::LobbyHandler()
    : m_pParent(nullptr)
    , m_pWsClient(nullptr)
{
}

LobbyHandler::~LobbyHandler()
{
}

void LobbyHandler::Init(SOUI::IWindow* pParent, WebSocketClient* pWsClient)
{
    m_pParent = pParent;
    m_pWsClient = pWsClient;
}

BOOL LobbyHandler::OnMessage(GS_PKG *pkg)
{
    // 处理大厅相关消息
    return TRUE;
}

void LobbyHandler::OnConnected()
{
    // 连接成功后的处理
}

void LobbyHandler::OnDisconnected()
{
    // 断开连接后的处理
}

void LobbyHandler::OnBtnCreateRoom()
{
    // 创建房间
}

void LobbyHandler::OnBtnJoinRoom()
{
    // 加入房间
}