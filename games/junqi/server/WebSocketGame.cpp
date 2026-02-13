// WebSocketGame.cpp: WebSocket版本的游戏服务器基类实现
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WebSocketGame.h"
#include <protocol.h>
#include <tchar.h>
#include <string>
#include <map>
#include <algorithm>
#include <sstream>
#include "Junqi.h"
#include "PropBag.h"
#include <helper/slog.h>
#define kLogTag "WebSocketGame"

class CUrlArgs {
	typedef std::map<std::string, std::string> ArgMap;
  public:
    CUrlArgs(LPCSTR pszUrl)
    {
        Parse(pszUrl);
    }

	std::string Get(LPCSTR pszKey) const
	{
		ArgMap::const_iterator it = m_args.find(pszKey);
		if (it != m_args.end())
			return it->second;
		return "";
	}
	int GetInt(LPCSTR pszKey, int nDefault) const
	{
		std::string strValue = Get(pszKey);
		return strValue.empty() ? nDefault : atoi(strValue.c_str());
	}
  private:
    void Parse(LPCSTR pszUrl)
    {
        std::string strUrl = pszUrl;
        size_t nStart = 0;
        while (nStart < strUrl.size())
        {
            size_t nEqual = strUrl.find('=', nStart);
            if (nEqual == std::string::npos)
                break;
            std::string strKey = strUrl.substr(nStart, nEqual - nStart);
            size_t nAnd = strUrl.find('&', nEqual + 1);
            if (nAnd == std::string::npos)
                break;
            std::string strValue = strUrl.substr(nEqual + 1, nAnd - nEqual - 1);
            m_args[strKey] = strValue;
            nStart = nAnd + 1;
        }
    }

    ArgMap m_args;
};

//////////////////////////////////////////////////////////////////////
// WebSocketSvrListener Implementation
//////////////////////////////////////////////////////////////////////

WebSocketSvrListener::WebSocketSvrListener(CWebSocketGame* pGame)
	: m_pGame(pGame)
{
}

WebSocketSvrListener::~WebSocketSvrListener()
{
}

BOOL WebSocketSvrListener::onConnected(ISvrConnection* pConn, const char* uriPath, const char* uriArgs)
{
	if (!m_pGame || !pConn)
		return FALSE;

	// 检查路径是否正确
	if (strcmp(uriPath, "/junqi/") != 0)
		return FALSE;

	// 创建客户端对象
    PWSCLIENT pClient = m_pGame->CreateClient(pConn, uriPath, uriArgs);
	if (!pClient)
		return FALSE;
	// send client room props

	return TRUE;
}

void WebSocketSvrListener::onConnError(ISvrConnection* pConn, const char* errStr)
{
	// 连接错误，暂时不需要特殊处理
}

void WebSocketSvrListener::onDisconnect(ISvrConnection* pConn)
{
	if (m_pGame && pConn)
	{
		m_pGame->RemoveClient(pConn);
	}
}

void WebSocketSvrListener::onDataSent(ISvrConnection* pConn, int nMsgId)
{
	// 数据发送完成，暂时不需要特殊处理
}

void WebSocketSvrListener::onDataRecv(ISvrConnection* pConn, const void* data, int len, BOOL bBinary)
{
	if (m_pGame && pConn && data && len > 0)
	{
		m_pGame->ProcessReceivedData(pConn, data, len);
	}
}

//////////////////////////////////////////////////////////////////////
// CWebSocketGame Implementation
//////////////////////////////////////////////////////////////////////

CWebSocketGame::CWebSocketGame()
{
	m_nextUid = 1;
	m_uPort = 0;
    m_pListener = NULL;
    m_pWsServer = NULL;
    m_nMaxTable = 100; // 默认最大桌子数;
}

