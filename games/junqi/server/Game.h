#ifndef __GAME_H__
#define __GAME_H__

#include "GameTable-i.h"
#include <helper/obj-ref-impl.hpp>
#include <vector>

SNSBEGIN

class CGameTable : public TObjRefImpl<IGameTable>
{
public:
    CGameTable(ITableListener* pListener, int nTableId, int nClients);
    virtual ~CGameTable();

    int GetID() const override{
        return m_nTableId;
    }
    void Boardcast(DWORD dwType, LPVOID pData, DWORD dwSize) override;

    BOOL SendMsg(PWSCLIENT pClient, DWORD dwType,LPVOID pData,DWORD dwSize) override;

    PWSCLIENT * GetClients(int *nCount) override;
    PWSCLIENT GetPlayer(int seatId) const override;
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
    void NotifyClient(PWSCLIENT pClient, LPCWSTR pszMsg) override; // 通知消息给客户端。pszMsg为消息内容。
    void EndGame(int type, LPCWSTR pszDesc) override; // 结束游戏。type为结束类型，pszDesc为结束描述。
protected:
    std::vector<PWSCLIENT> m_clients;
    TableState m_state;
    ITableListener* m_pListener; // 监听器。用于监听游戏状态变化。
    int m_nTableId;
};

SNSEND

#endif // !__GAME_H__
