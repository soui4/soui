#ifndef __SIPCOBJ_CAPI_H__
#define __SIPCOBJ_CAPI_H__

#include "../sipcobj-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * IPC constants
 */
#define IPC_FUN_ID_CONNECT      FUN_ID_CONNECT
#define IPC_FUN_ID_DISCONNECT   FUN_ID_DISCONNECT
#define IPC_FUN_ID_START        FUN_ID_START

#define IPC_UM_CALL_FUN         UM_CALL_FUN

#define IPC_SEEK_SET            IShareBuffer::seek_set
#define IPC_SEEK_CUR            IShareBuffer::seek_cur
#define IPC_SEEK_END            IShareBuffer::seek_end

/*
 * C API Helper Macros for sipcobj Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IShareBuffer C API Macros */
#define IShareBuffer_Write(This, data, nLen) \
    ((This)->Write(data, nLen))

#define IShareBuffer_Read(This, buf, nLen) \
    ((This)->Read(buf, nLen))

#define IShareBuffer_Tell(This) \
    ((This)->Tell())

#define IShareBuffer_Seek(This, mode, nOffset) \
    ((This)->Seek(mode, nOffset))

#define IShareBuffer_SetTail(This, uPos) \
    ((This)->SetTail(uPos))

#define IShareBuffer_Lock(This, timeout) \
    ((This)->Lock(timeout))

#define IShareBuffer_Unlock(This) \
    ((This)->Unlock())

/* IFunParams C API Macros */
#define IFunParams_GetID(This) \
    ((This)->GetID())

#define IFunParams_ToStream4Input(This, ps) \
    ((This)->ToStream4Input(ps))

#define IFunParams_ToStream4Output(This, ps) \
    ((This)->ToStream4Output(ps))

#define IFunParams_FromStream4Input(This, ps) \
    ((This)->FromStream4Input(ps))

#define IFunParams_FromStream4Output(This, ps) \
    ((This)->FromStream4Output(ps))

/* IIpcHandle C API Macros */
#define IIpcHandle_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IIpcHandle_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IIpcHandle_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IIpcHandle_IsConnected(This) \
    ((This)->lpVtbl->IsConnected(This))

#define IIpcHandle_SetIpcConnection(This, pConn) \
    ((This)->lpVtbl->SetIpcConnection(This, pConn))

#define IIpcHandle_GetIpcConnection(This) \
    ((This)->lpVtbl->GetIpcConnection(This))

#define IIpcHandle_OnMessage(This, idLocal, uMsg, wp, lp, bHandled) \
    ((This)->lpVtbl->OnMessage(This, idLocal, uMsg, wp, lp, bHandled))

#define IIpcHandle_ConnectTo(This, idLocal, idSvr) \
    ((This)->lpVtbl->ConnectTo(This, idLocal, idSvr))

#define IIpcHandle_Disconnect(This) \
    ((This)->lpVtbl->Disconnect(This))

#define IIpcHandle_CallFun(This, pParam) \
    ((This)->lpVtbl->CallFun(This, pParam))

#define IIpcHandle_GetLocalId(This) \
    ((This)->lpVtbl->GetLocalId(This))

#define IIpcHandle_GetRemoteId(This) \
    ((This)->lpVtbl->GetRemoteId(This))

#define IIpcHandle_GetSendBuffer(This) \
    ((This)->lpVtbl->GetSendBuffer(This))

#define IIpcHandle_GetRecvBuffer(This) \
    ((This)->lpVtbl->GetRecvBuffer(This))

#define IIpcHandle_InitShareBuf(This, idLocal, idRemote, nBufSize, pSa) \
    ((This)->lpVtbl->InitShareBuf(This, idLocal, idRemote, nBufSize, pSa))

#define IIpcHandle_ToStream4Input(This, pParams, pBuf) \
    ((This)->lpVtbl->ToStream4Input(This, pParams, pBuf))

#define IIpcHandle_FromStream4Input(This, pParams, pBuf) \
    ((This)->lpVtbl->FromStream4Input(This, pParams, pBuf))

#define IIpcHandle_ToStream4Output(This, pParams, pBuf) \
    ((This)->lpVtbl->ToStream4Output(This, pParams, pBuf))

#define IIpcHandle_FromStream4Output(This, pParams, pBuf) \
    ((This)->lpVtbl->FromStream4Output(This, pParams, pBuf))

/* IIpcConnection C API Macros */
#define IIpcConnection_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IIpcConnection_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IIpcConnection_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IIpcConnection_GetBufSize(This) \
    ((This)->lpVtbl->GetBufSize(This))

#define IIpcConnection_GetStackSize(This) \
    ((This)->lpVtbl->GetStackSize(This))

#define IIpcConnection_GetIpcHandle(This) \
    ((This)->lpVtbl->GetIpcHandle(This))

