#ifndef __SMENUEX__H__
#define __SMENUEX__H__

#include <core/SHostWnd.h>
#include <interface/smenuex-i.h>

SNSBEGIN

class SMenuExRoot;
class SMenuEx;

/**
 * @class SMenuExItem
 * @brief Extended menu item class
 */
class SOUI_EXP SMenuExItem : public SWindow {
    DEF_SOBJECT(SWindow, L"menuItem")
    friend class SMenuEx;

  public:
    /**
     * @brief Constructor
     * @param pOwnerMenu Pointer to owner menu object
     * @param pItemSkin Pointer to menu item skin object
     */
    SMenuExItem(SMenuEx *pOwnerMenu, ISkinObj *pItemSkin);

    /**
     * @brief Destructor
     */
    ~SMenuExItem();

    /**
     * @brief Get submenu
     * @return Pointer to submenu object
     */
    SMenuEx *GetSubMenu();

    /**
     * @brief Get owner menu
     * @return Pointer to owner menu object
     */
    SMenuEx *GetOwnerMenu();

    /**
     * @brief Hide submenu
     */
    void HideSubMenu();

    /**
     * @brief Show submenu
     * @param bCheckFirstItem Whether to select the first submenu item
     */
    void ShowSubMenu(BOOL bCheckFirstItem);

    /**
     * @brief Get hotkey character
     * @return Hotkey character
     */
    WCHAR GetHotKey() const;

  protected:
    void BeforePaint(IRenderTarget *pRT, SPainter &painter) const override;

    /**
     * @brief Handle submenu hidden event
     * @param bUncheckItem Whether to uncheck the submenu item
     */
    void OnSubMenuHided(BOOL bUncheckItem);

    /**
     * @brief Create child window
     * @param xmlNode XML node
     * @return Returns TRUE on success, FALSE on failure
     */
    BOOL CreateChildren(SXmlNode xmlNode) OVERRIDE;

    /**
     * @brief Create child window by name
     * @param pszName Child window name
     * @return Pointer to child window object
     */
    SWindow *CreateChildByName(LPCWSTR pszName) OVERRIDE;

    /**
     * @brief Get desired size
     * @param psz Pointer to size structure
     * @param wid Width
     * @param hei Height
     */
    STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE *psz, int wid, int hei) OVERRIDE;

    /**
     * @brief Get text rectangle
     * @param pRect Pointer to rectangle structure
     */
    void GetTextRect(LPRECT pRect) OVERRIDE;

  protected:
    /**
     * @brief Handle erase background message
     * @param pRT Pointer to render target object
     * @return Returns TRUE on success, FALSE on failure
     */
    BOOL OnEraseBkgnd(IRenderTarget *pRT);

    /**
     * @brief Handle paint message
     * @param pRT Pointer to render target object
     */
    void OnPaint(IRenderTarget *pRT);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_ERASEBKGND_EX(OnEraseBkgnd)
        MSG_WM_PAINT_EX(OnPaint)
    SOUI_MSG_MAP_END()

  protected:
    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"icon", m_iIcon, TRUE)       /**< Icon index */
        ATTR_INT(L"check", m_bCheck, TRUE)     /**< Whether selected */
        ATTR_INT(L"radio", m_bRadio, TRUE)     /**< Whether it is a radio button */
        ATTR_CHAR(L"hotKey", m_cHotKey, FALSE) /**< Hotkey character */
    SOUI_ATTRS_END()

    SMenuEx *m_pSubMenu;   /**< Pointer to submenu object */
    SMenuEx *m_pOwnerMenu; /**< Pointer to owner menu object */
    int m_iIcon;           /**< Icon index */
    BOOL m_bCheck;         /**< Whether selected */
    BOOL m_bRadio;         /**< Whether it is a radio button */
    WCHAR m_cHotKey;       /**< Hotkey character */
};

/**
 * @class SMenuEx
 * @brief Extended menu class
 */
