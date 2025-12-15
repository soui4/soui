#include "stdafx.h"
#include "WebSocketClient.h"
#include <tchar.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////
// WebSocketConnListener Implementation
//////////////////////////////////////////////////////////////////////

WebSocketConnListener::WebSocketConnListener(WebSocketClient* pClient)
	: m_pClient(pClient)
{
}

WebSocketConnListener::~WebSocketConnListener()
{
}

void WebSocketConnListener::onConnected()
{
	if (m_pClient)
	{
		m_pClient->m_bConnected = TRUE;
		// 通知连接成功
		std::shared_ptr<std::vector<BYTE> > data(new std::vector<BYTE>(sizeof(WORD)));
		WORD wCode = 0;
		memcpy(&(*data)[0], &wCode, sizeof(WORD));
		m_pClient->NotifyMessage(GMT_CONNECT, data);
	}
}

void WebSocketConnListener::onConnError(const char *errStr)
{
	if (m_pClient)
	{
		m_pClient->m_bConnected = FALSE;
		// 通知连接错误
		std::shared_ptr<std::vector<BYTE> > data(new std::vector<BYTE>(sizeof(WORD)));
		WORD wCode = 1; // 错误代码
		memcpy(&(*data)[0], &wCode, sizeof(WORD));
		m_pClient->NotifyMessage(GMT_CONNECT, data);
	}
}

void WebSocketConnListener::onDisconnect()
{
	if (m_pClient)
	{
		m_pClient->m_bConnected = FALSE;
		// 通知连接断开
		std::shared_ptr<std::vector<BYTE> > data;
		m_pClient->NotifyMessage(GMT_SOCKCLOSE, data);
	}
}

void WebSocketConnListener::onDataSent(int nMsgId)
{
	// 数据发送完成，暂时不需要特殊处理
}

void WebSocketConnListener::onDataRecv(const void *data, int len, BOOL bBinary)
{
	if (m_pClient && data && len > 0)
	{
		m_pClient->ProcessReceivedData(data, len);
	}
}

//////////////////////////////////////////////////////////////////////
// WebSocketClient Implementation
//////////////////////////////////////////////////////////////////////

WebSocketClient::WebSocketClient()
    : m_pMsgListener(NULL)
{
	m_bConnected = FALSE;
	m_wPort = 0;
	memset(m_szServerIP, 0, sizeof(m_szServerIP));
}

WebSocketClient::~WebSocketClient()
{
	Disconnect();
}

BOOL WebSocketClient::ConnectToServer(LPCSTR pszSvr, LPCSTR args)
{
	if (m_pWsClient)
		return FALSE; // 已经初始化

	// 解析服务器地址，格式：ip:port
	const char* pColon = strchr(pszSvr, ':');
	if (!pColon)
		return FALSE;

	int ipLen = pColon - pszSvr;
	if (ipLen >= sizeof(m_szServerIP))
		return FALSE;

	strncpy(m_szServerIP, pszSvr, ipLen);
	m_szServerIP[ipLen] = '\0';
	m_wPort = (WORD)atoi(pColon + 1);

	// 创建WebSocket组件
	HRESULT hr = m_comLoader.CreateInstance(_T("ws"), (IObjRef**)&m_pWebsocket);
	if (FAILED(hr) || !m_pWebsocket)
		return FALSE;

	// 创建连接监听器
	m_pListener = new WebSocketConnListener(this);
	if (!m_pListener)
		return FALSE;
	// 创建WebSocket客户端
	m_pWsClient = m_pWebsocket->CreateWsClient(m_pListener);
	if (!m_pWsClient)
		return FALSE;

	// 连接到服务器
	ClientOption option = { FALSE, NULL, TRUE, TRUE, TRUE }; // 非安全连接
	char szPath[256];
	snprintf(szPath, sizeof(szPath), "/upgrade/?%s", args);

	int nRet = m_pWsClient->connectTo(m_szServerIP, szPath, m_wPort, "", option);
	if (nRet != 0)
	{
		Disconnect();
		return FALSE;
	}

	return TRUE;
}

void WebSocketClient::Disconnect()
{
	// 关闭WebSocket连接
	if (m_pWsClient)
	{
		m_pWsClient->disconnect();
		m_pWsClient = NULL;
	}

	m_pWebsocket = NULL;
	m_pListener = NULL;
	m_bConnected = FALSE;
}

BOOL WebSocketClient::SendMsg(DWORD dwType, LPVOID lpData, DWORD dwSize)
{
	if (!m_pWsClient || !m_bConnected)
		return FALSE;

	if (dwSize > MAX_MSGSIZE)
		return FALSE;

	// 构造消息包：dwType + dwSize + data
	DWORD dwTotalSize = 8 + dwSize;
	char* pBuffer = (char*)malloc(dwTotalSize);
	if (!pBuffer)
		return FALSE;

	memcpy(pBuffer, &dwType, 4);
	memcpy(pBuffer + 4, &dwSize, 4);
	if (dwSize > 0 && lpData)
		memcpy(pBuffer + 8, lpData, dwSize);

	// 发送二进制数据
	int nRet = m_pWsClient->sendBinary(pBuffer, dwTotalSize);
	free(pBuffer);

	return (nRet == 0);
}

void WebSocketClient::ProcessReceivedData(const void* data, int len)
{
	if (!data || len < 8) // 至少需要8字节的消息头
		return;

	const BYTE* pData = (const BYTE*)data;
	DWORD dwType, dwSize;
	
	memcpy(&dwType, pData, 4);
	memcpy(&dwSize, pData + 4, 4);
	if (dwSize > MAX_MSGSIZE || (int)(dwSize + 8) != len)
		return; // 数据格式错误
	pData += 8;
	std::shared_ptr<std::vector<BYTE> > data2;
    if (dwSize > 0)
    {
		data2 = std::shared_ptr<std::vector<BYTE> >(new std::vector<BYTE>(pData, pData  + dwSize));
	}
	NotifyMessage(dwType, data2);
}

void WebSocketClient::NotifyMessage(DWORD dwType, std::shared_ptr<std::vector<BYTE> > data)
{
	if(m_pMsgListener)
		m_pMsgListener->OnMessage(dwType, data);
}

void WebSocketClient::SetMessageHandler(IListener *pMsgListener)
{
	m_pMsgListener = pMsgListener;
}