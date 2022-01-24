/************************************************************************/
/*   这里定义控件操作接口,所有接口都从IObjRef继承,并且指定自己的接口ID  */
/************************************************************************/
#pragma once
#include <interface/SWindow-i.h>
#include <interface/SSkinobj-i.h>
#include <interface/SRender-i.h>
#include <interface/SAdapter-i.h>
#include <interface/SListViewItemLocator-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE ICtrl
DECLARE_INTERFACE_(ICtrl, IObjRef)
{
	STDMETHOD_(IWindow*, ToIWindow)(THIS) PURE;
};

#undef INTERFACE
#define INTERFACE IImageWnd
DECLARE_INTERFACE_IID_(IImageWnd, ICtrl,"42148B05-6223-46f6-B903-D42D84038546")
{

    STDMETHOD_(BOOL, SetSkin)(THIS_ ISkinObj *pSkin, int iFrame /*= 0*/, BOOL bAutoFree /*= TRUE*/) PURE;

    /**
     * SImageWnd::SetImage
     * @param    IBitmap * pBitmap -- 图片对象
     * @param    FilterLevel fl -- FilterLevel
     * @return   void
     *
     * Describe  设置绘制图片
     */
    STDMETHOD_(void, SetImage)(THIS_ IBitmap *pBitmap, FilterLevel fl /*= kNone_FilterLevel*/) PURE;

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
    STDMETHOD_(ISkinObj *,GetSkin)(THIS) PURE;
};

#undef INTERFACE
#define INTERFACE IAnimateImgWnd
DECLARE_INTERFACE_IID_(IAnimateImgWnd, ICtrl,"374A5086-AD38-4f15-83E0-002822E2595A")
{
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
DECLARE_INTERFACE_IID_(IProgress, ICtrl,"77407E2D-582B-4ef2-A33A-427C933BAA8C")
{
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
DECLARE_INTERFACE_IID_(IPanel, ICtrl,"B1A97BB7-64BE-408f-AC7C-2197CC2F4DD0")
{
	STDMETHOD_(BOOL,ShowScrollBar)(THIS_ int wBar, BOOL bShow) PURE;

	STDMETHOD_(BOOL,EnableScrollBar)(THIS_ int wBar, BOOL bEnable) PURE;

	STDMETHOD_(BOOL,IsScrollBarEnable)(THIS_ BOOL bVertical) SCONST PURE;

	STDMETHOD_(void,SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) PURE;

	STDMETHOD_(BOOL,SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) PURE;

	STDMETHOD_(int,GetScrollPos)(THIS_ BOOL bVertical) SCONST PURE;

	STDMETHOD_(BOOL,SetScrollRange)(THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) PURE;

	STDMETHOD_(BOOL,GetScrollRange)(THIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST PURE;

	STDMETHOD_(BOOL,HasScrollBar)(THIS_ BOOL bVertical) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IScrollView
DECLARE_INTERFACE_IID_(IScrollView, IPanel,"49B024D6-221D-42d4-902B-AFAAC5AFFE41")
{
	STDMETHOD_(SIZE,GetViewSize)(THIS) SCONST PURE;

	STDMETHOD_(void,SetViewSize)(THIS_ SIZE szView) PURE;

	STDMETHOD_(POINT,GetViewOrigin)(THIS) SCONST PURE;

	STDMETHOD_(void,SetViewOrigin)(THIS_ POINT pt) PURE;
};


#undef INTERFACE
#define INTERFACE IListView
DECLARE_INTERFACE_IID_(IListView, IPanel,"E584B16A-6BDB-4afb-8BCC-3A1ABACD2FE2")
{
	STDMETHOD_(BOOL,SetAdapter)(THIS_ ILvAdapter *adapter) PURE;

	STDMETHOD_(ILvAdapter *,GetAdapter)(THIS) PURE;

	STDMETHOD_(IListViewItemLocator *,GetItemLocator)(THIS) PURE;

	STDMETHOD_(void,SetItemLocator)(THIS_ IListViewItemLocator *pItemLocator) PURE;
	
	STDMETHOD_(void,EnsureVisible)(THIS_ int iItem) PURE;

	STDMETHOD_(void,SetSel)(THIS_ int iItem, BOOL bNotify /*= FALSE*/) PURE;
	
	STDMETHOD_(int,GetSel)(THIS) SCONST PURE;

	STDMETHOD_(IWindow*,HitTest)(THIS_ const POINT * pt) SCONST PURE;
};

SNSEND