#define IIpcConnection_HandleFun(This, uFunID, ps) \
    ((This)->lpVtbl->HandleFun(This, uFunID, ps))

#define IIpcConnection_BuildShareBufferName(This, idLocal, idRemote, szBuf) \
    ((This)->lpVtbl->BuildShareBufferName(This, idLocal, idRemote, szBuf))

/* IIpcServer C API Macros */
#define IIpcServer_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IIpcServer_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IIpcServer_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IIpcServer_Init(This, idSvr, pCallback) \
    ((This)->lpVtbl->Init(This, idSvr, pCallback))

#define IIpcServer_CheckConnectivity(This) \
    ((This)->lpVtbl->CheckConnectivity(This))

#define IIpcServer_OnMessage(This, idLocal, uMsg, wp, lp, bHandled) \
    ((This)->lpVtbl->OnMessage(This, idLocal, uMsg, wp, lp, bHandled))

#define IIpcServer_EnumClient(This, funEnum, data) \
    ((This)->lpVtbl->EnumClient(This, funEnum, data))

#define IIpcServer_FindConnection(This, idConn) \
    ((This)->lpVtbl->FindConnection(This, idConn))

#define IIpcServer_Disconnect(This, idConn) \
    ((This)->lpVtbl->Disconnect(This, idConn))

/* IIpcFactory C API Macros */
#define IIpcFactory_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IIpcFactory_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IIpcFactory_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IIpcFactory_CreateIpcServer(This, ppServer) \
    ((This)->lpVtbl->CreateIpcServer(This, ppServer))

#define IIpcFactory_CreateIpcHandle(This, ppHandle) \
    ((This)->lpVtbl->CreateIpcHandle(This, ppHandle))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IShareBuffer Helper Functions */
static inline int IShareBuffer_Write_C(IShareBuffer* pThis, const void* data, UINT nLen)
{
    return IShareBuffer_Write(pThis, data, nLen);
}

static inline int IShareBuffer_Read_C(IShareBuffer* pThis, void* buf, UINT nLen)
{
    return IShareBuffer_Read(pThis, buf, nLen);
}

static inline UINT IShareBuffer_Tell_C(const IShareBuffer* pThis)
{
    return IShareBuffer_Tell(pThis);
}

static inline UINT IShareBuffer_Seek_C(IShareBuffer* pThis, int mode, int nOffset)
{
    return IShareBuffer_Seek(pThis, (IShareBuffer::SEEK)mode, nOffset);
}

static inline void IShareBuffer_SetTail_C(IShareBuffer* pThis, UINT uPos)
{
    IShareBuffer_SetTail(pThis, uPos);
}

static inline BOOL IShareBuffer_Lock_C(IShareBuffer* pThis, DWORD timeout)
{
    return IShareBuffer_Lock(pThis, timeout);
}

static inline void IShareBuffer_Unlock_C(IShareBuffer* pThis)
{
    IShareBuffer_Unlock(pThis);
}

/* IIpcHandle Helper Functions */
static inline long IIpcHandle_AddRef_C(IIpcHandle* pThis)
{
    return IIpcHandle_AddRef(pThis);
}

static inline long IIpcHandle_Release_C(IIpcHandle* pThis)
{
    return IIpcHandle_Release(pThis);
}

static inline BOOL IIpcHandle_IsConnected_C(const IIpcHandle* pThis)
{
    return IIpcHandle_IsConnected(pThis);
}

static inline HRESULT IIpcHandle_ConnectTo_C(IIpcHandle* pThis, ULONG_PTR idLocal, ULONG_PTR idSvr)
{
    return IIpcHandle_ConnectTo(pThis, idLocal, idSvr);
}

static inline HRESULT IIpcHandle_Disconnect_C(IIpcHandle* pThis)
{
    return IIpcHandle_Disconnect(pThis);
}

static inline bool IIpcHandle_CallFun_C(const IIpcHandle* pThis, IFunParams* pParam)
{
    return IIpcHandle_CallFun(pThis, pParam);
}

/* IIpcServer Helper Functions */
static inline long IIpcServer_AddRef_C(IIpcServer* pThis)
{
    return IIpcServer_AddRef(pThis);
}

static inline long IIpcServer_Release_C(IIpcServer* pThis)
{
    return IIpcServer_Release(pThis);
}

static inline HRESULT IIpcServer_Init_C(IIpcServer* pThis, ULONG_PTR idSvr, IIpcSvrCallback* pCallback)
{
    return IIpcServer_Init(pThis, idSvr, pCallback);
}

static inline BOOL IIpcServer_FindConnection_C(IIpcServer* pThis, ULONG_PTR idConn)
{
    return IIpcServer_FindConnection(pThis, idConn);
}

static inline BOOL IIpcServer_Disconnect_C(IIpcServer* pThis, ULONG_PTR idConn)
{
    return IIpcServer_Disconnect(pThis, idConn);
}

