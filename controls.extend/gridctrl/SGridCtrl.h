#ifndef __SGRIDCTRL__H__
#define __SGRIDCTRL__H__

#include "SSelRangeMgr.h"
#include <core/SPanel.h>
#include <proxy/SPanelProxy.h>


SNSBEGIN


/**
 * @class TplGridInplaceWnd
 * @brief Template class for grid inplace editing windows
 */
template <class T>
class TplGridInplaceWnd : public T, public IGridInplaceWnd
{
    IGridCtrl* m_pGrid;

public:
    STDMETHOD_(long, AddRef)(THIS) override{
        return T::AddRef();
    }

    STDMETHOD_(long, Release)(THIS) override{
        return T::Release();
    }

    STDMETHOD_(void, OnFinalRelease)(THIS) {
        delete this;
    }

public:
    TplGridInplaceWnd(IGridCtrl* pGrid, int nRow, int nCol)
        : m_pGrid(pGrid)
        , m_nRow(nRow)
        , m_nCol(nCol)
        , m_bUpdateData(TRUE)
    {
        IWindow *pGridWnd = pGrid->ToIWindow();
        pGridWnd->InsertIChild(this);
        T::GetStyle().SetScale(pGridWnd->GetScale());
    }

    // IGridInplaceWnd implementation
    STDMETHOD_(IGridCtrl*, GetGrid)(CTHIS) SCONST override{
        return m_pGrid;
    }
    STDMETHOD_(IWindow*, GetIWindow)(CTHIS) SCONST override{
        return (IWindow *)this;
    }
    STDMETHOD_(int, GetRow)(CTHIS) SCONST override{
        return m_nRow;
    }
    STDMETHOD_(int, GetCol)(CTHIS) SCONST override {
        return m_nCol;
    }
    STDMETHOD_(void, UpdateData)(THIS) override {
        // Default implementation for text controls
        SStringT strText = T::GetWindowText();
        GetGrid()->SetItemText(m_nRow, m_nCol, strText);
    }
    STDMETHOD_(void, InitData)(THIS_ LPCTSTR szText) override{
                // Default implementation
        T::SetWindowText(szText ? szText : _T(""));
    }
protected:
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        if (nChar == VK_ESCAPE)
        {
            m_bUpdateData = FALSE;
            T::GetParent()->SetFocus();
        }
        else if (nChar == VK_RETURN || nChar == VK_TAB)
        {
            T::GetParent()->SetFocus();
        }
        else
        {
            T::OnKeyDown(nChar, nRepCnt, nFlags);
        }
    }

    void OnKillFocus(SWND wndFocus)
    {

        T::OnKillFocus(wndFocus);
        GetGrid()->EndEdit(m_bUpdateData);
    }

protected:
    virtual BOOL ProcessSwndMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &lResult) override
    {
        if (uMsg == WM_KEYDOWN)
        {
            OnKeyDown((TCHAR)wParam, (UINT)lParam & 0xFFFF, (UINT)((lParam & 0xFFFF0000) >> 16));
            lResult = 0;
            return TRUE;
        }
        else if (uMsg == WM_KILLFOCUS)
        {
            OnKillFocus((SWND)wParam);
            lResult = 0;
            return TRUE;
        }
        else
        {
            return T::ProcessSwndMessage(uMsg, wParam, lParam, lResult);
        }
    }
protected:
    int m_nRow;
    int m_nCol;
    BOOL m_bUpdateData;
};

/**
 * @struct SMergedCell
 * @brief Merged cell information
 */
struct SMergedCell
{
    SCellRange range;           /**< Merged cell range */
    SCellID masterCell;         /**< Master cell (top-left) */

    SMergedCell() : masterCell(-1, -1) {}
    SMergedCell(const SCellRange& r) : range(r), masterCell(r.minRow, r.minCol) {}

    BOOL IsValid() const { return range.IsValid() && masterCell.IsValid(); }
    BOOL Contains(int row, int col) const
    {
        return row >= range.minRow && row <= range.maxRow &&
               col >= range.minCol && col <= range.maxCol;
    }
    BOOL Contains(const SCellID& cell) const { return Contains(cell.row, cell.col); }
};

