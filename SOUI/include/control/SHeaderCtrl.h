#ifndef __SHEADERCTRL__H__
#define __SHEADERCTRL__H__

#include <core/SWnd.h>
#include <interface/SCtrl-i.h>
#include <proxy/SWindowProxy.h>

SNSBEGIN

#define SORT_MASK  (HDF_SORTDOWN | HDF_SORTUP)
#define ALIGN_MASK (HDF_LEFT | HDF_RIGHT | HDF_CENTER)

/**
 * @class SHeaderCtrl
 * @brief Header Control
 * @details A header control that manages columns with titles and sorting indicators.
 */
class SOUI_EXP SHeaderCtrl : public TWindowProxy<IHeaderCtrl> {
    DEF_SOBJECT(SWindow, L"header")

    enum
    {
        MARGIN_ADJUST_ENABLE = 4,
        MARGIN_ADJUST_DISABLE = 2,
    };

    typedef struct SHDITEMEX : public SHDITEM
    {
        STrText strText;
    } * LPSHDITEMEX;

  public:
    /**
     * @brief Constructor
     */
    SHeaderCtrl(void);

    /**
     * @brief Destructor
     */
    ~SHeaderCtrl(void);

  public:
    /**
     * @brief Insert a new item
     * @param iItem Index of the new item
     * @param pszText Title of the new item
     * @param nWidth Width of the new item
     * @param fmt Format flag
     * @param lParam Additional parameter
     * @param bDpiAware DPI awareness flag
     * @param fWeight Font weight
     * @return Index of the inserted item
     */
    STDMETHOD_(int, InsertItem)
    (THIS_ int iItem, LPCTSTR pszText, int nWidth, UINT fmt, LPARAM lParam, BOOL bDpiAware = FALSE, float fWeight = 0.0f) OVERRIDE;

    /**
     * @brief Insert a new item (ANSI version)
     * @param iItem Index of the new item
     * @param pszText Title of the new item
     * @param nWidth Width of the new item
     * @param fmt Format flag
     * @param lParam Additional parameter
     * @param bDpiAware DPI awareness flag
     * @param fWeight Font weight
     * @return Index of the inserted item
     */
    STDMETHOD_(int, InsertItemA)
    (THIS_ int iItem, LPCSTR pszText, int nWidth, UINT fmt, LPARAM lParam, BOOL bDpiAware = FALSE, float fWeight = 0.0f) OVERRIDE
    {
        SStringT str = S_CA2T(pszText, CP_UTF8);
        return InsertItem(iItem, str, nWidth, fmt, lParam, bDpiAware, fWeight);
    }

    /**
     * @brief Get an item
     * @param iItem Index of the item
     * @param pItem Output structure for the item
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, GetItem)(THIS_ int iItem, SHDITEM *pItem) SCONST OVERRIDE;

    /**
     * @brief Set an item
     * @param iItem Index of the item
     * @param pItem Structure for the item
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, SetItem)(THIS_ int iItem, const SHDITEM *pItem) OVERRIDE;

    /**
     * @brief Get the number of items
     * @return Number of items
     */
    STDMETHOD_(UINT, GetItemCount)(THIS) SCONST OVERRIDE;

    /**
     * @brief Get the total width of all items
     * @param bMinWid Whether to get the minimum width
     * @return Total width
     */
    STDMETHOD_(int, GetTotalWidth)(THIS_ BOOL bMinWid = FALSE) SCONST OVERRIDE;

    /**
     * @brief Check if auto-resize is enabled
     * @return TRUE if auto-resize is enabled, FALSE otherwise
     */
    STDMETHOD_(BOOL, IsAutoResize)(THIS) SCONST OVERRIDE;

    /**
     * @brief Get the width of a specific item
     * @param iItem Index of the item
     * @return Width of the item
     */
    STDMETHOD_(int, GetItemWidth)(THIS_ int iItem) SCONST OVERRIDE;

    /**
     * @brief Delete a specific item
     * @param iItem Index of the item to delete
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, DeleteItem)(THIS_ int iItem) OVERRIDE;

    /**
     * @brief Delete all items
     */
    STDMETHOD_(void, DeleteAllItems)(THIS) OVERRIDE;

    /**
     * @brief Set the sort flag for an item
     * @param iItem Index of the item
     * @param sortFlag Sort flag
     */
    STDMETHOD_(void, SetItemSort)(THIS_ int iItem, UINT sortFlag) OVERRIDE;

    /**
     * @brief Set the visibility of an item
     * @param iItem Index of the item
     * @param visible Visibility flag
     */
    STDMETHOD_(void, SetItemVisible)(THIS_ int iItem, BOOL visible) OVERRIDE;

    /**
     * @brief Check if an item is visible
     * @param iItem Index of the item
     * @return TRUE if visible, FALSE otherwise
     */
    STDMETHOD_(BOOL, IsItemVisible)(THIS_ int iItem) SCONST OVERRIDE;

    /**
     * @brief Get the original item index from the order
     * @param iOrder Order index
     * @return Original item index
     */
    STDMETHOD_(int, GetOriItemIndex)(CTHIS_ int iOrder) SCONST OVERRIDE;

