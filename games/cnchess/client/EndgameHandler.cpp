#include "stdafx.h"
#include "EndgameHandler.h"
#include "MainDlg.h"
#include "myprofile.h"
#include "CnchessSkin.h"
#include <cnchessProtocol.h>
#include <helper/slog.h>
#include <helper/SAdapterBase.h>
#include <control/SCmnCtrl.h>
#include <layout/SAnchorLayout.h>
#include <map>
#include <memory>
#include "SRatingBar.h"
#include "events.h"

#define kLogTag "EndgameHandler"

// 残局桌适配器(大厅式游戏桌): 每个残局按配置 N 张桌, 项数 = 残局数*N
class CEndgameAdapter : public SAdapterBase
{
    std::vector<std::shared_ptr<ENDGAME_INFO> > m_endgames;
    std::map<int, std::shared_ptr<GAME_TABLE_INFO> > m_tables;   // key = 残局桌号(>=ENDGAME_TABLE_BASE)
    int m_nTablesPerEndgame = 2;                // 每残局桌数(由服务端配置下发)
    EndgameHandler *m_pHandler;
public:
    CEndgameAdapter(EndgameHandler *pHandler)
        : m_pHandler(pHandler)
    {
    }
    ~CEndgameAdapter() {}

    // 残局桌号: 残局序号*tablesPerEndgame + 槽位
    int GetTableId(int nEndgameIndex, int nSlot) const
    {
        return ENDGAME_TABLE_BASE + nEndgameIndex * m_nTablesPerEndgame + nSlot;
    }
    // 桌号 -> 项位置(残局序号*tablesPerEndgame + 槽位)
    int GetPositionForTable(int nTableId) const
    {
        int nOff = nTableId - ENDGAME_TABLE_BASE;
        if (nOff < 0)
            return -1;
        return (nOff / m_nTablesPerEndgame) * m_nTablesPerEndgame + (nOff % m_nTablesPerEndgame);
    }

    void SetEndgames(LPBYTE pInfo, int nCount, int nInfoSize, int nTablesPerEndgame)
    {
        m_endgames.clear();
        if (nTablesPerEndgame > 0)
            m_nTablesPerEndgame = nTablesPerEndgame;
        for (int i = 0; i < nCount; i++)
        {
            std::shared_ptr<ENDGAME_INFO> sp(new ENDGAME_INFO);
            memcpy(sp.get(), pInfo + (size_t)i * nInfoSize, sizeof(ENDGAME_INFO));
            m_endgames.push_back(sp);
        }
        notifyDataSetChanged();
    }

    // 解析房间信息, 仅保留残局桌(>=ENDGAME_TABLE_BASE)
    void SetRoomInfo(LPBYTE pData, int dwLen)
    {
        BYTE *p = pData;
        p += sizeof(int);                       // 跳过 m_nMaxTable
        int nTableCount = *(int *)p;
        p += sizeof(int);
        m_tables.clear();
        for (int i = 0; i < nTableCount; i++)
        {
            GAME_TABLE_INFO *pTableInfo = (GAME_TABLE_INFO *)p;
            int length = sizeof(GAME_TABLE_INFO) + (pTableInfo->nPlayers - 1) * sizeof(SEAT_INFO);
            if (pTableInfo->nTableId >= ENDGAME_TABLE_BASE)
                m_tables[pTableInfo->nTableId] = std::shared_ptr<GAME_TABLE_INFO>(
                    _CloneTable(pTableInfo, length));
            p += length;
        }
        notifyDataSetChanged();
    }

    // 单桌信息更新(仅残局桌)
    void SetTableInfo(GAME_TABLE_INFO *pTableInfo, int dwLen)
    {
        if (pTableInfo->nTableId < ENDGAME_TABLE_BASE)
            return;
        m_tables[pTableInfo->nTableId] = std::shared_ptr<GAME_TABLE_INFO>(_CloneTable(pTableInfo, dwLen));
        int nPos = GetPositionForTable(pTableInfo->nTableId);
        if (nPos >= 0)
            notifyItemDataChanged(nPos);
    }

    const ENDGAME_INFO *GetEndgame(int nIndex) const
    {
        if (nIndex < 0 || nIndex >= (int)m_endgames.size())
            return NULL;
        return m_endgames[nIndex].get();
    }

