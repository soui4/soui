#include <Windows.h>
#include "HttpClient.h"
#include "Winhttp.h"
#include <assert.h>
#include <string/sstringa.h>
#include <string/sstringw.h>
#include <string/strcpcvt.h>

using namespace std;

#pragma comment(lib, "Winhttp")

#define HTTP_READBUF_LEN	1024*1024		//1M的接收缓冲区

SNSBEGIN

inline void CloseInternetHandle(HINTERNET* hInternet)
{
	if ( *hInternet )
	{
		WinHttpCloseHandle(*hInternet);
		*hInternet = NULL;
	}
}


CWinHttp::CWinHttp(void)
	:m_hInternet(NULL)
	,m_hConnect(NULL)
	,m_hRequest(NULL)
	,m_nConnTimeout(5000)
	,m_nSendTimeout(5000)
	,m_nRecvTimeout(5000)
	, m_error(Hir_Success)
	, m_pCallback(NULL)
{
	Init();
}

CWinHttp::~CWinHttp(void)
{
	Close();
}

BOOL CWinHttp::Init()
{
	m_hInternet = ::WinHttpOpen(
		L"Microsoft Internet Explorer",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0);
	if ( NULL == m_hInternet )
	{
		m_error = Hir_InitErr;
		return false;
	}
	::WinHttpSetTimeouts(m_hInternet, 0, m_nConnTimeout, m_nSendTimeout, m_nRecvTimeout);

	return true;
}

void CWinHttp::Close()
{
	CloseInternetHandle(&m_hRequest);
	CloseInternetHandle(&m_hConnect);
	CloseInternetHandle(&m_hInternet);
}

BOOL CWinHttp::ConnectHttpServer(LPCSTR lpIP, WORD wPort)
{
	SStringW strIp=S_CA2W(lpIP,CP_UTF8);
	m_hConnect = ::WinHttpConnect(m_hInternet, strIp.c_str(), wPort, 0);
	return m_hConnect != NULL;
}

BOOL CWinHttp::CreateHttpRequest(LPCSTR lpPage, RequestType type, DWORD dwFlag/*=0*/)
{
	LPCWSTR pVerb = (type == Hr_Get)?L"GET":L"POST";
	SStringW strUri = S_CA2W(lpPage,CP_UTF8);
	m_hRequest = ::WinHttpOpenRequest(
		m_hConnect,
		pVerb,
		strUri.c_str(),
		NULL,
		WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES, 
		dwFlag);
	return m_hRequest!=NULL;
}

void CWinHttp::SetTimeOut( int dwConnectTime, int dwSendTime, int dwRecvTime )
{
	m_nConnTimeout = dwConnectTime;
	m_nSendTimeout = dwSendTime;
	m_nRecvTimeout = dwRecvTime;
}

