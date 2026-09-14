#include "stdafx.h"
#include "LobbyHandler.h"
#include "myprofile.h"
#include "MainDlg.h"
#include <cnchessProtocol.h>
#include <helper/slog.h>
#include <helper/SAdapterBase.h>
#include <control/SCmnCtrl.h>
#include <map>
#include <memory>
#include "events.h"

#define kLogTag "LobbyHandler"

class CTableAdapter: public SAdapterBase
{
    std::map<int, std::shared_ptr<GAME_TABLE_INFO> > m_tables;
    int m_nTableCount;
    LobbyHandler * m_pLobby;
public:
    CTableAdapter(LobbyHandler *pLobby)
        : m_pLobby(pLobby)
      , m_nTableCount(0)
    {
    }
    ~CTableAdapter()
    {
    }

    void SetRoomInfo(LPBYTE pData, int dwLen){
        m_tables.clear();
        BYTE *p = pData;
        int dwTableMax = *(DWORD *)p;
        p += sizeof(DWORD);
        m_nTableCount = dwTableMax;
        GAME_ROOM_INFO *pRoomInfo = (GAME_ROOM_INFO *)p;
        GAME_TABLE_INFO *pTableInfo = (GAME_TABLE_INFO*)pRoomInfo->tableInfo;
        for(int i=0;i<pRoomInfo->nTableCount;i++){
            int length = sizeof(GAME_TABLE_INFO) + (pTableInfo->nPlayers-1) * sizeof(SEAT_INFO);
            GAME_TABLE_INFO *table = (GAME_TABLE_INFO *)malloc(length);
            memcpy(table, pTableInfo, length);
            m_pLobby->OnTableInfo(table, length);
            m_tables.insert(std::make_pair(pTableInfo->nTableId, std::shared_ptr<GAME_TABLE_INFO>(table, free)));
            pTableInfo = (GAME_TABLE_INFO *)((BYTE *)pTableInfo + length);
        }
        notifyDataSetChanged();
    }

    void SetTableInfo(GAME_TABLE_INFO *pTableInfo, int dwLen){
        m_tables.erase(pTableInfo->nTableId);
        GAME_TABLE_INFO *pTableInfo2 = (GAME_TABLE_INFO *)malloc(dwLen);
        memcpy(pTableInfo2, pTableInfo, dwLen);
        m_tables.insert(std::make_pair(pTableInfo2->nTableId, std::shared_ptr<GAME_TABLE_INFO>(pTableInfo2, free)));
        notifyItemDataChanged(pTableInfo2->nTableId);
    }

