#ifndef __SREBAR__H__
#define __SREBAR__H__

#include <core/SWnd.h>
#include <wtl.mini/souimisc.h>

SNSBEGIN

/**
 * @class SRebar
 * @brief Rebar control
 *
 * SRebar is a SOUI control similar to MFC CRebar, used to implement a draggable, resizable toolbar container.
 * It organizes multiple child windows (bands) together and supports the following features:
 *
 * Main features:
 * - Multi-row layout support: controlled via the singleRow attribute for single-row or multi-row mode
 * - Drag reordering: drag bands via the gripper to change order
 * - Width adjustment: drag the splitter to adjust band width
 * - Collapse function: supports collapsing/expanding bands
 * - Auto wrap: bands can automatically wrap in multi-row mode
 * - Visual feedback: mouse hover highlight and drag state display
 *
 * Usage:
 * 1. Declare the rebar control in the XML layout
 * 2. Add child controls as band content
 * 3. Configure band behavior via attributes (collapse, size limits, etc.)
 *
 * Attributes supported:
 * - singleRow: whether single-row mode (default true)
 * - colorGripper: gripper background color
 * - colorGripperDot: gripper dot color
 */
class SOUI_EXP SRebar : public SWindow {
    DEF_SOBJECT(SWindow, L"rebar")
  public:
    /**
     * @struct BandInfo
     * @brief Rebar band information structure
     *
     * Stores the configuration and state information of each band in the Rebar control
     */
    struct BandInfo
    {
        SStringT title;    /**< Band title text */
        int height;        /**< Band height (pixels) */
        bool bCollapsible; /**< Whether collapsible */
        bool bCollapsed;   /**< Whether currently in collapsed state */
        SWindow *pChild;   /**< Pointer to the band content child window */
        int minHeight;     /**< Minimum height */
        int maxHeight;     /**< Maximum height */
        int width;         /**< Current width (used for horizontal Rebar) */
        int minWidth;      /**< Minimum width */
        int maxWidth;      /**< Maximum width */
        bool bNewRow;      /**< In multi-row mode, true means starting a new row from this band (produced by dragging) */
        BandInfo()
            : height(24)
            , bCollapsible(false)
            , bCollapsed(false)
            , pChild(NULL)
            , minHeight(20)
            , maxHeight(200)
            , width(0)
            , minWidth(40)
            , maxWidth(2000)
            , bNewRow(false)
        {
        }
    };

    SRebar();
    virtual ~SRebar();

    /** Band management methods */

    /**
     * @brief Add a new band to the Rebar control
     * @param pChild Pointer to the child window, used as the band content
     * @param title Band title text
     * @param height Band height, default is 24 pixels
     * @param bCollapsible Whether collapsible, default is false
     * @return Returns the index of the newly added band, or -1 if failed
     */
    int AddBand(SWindow *pChild, const SStringT &title, int height = 24, bool bCollapsible = false);

    /**
     * @brief Remove the band at the specified index
     * @param nIndex Index of the band to remove (starting from 0)
     * @return Returns true on success, false on failure (when index is invalid)
     */
    bool RemoveBand(int nIndex);

    /**
     * @brief Set the height of the specified band
     * @param nIndex Band index (starting from 0)
     * @param height New height value (pixels)
     * @return Returns true on success, false on failure (index invalid or height out of range)
     */
    bool SetBandHeight(int nIndex, int height);

    /**
     * @brief Set the title of the specified band
     * @param nIndex Band index (starting from 0)
     * @param title New title text
     * @return Returns true on success, false on failure (when index is invalid)
     */
    bool SetBandTitle(int nIndex, const SStringT &title);

    /**
     * @brief Set whether the specified band is collapsible
     * @param nIndex Band index (starting from 0)
     * @param bCollapsible true means collapsible, false means not collapsible
     * @return Returns true on success, false on failure (when index is invalid)
     */
    bool SetBandCollapsible(int nIndex, bool bCollapsible);

    /**
     * @brief Collapse or expand the specified band
     * @param nIndex Band index (starting from 0)
     * @param bCollapse true means collapse, false means expand
     * @return Returns true on success, false on failure (index invalid or band not collapsible)
     */
    bool CollapseBand(int nIndex, bool bCollapse);

    /**
     * @brief Check whether the specified band is collapsed
     * @param nIndex Band index (starting from 0)
     * @return Returns true if collapsed, false if not collapsed or index invalid
     */
    bool IsBandCollapsed(int nIndex) const;

    /**
     * @brief Get the total number of bands in the Rebar control
     * @return Band count
     */
    int GetBandCount() const;

    /**
     * @brief Get the detailed information of the specified band
     * @param nIndex Band index (starting from 0)
     * @return Pointer to the BandInfo structure, returns NULL when index is invalid
     */
    BandInfo *GetBandInfo(int nIndex);

  protected:
    /** Message handling methods */
    void OnPaint(IRenderTarget *pRT);
    void OnSize(UINT nType, CSize size);
    BOOL OnMouseHover(UINT nFlags, CPoint point);
    BOOL OnMouseLeave();
    BOOL OnLButtonDown(UINT nFlags, CPoint point);
    BOOL OnLButtonUp(UINT nFlags, CPoint point);
    BOOL OnMouseMove(UINT nFlags, CPoint point);
    int GetTotalHeight() const;
    int GetTotalHeightForWidth(int cx) const;
    virtual BOOL OnSetCursor(const CPoint &pt) OVERRIDE;
    /** Override methods */
    virtual BOOL CreateChildren(SXmlNode xmlNode) OVERRIDE;
    STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE *pSize, int nParentWid, int nParentHei) OVERRIDE;

    virtual BOOL CancelCaptureMode(int reason) override;

    /** Message map */
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_SIZE(OnSize)
        MSG_WM_MOUSEHOVER(OnMouseHover)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
    SOUI_MSG_MAP_END()

    /** Internal methods */
    void CalculateBandLayout();
    void UpdateBandLayout();
    int HitTest(CPoint point, int &nHitTest) const;
    void DrawBand(IRenderTarget *pRT, int nIndex, const CRect &rcBand);
    void DrawGripper(IRenderTarget *pRT, const CRect &rcGripper);

  protected:
    /** Color attributes */
    COLORREF m_crGripper;    /**< gripper color */
    COLORREF m_crGripperDot; /**< gripper dot color */

  protected:
    SArray<BandInfo> m_bands;
    /** Record the current rectangle position of each band, shared by drawing and hit testing, */
    /** Supports "horizontal layout + auto wrap" (VS Rebar style) */
    SArray<CRect> m_bandRects;
    int m_nDragBand;
    int m_nDragPos;        /**< X at press (not used for reorder; used to compute delta when resizing) */
    int m_nDragStartWidth; /**< Band width at press (only used for resize) */
    int m_nHoverBand;
    int m_nHoverPart;
    bool m_bDragging; /**< Dragging in progress (gripper reorder or splitter resize) */
    bool m_bResizing; /**< true=drag splitter to resize, false=drag gripper to reorder */
    CRect m_rcClient;
    bool m_bSingleRow; /**< true=single row (default), false=multi-row (bNewRow from dragging determines wrapping) */

    /** Attribute map */
    SOUI_ATTRS_BEGIN()
        ATTR_BOOL(L"singleRow", m_bSingleRow, FALSE)
        ATTR_COLOR(L"colorGripper", m_crGripper, TRUE)
        ATTR_COLOR(L"colorGripperDot", m_crGripperDot, TRUE)
    SOUI_ATTRS_END()
};

SNSEND

#endif