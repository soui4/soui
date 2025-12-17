#include "stdafx.h"
#include "ChessGame.h"
#include "MainDlg.h"
#include <helper/SFunctor.hpp>
#include <cnchessProtocol.h>
#include "CnchessSkin.h"
#include <helper/slog.h>
#define kLogTag "ChessGame"

#define TIMERID_CLOCK 503

POINT CChessGame::ChessAnchor2Pos(const AnchorPos &pos, const CRect &rcParent, const CSize & szChild, int nScale, void * userData){
    if(pos.type == 10){
        //type == 10 is chess anchor
        CChessGame *pThis = (CChessGame *)userData;
        SASSERT(pos.x.fSize>=0 && pos.x.fSize<9 && pos.y.fSize>=0 && pos.y.fSize<10);
        CPoint pt = pThis->m_ptBoardOrigin;
        pt.x += pThis->m_cellWidth * pos.x.fSize;
        pt.y -= pThis->m_cellHeight * pos.y.fSize;
        pt.x += pos.fOffsetX * szChild.cx;
        pt.y += pos.fOffsetY * szChild.cy;
        return pt;
    }else{
        return SAnchorLayout::DefaultPosition2Point(pos, rcParent, szChild, nScale, userData);
    }
}

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
    SAnchorLayout *pAnchorLayout = sobj_cast<SAnchorLayout>(pGameBoard->GetLayout());
    SASSERT(pAnchorLayout);

    pAnchorLayout->SetPosition2PointCallback(ChessAnchor2Pos,this);

    m_pGameBoard = pGameBoard;
    m_webSocketClient = pWs;
    m_pChessBoard = sobj_cast<SImageWnd>(m_pTheme->GetWidget(Sprites::board_main));
    m_pGameBoard->InsertChild(m_pChessBoard);
    m_pChessBoard->AddRef();
}

void CChessGame::OnGameBoardSizeChanged(IEvtArgs *e)
{
    EventSwndSize *evt = sobj_cast<EventSwndSize>(e);
    SSkinBoard * pSkin = sobj_cast<SSkinBoard>(m_pChessBoard->GetSkin());
    CSize szBoard = m_pChessBoard->GetSkin()->GetSkinSize();
    CRect rcGameGoard = m_pGameBoard->GetClientRect();
    CSize szGameBoard = evt->szWnd;
    float fRatio1 = (float)szGameBoard.cx / szGameBoard.cy;
    float fRatio2 = (float)szBoard.cx / szBoard.cy;
    CRect rcBoard =  rcGameGoard;
    float scale = 1.0f;
    if(fRatio1 > fRatio2)
    {
        int nWid = (int)(szGameBoard.cy * fRatio2);
        rcBoard.DeflateRect((szGameBoard.cx - nWid) / 2, 0);
        scale = (float)nWid / szBoard.cx;
        m_pChessBoard->Move(rcBoard);
    }
    else
    {
        int nHei = (int)(szGameBoard.cx / fRatio2);
        rcBoard.DeflateRect(0, (szGameBoard.cy - nHei) / 2);
        scale = (float)nHei / szBoard.cy;
        m_pChessBoard->Move(rcBoard);
    }
    CRect rcMargin = pSkin->GetMargin();
    CSize szCellAll = szBoard - CSize(rcMargin.left + rcMargin.right,rcMargin.top+rcMargin.bottom);

    //calc cell size
    m_cellWidth = szCellAll.cx * scale / 8;
    m_cellHeight = szCellAll.cy * scale / 9;

    rcMargin.left *= scale;
    rcMargin.top *= scale;
    rcMargin.right *= scale;
    rcMargin.bottom *= scale;
    rcBoard.DeflateRect(rcMargin);
    m_ptBoardOrigin.x = rcBoard.left;
    m_ptBoardOrigin.y = rcBoard.bottom;

    m_pGameBoard->SDispatchMessage(UM_SETSCALE, scale*100, 0);
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