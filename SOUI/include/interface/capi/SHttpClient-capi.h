#ifndef __SHTTPCLIENT_CAPI_H__
#define __SHTTPCLIENT_CAPI_H__

#include "../SHttpClient-i.h"

#ifdef __cplusplus
extern "C" {
#endif

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

#define IHttpClient_Request2(This, out, lpUrl, type, lpPostData, dwDataLen, lpHeader) \
    ((This)->lpVtbl->Request2(This, out, lpUrl, type, lpPostData, dwDataLen, lpHeader))

#define IHttpClient_SetTimeOut(This, dwConnectTime, dwSendTime, dwRecvTime) \
    ((This)->lpVtbl->SetTimeOut(This, dwConnectTime, dwSendTime, dwRecvTime))

#define IHttpClient_GetHttpCode(This) \
    ((This)->lpVtbl->GetHttpCode(This))

#ifdef __cplusplus
}
#endif

#endif /* __SHTTPCLIENT_CAPI_H__ */
