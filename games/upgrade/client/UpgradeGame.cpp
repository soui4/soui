#include "stdafx.h"
#include "UpgradeGame.h"
#include "upgcard.h"
#include "MainDlg.h"
#include <helper/SFunctor.hpp>
#include <protocol.h>
#include <helper/slog.h>
#include <sstream>
#include <mmsystem.h>
#include "myprofile.h"
#include <UpgAlgorithm.h>
#define kLogTag "UpgradeGame"

//#define ENABLE_MOCK

#define TIMERID_POSTCARD	500		//发牌定时器
#define TIMERID_SHOW_BOTTOM 501
#define TIMERID_CLOCK 503
#define TIMERID_THROW_FAILED 504 //甩牌失败时收回牌的定时器
#define TIMERID_THROW_FAILED2 505 //甩牌失败时重新出牌定时器
#define TIMERID_TURNOVER 510
#define TIMERID_ROUNDEND 511
#define TIMERID_SETBOTTOM 512
#define TIMERID_AUTO_PUTCARD 513

enum AniGroupID{
    AGID_POSTCARD_DONE = 100,
};
/**
 * @brief 构造函数
 * @param pMainDlg 主窗口指针
 */
UpgradeGame::UpgradeGame(CMainDlg* pMainDlg, SGameTheme* pTheme)
    : m_pMainDlg(pMainDlg)
    ,m_stage(STAGE_INIT)
    ,m_pTheme(pTheme)
    ,m_nCardInStack(0)
    ,m_iSelfIndex(0)
    ,m_iBankerIndex(-1)
    ,m_nLevelA(1)
    ,m_nLevelB(1)
    ,m_iSetBottomIndex(-1)
    ,m_nMainColor(-1)
    ,m_iCurPutCardIndex(-1)
    ,m_bBottomColor(FALSE)
    ,m_nScore(0)
    ,m_nDiscount(0)
    ,m_bShowBottom(FALSE)
    ,m_bShowPrevRound(FALSE)
    ,m_putCard(this)
    ,m_roundResult(RESULT_UNKNOWN)
{
    memset(m_pUserInfo, 0, sizeof(m_pUserInfo));
    m_curShowMain.index = -1;
    m_curShowMain.nCount = 0;
    m_curShowMain.nCard[0] = m_curShowMain.nCard[1] = -1;

    m_cardInHand[SEAT_INDEX_BOTTOM].Init(this,TRUE);
    m_cardInHand[SEAT_INDEX_LEFT].Init(this,FALSE);
    m_cardInHand[SEAT_INDEX_TOP].Init(this,TRUE);
    m_cardInHand[SEAT_INDEX_RIGHT].Init(this,FALSE);


    m_cardInHand[SEAT_INDEX_BOTTOM].SetBase(m_pTheme->GetAnchorPos(AnchorName::kCardBottom));
    m_cardInHand[SEAT_INDEX_LEFT].SetBase(m_pTheme->GetAnchorPos(AnchorName::kCardLeft));
    m_cardInHand[SEAT_INDEX_TOP].SetBase(m_pTheme->GetAnchorPos(AnchorName::kCardTop));
    m_cardInHand[SEAT_INDEX_RIGHT].SetBase(m_pTheme->GetAnchorPos(AnchorName::kCardRight));

    m_putCard.SetBase(m_pTheme->GetAnchorPos(AnchorName::kPutCardBottom),m_pTheme->GetAnchorPos(AnchorName::kPutCardLeft),m_pTheme->GetAnchorPos(AnchorName::kPutCardTop),m_pTheme->GetAnchorPos(AnchorName::kPutCardRight));
    m_putCard.SetAvatarPos(m_pTheme->GetAnchorPos(AnchorName::kAvatarBottom),m_pTheme->GetAnchorPos(AnchorName::kAvatarLeft),m_pTheme->GetAnchorPos(AnchorName::kAvatarTop),m_pTheme->GetAnchorPos(AnchorName::kAvatarRight));
    m_putCard.SetScoreCardPos(m_pTheme->GetAnchorPos(AnchorName::kScoreCard));
     // 初始化准备状态
    for(int i = 0; i < 4; i++)
    {
        m_bReady[i] = FALSE;
    }
}

/**
 * @brief 析构函数
 */
UpgradeGame::~UpgradeGame()
{
}

void UpgradeGame::Init(SWindow *pGameBoard, WebSocketClient *pWs)
{
    #ifndef ENABLE_MOCK
    pGameBoard->FindChildByName(L"btn_test")->SetVisible(FALSE, TRUE);
    #endif//ENABLE_MOCK
    m_pGameBoard = pGameBoard;
    m_webSocketClient = pWs;
    IWindow * pScoreContainer = m_pTheme->GetWidget(Sprites::win_score_container);
    m_pGameBoard->InsertIChild(pScoreContainer);
    pScoreContainer->AddRef();

    //load self info
    MyProfile *pMyProfile = MyProfile::getSingletonPtr();
    m_pUserInfo[0] = pMyProfile->GetUserInfo();
    m_userAvatar[0] = pMyProfile->GetAvatarData();

    //load avatars
    {
        SXmlNode xmlAvatar = m_pTheme->GetTemplate(L"avatar");
        SStringW strClsName = xmlAvatar.attribute(L"wndclass").as_string(L"window");
        for(int i = 0; i < 4; i++){
            IWindow *pAvatar = SApplication::getSingletonPtr()->CreateWindowByName(strClsName);
            pAvatar->InitFromXml(&xmlAvatar);
            m_pGameBoard->InsertIChild(pAvatar);
            SAnchorLayoutParam *pParam = (SAnchorLayoutParam*)pAvatar->GetLayoutParam();
            pParam->pos = m_pTheme->GetAnchorPos(AnchorName::kAvatarBottom + i);
            pAvatar->SetID(ID_AVATAR_BASE + i);
        }
        SWindow *pAvatar = m_pGameBoard->FindChildByID(ID_AVATAR_BASE + 0);
        pAvatar->FindChildByName(L"txt_name")->SetWindowText(pMyProfile->GetName());
        if(pMyProfile->GetAvatarData()){
            SGifPlayer *pGif = pAvatar->FindChildByName2<SGifPlayer>(L"gif_avatar");
            pGif->LoadFromMemory((LPVOID)pMyProfile->GetAvatarData()->data(), pMyProfile->GetAvatarData()->size());
        }
    }
    //load avatar flags
    {
        SXmlNode xmlColor = m_pTheme->GetTemplate(L"avatar_flags");
        SStringW strClsName = xmlColor.attribute(L"wndclass").as_string(L"window");
        for(int i = 0; i < 4; i++){
            IWindow *pColorFlag = SApplication::getSingletonPtr()->CreateWindowByName(strClsName);
            pColorFlag->InitFromXml(&xmlColor);
            m_pGameBoard->InsertIChild(pColorFlag);
            pColorFlag->SetID(ID_AVATAR_FLAGS_BASE + i);
            SAnchorLayoutParam *pParam = (SAnchorLayoutParam*)pColorFlag->GetLayoutParam();
            pParam->pos = m_pTheme->GetAnchorPos(AnchorName::kColorBottom + i);
            pColorFlag->RequestRelayout();
        }
    }
    {
        //load alarm clock
        IWindow *pClock = m_pTheme->GetWidget(Sprites::img_clock);
        m_pGameBoard->InsertIChild(pClock);
        pClock->AddRef();
        pClock->SetVisible(FALSE,FALSE);
        pClock->SetID(ID_ALARM_CLOCK);
    }
    m_iBankerIndex = -1;
    OnStageChanged(STAGE_CONNECTING);
}


void UpgradeGame::OnBtnStart()
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

    m_webSocketClient->SendMsg(GMT_READY, NULL, 0);
    #ifdef ENABLE_MOCK
    m_bReady[0] = TRUE;
    OnStageChanged(STAGE_WAIT_START);
    #endif
    PlayEffectSound(Sounds::Effects::kstart_game);
}

int UpgradeGame::GetLevel() const
{
	if(m_iBankerIndex==-1)
		return m_nLevelA;
	else if(m_iBankerIndex%2==0)
		return m_nLevelA;
	else
		return m_nLevelB;
}

void UpgradeGame::OnBtnReportRange(int nID)
{
    int nColor = nID - Sprites::btn_diamond;
    GAME_UPGRADE_SHOWMAIN req;
    req.index = m_iSelfIndex;
    req.nCount = m_cardInHand[SEAT_INDEX_BOTTOM].TestReport(nColor);
    if (req.nCount == 0)
    {
        SLOGI() << "无法亮主, color=" << nColor;
        return;
    }
    if (m_curShowMain.index == -1 && nColor < COLOR_SKING)
    { // 没有人报牌时默认报单张
        req.nCount = 1;
    }
    for(int i=0;i<req.nCount;i++)
        req.nCard[i] = CUpgCard::ClientGetCard(nColor, GetLevel());
    if (m_stage == STAGE_REBELBOTTOM)
        PlayTip(_T("反底请求己发送，正在等待其它用户的反底信息，请稍候..."));
    wsSendMsg(GMT_UPGRADE_REQSHOWMAIN, &req, sizeof(req));
    
    // 添加音效播放
    PlayEffectSound(Sounds::Effects::kbutton_focus);
}

void UpgradeGame::OnBtnSetBottom()
{
    if(m_stage != STAGE_SETBOTTOM)
        return;
    if(m_iSelfIndex != m_iSetBottomIndex)
        return;
    SLOGI() << "OnBtnSetBottom: player=" << m_iSelfIndex;
    SList<SAutoRefPtr<SSpriteCard> > cards;
    m_cardInHand[SEAT_INDEX_BOTTOM].GetSelectedCards(cards);
    int nCount = cards.GetCount();
    if(nCount != 8){
        PlayTip(_T("请选择8张牌"));
        return;
    }
    int nCard[8];
    int i = 0;
    for (SPOSITION it = cards.GetHeadPosition(); it;)
    {
        SSpriteCard *pCard = cards.GetNext(it);
        nCard[i++] = pCard->GetCard();
    }
    GAME_UPGRADE_REQPUTBOTTOM req;
    req.ownerIndex = m_iSelfIndex;
    memcpy(req.nBottom, nCard, 8 * sizeof(int));
    m_cardInHand[SEAT_INDEX_ME].SetSelectable(FALSE);
    wsSendMsg(GMT_UPGRADE_REQPUTBOTTOM, &req, sizeof(req));
    
    m_pGameBoard->FindChildByName(Sprites::btn_set_bottom)->SetVisible(FALSE, TRUE);
    // 添加音效播放
    PlayEffectSound(Sounds::Effects::kset_bottom);
}

void UpgradeGame::OnBtnPass()
{
    wsSendMsg(GMT_UPGRADE_REQREBELPASS, NULL, 0);
    IWindow *pWnd = m_pGameBoard->FindChildByName(Sprites::win_show_main_box);
    pWnd->SetVisible(FALSE,TRUE);
    HideClock();
}

