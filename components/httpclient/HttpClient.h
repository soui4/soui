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
	
	STDMETHOD_(void,SetHeader)(LPCSTR pszKey,LPCSTR pszValue);
	STDMETHOD_(void,SetTimeOut)(int dwConnectTime,  int dwSendTime, int dwRecvTime);
	STDMETHOD_(BOOL,Request)(IStringA *out,LPCSTR lpUrl, HttpRequest type, LPCSTR lpPostData = NULL, LPCSTR lpHeader=NULL);
	STDMETHOD_(HttpError, GetErrorCode)() const { return m_error;	}
	STDMETHOD_(void,SetDownloadCallback)(IHttpCallback* pCallback, void* pParam)		{ m_pCallback = pCallback; m_lpParam = pParam; }
	STDMETHOD_(BOOL,DownloadFile)(LPCSTR lpUrl, LPCSTR lpFilePath);
	STDMETHOD_(BOOL,DownloadToMem)(LPCSTR lpUrl, OUT void** ppBuffer, OUT int* nSize);
	STDMETHOD_(int,GetHttpCode)(CTHIS)SCONST {return m_httpCode;}
protected:
	BOOL	Init();
	void	Close();
	//init
	BOOL	InitConnect(LPCSTR lpUrl, HttpRequest type, LPCSTR lpPostData=NULL, LPCSTR lpHeader=NULL);
	BOOL	ConnectHttpServer(LPCSTR lpIP, WORD wPort);
	BOOL	CreateHttpRequest(LPCSTR lpPage, HttpRequest type, DWORD dwFlag=0);
	BOOL	SendHttpRequest(LPCSTR lpPostData=NULL, LPCSTR lpHeader=NULL);
	//query 
	BOOL	QueryRawHeaders(OUT std::wstring& strHeaders);
	BOOL	QueryContentLength(OUT DWORD& dwLength);
	int QueryStatusCode();
private:

	void*	m_lpParam;
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
