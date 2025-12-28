#include "stdafx.h"
#include "Game.h"
#include "GameClient.h"
#include <algorithm>
#include <helper/slog.h>
#include <string/strcpcvt.h>
#define kLogTag "CGameTable"

SNSBEGIN

CGameTable::CGameTable(ITableListener* pListener, int nTableId, int nClients)
{
    m_pListener = pListener;
    m_nTableId = nTableId;
    m_clients.assign(nClients, nullptr);
    m_state = TABLE_STATE_WAIT;
}

CGameTable::~CGameTable()
{
    for (auto &client : m_clients)
    {
        if (client != nullptr)
        {
            delete client;
            client = nullptr;
        }
    }
}

void CGameTable::Boardcast(DWORD dwType, LPVOID pData, DWORD dwSize)
{
	for (auto &client : m_clients)
	{
        if(client == nullptr){
            continue;
        }
        if (!client->m_pConn)
		{
			continue;
		}
		client->m_pConn->sendBinary2(dwType, pData, dwSize);
	}
}

BOOL CGameTable::SendMsg(PWSCLIENT pClient, DWORD dwType,LPVOID pData,DWORD dwSize)
{
    if(!pClient)
    {
        Boardcast(dwType, pData, dwSize);
        return TRUE;
    }
    if (!pClient->m_pConn)
    {
        return FALSE;
    }
    pClient->m_pConn->sendBinary2(dwType, pData, dwSize);
    return TRUE;
}

PWSCLIENT * CGameTable::GetClients(int *nCount)
{
    *nCount = m_clients.size();
    return m_clients.data();
}

PWSCLIENT CGameTable::GetPlayer(int seatId) const
{
    if(seatId >= 0 && seatId < m_clients.size()){
        return m_clients[seatId];
    }
    return nullptr;
}
int CGameTable::GetState() const
{
    return m_state;
}

int CGameTable::GetPlayerCount() const
{
    return std::count_if(m_clients.begin(), m_clients.end(), [](PWSCLIENT pClient){return pClient != NULL;});
}

void CGameTable::OnRemovePlayer(int seatId)
{
    SLOGI()<<"remove player, seat id:"<<seatId;
    if(seatId >= 0 && seatId < m_clients.size()){
        PWSCLIENT pClient = m_clients[seatId];
        if(!pClient){
            SLOGW()<<"player is null";
            return;
        }
        OnPlayerLeave(seatId, pClient);
        m_clients[seatId] = nullptr;
        delete pClient;
    }
}

void CGameTable::OnAddPlayer(int seatId, PWSCLIENT pClient)
{
    SLOGI()<<"add player, seat id:"<<seatId;
    if(seatId >= 0 && seatId < m_clients.size() && m_clients[seatId] == NULL){
        m_clients[seatId] = pClient;
    }
}

BOOL CGameTable::OnPlayerLeave(int seatId, PWSCLIENT pClient)
{
    SLOGI()<<"player leave, seat id:"<<seatId;
    if(seatId >= 0 && seatId < m_clients.size() && m_clients[seatId] == pClient){
        m_clients[seatId] = nullptr;
        OnGameEnd();
        return TRUE;
    }
    return FALSE;
}

BOOL CGameTable::HasPlayer(int seatId) const{
    if(seatId >= 0 && seatId < m_clients.size()){
        return m_clients[seatId] != nullptr;
    }
    return false;
}

int CGameTable::GetEmptySeat() const{
    for(int i=0;i<m_clients.size();i++){
        if(m_clients[i] == nullptr){
            return i;
        }
    }
    return -1;
}

void CGameTable::OnGameStart()
{
    m_state = TABLE_STATE_PLAYING;
}

void CGameTable::OnGameEnd()
{
    m_state = TABLE_STATE_WAIT;
    for (auto &client : m_clients)
    {
        if (client != nullptr)
        {
            client->m_bReady = FALSE;
        }
    }
}

void CGameTable::OnRoundEnd()
{
    m_state = TABLE_STATE_WAIT;
    for(auto &client : m_clients)
    {
        if(client != nullptr)
        {
            client->m_bReady = FALSE;
        }
    }
}

void CGameTable::NotifyClient(PWSCLIENT pClient, LPCWSTR pszMsg)
{
    SOUI::SStringA strMsg = S_CW2A(pszMsg, CP_UTF8);
    SendMsg(pClient, GMT_GAME_MSG, (LPVOID)strMsg.c_str(), strMsg.GetLength());
}

void CGameTable::EndGame(int type, LPCWSTR pszDesc)
{
    OnGameEnd();
    m_pListener->OnTableChange(GetID());
}

SNSEND