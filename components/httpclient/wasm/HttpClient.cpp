#include "HttpClient.h"
#include <emscripten.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"

SNSBEGIN

// 同源代理前缀。默认值与 demos/wasm-demo/serve-isolated.py 的 --http-proxy
// 端点一致；宿主可在启动时用 HttpClientWasmSetProxyPrefix 覆盖（例如换成线上
// 反向代理的路径）。
static std::string g_strProxyPrefix = "/__soui_http?url=";

// 同步 XHR。Emscripten 的 pthread worker 里允许 xhr.open(..., false)；
// 主线程上浏览器会抛 InvalidAccessError，这里返回 -2 交给调用方当作网络失败。
//
// 请求头里被浏览器列为"禁止头"的 Cookie / Referer / User-Agent / Accept-Language
// 改名为 X-Soui-*，由代理还原成真正的上游头（浏览器会静默忽略对这些名字的
// setRequestHeader）。
//
// 响应文本经 XHR 按上游 charset 解码成 JS 字符串（GBK 页面也能正确处理），
// 再用 TextEncoder 转回 UTF-8 字节写进 wasm 堆，C++ 侧因此永远拿到合法 UTF-8。
EM_JS(int, soui_wasm_http_request,
      (const char *pszUrl, int bPost, const char *pszHeaders, const char *pszBody, int nBodyLen,
       int nTimeoutMs, const char *pszProxy, char **ppBody, int *pBodyLen, char **ppCookie,
       int *pCookieLen),
      {
          if (typeof XMLHttpRequest === 'undefined')
              return -1;

          var url = UTF8ToString(pszUrl);
          var proxy = UTF8ToString(pszProxy);
          var target = proxy.length > 0 ? (proxy + encodeURIComponent(url)) : url;

          var xhr = new XMLHttpRequest();
          try
          {
              xhr.open(bPost ? 'POST' : 'GET', target, false);
          }
          catch (e)
          {
              return -2;
          }

          var skip = {
              'host' : 1,
              'content-length' : 1,
              'connection' : 1,
              'accept-encoding' : 1,
              'origin' : 1,
              'keep-alive' : 1,
              'te' : 1,
              'trailer' : 1,
              'transfer-encoding' : 1,
              'upgrade' : 1
          };
          var rename = {
              'cookie' : 'X-Soui-Cookie',
              'referer' : 'X-Soui-Referer',
              'user-agent' : 'X-Soui-User-Agent',
              'accept-language' : 'X-Soui-Accept-Language'
          };
          var text = UTF8ToString(pszHeaders);
          var lines = text.split('\r\n');
          for (var i = 0; i < lines.length; ++i)
          {
              var line = lines[i];
              if (!line)
                  continue;
              var c = line.indexOf(':');
              if (c <= 0)
                  continue;
              var k = line.substring(0, c).replace(/^\s+|\s+$/g, '');
              var v = line.substring(c + 1).replace(/^\s+|\s+$/g, '');
              var lk = k.toLowerCase();
              if (skip[lk])
                  continue;
              if (rename[lk])
                  k = rename[lk];
              try
              {
                  xhr.setRequestHeader(k, v);
              }
              catch (e)
              {
              }
          }

          var body = null;
          if (bPost && nBodyLen > 0)
              body = UTF8ToString(pszBody, nBodyLen);
          try
          {
              xhr.send(body);
          }
          catch (e)
          {
              return -3;
          }

          var M = (typeof Module !== 'undefined' && Module) ? Module : null;
          var mallocFn = (M && M._malloc) ? M._malloc : (typeof _malloc === 'function' ? _malloc : null);
          if (!mallocFn)
              return -4;

          var resp = (xhr.responseText === null || xhr.responseText === undefined) ? '' : xhr.responseText;
          var bytes = new TextEncoder().encode(resp);
          var ptr = mallocFn(bytes.length > 0 ? bytes.length : 1);
          if (bytes.length > 0)
              HEAPU8.set(bytes, ptr);
          HEAPU32[ppBody >> 2] = ptr;
          HEAP32[pBodyLen >> 2] = bytes.length;

          var cookie = '';
          try
          {
              cookie = xhr.getResponseHeader('X-Soui-Cookie') || '';
          }
          catch (e)
          {
              cookie = '';
          }
          var cbytes = new TextEncoder().encode(cookie);
          var cptr = mallocFn(cbytes.length > 0 ? cbytes.length : 1);
          if (cbytes.length > 0)
              HEAPU8.set(cbytes, cptr);
          HEAPU32[ppCookie >> 2] = cptr;
          HEAP32[pCookieLen >> 2] = cbytes.length;

          // 状态码 0 表示网络错误 / 被 CORS 拦截 / 代理没起。
          return xhr.status;
      });

CWasmHttp::CWasmHttp(void)
    : m_error(Hir_Success), m_pCallback(NULL), m_nConnTimeout(5000), m_nSendTimeout(5000),
      m_nRecvTimeout(5000), m_httpCode(-1)
{
}

CWasmHttp::~CWasmHttp(void)
{
}

void CWasmHttp::SetHeader(LPCSTR pszKey, LPCSTR pszValue)
{
    m_header.addHeader(pszKey, pszValue);
}

void CWasmHttp::SetTimeOut(int dwConnectTime, int dwSendTime, int dwRecvTime)
{
    m_nConnTimeout = dwConnectTime;
    m_nSendTimeout = dwSendTime;
    m_nRecvTimeout = dwRecvTime;
}

