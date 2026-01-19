#include "stdafx.h"
#include "JunqiGame.h"
#include "MainDlg.h"
#include <helper/SFunctor.hpp>
#include <junqiProtocol.h>
#include "JunqiPiece.h"
#include "utils.h"
#include "PeaceReqDlg.h"
#include "PeaceAckDlg.h"
#include <valueAnimator/SPropertyAnimator.h>
#include <layout/SAnchorLayout.h>
#include <algorithm>
#include <FileHelper.h>
#include <helper/slog.h>
#define kLogTag "JunqiGame"

//#define ENABLE_MOCK
 
#define TIMERID_CLOCK 503


static const float kPieceScale = 1.1f;
enum{
    kShadowHeight_Normal = 86,
    kShadowHeight_Up = 110,
};

POINT CJunqiGame::ChessAnchor2Pos(const AnchorPos &pos, const CRect &rcParent, const CSize & szChild, int nScale, void * userData){
    CJunqiGame *pThis = (CJunqiGame *)userData;
    if(pos.type == 10){
        //type == 10 is chess anchor
        CPoint pt = pThis->m_ptBoardOrigin;
        // offset 0.5 to center, origin is bottom-left corner of board
        pt.x += pThis->m_cellWidth * (pos.x.fSize+0.5);
        pt.y -= pThis->m_cellHeight * (pos.y.fSize+0.5);
        pt.x += pos.fOffsetX * szChild.cx;
        pt.y += pos.fOffsetY * szChild.cy;
        return pt;
    }else if(pos.type >= 11 && pos.type <= 14){
        static POINT kPtBoardConor[4] = {
            {11,6},//bottom
            {11,11}, //right
            {6,11}, //top
            {6,6} //left
        };
        CPoint ptRef = kPtBoardConor[pos.type-11];
        CPoint pt = pThis->m_ptBoardOrigin;
        pt.x += pThis->m_cellWidth * ptRef.x;
        pt.y -= pThis->m_cellHeight * ptRef.y;
        pt.x += pos.x.toPixelSize(nScale);
        pt.y += pos.y.toPixelSize(nScale);
        pt.x += pos.fOffsetX * szChild.cx;
        pt.y += pos.fOffsetY * szChild.cy;
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
CJunqiGame::CJunqiGame(CMainDlg* pMainDlg, SGameTheme* pTheme)
    : m_pMainDlg(pMainDlg)
    ,m_stage(STAGE_INIT)
    ,m_pTheme(pTheme)
    ,m_iSelfIndex(0)
    ,m_iActiveIndex(-1)
    ,m_roundResult(RESULT_DRAW)
    ,m_nSelectedChessID(-1)
    ,m_bMoving(FALSE)
    ,m_nHistoryIndex(0)
    ,m_bReplay(FALSE)
    ,m_bEnableAnimation(TRUE)
{
    memset(m_userInfo, 0, sizeof(m_userInfo));
    
     // 初始化准备状态
    for(int i = 0; i < PLAYER_COUNT; i++)
    {
        m_bReady[i] = FALSE;
    }
    
    memset(m_dwProps, 0, sizeof(m_dwProps));
    m_itCurrentHistory = m_lstHistory.end();
}

/**
 * @brief 析构函数
 */
CJunqiGame::~CJunqiGame()
{
}

enum{
    ANI_MOVE=1,
    ANI_SWAP,
    ANI_TIP,
};

/**
 * @brief 将玩家索引转换为座位索引
 * @param nIndex 玩家索引
 * @return 座位索引
 */
int CJunqiGame::Index2Seat(int nIndex)
{
    return (nIndex + PLAYER_COUNT - m_iSelfIndex) % PLAYER_COUNT;
}

void CJunqiGame::onAnimationStart(IValueAnimator *pAnimator)
{
    if (pAnimator->GetID() == ANI_MOVE)
    {
        PlayEffectSound(Sounds::Effects::kMove);
    }
}
void CJunqiGame::onAnimationEnd(IValueAnimator *pAnimator)
{
    IPropertyAnimator *pPropAnimator = sobj_cast<IPropertyAnimator>(pAnimator);

    if(pPropAnimator->GetID() == ANI_SWAP){
        CJunqiPiece *pPiece = (CJunqiPiece *)pPropAnimator->GetTarget();
        pPiece->SetLayer(SGameTheme::LAYER_PIESE);

        POINT pt = pPiece->GetPos();
        int id = pPiece->GetID();
        CELLCHESS cc = m_layout.GetCellChess(pt);
        CHESSMAN man = m_layout.GetCellChessMan(pt);
        SASSERT(man.nOffRank == pPiece->GetChessman());
        SASSERT(man.nID = id % 100);
    }
    if(pPropAnimator->GetID() == ANI_MOVE)
    {
        CJunqiPiece *pPiece = (CJunqiPiece *)pPropAnimator->GetTarget();
        POINT ptFrom = pPiece->GetPos();
        POINT ptTarget = pPiece->GetTarget();
        CELLCHESS cell = m_layout.GetCellChess(ptTarget);
        CJunqiPiece *pDstPiece = NULL;
        if(cell.pos != NULL){
            CHESSMAN chess = m_layout.GetCellChessMan(ptTarget);
            pDstPiece = m_pGameBoard->FindChildByID2<CJunqiPiece>(ID_CHESS_BASE + cell.nSide*100 + chess.nID);
        }
        {
            //show dest box
            CRotateImage *pDestBox = m_pGameBoard->FindChildByName2<CRotateImage>(Widgets::kimg_dest_box);
            pDestBox->SetVisible(TRUE, TRUE);
            SAnchorLayoutParam *pParam = (SAnchorLayoutParam*)pDestBox->GetLayoutParam();
            pParam->pos.type = 10;
            pParam->pos.x.fSize = ptTarget.x;
            pParam->pos.y.fSize = ptTarget.y;
            pDestBox->SetRotate(CRotateImage::CalcRotate(ptTarget, m_iSelfIndex, pPiece->GetColor()),TRUE);
            pDestBox->RequestRelayout();
        }
        int nResult = pPiece->GetMoveResult();
        m_layout.MoveChessmanClient(ptFrom, ptTarget,nResult);
        if(nResult != PUT_MOVE){
            SASSERT(pDstPiece);
            if(nResult == PUT_EAT){
                //remove target piece
                pPiece->SetPos(ptTarget);
                pPiece->SetTarget(CPoint(-1, -1));
                pPiece->SetLayer(SGameTheme::LAYER_PIESE);

                pDstPiece->Destroy();

                PlayEffectSound(Sounds::Effects::kEat);
            }else if(nResult == PUT_BEEAT){
                //remove source piece
                pPiece->Destroy();
                PlayEffectSound(Sounds::Effects::kKilled);
            }else if(nResult == PUT_BOMB){
                //remove both pieces
                pPiece->Destroy();
                pDstPiece->Destroy();
                PlayEffectSound(Sounds::Effects::kBomb);
            }
        }else{
            pPiece->SetPos(ptTarget);
            pPiece->SetTarget(CPoint(-1, -1));
            pPiece->SetLayer(SGameTheme::LAYER_PIESE);
            if(m_layout.GetJunQiMap()->GetCellType(ptTarget) == CP_BARBACK){
                PlayEffectSound(Sounds::Effects::kBarback);
            }
        }

        m_bMoving = FALSE;
        m_webSocketClient->blockReceive(FALSE);
    }else if(pPropAnimator->GetID() == ANI_TIP)
    {
        IWindow *pTip = pPropAnimator->GetTarget();
        pTip->Destroy();
    }
}

int CJunqiGame::CalcMoveDir(POINT ptFrom, POINT ptTo){
    int nDir = 0;
    int nDiffX = ptTo.x - ptFrom.x;
    int nDiffY = ptTo.y - ptFrom.y;
    if(nDiffX==0){
        nDir = nDiffY>0?0:4;
    }else if(nDiffY==0){ 
        nDir = nDiffX>0?2:6;
    }else{
        if(nDiffX>0 && nDiffY>0){
            nDir = 1;
        }else if(nDiffX>0 && nDiffY<0){
            nDir = 3;
        }else if(nDiffX<0 && nDiffY>0){
            nDir = 7;
        }else if(nDiffX<0 && nDiffY<0){
            nDir = 5;
        }
    }
    return nDir;
}

void CJunqiGame::onAnimationUpdate(IValueAnimator *pAnimator)
{
    IPropertyAnimator *pPropAnimator = sobj_cast<IPropertyAnimator>(pAnimator);
    if(pPropAnimator->GetID() == ANI_MOVE)
    {
        float fraction = pAnimator->getAnimatedFraction();
        IPropertyValuesHolder *pHolder = pPropAnimator->GetPropertyValuesHolderByIndex(0);
        int index = pHolder->Fraction2FrameIndex(fraction);
        int oldIndex = (int)(UINT_PTR)pAnimator->GetUserData();

        if(index != oldIndex)
        {    
            pAnimator->SetUserData((void *)(UINT_PTR)index);
            SXmlNode xmlMoveDir = m_pTheme->GetTemplate(Template::kMoveDir);
            SStringW strWndClass = xmlMoveDir.attribute(L"wndclass").as_string(L"img");
            SLOGI()<<"oldIndex="<<oldIndex<<",index="<<index;
            AnchorPos pos;
            pHolder->GetValueByIndex(oldIndex, &pos, sizeof(AnchorPos));
            for(int i = oldIndex+1; i <= index; i++)
            {
                AnchorPos pos2;
                pHolder->GetValueByIndex(i, &pos2, sizeof(AnchorPos));
                SImageWnd *pDir = sobj_cast<SImageWnd>(SApplication::getSingletonPtr()->CreateWindowByName(strWndClass));
                SASSERT(pDir);
                pDir->InitFromXml(&xmlMoveDir);
                pDir->SetID(ID_DIR_BASE + i - 1);
                SAnchorLayoutParam *pParam = (SAnchorLayoutParam*)pDir->GetLayoutParam();
                pParam->pos = pos;

                POINT pt1 = { (int)pos.x.fSize, (int)pos.y.fSize };
                POINT pt2 = { (int)pos2.x.fSize, (int)pos2.y.fSize };
                int nDir = CalcMoveDir(pt1, pt2);
                pDir->SetIcon(nDir);
                m_pGameBoard->InsertIChild(pDir);

                pos = pos2;
            }
            if(index < pHolder->GetKeyframeCount()-1)
                PlayEffectSound(Sounds::Effects::kMove);
        }
    }
}

void CJunqiGame::SelectPiece(int nId)
{
    if(m_bMoving){
        SLOGI() << "ignore select piece, moving...";
        return;
    }
    DeselectPiece();
    SLOGI() << "scale piece to: " << kPieceScale << " id=" << m_nSelectedChessID;
    IAnimation* pAni = m_pTheme->GetAnimation(Animations::ksel_piece);
    SWindow *pPiece = m_pGameBoard->FindChildByID(nId);
    pPiece->SetAnimation(pAni);
    pAni->Release();
    m_nSelectedChessID = nId;
}

void CJunqiGame::DeselectPiece()
{
    if (m_nSelectedChessID != -1)
    {
        SWindow *pPiece = m_pGameBoard->FindChildByID(m_nSelectedChessID);
        pPiece->ClearAnimation();
    }
    m_nSelectedChessID = -1;
}

void CJunqiGame::ClearMovePath(){
    for(int i=0;i<AC_RWSSIZE;i++)
    {
        SWindow *pDir = m_pGameBoard->FindChildByID(ID_DIR_BASE + i);
        if(pDir)
            pDir->Destroy();
        else
            break;
    }
}

static float distance(POINT pt1, POINT pt2){
    return sqrt((pt1.x-pt2.x)*(pt1.x-pt2.x) + (pt1.y-pt2.y)*(pt1.y-pt2.y));
}

BOOL CJunqiGame::MovePiece(int nId, POINT ptTarget)
{
    CJunqiPiece *pPiece = m_pGameBoard->FindChildByID2<CJunqiPiece>(nId);
    if (!pPiece)
        return FALSE;
    POINT pt = pPiece->GetPos();
    BOOL bValid = m_layout.IsValidMove(pt, ptTarget, 0);
    if (!bValid)
        return FALSE;
    MSG_MOVE msgMove;
    msgMove.iIndex = m_iSelfIndex;
    msgMove.ptBegin = PtLocal2Net(pt);
    msgMove.ptEnd = PtLocal2Net(ptTarget);
    msgMove.bSapper = m_layout.IsSapper(pt);
    wsSendMsg(MSG_REQ_MOVE, &msgMove, sizeof(msgMove));
    return TRUE;
}

BOOL CJunqiGame::DoMovePiece(POINT ptFrom, POINT ptTarget,BOOL bSapper, int nResult)
{
    DeselectPiece();
    CELLCHESS cell = m_layout.GetCellChess(ptFrom);
    if(cell.pos == NULL){
        return FALSE;
    }
    CHESSMAN chess = m_layout.GetCellChessMan(ptFrom);
    if(chess.nID < 0)
    {
        return FALSE;
    }    
    CJunqiPiece *pPiece = m_pGameBoard->FindChildByID2<CJunqiPiece>(ID_CHESS_BASE + cell.nSide * 100 + chess.nID);
    if(!pPiece)
    {
        PlayTip(_T("error!!!你没有这个棋子"));
        return FALSE;
    }    
    ClearMovePath();
    HideClock();
    pPiece->SetTarget(ptTarget);
    pPiece->SetMoveResult(nResult);
    pPiece->SetLayer(SGameTheme::LAYER_PIESE+1);
    SLOGI() << "move piece to: " << ptTarget.x << "," << ptTarget.y;

    POINT pt = pPiece->GetPos();
    std::vector<POINT> lstCell;
    m_layout.GetMoveCMPath(bSapper, pt, ptTarget, &lstCell);
    if(lstCell.empty())
        return FALSE;
    m_bMoving = TRUE;
    m_webSocketClient->blockReceive(TRUE);
    std::vector<AnchorPos> lstPos;
    std::vector<float> lstRotate;
    lstPos.resize(lstCell.size());
    lstRotate.resize(lstCell.size());
    std::vector<float> lstWeight;
    lstWeight.resize(lstCell.size()-1);
    float fTotalWeight = 0;
    for(int i=0;i<lstCell.size();i++)
    {
        lstPos[i].type = 10;
        lstPos[i].x.fSize = lstCell[i].x;
        lstPos[i].y.fSize = lstCell[i].y;
        lstPos[i].fOffsetX = -0.5f;
        lstPos[i].fOffsetY = -0.5f;
        lstRotate[i] = pPiece->CalcRotate(lstCell[i], m_iSelfIndex);
        if(i>0){
            lstWeight[i-1] = distance(lstCell[i], lstCell[i-1]);
            fTotalWeight += lstWeight[i-1];
        }
    }
    IPropertyValuesHolder *pHolderPos = SPropertyValuesHolder::ofPosition(LayoutProperty::POSITION, lstPos.data(), lstPos.size(), sizeof(AnchorPos));
    pHolderPos->SetKeyFrameWeights(lstWeight.data(), lstWeight.size());
    IPropertyValuesHolder *pHolderRotate = SPropertyValuesHolder::ofFloat(WindowProperty::ROTATE, lstRotate.data(), lstRotate.size());
    pHolderRotate->SetKeyFrameWeights(lstWeight.data(), lstWeight.size());
    IPropertyValuesHolder *holders[] = { pHolderPos, pHolderRotate };
    SAutoRefPtr<IPropertyAnimator> pAnimator(SPropertyAnimator::ofPropertyValuesHolder(pPiece, holders, 2), FALSE);
    pHolderPos->Release();
    pHolderRotate->Release();
    pAnimator->setDuration(100*fTotalWeight);
    pAnimator->SetID(ANI_MOVE);
    pAnimator->addListener(this);
    pAnimator->addUpdateListener(this);
    Util::StartAnimator(pAnimator, pPiece);
    pAnimator->SetUserData((void *)(UINT_PTR)0);
    if(!m_bEnableAnimation){
        pAnimator->end();
    }
    return TRUE;
}

BOOL CJunqiGame::CheckMove(POINT ptFrom, POINT ptTo, BOOL bSilent){
    BOOL bValidMove = FALSE;

    return bValidMove;
}

void CJunqiGame::Init(SWindow *pGameHost, WebSocketClient *pWs)
{
    CJunqiBoard* pGameBoard = pGameHost->FindChildByName2<CJunqiBoard>(L"board_host");
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

    IWindow *pDestBox = m_pTheme->GetWidget(Widgets::kimg_dest_box);
    m_pGameBoard->InsertIChild(pDestBox);
    pDestBox->AddRef();
    pDestBox->SetVisible(FALSE,FALSE);

    //load self info
    MyProfile *pMyProfile = MyProfile::getSingletonPtr();
    m_pUserInfo[0] = pMyProfile->GetUserInfo();
    m_userAvatar[0] = pMyProfile->GetAvatarData();
    //insert 4 avatars
    SXmlNode xmlAvatar = m_pTheme->GetTemplate(Template::kAvatar);
    for (int i = 0; i < PLAYER_COUNT; i++)
    { 
        SWindow *pAvatar = sobj_cast<SWindow>(SApplication::getSingletonPtr()->CreateWindowByName(xmlAvatar.attribute(L"wndclass").as_string()));
        pAvatar->InitFromXml(&xmlAvatar);
        pAvatar->SetID(ID_AVATAR_BASE+i);
        pAvatar->FindChildByName(L"txt_name")->SetWindowText(SStringT().Format(_T("seat %d"),i));
        SAnchorLayoutParam *pParam = (SAnchorLayoutParam*)pAvatar->GetLayoutParam();
        pParam->pos = m_pTheme->GetAnchorPos(AnchorName::kAvatarBottom+i);
        m_pGameBoard->InsertChild(pAvatar);
    }
    {
        SWindow *pAvatar = m_pGameBoard->FindChildByID(ID_AVATAR_BASE+0);
        pAvatar->FindChildByName(L"txt_name")->SetWindowText(pMyProfile->GetName());
        if(pMyProfile->GetAvatarData()){
            SGifPlayer *pGif = pAvatar->FindChildByName2<SGifPlayer>(L"gif_avatar");
            pGif->LoadFromMemory((LPVOID)pMyProfile->GetAvatarData()->data(), pMyProfile->GetAvatarData()->size());
        }
    }
    //insert clock
    SXmlNode xmlClock = m_pTheme->GetTemplate(Template::kClock);
    {
        SWindow *pClock = sobj_cast<SWindow>(SApplication::getSingletonPtr()->CreateWindowByName(xmlClock.attribute(L"wndclass").as_string()));
        pClock->InitFromXml(&xmlClock);
        pClock->SetID(ID_ALARM_CLOCK);
        pClock->SetVisible(FALSE);
        m_pGameBoard->InsertChild(pClock);
    }

    //insert toolbar
    STabCtrl *pToolBar = (STabCtrl*)m_pTheme->GetWidget(Widgets::ktab_toolbar);
    m_pGameBoard->InsertIChild(pToolBar);
    pToolBar->AddRef();
    pToolBar->SetCurSel(0);

    OnStageChanged(STAGE_CONNECTING);
}

void CJunqiGame::OnBtnStart()
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
    m_lstHistory.clear();
    m_bReplay = FALSE;
    DeselectPiece();
    MSGSTARTFIGHT msg;
    msg.iIndex = m_iSelfIndex;
    m_layout.MakeLayout(0, msg.byLayout);
    wsSendMsg(GMT_READY, NULL, 0);
    wsSendMsg(GLID_JQSTARTFIGHT, &msg, sizeof(msg));
    PlayEffectSound(Sounds::Effects::kBegin);
}

void CJunqiGame::OnBtnReqPeace()
{
    CPeaceReqDlg dlg;
    if(dlg.DoModal(m_pMainDlg->m_hWnd) == IDOK){
        MSG_PEACE msg;
        msg.iIndex = m_iSelfIndex; 
        SStringA strDesc = S_CT2A(dlg.m_strDesc,CP_UTF8);
        strcpy_s(msg.szMsg,100,strDesc);
        wsSendMsg(MSG_REQ_PEACE, &msg, sizeof(msg));
    }
}

void CJunqiGame::OnBtnReqSurrender()
{
    if(SMessageBox(m_pMainDlg->m_hWnd,_T("确定要认输吗？"),_T("提示"),MB_YESNO) == IDYES){
        MSG_SURRENDER msg;
        msg.iIndex = m_iSelfIndex; 
        wsSendMsg(MSG_REQ_SURRENDER, &msg, sizeof(msg));
    }
}

void CJunqiGame::OnBtnReqPass()
{
    if(m_iSelfIndex == m_iActiveIndex){
        MSG_PASS msg;
        msg.iIndex = m_iSelfIndex;
        wsSendMsg(MSG_REQ_PASS, &msg, sizeof(msg));
    }
}

void CJunqiGame::OnBtnNewRound()
{
    if (IWindow *pVectory = m_pGameBoard->FindChildByName(Sprites::sprite_vectory))
    {
        m_pGameBoard->RemoveIChild(pVectory);
        pVectory->Release();
    }
    OnStageChanged(STAGE_CONTINUE);
}

void CJunqiGame::OnBtnReplay()
{
    STabCtrl *pToolBar = (STabCtrl*)m_pTheme->GetWidget(Widgets::ktab_toolbar);
    pToolBar->SetCurSel(3);
    InitReplay();
}

void CJunqiGame::OnBtnPrev()
{
    if(!m_bReplay)
        return;
    StepBackward();
}

void CJunqiGame::OnBtnNext()
{ 
    if(!m_bReplay)
        return;
    StepForward();
}

void CJunqiGame::OnBtnSave()
{
    CFileDialogEx dlg(FALSE, _T("sjq"), _T("*.sjq"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("Junqi Files (*.sjq)|*.sjq|All Files (*.*)|*.*||"), 0);
    if(dlg.DoModal(m_pMainDlg->m_hWnd) == IDOK){
        SLOGI() << "Save to " << dlg.m_szFileName;
        FILE *fp = _tfopen(dlg.m_szFileName, _T("wb"));
        if(fp){
            //write header
            DWORD dwVers[4]={JUNQI_VERMAJOR,JUNQI_VERMINOR_L2,JUNQI_VERMINOR_S3,JUNQI_VERMINOR_S4};
            fwrite(dwVers, sizeof(dwVers), 1, fp);
            //write user info
            for (int i = 0; i < PLAYER_COUNT; i++)
            {
                fwrite(&m_userInfo[i], sizeof(GS_USERINFO), 1, fp);
            }
            fwrite(m_dwProps, sizeof(m_dwProps), 1, fp);
            //write init layout
            fwrite(m_byInitLayouts, sizeof(m_byInitLayouts), 1, fp);
            //write self index
            fwrite(&m_iSelfIndex, sizeof(m_iSelfIndex), 1, fp);
            //write history
            int nCount = m_lstHistory.size();
            fwrite(&nCount, sizeof(nCount), 1, fp);
            for (auto it = m_lstHistory.begin(); it != m_lstHistory.end(); it++)
            {
                fwrite(&it->first, sizeof(it->first), 1, fp);
                int nSize = it->second->size();
                fwrite(&nSize, sizeof(nSize), 1, fp);
                fwrite(it->second->data(), nSize, 1, fp);
            }
            fclose(fp);
        }
    }
}

void CJunqiGame::OnBtnLoad()
{
    CFileDialogEx dlg(TRUE, _T("sjq"), _T("qj.sjq"), OFN_HIDEREADONLY, _T("Junqi Files (*.sjq)\0*.sjq\0All Files (*.*)\0*.*\0\0"), 0);
    if(dlg.DoModal(m_pMainDlg->m_hWnd) == IDOK){
        SLOGI() << "Load from " << dlg.m_szFileName;

        DWORD dwVers[4]={JUNQI_VERMAJOR,JUNQI_VERMINOR_L2,JUNQI_VERMINOR_S3,JUNQI_VERMINOR_S4};
        FILE *fp = _tfopen(dlg.m_szFileName, _T("rb"));
        if(fp){
            //read header
            DWORD dwVer[4];
            fread(dwVer, sizeof(dwVer), 1, fp);
            if(memcmp(dwVer, dwVers, sizeof(dwVer)) != 0){
                SLOGW() << "File version mismatch";
                return;
            }
            m_lstHistory.clear();
            //read user info
            for (int i = 0; i < PLAYER_COUNT; i++)
            {
                fread(&m_userInfo[i], sizeof(GS_USERINFO), 1, fp);
            }
            fread(m_dwProps, sizeof(m_dwProps), 1, fp);
            //read init layout
            fread(m_byInitLayouts, sizeof(m_byInitLayouts), 1, fp);
            //read self index
            fread(&m_iSelfIndex, sizeof(m_iSelfIndex), 1, fp);
            //read history
            int nHistoryCount;
            fread(&nHistoryCount, sizeof(nHistoryCount), 1, fp);
            for (int i = 0; i < nHistoryCount; i++)
            {
                DWORD dwType;
                fread(&dwType, sizeof(dwType), 1, fp);
                int nSize;
                fread(&nSize, sizeof(nSize), 1, fp);
                std::shared_ptr<std::vector<BYTE> > data(new std::vector<BYTE>(nSize));
                fread(data->data(), nSize, 1, fp);
                m_lstHistory.push_back(std::make_pair(dwType, data));
            }
            fclose(fp);
            InitReplay();
        }
    }
}

void CJunqiGame::InitReplay()
{
    SLOGI() << "InitReplay: start replay mode";
    m_bReplay = TRUE;
    m_nHistoryIndex = 0;
    m_itCurrentHistory = m_lstHistory.begin();
    
    // 设置用户信息
    {
        for(int i = 0; i < PLAYER_COUNT; i++)
        {
            int iSeat = Index2Seat(i);
            SWindow *pAvatar = m_pGameBoard->FindChildByID(ID_AVATAR_BASE + iSeat);
            pAvatar->FindChildByName(L"txt_name")->SetWindowText(S_CA2T(m_userInfo[iSeat].szName, CP_UTF8));
        }
    }
    {
        IWindow *pVectory = m_pGameBoard->FindChildByName(Widgets::kVectory);
        if(pVectory){
            m_pGameBoard->RemoveIChild(pVectory);
            pVectory->Release();
        }
    }
    // 清空棋盘
    {
        for(int i=0;i<PLAYER_COUNT;i++){
            for(int j=0;j<25;j++){
                SWindow *pPiece = m_pGameBoard->FindChildByID(ID_CHESS_BASE + i*100 + j);
                if(pPiece){
                    pPiece->Destroy();
                }
            }
            m_layout.ClearLayout(i);
        }
        ClearMovePath();
        m_pGameBoard->FindChildByName(Widgets::kimg_dest_box)->SetVisible(FALSE, TRUE);
    }
    if(!m_lstHistory.empty()){
        // 初始化棋子从初始布局
        for(int i = 0; i < PLAYER_COUNT; i++)
        {
            InitLayout(m_byInitLayouts[i], i);
        }
    }    
    OnStageChanged(STAGE_PLAYING);
}

void CJunqiGame::StepForward()
{
    if(!m_bReplay || m_itCurrentHistory == m_lstHistory.end()){
        PlayTip(_T("已到最后"));
        return;
    }
    
    DWORD dwType = m_itCurrentHistory->first;
    std::shared_ptr<std::vector<BYTE> > data = m_itCurrentHistory->second;
    const LPBYTE pMsg = data ? data->data() : NULL;
    DWORD dwLen = data ? data->size() : 0;
    
    ProcessHistoryMessage(dwType, pMsg, dwLen);
    
    m_nHistoryIndex++;
    m_itCurrentHistory++;
    
    SLOGI() << "Replay step forward, index=" << m_nHistoryIndex;
}

void CJunqiGame::StepBackward()
{
    if(m_nHistoryIndex == 0){
        PlayTip(_T("已到开始"));
        return;
    }
    
    // 使用快速重放到上一个位置（无动画）
    int nTargetStep = m_nHistoryIndex - 1;
    FastReplayToStep(nTargetStep);
    
    SLOGI() << "Replay step backward, index=" << m_nHistoryIndex;
}

void CJunqiGame::FastReplayToStep(int nTargetStep)
{
    if(nTargetStep < 0){
        nTargetStep = 0;
    }
    
    if(nTargetStep > (int)m_lstHistory.size()){
        nTargetStep = (int)m_lstHistory.size();
    }
    
    SLOGI() << "FastReplayToStep: target=" << nTargetStep << ", current=" << m_nHistoryIndex;
    
    m_bEnableAnimation = FALSE;  // 禁用动画以提高速度
    m_pGameBoard->LockUpdate();
    InitReplay();
    
    // 快速重放到指定步数（无动画）
    m_itCurrentHistory = m_lstHistory.begin();
    for(int i = 0; i < nTargetStep && m_itCurrentHistory != m_lstHistory.end(); i++){
        DWORD dwType = m_itCurrentHistory->first;
        std::shared_ptr<std::vector<BYTE> > data = m_itCurrentHistory->second;
        const LPBYTE pMsg = data ? data->data() : NULL;
        DWORD dwLen = data ? data->size() : 0;
        
        ProcessHistoryMessage(dwType, pMsg, dwLen);
        m_itCurrentHistory++;
    }
    m_bEnableAnimation = TRUE;
    m_pGameBoard->UnlockUpdate();
    m_pGameBoard->InvalidateRect(NULL);
    m_nHistoryIndex = nTargetStep;
}

void CJunqiGame::ProcessHistoryMessage(DWORD dwType, const void *pData, int nSize)
{
    const LPBYTE pMsg = (const LPBYTE)pData;
    DWORD dwLen = nSize;
    
    switch(dwType)
    {
    case MSG_NOTIFY_PLAYER_LOSE:
        OnPlayerLose(pMsg, dwLen);
        break;
    case MSG_REQ_MOVE:
        OnMoveChess(pMsg, dwLen);
        break;
    case MSG_ACK_GAMEOVER:
        OnGameOver(pMsg, dwLen);
        break;
    case MSG_REQ_SURRENDER:
        OnReqSurrender(pMsg, dwLen);
        break;
    case MSG_REQ_PASS:
        OnPlayerPass(pMsg, dwLen);
        break;
    default:
        SLOGW() << "Unknown message type in replay: " << dwType;
        break;
    }
}

void CJunqiGame::MoveClock(int nSeat, int value)
{
    if(m_bReplay)
        return;
    SWindow *pClock = m_pGameBoard->FindChildByID(ID_ALARM_CLOCK);
    pClock->SetVisible(TRUE, TRUE);
    pClock->ClearAnimation();
    UpdateClock(value);
    AnchorPos pos = m_pTheme->GetAnchorPos(AnchorName::kClock_Bottom + nSeat);
    Util::MoveSpriteTo(pClock, pos, 200);
}

void CJunqiGame::HideClock()
{
    SWindow *pClock = m_pGameBoard->FindChildByID(ID_ALARM_CLOCK);
    pClock->ClearAnimation();
    pClock->SetVisible(FALSE, TRUE); 
    m_pMainDlg->KillTimer(TIMERID_CLOCK);
}
void CJunqiGame::OnSetActivePlayerIndex(int nActiveSeat){
    SLOGI() << "OnSetActivePlayerIndex, new index=" << nActiveSeat;
    m_iActiveIndex = nActiveSeat;
    MoveClock(Index2Seat(m_iActiveIndex), m_dwProps[PROPID_STEPTICK]);
    if(m_iActiveIndex == GetSelfIndex()){
        if(GetForegroundWindow() != m_pMainDlg->m_hWnd){
            FlashWindow(m_pMainDlg->m_hWnd, TRUE);
        }
    }
}

void CJunqiGame::OnStageChanged(STAGE stage)
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
            //remove vectory gif
            if (IWindow *pVectory = m_pGameBoard->FindChildByName(Sprites::sprite_vectory))
            {
                m_pGameBoard->RemoveIChild(pVectory);
                pVectory->Release();
            }
            //switch toolbar to page 0
            STabCtrl *pToolBar = (STabCtrl*)m_pTheme->GetWidget(Widgets::ktab_toolbar);
            pToolBar->SetCurSel(0);
            //enable btn_start
            SWindow *pBtnStart = pToolBar->FindChildByName(Widgets::kbtn_start);
            SASSERT(pBtnStart);
            pBtnStart->EnableWindow(TRUE,TRUE);

            {//clear old stuff
                for(int i=0;i<PLAYER_COUNT;i++){
                    for(int j=0;j<25;j++){
                        SWindow *pPiece = m_pGameBoard->FindChildByID(ID_CHESS_BASE + i*100 + j);
                        if(pPiece){
                            pPiece->Destroy();
                        }
                    }
                    m_layout.ClearLayout(i);
                }
                ClearMovePath();
                m_pGameBoard->FindChildByName(Widgets::kimg_dest_box)->SetVisible(FALSE, TRUE);
            }
            { // init chess pieces
                SXmlNode xmlPiece = m_pTheme->GetTemplate(Template::kPiece);
                static const int kInitLayout[6][5] = { 
                    { OR_PLATOON, OR_MINE, OR_COMPANY, OR_FLAG, OR_BATTALION }, 
                    { OR_MINE, OR_MINE, OR_ARMY, OR_BATTALION, OR_CHIEF },
                    { OR_SAPPER, OR_NULL, OR_SAPPER, OR_NULL, OR_DIVISION }, 
                    { OR_COMPANY, OR_DIVISION, OR_NULL, OR_BRIGADIER, OR_BOMB }, 
                    { OR_PLATOON, OR_NULL, OR_BOMB, OR_NULL, OR_COMPANY }, 
                    { OR_PLATOON, OR_BRIGADIER, OR_COLONEL, OR_SAPPER, OR_COLONEL }
                };
                m_layout.InitLayout(0, kInitLayout);
                for (int y = 0; y < 17; y++)
                {
                    for (int x = 0; x < 17; x++)
                    {
                        CELLCHESS cell = m_layout.GetCellChess(CPoint(x, y));
                        if (cell.pos)
                        {
                            CJunqiPiece *pPiece = (CJunqiPiece *)SApplication::getSingletonPtr()->CreateWindowByName(xmlPiece.attribute(L"wndclass").as_string());
                            pPiece->InitFromXml(&xmlPiece);
                            SAnchorLayoutParam *pParam = (SAnchorLayoutParam *)pPiece->GetLayoutParam();
                            pParam->pos.type = 10;
                            pParam->pos.x.fSize = x;
                            pParam->pos.y.fSize = y;
                            CHESSMAN chess = m_layout.GetCellChessMan(CPoint(x, y));
                            pPiece->SetColor(m_iSelfIndex);
                            pPiece->SetChessman(chess.nOffRank);
                            pPiece->SetPos(CPoint(x, y));
                            pPiece->SetID(ID_CHESS_BASE +  chess.nID);//side = 0 for self
                            float fRotate = pPiece->CalcRotate(CPoint(x, y),m_iSelfIndex);
                            pPiece->SetRotate(fRotate);
                            m_pGameBoard->InsertChild(pPiece);
                        }
                    }
                }
            }
        }
        break;
        case STAGE_WAIT_START:
        {
            //disable btn_start
            SWindow *pBtnStart = m_pGameBoard->FindChildByName(L"btn_start");
            pBtnStart->EnableWindow(FALSE,TRUE);
        }
        break;
    case STAGE_PLAYING:
        {
            OnSetActivePlayerIndex(GetActivePlayerIndex());
        }
        break;
    case STAGE_GAMEOVER:
        {
            STabCtrl *pToolBar = (STabCtrl *)m_pTheme->GetWidget(Widgets::ktab_toolbar);
            pToolBar->SetCurSel(2);
        }
        break;
    }    
}

