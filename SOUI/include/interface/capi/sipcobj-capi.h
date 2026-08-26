#ifndef __SIPCOBJ_CAPI_H__
#define __SIPCOBJ_CAPI_H__

#include "../sipcobj-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for IPC Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 *
 * Note: IShareBuffer and IFunParams are plain C++ virtual structs (no lpVtbl).
 *       IIpcHandle, IIpcConnection, IIpcServer, IIpcFactory inherit IObjRef.
 */

/* IPC constants */
#define IPC_FUN_ID_CONNECT      FUN_ID_CONNECT
#define IPC_FUN_ID_DISCONNECT   FUN_ID_DISCONNECT
#define IPC_FUN_ID_START        FUN_ID_START

#define IPC_UM_CALL_FUN        UM_CALL_FUN

/* IShareBuffer seek modes */
#define IPC_SEEK_SET    0
#define IPC_SEEK_CUR    1
#define IPC_SEEK_END    2

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

#ifdef __cplusplus
}
#endif

#endif /* __SIPCOBJ_CAPI_H__ */
