/**
 * @file       STabCtrl.h
 * @brief      Definition of the STabCtrl and related classes.
 * @version    v1.0
 * @author     soui
 * @date       2014-07-06
 *
 * @copyright  Copyright (C) 2014-2050 SOUI团队
 *             All rights reserved.
 */

#ifndef __STABCTRL__H__
#define __STABCTRL__H__

#include <core/SWnd.h>
#include <helper/SplitString.h>
#include <proxy/SWindowProxy.h>
#include <interface/SCtrl-i.h>

SNSBEGIN

/**
 * @class STabPage
 * @brief A tab page within a tab control.
 *
 * This class represents a tab page within an STabCtrl, which can have a title and an icon.
 */
class SOUI_EXP STabPage : public TWindowProxy<ITabPage> {
    DEF_SOBJECT(SWindow, L"page")
    friend class STabCtrl;

  public:
    /**
     * @brief Constructor for STabPage.
     */
    STabPage()
        : m_iIcon(-1)
        , m_strTitle(this)
    {
        m_bVisible = FALSE;
    }

    /**
     * @brief Destructor for STabPage.
     */
    virtual ~STabPage()
    {
    }

  public:
    /**
     * @brief Gets the title of the tab page.
     * @return The title of the tab page.
     */
    STDMETHOD_(LPCTSTR, GetTitle)(CTHIS) SCONST OVERRIDE
    {
        return m_strTitle.GetText(FALSE);
    }

    /**
     * @brief Sets the title of the tab page.
     * @param lpszTitle The new title for the tab page.
     */
    STDMETHOD_(void, SetTitle)(THIS_ LPCTSTR lpszTitle) OVERRIDE
    {
        m_strTitle.SetText(lpszTitle, false);
    }

    /**
     * @brief Gets the icon index of the tab page.
     * @return The icon index of the tab page.
     */
    STDMETHOD_(int, GetIconIndex)(CTHIS) SCONST OVERRIDE
    {
        return m_iIcon;
    }

    /**
     * @brief Sets the icon index of the tab page.
     * @param iIcon The new icon index for the tab page.
     */
    STDMETHOD_(void, SetIconIndex)(THIS_ int iIcon) OVERRIDE
    {
        m_iIcon = iIcon;
    }

  protected:
    /**
     * @brief Handles tooltip updates.
     * @param pt The mouse position.
     * @param tipInfo Tooltip information.
     * @return Always returns FALSE to disable tooltips on the page.
     */
    virtual BOOL UpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo)
    {
        return FALSE;
    }

    SOUI_ATTRS_BEGIN()
        ATTR_I18NSTRT(L"title", m_strTitle, FALSE) /**< Title of the tab page. */
        ATTR_INT(L"iconIndex", m_iIcon, FALSE)     /**< Icon index of the tab page. */
    SOUI_ATTRS_END()

  protected:
    STrText m_strTitle; /**< Title of the tab page. */
    int m_iIcon;        /**< Icon index of the tab page. */
};

class STabSlider;

/**
 * @class STabCtrl
 * @brief A tab control for managing multiple tab pages.
 *
 * This class represents a tab control that can manage multiple tab pages with various customization options.
 */
class SOUI_EXP STabCtrl : public TWindowProxy<ITabCtrl> {
    friend class STabSlider;
    DEF_SOBJECT(SWindow, L"tabctrl")

  protected:
    int m_nHoverTabItem;                   /**< Index of the tab item under the hover state. */
    int m_nCurrentPage;                    /**< Index of the currently selected page. */
    SLayoutSize m_nTabInterSize;           /**< Spacing between tab pages. */
    SLayoutSize m_szTab[2];                /**< Size of the tab pages. */
    SLayoutSize m_nTabPos;                 /**< Position of the tabs. */
    SAutoRefPtr<ISkinObj> m_pSkinTab;      /**< Skin object for the tabs. */
    SAutoRefPtr<ISkinObj> m_pSkinIcon;     /**< Skin object for the icons. */
    SAutoRefPtr<ISkinObj> m_pSkinTabInter; /**< Skin object for the tab spacing. */
    SAutoRefPtr<ISkinObj> m_pSkinFrame;    /**< Skin object for the frame. */
    SLayoutSize m_ptIcon[2];               /**< Position of the icons. */
    SLayoutSize m_ptText[2];               /**< Position of the text. */
    int m_nTabAlign;                       /**< Alignment of the tabs. */