int CJunqiGame::GetScale() const {
    return m_pMainDlg->GetRoot()->GetScale();
}
void CJunqiGame::OnTableInfo(IEvtArgs *e)
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
            if (m_bReady[j])
            {
                SLOGI() << "OnTableInfo: player leave, seat=" << j;
                // clear layout of this player
                for(int i=0;i<25;i++){
                    SWindow *pPiece = m_pGameBoard->FindChildByID(ID_CHESS_BASE + j*100+i);
                    if(pPiece){
                        pPiece->Destroy();
                    }
                }
                m_layout.ClearLayout(j);
            }
            m_pUserInfo[j].reset();
            m_userAvatar[j].reset();
            m_bReady[j] = FALSE;
        }
        //update ui. show avatar, ready flag.
        SWindow *pAvatar = m_pGameBoard->FindChildByID(ID_AVATAR_BASE + j);
        SGifPlayer *pGif = pAvatar->FindChildByName2<SGifPlayer>(L"gif_avatar");
        if(!m_pUserInfo[j]){
            pAvatar->FindChildByName(L"txt_name")->SetWindowText(SStringT().Format(_T("seat %d"),j));
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
    if (!m_bReady[0] && m_stage != STAGE_PLAYING && m_stage != STAGE_GAMEOVER)
    {
        OnStageChanged(STAGE_CONTINUE);
    }
}

