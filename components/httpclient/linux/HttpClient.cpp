#include "HttpClient.h"
#include <fstream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include "define.h"
#ifdef __ANDROID__
#include <android/log.h>
#endif
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
using namespace std;

#ifndef __ANDROID__
// libcurl 使用的默认 CA 来自 OpenSSL 编译期的 OPENSSLDIR（构建机上的 install 前缀），
// 在移动设备和部分 Linux 发行版上并不存在，找不到证书时 https 会报
// CURLE_PEER_FAILED_VERIFICATION(60)。这里按平台探测系统预置的证书 bundle。
// Android 没有通用路径，由启动阶段（AndroidPlatformAPI::initSouiApp）把证书拼成
// PEM 后写入 SOUI_CA_BUNDLE，因此不参与这里的探测。
static const char * FindCaBundle()
{
    static const char * s_caBundles[] = {
        "/etc/ssl/certs/cacert.pem",           // OpenHarmony 系统预置 CA
        "/etc/ssl/certs/ca-certificates.crt",  // Debian/Ubuntu
        "/etc/ssl/cert.pem",                   // macOS（iOS 模拟器走同一路径）
        "/etc/pki/tls/cert.pem",               // RHEL/Fedora
    };
    for (size_t i = 0; i < sizeof(s_caBundles) / sizeof(s_caBundles[0]); ++i) {
        if (access(s_caBundles[i], R_OK) == 0)
            return s_caBundles[i];
    }
#ifdef __APPLE__
    // iOS 真机没有可读的系统证书文件，需要把 cacert.pem 拷进 .app 根目录（与可执行文件同级），
    // 这里按可执行文件所在目录查找。
    static string s_strAppCa;
    if (s_strAppCa.empty()) {
        char szExe[1024];
        uint32_t uSize = sizeof(szExe);
        if (_NSGetExecutablePath(szExe, &uSize) != 0)
            return NULL;
        string strPath(szExe);
        size_t nPos = strPath.find_last_of('/');
        if (nPos == string::npos)
            return NULL;
        strPath.erase(nPos + 1);
        strPath += "cacert.pem";
        if (access(strPath.c_str(), R_OK) != 0)
            return NULL;
        s_strAppCa = strPath;
    }
    return s_strAppCa.c_str();
#endif
    return NULL;
}
#endif

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
    if(curl)
    {
        // 必须显式指定 CA：libcurl 默认使用 OpenSSL 编译期写死的 OPENSSLDIR，
        // 在移动设备/部分发行版上不存在，https 会因找不到证书报
        // CURLE_PEER_FAILED_VERIFICATION(60)。
        //   1) SOUI_CA_BUNDLE：平台层生成的 PEM（Android）
        //   2) SSL_CERT_FILE：OpenSSL 环境变量约定
        //   3) FindCaBundle()：系统预置证书 / Apple 应用包内的 cacert.pem
        const char *pszCaBundle = getenv("SOUI_CA_BUNDLE");
        if(!pszCaBundle || !pszCaBundle[0])
            pszCaBundle = getenv("SSL_CERT_FILE");
#ifndef __ANDROID__
        if(!pszCaBundle || !pszCaBundle[0])
            pszCaBundle = FindCaBundle();
#endif
        if(pszCaBundle && pszCaBundle[0])
            curl_easy_setopt(curl, CURLOPT_CAINFO, pszCaBundle);
    }

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

// 收集响应头原文（逐行回调），交给 CookieUtil 解析 Set-Cookie
size_t CWinHttp::collect_header(char* ptr, size_t size, size_t nmemb, void* user_data) {
    std::string * pHeader = (std::string*)user_data;
    const size_t len = size * nmemb;
    if (pHeader && len) pHeader->append(ptr, len);
    return len;
}

// 只要 Cookie，响应体全部丢弃（避免把整页 HTML 读进内存）
size_t CWinHttp::discard_body(char* ptr, size_t size, size_t nmemb, void* user_data) {
    (void)ptr; (void)user_data;
    return size * nmemb;
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
	// 先置为 -1：Init/perform 失败时调用方读 GetHttpCode 得到的是 -1（网络层失败），
	// 而不是上一次请求残留的状态码。
	m_httpCode = -1;
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
    // 与 WinHTTP 后端对齐：WinHTTP 默认跟随重定向，这里也开启，
    // 否则遇到 301/302 会拿到空 body（东财部分接口会做 http->https 跳转）。
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    std::string postData((const char*)lpPostData,dwDataLen);
    if(lpPostData && dwDataLen){
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
    }
    // 合并"全局头"（SetHeader 累积）与"本次请求头"（lpHeader）。
    // 以前这里只用 m_header，lpHeader 被静默丢弃，导致同一个调用在 WinHTTP 后端
    // 能带上 Referer/Cookie，在 libcurl 后端却发不出去 —— 跨平台行为不一致。
    std::string strHeader = m_header.toHttpHeaders();
    if (lpHeader && lpHeader[0] != '\0') {
        strHeader += lpHeader;
        strHeader += http_newline;
    }
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
#ifdef __ANDROID__
    if(res != CURLE_OK)
        __android_log_print(ANDROID_LOG_WARN, "httpclient", "curl error %d: %s url=%s", (int)res, curl_easy_strerror(res), lpUrl);
#endif
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

BOOL CWinHttp::GetCookie(IStringA *out, LPCSTR lpUrl)
{
	if (out == NULL || lpUrl == NULL) {
		m_error = Hir_ParamErr;
		return FALSE;
	}
	out->Empty();

	Close();
	if (!Init()) {
		m_error = Hir_InitErr;
		return FALSE;
	}

	std::string strRawHeaders;
	curl_easy_setopt(curl, CURLOPT_URL, lpUrl);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, m_nConnTimeout / 1000);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, (m_nConnTimeout + m_nRecvTimeout) / 1000);
	curl_easy_setopt(curl, CURLOPT_POST, 0L);
	// 与 WinHTTP 行为对齐：WinHTTP 默认跟随重定向（且会在重定向链路里带上 Cookie），
	// 所以这里也要开启，否则 ASP.NET 站点 302 之后就取不到 Set-Cookie 了。
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);

	std::string strHeader = m_header.toHttpHeaders();
	struct curl_slist* headers = add_headers_from_string(strHeader.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, collect_header);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &strRawHeaders);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard_body);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

	res = curl_easy_perform(curl);
	m_httpCode = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &m_httpCode);
	if (headers) {
		curl_slist_free_all(headers);
	}
	if (res != CURLE_OK) {
		// 与 Win 端一致：网络层失败才算错误，状态码交给调用方判断
		m_error = Hir_SendErr;
		Close();
		return FALSE;
	}

	std::string strCookie = CookieUtil::ParseFromRawHeaders(strRawHeaders);
	Close();
	if (strCookie.empty()) {
		// 没有 Cookie 不算错误，调用方按"无会话"继续请求即可
		m_error = Hir_Success;
		return FALSE;
	}
	out->Assign2(strCookie.c_str(), (int)strCookie.size());
	m_error = Hir_Success;
	return TRUE;
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