CWebSocketGame::~CWebSocketGame()
{
    m_tableClients.clear();
	
	// clear temp clients
    for (auto it = m_tmpClients.begin(); it != m_tmpClients.end(); ++it) {
		PWSCLIENT pClient = *it;
		delete pClient;
    }

	if (m_pWsServer)
	{
		m_pWsServer->quit();
		m_pWsServer = NULL;
	}
}

BOOL CWebSocketGame::GameStart(unsigned short uPort)
{
    if (m_pListener)
        return FALSE;
    // 创建WebSocket组件
    BOOL bOK = m_comLoader.CreateWS((IObjRef **)&m_pWebsocket);
    if (!bOK || !m_pWebsocket)
        return FALSE;

    // 创建服务器监听器
    m_pListener = new WebSocketSvrListener(this);
    if (!m_pListener)
        return FALSE;

    // 创建WebSocket服务器
    m_pWsServer = m_pWebsocket->CreateWsServer(m_pListener);
    if (!m_pWsServer)
    {
        return FALSE;
    }
	m_uPort = uPort;

	// 启动服务器
	SvrOption option = { FALSE, NULL, NULL }; // 非安全连接
    SvrPingCfg pingCfg = { 50, 100, 10 };
    SLOGI() << "Started on port:" << uPort;
	int nRet = m_pWsServer->start(uPort, "junqi", option, pingCfg);
	if (nRet != 0)
		return FALSE;
    bool bFinish = m_pWsServer->wait(-1);
    m_pWsServer->Release();
	return TRUE;
}

void CWebSocketGame::GameStop()
{
	if (m_pWsServer)
	{
		m_pWsServer->quit();
	}
}

PWSCLIENT CWebSocketGame::CreateClient(ISvrConnection *pConn, LPCSTR uriPath, LPCSTR pszArgs)
{
	CUrlArgs args(pszArgs);
    int nTable = args.GetInt("table", 0);
    int nSeat = args.GetInt("seat", -1);

	LPCSTR pszGameEnd = strchr(uriPath, '?');

    std::string strGame = "junqi";
    if (pszGameEnd)
	{
		strGame = std::string(uriPath + 1, pszGameEnd - uriPath - 1);
	}
    if (strGame == "junqi")
    {
        pConn->setGroupId(0); // set group id to 0 for upgrade server
        pConn->setId(-1);     // temp client id
        PWSCLIENT pClient = new GameClient;
        pClient->m_pConn = pConn;
        pClient->m_strGameName = strGame;
        pClient->m_nIndex = -1;
        pClient->m_nTable = -1;
        pClient->m_dwState = 0;
        m_tmpClients.push_back(pClient);
		SLOGI()<<"add client = "<<pClient;
        return pClient;
    }
	else
	{
        return NULL;
    }
}

void CWebSocketGame::RemoveClient(ISvrConnection* pConn)
{
    int gameId = pConn->getGroupId();
    if (gameId != 0)
    {
        return; // unknown client
    }
    int id = pConn->getId();
    if (id == -1)
    {
        // 在临时客户端列表中查找
        for (auto iter = m_tmpClients.begin(); iter != m_tmpClients.end(); ++iter)
        {
            PWSCLIENT pClient = *iter;
            if (pClient->m_pConn == pConn)
            {
				SLOGI()<<"remove tmp client = "<<pClient;
                delete pClient;
                m_tmpClients.erase(iter);
				break;
            }
        }
		return;
    }

    int iTable = id / PLAYER_COUNT;
    int iSeat = id % PLAYER_COUNT;
    auto it = m_tableClients.find(iTable);
	if (it == m_tableClients.end())
        return; // unknown client
	it->second->OnRemovePlayer(iSeat);
	if(it->second->GetPlayerCount() == 0){
		m_tableClients.erase(it);
	}
	notifyRoomInfoChanged();
}

