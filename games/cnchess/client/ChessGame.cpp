#include "stdafx.h"
#include "ChessGame.h"
#include "MainDlg.h"
#include <helper/SFunctor.hpp>
#include <cnchessProtocol.h>
#include <helper/slog.h>
#define kLogTag "ChessGame"

#define TIMERID_CLOCK 503

/**
 * @brief 构造函数
 * @param pMainDlg 主窗口指针
 */
CChessGame::CChessGame(CMainDlg* pMainDlg, SGameTheme* pTheme)
    : m_pMainDlg(pMainDlg)
    ,m_stage(STAGE_INIT)
    ,m_pTheme(pTheme)
    ,m_iSelfIndex(0)
    ,m_iBankerIndex(-1)
    ,m_roundResult(RESULT_UNKNOWN)
{
    memset(m_pUserInfo, 0, sizeof(m_pUserInfo));
    
     // 初始化准备状态
    for(int i = 0; i < PLAYER_COUNT; i++)
    {
        m_bReady[i] = FALSE;
    }
    
    memset(m_dwProps, 0, sizeof(m_dwProps));
}

/**
 * @brief 析构函数
 */
CChessGame::~CChessGame()
{
}

void CChessGame::Init(SWindow *pGameBoard, WebSocketClient *pWs)
{
    m_pGameBoard = pGameBoard;
    m_webSocketClient = pWs;
}

void WINAPI CChessGame::OnAnimatorGroupEnd(IAnimatorGroup * pGroup, int nID)
{
}

void CChessGame::OnConnected()
{
    SLOGI() << "Connected to server";
}

void CChessGame::OnDisconnected()
{
    SLOGI() << "Disconnected from server";
}

BOOL CChessGame::OnMessage(DWORD dwType, std::shared_ptr<std::vector<BYTE> > data)
{
    SLOGI() << "Received message, type: " << dwType << ", size: " << data->size();
    
    // switch(dwType)
    // {
    // case GAME_CNCHESS_GAMESTART:
    //     OnGameStart(data->data(), data->size());
    //     break;
        
    // case GAME_CNCHESS_MOVE:
    //     OnMoveChess(data->data(), data->size());
    //     break;
        
    // case GAME_CNCHESS_UNDOREQ:
    //     OnUndoRequest(data->data(), data->size());
    //     break;
        
    // case GAME_CNCHESS_UNDOACK:
    //     OnUndoResponse(data->data(), data->size());
    //     break;
        
    // case GAME_CNCHESS_GAMEOVER:
    //     OnGameOver(data->data(), data->size());
    //     break;
        
    // case GS_CMD_LOGIN_ACK:
    //     OnLoginAck(data->data(), data->size());
    //     break;
        
    // case GS_CMD_AVATAR_ACK:
    //     OnAvatarAck(data->data(), data->size());
    //     break;
        
    // default:
    //     SLOGW() << "Unknown message type: " << dwType;
    //     break;
    // }
    
    return TRUE;
}

void CChessGame::OnGameStart(const void *pData, int nSize)
{
    SLOGI() << "Game start";
    m_stage = STAGE_PLAYING;
}

void CChessGame::OnMoveChess(const void *pData, int nSize)
{
    SLOGI() << "Move chess";
}

void CChessGame::OnUndoRequest(const void *pData, int nSize)
{
    SLOGI() << "Undo request";
}

void CChessGame::OnUndoResponse(const void *pData, int nSize)
{
    SLOGI() << "Undo response";
}

void CChessGame::OnGameOver(const void *pData, int nSize)
{
    SLOGI() << "Game over";
    m_stage = STAGE_CONTINUE;
}

void CChessGame::OnLoginAck(const void *pData, int nSize)
{
    SLOGI() << "Login ack";
}

void CChessGame::OnAvatarAck(const void *pData, int nSize)
{
    SLOGI() << "Avatar ack";
}

void CChessGame::UpdateClock(int nSecond)
{
    SLOGI() << "Update clock, seconds: " << nSecond;
}

void CChessGame::PlayTip(const SStringT &strTip)
{
    SLOGI() << "Play tip: " << strTip;
}

void CChessGame::wsSendMsg(DWORD dwType, LPVOID lpData, DWORD dwSize)
{
    if(m_webSocketClient)
    {
        m_webSocketClient->SendMsg(dwType, lpData, dwSize);
    }
}

void CChessGame::PlayEffectSound(LPCWSTR pszSound)
{
    SLOGI() << "Play effect sound: " << pszSound;
}