// WebSocketGame.h: WebSocket版本的游戏服务器基类
// 替换原有的CGame类中的socket部分，使用SOUI5的ws模块实现
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <interface/ws-i.h>
#include <helper/obj-ref-impl.hpp>
#include <map>
#include <list>
#include <vector>
#include <memory>
#include <string>
#include <atomic>
#include "GameClient.h"
#include "GameTable-i.h"
#include <commgr2.h>
using namespace SOUI;

// 前向声明
class CWebSocketGame;

// WebSocket服务器监听器
// ISvrListener: 连接/收发事件; 
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

class CWebSocketGame : ITableListener, ITimerListener
{
	friend class WebSocketSvrListener;

public:
	CWebSocketGame();
	virtual ~CWebSocketGame();

	BOOL GameStart( unsigned short uPort);
	void GameStop();  // 添加GameStop方法声明
	unsigned short GetPort() { return m_uPort; }

	// 定时器ID: 在 ws 的定时器发生器上按此ID登记, ITimerListener::onTimer 也按此ID分派
	enum
	{
		TIMER_ONLINE_BROADCAST = 1, // 周期广播在线人数
	};
protected:
	void OnTableChange(int nTableId) override;
protected:
	// ITimerListener接口实现: 按ID分派定时器响应
	STDMETHODIMP_(void) onTimer(UINT_PTR uTimerID) override;
protected:
	// 消息发送接口
	BOOL SendMsg(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize);
	//BOOL ClientLogoff(PWSCLIENT pClient, LPVOID pData, DWORD dwSize);
    BOOL ClientLogin(PWSCLIENT pClient, LPVOID pData, DWORD dwSize);
    BOOL ClientSeatDown(PWSCLIENT pClient, LPVOID pData, DWORD dwSize);
	BOOL ClientGetUp(PWSCLIENT pClient, LPVOID pData, DWORD dwSize);
	BOOL ClientThemeReq(PWSCLIENT pClient, LPVOID pData, DWORD dwSize);
	BOOL ClientRobotInvite(PWSCLIENT pClient, LPVOID pData, DWORD dwSize);
    // 残局打谱相关
    BOOL ClientEndgameList(PWSCLIENT pClient, LPVOID pData, DWORD dwSize);

    BOOL OnQuerySeat(SeatID *pSeatID);
	BOOL OnMsg(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize);

	// 在线人数: 统计所有已登录且连接存活的客户端(不含机器人)
	int GetOnlineCount();
	// 向单个客户端下发在线人数(登录时调用)
	void sendOnlineCount(PWSCLIENT pClient);
	// 向所有在线客户端广播在线人数(由服务级定时器周期性触发)
	void broadcastOnlineCount();

	// WebSocket相关成员
	SComMgr2 m_comLoader;
	SAutoRefPtr<IWebsocket> m_pWebsocket;
	SAutoRefPtr<IWsServer> m_pWsServer;
	SAutoRefPtr<ITimerGenerator> m_pTimerGenerator;
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