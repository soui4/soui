#ifndef __SHTTPCLIENT_CAPI_H__
#define __SHTTPCLIENT_CAPI_H__

#include "../SHttpClient-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * HTTP Client Constants
 */
#define HTTP_REQUEST_GET    Hr_Get
#define HTTP_REQUEST_POST   Hr_Post

#define HTTP_DOWNLOAD_START     DS_Start
#define HTTP_DOWNLOAD_LOADING   DS_Loading
#define HTTP_DOWNLOAD_FINISHED  DS_Finished
#define HTTP_DOWNLOAD_FAILED    DS_Failed

#define HTTP_ERROR_SUCCESS      Hir_Success
#define HTTP_ERROR_INIT         Hir_InitErr
#define HTTP_ERROR_CONNECT      Hir_ConnectErr
#define HTTP_ERROR_SEND         Hir_SendErr
#define HTTP_ERROR_QUERY        Hir_QueryErr
#define HTTP_ERROR_404          Hir_404
#define HTTP_ERROR_ILLEGAL_URL  Hir_IllegalUrl
#define HTTP_ERROR_CREATE_FILE  Hir_CreateFileErr
#define HTTP_ERROR_DOWNLOAD     Hir_DownloadErr
#define HTTP_ERROR_QUERY_IP     Hir_QueryIPErr
#define HTTP_ERROR_SOCKET       Hir_SocketErr
#define HTTP_ERROR_USER_CANCEL  Hir_UserCancel
#define HTTP_ERROR_BUFFER       Hir_BufferErr
#define HTTP_ERROR_HEADER       Hir_HeaderErr
#define HTTP_ERROR_PARAM        Hir_ParamErr
#define HTTP_ERROR_UNKNOWN      Hir_UnknowErr

/*
 * C API Helper Macros for SHttpClient Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IHttpCallback C API Macros */
#define IHttpCallback_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IHttpCallback_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IHttpCallback_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IHttpCallback_OnDownloadCallback(This, state, nTotalSize, nLoadSize) \
    ((This)->lpVtbl->OnDownloadCallback(This, state, nTotalSize, nLoadSize))

/* IHttpClient C API Macros */
#define IHttpClient_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IHttpClient_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IHttpClient_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IHttpClient_SetHeader(This, pszKey, pszValue) \
    ((This)->lpVtbl->SetHeader(This, pszKey, pszValue))

#define IHttpClient_SetDownloadCallback(This, pCallback) \
    ((This)->lpVtbl->SetDownloadCallback(This, pCallback))

#define IHttpClient_DownloadFile(This, lpUrl, lpFilePath) \
    ((This)->lpVtbl->DownloadFile(This, lpUrl, lpFilePath))

#define IHttpClient_GetErrorCode(This) \
    ((This)->lpVtbl->GetErrorCode(This))

#define IHttpClient_Request(This, result, lpUrl, type, lpPostData, lpHeader) \
    ((This)->lpVtbl->Request(This, result, lpUrl, type, lpPostData, lpHeader))

#define IHttpClient_Request_Simple(This, result, lpUrl, type) \
    ((This)->lpVtbl->Request(This, result, lpUrl, type, NULL, NULL))

#define IHttpClient_Request2(This, out, lpUrl, type, lpPostData, dwDataLen, lpHeader) \
    ((This)->lpVtbl->Request2(This, out, lpUrl, type, lpPostData, dwDataLen, lpHeader))

#define IHttpClient_Request2_Simple(This, out, lpUrl, type, lpPostData, dwDataLen) \
    ((This)->lpVtbl->Request2(This, out, lpUrl, type, lpPostData, dwDataLen, NULL))

#define IHttpClient_SetTimeOut(This, dwConnectTime, dwSendTime, dwRecvTime) \
    ((This)->lpVtbl->SetTimeOut(This, dwConnectTime, dwSendTime, dwRecvTime))

#define IHttpClient_GetHttpCode(This) \
    ((This)->lpVtbl->GetHttpCode(This))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IHttpCallback Helper Functions */
static inline long IHttpCallback_AddRef_C(IHttpCallback* pThis)
{
    return IHttpCallback_AddRef(pThis);
}

static inline long IHttpCallback_Release_C(IHttpCallback* pThis)
{
    return IHttpCallback_Release(pThis);
}

static inline BOOL IHttpCallback_OnDownloadCallback_C(IHttpCallback* pThis, DownloadState state, uint64_t nTotalSize, uint64_t nLoadSize)
{
    return IHttpCallback_OnDownloadCallback(pThis, state, nTotalSize, nLoadSize);
}

/* IHttpClient Helper Functions */
static inline long IHttpClient_AddRef_C(IHttpClient* pThis)
{
    return IHttpClient_AddRef(pThis);
}

static inline long IHttpClient_Release_C(IHttpClient* pThis)
{
    return IHttpClient_Release(pThis);
}

static inline void IHttpClient_SetHeader_C(IHttpClient* pThis, LPCSTR pszKey, LPCSTR pszValue)
{
    IHttpClient_SetHeader(pThis, pszKey, pszValue);
}