/**
 * @enum ECellType
 * @brief Advanced cell types
 */
enum ECellType
{
    CELL_TYPE_TEXT = 0,         /**< Standard text cell */
    CELL_TYPE_URL,              /**< URL/hyperlink cell */
    CELL_TYPE_BUTTON,           /**< Button cell */
    CELL_TYPE_CHECKBOX,         /**< Checkbox cell */
    CELL_TYPE_OPTION,           /**< Combobox cell */
    CELL_TYPE_NUMERIC,          /**< Numeric input cell */
    CELL_TYPE_DATETIME,         /**< Date/time picker cell */
    CELL_TYPE_PROGRESS,         /**< Progress bar cell */
    CELL_TYPE_CUSTOM            /**< Custom cell type */
};

#define DEF_CELL_TYPE(T) \
    public: \
    static BOOL IsCellType(int nType) { return nType == T; } \
    static int GetCellType() { return T; } \
    STDMETHOD_(int, GetType)(CTHIS) SCONST OVERRIDE { return T; }


/**
 * @class SGridCell
 * @brief Default implementation of grid cell
 * @details Concrete implementation of IGridCell with default behavior
 */
class SGridCell : public TObjRefImpl<IGridCell>
{
    DEF_CELL_TYPE(CELL_TYPE_TEXT)
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
    STDMETHOD_(void, Init)(THIS_ IGridCtrl* pGrid, int nRow, int nCol) OVERRIDE;
    STDMETHOD_(IGridCtrl *, GetGrid)(CTHIS) SCONST OVERRIDE{return m_pGrid;}
    STDMETHOD_(int, GetRow)(CTHIS) SCONST OVERRIDE{
        return m_nRow;
    }
    STDMETHOD_(int, GetCol)(CTHIS) SCONST OVERRIDE{
        return m_nCol;
    }
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

    STDMETHOD_(IGridInplaceWnd *, CreateInplaceWnd)(CTHIS_ int nRow,int nCol) SCONST OVERRIDE;

public:
    STDMETHOD_(HRESULT,QueryInterface)(REFIID riid, LPVOID * ppvObject){return E_NOINTERFACE;}
public:
    STDMETHOD_(void, OnMouseEnter)(THIS) OVERRIDE {}
    STDMETHOD_(void, OnMouseOver)(THIS) OVERRIDE {}
    STDMETHOD_(void, OnMouseLeave)(THIS) OVERRIDE {}
    STDMETHOD_(void, OnClickUp)(THIS_ POINT PointCellRelative) OVERRIDE {}
    STDMETHOD_(void, OnClickDown)(THIS_ POINT PointCellRelative) OVERRIDE {}
    STDMETHOD_(void, OnRClick)(THIS_ POINT PointCellRelative) OVERRIDE {}
    STDMETHOD_(void, OnDblClick)(THIS_ POINT PointCellRelative) OVERRIDE {
        if(GetGrid()->IsEditable() && !IsReadOnly()){
            SCellID cellID(GetRow(), GetCol());
            GetGrid()->StartEdit(cellID);
        }
    }
    STDMETHOD_(BOOL, OnSetCursor)(THIS_ POINT PointCellRelative) OVERRIDE { return FALSE; }
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

    IGridCtrl* m_pGrid;         /**< Parent grid */
};

template <class T>
class TplCellOp : public SGridCell, public T{
public:
    STDMETHOD_(long, AddRef)(THIS) {
        return SGridCell::AddRef();
    }

    STDMETHOD_(long, Release)(THIS) {
        return SGridCell::Release();
    }

    STDMETHOD_(void, OnFinalRelease)(THIS) {
        delete this;
    }

    STDMETHOD_(HRESULT,QueryInterface)(REFIID riid, LPVOID * ppvObject){
        if(riid == IID_IGridCell){
            *ppvObject = (IGridCell *)this;
            AddRef();
            return S_OK;
        }else if(riid == T::GetIID()){
            *ppvObject = (T *)this;
            AddRef();
            return S_OK;
        }else{
            return E_NOINTERFACE;
        }
    }
};


/**
 * @class SGridURLCell
 * @brief URL/Hyperlink cell implementation
 */
