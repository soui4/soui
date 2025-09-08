#include "HttpClient.h"
#include <fstream>
#include <sstream>
#include "define.h"
using namespace std;

SNSBEGIN

CWinHttp::CWinHttp(void)
	:m_nConnTimeout(5000)
	,m_nSendTimeout(5000)
	,m_nRecvTimeout(5000)
	, m_error(Hir_Success)
	, m_pCallback(NULL)
    ,curl(NULL)
{
	Init();
}

CWinHttp::~CWinHttp(void)
{
	Close();
}

BOOL CWinHttp::Init()
{

    // 初始化 CURL 指针
    curl = curl_easy_init();

	return curl!=NULL;
}

void CWinHttp::Close()
{
    if(curl){
        curl_easy_cleanup(curl);
        curl=NULL;
    }
}

void CWinHttp::SetTimeOut( int dwConnectTime, int dwSendTime, int dwRecvTime )
{
	m_nConnTimeout = dwConnectTime;
	m_nSendTimeout = dwSendTime;
	m_nRecvTimeout = dwRecvTime;
}

// 回调函数：用于处理下载的数据
size_t CWinHttp::write_data_file(void* ptr, size_t size, size_t nmemb, void* user_data) {
    FILE * fout = (FILE*)user_data;
    return fwrite(ptr,size,nmemb,fout);
}

size_t CWinHttp::write_data_string(void* ptr, size_t size, size_t nmemb, void* user_data) {
    std::stringstream * fout = (std::stringstream*)user_data;
    fout->write((const char*)ptr,size*nmemb);
    return nmemb;
}

// 进度回调函数
int CWinHttp::progress_dl_callback(void* user_data, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    CWinHttp * _this = (CWinHttp*)user_data;
    if(dltotal==0)
        return 0;
    if(!_this->m_pCallback)
        return 0;
    if(_this->m_st==DS_Start){
        BOOL bContinue = _this->m_pCallback->OnDownloadCallback(_this->m_st,dltotal,dlnow);
        if(!bContinue) return 1;
        _this->m_st = DS_Loading;
    }
    return _this->m_pCallback->OnDownloadCallback(_this->m_st,dltotal,dlnow)?0:1;
}


static struct curl_slist* add_headers_from_string(const char* headers_str) {
    struct curl_slist* headers = NULL;
    char* buffer = strdup(headers_str);  // 复制字符串以便修改
    char* header;

    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    header = strtok(buffer, http_newline);  // 使用换行符拆分
    while (header) {
        headers = curl_slist_append(headers, header);
        header = strtok(NULL, http_newline);
    }

    free(buffer);  // 释放临时缓冲区
    return headers;
}

BOOL CWinHttp::DownloadFile( LPCSTR lpUrl, LPCSTR lpFilePath )
{
	Close();
	if ( !Init() )
		return false;
	BOOL bRet = false;
    // 设置下载的 URL
    FILE * out = fopen(lpFilePath,"wb");
    if(!out)
    {
        m_error = Hir_CreateFileErr;
        Close();
        return FALSE;
    }    
    curl_easy_setopt(curl, CURLOPT_URL, lpUrl);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, m_nConnTimeout/1000);

    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 10L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, m_nRecvTimeout);

    std::string strHeader = m_header.toHttpHeaders();
    struct curl_slist* headers = add_headers_from_string(strHeader.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // 设置回调函数，将下载的数据写入文件
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_file);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);

    // 设置进度回调函数
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_dl_callback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);  // 启用进度回调

    // 执行下载操作
    m_st  = DS_Start;
    res = curl_easy_perform(curl);
    m_httpCode = -1;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &m_httpCode);

    fclose(out);
    if(headers){
        curl_slist_free_all(headers);
    }
	bRet =  res == CURLE_OK && m_httpCode==200;
	if(!bRet){//下载失败，删除文件
		DeleteFileA(lpFilePath);
	}
	if(m_pCallback){
		m_pCallback->OnDownloadCallback(bRet?DS_Finished:DS_Failed, 0, 0);
	}
    Close();
	return bRet;
}

BOOL CWinHttp::Request(IStringA *out, LPCSTR lpUrl, RequestType type, LPCSTR lpPostData /*= NULL*/, LPCSTR lpHeader/*=NULL*/ )
{
	DWORD dwDataLen = lpPostData==NULL?0:(DWORD)strlen(lpPostData);
	return Request2(out,lpUrl,type,(LPVOID)lpPostData,dwDataLen,lpHeader);
}

BOOL CWinHttp::Request2(IStringA *out, LPCSTR lpUrl, RequestType type, LPVOID lpPostData, DWORD dwDataLen, LPCSTR lpHeader/*=NULL*/ )
{
	Close();
	if ( !Init() )
		return false;
	BOOL bRet = false;
    std::stringstream outStream;
    // 设置下载的 URL
    curl_easy_setopt(curl, CURLOPT_URL, lpUrl);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, m_nConnTimeout/1000);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, (m_nConnTimeout+m_nRecvTimeout)/1000);

    switch(type){
        case Hr_Post:
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        break;
        case Hr_Get:
        default:
        curl_easy_setopt(curl, CURLOPT_POST, 0L);
        break;
    }
    std::string postData((const char*)lpPostData,dwDataLen);
    if(lpPostData && dwDataLen){
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
    }
    std::string strHeader = m_header.toHttpHeaders();
    struct curl_slist* headers = add_headers_from_string(strHeader.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // 设置回调函数，将下载的数据写入文件
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_string);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outStream);

    // 设置进度回调函数
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_dl_callback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);  // 启用进度回调

    m_st  = DS_Start;
    // 执行下载操作
    res = curl_easy_perform(curl);
    m_httpCode = -1;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &m_httpCode);
    if(headers){
        curl_slist_free_all(headers);
    }
    
	bRet =  res == CURLE_OK && m_httpCode==200;
	if(m_pCallback){
		m_pCallback->OnDownloadCallback(bRet?DS_Finished:DS_Failed, 0, 0);
	}
    Close();
    if(bRet){
        out->Assign2(outStream.str().c_str(),outStream.str().length());
    }
	return bRet;
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
