#ifndef __SGRIDCTRL__H__
#define __SGRIDCTRL__H__

#include <core/SPanel.h>
#include <interface/SCtrl-i.h>
#include <proxy/SPanelProxy.h>
#include <control/SHeaderCtrl.h>
#include <control/SEdit.h>

SNSBEGIN

// Forward declarations
class SGridCellBase;
class SGridCell;
class SGridCtrl;

/**
 * @class SInPlaceEdit
 * @brief In-place edit control for grid cells
 */
class SOUI_EXP SInPlaceEdit : public SEdit
{
    DEF_SOBJECT(SEdit, L"inplaceedit");

public:
    SInPlaceEdit();
    virtual ~SInPlaceEdit();

    // Initialize the edit control
    BOOL Create(SGridCtrl* pParent, const CRect& rect, int nRow, int nCol,
                LPCTSTR szInitText, UINT nFirstChar);

    // End editing
    void EndEdit();

protected:
    void WINAPI OnFinalRelease() override;

  protected:
    // Message handlers
    void OnKillFocus(SWND wndFocus);
    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
        MSG_WM_CHAR(OnChar)
        MSG_WM_KEYDOWN(OnKeyDown)
    SOUI_MSG_MAP_END()

private:
    int m_nRow;
    int m_nCol;
    SStringT m_sInitText;
    UINT m_nLastChar;
    BOOL m_bExitOnArrows;
    SGridCtrl* m_pGrid;
};

/**
 * @struct SCellID
 * @brief Cell identifier structure
 * @details Structure to identify a specific cell in the grid by row and column
 */
struct SCellID : SCELLID
{
    
    SCellID(int nRow = -1, int nCol = -1) 
    {
        row = nRow;
        col = nCol;
    }
    SCellID(const SCELLID& rhs)
    {
        row = rhs.row;
        col = rhs.col;
    }
    
    BOOL IsValid() const { return (row >= 0 && col >= 0); }
    BOOL operator==(const SCellID& rhs) const { return (row == rhs.row && col == rhs.col); }
    BOOL operator!=(const SCellID& rhs) const { return !(*this == rhs); }

    void operator=(const SCELLID& rhs)
    {
        row = rhs.row;
        col = rhs.col;
    }
};

/**
 * @struct SCellRange
 * @brief Cell range structure
 * @details Structure to define a range of cells in the grid
 */
struct SCellRange : SCELLRANGE
{
    
    SCellRange(int nMinRow = -1, int nMinCol = -1, int nMaxRow = -1, int nMaxCol = -1)
    {
        minRow = nMinRow;
        minCol = nMinCol;
        maxRow = nMaxRow;
        maxCol = nMaxCol;
    }
    
    BOOL IsValid() const { return (minRow >= 0 && minCol >= 0 && maxRow >= minRow && maxCol >= minCol); }
    BOOL InRange(int row, int col) const { return (row >= minRow && row <= maxRow && col >= minCol && col <= maxCol); }
    BOOL InRange(const SCellID& cellID) const { return InRange(cellID.row, cellID.col); }
    
    SCellID GetTopLeft() const { return SCellID(minRow, minCol); }
    int GetRowSpan() const { return maxRow - minRow + 1; }
    int GetColSpan() const { return maxCol - minCol + 1; }
};


/**
 * @class SGridCellBase
 * @brief Base class for grid cells
 * @details Abstract base class that defines the interface for grid cells
 */
class SOUI_EXP SGridCellBase : public TObjRefImpl<IGridCell>
{
public:
    SGridCellBase();
    virtual ~SGridCellBase();

public:
    STDMETHOD_(void, OnEndEdit)(THIS) OVERRIDE {}
    STDMETHOD_(void, OnMouseEnter)(THIS) OVERRIDE {}
    STDMETHOD_(void, OnMouseOver)(THIS) OVERRIDE {}
    STDMETHOD_(void, OnMouseLeave)(THIS) OVERRIDE {}
    STDMETHOD_(void, OnClick)(THIS_ POINT PointCellRelative) OVERRIDE {}
    STDMETHOD_(void, OnClickDown)(THIS_ POINT PointCellRelative) OVERRIDE {}
    STDMETHOD_(void, OnRClick)(THIS_ POINT PointCellRelative) OVERRIDE {}
    STDMETHOD_(void, OnDblClick)(THIS_ POINT PointCellRelative) OVERRIDE {}
    STDMETHOD_(BOOL, OnSetCursor)(THIS) OVERRIDE { return FALSE; }
};

