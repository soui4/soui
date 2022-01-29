/************************************************************************/
/*   这里定义控件操作接口,所有接口都从IObjRef继承,并且指定自己的接口ID  */
/************************************************************************/
#pragma once
#include <interface/SWindow-i.h>
#include <interface/SSkinobj-i.h>
#include <interface/SRender-i.h>
#include <interface/sstring-i.h>
#include <interface/SAdapter-i.h>
#include <interface/SListViewItemLocator-i.h>
#include <interface/STreeViewItemLocator-i.h>
#include <interface/STileViewItemLocator-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE ICtrl
DECLARE_INTERFACE_(ICtrl, IObjRef)
{
	STDMETHOD_(long,AddRef) (THIS) PURE;
	STDMETHOD_(long,Release) (THIS) PURE;
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
};

#undef INTERFACE
#define INTERFACE IItemPanel
DECLARE_INTERFACE_IID_(IItemPanel, ICtrl, "176CDF98-260E-4070-91B5-E6E163F909A4")
{
	STDMETHOD_(long,AddRef) (THIS) PURE;
	STDMETHOD_(long,Release) (THIS) PURE;
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
	STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
	//////////////////////////////////////////////////////////////////////////
    STDMETHOD_(LPARAM, GetItemIndex)(THIS) SCONST PURE;
    STDMETHOD_(void, SetSkin)(THIS_ ISkinObj * pSkin) PURE;
    STDMETHOD_(void, SetColor)(THIS_ COLORREF crBk, COLORREF crSelBk) PURE;
    STDMETHOD_(void, SetItemData)(THIS_ LPARAM dwData) PURE;
    STDMETHOD_(LPARAM, GetItemData)(THIS) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IImageWnd
DECLARE_INTERFACE_IID_(IImageWnd, ICtrl, "42148B05-6223-46f6-B903-D42D84038546")
{
	STDMETHOD_(long,AddRef) (THIS) PURE;
	STDMETHOD_(long,Release) (THIS) PURE;
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
	STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
	//////////////////////////////////////////////////////////////////////////

    STDMETHOD_(BOOL, SetSkin)
    (THIS_ ISkinObj * pSkin, int iFrame /*= 0*/, BOOL bAutoFree /*= TRUE*/) PURE;

    /**
     * SImageWnd::SetImage
     * @param    IBitmap * pBitmap -- 图片对象
     * @param    FilterLevel fl -- FilterLevel
     * @return   void
     *
     * Describe  设置绘制图片
     */
    STDMETHOD_(void, SetImage)
    (THIS_ IBitmap * pBitmap, FilterLevel fl /*= kNone_FilterLevel*/) PURE;

    STDMETHOD_(IBitmap *, GetImage)(THIS) PURE;

    /**
     * SImageWnd::SetIcon
     * @param    int nSubID -- 资源ID
     * @brief    设置图标
     * @return   返回值BOOL 成功--TRUE 失败--FALSE
     *
     * Describe  设置图标
     */
    STDMETHOD_(BOOL, SetIcon)(THIS_ int nSubID) PURE;

    /**
     * SImageWnd::GetSkin
     * @brief    获取资源
     * @return   返回值ISkinObj指针
     *
     * Describe  获取资源
     */
    STDMETHOD_(ISkinObj *, GetSkin)(THIS) PURE;
};

#undef INTERFACE
#define INTERFACE IAnimateImgWnd
DECLARE_INTERFACE_IID_(IAnimateImgWnd, ICtrl, "374A5086-AD38-4f15-83E0-002822E2595A")
{
	STDMETHOD_(long,AddRef) (THIS) PURE;
	STDMETHOD_(long,Release) (THIS) PURE;
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
	STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
	//////////////////////////////////////////////////////////////////////////
    /**
     * SAnimateImgWnd::Start
     * @brief    启动动画
     *
     * Describe  启动动画
     */
    STDMETHOD_(void, Start)(THIS) PURE;
    /**
     * SAnimateImgWnd::Stop
     * @brief    停止动画
     *
     * Describe  停止动画
     */
    STDMETHOD_(void, Stop)(THIS) PURE;

    /**
     * SAnimateImgWnd::IsPlaying
     * @brief    判断动画运行状态
     * @return   返回值是动画状态 TRUE -- 运行中
     *
     * Describe  判断动画运行状态
     */
    STDMETHOD_(BOOL, IsPlaying)(THIS) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IProgress
DECLARE_INTERFACE_IID_(IProgress, ICtrl, "77407E2D-582B-4ef2-A33A-427C933BAA8C")
{
	STDMETHOD_(long,AddRef) (THIS) PURE;
	STDMETHOD_(long,Release) (THIS) PURE;
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
	STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
	//////////////////////////////////////////////////////////////////////////
    /**
     * SProgress::SetValue
     * @brief    设置进度条进度值
     * @param    int nValue  --  进度值
     * @return   返回值是 TRUE -- 设置成功
     *
     * Describe  设置进度条进度值
     */
    STDMETHOD_(BOOL, SetValue)(THIS_ int nValue) PURE;
    /**
     * SProgress::GetValue
     * @brief    获取进度值
     * @return   返回值是int
     *
     * Describe  获取进度值
     */
    STDMETHOD_(int, GetValue)(THIS) SCONST PURE;

    /**
     * SProgress::SetRange
     * @param    int nMin  --  进度最小值
     * @param    int nMax  --  进度最大值
     * @brief    设置进度值最小大值
     *
     * Describe  设置进度值
     */
    STDMETHOD_(void, SetRange)(THIS_ int nMin, int nMax) PURE;
    /**
     * SProgress::GetRange
     * @param    int nMin  --  进度最小值
     * @param    int nMax  --  进度最大值
     * @brief    获取进度值最小大值
     *
     * Describe  获取进度值
     */
    STDMETHOD_(void, GetRange)(THIS_ int *pMin, int *pMax) SCONST PURE;
    /**
     * SProgress::IsVertical
     * @brief    判断进度条是否为竖直状态
     * @return   返回值是 TRUE -- 竖直状态
     *
     * Describe  获取进度值
     */
    STDMETHOD_(BOOL, IsVertical)(THIS) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IPanel
DECLARE_INTERFACE_IID_(IPanel, ICtrl, "B1A97BB7-64BE-408f-AC7C-2197CC2F4DD0")
{
	STDMETHOD_(long,AddRef) (THIS) PURE;
	STDMETHOD_(long,Release) (THIS) PURE;
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
	STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
	//////////////////////////////////////////////////////////////////////////
    STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

    STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

    STDMETHOD_(BOOL, IsScrollBarEnable)(THIS_ BOOL bVertical) SCONST PURE;

    STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

    STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

    STDMETHOD_(int, GetScrollPos)(THIS_ BOOL bVertical) SCONST PURE;

    STDMETHOD_(BOOL, SetScrollRange)
    (THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

    STDMETHOD_(BOOL, GetScrollRange)
    (THIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;

    STDMETHOD_(BOOL, HasScrollBar)(THIS_ BOOL bVertical) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IScrollView
DECLARE_INTERFACE_IID_(IScrollView, IPanel, "49B024D6-221D-42d4-902B-AFAAC5AFFE41")
{
	STDMETHOD_(long,AddRef) (THIS) PURE;
	STDMETHOD_(long,Release) (THIS) PURE;
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
	STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
	//////////////////////////////////////////////////////////////////////////
	STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

	STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

	STDMETHOD_(BOOL, IsScrollBarEnable)(THIS_ BOOL bVertical) SCONST PURE;

	STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

	STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

	STDMETHOD_(int, GetScrollPos)(THIS_ BOOL bVertical) SCONST PURE;

	STDMETHOD_(BOOL, SetScrollRange)
		(THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

	STDMETHOD_(BOOL, GetScrollRange)
		(THIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;

	STDMETHOD_(BOOL, HasScrollBar)(THIS_ BOOL bVertical) SCONST PURE;
	//////////////////////////////////////////////////////////////////////////

    STDMETHOD_(SIZE, GetViewSize)(THIS) SCONST PURE;

    STDMETHOD_(void, SetViewSize)(THIS_ SIZE szView) PURE;

    STDMETHOD_(POINT, GetViewOrigin)(THIS) SCONST PURE;

    STDMETHOD_(void, SetViewOrigin)(THIS_ POINT pt) PURE;
};

enum
{
    SHDI_WIDTH = 0x0001,
    SHDI_TEXT = 0x0002,
    SHDI_FORMAT = 0x0004,
    SHDI_LPARAM = 0x0008,
    SHDI_ORDER = 0x0010,
    SHDI_VISIBLE = 0x0020,
    SHDI_WEIGHT = 0x0040,
    SHDI_ALL = 0xffff,
};

/**
 * @struct    _SHDITEM
 * @brief     列表头项
 *
 * Describe   列表头项
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
}SHDITEM, * LPSHDITEM;

#undef INTERFACE
#define INTERFACE IHeaderCtrl
DECLARE_INTERFACE_IID_(IHeaderCtrl, ICtrl, "60A8788F-C775-424f-AA82-DCA9CECE3D45")
{
	STDMETHOD_(long,AddRef) (THIS) PURE;
	STDMETHOD_(long,Release) (THIS) PURE;
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
	STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
	//////////////////////////////////////////////////////////////////////////

    /**
     * SHeaderCtrl::InsertItem
     * @brief    插入新项
     * @param    int iItem --  新项索引
     * @param    LPCTSTR pszText  --  新项标题
     * @param    int nWidth  -- 宽度
     * @param    BOOL bDpiAware -- dpi aware
     * @param    UINT fmt -- format flag
     * @param    LPARAM lParam -- 附加参数
     * @return   返回int
     *
     * Describe  插入新项
     */
    STDMETHOD_(int, InsertItem)
    (THIS_ int iItem, LPCTSTR pszText, int nWidth, UINT fmt, LPARAM lParam,
     BOOL bDpiAware /*=FALSE*/, float fWeight /*=0.0f*/) PURE;

    /**
     * SHeaderCtrl::GetItem
     * @brief    获得新项
     * @param    int iItem  --  索引
     * @param    SHDITEM *pItem  -- 返回列表项结构
     * @return   返回BOOL
     *
     * Describe  获得新项
     */
    STDMETHOD_(BOOL, GetItem)(THIS_ int iItem, SHDITEM *pItem) SCONST PURE;

    STDMETHOD_(BOOL, SetItem)(THIS_ int iItem, const SHDITEM *pItem) PURE;

    /**
     * SHeaderCtrl::GetItemCount
     * @brief    获取列表项个数
     * @return   返回int
     *
     * Describe  获取列表项个数
     */
    STDMETHOD_(UINT, GetItemCount)(THIS) SCONST PURE;
    /**
     * SHeaderCtrl::GetTotalWidth
     * @brief    获得所有宽度
     * @return   返回int
     *
     * Describe  获得所有宽度
     */
    STDMETHOD_(int, GetTotalWidth)(THIS_ BOOL bMinWid /*= FALSE*/) SCONST PURE;

    STDMETHOD_(BOOL, IsAutoResize)(THIS) SCONST PURE;

    /**
     * SHeaderCtrl::GetItemWidth
     * @brief    获得指定项宽度
     * @param    int iItem  --  索引
     * @return   返回int
     *
     * Describe  获得新项
     */
    STDMETHOD_(int, GetItemWidth)(THIS_ int iItem) SCONST PURE;

    /**
     * SHeaderCtrl::DeleteItem
     * @brief    删除指定项
     * @param    int iItem  --  索引
     * @return   返回BOOL
     *
     * Describe  删除指定项
     */
    STDMETHOD_(BOOL, DeleteItem)(THIS_ int iItem) PURE;

    /**
     * SHeaderCtrl::DeleteAllItems
     * @brief    删除所有项
     *
     * Describe  获得新项
     */
    STDMETHOD_(void, DeleteAllItems)(THIS) PURE;

    STDMETHOD_(void, SetItemSort)(THIS_ int iItem, UINT sortFlag) PURE;

    STDMETHOD_(void, SetItemVisible)(THIS_ int iItem, BOOL visible) PURE;

    STDMETHOD_(BOOL, IsItemVisible)(THIS_ int iItem) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IListView
DECLARE_INTERFACE_IID_(IListView, IPanel, "E584B16A-6BDB-4afb-8BCC-3A1ABACD2FE2")
{
	STDMETHOD_(long,AddRef) (THIS) PURE;
	STDMETHOD_(long,Release) (THIS) PURE;
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
	STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
	//////////////////////////////////////////////////////////////////////////
	STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

	STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

	STDMETHOD_(BOOL, IsScrollBarEnable)(THIS_ BOOL bVertical) SCONST PURE;

	STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

	STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

	STDMETHOD_(int, GetScrollPos)(THIS_ BOOL bVertical) SCONST PURE;

	STDMETHOD_(BOOL, SetScrollRange)
		(THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

	STDMETHOD_(BOOL, GetScrollRange)
		(THIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;
	STDMETHOD_(BOOL, HasScrollBar)(THIS_ BOOL bVertical) SCONST PURE;

	//////////////////////////////////////////////////////////////////////////
    STDMETHOD_(BOOL, SetAdapter)(THIS_ ILvAdapter * adapter) PURE;

    STDMETHOD_(ILvAdapter *, GetAdapter)(THIS) SCONST PURE;

    STDMETHOD_(IListViewItemLocator *, GetItemLocator)(THIS) SCONST PURE;

    STDMETHOD_(void, SetItemLocator)(THIS_ IListViewItemLocator * pItemLocator) PURE;

    STDMETHOD_(void, EnsureVisible)(THIS_ int iItem) PURE;

    STDMETHOD_(void, SetSel)(THIS_ int iItem, BOOL bNotify /*= FALSE*/) PURE;

    STDMETHOD_(int, GetSel)(THIS) SCONST PURE;

    STDMETHOD_(IItemPanel *, HitTest)(THIS_ const POINT *pt) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IMcListView
DECLARE_INTERFACE_IID_(IMcListView, IPanel, "E584B16A-6BDB-4afb-8BCC-3A1ABACD2FE2")
{
	STDMETHOD_(long,AddRef) (THIS) PURE;
	STDMETHOD_(long,Release) (THIS) PURE;
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
	STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
	//////////////////////////////////////////////////////////////////////////
	STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

	STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

	STDMETHOD_(BOOL, IsScrollBarEnable)(THIS_ BOOL bVertical) SCONST PURE;

	STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

	STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

	STDMETHOD_(int, GetScrollPos)(THIS_ BOOL bVertical) SCONST PURE;

	STDMETHOD_(BOOL, SetScrollRange)
		(THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

	STDMETHOD_(BOOL, GetScrollRange)
		(THIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;
	STDMETHOD_(BOOL, HasScrollBar)(THIS_ BOOL bVertical) SCONST PURE;
	//////////////////////////////////////////////////////////////////////////
    STDMETHOD_(BOOL, SetAdapter)(THIS_ IMcAdapter * adapter) PURE;

    STDMETHOD_(IMcAdapter *, GetAdapter)(THIS) SCONST PURE;

    STDMETHOD_(IListViewItemLocator *, GetItemLocator)(THIS) SCONST PURE;

    STDMETHOD_(void, SetItemLocator)(THIS_ IListViewItemLocator * pItemLocator) PURE;

    STDMETHOD_(void, EnsureVisible)(THIS_ int iItem) PURE;

    STDMETHOD_(void, SetSel)(THIS_ int iItem, BOOL bNotify /*= FALSE*/) PURE;

    STDMETHOD_(int, GetSel)(THIS) SCONST PURE;

    STDMETHOD_(IItemPanel *, HitTest)(THIS_ const POINT *pt) SCONST PURE;

    STDMETHOD_(IHeaderCtrl *, GetIHeaderCtrl)(THIS) SCONST PURE;

    STDMETHOD_(int, InsertColumn)
    (THIS_ int nIndex, LPCTSTR pszText, int nWidth, UINT fmt, LPARAM lParam /*= 0*/,
     BOOL bDpiAware /*=TRUE*/, float fWeight /*=0.0f*/) PURE;

    STDMETHOD_(void, DeleteColumn)(THIS_ int iCol) PURE;

    STDMETHOD_(int, GetColumnCount)(THIS) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE ITreeView
DECLARE_INTERFACE_IID_(ITreeView, IPanel, "4FB8BF5D-950C-4f05-861C-FBEB119E4C2B")
{
	STDMETHOD_(long,AddRef) (THIS) PURE;
	STDMETHOD_(long,Release) (THIS) PURE;
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
	STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
	//////////////////////////////////////////////////////////////////////////
	STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

	STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

	STDMETHOD_(BOOL, IsScrollBarEnable)(THIS_ BOOL bVertical) SCONST PURE;

	STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

	STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

	STDMETHOD_(int, GetScrollPos)(THIS_ BOOL bVertical) SCONST PURE;

	STDMETHOD_(BOOL, SetScrollRange)
		(THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

	STDMETHOD_(BOOL, GetScrollRange)
		(THIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;
	STDMETHOD_(BOOL, HasScrollBar)(THIS_ BOOL bVertical) SCONST PURE;
	//////////////////////////////////////////////////////////////////////////
    STDMETHOD_(BOOL, SetAdapter)(THIS_ ITvAdapter * adapter) PURE;

    STDMETHOD_(ITvAdapter *, GetAdapter)(THIS) SCONST PURE;

    STDMETHOD_(void, SetItemLocator)(THIS_ ITreeViewItemLocator * pItemLocator) PURE;

    STDMETHOD_(ITreeViewItemLocator *, GetItemLocator)(THIS) SCONST PURE;

    STDMETHOD_(void, SetSel)(THIS_ HSTREEITEM hItem, BOOL bNotify /*= FALSE*/) PURE;

    STDMETHOD_(HSTREEITEM, GetSel)(THIS) SCONST PURE;

    STDMETHOD_(void, EnsureVisible)(THIS_ HSTREEITEM hItem) PURE;

    STDMETHOD_(IItemPanel *, HitTest)(THIS_ const POINT *pt) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE ITileView
DECLARE_INTERFACE_IID_(ITileView, IPanel, "3718C98E-9177-4afb-986F-94B03A78F2C3")
{
	STDMETHOD_(long,AddRef) (THIS) PURE;
	STDMETHOD_(long,Release) (THIS) PURE;
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
	STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
	//////////////////////////////////////////////////////////////////////////
	STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

	STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

	STDMETHOD_(BOOL, IsScrollBarEnable)(THIS_ BOOL bVertical) SCONST PURE;

	STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

	STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

	STDMETHOD_(int, GetScrollPos)(THIS_ BOOL bVertical) SCONST PURE;

	STDMETHOD_(BOOL, SetScrollRange)
		(THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

	STDMETHOD_(BOOL, GetScrollRange)
		(THIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;
	STDMETHOD_(BOOL, HasScrollBar)(THIS_ BOOL bVertical) SCONST PURE;
	//////////////////////////////////////////////////////////////////////////
    STDMETHOD_(BOOL, SetAdapter)(THIS_ ILvAdapter * adapter) PURE;

    STDMETHOD_(ILvAdapter *, GetAdapter)(THIS) SCONST PURE;

    STDMETHOD_(void, SetItemLocator)(THIS_ ITileViewItemLocator * pItemLocator) PURE;

    STDMETHOD_(ITileViewItemLocator *, GetItemLocator)(THIS) SCONST PURE;

    STDMETHOD_(void, SetSel)(THIS_ int iItem, BOOL bNotify /*= FALSE*/) PURE;

    STDMETHOD_(int, GetSel)(THIS) SCONST PURE;

    STDMETHOD_(void, EnsureVisible)(THIS_ int iItem) PURE;

    STDMETHOD_(IItemPanel *, HitTest)(THIS_ const POINT *pt) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IListBox
DECLARE_INTERFACE_IID_(IListBox, IPanel, "4A36DC8A-7378-4a2d-A3AF-D04B0712ACCD")
{
	STDMETHOD_(long,AddRef) (THIS) PURE;
	STDMETHOD_(long,Release) (THIS) PURE;
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
	STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
	//////////////////////////////////////////////////////////////////////////
	STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

	STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

	STDMETHOD_(BOOL, IsScrollBarEnable)(THIS_ BOOL bVertical) SCONST PURE;

	STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

	STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

	STDMETHOD_(int, GetScrollPos)(THIS_ BOOL bVertical) SCONST PURE;

	STDMETHOD_(BOOL, SetScrollRange)
		(THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

	STDMETHOD_(BOOL, GetScrollRange)
		(THIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;
	STDMETHOD_(BOOL, HasScrollBar)(THIS_ BOOL bVertical) SCONST PURE;
	//////////////////////////////////////////////////////////////////////////
    /**
     * SListBox::GetCount
     * @brief    获取项个数
     * @return   返回int
     *
     * Describe  获取项个数
     */
    STDMETHOD_(int, GetCount)(THIS) SCONST PURE;

    /**
     * SListBox::GetCurSel
     * @brief    获取当前选中项索引
     * @return   返回int
     *
     * Describe  获取当前选中项索引
     */
    STDMETHOD_(int, GetCurSel)(THIS) SCONST PURE;

    /**
     * SListBox::SetCurSel
     * @brief    设置选中项
     * @param    int nIndex -- 待选中索引
     *
     * Describe  设置选中项
     */
    STDMETHOD_(BOOL, SetCurSel)(THIS_ int nIndex, BOOL bNotifyChange /* = FALSE*/) PURE;

    /**
     * SListBox::GetTopIndex
     * @brief    获取第一条可见记录索引
     * @return   返回int
     *
     * Describe
     */
    STDMETHOD_(int, GetTopIndex)(THIS) SCONST PURE;

    /**
     * SListBox::SetTopIndex
     * @brief    设置第一条可见记录索引
     * @param    int nIndex -- 第一条可见记录索引
     *
     * Describe
     */
    STDMETHOD_(BOOL, SetTopIndex)(THIS_ int nIndex) PURE;

    /**
     * SListBox::GetItemHeight
     * @brief    获取高度
     * @return   返回int
     *
     * Describe  获取高度
     */
    STDMETHOD_(int, GetItemHeight)(THIS) SCONST PURE;

    /**
     * SListBox::SetItemHeight
     * @brief    设置指定项高度
     * @param    int cyItemHeight -- 高度
     * @return   返回
     *
     * Describe  设置指定项高度
     */
    STDMETHOD_(void, SetItemHeight)(THIS_ int nItemHeight) PURE;

    /**
     * SListBox::GetItemData
     * @brief    获取附加数据
     * @param    int nIndex -- 选项索引
     * @return   返回LPARAM
     *
     * Describe  获取附加数据
     */
    STDMETHOD_(LPARAM, GetItemData)(THIS_ int nIndex) SCONST PURE;

    /**
     * SListBox::SetItemData
     * @brief    设置选项附加数据
     * @param    int nIndex -- 选项索引
     * @param    LPARAM lParam  -- 附加数据
     * @return   返回BOOL
     *
     * Describe  设置选项附加数据
     */
    STDMETHOD_(BOOL, SetItemData)(THIS_ int nIndex, LPARAM lParam) PURE;

    /**
     * SListBox::GetText
     * @brief    获取指定项文本
     * @param    int nIndex -- 选项索引
     * @param    BOOL bRawText -- 原始数据标志
     * @return   SStringT,列表项的原始字符串
     *
     * Describe  获取指定项文本
     */
    STDMETHOD_(BOOL, GetIText)
    (THIS_ int nIndex, BOOL bRawText /*= FALSE*/, IStringT *str) SCONST PURE;

    /**
     * SListBox::DeleteAll
     * @brief    删除所有
     *
     * Describe  删除所有
     */
    STDMETHOD_(void, DeleteAll)(THIS) PURE;

    /**
     * SListBox::DeleteString
     * @brief    设置指定项文本
     * @param    int nIndex -- 选项索引
     * @return   返回BOOL
     *
     * Describe  设置指定项文本
     */
    STDMETHOD_(BOOL, DeleteString)(THIS_ int nIndex) PURE;

    /**
     * SListBox::AddString
     * @brief    添加文本
     * @param    LPCTSTR lpszItem -- 文本
     * @param    int nImage -- 图标
     * @param    LPARAM lParam -- 附加数据
     * @return   返回int 索引
     *
     * Describe  添加文本
     */
    STDMETHOD_(int, AddString)
    (THIS_ LPCTSTR lpszItem, int nImage /*= -1*/, LPARAM lParam /*= 0*/) PURE;

    /**
     * SListBox::InsertString
     * @brief    插入文本
     * @param    int nIndex  -- 索引
     * @param    LPCTSTR lpszItem -- 文本
     * @param    int nImage -- 图标
     * @param    LPARAM lParam -- 附加数据
     * @return   返回int 索引
     *
     * Describe  在某项插入文本
     */
    STDMETHOD_(int, InsertString)
    (THIS_ int nIndex, LPCTSTR lpszItem, int nImage /*= -1*/, LPARAM lParam /*= 0*/) PURE;

    /**
     * SListBox::EnsureVisible
     * @brief    设置某项显示
     * @param    int nIndex  -- 索引
     *
     * Describe  设置某项显示
     */
    STDMETHOD_(void, EnsureVisible)(THIS_ int nIndex) PURE;

    STDMETHOD_(int, FindString)(THIS_ int iFindAfter, LPCTSTR pszText) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IComboBase
DECLARE_INTERFACE_IID_(IComboBase, ICtrl, "2BF10693-BEBD-4497-B6B5-5380BA8401EC")
{
	STDMETHOD_(long,AddRef) (THIS) PURE;
	STDMETHOD_(long,Release) (THIS) PURE;
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
	STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
	//////////////////////////////////////////////////////////////////////////
    /**
     * SComboBoxBase::GetCurSel
     * @brief    获取选中值索引
     * @return   返回int
     *
     * Describe  获取当前选中索引
     */
    STDMETHOD_(int, GetCurSel)(THIS) SCONST PURE;

    /**
     * SComboBoxBase::GetCount
     * @brief    获取下拉项个数
     * @return   返回int
     *
     * Describe  获取下拉项个数
     */
    STDMETHOD_(int, GetCount)(THIS) SCONST PURE;

    /**
     * SComboBoxBase::SetCurSel
     * @brief    设置当前选中
     * @param    int iSel -- 选中索引
     *
     * Describe  设置当前选中
     */
    STDMETHOD_(BOOL, SetCurSel)(THIS_ int iSel) PURE;

    STDMETHOD_(BOOL, GetItemText)(THIS_ int iItem, BOOL bRawText, IStringT *str) SCONST PURE;

    /**
     * FindString
     * @brief    查找字符串位置
     * @param    LPCTSTR pszFind --  查找目标
     * @param    int nAfter --  开始位置
     * @return   int -- 目标索引，失败返回-1。
     * Describe
     */
    STDMETHOD_(int, FindString)
    (THIS_ LPCTSTR pszFind, int nAfter /* = -1*/, BOOL bPartMatch /* = TRUE*/) PURE;

    /**
     * SComboBoxBase::DropDown
     * @brief    下拉事件
     *
     * Describe  下拉事件
     */
    STDMETHOD_(void, DropDown)(THIS) PURE;

    /**
     * SComboBoxBase::CloseUp
     * @brief    下拉关闭
     *
     * Describe  下拉关闭
     */
    STDMETHOD_(void, CloseUp)(THIS) PURE;

    STDMETHOD_(BOOL, IsDropdown)(THIS) SCONST PURE;

    STDMETHOD_(void, SetDropdown)(THIS_ BOOL bDropdown) PURE;
};

#undef INTERFACE
#define INTERFACE IComboBox
DECLARE_INTERFACE_IID_(IComboBox, IComboBase, "AC6C72BB-51BE-4216-ADA5-B394283DC9CF")
{
	STDMETHOD_(long,AddRef) (THIS) PURE;
	STDMETHOD_(long,Release) (THIS) PURE;
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
	STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
	//////////////////////////////////////////////////////////////////////////
    /**
     * SComboBoxBase::GetCurSel
     * @brief    获取选中值索引
     * @return   返回int
     *
     * Describe  获取当前选中索引
     */
    STDMETHOD_(int, GetCurSel)(THIS) SCONST PURE;

    /**
     * SComboBoxBase::GetCount
     * @brief    获取下拉项个数
     * @return   返回int
     *
     * Describe  获取下拉项个数
     */
    STDMETHOD_(int, GetCount)(THIS) SCONST PURE;

    /**
     * SComboBoxBase::SetCurSel
     * @brief    设置当前选中
     * @param    int iSel -- 选中索引
     *
     * Describe  设置当前选中
     */
    STDMETHOD_(BOOL, SetCurSel)(THIS_ int iSel) PURE;

    STDMETHOD_(BOOL, GetItemText)(THIS_ int iItem, BOOL bRawText, IStringT *str) SCONST PURE;

    /**
     * FindString
     * @brief    查找字符串位置
     * @param    LPCTSTR pszFind --  查找目标
     * @param    int nAfter --  开始位置
     * @return   int -- 目标索引，失败返回-1。
     * Describe
     */
    STDMETHOD_(int, FindString)
    (THIS_ LPCTSTR pszFind, int nAfter /* = -1*/, BOOL bPartMatch /* = TRUE*/) PURE;

    /**
     * SComboBoxBase::DropDown
     * @brief    下拉事件
     *
     * Describe  下拉事件
     */
    STDMETHOD_(void, DropDown)(THIS) PURE;

    /**
     * SComboBoxBase::CloseUp
     * @brief    下拉关闭
     *
     * Describe  下拉关闭
     */
    STDMETHOD_(void, CloseUp)(THIS) PURE;

    STDMETHOD_(BOOL, IsDropdown)(THIS) SCONST PURE;

    STDMETHOD_(void, SetDropdown)(THIS_ BOOL bDropdown) PURE;
	//////////////////////////////////////////////////////////////////////////

    /**
     * SComboBox::GetItemData
     * @brief    获取附加数据
     * @param    UINT iItem -- 选项值
     *
     * Describe  获取附加数据
     */
    STDMETHOD_(LPARAM, GetItemData)(THIS_ UINT iItem) SCONST PURE;

    /**
     * SComboBox::SetItemData
     * @brief    设置附加数据
     * @param    UINT iItem -- 索引值
     * @param    LPARAM lParam -- 附加值
     *
     * Describe  设置附加数据
     */
    STDMETHOD_(BOOL, SetItemData)(THIS_ UINT iItem, LPARAM lParam) PURE;

    /**
     * SComboBox::InsertItem
     * @brief    插入新项
     * @param    UINT iPos -- 位置
     * @param    LPCTSTR pszText -- 文本值
     * @param    int iIcon -- 图标
     * @param    LPARAM lParam -- 附加值
     *
     * Describe  插入新项
     */
    STDMETHOD_(int, InsertItem)(THIS_ int iPos, LPCTSTR pszText, int iIcon, LPARAM lParam) PURE;
    /**
     * SComboBox::DeleteString
     * @brief    删除某一项
     * @param    UINT iItem -- 索引值
     *
     * Describe  删除某一项
     */
    STDMETHOD_(BOOL, DeleteString)(THIS_ int iPos) PURE;
    /**
     * SComboBox::ResetContent
     * @brief    删除所有项
     *
     * Describe  设置附加数据
     */
    STDMETHOD_(void, ResetContent)(THIS) PURE;

    STDMETHOD_(IListBox *, GetIListBox)(THIS) PURE;
};

#undef INTERFACE
#define INTERFACE IComboView
DECLARE_INTERFACE_IID_(IComboView, IComboBase, "EADE040E-0D6A-47a3-882F-F70A94FC253D")
{
		STDMETHOD_(long,AddRef) (THIS) PURE;
	STDMETHOD_(long,Release) (THIS) PURE;
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
	STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
	//////////////////////////////////////////////////////////////////////////
    /**
     * SComboBoxBase::GetCurSel
     * @brief    获取选中值索引
     * @return   返回int
     *
     * Describe  获取当前选中索引
     */
    STDMETHOD_(int, GetCurSel)(THIS) SCONST PURE;

    /**
     * SComboBoxBase::GetCount
     * @brief    获取下拉项个数
     * @return   返回int
     *
     * Describe  获取下拉项个数
     */
    STDMETHOD_(int, GetCount)(THIS) SCONST PURE;

    /**
     * SComboBoxBase::SetCurSel
     * @brief    设置当前选中
     * @param    int iSel -- 选中索引
     *
     * Describe  设置当前选中
     */
    STDMETHOD_(BOOL, SetCurSel)(THIS_ int iSel) PURE;

    STDMETHOD_(BOOL, GetItemText)(THIS_ int iItem, BOOL bRawText, IStringT *str) SCONST PURE;

    /**
     * FindString
     * @brief    查找字符串位置
     * @param    LPCTSTR pszFind --  查找目标
     * @param    int nAfter --  开始位置
     * @return   int -- 目标索引，失败返回-1。
     * Describe
     */
    STDMETHOD_(int, FindString)
    (THIS_ LPCTSTR pszFind, int nAfter /* = -1*/, BOOL bPartMatch /* = TRUE*/) PURE;

    /**
     * SComboBoxBase::DropDown
     * @brief    下拉事件
     *
     * Describe  下拉事件
     */
    STDMETHOD_(void, DropDown)(THIS) PURE;

    /**
     * SComboBoxBase::CloseUp
     * @brief    下拉关闭
     *
     * Describe  下拉关闭
     */
    STDMETHOD_(void, CloseUp)(THIS) PURE;

    STDMETHOD_(BOOL, IsDropdown)(THIS) SCONST PURE;

    STDMETHOD_(void, SetDropdown)(THIS_ BOOL bDropdown) PURE;
	//////////////////////////////////////////////////////////////////////////
    STDMETHOD_(IListView *, GetIListView)(THIS) PURE;
};

#undef INTERFACE
#define INTERFACE IDateTimePicker
DECLARE_INTERFACE_IID_(IDateTimePicker, ICtrl, "6DB1EF42-AED7-4a36-8011-BE2AD7B45953")
{
	STDMETHOD_(long,AddRef) (THIS) PURE;
	STDMETHOD_(long,Release) (THIS) PURE;
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
	STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
	//////////////////////////////////////////////////////////////////////////

    STDMETHOD_(void, SetTime)
    (THIS_ WORD wYear, WORD wMonth, WORD wDay, WORD wHour, WORD wMinute, WORD wSecond) PURE;
    STDMETHOD_(void, GetTime)
    (THIS_ WORD * wYear, WORD * wMonth, WORD * wDay, WORD * wHour, WORD * wMinute, WORD * wSecond)
        SCONST PURE;
    STDMETHOD_(void, CloseUp)(THIS) PURE;
    STDMETHOD_(void, DropDown)(THIS) PURE;
    STDMETHOD_(void, Clear)(THIS) PURE;
};

typedef int(__cdecl *FunTreeSortCallback)(void *pCtx, const void *phItem1, const void *phItem2);

#undef INTERFACE
#define INTERFACE ITreeCtrl
DECLARE_INTERFACE_IID_(ITreeCtrl, ICtrl, "8FACB4B5-DD66-4755-AFD0-7DA8BB15611A")
{
	STDMETHOD_(long,AddRef) (THIS) PURE;
	STDMETHOD_(long,Release) (THIS) PURE;
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
	STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
	//////////////////////////////////////////////////////////////////////////

    STDMETHOD_(HSTREEITEM, InsertItem)
    (THIS_ LPCTSTR lpszItem, int nImage, int nSelectedImage, LPARAM lParam,
     HSTREEITEM hParent /*= STVI_ROOT*/, HSTREEITEM hInsertAfter /*= STVI_LAST*/) PURE;

    STDMETHOD_(BOOL, RemoveItem)(THIS_ HSTREEITEM hItem) PURE;
    STDMETHOD_(void, RemoveAllItems)(THIS) PURE;

    STDMETHOD_(HSTREEITEM, GetRootItem)(THIS) SCONST PURE;

    STDMETHOD_(HSTREEITEM, GetNextSiblingItem)(THIS_ HSTREEITEM hItem) SCONST PURE;
    STDMETHOD_(HSTREEITEM, GetPrevSiblingItem)(THIS_ HSTREEITEM hItem) SCONST PURE;
    STDMETHOD_(HSTREEITEM, GetChildItem)
    (THIS_ HSTREEITEM hItem, BOOL bFirst /*= TRUE*/) SCONST PURE;
    STDMETHOD_(HSTREEITEM, GetParentItem)(THIS_ HSTREEITEM hItem) SCONST PURE;
    STDMETHOD_(HSTREEITEM, GetSelectedItem)(THIS) SCONST PURE;
    STDMETHOD_(HSTREEITEM, GetNextItem)(THIS_ HSTREEITEM hItem) SCONST PURE;

    STDMETHOD_(void, SortChildren)
    (THIS_ HSTREEITEM hItem, FunTreeSortCallback sortFunc, void *pCtx) PURE;

    STDMETHOD_(BOOL, SelectItem)(THIS_ HSTREEITEM hItem, BOOL bEnsureVisible /* = TRUE*/) PURE;

    STDMETHOD_(BOOL, GetItemText)(THIS_ HSTREEITEM hItem, IStringT * strText) SCONST PURE;
    STDMETHOD_(BOOL, SetItemText)(THIS_ HSTREEITEM hItem, LPCTSTR lpszItem) PURE;
    STDMETHOD_(BOOL, GetItemImage)
    (THIS_ HSTREEITEM hItem, int *nImage, int *nSelectedImage) SCONST PURE;
    STDMETHOD_(BOOL, SetItemImage)(THIS_ HSTREEITEM hItem, int nImage, int nSelectedImage) PURE;
    STDMETHOD_(LPARAM, GetItemData)(THIS_ HSTREEITEM hItem) SCONST PURE;
    STDMETHOD_(BOOL, SetItemData)(THIS_ HSTREEITEM hItem, LPARAM lParam) PURE;
    STDMETHOD_(BOOL, ItemHasChildren)(THIS_ HSTREEITEM hItem) PURE;

    STDMETHOD_(BOOL, GetCheckState)(THIS_ HSTREEITEM hItem) SCONST PURE;
    STDMETHOD_(BOOL, SetCheckState)(THIS_ HSTREEITEM hItem, BOOL bCheck) PURE;

    STDMETHOD_(BOOL, Expand)(THIS_ HSTREEITEM hItem, UINT nCode /*= TVE_EXPAND*/) PURE;
    STDMETHOD_(BOOL, EnsureVisible)(THIS_ HSTREEITEM hItem) PURE;
};

#undef INTERFACE
#define INTERFACE IHotKeyCtrl
DECLARE_INTERFACE_IID_(IHotKeyCtrl, ICtrl, "8839DDF0-84CE-4bca-8BE4-FF55928E3A55")
{
	STDMETHOD_(long,AddRef) (THIS) PURE;
	STDMETHOD_(long,Release) (THIS) PURE;
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
	STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
	//////////////////////////////////////////////////////////////////////////

    /**
     * SHotKeyCtrl::SetRule
     * @brief    设置规则
     * @param    WORD wInvalidComp --
     * @param    WORD wModifier  --
     *
     * Describe  设置规则
     */
    STDMETHOD_(void, SetRule)(THIS_ WORD wInvalidComp, WORD wModifier) PURE;

    /**
     * SHotKeyCtrl::SetHotKey
     * @brief    设置热键
     * @param    WORD vKey --
     * @param    WORD wModifiers  --
     *
     * Describe  设置热键
     */
    STDMETHOD_(void, SetHotKey)(THIS_ WORD wKey, WORD wModifiers) PURE;

    /**
     * SHotKeyCtrl::GetHotKey
     * @brief    获取热键
     * @param    WORD &vKey --
     * @param    WORD wModifiers  --
     *
     * Describe  获取热键
     */
    STDMETHOD_(void, GetHotKey)(THIS_ WORD * wKey, WORD * wModifers) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IRichEdit
DECLARE_INTERFACE_IID_(IRichEdit, ICtrl, "6B72BCCE-9D42-4fb8-9CF4-F8F9605ACA9A")
{
	STDMETHOD_(long,AddRef) (THIS) PURE;
	STDMETHOD_(long,Release) (THIS) PURE;
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
	STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
	//////////////////////////////////////////////////////////////////////////

    STDMETHOD_(DWORD, SaveRtf)(THIS_ LPCTSTR pszFileName) PURE;

    STDMETHOD_(DWORD, LoadRtf)(THIS_ LPCTSTR pszFileName) PURE;

    /**
     * SRichEdit::GetWindowTextLength
     * @brief     获取窗口标题长度
     * @return    返回int
     *
     * Describe   获取窗口标题长度
     */
    STDMETHOD_(int, GetWindowTextLength)(THIS) SCONST PURE;

    /**
     * SRichEdit::SetSel
     * @brief     设置选中
     * @param     long nStartChar --
     * @param     long nEndChar --
     * @param     BOOL bNoScroll --
     *
     * Describe   设置选中, 支持超长文本
     */
    STDMETHOD_(void, SetSel)(THIS_ long nStartChar, long nEndChar, BOOL bNoScroll) PURE;
    /**
     * SRichEdit::ReplaceSel
     * @brief     替换选中项
     * @param     LPCWSTR pszText --
     * @param     BOOL bCanUndo --
     *
     * Describe   替换选中项
     */
    STDMETHOD_(void, ReplaceSel)(THIS_ LPCTSTR pszText, BOOL bCanUndo /*= TRUE*/) PURE;
    /**
     * SRichEdit::GetWordWrap
     * @brief
     * @return   返回BOOL
     *
     * Describe
     */
    STDMETHOD_(BOOL, GetWordWrap)(THIS) SCONST PURE;
    /**
     * SRichEdit::SetWordWrap
     * @brief
     * @param     BOOL fWordWrap --
     *
     * Describe
     */
    STDMETHOD_(void, SetWordWrap)(THIS_ BOOL fWordWrap) PURE;

    /**
     * SRichEdit::GetReadOnly
     * @brief    判断是否只读
     * @return   返回BOOL
     *
     * Describe  判断是否只读
     */
    STDMETHOD_(BOOL, GetReadOnly)(THIS) SCONST PURE;

    /**
     * SRichEdit::SetReadOnly
     * @brief     设置只读
     * @param     BOOL bReadOnly -- 是否只读
     * @return    返回BOOL
     *
     * Describe   设置只读
     */
    STDMETHOD_(BOOL, SetReadOnly)(THIS_ BOOL bReadOnly) PURE;

    /**
     * SRichEdit::GetLimitText
     * @brief     获取最小文本长度
     * @return    返回BOOL
     *
     * Describe   获取最小文本长度
     */
    STDMETHOD_(LONG, GetLimitText)(THIS) SCONST PURE;

    /**
     * SRichEdit::SetLimitText
     * @brief     设置最小文本长度
     * @param     int nLength -- 长度
     * @return    返回BOOL
     *
     * Describe   设置最小文本长度
     */
    STDMETHOD_(BOOL, SetLimitText)(THIS_ int nLength) PURE;

    /**
     * SRichEdit::GetDefaultAlign
     * @brief     获取对齐方式
     * @return    返回WORD
     *
     * Describe   设置最小文本长度
     */
    STDMETHOD_(WORD, GetDefaultAlign)(THIS) SCONST PURE;

    /**
     * SRichEdit::SetDefaultAlign
     * @brief     设置默认对齐方式
     * @param     WORD wNewAlign -- 对齐方式
     *
     * Describe   设置默认对齐方式
     */
    STDMETHOD_(void, SetDefaultAlign)(THIS_ WORD wNewAlign) PURE;

    /**
     * SRichEdit::GetRichTextFlag
     * @brief     获取标志
     * @return    返回BOOL
     *
     * Describe   获取标志
     */
    STDMETHOD_(BOOL, GetRichTextFlag)(THIS) SCONST PURE;

    /**
     * SRichEdit::SetRichTextFlag
     * @brief     设置标志
     * @param     BOOL fRich -- 标志
     *
     * Describe   设置标志
     */
    STDMETHOD_(void, SetRichTextFlag)(THIS_ BOOL fRich) PURE;

    /**
     * SRichEdit::SetRichTextFlag
     * @brief     设置标志
     * @param     BOOL fRich -- 标志
     * @return    返回LONG
     *
     * Describe   设置标志
     */
    STDMETHOD_(LONG, GetDefaultLeftIndent)(THIS) SCONST PURE;

    /**
     * SRichEdit::SetDefaultLeftIndent
     * @brief     设置缩进
     * @param     LONG lNewIndent -- 缩进字符数
     *
     * Describe   设置缩进
     */
    STDMETHOD_(void, SetDefaultLeftIndent)(THIS_ LONG lNewIndent) PURE;

    /**
     * SRichEdit::SetSaveSelection
     * @brief
     * @param     BOOL fSaveSelection
     * @return    返回BOOL
     *
     * Describe
     */
    STDMETHOD_(BOOL, SetSaveSelection)(THIS_ BOOL fSaveSelection) PURE;

    /**
     * SRichEdit::SetDefaultTextColor
     * @brief     默认文本颜色
     * @param     COLORREF cr -- 颜色
     * @return    返回COLORREF
     *
     * Describe   设置默认文本颜色
     */
    STDMETHOD_(COLORREF, SetDefaultTextColor)(THIS_ COLORREF cr) PURE;
};

#undef INTERFACE
#define INTERFACE ITabCtrl
DECLARE_INTERFACE_IID_(ITabCtrl, ICtrl, "CAD40CB4-A0E5-4bea-9CE6-8DFC45DEFFD4")
{
	STDMETHOD_(long,AddRef) (THIS) PURE;
	STDMETHOD_(long,Release) (THIS) PURE;
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
	STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
	//////////////////////////////////////////////////////////////////////////

    /**
     * STabCtrl::GetCurSel
     * @brief    获取当前选中
     * @return   返回int
     *
     * Describe  获取当前选中
     */
    STDMETHOD_(int, GetCurSel)(THIS) SCONST PURE;

    /**
     * STabCtrl::SetCurSel
     * @brief    设置当前选中
     * @param    int nIndex -- 索引
     * @return   返回BOOL
     *
     * Describe  获取当前选中
     */
    STDMETHOD_(BOOL, SetCurSel)(THIS_ int nIndex) PURE;

    /**
     * STabCtrl::SetItemTitle
     * @brief    设置标题
     * @param    int nIndex  -- 索引
     * @param    LPCTSTR lpszTitle  -- 标题
     * @return   返回BOOL
     *
     * Describe  获取当前选中
     */
    STDMETHOD_(BOOL, SetItemTitle)(THIS_ int nIndex, LPCTSTR lpszTitle) PURE;

    /**
     * STabCtrl::InsertItem
     * @brief    插入tab页面
     * @param    LPCWSTR lpContent  -- XML描述的page信息
     * @param    int iInsert  -- 位置
     * @return   返回插入位置
     *
     * Describe  插入tab页面
     */
    STDMETHOD_(int, InsertItem)(THIS_ LPCWSTR lpContent, int iInsert /* = -1*/) PURE;

    /**
     * STabCtrl::GetItemCount
     * @brief    获取tab页面数
     * @return   返回int
     *
     * Describe  获取tab页面数
     */
    STDMETHOD_(int, GetItemCount)(THIS) SCONST PURE;

    /**
     * STabCtrl::GetItem
     * @brief    获取指定tab页面
     * @param    int nIndex -- 索引
     * @return   返回int
     *
     * Describe  获取当前选中
     */
    STDMETHOD_(IWindow *, GetPage)(THIS_ int nIndex) PURE;

    /**
     * STabCtrl::RemoveItem
     * @brief    删除指定tab页面
     * @param    int nIndex -- 索引
     * @param    int nSelPage -- 选中页面
     * @return   删除指定tab页面
     *
     * Describe  获取当前选中
     */
    STDMETHOD_(BOOL, RemoveItem)(THIS_ int nIndex, int iSelPage /*= 0*/) PURE;

    /**
     * STabCtrl::RemoveAllItems
     * @brief    删除所有页面
     *
     * Describe  删除所有页面
     */
    STDMETHOD_(void, RemoveAllItems)(THIS) PURE;

    /**
     * STabCtrl::GetPageIndex
     * @brief    获取指定页面的索引
     * @param    LPCTSTR pszName -- 查询字符串
     * @param    BOOL bTitle --
     * TRUE:pszName代表的是page的title属性,FALSE：pszName代表的是page的name属性
     * @return   找到的页面索引号
     *
     * Describe
     */
    STDMETHOD_(int, GetPageIndex)(THIS_ LPCTSTR pszName, BOOL bTitle) PURE;
};

SNSEND