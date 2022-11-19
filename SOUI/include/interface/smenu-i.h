#pragma once

#include <interface/obj-ref-i.h>
#include <interface/sxml-i.h>
#include <interface/SSkinobj-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IMenu
DECLARE_INTERFACE_(IMenu, IObjRef)
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
    STDMETHOD_(BOOL, Attach)(THIS_ HMENU hMenu) PURE;

    STDMETHOD_(HMENU, Detach)(THIS) PURE;

    STDMETHOD_(HMENU, GetHMenu)(THIS) SCONST PURE;

    STDMETHOD_(BOOL, LoadMenu)(THIS_ LPCTSTR resId) PURE;

	STDMETHOD_(BOOL, LoadMenuA)(THIS_ LPCSTR resId) PURE;

    STDMETHOD_(BOOL, LoadMenu2)(THIS_ IXmlNode * xmlMenu) PURE;

    STDMETHOD_(void, SetIconSkin)(THIS_ ISkinObj * icons) PURE;

    /**
     * SMenu::InsertMenu
     * @brief    插入一个菜单项,参数和Windows API类似。
     * @param    int iIcon -- 显示的标图在iconSkin中的索引
     * @param    HICON hIcon -- 标图，默认为0，有值时，iIocn失效
     * @return   BOOL - TRUE:成功
     *
     * Describe  hIcon会在菜单退出时自动调用DestroyIcon.
     */
    STDMETHOD_(BOOL, InsertMenu)(THIS_ UINT uPosition, UINT uFlags, UINT_PTR nIDNewItem, LPCTSTR strText, int iIcon DEF_VAL(-1), HICON hIcon DEF_VAL(0)) PURE;

    STDMETHOD_(BOOL, AppendMenu)(THIS_ UINT uFlags, UINT_PTR uIDNewItem, LPCTSTR lpNewItem, int iIcon DEF_VAL(-1), HICON hIcon DEF_VAL(0)) PURE;

    STDMETHOD_(BOOL, CheckMenuItem)(THIS_ UINT uIdCheckItem, UINT uCheck) PURE;

    STDMETHOD_(BOOL, CheckMenuRadioItem)(THIS_ UINT idFirst, UINT idLast, UINT idCheck, UINT uFlags) PURE;

    STDMETHOD_(BOOL, DeleteMenu)(THIS_ UINT uPosition, UINT uFlags) PURE;

    STDMETHOD_(UINT, TrackPopupMenu)(THIS_ UINT uFlags, int x, int y, HWND hWnd, LPCRECT prcRect DEF_VAL(NULL), int nScale DEF_VAL(100)) PURE;

    STDMETHOD_(void, DestroyMenu)(THIS) PURE;

    STDMETHOD_(BOOL, ModifyMenuString)(THIS_ UINT uPosition, UINT uFlags, LPCTSTR lpItemString) PURE;

    STDMETHOD_(BOOL, SetMenuUserData)(THIS_ UINT uPosition, UINT uFlags, ULONG_PTR ulUserData) PURE;

    STDMETHOD_(ULONG_PTR, GetMenuUserData)(THIS_ UINT uPosition, UINT uFlags) PURE;

    STDMETHOD_(DWORD, GetContextHelpId)(THIS) SCONST PURE;

    STDMETHOD_(void, SetContextHelpId)(THIS_ DWORD dwId) PURE;

    STDMETHOD_(HMENU, GetSubMenu)(THIS_ int nPos) PURE;
};

SNSEND