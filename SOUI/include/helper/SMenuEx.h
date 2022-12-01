#pragma once

#include <core/shostwnd.h>
#include <interface/smenuex-i.h>

SNSBEGIN
class SMenuExRoot;
class SMenuEx;
class SOUI_EXP SMenuExItem : public SWindow {
    DEF_SOBJECT(SWindow, L"menuItem")
    friend class SMenuEx;

  public:
    SMenuExItem(SMenuEx *pOwnerMenu, ISkinObj *pItemSkin);

    ~SMenuExItem();

    SMenuEx *GetSubMenu();

    SMenuEx *GetOwnerMenu();

    void HideSubMenu();

    void ShowSubMenu(BOOL bCheckFirstItem);

    WCHAR GetHotKey() const;

  protected:
    void OnSubMenuHided(BOOL bUncheckItem);
    BOOL CreateChildren(SXmlNode xmlNode) OVERRIDE;
    SWindow *CreateChildByName(LPCWSTR pszName) OVERRIDE;

    STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE *psz,int wid, int hei) OVERRIDE;

    void GetTextRect(LPRECT pRect) OVERRIDE;

  protected:
    BOOL OnEraseBkgnd(IRenderTarget *pRT);

    void OnPaint(IRenderTarget *pRT);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_ERASEBKGND_EX(OnEraseBkgnd)
        MSG_WM_PAINT_EX(OnPaint)
    SOUI_MSG_MAP_END()

  protected:
    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"icon", m_iIcon, TRUE)
        ATTR_INT(L"check", m_bCheck, TRUE)
        ATTR_INT(L"radio", m_bRadio, TRUE)
        ATTR_CHAR(L"hotKey", m_cHotKey, FALSE)
    SOUI_ATTRS_END()

    SMenuEx *m_pSubMenu;
    SMenuEx *m_pOwnerMenu;
    int m_iIcon;
    BOOL m_bCheck;
    BOOL m_bRadio;
    WCHAR m_cHotKey;
};

class SOUI_EXP SMenuEx
    : protected SHostWnd
    , public IMenuEx {
    friend class SMenuExItem;
    friend class SMenuExRunData;
    friend class SMenuExRoot;
    DEF_SOBJECT(SHostWnd, L"smenuex")
  public:
    SMenuEx(void);
    virtual ~SMenuEx(void);

  public:
    //!添加引用
    /*!
     */
    STDMETHOD_(long, AddRef)(THIS) OVERRIDE
    {
        return SHostWnd::AddRef();
    }

    //!释放引用
    /*!
     */
    STDMETHOD_(long, Release)(THIS) OVERRIDE
    {
        return SHostWnd::Release();
    }

    //!释放对象
    /*!
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) OVERRIDE
    {
        delete this;
    }

    //=================================================================
    STDMETHOD_(BOOL, LoadMenu)(THIS_ LPCTSTR resId) OVERRIDE;

	STDMETHOD_(BOOL, LoadMenuA)(THIS_ LPCSTR resId) OVERRIDE;

    STDMETHOD_(BOOL, LoadMenu2)(THIS_ IXmlNode *xmlMenu) OVERRIDE;

    STDMETHOD_(BOOL, InsertMenu)(THIS_ UINT uPosition, UINT uFlags, int nIDNewItem, LPCTSTR strText, int iIcon DEF_VAL(-1)) OVERRIDE;

    STDMETHOD_(BOOL, AppendMenu)(THIS_ UINT uFlags, int uIDNewItem, LPCTSTR lpNewItem, int iIcon DEF_VAL(-1)) OVERRIDE;

    STDMETHOD_(BOOL, CheckMenuRadioItem)(THIS_ UINT idFirst, UINT idLast, UINT idCheck, UINT uFlags) OVERRIDE;

    STDMETHOD_(BOOL, CheckMenuItem)(THIS_ UINT uIdCheckItem, UINT uCheck) OVERRIDE;

    STDMETHOD_(BOOL, DeleteMenu)(THIS_ UINT uPosition, UINT uFlags) OVERRIDE;

    STDMETHOD_(UINT, TrackPopupMenu)(THIS_ UINT uFlags, int x, int y, HWND hWnd, int nScale DEF_VAL(100)) OVERRIDE;

    STDMETHOD_(void, DestroyMenu)(THIS) OVERRIDE;

    STDMETHOD_(BOOL, ModifyMenuString)(THIS_ UINT uPosition, UINT uFlags, LPCTSTR lpItemString) OVERRIDE;

    STDMETHOD_(BOOL, SetMenuUserData)(THIS_ UINT uPosition, UINT uFlags, ULONG_PTR ulUserData) OVERRIDE;

    STDMETHOD_(ULONG_PTR, GetMenuUserData)(THIS_ UINT uPosition, UINT uFlags) OVERRIDE;

    STDMETHOD_(DWORD, GetContextHelpId)(THIS) SCONST OVERRIDE;

    STDMETHOD_(void, SetContextHelpId)(THIS_ DWORD dwId) OVERRIDE;

    STDMETHOD_(IMenuEx *, GetSubMenu)(THIS_ int nPos) OVERRIDE;

  public:
    static void EndMenu(int nCmdId = 0);

    SMenuExItem *GetParentItem()
    {
        return m_pParent;
    }
    SMenuEx *GetSubMenu(int nID, BOOL byCmdId);
    SMenuExItem *GetMenuItem(int nID, BOOL byCmdId);

  protected:
    //创建一个空菜单,不应该在外部调用
    BOOL IniNullMenu(SMenuExRoot *ParentRoot);
    int OnMouseActivate(HWND wndTopLevel, UINT nHitTest, UINT message);
    void OnTimer(UINT_PTR timeID);
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    BEGIN_MSG_MAP_EX(SMenuEx)
        MSG_WM_MOUSEACTIVATE(OnMouseActivate)
        MSG_WM_TIMER(OnTimer)
        MSG_WM_KEYDOWN(OnKeyDown)
        CHAIN_MSG_MAP(SHostWnd)
    END_MSG_MAP()
  protected:
    virtual BOOL _HandleEvent(IEvtArgs *pEvt);
    STDMETHOD_(LPCWSTR, GetTranslatorContext)(THIS) SCONST OVERRIDE;
    STDMETHOD_(int, GetScale)() SCONST OVERRIDE;

    SWindow *FindItem(UINT uPos, UINT uFlag);

    void ShowMenu(UINT uFlag, int x, int y);
    void HideMenu(BOOL bUncheckParentItem);
    void HideSubMenu();
    void RunMenu(HWND hOwner);

    void PopupSubMenu(SMenuExItem *pItem, BOOL bCheckFirstItem);
    void OnSubMenuHided(BOOL bUncheckItem);

    SMenuEx(SMenuExItem *pParent);
    void SendInitPopupMenu2Owner(int idx);
    SMenuExItem *m_pParent;
    SMenuExItem *m_pHoverItem;
    SMenuExItem *m_pCheckItem;

    BOOL m_bMenuInitialized;
};

SNSEND