class SGridURLCell : public TplCellOp<IGridCellUrl>
{
    DEF_CELL_TYPE(CELL_TYPE_URL)
public:
    SGridURLCell();
    virtual ~SGridURLCell();

    // IGridCell interface
    STDMETHOD_(BOOL, Draw)(IRenderTarget* pRT, int nRow, int nCol, RECT rect, BOOL bEraseBkgnd) override;
    STDMETHOD_(void, OnClickUp)(POINT PointCellRelative) override;
    STDMETHOD_(BOOL, OnSetCursor)(POINT PointCellRelative) override;

    STDMETHOD_(IGridInplaceWnd *, CreateInplaceWnd)(CTHIS_ int nRow,int nCol) SCONST OVERRIDE;
    // URL specific methods
    STDMETHOD_(void, SetURL)(LPCTSTR lpszURL) OVERRIDE { m_strURL = lpszURL; }
    STDMETHOD_(LPCTSTR, GetURL)() SCONST OVERRIDE { return m_strURL; }
    STDMETHOD_(void, SetAutoLaunch)(BOOL bAutoLaunch) OVERRIDE { m_bAutoLaunch = bAutoLaunch; }
    STDMETHOD_(BOOL, GetAutoLaunch)() SCONST OVERRIDE { return m_bAutoLaunch; }

protected:
    BOOL HasURL(const SStringT& str) const;
    BOOL OverURL(const CPoint& pt, SStringT& strURL) const;

protected:
    SStringT m_strURL;          /**< URL string */
    COLORREF m_crURLColor;      /**< URL text color */
    BOOL m_bAutoLaunch;         /**< Auto launch URL on click */
    CRect m_rcCell;             /**< Cell rectangle for hit testing */
};

/**
 * @class SGridCheckBoxCell
 * @brief Checkbox cell implementation
 */
class SGridCheckBoxCell : public TplCellOp<IGridCellCheck>
{
    DEF_CELL_TYPE(CELL_TYPE_CHECKBOX)
public:
    SGridCheckBoxCell();
    virtual ~SGridCheckBoxCell();

    // IGridCell interface
    STDMETHOD_(BOOL, Draw)(IRenderTarget* pRT, int nRow, int nCol, RECT rect, BOOL bEraseBkgnd) override;
    STDMETHOD_(void, OnClickUp)(POINT PointCellRelative) override;
    STDMETHOD_(IGridInplaceWnd *, CreateInplaceWnd)(CTHIS_ int nRow,int nCol) SCONST OVERRIDE;

    // Checkbox specific methods
    STDMETHOD_(void, SetCheck)(BOOL bChecked) OVERRIDE { m_bChecked = bChecked; }
    STDMETHOD_(BOOL, GetCheck)() SCONST OVERRIDE { return m_bChecked; }
protected:
    BOOL m_bChecked;            /**< Checkbox state */
    int m_nCheckBoxSize;        /**< Checkbox size */
};

class SGridOptionCell : public SGridCell
{
    DEF_CELL_TYPE(CELL_TYPE_OPTION)
public:
    SGridOptionCell();
    virtual ~SGridOptionCell();

    // IGridCell interface
    STDMETHOD_(BOOL, Draw)(IRenderTarget* pRT, int nRow, int nCol, RECT rect, BOOL bEraseBkgnd) override;
    STDMETHOD_(IGridInplaceWnd *, CreateInplaceWnd)(CTHIS_ int nRow,int nCol) SCONST OVERRIDE;
    STDMETHOD_(void, OnClickUp)(THIS_ POINT PointCellRelative) OVERRIDE;
    STDMETHOD_(void, OnDblClick)(THIS_ POINT PointCellRelative) OVERRIDE;
};

template <class T>
class SGridCellFactory : public TObjRefImpl<IGridCellFactory>{
public:
    STDMETHOD_(int, GetCellType)() const { return T::GetCellType(); }
    STDMETHOD_(IGridCell *, CreateCell)() const { return new T; }
    STDMETHOD_(IGridCellFactory *, Clone)() const { return new SGridCellFactory<T>; }
};

