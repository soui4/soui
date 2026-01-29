#include "stdafx.h"
#include "ChessGame.h"
#include "MainDlg.h"
#include <helper/SFunctor.hpp>
#include <cnchessProtocol.h>
#include "CnchessSkin.h"
#include "ChessPiece.h"
#include "utils.h"
#include "PeaceReqDlg.h"
#include "PeaceAckDlg.h"
#include <valueAnimator/SPropertyAnimator.h>
#include <algorithm>
#include <helper/slog.h>
#define kLogTag "ChessGame"

//#define ENABLE_MOCK
 
#define TIMERID_CLOCK_ME 503
#define TIMERID_CLOCK_ENEMY 504


static const float kPieceScale = 1.1f;
enum{
    kShadowHeight_Normal = 86,
    kShadowHeight_Up = 110,
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
    }else if(pos.type == 11){
        //shadow anchor
        CChessGame *pThis = (CChessGame *)userData;
        CPoint pt = pThis->m_ptBoardOrigin;
        pt.x += pThis->m_cellWidth * pos.x.fSize;
        pt.y -= pThis->m_cellHeight * pos.y.fSize;
        pt.x += pos.fOffsetX * szChild.cx;
        pt.y += pos.fOffsetY * szChild.cx;//
        return pt;
    }
    else{
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
    ,m_iRedIndex(-1)
    ,m_roundResult(RESULT_DRAW)
    ,m_LytState(&m_layout)
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
    ANI_UP_MOVE,
    ANI_DOWN,
    ANI_MOVEDOWN,
    ANI_SHADOW_RESTORE,
    ANI_TIP,
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
    IPropertyAnimator *pPropAnimator = sobj_cast<IPropertyAnimator>(pAnimator);
    if (pPropAnimator->GetID() == ANI_MOVE)
    {
        CChessPiece *pPiece = (CChessPiece *)pPropAnimator->GetTarget();
        // start a new animation to move the piece down
        SLOGI()<<"restore piece scale: "<< 1.0f;
        auto pAni = Util::ScaleSprite(pPiece,1.0f, 1.0f/kPieceScale, 200);
        pAni->SetID(ANI_MOVEDOWN);
        pAni->addListener(this);

        // restore shadow height
        SWindow *pShadow = m_pGameBoard->FindChildByID(ID_SHADOW_BASE + pPiece->GetID()-ID_CHESS_BASE);
        CRect rc = pShadow->GetWindowRect();
        SLOGI()<<"restore shadow height: "<< kShadowHeight_Normal;
        Util::ChangeSpriteHeight(pShadow, kShadowHeight_Up, kShadowHeight_Normal, 200);
    }
    else if (pPropAnimator->GetID() == ANI_DOWN)
    {
        CChessPiece *pPiece = (CChessPiece *)pPropAnimator->GetTarget();
        //set the piece to normal state
        pPiece->SetPicesState(CChessPiece::STATE_NORMAL);
        pPiece->SetLayer(SGameTheme::LAYER_PIESE);//restore layer
    }
    else if (pPropAnimator->GetID() == ANI_MOVEDOWN)
    {
        CChessPiece *pPiece = (CChessPiece *)pPropAnimator->GetTarget();
        //move the piece down
        POINT ptPiece = pPiece->GetPos();
        POINT ptTarget = pPiece->GetTarget();

        //restore piece state
        pPiece->SetPicesState(CChessPiece::STATE_NORMAL);
        pPiece->SetLayer(SGameTheme::LAYER_PIESE);
        pPiece->SetPos(ptTarget);
        pPiece->SetTarget(CPoint(-1, -1));

        //move the piece to the layout.
        if(std::shared_ptr<MOVESTEP> step = pPiece->GetUndoMove()){
            IWindow *pFlagTo = m_pTheme->GetWidget(Widgets::kflag_pos_to);
            pFlagTo->SetVisible(FALSE,TRUE);
            IWindow *pFlagFrom = m_pTheme->GetWidget(Widgets::kflag_pos_from);
            pFlagFrom->SetVisible(FALSE,TRUE);

            if(step->nEnemyID != 0){
                int scale = pPiece->GetScale();
                SXmlNode xmlPiece = m_pTheme->GetTemplate(Template::kChessPiece);
                SStringW clsPieceName = xmlPiece.attribute(L"wndclass").as_string(L"chesspiece");
                SXmlNode xmlShadow = m_pTheme->GetTemplate(Template::kShadow);
                SStringW clsShadowName = xmlShadow.attribute(L"wndclass").as_string(L"img");
                {//restore enemy piece
                    auto slot = Subscriber(&CChessGame::OnChessPieceClick, this);
                    CChessPiece *pPiece = (CChessPiece *)SApplication::getSingletonPtr()->CreateWindowByName(clsPieceName);
                    pPiece->InitFromXml(&xmlPiece);
                    pPiece->SetChessMan(step->enemy);
                    SAnchorLayoutParam *pParam = (SAnchorLayoutParam*)pPiece->GetLayoutParam();
                    SAnchorLayoutParamStruct *pParamStruct = (SAnchorLayoutParamStruct*)pParam->GetRawData();
                    pParamStruct->pos.type = 10;
                    pParamStruct->pos.x.fSize = step->pt2.x;
                    pParamStruct->pos.y.fSize = step->pt2.y;
                    pPiece->SetID(ID_CHESS_BASE + step->nEnemyID);
                    pPiece->SetPos(step->pt2);
                    m_pGameBoard->InsertChild(pPiece);
                    pPiece->SubscribeEvent(EventCmd::EventID, &slot);
                    pPiece->SDispatchMessage(UM_SETSCALE, scale, 1);//note: use the same scale as the original piece
                }
                {//restore enemy shadow
                    IWindow *pShadow = SApplication::getSingletonPtr()->CreateWindowByName(clsShadowName);
                    pShadow->InitFromXml(&xmlShadow);
                    SAnchorLayoutParam *pParam = (SAnchorLayoutParam*)pShadow->GetLayoutParam();
                    SAnchorLayoutParamStruct *pParamStruct = (SAnchorLayoutParamStruct*)pParam->GetRawData();
                    pParamStruct->pos.type = 11;
                    pParamStruct->pos.x.fSize = step->pt2.x;
                    pParamStruct->pos.y.fSize = step->pt2.y;
                    pShadow->SetPivot(0.5f,0.0f);
                    pShadow->SetID(ID_SHADOW_BASE + step->nEnemyID);
                    m_pGameBoard->InsertIChild(pShadow);
                    pShadow->SDispatchMessage(UM_SETSCALE, scale, 1);//note: use the same scale as the original piece
                }
            }
            pPiece->SetUndoMove(nullptr);
            OnSetActivePlayerIndex(GetActivePlayerIndex()==m_iSelfIndex?0:1);
            if(--m_nUndoLeft > 0){
                UndoLastMove();
            }
        }else{
            MOVESTEP moveStep = m_layout.Move(ptPiece,ptTarget);
            OnChessMove(moveStep,TRUE);
            if(m_LytState.IsJiangJun(m_layout.m_actSide))
                PlayEffectSound(Sounds::Effects::kJiangjun);
            else if(moveStep.nEnemyID==0)
                PlayEffectSound(Sounds::Effects::kGo);
            else
                PlayEffectSound(Sounds::Effects::kEat);

            if(moveStep.nEnemyID != 0){
                CChessPiece *pEnemyPiece = (CChessPiece *)m_pGameBoard->FindChildByID(ID_CHESS_BASE+moveStep.nEnemyID);
                pEnemyPiece->Destroy();
                SWindow *pShadow = m_pGameBoard->FindChildByID( ID_SHADOW_BASE + moveStep.nEnemyID);
                pShadow->Destroy();
            }

            IWindow *pFlagTo = m_pTheme->GetWidget(Widgets::kflag_pos_to);
            SAnchorLayoutParam *pParam = (SAnchorLayoutParam *)pFlagTo->GetLayoutParam();
            pParam->pos.x.fSize = ptTarget.x;
            pParam->pos.y.fSize = ptTarget.y;
            pFlagTo->RequestRelayout();
            pFlagTo->SetVisible(TRUE,TRUE);

            OnSetActivePlayerIndex(GetActivePlayerIndex()==m_iSelfIndex?0:1);
            if(!pPiece->IsLocalMove()){
                pPiece->SetLocalMove(TRUE);
                // send the move to server
                MSG_MOVE msgMove;
                msgMove.bLocal = FALSE;
                msgMove.dwRoundTime = 0;
                msgMove.iIndex = m_iSelfIndex;
                msgMove.ptBegin = PtLocal2Net(ptPiece);
                msgMove.ptEnd = PtLocal2Net(ptTarget);
                wsSendMsg(MSG_REQ_MOVE, &msgMove, sizeof(msgMove));
            }
        }

    }else if(pPropAnimator->GetID() == ANI_UP_MOVE){
        CChessPiece *pPiece = (CChessPiece *)pPropAnimator->GetTarget();
        POINT ptTarget = pPiece->GetTarget();
        MovePiece(pPiece->GetID()-ID_CHESS_BASE, ptTarget);
    }
    else if(pPropAnimator->GetID() == ANI_TIP)
    {
        IWindow *pTip = pPropAnimator->GetTarget();
        pTip->Destroy();
    }
}

