#include "stdafx.h"
#include "ChessGame.h"
#include "MainDlg.h"
#include <helper/SFunctor.hpp>
#include <cnchessProtocol.h>
#include "CnchessSkin.h"
#include "ChessPiece.h"
#include "utils.h"
#include <valueAnimator/SPropertyAnimator.h>
#include <helper/slog.h>
#define kLogTag "ChessGame"

#define TIMERID_CLOCK 503

enum{
    ID_FLAG_BASE = 1000,
    ID_SHADOW_BASE = 2000,
};

POINT CChessGame::ChessAnchor2Pos(const AnchorPos &pos, const CRect &rcParent, const CSize & szChild, int nScale, void * userData){
    if(pos.type == 10){
        //type == 10 is chess anchor
        CChessGame *pThis = (CChessGame *)userData;
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

enum{
    ANI_UP=1,
    ANI_MOVE,
    ANI_DOWN,
    ANI_MOVEDOWN,
};

void CChessGame::ShowPosFlags(POINT ptPiece, BOOL bShow)
{
    // show or hide possible move positions
    POINT ptMoves[MAX_MOVES];
    int nMoves = m_layout.GetPossiableMoves(ptPiece.x, ptPiece.y, ptMoves);
    for (int i = 0; i < nMoves; i++)
    {
        if (m_layout.m_chesses[ptMoves[i].y][ptMoves[i].x] != CHSMAN_NULL)
            continue;
        IWindow *pFlag = m_pGameBoard->FindChildByID(ID_FLAG_BASE + ptMoves[i].y * 9 + ptMoves[i].x);
        pFlag->SetVisible(bShow, TRUE);
    }
}

void CChessGame::onAnimationEnd(IValueAnimator *pAnimator)
{
    SPropertyAnimator *pPropAnimator = sobj_cast<SPropertyAnimator>(pAnimator);
    CChessPiece *pPiece = (CChessPiece *)pPropAnimator->GetTarget();
    if(pPropAnimator->GetID() == ANI_MOVE ){
        //start a new animation to move the piece down
        auto pAnim = Util::OffsetSprite(pPiece, -0.1f, 0.1f, 100);
        pAnim->SetID(ANI_MOVEDOWN);
        pAnim->addListener(this);
    }else if(pPropAnimator->GetID() == ANI_DOWN){
        //set the piece to normal state
        pPiece->SetPicesState(CChessPiece::STATE_NORMAL);
        pPiece->SetLayer(1);//restore layer
        ShowPosFlags(pPiece->GetPos(), FALSE);
        m_nSelectedChessID = -1;
    }else if(pPropAnimator->GetID() == ANI_MOVEDOWN){
        //move the piece down
        POINT ptPiece = pPiece->GetPos();
        POINT ptTarget = pPiece->GetTarget();
        //move the piece to the layout.
        MOVESTEP moveStep = m_layout.Move(ptPiece,ptTarget);
        if(moveStep.nEnemyID != 0){
            CChessPiece *pEnemyPiece = (CChessPiece *)m_pGameBoard->FindChildByID(moveStep.nEnemyID);
            pEnemyPiece->Destroy();
        }

        pPiece->SetPicesState(CChessPiece::STATE_NORMAL);
        pPiece->SetLayer(1);
        pPiece->SetPos(ptTarget);
        pPiece->SetTarget(CPoint(-1,-1));
        m_nSelectedChessID = -1;

        //swap side
        m_bRedSide = !m_bRedSide;
    }
}

BOOL CChessGame::OnChessPieceClick(IEvtArgs *e)
{
    if(m_nSelectedChessID != -1)
    {
        SWindow *pTarget = sobj_cast<SWindow>(e->Sender());
        if(pTarget->GetID() == m_nSelectedChessID)
        {//cancel the selection
            CChessPiece *pSelPiece = (CChessPiece *)pTarget;
            auto pAni = Util::ScaleSprite(pSelPiece, 1.0f, 1.0f, 100);
            pAni->SetID(ANI_DOWN);
            pAni->addListener(this);
            m_nSelectedChessID = -1;
        }else{
            //move the selected piece to the target position
            CPoint ptTarget;
            if(pTarget->IsClass(CChessPiece::GetClassName())){
                CChessPiece *pTargetPiece = (CChessPiece *)pTarget;
                ptTarget = pTargetPiece->GetPos();
            }else{
                ULONG_PTR lp = pTarget->GetUserData();
                ptTarget=CPoint(GET_X_LPARAM(lp),GET_Y_LPARAM(lp));
            }
            CChessPiece *pSelPiece = (CChessPiece *)m_pGameBoard->FindChildByID(m_nSelectedChessID);
            SASSERT(pSelPiece);
            ShowPosFlags(pSelPiece->GetPos(), FALSE);
            pSelPiece->SetTarget(ptTarget);
            pSelPiece->SetLayer(3);

            SAnchorLayoutParam *pParam = (SAnchorLayoutParam*)pTarget->GetLayoutParam();
            SAnchorLayoutParamStruct *pParamStruct = (SAnchorLayoutParamStruct*)pParam->GetRawData();
            AnchorPos pos = pParamStruct->pos;
            pos.x.fSize += 0.1f;
            pos.y.fSize += -0.1f;
            auto pAnim = Util::MoveSpriteTo(pSelPiece, pos, 200);
            pAnim->SetID(ANI_MOVE);
            pAnim->addListener(this);
            m_nSelectedChessID = -1;
        }
    }else{
        CChessPiece *pTarget = sobj_cast<CChessPiece>(e->Sender());
        //select the piece that is on the same side as the player
        BOOL bRedSide = pTarget->IsRedSide();
        if(bRedSide == m_bRedSide){
            if(pTarget->GetPicesState() == CChessPiece::STATE_NORMAL)
            {//raise up the piece
                pTarget->SetPicesState(CChessPiece::STATE_UP);
                pTarget->SetLayer(SGameTheme::LAYER_PIESE_UP);
                //scale the piece by 110% of cell size for x and y
                Util::ScaleSprite(pTarget, 1.1f, 1.1f, 100);
                m_nSelectedChessID = pTarget->GetID();
            }
            else if(pTarget->GetPicesState() == CChessPiece::STATE_UP)
            {//drop down the piece
                auto pAnim = Util::ScaleSprite(pTarget, 1.0f, 1.0f, 100);
                pAnim->SetID(ANI_DOWN);
                pAnim->addListener(this);
                m_nSelectedChessID = -1;
            }

            //show or hide possible move positions
            ShowPosFlags(pTarget->GetPos(), m_nSelectedChessID != -1);
        }
    }
    return TRUE;
}
void CChessGame::Init(SWindow *pGameHost, WebSocketClient *pWs)
{
    SWindow* pGameBoard = pGameHost->FindChildByName(L"chessboard");
    SAnchorLayout *pAnchorLayout = sobj_cast<SAnchorLayout>(pGameBoard->GetLayout());
    SASSERT(pAnchorLayout);

    pAnchorLayout->SetPosition2PointCallback(ChessAnchor2Pos,this);

    m_pGameBoard = pGameBoard;
    m_webSocketClient = pWs;
    m_pChessBoard = sobj_cast<SImageWnd>(m_pTheme->GetWidget(Sprites::board_main));
    m_pGameBoard->InsertChild(m_pChessBoard);
    m_pChessBoard->AddRef();

    m_bRedSide = TRUE;
    m_nSelectedChessID = -1;
    m_layout.InitLayout(NULL,m_bRedSide?CS_RED:CS_BLACK);
    //init chess layout
    SXmlNode xmlPiece = m_pTheme->GetTemplate(Template::kChessPiece);
    SStringW clsPieceName = xmlPiece.attribute(L"wndclass").as_string(L"chesspiece");
    SXmlNode xmlShadow = m_pTheme->GetTemplate(Template::kShadow);
    SStringW clsShadowName = xmlShadow.attribute(L"wndclass").as_string(L"img");

    SXmlNode xmlPosFlag = m_pTheme->GetTemplate(Template::kPosFlag);
    SStringW flgClsName = xmlPosFlag.attribute(L"wndclass").as_string(L"img");
    auto slot = Subscriber(&CChessGame::OnChessPieceClick, this);
    for(int y=0;y<10;y++) for(int x=0;x<9;x++)
    {
        {//init pos flag
            IWindow *pFlag = SApplication::getSingletonPtr()->CreateWindowByName(flgClsName);
            pFlag->InitFromXml(&xmlPosFlag);
            SAnchorLayoutParam *pParam = (SAnchorLayoutParam*)pFlag->GetLayoutParam();
            SAnchorLayoutParamStruct *pParamStruct = (SAnchorLayoutParamStruct*)pParam->GetRawData();
            pParamStruct->pos.type = 10;
            pParamStruct->pos.x.fSize = x;
            pParamStruct->pos.y.fSize = y;
            m_pGameBoard->InsertIChild(pFlag);
            pFlag->SetID(ID_FLAG_BASE + y * 9 + x);
            pFlag->SetUserData(MAKELPARAM(x,y));
            pFlag->SetVisible(FALSE,TRUE);
            pFlag->SubscribeEvent(EventCmd::EventID, &slot);
        }

        if(m_layout.m_chesses[y][x] == CHSMAN_NULL) 
            continue;
        {//init chess piece
            CChessPiece *pPiece = (CChessPiece *)SApplication::getSingletonPtr()->CreateWindowByName(clsPieceName);
            pPiece->InitFromXml(&xmlPiece);
            pPiece->SetChessMan(m_layout.m_chesses[y][x]);
            SAnchorLayoutParam *pParam = (SAnchorLayoutParam*)pPiece->GetLayoutParam();
            SAnchorLayoutParamStruct *pParamStruct = (SAnchorLayoutParamStruct*)pParam->GetRawData();
            pParamStruct->pos.type = 10;
            pParamStruct->pos.x.fSize = x;
            pParamStruct->pos.y.fSize = y;
            pPiece->SetID(m_layout.m_nChsID[y][x]);
            pPiece->SetPos(CPoint(x,y));
            m_pGameBoard->InsertChild(pPiece);
            pPiece->SubscribeEvent(EventCmd::EventID, &slot);
        }
        {//init shadow
            IWindow *pShadow = SApplication::getSingletonPtr()->CreateWindowByName(clsShadowName);
            pShadow->InitFromXml(&xmlShadow);
            SAnchorLayoutParam *pParam = (SAnchorLayoutParam*)pShadow->GetLayoutParam();
            SAnchorLayoutParamStruct *pParamStruct = (SAnchorLayoutParamStruct*)pParam->GetRawData();
            pParamStruct->pos.type = 10;
            pParamStruct->pos.x.fSize = x;
            pParamStruct->pos.y.fSize = y;
            pShadow->SetPivot(0.5f,0.0f);
            pShadow->SetID(ID_SHADOW_BASE + m_layout.m_nChsID[y][x]);
            m_pGameBoard->InsertIChild(pShadow);
        }
    }

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

    m_pGameBoard->SDispatchMessage(UM_SETSCALE, scale*100, 1);
}

void CChessGame::OnTimer(UINT_PTR uIDEvent)
{
    SetMsgHandled(FALSE);
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