enum{
    EVT_GRID_INIT_INPLACE_WND = EVT_EXTERNAL_BEGIN + 1000,
    EVT_GRID_GET_INPLACE_STYLE,
};

DEF_EVT_EXT(EventGridInitInplaceWnd, EVT_GRID_INIT_INPLACE_WND,{
    IGridCell * pCell;
    IGridInplaceWnd * pInplaceWnd;
    BOOL bInited;
});

DEF_EVT_EXT(EventGridGetInplaceStyle, EVT_GRID_GET_INPLACE_STYLE,{
    IGridCell * pCell;
    IStringW * pstrXmlStyle;
});

class SSelRangeMgr;
/**
 * @class SGridCtrl
 * @brief Grid Control for SOUI
 * @details A fully featured grid control for displaying tabular data, ported from MFC-GridCtrl
 */
class SGridCtrl : public TPanelProxy<IGridCtrl>
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
    STDMETHOD_(BOOL, RedrawCell)(THIS_ int nRow, int nCol) OVERRIDE;
    STDMETHOD_(BOOL, RedrawCell2)(THIS_ SCELLID cell) OVERRIDE;
    STDMETHOD_(BOOL, RedrawRow)(THIS_ int row) OVERRIDE;
    STDMETHOD_(BOOL, RedrawColumn)(THIS_ int col) OVERRIDE;
    STDMETHOD_(BOOL, Refresh)(THIS) OVERRIDE;

    // Cell editing
    STDMETHOD_(BOOL, IsInEdit)(THIS) SCONST OVERRIDE;
    STDMETHOD_(BOOL, GetEditCell)(THIS_ SCELLID *pCell) SCONST OVERRIDE;
    STDMETHOD_(void, StartEdit)(THIS_ SCELLID cell) OVERRIDE;
    STDMETHOD_(BOOL, EndEdit)(THIS_ BOOL bSaveChanges) OVERRIDE;
    STDMETHOD_(IGridInplaceWnd *,GetInplaceWnd)() SCONST OVERRIDE{
        return m_pInplaceWnd;
    }

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

    // Cell merging support
    BOOL MergeCells(int nStartRow, int nStartCol, int nEndRow, int nEndCol);
    BOOL MergeCells(const SCELLRANGE* range);
    BOOL UnmergeCells(int nRow, int nCol);
    BOOL UnmergeCells(const SCellID& cell);
    BOOL IsCellMerged(int nRow, int nCol) const;
    BOOL IsCellMerged(const SCellID& cell) const;
    const SMergedCell* GetMergedCell(int nRow, int nCol) const;
    const SMergedCell* GetMergedCell(const SCellID& cell) const;
    SCellID GetMergedCellMaster(int nRow, int nCol) const;
    SCellID GetMergedCellMaster(const SCellID& cell) const;
    void ClearAllMergedCells();

    // Advanced cell types support
    IGridCell * CreateCellByType(ECellType cellType);
    BOOL SetCellType(int nRow, int nCol, ECellType cellType);
    BOOL SetCellType(const SCellID& cell, ECellType cellType);
    ECellType GetCellType(int nRow, int nCol) const;
    ECellType GetCellType(const SCellID& cell) const;


    /**
     * @brief Get cell with specific interface
     */
    template<class T>
    SAutoRefPtr<T> GetCellOp(int nRow, int nCol) const{
        SAutoRefPtr<T> pCell;
        IGridCell * p = GetCell(nRow, nCol);
        if(p){
            p->QueryInterface(T::GetIID(),(LPVOID*)&pCell);
        }
        return pCell;
    }

    // Selection and highlighting
    STDMETHOD_(void, SelectAll)(THIS) OVERRIDE;
    STDMETHOD_(void, SelectNone)(THIS) OVERRIDE;
    STDMETHOD_(void, SelectRow)(THIS_ int nRow, BOOL bAdd) OVERRIDE;
    STDMETHOD_(void, SelectColumn)(THIS_ int nCol, BOOL bAdd) OVERRIDE;

    // Multi-selection support
    void SelectCells(SCellID currentCell, BOOL bForceRedraw = FALSE);
    void SelectRows(SCellID currentCell, BOOL bForceRedraw = FALSE);
    void SelectColumns(SCellID currentCell, BOOL bForceRedraw = FALSE);
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


    STDMETHOD_(BOOL, RegisterCellFactory)(THIS_ IGridCellFactory *pFactory) OVERRIDE;
    STDMETHOD_(BOOL, UnregisterCellFactory)(THIS_ IGridCellFactory *pFactory) OVERRIDE;

    template<class T>
    BOOL RegisterCellFactory() {
        SGridCellFactory<T> fac;
        return RegisterCellFactory(&fac); 
    }
    template<class T>
    BOOL UnregisterCellFactory() { 
        SGridCellFactory<T> fac;
        return UnregisterCellFactory(&fac); 
    }
    // Inplace editing support
    virtual BOOL StartInplaceEdit(int nRow, int nCol);
    virtual void EndInplaceEdit(BOOL bSave = TRUE);
    virtual BOOL IsInplaceEditing() const { return m_pInplaceWnd != NULL; }

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
    virtual void DestroyCell(int nRow, int nCol);
    virtual IGridCell* GetDefaultCell(BOOL bFixedRow, BOOL bFixedCol) const;

    // Drawing
    virtual void OnPaint(IRenderTarget *pRT);
    virtual void DrawCells(IRenderTarget* pRT, CRect rect);
    virtual void DrawMergedCells(IRenderTarget* pRT, CRect rect);
    virtual void DrawFixedCell(IRenderTarget* pRT, int nRow, int nCol, const CRect& rect);
    virtual void DrawCell(IRenderTarget* pRT, int nRow, int nCol, const CRect& rect);
    virtual void DrawGridLines(IRenderTarget* pRT, const CRect& rect);

    // Layout and positioning
    virtual CRect GetCellRect(int nRow, int nCol) const;
    virtual CRect GetCellRect(const SCellID& cell) const;
    virtual CRect GetCellRectEx(int nRow, int nCol) const;
    virtual CRect GetCellRectEx(const SCellID &cell) const;
    STDMETHOD_(BOOL, GetCellRect)(THIS_ int nRow, int nCol, RECT *pRect) SCONST OVERRIDE;
    STDMETHOD_(BOOL, GetCellRectEx)(THIS_ int nRow, int nCol, RECT *pRect) SCONST OVERRIDE;
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
    virtual void OnCellClick(SCellID cell, UINT nFlags, CPoint pt);
    virtual void OnCellDblClick(SCellID cell, UINT nFlags,CPoint pt);
    virtual void OnCellRClick(SCellID cell, UINT nFlags,CPoint pt);
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
    virtual void OnEditChar(UINT nChar);
    virtual void OnEditKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    virtual BOOL ValidateEdit(SCellID cell, LPCTSTR lpszText);

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
    virtual SStringT FormatCellsAsText(const SSelRangeMgr& ranges) const;
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
    SSelRangeMgr m_SelectedCellRange; /**< Currently selected range(s) */

    // Inplace editing
    BOOL m_bInEdit;                 /**< Currently in edit mode */
    SCellID m_cellEdit;             /**< Cell being edited */
    SAutoRefPtr<IGridInplaceWnd> m_pInplaceWnd;         /**< Current inplace editing window */

    // Sorting
    int m_nSortColumn;              /**< Currently sorted column (-1 if none) */
    ESortOrder m_nSortOrder;        /**< Current sort order */
    BOOL m_bSortOnClick;            /**< Enable sorting on column header click */

    // Cell storage
    SArray<SArray<IGridCell*>*> m_RowData;  /**< 2D array of cells */

    // Default cell implementations
    SAutoRefPtr<IGridCell> m_pDefaultCell;      /**< Default cell for non-fixed cells */
    SAutoRefPtr<IGridCell> m_pDefaultFixedCell; /**< Default cell for fixed cells */

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
    SCellID m_lastHoverCell;        /**< last mouse hover cell */
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

    // Cell merging
    SArray<SMergedCell> m_MergedCells;  /**< Array of merged cells */

    // Scrolling
    CSize m_sizeGrid;               /**< Total grid size */

    SArray<SAutoRefPtr<IGridCellFactory> > m_cellFactories; 

};

SNSEND

#endif // __SGRIDCTRL__H__