void CChessGame::SelectPiece(int nId)
{
    CChessPiece *pTarget = m_pGameBoard->FindChildByID2<CChessPiece>(ID_CHESS_BASE+nId);
    pTarget->SetPicesState(CChessPiece::STATE_UP);
    pTarget->SetLayer(SGameTheme::LAYER_PIESE_UP);
    // scale the piece by 110% of cell size for x and y
    Util::ScaleSprite(pTarget, 1.0f, kPieceScale, 200);

    //show possible move positions
    ShowPosFlags(pTarget->GetPos(), TRUE);

    // increase shadow height
    SWindow *pShadow = m_pGameBoard->FindChildByID(ID_SHADOW_BASE + nId);
    Util::ChangeSpriteHeight(pShadow, kShadowHeight_Normal, kShadowHeight_Up, 200);

    m_nSelectedChessID = nId;
    SLOGI() << "scale piece to: " << kPieceScale << " id=" << m_nSelectedChessID;
}

void CChessGame::DeselectPiece(int nId)
{
    CChessPiece *pSelPiece = m_pGameBoard->FindChildByID2<CChessPiece>(ID_CHESS_BASE+nId);
    SLOGI() << "chess piece clicked, id=" << m_nSelectedChessID << " restore scale to 1.0";
    auto pAni = Util::ScaleSprite(pSelPiece, 1.0f, 1.0f / kPieceScale, 200);
    pAni->SetID(ANI_DOWN);
    pAni->addListener(this);
    //hide possible move positions
    ShowPosFlags(pSelPiece->GetPos(), FALSE);

    // restore shadow height
    SWindow *pShadow = m_pGameBoard->FindChildByID(ID_SHADOW_BASE + nId);
    CRect rc = pShadow->GetWindowRect();
    pAni = Util::ChangeSpriteHeight(pShadow, kShadowHeight_Up, kShadowHeight_Normal, 200);
    pAni->SetID(ANI_SHADOW_RESTORE);
    pAni->addListener(this);
    m_nSelectedChessID = -1;
}

void CChessGame::MovePiece(int nId, POINT ptTarget)
{
    CChessPiece *pSelPiece = m_pGameBoard->FindChildByID2<CChessPiece>(ID_CHESS_BASE+nId);
    // move the selected piece to the target position
    SLOGI() << "move piece to: " << ptTarget.x << "," << ptTarget.y << " id=" << nId;
    SASSERT(pSelPiece);
    ShowPosFlags(pSelPiece->GetPos(), FALSE);
    pSelPiece->SetTarget(ptTarget);
    pSelPiece->SetLayer(SGameTheme::LAYER_PIESE);
    SAnchorLayoutParam *pParam = (SAnchorLayoutParam *)pSelPiece->GetLayoutParam();
    AnchorPos pos = pParam->pos;
    pos.x.fSize = ptTarget.x;
    pos.y.fSize = ptTarget.y;
    // move the piece to the target position
    auto pAnim = Util::MoveSpriteTo(pSelPiece, pos, 200);
    pAnim->SetID(ANI_MOVE);
    pAnim->addListener(this);
    // move the shadow to the target position
    SWindow *pShadow = m_pGameBoard->FindChildByID(ID_SHADOW_BASE + nId);
    pParam = (SAnchorLayoutParam *)pShadow->GetLayoutParam();
    pos = pParam->pos;
    pos.x.fSize = ptTarget.x;
    pos.y.fSize = ptTarget.y;
    Util::MoveSpriteTo(pShadow, pos, 200);

    IWindow *pFlagFrom = m_pTheme->GetWidget(Widgets::kflag_pos_from);
    pParam = (SAnchorLayoutParam *)pFlagFrom->GetLayoutParam();
    pParam->pos.x.fSize = pSelPiece->GetPos().x;
    pParam->pos.y.fSize = pSelPiece->GetPos().y;
    pFlagFrom->RequestRelayout();
    pFlagFrom->SetVisible(TRUE,TRUE);

    m_nSelectedChessID = -1;
}

void CChessGame::SelectAndMovePiece(int nId, POINT pt)
{ 
    CChessPiece *pTarget = m_pGameBoard->FindChildByID2<CChessPiece>(ID_CHESS_BASE+nId);
    pTarget->SetPicesState(CChessPiece::STATE_UP);
    pTarget->SetLayer(SGameTheme::LAYER_PIESE_UP);
    // scale the piece by 110% of cell size for x and y
    auto pAni = Util::ScaleSprite(pTarget, 1.0f, kPieceScale, 200);

    pAni->SetID(ANI_UP_MOVE);
    pTarget->SetTarget(pt);
    pTarget->SetLocalMove(TRUE); // mark this is a local move, so we don't send the move to server after animation
    pAni->addListener(this);

    //show possible move positions
    ShowPosFlags(pTarget->GetPos(), TRUE);

    // increase shadow height
    SWindow *pShadow = m_pGameBoard->FindChildByID(ID_SHADOW_BASE + nId);
    Util::ChangeSpriteHeight(pShadow, kShadowHeight_Normal, kShadowHeight_Up, 200);

    m_nSelectedChessID = nId;   
}

