#pragma once
#include <interface/SHttpClient-i.h>
#include <helper/obj-ref-impl.hpp>
#include <string>
#include "httpHeader.h"

SNSBEGIN

// wasm（浏览器）后端：IHttpClient 的同步接口在浏览器里靠 XMLHttpRequest 实现。
// 同步 XHR 只允许在 dedicated worker 里使用，因此本实现假定调用方在 ITaskLoop 的
// worker 线程上执行（components/TaskLoop 的 wasm worker 后端，构建时带 -pthread）。
// 主线程上的调用会直接失败，不阻塞页面。
class CWasmHttp : public TObjRefImpl<IHttpClient>
{
  public:
    CWasmHttp(void);
    virtual ~CWasmHttp(void);

    STDMETHOD_(void, SetHeader)(THIS_ LPCSTR pszKey, LPCSTR pszValue) OVERRIDE;
    STDMETHOD_(void, SetTimeOut)(THIS_ int dwConnectTime, int dwSendTime, int dwRecvTime) OVERRIDE;
    STDMETHOD_(BOOL, Request)(THIS_ IStringA *out, LPCSTR lpUrl, RequestType type,
                              LPCSTR lpPostData = NULL, LPCSTR lpHeader = NULL) OVERRIDE;
    STDMETHOD_(BOOL, Request2)(THIS_ IStringA *out, LPCSTR lpUrl, RequestType type,
                               LPVOID lpPostData, DWORD dwDataLen, LPCSTR lpHeader = NULL) OVERRIDE;
    STDMETHOD_(HttpError, GetErrorCode)(CTHIS) SCONST OVERRIDE { return m_error; }
    STDMETHOD_(void, SetDownloadCallback)(THIS_ IHttpCallback *pCallback) OVERRIDE
    {
        m_pCallback = pCallback;
    }
    STDMETHOD_(BOOL, DownloadFile)(THIS_ LPCSTR lpUrl, LPCSTR lpFilePath) OVERRIDE;
    STDMETHOD_(int, GetHttpCode)(CTHIS) SCONST OVERRIDE { return m_httpCode; }
    STDMETHOD_(BOOL, GetCookie)(THIS_ IStringA *out, LPCSTR lpUrl) OVERRIDE;

  private:
    // 发一次同步 XHR。pszHeaderText 是 CRLF 分隔的请求头原文；
    // ppBody/pBodyLen/ppCookie/pCookieLen 由 JS 侧 malloc 出来，调用方负责 _free。
    // 返回 HTTP 状态码；<0 表示网络层失败（拿不到状态码）。
    int DoRequest(LPCSTR lpUrl, BOOL bPost, LPCSTR lpPostData, DWORD dwDataLen,
                  const std::string &strHeader, char **ppBody, int *pBodyLen,
                  char **ppCookie, int *pCookieLen);

    HttpError m_error;
    IHttpCallback *m_pCallback;

    int m_nConnTimeout;
    int m_nSendTimeout;
    int m_nRecvTimeout;
    int m_httpCode;
    CHttpHeader m_header;
};

namespace HttpClient
{
SOUI_COM_C BOOL SOUI_COM_API SCreateInstance(IObjRef **ppObj);
}
SNSEND

EXTERN_C BOOL SOUI_COM_API HttpClient_SCreateInstance(IObjRef **ppObj);

// 同源 HTTP 代理前缀（例如 "/__soui_http?url="）。浏览器不允许 XHR 设置
// Cookie/Referer/User-Agent 等禁止头，也不允许脚本读取 Set-Cookie，所以默认
// 走 demos/wasm-demo/serve-isolated.py 的 --http-proxy 端点：请求经同源脚本
// 转发到上游，禁止头改名为 X-Soui-*，响应 Cookie 由 X-Soui-Cookie 头带回。
// 传空串表示直连（受 CORS 限制，且取不到 Cookie）。
EXTERN_C void HttpClientWasmSetProxyPrefix(const char *pszPrefix);