BOOL CWasmHttp::DownloadFile(LPCSTR lpUrl, LPCSTR lpFilePath)
{
    // 同步 XHR 不允许 responseType=arraybuffer（会抛 InvalidAccessError），
    // 浏览器里也没有可直接落盘的路径。基金工具不使用这个接口。
    (void)lpUrl;
    (void)lpFilePath;
    m_error = Hir_UnknowErr;
    if (m_pCallback)
        m_pCallback->OnDownloadCallback(DS_Failed, 0, 0);
    return FALSE;
}

int CWasmHttp::DoRequest(LPCSTR lpUrl, BOOL bPost, LPCSTR lpPostData, DWORD dwDataLen,
                         const std::string &strHeader, char **ppBody, int *pBodyLen,
                         char **ppCookie, int *pCookieLen)
{
    *ppBody = NULL;
    *pBodyLen = 0;
    *ppCookie = NULL;
    *pCookieLen = 0;

    // 同步 XHR 没有可用的超时设置（timeout 属性只对异步请求有效），
    // m_nConnTimeout/m_nRecvTimeout 在这里仅保留语义，实际由代理侧的 socket 超时兜底。
    (void)m_nConnTimeout;
    (void)m_nSendTimeout;
    (void)m_nRecvTimeout;

    std::string body;
    if (bPost && lpPostData && dwDataLen)
        body.assign((const char *)lpPostData, dwDataLen);

    return soui_wasm_http_request(lpUrl, bPost ? 1 : 0, strHeader.c_str(), body.c_str(),
                                  (int)body.size(), 0, g_strProxyPrefix.c_str(), ppBody, pBodyLen,
                                  ppCookie, pCookieLen);
}

BOOL CWasmHttp::Request2(IStringA *out, LPCSTR lpUrl, RequestType type, LPVOID lpPostData,
                         DWORD dwDataLen, LPCSTR lpHeader)
{
    m_httpCode = -1;
    m_error = Hir_Success;
    if (!out || !lpUrl)
    {
        m_error = Hir_ParamErr;
        return FALSE;
    }

    std::string strHeader = m_header.toHttpHeaders();
    if (lpHeader && lpHeader[0])
    {
        strHeader += lpHeader;
        strHeader += http_newline;
    }

    char *pBody = NULL;
    int nBodyLen = 0;
    char *pCookie = NULL;
    int nCookieLen = 0;
    int nStatus = DoRequest(lpUrl, type == Hr_Post, (LPCSTR)lpPostData, dwDataLen, strHeader, &pBody,
                            &nBodyLen, &pCookie, &nCookieLen);
    if (pCookie)
        free(pCookie);

    m_httpCode = nStatus;
    BOOL bRet = (nStatus == 200);
    if (bRet)
    {
        out->Assign2(pBody ? pBody : "", nBodyLen);
    }
    else if (nStatus <= 0)
    {
        m_error = Hir_ConnectErr;
    }
    if (pBody)
        free(pBody);

    if (m_pCallback)
        m_pCallback->OnDownloadCallback(bRet ? DS_Finished : DS_Failed, 0, 0);
    return bRet;
}

BOOL CWasmHttp::Request(IStringA *out, LPCSTR lpUrl, RequestType type, LPCSTR lpPostData,
                        LPCSTR lpHeader)
{
    DWORD dwDataLen = lpPostData == NULL ? 0 : (DWORD)strlen(lpPostData);
    return Request2(out, lpUrl, type, (LPVOID)lpPostData, dwDataLen, lpHeader);
}

BOOL CWasmHttp::GetCookie(IStringA *out, LPCSTR lpUrl)
{
    m_httpCode = -1;
    m_error = Hir_Success;
    if (!out || !lpUrl)
    {
        m_error = Hir_ParamErr;
        return FALSE;
    }
    out->Empty();

    std::string strHeader = m_header.toHttpHeaders();
    char *pBody = NULL;
    int nBodyLen = 0;
    char *pCookie = NULL;
    int nCookieLen = 0;
    int nStatus = DoRequest(lpUrl, FALSE, NULL, 0, strHeader, &pBody, &nBodyLen, &pCookie,
                            &nCookieLen);
    m_httpCode = nStatus;
    if (pBody)
        free(pBody);

    if (nStatus <= 0)
    {
        if (pCookie)
            free(pCookie);
        m_error = Hir_ConnectErr;
        return FALSE;
    }

    BOOL bRet = FALSE;
    if (pCookie && nCookieLen > 0)
    {
        out->Assign2(pCookie, nCookieLen);
        bRet = TRUE;
    }
    if (pCookie)
        free(pCookie);
    return bRet;
}

namespace HttpClient
{
SOUI_COM_C BOOL SOUI_COM_API SCreateInstance(IObjRef **ppObj)
{
    *ppObj = new CWasmHttp;
    return TRUE;
}
}
SNSEND

EXTERN_C BOOL SOUI_COM_API HttpClient_SCreateInstance(IObjRef **ppObj)
{
    return SOUI::HttpClient::SCreateInstance(ppObj);
}

EXTERN_C void HttpClientWasmSetProxyPrefix(const char *pszPrefix)
{
    SOUI::g_strProxyPrefix = pszPrefix ? pszPrefix : "";
}
