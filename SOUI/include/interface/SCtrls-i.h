//********************************************************************
/** Control operation interfaces are defined here; all interfaces inherit from IObjRef and specify their own interface ID */
//********************************************************************
#ifndef __SCTRLS_I__H__
#define __SCTRLS_I__H__
#include <interface/SCtrl-i.h>
#include <interface/SSkinobj-i.h>
#include <interface/SRender-i.h>
#include <interface/sstring-i.h>
#include <interface/SAdapter-i.h>
#include <interface/SListViewItemLocator-i.h>
#include <interface/STreeViewItemLocator-i.h>
#include <interface/STileViewItemLocator-i.h>
SNSBEGIN

/** {36D49C0A-CCBA-4238-98E2-B6ACB18824B5} */
DEFINE_SGUID(IID_IOsrPanel, 0x36d49c0a, 0xccba, 0x4238, 0x98, 0xe2, 0xb6, 0xac, 0xb1, 0x88, 0x24, 0xb5);
#undef INTERFACE
#define INTERFACE IOsrPanel
DECLARE_INTERFACE_(IOsrPanel, ICtrl)
{
    DECLARE_CLASS_SIID(IID_IOsrPanel)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Set the corresponding index in the list
     * @param index -- index value; in listview it is an index, in treeview it is an HTREEITEM value
     * @return
     */
    STDMETHOD_(void, SetItemIndex)(THIS_ LPARAM index) PURE;

    /**
     * @brief Get the index in the list
     * @return LPARAM the index in the list
     */
    STDMETHOD_(LPARAM, GetItemIndex)(CTHIS) SCONST PURE;

    /**
     * @brief Set the user data of the list item
     * @param dwData -- user data
     * @return
     */
    STDMETHOD_(void, SetItemData)(THIS_ LPARAM dwData) PURE;

    /**
     * @brief Get the user data in the list
     * @return LPARAM the user data in the list
     */
    STDMETHOD_(LPARAM, GetItemData)(CTHIS) SCONST PURE;

    /**
     * @brief Convert the panel's coordinates to host coordinates
     * @param[in,out] POINT *pt window coordinates
     * @return void
     */
    STDMETHOD_(void, PtToHost)(THIS_ POINT * pt) SCONST PURE;
};

/** {F78CA81B-729B-4fe5-B3FE-6C70AD363017} */
DEFINE_SGUID(IID_IItemPanel, 0xf78ca81b, 0x729b, 0x4fe5, 0xb3, 0xfe, 0x6c, 0x70, 0xad, 0x36, 0x30, 0x17);

#undef INTERFACE
#define INTERFACE IItemPanel
DECLARE_INTERFACE_(IItemPanel, IOsrPanel)
{
    DECLARE_CLASS_SIID(IID_IItemPanel)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Set the corresponding index in the list
     * @param index -- index value; in listview it is an index, in treeview it is an HTREEITEM value
     * @return
     */
    STDMETHOD_(void, SetItemIndex)(THIS_ LPARAM index) PURE;

    /**
     * @brief Get the index in the list
     * @return LPARAM the index in the list
     */
    STDMETHOD_(LPARAM, GetItemIndex)(CTHIS) SCONST PURE;

    /**
     * @brief Set the user data of the list item
     * @param dwData -- user data
     * @return
     */
    STDMETHOD_(void, SetItemData)(THIS_ LPARAM dwData) PURE;

    /**
     * @brief Get the user data in the list
     * @return LPARAM the user data in the list
     */
    STDMETHOD_(LPARAM, GetItemData)(CTHIS) SCONST PURE;

    /**
     * @brief Convert the panel's coordinates to host coordinates
     * @param[in,out] POINT *pt window coordinates
     * @return void
     */
    STDMETHOD_(void, PtToHost)(THIS_ POINT * pt) SCONST PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Set the skin of the list item
     * @param pSkin -- the list item's skin
     * @return
     */
    STDMETHOD_(void, SetSkin)(THIS_ ISkinObj * pSkin) PURE;

    /**
     * @brief Set the color configuration of the list item
     * @param crBk -- background color
     * @param crSelBk -- background color in selected state
     * @return
     */
    STDMETHOD_(void, SetColor)(THIS_ COLORREF crBk, COLORREF crSelBk) PURE;

    /**
     * @brief Whether the list item is selected
     * @return BOOL TRUE if selected, FALSE if not
     */
    STDMETHOD_(BOOL, IsSelected)(CTHIS) SCONST PURE;

    /**
     * @brief Set the selected state of the list item
     * @param bSelected -- selection flag, TRUE-selected, FALSE-not selected
     * @param bUpdate -- whether to update display immediately, TRUE-update, FALSE-no update
     * @return
     */
    STDMETHOD_(void, SetSelected)(THIS_ BOOL bSelected, BOOL bUpdate DEF_VAL(TRUE)) PURE;
};

/** {310A0C73-1DCA-4ac2-B6C9-569B0EA40496} */
DEFINE_SGUID(IID_IImageWnd, 0x310a0c73, 0x1dca, 0x4ac2, 0xb6, 0xc9, 0x56, 0x9b, 0xe, 0xa4, 0x4, 0x96);

#undef INTERFACE
#define INTERFACE IImageWnd
DECLARE_INTERFACE_(IImageWnd, ICtrl)
{
    DECLARE_CLASS_SIID(IID_IImageWnd)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Set the skin
     * @param pSkin -- skin object
     * @param iFrame -- sub-image index of the skin
     * @param bAutoFree -- flag indicating whether the control manages pSkin; when TRUE the caller may free the Skin
     * @return BOOL TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, SetSkin)(THIS_ ISkinObj * pSkin, int iFrame DEF_VAL(0), BOOL bAutoFree DEF_VAL(TRUE)) PURE;

    /**
     * SImageWnd::GetSkin
     * @brief    Get the resource
     * @return   Return value: ISkinObj pointer
     *
     * Describe  Get the resource
     */
    STDMETHOD_(ISkinObj *, GetSkin)(CTHIS) SCONST PURE;

    /**
     * SImageWnd::SetImage
     * @param    IBitmap * pBitmap -- the image object
     * @param    FilterLevel fl -- FilterLevel
     * @return   void
     *
     * Describe  Set the image to draw
     */
    STDMETHOD_(void, SetImage)(THIS_ IBitmapS * pBitmap, FilterLevel fl DEF_VAL(kNone_FilterLevel)) PURE;

    /**
     * @brief Get the currently set IBitmapS object
     * @return IBitmapS * -- the currently set IBitmapS object
     */
    STDMETHOD_(IBitmapS *, GetImage)(THIS) PURE;

    /**
     * SImageWnd::SetIcon
     * @param    int iIcon -- index of the sub-image within the Skin
     * @brief     Set the icon
     * @return    Return value BOOL, TRUE on success, FALSE on failure
     *
     * Describe   Set the icon
     */
    STDMETHOD_(BOOL, SetIcon)(THIS_ int iIcon) PURE;

    /**
     * @brief Get the currently set icon
     * @return int -- the index of the currently set sub-image within the Skin
     */
    STDMETHOD_(int, GetIcon)(CTHIS) SCONST PURE;

    STDMETHOD_(void, SetSvg)(THIS_ ISvgObj * pSvg) PURE;

    STDMETHOD_(ISvgObj *, GetSvg)(CTHIS) SCONST PURE;
};

/** {C2378093-CB34-4811-98D1-A721637B3B87} */
DEFINE_SGUID(IID_IAnimateImgWnd, 0xc2378093, 0xcb34, 0x4811, 0x98, 0xd1, 0xa7, 0x21, 0x63, 0x7b, 0x3b, 0x87);

#undef INTERFACE
#define INTERFACE IAnimateImgWnd
DECLARE_INTERFACE_(IAnimateImgWnd, ICtrl)
{
    DECLARE_CLASS_SIID(IID_IAnimateImgWnd)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * SAnimateImgWnd::Start
     * @brief    Start the animation
     *
     * Describe  Start the animation
     */
    STDMETHOD_(void, Start)(THIS) PURE;
    /**
     * SAnimateImgWnd::Stop
     * @brief    Stop the animation
     *
     * Describe  Stop the animation
     */
    STDMETHOD_(void, Stop)(THIS) PURE;

    /**
     * SAnimateImgWnd::IsPlaying
     * @brief    Check the animation running state
     * @return   Return value is the animation state, TRUE -- running
     *
     * Describe  Check the animation running state
     */
    STDMETHOD_(BOOL, IsPlaying)(CTHIS) SCONST PURE;
};

/** {0A572106-14E0-48d7-85C3-3E08B9DA85A8} */
DEFINE_SGUID(IID_IProg, 0xa572106, 0x14e0, 0x48d7, 0x85, 0xc3, 0x3e, 0x8, 0xb9, 0xda, 0x85, 0xa8);

#undef INTERFACE
#define INTERFACE IProgress
DECLARE_INTERFACE_(IProgress, ICtrl)
{
    DECLARE_CLASS_SIID(IID_IProg)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////
    /**
     * SProgress::SetValue
     * @brief    Set the progress bar value
     * @param    int nValue  --  progress value
     * @return   Return value is TRUE -- set successfully
     *
     * Describe  Set the progress bar value
     */
    STDMETHOD_(BOOL, SetValue)(THIS_ int nValue) PURE;

    /**
     * SProgress::GetValue
     * @brief    Get the progress value
     * @return   Return value is int
     *
     * Describe  Get the progress value
     */
    STDMETHOD_(int, GetValue)(CTHIS) SCONST PURE;

    /**
     * SProgress::SetRange
     * @param    int nMin  --  minimum progress value
     * @param    int nMax  --  maximum progress value
     * @brief    Set the progress value range (min/max)
     *
     * Describe  Set the progress value
     */
    STDMETHOD_(void, SetRange)(THIS_ int nMin, int nMax) PURE;
    /**
     * SProgress::GetRange
     * @param    int nMin  --  minimum progress value
     * @param    int nMax  --  maximum progress value
     * @brief    Get the progress value range (min/max)
     *
     * Describe  Get the progress value
     */
    STDMETHOD_(void, GetRange)(CTHIS_ int *pMin, int *pMax) SCONST PURE;
    /**
     * SProgress::IsVertical
     * @brief    Check whether the progress bar is vertical
     * @return   Return value is TRUE -- vertical state
     *
     * Describe  Get the progress value
     */
    STDMETHOD_(BOOL, IsVertical)(CTHIS) SCONST PURE;
};

#define MAX_SEGMENT_LENGTH 100
/**
 * @brief Segment information structure
 */
typedef struct _SEGMENT
{
    int value;                            /**< Segment start value */
    TCHAR szDesc[MAX_SEGMENT_LENGTH + 1]; /**< Segment description */
} SEGMENT;

/** {0A572106-14E0-48d7-85C3-3E08B9DA85B8} */
DEFINE_SGUID(IID_ISliderBar, 0xa572106, 0x14e0, 0x48d7, 0x85, 0xc3, 0x3e, 0x8, 0xb9, 0xda, 0x85, 0xB8);

#undef INTERFACE
#define INTERFACE ISliderBar
DECLARE_INTERFACE_(ISliderBar, IProgress)
{
    DECLARE_CLASS_SIID(IID_ISliderBar)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////
    /**
     * SProgress::SetValue
     * @brief    Set the progress bar value
     * @param    int nValue  --  progress value
     * @return   Return value is TRUE -- set successfully
     *
     * Describe  Set the progress bar value
     */
    STDMETHOD_(BOOL, SetValue)(THIS_ int nValue) PURE;

    /**
     * SProgress::GetValue
     * @brief    Get the progress value
     * @return   Return value is int
     *
     * Describe  Get the progress value
     */
    STDMETHOD_(int, GetValue)(CTHIS) SCONST PURE;

    /**
     * SProgress::SetRange
     * @param    int nMin  --  minimum progress value
     * @param    int nMax  --  maximum progress value
     * @brief    Set the progress value range (min/max)
     *
     * Describe  Set the progress value
     */
    STDMETHOD_(void, SetRange)(THIS_ int nMin, int nMax) PURE;
    /**
     * SProgress::GetRange
     * @param    int nMin  --  minimum progress value
     * @param    int nMax  --  maximum progress value
     * @brief    Get the progress value range (min/max)
     *
     * Describe  Get the progress value
     */
    STDMETHOD_(void, GetRange)(CTHIS_ int *pMin, int *pMax) SCONST PURE;
    /**
     * SProgress::IsVertical
     * @brief    Check whether the progress bar is vertical
     * @return   Return value is TRUE -- vertical state
     *
     * Describe  Get the progress value
     */
    STDMETHOD_(BOOL, IsVertical)(CTHIS) SCONST PURE;
    ///////////////////////////////////////////////////////////////////////
    /**
     * @brief Set segment information
     * @param segments Segment array
     * @param count Segment count
     * @details Segments must be arranged in ascending order
     */
    STDMETHOD_(void, SetSegments)(THIS_ const SEGMENT *segments, int count) PURE;

    /**
     * @brief Get the number of segments
     * @return Segment count
     */
    STDMETHOD_(int, GetSegmentCount)(CTHIS) SCONST PURE;

    /**
     * @brief Get the segment information at the specified index
     * @param index Segment index
     * @param pSegment Output segment information
     * @return Returns TRUE on success
     */
    STDMETHOD_(BOOL, GetSegment)(CTHIS_ int index, SEGMENT *pSegment) SCONST PURE;

    /**
     * @brief Find the segment containing the given value
     * @param value Progress value
     * @return Segment index, -1 if not found
     */
    STDMETHOD_(int, FindSegmentByValue)(CTHIS_ int value) SCONST PURE;
};