BOOL CChessGame::CheckMove(POINT ptFrom, POINT ptTo, BOOL bSilent){
    BOOL bValidMove = FALSE;

    POINT ptMoves[MAX_MOVES];
    int nMoves = m_layout.GetPossiableMoves(ptFrom, ptMoves);
    for (int i = 0; i < nMoves; i++)
    {
        if (ptMoves[i].x == ptTo.x && ptMoves[i].y == ptTo.y)
        {
            bValidMove = TRUE;
            break;
        }
    }
    if (!bValidMove)
    {
        return FALSE;
    }

    CHSSIDE actSide=m_layout.m_actSide;
    BOOL bJiangJun = m_LytState.IsJiangJun(actSide); // 检查自己是否在被将军状态
    // 检查走棋后的效果，包括自杀及长捉
    // 备分当前状态
    CChsLytState lytStateBackup(&m_layout);
    lytStateBackup.Copy(&m_LytState);

    MOVESTEP ms = m_layout.Move(ptFrom, ptTo);
    OnChessMove(ms, FALSE);

    SStringT strTip;
    if (m_LytState.IsJiangJun(actSide))
    { // 检测到将军状态,显示走棋到对自己构成攻击的棋子
        if (bJiangJun)
            strTip = _T("请先应将!");
        else
        {
            POINT ptJiang = m_LytState.GetJiangJunPosition((CHSSIDE)!actSide);
            POINT ptAttacter = m_LytState.GetChessInfo(m_LytState.GetJiangJunPosition(actSide))->m_lstAttacker.front();
            if (ptJiang.x = ptAttacter.x && ptJiang.y == ptAttacter.y)
                strTip = _T("请勿明将!");
            else
                strTip = _T("请勿送将!");
        }
        bValidMove = FALSE;
        if (!bSilent)
        {
            PlayTip(strTip);
            PlayEffectSound(Sounds::Effects::kDanger);
        }
    }
    else
    {
        CHESSMAN chsEnemy = CHSMAN_NULL;
        UINT nRepeatMove = GetFarthestRepeat(chsEnemy);
        if (chsEnemy == CHSMAN_RED_JIANG || chsEnemy == CHSMAN_BLK_JIANG)
        {
            if (nRepeatMove >= m_dwProps[PROPID_MAX_STP_JIANG])
            {
                strTip =_T("请勿长将!");
                bValidMove = FALSE;
            }
        }
        else if (chsEnemy != CHSMAN_NULL)
        {
            if (nRepeatMove >= m_dwProps[PROPID_MAX_STP_CATCH])
            {
                strTip =_T("请勿长捉!");
                bValidMove = FALSE;
            }
        }
        if (!bSilent && !strTip.IsEmpty())
        {
            PlayTip(strTip);
        }
    }
    // 恢复状态
    m_layout.UndoMove(ms);
    m_history.pop_back();
    m_LytState.Copy(&lytStateBackup);
    return bValidMove;
}