void UpgradeGame::OnBtnPutCard()
{
    SASSERT(m_stage == STAGE_PUTCARD);

    if(m_iCurPutCardIndex != m_iSelfIndex){
        SLOGW()<<"OnBtnPutCard, not my turn";
        return;
    }

    SList<SAutoRefPtr<SSpriteCard> > cards;
    m_cardInHand[SEAT_INDEX_BOTTOM].GetSelectedCards(cards);
    if(m_iTurnFirstIndex != m_iSelfIndex){
        int nCount = cards.GetCount();
        if(nCount != m_nPutCardCount){
            PlayTip(_T("请选择正确的牌数"));
            return;
        }
    }
    GAME_UPGRADE_REQPUTCARD req;
    req.nIndex = m_iSelfIndex;
    int i=0;
    for(SPOSITION pos = cards.GetHeadPosition(); pos; i++){
        auto pCard = cards.GetNext(pos);
        req.nCard[i] = pCard->GetCard();
    }
    req.nCount = i;

    #ifdef ENABLE_MOCK
    if(req.nCount == 6 && m_iTurnFirstIndex == m_iSelfIndex){
        //模拟一个甩牌失败事件
        GAME_UPGRADE_THROWFAILED fail;
        fail.nIndex = m_iSelfIndex;
        fail.nDiscount = 30;
        fail.nPutCount = 6;
        memcpy(fail.nPutCard, req.nCard, 6 * sizeof(int));
        fail.nFailedCount = 3;
        fail.nFailedCard[0] = req.nCard[1];
        fail.nFailedCard[1] = req.nCard[3];
        fail.nFailedCard[2] = req.nCard[5];
        OnThrowFailed(&fail, sizeof(fail));
    }else{
        wsSendMsg(GMT_UPGRADE_REQPUTCARD, &req, sizeof(req));
    }
    #else
    wsSendMsg(GMT_UPGRADE_REQPUTCARD, &req, sizeof(req));
    #endif//ENABLE_MOCK
    
    m_pGameBoard->FindChildByName(Sprites::btn_put_card)->SetVisible(FALSE, TRUE);
    // 添加音效播放
    PlayEffectSound(Sounds::Effects::kbutton_focus);
}

void UpgradeGame::OnBtnPrevRound()
{
    if(m_stage != STAGE_PUTCARD)
        return;
    m_bShowPrevRound = !m_bShowPrevRound;
    m_putCard.ShowPrevRound(m_bShowPrevRound);
}

void UpgradeGame::OnBtnSeeBottom()
{
    if (m_iSetBottomIndex == m_iSelfIndex && m_stage == STAGE_PUTCARD)
    {
        m_bShowBottom = !m_bShowBottom;
        for(int i=0;i<8;i++){
            SSpriteCard * pCard = m_pGameBoard->FindChildByID2<SSpriteCard>(ID_STACK_CARD_BASE+i);
            pCard->SetCard(m_bShowBottom?m_nCardBottom[i]:-1);
        }
    }
}

void UpgradeGame::OnBtnTest()
{
    if (m_stage == STAGE_WAIT_START)
    {
        // self had seat down, mock other ready message.
        GAME_TABLE_INFO *buf = (GAME_TABLE_INFO*)malloc(sizeof(GAME_TABLE_INFO)+3*sizeof(SEAT_INFO));
        GAME_TABLE_INFO &tableInfo = *buf;
        tableInfo.nTableId = MyProfile::getSingletonPtr()->GetTableId();
        tableInfo.nPlayers = 4;
        for (int i = 0; i < 4; i++)
        {
            tableInfo.seatInfo[i].nIndex = i;
            if (i == m_iSelfIndex)
            {
                tableInfo.seatInfo[i].bReady = TRUE;
                tableInfo.seatInfo[i].stUserInfo = *m_pUserInfo[0];
            }
            else
            {
                tableInfo.seatInfo[i].bReady = TRUE;
                tableInfo.seatInfo[i].stUserInfo.uid = 1000 + i;
                sprintf(tableInfo.seatInfo[i].stUserInfo.szName, "player%d", i);
                tableInfo.seatInfo[i].stUserInfo.nSex = SEX_MALE;
            }
        }
        EventTableInfo evt(this);
        evt.pTableInfo = &tableInfo;
        OnTableInfo(&evt);
        free(buf);
        wsSendMsg(GMT_START, NULL, 0);
    }
    else if (m_stage == STAGE_WAIT_CARD)
    {
        // mock post card done.
        srand(2); // mock random seed
        int nCards[25];
        for (int i = 0; i < 25; i++)
        {
            nCards[i] = rand() % 54;
        }
        nCards[20] = 14;
        nCards[23] = 14;
        nCards[2] = 26;
        nCards[3] = 39;
        std::stringstream ss;
        for (int i = 0; i < 25; i++)
        {
            ss << nCards[i] << " ";
        }
        SLOGI() << "mock card: " << ss.str().c_str();
        OnPostCard(nCards, sizeof(nCards));
    }
    else if(m_stage == STAGE_POSTCARD){
        if(m_nCardInStack>8){
            //mock show color.
            if(m_curShowMain.index == -1)
            {
                GAME_UPGRADE_SHOWMAIN req;
                req.index = (m_iSelfIndex+1)%4;
                req.nCount = 1;
                req.nCard[0] =  CUpgCard::ClientGetCard(COLOR_DIAMOND,GetLevel());
                wsSendMsg(GMT_UPGRADE_REQSHOWMAIN, &req, sizeof(req));
            }else if(m_curShowMain.nCount == 1){
                GAME_UPGRADE_SHOWMAIN req;
                req.index = (m_iSelfIndex+2)%4;
                req.nCount = 2;
                req.nCard[0] = req.nCard[1] =CUpgCard::ClientGetCard(COLOR_DIAMOND,GetLevel());
                wsSendMsg(GMT_UPGRADE_REQSHOWMAIN, &req, sizeof(req));
            }
        }else{
            //牌已经发完，发底牌
            if (m_iBankerIndex == -1 && m_curShowMain.index == -1)
            {
                //模拟重新发牌
                wsSendMsg(GMT_START, NULL, 0);
            }
            else if(m_iBankerIndex != -1 && m_curShowMain.index==-1 && m_nMainColor==-1){
                //模拟从底牌抽主花色
                GAME_UPGRADE_AUTOCOLOR req;
                req.iBottomIndex = rand()%8;
                req.nCard = rand()%54;
                wsSendMsg(GMT_UPGRADE_AUTOCOLOR, &req, sizeof(req));
            }else{
                GAME_UPGRADE_POSTBOTTOM req;
                if(m_iBankerIndex == -1)
                    req.iOwnerIndex = m_curShowMain.index;
                else
                    req.iOwnerIndex = m_iBankerIndex;
                for(int i=0;i<8;i++){
                    req.nBottom[i]=rand()%54;
                }
                wsSendMsg(GMT_UPGRADE_POSTBOTTOM,&req,sizeof(req));
            }
        }
    }
    else if(m_stage == STAGE_SETBOTTOM){
        if(m_iSetBottomIndex != m_iSelfIndex){
            //模拟用户放底牌
            GAME_UPGRADE_REQPUTBOTTOM req;
            req.ownerIndex = m_iSetBottomIndex;
            for(int i=0;i<8;i++){
                req.nBottom[i]=rand()%54;
            }
            wsSendMsg(GMT_UPGRADE_REQPUTBOTTOM,&req,sizeof(req));
        }
    }
    else if(m_stage == STAGE_REBELBOTTOM){
        if(m_curShowMain.index == m_iSelfIndex 
            && m_curShowMain.nCount ==2 
            && m_iSetBottomIndex!=m_iSelfIndex)
        {//自己反底，模拟发底牌
            GAME_UPGRADE_POSTBOTTOM req;
            req.iOwnerIndex = m_iSelfIndex;
            for(int i=0;i<8;i++){
                req.nBottom[i]=rand()%54;
            }
            wsSendMsg(GMT_UPGRADE_POSTBOTTOM,&req,sizeof(req));
        }else{
            //通知庄家开始出牌
            GAME_UPGRADE_STARTPUTCARD req;
            req.winnerIndex = m_iBankerIndex;
            wsSendMsg(GMT_UPGRADE_STARTPUTCARD, &req, sizeof(req));
        }
    }
    else if(m_stage == STAGE_PUTCARD){
        if (m_iCurPutCardIndex != m_iSelfIndex)
        {
            //模拟用户出牌
            GAME_UPGRADE_REQPUTCARD req;
            req.nIndex = m_iCurPutCardIndex;
            if(m_iCurPutCardIndex == m_iTurnFirstIndex){
                req.nCount = 1;
            }else{
                req.nCount = m_nPutCardCount;
            }
            if (req.nCount)
            {
                for (int i = 0; i < req.nCount; i++)
                {
                    req.nCard[i] = rand() % 54;
                }
                wsSendMsg(GMT_UPGRADE_REQPUTCARD, &req, sizeof(req));
            }
        }
    }
}

void UpgradeGame::OnShuffleFrame(IEvtArgs *e)
{
    SGifPlayer *pSprite = sobj_cast<SGifPlayer>(e->Sender());
    if(pSprite->GetCurFrame() == 1){
        PlayEffectSound(Sounds::Effects::kshuffle);
    }
}

void UpgradeGame::OnShuffleLoopOver(IEvtArgs *e)
{
    if(m_cardRecieve.GetCount() == 0)
        return;
    //牌已经收到，停止洗牌动画。
    SGifPlayer *pSprite = sobj_cast<SGifPlayer>(e->Sender());
    pSprite->Stop();
    m_pGameBoard->RemoveIChild(pSprite);
    pSprite->Release();

    OnStageChanged(STAGE_POSTCARD);
}

void UpgradeGame::OnGifHighlightOver(IEvtArgs *e)
{
    SGifPlayer *pSprite = sobj_cast<SGifPlayer>(e->Sender());
    pSprite->Stop();
    pSprite->SetVisible(FALSE, TRUE);
}

