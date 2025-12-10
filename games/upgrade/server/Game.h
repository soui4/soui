#ifndef __GAME_H__
#define __GAME_H__

#include "GameTable-i.h"
#include <helper/obj-ref-impl.hpp>
#include <vector>

typedef class GameClient WSCLIENT, *PWSCLIENT;

SNSBEGIN

class CGameTable : public TObjRefImpl<IGameTable>
{
public:
    CGameTable(int nClients);
    virtual ~CGameTable();

    void Boardcast(DWORD dwType, LPVOID pData, DWORD dwSize) override;

    BOOL SendMsg(PWSCLIENT pClient, DWORD dwType,LPVOID pData,DWORD dwSize) override;

    PWSCLIENT * GetClients(int *nCount) override;

    int GetState() const override;

    int GetPlayerCount() const override;

    void OnRemovePlayer(int seatId) override;
    void OnAddPlayer(int seatId, PWSCLIENT pClient) override;
    BOOL OnPlayerLeave(int seatId, PWSCLIENT pClient) override;
    
    BOOL HasPlayer(int seatId) const override;
    int GetEmptySeat() const override; // return -1 if no empty seat. otherwise return seat id.
    void OnGameStart() override;
    void OnGameEnd() override;
    void OnRoundEnd() override;
protected:
    std::vector<PWSCLIENT> m_clients;
    TableState m_state;
};

SNSEND

#endif // !__GAME_H__