void CJunqiGame::OnGameBoardSizeChanged(IEvtArgs *e)
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
    m_pChessBoard->GetEventSet()->setMutedState(true);
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
    m_pChessBoard->GetEventSet()->setMutedState(false);
    CRect rcMargin = pSkin->GetMargin();
    CSize szCellAll = szBoard - CSize(rcMargin.left + rcMargin.right,rcMargin.top+rcMargin.bottom);

    //calc cell size
    m_cellWidth = szCellAll.cx * scale / 17;
    m_cellHeight = szCellAll.cy * scale / 17;

    rcMargin.left *= scale;
    rcMargin.top *= scale;
    rcMargin.right *= scale;
    rcMargin.bottom *= scale;
    rcBoard.DeflateRect(rcMargin);
    m_ptBoardOrigin.x = rcBoard.left;
    m_ptBoardOrigin.y = rcBoard.bottom;

    m_pGameBoard->SDispatchMessage(UM_SETSCALE, scale*100, 1);
}

void CJunqiGame::OnBtnTest()
{
}

void CJunqiGame::OnTimer(UINT_PTR uIDEvent)
{
    if(uIDEvent == TIMERID_CLOCK){
        m_pMainDlg->KillTimer(uIDEvent);
        IWindow *pClock = m_pGameBoard->FindChildByID(ID_ALARM_CLOCK);
        int nSecond = pClock->GetUserData();
        UpdateClock(nSecond-1);
    }else{
        SetMsgHandled(FALSE);
    }
}