void UpgradeGame::InitGame()
{
    m_nScore = m_nDiscount = 0;
    UpdateScore();

    //show main btns
    IWindow *pMainBtns = m_pTheme->GetWidget(Sprites::win_show_main_box);
    SASSERT(pMainBtns);
    m_pGameBoard->InsertIChild(pMainBtns);
    pMainBtns->AddRef();
    pMainBtns->SetVisible(TRUE, TRUE);
    for(int i = COLOR_DIAMOND; i <= COLOR_BKING; i++){
        pMainBtns->FindIChildByID(Sprites::btn_diamond + i)->EnableWindow(FALSE, TRUE);
    }
    pMainBtns->FindIChildByID(Sprites::btn_pass)->EnableWindow(FALSE, TRUE);

    CSize szCard;
    ISkinObj *pCard = GETSKIN(L"skin_card",GetScale());
    ISkinObj *pCardMask = GETSKIN(L"skin_card_sel",GetScale());
    szCard = pCard->GetSkinSize();

    int nAllWid= szCard.cx + kCardInterval * (kCardCount-1);
    m_nCardInStack = kCardCount;
    for(int i=0;i<kCardCount;i++)
	{
        SSpriteCard * pSprite = new SSpriteCard;
        pSprite->SetID(ID_STACK_CARD_BASE+i);
        pSprite->SetSkin(pCard,0,FALSE);
        pSprite->SetMaskSkin(pCardMask);
        pSprite->SetIcon(54);//54 is the card back.
        SAnchorLayoutParam * pParam = (SAnchorLayoutParam*)pSprite->GetLayoutParam();
        SAnchorLayoutParamStruct * pParamStruct = (SAnchorLayoutParamStruct*)pParam->GetRawData();
        pParamStruct->pos = m_pTheme->GetAnchorPos(AnchorName::kCardCenter);
        pParamStruct->pos.x = -nAllWid/2+i*kCardInterval;
        pSprite->SetLayer(i);
        pSprite->SetSelectable(FALSE);
        m_pGameBoard->InsertIChild(pSprite);
	}
}

void UpgradeGame::OnCardStartDrag(IEvtArgs *e)
{
    EventSpriteCardDragStart *pEvt = sobj_cast<EventSpriteCardDragStart>(e);
    SWindow *pSelectMask = m_pGameBoard->FindChildByName(L"mask_select");
    pSelectMask->SetVisible(TRUE,TRUE);
    pSelectMask->Move(pEvt->rcDraging);
}

void UpgradeGame::OnCardDrag(IEvtArgs *e)
{
    EventSpriteCardDrag *pEvt = sobj_cast<EventSpriteCardDrag>(e);
    SWindow *pSelectMask = m_pGameBoard->FindChildByName(L"mask_select");
    CRect rcDrag = pEvt->rcDraging;
    rcDrag.NormalizeRect();
    pSelectMask->Move(rcDrag);
    m_cardInHand[SEAT_INDEX_BOTTOM].SelectCardForRect(rcDrag);
}

void UpgradeGame::OnCardEndDrag(IEvtArgs *e)
{
    EventSpriteCardDragEnd *pEvt = sobj_cast<EventSpriteCardDragEnd>(e);
    SWindow *pSelectMask = m_pGameBoard->FindChildByName(L"mask_select");
    pSelectMask->SetVisible(FALSE,TRUE);
}

void UpgradeGame::OnGameBoardClick()
{
    m_cardInHand[SEAT_INDEX_BOTTOM].ClearSelect();
}

void UpgradeGame::OnTableInfo(IEvtArgs *e)
{
    EventTableInfo *pEvt = sobj_cast<EventTableInfo>(e);
    GAME_TABLE_INFO *pTableInfo = pEvt->pTableInfo;
    MyProfile   *pMyProfile = MyProfile::getSingletonPtr();
    if(pTableInfo->nTableId != pMyProfile->GetTableId())
        return;
    m_iSelfIndex = pMyProfile->GetSeatIndex();
    SASSERT(m_iSelfIndex >= 0 && m_iSelfIndex < 4);
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
    if(!m_bReady[0] && m_stage != STAGE_CONTINUE){
        OnGameEnd(NULL, 0);
    }
}

void UpgradeGame::OnStageChanged(STAGE stage)
{
    if(m_stage == stage)
        return;
    m_stage = stage;
    SLOGI() << "OnStageChanged, new state=" << stage;
    switch(stage)
    {
    case STAGE_SHOW_BOTTOM:
        {
            m_pMainDlg->SetTimer(TIMERID_SHOW_BOTTOM,2000,NULL);
            break;
        }
    case STAGE_WAIT_CARD:
        {
            // stop waiting sprite.
            StopWatingAnimation();
            // request card.
            wsSendMsg(GMT_UPGRADE_REQCARD, &m_iSelfIndex, sizeof(int));
            // play shuffle animation.
            IWindow *pShuffle = m_pTheme->GetWidget(Sprites::shuffle);
            m_pGameBoard->InsertIChild(pShuffle);
            pShuffle->AddRef();
            SGifPlayer *pGif = sobj_cast<SGifPlayer>(pShuffle);
            pGif->SetLoop(-1);
            pGif->Start(0);
        }
        break;
    case STAGE_CONNECTING:
        {
            IWindow *pConnecting = m_pTheme->GetWidget(Sprites::connecting);
            SASSERT(pConnecting);
            m_pGameBoard->InsertIChild(pConnecting);
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
            IWindow *pBtnStart = m_pTheme->GetWidget(Sprites::btn_start);
            SASSERT(pBtnStart);
            m_pGameBoard->InsertIChild(pBtnStart);
            pBtnStart->AddRef();
        }
        break;
    case STAGE_WAIT_START:
        {
            //remove btn_start
            if(IWindow *pBtnStart = m_pGameBoard->FindChildByName(Sprites::btn_start)){
                m_pGameBoard->RemoveIChild(pBtnStart);
                pBtnStart->Release();
            }
            PlayWattingAnimation(_T("等待游戏开始..."));
        }
        break;
    case STAGE_POSTCARD:
    {
        InitGame();
        m_pMainDlg->SetTimer(TIMERID_POSTCARD, SPEED_POSTCARD, NULL);
        OnTimer(TIMERID_POSTCARD);
    }
        break;
    case STAGE_SETBOTTOM:
    {
        int seat = Index2Seat(m_iBottomOwnerIndex);
        //收起底牌
        SList<SAutoRefPtr<SSpriteCard> > lstCards;
        for(int i=0;i<8;i++){
            SSpriteCard * pCard = m_pGameBoard->FindChildByID2<SSpriteCard>(ID_STACK_CARD_BASE+i);
            lstCards.AddTail(pCard);
            pCard->SetID(pCard->GetID()+200);//mark the card is in hand.
            if(seat != SEAT_INDEX_ME){
                pCard->SetCard(-1);
            }
        }
        // 收起亮主牌
        if (m_curShowMain.index != -1)
        {
            int seatMain = Index2Seat(m_curShowMain.index);
            SList<SAutoRefPtr<SSpriteCard> > lstMainCards;
            m_putCard.GetCard(seatMain, lstMainCards);
            if (seatMain != SEAT_INDEX_ME)
            {
                for (SPOSITION pos = lstMainCards.GetHeadPosition(); pos;)
                {
                    SAutoRefPtr<SSpriteCard> pCard = lstMainCards.GetNext(pos);
                    pCard->SetCard(-1);
                }
            }
            if(seatMain == seat){
                lstCards.AddTailList(&lstMainCards);
            }else{
                m_cardInHand[seatMain].ReceiveCards(lstMainCards);
            }
        }        
        m_cardInHand[seat].ReceiveCards(lstCards);
        m_cardInHand[SEAT_INDEX_ME].SetSelectable(TRUE);

        //remove show main box
        IWindow *pWnd = m_pGameBoard->FindChildByName(Sprites::win_show_main_box);
        pWnd->SetVisible(FALSE,TRUE);

        if(m_iSetBottomIndex == m_iSelfIndex){
            IWindow *pSetBottom = m_pTheme->GetWidget(Sprites::btn_set_bottom);
            m_pGameBoard->InsertIChild(pSetBottom);
            pSetBottom->AddRef();
            pSetBottom->SetVisible(TRUE, TRUE);
            MoveClock(SEAT_INDEX_ME, m_dwProps[PI_TIME_PUTBOTTOM]);
        }else{
            PlayWattingAnimation(_T("等待其他玩家垫底牌..."));
        }

    }
        break;
    case STAGE_REBELBOTTOM:
        if(m_dwProps[PI_RULE_REBEL] && !m_bAutoColor){
            if(m_iSetBottomIndex != m_iSelfIndex){
                if (m_curShowMain.index != m_iSelfIndex)
                {
                    IWindow *pWnd =  m_pGameBoard->FindChildByName(Sprites::win_show_main_box);
                    pWnd->SetVisible(TRUE, TRUE);
                    for (int i = 0; i <= COLOR_BKING; i++)
                    {
                        BOOL bEnable = m_cardInHand[SEAT_INDEX_ME].TestReport(i);
                        IWindow *pBtn = pWnd->FindIChildByID(Sprites::btn_diamond + i);
                        pBtn->EnableWindow(bEnable, TRUE);
                    }
                    pWnd->FindIChildByID(Sprites::btn_pass)->EnableWindow(TRUE, TRUE);
                    StopWatingAnimation();
                    //show clock
                    MoveClock(SEAT_INDEX_ME, m_dwProps[PI_TIME_REBEL]);
                }
                else
                {
                    PlayWattingAnimation(_T("等待其它玩家反底牌..."));
                }
            }else{
                PlayWattingAnimation(_T("等待其它玩家反底牌..."));
            }
        }
        break;
    case STAGE_PUTCARD:
    {//开始出牌
        // 确定这一局主牌花色
        if(m_nMainColor == -1){
            m_nMainColor = CUpgCard::ClientGetColor(m_curShowMain.nCard[0]);
            // 调整自己手牌顺序
            m_cardInHand[SEAT_INDEX_ME].SetMainColor(m_nMainColor, SPEED_POSTCARD);
        }

        //remove show main box and waiting sprite if exist
        IWindow *pWnd = m_pGameBoard->FindChildByName(Sprites::win_show_main_box);
        SASSERT(pWnd);
        m_pGameBoard->RemoveIChild(pWnd);
        pWnd->Release();

        StopWatingAnimation();
        int nSeat = Index2Seat(m_iCurPutCardIndex);
        //show put card button
        pWnd = m_pTheme->GetWidget(Sprites::btn_put_card);
        m_pGameBoard->InsertIChild(pWnd);
        pWnd->AddRef();
        pWnd->SetVisible(nSeat == SEAT_INDEX_ME,TRUE);
        m_cardInHand[SEAT_INDEX_ME].SetSelectable(TRUE);
        //show clock
        MoveClock(nSeat, m_dwProps[PI_TIME_PUTCARD]);
    }
        break;
    }    
}

