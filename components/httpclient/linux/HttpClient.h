#pragma once
#include <interface/SHttpClient-i.h>
#include <helper/obj-ref-impl.hpp>
#include <string>
#include <curl/curl.h>
#include <iostream>
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
	STDMETHOD_(int,GetHttpCode)(CTHIS) SCONST  OVERRIDE{return m_httpCode;}

protected:
	BOOL	Init();
	void	Close();

	static size_t write_data_file(void* ptr, size_t size, size_t nmemb, void* user_data);
	static size_t write_data_string(void* ptr, size_t size, size_t nmemb, void* user_data);
	static int progress_dl_callback(void* user_data, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

private:

	HttpError	m_error;
	IHttpCallback*	m_pCallback;

	int			m_nConnTimeout;
	int			m_nSendTimeout;
	int			m_nRecvTimeout;
	long 			m_httpCode;
	CHttpHeader m_header;
	DownloadState   m_st;
	CURL* curl;
    CURLcode res;
};

namespace HttpClient
{
	SOUI_COM_C BOOL SOUI_COM_API SCreateInstance(IObjRef **ppSObj);
}
SNSEND

EXTERN_C BOOL SOUI_COM_API HttpClient_SCreateInstance(IObjRef **ppSObj);
