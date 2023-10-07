#ifndef __SMENU_I__H__
#define __SMENU_I__H__

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

    /**
     * @brief attach到hmenu
     * @param hMenu
     * @return TRUE-SUCCEED
     */
    STDMETHOD_(BOOL, Attach)(THIS_ HMENU hMenu) PURE;

    /**
     * @brief detach and release its hmemu
     * @return hmenu
     */
    STDMETHOD_(HMENU, Detach)(THIS) PURE;

    /**
     * @brief get hmenu
     * @return hmenu
     */
    STDMETHOD_(HMENU, GetHMenu)(THIS) SCONST PURE;

    /**
     * @brief load menu from resouce id
     * @param resId resource id
     * @return TRUE-SUCCEED
     */
    STDMETHOD_(BOOL, LoadMenu)(THIS_ LPCTSTR resId) PURE;

    /**
     * @brief load menu from resouce id
     * @param resId resource id
     * @return TRUE-SUCCEED
     */
    STDMETHOD_(BOOL, LoadMenuU8)(THIS_ LPCSTR resId) PURE;

    /**
     * @brief load menu from IXmlNode object
     * @param xmlMenu IXmlNode object
     * @return
     */
    STDMETHOD_(BOOL, LoadMenu2)(THIS_ IXmlNode * xmlMenu) PURE;

    /**
     * @brief set icon skin of items
     * @param icons ISkinObj object that contain a goup of icons
     * @return void
     */
    STDMETHOD_(void, SetIconSkin)(THIS_ ISkinObj * icons) PURE;

    /**
     * @brief insert a menu item
     * @param uPosition insert position
     * @param uFlags flags, MF_BYCOMMAND or MF_BYPOSITION
     * @param nIDNewItem item id
     * @param strText text
     * @param iIcon icon idex
     * @param hIcon hicon
     * @return TRUE-SUCCEED
     * @remark if hIcon is not null, iIcon will be invalid. hIcon will be delete by DestroyIcon after the menu exit.
     */
    STDMETHOD_(BOOL, InsertMenu)(THIS_ UINT uPosition, UINT uFlags, UINT_PTR nIDNewItem, LPCTSTR strText, int iIcon DEF_VAL(-1), HICON hIcon DEF_VAL(0)) PURE;

    /**
     * @brief append a menu item
     * @param uFlags flags, MF_BYCOMMAND or MF_BYPOSITION
     * @param uIDNewItem item id
     * @param strText text
     * @param iIcon icon idex
     * @param hIcon hicon
     * @return TRUE-SUCCEED
     * @remark if hIcon is not null, iIcon will be invalid. hIcon will be delete by DestroyIcon after the menu exit.
     */
    STDMETHOD_(BOOL, AppendMenu)(THIS_ UINT uFlags, UINT_PTR uIDNewItem, LPCTSTR strText, int iIcon DEF_VAL(-1), HICON hIcon DEF_VAL(0)) PURE;

    /**
     * @brief set check flag for a item
     * @param uIdCheckItem item id or index
     * @param uCheck MF_BYCOMMAND/MF_BYPOSITION | MF_CHECKED/MF_UNCHECKED
     * @return TRUE-SUCCEED
     */
    STDMETHOD_(BOOL, CheckMenuItem)(THIS_ UINT uIdCheckItem, UINT uCheck) PURE;

    /**
     * @brief check menu radio items
     * @param idFirst first item
     * @param idLast  last item
     * @param uCheck the checked item
     * @param uFlags Value specifying the meaning of idFirst, idLast, and idCheck. If this parameter is MF_BYCOMMAND, the other parameters specify menu item identifiers. If it is MF_BYPOSITION, the other parameters specify the menu item positions.
     * @return TRUE-SUCCEED
     */
    STDMETHOD_(BOOL, CheckMenuRadioItem)(THIS_ UINT idFirst, UINT idLast, UINT idCheck, UINT uFlags) PURE;

    /**
     * @brief delete a item
     * @param uPosition pos
     * @param uFlags flag, MF_BYCOMMAND or MF_BYPOSITION
     * @return
     */
    STDMETHOD_(BOOL, DeleteMenu)(THIS_ UINT uPosition, UINT uFlags) PURE;

    /**
     * @brief show popup menu
     * @param uFlags flags
     * @param x pos x
     * @param y pos y
     * @param hWnd owner, messages will be sent to the owner.
     * @param prcRect limit rect
     * @param nScale  scale. default to 100
     * @return depends on uFlag.
     */
    STDMETHOD_(UINT, TrackPopupMenu)(THIS_ UINT uFlags, int x, int y, HWND hWnd, LPCRECT prcRect DEF_VAL(NULL), int nScale DEF_VAL(100)) PURE;

    /**
     * @brief destroy the menu object
     * @return
     */
    STDMETHOD_(void, DestroyMenu)(THIS) PURE;

    /**
     * @brief modify item text
     * @param uPosition item pos
     * @param uFlags flag for pos
     * @param lpItemString new string
     * @return
     */
    STDMETHOD_(BOOL, ModifyMenuString)(THIS_ UINT uPosition, UINT uFlags, LPCTSTR lpItemString) PURE;

    /**
     * @brief set item user data
     * @param uPosition pos
     * @param uFlags
     * @param ulUserData
     * @return
     */
    STDMETHOD_(BOOL, SetMenuUserData)(THIS_ UINT uPosition, UINT uFlags, ULONG_PTR ulUserData) PURE;

    /**
     * @brief get item user data
     * @param uPosition
     * @param uFlags
     * @return user data
     */
    STDMETHOD_(ULONG_PTR, GetMenuUserData)(THIS_ UINT uPosition, UINT uFlags) PURE;

    /**
     * @brief get menu context id
     * @return menu context id
     */
    STDMETHOD_(DWORD, GetContextHelpId)(THIS) SCONST PURE;

    /**
     * @brief set menu context id
     * @param dwId context id
     * @return
     */
    STDMETHOD_(void, SetContextHelpId)(THIS_ DWORD dwId) PURE;

    /**
     * @brief get sub menu
     * @param nPos sub menu idex
     * @return hmenu
     */
    STDMETHOD_(HMENU, GetSubMenu)(THIS_ int nPos) PURE;

    /**
     * @brief get menu text
     * @param uPosition pos
     * @param uFlags
     * @param lpItemString output string
     * @return BOOL
     */
    STDMETHOD_(BOOL, GetMenuString)(THIS_ UINT uPosition, UINT uFlags, IStringT * lpItemString) PURE;
};

SNSEND
#endif // __SMENU_I__H__