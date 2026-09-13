#ifndef __SMENUEX_I__H__
#define __SMENUEX_I__H__

#include <interface/obj-ref-i.h>
#include <interface/sxml-i.h>
#include <interface/SSkinobj-i.h>
#include <soui_exp.h>

SNSBEGIN

#define WM_INITMENUPOPUP_EX (WM_USER + 0x4004) /**<  Message sent to the window that owns the menu before the menu pops up */
#define WM_MENUSELECT_EX    (WM_USER + 0x4005) /**<  Message sent to the window that owns the menu when a menu item is selected */
#undef INTERFACE
#define INTERFACE IMenuEx
DECLARE_INTERFACE_(IMenuEx, IObjRef)
{

    /** Add reference */
    /**
     * @return long - current reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /** Release reference */
    /**
     * @return long - current reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /** Release object */
    /**
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    //=================================================================

    /**
     * @brief Load menu from resource ID
     * @param resId - resource ID
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, LoadMenu)(THIS_ LPCTSTR resId) PURE;

    /**
     * @brief Load menu from resource ID (UTF-8 encoding)
     * @param resId - resource ID
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, LoadMenuU8)(THIS_ LPCSTR resId) PURE;

    /**
     * @brief Load menu from IXmlNode object
     * @param xmlMenu - IXmlNode object
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, LoadMenu2)(THIS_ IXmlNode * xmlMenu) PURE;

    /**
     * @brief Insert a menu item
     * @param uPosition - insert position
     * @param uFlags - flag, MF_BYCOMMAND or MF_BYPOSITION
     * @param nIDNewItem - item ID
     * @param strText - text
     * @param iIcon - icon index
     * @param hIcon - icon handle
     * @return BOOL - returns TRUE on success, FALSE on failure
     * @remark If hIcon is not NULL, iIcon is ignored. hIcon will be deleted via DestroyIcon after the menu exits.
     */
    STDMETHOD_(BOOL, InsertMenu)(THIS_ UINT uPosition, UINT uFlags, int id, LPCTSTR strText, int iIcon DEF_VAL(-1)) PURE;

    /**
     * @brief Append a menu item
     * @param uFlags - flag, MF_BYCOMMAND or MF_BYPOSITION
     * @param uIDNewItem - new item ID
     * @param strText - text
     * @param iIcon - icon index
     * @param hIcon - icon handle
     * @return BOOL - returns TRUE on success, FALSE on failure
     * @remark If hIcon is not NULL, iIcon is ignored. hIcon will be deleted via DestroyIcon after the menu exits.
     */
    STDMETHOD_(BOOL, AppendMenu)(THIS_ UINT uFlags, int id, LPCTSTR lpNewItem, int iIcon DEF_VAL(-1)) PURE;

    /**
     * @brief Set menu item check flag
     * @param uIdCheckItem - item ID or index
     * @param uCheck - MF_BYCOMMAND/MF_BYPOSITION | MF_CHECKED/MF_UNCHECKED
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, CheckMenuItem)(THIS_ UINT uIdCheckItem, UINT uCheck) PURE;

    /**
     * @brief Check menu radio item
     * @param idFirst - first ID
     * @param idLast - last ID
     * @param uCheck - checked ID
     * @param uFlags - flag, specifies the meaning of idFirst, idLast and idCheck. If this parameter is MF_BYCOMMAND, the other parameters specify menu item identifiers. If MF_BYPOSITION, the other parameters specify menu item positions.
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, CheckMenuRadioItem)(THIS_ UINT idFirst, UINT idLast, UINT idCheck, UINT uFlags) PURE;

    /**
     * @brief Enable or disable menu item
     * @param uIDEnableItem - item ID or index
     * @param uEnable - MF_BYCOMMAND/MF_BYPOSITION | MF_ENABLED/MF_GRAYED/MF_DISABLED
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, EnableMenuItem)(THIS_ UINT uIDEnableItem, UINT uEnable) PURE;

    /**
     * @brief Delete menu item
     * @param uPosition - position
     * @param uFlags - flag, MF_BYCOMMAND or MF_BYPOSITION
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, DeleteMenu)(THIS_ UINT uPosition, UINT uFlags) PURE;

    /**
     * @brief Popup menu
     * @param uFlags - flag
     * @param x - X coordinate
     * @param y - Y coordinate
     * @param hWnd - window handle
     * @param nScale - zoom ratio, default value is 100
     * @return UINT - selected menu item ID
     */
    STDMETHOD_(UINT, TrackPopupMenu)(THIS_ UINT uFlags, int x, int y, HWND hWnd, int nScale DEF_VAL(100)) PURE;

    /**
     * @brief Destroy menu
     * @return void
     */
    STDMETHOD_(void, DestroyMenu)(THIS) PURE;

    /**
     * @brief Modify menu item text
     * @param uPosition - position
     * @param uFlags - flag
     * @param lpItemString - item text
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, ModifyMenuString)(THIS_ UINT uPosition, UINT uFlags, LPCTSTR lpItemString) PURE;

    /**
     * @brief Set menu item user data
     * @param uPosition - position
     * @param uFlags - flag
     * @param ulUserData - user data
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, SetMenuUserData)(THIS_ UINT uPosition, UINT uFlags, ULONG_PTR ulUserData) PURE;

    /**
     * @brief Get menu item user data
     * @param uPosition - position
     * @param uFlags - flag
     * @return ULONG_PTR - user data
     */
    STDMETHOD_(ULONG_PTR, GetMenuUserData)(THIS_ UINT uPosition, UINT uFlags) PURE;

    /**
     * @brief Get context help ID
     * @return DWORD - context help ID
     */
    STDMETHOD_(DWORD, GetContextHelpId)(CTHIS) SCONST PURE;

    /**
     * @brief Set context help ID
     * @param dwId - context help ID
     * @return void
     */
    STDMETHOD_(void, SetContextHelpId)(THIS_ DWORD dwId) PURE;

    /**
     * @brief Get submenu
     * @param nPos - submenu index
     * @return IMenuEx* - submenu pointer
     */
    STDMETHOD_(IMenuEx *, GetSubMenu)(THIS_ int nPos) PURE;

    /**
     * @brief Get menu item text
     * @param uPosition - position
     * @param uFlags - flag
     * @param lpItemString - item text
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, GetMenuString)(THIS_ UINT uPosition, UINT uFlags, IStringT * lpItemString) PURE;

    /**
     * @brief Get host window pointer
     * @return IHostWnd* - host window pointer
     */
    STDMETHOD_(IHostWnd *, GetHostWnd)(THIS) PURE;

    /**
     * @brief Get menu item count
     * @return int - menu item count
     */
    STDMETHOD_(int, GetMenuItemCount)(CTHIS) SCONST PURE;
};

EXTERN_C void SOUI_EXP EndMenuEx(int nCmdId);

SNSEND
#endif /**< __SMENUEX_I__H__ */
