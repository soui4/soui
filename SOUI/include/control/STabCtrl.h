/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserved.
 *
 * @file       stabctrl.h
 * @brief
 * @version    v1.0
 * @author     soui
 * @date       2014-07-06
 *
 * Describe    扩展列表框
 */
#pragma once
#include <core/SWnd.h>
#include <helper/SplitString.h>

SNSBEGIN
/**
 * @class     STabPage
 * @brief     tab标签页面
 *
 * Describe   tab标签页面
 */
class SOUI_EXP STabPage : public TWindowProxy<ITabPage>{
    DEF_SOBJECT(SWindow, L"page")
    friend class STabCtrl;

  public:
    /**
     * STabPage::STabPage
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    STabPage()
        : m_iIcon(-1)
        , m_strTitle(this)
    {
        m_bVisible = FALSE;
    }
    /**
     * STabPage::~STabPage
     * @brief    析构函数
     *
     * Describe  析构函数
     */
    virtual ~STabPage()
    {
    }
public:
	STDMETHOD_(LPCTSTR,GetTitle)(CTHIS) SCONST OVERRIDE
	{
		return m_strTitle.GetText(FALSE);
	}
	STDMETHOD_(void,SetTitle)(THIS_ LPCTSTR lpszTitle) OVERRIDE  
	{
		m_strTitle.SetText(lpszTitle);
	}

	STDMETHOD_(int,GetIconIndex)(CTHIS) SCONST OVERRIDE{
		return m_iIcon;
	}

    STDMETHOD_(void,SetIconIndex)(THIS_ int iIcon) OVERRIDE
    {
        m_iIcon = iIcon;
    }

protected:
    /**
     * OnUpdateToolTip
     * @brief    处理tooltip
     * @param    const CPoint & pt --  测试点
     * @param [out]  SwndToolTipInfo & tipInfo -- tip信息
     * @return   BOOL -- FALSE
     *
     * Describe  总是返回FALSE，禁止在page页面上显示tooltip
     */
    virtual BOOL UpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo)
    {
        return FALSE;
    }

    SOUI_ATTRS_BEGIN()
        ATTR_I18NSTRT(L"title", m_strTitle, FALSE)
        ATTR_INT(L"iconIndex", m_iIcon, FALSE)
    SOUI_ATTRS_END()
  protected:
    STrText m_strTitle; /**< 标题 */
    int m_iIcon;
};

/**
 * @class     STabCtrl
 * @brief     tab控件
 *
 * Describe   tab控件
 */
class SOUI_EXP STabCtrl : public TWindowProxy<ITabCtrl> {
    friend class STabSlider;
    DEF_SOBJECT(SWindow, L"tabctrl")

  protected:
    int m_nHoverTabItem;                   /**< hover状态item */
    int m_nCurrentPage;                    /**< 当前页码      */
    SLayoutSize m_nTabInterSize;           /**< tab页面间距   */
    SLayoutSize m_szTab[2];                /**< tab SIZE   */
    SLayoutSize m_nTabPos;                 /**< tab位置       */
    SAutoRefPtr<ISkinObj> m_pSkinTab;      /**< ISkibObj对象 */
    SAutoRefPtr<ISkinObj> m_pSkinIcon;     /**< ISkibObj对象  */
    SAutoRefPtr<ISkinObj> m_pSkinTabInter; /**< ISkibObj对象  */
    SAutoRefPtr<ISkinObj> m_pSkinFrame;    /**< ISkibObj对象  */
    SLayoutSize m_ptIcon[2];               /**< 图标位置 */
    SLayoutSize m_ptText[2];               /**< 标题位置 */
    int m_nTabAlign;                       /**< 排列方式 */

    SArray<STabPage *> m_lstPages; /**< tab标签页面链表 */
    STabSlider *m_tabSlider;
    enum
    {
        AlignTop,
        AlignLeft,
        AlignBottom,
        AlignRight,
    };

