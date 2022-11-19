#pragma once

#include <interface/obj-ref-i.h>
#include <interface/sxml-i.h>
#include <interface/SSkinobj-i.h>
#include <soui_exp.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IMenuEx
DECLARE_INTERFACE_(IMenuEx, IObjRef)
{

    //!添加引用
    /*!
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //!释放引用
    /*!
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //!释放对象
    /*!
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    //=================================================================
    STDMETHOD_(BOOL, LoadMenu)(THIS_ LPCTSTR resId) PURE;

	STDMETHOD_(BOOL, LoadMenuA)(THIS_ LPCSTR resId) PURE;

    STDMETHOD_(BOOL, LoadMenu2)(THIS_ IXmlNode * xmlMenu) PURE;

    STDMETHOD_(BOOL, InsertMenu)(THIS_ UINT uPosition, UINT uFlags, int id, LPCTSTR strText, int iIcon DEF_VAL(-1)) PURE;

    STDMETHOD_(BOOL, AppendMenu)(THIS_ UINT uFlags, int id, LPCTSTR lpNewItem, int iIcon DEF_VAL(-1)) PURE;

    STDMETHOD_(BOOL, CheckMenuItem)(THIS_ UINT uIdCheckItem, UINT uCheck) PURE;

    STDMETHOD_(BOOL, CheckMenuRadioItem)(THIS_ UINT idFirst, UINT idLast, UINT idCheck, UINT uFlags) PURE;

    STDMETHOD_(BOOL, DeleteMenu)(THIS_ UINT uPosition, UINT uFlags) PURE;

    STDMETHOD_(UINT, TrackPopupMenu)(THIS_ UINT uFlags, int x, int y, HWND hWnd, int nScale DEF_VAL(100)) PURE;

    STDMETHOD_(void, DestroyMenu)(THIS) PURE;

    STDMETHOD_(BOOL, ModifyMenuString)(THIS_ UINT uPosition, UINT uFlags, LPCTSTR lpItemString) PURE;

    STDMETHOD_(BOOL, SetMenuUserData)(THIS_ UINT uPosition, UINT uFlags, ULONG_PTR ulUserData) PURE;

    STDMETHOD_(ULONG_PTR, GetMenuUserData)(THIS_ UINT uPosition, UINT uFlags) PURE;

    STDMETHOD_(DWORD, GetContextHelpId)(CTHIS) SCONST PURE;

    STDMETHOD_(void, SetContextHelpId)(THIS_ DWORD dwId) PURE;

    STDMETHOD_(IMenuEx *, GetSubMenu)(THIS_ int nPos) PURE;
};

EXTERN_C void SOUI_EXP EndMenuEx(int nCmdId);

SNSEND