    STDMETHOD_(int, getCount)() override
    {
        return m_nTableCount; ;
    }
    STDMETHOD_(void, getView)(int position, SItemPanel *pItem, SXmlNode xmlTemplate) override
    {
        if(pItem->GetChildrenCount()==0)
        {
            pItem->InitFromXml(&xmlTemplate);
        }

        // 更新桌子ID显示
        SStringT strTableId = SStringT().Format(_T("桌号: %d"), position);
        pItem->FindChildByName(L"txt_table_id")->SetWindowText(strTableId);

        // 重置难度选择为收起状态
        SWindow *pLevels = pItem->FindChildByName(L"wnd_robot_levels");
        if (pLevels)
            pLevels->SetVisible(FALSE, TRUE);

        auto slot = Subscriber(&CTableAdapter::OnButtonClick, this);

        // 订阅邀请机器人及其难度按钮事件
        const wchar_t *kRobotBtns[] = { L"btn_invite_robot", L"btn_lvl_beginner", L"btn_lvl_medium", L"btn_lvl_advanced" };
        for (int i = 0; i < 4; i++)
        {
            SWindow *pBtn = pItem->FindChildByName(kRobotBtns[i]);
            if (pBtn)
                pBtn->SubscribeEvent(EventCmd::EventID, &slot);
        }

        // 初始化座位显示
        for (int i = 0; i < PLAYER_COUNT; i++)
        {
            SStringW strSeat = SStringW().Format(L"seat_%d", i);
            SWindow *pSeatWnd = pItem->FindChildByName(strSeat);
            SImageWnd *pSeatImg = sobj_cast<SImageWnd>(pSeatWnd);

            // 设置座位图标（使用 seat_states SVG 资源）
            pSeatImg->SetIcon(i == 0 ? 0 : 3);  // 红方座位(0)或黑方座位(3)

            // 订阅事件
            pSeatWnd->SubscribeEvent(EventCmd::EventID, &slot);

            // 更新座位文本
            SStringW strSeatText = SStringW().Format(L"txt_seat_%d", i);
            pItem->FindChildByName(strSeatText)->SetWindowText(SStringT(_T("空位")));
        }

        auto it = m_tables.find(position);
        if(it != m_tables.end()){
            GAME_TABLE_INFO * table = it->second.get();
            SEAT_INFO * seatInfo = table->seatInfo;
            for(int i=0;i<table->nPlayers;i++){
                int nSeat = seatInfo->nIndex;
                SStringW strSeat = SStringW().Format(L"seat_%d", nSeat);
                SStringT strName = S_CA2T(seatInfo->stUserInfo.szName, CP_UTF8);
                SWindow *pSeatWnd = pItem->FindChildByName(strSeat);
                SImageWnd *pSeatImg = sobj_cast<SImageWnd>(pSeatWnd);

                // 设置座位状态图标
                int nIconIndex = nSeat == 0 ? 1 : 4;  // 红方占据(1)或黑方占据(4)
                if(seatInfo->bReady){
                    nIconIndex = nSeat == 0 ? 2 : 5;  // 红方准备(2)或黑方准备(5)
                }
                pSeatImg->SetIcon(nIconIndex);

                // 更新座位名称
                SStringW strSeatText = SStringW().Format(L"txt_seat_%d", nSeat);
                pItem->FindChildByName(strSeatText)->SetWindowText(strName);

                seatInfo++;
            }

            // 更新游戏状态显示
            bool bPlaying = table->tableState == TABLE_STATE_PLAYING;
            pItem->FindChildByName(L"img_playing")->SetVisible(bPlaying, TRUE);

            // 更新状态文本
            SStringT strStatus;
            if(bPlaying){
                strStatus = _T("游戏进行中");
            }else{
                // 检查是否所有玩家都准备就绪
                bool allReady = true;
                SEAT_INFO *checkSeat = table->seatInfo;
                for(int i=0;i<table->nPlayers;i++){
                    if(!checkSeat->bReady){
                        allReady = false;
                        break;
                    }
                    checkSeat++;
                }
                strStatus = allReady ? _T("等待开始") : _T("等待准备");
            }
            pItem->FindChildByName(L"txt_status")->SetWindowText(strStatus);
        }else{
            pItem->FindChildByName(L"img_playing")->SetVisible(FALSE, TRUE);
            pItem->FindChildByName(L"txt_status")->SetWindowText(_T("等待入座"));
        }
    }

    BOOL OnButtonClick(IEvtArgs *e)
    {
        SWindow *pBtn = sobj_cast<SWindow>(e->Sender());
        SItemPanel *pItem = sobj_cast<SItemPanel>(pBtn->GetRoot());
        int nTableId = pItem->GetItemIndex();
        SStringW strName = pBtn->GetName();

        // 邀请机器人: 展开/收起难度选择(初/中/高 图片按钮)
        if (strName == L"btn_invite_robot")
        {
            SWindow *pLevels = pItem->FindChildByName(L"wnd_robot_levels");
            if (pLevels)
                pLevels->SetVisible(!pLevels->IsVisible(), TRUE);
            return TRUE;
        }

        // 选择难度: 邀请机器人坐到本桌的空位上(2人对局取当前用户对座)
        if (strName == L"btn_lvl_beginner" || strName == L"btn_lvl_medium" || strName == L"btn_lvl_advanced")
        {
            int nLevel = (strName == L"btn_lvl_beginner") ? ROBOT_LEVEL_BEGINNER
                       : (strName == L"btn_lvl_medium") ? ROBOT_LEVEL_MEDIUM
                       : ROBOT_LEVEL_ADVANCED;
            int nMySeat = MyProfile::getSingletonPtr()->GetSeatIndex();
            if (nMySeat < 0 || nMySeat >= PLAYER_COUNT)
            {
                m_pLobby->NotifyToast(L"请先入座后再邀请机器人！");
                return TRUE;
            }
            int nTargetSeat = (nMySeat + 1) % PLAYER_COUNT;
            m_pLobby->ReqRobotInvite(nTableId, nTargetSeat, nLevel);
            // 收起难度选择
            SWindow *pLevels = pItem->FindChildByName(L"wnd_robot_levels");
            if (pLevels)
                pLevels->SetVisible(FALSE, TRUE);
            return TRUE;
        }

        int id = pBtn->GetID()-10;
        m_pLobby->ReqSeatDown(nTableId, id);
        return TRUE;
    }
};

