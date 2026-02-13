#include <souistd.h>
#include "../include/plugin-i.h"
#include <SCtrlsRegister.h>
SNSBEGIN

SOUI_COM_C BOOL SOUI_COM_API RegisterExCtrls(SApplication *pApp)
{
    SCtrlsRegister::RegisterCtrls(pApp);
    return TRUE;
}

SNSEND