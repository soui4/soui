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
     * @brief Associate with hmenu
     * @param hMenu - menu handle
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, Attach)(THIS_ HMENU hMenu) PURE;

    /**
     * @brief Detach and release its hmenu
     * @return HMENU - menu handle
     */
    STDMETHOD_(HMENU, Detach)(THIS) PURE;

    /**
     * @brief Get hmenu
     * @return HMENU - menu handle
     */
    STDMETHOD_(HMENU, GetHMenu)(THIS) SCONST PURE;

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
     * @brief Set icon skin for menu item
     * @param icons - ISkinObj object containing a set of icons
     * @return void
     */
    STDMETHOD_(void, SetIconSkin)(THIS_ ISkinObj * icons) PURE;

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
    STDMETHOD_(BOOL, InsertMenu)(THIS_ UINT uPosition, UINT uFlags, UINT_PTR nIDNewItem, LPCTSTR strText, int iIcon DEF_VAL(-1), HICON hIcon DEF_VAL(0)) PURE;

    /**
     * @brief Append a menu item
     * @param uFlags - flag
     * @param uIDNewItem - new item ID
     * @param lpNewItem - new item text
     * @param iIcon - icon index
     * @param hIcon - icon handle
     * @return BOOL - returns TRUE on success, FALSE on failure
     * @remark If hIcon is not NULL, iIcon is ignored. hIcon will be deleted via DestroyIcon after the menu exits.
     */
    STDMETHOD_(BOOL, AppendMenu)(THIS_ UINT uFlags, UINT_PTR uIDNewItem, LPCTSTR strText, int iIcon DEF_VAL(-1), HICON hIcon DEF_VAL(0)) PURE;

    /**
     * @brief Enable menu item
     * @param uIdEnableItem - enabled item ID
     * @param uEnable - enable flag
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, EnableMenuItem)(THIS_ UINT uIDEnableItem, UINT uEnable) PURE;

    /**
     * @brief Check menu radio item
     * @param idFirst - first ID
     * @param idLast - last ID
     * @param idCheck - checked ID
     * @param uFlags - flag
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, CheckMenuItem)(THIS_ UINT uIdCheckItem, UINT uCheck) PURE;

    /**
     * @brief Check menu item
     * @param uIdCheckItem - checked item ID
     * @param uCheck - check flag
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, CheckMenuRadioItem)(THIS_ UINT idFirst, UINT idLast, UINT idCheck, UINT uFlags) PURE;

    /**
     * @brief Delete menu item
     * @param uPosition - position
     * @param uFlags - flag
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, DeleteMenu)(THIS_ UINT uPosition, UINT uFlags) PURE;

    /**
     * @brief Popup menu
     * @param uFlags - flag
     * @param x - X coordinate
     * @param y - Y coordinate
     * @param hWnd - window handle
     * @param nScale - zoom ratio
     * @return UINT - selected menu item ID
     */
    STDMETHOD_(UINT, TrackPopupMenu)(THIS_ UINT uFlags, int x, int y, HWND hWnd, LPCRECT prcRect DEF_VAL(NULL), int nScale DEF_VAL(100)) PURE;

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
    STDMETHOD_(DWORD, GetContextHelpId)(THIS) SCONST PURE;

    /**
     * @brief Set context help ID
     * @param dwId - context help ID
     * @return void
     */
    STDMETHOD_(void, SetContextHelpId)(THIS_ DWORD dwId) PURE;

    /**
     * @brief Get submenu
     * @param nPos - position
     * @return hmenu - submenu HMENU
     */
    STDMETHOD_(HMENU, GetSubMenu)(THIS_ int nPos) PURE;

    /**
     * @brief Get menu item text
     * @param uPosition - position
     * @param uFlags - flag
     * @param lpItemString - item text
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, GetMenuString)(THIS_ UINT uPosition, UINT uFlags, IStringT * lpItemString) PURE;

    /**
     * @brief Get menu item count
     * @return int - menu item count
     */
    STDMETHOD_(int, GetMenuItemCount)(CTHIS) SCONST PURE;
};

SNSEND
#endif /**< __SMENU_I__H__ */
