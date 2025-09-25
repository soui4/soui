#ifndef __SGRIDCTRL__I__H__
#define __SGRIDCTRL__I__H__

#include <interface/SSkinobj-i.h>
#include <interface/SCtrl-i.h>

SNSBEGIN

typedef struct _SCELLID
{
    int row;    /**< Row index */
    int col;    /**< Column index */
}SCELLID;

typedef struct _SCELLRANGE
{
    int minRow, minCol;    /**< Top-left cell */
    int maxRow, maxCol;    /**< Bottom-right cell */
}SCELLRANGE;


/**
 * @struct SGV_ITEM
 * @brief Grid view item structure
 * @details Structure for getting/setting cell data, similar to MFC's GV_ITEM
 */
typedef struct _SGV_ITEM
{
    int row, col;           /**< Row and column of item */
    UINT mask;              /**< Mask for use in getting/setting cell data */
    UINT state;             /**< Cell state (focus/highlighted etc) */
    UINT nFormat;           /**< Format of cell */
    LPCTSTR szText;        /**< Text in cell */
    int     cchText;        /**< Length of text */
    int iImage;             /**< Index of the cell's icon */
    COLORREF crBkClr;       /**< Background colour (or CLR_DEFAULT) */
    COLORREF crFgClr;       /**< Foreground colour (or CLR_DEFAULT) */
    LPARAM lParam;          /**< 32-bit value to associate with item */
    LOGFONT lfFont;         /**< Cell font */
}SGV_ITEM;


typedef struct IGridCtrl IGridCtrl;
/**
 * @interface IGridInplaceWnd
 * @brief Interface for grid inplace editing windows
 */