static inline void IHttpClient_SetDownloadCallback_C(IHttpClient* pThis, IHttpCallback* pCallback)
{
    IHttpClient_SetDownloadCallback(pThis, pCallback);
}

static inline BOOL IHttpClient_DownloadFile_C(IHttpClient* pThis, LPCSTR lpUrl, LPCSTR lpFilePath)
{
    return IHttpClient_DownloadFile(pThis, lpUrl, lpFilePath);
}

static inline HttpError IHttpClient_GetErrorCode_C(IHttpClient* pThis)
{
    return IHttpClient_GetErrorCode(pThis);
}

static inline BOOL IHttpClient_Request_C(IHttpClient* pThis, IStringA* result, LPCSTR lpUrl, RequestType type, LPCSTR lpPostData, LPCSTR lpHeader)
{
    return IHttpClient_Request(pThis, result, lpUrl, type, lpPostData, lpHeader);
}

static inline BOOL IHttpClient_Request2_C(IHttpClient* pThis, IStringA* out, LPCSTR lpUrl, RequestType type, LPVOID lpPostData, DWORD dwDataLen, LPCSTR lpHeader)
{
    return IHttpClient_Request2(pThis, out, lpUrl, type, lpPostData, dwDataLen, lpHeader);
}

static inline void IHttpClient_SetTimeOut_C(IHttpClient* pThis, int dwConnectTime, int dwSendTime, int dwRecvTime)
{
    IHttpClient_SetTimeOut(pThis, dwConnectTime, dwSendTime, dwRecvTime);
}

static inline int IHttpClient_GetHttpCode_C(IHttpClient* pThis)
{
    return IHttpClient_GetHttpCode(pThis);
}

/*
 * Convenience macros for common HTTP operations
 */

/* Request type shortcuts */
#define IHttpClient_GET(This, result, url) \
    IHttpClient_Request_Simple(This, result, url, HTTP_REQUEST_GET)

#define IHttpClient_POST(This, result, url, postData) \
    IHttpClient_Request(This, result, url, HTTP_REQUEST_POST, postData, NULL)

#define IHttpClient_POST_WithHeader(This, result, url, postData, header) \
    IHttpClient_Request(This, result, url, HTTP_REQUEST_POST, postData, header)

/* Binary request shortcuts */
#define IHttpClient_POST_Binary(This, result, url, data, dataLen) \
    IHttpClient_Request2_Simple(This, result, url, HTTP_REQUEST_POST, data, dataLen)

#define IHttpClient_POST_Binary_WithHeader(This, result, url, data, dataLen, header) \
    IHttpClient_Request2(This, result, url, HTTP_REQUEST_POST, data, dataLen, header)

/* Timeout shortcuts */
#define IHttpClient_SetTimeOut_Default(This) \
    IHttpClient_SetTimeOut(This, 30000, 30000, 30000)

#define IHttpClient_SetTimeOut_Fast(This) \
    IHttpClient_SetTimeOut(This, 5000, 5000, 5000)

#define IHttpClient_SetTimeOut_Slow(This) \
    IHttpClient_SetTimeOut(This, 60000, 60000, 60000)

#define IHttpClient_SetTimeOut_Seconds(This, seconds) \
    IHttpClient_SetTimeOut(This, (seconds) * 1000, (seconds) * 1000, (seconds) * 1000)

/* Common headers shortcuts */
#define IHttpClient_SetContentType_JSON(This) \
    IHttpClient_SetHeader(This, "Content-Type", "application/json")

#define IHttpClient_SetContentType_XML(This) \
    IHttpClient_SetHeader(This, "Content-Type", "application/xml")

#define IHttpClient_SetContentType_Form(This) \
    IHttpClient_SetHeader(This, "Content-Type", "application/x-www-form-urlencoded")

#define IHttpClient_SetUserAgent(This, agent) \
    IHttpClient_SetHeader(This, "User-Agent", agent)

#define IHttpClient_SetAuthorization(This, auth) \
    IHttpClient_SetHeader(This, "Authorization", auth)

#define IHttpClient_SetAccept_JSON(This) \
    IHttpClient_SetHeader(This, "Accept", "application/json")

/* Error checking shortcuts */
#define IHttpClient_IsSuccess(This) \
    (IHttpClient_GetErrorCode(This) == HTTP_ERROR_SUCCESS)

#define IHttpClient_HasError(This) \
    (IHttpClient_GetErrorCode(This) != HTTP_ERROR_SUCCESS)

#define IHttpClient_IsHttpOK(This) \
    (IHttpClient_GetHttpCode(This) == 200)

#define IHttpClient_IsHttpError(This) \
    (IHttpClient_GetHttpCode(This) >= 400)

/* Safe HTTP operations */
#define IHttpClient_SafeSetHeader(This, key, value) \
    do { \
        if ((This) && (key) && (value)) { \
            IHttpClient_SetHeader(This, key, value); \
        } \
    } while(0)

