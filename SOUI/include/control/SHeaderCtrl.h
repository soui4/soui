/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserved.
 *
 * @file       SHeaderCtrl.h
 * @brief
 * @version    v1.0
 * @author     soui
 * @date       2014-07-02
 *
 * Describe
 */
#pragma once

#include <core/SWnd.h>
#include <interface/SCtrl-i.h>

SNSBEGIN

#define SORT_MASK  (HDF_SORTDOWN | HDF_SORTUP)
#define ALIGN_MASK (HDF_LEFT | HDF_RIGHT | HDF_CENTER)

/**
 * @class     SHeaderCtrl
 * @brief     表头控件
 *
 * Describe   表头控件
 */
class SOUI_EXP SHeaderCtrl : public TWindowProxy<IHeaderCtrl> {
    DEF_SOBJECT(SWindow, L"header")

    enum
    {
        MARGIN_ADJUST_ENABLE = 4,
        MARGIN_ADJUST_DISABLE = 2,
    };

    typedef struct SHDITEMEX : public SHDITEM
    {
        STrText strText;
    } * LPSHDITEMEX;

  public:
    /**
     * SHeaderCtrl::SHeaderCtrl
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    SHeaderCtrl(void);
    /**
     * SHeaderCtrl::~SHeaderCtrl
     * @brief    析构函数
     *
     * Describe  析构函数
     */
    ~SHeaderCtrl(void);

  public:
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
    (THIS_ int iItem, LPCTSTR pszText, int nWidth, UINT fmt, LPARAM lParam, BOOL bDpiAware = FALSE, float fWeight = 0.0f) OVERRIDE;

	STDMETHOD_(int, InsertItemA)
		(THIS_ int iItem, LPCSTR pszText, int nWidth, UINT fmt, LPARAM lParam, BOOL bDpiAware = FALSE, float fWeight = 0.0f) OVERRIDE
	{
		SStringT str = S_CA2T(pszText,CP_UTF8);
		return InsertItem(iItem,str,nWidth,fmt,lParam,bDpiAware,fWeight);
	}

    /**
     * SHeaderCtrl::GetItem
     * @brief    获得新项
     * @param    int iItem  --  索引
     * @param    SHDITEM *pItem  -- 返回列表项结构
     * @return   返回BOOL
     *
     * Describe  获得新项
     */
    STDMETHOD_(BOOL, GetItem)(THIS_ int iItem, SHDITEM *pItem) SCONST OVERRIDE;

    STDMETHOD_(BOOL, SetItem)(THIS_ int iItem, const SHDITEM *pItem) OVERRIDE;

    /**
     * SHeaderCtrl::GetItemCount
     * @brief    获取列表项个数
     * @return   返回int
     *
     * Describe  获取列表项个数
     */
    STDMETHOD_(UINT, GetItemCount)(THIS) SCONST OVERRIDE;
    /**
     * SHeaderCtrl::GetTotalWidth
     * @brief    获得所有宽度
     * @return   返回int
     *
     * Describe  获得所有宽度
     */
    STDMETHOD_(int, GetTotalWidth)(THIS_ BOOL bMinWid = FALSE) SCONST OVERRIDE;

    STDMETHOD_(BOOL, IsAutoResize)(THIS) SCONST OVERRIDE;

    /**
     * SHeaderCtrl::GetItemWidth
     * @brief    获得指定项宽度
     * @param    int iItem  --  索引
     * @return   返回int
     *
     * Describe  获得新项
     */
    STDMETHOD_(int, GetItemWidth)(THIS_ int iItem) SCONST OVERRIDE;

    /**
     * SHeaderCtrl::DeleteItem
     * @brief    删除指定项
     * @param    int iItem  --  索引
     * @return   返回BOOL
     *
     * Describe  删除指定项
     */
    STDMETHOD_(BOOL, DeleteItem)(THIS_ int iItem) OVERRIDE;

    /**
     * SHeaderCtrl::DeleteAllItems
     * @brief    删除所有项
     *
     * Describe  获得新项
     */
    STDMETHOD_(void, DeleteAllItems)(THIS) OVERRIDE;

    STDMETHOD_(void, SetItemSort)(THIS_ int iItem, UINT sortFlag) OVERRIDE;

    STDMETHOD_(void, SetItemVisible)(THIS_ int iItem, BOOL visible) OVERRIDE;

    STDMETHOD_(BOOL, IsItemVisible)(THIS_ int iItem) SCONST OVERRIDE;

	STDMETHOD_(int,GetOriItemIndex)(CTHIS_ int iOrder) SCONST OVERRIDE;
	STDMETHOD_(void,GetItemRect)(CTHIS_ int iItem,LPRECT prc) SCONST OVERRIDE;