    STDMETHOD_(int, getCount)() override
    {
        return (int)m_endgames.size() * m_nTablesPerEndgame;
    }

    STDMETHOD_(void, getView)(int position, SItemPanel *pItem, SXmlNode xmlTemplate) override
    {
        if (pItem->GetChildrenCount() == 0)
            pItem->InitFromXml(&xmlTemplate);

        int nEndgameIndex = position / m_nTablesPerEndgame;
        int nSlot = position % m_nTablesPerEndgame;
        int nTableId = GetTableId(nEndgameIndex, nSlot);
        const ENDGAME_INFO *pEndgame = GetEndgame(nEndgameIndex);

        // 重置难度选择为收起状态
        SWindow *pLevels = pItem->FindChildByName(L"wnd_robot_levels");
        if (pLevels)
            pLevels->SetVisible(FALSE, TRUE);

        auto slot = Subscriber(&CEndgameAdapter::OnButtonClick, this);
        auto selSlot = Subscriber(&CEndgameAdapter::OnTitleClick, this);
        pItem->SubscribeEvent(EventItemPanelClick::EventID, &selSlot);

        // 订阅邀请机器人及其难度按钮、座位按钮事件
        const wchar_t *kRobotBtns[] = { L"btn_invite_robot", L"btn_lvl_beginner", L"btn_lvl_medium", L"btn_lvl_advanced" };
        for (int i = 0; i < 4; i++)
        {
            SWindow *pBtn = pItem->FindChildByName(kRobotBtns[i]);
            if (pBtn)
                pBtn->SubscribeEvent(EventCmd::EventID, &slot);
        }

        SWindow* pTitle = pItem->FindChildByName(L"txt_eg_table_title");
        // 标题 + 桌序号
        if (pTitle)
        {
            SStringT strTitle = S_CA2T(pEndgame->szTitle, CP_UTF8);
            pTitle->SetWindowText(strTitle);
        }

        // 难度系数(与 ENDGAME_INFO::nLevel 对应: 1初级/2中级/3高级)
        SWindow *pIndex = pItem->FindChildByName(L"txt_eg_table_index");
        if (pIndex)
        {
            SStringT strIndex;
            strIndex.Format(_T("第%d桌"), nSlot + 1);
            pIndex->SetWindowText(strIndex);
        }

        // 星级难度(RatingBar): value = 难度系数 1/2/3
        SRatingBar *pRating = sobj_cast<SRatingBar>(pItem->FindChildByName(L"rtg_eg_table_difficulty"));
        if (pRating)
            pRating->SetValue(pEndgame ? (float)pEndgame->nLevel : 0.0f);

        // 初始化座位
        for (int i = 0; i < PLAYER_COUNT; i++)
        {
            SStringW strSeat = SStringW().Format(L"seat_%d", i);
            SWindow *pSeatWnd = pItem->FindChildByName(strSeat);
            SImageWnd *pSeatImg = sobj_cast<SImageWnd>(pSeatWnd);
            pSeatImg->SetIcon(i == 0 ? 0 : 3);  // 红方/黑方空位
            pSeatWnd->SubscribeEvent(EventCmd::EventID, &slot);
            SStringW strSeatText = SStringW().Format(L"txt_seat_%d", i);
            pItem->FindChildByName(strSeatText)->SetWindowText(i == 0 ? SStringT(_T("红方")) : SStringT(_T("黑方")));
        }

        auto it = m_tables.find(nTableId);
        if (it != m_tables.end())
        {
            GAME_TABLE_INFO *table = it->second.get();
            SEAT_INFO *seatInfo = table->seatInfo;
            for (int i = 0; i < table->nPlayers; i++)
            {
                int nSeat = seatInfo->nIndex;
                SStringW strSeat = SStringW().Format(L"seat_%d", nSeat);
                SStringT strName = S_CA2T(seatInfo->stUserInfo.szName, CP_UTF8);
                SWindow *pSeatWnd = pItem->FindChildByName(strSeat);
                SImageWnd *pSeatImg = sobj_cast<SImageWnd>(pSeatWnd);
                int nIconIndex = nSeat == 0 ? 1 : 4;  // 红方/黑方占据
                if (seatInfo->bReady)
                    nIconIndex = nSeat == 0 ? 2 : 5;  // 红方/黑方准备
                pSeatImg->SetIcon(nIconIndex);
                SStringW strSeatText = SStringW().Format(L"txt_seat_%d", nSeat);
                pItem->FindChildByName(strSeatText)->SetWindowText(strName);
                seatInfo++;
            }

            pItem->FindChildByName(L"img_playing")->SetVisible(table->tableState == TABLE_STATE_PLAYING, TRUE);

            if (table->tableState == TABLE_STATE_PLAYING)
            {
                pItem->FindChildByName(L"txt_status")->SetWindowText(_T("游戏进行中"));
            }
            else
            {
                bool allReady = true;
                SEAT_INFO *checkSeat = table->seatInfo;
                for (int i = 0; i < table->nPlayers; i++)
                {
                    if (!checkSeat->bReady) { allReady = false; break; }
                    checkSeat++;
                }
                pItem->FindChildByName(L"txt_status")->SetWindowText(allReady ? _T("等待开始") : _T("等待准备"));
            }
        }
        else
        {
            pItem->FindChildByName(L"img_playing")->SetVisible(FALSE, TRUE);
            pItem->FindChildByName(L"txt_status")->SetWindowText(_T("等待入座"));
        }
    }