void UpgradeGame::OnTimer(UINT_PTR uIDEvent)
{
    if(uIDEvent == TIMERID_CLOCK){
        m_pMainDlg->KillTimer(uIDEvent);
        IWindow *pClock = m_pGameBoard->FindChildByID(ID_ALARM_CLOCK);
        int nSecond = pClock->GetUserData();
        UpdateClock(nSecond-1);
    }
    else if(uIDEvent == TIMERID_TURNOVER){
        m_pMainDlg->KillTimer(uIDEvent);
        SASSERT(m_turnOverMsg);
        OnTurnOverTimer(m_turnOverMsg.get());
        m_turnOverMsg = nullptr;
    }else if(uIDEvent == TIMERID_ROUNDEND){
        SASSERT(m_roundEndMsg);
        m_pMainDlg->KillTimer(uIDEvent);
        OnRoundEndTimer(m_roundEndMsg.get());
        m_roundEndMsg = nullptr;
    }
    else if(uIDEvent == TIMERID_SHOW_BOTTOM){
        m_pMainDlg->KillTimer(uIDEvent);
        OnStageChanged(STAGE_SETBOTTOM);
    }
    else if(uIDEvent == TIMERID_POSTCARD)
    {
        OnTimerPostCard();
    }else if(uIDEvent == TIMERID_THROW_FAILED){
        m_pMainDlg->KillTimer(uIDEvent);
        SASSERT(m_throwFailMsg);
        OnThrowFailedTimer(m_throwFailMsg.get());
    }else if(uIDEvent == TIMERID_THROW_FAILED2){
        m_pMainDlg->KillTimer(uIDEvent);
        SASSERT(m_throwFailMsg);
        OnThrowFailedTimer2(m_throwFailMsg.get());
        m_throwFailMsg=nullptr;
    }else if(uIDEvent == TIMERID_SETBOTTOM){
        m_pMainDlg->KillTimer(uIDEvent);
        SASSERT(m_reqPutBottomMsg);
        OnSetBottomTimer(m_reqPutBottomMsg.get());
        m_reqPutBottomMsg = nullptr;
    }else if(uIDEvent == TIMERID_AUTO_PUTCARD){
        m_pMainDlg->KillTimer(uIDEvent);
        OnBtnPutCard();
    }
    else{
        SetMsgHandled(FALSE);
    }
}

void UpgradeGame::DoPostCard()
{
    int nCard,iToSeat;
    if(m_cardRecieve.FatchCard(nCard,iToSeat))
    {
        SSpriteCard * pSprite = (SSpriteCard*)m_pGameBoard->FindChildByID(ID_STACK_CARD_BASE+m_nCardInStack-1);
        SASSERT(pSprite);
        pSprite->SetCard(nCard);
        pSprite->SetID(pSprite->GetID()+200);//mark the card is in hand.
        int nPlayer = Index2Seat(iToSeat);
        switch(nPlayer){
            case 0:
            {
                SASSERT(nCard != -1);
                m_cardInHand[SEAT_INDEX_BOTTOM].ReceiveCard(pSprite);
                for(int i = COLOR_DIAMOND;i<=COLOR_BKING;i++){
                    int nRet = m_cardInHand[SEAT_INDEX_BOTTOM].TestReport(i);
                    SWindow * pBtn = m_pGameBoard->FindChildByID(Sprites::btn_diamond + i);
                    SASSERT(pBtn);
                    BOOL bEnable = !pBtn->IsDisabled();
                    if(bEnable != (nRet != 0))
                    {
                        pBtn->EnableWindow(nRet != 0, TRUE);
                        if(nRet != 0){
                            SGifPlayer *pGif = sobj_cast<SGifPlayer>(pBtn->FindChildByID(Sprites::gif_highlight));
                            SASSERT(pGif);
                            pGif->SetVisible(nRet!=0, TRUE);
                            pGif->Start(0);
                        }
                    }
                }
            }
            break;
            case 1:
            m_cardInHand[SEAT_INDEX_LEFT].ReceiveCard(pSprite);
            break;
            case 2:
            m_cardInHand[SEAT_INDEX_TOP].ReceiveCard(pSprite);
            break;
            case 3:
            m_cardInHand[SEAT_INDEX_RIGHT].ReceiveCard(pSprite);
            break;
        }
        m_nCardInStack--;
    }
}
void UpgradeGame::OnPostCardDone()
{
    //move the last 8 card to center.
    CSize szCard = m_pTheme->GetCardSize();
    int nAllWid = szCard.cx + kCardInterval * (54-1);
    SAutoRefPtr<IAnimatorGroup> pAnimatorGroup(new SAnimatorGroup(AGID_POSTCARD_DONE),FALSE);
    pAnimatorGroup->SetListener(this);
    for(int i=0;i<8;i++)
    {
        SSpriteCard * pSprite = (SSpriteCard*)m_pGameBoard->FindChildByID(ID_STACK_CARD_BASE+i);
        SASSERT(pSprite);
        AnchorPos toPos = m_pTheme->GetAnchorPos(AnchorName::kCardCenter);
        toPos.x = -nAllWid/2 + i*(kCardInterval * (54-1)/8) + szCard.cx/2;
        pAnimatorGroup->AddAnimator(Util::MoveSpriteTo(pSprite,toPos,SPEED_POSTCARD));
    }
    m_animatorGroupList.AddTail(pAnimatorGroup);
}

void UpgradeGame::OnTimerPostCard()
{
    if(m_nCardInStack <= 8)
    {
        m_pMainDlg->KillTimer(TIMERID_POSTCARD);
        OnPostCardDone();
    }else{
        DoPostCard();    //发牌动画
        
        // 添加音效播放
        PlayEffectSound(Sounds::Effects::kpost_card);
    }
}

void UpgradeGame::OnConnected()
{
    OnStageChanged(STAGE_CONTINUE);
}

void UpgradeGame::OnDisconnected()
{
    OnStageChanged(STAGE_CONNECTING);
}

BOOL UpgradeGame::OnMessage(DWORD dwType, std::shared_ptr<std::vector<BYTE> > data){
    const BYTE *pData = data?data->data():NULL;
    int nSize = data?data->size():0;
    switch(dwType)
    {
    case GMT_LOGIN_ACK:
        OnLoginAck(pData, nSize);
        break;
    case GMT_AVATAR_ACK: 
        OnAvatarAck(pData, nSize);
        break;
    case GMT_START:
        OnStartGame(pData, nSize);
        break;
    case GMT_END:
        OnGameEnd(pData, nSize);
        break;
    case GMT_UPGRADE_POSTCARD:
        OnPostCard(pData, nSize);
        break;
    case GMT_UPGRADE_REQSHOWMAIN:
        OnSetColor(pData, nSize);
        break;
    case GMT_UPGRADE_POSTBOTTOM:
        OnPostBottom(pData, nSize);
        break;
    case GMT_UPGRADE_REQPUTBOTTOM:
        OnSetBottom(pData, nSize);
        break;
    case GMT_UPGRADE_STARTPUTCARD:
        OnStartPutCard(pData, nSize);
        break;
    case GMT_UPGRADE_REQPUTCARD:
        OnPutCard(pData, nSize);
        break;
    case GMT_UPGRADE_TURNOVER:
        OnTurnOver(pData, nSize);
        break;
    case GMT_UPGRADE_ROUNDEND:
        OnRoundEnd(pData, nSize);
        break;
    case GMT_UPGRADE_THROWFAILED:
        OnThrowFailed(pData, nSize);
        break;
    case GMT_UPGRADE_AUTOCOLOR:
        OnAutoColor(pData, nSize);
        break;
    case GMT_UPGRADE_PUTCARDERR:
        OnPutCardError(pData, nSize);
        break;
    default:
        SLOGW() << "Unknown message type: " << dwType;
        break;
    }
    return TRUE;
}

void UpgradeGame::OnPostCard(const void *lpData, int nSize)
{
     SLOGI()<<"OnPostCard, cur stage="<<m_stage;
    if(m_stage != STAGE_WAIT_CARD)
    {
        return;
    }
    //等待发牌动画播放完毕再开始发牌
    SASSERT(nSize==sizeof(GAME_UPGRADE_POSTCARD));
    const GAME_UPGRADE_POSTCARD *pCards = (const GAME_UPGRADE_POSTCARD*)lpData;
	m_cardRecieve.Init(pCards->nCards,m_iSelfIndex,m_iBankerIndex==-1?0:m_iBankerIndex);
}

// ============================================================================
// 游戏消息处理函数实现
// ============================================================================

/**
 * @brief 处理游戏开始消息
 */
void UpgradeGame::OnStartGame(const void *lpData, int nSize)
{
    SLOGI() << "OnStartGame, data size="<< nSize;
    //clear table element of previous round.

    if (IWindow *pBtnStart = m_pGameBoard->FindChildByName(Sprites::btn_start))
    {
        m_pGameBoard->RemoveIChild(pBtnStart);
        pBtnStart->Release();
    }
    // hide ready flag
    for(int i = 0; i < PLAYER_COUNT; i++)
    {
        m_bReady[i] = FALSE;
        m_pGameBoard->FindChildByID(ID_AVATAR_BASE + i)->FindChildByName(L"flag_ready")->SetVisible(FALSE, TRUE);
    }
    {
        //clear bottom score multi
        SWindow *p = m_pGameBoard->FindChildByName(Sprites::win_bottom_score_multi);
        if(p){
            m_pGameBoard->RemoveChild(p);
            p->Release();
        }
    }
    {
        for(int i=0;i<PLAYER_COUNT;i++){
            m_cardInHand[i].Clear();
        }
    }
    {
        //clear score cards.
        m_putCard.Clear();
        m_putCard.ClearScoreCards();
    }
    //clear all cards in stack.
    for(int i=0;i<108;i++){
        SWindow * pCard = m_pGameBoard->FindChildByID(ID_STACK_CARD_BASE + i);
        if(pCard)
            pCard->Destroy();
    }
    {
        //clear sprite_vectory
        SWindow *p = m_pGameBoard->FindChildByName(Sprites::sprite_vectory);
        if(p){
            m_pGameBoard->RemoveChild(p);
            p->Release();
        }
    }
    if(m_curShowMain.index != -1){
        //clear show main box
        int nSeat = Index2Seat(m_curShowMain.index);
        GetSeatFlag(nSeat,FLAG_COLOR_1)->SetVisible(FALSE,FALSE);
        GetSeatFlag(nSeat,FLAG_COLOR_2)->SetVisible(FALSE,FALSE);
    }
    if(IWindow *pWnd = m_pGameBoard->FindChildByName(Sprites::win_show_main_box)){
        m_pGameBoard->RemoveIChild(pWnd);
        pWnd->Release();
    }
    m_curShowMain.index = -1;
    m_curShowMain.nCount = 0;
    m_curShowMain.nCard[0] = m_curShowMain.nCard[1] = -1;
    m_iSetBottomIndex = -1;
    m_nMainColor = -1;
    m_iCurPutCardIndex = -1;
    m_iTurnFirstIndex = -1;
    m_iBottomOwnerIndex = -1;
    m_nPutCardCount = 0;
    m_nScore = 0;
    m_nDiscount = 0;
    m_bAutoColor = FALSE;
    
    m_curShowMain.index = -1;
    m_curShowMain.nCount = 0;
    m_curShowMain.nCard[0] = m_curShowMain.nCard[1] = -1;

    m_iCurPutCardIndex = -1;
    m_nMainColor = -1;
    m_iSetBottomIndex = -1;

    m_bShowBottom = FALSE;
    m_bShowPrevRound = FALSE;

    OnStageChanged(STAGE_WAIT_CARD);
}