  public:
    /**
     * SHeaderCtrl::GetItemRect
     * @brief    获取表头项得位置
     * @param    UINT iItem  --  索引
     *
     * Describe  获取表头项得位置
     */
    CRect GetItemRect(int iItem) const;

    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"itemSkin", m_pSkinItem, FALSE)
        ATTR_SKIN(L"sortSkin", m_pSkinSort, FALSE)
        ATTR_INT(L"fixWidth", m_bFixWidth, FALSE)
        ATTR_INT(L"itemSwapEnable", m_bItemSwapEnable, FALSE)
        ATTR_INT(L"sortHeader", m_bSortHeader, FALSE)
    SOUI_ATTRS_END()
  protected:
    /**
     * SHeaderCtrl::CreateChildren
     * @brief    创建新项
     * @param    SXmlNode xmlNode  -- xml配置文件
     *
     * Describe  创建新项
     */
    virtual BOOL CreateChildren(SXmlNode xmlNode);

    /**
     * SHeaderCtrl::OnSetCursor
     * @brief    设置选中
     * @param    const CPoint &pt  --  坐标点
     *
     * Describe  获得新项
     */
    virtual BOOL OnSetCursor(const CPoint &pt);
    /**
     * SHeaderCtrl::OnPaint
     * @brief    设置选中
     * @param    IRenderTarget * pRT  --  渲染目标
     *
     * Describe  消息响应函数
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * SHeaderCtrl::DrawItem
     * @brief    绘画
     * @param    IRenderTarget * pRT  --  渲染目标
     * @param    CRect rcItem  --  目标区域
     * @param    const LPSHDITEM pItem  --  列表头
     *
     * Describe  绘画
     */
    virtual void DrawItem(IRenderTarget *pRT, CRect rcItem, const LPSHDITEMEX pItem);

    virtual void OnColorize(COLORREF cr);

    virtual HRESULT OnLanguageChanged();

    virtual void OnScaleChanged(int nScale);

    virtual BOOL OnRelayout(const CRect &rcWnd);
    /**
     * SHeaderCtrl::HitTest
     * @brief
     * @param    CPoint pt  --  坐标
     *
     * Describe  鼠标位置分析，返回一个DWORD，LOWORD为前面的项，
     *           HIWORD为后面的项， 相同时代表在一个项的中间
     */
    DWORD HitTest(CPoint pt);

    /**
     * SHeaderCtrl::RedrawItem
     * @brief    重绘
     * @param    CPoint pt  --  坐标
     *
     * Describe  重绘
     */
    void RedrawItem(int iItem);

    /**
     * SHeaderCtrl::DrawDraggingState
     * @brief    绘制拖拽状态
     * @param    DWORD dwDragTo  --
     *
     * Describe  绘制拖拽状态
     */
    void DrawDraggingState(DWORD dwDragTo);

    /**
     * SHeaderCtrl::IsItemHover
     * @brief    判断是否Hover状态
     * @param    DWORD dwHitTest
     * @return   返回BOOL
     *
     * Describe  判断是否Hover状态
     */
    BOOL IsItemHover(DWORD dwHitTest)
    {
        return dwHitTest != -1 && LOWORD(dwHitTest) == HIWORD(dwHitTest);
    }

    /**
     * SHeaderCtrl::CreateDragImage
     * @brief    创建拖拽图片
     * @param    UINT iItem -- 列表项
     * @return   返回HBITMAP
     *
     * Describe  判断是否Hover状态
     */
    HBITMAP CreateDragImage(UINT iItem);
    /**
     * SHeaderCtrl::OnLButtonDown
     * @brief    左键按下事件
     * @param    UINT nFlags -- 标志
     * @param    CPoint point -- 鼠标坐标
     *
     * Describe  此函数是消息响应函数
     */
    void OnLButtonDown(UINT nFlags, CPoint pt);
    /**
     * SHeaderCtrl::OnLButtonUp
     * @brief    左键抬起事件
     * @param    UINT nFlags -- 标志
     * @param    CPoint point -- 鼠标坐标
     *
     * Describe  此函数是消息响应函数
     */
    void OnLButtonUp(UINT nFlags, CPoint pt);
    /**
     * SHeaderCtrl::OnMouseMove
     * @brief    鼠标移动事件
     * @param    UINT nFlags -- 标志
     * @param    CPoint point -- 鼠标坐标
     *
     * Describe  此函数是消息响应函数
     */
    void OnMouseMove(UINT nFlags, CPoint pt);

    /**
     * SHeaderCtrl::OnMouseLeave
     * @brief    鼠标离开事件
     *
     * Describe  此函数是消息响应函数
     */
    void OnMouseLeave();

    /**
     * SHeaderCtrl::OnDestroy
     * @brief    销毁事件
     *
     * Describe  此函数是消息响应函数
     */
    void OnDestroy();

    void OnActivateApp(BOOL bActive, DWORD dwThreadID);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_ACTIVATEAPP(OnActivateApp)
        MSG_WM_DESTROY(OnDestroy)
    SOUI_MSG_MAP_END()

    SAutoRefPtr<ISkinObj> m_pSkinItem; /**< 表头绘制Skin */
    SAutoRefPtr<ISkinObj> m_pSkinSort; /**< 排序标志Skin */
    BOOL m_bSortHeader;                /**< 表头可以点击排序 */
    BOOL m_bFixWidth;                  /**< 表项宽度固定开关 */
    BOOL m_bItemSwapEnable;            /**< 允许拖动调整位置开关 */

    BOOL m_bDragging;             /**< 正在拖动标志 */
    HBITMAP m_hDragImg;           /**< 显示拖动窗口的临时位图 */
    CPoint m_ptClick;             /**< 当前点击坐标 */
    DWORD m_dwHitTest;            /**< 鼠标位置 */
    DWORD m_dwDragTo;             /**< 拖放目标 */
    int m_nAdjItemOldWidth;       /**< 保存被拖动项的原始宽度 */
    SArray<SHDITEMEX> m_arrItems; /**< 列表项集合 */
    int m_nScale;                 /**< Current Scale */
};
SNSEND