    enum TEXTDIR
    {
        Text_Horz,
        Text_Vert,
    } m_txtDir;
    int m_nAnimateSteps; /**< 动画次数 */
    int m_nAniamteType;  /*动画样式*/
    SAutoRefPtr<IInterpolator> m_aniInterpolator;

  public:
    /**
     * STabCtrl::STabCtrl
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    STabCtrl();

    /**
     * STabCtrl::~STabCtrl
     * @brief    析构函数
     *
     * Describe  析构函数
     */
    virtual ~STabCtrl()
    {
    }

  public:
    /**
     * STabCtrl::GetCurSel
     * @brief    获取当前选中
     * @return   返回int
     *
     * Describe  获取当前选中
     */
    STDMETHOD_(int, GetCurSel)(THIS) SCONST OVERRIDE;

    /**
     * STabCtrl::SetCurSel
     * @brief    设置当前选中
     * @param    int nIndex -- 索引
     * @return   返回BOOL
     *
     * Describe  获取当前选中
     */
    STDMETHOD_(BOOL, SetCurSel)(THIS_ int nIndex) OVERRIDE;

    /**
     * STabCtrl::SetItemTitle
     * @brief    设置标题
     * @param    int nIndex  -- 索引
     * @param    LPCTSTR lpszTitle  -- 标题
     * @return   返回BOOL
     *
     * Describe  获取当前选中
     */
    STDMETHOD_(BOOL, SetItemTitle)(THIS_ int nIndex, LPCTSTR lpszTitle) OVERRIDE;

    /**
     * STabCtrl::InsertItem
     * @brief    插入tab页面
     * @param    LPCWSTR lpContent  -- XML描述的page信息
     * @param    int iInsert  -- 位置
     * @return   返回插入位置
     *
     * Describe  插入tab页面
     */
    STDMETHOD_(int, InsertItem)(THIS_ LPCWSTR lpContent, int iInsert = -1) OVERRIDE;

    /**
     * STabCtrl::GetItemCount
     * @brief    获取tab页面数
     * @return   返回int
     *
     * Describe  获取tab页面数
     */
    STDMETHOD_(int, GetItemCount)(THIS) SCONST OVERRIDE;

    /**
     * STabCtrl::GetItem
     * @brief    获取指定tab页面
     * @param    int nIndex -- 索引
     * @return   返回int
     *
     * Describe  获取当前选中
     */
    STDMETHOD_(ITabPage *, GetPage)(THIS_ int nIndex) OVERRIDE;

    /**
     * STabCtrl::RemoveItem
     * @brief    删除指定tab页面
     * @param    int nIndex -- 索引
     * @param    int nSelPage -- 选中页面
     * @return   删除指定tab页面
     *
     * Describe  获取当前选中
     */
    STDMETHOD_(BOOL, RemoveItem)(THIS_ int nIndex, int iSelPage = 0) OVERRIDE;

    /**
     * STabCtrl::RemoveAllItems
     * @brief    删除所有页面
     *
     * Describe  删除所有页面
     */
    STDMETHOD_(void, RemoveAllItems)(THIS) OVERRIDE;

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
    STDMETHOD_(int, GetPageIndex)(THIS_ LPCTSTR pszName, BOOL bTitle) OVERRIDE;

  public:
    STabPage *GetItem(int iPage);

    STabPage *GetPage(LPCTSTR pszName, BOOL bTitle = TRUE);

    /**
     * STabCtrl::SetCurSel
     * @brief    设置当前选中
     * @param    LPCTSTR pszTitle -- 标题
     * @return   返回BOOL
     *
     * Describe  获取当前选中
     */
    BOOL SetCurSel(LPCTSTR pszName, BOOL bTitle = TRUE);

  protected:
    /**
     * STabCtrl::CreateChildren
     * @brief    创建tab页面
     * @param    SXmlNode xmlNode  -- xml文件
     * @return   返回BOOL
     *
     * Describe  创建tab页面
     */
    BOOL CreateChildren(SXmlNode xmlNode);