void CJunqiGame::OnConnected()
{
    SLOGI() << "Connected to server";
    IWindow *pConn = m_pGameBoard->FindChildByName(Sprites::connecting);
    if (pConn)
    {
        m_pGameBoard->RemoveIChild(pConn);
        pConn->Release();
    }
    OnStageChanged(STAGE_CONTINUE);
}

void CJunqiGame::OnDisconnected()
{
    SLOGI() << "Disconnected from server";
    OnStageChanged(STAGE_CONNECTING);
}

void CJunqiGame::OnFlagPos(const void *pData, int nSize){
    const MSG_FLAGPOS *pMsg = (const MSG_FLAGPOS *)pData;
    int nSeat = Index2Seat(pMsg->iIndex);
    if (nSeat != 0)
    {
        POINT ptFlag = PtNet2Local(pMsg->pt);
        m_layout.ViewChessman(ptFlag, OR_FLAG);
        CHESSMAN chessman = m_layout.GetCellChessMan(ptFlag);
        CJunqiPiece *pPiece = m_pGameBoard->FindChildByID2<CJunqiPiece>(ID_CHESS_BASE + nSeat * 100 + chessman.nID);
        pPiece->SetChessman(chessman.nOffRank);    
    }
    PlayEffectSound(Sounds::Effects::kShowFlag);
}

