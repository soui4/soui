/**
 * @file       SSplitWnd.h
 * @brief      Definition of the SSplitWnd and related classes.
 * @version    v1.0
 * @author     soui
 * @date       2014-07-08
 *
 * @copyright  Copyright (C) 2014-2050 SOUI团队
 *             All rights reserved.
 */

#ifndef __SSPLITWND__H__
#define __SSPLITWND__H__

#include <core/SWnd.h>
#include <layout/SLayoutSize.h>

SNSBEGIN

/**
 * @class SSplitPane
 * @brief A pane within a split window.
 *
 * This class represents a pane within a split window, which can have ideal, minimum, and maximum sizes.
 */
class SOUI_EXP SSplitPane : public SWindow {
    friend class SSplitWnd;
    DEF_SOBJECT(SWindow, L"pane")

  public:
    /**
     * @brief Constructor for SSplitPane.
     */
    SSplitPane();

    /**
     * @brief Destructor for SSplitPane.
     */
    virtual ~SSplitPane()
    {
    }

    /**
     * @brief Moves the pane to a new position.
     * @param rc The new rectangle position.
     */
    void Move(CRect rc);

    SOUI_ATTRS_BEGIN()
        ATTR_LAYOUTSIZE(L"idealSize", m_nSizeIdeal, FALSE) /**< Ideal size of the pane. */
        ATTR_LAYOUTSIZE(L"minSize", m_nSizeMin, FALSE)     /**< Minimum size of the pane. */
        ATTR_LAYOUTSIZE(L"maxSize", m_nSizeMax, FALSE)     /**< Maximum size of the pane. */
        ATTR_INT(L"priority", m_nPriority, FALSE)          /**< Priority of the pane. Higher priority means the pane will reach its ideal size first. */
    SOUI_ATTRS_END()

  protected:
    SLayoutSize m_nSizeIdeal; /**< Ideal size of the pane. */
    SLayoutSize m_nSizeMin;   /**< Minimum size of the pane. */
    SLayoutSize m_nSizeMax;   /**< Maximum size of the pane. */
    int m_nPriority;          /**< Priority of the pane. Higher priority means the pane will reach its ideal size first. */
};

/**
 * @class SSplitWnd
 * @brief A split window that can contain multiple panes.
 *
 * This class represents a split window that can be divided into multiple panes, which can be adjusted in size.
 */
class SOUI_EXP SSplitWnd : public SWindow {
    DEF_SOBJECT(SWindow, L"splitwnd")

    /**
     * @enum SPLIT_ORINTATION
     * @brief Orientation of the split window.
     */
    enum SPLIT_ORINTATION
    {
        Horizontal = 0, /**< Horizontal orientation. */
        Vertical,       /**< Vertical orientation. */
    };

    /**
     * @struct PANESIZE
     * @brief Size information for a pane.
     */
    struct PANESIZE
    {
        int actural;   /**< Actual size of the pane. */
        int preferred; /**< Preferred size of the pane. */
        int minimum;   /**< Minimum size of the pane. */
        int maximum;   /**< Maximum size of the pane. */
    };

    typedef SArray<PANESIZE> PANESIZELIST; /**< List of pane sizes. */

  protected:
    typedef SArray<SSplitPane *> SplitPaneList;
    SplitPaneList m_lstPane;     /**< List of panes sorted by display order. */
    SplitPaneList m_lstPriority; /**< List of panes sorted by priority. */
    int m_spliterSize;           /**< Size of the splitter bar. */
    int m_orintation;            /**< Orientation of the split window, see SPLIT_ORINTATION. */

    BOOL m_bAdjustable;               /**< Flag indicating if the panes are adjustable in size. */
    SAutoRefPtr<ISkinObj> m_pSkinSep; /**< Skin object for the splitter bar. */

    CPoint m_ptDragPrev; /**< Previous position during dragging. */
    int m_iDragSep;      /**< Index of the splitter bar being dragged, -1 if no dragging. */

  public:
    /**
     * @brief Constructor for SSplitWnd.
     */
    SSplitWnd(void);

    /**
     * @brief Destructor for SSplitWnd.
     */
    virtual ~SSplitWnd(void);

    /**
     * @brief Retrieves a pane by index.
     * @param iPane The index of the pane.
     * @return Pointer to the SSplitPane object.
     */
    SSplitPane *GetPane(UINT iPane);

    /**
     * @brief Shows a pane.
     * @param iPane The index of the pane to show.
     * @return TRUE if successful, otherwise FALSE.
     */
    BOOL ShowPane(UINT iPane);

    /**
     * @brief Hides a pane.
     * @param iPane The index of the pane to hide.
     * @return TRUE if successful, otherwise FALSE.
     */
    BOOL HidePane(UINT iPane);

    /**
     * @brief Retrieves the index of a pane by its name.
     * @param strName The name of the pane.
     * @return Index of the pane, or -1 if not found.
     */
    int PaneIndex(const SStringW &strName) const;

    /**
     * @brief Inserts a new pane.
     * @param pane Pointer to the SSplitPane object to insert.
     * @param index The index at which to insert the pane, default is -1 (end).
     * @return Index of the inserted pane.
     */
    int InsertItem(SSplitPane *pane, int index = -1);