typedef enum _ScrollBarID
{
    SSB_NULL = 0,
    SSB_HORZ = 1,
    SSB_VERT = 2,
    SSB_BOTH = (SSB_HORZ | SSB_VERT),
} ScrollBarID;

/** {8DE0A6C5-9CBC-465f-B784-EBA4CB8AE6D9} */
DEFINE_SGUID(IID_IPanel, 0x8de0a6c5, 0x9cbc, 0x465f, 0xb7, 0x84, 0xeb, 0xa4, 0xcb, 0x8a, 0xe6, 0xd9);

#undef INTERFACE
#define INTERFACE IPanel
DECLARE_INTERFACE_(IPanel, ICtrl)
{
    DECLARE_CLASS_SIID(IID_IPanel)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Show/hide the scroll bar
     * @param wBar scroll bar flag, see ScrollBarID
     * @param bShow TRUE-show, FALSE-hide
     * @return
     */
    STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

    /**
     * @brief Enable/Disable the scroll bar
     * @param wBar scroll bar flag, see ScrollBarID
     * @param bShow TRUE-show, FALSE-hide
     * @return
     */
    STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

    /**
     * @brief Get the scroll bar Enable state
     * @param bVertical -- TRUE-vertical scroll bar
     * @return TRUE if enabled
     */
    STDMETHOD_(BOOL, IsScrollBarEnable)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief Set the scroll bar data
     * @param si -- scroll bar data
     * @param bVertical -- TRUE-vertical scroll bar
     * @return
     */
    STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

    /**
     * @brief Set the scroll bar position
     * @param bVertical -- TRUE-vertical scroll bar
     * @param nNewPos -- scroll position
     * @param bRedraw -- redraw flag
     * @return
     */
    STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

    /**
     * @brief Get the scroll bar position
     * @param bVertical -- TRUE-vertical scroll bar
     * @return scroll bar position
     */
    STDMETHOD_(int, GetScrollPos)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief Set the scroll bar range
     * @param bVertical -- TRUE-vertical scroll bar
     * @param nMinPos -- minimum value
     * @param nMaxPos -- maximum value
     * @param bRedraw -- redraw flag
     * @return TRUE on success
     */
    STDMETHOD_(BOOL, SetScrollRange)
    (THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

    /**
     * @brief Get the scroll bar range
     * @param bVertical -- TRUE-vertical scroll bar
     * @param lpMinPos -- minimum value
     * @param lpMaxPos -- maximum value
     * @return
     */
    STDMETHOD_(BOOL, GetScrollRange)
    (CTHIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;

    /**
     * @brief Query the scroll bar enabled state
     * @param bVertical -- TRUE-vertical scroll bar
     * @return TRUE if enabled
     */
    STDMETHOD_(BOOL, HasScrollBar)(CTHIS_ BOOL bVertical) SCONST PURE;
};

/** {F7A4A172-31B2-4597-9DE5-F62160723733} */
DEFINE_SGUID(IID_IScrollView, 0xf7a4a172, 0x31b2, 0x4597, 0x9d, 0xe5, 0xf6, 0x21, 0x60, 0x72, 0x37, 0x33);
#undef INTERFACE
#define INTERFACE IScrollView
DECLARE_INTERFACE_(IScrollView, IPanel)
{
    DECLARE_CLASS_SIID(IID_IScrollView)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Show/hide the scroll bar
     * @param wBar scroll bar flag, see ScrollBarID
     * @param bShow TRUE-show, FALSE-hide
     * @return
     */
    STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

    /**
     * @brief Enable/Disable the scroll bar
     * @param wBar scroll bar flag, see ScrollBarID
     * @param bShow TRUE-show, FALSE-hide
     * @return
     */
    STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

    /**
     * @brief Get the scroll bar Enable state
     * @param bVertical -- TRUE-vertical scroll bar
     * @return TRUE if enabled
     */
    STDMETHOD_(BOOL, IsScrollBarEnable)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief Set the scroll bar data
     * @param si -- scroll bar data
     * @param bVertical -- TRUE-vertical scroll bar
     * @return
     */
    STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

    /**
     * @brief Set the scroll bar position
     * @param bVertical -- TRUE-vertical scroll bar
     * @param nNewPos -- scroll position
     * @param bRedraw -- redraw flag
     * @return
     */
    STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

    /**
     * @brief Get the scroll bar position
     * @param bVertical -- TRUE-vertical scroll bar
     * @return scroll bar position
     */
    STDMETHOD_(int, GetScrollPos)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief Set the scroll bar range
     * @param bVertical -- TRUE-vertical scroll bar
     * @param nMinPos -- minimum value
     * @param nMaxPos -- maximum value
     * @param bRedraw -- redraw flag
     * @return TRUE on success
     */
    STDMETHOD_(BOOL, SetScrollRange)
    (THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

    /**
     * @brief Get the scroll bar range
     * @param bVertical -- TRUE-vertical scroll bar
     * @param lpMinPos -- minimum value
     * @param lpMaxPos -- maximum value
     * @return
     */
    STDMETHOD_(BOOL, GetScrollRange)
    (CTHIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;

    /**
     * @brief Query the scroll bar enabled state
     * @param bVertical -- TRUE-vertical scroll bar
     * @return TRUE if enabled
     */
    STDMETHOD_(BOOL, HasScrollBar)(CTHIS_ BOOL bVertical) SCONST PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Get the view size
     * @return SIZE - view size
     */
    STDMETHOD_(void, GetViewSize)(CTHIS_ SIZE * szView) SCONST PURE;

    /**
     * @brief Set the view size
     * @param szView view size
     * @return
     */
    STDMETHOD_(void, SetViewSize)(THIS_ SIZE szView) PURE;

    /**
     * @brief Get the view origin coordinates
     * @return POINT - origin coordinates
     */
    STDMETHOD_(void, GetViewOrigin)(CTHIS_ POINT * ptOri) SCONST PURE;

    /**
     * @brief Set the view origin coordinates
     * @param pt origin coordinates
     * @return
     */
    STDMETHOD_(void, SetViewOrigin)(THIS_ POINT pt) PURE;
};

typedef enum _HeaderMask
{
    SHDI_WIDTH = 0x0001,
    SHDI_TEXT = 0x0002,
    SHDI_FORMAT = 0x0004,
    SHDI_LPARAM = 0x0008,
    SHDI_ORDER = 0x0010,
    SHDI_VISIBLE = 0x0020,
    SHDI_WEIGHT = 0x0040,
    SHDI_ALL = 0xffff,
} HeaderMask;

/**
 * @struct    _SHDITEM
 * @brief     List header item
 *
 * Describe   List header item
 */
typedef struct SHDITEM
{
    UINT mask;
    int cx;
    UINT fmt;
    UINT state;
    int iOrder;
    LPTSTR pszText;
    int cchMaxText;
    LPARAM lParam;
    BOOL bDpiAware;
    BOOL bVisible;
    float fWeight;
} SHDITEM, *LPSHDITEM;

/** {EE19AA27-7C36-4426-8700-8FBCB0C738F5} */
DEFINE_SGUID(IID_IHeaderCtrl, 0xee19aa27, 0x7c36, 0x4426, 0x87, 0x0, 0x8f, 0xbc, 0xb0, 0xc7, 0x38, 0xf5);
#undef INTERFACE
#define INTERFACE IHeaderCtrl
DECLARE_INTERFACE_(IHeaderCtrl, ICtrl)
{
    DECLARE_CLASS_SIID(IID_IHeaderCtrl)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * SHeaderCtrl::InsertItem
     * @brief    Insert a new item
     * @param    int iItem --  new item index
     * @param    LPCTSTR pszText  --  new item title
     * @param    int nWidth  -- width
     * @param    BOOL bDpiAware -- dpi aware
     * @param    UINT fmt -- format flag
     * @param    LPARAM lParam -- additional parameter
     * @return   returns int
     *
     * Describe  Insert a new item
     */
    STDMETHOD_(int, InsertItem)
    (THIS_ int iItem, LPCTSTR pszText, int nWidth, UINT fmt, LPARAM lParam, BOOL bDpiAware /**< =FALSE */, float fWeight /**< =0.0f */) PURE;

    STDMETHOD_(int, InsertItemA)
    (THIS_ int iItem, LPCSTR pszText, int nWidth, UINT fmt, LPARAM lParam, BOOL bDpiAware /**< =FALSE */, float fWeight /**< =0.0f */) PURE;

    /**
     * SHeaderCtrl::GetItem
     * @brief    Get an item
     * @param    int iItem  --  index
     * @param    SHDITEM *pItem  -- returns the list item structure
     * @return   returns BOOL
     *
     * Describe  Get an item
     */
    STDMETHOD_(BOOL, GetItem)(CTHIS_ int iItem, SHDITEM *pItem) SCONST PURE;

    /**
     * @brief Set the header item data
     * @param iItem -- index
     * @param pItem -- data item
     * @return
     */
    STDMETHOD_(BOOL, SetItem)(THIS_ int iItem, const SHDITEM *pItem) PURE;

    /**
     * SHeaderCtrl::GetItemCount
     * @brief    Get the number of list items
     * @return   returns int
     *
     * Describe  Get the number of list items
     */
    STDMETHOD_(UINT, GetItemCount)(CTHIS) SCONST PURE;
    /**
     * SHeaderCtrl::GetTotalWidth
     * @brief    Get the total width
     * @return   returns int
     *
     * Describe  Get the total width
     */
    STDMETHOD_(int, GetTotalWidth)(CTHIS_ BOOL bMinWid DEF_VAL(FALSE)) SCONST PURE;

    /**
     * @brief Flag indicating whether the header auto-fills the parent window width
     * @return TRUE if the header auto-fills the parent window width
     */
    STDMETHOD_(BOOL, IsAutoResize)(CTHIS) SCONST PURE;

    /**
     * SHeaderCtrl::GetItemWidth
     * @brief    Get the width of the specified item
     * @param    int iItem  --  index
     * @return   returns int
     *
     * Describe  Get a new item
     */
    STDMETHOD_(int, GetItemWidth)(CTHIS_ int iItem) SCONST PURE;

    /**
     * SHeaderCtrl::DeleteItem
     * @brief    Delete the specified item
     * @param    int iItem  --  index
     * @return   returns BOOL
     *
     * Describe  Delete the specified item
     */
    STDMETHOD_(BOOL, DeleteItem)(THIS_ int iItem) PURE;

    /**
     * SHeaderCtrl::DeleteAllItems
     * @brief    Delete all items
     *
     * Describe  Get a new item
     */
    STDMETHOD_(void, DeleteAllItems)(THIS) PURE;

    /**
     * @brief Set the sort flag of the header item
     * @param iItem -- header item index
     * @param sortFlag -- sort flag
     * @remark The sort flag is passed to the list's sort callback interface, where the user implements data sorting
     */
    STDMETHOD_(void, SetItemSort)(THIS_ int iItem, UINT sortFlag) PURE;

    /**
     * @brief Set whether the header item is hidden
     * @param iItem -- header item index
     * @param visible -- TRUE-show, FALSE-hide
     * @return
     */
    STDMETHOD_(void, SetItemVisible)(THIS_ int iItem, BOOL visible) PURE;

    /**
     * @brief Query whether the header item is hidden
     * @param iItem -- header item index
     * @return BOOL
     */
    STDMETHOD_(BOOL, IsItemVisible)(CTHIS_ int iItem) SCONST PURE;

    /**
     * @brief    Get the header item
     * @param    int iOrder  -- show order
     * @return   int item index
     *
     * Describe  Get the header item position
     */
    STDMETHOD_(int, GetOriItemIndex)(CTHIS_ int iOrder) SCONST PURE;

    /**
     * @brief    Get the header item position
     * @param    int iItem  --  index
     * @param    LPRECT prc  --  header item position
     *
     * Describe  Get the header item position
     */
    STDMETHOD_(void, GetItemRect)(CTHIS_ int iItem, LPRECT prc) SCONST PURE;
};

/** {107ED2EA-6C95-4cec-A300-2CE5AB0A7DDB} */
DEFINE_SGUID(IID_IListView, 0x107ed2ea, 0x6c95, 0x4cec, 0xa3, 0x0, 0x2c, 0xe5, 0xab, 0xa, 0x7d, 0xdb);
#undef INTERFACE
#define INTERFACE IListView
DECLARE_INTERFACE_(IListView, IPanel)
{
    DECLARE_CLASS_SIID(IID_IListView)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Show/hide the scroll bar
     * @param wBar scroll bar flag, see ScrollBarID
     * @param bShow TRUE-show, FALSE-hide
     * @return
     */
    STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

    /**
     * @brief Enable/Disable the scroll bar
     * @param wBar scroll bar flag, see ScrollBarID
     * @param bShow TRUE-show, FALSE-hide
     * @return
     */
    STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

    /**
     * @brief Get the scroll bar Enable state
     * @param bVertical -- TRUE-vertical scroll bar
     * @return TRUE if enabled
     */
    STDMETHOD_(BOOL, IsScrollBarEnable)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief Set the scroll bar data
     * @param si -- scroll bar data
     * @param bVertical -- TRUE-vertical scroll bar
     * @return
     */
    STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

    /**
     * @brief Set the scroll bar position
     * @param bVertical -- TRUE-vertical scroll bar
     * @param nNewPos -- scroll position
     * @param bRedraw -- redraw flag
     * @return
     */
    STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

    /**
     * @brief Get the scroll bar position
     * @param bVertical -- TRUE-vertical scroll bar
     * @return scroll bar position
     */
    STDMETHOD_(int, GetScrollPos)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief Set the scroll bar range
     * @param bVertical -- TRUE-vertical scroll bar
     * @param nMinPos -- minimum value
     * @param nMaxPos -- maximum value
     * @param bRedraw -- redraw flag
     * @return TRUE on success
     */
    STDMETHOD_(BOOL, SetScrollRange)
    (THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

    /**
     * @brief Get the scroll bar range
     * @param bVertical -- TRUE-vertical scroll bar
     * @param lpMinPos -- minimum value
     * @param lpMaxPos -- maximum value
     * @return
     */
    STDMETHOD_(BOOL, GetScrollRange)
    (CTHIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;

    /**
     * @brief Query the scroll bar enabled state
     * @param bVertical -- TRUE-vertical scroll bar
     * @return TRUE if enabled
     */
    STDMETHOD_(BOOL, HasScrollBar)(CTHIS_ BOOL bVertical) SCONST PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Set the list's Adapter
     * @param adapter -- Adapter object
     * @return BOOL
     * @remark Usually should only be called once during list initialization; the list increments the Adapter's reference count
     */
    STDMETHOD_(BOOL, SetAdapter)(THIS_ ILvAdapter * adapter) PURE;

    /**
     * @brief Get the control's Adapter object
     * @return ILvAdapter* - Adapter object
     */
    STDMETHOD_(ILvAdapter *, GetAdapter)(CTHIS) SCONST PURE;

    /**
     * @brief Set the list item position locator
     * @param pItemLocator -- position locator
     * @remark The position locator is an interface that maps index numbers to list display positions; by implementing it, the user can control the display size of list items
     */
    STDMETHOD_(void, SetItemLocator)(THIS_ IListViewItemLocator * pItemLocator) PURE;

    /**
     * @brief Get the position locator
     * @return IListViewItemLocator *
     */
    STDMETHOD_(IListViewItemLocator *, GetItemLocator)(CTHIS) SCONST PURE;

    /**
     * @brief Scroll the specified row into the visible view area
     * @param iItem -- row number
     * @return
     */
    STDMETHOD_(void, EnsureVisible)(THIS_ int iItem) PURE;

    /**
     * @brief Select the specified row
     * @param iItem -- row number
     * @param bNotify -- flag to trigger the selection state event
     * @return
     */
    STDMETHOD_(void, SetSel)(THIS_ int iItem, BOOL bNotify DEF_VAL(FALSE)) PURE;

    /**
     * @brief Get the currently selected row
     * @return int selected row index, -1 means no row is currently selected
     */
    STDMETHOD_(int, GetSel)(CTHIS) SCONST PURE;

    /**
     * @brief Hit test
     * @param pt -- input coordinates
     * @return IItemPanel *-- the ItemPanel under the coordinates
     */
    STDMETHOD_(IItemPanel *, HitTest)(CTHIS_ const POINT *pt) SCONST PURE;

    /**
     * @brief Sets the multiple selection mode.
     * @param bMultiSel TRUE to enable multiple selection, FALSE otherwise.
     */
    STDMETHOD_(void, SetMultiSel)(THIS_ BOOL bMultiSel) PURE;

    /**
     * @brief Gets the multiple selection mode.
     * @return TRUE if multiple selection is enabled, FALSE otherwise.
     */
    STDMETHOD_(BOOL, GetMultiSel)(CTHIS) SCONST PURE;

    /**
     * @brief Adds an item to the selection.
     * @param iItem Index of the item.
     */
    STDMETHOD_(void, AddSelItem)(THIS_ int iItem) PURE;

    /**
     * @brief Removes an item from the selection.
     * @param iItem Index of the item.
     */
    STDMETHOD_(void, RemoveSelItem)(THIS_ int iItem) PURE;

    /**
     * @brief Clears all selected items.
     */
    STDMETHOD_(void, ClearSelItems)(THIS) PURE;

    /**
     * @brief Checks if an item is selected.
     * @param iItem Index of the item.
     * @return TRUE if the item is selected, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsItemSelected)(THIS_ int iItem) SCONST PURE;

    /**
     * @brief Gets the count of selected items.
     * @return Number of selected items.
     */
    STDMETHOD_(int, GetSelItemCount)(CTHIS) SCONST PURE;

    /**
     * @brief Gets all selected items.
     * @param items Output parameter to store the selected item indices.
     * @param nMaxCount Maximum number of items to retrieve.
     * @return Number of selected items.
     */
    STDMETHOD_(int, GetSelItems)(THIS_ int *items, int nMaxCount) SCONST PURE;
};

/** {A45F148C-F628-4df7-A60E-B51DA0017B00} */
DEFINE_SGUID(IID_IMcListView, 0xa45f148c, 0xf628, 0x4df7, 0xa6, 0xe, 0xb5, 0x1d, 0xa0, 0x1, 0x7b, 0x0);
#undef INTERFACE
#define INTERFACE IMcListView
DECLARE_INTERFACE_(IMcListView, IPanel)
{
    DECLARE_CLASS_SIID(IID_IMcListView)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Show/hide the scroll bar
     * @param wBar scroll bar flag, see ScrollBarID
     * @param bShow TRUE-show, FALSE-hide
     * @return
     */
    STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

    /**
     * @brief Enable/Disable the scroll bar
     * @param wBar scroll bar flag, see ScrollBarID
     * @param bShow TRUE-show, FALSE-hide
     * @return
     */
    STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

    /**
     * @brief Get the scroll bar Enable state
     * @param bVertical -- TRUE-vertical scroll bar
     * @return TRUE if enabled
     */
    STDMETHOD_(BOOL, IsScrollBarEnable)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief Set the scroll bar data
     * @param si -- scroll bar data
     * @param bVertical -- TRUE-vertical scroll bar
     * @return
     */
    STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

    /**
     * @brief Set the scroll bar position
     * @param bVertical -- TRUE-vertical scroll bar
     * @param nNewPos -- scroll position
     * @param bRedraw -- redraw flag
     * @return
     */
    STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

    /**
     * @brief Get the scroll bar position
     * @param bVertical -- TRUE-vertical scroll bar
     * @return scroll bar position
     */
    STDMETHOD_(int, GetScrollPos)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief Set the scroll bar range
     * @param bVertical -- TRUE-vertical scroll bar
     * @param nMinPos -- minimum value
     * @param nMaxPos -- maximum value
     * @param bRedraw -- redraw flag
     * @return TRUE on success
     */
    STDMETHOD_(BOOL, SetScrollRange)
    (THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

    /**
     * @brief Get the scroll bar range
     * @param bVertical -- TRUE-vertical scroll bar
     * @param lpMinPos -- minimum value
     * @param lpMaxPos -- maximum value
     * @return
     */
    STDMETHOD_(BOOL, GetScrollRange)
    (CTHIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;

    /**
     * @brief Query the scroll bar enabled state
     * @param bVertical -- TRUE-vertical scroll bar
     * @return TRUE if enabled
     */
    STDMETHOD_(BOOL, HasScrollBar)(CTHIS_ BOOL bVertical) SCONST PURE;
    ///////////////////////////////////////////////////////////////////////
    /**
     * @brief Set the list's Adapter
     * @param adapter -- Adapter object
     * @return BOOL
     * @remark Usually should only be called once during list initialization; the list increments the Adapter's reference count
     */
    STDMETHOD_(BOOL, SetAdapter)(THIS_ IMcAdapter * adapter) PURE;

    /**
     * @brief Get the control's Adapter object
     * @return ILvAdapter* - Adapter object
     */
    STDMETHOD_(IMcAdapter *, GetAdapter)(CTHIS) SCONST PURE;

    /**
     * @brief Set the list item position locator
     * @param pItemLocator -- position locator
     * @remark The position locator is an interface that maps index numbers to list display positions; by implementing it, the user can control the display size of list items
     */
    STDMETHOD_(void, SetItemLocator)(THIS_ IListViewItemLocator * pItemLocator) PURE;

    /**
     * @brief Get the position locator
     * @return IListViewItemLocator *
     */
    STDMETHOD_(IListViewItemLocator *, GetItemLocator)(CTHIS) SCONST PURE;

    /**
     * @brief Scroll the specified row into the visible view area
     * @param iItem -- row number
     * @return
     */
    STDMETHOD_(void, EnsureVisible)(THIS_ int iItem) PURE;

    /**
     * @brief Select the specified row
     * @param iItem -- row number
     * @param bNotify -- flag to trigger the selection state event
     * @return
     */
    STDMETHOD_(void, SetSel)(THIS_ int iItem, BOOL bNotify /**< = FALSE */) PURE;

    /**
     * @brief Get the currently selected row
     * @return int selected row index, -1 means no row is currently selected
     */
    STDMETHOD_(int, GetSel)(CTHIS) SCONST PURE;

    /**
     * @brief Hit test
     * @param pt -- input coordinates
     * @return IItemPanel *-- the ItemPanel under the coordinates
     */
    STDMETHOD_(IItemPanel *, HitTest)(CTHIS_ const POINT *pt) SCONST PURE;

    /**
     * @brief Get the header control
     * @return IHeaderCtrl *-- the header control
     */
    STDMETHOD_(IHeaderCtrl *, GetIHeaderCtrl)(CTHIS) SCONST PURE;

    /**
     * @brief Insert a column
     * @param nIndex -- insert position, -1 means append
     * @param pszText -- text
     * @param nWidth -- occupied width
     * @param fmt -- display format
     * @param lParam -- custom data
     * @param bDpiAware -- width aware of dpi scaling
     * @param fWeight -- the weight property of the width
     * @return insert position
     */
    STDMETHOD_(int, InsertColumn)
    (THIS_ int nIndex, LPCTSTR pszText, int nWidth, UINT fmt, LPARAM lParam DEF_VAL(0), BOOL bDpiAware DEF_VAL(TRUE), float fWeight DEF_VAL(0.0f)) PURE;

    /**
     * @brief Delete a column
     * @param iCol -- column index
     * @return
     */
    STDMETHOD_(void, DeleteColumn)(THIS_ int iCol) PURE;

    /**
     * @brief Get the number of columns
     * @return int column count
     */
    STDMETHOD_(int, GetColumnCount)(CTHIS) SCONST PURE;

    /**
     * @brief Sets the multiple selection mode.
     * @param bMultiSel TRUE to enable multiple selection, FALSE otherwise.
     */
    STDMETHOD_(void, SetMultiSel)(THIS_ BOOL bMultiSel) PURE;

    /**
     * @brief Gets the multiple selection mode.
     * @return TRUE if multiple selection is enabled, FALSE otherwise.
     */
    STDMETHOD_(BOOL, GetMultiSel)(CTHIS) SCONST PURE;

    /**
     * @brief Adds an item to the selection.
     * @param iItem Index of the item.
     */
    STDMETHOD_(void, AddSelItem)(THIS_ int iItem) PURE;

    /**
     * @brief Removes an item from the selection.
     * @param iItem Index of the item.
     */
    STDMETHOD_(void, RemoveSelItem)(THIS_ int iItem) PURE;

    /**
     * @brief Clears all selected items.
     */
    STDMETHOD_(void, ClearSelItems)(THIS) PURE;

    /**
     * @brief Checks if an item is selected.
     * @param iItem Index of the item.
     * @return TRUE if the item is selected, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsItemSelected)(THIS_ int iItem) SCONST PURE;

    /**
     * @brief Gets the count of selected items.
     * @return Number of selected items.
     */
    STDMETHOD_(int, GetSelItemCount)(CTHIS) SCONST PURE;

    /**
     * @brief Gets all selected items.
     * @param items Output parameter to store the selected item indices.
     * @param nMaxCount Maximum number of items to retrieve.
     * @return Number of selected items.
     */
    STDMETHOD_(int, GetSelItems)(THIS_ int *items, int nMaxCount) SCONST PURE;
};

/** {572CA529-AFFF-412c-8A51-2014A9801739} */
DEFINE_SGUID(IID_ITreeView, 0x572ca529, 0xafff, 0x412c, 0x8a, 0x51, 0x20, 0x14, 0xa9, 0x80, 0x17, 0x39);
#undef INTERFACE
#define INTERFACE ITreeView
DECLARE_INTERFACE_(ITreeView, IPanel)
{
    DECLARE_CLASS_SIID(IID_ITreeView)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Show/hide the scroll bar
     * @param wBar scroll bar flag, see ScrollBarID
     * @param bShow TRUE-show, FALSE-hide
     * @return
     */
    STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

    /**
     * @brief Enable/Disable the scroll bar
     * @param wBar scroll bar flag, see ScrollBarID
     * @param bShow TRUE-show, FALSE-hide
     * @return
     */
    STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

    /**
     * @brief Get the scroll bar Enable state
     * @param bVertical -- TRUE-vertical scroll bar
     * @return TRUE if enabled
     */
    STDMETHOD_(BOOL, IsScrollBarEnable)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief Set the scroll bar data
     * @param si -- scroll bar data
     * @param bVertical -- TRUE-vertical scroll bar
     * @return
     */
    STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

    /**
     * @brief Set the scroll bar position
     * @param bVertical -- TRUE-vertical scroll bar
     * @param nNewPos -- scroll position
     * @param bRedraw -- redraw flag
     * @return
     */
    STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

    /**
     * @brief Get the scroll bar position
     * @param bVertical -- TRUE-vertical scroll bar
     * @return scroll bar position
     */
    STDMETHOD_(int, GetScrollPos)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief Set the scroll bar range
     * @param bVertical -- TRUE-vertical scroll bar
     * @param nMinPos -- minimum value
     * @param nMaxPos -- maximum value
     * @param bRedraw -- redraw flag
     * @return TRUE on success
     */
    STDMETHOD_(BOOL, SetScrollRange)
    (THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

    /**
     * @brief Get the scroll bar range
     * @param bVertical -- TRUE-vertical scroll bar
     * @param lpMinPos -- minimum value
     * @param lpMaxPos -- maximum value
     * @return
     */
    STDMETHOD_(BOOL, GetScrollRange)
    (CTHIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;

    /**
     * @brief Query the scroll bar enabled state
     * @param bVertical -- TRUE-vertical scroll bar
     * @return TRUE if enabled
     */
    STDMETHOD_(BOOL, HasScrollBar)(CTHIS_ BOOL bVertical) SCONST PURE;
    ///////////////////////////////////////////////////////////////////////
    /**
     * @brief Set the list's Adapter
     * @param adapter -- Adapter object
     * @return BOOL
     * @remark Usually should only be called once during list initialization; the list increments the Adapter's reference count
     */
    STDMETHOD_(BOOL, SetAdapter)(THIS_ ITvAdapter * adapter) PURE;

    /**
     * @brief Get the control's Adapter object
     * @return ILvAdapter* - Adapter object
     */
    STDMETHOD_(ITvAdapter *, GetAdapter)(CTHIS) SCONST PURE;

    /**
     * @brief Set the list item position locator
     * @param pItemLocator -- position locator
     * @remark The position locator is an interface that maps index numbers to list display positions; by implementing it, the user can control the display size of list items
     */
    STDMETHOD_(void, SetItemLocator)(THIS_ ITreeViewItemLocator * pItemLocator) PURE;

    /**
     * @brief Get the position locator
     * @return IListViewItemLocator *
     */
    STDMETHOD_(ITreeViewItemLocator *, GetItemLocator)(CTHIS) SCONST PURE;

    /**
     * @brief Scroll the specified row into the visible view area
     * @param hItem -- HSTREEITEM
     * @return
     */
    STDMETHOD_(void, EnsureVisible)(THIS_ HSTREEITEM hItem) PURE;

    /**
     * @brief Select the specified row
     * @param hItem -- HSTREEITEM
     * @param bNotify -- flag to trigger the selection state event
     * @return
     */
    STDMETHOD_(void, SetSel)(THIS_ HSTREEITEM hItem, BOOL bNotify DEF_VAL(FALSE)) PURE;

    /**
     * @brief Get the currently selected row
     * @return HSTREEITEM selected row index, 0 means no row is currently selected
     */
    STDMETHOD_(HSTREEITEM, GetSel)(CTHIS) SCONST PURE;

    /**
     * @brief Hit test
     * @param pt -- input coordinates
     * @return IItemPanel *-- the ItemPanel under the coordinates
     */
    STDMETHOD_(IItemPanel *, HitTest)(CTHIS_ const POINT *pt) SCONST PURE;

    /**
     * @brief Sets the multiple selection mode.
     * @param bMultiSel TRUE to enable multiple selection, FALSE otherwise.
     */
    STDMETHOD_(void, SetMultiSel)(THIS_ BOOL bMultiSel) PURE;

    /**
     * @brief Gets the multiple selection mode.
     * @return TRUE if multiple selection is enabled, FALSE otherwise.
     */
    STDMETHOD_(BOOL, GetMultiSel)(THIS) SCONST PURE;

    /**
     * @brief Adds an item to the selection.
     * @param hItem Handle to the item.
     */
    STDMETHOD_(void, AddSelItem)(THIS_ HSTREEITEM hItem) PURE;

    /**
     * @brief Removes an item from the selection.
     * @param hItem Handle to the item.
     */
    STDMETHOD_(void, RemoveSelItem)(THIS_ HSTREEITEM hItem) PURE;

    /**
     * @brief Clears all selected items.
     */
    STDMETHOD_(void, ClearSelItems)(THIS) PURE;

    /**
     * @brief Checks if an item is selected.
     * @param hItem Handle to the item.
     * @return TRUE if the item is selected, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsItemSelected)(THIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Gets the count of selected items.
     * @return Number of selected items.
     */
    STDMETHOD_(int, GetSelItemCount)(THIS) SCONST PURE;

    /**
     * @brief Gets all selected items.
     * @param items Output parameter to store the selected item handles.
     * @param nMaxCount Maximum number of items to retrieve.
     * @return Number of selected items.
     */
    STDMETHOD_(int, GetSelItems)(THIS_ HSTREEITEM * items, int nMaxCount) SCONST PURE;
};

/** {5586EA88-3D4B-4d5f-8F71-436D43420D00} */
DEFINE_SGUID(IID_ITileView, 0x5586ea88, 0x3d4b, 0x4d5f, 0x8f, 0x71, 0x43, 0x6d, 0x43, 0x42, 0xd, 0x0);
#undef INTERFACE
#define INTERFACE ITileView
DECLARE_INTERFACE_(ITileView, IPanel)
{
    DECLARE_CLASS_SIID(IID_ITileView)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Show/hide the scroll bar
     * @param wBar scroll bar flag, see ScrollBarID
     * @param bShow TRUE-show, FALSE-hide
     * @return
     */
    STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

    /**
     * @brief Enable/Disable the scroll bar
     * @param wBar scroll bar flag, see ScrollBarID
     * @param bShow TRUE-show, FALSE-hide
     * @return
     */
    STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

    /**
     * @brief Get the scroll bar Enable state
     * @param bVertical -- TRUE-vertical scroll bar
     * @return TRUE if enabled
     */
    STDMETHOD_(BOOL, IsScrollBarEnable)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief Set the scroll bar data
     * @param si -- scroll bar data
     * @param bVertical -- TRUE-vertical scroll bar
     * @return
     */
    STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

    /**
     * @brief Set the scroll bar position
     * @param bVertical -- TRUE-vertical scroll bar
     * @param nNewPos -- scroll position
     * @param bRedraw -- redraw flag
     * @return
     */
    STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

    /**
     * @brief Get the scroll bar position
     * @param bVertical -- TRUE-vertical scroll bar
     * @return scroll bar position
     */
    STDMETHOD_(int, GetScrollPos)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief Set the scroll bar range
     * @param bVertical -- TRUE-vertical scroll bar
     * @param nMinPos -- minimum value
     * @param nMaxPos -- maximum value
     * @param bRedraw -- redraw flag
     * @return TRUE on success
     */
    STDMETHOD_(BOOL, SetScrollRange)
    (THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

    /**
     * @brief Get the scroll bar range
     * @param bVertical -- TRUE-vertical scroll bar
     * @param lpMinPos -- minimum value
     * @param lpMaxPos -- maximum value
     * @return
     */
    STDMETHOD_(BOOL, GetScrollRange)
    (CTHIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;

    /**
     * @brief Query the scroll bar enabled state
     * @param bVertical -- TRUE-vertical scroll bar
     * @return TRUE if enabled
     */
    STDMETHOD_(BOOL, HasScrollBar)(CTHIS_ BOOL bVertical) SCONST PURE;
    ///////////////////////////////////////////////////////////////////////
    /**
     * @brief Set the list's Adapter
     * @param adapter -- Adapter object
     * @return BOOL
     * @remark Usually should only be called once during list initialization; the list increments the Adapter's reference count
     */
    STDMETHOD_(BOOL, SetAdapter)(THIS_ ILvAdapter * adapter) PURE;

    /**
     * @brief Get the control's Adapter object
     * @return ILvAdapter* - Adapter object
     */
    STDMETHOD_(ILvAdapter *, GetAdapter)(CTHIS) SCONST PURE;

    /**
     * @brief Set the list item position locator
     * @param pItemLocator -- position locator
     * @remark The position locator is an interface that maps index numbers to list display positions; by implementing it, the user can control the display size of list items
     */
    STDMETHOD_(void, SetItemLocator)(THIS_ ITileViewItemLocator * pItemLocator) PURE;

    /**
     * @brief Get the position locator
     * @return ITileViewItemLocator *
     */
    STDMETHOD_(ITileViewItemLocator *, GetItemLocator)(CTHIS) SCONST PURE;

    /**
     * @brief Scroll the specified row into the visible view area
     * @param iItem -- row number
     * @return
     */
    STDMETHOD_(void, EnsureVisible)(THIS_ int iItem) PURE;

    /**
     * @brief Select the specified row
     * @param iItem -- row number
     * @param bNotify -- flag to trigger the selection state event
     * @return
     */
    STDMETHOD_(void, SetSel)(THIS_ int iItem, BOOL bNotify DEF_VAL(FALSE)) PURE;

    /**
     * @brief Get the currently selected row
     * @return int selected row index, -1 means no row is currently selected
     */
    STDMETHOD_(int, GetSel)(CTHIS) SCONST PURE;

    /**
     * @brief Hit test
     * @param pt -- input coordinates
     * @return IItemPanel *-- the ItemPanel under the coordinates
     */
    STDMETHOD_(IItemPanel *, HitTest)(CTHIS_ const POINT *pt) SCONST PURE;

    /**
     * @brief Sets the multiple selection mode.
     * @param bMultiSel TRUE to enable multiple selection, FALSE otherwise.
     */
    STDMETHOD_(void, SetMultiSel)(THIS_ BOOL bMultiSel) PURE;

    /**
     * @brief Gets the multiple selection mode.
     * @return TRUE if multiple selection is enabled, FALSE otherwise.
     */
    STDMETHOD_(BOOL, GetMultiSel)(CTHIS) SCONST PURE;

    /**
     * @brief Adds an item to the selection.
     * @param iItem Index of the item.
     */
    STDMETHOD_(void, AddSelItem)(THIS_ int iItem) PURE;

    /**
     * @brief Removes an item from the selection.
     * @param iItem Index of the item.
     */
    STDMETHOD_(void, RemoveSelItem)(THIS_ int iItem) PURE;

    /**
     * @brief Clears all selected items.
     */
    STDMETHOD_(void, ClearSelItems)(THIS) PURE;

    /**
     * @brief Checks if an item is selected.
     * @param iItem Index of the item.
     * @return TRUE if the item is selected, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsItemSelected)(THIS_ int iItem) SCONST PURE;

    /**
     * @brief Gets the count of selected items.
     * @return Number of selected items.
     */
    STDMETHOD_(int, GetSelItemCount)(CTHIS) SCONST PURE;

    /**
     * @brief Gets all selected items.
     * @param items Output parameter to store the selected item indices.
     * @param nMaxCount Maximum number of items to retrieve.
     * @return Number of selected items.
     */
    STDMETHOD_(int, GetSelItems)(THIS_ int *items, int nMaxCount) SCONST PURE;
};

/** {B0B45363-3D10-4889-9459-A2A84E37575F} */
DEFINE_SGUID(IID_IListBox, 0xb0b45363, 0x3d10, 0x4889, 0x94, 0x59, 0xa2, 0xa8, 0x4e, 0x37, 0x57, 0x5f);
#undef INTERFACE
#define INTERFACE IListBox
DECLARE_INTERFACE_(IListBox, IPanel)
{
    DECLARE_CLASS_SIID(IID_IListBox)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Show/hide the scroll bar
     * @param wBar scroll bar flag, see ScrollBarID
     * @param bShow TRUE-show, FALSE-hide
     * @return
     */
    STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

    /**
     * @brief Enable/Disable the scroll bar
     * @param wBar scroll bar flag, see ScrollBarID
     * @param bShow TRUE-show, FALSE-hide
     * @return
     */
    STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

    /**
     * @brief Get the scroll bar Enable state
     * @param bVertical -- TRUE-vertical scroll bar
     * @return TRUE if enabled
     */
    STDMETHOD_(BOOL, IsScrollBarEnable)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief Set the scroll bar data
     * @param si -- scroll bar data
     * @param bVertical -- TRUE-vertical scroll bar
     * @return
     */
    STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

    /**
     * @brief Set the scroll bar position
     * @param bVertical -- TRUE-vertical scroll bar
     * @param nNewPos -- scroll position
     * @param bRedraw -- redraw flag
     * @return
     */
    STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

    /**
     * @brief Get the scroll bar position
     * @param bVertical -- TRUE-vertical scroll bar
     * @return scroll bar position
     */
    STDMETHOD_(int, GetScrollPos)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief Set the scroll bar range
     * @param bVertical -- TRUE-vertical scroll bar
     * @param nMinPos -- minimum value
     * @param nMaxPos -- maximum value
     * @param bRedraw -- redraw flag
     * @return TRUE on success
     */
    STDMETHOD_(BOOL, SetScrollRange)
    (THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

    /**
     * @brief Get the scroll bar range
     * @param bVertical -- TRUE-vertical scroll bar
     * @param lpMinPos -- minimum value
     * @param lpMaxPos -- maximum value
     * @return
     */
    STDMETHOD_(BOOL, GetScrollRange)
    (CTHIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;

    /**
     * @brief Query the scroll bar enabled state
     * @param bVertical -- TRUE-vertical scroll bar
     * @return TRUE if enabled
     */
    STDMETHOD_(BOOL, HasScrollBar)(CTHIS_ BOOL bVertical) SCONST PURE;
    ///////////////////////////////////////////////////////////////////////
    /**
     * SListBox::GetCount
     * @brief    Get the item count
     * @return   returns int
     *
     * Describe  Get the item count
     */
    STDMETHOD_(int, GetCount)(CTHIS) SCONST PURE;

    /**
     * SListBox::GetCurSel
     * @brief    Get the current selected item index
     * @return   returns int
     *
     * Describe  Get the current selected item index
     */
    STDMETHOD_(int, GetCurSel)(CTHIS) SCONST PURE;

    /**
     * SListBox::SetCurSel
     * @brief    Set the selected item
     * @param    int nIndex -- index to select
     *
     * Describe  Set the selected item
     */
    STDMETHOD_(BOOL, SetCurSel)(THIS_ int nIndex, BOOL bNotifyChange /**< = FALSE */) PURE;

    /**
     * SListBox::GetTopIndex
     * @brief    Get the index of the first visible record
     * @return   returns int
     *
     * Describe
     */
    STDMETHOD_(int, GetTopIndex)(CTHIS) SCONST PURE;

    /**
     * SListBox::SetTopIndex
     * @brief    Set the index of the first visible record
     * @param    int nIndex -- index of the first visible record
     *
     * Describe
     */
    STDMETHOD_(BOOL, SetTopIndex)(THIS_ int nIndex) PURE;

    /**
     * SListBox::GetItemHeight
     * @brief    Get the height
     * @return   returns int
     *
     * Describe  Get the height
     */
    STDMETHOD_(int, GetItemHeight)(CTHIS) SCONST PURE;

    /**
     * SListBox::SetItemHeight
     * @brief    Set the height of the specified item
     * @param    int cyItemHeight -- height
     * @return   returns
     *
     * Describe  Set the height of the specified item
     */
    STDMETHOD_(void, SetItemHeight)(THIS_ int nItemHeight) PURE;

    /**
     * SListBox::GetItemData
     * @brief    Get the associated data
     * @param    int nIndex -- option index
     * @return   returns LPARAM
     *
     * Describe  Get the associated data
     */
    STDMETHOD_(LPARAM, GetItemData)(CTHIS_ int nIndex) SCONST PURE;

    /**
     * SListBox::SetItemData
     * @brief    Set the associated data of the item
     * @param    int nIndex -- option index
     * @param    LPARAM lParam  -- associated data
     * @return   returns BOOL
     *
     * Describe  Set the associated data of the item
     */
    STDMETHOD_(BOOL, SetItemData)(THIS_ int nIndex, LPARAM lParam) PURE;

    /**
     * SListBox::GetText
     * @brief    Get the text of the specified item
     * @param    int nIndex -- option index
     * @param    BOOL bRawText -- raw data flag
     * @return   SStringT, the raw string of the list item
     *
     * Describe  Get the text of the specified item
     */
    STDMETHOD_(BOOL, GetIText)
    (CTHIS_ int nIndex, BOOL bRawText /**< = FALSE */, IStringT *str) SCONST PURE;

    /**
     * SListBox::DeleteAll
     * @brief    Delete all
     *
     * Describe  Delete all
     */
    STDMETHOD_(void, DeleteAll)(THIS) PURE;

    /**
     * SListBox::DeleteString
     * @brief    Set the text of the specified item
     * @param    int nIndex -- option index
     * @return   returns BOOL
     *
     * Describe  Set the text of the specified item
     */
    STDMETHOD_(BOOL, DeleteString)(THIS_ int nIndex) PURE;

    /**
     * SListBox::AddString
     * @brief    Add text
     * @param    LPCTSTR lpszItem -- text
     * @param    int nImage -- icon
     * @param    LPARAM lParam -- additional data
     * @return   returns int index
     *
     * Describe  Add text
     */
    STDMETHOD_(int, AddString)
    (THIS_ LPCTSTR lpszItem, int nImage /**< = -1 */, LPARAM lParam /**< = 0 */) PURE;

    /**
     * SListBox::InsertString
     * @brief    Insert text
     * @param    int nIndex  -- index
     * @param    LPCTSTR lpszItem -- text
     * @param    int nImage -- icon
     * @param    LPARAM lParam -- additional data
     * @return   returns int index
     *
     * Describe  Insert text at a specified item
     */
    STDMETHOD_(int, InsertString)
    (THIS_ int nIndex, LPCTSTR lpszItem, int nImage /**< = -1 */, LPARAM lParam /**< = 0 */) PURE;

    /**
     * SListBox::EnsureVisible
     * @brief    Make a specified item visible
     * @param    int nIndex  -- index
     *
     * Describe  Make a specified item visible
     */
    STDMETHOD_(void, EnsureVisible)(THIS_ int nIndex) PURE;

    /**
     * @brief Find the position of a string in the list
     * @param iFindAfter --start index
     * @param pszText --target string
     * @return int index found, -1 means not found
     */
    STDMETHOD_(int, FindString)(CTHIS_ int iFindAfter, LPCTSTR pszText) SCONST PURE;

    STDMETHOD_(BOOL, SetItemImage)(THIS_ int nIndex, int iImage) PURE;

    STDMETHOD_(int, GetItemImage)(THIS_ int nIndex) PURE;
};

/** {5FB30471-54CB-4db8-9160-31B545E28BC5} */
DEFINE_SGUID(IID_IComboBase, 0x5fb30471, 0x54cb, 0x4db8, 0x91, 0x60, 0x31, 0xb5, 0x45, 0xe2, 0x8b, 0xc5);
#undef INTERFACE
#define INTERFACE IComboBase
DECLARE_INTERFACE_(IComboBase, ICtrl)
{
    DECLARE_CLASS_SIID(IID_IComboBase)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////
    /**
     * SComboBoxBase::GetCurSel
     * @brief    Get the selected value index
     * @return   returns int
     *
     * Describe  Get the current selected index
     */
    STDMETHOD_(int, GetCurSel)(CTHIS) SCONST PURE;

    /**
     * SComboBoxBase::GetCount
     * @brief    Get the number of dropdown items
     * @return   returns int
     *
     * Describe  Get the number of dropdown items
     */
    STDMETHOD_(int, GetCount)(CTHIS) SCONST PURE;

    /**
     * SComboBoxBase::SetCurSel
     * @brief    Set current selection
     * @param    int iSel -- selected index
     *
     * Describe  Set current selection
     */
    STDMETHOD_(BOOL, SetCurSel)(THIS_ int iSel) PURE;

    /**
     * @brief Get the text data of the specified item
     * @param iItem --index
     * @param bRawText --flag to get raw text
     * @param[out] str output string object
     * @return BOOL TRUE-success
     * @remark When bRawText is TRUE, outputs untranslated text
     */
    STDMETHOD_(BOOL, GetItemText)(CTHIS_ int iItem, BOOL bRawText, IStringT *str) SCONST PURE;
    STDMETHOD_(BOOL, GetItemTextA)(CTHIS_ int iItem, BOOL bRawText, IStringA *str) SCONST PURE;

    /**
     * FindString
     * @brief    Find string position
     * @param    LPCTSTR pszFind --  search target
     * @param    int nAfter --  start position
     * @return   int -- target index, returns -1 on failure.
     * Describe
     */
    STDMETHOD_(int, FindString)
    (THIS_ LPCTSTR pszFind, int nAfter DEF_VAL(-1), BOOL bPartMatch DEF_VAL(TRUE)) PURE;
    STDMETHOD_(int, FindStringA)
    (THIS_ LPCSTR pszFind, int nAfter DEF_VAL(-1), BOOL bPartMatch DEF_VAL(TRUE)) PURE;

    ///////////////////////////////////////////////////////////////////////
    /**
     * SComboBoxBase::DropDown
     * @brief    Dropdown event
     *
     * Describe  Dropdown event
     */
    STDMETHOD_(void, DropDown)(THIS) PURE;

    /**
     * SComboBoxBase::CloseUp
     * @brief    Dropdown closed
     *
     * Describe  Dropdown closed
     */
    STDMETHOD_(void, CloseUp)(THIS) PURE;

    /**
     * @brief Query dropdown state flag
     * @return TRUE-current ComboBase is a dropdown list
     * @remark Internal use only
     */
    STDMETHOD_(BOOL, IsDropdown)(CTHIS) SCONST PURE;

    /**
     * @brief Set dropdown state flag
     * @param bDropdown -- TRUE:dropdown list
     * @return
     * @remark Internal use only
     */
    STDMETHOD_(void, SetDropdown)(THIS_ BOOL bDropdown) PURE;
};

/** {E7F9C5A6-EA49-41c8-9D8F-7075CB9674EA} */
DEFINE_SGUID(IID_IComboBox, 0xe7f9c5a6, 0xea49, 0x41c8, 0x9d, 0x8f, 0x70, 0x75, 0xcb, 0x96, 0x74, 0xea);
#undef INTERFACE
#define INTERFACE IComboBox
DECLARE_INTERFACE_(IComboBox, IComboBase)
{
    DECLARE_CLASS_SIID(IID_IComboBox)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////
    /**
     * SComboBoxBase::GetCurSel
     * @brief    Get the selected value index
     * @return   returns int
     *
     * Describe  Get the current selected index
     */
    STDMETHOD_(int, GetCurSel)(CTHIS) SCONST PURE;

    /**
     * SComboBoxBase::GetCount
     * @brief    Get the number of dropdown items
     * @return   returns int
     *
     * Describe  Get the number of dropdown items
     */
    STDMETHOD_(int, GetCount)(CTHIS) SCONST PURE;

    /**
     * SComboBoxBase::SetCurSel
     * @brief    Set current selection
     * @param    int iSel -- selected index
     *
     * Describe  Set current selection
     */
    STDMETHOD_(BOOL, SetCurSel)(THIS_ int iSel) PURE;

    /**
     * @brief Get the text data of the specified item
     * @param iItem --index
     * @param bRawText --flag to get raw text
     * @param[out] str output string object
     * @return BOOL TRUE-success
     * @remark When bRawText is TRUE, outputs untranslated text
     */
    STDMETHOD_(BOOL, GetItemText)(CTHIS_ int iItem, BOOL bRawText, IStringT *str) SCONST PURE;

    STDMETHOD_(BOOL, GetItemTextA)(CTHIS_ int iItem, BOOL bRawText, IStringA *str) SCONST PURE;

    /**
     * FindString
     * @brief    Find string position
     * @param    LPCTSTR pszFind --  search target
     * @param    int nAfter --  start position
     * @return   int -- target index, returns -1 on failure.
     * Describe
     */
    STDMETHOD_(int, FindString)
    (THIS_ LPCTSTR pszFind, int nAfter DEF_VAL(-1), BOOL bPartMatch DEF_VAL(TRUE)) PURE;
    STDMETHOD_(int, FindStringA)
    (THIS_ LPCSTR pszFind, int nAfter DEF_VAL(-1), BOOL bPartMatch DEF_VAL(TRUE)) PURE;

    /**
     * SComboBoxBase::DropDown
     * @brief    Dropdown event
     *
     * Describe  Dropdown event
     */
    STDMETHOD_(void, DropDown)(THIS) PURE;

    /**
     * SComboBoxBase::CloseUp
     * @brief    Dropdown closed
     *
     * Describe  Dropdown closed
     */
    STDMETHOD_(void, CloseUp)(THIS) PURE;

    /**
     * @brief Query dropdown state flag
     * @return TRUE-current ComboBase is a dropdown list
     * @remark Internal use only
     */
    STDMETHOD_(BOOL, IsDropdown)(CTHIS) SCONST PURE;

    /**
     * @brief Set dropdown state flag
     * @param bDropdown -- TRUE:dropdown list
     * @return
     * @remark Internal use only
     */
    STDMETHOD_(void, SetDropdown)(THIS_ BOOL bDropdown) PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * SComboBox::GetItemData
     * @brief    Get additional data
     * @param    UINT iItem -- option value
     *
     * Describe  Get additional data
     */
    STDMETHOD_(LPARAM, GetItemData)(CTHIS_ UINT iItem) SCONST PURE;

    /**
     * SComboBox::SetItemData
     * @brief    Set additional data
     * @param    UINT iItem -- index value
     * @param    LPARAM lParam -- additional value
     *
     * Describe  Set additional data
     */
    STDMETHOD_(BOOL, SetItemData)(THIS_ UINT iItem, LPARAM lParam) PURE;

    /**
     * SComboBox::InsertItem
     * @brief    Insert a new item
     * @param    UINT iPos -- position
     * @param    LPCTSTR pszText -- text value
     * @param    int iIcon -- icon
     * @param    LPARAM lParam -- additional value
     *
     * Describe  Insert a new item
     */
    STDMETHOD_(int, InsertItem)(THIS_ int iPos, LPCTSTR pszText, int iIcon, LPARAM lParam) PURE;
    STDMETHOD_(int, InsertItemA)(THIS_ int iPos, LPCSTR pszText, int iIcon, LPARAM lParam) PURE;
    /**
     * SComboBox::DeleteString
     * @brief    Delete an item
     * @param    UINT iItem -- index value
     *
     * Describe  Delete an item
     */
    STDMETHOD_(BOOL, DeleteString)(THIS_ int iPos) PURE;
    /**
     * SComboBox::ResetContent
     * @brief    Delete all items
     *
     * Describe  Set additional data
     */
    STDMETHOD_(void, ResetContent)(THIS) PURE;

    /**
     * @brief Get the IListBox interface
     * @return IListBox *
     */
    STDMETHOD_(IListBox *, GetIListBox)(THIS) PURE;
};

/** {36DD3BAD-523B-421a-9311-3DD0F0BF06D1} */
DEFINE_SGUID(IID_IComboView, 0x36dd3bad, 0x523b, 0x421a, 0x93, 0x11, 0x3d, 0xd0, 0xf0, 0xbf, 0x6, 0xd1);
#undef INTERFACE
#define INTERFACE IComboView
DECLARE_INTERFACE_(IComboView, IComboBase)
{
    DECLARE_CLASS_SIID(IID_IComboView)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////
    /**
     * SComboBoxBase::GetCurSel
     * @brief    Get the selected value index
     * @return   returns int
     *
     * Describe  Get the current selected index
     */
    STDMETHOD_(int, GetCurSel)(CTHIS) SCONST PURE;

    /**
     * SComboBoxBase::GetCount
     * @brief    Get the number of dropdown items
     * @return   returns int
     *
     * Describe  Get the number of dropdown items
     */
    STDMETHOD_(int, GetCount)(CTHIS) SCONST PURE;

    /**
     * SComboBoxBase::SetCurSel
     * @brief    Set current selection
     * @param    int iSel -- selected index
     *
     * Describe  Set current selection
     */
    STDMETHOD_(BOOL, SetCurSel)(THIS_ int iSel) PURE;

    /**
     * @brief Get the text data of the specified item
     * @param iItem --index
     * @param bRawText --flag to get raw text
     * @param[out] str output string object
     * @return BOOL TRUE-success
     * @remark When bRawText is TRUE, outputs untranslated text
     */
    STDMETHOD_(BOOL, GetItemText)(CTHIS_ int iItem, BOOL bRawText, IStringT *str) SCONST PURE;

    /**
     * FindString
     * @brief    Find string position
     * @param    LPCTSTR pszFind --  search target
     * @param    int nAfter --  start position
     * @return   int -- target index, returns -1 on failure.
     * Describe
     */
    STDMETHOD_(int, FindString)
    (THIS_ LPCTSTR pszFind, int nAfter /**< = -1 */, BOOL bPartMatch /**< = TRUE */) PURE;

    /**
     * SComboBoxBase::DropDown
     * @brief    Dropdown event
     *
     * Describe  Dropdown event
     */
    STDMETHOD_(void, DropDown)(THIS) PURE;

    /**
     * SComboBoxBase::CloseUp
     * @brief    Dropdown closed
     *
     * Describe  Dropdown closed
     */
    STDMETHOD_(void, CloseUp)(THIS) PURE;

    /**
     * @brief Query dropdown state flag
     * @return TRUE-current ComboBase is a dropdown list
     * @remark Internal use only
     */
    STDMETHOD_(BOOL, IsDropdown)(CTHIS) SCONST PURE;

    /**
     * @brief Set dropdown state flag
     * @param bDropdown -- TRUE:dropdown list
     * @return
     * @remark Internal use only
     */
    STDMETHOD_(void, SetDropdown)(THIS_ BOOL bDropdown) PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Get the IListView interface
     * @return IListView *
     */
    STDMETHOD_(IListView *, GetIListView)(THIS) PURE;
};

/** {846F25CD-E0DB-49b0-8F20-0BD70538F631} */
DEFINE_SGUID(IID_IDateTimePicker, 0x846f25cd, 0xe0db, 0x49b0, 0x8f, 0x20, 0xb, 0xd7, 0x5, 0x38, 0xf6, 0x31);
#undef INTERFACE
#define INTERFACE IDateTimePicker
DECLARE_INTERFACE_(IDateTimePicker, ICtrl)
{
    DECLARE_CLASS_SIID(IID_IDateTimePicker)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Set time
     * @param wYear --year
     * @param wMonth --month
     * @param wDay --day
     * @param wHour --hour
     * @param wMinute --minute
     * @param wSecond --second
     * @return
     */
    STDMETHOD_(void, SetTime)
    (THIS_ WORD wYear, WORD wMonth, WORD wDay, WORD wHour, WORD wMinute, WORD wSecond) PURE;

    /**
     * @brief Get current time
     * @param[out] wYear --year
     * @param[out] wMonth --month
     * @param[out] wDay --day
     * @param[out] wHour --hour
     * @param[out] wMinute --minute
     * @param[out] wSecond --second
     * @return
     */
    STDMETHOD_(void, GetTime)
    (CTHIS_ WORD * wYear, WORD * wMonth, WORD * wDay, WORD * wHour, WORD * wMinute, WORD * wSecond) SCONST PURE;

    /**
     * @brief Close the dropdown list
     * @return
     */
    STDMETHOD_(void, CloseUp)(THIS) PURE;

    /**
     * @brief Show the dropdown list
     * @return
     */
    STDMETHOD_(void, DropDown)(THIS) PURE;

    /**
     * @brief Clear data
     * @return
     */
    STDMETHOD_(void, Clear)(THIS) PURE;
};

typedef int (*FunTreeSortCallback)(void *pCtx, const void *phItem1, const void *phItem2);

#ifndef STVI_ROOT
#ifdef _WIN64
#define STVI_ROOT  ((HSTREEITEM)0xFFFF000000000000) /**< =TVI_ROOT */
#define STVI_FIRST ((HSTREEITEM)0xFFFF000000000001) /**< =TVI_FIRST */
#define STVI_LAST  ((HSTREEITEM)0xFFFF000000000002) /**< =TVI_LAST */
#else
#define STVI_ROOT  ((HSTREEITEM)0xFFFF0000) /**< =TVI_ROOT */
#define STVI_FIRST ((HSTREEITEM)0xFFFF0001) /**< =TVI_FIRST */
#define STVI_LAST  ((HSTREEITEM)0xFFFF0002) /**< =TVI_LAST */
#endif
#endif /**< STVI_ROOT */

/** {A877B61C-7E5E-4716-8423-8DC61EAF61E9} */
DEFINE_SGUID(IID_ITreeCtrl, 0xa877b61c, 0x7e5e, 0x4716, 0x84, 0x23, 0x8d, 0xc6, 0x1e, 0xaf, 0x61, 0xe9);
#undef INTERFACE
#define INTERFACE ITreeCtrl
DECLARE_INTERFACE_(ITreeCtrl, IPanel)
{
    DECLARE_CLASS_SIID(IID_ITreeCtrl)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Show/hide the scroll bar
     * @param wBar scroll bar flag, see ScrollBarID
     * @param bShow TRUE-show, FALSE-hide
     * @return
     */
    STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

    /**
     * @brief Enable/Disable the scroll bar
     * @param wBar scroll bar flag, see ScrollBarID
     * @param bShow TRUE-show, FALSE-hide
     * @return
     */
    STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

    /**
     * @brief Get the scroll bar Enable state
     * @param bVertical -- TRUE-vertical scroll bar
     * @return TRUE if enabled
     */
    STDMETHOD_(BOOL, IsScrollBarEnable)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief Set the scroll bar data
     * @param si -- scroll bar data
     * @param bVertical -- TRUE-vertical scroll bar
     * @return
     */
    STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

    /**
     * @brief Set the scroll bar position
     * @param bVertical -- TRUE-vertical scroll bar
     * @param nNewPos -- scroll position
     * @param bRedraw -- redraw flag
     * @return
     */
    STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

    /**
     * @brief Get the scroll bar position
     * @param bVertical -- TRUE-vertical scroll bar
     * @return scroll bar position
     */
    STDMETHOD_(int, GetScrollPos)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief Set the scroll bar range
     * @param bVertical -- TRUE-vertical scroll bar
     * @param nMinPos -- minimum value
     * @param nMaxPos -- maximum value
     * @param bRedraw -- redraw flag
     * @return TRUE on success
     */
    STDMETHOD_(BOOL, SetScrollRange)
    (THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

    /**
     * @brief Get the scroll bar range
     * @param bVertical -- TRUE-vertical scroll bar
     * @param lpMinPos -- minimum value
     * @param lpMaxPos -- maximum value
     * @return
     */
    STDMETHOD_(BOOL, GetScrollRange)
    (CTHIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;

    /**
     * @brief Query the scroll bar enabled state
     * @param bVertical -- TRUE-vertical scroll bar
     * @return TRUE if enabled
     */
    STDMETHOD_(BOOL, HasScrollBar)(CTHIS_ BOOL bVertical) SCONST PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Insert an HSTREEITEM
     * @param lpszItem --display text
     * @param nImage --image index
     * @param nSelectedImage --image index in selected state
     * @param lParam --custom data
     * @param hParent --parent HSTREEITEM
     * @param hInsertAfter --preceding sibling HSTREEITEM
     * @return HSTREEITEM--newly inserted HSTREEITEM
     */
    STDMETHOD_(HSTREEITEM, InsertItem)
    (THIS_ LPCTSTR lpszItem, int nImage, int nSelectedImage, LPARAM lParam, HSTREEITEM hParent DEF_VAL(STVI_ROOT), HSTREEITEM hInsertAfter DEF_VAL(STVI_LAST)) PURE;

    STDMETHOD_(HSTREEITEM, InsertItemA)
    (THIS_ LPCSTR lpszItem, int nImage, int nSelectedImage, LPARAM lParam, HSTREEITEM hParent DEF_VAL(STVI_ROOT), HSTREEITEM hInsertAfter DEF_VAL(STVI_LAST)) PURE;

    /**
     * @brief Delete an HSTREEITEM
     * @param hItem --HSTREEITEM to delete
     * @return BOOL
     */
    STDMETHOD_(BOOL, RemoveItem)(THIS_ HSTREEITEM hItem) PURE;

    /**
     * @brief Delete all data
     * @return
     */
    STDMETHOD_(void, RemoveAllItems)(THIS) PURE;

    /**
     * @brief Get the root node
     * @return HSTREEITEM --root node
     */
    STDMETHOD_(HSTREEITEM, GetRootItem)(CTHIS) SCONST PURE;

    /**
     * @brief Get the next sibling node of the specified node
     * @param hItem --specified node
     * @return HSTREEITEM--next sibling node
     */
    STDMETHOD_(HSTREEITEM, GetNextSiblingItem)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Get the previous sibling node of the specified node
     * @param hItem --specified node
     * @return HSTREEITEM--previous sibling node
     */
    STDMETHOD_(HSTREEITEM, GetPrevSiblingItem)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Get the child node of the specified node
     * @param hItem --specified node
     * @param bFirst -- TRUE:get the first child node, FALSE:get the last child node
     * @return HSTREEITEM--child node
     */
    STDMETHOD_(HSTREEITEM, GetChildItem)
    (CTHIS_ HSTREEITEM hItem, BOOL bFirst DEF_VAL(TRUE)) SCONST PURE;

    /**
     * @brief Get the parent node of the specified node
     * @param hItem --specified node
     * @return HSTREEITEM--parent node
     */
    STDMETHOD_(HSTREEITEM, GetParentItem)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Get the selected node
     * @return HSTREEITEM--selected node
     */
    STDMETHOD_(HSTREEITEM, GetSelectedItem)(CTHIS) SCONST PURE;

    /**
     * @brief Get the next node
     * @param hItem -- current node
     * @return HSTREEITEM--next node
     * @remark If it has child nodes, returns the first child; otherwise the next sibling; if no sibling either, the parent's next sibling
     */
    STDMETHOD_(HSTREEITEM, GetNextItem)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Sort child nodes
     * @param hItem --current node
     * @param sortFunc --sort callback function
     * @param pCtx --context for the callback function
     * @return
     */
    STDMETHOD_(void, SortChildren)
    (THIS_ HSTREEITEM hItem, FunTreeSortCallback sortFunc, void *pCtx) PURE;

    /**
     * @brief Select the specified item
     * @param hItem --specified item
     * @param bEnsureVisible --flag to automatically scroll into view
     * @return
     */
    STDMETHOD_(BOOL, SelectItem)(THIS_ HSTREEITEM hItem, BOOL bEnsureVisible DEF_VAL(TRUE)) PURE;

    /**
     * @brief Get the text data of the specified item
     * @param hItem --specified item
     * @param[out] strText --text data
     * @return BOOL
     * @remark Translation not supported
     */
    STDMETHOD_(BOOL, GetItemText)(CTHIS_ HSTREEITEM hItem, IStringT * strText) SCONST PURE;
    STDMETHOD_(BOOL, GetItemTextA)(CTHIS_ HSTREEITEM hItem, IStringA * strText) SCONST PURE;

    /**
     * @brief Set text data
     * @param hItem --specified item
     * @param lpszItem  --text data
     * @return BOOL
     */
    STDMETHOD_(BOOL, SetItemText)(THIS_ HSTREEITEM hItem, LPCTSTR lpszItem) PURE;
    STDMETHOD_(BOOL, SetItemTextA)(THIS_ HSTREEITEM hItem, LPCSTR lpszItem) PURE;

    /**
     * @brief Get the icon display data of the specified item
     * @param hItem --specified item
     * @param nImage --image index
     * @param nSelectedImage --image index in selected state
     * @return BOOL
     */
    STDMETHOD_(BOOL, GetItemImage)
    (CTHIS_ HSTREEITEM hItem, int *nImage, int *nSelectedImage) SCONST PURE;

    /**
     * @brief Set the image index of the specified item
     * @param hItem --specified item
     * @param nImage --image index
     * @param nSelectedImage --image index in selected state
     * @return BOOL
     */
    STDMETHOD_(BOOL, SetItemImage)(THIS_ HSTREEITEM hItem, int nImage, int nSelectedImage) PURE;

    /**
     * @brief Get the custom data of the specified item
     * @param hItem --specified item
     * @return LPARAM--custom data
     */
    STDMETHOD_(LPARAM, GetItemData)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Set the custom data of the specified item
     * @param hItem --specified item
     * @param lParam --custom data
     * @return BOOL
     */
    STDMETHOD_(BOOL, SetItemData)(THIS_ HSTREEITEM hItem, LPARAM lParam) PURE;

    /**
     * @brief Query whether the specified item has child items
     * @param hItem --specified item
     * @return BOOL TRUE:has child items
     */
    STDMETHOD_(BOOL, ItemHasChildren)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Get the check state of the specified item
     * @param hItem --specified item
     * @return int 0--unchecked,1--checked,2--part checked
     */
    STDMETHOD_(int, GetCheckState)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Set the check state of the specified item
     * @param hItem --specified item
     * @param bCheck --TRUE:checked
     * @return
     */
    STDMETHOD_(BOOL, SetCheckState)(THIS_ HSTREEITEM hItem, BOOL bCheck) PURE;

    /**
     * @brief Expand or collapse the child items of the specified item
     * @param hItem --specified item
     * @param nCode --flag
     * @return
     */
    STDMETHOD_(BOOL, Expand)(THIS_ HSTREEITEM hItem, UINT nCode DEF_VAL(TVE_EXPAND)) PURE;

    /**
     * @brief Scroll the specified item into the visible area
     * @param hItem  --specified item
     * @return
     */
    STDMETHOD_(BOOL, EnsureVisible)(THIS_ HSTREEITEM hItem) PURE;
};

/** {E21767AC-6A40-45a2-9E5C-EA5CDF516C60} */
DEFINE_SGUID(IID_IHotKeyCtrl, 0xe21767ac, 0x6a40, 0x45a2, 0x9e, 0x5c, 0xea, 0x5c, 0xdf, 0x51, 0x6c, 0x60);
#undef INTERFACE
#define INTERFACE IHotKeyCtrl
DECLARE_INTERFACE_(IHotKeyCtrl, ICtrl)
{
    DECLARE_CLASS_SIID(IID_IHotKeyCtrl)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * SHotKeyCtrl::SetRule
     * @brief    Set rule
     * @param    WORD wInvalidComp --
     * @param    WORD wModifier  --
     *
     * Describe  Set rule
     */
    STDMETHOD_(void, SetRule)(THIS_ WORD wInvalidComp, WORD wModifier) PURE;

    /**
     * SHotKeyCtrl::SetHotKey
     * @brief    Set hot key
     * @param    WORD vKey --
     * @param    WORD wModifiers  --
     *
     * Describe  Set hot key
     */
    STDMETHOD_(void, SetHotKey)(THIS_ WORD wKey, WORD wModifiers) PURE;

    /**
     * SHotKeyCtrl::GetHotKey
     * @brief    Get hot key
     * @param    WORD &vKey --
     * @param    WORD wModifiers  --
     *
     * Describe  Get hot key
     */
    STDMETHOD_(void, GetHotKey)(CTHIS_ WORD * wKey, WORD * wModifers) SCONST PURE;
};

/** {7B9C2C3E-3101-4cda-9436-F88D9993BA5F} */
DEFINE_SGUID(IID_IRichEdit, 0x7b9c2c3e, 0x3101, 0x4cda, 0x94, 0x36, 0xf8, 0x8d, 0x99, 0x93, 0xba, 0x5f);
#undef INTERFACE
#define INTERFACE IRichEdit
DECLARE_INTERFACE_(IRichEdit, IPanel)
{
    DECLARE_CLASS_SIID(IID_IRichEdit)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Show/hide the scroll bar
     * @param wBar scroll bar flag, see ScrollBarID
     * @param bShow TRUE-show, FALSE-hide
     * @return
     */
    STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

    /**
     * @brief Enable/Disable the scroll bar
     * @param wBar scroll bar flag, see ScrollBarID
     * @param bShow TRUE-show, FALSE-hide
     * @return
     */
    STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

    /**
     * @brief Get the scroll bar Enable state
     * @param bVertical -- TRUE-vertical scroll bar
     * @return TRUE if enabled
     */
    STDMETHOD_(BOOL, IsScrollBarEnable)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief Set the scroll bar data
     * @param si -- scroll bar data
     * @param bVertical -- TRUE-vertical scroll bar
     * @return
     */
    STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

    /**
     * @brief Set the scroll bar position
     * @param bVertical -- TRUE-vertical scroll bar
     * @param nNewPos -- scroll position
     * @param bRedraw -- redraw flag
     * @return
     */
    STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

    /**
     * @brief Get the scroll bar position
     * @param bVertical -- TRUE-vertical scroll bar
     * @return scroll bar position
     */
    STDMETHOD_(int, GetScrollPos)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief Set the scroll bar range
     * @param bVertical -- TRUE-vertical scroll bar
     * @param nMinPos -- minimum value
     * @param nMaxPos -- maximum value
     * @param bRedraw -- redraw flag
     * @return TRUE on success
     */
    STDMETHOD_(BOOL, SetScrollRange)
    (THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

    /**
     * @brief Get the scroll bar range
     * @param bVertical -- TRUE-vertical scroll bar
     * @param lpMinPos -- minimum value
     * @param lpMaxPos -- maximum value
     * @return
     */
    STDMETHOD_(BOOL, GetScrollRange)
    (CTHIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;

    /**
     * @brief Query the scroll bar enabled state
     * @param bVertical -- TRUE-vertical scroll bar
     * @return TRUE if enabled
     */
    STDMETHOD_(BOOL, HasScrollBar)(CTHIS_ BOOL bVertical) SCONST PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * IRichEdit::SaveRtf
     * @brief    Save data to RTF file
     * @param LPCTSTR pszFileName -- file name
     * @return    DWORD data length
     *
     * Describe
     */
    STDMETHOD_(DWORD, SaveRtf)(THIS_ LPCTSTR pszFileName) PURE;

    /**
     * IRichEdit::LoadRtf
     * @brief    Load data from RTF file
     * @param LPCTSTR pszFileName -- file name
     * @return    DWORD data length
     *
     * Describe
     */
    STDMETHOD_(DWORD, LoadRtf)(THIS_ LPCTSTR pszFileName) PURE;

    /**
     * IRichEdit::SetSel
     * @brief    Set selection
     * @param     long nStartChar -- start character index
     * @param     long nEndChar -- end character index
     * @param     BOOL bNoScroll -- flag to disable scrolling
     *
     * Describe  Set selection; supports very long text
     */
    STDMETHOD_(void, SetSel)(THIS_ long nStartChar, long nEndChar, BOOL bNoScroll) PURE;
    /**
     * IRichEdit::ReplaceSel
     * @brief    Replace selection
     * @param     LPCWSTR pszText -- replacement data
     * @param     BOOL bCanUndo -- flag to allow undo
     *
     * Describe  Replace selection
     */
    STDMETHOD_(void, ReplaceSel)(THIS_ LPCTSTR pszText, BOOL bCanUndo DEF_VAL(TRUE)) PURE;
    /**
     * IRichEdit::GetWordWrap
     * @brief
     * @return   returns BOOL
     *
     * Describe
     */
    STDMETHOD_(BOOL, GetWordWrap)(CTHIS) SCONST PURE;
    /**
     * IRichEdit::SetWordWrap
     * @brief
     * @param     BOOL fWordWrap -- word wrap flag
     *
     * Describe
     */
    STDMETHOD_(void, SetWordWrap)(THIS_ BOOL fWordWrap) PURE;

    /**
     * IRichEdit::GetReadOnly
     * @brief    Check whether read-only
     * @return   returns BOOL
     *
     * Describe  Check whether read-only
     */
    STDMETHOD_(BOOL, GetReadOnly)(CTHIS) SCONST PURE;

    /**
     * IRichEdit::SetReadOnly
     * @brief    Set read-only
     * @param     BOOL bReadOnly -- whether read-only
     * @return    returns BOOL
     *
     * Describe  Set read-only
     */
    STDMETHOD_(BOOL, SetReadOnly)(THIS_ BOOL bReadOnly) PURE;

    /**
     * IRichEdit::GetLimitText
     * @brief    Get the minimum text length
     * @return   returns BOOL
     *
     * Describe  Get the minimum text length
     */
    STDMETHOD_(LONG, GetLimitText)(CTHIS) SCONST PURE;

    /**
     * IRichEdit::SetLimitText
     * @brief    Set the minimum text length
     * @param     int nLength -- length
     * @return    returns BOOL
     *
     * Describe  Set the minimum text length
     */
    STDMETHOD_(BOOL, SetLimitText)(THIS_ int nLength) PURE;

    /**
     * IRichEdit::GetDefaultAlign
     * @brief    Get alignment
     * @return   returns WORD
     *
     * Describe  Set the minimum text length
     */
    STDMETHOD_(WORD, GetDefaultAlign)(CTHIS) SCONST PURE;

    /**
     * IRichEdit::SetDefaultAlign
     * @brief    Set default alignment
     * @param     WORD wNewAlign -- alignment
     *
     * Describe  Set default alignment
     */
    STDMETHOD_(void, SetDefaultAlign)(THIS_ WORD wNewAlign) PURE;

    /**
     * IRichEdit::GetRichTextFlag
     * @brief    Get flag
     * @return   returns BOOL
     *
     * Describe  Get flag
     */
    STDMETHOD_(BOOL, GetRichTextFlag)(CTHIS) SCONST PURE;

    /**
     * IRichEdit::SetRichTextFlag
     * @brief    Set flag
     * @param     BOOL fRich -- flag
     *
     * Describe  Set flag
     */
    STDMETHOD_(void, SetRichTextFlag)(THIS_ BOOL fRich) PURE;

    /**
     * IRichEdit::SetRichTextFlag
     * @brief    Set flag
     * @param     BOOL fRich -- flag
     * @return    returns LONG
     *
     * Describe  Set flag
     */
    STDMETHOD_(LONG, GetDefaultLeftIndent)(CTHIS) SCONST PURE;

    /**
     * IRichEdit::SetDefaultLeftIndent
     * @brief    Set indent
     * @param     LONG lNewIndent -- number of indent characters
     *
     * Describe  Set indent
     */
    STDMETHOD_(void, SetDefaultLeftIndent)(THIS_ LONG lNewIndent) PURE;

    /**
     * IRichEdit::SetSaveSelection
     * @brief
     * @param     BOOL fSaveSelection
     * @return    returns BOOL
     *
     * Describe
     */
    STDMETHOD_(BOOL, SetSaveSelection)(THIS_ BOOL fSaveSelection) PURE;

    /**
     * IRichEdit::SetDefaultTextColor
     * @brief    Default text color
     * @param     COLORREF cr -- color
     * @return    returns COLORREF
     *
     * Describe  Set default text color
     */
    STDMETHOD_(COLORREF, SetDefaultTextColor)(THIS_ COLORREF cr) PURE;
};

/** {A32ED365-E6B8-4ada-BE49-BCF127F94427} */
DEFINE_SGUID(IID_ITabPage, 0xa32ed365, 0xe6b8, 0x4ada, 0xbe, 0x49, 0xbc, 0xf1, 0x27, 0xf9, 0x44, 0x27);
#undef INTERFACE
#define INTERFACE ITabPage
DECLARE_INTERFACE_(ITabPage, ICtrl)
{
    DECLARE_CLASS_SIID(IID_ITabPage)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * GetTitle
     * @brief    Get title
     * @return   LPCTSTR --- title
     *
     * Describe  Get title
     */
    STDMETHOD_(LPCTSTR, GetTitle)(CTHIS) SCONST PURE;

    /**
     * SetTitle
     * @brief    Set title
     * @param    LPCTSTR lpszTitle --- title
     *
     * Describe  Set title
     */
    STDMETHOD_(void, SetTitle)(THIS_ LPCTSTR lpszTitle) PURE;

    /**
     * GetIconIndex
     * @brief    Get icon index
     * @return   int, icon index
     */
    STDMETHOD_(int, GetIconIndex)(CTHIS) SCONST PURE;

    /**
     * SetIconIndex
     * @brief    Set icon index
     * @param    int iIcon -- icon index
     * @return   void
     */
    STDMETHOD_(void, SetIconIndex)(THIS_ int iIcon) PURE;
};

/** {17714866-88B9-480b-83D1-F044486D78D1} */
DEFINE_SGUID(IID_ITabCtrl, 0x17714866, 0x88b9, 0x480b, 0x83, 0xd1, 0xf0, 0x44, 0x48, 0x6d, 0x78, 0xd1);
#undef INTERFACE
#define INTERFACE ITabCtrl
DECLARE_INTERFACE_(ITabCtrl, ICtrl)
{
    DECLARE_CLASS_SIID(IID_ITabCtrl)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * ITabCtrl::GetCurSel
     * @brief    Get current selection
     * @return   returns int
     *
     * Describe  Get current selection
     */
    STDMETHOD_(int, GetCurSel)(CTHIS) SCONST PURE;

    /**
     * ITabCtrl::SetCurSel
     * @brief    Set current selection
     * @param    int nIndex -- index
     * @return   returns BOOL
     *
     * Describe  Get current selection
     */
    STDMETHOD_(BOOL, SetCurSel)(THIS_ int nIndex) PURE;

    /**
     * ITabCtrl::SetItemTitle
     * @brief    Set title
     * @param    int nIndex  -- index
     * @param    LPCTSTR lpszTitle  -- title
     * @return   returns BOOL
     *
     * Describe  Get current selection
     */
    STDMETHOD_(BOOL, SetItemTitle)(THIS_ int nIndex, LPCTSTR lpszTitle) PURE;

    /**
     * ITabCtrl::InsertItem
     * @brief    Insert a tab page
     * @param    LPCWSTR lpContent  -- page info described by XML
     * @param    int iInsert  -- position
     * @return   returns the insertion position
     *
     * Describe  Insert a tab page
     */
    STDMETHOD_(int, InsertItem)(THIS_ LPCWSTR lpContent, int iInsert DEF_VAL(-1)) PURE;

    /**
     * ITabCtrl::GetItemCount
     * @brief    Get the number of tab pages
     * @return   returns int
     *
     * Describe  Get the number of tab pages
     */
    STDMETHOD_(int, GetItemCount)(CTHIS) SCONST PURE;

    /**
     * ITabCtrl::GetItem
     * @brief    Get the specified tab page
     * @param    int nIndex -- index
     * @return   returns int
     *
     * Describe  Get current selection
     */
    STDMETHOD_(IWindow *, GetPage)(THIS_ int nIndex) PURE;

    /**
     * ITabCtrl::RemoveItem
     * @brief    Delete the specified tab page
     * @param    int nIndex -- index
     * @param    int nSelPage -- selected page
     * @return   Delete the specified tab page
     *
     * Describe  Get current selection
     */
    STDMETHOD_(BOOL, RemoveItem)(THIS_ int nIndex, int iSelPage DEF_VAL(0)) PURE;

    /**
     * ITabCtrl::RemoveAllItems
     * @brief    Delete all pages
     *
     * Describe  Delete all pages
     */
    STDMETHOD_(void, RemoveAllItems)(THIS) PURE;

    /**
     * ITabCtrl::GetPageIndex
     * @brief    Get the index of the specified page
     * @param    LPCTSTR pszName -- query string
     * @param    BOOL bTitle --
     TRUE:pszName represents the page's title attribute, FALSE:pszName represents the page's name attribute
     * @return   found page index
     *
     * Describe
     */
    STDMETHOD_(int, GetPageIndex)(THIS_ LPCTSTR pszName, BOOL bTitle) PURE;
};

/** {71CCB156-39DD-4eee-842C-C8363368AF30} */
DEFINE_SGUID(IID_IEdit, 0x71ccb156, 0x39dd, 0x4eee, 0x84, 0x2c, 0xc8, 0x36, 0x33, 0x68, 0xaf, 0x30);
#undef INTERFACE
#define INTERFACE IEdit
DECLARE_INTERFACE_(IEdit, ICtrl)
{
    DECLARE_CLASS_SIID(IID_IEdit)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Get the hint string
     * @param[out] IStringT *pStr--hint string
     * @return void
     * @remark Set via SetAttribute
     */
    STDMETHOD_(void, GetCueText)(CTHIS_ IStringT * pStr) SCONST PURE;

    /**
     * @brief Get the hint string color
     * @return COLORREF--hint string color
     * @remark Set via SetAttribute
     */
    STDMETHOD_(COLORREF, GetCueColor)(CTHIS) SCONST PURE;
};

/** {7F8DF81C-6C6E-4572-BDE6-0757A4D07C5F} */
DEFINE_SGUID(IID_ISpinButtonCtrl, 0x7f8df81c, 0x6c6e, 0x4572, 0xbd, 0xe6, 0x7, 0x57, 0xa4, 0xd0, 0x7c, 0x5f);
#undef INTERFACE
#define INTERFACE ISpinButtonCtrl
DECLARE_INTERFACE_(ISpinButtonCtrl, ICtrl)
{
    DECLARE_CLASS_SIID(IID_ISpinButtonCtrl)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Set value
     * @param nValue --new value
     * @return
     */
    STDMETHOD_(void, SetValue)(THIS_ int nValue) PURE;

    /**
     * @brief Set range
     * @param nMin --minimum value
     * @param nMax --maximum value
     * @return
     */
    STDMETHOD_(void, SetRange)(THIS_ int nMin, int nMax) PURE;

    /**
     * @brief Set step
     * @param nStep --step
     * @return
     */
    STDMETHOD_(void, SetStep)(THIS_ UINT nStep) PURE;

    /**
     * @brief Get value
     * @return int --current value
     */
    STDMETHOD_(int, GetValue)(CTHIS) SCONST PURE;

    /**
     * @brief Get the associated window object
     * @return IWindow *--associated window object
     */
    STDMETHOD_(IWindow *, GetIBuddy)(CTHIS) SCONST PURE;

    /**
     * @brief Set the associated window object
     * @param pBuddy --associated window object
     * @return void
     */
    STDMETHOD_(void, SetBuddy)(THIS_ IWindow * pBuddy) PURE;
};

/** {80E930E7-BFC2-4e5e-8FFC-A2F0B4EC24E9} */
DEFINE_SGUID(IID_IIconWnd, 0x80e930e7, 0xbfc2, 0x4e5e, 0x8f, 0xfc, 0xa2, 0xf0, 0xb4, 0xec, 0x24, 0xe9);
#undef INTERFACE
#define INTERFACE IIconWnd
DECLARE_INTERFACE_(IIconWnd, ICtrl)
{
    DECLARE_CLASS_SIID(IID_IIconWnd)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief    SetIcon
     * @param    HICON hIcon -- icon resource handle
     * @return   void
     *
     * Describe  Icon resource
     */
    STDMETHOD_(void, SetIcon)(THIS_ HICON hIcon) PURE;
};

/** {EDFF5B4F-8BF0-46fb-9399-C91859D5A1BC} */
DEFINE_SGUID(IID_IRealWnd, 0xedff5b4f, 0x8bf0, 0x46fb, 0x93, 0x99, 0xc9, 0x18, 0x59, 0xd5, 0xa1, 0xbc);
#undef INTERFACE
#define INTERFACE IRealWnd
DECLARE_INTERFACE_(IRealWnd, ICtrl)
{
    DECLARE_CLASS_SIID(IID_IRealWnd)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    STDMETHOD_(const IStringT *, GetRealClassName)(CTHIS) SCONST PURE;

    STDMETHOD_(const IStringT *, GetRealWindowName)(CTHIS) SCONST PURE;

    STDMETHOD_(DWORD, GetRealStyle)(CTHIS) SCONST PURE;

    STDMETHOD_(DWORD, GetRealStyleEx)(CTHIS) SCONST PURE;

    STDMETHOD_(IXmlNode *, GetRealParam)(CTHIS) PURE;

    /**
     * SRealWnd::GetRealHwnd
     * @brief    Get the window handle
     * @param    BOOL bAutoCreate -- auto create
     * @return   returns HWND
     *
     * Describe  Get the window handle
     */
    STDMETHOD_(HWND, GetRealHwnd)(THIS_ BOOL bAutoCreate DEF_VAL(TRUE)) PURE;

    /**
     * SRealWnd::SetData
     * @brief    Get additional data
     * @param    LPVOID lpData -- additional data
     *
     * Describe  Get additional data
     */
    STDMETHOD_(void, SetData)(THIS_ LPVOID lpData) PURE;

    /**
     * SRealWnd::GetData
     * @brief    Get additional data
     * @return   returns LPVOID
     *
     * Describe  Get additional data
     */
    STDMETHOD_(LPVOID, GetData)(THIS) PURE;
};

typedef enum _StackViewAniStyle
{
    kAniNone = 0,
    kFadeInOut,
    kMoveInOut,
    kPushInOut
} StackViewAniStyle;

/** {59D1145E-C906-404e-8128-29C7CC80E3DC} */
DEFINE_SGUID(IID_IStackView, 0x59d1145e, 0xc906, 0x404e, 0x81, 0x28, 0x29, 0xc7, 0xcc, 0x80, 0xe3, 0xdc);
#undef INTERFACE
#define INTERFACE IStackView
DECLARE_INTERFACE_(IStackView, ICtrl)
{
    DECLARE_CLASS_SIID(IID_IStackView)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    STDMETHOD_(BOOL, SelectPage)(THIS_ int iView, BOOL enableAnimate DEF_VAL(TRUE)) PURE;
    STDMETHOD_(void, SetAniStyle)(THIS_ StackViewAniStyle aniStyle) PURE;
    STDMETHOD_(void, SetAniDir)(THIS_ BOOL bVert) PURE;
    STDMETHOD_(IWindow *, GetSelPage)(CTHIS) SCONST PURE;
    STDMETHOD_(IWindow *, GetPage)(CTHIS_ int iPage) SCONST PURE;
};

/** {59D1145E-C906-404e-8128-29C7CC80E3DD} */
DEFINE_SGUID(IID_ISearchDropdownList, 0x59d1145e, 0xc906, 0x404e, 0x81, 0x28, 0x29, 0xc7, 0xcc, 0x80, 0xe3, 0xdd);
#undef INTERFACE
#define INTERFACE ISearchDropdownList
DECLARE_INTERFACE_(ISearchDropdownList, ICtrl)
{
    DECLARE_CLASS_SIID(IID_ISearchDropdownList)
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Show the dropdown box
     * @param pText display text
     * @return void
     */
    STDMETHOD_(void, DropDown)(THIS_ const IStringT *pText) PURE;

    /**
     * @brief Close the dropdown box
     * @param code end code
     * @return void
     */
    STDMETHOD_(void, CloseUp)(THIS_ int code DEF_VAL(IDCANCEL)) PURE;

    /**
     * @brief Test whether it is a dropdown box
     * @return BOOL
     */
    STDMETHOD_(BOOL, IsDropdown)(THIS) SCONST PURE;
};

SNSEND
#endif /**< __SCTRLS_I__H__ */