void UpgradeGame::OnGameEnd(const void *lpData, int nSize)
{
    if (m_stage == STAGE_POSTCARD)
    {
        m_pMainDlg->KillTimer(TIMERID_POSTCARD);
        m_pGameBoard->FindChildByName(Sprites::win_show_main_box)->SetVisible(FALSE, TRUE);
    }
    if (m_stage == STAGE_SETBOTTOM)
    {
        m_pMainDlg->KillTimer(TIMERID_SETBOTTOM);
        m_pGameBoard->FindChildByName(Sprites::btn_set_bottom)->SetVisible(FALSE, TRUE);
    }
    if (m_stage == STAGE_SHOW_BOTTOM)
    {
        m_pMainDlg->KillTimer(TIMERID_SHOW_BOTTOM);
    }
    if (m_stage == STAGE_REBELBOTTOM)
    {
        m_pMainDlg->KillTimer(TIMERID_SETBOTTOM);
        m_pGameBoard->FindChildByName(Sprites::win_show_main_box)->SetVisible(FALSE, TRUE);
        if (IWindow *pBtnSetBottom = m_pGameBoard->FindChildByName(Sprites::btn_set_bottom))
        {
            m_pGameBoard->RemoveIChild(pBtnSetBottom);
            pBtnSetBottom->Release();
        }
    }
    if (m_stage == STAGE_PUTCARD)
    {
        m_pMainDlg->KillTimer(TIMERID_AUTO_PUTCARD);
        if (IWindow *pBtnPutCard = m_pGameBoard->FindChildByName(Sprites::btn_put_card))
        {
            m_pGameBoard->RemoveIChild(pBtnPutCard);
            pBtnPutCard->Release();
        }
    }
    HideClock();
    StopWatingAnimation();
    PlayTip(_T("本局游戏结束"));
    OnStageChanged(STAGE_CONTINUE);
}

/**
 * @brief 处理亮主消息
 */
void UpgradeGame::OnSetColor(const void *lpData, int nSize)
{
    if(nSize < sizeof(GAME_UPGRADE_SHOWMAIN))
        return;
    if(STAGE_POSTCARD!=m_stage && STAGE_REBELBOTTOM!=m_stage){
        SLOGI()<<"OnSetColor, current stage is invalid: "<<m_stage;
        return;
    }
    GAME_UPGRADE_SHOWMAIN *pShowMain = (GAME_UPGRADE_SHOWMAIN*)lpData;
    int index = pShowMain->index;
    int nCount = pShowMain->nCount;
    int nCard = pShowMain->nCard[0];

    SLOGI() << "OnSetColor: index=" << index << ", count=" << nCount << ", card=" << nCard;
	int nSeat=Index2Seat(index);
 	if(m_curShowMain.index!=-1 && m_curShowMain.index != index)
	{//在此之前已经有人报牌
        int iReportSeat = Index2Seat(m_curShowMain.index);
        SList<SAutoRefPtr<SSpriteCard> > lstCards;
        m_putCard.GetCard(iReportSeat, lstCards);
        if(iReportSeat != SEAT_INDEX_ME){
            for (SPOSITION pos = lstCards.GetHeadPosition(); pos;)
            {
                SAutoRefPtr<SSpriteCard> pCard = lstCards.GetNext(pos);
                pCard->SetCard(-1);
            }
        }
        m_cardInHand[iReportSeat].ReceiveCards(lstCards);
        // hide report color flag
        SImageWnd *pFlag = GetSeatFlag(iReportSeat, FLAG_COLOR_1);
        pFlag->SetVisible(FALSE, TRUE);
        pFlag = GetSeatFlag(iReportSeat, FLAG_COLOR_2);
        pFlag->SetVisible(FALSE, TRUE);

		PlayProcessSound(Sounds::Processes::krebels_main, m_pUserInfo[nSeat]->nSex);
        PlayEffectSound(Sounds::Effects::krebels_main);
	}else
	{
		PlayProcessSound(Sounds::Processes::kshow_main, m_pUserInfo[nSeat]->nSex);
	}
    {
        int reportCard = nCard;
        if(nSeat != SEAT_INDEX_ME)
            reportCard = -1;
        SList<SAutoRefPtr<SSpriteCard> > lstCards;
        if (index != m_curShowMain.index)
            m_cardInHand[nSeat].ReportColor(reportCard, nCount == 2, lstCards);
        else
            m_cardInHand[nSeat].ReportColor(reportCard, FALSE, lstCards);
        if(nSeat != SEAT_INDEX_ME){
            for(SPOSITION pos = lstCards.GetHeadPosition();pos;){
                auto pCard = lstCards.GetNext(pos);
                pCard->SetCard(nCard);
            }
        }
        m_putCard.SetPutCard(nSeat, lstCards);
    }

    m_curShowMain = *pShowMain;
	int nMainColor =CUpgCard::ClientGetColor(nCard);

	//更新报牌按钮状态
	for(int nColor=COLOR_DIAMOND;nColor<=COLOR_BKING;nColor++)
	{
        SWindow * pBtn = m_pGameBoard->FindChildByID(Sprites::btn_diamond + nColor);
        pBtn->EnableWindow(m_cardInHand[SEAT_INDEX_BOTTOM].TestReport(nColor)!=0, TRUE);//在更换底牌后有可能可以再炒底
	}
    // show report color flag
    SImageWnd *pFlag = GetSeatFlag(nSeat, FLAG_COLOR_1);
    pFlag->SetIcon(nMainColor);
    pFlag->SetVisible(TRUE, TRUE);
    pFlag = GetSeatFlag(nSeat, FLAG_COLOR_2);
    pFlag->SetIcon(nMainColor);
    pFlag->SetVisible(m_curShowMain.nCount==2, TRUE);
}

/**
 * @brief 处理发底牌消息
 */
void UpgradeGame::OnPostBottom(const void *lpData, int nSize)
{
    if(nSize < sizeof(GAME_UPGRADE_POSTBOTTOM))
        return;
    GAME_UPGRADE_POSTBOTTOM *req = (GAME_UPGRADE_POSTBOTTOM*)lpData;
    m_iBottomOwnerIndex = req->iOwnerIndex;
    m_iSetBottomIndex = m_iBottomOwnerIndex;
    int seat = Index2Seat(m_iBottomOwnerIndex);
    SLOGI() << "OnPostBottom: player=" << m_iBottomOwnerIndex;

    // 确定庄家
    if(m_iBankerIndex == -1)
    {
        m_iBankerIndex = m_iBottomOwnerIndex;
        SLOGI() << "Banker player: " << m_iBankerIndex;
        SImageWnd *pFlag = GetSeatFlag(seat,FLAG_BANKER);//3 is banker flag id.
        pFlag->SetVisible(TRUE,TRUE);

        BOOL bOurBanker = (m_iBankerIndex % 2 == m_iSelfIndex % 2);
        m_pGameBoard->FindChildByNamePath(L"win_levela/img_banker")->SetVisible(bOurBanker, TRUE);
        m_pGameBoard->FindChildByNamePath(L"win_levelb/img_banker")->SetVisible(!bOurBanker, TRUE);
        
        // 添加音效播放
        PlayEffectSound(Sounds::Effects::kbanker);
    }
    //显示底牌
    for(int i=0;i<8;i++){
        SSpriteCard * pCard = m_pGameBoard->FindChildByID2<SSpriteCard>(ID_STACK_CARD_BASE+i);
        pCard->SetCard(req->nBottom[i]);
        pCard->Invalidate();
    }
    OnStageChanged(STAGE_SHOW_BOTTOM);

}

void UpgradeGame::OnSetBottomTimer(GAME_UPGRADE_REQPUTBOTTOM *req)
{
    int nPlayer = req->ownerIndex;

    SLOGI() << "OnSetBottomTimer: player=" << nPlayer;

    if(nPlayer == m_iSelfIndex)
    {
        // 自己垫底牌
        memcpy(m_nCardBottom, req->nBottom, 8 * sizeof(int));
        SList<SAutoRefPtr<SSpriteCard> > lstCards;
        m_cardInHand[SEAT_INDEX_BOTTOM].PutCard(lstCards);
        SASSERT(lstCards.GetCount() == 8);
        AnchorPos posAnchor=m_pTheme->GetAnchorPos(AnchorName::kCardCenter);
        CSize szCard = m_pTheme->GetCardSize();
        int nAllWid = szCard.cx + kCardInterval * (54-1);
        int i=0;
        for(SPOSITION pos = lstCards.GetHeadPosition();pos;)
        {
            SSpriteCard *pCard = lstCards.GetNext(pos);
            pCard->SetID(ID_STACK_CARD_BASE + i);
            pCard->SetLayer(i);
            pCard->SetCard(-1);
            pCard->ClearSelect();
            AnchorPos toPos = posAnchor;
            toPos.x = -nAllWid/2 + i*(kCardInterval * (54-1)/8) + szCard.cx/2;
            Util::MoveSpriteTo(pCard,toPos,SPEED_POSTCARD);
            i++;
        }
        SWindow *pBottomBtn = m_pGameBoard->FindChildByName(Sprites::btn_set_bottom);
        if(pBottomBtn){
            m_pGameBoard->RemoveChild(pBottomBtn);
            pBottomBtn->Release();
        }
        HideClock();
        OnStageChanged(STAGE_REBELBOTTOM);
    }
    else
    {
        // 其他玩家垫底牌
        memcpy(m_nCardBottom, req->nBottom, 8 * sizeof(int));
        SList<SAutoRefPtr<SSpriteCard> > lstCards;
        int seat = Index2Seat(req->ownerIndex);
        m_cardInHand[seat].PutCard(req->nBottom, 8, lstCards);
        SASSERT(lstCards.GetCount() == 8);
        AnchorPos posAnchor=m_pTheme->GetAnchorPos(AnchorName::kCardCenter);
        CSize szCard = m_pTheme->GetCardSize();
        int nAllWid = szCard.cx + kCardInterval * (54-1);
        int i=0;
        for(SPOSITION pos = lstCards.GetHeadPosition();pos;)
        {
            SSpriteCard *pCard = lstCards.GetNext(pos);
            pCard->SetID(ID_STACK_CARD_BASE + i);
            pCard->SetLayer(i);
            pCard->SetCard(-1);
            AnchorPos toPos = posAnchor;
            toPos.x = -nAllWid/2 + i*(kCardInterval * (54-1)/8) + szCard.cx/2;
            Util::MoveSpriteTo(pCard,toPos,SPEED_POSTCARD);
            i++;
        }
        m_cardInHand[seat].AdjustCardsPosition();
        OnStageChanged(STAGE_REBELBOTTOM);
    }
    
    // 添加音效播放
    PlayEffectSound(Sounds::Effects::kset_bottom);
}
/**
 * @brief 处理垫底牌消息
 */
void UpgradeGame::OnSetBottom(const void *lpData, int nSize)
{
    if(nSize < sizeof(GAME_UPGRADE_REQPUTBOTTOM))
        return;
    GAME_UPGRADE_REQPUTBOTTOM *req = (GAME_UPGRADE_REQPUTBOTTOM*)lpData;
    if(m_bAutoColor){
        m_reqPutBottomMsg = std::make_shared<GAME_UPGRADE_REQPUTBOTTOM>(*req);
        m_pMainDlg->SetTimer(TIMERID_SETBOTTOM,2000);
    }else{
        OnSetBottomTimer(req);
    }
}

