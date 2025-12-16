#include "stdafx.h"
#include "ChessGame.h"

CChessGame::CChessGame(SOUI::IWindow* pParent, SGameTheme* pTheme)
    : m_pParent(pParent)
    , m_pTheme(pTheme)
    , m_pWsClient(nullptr)
{
}

CChessGame::~CChessGame()
{
}

void CChessGame::Init(SOUI::IWindow* pParent, WebSocketClient* pWsClient)
{
    m_pParent = pParent;
    m_pWsClient = pWsClient;
    pWsClient->SetMessageHandler(this);
}

BOOL CChessGame::OnMessage(GS_PKG *pkg)
{
    // 处理游戏相关消息
    return TRUE;
}

void CChessGame::OnConnected()
{
    // 连接成功后的处理
}

void CChessGame::OnDisconnected()
{
    // 断开连接后的处理
}

void CChessGame::OnScaleChanged(int nScale)
{
    // 处理缩放变化
}
