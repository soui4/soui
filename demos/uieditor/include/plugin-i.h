#ifndef __PLUGIN_I_H__
#define __PLUGIN_I_H__
#include <SApp.h>

SNSBEGIN
typedef BOOL (* FUN_REG_EXCTRLS)(SApplication *pApp);

EXTERN_C BOOL  SOUI_COM_API RegisterExCtrls(SApplication *pApp);

SNSEND    
#endif//__PLUGIN_I_H__