#undef INTERFACE
#define INTERFACE IGridInplaceWnd
DECLARE_INTERFACE_(IGridInplaceWnd, IObjRef)
{
        /**
     * @brief Increases the reference count.
     * @return The new reference count.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decreases the reference count.
     * @return The new reference count.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Called when the reference count reaches zero and the object is about to be released.
     * @details This method is intended to perform any necessary cleanup before the object is deleted.
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    //---------------------------------------------------------

    STDMETHOD_(IGridCtrl*, GetGrid)(CTHIS) SCONST PURE;
    STDMETHOD_(IWindow*, GetIWindow)(CTHIS) SCONST PURE;
    STDMETHOD_(int, GetRow)(CTHIS) SCONST PURE;
    STDMETHOD_(int, GetCol)(CTHIS) SCONST PURE;
    STDMETHOD_(void, UpdateData)(THIS) PURE;
    STDMETHOD_(void, InitData)(THIS_ LPCTSTR szText) PURE;
};

// {F2A8B7C3-4D5E-4F6A-8B9C-1D2E3F4A5B6C}
DEFINE_SGUID2(IID_IGridCell, 0xf2a8b7c3, 0x4d5e, 0x4f6a, 0x8b, 0x9c, 0x1d, 0x2e, 0x3f, 0x4a, 0x5b, 0x6d);
#undef INTERFACE
#define INTERFACE IGridCell
DECLARE_INTERFACE_(IGridCell, IObjRef)
{
    DECLARE_CLASS_SIID(IID_IGridCell)
    
    /**
     * @brief Increases the reference count.
     * @return The new reference count.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decreases the reference count.
     * @return The new reference count.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Called when the reference count reaches zero and the object is about to be released.
     * @details This method is intended to perform any necessary cleanup before the object is deleted.
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    STDMETHOD_(HRESULT, QueryInterface)(REFIID riid, void** ppvObject) PURE;
    //--------------------------------------------------------------------
    STDMETHOD_(void, Init)(THIS_ IGridCtrl* pGrid, int nRow, int nCol) PURE;
    STDMETHOD_(int, GetType)(CTHIS) SCONST PURE;
    STDMETHOD_(IGridCtrl *, GetGrid)(CTHIS) SCONST PURE;
    STDMETHOD_(int, GetRow)(CTHIS) SCONST PURE;
    STDMETHOD_(int, GetCol)(CTHIS) SCONST PURE;
    // Attributes
    STDMETHOD_(void, SetText)(THIS_ LPCTSTR szText) PURE;
    STDMETHOD_(void, SetImage)(THIS_ int nImage) PURE;
    STDMETHOD_(void, SetData)(THIS_ LPARAM lParam) PURE;
    STDMETHOD_(void, SetState)(THIS_ DWORD nState) PURE;
    STDMETHOD_(void, SetFormat)(THIS_ DWORD nFormat) PURE;
    STDMETHOD_(void, SetTextClr)(THIS_ COLORREF clr) PURE;
    STDMETHOD_(void, SetBackClr)(THIS_ COLORREF clr) PURE;
    STDMETHOD_(void, SetFont)(THIS_ const LOGFONT* plf) PURE;
    STDMETHOD_(void, SetMargin)(THIS_ UINT nMargin) PURE;
    STDMETHOD_(void, SetReadOnly)(THIS_ BOOL bReadOnly) PURE;
    STDMETHOD_(LPCTSTR, GetText)(CTHIS) SCONST PURE;
    STDMETHOD_(LPCTSTR, GetTipText)(CTHIS) SCONST PURE;
    STDMETHOD_(int, GetImage)(CTHIS) SCONST PURE;
    STDMETHOD_(LPARAM, GetData)(CTHIS) SCONST PURE;
    STDMETHOD_(DWORD, GetState)(CTHIS) SCONST PURE;
    STDMETHOD_(DWORD, GetFormat)(CTHIS) SCONST PURE;
    STDMETHOD_(COLORREF, GetTextClr)(CTHIS) SCONST PURE;
    STDMETHOD_(COLORREF, GetBackClr)(CTHIS) SCONST PURE;
    STDMETHOD_(LOGFONT*, GetFont)(CTHIS) SCONST PURE;
    STDMETHOD_(UINT, GetMargin)(CTHIS) SCONST PURE;
    // State queries
    STDMETHOD_(BOOL, IsEditing)(CTHIS) SCONST PURE;
    STDMETHOD_(BOOL, IsFocused)(CTHIS) SCONST PURE;
    STDMETHOD_(BOOL, IsFixed)(CTHIS) SCONST PURE;
    STDMETHOD_(BOOL, IsFixedCol)(CTHIS) SCONST PURE;
    STDMETHOD_(BOOL, IsFixedRow)(CTHIS) SCONST PURE;
    STDMETHOD_(BOOL, IsSelected)(CTHIS) SCONST PURE;
    STDMETHOD_(BOOL, IsReadOnly)(CTHIS) SCONST PURE;
    STDMETHOD_(BOOL, IsModified)(CTHIS) SCONST PURE;
    STDMETHOD_(BOOL, IsDropHighlighted)(CTHIS) SCONST PURE;

    // Operations
    STDMETHOD_(void, Reset)(THIS) PURE;
    STDMETHOD_(BOOL, Draw)(THIS_ IRenderTarget* pRT, int nRow, int nCol, RECT rect, BOOL bEraseBkgnd DEF_VAL(TRUE)) PURE;
    STDMETHOD_(BOOL, GetTextRect)(THIS_ LPRECT pRect) PURE;
    STDMETHOD_(void, GetTextExtent)(THIS_ LPCTSTR str, SIZE* pSize) PURE;
    STDMETHOD_(void, GetCellExtent)(THIS_ IRenderTarget* pRT, SIZE* pSize) PURE;

    // Editing
    STDMETHOD_(BOOL, Edit)(THIS_ int nRow, int nCol, RECT rect, POINT point, UINT nID, UINT nChar) PURE;
    STDMETHOD_(void, EndEdit)(THIS) PURE;
    STDMETHOD_(BOOL, ValidateEdit)(THIS_ LPCTSTR str) PURE;

    // Printing
    STDMETHOD_(BOOL, PrintCell)(THIS_ IRenderTarget* pRT, int nRow, int nCol, RECT rect) PURE;

    // Overridables
    STDMETHOD_(void, OnMouseEnter)(THIS) PURE;
    STDMETHOD_(void, OnMouseOver)(THIS) PURE;
    STDMETHOD_(void, OnMouseLeave)(THIS) PURE;
    STDMETHOD_(void, OnClickDown)(THIS_ POINT PointCellRelative) PURE;
    STDMETHOD_(void, OnClickUp)(THIS_ POINT PointCellRelative) PURE;
    STDMETHOD_(void, OnRClick)(THIS_ POINT PointCellRelative) PURE;
    STDMETHOD_(void, OnDblClick)(THIS_ POINT PointCellRelative) PURE;
    STDMETHOD_(BOOL, OnSetCursor)(THIS_ POINT PointCellRelative) PURE;

    STDMETHOD_(IGridInplaceWnd *, CreateInplaceWnd)(CTHIS_ int nRow,int nCol) SCONST PURE;
};


#undef INTERFACE
#define INTERFACE IGridCellFactory
DECLARE_INTERFACE_(IGridCellFactory,IObjRef)
{
    /**
     * @brief 增加引用计数
     * @return 新引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief 减少引用计数
     * @return 新引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief 释放对象
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    //----------------------------------------------------
    STDMETHOD_(int, GetCellType)(CTHIS) SCONST PURE;
    STDMETHOD_(IGridCell *, CreateCell)(CTHIS) SCONST PURE;
    STDMETHOD_(IGridCellFactory *, Clone)(CTHIS) SCONST PURE;
};

// Cell data mask flags
#define SGVIF_TEXT      0x0001    /**< Cell text will be accessed */
#define SGVIF_IMAGE     0x0002    /**< Cell image number will be accessed */
#define SGVIF_PARAM     0x0004    /**< Cell user data (lParam) will be accessed */
#define SGVIF_STATE     0x0008    /**< Cell state will be accessed */
#define SGVIF_BKCLR     0x0010    /**< Cell background colour will be accessed */
#define SGVIF_FGCLR     0x0020    /**< Cell foreground colour will be accessed */
#define SGVIF_FORMAT    0x0040    /**< Cell format field will be accessed */
#define SGVIF_FONT      0x0080    /**< Cell logical font will be accessed */
#define SGVIF_ALL       0x00FF    /**< All information will be accessed */

// Cell states
#define SGVIS_FOCUSED       0x0001    /**< Cell has focus */
#define SGVIS_SELECTED      0x0002    /**< Cell is selected */
#define SGVIS_DROPHILITED   0x0004    /**< Cell is drop highlighted */
#define SGVIS_READONLY      0x0008    /**< Cell is read-only and cannot be edited */
#define SGVIS_FIXED         0x0010    /**< Cell is fixed */
#define SGVIS_FIXEDROW      0x0020    /**< Cell is part of a fixed row */
#define SGVIS_FIXEDCOL      0x0040    /**< Cell is part of a fixed column */
#define SGVIS_MODIFIED      0x0080    /**< Cell has been modified */

// Grid line options
#define SGVL_NONE       0x0000    /**< No grid lines */
#define SGVL_HORZ       0x0001    /**< Horizontal lines only */
#define SGVL_VERT       0x0002    /**< Vertical lines only */
#define SGVL_BOTH       0x0003    /**< Both vertical and horizontal lines */

// Sort order
typedef enum _ESortOrder
{
    SORT_NONE = 0,          /**< No sorting */
    SORT_ASCENDING = 1,     /**< Ascending sort */
    SORT_DESCENDING = 2     /**< Descending sort */
}ESortOrder;

// Auto-sizing options
#define SGVS_DEFAULT    0x0000    /**< Default */
#define SGVS_HEADER     0x0001    /**< Size using column fixed cells data only */
#define SGVS_DATA       0x0002    /**< Size using column non-fixed cells data only */
#define SGVS_BOTH       0x0003    /**< Size using column fixed and non-fixed */


// {F2A8B7C3-4D5E-4F6A-8B9C-1D2E3F4A5B6C}
DEFINE_SGUID2(IID_IGridCtrl, 0xf2a8b7c3, 0x4d5e, 0x4f6a, 0x8b, 0x9c, 0x1d, 0x2e, 0x3f, 0x4a, 0x5b, 0x6c);
#undef INTERFACE
#define INTERFACE IGridCtrl
DECLARE_INTERFACE_(IGridCtrl, ICtrl)
{
    DECLARE_CLASS_SIID(IID_IGridCtrl)
    /**
     * @brief 增加引用计数
     * @return 新引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief 减少引用计数
     * @return 新引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief 释放对象
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;

    //////////////////////////////////////////////////////////////////////////
    // Grid dimensions
    STDMETHOD_(int, GetRowCount)(THIS) SCONST PURE;
    STDMETHOD_(int, GetColumnCount)(THIS) SCONST PURE;
    STDMETHOD_(int, GetFixedRowCount)(THIS) SCONST PURE;
    STDMETHOD_(int, GetFixedColumnCount)(THIS) SCONST PURE;
    STDMETHOD_(BOOL, SetRowCount)(THIS_ int nRows) PURE;
    STDMETHOD_(BOOL, SetColumnCount)(THIS_ int nCols) PURE;
    STDMETHOD_(BOOL, SetFixedRowCount)(THIS_ int nFixedRows) PURE;
    STDMETHOD_(BOOL, SetFixedColumnCount)(THIS_ int nFixedCols) PURE;
    // Grid appearance
    STDMETHOD_(int, GetRowHeight)(THIS_ int nRow) SCONST PURE;
    STDMETHOD_(int, GetColumnWidth)(THIS_ int nCol) SCONST PURE;
    STDMETHOD_(BOOL, SetRowHeight)(THIS_ int nRow, int height) PURE;
    STDMETHOD_(BOOL, SetColumnWidth)(THIS_ int nCol, int width) PURE;
    STDMETHOD_(int, GetDefCellHeight)(THIS) SCONST PURE;
    STDMETHOD_(int, GetDefCellWidth)(THIS) SCONST PURE;
    STDMETHOD_(void, SetDefCellHeight)(THIS_ int nHeight) PURE;
    STDMETHOD_(void, SetDefCellWidth)(THIS_ int nWidth) PURE;

    // Image list support
    STDMETHOD_(void, SetImageList)(THIS_ ISkinObj* pSkin) PURE;
    STDMETHOD_(ISkinObj*, GetImageList)(THIS) SCONST PURE;

    // Grid lines and appearance
    STDMETHOD_(int, GetGridLines)(THIS) SCONST PURE;
    STDMETHOD_(void, SetGridLines)(THIS_ int nWhichLines) PURE;
    STDMETHOD_(COLORREF, GetGridLineColor)(THIS) SCONST PURE;
    STDMETHOD_(void, SetGridLineColor)(THIS_ COLORREF clr) PURE;
    STDMETHOD_(COLORREF, GetGridBkColor)(THIS) SCONST PURE;
    STDMETHOD_(void, SetGridBkColor)(THIS_ COLORREF clr) PURE;

    // Grid behavior
    STDMETHOD_(BOOL, IsEditable)(THIS) SCONST PURE;
    STDMETHOD_(void, SetEditable)(THIS_ BOOL bEditable) PURE;
    STDMETHOD_(BOOL, GetListMode)(THIS) SCONST PURE;
    STDMETHOD_(void, SetListMode)(THIS_ BOOL bEnableListMode) PURE;
    STDMETHOD_(BOOL, GetSingleRowSelection)(THIS) SCONST PURE;
    STDMETHOD_(void, SetSingleRowSelection)(THIS_ BOOL bSing) PURE;
    STDMETHOD_(BOOL, GetSingleColSelection)(THIS) SCONST PURE;
    STDMETHOD_(void, SetSingleColSelection)(THIS_ BOOL bSing) PURE;

    // Cell operations
    STDMETHOD_(IGridCell*, GetCell)(THIS_ int nRow, int nCol) SCONST PURE;
    STDMETHOD_(BOOL, SetItem)(THIS_ const SGV_ITEM* pItem) PURE;
    STDMETHOD_(BOOL, GetItem)(THIS_ SGV_ITEM* pItem) SCONST PURE;
    STDMETHOD_(BOOL, SetItemText)(THIS_ int nRow, int nCol, LPCTSTR str) PURE;
    STDMETHOD_(BOOL, GetItemText)(THIS_ int nRow, int nCol, IStringT *str) SCONST PURE;
    STDMETHOD_(BOOL, SetItemData)(THIS_ int nRow, int nCol, LPARAM lParam) PURE;
    STDMETHOD_(LPARAM, GetItemData)(THIS_ int nRow, int nCol) SCONST PURE;

    // Selection
    STDMETHOD_(BOOL, GetFocusCell)(CTHIS_ SCELLID *cell) SCONST PURE;
    STDMETHOD_(BOOL, SetFocusCell)(THIS_ int nRow, int nCol) PURE;
    STDMETHOD_(BOOL, SetFocusCell2)(THIS_ SCELLID cell) PURE;
    STDMETHOD_(int, GetSelectedCount)(THIS) SCONST PURE;
    STDMETHOD_(BOOL, GetSelectedCellRange)(CTHIS_ SCELLRANGE *pRange) SCONST PURE;
    STDMETHOD_(void, SetSelectedRange)(THIS_ int nMinRow, int nMinCol, int nMaxRow, int nMaxCol, BOOL bForceRepaint) PURE;
    STDMETHOD_(void, SetSelectedRange2)(THIS_ const SCELLRANGE* pRange, BOOL bForceRepaint) PURE;

    // Operations
    STDMETHOD_(int, InsertColumn)(THIS_ LPCTSTR strHeading, UINT nFormat, int nColumn) PURE;
    STDMETHOD_(int, InsertRow)(THIS_ LPCTSTR strHeading, int nRow) PURE;
    STDMETHOD_(BOOL, DeleteColumn)(THIS_ int nColumn) PURE;
    STDMETHOD_(BOOL, DeleteRow)(THIS_ int nRow) PURE;
    STDMETHOD_(BOOL, DeleteAllItems)(THIS) PURE;
    STDMETHOD_(BOOL, DeleteNonFixedRows)(THIS) PURE;

    // Auto-sizing
    STDMETHOD_(BOOL, AutoSizeRow)(THIS_ int nRow, BOOL bResetScroll) PURE;
    STDMETHOD_(BOOL, AutoSizeColumn)(THIS_ int nCol, UINT nAutoSizeStyle, BOOL bResetScroll) PURE;
    STDMETHOD_(void, AutoSizeRows)(THIS) PURE;
    STDMETHOD_(void, AutoSizeColumns)(THIS_ UINT nAutoSizeStyle) PURE;
    STDMETHOD_(void, AutoSize)(THIS_ UINT nAutoSizeStyle) PURE;

    // Validation and utilities
    STDMETHOD_(BOOL, IsValid)(THIS_ int nRow, int nCol) SCONST PURE;
    STDMETHOD_(BOOL, IsValid2)(THIS_ SCELLID cell) SCONST PURE;
    STDMETHOD_(BOOL, IsRangeValid)(THIS_ const SCELLRANGE* range) SCONST PURE;
    STDMETHOD_(BOOL, IsCellVisible)(THIS_ SCELLID cell) SCONST PURE;
    STDMETHOD_(BOOL, SetCellEditable)(THIS_ SCELLID cell, BOOL bEditable) PURE;
    STDMETHOD_(BOOL, IsCellEditable)(THIS_ SCELLID cell) SCONST PURE;
    STDMETHOD_(BOOL, IsCellSelected)(THIS_ SCELLID cell) SCONST PURE;
    STDMETHOD_(BOOL, IsCellFixed)(THIS_ int nRow, int nCol) SCONST PURE;
    STDMETHOD_(BOOL, IsCellFixed2)(THIS_ SCELLID cell) SCONST PURE;

    // Scrolling and view
    STDMETHOD_(void, EnsureVisible)(THIS_ int nRow, int nCol) PURE;
    STDMETHOD_(void, EnsureVisible2)(THIS_ SCELLID cell) PURE;
    STDMETHOD_(BOOL, GetCellRange)(CTHIS_ SCELLRANGE *pRange) SCONST PURE;
    STDMETHOD_(BOOL, GetVisibleCellRange)(CTHIS_ SCELLRANGE *pRange) SCONST PURE;
    STDMETHOD_(BOOL, GetCellFromPt)(THIS_ POINT point, BOOL bAllowFixedCells, SCELLID *pCell) SCONST PURE;

    // Refresh and redraw
    STDMETHOD_(BOOL, RedrawCell)(THIS_ int nRow, int nCol) PURE;
    STDMETHOD_(BOOL, RedrawCell2)(THIS_ SCELLID cell) PURE;
    STDMETHOD_(BOOL, RedrawRow)(THIS_ int row) PURE;
    STDMETHOD_(BOOL, RedrawColumn)(THIS_ int col) PURE;
    STDMETHOD_(BOOL, Refresh)(THIS) PURE;

    // Cell editing
    STDMETHOD_(BOOL, IsInEdit)(THIS) SCONST PURE;
    STDMETHOD_(BOOL, GetEditCell)(THIS_ SCELLID *pCell) SCONST PURE;
    STDMETHOD_(void, StartEdit)(THIS_ SCELLID cell) PURE;
    STDMETHOD_(BOOL, EndEdit)(THIS_ BOOL bSaveChanges) PURE;

    // Selection and highlighting
    STDMETHOD_(void, SelectAll)(THIS) PURE;
    STDMETHOD_(void, SelectNone)(THIS) PURE;
    STDMETHOD_(void, SelectRow)(THIS_ int nRow, BOOL bAdd) PURE;
    STDMETHOD_(void, SelectColumn)(THIS_ int nCol, BOOL bAdd) PURE;

    // Sorting
    STDMETHOD_(BOOL, SortItems)(THIS_ int nCol, int order) PURE;
    STDMETHOD_(int, GetSortColumn)(THIS) SCONST PURE;
    STDMETHOD_(int, GetSortOrder)(THIS) SCONST PURE;
    STDMETHOD_(void, SetSortOnClick)(THIS_ BOOL bSort) PURE;
    STDMETHOD_(BOOL, GetSortOnClick)(THIS) SCONST PURE;

    // Clipboard operations
    STDMETHOD_(BOOL, Copy)(THIS) PURE;
    STDMETHOD_(BOOL, Cut)(THIS) PURE;
    STDMETHOD_(BOOL, Paste)(THIS) PURE;
    STDMETHOD_(BOOL, CanCopy)(THIS) SCONST PURE;
    STDMETHOD_(BOOL, CanCut)(THIS) SCONST PURE;
    STDMETHOD_(BOOL, CanPaste)(THIS) SCONST PURE;

    // Cell Factory
    STDMETHOD_(BOOL, RegisterCellFactory)(THIS_ IGridCellFactory *pFactory) PURE;
    STDMETHOD_(BOOL, UnregisterCellFactory)(THIS_ IGridCellFactory *pFactory) PURE;
    
    STDMETHOD_(BOOL, GetCellRect)(THIS_ int nRow, int nCol, RECT *pRect) SCONST PURE;
    STDMETHOD_(BOOL, GetCellRectEx)(THIS_ int nRow, int nCol, RECT *pRect) SCONST PURE;
    STDMETHOD_(IGridInplaceWnd *,GetInplaceWnd)() SCONST PURE;
};

// {F2A8B7C3-4D5E-4F6A-8B9C-1D2E3F4A5B00}
DEFINE_SGUID2(IID_IGridCellUrl, 0xf2a8b7c3, 0x4d5e, 0x4f6a, 0x8b, 0x9c, 0x1d, 0x2e, 0x3f, 0x4a, 0x5b, 0x00);

#undef INTERFACE
#define INTERFACE IGridCellUrl
DECLARE_INTERFACE_(IGridCellUrl, IObjRef)
{
    DECLARE_CLASS_SIID(IID_IGridCellUrl)
    /**
     * @brief 增加引用计数
     * @return 新引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief 减少引用计数
     * @return 新引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief 释放对象
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;
    //--------------------------------------------------------------------------
    STDMETHOD_(BOOL, GetAutoLaunch)() SCONST PURE;
    STDMETHOD_(LPCTSTR, GetURL)() SCONST PURE;
    STDMETHOD_(void, SetAutoLaunch)(BOOL bAutoLaunch) PURE;
    STDMETHOD_(void, SetURL)(LPCTSTR lpszURL) PURE;
};



// {F2A8B7C3-4D5E-4F6A-8B9C-1D2E3F4A5B01}
DEFINE_SGUID2(IID_IGridCellCheck, 0xf2a8b7c3, 0x4d5e, 0x4f6a, 0x8b, 0x9c, 0x1d, 0x2e, 0x3f, 0x4a, 0x5b, 0x01);

#undef INTERFACE
#define INTERFACE IGridCellCheck
DECLARE_INTERFACE_(IGridCellCheck, IObjRef)
{
    DECLARE_CLASS_SIID(IID_IGridCellCheck)
    /**
     * @brief 增加引用计数
     * @return 新引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief 减少引用计数
     * @return 新引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief 释放对象
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;
    //--------------------------------------------------------------------------
    STDMETHOD_(BOOL, GetCheck)() SCONST PURE;
    STDMETHOD_(void, SetCheck)(BOOL bCheck) PURE;
};

// {F2A8B7C3-4D5E-4F6A-8B9C-1D2E3F4A5B02}
DEFINE_SGUID2(IID_IGridCellColor, 0xf2a8b7c3, 0x4d5e, 0x4f6a, 0x8b, 0x9c, 0x1d, 0x2e, 0x3f, 0x4a, 0x5b, 0x02);

#undef INTERFACE
#define INTERFACE IGridCellColor
DECLARE_INTERFACE_(IGridCellColor, IObjRef)
{
    DECLARE_CLASS_SIID(IID_IGridCellColor)
    /**
     * @brief 增加引用计数
     * @return 新引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief 减少引用计数
     * @return 新引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief 释放对象
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;
    //--------------------------------------------------------------------------

    /**
     * @brief 获取颜色值
     * @return 颜色值 (COLORREF)
     */
    STDMETHOD_(COLORREF, GetColor)() SCONST PURE;

    /**
     * @brief 设置颜色值
     * @param cr 颜色值
     */
    STDMETHOD_(void, SetColor)(COLORREF cr) PURE;

    /**
     * @brief 获取颜色格式字符串
     * @return 格式字符串 (如 "#%02X%02X%02X")
     */
    STDMETHOD_(LPCTSTR, GetColorFormat)() SCONST PURE;

    /**
     * @brief 设置颜色格式字符串
     * @param lpszFormat 格式字符串
     */
    STDMETHOD_(void, SetColorFormat)(LPCTSTR lpszFormat) PURE;

    /**
     * @brief 显示颜色选择器
     * @param pRect 单元格矩形区域
     * @return 是否成功显示
     */
    STDMETHOD_(BOOL, ShowColorPicker)(const RECT* pRect) PURE;
};

// {F2A8B7C3-4D5E-4F6A-8B9C-1D2E3F4A5B03}
DEFINE_SGUID2(IID_IGridCellDateTime, 0xf2a8b7c3, 0x4d5e, 0x4f6a, 0x8b, 0x9c, 0x1d, 0x2e, 0x3f, 0x4a, 0x5b, 0x03);

#undef INTERFACE
#define INTERFACE IGridCellDateTime
DECLARE_INTERFACE_(IGridCellDateTime, IObjRef)
{
    DECLARE_CLASS_SIID(IID_IGridCellDateTime)
    /**
     * @brief 增加引用计数
     * @return 新引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief 减少引用计数
     * @return 新引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief 释放对象
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;
    //--------------------------------------------------------------------------

    /**
     * @brief 获取日期时间值
     * @param pSysTime 输出的系统时间结构
     */
    STDMETHOD_(void, GetDateTime)(SYSTEMTIME* pSysTime) SCONST PURE;

    /**
     * @brief 设置日期时间值
     * @param pSysTime 系统时间结构
     */
    STDMETHOD_(void, SetDateTime)(const SYSTEMTIME* pSysTime) PURE;

    /**
     * @brief 获取日期时间格式字符串
     * @return 格式字符串
     */
    STDMETHOD_(LPCTSTR, GetDateTimeFormat)() SCONST PURE;

    /**
     * @brief 设置日期时间格式字符串
     * @param lpszFormat 格式字符串
     */
    STDMETHOD_(void, SetDateTimeFormat)(LPCTSTR lpszFormat) PURE;

    /**
     * @brief 获取是否启用时间部分
     * @return 是否启用时间
     */
    STDMETHOD_(BOOL, IsTimeEnabled)() SCONST PURE;

    /**
     * @brief 设置是否启用时间部分
     * @param bEnable 是否启用时间
     */
    STDMETHOD_(void, SetTimeEnabled)(BOOL bEnable) PURE;

    /**
     * @brief 检查是否有有效的日期时间值
     * @return 是否有有效值
     */
    STDMETHOD_(BOOL, HasValue)() SCONST PURE;

    /**
     * @brief 清除日期时间值
     */
    STDMETHOD_(void, ClearValue)() PURE;
};


SNSEND
#endif//__SGRIDCTRL__I__H__