#define IHttpClient_SafeSetCallback(This, callback) \
    do { \
        if ((This) && (callback)) { \
            IHttpClient_SetDownloadCallback(This, callback); \
        } \
    } while(0)

#define IHttpClient_SafeDownloadFile(This, url, filePath) \
    ((This) && (url) && (filePath) ? IHttpClient_DownloadFile(This, url, filePath) : FALSE)

#define IHttpClient_SafeGET(This, result, url) \
    ((This) && (result) && (url) ? IHttpClient_GET(This, result, url) : FALSE)

#define IHttpClient_SafePOST(This, result, url, postData) \
    ((This) && (result) && (url) ? IHttpClient_POST(This, result, url, postData) : FALSE)

/*
 * HTTP status code helpers
 */
static inline BOOL IHttpClient_IsHttpSuccess_C(IHttpClient* pThis)
{
    if (!pThis) return FALSE;
    int code = IHttpClient_GetHttpCode(pThis);
    return (code >= 200 && code < 300);
}

static inline BOOL IHttpClient_IsHttpRedirect_C(IHttpClient* pThis)
{
    if (!pThis) return FALSE;
    int code = IHttpClient_GetHttpCode(pThis);
    return (code >= 300 && code < 400);
}

static inline BOOL IHttpClient_IsHttpClientError_C(IHttpClient* pThis)
{
    if (!pThis) return FALSE;
    int code = IHttpClient_GetHttpCode(pThis);
    return (code >= 400 && code < 500);
}

static inline BOOL IHttpClient_IsHttpServerError_C(IHttpClient* pThis)
{
    if (!pThis) return FALSE;
    int code = IHttpClient_GetHttpCode(pThis);
    return (code >= 500 && code < 600);
}

/*
 * Error message helpers
 */
static inline LPCSTR HttpError_ToString(HttpError error)
{
    switch (error) {
        case HTTP_ERROR_SUCCESS:      return "Success";
        case HTTP_ERROR_INIT:         return "Initialization failed";
        case HTTP_ERROR_CONNECT:      return "Connection failed";
        case HTTP_ERROR_SEND:         return "Send request failed";
        case HTTP_ERROR_QUERY:        return "Query header failed";
        case HTTP_ERROR_404:          return "Page not found";
        case HTTP_ERROR_ILLEGAL_URL:  return "Illegal URL";
        case HTTP_ERROR_CREATE_FILE:  return "Create file failed";
        case HTTP_ERROR_DOWNLOAD:     return "Download failed";
        case HTTP_ERROR_QUERY_IP:     return "Query IP failed";
        case HTTP_ERROR_SOCKET:       return "Socket error";
        case HTTP_ERROR_USER_CANCEL:  return "User cancelled";
        case HTTP_ERROR_BUFFER:       return "Buffer error";
        case HTTP_ERROR_HEADER:       return "Header error";
        case HTTP_ERROR_PARAM:        return "Parameter error";
        case HTTP_ERROR_UNKNOWN:      return "Unknown error";
        default:                      return "Invalid error code";
    }
}

static inline LPCSTR DownloadState_ToString(DownloadState state)
{
    switch (state) {
        case HTTP_DOWNLOAD_START:     return "Start";
        case HTTP_DOWNLOAD_LOADING:   return "Loading";
        case HTTP_DOWNLOAD_FINISHED:  return "Finished";
        case HTTP_DOWNLOAD_FAILED:    return "Failed";
        default:                      return "Unknown";
    }
}

/*
 * Reference counting helpers
 */
#define IHttpCallback_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IHttpCallback_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

#define IHttpClient_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IHttpClient_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * HTTP client state management
 */
typedef struct HttpClientState {
    IHttpClient* client;
    HttpError lastError;
    int lastHttpCode;
    BOOL isValid;
} HttpClientState;

static inline void HttpClientState_Init(HttpClientState* state, IHttpClient* client)
{
    if (state) {
        state->client = client;
        state->lastError = client ? IHttpClient_GetErrorCode(client) : HTTP_ERROR_PARAM;
        state->lastHttpCode = client ? IHttpClient_GetHttpCode(client) : 0;
        state->isValid = (client != NULL);
    }
}

static inline void HttpClientState_Update(HttpClientState* state)
{
    if (state && state->client) {
        state->lastError = IHttpClient_GetErrorCode(state->client);
        state->lastHttpCode = IHttpClient_GetHttpCode(state->client);
    }
}

static inline BOOL HttpClientState_IsSuccess(const HttpClientState* state)
{
    return state && state->isValid && 
           (state->lastError == HTTP_ERROR_SUCCESS) && 
           (state->lastHttpCode >= 200 && state->lastHttpCode < 300);
}

static inline BOOL HttpClientState_HasError(const HttpClientState* state)
{
    return !state || !state->isValid || 
           (state->lastError != HTTP_ERROR_SUCCESS) || 
           (state->lastHttpCode >= 400);
}

#ifdef __cplusplus
}
#endif

#endif /* __SHTTPCLIENT_CAPI_H__ */
