#ifndef WS_LISTENER_H
#define WS_LISTENER_H

#define _WINSOCKAPI_
#include <interface/obj-ref-i.h>
#include <stdint.h>

SNSBEGIN

enum
{
    kAllGroupId = -1,
    kDefGroupId = 0,
    kSocketBufSize = 4096,
};

#undef INTERFACE
#define INTERFACE IConnection
DECLARE_INTERFACE_(IConnection, IObjRef)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    STDMETHOD_(int, isValid)(CTHIS) SCONST PURE;
    STDMETHOD_(int, sendText)(THIS_ const char *text, int nLen DEF_VAL(-1)) PURE;
    STDMETHOD_(int, sendBinary)(THIS_ const void *data, int nLen) PURE;
};

#undef INTERFACE
#define INTERFACE ISvrConnection
DECLARE_INTERFACE_(ISvrConnection, IConnection)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    STDMETHOD_(int, isValid)(CTHIS) SCONST PURE;
    STDMETHOD_(int, sendText)(THIS_ const char *text, int nLen DEF_VAL(-1)) PURE;
    STDMETHOD_(int, sendBinary)(THIS_ const void *data, int nLen) PURE;
    STDMETHOD_(int, sendBinary2)(THIS_ DWORD dwType, const void *data, int nLen) PURE;

    STDMETHOD_(void, close)(THIS_ const char *reason) PURE;
    STDMETHOD_(void, setId)(THIS_ int id) PURE;
    STDMETHOD_(int, getId)(CTHIS) SCONST PURE;
    STDMETHOD_(void, setGroupId)(THIS_ int id) PURE;
    STDMETHOD_(int, getGroupId)(CTHIS) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IConnListener
DECLARE_INTERFACE_(IConnListener, IObjRef)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    STDMETHOD_(void, onConnected)(THIS) PURE;
    STDMETHOD_(void, onConnError)(THIS_ const char *errStr) PURE;
    STDMETHOD_(void, onDisconnect)(THIS) PURE;
    STDMETHOD_(void, onDataSent)(THIS_ int nMsgId) PURE;
    STDMETHOD_(void, onDataRecv)(THIS_ const void *data, int len, BOOL bBinary) PURE;
};

#undef INTERFACE
#define INTERFACE ISvrListener
DECLARE_INTERFACE_(ISvrListener, IObjRef)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    STDMETHOD_(BOOL, onConnected)(THIS_ ISvrConnection * pConn, const char *uriPath, const char *uriArgs) PURE;
    STDMETHOD_(void, onConnError)(THIS_ ISvrConnection * pConn, const char *errStr) PURE;
    STDMETHOD_(void, onDisconnect)(THIS_ ISvrConnection * pConn) PURE;
    STDMETHOD_(void, onDataSent)(THIS_ ISvrConnection * pConn, int nMsgId) PURE;
    STDMETHOD_(void, onDataRecv)(THIS_ ISvrConnection * pConn, const void *data, int len, BOOL bBinary) PURE;
};

typedef struct _SvrOption
{
    BOOL bSecure;
    const char *cert_u8;
    const char *priv_key_u8;
} SvrOption;
typedef struct _SvrPingCfg
{
    int pingIntervalSeconds; // ping间隔时间，单位秒
    int nHeartbeatSeconds;   // 心跳间隔时间，单位秒，ping间隔时间的2倍即可
    int nPingTimeoutCount;   // ping超时次数，超过此次数则关闭连接
}SvrPingCfg;

#undef INTERFACE
#define INTERFACE IWsServer
DECLARE_INTERFACE_(IWsServer, IObjRef)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    STDMETHOD_(int, start)(THIS_ uint16_t port, const char *protocolName, SvrOption option, SvrPingCfg pingCfg) PURE;
    STDMETHOD_(BOOL, wait)(THIS_ int timeout) PURE;
    STDMETHOD_(void, quit)(THIS) PURE;
};

typedef struct _ClientOption
{
    BOOL bSecure;
    const char *ca_u8;
    BOOL allowSelfSigned;
    BOOL skipServerCertHostnameCheck;
    BOOL allowExpired;
} ClientOption;

#undef INTERFACE
#define INTERFACE IWsClient
DECLARE_INTERFACE_(IWsClient, IConnection)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    STDMETHOD_(int, isValid)(CTHIS) SCONST PURE;
    STDMETHOD_(int, sendText)(THIS_ const char *text, int nLen DEF_VAL(-1)) PURE;
    STDMETHOD_(int, sendBinary)(THIS_ const void *data, int nLen) PURE;
    STDMETHOD_(int, sendBinary2)(THIS_ DWORD dwType, const void *data, int nLen) PURE;

    STDMETHOD_(int, connectTo)(THIS_ const char *pszSvr, const char *pszPath, uint16_t nPort, const char *pszProtocol, ClientOption option) PURE;
    STDMETHOD_(void, disconnect)(THIS) PURE;
    STDMETHOD_(BOOL, wait)(THIS_ int timeoutMs) PURE;
};

typedef void (*WsLogCallback)(int level, const char *line);

#undef INTERFACE
#define INTERFACE IWebsocket
DECLARE_INTERFACE_(IWebsocket, IObjRef)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    STDMETHOD_(IWsClient *, CreateWsClient)(THIS_ IConnListener * pListener) PURE;
    STDMETHOD_(IWsServer *, CreateWsServer)(THIS_ ISvrListener * pListener) PURE;
    STDMETHOD_(void, SetLogCallback)(THIS_ WsLogCallback cb) PURE;
};

SNSEND
#endif // WS_LISTENER_H