/* IIpcFactory Helper Functions */
static inline HRESULT IIpcFactory_CreateIpcServer_C(IIpcFactory* pThis, IIpcServer** ppServer)
{
    return IIpcFactory_CreateIpcServer(pThis, ppServer);
}

static inline HRESULT IIpcFactory_CreateIpcHandle_C(IIpcFactory* pThis, IIpcHandle** ppHandle)
{
    return IIpcFactory_CreateIpcHandle(pThis, ppHandle);
}

/*
 * Convenience macros for common IPC operations
 */

/* Buffer operations shortcuts */
#define IShareBuffer_WriteData(This, data, len) \
    IShareBuffer_Write(This, data, len)

#define IShareBuffer_ReadData(This, buf, len) \
    IShareBuffer_Read(This, buf, len)

#define IShareBuffer_GetPosition(This) \
    IShareBuffer_Tell(This)

#define IShareBuffer_SetPosition(This, pos) \
    IShareBuffer_Seek(This, IPC_SEEK_SET, pos)

#define IShareBuffer_SeekToEnd(This) \
    IShareBuffer_Seek(This, IPC_SEEK_END, 0)

#define IShareBuffer_SeekToBegin(This) \
    IShareBuffer_Seek(This, IPC_SEEK_SET, 0)

/* Connection shortcuts */
#define IIpcHandle_Connect(This, localId, serverId) \
    IIpcHandle_ConnectTo(This, localId, serverId)

#define IIpcHandle_IsActive(This) \
    IIpcHandle_IsConnected(This)

#define IIpcHandle_GetConnection(This) \
    IIpcHandle_GetIpcConnection(This)

#define IIpcHandle_SetConnection(This, conn) \
    IIpcHandle_SetIpcConnection(This, conn)

/* Server shortcuts */
#define IIpcServer_Initialize(This, serverId, callback) \
    IIpcServer_Init(This, serverId, callback)

#define IIpcServer_HasConnection(This, connId) \
    IIpcServer_FindConnection(This, connId)

#define IIpcServer_DisconnectClient(This, connId) \
    IIpcServer_Disconnect(This, connId)

/* Factory shortcuts */
#define IIpcFactory_NewServer(This, ppServer) \
    IIpcFactory_CreateIpcServer(This, ppServer)

#define IIpcFactory_NewHandle(This, ppHandle) \
    IIpcFactory_CreateIpcHandle(This, ppHandle)

/*
 * Safe IPC operations
 */
#define IIpcHandle_SafeConnect(This, localId, serverId) \
    ((This) ? IIpcHandle_ConnectTo(This, localId, serverId) : E_INVALIDARG)

#define IIpcHandle_SafeDisconnect(This) \
    ((This) ? IIpcHandle_Disconnect(This) : E_INVALIDARG)

#define IIpcHandle_SafeCallFun(This, param) \
    ((This) && (param) ? IIpcHandle_CallFun(This, param) : false)

#define IIpcServer_SafeInit(This, serverId, callback) \
    ((This) && (callback) ? IIpcServer_Init(This, serverId, callback) : E_INVALIDARG)

#define IShareBuffer_SafeWrite(This, data, len) \
    ((This) && (data) && (len) > 0 ? IShareBuffer_Write(This, data, len) : 0)

#define IShareBuffer_SafeRead(This, buf, len) \
    ((This) && (buf) && (len) > 0 ? IShareBuffer_Read(This, buf, len) : 0)

/*
 * Reference counting helpers
 */
#define IIpcHandle_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IIpcHandle_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

#define IIpcConnection_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IIpcConnection_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

#define IIpcServer_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IIpcServer_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

#define IIpcFactory_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IIpcFactory_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Debugging helpers
 */
#ifdef _DEBUG
static inline void IIpcHandle_DebugInfo_C(IIpcHandle* pThis)
{
    if (!pThis) {
        printf("IpcHandle: NULL\n");
        return;
    }
    
    printf("IpcHandle: %p\n", pThis);
    printf("  Connected: %s\n", IIpcHandle_IsConnected(pThis) ? "Yes" : "No");
    printf("  Local ID: %lu\n", IIpcHandle_GetLocalId(pThis));
    printf("  Remote ID: %lu\n", IIpcHandle_GetRemoteId(pThis));
    printf("  Connection: %p\n", IIpcHandle_GetIpcConnection(pThis));
}

static inline void IIpcServer_DebugInfo_C(IIpcServer* pThis)
{
    if (!pThis) {
        printf("IpcServer: NULL\n");
        return;
    }
    
    printf("IpcServer: %p\n", pThis);
}
#else
#define IIpcHandle_DebugInfo_C(This) ((void)0)
#define IIpcServer_DebugInfo_C(This) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SIPCOBJ_CAPI_H__ */
