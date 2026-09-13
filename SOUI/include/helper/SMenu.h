#ifndef __SMENU__H__
#define __SMENU__H__

#include <windows.h>
#include <sobject/Sobject.hpp>
#include <core/SNativeWnd.h>
#include <res.mgr/SSkinPool.h>
#include <helper/obj-ref-impl.hpp>
#include <interface/smenu-i.h>

SNSBEGIN

#define CX_ICON 16 /**< Width of supported icons */
#define CY_ICON 16 /**< Height of supported icons */

/**
 * @class SMenuAttr
 * @brief Menu attribute class
 */
class SMenuAttr : public TObjRefImpl<SObject> {
    friend class SMenu;
    friend class SMenuODWnd;
    DEF_SOBJECT(TObjRefImpl<SObject>, L"menuattr")

  public:
    /**
     * @brief Constructor
     */
    SMenuAttr();

    /**
     * @brief Destructor
     */
    ~SMenuAttr();

    /**
     * @brief Set scale
     * @param scale Scale
     */
    void SetScale(int scale);

    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"itemSkin", m_pItemSkin, FALSE)           /**< Menu item skin, containing 2 states: normal state + selected state */
        ATTR_SKIN(L"iconSkin", m_pIconSkin, FALSE)           /**< Menu icon */
        ATTR_SKIN(L"sepSkin", m_pSepSkin, FALSE)             /**< Separator bar skin */
        ATTR_SKIN(L"checkSkin", m_pCheckSkin, FALSE)         /**< Selected state, containing two states: checked + radio-selected */
        ATTR_LAYOUTSIZE(L"itemHeight", m_nItemHei, FALSE)    /**< Menu item height */
        ATTR_LAYOUTSIZE(L"iconMargin", m_nIconMargin, FALSE) /**< Icon margin space */
        ATTR_LAYOUTSIZE(L"textMargin", m_nTextMargin, FALSE) /**< Text margin space */
        ATTR_LAYOUTSIZE(L"maxWidth", m_nMaxWidth, FALSE)     /**< Menu item max width */
        ATTR_LAYOUTSIZE2(L"iconSize", m_szIcon, FALSE)       /**< Icon size */
        ATTR_FONT(L"font", m_dpiFont, FALSE)                 /**< Font */
        ATTR_COLOR(L"colorText", m_crTxtNormal, FALSE)       /**< Normal text color */
        ATTR_COLOR(L"colorTextSel", m_crTxtSel, FALSE)       /**< Selected text color */
        ATTR_COLOR(L"colorTextGray", m_crTxtGray, FALSE)     /**< Grayed text color */
        ATTR_STRINGW(L"trCtx", m_strTrCtx, FALSE)            /**< Translation context */
    SOUI_ATTRS_END()

  protected:
    /**
     * @brief Get text margin space
     * @return Text margin space
     */
    int GetTextMargin();

    /**
     * @brief Get icon margin space
     * @return Icon margin space
     */
    int GetIconMargin();

    /**
     * @brief Get icon size
     * @return Icon size
     */
    CSize GetIconSize();

    /**
     * @brief Get menu item height
     * @return Menu item height
     */
    int GetItemHeight();

    /**
     * @brief Get menu item max width
     * @return Menu item max width
     */
    int GetMaxWidth();

    /**
     * @brief Get scale
     * @return Scale
     */
    int GetScale() const
    {
        return m_scale;
    }

    /**
     * @brief Get font pointer
     * @return Font pointer
     */
    SAutoRefPtr<IFontS> GetFontPtr();

  protected:
    /**
     * @brief Callback function after initialization completes
     * @param xmlNode XML node
     */
    virtual void WINAPI OnInitFinished(SXmlNode xmlNode);

    SAutoRefPtr<ISkinObj> m_pItemSkin;  /**< Menu item skin, containing 2 states: normal state + selected state */
    SAutoRefPtr<ISkinObj> m_pIconSkin;  /**< Menu icon */
    SAutoRefPtr<ISkinObj> m_pSepSkin;   /**< Separator bar skin */
    SAutoRefPtr<ISkinObj> m_pCheckSkin; /**< Selected state, containing two states: checked + radio-selected */
    SLayoutSize m_nItemHei;             /**< Menu item height */
    SLayoutSize m_nIconMargin;          /**< Icon margin space */
    SLayoutSize m_nTextMargin;          /**< Text margin space */
    COLORREF m_crTxtNormal;             /**< Normal text color */
    COLORREF m_crTxtSel;                /**< Selected text color */
    COLORREF m_crTxtGray;               /**< Grayed text color */
    SLayoutSize m_szIcon[2];            /**< Icon size */
    SDpiAwareFont m_dpiFont;            /**< Font */
    SStringW m_strTrCtx;                /**< Translation context */
    SLayoutSize m_nMaxWidth;            /**< Menu item max width */
    int m_scale;                        /**< Scale */
};

