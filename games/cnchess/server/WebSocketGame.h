// WebSocketGame.h: WebSocket版本的游戏服务器基类
// 替换原有的CGame类中的socket部分，使用SOUI5的ws模块实现
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <interface/ws-i.h>
#include <helper/obj-ref-impl.hpp>
#include <com-loader.hpp>
#include <map>
#include <list>
#include <vector>
#include <memory>
#include <string>
#include "GameClient.h"
#include "GameTable-i.h"

using namespace SOUI;

// 前向声明
class CWebSocketGame;

// WebSocket服务器监听器
class WebSocketSvrListener : public TObjRefImpl<ISvrListener>
{
public:
	WebSocketSvrListener(CWebSocketGame* pGame);
	virtual ~WebSocketSvrListener();

	// ISvrListener接口实现
	STDMETHODIMP_(BOOL) onConnected(ISvrConnection* pConn, const char* uriPath, const char* uriArgs) override;
	STDMETHODIMP_(void) onConnError(ISvrConnection* pConn, const char* errStr) override;
	STDMETHODIMP_(void) onDisconnect(ISvrConnection* pConn) override;
	STDMETHODIMP_(void) onDataSent(ISvrConnection* pConn, int nMsgId) override;
	STDMETHODIMP_(void) onDataRecv(ISvrConnection* pConn, const void* data, int len, BOOL bBinary) override;

private:
	CWebSocketGame* m_pGame;
};

class CWebSocketGame : ITableListener
{
	friend class WebSocketSvrListener;

public:
	CWebSocketGame();
	virtual ~CWebSocketGame();

	BOOL GameStart( unsigned short uPort);
	void GameStop();  // 添加GameStop方法声明
	unsigned short GetPort() { return m_uPort; }
protected:
	void OnTableChange(int nTableId) override;
protected:
	// 消息发送接口
	BOOL SendMsg(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize);
	//BOOL ClientLogoff(PWSCLIENT pClient, LPVOID pData, DWORD dwSize);
    BOOL ClientLogin(PWSCLIENT pClient, LPVOID pData, DWORD dwSize);
    BOOL ClientSeatDown(PWSCLIENT pClient, LPVOID pData, DWORD dwSize);
	BOOL ClientGetUp(PWSCLIENT pClient, LPVOID pData, DWORD dwSize);
	

    BOOL OnQuerySeat(SeatID *pSeatID);
	BOOL OnMsg(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize);

	// WebSocket相关成员
	SComLoader m_comLoader;
	SAutoRefPtr<IWebsocket> m_pWebsocket;
	SAutoRefPtr<IWsServer> m_pWsServer;
	SAutoRefPtr<WebSocketSvrListener> m_pListener;

    std::list<PWSCLIENT> m_tmpClients;

	int m_nMaxTable;                                      // 最大桌子数
	std::map<int, SAutoRefPtr<IGameTable> > m_tableClients; // 按桌号索引的客户端列表
	uint32_t m_nextUid;
  private:
	unsigned short m_uPort;
	// 内部方法
	void ProcessReceivedData(ISvrConnection* pConn, const void* data, int len);
	PWSCLIENT CreateClient(ISvrConnection* pConn,LPCSTR pszUriPath, LPCSTR pszArgs);
	void RemoveClient(ISvrConnection* pConn);
	void sendRoomInfo(PWSCLIENT pClient);
    BOOL ClientAvatar(PWSCLIENT pClient, LPVOID pData, DWORD dwSize);
    BOOL ClientReady(PWSCLIENT pClient, LPVOID pData, DWORD dwSize);
    void notifyRoomInfoChanged();
};

// 为了保持兼容性，定义类型别名
typedef WSCLIENT CLIENT;
typedef PWSCLIENT PCLIENT;