void CWebSocketGame::ProcessReceivedData(ISvrConnection* pConn, const void* data, int len)
{
	if (!data || len < 8) // 至少需要8字节的消息头
		return;
    PWSCLIENT pClient = NULL;
	int gameId = pConn->getGroupId();
	if(gameId !=0)
	{
		SLOGW()<<"invalid game Id:"<<gameId;
		return;
	}
	int id = pConn->getId();
	if(id != -1){
		int iTable = id / PLAYER_COUNT;
        int iSeat = id % PLAYER_COUNT;
		auto it = m_tableClients.find(iTable);
		if (it == m_tableClients.end())
		{
			SLOGW()<<"table not found, table id:"<<iTable;
			return;
		}
		int playerCount = 0;
		PWSCLIENT * pClients = it->second->GetClients(&playerCount);
		if(iSeat >= playerCount){
			SLOGW()<<"invalid seat id:"<<iSeat<<" player count:"<<playerCount;
			return;
		}
		pClient = pClients[iSeat];
		if (!pClient)
		{
			SLOGW()<<"client not found, table id:"<<iTable<<" seat:"<<iSeat;
			return;
		}
	}else{
				// 在临时客户端列表中查找
		for (auto it = m_tmpClients.begin(); it != m_tmpClients.end(); ++it) {
			PWSCLIENT pTmpClient = *it;
			if (pTmpClient->m_pConn == pConn) {
				pClient = pTmpClient;
				break;
			}
        }
	}

	const char* pData = (const char*)data;
	DWORD dwType, dwSize;

	memcpy(&dwType, pData, 4);
	memcpy(&dwSize, pData + 4, 4);

	if (dwSize > MAX_MSGSIZE || (int)(dwSize + 8) != len)
		return; // 数据格式错误

	// 调用消息处理函数
    OnMsg(pClient, dwType, (LPVOID)(pData+8), dwSize);
}

BOOL CWebSocketGame::SendMsg(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)
{
	if (dwSize > MAX_MSGSIZE)
		return FALSE;

	BOOL bResult = FALSE;
	if (pClient == NULL)
	{
		// 广播给所有客户端
		for(auto it = m_tmpClients.begin(); it != m_tmpClients.end(); ++it)
		{
			PWSCLIENT pTmpClient = *it;
			if (pTmpClient->m_pConn)
			{
				pTmpClient->m_pConn->sendBinary2(dwType, pData, dwSize);
			}
		}
		for(auto it = m_tableClients.begin(); it != m_tableClients.end(); ++it)
		{
			it->second->Boardcast(dwType, pData, dwSize);
		}

		bResult = TRUE;
	}
	else if (pClient->m_pConn)
	{
		// 发送给指定客户端
		int nRet = pClient->m_pConn->sendBinary2(dwType, pData, dwSize);
		bResult = (nRet == 0);
	}
	return bResult;
}


BOOL CWebSocketGame::ClientLogin(PWSCLIENT pClient, LPVOID pData, DWORD dwSize)
{
	GAME_LOGIN_REQ *pLogin = (GAME_LOGIN_REQ *)pData;
    if (dwSize < sizeof(GS_USERINFO))
        return FALSE;
	if(pClient->m_userInfo.uid!=0)
	{
		GAME_LOGIN_ACK ack;
		ack.uid = 0;
		ack.errCode = ERR_USER_EXIST;
		SendMsg(pClient, GMT_LOGIN_ACK, &ack, sizeof(ack));
		return FALSE;
	}
	memcpy(&pClient->m_userInfo, pLogin, sizeof(GS_USERINFO));
	pClient->m_userInfo.uid = m_nextUid++;
	if(dwSize>=sizeof(GS_USERINFO)+sizeof(DWORD)+pLogin->dwLen){
		//login with avatar
		pClient->m_avatar = std::make_shared<std::vector<BYTE> >(pLogin->byData, (BYTE*)pLogin+dwSize);
	}else{
		pClient->m_avatar = nullptr;
	}
	GAME_LOGIN_ACK ack;
	ack.uid = pClient->m_userInfo.uid;
	ack.errCode = ERR_SUCCESS;
	memcpy(&ack.dwProps, PropBag::getSingletonPtr()->m_dwProps, sizeof(ack.dwProps));
	SendMsg(pClient, GMT_LOGIN_ACK, &ack, sizeof(ack));
    sendRoomInfo(pClient);
    return TRUE;
}