//---------------------------------------------------------------------------
LobbyHandler::LobbyHandler(CMainDlg *pMainDlg):m_pRoot(NULL),m_pAdapter(NULL),m_pMainDlg(pMainDlg)
{

}
LobbyHandler::~LobbyHandler()
{
    if(m_pAdapter){
        m_pAdapter->Release();
        m_pAdapter = NULL;
    }
}

void LobbyHandler::Init(SWindow *pRoot, WebSocketClient *pWs)
{
    m_pRoot = pRoot;
    m_pRoot->AddEvent(EVENTID(EventTableInfo));

    m_ws = pWs;
    STileView *pTileView = m_pRoot->FindChildByName2<STileView>(L"tileview_lobby");
    m_pAdapter = new CTableAdapter(this);
    pTileView->SetAdapter(m_pAdapter);
}

BOOL LobbyHandler::OnMessage(DWORD dwType, std::shared_ptr<std::vector<BYTE> > data)
{
    const BYTE *pData = data?data->data():NULL;
    int nSize = data?data->size():0;
    BOOL ret = FALSE;
    switch(dwType)
    {
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
        case GMT_LOGIN_ACK:
            OnLoginAck(pData, nSize);
            break;
    }
    return ret;
}

BOOL LobbyHandler::OnRoomInfo(const void *lpData, int nSize)
{
    GAME_ROOM_INFO *pRoomInfo = (GAME_ROOM_INFO *)lpData;
    SLOGI() << "OnRoomInfo: nTableCount=" << pRoomInfo->nTableCount;
    m_pAdapter->SetRoomInfo((LPBYTE)pRoomInfo, nSize);
    return TRUE;
}

BOOL LobbyHandler::OnTableInfo(const void *lpData, int nSize)
{
    GAME_TABLE_INFO *pTableInfo = (GAME_TABLE_INFO *)lpData;
    SLOGI() << "OnTableInfo: nTableId=" << pTableInfo->nTableId << " nPlayers=" << pTableInfo->nPlayers;
    m_pAdapter->SetTableInfo(pTableInfo, nSize);

    MyProfile *pMyProfile = MyProfile::getSingletonPtr();
    if(pTableInfo->nTableId == pMyProfile->GetTableId()){
        //notify table info
        SLOGI() << "OnTableInfo: notify table info";
        EventTableInfo evt(this);
        evt.pTableInfo = pTableInfo;
        evt.nSize = nSize;
        m_pRoot->FireEvent(evt);
    }
    return TRUE;
}

BOOL LobbyHandler::OnSeatDownAck(const void *lpData, int nSize)
{
    GAME_SEATDOWN_ACK *pAck = (GAME_SEATDOWN_ACK *)lpData;
    SLOGI() << "OnSeatDownAck: nTableId=" << pAck->nTableId << " nSeat=" << pAck->nSeat << " bSuccess=" << pAck->bSuccess;
    if(pAck->bSuccess == FALSE)
        return TRUE;
    MyProfile *pMyProfile = MyProfile::getSingletonPtr();
    pMyProfile->SetTableId(pAck->nTableId);
    pMyProfile->SetSeatIndex(pAck->nSeat);

    return TRUE;
}

void LobbyHandler::ReqSeatDown(int iTable, int iSeat)
{
    SLOGI() << "ReqSeatDown: iTable=" << iTable << " iSeat=" << iSeat;
    GAME_SEATDOWN_REQ seatId = {iTable, iSeat};
    m_ws->SendMsg(GMT_SEATDOWN_REQ, &seatId, sizeof(seatId));
}

