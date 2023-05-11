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
};

#undef INTERFACE
#define INTERFACE IOsrPanel
DECLARE_INTERFACE_IID_(IOsrPanel, ICtrl, "85A3CD3C-D665-454b-AABC-EE8389BBD914")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 设置在列表中对应的索引
     * @param index -- 索引值，在listview中它是一个索引号，在treeview中它是一个HTREEITEM值
     * @return
     */
    STDMETHOD_(void, SetItemIndex)(THIS_ LPARAM index) PURE;

    /**
     * @brief 获取在列表中的索引
     * @return LPARAM 在列表中的索引
     */
    STDMETHOD_(LPARAM, GetItemIndex)(CTHIS) SCONST PURE;

    /**
     * @brief 设置列表项的用户数据
     * @param dwData -- 用户数据
     * @return
     */
    STDMETHOD_(void, SetItemData)(THIS_ LPARAM dwData) PURE;

    /**
     * @brief 获取在列表中的用户数据
     * @return LPARAM 在列表中的用户数据
     */
    STDMETHOD_(LPARAM, GetItemData)(CTHIS) SCONST PURE;

	/**
	 * @brief 将panel的坐标转换为host坐标
     * @param[in,out] POINT *pt 窗口坐标
     * @return void
	*/
	STDMETHOD_(void,PtToHost)(THIS_ POINT *pt) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IItemPanel
DECLARE_INTERFACE_IID_(IItemPanel, IOsrPanel, "176CDF98-260E-4070-91B5-E6E163F909A4")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 设置在列表中对应的索引
     * @param index -- 索引值，在listview中它是一个索引号，在treeview中它是一个HTREEITEM值
     * @return
     */
    STDMETHOD_(void, SetItemIndex)(THIS_ LPARAM index) PURE;

    /**
     * @brief 获取在列表中的索引
     * @return LPARAM 在列表中的索引
     */
    STDMETHOD_(LPARAM, GetItemIndex)(CTHIS) SCONST PURE;

    /**
     * @brief 设置列表项的用户数据
     * @param dwData -- 用户数据
     * @return
     */
    STDMETHOD_(void, SetItemData)(THIS_ LPARAM dwData) PURE;

    /**
     * @brief 获取在列表中的用户数据
     * @return LPARAM 在列表中的用户数据
     */
    STDMETHOD_(LPARAM, GetItemData)(CTHIS) SCONST PURE;

    /**
     * @brief 将panel的坐标转换为host坐标
     * @param[in,out] POINT *pt 窗口坐标
     * @return void
    */
    STDMETHOD_(void, PtToHost)(THIS_ POINT * pt) SCONST PURE;
    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 设置列表项的skin
     * @param pSkin -- 列表项的skin
     * @return
     */
    STDMETHOD_(void, SetSkin)(THIS_ ISkinObj * pSkin) PURE;

    /**
     * @brief 设置列表项的颜色配置
     * @param crBk -- 背景颜色
     * @param crSelBk -- 选中状态的背景颜色
     * @return
     */
    STDMETHOD_(void, SetColor)(THIS_ COLORREF crBk, COLORREF crSelBk) PURE;
};

#undef INTERFACE
#define INTERFACE IImageWnd
DECLARE_INTERFACE_IID_(IImageWnd, ICtrl, "42148B05-6223-46f6-B903-D42D84038546")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 设置skin
     * @param pSkin -- skin对象
     * @param iFrame -- skin的子图索引
     * @param bAutoFree -- 控件管理pSkin标志，为TRUE时调用方可以释放该Skin
     * @return BOOL TRUE-成功，FALSE-失败
     */
    STDMETHOD_(BOOL, SetSkin)(THIS_ ISkinObj * pSkin, int iFrame DEF_VAL(0), BOOL bAutoFree DEF_VAL(TRUE)) PURE;

    /**
     * SImageWnd::GetSkin
     * @brief    获取资源
     * @return   返回值ISkinObj指针
     *
     * Describe  获取资源
     */
    STDMETHOD_(ISkinObj *, GetSkin)(THIS) PURE;

    /**
     * SImageWnd::SetImage
     * @param    IBitmap * pBitmap -- 图片对象
     * @param    FilterLevel fl -- FilterLevel
     * @return   void
     *
     * Describe  设置绘制图片
     */
    STDMETHOD_(void, SetImage)(THIS_ IBitmapS * pBitmap, FilterLevel fl DEF_VAL(kNone_FilterLevel)) PURE;

    /**
     * @brief 获取当前设置的IBitmapS对象
     * @return IBitmapS * -- 当前设置的IBitmapS对象
     */
    STDMETHOD_(IBitmapS *, GetImage)(THIS) PURE;

    /**
     * SImageWnd::SetIcon
     * @param    int nSubID -- 子图在Skin中的索引号
     * @brief    设置图标
     * @return   返回值BOOL 成功--TRUE 失败--FALSE
     *
     * Describe  设置图标
     */
    STDMETHOD_(BOOL, SetIcon)(THIS_ int nSubID) PURE;
};

#undef INTERFACE
#define INTERFACE IAnimateImgWnd
DECLARE_INTERFACE_IID_(IAnimateImgWnd, ICtrl, "374A5086-AD38-4f15-83E0-002822E2595A")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
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
    STDMETHOD_(BOOL, IsPlaying)(CTHIS) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IProgress
DECLARE_INTERFACE_IID_(IProgress, ICtrl, "77407E2D-582B-4ef2-A33A-427C933BAA8C")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
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
    STDMETHOD_(int, GetValue)(CTHIS) SCONST PURE;

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
    STDMETHOD_(void, GetRange)(CTHIS_ int *pMin, int *pMax) SCONST PURE;
    /**
     * SProgress::IsVertical
     * @brief    判断进度条是否为竖直状态
     * @return   返回值是 TRUE -- 竖直状态
     *
     * Describe  获取进度值
     */
    STDMETHOD_(BOOL, IsVertical)(CTHIS) SCONST PURE;
};

typedef enum _ScrollBarID
{
    SSB_NULL = 0,
    SSB_HORZ = 1,
    SSB_VERT = 2,
    SSB_BOTH = (SSB_HORZ | SSB_VERT),
} ScrollBarID;