BOOL CJunqiGame::OnMessage(DWORD dwType, std::shared_ptr<std::vector<BYTE> > data)
{
    const LPBYTE pMsg = data ? data->data() : NULL;
    DWORD dwLen = data ? data->size() : 0;
    SLOGI() << "Received message, type: " << dwType << ", size: " << dwLen;
    switch(dwType)
    {
    case GLID_JQSTARTFIGHT:
        OnStartFight(pMsg, dwLen);
        break;
    case GLID_JQALLSTARTED:
        OnGameStart(pMsg, dwLen);
        break;
    case MSG_NOTIFY_PLAYER_LOSE:
        m_lstHistory.push_back(std::make_pair(dwType, data));
        OnPlayerLose(pMsg, dwLen);
        break;
    case MSG_NOTIFY_PLAYER_ACTIVE:
        OnActivePlayer(pMsg, dwLen);
        break;
    case MSG_NOTIFY_FLAGPOS:
        OnFlagPos(pMsg, dwLen);
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
    case MSG_REQ_MOVE:
        m_lstHistory.push_back(std::make_pair(dwType, data));
        OnMoveChess(pMsg, dwLen);
        break;
    case MSG_REQ_PEACE:
        OnReqPeace(pMsg, dwLen);
        break;
    case MSG_ACK_PEACE:
        OnAckPeace(pMsg,dwLen);
        break;
    case MSG_ACK_GAMEOVER:
        m_lstHistory.push_back(std::make_pair(dwType, data));
        OnGameOver(pMsg, dwLen);
        break;
    case MSG_REQ_SURRENDER:
        m_lstHistory.push_back(std::make_pair(dwType, data));
        OnReqSurrender(pMsg, dwLen);
        break;
    case GMT_SEATDOWN_ACK:
        OnSeatDownAck(pMsg, dwLen);
        break;
    case MSG_REQ_PASS:
        m_lstHistory.push_back(std::make_pair(dwType, data));
        OnPlayerPass(pMsg, dwLen);
        break;
    default:
        SLOGW() << "Unknown message type: " << dwType;
        break;
    }
    
    return TRUE;
}