/**
 * @brief 处理从底牌抽牌确定花色消息
 */
void UpgradeGame::OnAutoColor(const void *lpData, int nSize)
{
    if(nSize < sizeof(GMT_UPGRADE_AUTOCOLOR))
        return;
    GAME_UPGRADE_AUTOCOLOR *req = (GAME_UPGRADE_AUTOCOLOR*)lpData;
    SASSERT(req->iBottomIndex<8);
    SLOGI() << "OnSetColorFromBottom: card=" << req->nCard;

    SSpriteCard *pCard = m_pGameBoard->FindChildByID2<SSpriteCard>(ID_STACK_CARD_BASE + req->iBottomIndex);
    pCard->SetCard(req->nCard);
    Util::OffsetSprite(pCard,0,-20,SPEED_POSTCARD);

    m_nMainColor = CUpgCard::ClientGetColor(req->nCard);
    m_cardInHand[SEAT_INDEX_ME].SetMainColor(m_nMainColor, SPEED_POSTCARD);
    m_bAutoColor = TRUE;
    
    // 添加音效播放
    PlayEffectSound(Sounds::Effects::kauto_color);
}

void UpgradeGame::OnStartPutCard(const void *lpData, int nSize)
{
    if(nSize < sizeof(GAME_UPGRADE_STARTPUTCARD))
        return;

    GAME_UPGRADE_STARTPUTCARD *req = (GAME_UPGRADE_STARTPUTCARD*)lpData;
    m_iTurnFirstIndex = req->winnerIndex;
    m_iCurPutCardIndex = req->winnerIndex;
    m_putCard.Clear();
    OnStageChanged(STAGE_PUTCARD);    
}

/**
 * @brief 处理出牌消息
 */
void UpgradeGame::OnPutCard(const void *lpData, int nSize)
{
    if(nSize < 2 * sizeof(int))
        return;

    GAME_UPGRADE_REQPUTCARD *req = (GAME_UPGRADE_REQPUTCARD*)lpData;
    int nPlayer = req->nIndex;
    int nCardCount = req->nCount;
    if(nPlayer == m_iTurnFirstIndex){
        m_nPutCardCount = nCardCount;
    }
    SLOGI() << "OnPutCard: player=" << nPlayer << ", count=" << nCardCount;
    int nSeat = Index2Seat(nPlayer);
    if(nPlayer == m_iSelfIndex)
    {
        m_pTheme->GetWidget(Sprites::btn_put_card)->SetVisible(FALSE,TRUE);

        SList<SAutoRefPtr<SSpriteCard> > cards;
        BOOL bSuccess = m_cardInHand[SEAT_INDEX_ME].PutCard2(req->nCard, nCardCount,cards);
        if(bSuccess){
             for(SPOSITION pos = cards.GetHeadPosition();pos;){
                SSpriteCard *pCard = cards.GetNext(pos);
                pCard->ClearSelect();
            }
            m_putCard.SetPutCard(0,cards);
        }else{
            SLOGE()<<"OnPutCard, put card failed";
        }
    }else{
        SList<SAutoRefPtr<SSpriteCard> > cards;
        m_cardInHand[nSeat].PutCard(req->nCard, nCardCount, cards);
        m_putCard.SetPutCard(nSeat, cards);
    }
    
    // 计算牌型并播放相应音效
    int nCardType = CUpgCard::ClientGetCardType(req->nCard, req->nCount, 4, m_nMainColor, GetLevel(), m_dwProps[PI_RULE_2_MAIN]);
    if(nCardType && nPlayer == m_iTurnFirstIndex)
    {
        // 拖拉机
        PlayEffectSound(Sounds::Effects::ktractor);
        PlayProcessSound(Sounds::Processes::ktractor, m_pUserInfo[nSeat]->nSex);
    }else 
    {
        // 判断是否是杀牌
        BOOL bCut = FALSE;
        if(nPlayer != m_iTurnFirstIndex)
        {//不是第一个出牌
            int nCard = m_putCard.GetFirstCard(m_iTurnFirstIndex);
            int nColor1 = CUpgCard::ClientGetColor2(nCard, m_nMainColor, GetLevel(), m_dwProps[PI_RULE_2_MAIN]);
            if(nColor1 != COLOR_MAIN)//王牌之外的花色
            {
                bCut = TRUE;
                for(int k = 0; k < req->nCount; k++)
                {
                    if(CUpgCard::ClientGetColor2(req->nCard[k], m_nMainColor, GetLevel(),m_dwProps[PI_RULE_2_MAIN]) != 10)
                    {
                        bCut = FALSE;
                        break;
                    }
                }
            }
        }
        if(bCut)
        {
            PlayEffectSound(Sounds::Effects::kcut_card);
            PlayProcessSound(Sounds::Processes::kcut_card, m_pUserInfo[nSeat]->nSex);
        }
        if(!bCut)
        {
            BOOL bThrow = TRUE;
            if(req->nCount == 2 && req->nCard[0] == req->nCard[1])
                bThrow = FALSE;
            else if(nPlayer != m_iTurnFirstIndex)
                bThrow = FALSE;
            else if(req->nCount < 2)
                bThrow = FALSE;
            if(bThrow)
            {
                PlayEffectSound(Sounds::Effects::kthrow_card);
                PlayProcessSound(Sounds::Processes::kthrow_card, m_pUserInfo[nSeat]->nSex);
            }
            if(!bThrow)
            {
                if(req->nCount < 3) 
                    PlayEffectSound(Sounds::Effects::kput_card_1_2);
                else if(req->nCount < 6) 
                    PlayEffectSound(Sounds::Effects::kput_card_3_5);
                else 
                    PlayEffectSound(Sounds::Effects::kput_card_6);
            }
        }
    }
    
    // 计算下一个出牌玩家
    int nNextPlayer = (nPlayer + 1) % 4;
    if(nNextPlayer == m_iTurnFirstIndex){
        // 一轮出牌结束
#ifdef ENABLE_MOCK
        if(!m_cardInHand[SEAT_INDEX_BOTTOM].IsEmpty())
        {
            GAME_UPGRADE_TURNOVER req;
            if(m_nPutCardCount == 1){
                req.winnerIndex = m_iSelfIndex;
            }else{
                req.winnerIndex = rand()%4;
            }
            wsSendMsg(GMT_UPGRADE_TURNOVER,&req,sizeof(req));
        }else{
            int nextBanker =  rand()%4;
            BOOL bBankerWin = rand() % 2;
            if (bBankerWin)
            {
                nextBanker = (m_iBankerIndex + 2) % 4;
            }
            else
            {
                nextBanker = (m_iBankerIndex + 1) % 4;
            }
            GAME_UPGRADE_ROUNDEND req;
            req.nMaxIndex =  (m_iBankerIndex+1)%4;
            req.nNewBankerIndex = nextBanker;
            req.nMultiple = rand()%100;
            req.nScore = 100;
            if (bBankerWin)
            {
                if (m_iBankerIndex % 2 == 0)
                {
                    req.nLevelA = m_nLevelA + 1;
                    req.nLevelB = m_nLevelB;
                }
                else
                {
                    req.nLevelA = m_nLevelA;
                    req.nLevelB = m_nLevelB + 1;
                }
            }else{
                req.nLevelA = m_nLevelA;
                req.nLevelB = m_nLevelB;
            }
            for(int i=0;i<8;i++){
                req.nBottom[i] = rand()%54;
            }
            wsSendMsg(GMT_UPGRADE_ROUNDEND, &req, sizeof(req));
        }
#endif
    }
    else
    {
        m_iCurPutCardIndex = nNextPlayer;
        int nNextSeat = Index2Seat(nNextPlayer);
        MoveClock(nNextSeat,m_dwProps[PI_TIME_PUTCARD]);
        m_pTheme->GetWidget(Sprites::btn_put_card)->SetVisible(nNextSeat == SEAT_INDEX_ME,TRUE);
        if(nNextSeat == SEAT_INDEX_ME){
            int nFristCards[25];
            int nFristCount = m_putCard.GetCards(Index2Seat(m_iTurnFirstIndex), nFristCards);
            m_cardInHand[SEAT_INDEX_ME].AutoSelectCard(nFristCards, nFristCount);
            if(GetForegroundWindow() != m_pMainDlg->m_hWnd){
                FlashWindow(m_pMainDlg->m_hWnd, TRUE);
            }
            if(m_nPutCardCount == m_cardInHand[SEAT_INDEX_ME].GetCount()){
                m_cardInHand[SEAT_INDEX_ME].SelectAllCard();
                m_pMainDlg->SetTimer(TIMERID_AUTO_PUTCARD,2000,NULL);
            }
        }
    }
}

/**
 * @brief 处理一轮牌结束消息
 */
void UpgradeGame::OnTurnOver(const void *lpData, int nSize)
{
    if(nSize < sizeof(GAME_UPGRADE_TURNOVER))
        return;
    GAME_UPGRADE_TURNOVER *req = (GAME_UPGRADE_TURNOVER*)lpData;
    m_turnOverMsg = std::make_shared<GAME_UPGRADE_TURNOVER>(*req);
    m_pMainDlg->SetTimer(TIMERID_TURNOVER,2000,NULL);
    m_nPutCardCount = 0;
    HideClock();
}

void UpgradeGame::OnTurnOverTimer(GAME_UPGRADE_TURNOVER *pMsg){
    m_iTurnFirstIndex = pMsg->winnerIndex;
    m_iCurPutCardIndex = pMsg->winnerIndex;
    SLOGI() << "OnTurnOverTimer: next player=" << m_iCurPutCardIndex;
    int nWinner = Index2Seat(m_iTurnFirstIndex);
    int nScore = m_putCard.TakeinCard(nWinner);
    m_bShowPrevRound = FALSE;

    m_nScore += nScore;
    m_nPutCardCount = 0;
    UpdateScore();

    BOOL bGameOver = m_cardInHand[SEAT_INDEX_BOTTOM].IsEmpty();
    if (!bGameOver)
    {
        int nNextSeat = Index2Seat(m_iCurPutCardIndex);
        m_pTheme->GetWidget(Sprites::btn_put_card)->SetVisible(nNextSeat == SEAT_INDEX_ME, TRUE);
        MoveClock(nNextSeat, m_dwProps[PI_TIME_PUTCARD]);
        if(GetForegroundWindow() != m_pMainDlg->m_hWnd){
            FlashWindow(m_pMainDlg->m_hWnd,TRUE);
        }
    }
    
    // 添加音效播放
    if(nScore > 0) {
        PlayEffectSound(Sounds::Effects::kget_score);
    }
}