BOOL CChessGame::OnChessPieceClick(IEvtArgs *e)
{
    if(GetActivePlayerIndex() != GetSelfIndex()) 
    {
        PlayTip(_T("It's not your turn"));
        return TRUE;
    }
    if(m_nUndoLeft > 0)
    {
        PlayTip(_T("正在悔棋中，请稍候"));
        return TRUE;
    }
    if(m_nSelectedChessID != -1)
    {
        SWindow *pTarget = sobj_cast<SWindow>(e->Sender());
        CChessPiece *pSelPiece = m_pGameBoard->FindChildByID2 <CChessPiece>(m_nSelectedChessID + ID_CHESS_BASE);
        BOOL isTargetSpiece = pTarget->IsClass(CChessPiece::GetClassName());
        if(pTarget->GetID()-ID_CHESS_BASE == m_nSelectedChessID)
        {//cancel the selection
            DeselectPiece(m_nSelectedChessID);
        }
        else
        {
            CChessPiece *pTargetPiece = (CChessPiece *)pTarget;
            if(isTargetSpiece)
            {
                if( pTargetPiece->IsRedSide() == IsRedSide())
                {//select the piece that is on the same side as the player, cancel the previous selection and select the new one
                    DeselectPiece(m_nSelectedChessID);
                    SelectPiece(pTarget->GetID()-ID_CHESS_BASE);
                }else{
                    POINT ptSel = pSelPiece->GetPos();
                    POINT ptTarget = pTargetPiece->GetPos();
                    if(CheckMove(ptSel,ptTarget)){
                        //attack the piece that is on the opposite side of the player
                        pSelPiece->SetLocalMove(FALSE);
                        MovePiece(m_nSelectedChessID, ptTarget);
                    }
                }
            }else{
                //move the selected piece to the target position
                CPoint ptTarget;
                if(isTargetSpiece){
                    CChessPiece *pTargetPiece = (CChessPiece *)pTarget;
                    ptTarget = pTargetPiece->GetPos();
                }else{
                    ULONG_PTR lp = pTarget->GetUserData();
                    ptTarget=CPoint(GET_X_LPARAM(lp),GET_Y_LPARAM(lp));
                }
                POINT ptSel = pSelPiece->GetPos();
                if (CheckMove(ptSel, ptTarget))
                {
                    // move the selected piece to the target position
                    pSelPiece->SetLocalMove(FALSE);
                    MovePiece(m_nSelectedChessID, ptTarget);
                    m_nSelectedChessID = -1;
                }
            }
        }
    }else{
        CChessPiece *pTarget = sobj_cast<CChessPiece>(e->Sender());
        //select the piece that is on the same side as the player
        BOOL bRedSide = pTarget->IsRedSide();
        if(bRedSide == IsRedSide()){
            SelectPiece(pTarget->GetID()-ID_CHESS_BASE);
        }
    }
    return TRUE;
}
void CChessGame::Init(SWindow *pGameHost, WebSocketClient *pWs)
{
    CChessBoard* pGameBoard = pGameHost->FindChildByName2<CChessBoard>(L"chessboard");
    SAnchorLayout *pAnchorLayout = sobj_cast<SAnchorLayout>(pGameBoard->GetLayout());
    SASSERT(pAnchorLayout);
    #ifndef ENABLE_MOCK
    pGameBoard->FindChildByName(L"btn_test")->SetVisible(FALSE);
    #endif//ENABLE_MOCK
    pAnchorLayout->SetPosition2PointCallback(ChessAnchor2Pos,this);

    m_pGameBoard = pGameBoard;
    m_webSocketClient = pWs;

    IWindow *pBackground = sobj_cast<SImageWnd>(m_pTheme->GetWidget(Sprites::img_background));
    m_pGameBoard->InsertIChild(pBackground);
    pBackground->AddRef();
    
    m_pChessBoard = sobj_cast<SImageWnd>(m_pTheme->GetWidget(Sprites::board_main));
    m_pGameBoard->InsertChild(m_pChessBoard);
    m_pChessBoard->AddRef();

    //init pos flag
    SXmlNode xmlPosFlag = m_pTheme->GetTemplate(Template::kPosFlag);
    SStringW flgClsName = xmlPosFlag.attribute(L"wndclass").as_string(L"img");
    auto slot = Subscriber(&CChessGame::OnChessPieceClick, this);
    for(int y=0;y<10;y++) for(int x=0;x<9;x++)
    {
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

    //load self info
    MyProfile *pMyProfile = MyProfile::getSingletonPtr();
    m_pUserInfo[0] = pMyProfile->GetUserInfo();
    m_userAvatar[0] = pMyProfile->GetAvatarData();
    //insert two avatars to bottom and top
    SXmlNode xmlAvatar = m_pTheme->GetTemplate(Template::kAvatar);
    {
        SWindow *pAvatar = sobj_cast<SWindow>(SApplication::getSingletonPtr()->CreateWindowByName(xmlAvatar.attribute(L"wndclass").as_string()));
        pAvatar->InitFromXml(&xmlAvatar);
        pAvatar->SetID(ID_AVATAR_BASE+0);
        SAnchorLayoutParam *pParam = (SAnchorLayoutParam*)pAvatar->GetLayoutParam();
        pParam->pos = m_pTheme->GetAnchorPos(AnchorName::kAvatarSelf);
        m_pGameBoard->InsertChild(pAvatar);

        pAvatar->FindChildByName(L"txt_name")->SetWindowText(pMyProfile->GetName());
        if(pMyProfile->GetAvatarData()){
            SGifPlayer *pGif = pAvatar->FindChildByName2<SGifPlayer>(L"gif_avatar");
            pGif->LoadFromMemory((LPVOID)pMyProfile->GetAvatarData()->data(), pMyProfile->GetAvatarData()->size());
        }
    }
    {
        SWindow *pAvatar = sobj_cast<SWindow>(SApplication::getSingletonPtr()->CreateWindowByName(xmlAvatar.attribute(L"wndclass").as_string()));
        pAvatar->InitFromXml(&xmlAvatar);
        pAvatar->SetID(ID_AVATAR_BASE+1);
        SAnchorLayoutParam *pParam = (SAnchorLayoutParam*)pAvatar->GetLayoutParam();
        pParam->pos = m_pTheme->GetAnchorPos(AnchorName::kAvatarEnemy);
        m_pGameBoard->InsertChild(pAvatar);
    }


    //insert two clocks to bottom and top
    SXmlNode xmlClock = m_pTheme->GetTemplate(Template::kClock);
    {
        SWindow *pClock = sobj_cast<SWindow>(SApplication::getSingletonPtr()->CreateWindowByName(xmlClock.attribute(L"wndclass").as_string()));
        pClock->InitFromXml(&xmlClock);
        pClock->SetID(ID_ALARM_CLOCK_ME);
        SAnchorLayoutParam *pParam = (SAnchorLayoutParam*)pClock->GetLayoutParam();
        pParam->pos = m_pTheme->GetAnchorPos(AnchorName::kClockSelf);
        pClock->SetVisible(FALSE);
        m_pGameBoard->InsertChild(pClock);
    }
    {
        SWindow *pClock = sobj_cast<SWindow>(SApplication::getSingletonPtr()->CreateWindowByName(xmlClock.attribute(L"wndclass").as_string()));
        pClock->InitFromXml(&xmlClock);
        pClock->SetID(ID_ALARM_CLOCK_ENEMY);
        SAnchorLayoutParam *pParam = (SAnchorLayoutParam*)pClock->GetLayoutParam();
        pParam->pos = m_pTheme->GetAnchorPos(AnchorName::kClockEnemy);
        pClock->SetVisible(FALSE);
        m_pGameBoard->InsertChild(pClock);
    }

    //insert toolbar
    STabCtrl *pToolBar = (STabCtrl*)m_pTheme->GetWidget(Widgets::ktab_toolbar);
    m_pGameBoard->InsertIChild(pToolBar);
    pToolBar->AddRef();
    pToolBar->SetCurSel(0);

    IWindow *pFlagFrom = m_pTheme->GetWidget(Widgets::kflag_pos_from);
    m_pGameBoard->InsertIChild(pFlagFrom);
    pFlagFrom->AddRef();
    IWindow *pFlagTo = m_pTheme->GetWidget(Widgets::kflag_pos_to);
    m_pGameBoard->InsertIChild(pFlagTo);
    pFlagTo->AddRef();

    OnStageChanged(STAGE_CONNECTING);
}

void CChessGame::OnBtnStart()
{
    MyProfile *pMyProfile = MyProfile::getSingletonPtr();
    if(pMyProfile->GetUID() == 0){
        PlayTip(_T("请先登录"));
        return;
    }
    if(pMyProfile->GetTableId()==-1 || pMyProfile->GetSeatIndex()==-1)
    {
        PlayTip(_T("请先在大厅找个桌子坐下:)"));
        return;
    }

    wsSendMsg(GMT_READY, NULL, 0);
    PlayEffectSound(L"btn_start");
}

void CChessGame::OnBtnReqPeace()
{
    CPeaceReqDlg dlg;
    if(dlg.DoModal(m_pMainDlg->m_hWnd) == IDOK){
        MSG_PEACE msg;
        msg.iIndex = m_iSelfIndex; 
        SStringA strDesc = S_CT2A(dlg.m_strDesc,CP_UTF8);
        strcpy_s(msg.szMsg,100,strDesc);
        wsSendMsg(MSG_REQ_PEACE, &msg, sizeof(msg));
        PlayTip(_T("已发送求和请求"));
    }
}

void CChessGame::OnBtnReqSurrender()
{
    if(SMessageBox(m_pMainDlg->m_hWnd,_T("确定要认输吗？"),_T("提示"),MB_YESNO) == IDYES){
        MSG_SURRENDER msg;
        msg.iIndex = m_iSelfIndex; 
        wsSendMsg(MSG_REQ_SURRENDER, &msg, sizeof(msg));
        PlayTip(_T("已发送投降请求"));
    }
}

void CChessGame::OnBtnReqRegret()
{
    if(m_nUsedRegretCount >= m_dwProps[PROPID_REGRET])
    {
        PlayTip(_T("已用完悔棋次数"));
        return;
    }
    if(m_history.empty())
    {
        PlayTip(_T("没有可悔棋的棋子"));
        return;
    }
    if(GetActivePlayerIndex() != m_iSelfIndex){
        PlayTip(_T("请等待对方落子后再悔棋"));
        return;
    }

    MSG_REGRET msg;
    msg.iIndex = m_iSelfIndex; 
    wsSendMsg(MSG_REQ_REGRET, &msg, sizeof(msg));
    PlayTip(_T("已发送悔棋请求"));
}

void CChessGame::OnSetActivePlayerIndex(int nActiveSeat){
    if(nActiveSeat == 0)
    {
        ShowClock(1,FALSE);
        ShowClock(0, TRUE);
        UpdateClock(m_dwProps[PROPID_TIME_STEP], 0);
    }
    else
    {
        ShowClock(0,FALSE);
        ShowClock(1, TRUE);
        UpdateClock(m_dwProps[PROPID_TIME_STEP], 1);
    }
}

void CChessGame::OnStageChanged(STAGE stage)
{
    if(m_stage == stage)
        return;
    m_stage = stage;
    SLOGI() << "OnStageChanged, new state=" << stage;
    switch(stage)
    {
    case STAGE_CONNECTING:
        {
            //insert connnecting animation
            SGifPlayer *pConnecting = (SGifPlayer *)m_pTheme->GetWidget(Sprites::connecting);
            SASSERT(pConnecting);
            m_pGameBoard->InsertChild(pConnecting);
            pConnecting->AddRef();
        }
        break;
    case STAGE_CONTINUE:
        {
            //connected, remove connecting sprite.
            IWindow *pConnecting = m_pGameBoard->FindChildByName(Sprites::connecting);
            if(pConnecting)
            {
                m_pGameBoard->RemoveIChild(pConnecting);
                pConnecting->Release();
            }
            IWindow *pWaitting = m_pGameBoard->FindChildByName(Sprites::sprite_waiting);
            if(pWaitting)
            {
                m_pGameBoard->RemoveIChild(pWaitting);
                pWaitting->Release();
            }
            //switch toolbar to page 0
            STabCtrl *pToolBar = (STabCtrl*)m_pTheme->GetWidget(Widgets::ktab_toolbar);
            pToolBar->SetCurSel(0);
            //enable btn_start
            SWindow *pBtnStart = pToolBar->FindChildByName(Widgets::kbtn_start);
            SASSERT(pBtnStart);
            pBtnStart->EnableWindow(TRUE,TRUE);

            //hide flag for from and to
            IWindow *pFlagFrom = m_pTheme->GetWidget(Widgets::kflag_pos_from);
            pFlagFrom->SetVisible(FALSE,TRUE);
            IWindow *pFlagTo = m_pTheme->GetWidget(Widgets::kflag_pos_to);
            pFlagTo->SetVisible(FALSE,TRUE);
        }
        break;
    case STAGE_WAIT_START:
        {
            //disable btn_start
            SWindow *pBtnStart = m_pGameBoard->FindChildByName(L"btn_start");
            pBtnStart->EnableWindow(FALSE,TRUE);

            PlayWattingAnimation(_T("等待游戏开始..."));
        }
        break;
    case STAGE_PLAYING:
        {
            StopWatingAnimation();
            OnSetActivePlayerIndex(GetActivePlayerIndex()==m_iSelfIndex?0:1);
        }
        break;
    }    
}

int CChessGame::GetScale() const {
    return m_pMainDlg->GetRoot()->GetScale();
}
void CChessGame::OnTableInfo(IEvtArgs *e)
{
    EventTableInfo *pEvt = sobj_cast<EventTableInfo>(e);
    GAME_TABLE_INFO *pTableInfo = pEvt->pTableInfo;
    MyProfile   *pMyProfile = MyProfile::getSingletonPtr();
    if(pTableInfo->nTableId != pMyProfile->GetTableId())
        return;
    m_iSelfIndex = pMyProfile->GetSeatIndex();
    SASSERT(m_iSelfIndex >= 0 && m_iSelfIndex < PLAYER_COUNT);
    for(int j=0;j<PLAYER_COUNT;j++){
        BOOL bFound = FALSE;
        for(int i=0;i<pTableInfo->nPlayers;i++){
            int iSeat = Index2Seat(pTableInfo->seatInfo[i].nIndex);
            if(iSeat != j)
                continue;
            bFound = TRUE;
            m_bReady[iSeat] = pTableInfo->seatInfo[i].bReady;
            if(m_pUserInfo[iSeat])
            {
                if(m_pUserInfo[iSeat]->uid == pTableInfo->seatInfo[i].stUserInfo.uid)
                    break;
                m_pUserInfo[iSeat].reset();
                m_userAvatar[iSeat].reset();
            }
            m_pUserInfo[iSeat] = std::make_shared<GS_USERINFO>(pTableInfo->seatInfo[i].stUserInfo);
            GAME_AVATAR_REQ req = {
                pTableInfo->nTableId,
                pTableInfo->seatInfo[i].stUserInfo.uid
            };
            wsSendMsg(GMT_AVATAR_REQ, &req, sizeof(req));
            SLOGI() << "OnTableInfo: request avatar, iSeat=" << iSeat << ", uid=" << pTableInfo->seatInfo[i].stUserInfo.uid;
            break;
        }
        if(!bFound){
            m_pUserInfo[j].reset();
            m_userAvatar[j].reset();
            m_bReady[j] = FALSE;
        }
        //update ui. show avatar, ready flag.
        SWindow *pAvatar = m_pGameBoard->FindChildByID(ID_AVATAR_BASE + j);
        SGifPlayer *pGif = pAvatar->FindChildByName2<SGifPlayer>(L"gif_avatar");
        if(!m_pUserInfo[j]){
            pAvatar->FindChildByName(L"txt_name")->SetWindowText(_T(""));
            pAvatar->FindChildByName(L"flag_ready")->SetVisible(FALSE, TRUE);
            ISkinObj *pSkin = GETSKIN(L"gif_chair", GetScale());
            SSkinAni *pAniSkin = sobj_cast<SSkinAni>(pSkin);
            pGif->SetAniSkin(pAniSkin);
        }else if(j!=0)  //don't update self name and avatar
        {
            SStringT strName = S_CA2T(m_pUserInfo[j]->szName, CP_UTF8);
            pAvatar->FindChildByName(L"txt_name")->SetWindowText(strName);
            if(m_userAvatar[j]){
                uint32_t uid = pGif->GetUserData();
                if(uid != m_pUserInfo[j]->uid){
                    pGif->LoadFromMemory((LPVOID)m_userAvatar[j]->data(), m_userAvatar[j]->size());
                    pGif->SetUserData(m_pUserInfo[j]->uid);
                }
            }else{
                if(m_pUserInfo[j]->nSex == 1){
                    pGif->SetAniSkin((SSkinAni*)GETSKIN(L"gif_avatar_female", GetScale()));
                }else{
                    pGif->SetAniSkin((SSkinAni*)GETSKIN(L"gif_avatar_male", GetScale()));
                }
            }
        }
        if(pTableInfo->tableState == TABLE_STATE_WAIT){
            pAvatar->FindChildByName(L"flag_ready")->SetVisible(m_bReady[j], TRUE);
        }
    }
    if (m_bReady[0] && pTableInfo->tableState == TABLE_STATE_WAIT)
    {
        OnStageChanged(STAGE_WAIT_START);
    }
    if (!m_bReady[0] && m_stage != STAGE_PLAYING)
    {
        OnStageChanged(STAGE_CONTINUE);
    }
}

void CChessGame::PlayWattingAnimation(LPCTSTR pszTip)
{
    SGifPlayer *pWaiting = (SGifPlayer *)m_pTheme->GetWidget(Sprites::sprite_waiting);
    if(pWaiting){
        m_pGameBoard->InsertIChild(pWaiting);
        pWaiting->AddRef();
    }
    PlayTip(SStringT(pszTip));
}

void CChessGame::StopWatingAnimation()
{
    SWindow *pWaiting = m_pGameBoard->FindChildByName(Sprites::sprite_waiting);
    if (pWaiting)
    {
        m_pGameBoard->RemoveIChild(pWaiting);
        pWaiting->Release();
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

void CChessGame::OnBtnTest()
{
    if(m_stage == STAGE_CONNECTING){
        OnStageChanged(STAGE_CONTINUE);
        GAME_LOGIN_ACK ack = { 1, ERR_SUCCESS, {0} };
        ack.dwProps[PROPID_TIME_STEP] = 60;
        ack.dwProps[PROPID_TIME_ROUND] = 560;
        ack.dwProps[PROPID_MAX_STP_CATCH] = 5;
        ack.dwProps[PROPID_MAX_STP_JIANG] = 5;
        ack.dwProps[PROPID_MAX_STP_PEACE] = 50;
        wsSendMsg(GMT_LOGIN_ACK,&ack, sizeof(ack));
    }
    else if (m_stage == STAGE_CONTINUE)
    {
        MSG_INIT msg = { 0 };
        msg.iRedIndex = (rand()+1)%2;
        CChessLayout::GetDefaultLayout(msg.layout);
        wsSendMsg(GMT_START, &msg, sizeof(msg));
    }else if(m_stage == STAGE_PLAYING)
    {
        // mock move chess, randomly select one chess and one move
        CHSSIDE nSide = m_layout.m_actSide;
        POINT ptChs[16];
        int nCount = m_layout.GetAllChess(ptChs, nSide);
        POINT ptMoves[16][MAX_MOVES];
        int nMoves[16];
        int nTotalMoves = 0;
        for (int i = 0; i < nCount; i++)
        {
            int moves = m_layout.GetPossiableMoves(ptChs[i], ptMoves[i]);
            int nValidMoves = 0;
            for(int j = 0; j < moves; j++){
                if(CheckMove(ptChs[i], ptMoves[i][j],TRUE)){
                    ptMoves[i][nValidMoves++] = ptMoves[i][j];
                }
            }
            nMoves[i] = moves;
            nTotalMoves += nMoves[i] > 0;
        }
        if (nTotalMoves == 0)
        {
            // send timeout message
            MSG_TIMEOUT msg = { GetEnemyIndex() };
            wsSendMsg(MSG_REQ_TIMEOUT, &msg, sizeof(msg));
        }
        else
        {
            int iChs = rand() % nTotalMoves;
            for (int i = 0, j = 0; i < nCount; i++)
            {
                if (nMoves[i] == 0)
                    continue;
                if (iChs == j)
                {
                    iChs = i;
                    break;
                }
                j++;
            }
            int moves = nMoves[iChs];
            int iMove = rand() % moves;
            MSG_MOVE msg = { PtLocal2Net(ptChs[iChs]), PtLocal2Net(ptMoves[iChs][iMove]), TRUE, GetActivePlayerIndex(), 0 };
            wsSendMsg(MSG_REQ_MOVE, &msg, sizeof(msg));
        }
    }
}

void CChessGame::OnTimer(UINT_PTR uIDEvent)
{
    if(uIDEvent == TIMERID_CLOCK_ME){
        int nSecond = m_pGameBoard->FindChildByID(ID_ALARM_CLOCK_ME)->GetUserData();
        m_pMainDlg->KillTimer(uIDEvent);
        UpdateClock(nSecond-1, 0);
    }else if(uIDEvent == TIMERID_CLOCK_ENEMY){
        int nSecond = m_pGameBoard->FindChildByID(ID_ALARM_CLOCK_ENEMY)->GetUserData();
        m_pMainDlg->KillTimer(uIDEvent);
        UpdateClock(nSecond-1, 1);
    }else{
        SetMsgHandled(FALSE);
    }
}

void CChessGame::OnConnected()
{
    SLOGI() << "Connected to server";
    OnStageChanged(STAGE_CONTINUE);
}

void CChessGame::OnDisconnected()
{
#ifndef ENABLE_MOCK
    SLOGI() << "Disconnected from server";
    OnStageChanged(STAGE_CONNECTING);
#endif
}

BOOL CChessGame::OnMessage(DWORD dwType, std::shared_ptr<std::vector<BYTE> > data)
{
    const LPBYTE pMsg = data ? data->data() : NULL;
    DWORD dwLen = data ? data->size() : 0;
    SLOGI() << "Received message, type: " << dwType << ", size: " << dwLen;
    switch(dwType)
    {
    case GMT_START:
        OnGameStart(pMsg, dwLen);
        break;
    case MSG_REQ_MOVE:
        OnMoveChess(pMsg, dwLen);
        break;
    case MSG_REQ_PEACE:
        OnReqPeace(pMsg, dwLen);
        break;
    case MSG_ACK_PEACE:
        OnAckPeace(pMsg,dwLen);
        break;
    case MSG_REQ_REGRET:
        OnAckRegret(pMsg, dwLen);
        break;
    case MSG_ACK_GAMEOVER:
        OnGameOver(pMsg, dwLen);
        break;
    case GMT_LOGIN_ACK:
        OnLoginAck(pMsg, dwLen);
        break;
    case GMT_AVATAR_ACK:
        OnAvatarAck(pMsg, dwLen);
        break;
    case GMT_GAME_MSG:
        OnGameMsg(pMsg, dwLen);
        break;
    default:
        SLOGW() << "Unknown message type: " << dwType;
        break;
    }
    
    return TRUE;
}

void CChessGame::OnGameStart(const void *pData, int nSize)
{
    //remove vectory gif
    if (IWindow *pVectory = m_pGameBoard->FindChildByName(Sprites::sprite_vectory))
    {
        m_pGameBoard->RemoveIChild(pVectory);
        pVectory->Release();
    }

    //remove ready flag
    for(int i = 0; i < PLAYER_COUNT; i++)
    {
        SWindow *pAvatar = m_pGameBoard->FindChildByID(ID_AVATAR_BASE + i);
        pAvatar->FindChildByName(L"flag_ready")->SetVisible(FALSE, TRUE);
    }
    // set toolbar for playing
    STabCtrl *pToolBar = (STabCtrl *)m_pTheme->GetWidget(Widgets::ktab_toolbar);
    pToolBar->SetCurSel(1);

    //clean previous game board, chess id start from 1
    for(int i=1;i<=MAX_CHESS_COUNT;i++){
        SWindow *pWnd = m_pGameBoard->FindChildByID(ID_CHESS_BASE+i);
        if(pWnd)
        {
            pWnd->Destroy();
        }
        pWnd = m_pGameBoard->FindChildByID(ID_SHADOW_BASE+i);
        if(pWnd)
        {
            pWnd->Destroy();
        } 
    }

    SLOGI() << "Game start";
    MSG_INIT    *pInit = (MSG_INIT *)pData;
    m_iRedIndex = pInit->iRedIndex;
    //服务器发来的布局信息,总是下方是红棋,上方是黑棋
    m_layout.InitLayout(pInit->layout, IsRedSide()?CS_RED:CS_BLACK);
    m_layout.SetActiveSide(CS_RED);
    m_LytState.UpdateState();
    m_history.clear();
    m_nUndoLeft = 0;
    m_nUsedRegretCount = 0;

    m_nSelectedChessID = -1;
    //init chess layout
    SXmlNode xmlPiece = m_pTheme->GetTemplate(Template::kChessPiece);
    SStringW clsPieceName = xmlPiece.attribute(L"wndclass").as_string(L"chesspiece");
    SXmlNode xmlShadow = m_pTheme->GetTemplate(Template::kShadow);
    SStringW clsShadowName = xmlShadow.attribute(L"wndclass").as_string(L"img");

    auto slot = Subscriber(&CChessGame::OnChessPieceClick, this);
    for(int y=0;y<10;y++) for(int x=0;x<9;x++)
    {
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
            pPiece->SetID(ID_CHESS_BASE + m_layout.m_nChsID[y][x]);
            pPiece->SetPos(CPoint(x,y));
            m_pGameBoard->InsertChild(pPiece);
            pPiece->SubscribeEvent(EventCmd::EventID, &slot);
        }
        {//init shadow
            IWindow *pShadow = SApplication::getSingletonPtr()->CreateWindowByName(clsShadowName);
            pShadow->InitFromXml(&xmlShadow);
            SAnchorLayoutParam *pParam = (SAnchorLayoutParam*)pShadow->GetLayoutParam();
            SAnchorLayoutParamStruct *pParamStruct = (SAnchorLayoutParamStruct*)pParam->GetRawData();
            pParamStruct->pos.type = 11;
            pParamStruct->pos.x.fSize = x;
            pParamStruct->pos.y.fSize = y;
            pShadow->SetPivot(0.5f,0.0f);
            pShadow->SetID(ID_SHADOW_BASE + m_layout.m_nChsID[y][x]);
            m_pGameBoard->InsertIChild(pShadow);
        }
    }
    m_pGameBoard->SDispatchMessage(UM_SETSCALE, m_pGameBoard->GetChessScale(), 1);
    OnStageChanged(STAGE_PLAYING);
}

//将物理坐标转换成逻辑坐标
POINT CChessGame::PtNet2Local(POINT ptNet) const
{
	if(!m_layout.IsRedSide())
	{
		ptNet.x=8-ptNet.x;
		ptNet.y=9-ptNet.y;
	}
	return ptNet;
}

POINT CChessGame::PtLocal2Net(POINT ptLocal) const
{
	if(!m_layout.IsRedSide())
	{
		ptLocal.x=8-ptLocal.x;
		ptLocal.y=9-ptLocal.y;
	}
	return ptLocal;
}

void CChessGame::OnMoveChess(const void *pData, int nSize)
{
    SLOGI() << "Move chess";
    MSG_MOVE *pMove = (MSG_MOVE *)pData;
    if(pMove->iIndex == m_iSelfIndex && !pMove->bLocal)
        return;
    pMove->ptBegin = PtNet2Local(pMove->ptBegin);
    pMove->ptEnd = PtNet2Local(pMove->ptEnd);

    int nChessID = m_layout.m_nChsID[pMove->ptBegin.y][pMove->ptBegin.x];
    SASSERT(nChessID > 0);
    CChessPiece *pPiece = (CChessPiece *)m_pGameBoard->FindChildByID(ID_CHESS_BASE+nChessID);
    SASSERT(pPiece);
    if(pPiece->GetPicesState() == CChessPiece::STATE_UP )
    {
        pPiece->SetLocalMove(FALSE); // mark this is not a local move, so we send the move to server after animation
        MovePiece(nChessID, pMove->ptEnd);
    }else{
        SelectAndMovePiece(nChessID, pMove->ptEnd);
    }
}

void CChessGame::OnReqPeace(const void *pData, int nSize){
    MSG_PEACE *pPeace = (MSG_PEACE *)pData;
    if(pPeace->iIndex == m_iSelfIndex)
        return;
    SLOGI() << "Peace request, desc: " << pPeace->szMsg;
    CPeaceAckDlg ackDlg(pPeace->szMsg);
    MSG_PEACE msg;
    msg.iIndex = m_iSelfIndex;  
    msg.nResult = ackDlg.DoModal(m_pMainDlg->m_hWnd)==IDOK?1:0;
    wsSendMsg(MSG_ACK_PEACE, &msg, sizeof(msg));
}

void CChessGame::OnAckPeace(const void *pData, int nSize)
{
    MSG_PEACE *pPeace = (MSG_PEACE *)pData;
    if(pPeace->iIndex == m_iSelfIndex)
        return;
    SLOGI() << "Peace request, desc: " << pPeace->szMsg;
    SASSERT(pPeace->nResult == 0);
    PlayTip(SStringT().Format(_T("对家不同意和棋,%s"),S_CA2T(pPeace->szMsg,CP_UTF8).c_str()));
}

void CChessGame::OnAckRegret(const void *pData, int nSize)
{
    SLOGI() << "Ack regret";
    MSG_REGRET *pRegret = (MSG_REGRET *)pData;
    if(m_history.size()<2)//悔棋包含对家和自己两步
    {
        SLOGW() << "No history to regret";
        return;
    }
    if(pRegret->iIndex == m_iSelfIndex)
    {
        m_nUsedRegretCount++;
    }
    m_nUndoLeft = 2;
    UndoLastMove();
}

void CChessGame::UndoLastMove()
{
    SASSERT(!m_history.empty());
    MOVESTEP ms = m_history.back();
    int chsId = m_layout.m_nChsID[ms.pt2.y][ms.pt2.x];
    m_history.pop_back(); 
    m_layout.UndoMove(ms);

    CChessPiece *pPiece = (CChessPiece *)m_pGameBoard->FindChildByID(ID_CHESS_BASE+chsId);
    pPiece->SetUndoMove(std::make_shared<MOVESTEP>(ms));
    if(pPiece->GetPicesState() == CChessPiece::STATE_UP){
        MovePiece(chsId, ms.pt1);
    }else{
        SelectAndMovePiece(chsId, ms.pt1);
    }
}


//BOOL bCheckResult:检查游戏结束标志,在重现历史过程中不需要检查
void CChessGame::OnChessMove(const MOVESTEP & mstep,BOOL bCheckResult)
{
	m_history.push_back(mstep);
	m_LytState.UpdateState();
	if(bCheckResult)
	{
		if(m_LytState.GetWinner()!=CS_NEUTRAL)
		{
            BOOL bWin = m_LytState.GetWinner()==m_layout.m_selfSide;
        #ifdef ENABLE_MOCK
            MSG_GAMEOVER msg;
            msg.iWinner = bWin ? m_iSelfIndex : -1;
            msg.szDesc[0] = 0;
			OnGameOver(&msg, sizeof(msg));
        #else
            MSG_RESULT msgResult={m_iSelfIndex,bWin};
            wsSendMsg(MSG_REQ_RESULT,&msgResult,sizeof(MSG_RESULT));
        #endif
		}else if(m_layout.m_actSide==m_layout.m_selfSide)
		{//检查对方走棋后的长将及长捉状态
			CHESSMAN chsEnemy=CHSMAN_NULL;
			UINT nRepeatMove=GetFarthestRepeat(chsEnemy);

			if(chsEnemy==CHSMAN_RED_JIANG || chsEnemy==CHSMAN_BLK_JIANG)
			{
				if(nRepeatMove>=m_dwProps[PROPID_MAX_STP_JIANG])
				{//通知服务器对方长将
					MSG_LONGCATCH msg={(m_iSelfIndex+1)%2,LCT_JIANG};
                    wsSendMsg(MSG_REQ_LONGCATCH,&msg,sizeof(MSG_LONGCATCH));
				}
			}else if(chsEnemy!=CHSMAN_NULL)
			{
				if(nRepeatMove>=m_dwProps[PROPID_MAX_STP_CATCH])
				{//通知服务器对方长捉
					MSG_LONGCATCH msg={(m_iSelfIndex+1)%2,LCT_CATCH};
                    wsSendMsg(MSG_REQ_LONGCATCH,&msg,sizeof(MSG_LONGCATCH));
				}
			}
		}
	}
}


//计算当前步最远的一次重复走棋，在计算长捉时使用
UINT CChessGame::GetFarthestRepeat(CHESSMAN & chsEnemy)
{
	UINT nRet=0,nSteps=0;
	int	nChsID=-1,nChsEnemy=-1;
	POINT ptLastDest={-1,-1};
	POINT ptEnemy={-1,-1};
	
	CChessLayout layout;
	layout.Copy(&m_layout);
	CChsLytState state(&layout);
	CHSSIDE actSide=(CHSSIDE)!layout.m_actSide;

	chsEnemy=CHSMAN_NULL;

	auto p=m_history.end();
	while(p!=m_history.begin())
	{
		p--;
		if(CHSMANSIDE(layout.m_chesses[p->pt2.y][p->pt2.x])==actSide)
		{
			state.UpdateState();
			nSteps++;
			//找到攻击的无根子数量
			CChsLytState::CChessInfo *pChessInfo=state.GetChessInfo(p->pt2);
			int nNoRootChs=0;
			POINT ptDest;
			std::vector<POINT>::iterator p2=pChessInfo->m_lstDest.begin();
			while(p2!=pChessInfo->m_lstDest.end())
			{
				if(CHSMANSIDE(layout.m_chesses[p2->y][p2->x])==layout.m_actSide
					&& state.GetProtecter(*p2)==0
					&& state.GetAttacter(*p2)==1)
				{
					nNoRootChs++;
					ptDest=*p2;
					if(nNoRootChs>1) break;
				}
				p2++;
			}
			if(nNoRootChs!=1) break;

			if(nChsID==-1)
			{//最后一步
				nChsID=layout.m_nChsID[p->pt2.y][p->pt2.x];
				nChsEnemy=layout.m_nChsID[ptDest.y][ptDest.x];
				ptLastDest=p->pt2;
				ptEnemy=ptDest;
				chsEnemy=layout.m_chesses[ptDest.y][ptDest.x];
			}else
			{
				//攻击及被攻击的棋子不是原棋或者退出计算
				if(nChsID!=layout.m_nChsID[p->pt2.y][p->pt2.x]
				|| nChsEnemy!=layout.m_nChsID[ptDest.y][ptDest.x] )
					break;
				if(p->pt2.x==ptLastDest.x && p->pt2.y==ptLastDest.y)
					nRet=nSteps;//发现重复
			}
		}else
		{
			//移动的棋子不是被攻击棋子
			if(layout.m_nChsID[p->pt2.y][p->pt2.x]!=nChsEnemy)
				break;
		}
		layout.UndoMove(*p);
	}
	return nRet;
}


void CChessGame::OnGifVectoryOver(IEvtArgs *e)
{
    SGifPlayer *pVectory = sobj_cast<SGifPlayer>(e->Sender());
    SSkinAni *pAniSkin = (SSkinAni *)GETSKIN(Skins::kvectory[rand() % ARRAYSIZE(Skins::kvectory)], 1000);
    pVectory->SetAniSkin(pAniSkin);
    pVectory->Start();
    CRect rc = m_pGameBoard->GetClientRect();
    SAnchorLayoutParamStruct *pParamStruct = (SAnchorLayoutParamStruct *)pVectory->GetLayoutParam()->GetRawData();
    pParamStruct->pos.type = APT_Center_Center;
    int x = rand() % rc.Width();
    int y = rand() % rc.Height();
    x -= rc.Width() / 2;
    y -= rc.Height() / 2;
    pParamStruct->pos.x.fSize = x;
    pParamStruct->pos.x.unit = px;
    pParamStruct->pos.y.fSize = y;
    pParamStruct->pos.y.unit = px;
    pVectory->RequestRelayout();
    PlayEffectSound(Sounds::Effects::kfireworks);
}


void CChessGame::OnGameOver(const void *pData, int nSize)
{
    MSG_GAMEOVER *pOver = (MSG_GAMEOVER *)pData;
    SLOGI() << "Game over, desc: " << pOver->szDesc;
    PlayTip(S_CA2T(pOver->szDesc, CP_UTF8));
    m_pMainDlg->KillTimer(TIMERID_CLOCK_ME);
    m_pMainDlg->KillTimer(TIMERID_CLOCK_ENEMY);
    for(int i = 0; i < PLAYER_COUNT; i++){
        ShowClock(i, FALSE);
        m_bReady[i] = FALSE;
    }
    if(pOver->iWinner == m_iSelfIndex){
        m_roundResult = RESULT_WIN;
        SGifPlayer *pVectory = (SGifPlayer *)m_pTheme->GetWidget(Sprites::sprite_vectory);
        m_pGameBoard->InsertIChild(pVectory);
        pVectory->AddRef();
        pVectory->SetVisible(TRUE, TRUE);
        // restart vectory animation.
        EventGifPlayOver e(pVectory);
        OnGifVectoryOver(&e);
    }else if(pOver->iWinner == (m_iSelfIndex + 1) % PLAYER_COUNT){
        m_roundResult = RESULT_LOSE;
    }else{
        m_roundResult = RESULT_DRAW;
    }
    OnStageChanged(STAGE_CONTINUE);
}

void CChessGame::OnLoginAck(const void *pData, int nSize)
{
    GAME_LOGIN_ACK *pAck = (GAME_LOGIN_ACK *)pData;
    if(pAck->errCode == ERR_SUCCESS){
        memcpy(m_dwProps,pAck->dwProps,sizeof(m_dwProps));
    }
}

void CChessGame::OnGameMsg(const void *pData, int nSize)
{
    SStringA msgUtf8((LPCSTR)pData, nSize);
    PlayTip(S_CA2T(msgUtf8, CP_UTF8));
}

void CChessGame::OnAvatarAck(const void *pData, int nSize)
{
    if(nSize < sizeof(GAME_AVATAR_ACK))
        return;
    GAME_AVATAR_ACK *pAck = (GAME_AVATAR_ACK*)pData;
    if(pAck->dwLen == 0){
        return;
    }
    for(int i = 0; i < PLAYER_COUNT; i++){
        if(m_pUserInfo[i] && m_pUserInfo[i]->uid == pAck->uid){
            m_userAvatar[i] = std::make_shared<std::vector<BYTE> >(pAck->byData, pAck->byData + pAck->dwLen);
            SLOGI() << "OnAvatarAck: player=" << i << ", size=" << pAck->dwLen;
            SGifPlayer *pGif = m_pGameBoard->FindChildByID(ID_AVATAR_BASE + i)->FindChildByName2<SGifPlayer>(L"gif_avatar");
            pGif->LoadFromMemory((LPVOID)pAck->byData, pAck->dwLen);
            break;
        }
    }
}

int CChessGame::GetActivePlayerIndex() const
{
    int iRedIndex = m_iRedIndex;
    if (m_layout.m_actSide == CS_RED)
        return iRedIndex;
    else
        return (iRedIndex + 1) % PLAYER_COUNT;
}

void CChessGame::OnTimeout()
{
    MSG_TIMEOUT timeout;
    timeout.iIndex = GetActivePlayerIndex();
    wsSendMsg(MSG_REQ_TIMEOUT, &timeout, sizeof(timeout));
}

void CChessGame::ShowClock(int iSeat,BOOL bShow)
{
    SWindow *pClock = m_pGameBoard->FindChildByID(ID_ALARM_CLOCK_ME+iSeat);
    if (bShow)
    {
        pClock->SetVisible(TRUE, TRUE);
    }
    else
    {
        m_pMainDlg->KillTimer(TIMERID_CLOCK_ME + iSeat);
        pClock->SetVisible(FALSE, TRUE);
        pClock->ClearAnimation();
    }
}

void CChessGame::UpdateClock(int nSecond, int iSeat)
{
    nSecond %=100;  //max 99s
    SWindow *pClock = m_pGameBoard->FindChildByID(ID_ALARM_CLOCK_ME+iSeat);
    pClock->SetUserData(nSecond);
    SStackView * pDigit1 = pClock->FindChildByName2<SStackView>(L"digit1");
    SStackView * pDigit2 = pClock->FindChildByName2<SStackView>(L"digit2");
    pDigit1->SelectPage(nSecond/10);
    pDigit2->SelectPage(nSecond%10);
    if(nSecond == 10){
        IAnimation * pAni = m_pTheme->GetAnimation(Animations::kclock_alarm);
        pClock->SetAnimation(pAni);
    }else if(nSecond == 0){
        pClock->ClearAnimation();
        OnTimeout();
    }
    if(nSecond>0){
        m_pMainDlg->SetTimer(TIMERID_CLOCK_ME+iSeat, 1000);
    }
    if(nSecond<=10)
    {
        PlayEffectSound(Sounds::Effects::kcount_down);
    }    
}

void CChessGame::PlayTip(const SStringT &strTip)
{
    SXmlNode xmlTip = m_pTheme->GetTemplate(L"tip");
    SASSERT(xmlTip);
    SStringW strWndClass = xmlTip.attribute(L"wndclass").as_string(L"text");
    IWindow *pTip = SApplication::getSingletonPtr()->CreateWindowByName(strWndClass);
    pTip->InitFromXml(&xmlTip);
    m_pGameBoard->InsertIChild(pTip);
    pTip->SetWindowText(strTip);
    AnchorPos toPos;
    toPos.type = APT_Center_Top;
    toPos.x=SLayoutSize(-10,dp);
    toPos.y=SLayoutSize(-10,dp);
    toPos.fOffsetX = -0.5f;
    toPos.fOffsetY = -0.5f;
    SAutoRefPtr<IValueAnimator> pAnim = Util::MoveAndHideSprite(pTip, toPos, 5000);
    pAnim->SetID(ANI_TIP);
    pAnim->addListener(this);//destroy the tip when animation end.
}

void CChessGame::wsSendMsg(DWORD dwType, LPVOID lpData, DWORD dwSize)
{
#ifdef ENABLE_MOCK
    std::shared_ptr<std::vector<BYTE> > data(new std::vector<BYTE>);
    if (dwSize)
    {
        data->resize(dwSize);
        memcpy(&(*data)[0], lpData, dwSize);
    }
    OnMessage(dwType, data);
#else
    m_webSocketClient->SendMsg(dwType, lpData, dwSize);
#endif//ENABLE_MOCK
}

void CChessGame::PlayEffectSound(LPCWSTR pszSound)
{
    SStringW strFile = m_pTheme->GetEffectSoundFile(pszSound);
    if(!strFile.IsEmpty()){
        TCHAR szResPrefix[MAX_PATH]={0};
        SApplication::getSingletonPtr()->GetFilePrefix(szResPrefix, MAX_PATH);
        SStringT strFull = SStringT(szResPrefix) + S_CW2T(strFile);
        m_pMainDlg->PlayWave(strFull);
    } 
}