    /**
     * STabCtrl::InsertItem
     * @brief    插入tab页面
     * @param    SXmlNode xmlNode  -- xml文件
     * @param    int iInsert  -- 位置
     * @param    BOOL bLoading -- 是否加载
     * @return   返回int
     *
     * Describe  插入tab页面
     */
    virtual int InsertItem(SXmlNode xmlNode, int iInsert = -1, BOOL bLoading = FALSE);

    /**
     * OnItemInserted
     * @brief    插入page状态
     * @param    STabPage *pItem --  插入的PAGE
     * @return   void
     *
     * Describe
     */
    virtual void OnItemInserted(STabPage *pItem)
    {
    }

    /**
     * OnItemRemoved
     * @brief    删除page状态
     * @param    STabPage *pItem --  删除的PAGE
     * @return   void
     *
     * Describe
     */
    virtual void OnItemRemoved(STabPage *pItem)
    {
    }

    /**
     * BeforePaint
     * @brief    为RT准备好当前窗口的绘图环境
     * @param    IRenderTarget * pRT --
     * @param    SPainter & painter --
     * @return   void
     *
     * Describe  和SWindow不同，STabCtrl中强制使用normal状态配置字体及颜色，其它状态给tab头使用
     */
    virtual void BeforePaint(IRenderTarget *pRT, SPainter &painter);

    /**
     * STabCtrl::GetChildrenLayoutRect
     * @brief
     *
     * Describe
     */
    STDMETHOD_(void, GetChildrenLayoutRect)(THIS_ RECT *prc) SCONST OVERRIDE;

    /**
     * GetTitleRect
     * @brief    获取tab头的矩形
     * @return   CRect
     *
     * Describe
     */
    virtual CRect GetTitleRect();

    /**
     * STabCtrl::GetItemRect
     * @brief    获取指定item位置
     * @param    int nIndex -- 索引
     * @param     CRect &rcItem -- 位置
     *
     * Describe  获取指定item位置
     */
    virtual BOOL GetItemRect(int nIndex, CRect &rcItem);

    /**
     * STabCtrl::DrawItem
     * @brief    绘制item
     * @param    IRenderTarget *pRT -- 绘制设备
     * @param    const CRect &rcItem -- 绘制区域
     * @param    int iItem  -- 索引
     * @param    DWORD dwState  -- 状态
     *
     * Describe  绘制item
     */
    virtual void DrawItem(IRenderTarget *pRT, const CRect &rcItem, int iItem, DWORD dwState);

    virtual STabPage *CreatePageFromXml(SXmlNode xmlPage);

    /**
     * STabCtrl::OnGetDlgCode
     * @brief    获取窗口消息码
     * @return   返回UINT
     *
     * Describe  获取窗口消息码
     */
    virtual UINT WINAPI OnGetDlgCode() const
    {
        return SC_WANTARROWS;
    }

