#include "stdafx.h"
#include "LobbyHandler.h"
#include "myprofile.h"
#include <cnchessProtocol.h>
#include <helper/slog.h>
#include <helper/SAdapterBase.h>
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
        pItem->FindChildByName(L"txt_table_id")->SetWindowText(SStringT().Format(_T("%d"), position));
        auto slot = Subscriber(&CTableAdapter::OnButtonClick, this);

        for (int i = 0; i < PLAYER_COUNT; i++)
        {
            SStringW strSeat = SStringW().Format(L"seat_%d", i);
            SWindow *pSeat = pItem->FindChildByName(strSeat);
            pSeat->SubscribeEvent(EventCmd::EventID, &slot);
            pSeat->SetWindowText(_T("-"));
            pSeat->FindChildByName(L"seat_ready")->SetVisible(FALSE, TRUE);
        }
        auto it = m_tables.find(position);
        if(it != m_tables.end()){
            GAME_TABLE_INFO * table = it->second.get();
            SEAT_INFO * seatInfo = table->seatInfo;
            for(int i=0;i<table->nPlayers;i++){
                int nSeat = seatInfo->nIndex;
                SStringW strSeat = SStringW().Format(L"seat_%d", nSeat);
                SStringT strName = S_CA2T(seatInfo->stUserInfo.szName, CP_UTF8);
                SWindow *pSeat = pItem->FindChildByName(strSeat);
                pSeat->SetWindowText(strName);
                pSeat->FindChildByName(L"seat_ready")->SetVisible(seatInfo->bReady, TRUE);
                seatInfo++;
            }
            pItem->FindChildByName(L"img_playing")->SetVisible(table->tableState == TABLE_STATE_PLAYING, TRUE);
        }else{
            pItem->FindChildByName(L"img_playing")->SetVisible(FALSE, TRUE);
        }
    }

    BOOL OnButtonClick(IEvtArgs *e)
    {
        SWindow *pBtn = sobj_cast<SButton>(e->Sender());
        SItemPanel *pItem = sobj_cast<SItemPanel>(pBtn->GetRoot());
        int nTableId = pItem->GetItemIndex();
        int id = pBtn->GetID()-10;
        m_pLobby->ReqSeatDown(nTableId, id);
        return TRUE;
    }
};

//---------------------------------------------------------------------------
LobbyHandler::LobbyHandler():m_pRoot(NULL),m_pAdapter(NULL)
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
    SStringT strInfo;
    strInfo = _T("步时: ") + SStringT().Format(_T("%d"), pAck->dwProps[PROPID_TIME_STEP]);
    strInfo = _T("最小步数限制: ") + SStringT().Format(_T("%d"), pAck->dwProps[PROPID_MIN_STEPS]);
    strInfo = _T("悔棋次数: ") + SStringT().Format(_T("%d"), pAck->dwProps[PROPID_REGRET]);
    strInfo = _T("长捉次数: ") + SStringT().Format(_T("%d"), pAck->dwProps[PROPID_MAX_STP_CATCH]);
    strInfo = _T("长将次数: ") + SStringT().Format(_T("%d"), pAck->dwProps[PROPID_MAX_STP_JIANG]);
    strInfo = _T("不吃子步数: ") + SStringT().Format(_T("%d"), pAck->dwProps[PROPID_MAX_STP_PEACE]);
    strInfo = _T("局时: ") + SStringT().Format(_T("%d"), pAck->dwProps[PROPID_TIME_ROUND]);

    m_pRoot->FindChildByName(L"txt_table_info")->SetWindowText(strInfo);
    return TRUE;
}

void LobbyHandler::OnDisconnected()
{
}