    BOOL OnTitleClick(IEvtArgs *e)
    {
        SWindow *pBtn = sobj_cast<SWindow>(e->Sender());
        SItemPanel *pItem = sobj_cast<SItemPanel>(pBtn->GetRoot());
        int nPos = pItem->GetItemIndex();
        if (m_pHandler)
            m_pHandler->SelectEndgame(nPos / m_nTablesPerEndgame);
        return TRUE;
    }

    BOOL OnButtonClick(IEvtArgs *e)
    {
        SWindow *pBtn = sobj_cast<SWindow>(e->Sender());
        SItemPanel *pItem = sobj_cast<SItemPanel>(pBtn->GetRoot());
        int nPos = pItem->GetItemIndex();
        int nEndgameIndex = nPos / m_nTablesPerEndgame;
        int nTableId = GetTableId(nEndgameIndex, nPos % m_nTablesPerEndgame);
        SStringW strName = pBtn->GetName();

        // 邀请机器人: 展开/收起难度选择
        if (strName == L"btn_invite_robot")
        {
            SWindow *pLevels = pItem->FindChildByName(L"wnd_robot_levels");
            if (pLevels)
                pLevels->SetVisible(!pLevels->IsVisible(), TRUE);
            return TRUE;
        }

        // 选择难度: 邀请机器人坐到本桌对座
        if (strName == L"btn_lvl_beginner" || strName == L"btn_lvl_medium" || strName == L"btn_lvl_advanced")
        {
            int nLevel = (strName == L"btn_lvl_beginner") ? ROBOT_LEVEL_BEGINNER
                       : (strName == L"btn_lvl_medium") ? ROBOT_LEVEL_MEDIUM
                       : ROBOT_LEVEL_ADVANCED;
            MyProfile *pMy = MyProfile::getSingletonPtr();
            int nMySeat = pMy->GetSeatIndex();
            if (pMy->GetTableId() != nTableId || nMySeat < 0 || nMySeat >= PLAYER_COUNT)
            {
                m_pHandler->PlayTip(_T("请先在此桌入座后再邀请机器人"));
                return TRUE;
            }
            int nTargetSeat = (nMySeat + 1) % PLAYER_COUNT;
            m_pHandler->ReqRobotInvite(nTableId, nTargetSeat, nLevel);
            SWindow *pLevels = pItem->FindChildByName(L"wnd_robot_levels");
            if (pLevels)
                pLevels->SetVisible(FALSE, TRUE);
            return TRUE;
        }

        int id = pBtn->GetID() - 10;
        m_pHandler->ReqSeatDown(nTableId, id);
        return TRUE;
    }

private:
    // 深拷贝单桌信息
    static GAME_TABLE_INFO *_CloneTable(GAME_TABLE_INFO *pSrc, int dwLen)
    {
        GAME_TABLE_INFO *pDst = (GAME_TABLE_INFO *)malloc(dwLen);
        if (pDst)
            memcpy(pDst, pSrc, dwLen);
        return pDst;
    }
};