    SArray<STabPage *> m_lstPages; /**< List of tab pages. */
    STabSlider *m_tabSlider;
    enum
    {
        AlignTop,
        AlignLeft,
        AlignBottom,
        AlignRight,
    };

    enum TEXTDIR
    {
        Text_Horz,
        Text_Vert,
    } m_txtDir;
    int m_nAnimateSteps; /**< Number of animation steps. */
    int m_nAniamteType;  /**< Animation style. */
    SAutoRefPtr<IInterpolator> m_aniInterpolator;

  public:
    /**
     * @brief Constructor for STabCtrl.
     */
    STabCtrl();

    /**
     * @brief Destructor for STabCtrl.
     */
    virtual ~STabCtrl()
    {
    }

  public:
    /**
     * @brief Gets the index of the currently selected tab page.
     * @return Index of the currently selected tab page.
     */
    STDMETHOD_(int, GetCurSel)(THIS) SCONST OVERRIDE;

    /**
     * @brief Sets the currently selected tab page.
     * @param nIndex Index of the tab page to select.
     * @return TRUE if successful, otherwise FALSE.
     */
    STDMETHOD_(BOOL, SetCurSel)(THIS_ int nIndex) OVERRIDE;

    /**
     * @brief Sets the title of a tab page.
     * @param nIndex Index of the tab page.
     * @param lpszTitle New title for the tab page.
     * @return TRUE if successful, otherwise FALSE.
     */
    STDMETHOD_(BOOL, SetItemTitle)(THIS_ int nIndex, LPCTSTR lpszTitle) OVERRIDE;

    /**
     * @brief Inserts a new tab page.
     * @param lpContent XML description of the page.
     * @param iInsert Position to insert the page.
     * @return Index of the inserted page.
     */
    STDMETHOD_(int, InsertItem)(THIS_ LPCWSTR lpContent, int iInsert = -1) OVERRIDE;

    /**
     * @brief Gets the number of tab pages.
     * @return Number of tab pages.
     */
    STDMETHOD_(int, GetItemCount)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets a tab page by index.
     * @param nIndex Index of the tab page.
     * @return Pointer to the tab page.
     */
    STDMETHOD_(IWindow *, GetPage)(THIS_ int nIndex) OVERRIDE;

    /**
     * @brief Removes a tab page by index.
     * @param nIndex Index of the tab page to remove.
     * @param iSelPage Index of the page to select after removal.
     * @return TRUE if successful, otherwise FALSE.
     */
    STDMETHOD_(BOOL, RemoveItem)(THIS_ int nIndex, int iSelPage = 0) OVERRIDE;

    /**
     * @brief Removes all tab pages.
     */
    STDMETHOD_(void, RemoveAllItems)(THIS) OVERRIDE;

    /**
     * @brief Gets the index of a tab page by name or title.
     * @param pszName Name or title of the page.
     * @param bTitle TRUE if pszName is a title, FALSE if it is a name.
     * @return Index of the found page.
     */
    STDMETHOD_(int, GetPageIndex)(THIS_ LPCTSTR pszName, BOOL bTitle) OVERRIDE;

  public:
    /**
     * @brief Gets a tab page by index.
     * @param iPage Index of the tab page.
     * @return Pointer to the tab page.
     */
    STabPage *GetItem(int iPage);

    /**
     * @brief Gets a tab page by name or title.
     * @param pszName Name or title of the page.
     * @param bTitle TRUE if pszName is a title, FALSE if it is a name.
     * @return Pointer to the tab page.
     */
    STabPage *GetPage(LPCTSTR pszName, BOOL bTitle = TRUE);

    /**
     * @brief Sets the currently selected tab page by name or title.
     * @param pszName Name or title of the page.
     * @param bTitle TRUE if pszName is a title, FALSE if it is a name.
     * @return TRUE if successful, otherwise FALSE.
     */
    BOOL SetCurSel(LPCTSTR pszName, BOOL bTitle = TRUE);

  protected:
    /**
     * @brief Creates tab pages from an XML node.
     * @param xmlNode XML node containing the tab page definitions.
     * @return TRUE if successful, otherwise FALSE.
     */
    BOOL CreateChildren(SXmlNode xmlNode);