void CJunqiGame::OnSeatDownAck(const void *lpData, int nSize){
    GAME_SEATDOWN_ACK *pAck = (GAME_SEATDOWN_ACK *)lpData;
    if(pAck->bSuccess && m_stage == STAGE_CONTINUE){
        m_iSelfIndex = pAck->nSeat;
        //init pieces color
        for(int i=0;i<25;i++){
            CJunqiPiece *pPiece = m_pGameBoard->FindChildByID2<CJunqiPiece>(ID_CHESS_BASE + i);
            if(pPiece){
                pPiece->SetColor(m_iSelfIndex);
            }   
        }
    }
}

void CJunqiGame::InitLayout(BYTE byLayout[13], int idx){
    int iSeat = Index2Seat(idx);
    m_bReady[iSeat] = TRUE;
    m_layout.InitLayout(iSeat, byLayout);
    const SOUI::SList<CHESSMAN> * pTroopList = m_layout.GetTroopList(iSeat);
    SXmlNode xmlPiece = m_pTheme->GetTemplate(Template::kPiece);
    for(SOUI::SPOSITION it = pTroopList->GetHeadPosition(); it ; )
    {
        CHESSMAN  cm = pTroopList->GetNext(it);
        CJunqiPiece *pPiece = (CJunqiPiece *)SApplication::getSingletonPtr()->CreateWindowByName(xmlPiece.attribute(L"wndclass").as_string());
        pPiece->InitFromXml(&xmlPiece);
        SAnchorLayoutParam *pParam = (SAnchorLayoutParam *)pPiece->GetLayoutParam();
        pParam->pos.type = 10;
        pParam->pos.x.fSize = cm.ptCell.x;
        pParam->pos.y.fSize = cm.ptCell.y;
        pPiece->SetColor(idx);
        pPiece->SetChessman(cm.nOffRank);
        pPiece->SetPos(cm.ptCell);
        pPiece->SetID(ID_CHESS_BASE + iSeat * 100 + cm.nID);
        float fRotate = pPiece->CalcRotate(cm.ptCell,m_iSelfIndex);
        pPiece->SetRotate(fRotate);
        m_pGameBoard->InsertChild(pPiece);
    }
    m_pGameBoard->RequestRelayout();
}