/**
 * @struct SMenuItemData
 * @brief Menu item data structure
 */
struct SMenuItemData
{
    /**
     * @brief Constructor
     */
    SMenuItemData();

    /**
     * @brief Destructor
     */
    ~SMenuItemData();

    int iIcon;            /**< Icon index */
    SStringT strText;     /**< Text */
    UINT vHotKey;         /**< Accelerator key */
    ULONG_PTR dwUserData; /**< User data */
    HICON hIcon;          /**< Icon handle */
};

/**
 * @class SOwnerDraw
 * @brief Owner-draw template class
 * @tparam T Derived class type
 */
template <class T>
class SOwnerDraw {
  public:
    /** Message map and handlers */
    BEGIN_MSG_MAP_EX(SOwnerDraw<T>)
        MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
        MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
        MESSAGE_HANDLER(WM_COMPAREITEM, OnCompareItem)
        MESSAGE_HANDLER(WM_DELETEITEM, OnDeleteItem)
        MESSAGE_HANDLER(WM_MENUCHAR, OnMenuChar)
        ALT_MSG_MAP(1)
        MESSAGE_HANDLER(OCM_DRAWITEM, OnDrawItem)
        MESSAGE_HANDLER(OCM_MEASUREITEM, OnMeasureItem)
        MESSAGE_HANDLER(OCM_COMPAREITEM, OnCompareItem)
        MESSAGE_HANDLER(OCM_DELETEITEM, OnDeleteItem)
    END_MSG_MAP()

    /**
     * @brief Handle WM_DRAWITEM message
     * @param uMsg Message ID
     * @param wParam Additional parameter 1
     * @param lParam Additional parameter 2
     * @param bHandled Whether the message is handled
     * @return Result
     */
    LRESULT OnDrawItem(UINT /**< uMsg */, WPARAM /**< wParam */, LPARAM lParam, BOOL &bHandled)
    {
        T *pT = static_cast<T *>(this);
        pT->SetMsgHandled(TRUE);
        pT->DrawItem((LPDRAWITEMSTRUCT)lParam);
        bHandled = pT->IsMsgHandled();
        return (LRESULT)TRUE;
    }

    /**
     * @brief Handle WM_MEASUREITEM message
     * @param uMsg Message ID
     * @param wParam Additional parameter 1
     * @param lParam Additional parameter 2
     * @param bHandled Whether the message is handled
     * @return Result
     */
    LRESULT OnMeasureItem(UINT /**< uMsg */, WPARAM /**< wParam */, LPARAM lParam, BOOL &bHandled)
    {
        T *pT = static_cast<T *>(this);
        pT->SetMsgHandled(TRUE);
        pT->MeasureItem((LPMEASUREITEMSTRUCT)lParam);
        bHandled = pT->IsMsgHandled();
        return (LRESULT)TRUE;
    }

    /**
     * @brief Handle WM_COMPAREITEM message
     * @param uMsg Message ID
     * @param wParam Additional parameter 1
     * @param lParam Additional parameter 2
     * @param bHandled Whether the message is handled
     * @return Result
     */
    LRESULT OnCompareItem(UINT /**< uMsg */, WPARAM /**< wParam */, LPARAM lParam, BOOL &bHandled)
    {
        T *pT = static_cast<T *>(this);
        pT->SetMsgHandled(TRUE);
        bHandled = pT->IsMsgHandled();
        return (LRESULT)pT->CompareItem((LPCOMPAREITEMSTRUCT)lParam);
    }

    /**
     * @brief Handle WM_DELETEITEM message
     * @param uMsg Message ID
     * @param wParam Additional parameter 1
     * @param lParam Additional parameter 2
     * @param bHandled Whether the message is handled
     * @return Result
     */
    LRESULT OnDeleteItem(UINT /**< uMsg */, WPARAM /**< wParam */, LPARAM lParam, BOOL &bHandled)
    {
        T *pT = static_cast<T *>(this);
        pT->SetMsgHandled(TRUE);
        pT->DeleteItem((LPDELETEITEMSTRUCT)lParam);
        bHandled = pT->IsMsgHandled();
        return (LRESULT)TRUE;
    }

