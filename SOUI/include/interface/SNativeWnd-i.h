/**
 * @file       SNativeWnd-i.h
 * @brief      Interface for Native Window Operations
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/01
 *
 * @details    This file defines the interface for native window operations, including message handling and event arguments.
 */

#ifndef __SNATIVEWND_I__H__
#define __SNATIVEWND_I__H__

#include <interface/obj-ref-i.h>
#include <interface/SEvtArgs-i.h>

SNSBEGIN

/**
 * @typedef FunMsgHandler
 * @brief Function pointer type for message handlers.
 *
 * @details This function pointer type is used to define custom message handlers for native windows.
 *
 * @param pMsg Pointer to the message structure.
 * @param pRes Pointer to the result of the message processing.
 * @param ctx Context pointer for the message handler.
 * @return BOOL TRUE if the message was handled, FALSE otherwise.
 */
typedef BOOL (*FunMsgHandler)(const LPMSG pMsg, LRESULT *pRes, void *ctx);

/**
 * @struct MsgHandlerInfo
 * @brief Structure containing message handler information.
 *
 * @details This structure holds the function pointer and context for a message handler.
 */
typedef struct _MsgHandlerInfo
{
    FunMsgHandler fun; //!< Function pointer to the message handler.
    void *ctx;         //!< Context pointer for the message handler.
} MsgHandlerInfo;

/**
 * @interface INativeWnd
 * @brief Interface for Native Window Operations
 *
 * @details This interface defines the methods for managing native windows, including message handling and event arguments.
 */
#undef INTERFACE
#define INTERFACE INativeWnd
DECLARE_INTERFACE_(INativeWnd, IObjRef){
#include "SNativeWndApi.h"
};

SNSEND
#endif // __SNATIVEWND_I__H__