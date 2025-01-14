#include "stdafx.h"
#include "SmileyCreateHook.h"
#include <detours.h>

typedef HRESULT (STDAPICALLTYPE *_CoCreateInstance)(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv);


static _CoCreateInstance TrueCoCreateInstance = (_CoCreateInstance)GetProcAddress(GetModuleHandle(_T("ole32")),"CoCreateInstance");


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
#ifdef _WIN32
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID &)TrueCoCreateInstance, HookCoCreateInstance);
    DetourAttach(&(PVOID &)TrueProgIDFromCLSID, HookProgIDFromCLSID);
    LONG error = DetourTransactionCommit();
    SASSERT(error == 0);
#endif//_WIN32
}

SmileyCreateHook::~SmileyCreateHook(void)
{
#ifdef _WIN32
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID &)TrueCoCreateInstance, HookCoCreateInstance);
    DetourDetach(&(PVOID &)TrueProgIDFromCLSID, HookProgIDFromCLSID);
    LONG error = DetourTransactionCommit();
    SASSERT(error == 0);
#endif //_WIN32
}
