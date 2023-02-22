#pragma once

#include "core/SWnd.h"
#include "SRichEdit.h"
#include "SDropDown.h"

SNSBEGIN
#define IDC_CB_EDIT       -100
#define IDC_DROPDOWN_LIST -200

/**
 * @class      SComboEdit
 * @brief      在CommboBox中嵌入的Edit控件
 *
 * Describe
 */
class SOUI_EXP SComboEdit : public SEdit {
	DEF_SOBJECT(SEdit,L"ComboEdit")
  public:
    /**
     * SComboEdit::SComboEdit
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    SComboEdit();

    /**
     * SComboEdit::~SComboEdit
     * @brief    析构函数
     *
     * Describe  析构函数
     */
    virtual ~SComboEdit();

  protected:
    /**
     * SComboEdit::OnMouseHover
     * @brief    键盘鼠标悬停事件
     * @param    WPARAM wParam
     * @param    CPoint ptPos -- 鼠标所在位置
     *
     * Describe  此函数是消息响应函数
     */
    void OnMouseHover(WPARAM wParam, CPoint ptPos);
    /**
     * SComboEdit::OnMouseLeave
     * @brief    键盘鼠标离开事件
     *
     * Describe  此函数是消息响应函数
     */
    void OnMouseLeave();
    /**
     * SComboEdit::OnKeyDown
     * @brief    键盘按下事件
     * @param    UINT nChar -- 按键对应的码值
     * @param    UINT nRepCnt -- 重复次数
     * @param    UINT nFlags -- 标志
     *
     * Describe  此函数是消息响应函数
     */
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * SComboEdit::FireEvent
     * @brief    通知消息
     * @param    IEvtArgs * evt -- 事件对象
     *
     * Describe  此函数是消息响应函数
     */
    STDMETHOD_(BOOL, FireEvent)(THIS_ IEvtArgs *evt) OVERRIDE;

    void OnKillFocus(SWND wndFocus);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_MOUSEHOVER(OnMouseHover)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
    SOUI_MSG_MAP_END()
};

class SOUI_EXP SDropDownWnd_ComboBox : public SDropDownWnd {
  public:
    SDropDownWnd_ComboBox(ISDropDownOwner *pOwner)
        : SDropDownWnd(pOwner)
    {
    }

    virtual BOOL WINAPI PreTranslateMessage(MSG *pMsg);
};

/**
 * @class      SComboBoxBase
 * @brief      可输入CommboBox
 *
 * Describe    可输入下拉列表
 */