//---------------------------------------------------------------------------
EndgameHandler::EndgameHandler(CMainDlg *pMainDlg, SGameTheme *pTheme)
    : m_pMainDlg(pMainDlg)
    , m_pTheme(pTheme)
    , m_pRoot(NULL)
    , m_ws(NULL)
    , m_pAdapter(NULL)
    , m_pPreviewBoard(NULL)
    , m_pChessBoard(NULL)
    , m_cellWidth(0)
    , m_cellHeight(0)
    , m_nCurEndgameIndex(-1)
    , m_nTablesPerEndgame(2)
    , m_bAutoStartSent(false)
{
    m_ptBoardOrigin = CPoint(0, 0);
}

EndgameHandler::~EndgameHandler()
{
    if (m_pAdapter)
    {
        m_pAdapter->Release();
        m_pAdapter = NULL;
    }
}

void EndgameHandler::SetWebSocket(WebSocketClient* pWs) {
    m_ws = pWs;
}

void EndgameHandler::OnThemdReady() {
    if (m_pPreviewBoard)
    {
        SAnchorLayout* pAnchorLayout = sobj_cast<SAnchorLayout>(m_pPreviewBoard->GetLayout());
        if (pAnchorLayout)
            pAnchorLayout->SetPosition2PointCallback(ChessAnchor2Pos, this);
        m_pChessBoard = sobj_cast<SImageWnd>(m_pTheme->GetWidget(Sprites::board_preview));
        if (m_pChessBoard)
        {
            m_pPreviewBoard->InsertChild(m_pChessBoard);
            m_pChessBoard->AddRef();
        }
    }
}
void EndgameHandler::Init(SWindow *pRoot)
{
    m_pRoot = pRoot;

    STileView *pTileView = m_pRoot->FindChildByName2<STileView>(L"tileview_endgame");
    if (pTileView)
    {
        m_pAdapter = new CEndgameAdapter(this);
        pTileView->SetAdapter(m_pAdapter);
    }

    m_pPreviewBoard = m_pRoot->FindChildByName2<CChessBoard>(L"chessboard_preview");
}

BOOL EndgameHandler::OnMessage(DWORD dwType, std::shared_ptr<std::vector<BYTE> > data)
{
    const BYTE *pData = data ? data->data() : NULL;
    int nSize = data ? data->size() : 0;
    BOOL ret = FALSE;
    switch (dwType)
    {
    case GMT_ENDGAME_LIST_ACK:
        ret = OnEndgameListAck(pData, nSize);
        break;
    case GMT_ROOM_INFO:
        ret = OnRoomInfo(pData, nSize);
        break;
    case GMT_TABLE_INFO:
        ret = OnTableInfo(pData, nSize);
        break;
    case GMT_SEATDOWN_ACK:
        ret = OnSeatDownAck(pData, nSize);
        break;
    case GMT_ROBOT_INVITE_ACK:
        ret = OnRobotInviteAck(pData, nSize);
        break;
    }
    return ret;
}

void EndgameHandler::OnConnected()
{
    RequestList();
}

void EndgameHandler::OnDisconnected()
{
}

void EndgameHandler::RequestList()
{
    if (m_ws)
        m_ws->SendMsg(GMT_ENDGAME_LIST_REQ, NULL, 0);
    SLOGI() << "EndgameHandler: request endgame list";
}

BOOL EndgameHandler::OnEndgameListAck(const void *lpData, int nSize)
{
    if (nSize < (int)(sizeof(GAME_ENDGAME_LIST) - 1) || !m_pAdapter)
        return FALSE;
    PGAME_ENDGAME_LIST pList = (PGAME_ENDGAME_LIST)lpData;
    SLOGI() << "EndgameHandler: endgame list count=" << pList->nCount
            << " tablesPerEndgame=" << pList->nTablesPerEndgame;
    if (pList->nTablesPerEndgame > 0)
        m_nTablesPerEndgame = pList->nTablesPerEndgame;
    if (pList->nCount > 0)
        m_pAdapter->SetEndgames((LPBYTE)&pList->vInfo[0], pList->nCount, sizeof(ENDGAME_INFO), m_nTablesPerEndgame);
    else
        m_pAdapter->SetEndgames(NULL, 0, 0, m_nTablesPerEndgame);

    // 默认预览第一个残局
    if (m_nCurEndgameIndex < 0 && pList->nCount > 0)
        SelectEndgame(0);
    return TRUE;
}