    /**
     * @brief Inserts a new tab page from an XML node.
     * @param xmlNode XML node containing the tab page definition.
     * @param iInsert Position to insert the page.
     * @param bLoading TRUE if the page is being loaded.
     * @return Index of the inserted page.
     */
    virtual int InsertItem(SXmlNode xmlNode, int iInsert = -1, BOOL bLoading = FALSE);

    /**
     * @brief Handles the insertion of a tab page.
     * @param pItem Pointer to the inserted tab page.
     */
    virtual void OnItemInserted(STabPage *pItem)
    {
    }

    /**
     * @brief Handles the removal of a tab page.
     * @param pItem Pointer to the removed tab page.
     */
    virtual void OnItemRemoved(STabPage *pItem)
    {
    }

    /**
     * @brief Prepares the rendering target for painting the current window.
     * @param pRT Pointer to the rendering target.
     * @param painter Painter object.
     */
    virtual void BeforePaint(IRenderTarget *pRT, SPainter &painter);

    /**
     * @brief Gets the layout rectangle for child windows.
     * @param prc Pointer to the rectangle to receive the layout rectangle.
     */
    STDMETHOD_(void, GetChildrenLayoutRect)(THIS_ RECT *prc) SCONST OVERRIDE;

    /**
     * @brief Gets the rectangle of the tab header.
     * @return Rectangle of the tab header.
     */
    virtual CRect GetTitleRect();

    /**
     * @brief Gets the rectangle of a specified tab item.
     * @param nIndex Index of the tab item.
     * @param rcItem Rectangle to receive the item position.
     * @return TRUE if successful, otherwise FALSE.
     */
    virtual BOOL GetItemRect(int nIndex, CRect &rcItem);

    /**
     * @brief Draws a tab item.
     * @param pRT Pointer to the rendering target.
     * @param rcItem Rectangle of the item to draw.
     * @param iItem Index of the item.
     * @param dwState State of the item.
     */
    virtual void DrawItem(IRenderTarget *pRT, const CRect &rcItem, int iItem, DWORD dwState);

    virtual STabPage *CreatePageFromXml(SXmlNode xmlPage);

    /**
     * @brief Gets the dialog code for the window.
     * @return Dialog code.
     */
    virtual UINT WINAPI OnGetDlgCode() const
    {
        return SC_WANTARROWS;
    }