class SGridCtrl;
/**
 * @class SGridCell
 * @brief Default implementation of grid cell
 * @details Concrete implementation of IGridCell with default behavior
 */
class SOUI_EXP SGridCell : public SGridCellBase
{
    friend class SGridCtrl;
public:
    /**
     * @brief Constructor
     */
    SGridCell();
    
    /**
     * @brief Destructor
     */
    virtual ~SGridCell();

    // IGridCell interface implementation
    STDMETHOD_(void, SetText)(THIS_ LPCTSTR szText) OVERRIDE;
    STDMETHOD_(void, SetImage)(THIS_ int nImage) OVERRIDE;
    STDMETHOD_(void, SetData)(THIS_ LPARAM lParam) OVERRIDE;
    STDMETHOD_(void, SetState)(THIS_ DWORD nState) OVERRIDE;
    STDMETHOD_(void, SetFormat)(THIS_ DWORD nFormat) OVERRIDE;
    STDMETHOD_(void, SetTextClr)(THIS_ COLORREF clr) OVERRIDE;
    STDMETHOD_(void, SetBackClr)(THIS_ COLORREF clr) OVERRIDE;
    STDMETHOD_(void, SetFont)(THIS_ const LOGFONT* plf) OVERRIDE;
    STDMETHOD_(void, SetMargin)(THIS_ UINT nMargin) OVERRIDE;
    STDMETHOD_(void, SetReadOnly)(THIS_ BOOL bReadOnly) OVERRIDE;
    
    STDMETHOD_(LPCTSTR, GetText)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(LPCTSTR, GetTipText)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(int, GetImage)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(LPARAM, GetData)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(DWORD, GetState)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(DWORD, GetFormat)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(COLORREF, GetTextClr)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(COLORREF, GetBackClr)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(LOGFONT*, GetFont)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(UINT, GetMargin)(CTHIS) SCONST OVERRIDE;

    STDMETHOD_(BOOL, IsEditing)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(BOOL, IsFocused)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(BOOL, IsFixed)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(BOOL, IsFixedCol)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(BOOL, IsFixedRow)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(BOOL, IsSelected)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(BOOL, IsReadOnly)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(BOOL, IsModified)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(BOOL, IsDropHighlighted)(CTHIS) SCONST OVERRIDE;

    STDMETHOD_(void, Reset)(THIS) OVERRIDE;
    STDMETHOD_(BOOL, Draw)(THIS_ IRenderTarget* pRT, int nRow, int nCol, RECT rect, BOOL bEraseBkgnd DEF_VAL(TRUE)) OVERRIDE;
    STDMETHOD_(BOOL, GetTextRect)(THIS_ LPRECT pRect) OVERRIDE;
    STDMETHOD_(void, GetTextExtent)(THIS_ LPCTSTR str, SIZE* pSize) OVERRIDE;
    STDMETHOD_(void, GetCellExtent)(THIS_ IRenderTarget* pRT, SIZE* pSize) OVERRIDE;

    STDMETHOD_(BOOL, Edit)(THIS_ int nRow, int nCol, RECT rect, POINT point, UINT nID, UINT nChar) OVERRIDE;
    STDMETHOD_(void, EndEdit)(THIS) OVERRIDE;
    STDMETHOD_(BOOL, ValidateEdit)(THIS_ LPCTSTR str) OVERRIDE;