BOOL CWebSocketGame::ClientSeatDown(PWSCLIENT pClient, LPVOID pData, DWORD dwSize)
{
    if (dwSize < sizeof(SeatID))
        return FALSE;
    int gameId = pClient->m_pConn->getGroupId();
    if (gameId != 0)
        return FALSE;
    SeatID *pSeatID = (SeatID *)pData;
	int id = pClient->m_pConn->getId();
	if(id != -1){
        int nTable = id / PLAYER_COUNT;
        auto it = m_tableClients.find(nTable);
        if(it->second->GetState() != TABLE_STATE_WAIT){
			GAME_SEATDOWN_ACK ack = {pSeatID->nTableId, pSeatID->nSeat, FALSE};
			SendMsg(pClient, GMT_SEATDOWN_ACK, &ack, sizeof(ack));
			return FALSE;
		}
	}
    if (pSeatID->nTableId >= m_nMaxTable)
        return FALSE;
    if (pSeatID->nTableId < 0 || pSeatID->nSeat < 0)
    {
        GAME_SEATDOWN_ACK ack = { pSeatID->nTableId, pSeatID->nSeat , FALSE };
        if (!OnQuerySeat(pSeatID))
        {
			SendMsg(pClient, GMT_SEATDOWN_ACK, &ack, sizeof(ack));
			return FALSE;
		}    
    }
    if (pSeatID->nSeat < 0 || pSeatID->nSeat >= PLAYER_COUNT)
    {
        SLOGE() << "invalid seat number:" << pSeatID->nSeat;
        return FALSE;
	}
	if(id != -1){
		//get up first and set ready to false
        int nTable = id / PLAYER_COUNT;
        int nSeat = id % PLAYER_COUNT;
        auto it = m_tableClients.find(nTable);
        it->second->OnPlayerLeave(nSeat, pClient);
		pClient->m_bReady = FALSE;
        pClient->m_pConn->setId(-1);
        id = -1;
        if (it->second->GetPlayerCount() == 0)
        {//dismiss the table
            m_tableClients.erase(it);
        }
	}
    // remove client from temp list
    for (auto it = m_tmpClients.begin(); it != m_tmpClients.end(); ++it)
    {
        if (*it == pClient)
        {
            m_tmpClients.erase(it);
            break;
        }
    }
    auto it = m_tableClients.find(pSeatID->nTableId);
    if (it == m_tableClients.end())
    {
		SAutoRefPtr<IGameTable> pTable(new CJunQi(this, pSeatID->nTableId),false);
        it = m_tableClients.insert(std::make_pair(pSeatID->nTableId, pTable)).first;
    }else{
		if(it->second->HasPlayer( pSeatID->nSeat))
		{
			GAME_SEATDOWN_ACK ack = {pSeatID->nTableId, pSeatID->nSeat, FALSE};
			m_tmpClients.push_back(pClient);
			pClient->m_pConn->setId(-1);
			//notify the client seatdown failed
			SendMsg(pClient, GMT_SEATDOWN_ACK, &ack, sizeof(ack));
            if (id != -1)
            {
                notifyRoomInfoChanged();
            }
			return FALSE;
		}	
	}
	GAME_SEATDOWN_ACK ack = {pSeatID->nTableId, pSeatID->nSeat, TRUE};
	SendMsg(pClient, GMT_SEATDOWN_ACK, &ack, sizeof(ack));
	
    pClient->m_nTable = pSeatID->nTableId;
    pClient->m_nIndex = pSeatID->nSeat;
    id = pSeatID->nTableId * PLAYER_COUNT + pSeatID->nSeat;
    pClient->m_pConn->setId(id);
	it->second->OnAddPlayer(pSeatID->nSeat, pClient);

	OnTableChange(pClient->m_nTable);
    SLOGI() << "Client Seat Down: table=" << pSeatID->nTableId << ", seat=" << pSeatID->nSeat << ", client=" << pClient << " id="<<id;
    return TRUE;
}