void UpgradeGame::OnRoundEndTimer(GAME_UPGRADE_ROUNDEND *pgsri){
    SLOGI() << "OnRoundEndTimer: score=" << pgsri->nScore << ", newBanker=" << pgsri->nNewBankerIndex;

    // 处理一轮牌结束消息
    GAME_UPGRADE_TURNOVER turnOver;
    turnOver.winnerIndex = pgsri->nMaxIndex;
    OnTurnOver(&turnOver, sizeof(turnOver));

    // 隐藏庄家标志
    int iBankerSeat = Index2Seat(m_iBankerIndex);
    SImageWnd *pFlag = GetSeatFlag(iBankerSeat, FLAG_BANKER); // 3 is banker flag id.
    pFlag->SetVisible(FALSE, TRUE);
    // 更新庄家
    m_iBankerIndex = pgsri->nNewBankerIndex;
    // 显示新庄家标志
    iBankerSeat = Index2Seat(m_iBankerIndex);
    pFlag = GetSeatFlag(iBankerSeat, FLAG_BANKER); // 3 is banker flag id.
    pFlag->SetVisible(TRUE, TRUE);
    BOOL bOurBanker = (m_iBankerIndex % 2 == m_iSelfIndex % 2);
    m_pGameBoard->FindChildByNamePath(L"win_levela/img_banker")->SetVisible(bOurBanker, TRUE);
    m_pGameBoard->FindChildByNamePath(L"win_levelb/img_banker")->SetVisible(!bOurBanker, TRUE);
    // 更新级牌
    m_nLevelA = pgsri->nLevelA;
    m_nLevelB = pgsri->nLevelB;
    UpdateLevel();
    // 更新分数
    m_nScore = pgsri->nScore;
    UpdateScore();

    // 显示底牌
    for (int i = 0; i < 8; i++)
    {
        SSpriteCard *pCard = m_pGameBoard->FindChildByID2<SSpriteCard>(ID_STACK_CARD_BASE + i);
        pCard->SetCard(pgsri->nBottom[i]);
        int nCard = pgsri->nBottom[i];
        int nValue = nCard % 13;
        if (nValue == 4 || nValue == 9 || nValue == 12)
        {
            pCard->SetSelected(TRUE); // highlight score cards
        }
        pCard->Invalidate();
    }
    // 显示倍数
    if (pgsri->nMultiple > 0)
    {
        int nMulti = pgsri->nMultiple % 99;
        SWindow *pWnd = (SWindow *)m_pTheme->GetWidget(Sprites::win_bottom_score_multi);
        m_pGameBoard->InsertIChild(pWnd);
        pWnd->SetVisible(TRUE, TRUE);
        pWnd->AddRef();
        int num0 = nMulti % 10;
        int num1 = nMulti / 10;
        SStackView *digit0 = pWnd->FindChildByName2<SStackView>(L"digit_0");
        SStackView *digit1 = pWnd->FindChildByName2<SStackView>(L"digit_1");
        digit1->SetVisible(num1 > 0);
        m_pGameBoard->UpdateLayout(); // 强制更新布局，否则digit1的可见性不会生效
        
        digit0->SelectPage(num0);
        digit1->SelectPage(num1);

        IAnimation *pAni = m_pTheme->GetAnimation(Animations::kbottom_multi);
        pWnd->SetAnimation(pAni);
        
        // 添加音效播放
        PlayEffectSound(Sounds::Effects::kbottom_multi);
    }
    // 播放胜利/失败动画
    if (m_iBankerIndex % 2 == m_iSelfIndex % 2)
    {
        m_roundResult = RESULT_WIN;
        SGifPlayer *pVectory = (SGifPlayer *)m_pTheme->GetWidget(Sprites::sprite_vectory);
        m_pGameBoard->InsertIChild(pVectory);
        pVectory->AddRef();
        pVectory->SetVisible(TRUE, TRUE);
        // restart vectory animation.
        EventGifPlayOver e(pVectory);
        OnGifVectoryOver(&e);
    }
    else
    {
        m_roundResult = RESULT_LOSE;
    }
    OnStageChanged(STAGE_CONTINUE);
}

/**
 * @brief 处理出牌错误消息
 */
void UpgradeGame::OnPutCardError(const void *lpData, int nSize)
{
    if(nSize < sizeof(GAME_UPGRADE_PUTCARDERR))
        return;
    GAME_UPGRADE_PUTCARDERR *pErrorCode = (GAME_UPGRADE_PUTCARDERR*)lpData;

    SLOGI() << "OnPutCardError: code=" << pErrorCode->errCode;
    static const LPCWSTR kErrString[]={
        L"@string/err_putcard_0",
        L"@string/err_putcard_1",
        L"@string/err_putcard_2",
        L"@string/err_putcard_3",
        L"@string/err_putcard_4",
    };
    if(pErrorCode->errCode >= PCE_SAMECOLOR && pErrorCode->errCode <= PCE_UNKNOWN){
        SStringW strErr = GETSTRING(kErrString[pErrorCode->errCode - 1]);
        PlayTip(S_CW2T(strErr));
    }
    m_pGameBoard->FindChildByName(Sprites::btn_put_card)->SetVisible(TRUE, TRUE);
    // 添加音效播放
    PlayEffectSound(Sounds::Effects::kput_card_error);
}

/**
 * @brief 处理游戏结束消息
 */
void UpgradeGame::OnRoundEnd(const void *lpData, int nSize)
{
    if(nSize < sizeof(GAME_UPGRADE_ROUNDEND))
        return;
    HideClock();
    GAME_UPGRADE_ROUNDEND *pgsri = (GAME_UPGRADE_ROUNDEND*)lpData;
    m_roundEndMsg = std::make_shared<GAME_UPGRADE_ROUNDEND>(*pgsri);
    m_pMainDlg->SetTimer(TIMERID_ROUNDEND,2000,NULL);
}

void UpgradeGame::OnGifVectoryOver(IEvtArgs *e)
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

void UpgradeGame::OnGifWaitingFrame(IEvtArgs *e)
{
    EventGifPlayFrame *e2=sobj_cast<EventGifPlayFrame>(e);
    int nFrame = e2->iFrame;
    if (nFrame == 1 || nFrame == 34)
    {
        PlayEffectSound(Sounds::Effects::kwaiting_walk);
    }
    else if (nFrame == 33 || nFrame == 0)
    {
        PlayEffectSound(Sounds::Effects::kwaiting_turn);
    }
}

//甩牌失败定时器到时，执行甩牌失败逻辑。
void UpgradeGame::OnThrowFailedTimer(GAME_UPGRADE_THROWFAILED * pMsg)
{
    int index = pMsg->nIndex;
    int seat = Index2Seat(index);
    SList<SAutoRefPtr<SSpriteCard> > cards;
    m_putCard.GetCard(seat, cards);
    if(seat != SEAT_INDEX_ME)
    {
        for(SPOSITION pos = cards.GetHeadPosition();pos;){
            SSpriteCard *pCard = cards.GetNext(pos);
            pCard->SetCard(-1);
        }
    }else{
        for(SPOSITION pos = cards.GetHeadPosition();pos;){
            SSpriteCard *pCard = cards.GetNext(pos);
            pCard->SetSelectable(TRUE);
        }
    }
    m_cardInHand[seat].ReceiveCards(cards);
    m_pMainDlg->SetTimer(TIMERID_THROW_FAILED2, SPEED_POSTCARD+100);
}

//甩牌失败定时器到时，执行甩牌失败逻辑。
void UpgradeGame::OnThrowFailedTimer2(GAME_UPGRADE_THROWFAILED * pMsg)
{
    GAME_UPGRADE_REQPUTCARD req;
    req.nIndex = pMsg->nIndex;
    req.nCount = pMsg->nFailedCount;
    memcpy(req.nCard, pMsg->nFailedCard, pMsg->nFailedCount * sizeof(int));
    OnPutCard(&req,sizeof(req));
}

/**
 * @brief 处理甩牌失败消息
 */
void UpgradeGame::OnThrowFailed(const void *lpData, int nSize)
{
    SLOGI() << "OnThrowFailed";
    if(nSize< sizeof(GAME_UPGRADE_THROWFAILED))
        return;
    GAME_UPGRADE_THROWFAILED *req = (GAME_UPGRADE_THROWFAILED*)lpData;
    m_throwFailMsg = std::shared_ptr<GAME_UPGRADE_THROWFAILED>(new GAME_UPGRADE_THROWFAILED);
    memcpy(m_throwFailMsg.get(), req, sizeof(GAME_UPGRADE_THROWFAILED));
    //执行出牌逻辑。
    int index = req->nIndex;
    int nCardCount = req->nPutCount;
    SASSERT(index == m_iTurnFirstIndex);
    int nSeat = Index2Seat(index);
    if(index == m_iSelfIndex)
    {
        SList<SAutoRefPtr<SSpriteCard> > cards;
        BOOL bRet = m_cardInHand[SEAT_INDEX_BOTTOM].PutCard2(req->nPutCard, nCardCount,cards);
        for(SPOSITION pos = cards.GetHeadPosition();pos;){
            SSpriteCard *pCard = cards.GetNext(pos);
            pCard->ClearSelect();
        }
        if(bRet){
            m_putCard.SetPutCard(0,cards);
        }
    }else{
        SList<SAutoRefPtr<SSpriteCard> > cards;
        m_cardInHand[nSeat].PutCard(req->nPutCard, nCardCount, cards);
        m_putCard.SetPutCard(nSeat, cards);
    }
    //start a timer to show failed cards.
    m_pMainDlg->SetTimer(TIMERID_THROW_FAILED, 2000);
    SStringW strMsg = GETSTRING(L"@string/msg_discount");
    strMsg.Format(strMsg, req->nDiscount);
    PlayTip(S_CW2T(strMsg));
    if(index %2 == m_iBankerIndex %2){
        m_nDiscount -= req->nDiscount;
    }else{
        m_nDiscount += req->nDiscount;
    }
    UpdateScore();
}

static void ShowDigit(SWindow *pWnd, int nValue)
{
    BOOL bNegative = nValue < 0;
    nValue = abs(nValue);
    if(nValue>=10000){
        SStackView * pStack = pWnd->FindChildByName2<SStackView>(L"digit4");
        pStack->SelectPage(11);//w
        nValue/=10000;
        pStack = pWnd->FindChildByName2<SStackView>(L"digit3");
        pStack->SelectPage(nValue%10);
        nValue/=10;
        pStack = pWnd->FindChildByName2<SStackView>(L"digit2");
        pStack->SelectPage(nValue%10);
        nValue/=10;
        pStack = pWnd->FindChildByName2<SStackView>(L"digit1");
        if(bNegative){
            pStack->SelectPage(10);//-
        }else{
            pStack->SelectPage(nValue%10);
        }
    }else{
        SStackView * pStack = pWnd->FindChildByName2<SStackView>(L"digit4");
        pStack->SelectPage(nValue%10);
        nValue/=10;
        pStack = pWnd->FindChildByName2<SStackView>(L"digit3");
        pStack->SelectPage(nValue%10);
        nValue/=10;
        pStack = pWnd->FindChildByName2<SStackView>(L"digit2");
        pStack->SelectPage(nValue%10);
        nValue/=10;
        pStack = pWnd->FindChildByName2<SStackView>(L"digit1");
        if(bNegative){
            pStack->SelectPage(10);//-
        }else{
            pStack->SelectPage(nValue%10);
        }
    }
    SLOGI() << "ShowDigit: " << nValue;
}