    STDMETHOD_(BOOL, PrintCell)(THIS_ IRenderTarget* pRT, int nRow, int nCol, RECT rect) OVERRIDE;

protected:
    SStringT m_strText;         /**< Cell text */
    int m_nImage;               /**< Image index */
    LPARAM m_lParam;            /**< User data */
    DWORD m_nState;             /**< Cell state */
    DWORD m_nFormat;            /**< Text format */
    COLORREF m_crFgClr;         /**< Foreground color */
    COLORREF m_crBkClr;         /**< Background color */
    LOGFONT m_lfFont;           /**< Font */
    UINT m_nMargin;             /**< Cell margin */
    BOOL m_bDefaultFont;        /**< Using default font flag */

    int m_nRow;                 /**< Row index */
    int m_nCol;                 /**< Column index */

    SGridCtrl* m_pGrid;         /**< Parent grid */
};

/**
 * @class SGridCtrl
 * @brief Grid Control for SOUI
 * @details A fully featured grid control for displaying tabular data, ported from MFC-GridCtrl
 */
class SOUI_EXP SGridCtrl : public TPanelProxy<IGridCtrl>
{
    DEF_SOBJECT(SPanel, L"gridctrl")

public:
    /**
     * @brief Constructor
     */
    SGridCtrl();

    /**
     * @brief Destructor
     */
    virtual ~SGridCtrl();

    // Grid dimensions
    STDMETHOD_(int, GetRowCount)(THIS) SCONST OVERRIDE;
    STDMETHOD_(int, GetColumnCount)(THIS) SCONST OVERRIDE;
    STDMETHOD_(int, GetFixedRowCount)(THIS) SCONST OVERRIDE;
    STDMETHOD_(int, GetFixedColumnCount)(THIS) SCONST OVERRIDE;
    STDMETHOD_(BOOL, SetRowCount)(THIS_ int nRows) OVERRIDE;
    STDMETHOD_(BOOL, SetColumnCount)(THIS_ int nCols) OVERRIDE;
    STDMETHOD_(BOOL, SetFixedRowCount)(THIS_ int nFixedRows) OVERRIDE;
    STDMETHOD_(BOOL, SetFixedColumnCount)(THIS_ int nFixedCols) OVERRIDE;

    // Grid appearance
    STDMETHOD_(int, GetRowHeight)(THIS_ int nRow) SCONST OVERRIDE;
    STDMETHOD_(int, GetColumnWidth)(THIS_ int nCol) SCONST OVERRIDE;
    STDMETHOD_(BOOL, SetRowHeight)(THIS_ int nRow, int height) OVERRIDE;
    STDMETHOD_(BOOL, SetColumnWidth)(THIS_ int nCol, int width) OVERRIDE;
    STDMETHOD_(int, GetDefCellHeight)(THIS) SCONST OVERRIDE;
    STDMETHOD_(int, GetDefCellWidth)(THIS) SCONST OVERRIDE;
    STDMETHOD_(void, SetDefCellHeight)(THIS_ int nHeight) OVERRIDE;
    STDMETHOD_(void, SetDefCellWidth)(THIS_ int nWidth) OVERRIDE;

    // Image list support
    STDMETHOD_(void, SetImageList)(THIS_ ISkinObj* pSkin) OVERRIDE;
    STDMETHOD_(ISkinObj*, GetImageList)(THIS) SCONST OVERRIDE;

    // Grid lines and appearance
    STDMETHOD_(int, GetGridLines)(THIS) SCONST OVERRIDE;
    STDMETHOD_(void, SetGridLines)(THIS_ int nWhichLines) OVERRIDE;
    STDMETHOD_(COLORREF, GetGridLineColor)(THIS) SCONST OVERRIDE;
    STDMETHOD_(void, SetGridLineColor)(THIS_ COLORREF clr) OVERRIDE;
    STDMETHOD_(COLORREF, GetGridBkColor)(THIS) SCONST OVERRIDE;
    STDMETHOD_(void, SetGridBkColor)(THIS_ COLORREF clr) OVERRIDE;

    // Grid behavior
    STDMETHOD_(BOOL, IsEditable)(THIS) SCONST;
    STDMETHOD_(void, SetEditable)(THIS_ BOOL bEditable);
    STDMETHOD_(BOOL, GetListMode)(THIS) SCONST;
    STDMETHOD_(void, SetListMode)(THIS_ BOOL bEnableListMode);
    STDMETHOD_(BOOL, GetSingleRowSelection)(THIS) SCONST;
    STDMETHOD_(void, SetSingleRowSelection)(THIS_ BOOL bSing);
    STDMETHOD_(BOOL, GetSingleColSelection)(THIS) SCONST;
    STDMETHOD_(void, SetSingleColSelection)(THIS_ BOOL bSing);