    virtual BOOL UpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo);

    /**
     * @brief Updates the positions of child windows.
     */
    STDMETHOD_(void, UpdateChildrenPosition)(THIS) OVERRIDE;

    STDMETHOD_(void, OnInitFinished)(THIS_ IXmlNode *xmlNode) OVERRIDE;

    virtual void OnColorize(COLORREF cr);
    virtual void OnScaleChanged(int nScale);

    virtual HRESULT OnLanguageChanged();

  protected:
    /**
     * @brief Performs a hit test on the tab control.
     * @param pt Mouse position.
     * @return Index of the tab item under the mouse, or -1 if none.
     */
    int HitTest(CPoint pt);

    void OnSliderFinish();

  protected:
    /**
     * @brief Outputs text vertically.
     * @param pRT Pointer to the rendering target.
     * @param x X-coordinate of the text.
     * @param y Y-coordinate of the text.
     * @param strText Text to output.
     */
    void TextOutV(IRenderTarget *pRT, int x, int y, const SStringT &strText);

    /**
     * @brief Draws text vertically.
     * @param pRT Pointer to the rendering target.
     * @param rcText Rectangle for the text.
     * @param strText Text to draw.
     */
    void DrawTextV(IRenderTarget *pRT, CRect rcText, const SStringT &strText);

    /**
     * @brief Measures text vertically.
     * @param pRT Pointer to the rendering target.
     * @param strText Text to measure.
     * @return Size of the text.
     */
    SIZE MeasureTextV(IRenderTarget *pRT, const SStringT &strText);

  protected:
    /**
     * @brief Handles the paint event.
     * @param pRT Pointer to the rendering target.
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief Handles the left mouse button down event.
     * @param nFlags Flags associated with the mouse event.
     * @param point Mouse position.
     */
    void OnLButtonDown(UINT nFlags, CPoint point);

    /**
     * @brief Handles the mouse move event.
     * @param nFlags Flags associated with the mouse event.
     * @param point Mouse position.
     */
    void OnMouseMove(UINT nFlags, CPoint point);

    /**
     * @brief Handles the mouse leave event.
     */
    void OnMouseLeave()
    {
        OnMouseMove(0, CPoint(-1, -1));
    }

    /**
     * @brief Handles the key down event.
     * @param nChar Virtual key code of the key.
     * @param nRepCnt Repeat count for the key.
     * @param nFlags Flags associated with the key event.
     */
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Handles the destroy event.
     */
    void OnDestroy();

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_KEYDOWN(OnKeyDown)
    SOUI_MSG_MAP_END()

    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"curSel", m_nCurrentPage, FALSE)               /**< Index of the currently selected page. */
        ATTR_LAYOUTSIZE2(L"tabSize", m_szTab, TRUE)              /**< Size of the tab pages. */
        ATTR_LAYOUTSIZE(L"tabWidth", m_szTab[0], FALSE)          /**< Width of the tab pages. */
        ATTR_LAYOUTSIZE(L"tabHeight", m_szTab[1], FALSE)         /**< Height of the tab pages. */
        ATTR_LAYOUTSIZE(L"tabPos", m_nTabPos, FALSE)             /**< Position of the tabs. */
        ATTR_LAYOUTSIZE(L"tabInterSize", m_nTabInterSize, FALSE) /**< Spacing between tab pages. */
        ATTR_SKIN(L"tabInterSkin", m_pSkinTabInter, FALSE)       /**< Skin object for the tab spacing. */
        ATTR_SKIN(L"tabSkin", m_pSkinTab, FALSE)                 /**< Skin object for the tabs. */
        ATTR_SKIN(L"iconSkin", m_pSkinIcon, FALSE)               /**< Skin object for the icons. */
        ATTR_SKIN(L"frameSkin", m_pSkinFrame, FALSE)             /**< Skin object for the frame. */
        ATTR_LAYOUTSIZE(L"icon-x", m_ptIcon[0], FALSE)           /**< X-coordinate of the icons. */
        ATTR_LAYOUTSIZE(L"icon-y", m_ptIcon[1], FALSE)           /**< Y-coordinate of the icons. */
        ATTR_LAYOUTSIZE(L"text-x", m_ptText[0], FALSE)           /**< X-coordinate of the text. */
        ATTR_LAYOUTSIZE(L"text-y", m_ptText[1], FALSE)           /**< Y-coordinate of the text. */
        ATTR_ENUM_BEGIN(L"tabAlign", int, TRUE)
            ATTR_ENUM_VALUE(L"top", AlignTop)       /**< Align tabs at the top. */
            ATTR_ENUM_VALUE(L"left", AlignLeft)     /**< Align tabs on the left. */
            ATTR_ENUM_VALUE(L"right", AlignRight)   /**< Align tabs on the right. */
            ATTR_ENUM_VALUE(L"bottom", AlignBottom) /**< Align tabs at the bottom. */
        ATTR_ENUM_END(m_nTabAlign)
        ATTR_ENUM_BEGIN(L"textDir", TEXTDIR, TRUE)
            ATTR_ENUM_VALUE(L"hori", Text_Horz)       /**< Horizontal text direction. */
            ATTR_ENUM_VALUE(L"horizontal", Text_Horz) /**< Horizontal text direction. */
            ATTR_ENUM_VALUE(L"vert", Text_Vert)       /**< Vertical text direction. */
            ATTR_ENUM_VALUE(L"vertical", Text_Vert)   /**< Vertical text direction. */
        ATTR_ENUM_END(m_txtDir)
        ATTR_INT(L"animateSteps", m_nAnimateSteps, FALSE)            /**< Number of animation steps. */
        ATTR_INT(L"animateType", m_nAniamteType, FALSE)              /**< Animation style. */
        ATTR_INTERPOLATOR(L"interpolator", m_aniInterpolator, FALSE) /**< Interpolator for animations. */
        ATTR_CHAIN_PTR(m_aniInterpolator, 0)                         /**< Chain attributes to interpolator. */
    SOUI_ATTRS_END()
};

SNSEND

#endif // __STABCTRL__H__