void CJunqiGame::OnStartFight(const void *pData, int nSize)
{
    SLOGI() << "Start fight";
    if(nSize < sizeof(MSGSTARTFIGHT))
        return;
    MSGSTARTFIGHT *pStart = (MSGSTARTFIGHT *)pData;
    if(pStart->iIndex == m_iSelfIndex)
        return;
    InitLayout(pStart->byLayout, pStart->iIndex);
}

void CJunqiGame::OnGameStart(const void *pData, int nSize)
{
    //remove ready flag
    for(int i = 0; i < PLAYER_COUNT; i++)
    {
        SWindow *pAvatar = m_pGameBoard->FindChildByID(ID_AVATAR_BASE + i);
        pAvatar->FindChildByName(L"flag_ready")->SetVisible(FALSE, TRUE);
        m_userInfo[i] = *m_pUserInfo[i].get();//backup user info
    }
    // set toolbar for playing
    STabCtrl *pToolBar = (STabCtrl *)m_pTheme->GetWidget(Widgets::ktab_toolbar);
    pToolBar->SetCurSel(1);
    pToolBar->GetPage(1)->EnableWindow(TRUE, TRUE);

    OnStageChanged(STAGE_PLAYING);
    MSGALLSTART * pStart = (MSGALLSTART *)pData;
    OnSetActivePlayerIndex(pStart->iFstIndex);
    PlayEffectSound(Sounds::Effects::kstart_game);
}

//将物理坐标转换成逻辑坐标
POINT CJunqiGame::PtNet2Local(POINT ptNet) const
{
    return CJunQiMap::RotateCoordinate(ptNet, m_iSelfIndex, SIDE_BOTTOM);
}

POINT CJunqiGame::PtLocal2Net(POINT ptLocal) const
{
    return CJunQiMap::RotateCoordinate(ptLocal, SIDE_BOTTOM, m_iSelfIndex);
}

void CJunqiGame::OnMoveChess(const void *pData, int nSize)
{
    SLOGI() << "Move chess";
    MSG_MOVE *pMove = (MSG_MOVE *)pData;
    POINT ptFrom = PtNet2Local(pMove->ptBegin);
    POINT ptTo = PtNet2Local(pMove->ptEnd);
    DoMovePiece(ptFrom, ptTo, pMove->bSapper, pMove->result);
}

void CJunqiGame::OnReqPeace(const void *pData, int nSize){
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

void CJunqiGame::OnAckPeace(const void *pData, int nSize)
{
    MSG_PEACE *pPeace = (MSG_PEACE *)pData;
    if(pPeace->iIndex == m_iSelfIndex)
        return;
    SLOGI() << "Peace ack, desc: " << pPeace->szMsg;
    if(pPeace->nResult == 1){
        PlayTip(_T("同意和棋"));
    }else{
        std::shared_ptr<GS_USERINFO> pUserInfo = m_pUserInfo[Index2Seat(pPeace->iIndex)];
        if(pUserInfo){
            SStringT strName = S_CA2T(pUserInfo->szName, CP_UTF8);
            PlayTip(SStringT().Format(_T("对方不同意和棋,%s"), strName.c_str()));
        }else{
            PlayTip(_T("对方不同意和棋"));
        }
    }
}


void CJunqiGame::OnGifVectoryOver(IEvtArgs *e)
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


void CJunqiGame::OnGameOver(const void *pData, int nSize)
{
    MSG_GAMEOVER *pOver = (MSG_GAMEOVER *)pData;
    SLOGI() << "Game over, desc: " << pOver->szDesc;
    memcpy(m_byInitLayouts, pOver->byLayout, sizeof(m_byInitLayouts));

    PlayTip(S_CA2T(pOver->szDesc, CP_UTF8));
    HideClock();
    for(int i = 0; i < PLAYER_COUNT; i++){
        m_bReady[i] = FALSE;
    }
    if(pOver->iWinner==-1){
        m_roundResult = RESULT_DRAW;
    }else if(pOver->iWinner == m_iSelfIndex){
        m_roundResult = RESULT_WIN;
        SGifPlayer *pVectory = (SGifPlayer *)m_pTheme->GetWidget(Sprites::sprite_vectory);
        m_pGameBoard->InsertIChild(pVectory);
        pVectory->AddRef();
        pVectory->SetVisible(TRUE, TRUE);
        // restart vectory animation.
        EventGifPlayOver e(pVectory);
        OnGifVectoryOver(&e);
    }else{
        m_roundResult = RESULT_LOSE;
    }
    if(!m_bReplay)
        OnStageChanged(STAGE_GAMEOVER);
}

void CJunqiGame::OnPlayerLose(const void *pData, int nSize){
    if(nSize < sizeof(MSG_PLAYERLOSE))
        return;
    MSG_PLAYERLOSE *pLose = (MSG_PLAYERLOSE *)pData;
    int nSeat = Index2Seat(pLose->iIndex);
    SLOGI()<<"player lose, seat:"<<nSeat;
    m_layout.PlayerLose(nSeat);
    for(int i=0;i<25;i++){
        //删除所有棋子
        CJunqiPiece *pPiece = m_pGameBoard->FindChildByID2<CJunqiPiece>(ID_CHESS_BASE+ nSeat*100 + i);
        if(pPiece){
            pPiece->Destroy();
        }
    }
    if(nSeat == 0){
        //disable the tab
        STabCtrl *pToolBar = (STabCtrl *)m_pTheme->GetWidget(Widgets::ktab_toolbar);
        pToolBar->GetPage(1)->EnableWindow(FALSE, TRUE);
    }
}

void CJunqiGame::OnReqSurrender(const void *pData, int nSize)
{
    if(nSize < sizeof(MSG_SURRENDER))
        return;
    MSG_SURRENDER *pSurrender = (MSG_SURRENDER *)pData;
    SLOGI() << "Surrender, iIndex: " << pSurrender->iIndex;
    MSG_PLAYERLOSE playerLose;
    playerLose.iIndex = pSurrender->iIndex;
    OnPlayerLose(&playerLose, sizeof(playerLose));

    int nSeat = Index2Seat(pSurrender->iIndex);
    std::shared_ptr<GS_USERINFO> pUserInfo = m_pUserInfo[nSeat];
    if(pUserInfo){
        SStringT strName = S_CA2T(pUserInfo->szName, CP_UTF8);
        SStringT strMsg = SStringT().Format(_T("玩家%s认输"), strName.c_str());
        PlayTip(strMsg);
    }else{
        PlayTip(_T("玩家认输"));
    }
}

void CJunqiGame::OnPlayerPass(const void *pData, int nSize){
    if(nSize < sizeof(MSG_PASS))
        return;
    MSG_PASS *pPass = (MSG_PASS *)pData;
    int nSeat = Index2Seat(pPass->iIndex);
    std::shared_ptr<GS_USERINFO> pUserInfo = m_pUserInfo[nSeat];
    if(pUserInfo){
        SStringT strName = S_CA2T(pUserInfo->szName, CP_UTF8);
        SStringT strMsg = SStringT().Format(_T("玩家%s超时"), strName.c_str());
        PlayTip(strMsg);
    }else{
        PlayTip(_T("玩家超时"));
    }
}

void CJunqiGame::OnActivePlayer(const void *pData, int nSize)
{ 
    MSG_PLAYERACTIVE *pActive = (MSG_PLAYERACTIVE *)pData;
    OnSetActivePlayerIndex(pActive->iIndex);
}
void CJunqiGame::OnLoginAck(const void *pData, int nSize)
{
    GAME_LOGIN_ACK *pAck = (GAME_LOGIN_ACK *)pData;
    if(pAck->errCode == ERR_SUCCESS){
        memcpy(m_dwProps,pAck->dwProps,sizeof(m_dwProps));
        m_layout.SetProperty(PLAYER_COUNT,m_dwProps[PROPID_BOMBFLY],m_dwProps[PROPID_MINEASBOMB]);
    }
}

void CJunqiGame::OnGameMsg(const void *pData, int nSize)
{
    SStringA msgUtf8((LPCSTR)pData, nSize);
    PlayTip(S_CA2T(msgUtf8, CP_UTF8));
}

void CJunqiGame::OnAvatarAck(const void *pData, int nSize)
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

int CJunqiGame::GetActivePlayerIndex() const
{
    return m_iActiveIndex;
}

void CJunqiGame::UpdateClock(int nSecond)
{
    if(m_bReplay)
        return;
    nSecond %=100;  //max 99s
    SWindow *pClock = m_pGameBoard->FindChildByID(ID_ALARM_CLOCK);
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
        OnBtnReqPass();
    }
    if(nSecond>0){
        m_pMainDlg->SetTimer(TIMERID_CLOCK, 1000);
    }
    if(nSecond<=10)
    {
        PlayEffectSound(Sounds::Effects::kcount_down);
    }    
}