class SOUI_EXP SComboBase
    : public TWindowProxy<IComboBase>
    , public ISDropDownOwner {
    DEF_SOBJECT(SWindow, L"combobase")
  public:
    /**
     * SComboBoxBase::SComboBoxBase
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    SComboBase(void);

    /**
     * SComboBoxBase::~SComboBoxBase
     * @brief    析构函数
     *
     * Describe  析构函数
     */
    virtual ~SComboBase(void);

  public:
    STDMETHOD_(int, FindString)
    (THIS_ LPCTSTR pszFind, int nAfter = -1, BOOL bPartMatch = TRUE) OVERRIDE;
	STDMETHOD_(int, FindStringA)
		(THIS_ LPCSTR pszFind, int nAfter = -1, BOOL bPartMatch = TRUE) OVERRIDE{
			SStringT str = S_CA2T(pszFind,CP_UTF8);
			return FindString(str,nAfter,bPartMatch);
	}

    STDMETHOD_(void, DropDown)(THIS) OVERRIDE;

    STDMETHOD_(void, CloseUp)(THIS) OVERRIDE;

    STDMETHOD_(BOOL, IsDropdown)(THIS) SCONST OVERRIDE;

    STDMETHOD_(void, SetDropdown)(THIS_ BOOL bDropdown) OVERRIDE;

  public:
    STDMETHOD_(void, SetWindowText)(LPCTSTR pszText) OVERRIDE;

    STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE *psz,int nParentWid, int nParentHei) OVERRIDE;

    STDMETHOD_(void, SetFocus)(THIS) OVERRIDE;

    STDMETHOD_(BOOL, FireEvent)(THIS_ IEvtArgs *evt) OVERRIDE;

    SStringT GetWindowText(BOOL bRawText = FALSE) OVERRIDE;

    SStringT GetLBText(int iItem, BOOL bRawText = FALSE);

  protected:
    /**
     * SComboBoxBase::GetDropDownOwner
     * @brief    获取owner
     * @return   SWindow
     *
     * Describe  获取owner
     */
    virtual SWindow *GetDropDownOwner();

    /**
     * SComboBoxBase::OnCreateDropDown
     * @brief    下拉事件
     * @param     SDropDownWnd *pDropDown -- 下拉窗口指针
     *
     * Describe  下拉事件
     */
    virtual void OnCreateDropDown(SDropDownWnd *pDropDown);

    /**
     * SComboBoxBase::OnDestroyDropDown
     * @brief    下拉事件
     * @param     SDropDownWnd *pDropDown -- 下拉窗口指针
     * @param     UINT uCode -- 消息码
     *
     * Describe  下拉关闭
     */
    virtual void OnDestroyDropDown(SDropDownWnd *pDropDown);

    /**
     * SComboBoxBase::OnSelChanged
     * @brief    下拉窗口改变事件
     *
     * Describe  下拉关闭
     */
    virtual void OnSelChanged();

  protected:
    /**
     * SComboBoxBase::CalcPopupRect
     * @brief    计算弹出窗口位置
     * @param    int nHeight -- 下拉窗口高度
     * @param    CRect & rcPopup -- 保存弹出窗口Rect
     * @return   BOOL  TRUE -- 成功  FALSE -- 失败
     *
     * Describe  计算弹出窗口位置,保存在rcPopup中
     */
    BOOL CalcPopupRect(int nHeight, CRect &rcPopup);

    /**
     * SComboBoxBase::CreateListBox
     * @brief    创建下拉列表
     * @param    SXmlNode xmlNode  -- xml对象
     * @return   BOOL  TRUE -- 成功  FALSE -- 失败
     *
     * Describe  创建下拉列表
     */
    virtual BOOL CreateListBox(SXmlNode xmlNode) = 0;

    /**
     * SComboBoxBase::GetListBoxHeight
     * @brief    获取下拉列表高度
     * @return   返回int 高度
     *
     * Describe  获取下拉列表高度
     */
    virtual int GetListBoxHeight() = 0;

    /**
     * SComboBoxBase::GetDropBtnRect
     * @brief    获取下拉列表按钮位置
     * @param    LPRECT prc -- 按钮Rect
     *
     * Describe  获取下拉列表右侧按钮位置
     */
    void GetDropBtnRect(LPRECT prc);
    /**
     * SComboBoxBase::LoadChildren
     * @brief    加载子项
     * @param    SXmlNode xmlNode  -- xml文件
     * @return   返回BOOL  TRUE -- 成功 FALSE -- 失败
     *
     * Describe  加载子项
     */
    virtual BOOL CreateChildren(SXmlNode xmlNode);
    /**
     * SComboBoxBase::GetTextRect
     * @brief    获取文本位置
     * @param    LPRECT pRect -- 文本位置
     *
     * Describe  获取文本位置
     */
    virtual void GetTextRect(LPRECT pRect);

    virtual void OnColorize(COLORREF cr);

    virtual HRESULT OnLanguageChanged();

    virtual void OnScaleChanged(int nScale);

    /**
     * SComboBoxBase::OnPaint
     * @brief    绘制消息
     * @param    IRenderTarget * pRT -- 暂无
     *
     * Describe  此函数是消息响应函数
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * SComboBoxBase::OnLButtonDown
     * @brief    左键按下事件
     * @param    UINT nFlags -- 标志
     * @param    CPoint point -- 鼠标坐标
     *
     * Describe  此函数是消息响应函数
     */
    void OnLButtonDown(UINT nFlags, CPoint pt);

    /**
     * SComboBoxBase::OnMouseMove
     * @brief    键盘鼠标移动事件
     * @param    UINT nFlags -- 标志
     * @param    CPoint point -- 鼠标坐标
     *
     * Describe  此函数是消息响应函数
     */
    void OnMouseMove(UINT nFlags, CPoint pt);

    /**
     * SComboBoxBase::OnMouseLeave
     * @brief    键盘鼠标移动事件
     *
     * Describe  此函数是消息响应函数
     */
    void OnMouseLeave();

    /**
     * SComboBoxBase::OnKeyDown
     * @brief    键盘按下事件
     * @param    UINT nChar -- 按键对应的码值
     * @param    UINT nRepCnt -- 重复次数
     * @param    UINT nFlags -- 标志
     *
     * Describe  此函数是消息响应函数
     */
    void OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags);

    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    /**
     * SComboBoxBase::OnChar
     * @brief    字符消息
     * @param    UINT nChar -- 按键对应的码值
     * @param    UINT nRepCnt -- 重复次数
     * @param    UINT nFlags -- 标志
     *
     * Describe  此函数是消息响应函数
     */
    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * SComboBoxBase::OnDestroy
     * @brief    下拉窗口销毁
     *
     * Describe  此函数是用于销毁下拉窗口
     */
    void OnDestroy();
    /**
     * SComboBoxBase::OnGetDlgCode
     * @brief    获取消息码
     *
     * Describe  获取消息码
     */
    virtual UINT WINAPI OnGetDlgCode() const;

    /**
     * SComboBoxBase::IsFocusable
     * @brief    是否禁止TAB键
     *
     * Describe  是否禁止TAB键
     */
    BOOL WINAPI IsFocusable() const;

    STDMETHOD_(void, UpdateChildrenPosition)(THIS) OVERRIDE;

    void OnKillFocus(SWND wndFocus);

    HRESULT OnAttrDropDown(const SStringW &strValue, BOOL bLoading);

    SStringT GetCueText(BOOL bRawText = FALSE) const;

    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"dropDown", OnAttrDropDown)
        ATTR_LAYOUTSIZE(L"dropHeight", m_nDropHeight, TRUE)
        ATTR_INT(L"curSel", m_iInitSel, FALSE)
        ATTR_SKIN(L"btnSkin", m_pSkinBtn, TRUE)
        ATTR_INT(L"animateTime", m_nAnimTime, FALSE)
        ATTR_INT(L"autoFitDropBtn", m_bAutoFitDropBtn, TRUE)
        ATTR_COLOR(L"cueColor", m_crCue, TRUE)
        ATTR_I18NSTRT(L"cueText", m_strCue, TRUE)
        ATTR_BOOL(L"autoMatch", m_bAutoMatch, FALSE)
    SOUI_ATTRS_END()

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_CHAR(OnChar)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
    SOUI_MSG_MAP_END()

  protected:
    /**
     * SComboBoxBase::GetEditText
     * @brief    获取编辑框内容
     *
     * Describe  获取编辑框内容
     */
    SStringT GetEditText() const
    {
        if (!m_bDropdown)
        {
            return m_pEdit->GetWindowText();
        }
        else
        {
            return SStringT();
        }
    }
    SRichEdit *m_pEdit;               /**< SRichEdit指针 */
    DWORD m_dwBtnState;               /**< 按钮状态      */
    SAutoRefPtr<ISkinObj> m_pSkinBtn; /**< 按钮资源      */

    BOOL m_bDropdown;             /**< Editable or dropdown only   */
    SLayoutSize m_nDropHeight;    /**< 下拉框高度 */
    int m_nAnimTime;              /**< 动画时间   */
    int m_iInitSel;               /**< 默认选中索引 */
    SDropDownWnd *m_pDropDownWnd; /**< DropDown指针 */
    SXmlDoc m_xmlDropdownStyle;   /**< DropDown的style */
    BOOL m_bAutoFitDropBtn;       /**< 自适应下拉按钮大小*/
    COLORREF m_crCue;
    STrText m_strCue;

    SStringT m_strMatch;   /*快速输入用来定位下拉列表中的项*/
    DWORD m_LastPressTime; /*最后按键的时间,*/
    BOOL m_bAutoMatch;
    int m_nTextLength;
};

