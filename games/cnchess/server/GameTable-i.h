#ifndef __GAMETABLE_I_H__
#define __GAMETABLE_I_H__

#include <windows.h>
#include <interface/obj-ref-i.h>
#include <protocol.h>
typedef class GameClient WSCLIENT, *PWSCLIENT;

struct ITableListener
{
    virtual void OnTableChange(int nTableId) = 0;
};

struct IGameTable : IObjRef
{
    virtual int GetID() const = 0;
    virtual void Boardcast(DWORD dwType, LPVOID pData, DWORD dwSize) = 0;

    virtual BOOL SendMsg(PWSCLIENT pClient, DWORD dwType,LPVOID pData,DWORD dwSize) =0;

    virtual BOOL OnMsg(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)=0;
	virtual void OnRemovePlayer(int seatId) = 0;
    virtual void OnAddPlayer(int seatId, PWSCLIENT pClient) = 0;
    virtual BOOL OnPlayerLeave(int seatId, PWSCLIENT pClient) = 0; // return true if player leave success, otherwise return false.
    virtual int GetState() const = 0;
    virtual PWSCLIENT * GetClients(int *nCount) = 0;
    virtual PWSCLIENT GetPlayer(int seatId) const = 0;
    virtual int GetPlayerCount() const = 0;
    virtual BOOL HasPlayer(int seatId) const = 0;
    virtual int GetEmptySeat() const = 0;
    virtual void OnGameStart() = 0; // called when game start.
    virtual void OnGameEnd() = 0;
    virtual void OnRoundEnd() = 0; // called when round end.
    virtual void NotifyClient(PWSCLIENT pClient, LPCWSTR pszMsg) = 0;
    virtual void EndGame(int type, LPCWSTR pszDesc) = 0;
};

#endif // !__GAMETABLE_I_H__