    /**
     * @brief Get the rectangle of an item
     * @param iItem Index of the item
     * @param prc Output rectangle
     */
    STDMETHOD_(void, GetItemRect)(CTHIS_ int iItem, LPRECT prc) SCONST OVERRIDE;

  public:
    /**
     * @brief Get the rectangle of an item
     * @param iItem Index of the item
     * @return Rectangle of the item
     */
    CRect GetItemRect(int iItem) const;

    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"itemSkin", m_pSkinItem, FALSE)
        ATTR_SKIN(L"sortSkin", m_pSkinSort, FALSE)
        ATTR_INT(L"fixWidth", m_bFixWidth, FALSE)
        ATTR_INT(L"itemSwapEnable", m_bItemSwapEnable, FALSE)
        ATTR_INT(L"sortHeader", m_bSortHeader, FALSE)
    SOUI_ATTRS_END()

  protected:
    /**
     * @brief Create child items from XML configuration
     * @param xmlNode XML node for the child items
     * @return TRUE if successful, FALSE otherwise
     */
    virtual BOOL CreateChildren(SXmlNode xmlNode);

    /**
     * @brief Set the cursor based on the mouse position
     * @param pt Mouse coordinates
     * @return TRUE if successful, FALSE otherwise
     */
    virtual BOOL OnSetCursor(const CPoint &pt);

    /**
     * @brief Paint the control
     * @param pRT Rendering target handle
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief Draw an item
     * @param pRT Rendering target handle
     * @param rcItem Rectangle for the item
     * @param pItem Pointer to the item structure
     */
    virtual void DrawItem(IRenderTarget *pRT, CRect rcItem, const LPSHDITEMEX pItem);

    /**
     * @brief Handle colorization event
     * @param cr Color reference
     */
    virtual void OnColorize(COLORREF cr);

    /**
     * @brief Handle language change event
     * @return HRESULT
     */
    virtual HRESULT OnLanguageChanged();

    /**
     * @brief Handle scale change event
     * @param nScale Scale factor
     */
    virtual void OnScaleChanged(int nScale);

    /**
     * @brief Handle relayout event
     * @param rcWnd Window rectangle
     * @return TRUE if successful, FALSE otherwise
     */
    virtual BOOL OnRelayout(const CRect &rcWnd);

    /**
     * @brief Hit test to determine the item under the mouse
     * @param pt Mouse coordinates
     * @return Hit test result (DWORD)
     */
    DWORD HitTest(CPoint pt);

    /**
     * @brief Redraw a specific item
     * @param iItem Index of the item to redraw
     */
    void RedrawItem(int iItem);

    /**
     * @brief Draw the dragging state
     * @param dwDragTo Drag target
     */
    void DrawDraggingState(DWORD dwDragTo);

    /**
     * @brief Check if an item is in hover state
     * @param dwHitTest Hit test result
     * @return TRUE if in hover state, FALSE otherwise
     */
    BOOL IsItemHover(DWORD dwHitTest)
    {
        return dwHitTest != -1 && LOWORD(dwHitTest) == HIWORD(dwHitTest);
    }

    /**
     * @brief Create a drag image for an item
     * @param iItem Index of the item
     * @return Handle to the drag image
     */
    HBITMAP CreateDragImage(UINT iItem);

    /**
     * @brief Handle left mouse button down event
     * @param nFlags Flags
     * @param pt Mouse coordinates
     */
    void OnLButtonDown(UINT nFlags, CPoint pt);

    /**
     * @brief Handle left mouse button up event
     * @param nFlags Flags
     * @param pt Mouse coordinates
     */
    void OnLButtonUp(UINT nFlags, CPoint pt);

    /**
     * @brief Handle mouse move event
     * @param nFlags Flags
     * @param pt Mouse coordinates
     */
    void OnMouseMove(UINT nFlags, CPoint pt);

    /**
     * @brief Handle mouse leave event
     */
    void OnMouseLeave();

    /**
     * @brief Handle destroy event
     */
    void OnDestroy();

    /**
     * @brief Handle activate application event
     * @param bActive Activation flag
     * @param dwThreadID Thread ID
     */
    void OnActivateApp(BOOL bActive, DWORD dwThreadID);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_ACTIVATEAPP(OnActivateApp)
        MSG_WM_DESTROY(OnDestroy)
    SOUI_MSG_MAP_END()

  protected:
    SAutoRefPtr<ISkinObj> m_pSkinItem; /**< Skin for drawing header items */
    SAutoRefPtr<ISkinObj> m_pSkinSort; /**< Skin for sorting indicators */
    BOOL m_bSortHeader;                /**< Whether header can be clicked to sort */
    BOOL m_bFixWidth;                  /**< Whether item widths are fixed */
    BOOL m_bItemSwapEnable;            /**< Whether item swapping is enabled */

    BOOL m_bDragging;             /**< Dragging flag */
    HBITMAP m_hDragImg;           /**< Bitmap for drag image */
    CPoint m_ptClick;             /**< Click point */
    DWORD m_dwHitTest;            /**< Hit test result */
    DWORD m_dwDragTo;             /**< Drag target */
    int m_nAdjItemOldWidth;       /**< Original width of the dragged item */
    SArray<SHDITEMEX> m_arrItems; /**< Array of header items */
    int m_nScale;                 /**< Current scale factor */
};

SNSEND

#endif // __SHEADERCTRL__H__