template <class T>
class TComboBaseProxy
    : public T
    , public SComboBase {
  public:
    STDMETHOD_(long, AddRef)(THIS) OVERRIDE
    {
        return SComboBase::AddRef();
    }
    STDMETHOD_(long, Release)(THIS) OVERRIDE
    {
        return SComboBase::Release();
    }
    STDMETHOD_(void, OnFinalRelease)(THIS) OVERRIDE
    {
        SComboBase::OnFinalRelease();
    }

    STDMETHOD_(IWindow *, ToIWindow)(THIS) OVERRIDE
    {
        return (SComboBase *)this;
    }
    STDMETHOD_(HRESULT, QueryInterface)(REFGUID id, IObjRef **ppRet) OVERRIDE
    {
        if (id == __uuidof(T))
        {
            *ppRet = (T *)this;
            AddRef();
            return S_OK;
        }
        else
        {
            return SComboBase::QueryInterface(id, ppRet);
        }
    }

    STDMETHOD_(int, FindString)
    (THIS_ LPCTSTR pszFind, int nAfter = -1, BOOL bPartMatch = TRUE) OVERRIDE
    {
        return SComboBase::FindString(pszFind, nAfter, bPartMatch);
    }

	STDMETHOD_(int, FindStringA)
		(THIS_ LPCSTR pszFind, int nAfter = -1, BOOL bPartMatch = TRUE) OVERRIDE
	{
		return SComboBase::FindStringA(pszFind, nAfter, bPartMatch);
	}

    STDMETHOD_(void, DropDown)(THIS) OVERRIDE
    {
        return SComboBase::DropDown();
    }

    STDMETHOD_(void, CloseUp)(THIS) OVERRIDE
    {
        return SComboBase::CloseUp();
    }

    STDMETHOD_(BOOL, IsDropdown)(THIS) SCONST OVERRIDE
    {
        return SComboBase::IsDropdown();
    }

    STDMETHOD_(void, SetDropdown)(THIS_ BOOL bDropdown) OVERRIDE
    {
        return SComboBase::SetDropdown(bDropdown);
    }
};

SNSEND