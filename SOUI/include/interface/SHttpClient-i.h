/*****************************************************************
 *HTTP handler class, mainly used for HTTP GET/POST and downloading (with redirect support)
 *Author：	JelinYao
 *Date：		2015/2/14 12:11
 *Email：	mailto://jelinyao@163.com
 */
//**************************************************************
#ifndef __SHTTPCLIENT_I__H__
#define __SHTTPCLIENT_I__H__
#include <interface/obj-ref-i.h>
#include <interface/sstring-i.h>
#include <stdint.h>

SNSBEGIN

typedef enum RequestType
{
    Hr_Post,
    Hr_Get,
} RequestType;

/** Enumerate download states */
typedef enum DownloadState
{
    DS_Start = 0,
    DS_Loading,
    DS_Finished,
    DS_Failed,
} DownloadState;

/******************************************************
 * Define error messages
 ******************************************************/
typedef enum HttpError
{
    Hir_Success = 0,   /**< Success */
    Hir_InitErr,       /**< Initialization failed */
    Hir_ConnectErr,    /**< Failed to connect to HTTP server */
    Hir_SendErr,       /**< Failed to send request */
    Hir_QueryErr,      /**< Failed to query HTTP request header */
    Hir_404,           /**< Page not found */
    Hir_IllegalUrl,    /**< Invalid URL */
    Hir_CreateFileErr, /**< Failed to create file */
    Hir_DownloadErr,   /**< Download failed */
    Hir_QueryIPErr,    /**< Failed to resolve domain name to address */
    Hir_SocketErr,     /**< Socket error */
    Hir_UserCancel,    /**< User canceled download */
    Hir_BufferErr,     /**< File too large, buffer insufficient */
    Hir_HeaderErr,     /**< HTTP request header error */
    Hir_ParamErr,      /**< Parameter error, null pointer, empty string…… */
    Hir_UnknowErr,     /**< Unknown error */
} HttpError;

#undef INTERFACE
#define INTERFACE IHttpCallback
DECLARE_INTERFACE_(IHttpCallback, IObjRef)
{
    /**
     * @brief Increment reference count
     * @return long - new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement reference count
     * @return long - new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Download callback
     * @param state - download state
     * @param nTotalSize - total size
     * @param nLoadSize - downloaded size
     * @return BOOL - TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, OnDownloadCallback)(THIS_ DownloadState state, uint64_t nTotalSize, uint64_t nLoadSize) PURE;
};

#undef INTERFACE
#define INTERFACE IHttpClient
DECLARE_INTERFACE_(IHttpClient, IObjRef)
{
    /**
     * @brief Increment reference count
     * @return long - new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement reference count
     * @return long - new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Set HTTP request header
     * @param pszKey - request header key
     * @param pszValue - request header value
     * @return void
     */
    STDMETHOD_(void, SetHeader)(THIS_ LPCSTR pszKey, LPCSTR pszValue) PURE;

    /**
     * @brief Set download callback
     * @param pCallback - callback pointer
     * @return void
     */
    STDMETHOD_(void, SetDownloadCallback)(THIS_ IHttpCallback * pCallback) PURE;

    /**
     * @brief Download file
     * @param lpUrl - file URL
     * @param lpFilePath - file save path
     * @return BOOL - TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, DownloadFile)(THIS_ LPCSTR lpUrl, LPCSTR lpFilePath) PURE;

    /**
     * @brief Get error code
     * @return HttpError - error code
     */
    STDMETHOD_(HttpError, GetErrorCode)(CTHIS) SCONST PURE;

    /**
     * @brief Send HTTP request
     * @param result - return result
     * @param lpUrl - request URL
     * @param type - request type (GET or POST)
     * @param lpPostData - POST data
     * @param lpHeader - request header
     * @return BOOL - TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, Request)(THIS_ IStringA * result, LPCSTR lpUrl, RequestType type, LPCSTR lpPostData DEF_VAL(NULL), LPCSTR lpHeader DEF_VAL(NULL)) PURE;

    /**
     * @brief Send HTTP request
     * @param out - return result
     * @param lpUrl - request URL
     * @param type - request type (GET or POST)
     * @param lpPostData - POST data
     * @param dwDataLen - POST data length
     * @param lpHeader - request header
     * @return BOOL - TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, Request2)(THIS_ IStringA * out, LPCSTR lpUrl, RequestType type, LPVOID lpPostData, DWORD dwDataLen, LPCSTR lpHeader DEF_VAL(NULL)) PURE;

    /**
     * @brief Set timeout
     * @param dwConnectTime - connection timeout
     * @param dwSendTime - send timeout
     * @param dwRecvTime - receive timeout
     * @return void
     */
    STDMETHOD_(void, SetTimeOut)(THIS_ int dwConnectTime, int dwSendTime, int dwRecvTime) PURE;

    /**
     * @brief Get HTTP status code
     * @return int - HTTP status code
     */
    STDMETHOD_(int, GetHttpCode)(CTHIS) SCONST PURE;
};

SNSEND

#endif /**< __SHTTPCLIENT_I__H__ */