BOOL EndgameHandler::OnRoomInfo(const void *lpData, int nSize)
{
    if (!m_pAdapter)
        return FALSE;
    m_pAdapter->SetRoomInfo((LPBYTE)lpData, nSize);
    return TRUE;
}

BOOL EndgameHandler::OnTableInfo(const void *lpData, int nSize)
{
    if (!m_pAdapter || nSize < (int)sizeof(GAME_TABLE_INFO))
        return FALSE;
    GAME_TABLE_INFO *pInfo = (GAME_TABLE_INFO *)lpData;
    m_pAdapter->SetTableInfo(pInfo, nSize);
    TryAutoStart(pInfo);
    return TRUE;
}

void EndgameHandler::TryAutoStart(GAME_TABLE_INFO *pInfo)
{
    if (!pInfo || m_bAutoStartSent)
        return;
    if (pInfo->nTableId < ENDGAME_TABLE_BASE)
        return;
    MyProfile *pMy = MyProfile::getSingletonPtr();
    // 仅处理玩家本人所在桌, 且玩家已入座
    if (pMy->GetTableId() != pInfo->nTableId)
        return;
    if (pMy->GetSeatIndex() < 0 || pMy->GetSeatIndex() >= PLAYER_COUNT)
        return;
    // 真人+机器人坐满后才自动开局
    if (pInfo->nPlayers < PLAYER_COUNT)
        return;
    m_bAutoStartSent = true;
    if (m_ws)
        m_ws->SendMsg(GMT_READY, NULL, 0);
    SLOGI() << "EndgameHandler: 残局桌坐满, 自动就绪并跳转到对局页";
    if (m_pMainDlg)
        m_pMainDlg->SwitchToGame();
}

BOOL EndgameHandler::OnSeatDownAck(const void *lpData, int nSize)
{
    if (nSize < (int)sizeof(GAME_SEATDOWN_ACK))
        return FALSE;
    GAME_SEATDOWN_ACK *pAck = (GAME_SEATDOWN_ACK *)lpData;
    if (pAck->nTableId < ENDGAME_TABLE_BASE || !pAck->bSuccess)
        return TRUE;    // 非残局桌的坐桌由普通大厅处理
    MyProfile *pMy = MyProfile::getSingletonPtr();
    pMy->SetTableId(pAck->nTableId);
    pMy->SetSeatIndex(pAck->nSeat);
    // 重新入座后允许再次自动开局
    m_bAutoStartSent = false;
    SLOGI() << "EndgameHandler: seatdown ok table=" << pAck->nTableId << " seat=" << pAck->nSeat;
    return TRUE;
}

BOOL EndgameHandler::OnRobotInviteAck(const void *lpData, int nSize)
{
    if (nSize < (int)sizeof(GAME_ROBOT_INVITE_ACK))
        return FALSE;
    GAME_ROBOT_INVITE_ACK *pAck = (GAME_ROBOT_INVITE_ACK *)lpData;
    PlayTip(pAck->bSuccess ? _T("机器人已入座！") : _T("邀请机器人失败，座位可能已被占用。"));
    return TRUE;
}

void EndgameHandler::ReqSeatDown(int iTable, int iSeat)
{
    if (!m_ws)
        return;
    if (MyProfile::getSingletonPtr()->GetUID() == 0)
    {
        PlayTip(_T("请先登录"));
        return;
    }
    GAME_SEATDOWN_REQ seatId = { iTable, iSeat };
    m_ws->SendMsg(GMT_SEATDOWN_REQ, &seatId, sizeof(seatId));
    SLOGI() << "EndgameHandler: seatdown table=" << iTable << " seat=" << iSeat;
}

