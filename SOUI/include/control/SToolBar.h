#ifndef __SToolBar_H__
#define __SToolBar_H__

SNSBEGIN

namespace SToolBar_style
{
    static const LPCWSTR kStyle_data = L"data";
    static const LPCWSTR kStyle_item = L"item";
    static const LPCWSTR kStyle_sep = L"sep";
    static const LPCWSTR kStyle_iconIndex = L"iconIndex";
    static const LPCWSTR kStyle_id = L"id";
    static const LPCWSTR kStyle_dataAttr = L"data";
    static const LPCWSTR kStyle_text = L"text";
    static const LPCWSTR kStyle_tip = L"tip";
    static const LPCWSTR kStyle_disable = L"disable";
    static const LPCWSTR kStyle_check = L"check";
    static const LPCWSTR kStyle_dropdown = L"dropdown";
    static const LPCWSTR kStyle_showText = L"showText";
    static const LPCWSTR kStyle_checked = L"checked";
    static const LPCWSTR kStyle_menuStyle = L"menuStyle";
}

class SOUI_EXP SToolBar : public SWindow {
    DEF_SOBJECT(SWindow, L"toolbar")
  public:
    SToolBar(void);
    ~SToolBar(void);

  public:
    struct ToolBarItem
    {
        int nId;
        int iIcon;
        LPARAM lParam;
        SStringT strText;
        SStringT strTip;
        int dwState;
        int dwStyle; // Item style flags (TBSTYLE_*)
        SAutoRefPtr<IBitmapS> icon;

        ToolBarItem()
            : nId(0)
            , iIcon(0)
            , lParam(0)
            , dwState(0)
            , dwStyle(TBSTYLE_BUTTON)
        {
        }
    };

    void InsertItem(const ToolBarItem &item, int nPos = -1);
    void AddButton(int nID, int nImage, LPCTSTR lpszText = NULL, int dwStyle = TBSTYLE_BUTTON);
    BOOL DeleteButton(int nIndex);

    // Item query methods
    int GetItemCount() const
    {
        return (int)m_arrItems.GetCount();
    }
    int GetItemID(int nIndex) const;
    int CommandToIndex(int nID) const;

    // Item information
    BOOL GetItemInfo(int nIndex, ToolBarItem *pItem) const;
    BOOL SetItemInfo(int nIndex, const ToolBarItem *pItem);

    void SetIconsSkin(SAutoRefPtr<ISkinObj> skinIcons);
    void SetDropDownSize(int nSize)
    {
        m_nDropDownSize = nSize;
    }

    // Button state methods (MFC-compatible)
    BOOL EnableButton(int nID, BOOL bEnable = TRUE);
    BOOL CheckButton(int nID, BOOL bCheck = TRUE);
    BOOL PressButton(int nID, BOOL bPress = TRUE);
    BOOL IsButtonEnabled(int nID) const;
    BOOL IsButtonChecked(int nID) const;
    BOOL IsButtonPressed(int nID) const;

    // Button style methods
    int GetButtonStyle(int nIndex) const;
    void SetButtonStyle(int nIndex, int nStyle);

    // Display control methods
    BOOL IsItemShowText(int nItemId) const;
    BOOL IsDropDownItem(int nItemId) const;

    // Menu style methods
    BOOL LoadMenuStyle(LPCTSTR pszResName);
    void SetMenuStyle(SXmlNode xmlMenuStyle);

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"iconSkin", m_skinIcons, TRUE)
        ATTR_SKIN(L"sepSkin", m_skinSep, TRUE)
        ATTR_SKIN(L"skin", m_skinState, TRUE)
        ATTR_SKIN(L"dropArrowSkin", m_skinDropArrow, TRUE)
        ATTR_INT(L"dropDownSize", m_nDropDownSize, TRUE)
        ATTR_BOOL(L"vertical", m_bVert, TRUE)
        ATTR_INT(L"maxItemWidth", m_nMaxItemWidth, TRUE)
        ATTR_INT(L"interItemSep", m_nInterItemSep, TRUE)
    SOUI_ATTRS_END()

  protected:
    virtual void DrawItem(IRenderTarget *pRT, const CRect &rc, const ToolBarItem *pItem);
    virtual void DrawDropButton(IRenderTarget *pRT, const CRect &rc, DWORD dwState);
    virtual BOOL CreateChildren(SXmlNode xmlNode) override;
    virtual SIZE MeasureContent(int nParentWid, int nParentHei) override;
    virtual BOOL UpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo) override;

  protected:
    void OnPaint(IRenderTarget *pRT);
    void OnLButtonDown(UINT nFlags, CPoint pt);
    void OnLButtonUp(UINT nFlags, CPoint pt);
    void OnMouseMove(UINT nFlags, CPoint pt);
    void OnMouseLeave();
    void OnSize(UINT nType, CSize size);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_SIZE(OnSize)
    SOUI_MSG_MAP_END()

  protected:
    CSize GetItemSize(IRenderTarget *pRT, int iItem) const;
    int GetSepWid() const;
    CRect GetItemRect(int iItem) const;
    int HitTest(CPoint pt) const;
    BOOL IsSeparator(const ToolBarItem *pItem) const;
    void UpdateVisibleItemCount();
    BOOL HasDropDownButton() const;
    CRect GetDropdownButtonRect() const;
    void ShowDropDownMenu(const CRect &rc);
    int GetItemByID(int nID) const;

    /**
     * Check if point is on the dropdown arrow of an item with TBSTYLE_DROPDOWN
     * @param iItem Item index
     * @param pt Point to test
     * @return TRUE if point is on dropdown arrow
     */
    BOOL IsPointOnDropDownArrow(int iItem, CPoint pt) const;

  private:
    /**
     * Show dropdown menu for a specific item (TBSTYLE_DROPDOWN)
     * @param rc Rectangle of the item
     * @param iItem Item index
     */
    void ShowItemDropDownMenu(const CRect &rc, int iItem);

  protected:
    SAutoRefPtr<ISkinObj> m_skinState;
    SAutoRefPtr<ISkinObj> m_skinIcons;
    SAutoRefPtr<ISkinObj> m_skinSep;
    SAutoRefPtr<ISkinObj> m_skinDropArrow;
    SXmlDoc m_menuStyle;
    SArray<ToolBarItem> m_arrItems;

    int m_iClickItem;
    int m_iHoverItem;
    BOOL m_bVert;
    int m_nMaxItemWidth;
    int m_nVisibleItems;
    int m_nDropDownSize; // Size of dropdown arrow for TBSTYLE_DROPDOWN items
    DWORD m_dwDropBtnState;
    int m_nMoreButtonSize; // Size of more button (calculated dynamically)
    int m_nInterItemSep;   // Separation between items (in pixels)
};

SNSEND

#endif //__SToolBar_H__