    // Cell operations
    STDMETHOD_(IGridCell*, GetCell)(THIS_ int nRow, int nCol) SCONST;
    STDMETHOD_(BOOL, SetItem)(THIS_ const SGV_ITEM* pItem);
    STDMETHOD_(BOOL, GetItem)(THIS_ SGV_ITEM* pItem) SCONST;
    STDMETHOD_(BOOL, SetItemText)(THIS_ int nRow, int nCol, LPCTSTR str);
    STDMETHOD_(BOOL, GetItemText)(THIS_ int nRow, int nCol, IStringT *str) SCONST;
    STDMETHOD_(BOOL, SetItemData)(THIS_ int nRow, int nCol, LPARAM lParam);
    STDMETHOD_(LPARAM, GetItemData)(THIS_ int nRow, int nCol) SCONST;

    // Selection
    STDMETHOD_(BOOL, GetFocusCell)(CTHIS_ SCELLID *cell) SCONST OVERRIDE;
    STDMETHOD_(BOOL, SetFocusCell)(THIS_ int nRow, int nCol) OVERRIDE;
    STDMETHOD_(BOOL, SetFocusCell2)(THIS_ SCELLID cell) OVERRIDE;
    STDMETHOD_(int, GetSelectedCount)(THIS) SCONST OVERRIDE;
    STDMETHOD_(BOOL, GetSelectedCellRange)(CTHIS_ SCELLRANGE *pRange) SCONST OVERRIDE;
    STDMETHOD_(void, SetSelectedRange)(THIS_ int nMinRow, int nMinCol, int nMaxRow, int nMaxCol, BOOL bForceRepaint) OVERRIDE;
    STDMETHOD_(void, SetSelectedRange2)(THIS_ const SCELLRANGE* pRange, BOOL bForceRepaint) OVERRIDE;

    // Operations
    STDMETHOD_(int, InsertColumn)(THIS_ LPCTSTR strHeading, UINT nFormat, int nColumn);
    STDMETHOD_(int, InsertRow)(THIS_ LPCTSTR strHeading, int nRow);
    STDMETHOD_(BOOL, DeleteColumn)(THIS_ int nColumn);
    STDMETHOD_(BOOL, DeleteRow)(THIS_ int nRow);
    STDMETHOD_(BOOL, DeleteAllItems)(THIS);
    STDMETHOD_(BOOL, DeleteNonFixedRows)(THIS);

    // Auto-sizing
    STDMETHOD_(BOOL, AutoSizeRow)(THIS_ int nRow, BOOL bResetScroll);
    STDMETHOD_(BOOL, AutoSizeColumn)(THIS_ int nCol, UINT nAutoSizeStyle, BOOL bResetScroll);
    STDMETHOD_(void, AutoSizeRows)(THIS);
    STDMETHOD_(void, AutoSizeColumns)(THIS_ UINT nAutoSizeStyle);
    STDMETHOD_(void, AutoSize)(THIS_ UINT nAutoSizeStyle);