void EndgameHandler::ReqRobotInvite(int iTable, int iSeat, int nLevel)
{
    if (!m_ws)
        return;
    GAME_ROBOT_INVITE_REQ req;
    memset(&req, 0, sizeof(req));
    req.nTableId = iTable;
    req.nSeat = iSeat;
    req.nLevel = nLevel;
    const wchar_t *kNames[] = { L"残局机器人·初", L"残局机器人·中", L"残局机器人·高" };
    int idx = (nLevel >= ROBOT_LEVEL_BEGINNER && nLevel <= ROBOT_LEVEL_ADVANCED) ? nLevel - ROBOT_LEVEL_BEGINNER : 0;
    SStringA strName = S_CW2A(kNames[idx], CP_UTF8);
    strcpy(req.stUserInfo.szName, strName.c_str());
    req.stUserInfo.nSex = SEX_SECRET;
    req.stUserInfo.nAvatarId = 1;
    m_ws->SendMsg(GMT_ROBOT_INVITE_REQ, &req, sizeof(req));
    SLOGI() << "EndgameHandler: robot invite table=" << iTable << " seat=" << iSeat << " level=" << nLevel;
}

void EndgameHandler::SelectEndgame(int nEndgameIndex)
{
    if (!m_pAdapter || nEndgameIndex < 0)
        return;
    const ENDGAME_INFO *pInfo = m_pAdapter->GetEndgame(nEndgameIndex);
    if (!pInfo)
        return;
    m_nCurEndgameIndex = nEndgameIndex;
    RenderLayout(pInfo->layout);

    SWindow *pTxtTitle = m_pRoot ? m_pRoot->FindChildByName(L"txt_eg_detail_title") : NULL;
    SWindow *pIntro = m_pRoot ? m_pRoot->FindChildByName(L"txt_eg_detail_intro") : NULL;
    if (pTxtTitle)
    {
        SStringT strTitle;
        if (pInfo->nId > 0)
            strTitle.Format(_T("#%d %s"), pInfo->nId, S_CA2T(pInfo->szTitle, CP_UTF8).c_str());
        else
            strTitle = S_CA2T(pInfo->szTitle, CP_UTF8);
        pTxtTitle->SetWindowText(strTitle);
    }
    if (pIntro)
    {
        SStringT strIntro = SStringT().Format(_T("难度: %s | 行棋: %s"),
            (pInfo->nLevel >= 1 && pInfo->nLevel <= 3) ? (pInfo->nLevel == 1 ? _T("初级") : (pInfo->nLevel == 2 ? _T("中级") : _T("高级"))) : _T("初级"),
           pInfo->nPlayer == 0 ? _T("红先") : _T("黑先"));
        pIntro->SetWindowText(strIntro);
    }

    // 残局介绍(简述)
    SWindow *pDesc = m_pRoot ? m_pRoot->FindChildByName(L"txt_eg_detail_intro_desc") : NULL;
    if (pDesc)
        pDesc->SetWindowText(pInfo->szIntro[0] ? S_CA2T(pInfo->szIntro, CP_UTF8) : _T(""));
}

void EndgameHandler::ClearPieces()
{
    for (size_t i = 0; i < m_pieces.size(); i++)
    {
        IWindow *pPiece = m_pieces[i];
        if (pPiece)
            pPiece->Destroy();
    }
    m_pieces.clear();
}

void EndgameHandler::RenderLayout(const int layout[10][9])
{
    if (!m_pPreviewBoard || !m_pTheme)
        return;
    ClearPieces();

    SXmlNode xmlPiece = m_pTheme->GetTemplate(Template::kChessPiece);
    SStringW clsPieceName = xmlPiece.attribute(L"wndclass").as_string(L"chesspiece");
    // 与对局棋盘一致, 为每颗棋子铺设阴影层(type==11)
    SXmlNode xmlShadow = m_pTheme->GetTemplate(Template::kShadow);
    SStringW clsShadowName = xmlShadow.attribute(L"wndclass").as_string(L"img");

    for (int y = 0; y < 10; y++)
    {
        for (int x = 0; x < 9; x++)
        {
            int nChs = layout[y][x];
            if (nChs == CHSMAN_NULL)
                continue;
            CChessPiece *pPiece = (CChessPiece *)SApplication::getSingletonPtr()->CreateWindowByName(clsPieceName);
            pPiece->InitFromXml(&xmlPiece);
            pPiece->SetChessMan((CHESSMAN)nChs);
            SAnchorLayoutParam *pParam = (SAnchorLayoutParam *)pPiece->GetLayoutParam();
            SAnchorLayoutParamStruct *pParamStruct = (SAnchorLayoutParamStruct *)pParam->GetRawData();
            pParamStruct->pos.type = 10;
            pParamStruct->pos.x.fSize = (float)x;
            pParamStruct->pos.y.fSize = (float)y;//using 9-y to swap top bottom color.
            pPiece->SetPos(CPoint(x, y));
            m_pPreviewBoard->InsertIChild(pPiece);
            m_pieces.push_back(pPiece);

            // 阴影: 与 CChessGame::OnGameStart 相同的锚点(type==11)+枢轴
            IWindow *pShadow = SApplication::getSingletonPtr()->CreateWindowByName(clsShadowName);
            pShadow->InitFromXml(&xmlShadow);
            SAnchorLayoutParam *pShadowParam = (SAnchorLayoutParam *)pShadow->GetLayoutParam();
            SAnchorLayoutParamStruct *pShadowStruct = (SAnchorLayoutParamStruct *)pShadowParam->GetRawData();
            pShadowStruct->pos.type = 11;
            pShadowStruct->pos.x.fSize = (float)x;
            pShadowStruct->pos.y.fSize = (float)y;
            pShadow->SetPivot(0.5f, 0.0f);
            m_pPreviewBoard->InsertIChild(pShadow);
            m_pieces.push_back(pShadow);
        }
    }
    if (m_pPreviewBoard)
        m_pPreviewBoard->SDispatchMessage(UM_SETSCALE, m_pPreviewBoard->GetChessScale(), 1);
}

