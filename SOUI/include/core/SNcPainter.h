#ifndef __SNCPAINTER__H__
#define __SNCPAINTER__H__

#include <sobject/Sobject.hpp>
#include <interface/SNcPainter-i.h>
#include <core/SItemPanel.h>

SNSBEGIN

// Forward declarations
class SNcPanel;
class SHostWnd;

/**
 * @brief Non-client area painter class.
 * This class handles the painting and event processing for the non-client area of a window.
 */
class SOUI_EXP SNcPainter
    : public TObjRefImpl<SObjectImpl<INcPainter> >
    , public IItemContainer
    , public IHostProxy {
    DEF_SOBJECT(SObjectImpl<INcPainter>, L"ncpainter")
    friend class SHostWnd;
    friend class SNcPanel;

    enum
    {
        IDC_SYS_ICON = -100,
        IDC_SYS_TITLE = -101,
        IDC_SYS_CLOSE = -102,
        IDC_SYS_MIN = -103,
        IDC_SYS_MAX = -104,
        IDC_SYS_RESTORE = -105,
    };

  public:
    /**
     * @brief Constructor for SNcPainter.
     * @param pHost Pointer to the host window.
     */
    SNcPainter(SHostWnd *pHost);

    /**
     * @brief Destructor for SNcPainter.
     */
    ~SNcPainter(void);

    /**
     * @brief Converts a string to a built-in non-client ID.
     * @param str String representing the non-client ID.
     * @return The corresponding non-client ID.
     */
    static int toNcBuiltinID(const SStringW &str);

    /**
     * @brief Updates system buttons in the non-client area.
     * @param pRoot Pointer to the root window.
     * @param nResizeMode Resize mode for the window.
     */
    static void updateSystemButton(SWindow *pRoot, UINT nResizeMode);

  public:
    /**
     * @brief Initializes the non-client painter from an XML node.
     * @param pXmlNode Pointer to the XML node.
     * @return TRUE if initialization is successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, InitFromXml)(THIS_ IXmlNode *pXmlNode) OVERRIDE;

  public:
    /**
     * @brief Gets the root window.
     * @return Pointer to the root window.
     */
    STDMETHOD_(IWindow *, GetRoot)(THIS) OVERRIDE;

  protected:
    /**
     * @brief Called when an item sets capture.
     * @param pItem Pointer to the item.
     * @param bCapture TRUE if the item sets capture, FALSE otherwise.
     */
    virtual void OnItemSetCapture(SOsrPanel *pItem, BOOL bCapture);

    /**
     * @brief Called to get the rectangle of an item.
     * @param pItem Pointer to the item.
     * @param rcItem Rectangle to receive the item's bounds.
     * @return TRUE if the rectangle is successfully retrieved, FALSE otherwise.
     */
    virtual BOOL OnItemGetRect(const SOsrPanel *pItem, CRect &rcItem) const;

    /**
     * @brief Checks if item redraw is delayed.
     * @return TRUE if redraw is delayed, FALSE otherwise.
     */
    virtual BOOL IsItemRedrawDelay() const;

    virtual BOOL IsTimelineEnabled() const{
        return TRUE;
    }
    /**
     * @brief Gets the host object.
     * @return Pointer to the host object.
     */
    virtual IObject *GetHost();

  protected:
    /**
     * @brief Called when an event is fired on the host.
     * @param e Pointer to the event arguments.
     * @return TRUE if the event is handled, FALSE otherwise.
     */
    virtual BOOL OnHostFireEvent(IEvtArgs *e);

    /**
     * @brief Checks if host updates are locked.
     * @return TRUE if updates are locked, FALSE otherwise.
     */
    virtual BOOL IsHostUpdateLocked() const;

    /**
     * @brief Checks if the host is visible.
     * @return TRUE if the host is visible, FALSE otherwise.
     */
    virtual BOOL IsHostVisible() const;

    /**
     * @brief Gets the rectangle of the host.
     * @return Rectangle of the host.
     */
    virtual CRect GetHostRect() const;

    /**
     * @brief Invalidates a rectangle in the host.
     * @param pRc Pointer to the rectangle to invalidate.
     * @param bClip TRUE to clip the rectangle, FALSE otherwise.
     */
    virtual void InvalidateHostRect(LPCRECT pRc, BOOL bClip);

    /**
     * @brief Gets the host container.
     * @return Pointer to the host container.
     */
    virtual ISwndContainer *GetHostContainer();

    /**
     * @brief Called to get the host render target.
     * @param rc Rectangle for the render target.
     * @param gdcFlags Flags for the render target.
     * @return Pointer to the render target.
     */
    virtual IRenderTarget *OnGetHostRenderTarget(LPCRECT rc, GrtFlag gdcFlags);

    /**
     * @brief Called to release the host render target.
     * @param pRT Pointer to the render target.
     * @param rc Rectangle for the render target.
     * @param gdcFlags Flags for the render target.
     */
    virtual void OnReleaseHostRenderTarget(IRenderTarget *pRT, LPCRECT rc, GrtFlag gdcFlags);

    /**
     * @brief Called when layout is dirty.
     */
    virtual void OnLayoutDirty();

  protected:
    /**
     * @brief Gets the scale factor.
     * @return Scale factor.
     */
    int GetScale() const;

    /**
     * @brief Gets the size of the non-client area.
     * @return Size of the non-client area.
     */
    CSize GetNcSize() const;

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_LAYOUTSIZE(L"titleHeight", m_titleHeight, FALSE)
        ATTR_LAYOUTSIZE(L"borderWidth", m_borderWidth, FALSE)
        ATTR_SKIN(L"skinBorder", m_skinBorder, FALSE)
        ATTR_BOOL(L"system", m_bSysNcPainter, FALSE)
    SOUI_ATTRS_END()

  protected:
    /**
     * @brief Handles the WM_NCCALCSIZE message.
     * @param bCalcValidRects TRUE if valid rectangle calculation is required.
     * @param lParam LPARAM for the message.
     * @return Result of the message handling.
     */
    LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam);

    /**
     * @brief Handles the WM_NCACTIVATE message.
     * @param bActive TRUE if the window is activated, FALSE otherwise.
     * @return Result of the message handling.
     */
    BOOL OnNcActivate(BOOL bActive);

    /**
     * @brief Handles the WM_NCHITTEST message.
     * @param point Point to test.
     * @return Hit test result.
     */
    UINT OnNcHitTest(CPoint point);

    /**
     * @brief Handles the WM_NCPAINT message.
     * @param hRgn Region to paint.
     */
    void OnNcPaint(HRGN hRgn);

    /**
     * @brief Handles the WM_SETTEXT message.
     * @param pszText New text for the window.
     * @return Result of the message handling.
     */
    LRESULT OnSetText(LPCTSTR pszText);

    /**
     * @brief Handles repaint messages.
     * @param msg Message identifier.
     * @param wp WPARAM for the message.
     * @param lp LPARAM for the message.
     * @return Result of the message handling.
     */
    LRESULT OnRepaint(UINT msg, WPARAM wp, LPARAM lp);

    /**
     * @brief Handles the WM_NCDESTROY message.
     */
    void OnNcDestroy();

    /**
     * @brief Handles the WM_NCLBUTTONDOWN message.
     * @param flag Flags for the message.
     * @param pt Point where the button was pressed.
     */
    void OnNcLButtonDown(UINT flag, CPoint pt);

    /**
     * @brief Handles the WM_LBUTTONUP message.
     * @param wp WPARAM for the message.
     * @param lp LPARAM for the message.
     */
    void OnLButtonUp(WPARAM wp, LPARAM lp);

    /**
     * @brief Handles the WM_MOUSEMOVE message.
     * @param wp WPARAM for the message.
     * @param lp LPARAM for the message.
     */
    void OnMouseMove(WPARAM wp, LPARAM lp);

    /**
     * @brief Handles non-client mouse events.
     * @param msg Message identifier.
     * @param wp WPARAM for the message.
     * @param lp LPARAM for the message.
     * @return Result of the message handling.
     */
    LRESULT OnNcMouseEvent(UINT msg, WPARAM wp, LPARAM lp);

    /**
     * @brief Handles the WM_NCMOUSELEAVE message.
     * @param msg Message identifier.
     * @param wp WPARAM for the message.
     * @param lp LPARAM for the message.
     * @return Result of the message handling.
     */
    LRESULT OnNcMouseLeave(UINT msg, WPARAM wp, LPARAM lp);

    /**
     * @brief Handles the WM_SIZE message.
     * @param nType Type of size change.
     * @param size New size of the window.
     */
    void OnSize(UINT nType, CSize size);

    /**
     * @brief Handles the WM_TIMER message.
     * @param tid Timer ID.
     */
    void OnTimer(UINT_PTR tid);

    BEGIN_MSG_MAP_EX(SNcPainter)
        if (m_bSysNcPainter)
            return FALSE;
        MSG_WM_NCCALCSIZE(OnNcCalcSize)
        MSG_WM_NCACTIVATE(OnNcActivate)
        MSG_WM_NCHITTEST(OnNcHitTest)
        MSG_WM_NCPAINT(OnNcPaint)
        MSG_WM_NCDESTROY(OnNcDestroy)
        MSG_WM_SETTEXT(OnSetText)
        MSG_WM_SIZE(OnSize)
        MSG_WM_NCLBUTTONDOWN(OnNcLButtonDown)
        MESSAGE_HANDLER_EX(WM_NCMOUSELEAVE, OnNcMouseLeave)
        MESSAGE_RANGE_HANDLER_EX(WM_NCMOUSEMOVE, WM_NCMBUTTONDBLCLK, OnNcMouseEvent)
        MESSAGE_HANDLER_EX(WM_SETTINGCHANGE, OnRepaint)
        MESSAGE_HANDLER_EX(WM_SYSCOLORCHANGE, OnRepaint)
        MSG_WM_TIMER(OnTimer)
    END_MSG_MAP()

  protected:
    /**
     * @brief Resets the non-client painter.
     */
    void Reset();

    /**
     * @brief Checks if non-client area should be drawn.
     * @return TRUE if non-client area should be drawn, FALSE otherwise.
     */
    BOOL IsDrawNc() const;

    /**
     * @brief Paints the caption.
     */
    void PaintCaption();

    /**
     * @brief Updates the tooltip.
     */
    void UpdateToolTip();

  private:
    // Pointer to the host window
    SHostWnd *m_pHost;

    // Height of the title bar
    SLayoutSize m_titleHeight;
    // Width of the border
    SLayoutSize m_borderWidth;
    // Skin for the border
    SAutoRefPtr<ISkinObj> m_skinBorder;
    // Flag indicating if the system non-client painter is used
    BOOL m_bSysNcPainter;

    // Memory render target for the main area
    SAutoRefPtr<IRenderTarget> m_memRT;
    // Memory render target for the left side
    SAutoRefPtr<IRenderTarget> m_memLeft;
    // Memory render target for the right side
    SAutoRefPtr<IRenderTarget> m_memRight;
    // Memory render target for the top side
    SAutoRefPtr<IRenderTarget> m_memTop;
    // Memory render target for the bottom side
    SAutoRefPtr<IRenderTarget> m_memBottom;
    // Hit test part
    UINT m_htPart;
    // Flag indicating if painting is in progress
    BOOL m_bInPaint;
    // Flag indicating if the left mouse button is down
    BOOL m_bLButtonDown;
    // Flag indicating if the mouse is hovering
    BOOL m_bMouseHover;
    // Root non-client panel
    SNcPanel *m_root;
    // Invalid rectangle
    CRect m_rcInvalid;
};