BOOL CWinHttp::DownloadFile( LPCSTR lpUrl, LPCSTR lpFilePath )
{
	Close();
	if ( !Init() )
		return false;
	BOOL bRet = false;
	DWORD dwBytesToRead = 0, dwFileSize = 0, dwReadSize=0, dwRecvSize =0;
	if ( !InitConnect(lpUrl, Hr_Get) )
		return false;
	m_httpCode = QueryStatusCode();
	if (m_httpCode == HTTP_STATUS_NOT_FOUND) {
		m_error = Hir_404;
		return FALSE;
	}

	if ( !QueryContentLength(dwFileSize) )
	{
		m_error = Hir_QueryErr;
		return false; 
	}
	if( m_pCallback )
	{
		BOOL bRet = m_pCallback->OnDownloadCallback(DS_Start, dwFileSize, 0);
		if(!bRet){
			m_error = Hir_UserCancel;
			return false;
		}
	}
	wstring strHeaders;
	BOOL bQuery = QueryRawHeaders(strHeaders);
	if ( bQuery && (strHeaders.find(L"404")!=wstring::npos) )
	{
		m_error = Hir_404;
		return false;
	}
	SStringT strPath = S_CA2T(lpFilePath,CP_UTF8);
	HANDLE hFile = CreateFile(strPath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( INVALID_HANDLE_VALUE == hFile )
	{
		m_error = Hir_CreateFileErr;
		return false;
	}
	SetFilePointer(hFile, dwFileSize, 0, FILE_BEGIN);
	SetEndOfFile(hFile);
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	if ( !::WinHttpQueryDataAvailable(m_hRequest, &dwBytesToRead) )
	{
		CloseHandle(hFile);
		DeleteFile(strPath.c_str());
		return false;
	}
	void* lpBuff = malloc(HTTP_READBUF_LEN);
	BOOL bCancel = FALSE;
	while( !bCancel )
	{
		if ( dwBytesToRead>HTTP_READBUF_LEN )
		{
			free(lpBuff);
			lpBuff = malloc(dwBytesToRead);
		}
		if ( !::WinHttpReadData(m_hRequest, lpBuff, dwBytesToRead, &dwReadSize) )
			break;
		DWORD dwWriteByte;
		if ( !WriteFile(hFile, lpBuff, dwReadSize, &dwWriteByte, NULL) || (dwReadSize != dwWriteByte) )
			break;
		dwRecvSize += dwReadSize;
		if( m_pCallback )
		{
			BOOL bRet = m_pCallback->OnDownloadCallback(DS_Loading, dwFileSize, dwRecvSize);
			if(!bRet){
				m_error = Hir_UserCancel;
				bCancel = TRUE;
				break;
			}
		}
		if ( !::WinHttpQueryDataAvailable(m_hRequest, &dwBytesToRead) )
			break;
		if ( dwBytesToRead<= 0 )
		{
			bRet = true;
			break;
		}
	}
	free(lpBuff);
	CloseHandle(hFile);
	if ( !bRet )
	{//下载失败，删除文件
		DeleteFile(strPath.c_str());
	}
	if(m_pCallback){
		m_pCallback->OnDownloadCallback(bRet?DS_Finished:DS_Failed, 0, 0);
	}

	return bRet;
}

BOOL CWinHttp::DownloadToMem(LPCSTR lpUrl, OUT void** ppBuffer, OUT int* nSize)
{
	BOOL bResult = false;
	BYTE* lpFileMem = NULL;
	void* lpBuff = NULL;
	DWORD dwLength = 0, dwBytesToRead = 0,  dwReadSize = 0, dwRecvSize = 0;
	try
	{
		if ( !InitConnect(lpUrl, Hr_Get) )
			throw Hir_InitErr;
		if ( !QueryContentLength(dwLength) )
			throw Hir_QueryErr;
		wstring strHeaders;
		BOOL bQuery = QueryRawHeaders(strHeaders);
		if ( bQuery && (strHeaders.find(L"404")!=wstring::npos) )
			throw Hir_404;
		if ( !::WinHttpQueryDataAvailable(m_hRequest, &dwBytesToRead) )
			throw Hir_QueryErr;
		lpFileMem = (BYTE*)malloc(dwLength);
		if(!lpFileMem)
			throw Hir_BufferErr;
		lpBuff = malloc(HTTP_READBUF_LEN);
		while( true )
		{
			if ( dwBytesToRead>HTTP_READBUF_LEN )
			{
				free(lpBuff);
				lpBuff = malloc(dwBytesToRead);
			}
			if ( !::WinHttpReadData(m_hRequest, lpBuff, dwBytesToRead, &dwReadSize) )
				throw Hir_DownloadErr;
			memcpy(lpFileMem+dwRecvSize, lpBuff, dwReadSize);
			dwRecvSize += dwReadSize;
			if ( !::WinHttpQueryDataAvailable(m_hRequest, &dwBytesToRead) )
				throw Hir_DownloadErr;
			if ( dwBytesToRead<= 0 )
			{
				bResult = true;
				break;
			}
		}
	}
	catch( HttpError error )
	{
		m_error = error;
	}
	if ( lpBuff )
		free(lpBuff);
	if ( bResult )
	{
		*ppBuffer = lpFileMem;
		*nSize = dwRecvSize;
	}
	else if(lpFileMem)
	{
		free(lpFileMem);
	}
	return bResult;
}

int CWinHttp::QueryStatusCode()
{
	int http_code = 0;
	wchar_t szBuffer[24] = { 0 };
	DWORD dwSize = 24 * sizeof(wchar_t);
	if (::WinHttpQueryHeaders(m_hRequest, WINHTTP_QUERY_STATUS_CODE, WINHTTP_HEADER_NAME_BY_INDEX, szBuffer, &dwSize, WINHTTP_NO_HEADER_INDEX)) {
		wchar_t *p = NULL;
		http_code = wcstoul(szBuffer, &p, 10);
	}
	return http_code;
}
BOOL CWinHttp::Request(IStringA *out, LPCSTR lpUrl, RequestType type, LPCSTR lpPostData /*= NULL*/, LPCSTR lpHeader/*=NULL*/ )
{
	DWORD dwDataLen = lpPostData==NULL?0:(DWORD)strlen(lpPostData);
	return Request2(out,lpUrl,type,(LPVOID)lpPostData,dwDataLen,lpHeader);
}

BOOL CWinHttp::Request2(IStringA *out, LPCSTR lpUrl, RequestType type, LPVOID lpPostData, DWORD dwDataLen, LPCSTR lpHeader/*=NULL*/ )
{
	if ( !InitConnect(lpUrl, type, lpHeader,lpPostData,dwDataLen) )
		return FALSE;
	m_httpCode = QueryStatusCode();
	if (m_httpCode == HTTP_STATUS_NOT_FOUND) {
		m_error = Hir_404;
		return FALSE;
	}
	string strRet;
	DWORD dwBytesToRead, dwReadSize;
	void* lpBuff = malloc(HTTP_READBUF_LEN);
	BOOL bFinish = false;
	while ( true )
	{
		if ( !::WinHttpQueryDataAvailable(m_hRequest, &dwBytesToRead) )
			break;
		if ( dwBytesToRead<= 0 )
		{
			bFinish = true;
			break;
		}
		if ( dwBytesToRead>HTTP_READBUF_LEN )
		{
			free(lpBuff);
			lpBuff = malloc(dwBytesToRead);
		}
		if ( !::WinHttpReadData(m_hRequest, lpBuff, dwBytesToRead, &dwReadSize) )
			break;
		strRet.append((const char*)lpBuff, dwReadSize);
	}
	free(lpBuff);
	if ( !bFinish )
		strRet.clear();
	out->Assign2(strRet.c_str(),(int)strRet.length());
	return !strRet.empty();
}

BOOL CWinHttp::QueryRawHeaders( OUT std::wstring& strHeaders )
{
	BOOL bRet = false;
	DWORD dwSize;
	BOOL bResult = ::WinHttpQueryHeaders(m_hRequest, WINHTTP_QUERY_RAW_HEADERS, WINHTTP_HEADER_NAME_BY_INDEX, NULL, &dwSize, WINHTTP_NO_HEADER_INDEX);
	if ( ERROR_INSUFFICIENT_BUFFER == GetLastError() )
	{
		wchar_t* lpData = (wchar_t*)malloc(dwSize);
		bResult = ::WinHttpQueryHeaders(m_hRequest, WINHTTP_QUERY_RAW_HEADERS, WINHTTP_HEADER_NAME_BY_INDEX, lpData, &dwSize, WINHTTP_NO_HEADER_INDEX);
		if ( bResult )
		{
			strHeaders = lpData;
			bRet = true;
		}
		free(lpData);
	}
	return bRet;
}

BOOL CWinHttp::QueryContentLength( OUT DWORD& dwLength )
{
	BOOL bRet = false;
	wchar_t szBuffer[30] = {0};
	DWORD dwSize = 30*sizeof(wchar_t);
	if ( ::WinHttpQueryHeaders(m_hRequest, WINHTTP_QUERY_CONTENT_LENGTH, WINHTTP_HEADER_NAME_BY_INDEX, szBuffer, &dwSize, WINHTTP_NO_HEADER_INDEX) )
	{
		wchar_t*p = NULL;
		dwLength = wcstoul(szBuffer, &p, 10);
		bRet = true;
	}
	return bRet;
}

static void ParseUrl( LPCSTR lpUrl, string& strHostName, string& strPage, WORD& sPort )
{
	sPort = 80;
	string strTemp(lpUrl);
	int headerLen = 7;
	size_t nPos=strTemp.find("http://");

	if(string::npos == nPos)
	{
		nPos=strTemp.find("https://");
		if(nPos != string::npos)
		{
			sPort = INTERNET_DEFAULT_HTTPS_PORT;
			headerLen =8;
		}
	}
	if (string::npos != nPos )
		strTemp=strTemp.substr(nPos+headerLen, strTemp.size()-nPos-headerLen);
	nPos=strTemp.find('/');
	if ( string::npos == nPos )//没有找到 /
		strHostName=strTemp;
	else
		strHostName = strTemp.substr(0, nPos);
	size_t nPos1 = strHostName.find(':');
	if ( nPos1 != string::npos )
	{
		string strPort = strTemp.substr(nPos1+1, strHostName.size()-nPos1-1);
		strHostName = strHostName.substr(0, nPos1);
		sPort = (WORD)atoi(strPort.c_str());
	}
	if ( string::npos == nPos )
		return ;
	strPage=strTemp.substr(nPos, strTemp.size()-nPos);
}

BOOL CWinHttp::InitConnect( LPCSTR lpUrl, RequestType type, LPCSTR lpHeader, LPVOID lpPostData,DWORD dwDataLen )
{
	Close();
	if ( !Init() )
		return false;
	string strHostName, strPage;
	WORD wPort;
	ParseUrl(lpUrl, strHostName, strPage, wPort);
	if ( !ConnectHttpServer(strHostName.c_str(), wPort) )
	{
		m_error = Hir_ConnectErr;
		return false;
	}
	BOOL bHttps=FALSE;
	if (wPort == INTERNET_DEFAULT_HTTPS_PORT || strncmp(lpUrl,"https",5)==0)
		bHttps = true;

	if ( !CreateHttpRequest(strPage.c_str(), type, bHttps?WINHTTP_FLAG_SECURE:0) )
	{
		m_error = Hir_InitErr;
		return false;
	}
	if (bHttps)
	{
		DWORD dwFlags = SECURITY_FLAG_SECURE |
			SECURITY_FLAG_IGNORE_UNKNOWN_CA |
			SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
			SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
			SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
		WinHttpSetOption(m_hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));
	}
	if ( !SendHttpRequest(lpHeader,lpPostData, dwDataLen) )
	{
		m_error = Hir_SendErr;
		return false;
	}
	if ( !WinHttpReceiveResponse(m_hRequest, NULL) )
	{
		m_error = Hir_InitErr;;
		return false;
	}
	return true;
}