void EndgameHandler::OnPreviewSizeChanged(IEvtArgs *e)
{
    if (!m_pPreviewBoard || !m_pChessBoard)
        return;
    SSkinBoard *pSkin = sobj_cast<SSkinBoard>(GETSKIN(L"skin_chessboard", 100));
    if (!pSkin)
        return;
    CSize szBoard = pSkin->GetSkinSize();
    CRect rcGameGoard = m_pPreviewBoard->GetClientRect();
    CSize szGameBoard = rcGameGoard.Size();
    float fRatio1 = (float)szGameBoard.cx / (float)szGameBoard.cy;
    float fRatio2 = (float)szBoard.cx / (float)szBoard.cy;
    CRect rcBoard = rcGameGoard;
    float scale = 1.0f;
    if (fRatio1 > fRatio2)
    {
        int nWid = (int)(szGameBoard.cy * fRatio2);
        rcBoard.DeflateRect((szGameBoard.cx - nWid) / 2, 0);
        scale = (float)nWid / (float)szBoard.cx;
    }
    else
    {
        int nHei = (int)(szGameBoard.cx / fRatio2);
        rcBoard.DeflateRect(0, (szGameBoard.cy - nHei) / 2);
        scale = (float)nHei / (float)szBoard.cy;
    }
    m_pChessBoard->Move(rcBoard);
    CRect rcMargin = pSkin->GetMargin();
    rcMargin.left = (int)(rcMargin.left * scale);
    rcMargin.top = (int)(rcMargin.top * scale);
    rcMargin.right = (int)(rcMargin.right * scale);
    rcMargin.bottom = (int)(rcMargin.bottom * scale);
    rcBoard.DeflateRect(rcMargin);
    m_ptBoardOrigin.x = rcBoard.left;
    m_ptBoardOrigin.y = rcBoard.bottom;
    m_cellWidth = rcBoard.Width() * 1.f / 8;
    m_cellHeight = rcBoard.Height() * 1.f / 9;
    m_pPreviewBoard->SDispatchMessage(UM_SETSCALE, scale * 100, 1);
}

POINT CALLBACK EndgameHandler::ChessAnchor2Pos(const AnchorPos &pos, const CRect &rcParent,
                                               const CSize &szChild, int nScale, void *userData)
{
    if (pos.type == 10)
    {
        EndgameHandler *pThis = (EndgameHandler *)userData;
        CPoint pt = pThis->m_ptBoardOrigin;
        pt.x += (int)(pThis->m_cellWidth * pos.x.fSize);
        pt.y -= (int)(pThis->m_cellHeight * pos.y.fSize);
        pt.x += (int)(pos.fOffsetX * szChild.cx);
        pt.y += (int)(pos.fOffsetY * szChild.cy);
        return pt;
    }
    return SAnchorLayout::DefaultPosition2Point(pos, rcParent, szChild, nScale, userData);
}

void EndgameHandler::PlayTip(LPCTSTR pszTip)
{
    if (m_pMainDlg)
        m_pMainDlg->PlayTip(pszTip);
}