#undef INTERFACE
#define INTERFACE IPanel
DECLARE_INTERFACE_IID_(IPanel, ICtrl, "B1A97BB7-64BE-408f-AC7C-2197CC2F4DD0")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 显示/隐藏滚动条
     * @param wBar 滚动条标志,see ScrollBarID
     * @param bShow TRUE-显示，FALSE-隐藏
     * @return
     */
    STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

    /**
     * @brief Enable/Disable滚动条
     * @param wBar 滚动条标志,see ScrollBarID
     * @param bShow TRUE-显示，FALSE-隐藏
     * @return
     */
    STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

    /**
     * @brief 获取滚动条Enable状态
     * @param bVertical -- TRUE-垂直滚动条
     * @return TRUE-enable
     */
    STDMETHOD_(BOOL, IsScrollBarEnable)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief 设置滚动条数据
     * @param si --滚动条数据
     * @param bVertical -- TRUE-垂直滚动条
     * @return
     */
    STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

    /**
     * @brief 设置滚动条位置
     * @param bVertical -- TRUE-垂直滚动条
     * @param nNewPos -- 滚动位置
     * @param bRedraw -- 重绘标志
     * @return
     */
    STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

    /**
     * @brief 获取滚动条位置
     * @param bVertical -- TRUE-垂直滚动条
     * @return 滚动条位置
     */
    STDMETHOD_(int, GetScrollPos)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief 设置滚动条范围
     * @param bVertical -- TRUE-垂直滚动条
     * @param nMinPos -- 最小值
     * @param nMaxPos -- 最大值
     * @param bRedraw -- 重绘标志
     * @return TRUE-成功
     */
    STDMETHOD_(BOOL, SetScrollRange)
    (THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

    /**
     * @brief 获取滚动条范围
     * @param bVertical -- TRUE-垂直滚动条
     * @param lpMinPos -- 最小值
     * @param lpMaxPos -- 最大值
     * @return
     */
    STDMETHOD_(BOOL, GetScrollRange)
    (CTHIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;

    /**
     * @brief 查询滚动条启用状态
     * @param bVertical -- TRUE-垂直滚动条
     * @return TRUE-启用
     */
    STDMETHOD_(BOOL, HasScrollBar)(CTHIS_ BOOL bVertical) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IScrollView
DECLARE_INTERFACE_IID_(IScrollView, IPanel, "49B024D6-221D-42d4-902B-AFAAC5AFFE41")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 显示/隐藏滚动条
     * @param wBar 滚动条标志,see ScrollBarID
     * @param bShow TRUE-显示，FALSE-隐藏
     * @return
     */
    STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

    /**
     * @brief Enable/Disable滚动条
     * @param wBar 滚动条标志,see ScrollBarID
     * @param bShow TRUE-显示，FALSE-隐藏
     * @return
     */
    STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

    /**
     * @brief 获取滚动条Enable状态
     * @param bVertical -- TRUE-垂直滚动条
     * @return TRUE-enable
     */
    STDMETHOD_(BOOL, IsScrollBarEnable)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief 设置滚动条数据
     * @param si --滚动条数据
     * @param bVertical -- TRUE-垂直滚动条
     * @return
     */
    STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

    /**
     * @brief 设置滚动条位置
     * @param bVertical -- TRUE-垂直滚动条
     * @param nNewPos -- 滚动位置
     * @param bRedraw -- 重绘标志
     * @return
     */
    STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

    /**
     * @brief 获取滚动条位置
     * @param bVertical -- TRUE-垂直滚动条
     * @return 滚动条位置
     */
    STDMETHOD_(int, GetScrollPos)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief 设置滚动条范围
     * @param bVertical -- TRUE-垂直滚动条
     * @param nMinPos -- 最小值
     * @param nMaxPos -- 最大值
     * @param bRedraw -- 重绘标志
     * @return TRUE-成功
     */
    STDMETHOD_(BOOL, SetScrollRange)
    (THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

    /**
     * @brief 获取滚动条范围
     * @param bVertical -- TRUE-垂直滚动条
     * @param lpMinPos -- 最小值
     * @param lpMaxPos -- 最大值
     * @return
     */
    STDMETHOD_(BOOL, GetScrollRange)
    (CTHIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;

    /**
     * @brief 查询滚动条启用状态
     * @param bVertical -- TRUE-垂直滚动条
     * @return TRUE-启用
     */
    STDMETHOD_(BOOL, HasScrollBar)(CTHIS_ BOOL bVertical) SCONST PURE;
    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 获取视图尺寸
     * @return SIZE-视图尺寸
     */
    STDMETHOD_(SIZE, GetViewSize)(CTHIS) SCONST PURE;

    /**
     * @brief 设置视图尺寸
     * @param szView 视图尺寸
     * @return
     */
    STDMETHOD_(void, SetViewSize)(THIS_ SIZE szView) PURE;

    /**
     * @brief 获取视图原点坐标
     * @return POINT-原点坐标
     */
    STDMETHOD_(POINT, GetViewOrigin)(CTHIS) SCONST PURE;

    /**
     * @brief 设置视图原点坐标
     * @param pt 原点坐标
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
} SHDITEM, *LPSHDITEM;

#undef INTERFACE
#define INTERFACE IHeaderCtrl
DECLARE_INTERFACE_IID_(IHeaderCtrl, ICtrl, "60A8788F-C775-424f-AA82-DCA9CECE3D45")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
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
    (THIS_ int iItem, LPCTSTR pszText, int nWidth, UINT fmt, LPARAM lParam, BOOL bDpiAware /*=FALSE*/, float fWeight /*=0.0f*/) PURE;

	STDMETHOD_(int, InsertItemA)
		(THIS_ int iItem, LPCSTR pszText, int nWidth, UINT fmt, LPARAM lParam, BOOL bDpiAware /*=FALSE*/, float fWeight /*=0.0f*/) PURE;

    /**
     * SHeaderCtrl::GetItem
     * @brief    获得新项
     * @param    int iItem  --  索引
     * @param    SHDITEM *pItem  -- 返回列表项结构
     * @return   返回BOOL
     *
     * Describe  获得新项
     */
    STDMETHOD_(BOOL, GetItem)(CTHIS_ int iItem, SHDITEM *pItem) SCONST PURE;

    /**
     * @brief 设置表头项数据
     * @param iItem --索引号
     * @param pItem --数据项
     * @return
     */
    STDMETHOD_(BOOL, SetItem)(THIS_ int iItem, const SHDITEM *pItem) PURE;

    /**
     * SHeaderCtrl::GetItemCount
     * @brief    获取列表项个数
     * @return   返回int
     *
     * Describe  获取列表项个数
     */
    STDMETHOD_(UINT, GetItemCount)(CTHIS) SCONST PURE;
    /**
     * SHeaderCtrl::GetTotalWidth
     * @brief    获得所有宽度
     * @return   返回int
     *
     * Describe  获得所有宽度
     */
    STDMETHOD_(int, GetTotalWidth)(CTHIS_ BOOL bMinWid DEF_VAL(FALSE)) SCONST PURE;

    /**
     * @brief 表头自动填充父窗口宽度标志
     * @return TRUE-表头自动填充父窗口宽度
     */
    STDMETHOD_(BOOL, IsAutoResize)(CTHIS) SCONST PURE;

    /**
     * SHeaderCtrl::GetItemWidth
     * @brief    获得指定项宽度
     * @param    int iItem  --  索引
     * @return   返回int
     *
     * Describe  获得新项
     */
    STDMETHOD_(int, GetItemWidth)(CTHIS_ int iItem) SCONST PURE;

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

    /**
     * @brief 设置表头项的排序标志
     * @param iItem --表头项索引
     * @param sortFlag --排序标志
     * @remark 排序标志会传递到列表的排序回调接口,用户在回调接口中实现数据排序
     */
    STDMETHOD_(void, SetItemSort)(THIS_ int iItem, UINT sortFlag) PURE;

    /**
     * @brief 设置表头项是否隐藏
     * @param iItem --表头项索引
     * @param visible -- TRUE-显示，FALSE-隐藏
     * @return
     */
    STDMETHOD_(void, SetItemVisible)(THIS_ int iItem, BOOL visible) PURE;

    /**
     * @brief 查询表头项是否隐藏
     * @param iItem --表头项索引
     * @return BOOL
     */
    STDMETHOD_(BOOL, IsItemVisible)(CTHIS_ int iItem) SCONST PURE;

    /**
     * @brief    获取表头项
     * @param    int iOrder  -- show order
	 * @return   int item index
     *
     * Describe  获取表头项得位置
     */
	STDMETHOD_(int,GetOriItemIndex)(CTHIS_ int iOrder) SCONST PURE;

	/**
     * @brief    获取表头项得位置
     * @param    int iItem  --  索引
	 * @param    LPRECT prc  --  表头项得位置
     *
     * Describe  获取表头项得位置
     */
	STDMETHOD_(void, GetItemRect)(CTHIS_ int iItem,LPRECT prc) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IListView
DECLARE_INTERFACE_IID_(IListView, IPanel, "E584B16A-6BDB-4afb-8BCC-3A1ABACD2FE2")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 显示/隐藏滚动条
     * @param wBar 滚动条标志,see ScrollBarID
     * @param bShow TRUE-显示，FALSE-隐藏
     * @return
     */
    STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

    /**
     * @brief Enable/Disable滚动条
     * @param wBar 滚动条标志,see ScrollBarID
     * @param bShow TRUE-显示，FALSE-隐藏
     * @return
     */
    STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

    /**
     * @brief 获取滚动条Enable状态
     * @param bVertical -- TRUE-垂直滚动条
     * @return TRUE-enable
     */
    STDMETHOD_(BOOL, IsScrollBarEnable)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief 设置滚动条数据
     * @param si --滚动条数据
     * @param bVertical -- TRUE-垂直滚动条
     * @return
     */
    STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

    /**
     * @brief 设置滚动条位置
     * @param bVertical -- TRUE-垂直滚动条
     * @param nNewPos -- 滚动位置
     * @param bRedraw -- 重绘标志
     * @return
     */
    STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

    /**
     * @brief 获取滚动条位置
     * @param bVertical -- TRUE-垂直滚动条
     * @return 滚动条位置
     */
    STDMETHOD_(int, GetScrollPos)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief 设置滚动条范围
     * @param bVertical -- TRUE-垂直滚动条
     * @param nMinPos -- 最小值
     * @param nMaxPos -- 最大值
     * @param bRedraw -- 重绘标志
     * @return TRUE-成功
     */
    STDMETHOD_(BOOL, SetScrollRange)
    (THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

    /**
     * @brief 获取滚动条范围
     * @param bVertical -- TRUE-垂直滚动条
     * @param lpMinPos -- 最小值
     * @param lpMaxPos -- 最大值
     * @return
     */
    STDMETHOD_(BOOL, GetScrollRange)
    (CTHIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;

    /**
     * @brief 查询滚动条启用状态
     * @param bVertical -- TRUE-垂直滚动条
     * @return TRUE-启用
     */
    STDMETHOD_(BOOL, HasScrollBar)(CTHIS_ BOOL bVertical) SCONST PURE;

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 设置列表的Adapter
     * @param adapter -- Adapter对象
     * @return BOOL
     * @remark 通常只应该在列表初始化的时调用一次，列表会增加该Adapter的引用计数
     */
    STDMETHOD_(BOOL, SetAdapter)(THIS_ ILvAdapter * adapter) PURE;

    /**
     * @brief 获取控件的Adapter对象
     * @return ILvAdapter* - Adapter对象
     */
    STDMETHOD_(ILvAdapter *, GetAdapter)(CTHIS) SCONST PURE;

    /**
     * @brief 设置列表项位置定位器
     * @param pItemLocator -- 位置定位器
     * @remark  位置定位器是一个将索引号和列表显示位置相互映射的接口，通过实现位置定位器，用户可以控制列表项的显示大小
     */
    STDMETHOD_(void, SetItemLocator)(THIS_ IListViewItemLocator * pItemLocator) PURE;

    /**
     * @brief 获取位置定位器
     * @return IListViewItemLocator *
     */
    STDMETHOD_(IListViewItemLocator *, GetItemLocator)(CTHIS) SCONST PURE;

    /**
     * @brief 使指定行滚动到视图可见区
     * @param iItem --行号
     * @return
     */
    STDMETHOD_(void, EnsureVisible)(THIS_ int iItem) PURE;

    /**
     * @brief 选中指定行
     * @param iItem --行号
     * @param bNotify --触发选中状态事件标志
     * @return
     */
    STDMETHOD_(void, SetSel)(THIS_ int iItem, BOOL bNotify DEF_VAL(FALSE)) PURE;

    /**
     * @brief 获取当前选中行
     * @return int 选中行索引，-1代表当前没有选中行
     */
    STDMETHOD_(int, GetSel)(CTHIS) SCONST PURE;

    /**
     * @brief 热点测试
     * @param pt -- 输入坐标
     * @return IItemPanel *--坐标下的ItemPanel
     */
    STDMETHOD_(IItemPanel *, HitTest)(CTHIS_ const POINT *pt) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IMcListView
DECLARE_INTERFACE_IID_(IMcListView, IPanel, "485AB4B5-6018-4710-BF8E-751FDE53E335")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 显示/隐藏滚动条
     * @param wBar 滚动条标志,see ScrollBarID
     * @param bShow TRUE-显示，FALSE-隐藏
     * @return
     */
    STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

    /**
     * @brief Enable/Disable滚动条
     * @param wBar 滚动条标志,see ScrollBarID
     * @param bShow TRUE-显示，FALSE-隐藏
     * @return
     */
    STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

    /**
     * @brief 获取滚动条Enable状态
     * @param bVertical -- TRUE-垂直滚动条
     * @return TRUE-enable
     */
    STDMETHOD_(BOOL, IsScrollBarEnable)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief 设置滚动条数据
     * @param si --滚动条数据
     * @param bVertical -- TRUE-垂直滚动条
     * @return
     */
    STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

    /**
     * @brief 设置滚动条位置
     * @param bVertical -- TRUE-垂直滚动条
     * @param nNewPos -- 滚动位置
     * @param bRedraw -- 重绘标志
     * @return
     */
    STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

    /**
     * @brief 获取滚动条位置
     * @param bVertical -- TRUE-垂直滚动条
     * @return 滚动条位置
     */
    STDMETHOD_(int, GetScrollPos)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief 设置滚动条范围
     * @param bVertical -- TRUE-垂直滚动条
     * @param nMinPos -- 最小值
     * @param nMaxPos -- 最大值
     * @param bRedraw -- 重绘标志
     * @return TRUE-成功
     */
    STDMETHOD_(BOOL, SetScrollRange)
    (THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

    /**
     * @brief 获取滚动条范围
     * @param bVertical -- TRUE-垂直滚动条
     * @param lpMinPos -- 最小值
     * @param lpMaxPos -- 最大值
     * @return
     */
    STDMETHOD_(BOOL, GetScrollRange)
    (CTHIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;

    /**
     * @brief 查询滚动条启用状态
     * @param bVertical -- TRUE-垂直滚动条
     * @return TRUE-启用
     */
    STDMETHOD_(BOOL, HasScrollBar)(CTHIS_ BOOL bVertical) SCONST PURE;
    //////////////////////////////////////////////////////////////////////////
    /**
     * @brief 设置列表的Adapter
     * @param adapter -- Adapter对象
     * @return BOOL
     * @remark 通常只应该在列表初始化的时调用一次，列表会增加该Adapter的引用计数
     */
    STDMETHOD_(BOOL, SetAdapter)(THIS_ IMcAdapter * adapter) PURE;

    /**
     * @brief 获取控件的Adapter对象
     * @return ILvAdapter* - Adapter对象
     */
    STDMETHOD_(IMcAdapter *, GetAdapter)(CTHIS) SCONST PURE;

    /**
     * @brief 设置列表项位置定位器
     * @param pItemLocator -- 位置定位器
     * @remark  位置定位器是一个将索引号和列表显示位置相互映射的接口，通过实现位置定位器，用户可以控制列表项的显示大小
     */
    STDMETHOD_(void, SetItemLocator)(THIS_ IListViewItemLocator * pItemLocator) PURE;

    /**
     * @brief 获取位置定位器
     * @return IListViewItemLocator *
     */
    STDMETHOD_(IListViewItemLocator *, GetItemLocator)(CTHIS) SCONST PURE;

    /**
     * @brief 使指定行滚动到视图可见区
     * @param iItem --行号
     * @return
     */
    STDMETHOD_(void, EnsureVisible)(THIS_ int iItem) PURE;

    /**
     * @brief 选中指定行
     * @param iItem --行号
     * @param bNotify --触发选中状态事件标志
     * @return
     */
    STDMETHOD_(void, SetSel)(THIS_ int iItem, BOOL bNotify /*= FALSE*/) PURE;

    /**
     * @brief 获取当前选中行
     * @return int 选中行索引，-1代表当前没有选中行
     */
    STDMETHOD_(int, GetSel)(CTHIS) SCONST PURE;

    /**
     * @brief 热点测试
     * @param pt -- 输入坐标
     * @return IItemPanel *--坐标下的ItemPanel
     */
    STDMETHOD_(IItemPanel *, HitTest)(CTHIS_ const POINT *pt) SCONST PURE;

    /**
     * @brief 获取表头控件
     * @return IHeaderCtrl *--表头控件
     */
    STDMETHOD_(IHeaderCtrl *, GetIHeaderCtrl)(CTHIS) SCONST PURE;

    /**
     * @brief 插入一列
     * @param nIndex --插入位置,-1代表append
     * @param pszText --文本
     * @param nWidth --占用宽度
     * @param fmt --显示格式
     * @param lParam --自定义数据
     * @param bDpiAware --宽度感知dpi缩放
     * @param fWeight --宽度的weight属性
     * @return 插入位置
     */
    STDMETHOD_(int, InsertColumn)
    (THIS_ int nIndex, LPCTSTR pszText, int nWidth, UINT fmt, LPARAM lParam DEF_VAL(0), BOOL bDpiAware DEF_VAL(TRUE), float fWeight DEF_VAL(0.0f)) PURE;

    /**
     * @brief 删除列
     * @param iCol --列索引
     * @return
     */
    STDMETHOD_(void, DeleteColumn)(THIS_ int iCol) PURE;

    /**
     * @brief 获取列数
     * @return int 列数
     */
    STDMETHOD_(int, GetColumnCount)(CTHIS) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE ITreeView
DECLARE_INTERFACE_IID_(ITreeView, IPanel, "4FB8BF5D-950C-4f05-861C-FBEB119E4C2B")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 显示/隐藏滚动条
     * @param wBar 滚动条标志,see ScrollBarID
     * @param bShow TRUE-显示，FALSE-隐藏
     * @return
     */
    STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

    /**
     * @brief Enable/Disable滚动条
     * @param wBar 滚动条标志,see ScrollBarID
     * @param bShow TRUE-显示，FALSE-隐藏
     * @return
     */
    STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

    /**
     * @brief 获取滚动条Enable状态
     * @param bVertical -- TRUE-垂直滚动条
     * @return TRUE-enable
     */
    STDMETHOD_(BOOL, IsScrollBarEnable)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief 设置滚动条数据
     * @param si --滚动条数据
     * @param bVertical -- TRUE-垂直滚动条
     * @return
     */
    STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

    /**
     * @brief 设置滚动条位置
     * @param bVertical -- TRUE-垂直滚动条
     * @param nNewPos -- 滚动位置
     * @param bRedraw -- 重绘标志
     * @return
     */
    STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

    /**
     * @brief 获取滚动条位置
     * @param bVertical -- TRUE-垂直滚动条
     * @return 滚动条位置
     */
    STDMETHOD_(int, GetScrollPos)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief 设置滚动条范围
     * @param bVertical -- TRUE-垂直滚动条
     * @param nMinPos -- 最小值
     * @param nMaxPos -- 最大值
     * @param bRedraw -- 重绘标志
     * @return TRUE-成功
     */
    STDMETHOD_(BOOL, SetScrollRange)
    (THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

    /**
     * @brief 获取滚动条范围
     * @param bVertical -- TRUE-垂直滚动条
     * @param lpMinPos -- 最小值
     * @param lpMaxPos -- 最大值
     * @return
     */
    STDMETHOD_(BOOL, GetScrollRange)
    (CTHIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;

    /**
     * @brief 查询滚动条启用状态
     * @param bVertical -- TRUE-垂直滚动条
     * @return TRUE-启用
     */
    STDMETHOD_(BOOL, HasScrollBar)(CTHIS_ BOOL bVertical) SCONST PURE;
    //////////////////////////////////////////////////////////////////////////
    /**
     * @brief 设置列表的Adapter
     * @param adapter -- Adapter对象
     * @return BOOL
     * @remark 通常只应该在列表初始化的时调用一次，列表会增加该Adapter的引用计数
     */
    STDMETHOD_(BOOL, SetAdapter)(THIS_ ITvAdapter * adapter) PURE;

    /**
     * @brief 获取控件的Adapter对象
     * @return ILvAdapter* - Adapter对象
     */
    STDMETHOD_(ITvAdapter *, GetAdapter)(CTHIS) SCONST PURE;

    /**
     * @brief 设置列表项位置定位器
     * @param pItemLocator -- 位置定位器
     * @remark  位置定位器是一个将索引号和列表显示位置相互映射的接口，通过实现位置定位器，用户可以控制列表项的显示大小
     */
    STDMETHOD_(void, SetItemLocator)(THIS_ ITreeViewItemLocator * pItemLocator) PURE;

    /**
     * @brief 获取位置定位器
     * @return IListViewItemLocator *
     */
    STDMETHOD_(ITreeViewItemLocator *, GetItemLocator)(CTHIS) SCONST PURE;

    /**
     * @brief 使指定行滚动到视图可见区
     * @param hItem --HSTREEITEM
     * @return
     */
    STDMETHOD_(void, EnsureVisible)(THIS_ HSTREEITEM hItem) PURE;

    /**
     * @brief 选中指定行
     * @param hItem --HSTREEITEM
     * @param bNotify --触发选中状态事件标志
     * @return
     */
    STDMETHOD_(void, SetSel)(THIS_ HSTREEITEM hItem, BOOL bNotify DEF_VAL(FALSE)) PURE;

    /**
     * @brief 获取当前选中行
     * @return HSTREEITEM 选中行索引，0代表当前没有选中行
     */
    STDMETHOD_(HSTREEITEM, GetSel)(CTHIS) SCONST PURE;

    /**
     * @brief 热点测试
     * @param pt -- 输入坐标
     * @return IItemPanel *--坐标下的ItemPanel
     */
    STDMETHOD_(IItemPanel *, HitTest)(CTHIS_ const POINT *pt) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE ITileView
DECLARE_INTERFACE_IID_(ITileView, IPanel, "3718C98E-9177-4afb-986F-94B03A78F2C3")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 显示/隐藏滚动条
     * @param wBar 滚动条标志,see ScrollBarID
     * @param bShow TRUE-显示，FALSE-隐藏
     * @return
     */
    STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

    /**
     * @brief Enable/Disable滚动条
     * @param wBar 滚动条标志,see ScrollBarID
     * @param bShow TRUE-显示，FALSE-隐藏
     * @return
     */
    STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

    /**
     * @brief 获取滚动条Enable状态
     * @param bVertical -- TRUE-垂直滚动条
     * @return TRUE-enable
     */
    STDMETHOD_(BOOL, IsScrollBarEnable)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief 设置滚动条数据
     * @param si --滚动条数据
     * @param bVertical -- TRUE-垂直滚动条
     * @return
     */
    STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

    /**
     * @brief 设置滚动条位置
     * @param bVertical -- TRUE-垂直滚动条
     * @param nNewPos -- 滚动位置
     * @param bRedraw -- 重绘标志
     * @return
     */
    STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

    /**
     * @brief 获取滚动条位置
     * @param bVertical -- TRUE-垂直滚动条
     * @return 滚动条位置
     */
    STDMETHOD_(int, GetScrollPos)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief 设置滚动条范围
     * @param bVertical -- TRUE-垂直滚动条
     * @param nMinPos -- 最小值
     * @param nMaxPos -- 最大值
     * @param bRedraw -- 重绘标志
     * @return TRUE-成功
     */
    STDMETHOD_(BOOL, SetScrollRange)
    (THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

    /**
     * @brief 获取滚动条范围
     * @param bVertical -- TRUE-垂直滚动条
     * @param lpMinPos -- 最小值
     * @param lpMaxPos -- 最大值
     * @return
     */
    STDMETHOD_(BOOL, GetScrollRange)
    (CTHIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;

    /**
     * @brief 查询滚动条启用状态
     * @param bVertical -- TRUE-垂直滚动条
     * @return TRUE-启用
     */
    STDMETHOD_(BOOL, HasScrollBar)(CTHIS_ BOOL bVertical) SCONST PURE;
    //////////////////////////////////////////////////////////////////////////
    /**
     * @brief 设置列表的Adapter
     * @param adapter -- Adapter对象
     * @return BOOL
     * @remark 通常只应该在列表初始化的时调用一次，列表会增加该Adapter的引用计数
     */
    STDMETHOD_(BOOL, SetAdapter)(THIS_ ILvAdapter * adapter) PURE;

    /**
     * @brief 获取控件的Adapter对象
     * @return ILvAdapter* - Adapter对象
     */
    STDMETHOD_(ILvAdapter *, GetAdapter)(CTHIS) SCONST PURE;

    /**
     * @brief 设置列表项位置定位器
     * @param pItemLocator -- 位置定位器
     * @remark  位置定位器是一个将索引号和列表显示位置相互映射的接口，通过实现位置定位器，用户可以控制列表项的显示大小
     */
    STDMETHOD_(void, SetItemLocator)(THIS_ ITileViewItemLocator * pItemLocator) PURE;

    /**
     * @brief 获取位置定位器
     * @return ITileViewItemLocator *
     */
    STDMETHOD_(ITileViewItemLocator *, GetItemLocator)(CTHIS) SCONST PURE;

    /**
     * @brief 使指定行滚动到视图可见区
     * @param iItem --行号
     * @return
     */
    STDMETHOD_(void, EnsureVisible)(THIS_ int iItem) PURE;

    /**
     * @brief 选中指定行
     * @param iItem --行号
     * @param bNotify --触发选中状态事件标志
     * @return
     */
    STDMETHOD_(void, SetSel)(THIS_ int iItem, BOOL bNotify DEF_VAL(FALSE)) PURE;

    /**
     * @brief 获取当前选中行
     * @return int 选中行索引，-1代表当前没有选中行
     */
    STDMETHOD_(int, GetSel)(CTHIS) SCONST PURE;

    /**
     * @brief 热点测试
     * @param pt -- 输入坐标
     * @return IItemPanel *--坐标下的ItemPanel
     */
    STDMETHOD_(IItemPanel *, HitTest)(CTHIS_ const POINT *pt) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IListBox
DECLARE_INTERFACE_IID_(IListBox, IPanel, "4A36DC8A-7378-4a2d-A3AF-D04B0712ACCD")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 显示/隐藏滚动条
     * @param wBar 滚动条标志,see ScrollBarID
     * @param bShow TRUE-显示，FALSE-隐藏
     * @return
     */
    STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

    /**
     * @brief Enable/Disable滚动条
     * @param wBar 滚动条标志,see ScrollBarID
     * @param bShow TRUE-显示，FALSE-隐藏
     * @return
     */
    STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

    /**
     * @brief 获取滚动条Enable状态
     * @param bVertical -- TRUE-垂直滚动条
     * @return TRUE-enable
     */
    STDMETHOD_(BOOL, IsScrollBarEnable)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief 设置滚动条数据
     * @param si --滚动条数据
     * @param bVertical -- TRUE-垂直滚动条
     * @return
     */
    STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

    /**
     * @brief 设置滚动条位置
     * @param bVertical -- TRUE-垂直滚动条
     * @param nNewPos -- 滚动位置
     * @param bRedraw -- 重绘标志
     * @return
     */
    STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

    /**
     * @brief 获取滚动条位置
     * @param bVertical -- TRUE-垂直滚动条
     * @return 滚动条位置
     */
    STDMETHOD_(int, GetScrollPos)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief 设置滚动条范围
     * @param bVertical -- TRUE-垂直滚动条
     * @param nMinPos -- 最小值
     * @param nMaxPos -- 最大值
     * @param bRedraw -- 重绘标志
     * @return TRUE-成功
     */
    STDMETHOD_(BOOL, SetScrollRange)
    (THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

    /**
     * @brief 获取滚动条范围
     * @param bVertical -- TRUE-垂直滚动条
     * @param lpMinPos -- 最小值
     * @param lpMaxPos -- 最大值
     * @return
     */
    STDMETHOD_(BOOL, GetScrollRange)
    (CTHIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;

    /**
     * @brief 查询滚动条启用状态
     * @param bVertical -- TRUE-垂直滚动条
     * @return TRUE-启用
     */
    STDMETHOD_(BOOL, HasScrollBar)(CTHIS_ BOOL bVertical) SCONST PURE;
    //////////////////////////////////////////////////////////////////////////
    /**
     * SListBox::GetCount
     * @brief    获取项个数
     * @return   返回int
     *
     * Describe  获取项个数
     */
    STDMETHOD_(int, GetCount)(CTHIS) SCONST PURE;

    /**
     * SListBox::GetCurSel
     * @brief    获取当前选中项索引
     * @return   返回int
     *
     * Describe  获取当前选中项索引
     */
    STDMETHOD_(int, GetCurSel)(CTHIS) SCONST PURE;

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
    STDMETHOD_(int, GetTopIndex)(CTHIS) SCONST PURE;

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
    STDMETHOD_(int, GetItemHeight)(CTHIS) SCONST PURE;

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
    STDMETHOD_(LPARAM, GetItemData)(CTHIS_ int nIndex) SCONST PURE;

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
    (CTHIS_ int nIndex, BOOL bRawText /*= FALSE*/, IStringT *str) SCONST PURE;

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

    /**
     * @brief 查找字符串在列表中的位置
     * @param iFindAfter --开始索引
     * @param pszText --目标字符串
     * @return int 找到的索引，-1代表没有找到
     */
    STDMETHOD_(int, FindString)(CTHIS_ int iFindAfter, LPCTSTR pszText) SCONST PURE;

	STDMETHOD_(BOOL, SetItemImage)(THIS_ int nIndex,int iImage) PURE;

	STDMETHOD_(int, GetItemImage)(THIS_ int nIndex) PURE;
};

#undef INTERFACE
#define INTERFACE IComboBase
DECLARE_INTERFACE_IID_(IComboBase, ICtrl, "2BF10693-BEBD-4497-B6B5-5380BA8401EC")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    //////////////////////////////////////////////////////////////////////////
    /**
     * SComboBoxBase::GetCurSel
     * @brief    获取选中值索引
     * @return   返回int
     *
     * Describe  获取当前选中索引
     */
    STDMETHOD_(int, GetCurSel)(CTHIS) SCONST PURE;

    /**
     * SComboBoxBase::GetCount
     * @brief    获取下拉项个数
     * @return   返回int
     *
     * Describe  获取下拉项个数
     */
    STDMETHOD_(int, GetCount)(CTHIS) SCONST PURE;

    /**
     * SComboBoxBase::SetCurSel
     * @brief    设置当前选中
     * @param    int iSel -- 选中索引
     *
     * Describe  设置当前选中
     */
    STDMETHOD_(BOOL, SetCurSel)(THIS_ int iSel) PURE;

    /**
     * @brief 获取指定项文本数据
     * @param iItem --索引号
     * @param bRawText --获取原始文本标志
     * @param[out] str 输出字符串对象
     * @return BOOL TRUE-成功
     * @remark bRawText为TRUE时输出未翻译的文本
     */
    STDMETHOD_(BOOL, GetItemText)(CTHIS_ int iItem, BOOL bRawText, IStringT *str) SCONST PURE;
	STDMETHOD_(BOOL, GetItemTextA)(CTHIS_ int iItem, BOOL bRawText, IStringA *str) SCONST PURE;

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
	STDMETHOD_(int, FindStringA)
		(THIS_ LPCSTR pszFind, int nAfter /* = -1*/, BOOL bPartMatch /* = TRUE*/) PURE;

	//////////////////////////////////////////////////////////////////////////
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

    /**
     * @brief 查询DropDown状态标志
     * @return TRUE-当前ComboBase为下拉列表
     * @remark 内部使用方法
     */
    STDMETHOD_(BOOL, IsDropdown)(CTHIS) SCONST PURE;

    /**
     * @brief 设置下拉状态标志
     * @param bDropdown -- TRUE:下拉列表
     * @return
     * @remark 内部使用方法
     */
    STDMETHOD_(void, SetDropdown)(THIS_ BOOL bDropdown) PURE;
};

#undef INTERFACE
#define INTERFACE IComboBox
DECLARE_INTERFACE_IID_(IComboBox, IComboBase, "AC6C72BB-51BE-4216-ADA5-B394283DC9CF")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    //////////////////////////////////////////////////////////////////////////
    /**
     * SComboBoxBase::GetCurSel
     * @brief    获取选中值索引
     * @return   返回int
     *
     * Describe  获取当前选中索引
     */
    STDMETHOD_(int, GetCurSel)(CTHIS) SCONST PURE;

    /**
     * SComboBoxBase::GetCount
     * @brief    获取下拉项个数
     * @return   返回int
     *
     * Describe  获取下拉项个数
     */
    STDMETHOD_(int, GetCount)(CTHIS) SCONST PURE;

    /**
     * SComboBoxBase::SetCurSel
     * @brief    设置当前选中
     * @param    int iSel -- 选中索引
     *
     * Describe  设置当前选中
     */
    STDMETHOD_(BOOL, SetCurSel)(THIS_ int iSel) PURE;

    /**
     * @brief 获取指定项文本数据
     * @param iItem --索引号
     * @param bRawText --获取原始文本标志
     * @param[out] str 输出字符串对象
     * @return BOOL TRUE-成功
     * @remark bRawText为TRUE时输出未翻译的文本
     */
    STDMETHOD_(BOOL, GetItemText)(CTHIS_ int iItem, BOOL bRawText, IStringT *str) SCONST PURE;

	STDMETHOD_(BOOL, GetItemTextA)(CTHIS_ int iItem, BOOL bRawText, IStringA *str) SCONST PURE;

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
	STDMETHOD_(int, FindStringA)
		(THIS_ LPCSTR pszFind, int nAfter /* = -1*/, BOOL bPartMatch /* = TRUE*/) PURE;

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

    /**
     * @brief 查询DropDown状态标志
     * @return TRUE-当前ComboBase为下拉列表
     * @remark 内部使用方法
     */
    STDMETHOD_(BOOL, IsDropdown)(CTHIS) SCONST PURE;

    /**
     * @brief 设置下拉状态标志
     * @param bDropdown -- TRUE:下拉列表
     * @return
     * @remark 内部使用方法
     */
    STDMETHOD_(void, SetDropdown)(THIS_ BOOL bDropdown) PURE;
    //////////////////////////////////////////////////////////////////////////

    /**
     * SComboBox::GetItemData
     * @brief    获取附加数据
     * @param    UINT iItem -- 选项值
     *
     * Describe  获取附加数据
     */
    STDMETHOD_(LPARAM, GetItemData)(CTHIS_ UINT iItem) SCONST PURE;

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
	STDMETHOD_(int, InsertItemA)(THIS_ int iPos, LPCSTR pszText, int iIcon, LPARAM lParam) PURE;
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

    /**
     * @brief 获取IListBox接口
     * @return IListBox *
     */
    STDMETHOD_(IListBox *, GetIListBox)(THIS) PURE;
};

#undef INTERFACE
#define INTERFACE IComboView
DECLARE_INTERFACE_IID_(IComboView, IComboBase, "EADE040E-0D6A-47a3-882F-F70A94FC253D")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    //////////////////////////////////////////////////////////////////////////
    /**
     * SComboBoxBase::GetCurSel
     * @brief    获取选中值索引
     * @return   返回int
     *
     * Describe  获取当前选中索引
     */
    STDMETHOD_(int, GetCurSel)(CTHIS) SCONST PURE;

    /**
     * SComboBoxBase::GetCount
     * @brief    获取下拉项个数
     * @return   返回int
     *
     * Describe  获取下拉项个数
     */
    STDMETHOD_(int, GetCount)(CTHIS) SCONST PURE;

    /**
     * SComboBoxBase::SetCurSel
     * @brief    设置当前选中
     * @param    int iSel -- 选中索引
     *
     * Describe  设置当前选中
     */
    STDMETHOD_(BOOL, SetCurSel)(THIS_ int iSel) PURE;

    /**
     * @brief 获取指定项文本数据
     * @param iItem --索引号
     * @param bRawText --获取原始文本标志
     * @param[out] str 输出字符串对象
     * @return BOOL TRUE-成功
     * @remark bRawText为TRUE时输出未翻译的文本
     */
    STDMETHOD_(BOOL, GetItemText)(CTHIS_ int iItem, BOOL bRawText, IStringT *str) SCONST PURE;

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

    /**
     * @brief 查询DropDown状态标志
     * @return TRUE-当前ComboBase为下拉列表
     * @remark 内部使用方法
     */
    STDMETHOD_(BOOL, IsDropdown)(CTHIS) SCONST PURE;

    /**
     * @brief 设置下拉状态标志
     * @param bDropdown -- TRUE:下拉列表
     * @return
     * @remark 内部使用方法
     */
    STDMETHOD_(void, SetDropdown)(THIS_ BOOL bDropdown) PURE;
    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 获取ListView接口
     * @return IListView *
     */
    STDMETHOD_(IListView *, GetIListView)(THIS) PURE;
};

#undef INTERFACE
#define INTERFACE IDateTimePicker
DECLARE_INTERFACE_IID_(IDateTimePicker, ICtrl, "6DB1EF42-AED7-4a36-8011-BE2AD7B45953")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 设置时间
     * @param wYear --年
     * @param wMonth --月
     * @param wDay --日
     * @param wHour --时
     * @param wMinute --分
     * @param wSecond --秒
     * @return
     */
    STDMETHOD_(void, SetTime)
    (THIS_ WORD wYear, WORD wMonth, WORD wDay, WORD wHour, WORD wMinute, WORD wSecond) PURE;

    /**
     * @brief 获取当前时间
     * @param[out] wYear --年
     * @param[out] wMonth --月
     * @param[out] wDay --日
     * @param[out] wHour --时
     * @param[out] wMinute --分
     * @param[out] wSecond --秒
     * @return
     */
    STDMETHOD_(void, GetTime)
    (CTHIS_ WORD * wYear, WORD * wMonth, WORD * wDay, WORD * wHour, WORD * wMinute, WORD * wSecond) SCONST PURE;

    /**
     * @brief 关闭下拉列表
     * @return
     */
    STDMETHOD_(void, CloseUp)(THIS) PURE;

    /**
     * @brief 显示下拉列表
     * @return
     */
    STDMETHOD_(void, DropDown)(THIS) PURE;

    /**
     * @brief 清除数据
     * @return
     */
    STDMETHOD_(void, Clear)(THIS) PURE;
};

typedef int(__cdecl *FunTreeSortCallback)(void *pCtx, const void *phItem1, const void *phItem2);

#define STVI_ROOT  ((HSTREEITEM)0xFFFF0000) //=TVI_ROOT
#define STVI_FIRST ((HSTREEITEM)0xFFFF0001) //=TVI_FIRST
#define STVI_LAST  ((HSTREEITEM)0xFFFF0002) //=TVI_LAST

#undef INTERFACE
#define INTERFACE ITreeCtrl
DECLARE_INTERFACE_IID_(ITreeCtrl, IPanel, "8FACB4B5-DD66-4755-AFD0-7DA8BB15611A")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 显示/隐藏滚动条
     * @param wBar 滚动条标志,see ScrollBarID
     * @param bShow TRUE-显示，FALSE-隐藏
     * @return
     */
    STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

    /**
     * @brief Enable/Disable滚动条
     * @param wBar 滚动条标志,see ScrollBarID
     * @param bShow TRUE-显示，FALSE-隐藏
     * @return
     */
    STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

    /**
     * @brief 获取滚动条Enable状态
     * @param bVertical -- TRUE-垂直滚动条
     * @return TRUE-enable
     */
    STDMETHOD_(BOOL, IsScrollBarEnable)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief 设置滚动条数据
     * @param si --滚动条数据
     * @param bVertical -- TRUE-垂直滚动条
     * @return
     */
    STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

    /**
     * @brief 设置滚动条位置
     * @param bVertical -- TRUE-垂直滚动条
     * @param nNewPos -- 滚动位置
     * @param bRedraw -- 重绘标志
     * @return
     */
    STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

    /**
     * @brief 获取滚动条位置
     * @param bVertical -- TRUE-垂直滚动条
     * @return 滚动条位置
     */
    STDMETHOD_(int, GetScrollPos)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief 设置滚动条范围
     * @param bVertical -- TRUE-垂直滚动条
     * @param nMinPos -- 最小值
     * @param nMaxPos -- 最大值
     * @param bRedraw -- 重绘标志
     * @return TRUE-成功
     */
    STDMETHOD_(BOOL, SetScrollRange)
    (THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

    /**
     * @brief 获取滚动条范围
     * @param bVertical -- TRUE-垂直滚动条
     * @param lpMinPos -- 最小值
     * @param lpMaxPos -- 最大值
     * @return
     */
    STDMETHOD_(BOOL, GetScrollRange)
    (CTHIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;

    /**
     * @brief 查询滚动条启用状态
     * @param bVertical -- TRUE-垂直滚动条
     * @return TRUE-启用
     */
    STDMETHOD_(BOOL, HasScrollBar)(CTHIS_ BOOL bVertical) SCONST PURE;
    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 插入一个HSTREEITEM
     * @param lpszItem --显示文本
     * @param nImage --图片索引
     * @param nSelectedImage --选中状态的图片索引
     * @param lParam --自定义数据
     * @param hParent --父HSTREEITEM
     * @param hInsertAfter --前面的兄弟HSTREEITEM
     * @return HSTREEITEM--新插入HSTREEITEM
     */
    STDMETHOD_(HSTREEITEM, InsertItem)
    (THIS_ LPCTSTR lpszItem, int nImage, int nSelectedImage, LPARAM lParam, HSTREEITEM hParent DEF_VAL(STVI_ROOT), HSTREEITEM hInsertAfter DEF_VAL(STVI_LAST)) PURE;

	STDMETHOD_(HSTREEITEM, InsertItemA)
		(THIS_ LPCSTR lpszItem, int nImage, int nSelectedImage, LPARAM lParam, HSTREEITEM hParent DEF_VAL(STVI_ROOT), HSTREEITEM hInsertAfter DEF_VAL(STVI_LAST)) PURE;

    /**
     * @brief 删除一个HSTREEITEM
     * @param hItem --待删除HSTREEITEM
     * @return BOOL
     */
    STDMETHOD_(BOOL, RemoveItem)(THIS_ HSTREEITEM hItem) PURE;

    /**
     * @brief 删除所有数据
     * @return
     */
    STDMETHOD_(void, RemoveAllItems)(THIS) PURE;

    /**
     * @brief 获取根节点
     * @return HSTREEITEM --根节点
     */
    STDMETHOD_(HSTREEITEM, GetRootItem)(CTHIS) SCONST PURE;

    /**
     * @brief 获取指定节点的下一个兄弟结点
     * @param hItem --指定节点
     * @return HSTREEITEM--下一个兄弟结点
     */
    STDMETHOD_(HSTREEITEM, GetNextSiblingItem)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief 获取指定节点的上一个兄弟结点
     * @param hItem --指定节点
     * @return HSTREEITEM--上一个兄弟结点
     */
    STDMETHOD_(HSTREEITEM, GetPrevSiblingItem)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief 获取指定结点的子节点
     * @param hItem --指定结点
     * @param bFirst -- TRUE：获取第一个子节点，FALSE:获取最后一个子节点
     * @return HSTREEITEM--子节点
     */
    STDMETHOD_(HSTREEITEM, GetChildItem)
    (CTHIS_ HSTREEITEM hItem, BOOL bFirst DEF_VAL(TRUE)) SCONST PURE;

    /**
     * @brief 获取指定结点的父节点
     * @param hItem --指定结点
     * @return HSTREEITEM--父节点
     */
    STDMETHOD_(HSTREEITEM, GetParentItem)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief 获取选中结点
     * @return HSTREEITEM--选中结点
     */
    STDMETHOD_(HSTREEITEM, GetSelectedItem)(CTHIS) SCONST PURE;

    /**
     * @brief 获取下一个结点
     * @param hItem -- 当前结点
     * @return HSTREEITEM--下一个结点
     * @remark 有子节点时是第一个子节点，否则为下一个兄弟结点，也没有兄弟则是父节点的下一个兄弟
     */
    STDMETHOD_(HSTREEITEM, GetNextItem)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief 排序子节点
     * @param hItem --当前节点
     * @param sortFunc --排序回调函数
     * @param pCtx --回调函数的Context
     * @return
     */
    STDMETHOD_(void, SortChildren)
    (THIS_ HSTREEITEM hItem, FunTreeSortCallback sortFunc, void *pCtx) PURE;

    /**
     * @brief 选中指定项
     * @param hItem --指定项
     * @param bEnsureVisible --自动滚动到可见区标志
     * @return
     */
    STDMETHOD_(BOOL, SelectItem)(THIS_ HSTREEITEM hItem, BOOL bEnsureVisible DEF_VAL(TRUE)) PURE;

    /**
     * @brief 获取指定项的文本数据
     * @param hItem --指定项
     * @param[out] strText --文本数据
     * @return BOOL
     * @remark 不支持翻译
     */
    STDMETHOD_(BOOL, GetItemText)(CTHIS_ HSTREEITEM hItem, IStringT * strText) SCONST PURE;
    STDMETHOD_(BOOL, GetItemTextA)(CTHIS_ HSTREEITEM hItem, IStringA* strText) SCONST PURE;

    /**
     * @brief 设置文本数据
     * @param hItem --指定项
     * @param lpszItem  --文本数据
     * @return BOOL
     */
    STDMETHOD_(BOOL, SetItemText)(THIS_ HSTREEITEM hItem, LPCTSTR lpszItem) PURE;
    STDMETHOD_(BOOL, SetItemTextA)(THIS_ HSTREEITEM hItem, LPCSTR lpszItem) PURE;

    /**
     * @brief 获取指定项的图标显示数据
     * @param hItem --指定项
     * @param nImage --图片索引
     * @param nSelectedImage --选中状态图片索引
     * @return BOOL
     */
    STDMETHOD_(BOOL, GetItemImage)
    (CTHIS_ HSTREEITEM hItem, int *nImage, int *nSelectedImage) SCONST PURE;

    /**
     * @brief 设置指定项的图片索引
     * @param hItem --指定项
     * @param nImage --图片索引
     * @param nSelectedImage --选中状态图片索引
     * @return BOOL
     */
    STDMETHOD_(BOOL, SetItemImage)(THIS_ HSTREEITEM hItem, int nImage, int nSelectedImage) PURE;

    /**
     * @brief 获取指定项的自定义数据
     * @param hItem --指定项
     * @return LPARAM--自定义数据
     */
    STDMETHOD_(LPARAM, GetItemData)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief 设置指定项的自定义数据
     * @param hItem --指定项
     * @param lParam --自定义数据
     * @return BOOL
     */
    STDMETHOD_(BOOL, SetItemData)(THIS_ HSTREEITEM hItem, LPARAM lParam) PURE;

    /**
     * @brief 查询指定项是否有子项
     * @param hItem --指定项
     * @return BOOL TRUE:有子项
     */
    STDMETHOD_(BOOL, ItemHasChildren)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief 获取指定项的Check状态
     * @param hItem --指定项
     * @return int 0--unchecked,1--checked,2--part checked
     */
    STDMETHOD_(int, GetCheckState)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief 设置指定项的Check状态
     * @param hItem --指定项
     * @param bCheck --TRUE:checked
     * @return
     */
    STDMETHOD_(BOOL, SetCheckState)(THIS_ HSTREEITEM hItem, BOOL bCheck) PURE;

    /**
     * @brief 展开或者收缩指定项的子项
     * @param hItem --指定项
     * @param nCode --标志位
     * @return
     */
    STDMETHOD_(BOOL, Expand)(THIS_ HSTREEITEM hItem, UINT nCode DEF_VAL(TVE_EXPAND)) PURE;

    /**
     * @brief 使指定项滚动到可见区
     * @param hItem  --指定项
     * @return
     */
    STDMETHOD_(BOOL, EnsureVisible)(THIS_ HSTREEITEM hItem) PURE;
};

#undef INTERFACE
#define INTERFACE IHotKeyCtrl
DECLARE_INTERFACE_IID_(IHotKeyCtrl, ICtrl, "8839DDF0-84CE-4bca-8BE4-FF55928E3A55")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
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
    STDMETHOD_(void, GetHotKey)(CTHIS_ WORD * wKey, WORD * wModifers) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IRichEdit
DECLARE_INTERFACE_IID_(IRichEdit, IPanel, "6B72BCCE-9D42-4fb8-9CF4-F8F9605ACA9A")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 显示/隐藏滚动条
     * @param wBar 滚动条标志,see ScrollBarID
     * @param bShow TRUE-显示，FALSE-隐藏
     * @return
     */
    STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

    /**
     * @brief Enable/Disable滚动条
     * @param wBar 滚动条标志,see ScrollBarID
     * @param bShow TRUE-显示，FALSE-隐藏
     * @return
     */
    STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

    /**
     * @brief 获取滚动条Enable状态
     * @param bVertical -- TRUE-垂直滚动条
     * @return TRUE-enable
     */
    STDMETHOD_(BOOL, IsScrollBarEnable)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief 设置滚动条数据
     * @param si --滚动条数据
     * @param bVertical -- TRUE-垂直滚动条
     * @return
     */
    STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

    /**
     * @brief 设置滚动条位置
     * @param bVertical -- TRUE-垂直滚动条
     * @param nNewPos -- 滚动位置
     * @param bRedraw -- 重绘标志
     * @return
     */
    STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

    /**
     * @brief 获取滚动条位置
     * @param bVertical -- TRUE-垂直滚动条
     * @return 滚动条位置
     */
    STDMETHOD_(int, GetScrollPos)(CTHIS_ BOOL bVertical) SCONST PURE;

    /**
     * @brief 设置滚动条范围
     * @param bVertical -- TRUE-垂直滚动条
     * @param nMinPos -- 最小值
     * @param nMaxPos -- 最大值
     * @param bRedraw -- 重绘标志
     * @return TRUE-成功
     */
    STDMETHOD_(BOOL, SetScrollRange)
    (THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

    /**
     * @brief 获取滚动条范围
     * @param bVertical -- TRUE-垂直滚动条
     * @param lpMinPos -- 最小值
     * @param lpMaxPos -- 最大值
     * @return
     */
    STDMETHOD_(BOOL, GetScrollRange)
    (CTHIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;

    /**
     * @brief 查询滚动条启用状态
     * @param bVertical -- TRUE-垂直滚动条
     * @return TRUE-启用
     */
    STDMETHOD_(BOOL, HasScrollBar)(CTHIS_ BOOL bVertical) SCONST PURE;
	//////////////////////////////////////////////////////////////////////////

    /**
     * IRichEdit::SaveRtf
     * @brief     保存数据到RTF文件
     * @param LPCTSTR pszFileName -- 文件名
     * @return    DWORD 数据长度
     *
     * Describe
     */
    STDMETHOD_(DWORD, SaveRtf)(THIS_ LPCTSTR pszFileName) PURE;

    /**
     * IRichEdit::LoadRtf
     * @brief     从RTF文件加载数据
     * @param LPCTSTR pszFileName -- 文件名
     * @return    DWORD 数据长度
     *
     * Describe
     */
    STDMETHOD_(DWORD, LoadRtf)(THIS_ LPCTSTR pszFileName) PURE;

    /**
     * IRichEdit::SetSel
     * @brief     设置选中
     * @param     long nStartChar -- 开始字符索引
     * @param     long nEndChar -- 结束字符索引
     * @param     BOOL bNoScroll -- 禁止滚动标志
     *
     * Describe   设置选中, 支持超长文本
     */
    STDMETHOD_(void, SetSel)(THIS_ long nStartChar, long nEndChar, BOOL bNoScroll) PURE;
    /**
     * IRichEdit::ReplaceSel
     * @brief     替换选中项
     * @param     LPCWSTR pszText -- 替换数据
     * @param     BOOL bCanUndo -- 允许Undo标志
     *
     * Describe   替换选中项
     */
    STDMETHOD_(void, ReplaceSel)(THIS_ LPCTSTR pszText, BOOL bCanUndo DEF_VAL(TRUE)) PURE;
    /**
     * IRichEdit::GetWordWrap
     * @brief
     * @return   返回BOOL
     *
     * Describe
     */
    STDMETHOD_(BOOL, GetWordWrap)(CTHIS) SCONST PURE;
    /**
     * IRichEdit::SetWordWrap
     * @brief
     * @param     BOOL fWordWrap -- word wrap标志
     *
     * Describe
     */
    STDMETHOD_(void, SetWordWrap)(THIS_ BOOL fWordWrap) PURE;

    /**
     * IRichEdit::GetReadOnly
     * @brief    判断是否只读
     * @return   返回BOOL
     *
     * Describe  判断是否只读
     */
    STDMETHOD_(BOOL, GetReadOnly)(CTHIS) SCONST PURE;

    /**
     * IRichEdit::SetReadOnly
     * @brief     设置只读
     * @param     BOOL bReadOnly -- 是否只读
     * @return    返回BOOL
     *
     * Describe   设置只读
     */
    STDMETHOD_(BOOL, SetReadOnly)(THIS_ BOOL bReadOnly) PURE;

    /**
     * IRichEdit::GetLimitText
     * @brief     获取最小文本长度
     * @return    返回BOOL
     *
     * Describe   获取最小文本长度
     */
    STDMETHOD_(LONG, GetLimitText)(CTHIS) SCONST PURE;

    /**
     * IRichEdit::SetLimitText
     * @brief     设置最小文本长度
     * @param     int nLength -- 长度
     * @return    返回BOOL
     *
     * Describe   设置最小文本长度
     */
    STDMETHOD_(BOOL, SetLimitText)(THIS_ int nLength) PURE;

    /**
     * IRichEdit::GetDefaultAlign
     * @brief     获取对齐方式
     * @return    返回WORD
     *
     * Describe   设置最小文本长度
     */
    STDMETHOD_(WORD, GetDefaultAlign)(CTHIS) SCONST PURE;

    /**
     * IRichEdit::SetDefaultAlign
     * @brief     设置默认对齐方式
     * @param     WORD wNewAlign -- 对齐方式
     *
     * Describe   设置默认对齐方式
     */
    STDMETHOD_(void, SetDefaultAlign)(THIS_ WORD wNewAlign) PURE;

    /**
     * IRichEdit::GetRichTextFlag
     * @brief     获取标志
     * @return    返回BOOL
     *
     * Describe   获取标志
     */
    STDMETHOD_(BOOL, GetRichTextFlag)(CTHIS) SCONST PURE;

    /**
     * IRichEdit::SetRichTextFlag
     * @brief     设置标志
     * @param     BOOL fRich -- 标志
     *
     * Describe   设置标志
     */
    STDMETHOD_(void, SetRichTextFlag)(THIS_ BOOL fRich) PURE;

    /**
     * IRichEdit::SetRichTextFlag
     * @brief     设置标志
     * @param     BOOL fRich -- 标志
     * @return    返回LONG
     *
     * Describe   设置标志
     */
    STDMETHOD_(LONG, GetDefaultLeftIndent)(CTHIS) SCONST PURE;

    /**
     * IRichEdit::SetDefaultLeftIndent
     * @brief     设置缩进
     * @param     LONG lNewIndent -- 缩进字符数
     *
     * Describe   设置缩进
     */
    STDMETHOD_(void, SetDefaultLeftIndent)(THIS_ LONG lNewIndent) PURE;

    /**
     * IRichEdit::SetSaveSelection
     * @brief
     * @param     BOOL fSaveSelection
     * @return    返回BOOL
     *
     * Describe
     */
    STDMETHOD_(BOOL, SetSaveSelection)(THIS_ BOOL fSaveSelection) PURE;

    /**
     * IRichEdit::SetDefaultTextColor
     * @brief     默认文本颜色
     * @param     COLORREF cr -- 颜色
     * @return    返回COLORREF
     *
     * Describe   设置默认文本颜色
     */
    STDMETHOD_(COLORREF, SetDefaultTextColor)(THIS_ COLORREF cr) PURE;
};

#undef INTERFACE
#define INTERFACE ITabPage
DECLARE_INTERFACE_IID_(ITabPage, ICtrl, "7E1C8BBF-1F87-4174-A30C-6CE3E4A47A13")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;

    //////////////////////////////////////////////////////////////////////////

	/**
     * GetTitle
     * @brief    获取标题
     * @return   LPCTSTR --- 标题
     *
     * Describe  获取标题
     */
    STDMETHOD_(LPCTSTR,GetTitle)(CTHIS) SCONST PURE;

    /**
     * SetTitle
     * @brief    设置标题
     * @param    LPCTSTR lpszTitle --- 标题
     *
     * Describe  设置标题
     */
    STDMETHOD_(void,SetTitle)(THIS_ LPCTSTR lpszTitle) PURE;

    /**
     * GetIconIndex
     * @brief    获取图标索引
     * @return   int, 图标索引
     */
    STDMETHOD_(int,GetIconIndex)(CTHIS) SCONST PURE;

    /**
     * SetIconIndex
     * @brief    设置图标索引
	 * @param    int iIcon -- 图标索引
     * @return   void
     */
    STDMETHOD_(void,SetIconIndex)(THIS_ int iIcon) PURE;
};

#undef INTERFACE
#define INTERFACE ITabCtrl
DECLARE_INTERFACE_IID_(ITabCtrl, ICtrl, "CAD40CB4-A0E5-4bea-9CE6-8DFC45DEFFD4")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    //////////////////////////////////////////////////////////////////////////

    /**
     * ITabCtrl::GetCurSel
     * @brief    获取当前选中
     * @return   返回int
     *
     * Describe  获取当前选中
     */
    STDMETHOD_(int, GetCurSel)(CTHIS) SCONST PURE;

    /**
     * ITabCtrl::SetCurSel
     * @brief    设置当前选中
     * @param    int nIndex -- 索引
     * @return   返回BOOL
     *
     * Describe  获取当前选中
     */
    STDMETHOD_(BOOL, SetCurSel)(THIS_ int nIndex) PURE;

    /**
     * ITabCtrl::SetItemTitle
     * @brief    设置标题
     * @param    int nIndex  -- 索引
     * @param    LPCTSTR lpszTitle  -- 标题
     * @return   返回BOOL
     *
     * Describe  获取当前选中
     */
    STDMETHOD_(BOOL, SetItemTitle)(THIS_ int nIndex, LPCTSTR lpszTitle) PURE;

    /**
     * ITabCtrl::InsertItem
     * @brief    插入tab页面
     * @param    LPCWSTR lpContent  -- XML描述的page信息
     * @param    int iInsert  -- 位置
     * @return   返回插入位置
     *
     * Describe  插入tab页面
     */
    STDMETHOD_(int, InsertItem)(THIS_ LPCWSTR lpContent, int iInsert /* = -1*/) PURE;

    /**
     * ITabCtrl::GetItemCount
     * @brief    获取tab页面数
     * @return   返回int
     *
     * Describe  获取tab页面数
     */
    STDMETHOD_(int, GetItemCount)(CTHIS) SCONST PURE;

    /**
     * ITabCtrl::GetItem
     * @brief    获取指定tab页面
     * @param    int nIndex -- 索引
     * @return   返回int
     *
     * Describe  获取当前选中
     */
    STDMETHOD_(ITabPage *, GetPage)(THIS_ int nIndex) PURE;

    /**
     * ITabCtrl::RemoveItem
     * @brief    删除指定tab页面
     * @param    int nIndex -- 索引
     * @param    int nSelPage -- 选中页面
     * @return   删除指定tab页面
     *
     * Describe  获取当前选中
     */
    STDMETHOD_(BOOL, RemoveItem)(THIS_ int nIndex, int iSelPage /*= 0*/) PURE;

    /**
     * ITabCtrl::RemoveAllItems
     * @brief    删除所有页面
     *
     * Describe  删除所有页面
     */
    STDMETHOD_(void, RemoveAllItems)(THIS) PURE;

    /**
     * ITabCtrl::GetPageIndex
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

#undef INTERFACE
#define INTERFACE IEdit
DECLARE_INTERFACE_IID_(IEdit, ICtrl, "E682E0FF-1B1C-4a15-AB43-552E705B2560"){
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

    /**
     * @brief 获取提示字符串
	 * @param[out] IStringT *pStr--提示字符串
     * @return void
	 * @remark 设置使用SetAttribute
     */
	STDMETHOD_(void,GetCueText)(CTHIS_ IStringT *pStr) SCONST PURE;

    /**
     * @brief 获取提示字符串颜色
     * @return COLORREF--提示字符串颜色
	 * @remark 设置使用SetAttribute
     */
	STDMETHOD_(COLORREF,GetCueColor)(CTHIS) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE ISpinButtonCtrl
DECLARE_INTERFACE_IID_(ISpinButtonCtrl, ICtrl, "C04997B9-E2AA-48bf-AEA5-FF1A03561F8C")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 设置值
     * @param nValue --新值
     * @return
     */
    STDMETHOD_(void, SetValue)(THIS_ int nValue) PURE;

    /**
     * @brief 设置范围
     * @param nMin --最小值
     * @param nMax --最大值
     * @return
     */
    STDMETHOD_(void, SetRange)(THIS_ int nMin, int nMax) PURE;

    /**
     * @brief 设置步长
     * @param nStep --步长
     * @return
     */
    STDMETHOD_(void, SetStep)(THIS_ UINT nStep) PURE;

    /**
     * @brief 获取值
     * @return int --当前值
     */
    STDMETHOD_(int, GetValue)(CTHIS) SCONST PURE;

    /**
     * @brief 获取关联的窗口对象
     * @return IWindow *--关联的窗口对象
     */
    STDMETHOD_(IWindow *, GetIBuddy)(CTHIS) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IIconWnd
DECLARE_INTERFACE_IID_(IIconWnd, ICtrl, "8BBD8033-9955-41FD-8C3D-19FE702A9DB0")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;

    //////////////////////////////////////////////////////////////////////////

	/**
     * @brief    SetIcon
     * @param    HICON hIcon -- 图标资源句柄
     * @return   void
     *
     * Describe  图标资源
     */
    STDMETHOD_(void, SetIcon)(THIS_ HICON hIcon) PURE;
};


#undef INTERFACE
#define INTERFACE IRealWnd
DECLARE_INTERFACE_IID_(IRealWnd, ICtrl, "3E9F9B19-68D1-47f1-9011-2F8B98C0A628")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;

    //////////////////////////////////////////////////////////////////////////

    STDMETHOD_(const IStringT*, GetRealClassName)(CTHIS) SCONST PURE;

	STDMETHOD_(const IStringT*, GetRealWindowName)(CTHIS) SCONST PURE;

	STDMETHOD_(DWORD, GetRealStyle)(CTHIS) SCONST PURE;

	STDMETHOD_(DWORD, GetRealStyleEx)(CTHIS) SCONST PURE;

	STDMETHOD_(IXmlNode*, GetRealParam)(CTHIS) PURE;

	/**
     * SRealWnd::GetRealHwnd
     * @brief    获取窗口句柄
     * @param    BOOL bAutoCreate -- 自动创建
     * @return   返回HWND
     *
     * Describe  获取窗口句柄
     */
	STDMETHOD_(HWND, GetRealHwnd)(THIS_ BOOL bAutoCreate DEF_VAL(TRUE)) PURE;


    /**
     * SRealWnd::SetData
     * @brief    获取附加数据
     * @param    LPVOID lpData -- 附加数据
     *
     * Describe  获取附加数据
     */
    STDMETHOD_(void,SetData)(THIS_ LPVOID lpData) PURE;

    /**
     * SRealWnd::GetData
     * @brief    获取附加数据
     * @return   返回LPVOID
     *
     * Describe  获取附加数据
     */
    STDMETHOD_(LPVOID,GetData)(THIS) PURE;
};

typedef enum _StackViewAniStyle{
	kAniNone=0,
	kFadeInOut,
	kMoveInOut,
	kPushInOut
}StackViewAniStyle;

#undef INTERFACE
#define INTERFACE IStackView
DECLARE_INTERFACE_IID_(IStackView, ICtrl, "1A7172D8-F45B-45fe-A73E-2B7F07E7EB70")
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

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 转换为IWindow*接口
     * @return IWindow*接口
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;

    //////////////////////////////////////////////////////////////////////////

	STDMETHOD_(BOOL,SelectView)(THIS_ int iView) PURE;
	STDMETHOD_(void,SetAniStyle)(THIS_ StackViewAniStyle aniStyle) PURE;
	STDMETHOD_(void,SetAniDir)(THIS_ BOOL bVert) PURE;	
};

SNSEND