#ifndef __SMESSAGEBOX_CAPI_H__
#define __SMESSAGEBOX_CAPI_H__

#include "../SMessageBox-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SMessageBox
 * SMessageBox is a C function exported by SOUI.
 */
#define SOUI_MessageBox(hWnd, lpText, lpCaption, uType) \
    SMessageBox(hWnd, lpText, lpCaption, uType)

#ifdef __cplusplus
}
#endif

#endif /* __SMESSAGEBOX_CAPI_H__ */