    /**
     * @brief Handle WM_MENUCHAR message
     * @param uMsg Message ID
     * @param wParam Additional parameter 1
     * @param lParam Additional parameter 2
     * @param bHandled Whether the message is handled
     * @return Result
     */
    LRESULT OnMenuChar(UINT /**< uMsg */, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
    {
        T *pT = static_cast<T *>(this);
        pT->SetMsgHandled(TRUE);

        UINT nChar = LOWORD(wParam);
        UINT nFlags = HIWORD(wParam);
        HMENU hMenu = (HMENU)lParam;
        LRESULT lRes = pT->OnMenuChar(nChar, nFlags, hMenu);

        bHandled = pT->IsMsgHandled();
        return lRes;
    }

    /** Overrideables */
    /**
     * @brief Draw menu item
     * @param lpDrawItemStruct Draw item structure
     */
    void DrawItem(LPDRAWITEMSTRUCT /**< lpDrawItemStruct */)
    {
        // must be implemented
        SASSERT(FALSE);
    }

    /**
     * @brief Measure menu item
     * @param lpMeasureItemStruct Measure item structure
     */
    void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
    {
        if (lpMeasureItemStruct->CtlType != ODT_MENU)
        {
            // return default height for a system font
            T *pT = static_cast<T *>(this);
            HWND hWnd = pT->GetDlgItem(lpMeasureItemStruct->CtlID);
            HDC dc = ::GetDC(hWnd);
            TEXTMETRIC tm = { 0 };
            GetTextMetrics(dc, &tm);

            lpMeasureItemStruct->itemHeight = tm.tmHeight;
            ::ReleaseDC(hWnd, dc);
        }
        else
            lpMeasureItemStruct->itemHeight = ::GetSystemMetrics(SM_CYMENU);
    }

    /**
     * @brief Compare menu items
     * @param lpCompareItemStruct Compare item structure
     * @return Comparison result
     */
    int CompareItem(LPCOMPAREITEMSTRUCT /**< lpCompareItemStruct */)
    {
        // all items are equal
        return 0;
    }

    /**
     * @brief Delete menu item
     * @param lpDeleteItemStruct Delete item structure
     */
    void DeleteItem(LPDELETEITEMSTRUCT /**< lpDeleteItemStruct */)
    {
        // default - nothing
    }
};

/**
 * @class SMenuODWnd
 * @brief Owner-draw menu window class
 */
class SMenuODWnd
    : public SNativeWnd
    , public SOwnerDraw<SMenuODWnd> {
    friend class SOwnerDraw<SMenuODWnd>;

  public:
    /**
     * @brief Constructor
     * @param hMenuOwner Menu owner window handle
     * @param pMenuAttr Pointer to menu attribute object
     */
    SMenuODWnd(HWND hMenuOwner, SMenuAttr *pMenuAttr);

    /**
     * @brief Destructor
     */
    ~SMenuODWnd();

  protected:
    /**
     * @brief Initialize menu
     * @param menu Menu handle
     */
    void OnInitMenu(HMENU menu);

    /**
     * @brief Initialize popup menu
     * @param menuPopup Popup menu handle
     * @param nIndex Index
     * @param bSysMenu Whether it is a system menu
     */
    void OnInitMenuPopup(HMENU menuPopup, UINT nIndex, BOOL bSysMenu);

    /**
     * @brief Draw menu item
     * @param lpDrawItemStruct Draw item structure
     */
    void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

    /**
     * @brief Measure menu item
     * @param lpMeasureItemStruct Measure item structure
     */
    void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);

    /**
     * @brief Handle menu selection message
     * @param nItemID Item ID
     * @param nFlags Flags
     * @param menu Menu handle
     */
    void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU menu);

    /**
     * @brief Handle menu character message
     * @param nChar Character
     * @param nFlags Flags
     * @param hMenu Menu handle
     * @return Result
     */
    LRESULT OnMenuChar(UINT nChar, UINT nFlags, HMENU hMenu);

    BEGIN_MSG_MAP_EX(SMenuODWnd)
        MSG_WM_INITMENU(OnInitMenu)
        MSG_WM_INITMENUPOPUP(OnInitMenuPopup)
        MSG_WM_MENUSELECT(OnMenuSelect)
        CHAIN_MSG_MAP(SOwnerDraw<SMenuODWnd>)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

    HWND m_hMenuOwner;             /**< Menu owner window handle */
    SAutoRefPtr<SMenuAttr> m_attr; /**< Pointer to menu attribute object */
};