class SOUI_EXP SMenuEx
    : public SHostWnd
    , public IMenuEx {
    friend class SMenuExItem;
    friend class SMenuExRunData;
    friend class SMenuExRoot;
    typedef SHostWnd __baseCls;

  public:
    /**
     * @brief Constructor
     */
    SMenuEx(void);

    /**
     * @brief Destructor
     */
    virtual ~SMenuEx(void);

  public:
    /** Add reference */
    STDMETHOD_(long, AddRef)(THIS) OVERRIDE
    {
        return SHostWnd::AddRef();
    }

    /** Release reference */
    STDMETHOD_(long, Release)(THIS) OVERRIDE
    {
        return SHostWnd::Release();
    }

    /** Release object */
    STDMETHOD_(void, OnFinalRelease)(THIS) OVERRIDE
    {
        delete this;
    }

    //=================================================================
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
     * @brief Insert menu item
     * @param uPosition Position
     * @param uFlags Flags
     * @param nIDNewItem New item ID
     * @param strText Text
     * @param iIcon Icon index (default is -1)
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, InsertMenu)(THIS_ UINT uPosition, UINT uFlags, int nIDNewItem, LPCTSTR strText, int iIcon DEF_VAL(-1)) OVERRIDE;

    /**
     * @brief Append menu item
     * @param uFlags Flags
     * @param uIDNewItem New item ID
     * @param lpNewItem Text
     * @param iIcon Icon index (default is -1)
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, AppendMenu)(THIS_ UINT uFlags, int uIDNewItem, LPCTSTR lpNewItem, int iIcon DEF_VAL(-1)) OVERRIDE;

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
     * @brief Check menu item
     * @param uIdCheckItem Menu item ID to check
     * @param uCheck Check flag (e.g., MF_CHECKED, MF_UNCHECKED)
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, CheckMenuItem)(THIS_ UINT uIdCheckItem, UINT uCheck) OVERRIDE;

    /**
     * @brief Enable or disable menu item
     * @param uIDEnableItem Item ID or index
     * @param uEnable MF_BYCOMMAND/MF_BYPOSITION | MF_ENABLED/MF_GRAYED/MF_DISABLED
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, EnableMenuItem)(THIS_ UINT uIDEnableItem, UINT uEnable) OVERRIDE;

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
     * @param nScale Scale (default is 100)
     * @return Menu item ID
     */
    STDMETHOD_(UINT, TrackPopupMenu)(THIS_ UINT uFlags, int x, int y, HWND hWnd, int nScale DEF_VAL(100)) OVERRIDE;

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
     * @return Pointer to submenu object
     */
    STDMETHOD_(IMenuEx *, GetSubMenu)(THIS_ int nPos) OVERRIDE;

    /**
     * @brief Get menu item string
     * @param uPosition Menu item position
     * @param uFlags Flags (e.g., MF_BYCOMMAND, MF_BYPOSITION)
     * @param lpItemString Pointer to object receiving the menu item string
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, GetMenuString)(THIS_ UINT uPosition, UINT uFlags, IStringT *lpItemString) OVERRIDE;

    /**
     * @brief Get host window
     * @return Pointer to host window object
     */
    STDMETHOD_(IHostWnd *, GetHostWnd)(THIS) OVERRIDE
    {
        return this;
    }

    STDMETHOD_(int, GetMenuItemCount)(CTHIS) SCONST OVERRIDE;

  public:
    /**
     * @brief End menu
     * @param nCmdId Command ID (default is 0)
     */
    static void EndMenu(int nCmdId = 0);

    /**
     * @brief Get event owner
     * @return Pointer to event owner object
     */
    static SMenuEx *GetEvtOwner();

    /**
     * @brief Get parent menu item
     * @return Pointer to parent menu item object
     */
    SMenuExItem *GetParentItem()
    {
        return m_pParent;
    }

    /**
     * @brief Get submenu
     * @param nID Menu item ID
     * @param byCmdId Whether to look up by command ID
     * @return Pointer to submenu object
     */
    SMenuEx *GetSubMenu(int nID, BOOL byCmdId);

    /**
     * @brief Get menu item
     * @param nID Menu item ID
     * @param byCmdId Whether to look up by command ID
     * @return Pointer to menu item object
     */
    SMenuExItem *GetMenuItem(int nID, BOOL byCmdId);

  protected:
    void OnMenuEnd();

    /**
     * @brief Menu item selected state changed
     * @param pItem Pointer to menu item object
     * @param bByMouse Whether triggered by mouse
     */
    void OnSelItemChanged(SMenuExItem *pItem, BOOL bByMouse);

    /**
     * @brief Initialize an empty menu (should not be called externally)
     * @param ParentRoot Pointer to parent menu root object
     * @return Returns TRUE on success, FALSE on failure
     */
    BOOL IniNullMenu(SMenuExRoot *ParentRoot);

    /**
     * @brief Handle mouse activate message
     * @param wndTopLevel Top-level window handle
     * @param nHitTest Hit test result
     * @param message Message ID
     * @return Activation state
     */
    int OnMouseActivate(HWND wndTopLevel, UINT nHitTest, UINT message);

    /**
     * @brief Handle timer message
     * @param timeID Timer ID
     */
    void OnTimer(UINT_PTR timeID);

    /**
     * @brief Handle key message
     * @param nChar Character
     * @param nRepCnt Repeat count
     * @param nFlags Flags
     */
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Handle mouse leave message
     */
    void OnMouseLeave();

    BEGIN_MSG_MAP_EX(SMenuEx)
        MSG_WM_MOUSEACTIVATE(OnMouseActivate)
        MSG_WM_TIMER(OnTimer)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        CHAIN_MSG_MAP(SHostWnd)
    END_MSG_MAP()

  protected:
    /**
     * @brief Handle event
     * @param pEvt Pointer to event parameter object
     * @return Returns TRUE on success, FALSE on failure
     */
    virtual BOOL _HandleEvent(IEvtArgs *pEvt) OVERRIDE;

    /**
     * @brief Load layout from resource ID
     * @param xmlDoc Reference to XML document object
     * @return Returns TRUE on success, FALSE on failure
     */
    BOOL OnLoadLayoutFromResourceID(SXmlDoc &xmlDoc) override;

    /**
     * @brief Create root window
     * @return Pointer to root window object
     */
    SRootWindow *CreateRoot() override;

    /**
     * @brief Get translation context
     * @return Translation context string
     */
    STDMETHOD_(LPCWSTR, GetTranslatorContext)(THIS) SCONST OVERRIDE;

    /**
     * @brief Get scale
     * @return Scale
     */
    STDMETHOD_(int, GetScale)() SCONST OVERRIDE;

    /**
     * @brief Find menu item
     * @param uPos Position
     * @param uFlag Flag
     * @return Pointer to menu item object
     */
    SWindow *FindItem(UINT uPos, UINT uFlag);

    /**
     * @brief Show menu
     * @param uFlag Flag
     * @param x X coordinate
     * @param y Y coordinate
     */
    void ShowMenu(UINT uFlag, int x, int y);

    /**
     * @brief Hide menu
     * @param bUncheckParentItem Whether to uncheck the parent menu item
     */
    void HideMenu(BOOL bUncheckParentItem);

    /**
     * @brief Hide submenu
     */
    void HideSubMenu();

    /**
     * @brief Run menu
     * @param hOwner Owner window handle
     */
    void RunMenu(HWND hOwner);

    /**
     * @brief Pop up submenu
     * @param pItem Pointer to menu item object
     * @param bCheckFirstItem Whether to select the first submenu item
     */
    void PopupSubMenu(SMenuExItem *pItem, BOOL bCheckFirstItem);

    /**
     * @brief Handle submenu hidden event
     * @param bUncheckItem Whether to uncheck the submenu item
     */
    void OnSubMenuHided(BOOL bUncheckItem);

    /**
     * @brief Constructor (with parent menu item)
     * @param pParent Pointer to parent menu item object
     */
    SMenuEx(SMenuExItem *pParent);

    /**
     * @brief Send initialization popup menu event to owner
     * @param idx Index
     */
    void SendInitPopupMenu2Owner(int idx);

    /**
     * @brief Convert menu item to index
     * @param pItem Pointer to menu item object
     * @return Index
     */
    int MenuItem2Index(SMenuExItem *pItem) const;

    SMenuExItem *m_pParent;    /**< Pointer to parent menu item object */
    SMenuExItem *m_pHoverItem; /**< Pointer to hovered menu item object */
    SMenuExItem *m_pCheckItem; /**< Pointer to selected menu item object */

    BOOL m_bMenuInitialized; /**< Whether the menu is initialized */
};

SNSEND

#endif /**< __SMENUEX__H__ */