BOOL CWebSocketGame::ClientGetUp(PWSCLIENT pClient, LPVOID pData, DWORD dwSize)
{
	GAME_GETUP_REQ *pMsg = (GAME_GETUP_REQ *)pData;
	int id = pClient->m_pConn->getId();
	if(id < 0)
		return FALSE;
	int nTableId = id / PLAYER_COUNT;
	int nSeat = id % PLAYER_COUNT;
	if(nTableId != pMsg->nTableId || nSeat != pMsg->nSeat)
		return FALSE;
	auto it = m_tableClients.find(nTableId);
	if (it == m_tableClients.end())
		return FALSE;
	if(!it->second->OnPlayerLeave(nSeat,pClient))
		return FALSE;
	pClient->m_pConn->setId(-1);
	pClient->m_nTable = -1;
	pClient->m_nIndex = -1;
	m_tmpClients.push_back(pClient);

	OnTableChange(nTableId);

    return TRUE;
}

BOOL CWebSocketGame::OnQuerySeat(SeatID * pSeatID)
{
	int iTable = pSeatID->nTableId;
	if(iTable<0){
		// find seat from existing tables
		for (auto it = m_tableClients.begin(); it != m_tableClients.end(); ++it){
			int nSeat = it->second->GetEmptySeat();
			if(nSeat>=0){
				pSeatID->nTableId = it->first;
				pSeatID->nSeat = nSeat;
				return TRUE;
			}
		}
        // find empty table
        for (int i = 0; i < m_nMaxTable; i++)
        {
            if (m_tableClients.find(i) == m_tableClients.end())
            {
                iTable = i;
                break;
            }
        }
        if (iTable < 0)
        {
            return FALSE;
        }
    }
	if(iTable >= m_nMaxTable){
		return FALSE;
    }
    BOOL bRet = FALSE;
    auto it = m_tableClients.find(iTable);
    if (it != m_tableClients.end())
    {
		int nSeat = it->second->GetEmptySeat();
		if(nSeat>=0){
			pSeatID->nTableId = iTable;
			pSeatID->nSeat = nSeat;
			return TRUE;
		}
    }
    else
    {
        pSeatID->nTableId = iTable;
        pSeatID->nSeat = 0;
        bRet = TRUE; // create new table, just return seatId, not need to send seatdown msg to client.
    }

    return bRet; // not found seat, return false. need to send seatdown msg to client.
}

BOOL CWebSocketGame::OnMsg(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)
{
	// 处理基本消息类型
	switch (dwType)
	{
	case GMT_LOGIN_REQ:
		return ClientLogin(pClient, pData, dwSize);
	case GMT_SEATDOWN_REQ:
		return ClientSeatDown(pClient, pData, dwSize);
	case GMT_GETUP_REQ:
		return ClientGetUp(pClient, pData, dwSize);
	case GMT_AVATAR_REQ:
		return ClientAvatar(pClient, pData, dwSize);
	case GMT_READY:
		return ClientReady(pClient, pData, dwSize);
	default:
		{
			//转发到游戏桌处理
		   int tableId = pClient->m_nTable;
		   auto it = m_tableClients.find(tableId);
		   if (it != m_tableClients.end())
		   {
			   return it->second->OnMsg(pClient, dwType, pData, dwSize);
		   }else{
			   SLOGE()<<"tableId not found:"<<tableId<<" type:"<<dwType;
		   }
		}
		return FALSE;
	}
}