/**
 * @class SMenu
 * @brief Menu class
 */
class SOUI_EXP SMenu : public TObjRefImpl<IMenu> {
  public:
    /**
     * @brief Copy constructor
     * @param src Source menu object
     */
    SMenu(const SMenu &src);

    /**
     * @brief Constructor
     * @param hMenu Menu handle (default is 0)
     */
    SMenu(HMENU hMenu = 0);

    /**
     * @brief Destructor
     */
    ~SMenu(void);

  public:
    HMENU m_hMenu; /**< Menu handle */

  public: /**< IMenu */
    /**
     * @brief Attach menu handle
     * @param hMenu Menu handle
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, Attach)(THIS_ HMENU hMenu) OVERRIDE;

    /**
     * @brief Detach menu handle
     * @return Detached menu handle
     */
    STDMETHOD_(HMENU, Detach)(THIS) OVERRIDE;

    /**
     * @brief Get menu handle
     * @return Menu handle
     */
    STDMETHOD_(HMENU, GetHMenu)(THIS) SCONST OVERRIDE;

    /**
     * @brief Load menu resource
     * @param resId Resource ID
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, LoadMenu)(THIS_ LPCTSTR resId) OVERRIDE;

    /**
     * @brief Load menu resource (UTF-8)
     * @param resId Resource ID
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, LoadMenuU8)(THIS_ LPCSTR resId) OVERRIDE;

    /**
     * @brief Load menu resource (XML)
     * @param xmlMenu XML node
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, LoadMenu2)(THIS_ IXmlNode *xmlMenu) OVERRIDE;

    /**
     * @brief Set menu icon skin
     * @param icons Pointer to icon skin object
     */
    STDMETHOD_(void, SetIconSkin)(THIS_ ISkinObj *icons) OVERRIDE;

    /**
     * @brief Insert menu item
     * @param uPosition Position
     * @param uFlags Flags
     * @param nIDNewItem New item ID
     * @param strText Text
     * @param iIcon Icon index (default is -1)
     * @param hIcon Icon handle (default is 0)
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, InsertMenu)(THIS_ UINT uPosition, UINT uFlags, UINT_PTR nIDNewItem, LPCTSTR strText, int iIcon DEF_VAL(-1), HICON hIcon DEF_VAL(0)) OVERRIDE;

    /**
     * @brief Append menu item
     * @param uFlags Flags
     * @param uIDNewItem New item ID
     * @param lpNewItem Text
     * @param iIcon Icon index (default is -1)
     * @param hIcon Icon handle (default is 0)
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, AppendMenu)(THIS_ UINT uFlags, UINT_PTR uIDNewItem, LPCTSTR lpNewItem, int iIcon DEF_VAL(-1), HICON hIcon DEF_VAL(0)) OVERRIDE;

    /**
     * @brief Enable or disable menu item
     * @param uIDEnableItem Item ID or index
     * @param uEnable MF_BYCOMMAND/MF_BYPOSITION | MF_ENABLED/MF_GRAYED/MF_DISABLED
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, EnableMenuItem)(THIS_ UINT uIDEnableItem, UINT uEnable) OVERRIDE;

    /**
     * @brief Check menu item
     * @param uIdCheckItem Menu item ID to check
     * @param uCheck Check flag (e.g., MF_CHECKED, MF_UNCHECKED)
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, CheckMenuItem)(THIS_ UINT uIdCheckItem, UINT uCheck) OVERRIDE;

    /**
     * @brief Check menu item (radio button)
     * @param idFirst First menu item ID
     * @param idLast Last menu item ID
     * @param idCheck Menu item ID to check
     * @param uFlags Check flag (e.g., MF_BYCOMMAND, MF_BYPOSITION)
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, CheckMenuRadioItem)(THIS_ UINT idFirst, UINT idLast, UINT idCheck, UINT uFlags) OVERRIDE;

    /**
     * @brief Delete menu item
     * @param uPosition Menu item position
     * @param uFlags Delete flag (e.g., MF_BYCOMMAND, MF_BYPOSITION)
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, DeleteMenu)(THIS_ UINT uPosition, UINT uFlags) OVERRIDE;

    /**
     * @brief Track popup menu
     * @param uFlags Track flag (e.g., TPM_LEFTALIGN, TPM_RIGHTALIGN)
     * @param x X coordinate of the menu's top-left corner
     * @param y Y coordinate of the menu's top-left corner
     * @param hWnd Parent window handle
     * @param prcRect Optional rectangle region
     * @param nScale Scale (default is 100)
     * @return Menu item ID
     */
    STDMETHOD_(UINT, TrackPopupMenu)(THIS_ UINT uFlags, int x, int y, HWND hWnd, LPCRECT prcRect DEF_VAL(NULL), int nScale DEF_VAL(100)) OVERRIDE;