    virtual BOOL UpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo);

    /**
     * UpdateChildrenPosition
     * @brief    更新子窗口位置
     * @return   void
     *
     * Describe
     */
    STDMETHOD_(void, UpdateChildrenPosition)(THIS) OVERRIDE;

    STDMETHOD_(void, OnInitFinished)(THIS_ IXmlNode *xmlNode) OVERRIDE;

    virtual void OnColorize(COLORREF cr);
    virtual void OnScaleChanged(int nScale);

    virtual HRESULT OnLanguageChanged();

  protected:
    int HitTest(CPoint pt);

    void OnSliderFinish();

  protected:
    void TextOutV(IRenderTarget *pRT, int x, int y, const SStringT &strText);
    void DrawTextV(IRenderTarget *pRT, CRect rcText, const SStringT &strText);
    SIZE MeasureTextV(IRenderTarget *pRT, const SStringT &strText);

  protected:
    /**
     * STabCtrl::OnPaint
     * @brief    绘画消息
     * @param    IRenderTarget *pRT -- 绘制设备句柄
     *
     * Describe  此函数是消息响应函数
     */
    void OnPaint(IRenderTarget *pRT);
    /**
     * STabCtrl::OnLButtonDown
     * @brief    鼠标左键按下事件
     * @param    UINT nFlags -- 标志
     * @param    CPoint point -- 鼠标坐标
     *
     * Describe  此函数是消息响应函数
     */
    void OnLButtonDown(UINT nFlags, CPoint point);
    /**
     * STabCtrl::OnMouseMove
     * @brief    鼠标移动事件
     * @param    UINT nFlags -- 标志
     * @param    CPoint point -- 鼠标坐标
     *
     * Describe  此函数是消息响应函数
     */
    void OnMouseMove(UINT nFlags, CPoint point);
    /**
     * STabCtrl::OnMouseLeave
     * @brief    鼠标离开事件
     *
     * Describe  此函数是消息响应函数
     */
    void OnMouseLeave()
    {
        OnMouseMove(0, CPoint(-1, -1));
    }
    /**
     * STabCtrl::OnKeyDown
     * @brief    键盘按下
     * @param    UINT nChar -- 键码
     * @param    UINT nRepCnt -- 重复次数
     * @param    UINT nFlags -- 标志
     *
     * Describe  此函数是消息响应函数
     */
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    /**
     * STabCtrl::OnDestroy
     * @brief    销毁
     *
     * Describe  此函数是消息响应函数
     */
    void OnDestroy();

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_KEYDOWN(OnKeyDown)
    SOUI_MSG_MAP_END()

    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"curSel", m_nCurrentPage, FALSE)
        ATTR_LAYOUTSIZE2(L"tabSize", m_szTab, TRUE)
        ATTR_LAYOUTSIZE(L"tabWidth", m_szTab[0], FALSE)
        ATTR_LAYOUTSIZE(L"tabHeight", m_szTab[1], FALSE)
        ATTR_LAYOUTSIZE(L"tabPos", m_nTabPos, FALSE)
        ATTR_LAYOUTSIZE(L"tabInterSize", m_nTabInterSize, FALSE)
        ATTR_SKIN(L"tabInterSkin", m_pSkinTabInter, FALSE)
        ATTR_SKIN(L"tabSkin", m_pSkinTab, FALSE)
        ATTR_SKIN(L"iconSkin", m_pSkinIcon, FALSE)
        ATTR_SKIN(L"frameSkin", m_pSkinFrame, FALSE)
        ATTR_LAYOUTSIZE(L"icon-x", m_ptIcon[0], FALSE)
        ATTR_LAYOUTSIZE(L"icon-y", m_ptIcon[1], FALSE)
        ATTR_LAYOUTSIZE(L"text-x", m_ptText[0], FALSE)
        ATTR_LAYOUTSIZE(L"text-y", m_ptText[1], FALSE)
        ATTR_ENUM_BEGIN(L"tabAlign", int, TRUE)
            ATTR_ENUM_VALUE(L"top", AlignTop)
            ATTR_ENUM_VALUE(L"left", AlignLeft)
            ATTR_ENUM_VALUE(L"right", AlignRight)
            ATTR_ENUM_VALUE(L"bottom", AlignBottom)
        ATTR_ENUM_END(m_nTabAlign)
        ATTR_ENUM_BEGIN(L"textDir", TEXTDIR, TRUE)
            ATTR_ENUM_VALUE(L"hori", Text_Horz)
            ATTR_ENUM_VALUE(L"horizontal", Text_Horz)
            ATTR_ENUM_VALUE(L"vert", Text_Vert)
            ATTR_ENUM_VALUE(L"vertical", Text_Vert)
        ATTR_ENUM_END(m_txtDir)
        ATTR_INT(L"animateSteps", m_nAnimateSteps, FALSE)
        ATTR_INT(L"animateType", m_nAniamteType, FALSE) /*动画样式0：背景跟着动，1：背景不动*/
        ATTR_INTERPOLATOR(L"interpolator", m_aniInterpolator, FALSE)
        ATTR_CHAIN_PTR(m_aniInterpolator, 0) // chain attributes to interpolator
    SOUI_ATTRS_END()
};

SNSEND