void CJunqiGame::PlayTip(const SStringT &strTip)
{
    if(!m_bEnableAnimation)
        return;
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

void CJunqiGame::wsSendMsg(DWORD dwType, LPVOID lpData, DWORD dwSize)
{
    m_webSocketClient->SendMsg(dwType, lpData, dwSize);
}

void CJunqiGame::PlayEffectSound(LPCWSTR pszSound)
{
    if(!m_bEnableAnimation){
        return;
    }
    SStringW strFile = m_pTheme->GetEffectSoundFile(pszSound);
    if(!strFile.IsEmpty()){
        TCHAR szResPrefix[MAX_PATH]={0};
        SApplication::getSingletonPtr()->GetFilePrefix(szResPrefix, MAX_PATH);
        SStringT strFull = SStringT(szResPrefix) + S_CW2T(strFile);
        m_pMainDlg->PlayWave(strFull);
    } 
}

void CJunqiGame::OnChessPieceClick(IEvtArgs *e){
    SLOGI()<<"chess piece clicked, id="<<e->Sender()->GetID();
    CJunqiPiece *pPiece = sobj_cast<CJunqiPiece>(e->Sender());
    POINT pt = pPiece->GetPos();
    if (m_nSelectedChessID != -1)
    {
        if(m_stage == STAGE_PLAYING){
            if (pPiece->GetColor()%2 != (m_iSelfIndex%2))
            {//enemy piece clicked
                MovePiece(m_nSelectedChessID, pt);
            }
            else if(pPiece->GetColor() == m_iSelfIndex)
            {
                DeselectPiece();
                SelectPiece(pPiece->GetID());
            }
        }else if(m_stage == STAGE_CONTINUE){
            //swap clicked piece with selected piece
            CJunqiPiece *pSelectedPiece = m_pGameBoard->FindChildByID2<CJunqiPiece>(m_nSelectedChessID);
            POINT ptSelected = pSelectedPiece->GetPos();
            DeselectPiece();
            if (!m_layout.ExchangeChessman(ptSelected, pt))
            {
                LayoutError err1 = m_layout.IsValidCellForChessman(ptSelected, pPiece->GetChessman());
                LayoutError err2 = m_layout.IsValidCellForChessman(pt, pSelectedPiece->GetChessman());
                LayoutError err = err1 != LE_OK ? err1 : err2;
                switch(err){
                    case LE_INVALID_CELL:
                        PlayTip(_T("不能将棋子放在此兵营"));
                        break;
                    case LE_INVALID_FLAG:
                        PlayTip(_T("军旗只能放在大本营"));
                        break;
                    case LE_INVALID_MINE:
                        PlayTip(_T("地雷只能放在最后两排"));
                        break;
                    case LE_INVALID_BOMB:
                        PlayTip(_T("炸弹不能放在第一排"));
                        break;
                }
                return;
            }
            pSelectedPiece->SetPos(pt);
            pPiece->SetPos(ptSelected);

            //animate swap the two pieces.
            AnchorPos toPos;
            toPos.type = 10;
            toPos.x = SLayoutSize(ptSelected.x,dp);
            toPos.y = SLayoutSize(ptSelected.y,dp);
            toPos.fOffsetX = -0.5f;
            toPos.fOffsetY = -0.5f;
            pPiece->SetLayer(SGameTheme::LAYER_PIESE + 1);
            auto pAnim = Util::MoveSpriteTo(pPiece, toPos, 200);
            pAnim->addListener(this);
            pAnim->SetID(ANI_SWAP);

            toPos.x = SLayoutSize(pt.x,dp);
            toPos.y = SLayoutSize(pt.y,dp);
            pPiece->SetLayer(SGameTheme::LAYER_PIESE + 1);
            pAnim = Util::MoveSpriteTo(pSelectedPiece, toPos, 200);
            pSelectedPiece->SetLayer(SGameTheme::LAYER_PIESE);
            pAnim->addListener(this);
            pAnim->SetID(ANI_SWAP);
        }
    }else if (pPiece->GetColor() == m_iSelfIndex)
    {
        if((m_stage == STAGE_PLAYING && GetActivePlayerIndex() == m_iSelfIndex)
            || m_stage == STAGE_CONTINUE){
            SelectPiece(pPiece->GetID());
        }
    }
}

void CJunqiGame::OnBoardClick(IEvtArgs *e){
    if(m_stage != STAGE_PLAYING){
        return;
    }
    EvtGridClick *pEvt = sobj_cast<EvtGridClick>(e);
    SLOGI()<<"board clicked, pt="<<pEvt->pt;
    if (m_nSelectedChessID != -1)
    {
        MovePiece(m_nSelectedChessID, pEvt->pt);
    }
}