    /**
     * @brief Removes a pane.
     * @param pane Pointer to the SSplitPane object to remove.
     */
    void RemoveItem(SSplitPane *pane);

  protected:
    /**
     * @brief Retrieves the layout rectangle for child windows.
     * @param prc Pointer to the rectangle to receive the layout rectangle.
     */
    STDMETHOD_(void, GetChildrenLayoutRect)(THIS_ RECT *prc) SCONST OVERRIDE;

    /**
     * @brief Updates the positions of child windows.
     */
    STDMETHOD_(void, UpdateChildrenPosition)(THIS) OVERRIDE;

    /**
     * @brief Creates child windows from an XML node.
     * @param xmlNode The XML node containing the child window definitions.
     * @return TRUE if successful, otherwise FALSE.
     */
    virtual BOOL CreateChildren(SXmlNode xmlNode);

    /**
     * @brief Sets the cursor.
     * @param pt The mouse position.
     * @return TRUE if the cursor was set, otherwise FALSE.
     */
    virtual BOOL OnSetCursor(const CPoint &pt);

    /**
     * @brief Paints the window.
     * @param pRT Pointer to the render target.
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief Handles left mouse button down events.
     * @param nFlags Flags associated with the mouse event.
     * @param pt The mouse position.
     */
    void OnLButtonDown(UINT nFlags, CPoint pt);

    /**
     * @brief Handles left mouse button up events.
     * @param nFlags Flags associated with the mouse event.
     * @param pt The mouse position.
     */
    void OnLButtonUp(UINT nFlags, CPoint pt);

    /**
     * @brief Handles mouse move events.
     * @param nFlags Flags associated with the mouse event.
     * @param pt The mouse position.
     */
    void OnMouseMove(UINT nFlags, CPoint pt);

  protected:
    /**
     * @brief Comparison function for qsort.
     * @param p1 Pointer to the first element.
     * @param p2 Pointer to the second element.
     * @return Comparison result.
     */
    static int FunComp(const void *p1, const void *p2);

    /**
     * @brief Sorts the pane list by priority.
     * @param lstPane The list of panes to sort.
     */
    void SortPriorityList(SplitPaneList &lstPane);

    /**
     * @brief Fetches size information for the panes.
     * @param lstPane The list of panes.
     * @param lstPaneSize The list to store the size information.
     */
    void FatchPaneSizeInfo(const SplitPaneList &lstPane, PANESIZELIST &lstPaneSize);

    /**
     * @brief Resets the positions of the panes based on size information.
     * @param lstPane The list of panes.
     * @param lstPanePriority The list of panes sorted by priority.
     * @param lstPaneSize The list of pane size information.
     * @param offset The offset to apply.
     * @return The remaining space after resetting positions.
     */
    int ResetPanesPostion(SplitPaneList &lstPane, SplitPaneList &lstPanePriority, PANESIZELIST &lstPaneSize, int offset);

    /**
     * @brief Adjusts the sizes of the panes.
     * @param lstPriority The list of panes sorted by priority.
     * @param remain The remaining space to distribute.
     * @return The remaining space after adjustment.
     */
    int AdjustPanesSize(PANESIZELIST &lstPriority, int remain);

    /**
     * @brief Relayouts the panes within the window.
     * @param rc The new rectangle for the window.
     * @param lstPaneSize The list of pane size information.
     */
    void Relayout(const CRect &rc, PANESIZELIST lstPaneSize = PANESIZELIST());

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"sepSize", m_spliterSize, TRUE)     /**< Size of the splitter bar. */
        ATTR_SKIN(L"sepSkin", m_pSkinSep, TRUE)       /**< Skin object for the splitter bar. */
        ATTR_BOOL(L"adjustable", m_bAdjustable, TRUE) /**< Flag indicating if the panes are adjustable in size. */
        ATTR_INT(L"vertical", m_orintation, TRUE)     /**< Orientation of the split window, see SPLIT_ORINTATION. */
    SOUI_ATTRS_END()

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
    SOUI_MSG_MAP_END()
};

/**
 * @class SSplitWnd_Col
 * @brief A vertical split window.
 *
 * This class represents a vertical split window, where panes are stacked vertically.
 */
class SOUI_EXP SSplitWnd_Col : public SSplitWnd {
    DEF_SOBJECT(SSplitWnd, L"splitcol")

  public:
    /**
     * @brief Constructor for SSplitWnd_Col.
     */
    SSplitWnd_Col()
    {
        m_pSkinSep = GETBUILTINSKIN(SKIN_SYS_SPLIT_VERT);
        m_orintation = Vertical;
    }
};

/**
 * @class SSplitWnd_Row
 * @brief A horizontal split window.
 *
 * This class represents a horizontal split window, where panes are arranged horizontally.
 */
class SOUI_EXP SSplitWnd_Row : public SSplitWnd {
    DEF_SOBJECT(SSplitWnd, L"splitrow")

  public:
    /**
     * @brief Constructor for SSplitWnd_Row.
     */
    SSplitWnd_Row()
    {
        m_pSkinSep = GETBUILTINSKIN(SKIN_SYS_SPLIT_HORZ);
        m_orintation = Horizontal;
    }
};

SNSEND

#endif // __SSPLITWND__H__