    // Validation and utilities
    STDMETHOD_(BOOL, IsValid)(THIS_ int nRow, int nCol) SCONST OVERRIDE;
    STDMETHOD_(BOOL, IsValid2)(THIS_ SCELLID cell) SCONST OVERRIDE;
    STDMETHOD_(BOOL, IsRangeValid)(THIS_ const SCELLRANGE* range) SCONST OVERRIDE;
    STDMETHOD_(BOOL, IsCellVisible)(THIS_ SCELLID cell) SCONST OVERRIDE;
    STDMETHOD_(BOOL, SetCellEditable)(THIS_ SCELLID cell, BOOL bEditable) OVERRIDE;
    STDMETHOD_(BOOL, IsCellEditable)(THIS_ SCELLID cell) SCONST OVERRIDE;
    STDMETHOD_(BOOL, IsCellSelected)(THIS_ SCELLID cell) SCONST OVERRIDE;
    STDMETHOD_(BOOL, IsCellFixed)(THIS_ int nRow, int nCol) SCONST OVERRIDE;
    STDMETHOD_(BOOL, IsCellFixed2)(THIS_ SCELLID cell) SCONST OVERRIDE;
    // Scrolling and view
    STDMETHOD_(void, EnsureVisible)(THIS_ int nRow, int nCol) OVERRIDE;
    STDMETHOD_(void, EnsureVisible2)(THIS_ SCELLID cell) OVERRIDE;
    STDMETHOD_(BOOL, GetCellRange)(CTHIS_ SCELLRANGE *pRange) SCONST OVERRIDE;
    STDMETHOD_(BOOL, GetVisibleCellRange)(CTHIS_ SCELLRANGE *pRange) SCONST OVERRIDE;
    STDMETHOD_(BOOL, GetCellFromPt)(THIS_ POINT point, BOOL bAllowFixedCells, SCELLID *pCell) SCONST OVERRIDE;

    // Refresh and redraw
    STDMETHOD_(BOOL, RedrawCell)(THIS_ int nRow, int nCol, IRenderTarget* pRT) OVERRIDE;
    STDMETHOD_(BOOL, RedrawCell2)(THIS_ const SCELLID cell, IRenderTarget* pRT) OVERRIDE;
    STDMETHOD_(BOOL, RedrawRow)(THIS_ int row) OVERRIDE;
    STDMETHOD_(BOOL, RedrawColumn)(THIS_ int col) OVERRIDE;
    STDMETHOD_(BOOL, Refresh)(THIS) OVERRIDE;

    // Cell editing
    STDMETHOD_(BOOL, IsInEdit)(THIS) SCONST OVERRIDE;
    STDMETHOD_(BOOL, GetEditCell)(THIS_ SCELLID *pCell) SCONST OVERRIDE;
    STDMETHOD_(void, StartEdit)(THIS_ SCELLID cell) OVERRIDE;
    STDMETHOD_(BOOL, EndEdit)(THIS_ BOOL bSaveChanges) OVERRIDE;

    // Internal editing support
    void OnEndEdit(int nRow, int nCol, LPCTSTR szText, UINT nLastChar);

    // Drag and drop support
    virtual void OnBeginDrag();
    virtual void OnDragEnter();
    virtual void OnDragOver(CPoint point);
    virtual void OnDragLeave();
    virtual BOOL OnDrop(CPoint point, LPCTSTR lpszText);
    BOOL PasteTextToCell(SCellID cell, LPCTSTR lpszText);
    BOOL PasteMultiCellData(SCellID startCell, LPCTSTR lpszData);
    void SplitString(const SStringT& str, LPCTSTR delimiter, SArray<SStringT>& result);

    // Row/Column resizing support
    BOOL MouseOverResizeArea(const CPoint& point,BOOL bColumn);
    void StartRowResize(int nRow, CPoint point);
    void StartColumnResize(int nCol, CPoint point);
    void DoRowResize(CPoint point);
    void DoColumnResize(CPoint point);
    void EndResize();

    // Selection and highlighting
    STDMETHOD_(void, SelectAll)(THIS) OVERRIDE;
    STDMETHOD_(void, SelectNone)(THIS) OVERRIDE;
    STDMETHOD_(void, SelectRow)(THIS_ int nRow, BOOL bAdd) OVERRIDE;
    STDMETHOD_(void, SelectColumn)(THIS_ int nCol, BOOL bAdd) OVERRIDE;

    // Multi-selection support
    void SelectCells(SCellID currentCell, BOOL bForceRedraw = FALSE, BOOL bSelectCells = TRUE);
    void SelectRows(SCellID currentCell, BOOL bForceRedraw = FALSE, BOOL bSelectCells = TRUE);
    void SelectColumns(SCellID currentCell, BOOL bForceRedraw = FALSE, BOOL bSelectCells = TRUE);
    void OnSelecting(const SCellID& currentCell);