BOOL CWinHttp::SendHttpRequest( LPCSTR lpHeader, LPVOID lpPostData, DWORD dataLen)
{
	//添加HTTP头
	SStringW header = S_CA2W(m_header.toHttpHeaders().c_str(),CP_UTF8);
	::WinHttpAddRequestHeaders(m_hRequest, header.c_str(), header.GetLength(), WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);

	if ( !lpHeader){
		return ::WinHttpSendRequest(m_hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, lpPostData, dataLen, dataLen, NULL) == TRUE;
	}else{
		SStringW strHeader=S_CA2W(lpHeader,CP_UTF8);
		return ::WinHttpSendRequest(m_hRequest, strHeader.c_str(), strHeader.GetLength(), lpPostData, dataLen, dataLen, NULL) == TRUE;
	}
}

void CWinHttp::SetHeader(LPCSTR pszKey,LPCSTR pszValue)
{
	m_header.addHeader(pszKey,pszValue);
}

namespace HttpClient
{
	SOUI_COM_C BOOL SOUI_COM_API SCreateInstance(IObjRef **ppTaskLoop){
		*ppTaskLoop = new CWinHttp;
		return TRUE;
	}
}
SNSEND

EXTERN_C BOOL SOUI_COM_API HttpClient_SCreateInstance(IObjRef **ppTaskLoop){
	return SOUI::HttpClient::SCreateInstance(ppTaskLoop);
}