void LobbyHandler::ReqRobotInvite(int iTable, int iSeat, int nLevel)
{
    SLOGI() << "ReqRobotInvite: iTable=" << iTable << " iSeat=" << iSeat << " nLevel=" << nLevel;
    GAME_ROBOT_INVITE_REQ req;
    memset(&req, 0, sizeof(req));
    req.nTableId = iTable;
    req.nSeat = iSeat;
    req.nLevel = nLevel;
    const wchar_t *kNames[] = { L"机器人·初", L"机器人·中", L"机器人·高" };
    int idx = (nLevel >= ROBOT_LEVEL_BEGINNER && nLevel <= ROBOT_LEVEL_ADVANCED) ? nLevel - ROBOT_LEVEL_BEGINNER : 0;
    SStringA strName = S_CW2A(kNames[idx], CP_UTF8);
    strcpy(req.stUserInfo.szName, strName.c_str());
    req.stUserInfo.nSex = SEX_SECRET;
    req.stUserInfo.nAvatarId = 1; // 男性内置头像
    m_ws->SendMsg(GMT_ROBOT_INVITE_REQ, &req, sizeof(req));
}

BOOL LobbyHandler::OnRobotInviteAck(const void *lpData, int nSize)
{
    if (nSize < sizeof(GAME_ROBOT_INVITE_ACK))
        return FALSE;
    GAME_ROBOT_INVITE_ACK *pAck = (GAME_ROBOT_INVITE_ACK *)lpData;
    SLOGI() << "OnRobotInviteAck: nTableId=" << pAck->nTableId << " nSeat=" << pAck->nSeat << " bSuccess=" << pAck->bSuccess;
    if (pAck->bSuccess)
    {
        NotifyToast(_T("机器人已入座！"));
    }
    else
    {
        NotifyToast(_T("邀请机器人失败，座位可能已被占用。"));
    }
    return TRUE;
}

void LobbyHandler::NotifyToast(LPCTSTR pszMsg)
{
    m_pMainDlg->PlayTip(pszMsg);
}

void LobbyHandler::OnConnected()
{
   //req login.
   MyProfile *pMyProfile = MyProfile::getSingletonPtr();
   int avatarSize = pMyProfile->GetAvatarData()?pMyProfile->GetAvatarData()->size():0;
   int len= sizeof(GAME_LOGIN_REQ) -1 + avatarSize;
   std::shared_ptr<GS_USERINFO> pUserInfo = pMyProfile->GetUserInfo();
   GAME_LOGIN_REQ *pLogin = (GAME_LOGIN_REQ *)malloc(len);
   memcpy(pLogin, pUserInfo.get(), sizeof(GS_USERINFO));
   pLogin->dwLen = avatarSize;
   if(avatarSize){
       memcpy(pLogin->byData, pMyProfile->GetAvatarData()->data(), pMyProfile->GetAvatarData()->size());
   }
   m_ws->SendMsg(GMT_LOGIN_REQ, (LPVOID)pLogin, len);
   free(pLogin);
}

BOOL LobbyHandler::OnLoginAck(const void *lpData, int nSize)
{
    if(nSize < sizeof(GAME_LOGIN_ACK))
        return FALSE;
    GAME_LOGIN_ACK *pAck = (GAME_LOGIN_ACK *)lpData;
    SLOGI() << "OnLoginAck: uid=" << pAck->uid << " errCode=" << pAck->errCode;
    if(pAck->errCode != ERR_SUCCESS)
        return FALSE;
    MyProfile *pMyProfile = MyProfile::getSingletonPtr();
    pMyProfile->SetUID(pAck->uid);

    // 格式化游戏规则信息
    SStringT strInfo;
    strInfo.Format(_T("步时: %d | 最小步数: %d | 悔棋: %d | 局时: %d"),
        pAck->dwProps[PROPID_TIME_STEP],
        pAck->dwProps[PROPID_MIN_STEPS],
        pAck->dwProps[PROPID_REGRET],
        pAck->dwProps[PROPID_TIME_ROUND]);

    m_pRoot->FindChildByName(L"txt_table_info")->SetWindowText(strInfo);
    return TRUE;
}

void LobbyHandler::OnDisconnected()
{
}