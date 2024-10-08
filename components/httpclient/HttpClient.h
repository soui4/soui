#pragma once
#include <interface/SHttpClient-i.h>
#include <WinInet.h>
#include <helper/obj-ref-impl.hpp>
#include <string>
#include "httpHeader.h"

SNSBEGIN

class CWinHttp
	: public TObjRefImpl<IHttpClient>
{
public:
	CWinHttp(void);
	virtual ~CWinHttp(void);
	
	STDMETHOD_(void,SetHeader)(THIS_ LPCSTR pszKey,LPCSTR pszValue) OVERRIDE;
	STDMETHOD_(void,SetTimeOut)(THIS_ int dwConnectTime,  int dwSendTime, int dwRecvTime) OVERRIDE;
	STDMETHOD_(BOOL,Request)(THIS_ IStringA *out,LPCSTR lpUrl, RequestType type, LPCSTR lpPostData = NULL, LPCSTR lpHeader=NULL) OVERRIDE;
	STDMETHOD_(BOOL,Request2)(THIS_ IStringA *out,LPCSTR lpUrl, RequestType type, LPVOID lpPostData, DWORD dwDataLen, LPCSTR lpHeader=NULL) OVERRIDE;
	STDMETHOD_(HttpError, GetErrorCode)(CTHIS) SCONST OVERRIDE{ return m_error;	}
	STDMETHOD_(void,SetDownloadCallback)(THIS_ IHttpCallback* pCallback)  OVERRIDE{ m_pCallback = pCallback; }
	STDMETHOD_(BOOL,DownloadFile)(THIS_ LPCSTR lpUrl, LPCSTR lpFilePath)  OVERRIDE;
	STDMETHOD_(BOOL,DownloadToMem)(THIS_ LPCSTR lpUrl, OUT void** ppBuffer, OUT int* nSize) OVERRIDE;
	STDMETHOD_(int,GetHttpCode)(CTHIS) SCONST  OVERRIDE{return m_httpCode;}
protected:
	BOOL	Init();
	void	Close();
	//init
	BOOL	InitConnect(LPCSTR lpUrl, RequestType type, LPCSTR lpHeader=NULL, LPVOID lpPostData=NULL,DWORD dwDataLen=0);
	BOOL	ConnectHttpServer(LPCSTR lpIP, WORD wPort);
	BOOL	CreateHttpRequest(LPCSTR lpPage, RequestType type, DWORD dwFlag=0);
	BOOL	SendHttpRequest(LPCSTR lpHeader,LPVOID lpPostData, DWORD dataLen);
	//query 
	BOOL	QueryRawHeaders(OUT std::wstring& strHeaders);
	BOOL	QueryContentLength(OUT DWORD& dwLength);
	int QueryStatusCode();
private:

	HttpError	m_error;
	IHttpCallback*	m_pCallback;

	HINTERNET	m_hInternet;
	HINTERNET	m_hConnect;
	HINTERNET	m_hRequest;
	int			m_nConnTimeout;
	int			m_nSendTimeout;
	int			m_nRecvTimeout;
	int			m_httpCode;
	CHttpHeader m_header;
};

namespace HttpClient
{
	SOUI_COM_C BOOL SOUI_COM_API SCreateInstance(IObjRef **ppTaskLoop);
}
SNSEND
EXTERN_C BOOL SOUI_COM_API HttpClient_SCreateInstance(IObjRef **ppTaskLoop);