//notify all clients in the room table information change
void CWebSocketGame::OnTableChange(int nTableId)
{
	auto it = m_tableClients.find(nTableId);
	if (it == m_tableClients.end())
	{
		SLOGW()<<"Table not found, table id:"<<nTableId;
		return;
	}
	int nPlayerCount = 0;
	PWSCLIENT *pClients = it->second->GetClients(&nPlayerCount);
	if(it->second->GetState() == TABLE_STATE_WAIT){
		//check for ready 
		BOOL bAllReady = TRUE;
		for (int i=0;i<nPlayerCount;i++)
		{
			PWSCLIENT client = pClients[i];
			if(!client || !client->m_bReady){
				bAllReady = FALSE;
				break;
			}
		}
		if(bAllReady){
			it->second->OnGameStart();
		}
	}

	int nPlayers = it->second->GetPlayerCount();
	if(nPlayers == 0){
		m_tableClients.erase(it);
	}else{
		std::stringstream ss;
		ss.write((char*)&nTableId, sizeof(int));
		ss.write((char*)&nPlayers, sizeof(int));
		int state = it->second->GetState();
		ss.write((char*)&state, sizeof(int));
		for (int i=0;i<nPlayerCount;i++)
		{
			PWSCLIENT client = pClients[i];
			if(client){
				ss.write((char*)&client->m_nIndex, sizeof(int));
				ss.write((char*)&client->m_bReady, sizeof(uint32_t));
				ss.write((char*)&client->m_userInfo, sizeof(GS_USERINFO));
			}
		}
		it->second->Boardcast(GMT_TABLE_INFO, (LPVOID)ss.str().c_str(), ss.str().size());
	}
    notifyRoomInfoChanged();
}


void CWebSocketGame::notifyRoomInfoChanged()
{
    std::stringstream ss;
    ss.write((char *)&m_nMaxTable, sizeof(int));
    int nTableCount = m_tableClients.size();
    ss.write((char *)&nTableCount, sizeof(int));
    for (auto &table : m_tableClients)
    {
        ss.write((char *)&table.first, sizeof(int));
        int nPlayers = table.second->GetPlayerCount();
        ss.write((char *)&nPlayers, sizeof(int));
        int state = table.second->GetState();
        ss.write((char *)&state, sizeof(int));
        int nPlayerCount = 0;
        PWSCLIENT *pClients = table.second->GetClients(&nPlayerCount);
        for (int i = 0; i < nPlayerCount; i++)
        {
            if (pClients[i] == nullptr)
            {
                continue;
            }
            ss.write((char *)&pClients[i]->m_nIndex, sizeof(int));
            ss.write((char *)&pClients[i]->m_bReady, sizeof(uint32_t));
            ss.write((char *)&pClients[i]->m_userInfo, sizeof(GS_USERINFO));
        }
    }

	for(auto it = m_tmpClients.begin(); it != m_tmpClients.end(); ++it)
	{
		PWSCLIENT pClient = *it;
        SendMsg(pClient, GMT_ROOM_INFO, (LPVOID)ss.str().c_str(), ss.str().size());
	}
	for(auto it = m_tableClients.begin(); it != m_tableClients.end(); ++it)
	{
		int nCount = 0;
		PWSCLIENT * pClients = it->second->GetClients(&nCount);
		for (int i = 0; i < nCount; i++)
		{
			if (pClients[i])
			{
                SendMsg(pClients[i], GMT_ROOM_INFO, (LPVOID)ss.str().c_str(), ss.str().size());
			}
		}
	}
}

