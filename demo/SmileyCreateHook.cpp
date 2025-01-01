#include "stdafx.h"
#include "SmileyCreateHook.h"
#if defined(_M_ARM64) || !defined(_WIN32)
#define Mhook_SetHook(a, b) (0)
#define Mhook_Unhook(a) (0);
#else
#include "mhook.h"
#endif


typedef HRESULT (STDAPICALLTYPE *_CoCreateInstance)(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv);


_CoCreateInstance TrueCoCreateInstance = (_CoCreateInstance)GetProcAddress(GetModuleHandle(_T("ole32")),"CoCreateInstance");



HRESULT STDAPICALLTYPE HookCoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv)
{
    if(rclsid == CLSID_SSmileyCtrl)
    {
#ifdef _WIN32
        return CreateSmiley(riid,ppv);
#endif
    }
    return TrueCoCreateInstance(rclsid,pUnkOuter,dwClsContext,riid,ppv);
}

typedef HRESULT (STDAPICALLTYPE * _ProgIDFromCLSID) (REFCLSID clsid, LPOLESTR FAR* lplpszProgID);

_ProgIDFromCLSID TrueProgIDFromCLSID = (_ProgIDFromCLSID)GetProcAddress(GetModuleHandle(_T("ole32")),"ProgIDFromCLSID");

HRESULT STDAPICALLTYPE HookProgIDFromCLSID (REFCLSID clsid, LPOLESTR FAR* lplpszProgID)
{
    if(clsid == CLSID_SSmileyCtrl)
    {
        const WCHAR KProgID[] = L"SSmileyCtrl.NoCOM.1";
        *lplpszProgID = (LPOLESTR)CoTaskMemAlloc(sizeof(KProgID));
        wcscpy(*lplpszProgID,KProgID);
        return S_OK;
    }
    return TrueProgIDFromCLSID(clsid,lplpszProgID);
}


SmileyCreateHook::SmileyCreateHook(void)
{
    if (!Mhook_SetHook((PVOID*)&TrueCoCreateInstance, HookCoCreateInstance))
    {
        SASSERT(FALSE);
    }

    if (!Mhook_SetHook((PVOID*)&TrueProgIDFromCLSID, HookProgIDFromCLSID))
    {
        SASSERT(FALSE);
    }
}

SmileyCreateHook::~SmileyCreateHook(void)
{
    Mhook_Unhook((PVOID*)&TrueProgIDFromCLSID);
    Mhook_Unhook((PVOID*)&TrueCoCreateInstance);

}