void UpgradeGame::UpdateScore()
{
    ShowDigit(m_pGameBoard->FindChildByName2<SWindow>(L"win_score"), m_nScore);
    ShowDigit(m_pGameBoard->FindChildByName2<SWindow>(L"win_discount"), m_nDiscount);
}

void UpgradeGame::UpdateLevel()
{
    SStackView *pLevel0 = sobj_cast<SStackView>(m_pGameBoard->FindChildByNamePath(L"win_levela/level"));
    SStackView *pLevel1 = sobj_cast<SStackView>(m_pGameBoard->FindChildByNamePath(L"win_levelb/level"));
    if(m_iSelfIndex%2 == 0){
        pLevel0->SelectPage(m_nLevelA);
        pLevel1->SelectPage(m_nLevelB);
    }else{
        pLevel0->SelectPage(m_nLevelB);
        pLevel1->SelectPage(m_nLevelA);
    }
}

void UpgradeGame::UpdateClock(int nSecond)
{
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
        OnTimeout();
    }
    if(nSecond>0){
        m_pMainDlg->SetTimer(TIMERID_CLOCK, 1000);
    }
    if(nSecond<=10)
    {
        PlayEffectSound(Sounds::Effects::kcount_down);
    }
}

void UpgradeGame::PlayTip(const SStringT &strTip)
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
    pAnim->addListener(&m_putCard);//destroy the tip when animation end.
}

void UpgradeGame::OnLoginAck(const void *pData, int nSize){
    GAME_LOGIN_ACK *pAck = (GAME_LOGIN_ACK *)pData;
    if(pAck->errCode == ERR_SUCCESS){
        memcpy(m_dwProps,pAck->dwProps,sizeof(m_dwProps));
        if(m_dwProps[PI_RULE_LEVEL_5_10_K])
        {
            m_nLevelA = m_nLevelB = 4;
        }else
        {
            m_nLevelA = m_nLevelB = 1;
        }
        UpdateLevel();
    }
}

void UpgradeGame::OnAvatarAck(const void *pData, int nSize){
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

void UpgradeGame::wsSendMsg(DWORD dwType, LPVOID lpData, DWORD dwSize)
{
    m_webSocketClient->SendMsg(dwType, lpData, dwSize);
    #ifdef ENABLE_MOCK
    //mock msg to self
    OnMessage(dwType, std::shared_ptr<std::vector<BYTE> >(new std::vector<BYTE>((LPBYTE)lpData, (LPBYTE)lpData + dwSize)));
    #endif
}

SImageWnd* UpgradeGame::GetSeatFlag(int nSeat,int flagId){
    int id = ID_AVATAR_FLAGS_BASE + nSeat;
    SWindow *pColorFlag = m_pGameBoard->FindChildByID(id);
    SImageWnd *pImg = pColorFlag->FindChildByID2<SImageWnd>(flagId);
    return pImg;
}

/**
 * @brief 播放特效音效
 * @param pszSound 音效名称
 * 
 * 该函数根据音效名称从主题配置中获取对应的音效文件路径，
 * 并使用PlaySound函数异步播放音效。
 * 如果音效功能被禁用(m_bSound为FALSE)则直接返回。
 */
void UpgradeGame::PlayEffectSound(LPCWSTR pszSound)
{
    SStringW strFile = m_pTheme->GetEffectSoundFile(pszSound);
    if(!strFile.IsEmpty()){
        TCHAR szResPrefix[MAX_PATH]={0};
        SApplication::getSingletonPtr()->GetFilePrefix(szResPrefix, MAX_PATH);
        SStringT strFull = SStringT(szResPrefix) + _T("/") + S_CW2T(strFile);
        m_pMainDlg->PlayWave(strFull);
    }   
}

/**
 * @brief 播放过程语音
 * @param pszSound 语音名称
 * @param nSex 性别标识，SEX_MALE表示男性，其他值表示女性
 * 
 * 该函数根据语音名称和性别从主题配置中获取对应的语音文件路径，
 * 并使用PlaySound函数异步播放语音。
 * 如果音效功能被禁用(m_bSound为FALSE)则直接返回。
 */
void UpgradeGame::PlayProcessSound(LPCWSTR pszSound,int nSex)
{
    SStringW strFile = m_pTheme->GetProcSoundFile(nSex == SEX_MALE, pszSound);
    if(!strFile.IsEmpty()){
        TCHAR szResPrefix[MAX_PATH]={0};
        SApplication::getSingletonPtr()->GetFilePrefix(szResPrefix, MAX_PATH);
        SStringT strFull = SStringT(szResPrefix) + _T("/") + S_CW2T(strFile);
        m_pMainDlg->PlayWave(strFull);
    } 
}

void UpgradeGame::PlayWattingAnimation(LPCTSTR pszTip){
    SGifPlayer *pWaiting = (SGifPlayer *)m_pTheme->GetWidget(Sprites::sprite_waiting);
    SASSERT(pWaiting);
    m_pGameBoard->InsertIChild(pWaiting);
    pWaiting->AddRef();
    SSkinAni *pAniSkin = (SSkinAni *)GETSKIN(Skins::kwaiting[rand() % PLAYER_COUNT], 100);
    SASSERT(pAniSkin);
    pWaiting->SetAniSkin(pAniSkin);
    pWaiting->Start();
    PlayTip(pszTip);
}

void UpgradeGame::StopWatingAnimation(){
    SWindow *pWaiting = m_pGameBoard->FindChildByName(Sprites::sprite_waiting);
    if (pWaiting)
    {
        m_pGameBoard->RemoveIChild(pWaiting);
        pWaiting->Release();
    }
}

void UpgradeGame::OnTimeout()
{
    if(m_stage == STAGE_POSTCARD){
        OnBtnPass();
    }
    if(m_stage == STAGE_SETBOTTOM){
        if(m_iBottomOwnerIndex == m_iSelfIndex){
            //自己垫底牌, 随机选择8张牌
            m_cardInHand[SEAT_INDEX_ME].SelectCardRandom(8, TRUE, -1 ,m_dwProps[PI_RULE_2_MAIN]);
            OnBtnSetBottom();
        }
    }
    if(m_stage == STAGE_REBELBOTTOM){
        if(m_curShowMain.index != m_iSelfIndex){
            OnBtnPass();
        }
    }
    if(m_stage == STAGE_PUTCARD){
        //select card and send put card msg.
        if(m_iSelfIndex == m_iTurnFirstIndex){
            //first player, select 1 card.
            if(m_cardInHand[SEAT_INDEX_BOTTOM].IsEmpty())//no card in hand
                return;
            m_cardInHand[SEAT_INDEX_BOTTOM].SelectCardRandom(1,TRUE,-1,m_dwProps[PI_RULE_2_MAIN]);
            OnBtnPutCard();
        }else if(m_iSelfIndex == m_iCurPutCardIndex){
            //根据首家出的牌型，选择合适的牌型。
            int cards[25];
            int nCard = m_putCard.GetCards(Index2Seat(m_iTurnFirstIndex), cards);
            SASSERT(nCard>0);
            int nColor = CUpgCard::ClientGetColor2(cards[0], m_nMainColor, GetLevel(), m_dwProps[PI_RULE_2_MAIN]);
            int nCardType = CUpgCard::ClientGetCardType(cards, nCard, 4, m_nMainColor, GetLevel(), m_dwProps[PI_RULE_2_MAIN]);
            m_cardInHand[SEAT_INDEX_ME].ClearSelect();
            int nSel = m_cardInHand[SEAT_INDEX_ME].AutoSelectCard(cards, nCard);
            if(nSel < nCard){
                //选择的牌不够，选择其它牌
                m_cardInHand[SEAT_INDEX_ME].SelectCardRandom(nCard-nSel, FALSE,nColor,m_dwProps[PI_RULE_2_MAIN]);
            }
            OnBtnPutCard();
        }
    }
}

void UpgradeGame::MoveClock(int nSeat, int value)
{
    SWindow *pClock = m_pGameBoard->FindChildByID(ID_ALARM_CLOCK);
    pClock->SetVisible(TRUE, TRUE);
    pClock->ClearAnimation();
    UpdateClock(value);
    AnchorPos pos = m_pTheme->GetAnchorPos(AnchorName::kClockBottom + nSeat);
    Util::MoveSpriteTo(pClock, pos, SPEED_POSTCARD);
}

void UpgradeGame::HideClock()
{
    // 隐藏闹钟
    IWindow *pClock = m_pGameBoard->FindChildByID(ID_ALARM_CLOCK);
    m_pMainDlg->KillTimer(TIMERID_CLOCK);
    pClock->ClearAnimation();
    pClock->SetVisible(FALSE, TRUE); 
}

void UpgradeGame::OnButtonHover(IEvtArgs *e)
{
    SImageButton *pBtn = sobj_cast<SImageButton>(e->Sender());
    if(!pBtn)
        return;
    if(pBtn->IsDisabled(TRUE))
        return;
    PlayEffectSound(Sounds::Effects::kbutton_focus);
}

int UpgradeGame::GetScale() const
{
    return m_pGameBoard->GetScale();
}

void UpgradeGame::OnAnimatorGroupEnd(IAnimatorGroup * pGroup, int nID)
{
    for (SPOSITION it = m_animatorGroupList.GetHeadPosition(); it;)
    {
        SPOSITION pos = it;
        if(m_animatorGroupList.GetNext(it) == pGroup)
        {
            m_animatorGroupList.RemoveAt(pos);
            break;
        }
    }
    switch(nID){
        case AGID_POSTCARD_DONE:
            onBottomCardsExtended();
            break;
        default:
            break;
    }
}

// 底牌扩展完毕
void UpgradeGame::onBottomCardsExtended()
{
    MoveClock(SEAT_INDEX_ME, m_dwProps[PI_TIME_REPORT]);
    m_pGameBoard->FindChildByID(Sprites::btn_pass)->EnableWindow(TRUE, TRUE);
    if(CUpgCard::ClientGetColor(m_curShowMain.nCard[0]) == COLOR_BKING)
    {
        OnBtnPass();
    }else if(m_curShowMain.index == -1){
        if(m_iBankerIndex == -1){
            PlayTip(SStringT().Format(_T("请在%d秒内亮主,否则系统将重新发牌。"), m_dwProps[PI_TIME_REPORT]));
        }else{
            PlayTip(SStringT().Format(_T("请在%d秒内亮主,否则系统将从底牌中随机抽一张牌为主花色。"), m_dwProps[PI_TIME_REPORT]));
        }
    }
}