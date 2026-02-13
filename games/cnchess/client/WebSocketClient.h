#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <interface/ws-i.h>
#include <helper/obj-ref-impl.hpp>
#include <commgr2.h>
#include <vector>
#include <memory>

#define MAX_MSGSIZE		(1<<18)	//256K

// 前向声明
class WebSocketClient;

// WebSocket连接监听器
class WebSocketConnListener : public TObjRefImpl<IConnListener>
{
public:
	WebSocketConnListener(WebSocketClient* pClient);
	virtual ~WebSocketConnListener();

	// IConnListener接口实现
	STDMETHODIMP_(void) onConnected() override;
	STDMETHODIMP_(void) onConnError(const char *errStr) override;
	STDMETHODIMP_(void) onDisconnect() override;
	STDMETHODIMP_(void) onDataSent(int nMsgId) override;
	STDMETHODIMP_(void) onDataRecv(const void *data, int len, BOOL bBinary) override;

private:
	WebSocketClient* m_pClient;
};

// WebSocket客户端
class WebSocketClient  
{
	friend class WebSocketConnListener;
public:
	struct IListener
	{
		virtual BOOL OnMessage(DWORD dwType, std::shared_ptr<std::vector<BYTE> > data) = 0;
	};
public:

	WebSocketClient();
	virtual ~WebSocketClient();

	// 连接和断开连接
	BOOL ConnectToServer(LPCSTR pszSvr, LPCSTR args);
	void Disconnect();

	// 消息发送
	BOOL SendMsg(DWORD dwType, LPVOID lpData, DWORD dwSize);

	// 消息处理回调设置
	void SetMessageHandler(IListener *pMsgListener);

private:
	// WebSocket相关成员
	SComMgr2 m_comLoader;
	SAutoRefPtr<IWebsocket> m_pWebsocket;
	SAutoRefPtr<IWsClient> m_pWsClient;
	SAutoRefPtr<WebSocketConnListener> m_pListener;

	// 消息处理相关
	IListener *m_pMsgListener;
	BOOL m_bConnected;

	// 连接参数
	char m_szServerIP[64];
	WORD m_wPort;

	// 内部方法
	void ProcessReceivedData(const void* data, int len);
	void NotifyMessage(DWORD dwType, std::shared_ptr<std::vector<BYTE> > data);
};

#endif//WEBSOCKETCLIENT_H