    // Sorting
    STDMETHOD_(BOOL, SortItems)(THIS_ int nCol, int order) OVERRIDE;
    STDMETHOD_(int, GetSortColumn)(THIS) SCONST OVERRIDE;
    STDMETHOD_(int, GetSortOrder)(THIS) SCONST OVERRIDE;
    STDMETHOD_(void, SetSortOnClick)(THIS_ BOOL bSort) OVERRIDE;
    STDMETHOD_(BOOL, GetSortOnClick)(THIS) SCONST OVERRIDE;

    // Clipboard operations
    STDMETHOD_(BOOL, Copy)(THIS) OVERRIDE;
    STDMETHOD_(BOOL, Cut)(THIS) OVERRIDE;
    STDMETHOD_(BOOL, Paste)(THIS) OVERRIDE;
    STDMETHOD_(BOOL, CanCopy)(THIS) SCONST OVERRIDE;
    STDMETHOD_(BOOL, CanCut)(THIS) SCONST OVERRIDE;
    STDMETHOD_(BOOL, CanPaste)(THIS) SCONST OVERRIDE;

  protected:
    SCellID GetCellFromPt(POINT point, BOOL bAllowFixedCells) const;
    HRESULT OnAttrRowCount(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrColCount(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrFixedRows(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrFixedCols(const SStringW &strValue, BOOL bLoading);

    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"rowCount", OnAttrRowCount)
        ATTR_CUSTOM(L"colCount", OnAttrColCount)
        ATTR_CUSTOM(L"fixedRows", OnAttrFixedRows)
        ATTR_CUSTOM(L"fixedCols", OnAttrFixedCols)
        ATTR_INT(L"defRowHeight", m_nDefCellHeight, FALSE)
        ATTR_INT(L"defColWidth", m_nDefCellWidth, FALSE)
        ATTR_INT(L"gridLines", m_nGridLines, FALSE)
        ATTR_COLOR(L"gridLineColor", m_crGridLineColor, FALSE)
        ATTR_COLOR(L"gridBkColor", m_crGridBkColor, FALSE)
        ATTR_BOOL(L"editable", m_bEditable, FALSE)
        ATTR_BOOL(L"listMode", m_bListMode, FALSE)
        ATTR_BOOL(L"singleRowSel", m_bSingleRowSelection, FALSE)
        ATTR_BOOL(L"singleColSel", m_bSingleColSelection, FALSE)
    SOUI_ATTRS_END()

protected:
    // Virtual functions for customization
    virtual IGridCell* CreateCell(int nRow, int nCol);
    virtual void DestroyCell(int nRow, int nCol);
    virtual SGridCellBase* GetDefaultCell(BOOL bFixedRow, BOOL bFixedCol) const;

    // Drawing
    virtual void OnPaint(IRenderTarget *pRT);
    virtual void DrawCells(IRenderTarget* pRT, const CRect& rcClient);
    virtual void DrawFixedCell(IRenderTarget* pRT, int nRow, int nCol, CRect rect);
    virtual void DrawCell(IRenderTarget* pRT, int nRow, int nCol, CRect rect);
    virtual void DrawGridLines(IRenderTarget* pRT, const CRect& rect);

    // Layout and positioning
    virtual CRect GetCellRect(int nRow, int nCol) const;
    virtual void RecalcLayout();
    virtual void UpdateScrollBars();

    // Event handlers
    virtual void OnSize(UINT nType, CSize size);
    virtual void OnLButtonDown(UINT nFlags, CPoint pt);
    virtual void OnLButtonUp(UINT nFlags, CPoint pt);
    virtual void OnLButtonDbClick(UINT nFlags, CPoint pt);
    virtual void OnRButtonDown(UINT nFlags, CPoint pt);
    virtual void OnMouseMove(UINT nFlags, CPoint pt);
    virtual void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    virtual BOOL OnScroll(BOOL bVertical, UINT uCode, int nPos);

    // Cell event handlers
    virtual void OnFixedCellClick(SCellID cell);
    virtual void OnCellClick(SCellID cell, UINT nFlags);
    virtual void OnCellDblClick(SCellID cell, UINT nFlags);
    virtual void OnCellRClick(SCellID cell, UINT nFlags);
    virtual void OnCellMouseEnter(SCellID cell);
    virtual void OnCellMouseLeave(SCellID cell);
    virtual void OnCellMouseOver(SCellID cell);
    virtual void OnHeaderClick(int nCol);
    virtual void OnRowHeaderClick(int nRow);

    // Selection helpers
    virtual void ToggleCellSelection(SCellID cell);
    virtual void ExtendSelection(SCellID cell);

    // Keyboard navigation
    virtual BOOL OnKeyLeft(SCellID& newFocus, BOOL bShift, BOOL bCtrl);
    virtual BOOL OnKeyRight(SCellID& newFocus, BOOL bShift, BOOL bCtrl);
    virtual BOOL OnKeyUp(SCellID& newFocus, BOOL bShift, BOOL bCtrl);
    virtual BOOL OnKeyDownNav(SCellID& newFocus, BOOL bShift, BOOL bCtrl);
    virtual BOOL OnKeyHome(SCellID& newFocus, BOOL bShift, BOOL bCtrl);
    virtual BOOL OnKeyEnd(SCellID& newFocus, BOOL bShift, BOOL bCtrl);
    virtual BOOL OnKeyPageUp(SCellID& newFocus, BOOL bShift, BOOL bCtrl);
    virtual BOOL OnKeyPageDown(SCellID& newFocus, BOOL bShift, BOOL bCtrl);
    virtual BOOL OnKeyTab(SCellID& newFocus, BOOL bShift, BOOL bCtrl);
    virtual BOOL OnKeyReturn(SCellID& newFocus, BOOL bShift, BOOL bCtrl);
    virtual BOOL OnKeyEscape(SCellID& newFocus, BOOL bShift, BOOL bCtrl);
    virtual BOOL OnKeyDelete(SCellID& newFocus, BOOL bShift, BOOL bCtrl);
    virtual BOOL OnKeyF2(SCellID& newFocus, BOOL bShift, BOOL bCtrl);
    virtual BOOL OnKeyChar(UINT nChar, SCellID& newFocus, BOOL bShift, BOOL bCtrl);
    virtual BOOL MoveFocusAndSelection(const SCellID& newFocus, BOOL bExtendSelection);

    // Cell editing (non-interface methods)
    virtual SWindow* CreateEditControl(SCellID cell, CRect rect, UINT nID);
    virtual void OnEditChar(UINT nChar);
    virtual void OnEditKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    virtual BOOL ValidateEdit(SCellID cell, LPCTSTR lpszText);
    virtual void OnEndEdit(SCellID cell, LPCTSTR lpszText, BOOL bCancelled);

    // Selection and highlighting (non-interface methods)
    virtual void InvalidateSelection();
    virtual void HighlightCell(int nRow, int nCol, BOOL bHighlight);
    virtual void HighlightRange(const SCellRange& range, BOOL bHighlight);
    virtual CRect GetRangeRect(const SCellRange& range) const;
    virtual BOOL IsRangeVisible(const SCellRange& range) const;

    // Sorting (non-interface methods)
    virtual BOOL SortTextItems(int nCol, ESortOrder order);
    virtual BOOL SortNumericItems(int nCol, ESortOrder order);
    virtual void DrawSortArrow(IRenderTarget* pRT, CRect rect, int order);

    // Clipboard operations (non-interface methods)
    virtual SStringT GetClipboardText() const;
    virtual BOOL SetClipboardText(LPCTSTR lpszText);
    virtual SStringT FormatCellsAsText(const SCellRange& range) const;
    virtual BOOL ParseTextIntoCells(LPCTSTR lpszText, const SCellRange& range);

    // Cell management (implemented methods only)
    void ClearCells();
    int GetFixedColsWidth() const;
    int GetFixedRowsHeight() const;
    void ResetSelectedRange();

    BOOL OnSetCursor(const CPoint &pt) override;

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_SIZE(OnSize)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDbClick)
        MSG_WM_RBUTTONDOWN(OnRButtonDown)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_KEYDOWN(OnKeyDown)
    SOUI_MSG_MAP_END()

protected:
    // Grid dimensions
    int m_nRows;                    /**< Number of rows */
    int m_nCols;                    /**< Number of columns */
    int m_nFixedRows;               /**< Number of fixed rows */
    int m_nFixedCols;               /**< Number of fixed columns */

    // Cell dimensions
    int m_nDefCellWidth;            /**< Default cell width */
    int m_nDefCellHeight;           /**< Default cell height */
    SArray<int> m_arRowHeights;     /**< Array of row heights */
    SArray<int> m_arColWidths;      /**< Array of column widths */

    // Appearance
    int m_nGridLines;               /**< Grid line display options */
    COLORREF m_crGridLineColor;     /**< Grid line color */
    COLORREF m_crGridBkColor;       /**< Grid background color */

    // Behavior
    BOOL m_bEditable;               /**< Whether grid is editable */
    BOOL m_bListMode;               /**< List mode flag */
    BOOL m_bSingleRowSelection;     /**< Single row selection mode */
    BOOL m_bSingleColSelection;     /**< Single column selection mode */

    // Selection and focus
    SCellID m_cellFocus;            /**< Currently focused cell */
    SCellRange m_SelectedCellRange; /**< Currently selected range */

    // Editing
    BOOL m_bInEdit;                 /**< Currently in edit mode */
    SCellID m_cellEdit;             /**< Cell being edited */
    SAutoRefPtr<SWindow> m_pEditWnd;            /**< Edit control window */

    // Sorting
    int m_nSortColumn;              /**< Currently sorted column (-1 if none) */
    ESortOrder m_nSortOrder;        /**< Current sort order */
    BOOL m_bSortOnClick;            /**< Enable sorting on column header click */

    // Cell storage
    SArray<SArray<SGridCellBase*>*> m_RowData;  /**< 2D array of cells */

    // Default cell implementations
    SGridCell* m_pDefaultCell;      /**< Default cell for non-fixed cells */
    SGridCell* m_pDefaultFixedCell; /**< Default cell for fixed cells */

    // Image list
    ISkinObj* m_pImageList;         /**< Image list for cell icons */

    // Mouse operations and selection
    enum MouseMode {
        MOUSE_NOTHING = 0,
        MOUSE_SELECT_ALL,
        MOUSE_SELECT_COL,
        MOUSE_SELECT_ROW,
        MOUSE_SELECT_CELLS,
        MOUSE_SCROLLING_CELLS,
        MOUSE_OVER_ROW_DIVIDE,
        MOUSE_SIZING_ROW,
        MOUSE_OVER_COL_DIVIDE,
        MOUSE_SIZING_COL,
        MOUSE_PREPARE_EDIT,
        MOUSE_PREPARE_DRAG,
        MOUSE_DRAGGING
    };

    int m_MouseMode;                /**< Current mouse mode */
    BOOL m_bLMouseButtonDown;       /**< Left mouse button state */
    BOOL m_bRMouseButtonDown;       /**< Right mouse button state */
    CPoint m_LeftClickDownPoint;    /**< Point where left click started */
    CPoint m_LastMousePoint;        /**< Last mouse position */
    SCellID m_LeftClickDownCell;    /**< Cell where left click started */
    SCellID m_SelectionStartCell;   /**< Cell where selection started */
    SCellID m_LastDragOverCell;     /**< Last cell dragged over */

    // Drag and drop
    BOOL m_bAllowDragAndDrop;       /**< Enable drag and drop */
    BOOL m_bDragRowMode;            /**< Enable row drag mode */
    SStringT m_strDragData;         /**< Data being dragged */

    // Row/Column resizing
    BOOL m_bAllowRowResize;         /**< Allow row height resizing */
    BOOL m_bAllowColumnResize;      /**< Allow column width resizing */
    int m_nResizeCaptureRange;      /**< Pixel range for resize detection */

    // Scrolling
    CSize m_sizeGrid;               /**< Total grid size */
};

SNSEND

#endif // __SGRIDCTRL__H__