/**
 * @brief Non-client panel class.
 * This class represents a panel in the non-client area.
 */
class SNcPanel : public SOsrPanel {
    DEF_SOBJECT(SOsrPanel, L"ncpanel")

  public:
    /**
     * @brief Constructor for SNcPanel.
     * @param pFrameHost Pointer to the frame host.
     * @param pItemContainer Pointer to the item container.
     */
    SNcPanel(IHostProxy *pFrameHost, IItemContainer *pItemContainer);

    /**
     * @brief Sets the active state of the panel.
     * @param bActive TRUE if the panel is active, FALSE otherwise.
     */
    void SetActive(BOOL bActive);

    /**
     * @brief Checks if the panel is active.
     * @return TRUE if the panel is active, FALSE otherwise.
     */
    BOOL IsActive() const
    {
        return m_bActive;
    }

  protected:
    /**
     * @brief Erases the background of the panel.
     * @param pRT Pointer to the render target.
     * @return TRUE if the background is erased, FALSE otherwise.
     */
    BOOL OnEraseBkgnd(IRenderTarget *pRT);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_ERASEBKGND_EX(OnEraseBkgnd)
    SOUI_MSG_MAP_END()

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"skinActive", m_skinActive, FALSE)
        ATTR_SKIN(L"skinInactive", m_skinInactive, FALSE)
        ATTR_COLOR(L"colorActiveTitle", m_crActiveTitle, TRUE)
        ATTR_COLOR(L"colorInactiveTitle", m_crInactiveTitle, TRUE)
    SOUI_ATTRS_END()

  protected:
    // Skin for the active state
    SAutoRefPtr<ISkinObj> m_skinActive;
    // Skin for the inactive state
    SAutoRefPtr<ISkinObj> m_skinInactive;
    // Color for the active title
    COLORREF m_crActiveTitle;
    // Color for the inactive title
    COLORREF m_crInactiveTitle;

    // Flag indicating if the panel is active
    BOOL m_bActive;
};

SNSEND

#endif // __SNCPAINTER__H__