    /**
     * @brief Destroy menu
     */
    STDMETHOD_(void, DestroyMenu)(THIS) OVERRIDE;

    /**
     * @brief Modify menu item string
     * @param uPosition Menu item position
     * @param uFlags Flags (e.g., MF_BYCOMMAND, MF_BYPOSITION)
     * @param lpItemString New menu item string
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, ModifyMenuString)(THIS_ UINT uPosition, UINT uFlags, LPCTSTR lpItemString) OVERRIDE;

    /**
     * @brief Set menu item user data
     * @param uPosition Menu item position
     * @param uFlags Flags (e.g., MF_BYCOMMAND, MF_BYPOSITION)
     * @param ulUserData User data
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, SetMenuUserData)(THIS_ UINT uPosition, UINT uFlags, ULONG_PTR ulUserData) OVERRIDE;

    /**
     * @brief Get menu item user data
     * @param uPosition Menu item position
     * @param uFlags Flags (e.g., MF_BYCOMMAND, MF_BYPOSITION)
     * @return User data
     */
    STDMETHOD_(ULONG_PTR, GetMenuUserData)(THIS_ UINT uPosition, UINT uFlags) OVERRIDE;

    /**
     * @brief Get context help ID
     * @return Context help ID
     */
    STDMETHOD_(DWORD, GetContextHelpId)(THIS) SCONST OVERRIDE;

    /**
     * @brief Set context help ID
     * @param dwId Context help ID
     */
    STDMETHOD_(void, SetContextHelpId)(THIS_ DWORD dwId) OVERRIDE;

    /**
     * @brief Get submenu
     * @param nPos Submenu position
     * @return Submenu handle
     */
    STDMETHOD_(HMENU, GetSubMenu)(THIS_ int nPos) OVERRIDE;

    /**
     * @brief Get menu item string
     * @param uPosition Menu item position
     * @param uFlags Flags (e.g., MF_BYCOMMAND, MF_BYPOSITION)
     * @param lpItemString Pointer to object receiving the menu item string
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, GetMenuString)(THIS_ UINT uPosition, UINT uFlags, IStringT *lpItemString) OVERRIDE;

    /**
     * @brief Get menu item count
     * @return Menu item count
     */
    STDMETHOD_(int, GetMenuItemCount)(CTHIS) SCONST OVERRIDE;

  protected:
    /**
     * @brief Update scale
     * @param nScale Scale
     */
    void UpdateScale(int nScale);

    /**
     * @brief Build menu
     * @param menuPopup Popup menu handle
     * @param xmlNode XML node
     */
    void BuildMenu(HMENU menuPopup, SXmlNode xmlNode);

    /**
     * @brief Initialize menu item data
     * @param itemInfo Pointer to menu item data
     * @param strText Menu item text
     */
    void InitMenuItemData(SMenuItemData *itemInfo, const SStringW &strText);

    /**
     * @brief Release menu item data
     * @param hMemu Menu handle
     */
    void FreeMenuItemData(HMENU hMemu);

    /**
     * @brief Get pointer to menu attribute object
     * @param hMenu Menu handle
     * @return Pointer to menu attribute object
     */
    SMenuAttr *GetMenuAttr(HMENU hMenu) const;

    /**
     * @brief Set pointer to menu attribute object
     * @param hMenu Menu handle
     * @param pMenuAttr Pointer to menu attribute object
     */
    void SetMenuAttr(HMENU hMenu, SMenuAttr *pMenuAttr) const;

    bool m_bAttached;              /**< Whether the menu handle is attached */
    SAutoRefPtr<ISkinObj> m_icons; /**< Pointer to icon skin object */
};

SNSEND
#endif /**< __SMENU__H__ */