void CWebSocketGame::sendRoomInfo(PWSCLIENT pClient)
{
	std::stringstream ss;
	ss.write((char*)&m_nMaxTable, sizeof(int));
	int nTableCount = m_tableClients.size();
	ss.write((char*)&nTableCount, sizeof(int));
	for (auto &table : m_tableClients)
	{
		ss.write((char*)&table.first, sizeof(int));
		int nPlayers = table.second->GetPlayerCount(); 
		ss.write((char*)&nPlayers, sizeof(int));
        int state = table.second->GetState();
        ss.write((char *)&state, sizeof(int));
		int nPlayerCount = 0;
		PWSCLIENT *pClients = table.second->GetClients(&nPlayerCount);
		for (int i = 0; i < nPlayerCount; i++)
		{
			if(pClients[i] == nullptr){
				continue;
			}
			ss.write((char*)&pClients[i]->m_nIndex, sizeof(int));
			ss.write((char *)&pClients[i]->m_bReady, sizeof(uint32_t));
			ss.write((char *)&pClients[i]->m_userInfo, sizeof(GS_USERINFO));
		}
	}
	SendMsg(pClient, GMT_ROOM_INFO, (LPVOID)ss.str().c_str(), ss.str().size());	
	SLOGI() << "sendRoomInfo to client " << pClient << " data size="<<ss.str().size();
}

BOOL CWebSocketGame::ClientAvatar(PWSCLIENT pClient, LPVOID pData, DWORD dwSize){
	if(dwSize < sizeof(GAME_AVATAR_REQ)){
		return FALSE;
	}
	GAME_AVATAR_REQ *pReq = (GAME_AVATAR_REQ *)pData;
    SLOGI() << "GetClientAvatar, uid=" << pReq->uid << " from uid="<<pClient->m_userInfo.uid;
	if(pReq->nTableId == -1){
		//target uid has not seat down
		for(auto &client : m_tmpClients){
			if(client->m_userInfo.uid == pReq->uid){
				if(client->m_avatar){
					std::stringstream ss;
					ss.write((char*)&client->m_userInfo.uid, sizeof(uint32_t));
					DWORD dwSize = client->m_avatar->size();
					ss.write((char*)&dwSize, sizeof(DWORD));
					ss.write((char*)client->m_avatar->data(), client->m_avatar->size());
					SendMsg(pClient, GMT_AVATAR_ACK, (LPVOID)ss.str().c_str(), ss.str().size());
				}
				return TRUE;
			}
		}
	}else{
		//target uid had seat down
		auto it = m_tableClients.find(pReq->nTableId);
		if (it == m_tableClients.end())
		{
			return FALSE;
		}
		int nCount = 0;
		PWSCLIENT *clients = it->second->GetClients(&nCount);
		for (int i = 0; i < nCount; i++)
		{
			if (clients[i] && clients[i]->m_userInfo.uid == pReq->uid)
			{
				if (clients[i]->m_avatar)
				{
					std::stringstream ss;
					ss.write((char*)&clients[i]->m_userInfo.uid, sizeof(uint32_t));
					DWORD dwSize = clients[i]->m_avatar->size();
					ss.write((char*)&dwSize, sizeof(DWORD));
					ss.write((char*)clients[i]->m_avatar->data(), dwSize);
					SendMsg(pClient, GMT_AVATAR_ACK, (LPVOID)ss.str().c_str(), ss.str().size());
				}else{
					GAME_AVATAR_ACK ack;
					ack.uid = pReq->uid;
					ack.dwLen = 0;
					SendMsg(pClient, GMT_AVATAR_ACK, &ack, sizeof(GAME_AVATAR_ACK)-1);
				}
				return TRUE;
			}
		}
	}
	SLOGE() << "GetClientAvatar, uid=" << pReq->uid << " not found";
	return FALSE;
	
}

BOOL CWebSocketGame::ClientReady(PWSCLIENT pClient, LPVOID pData, DWORD dwSize){
	if(pClient->m_nTable == -1 || pClient->m_nIndex == -1){
		return FALSE;
	}
    SLOGI() << "ClientReady: table=" << pClient->m_nTable << ", seat=" << pClient->m_nIndex << " uid="<<pClient->m_userInfo.uid;
	if (pClient->m_bReady)
	{
        SLOGW() << "ClientReady, uid = " << pClient->m_userInfo.uid << " is ready already";
        return FALSE;
	}
	pClient->m_bReady = TRUE;
	OnTableChange(pClient->m_nTable);
	return TRUE;
}