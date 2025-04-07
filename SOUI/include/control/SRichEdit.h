/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserved.
 *
 * @file       SRichEdit.h
 * @brief      RichEdit控件
 * @version    v1.0
 * @author     soui
 * @date       2014-07-13
 *
 * Describe    RichEdit控件
 */
#ifndef __SRICHEDIT__H__
#define __SRICHEDIT__H__

#include <richedit.h>
#include <core/SPanel.h>
#include <proxy/SCtrlProxy.h>
#include <interface/STimer-i.h>

#include <imm.h>
#include <textserv.h>

SNSBEGIN
enum
{
    MENU_CUT = 1,
    MENU_COPY,
    MENU_PASTE,
    MENU_DEL,
    MENU_SELALL,
    //---------------
    MENU_USER = 20,
};

/**
 * @class      SRichEdit
 * @brief      使用Windowless Richedit实现的edit控件
 *
 * Describe    SRichEdit
 */
class STextHost;
class SOUI_EXP SRichEdit : public TPanelProxy<IRichEdit> {
    friend class STextHost;
    friend class SApplication;

  public:
    DEF_SOBJECT(SPanel, L"richedit")

    /**
     * SRichEdit::SRichEdit
     * @brief     构造函数
     *
     * Describe   构造函数
     */
    SRichEdit();

    /**
     * SRichEdit::~SRichEdit
     * @brief     析构函数
     *
     * Describe   析构函数
     */
    virtual ~SRichEdit()
    {
    }

  private:
    static void InitTextService();
    static void UninitTextService();

  public: // richedit interface
    STDMETHOD_(DWORD, SaveRtf)(THIS_ LPCTSTR pszFileName) OVERRIDE;

    STDMETHOD_(DWORD, LoadRtf)(THIS_ LPCTSTR pszFileName) OVERRIDE;

    /**
     * SRichEdit::SetSel
     * @brief     设置选中
     * @param     long nStartChar --
     * @param     long nEndChar --
     * @param     BOOL bNoScroll --
     *
     * Describe   设置选中, 支持超长文本
     */
    STDMETHOD_(void, SetSel)(THIS_ long nStartChar, long nEndChar, BOOL bNoScroll) OVERRIDE;
    /**
     * SRichEdit::ReplaceSel
     * @brief     替换选中项
     * @param     LPCWSTR pszText --
     * @param     BOOL bCanUndo --
     *
     * Describe   替换选中项
     */
    STDMETHOD_(void, ReplaceSel)(THIS_ LPCTSTR pszText, BOOL bCanUndo = TRUE) OVERRIDE;
    /**
     * SRichEdit::GetWordWrap
     * @brief
     * @return   返回BOOL
     *
     * Describe
     */
    STDMETHOD_(BOOL, GetWordWrap)(THIS) SCONST OVERRIDE;
    /**
     * SRichEdit::SetWordWrap
     * @brief
     * @param     BOOL fWordWrap --
     *
     * Describe
     */
    STDMETHOD_(void, SetWordWrap)(THIS_ BOOL fWordWrap) OVERRIDE;

    /**
     * SRichEdit::GetReadOnly
     * @brief    判断是否只读
     * @return   返回BOOL
     *
     * Describe  判断是否只读
     */
    STDMETHOD_(BOOL, GetReadOnly)(THIS) SCONST OVERRIDE;

    /**
     * SRichEdit::SetReadOnly
     * @brief     设置只读
     * @param     BOOL bReadOnly -- 是否只读
     * @return    返回BOOL
     *
     * Describe   设置只读
     */
    STDMETHOD_(BOOL, SetReadOnly)(THIS_ BOOL bReadOnly) OVERRIDE;

    /**
     * SRichEdit::GetLimitText
     * @brief     获取最小文本长度
     * @return    返回BOOL
     *
     * Describe   获取最小文本长度
     */
    STDMETHOD_(LONG, GetLimitText)(THIS) SCONST OVERRIDE;

    /**
     * SRichEdit::SetLimitText
     * @brief     设置最小文本长度
     * @param     int nLength -- 长度
     * @return    返回BOOL
     *
     * Describe   设置最小文本长度
     */
    STDMETHOD_(BOOL, SetLimitText)(THIS_ int nLength) OVERRIDE;

    /**
     * SRichEdit::GetDefaultAlign
     * @brief     获取对齐方式
     * @return    返回WORD
     *
     * Describe   设置最小文本长度
     */
    STDMETHOD_(WORD, GetDefaultAlign)(THIS) SCONST OVERRIDE;

    /**
     * SRichEdit::SetDefaultAlign
     * @brief     设置默认对齐方式
     * @param     WORD wNewAlign -- 对齐方式
     *
     * Describe   设置默认对齐方式
     */
    STDMETHOD_(void, SetDefaultAlign)(THIS_ WORD wNewAlign) OVERRIDE;

    /**
     * SRichEdit::GetRichTextFlag
     * @brief     获取标志
     * @return    返回BOOL
     *
     * Describe   获取标志
     */
    STDMETHOD_(BOOL, GetRichTextFlag)(THIS) SCONST OVERRIDE;

    /**
     * SRichEdit::SetRichTextFlag
     * @brief     设置标志
     * @param     BOOL fRich -- 标志
     *
     * Describe   设置标志
     */
    STDMETHOD_(void, SetRichTextFlag)(THIS_ BOOL fRich) OVERRIDE;

    /**
     * SRichEdit::SetRichTextFlag
     * @brief     设置标志
     * @param     BOOL fRich -- 标志
     * @return    返回LONG
     *
     * Describe   设置标志
     */
    STDMETHOD_(LONG, GetDefaultLeftIndent)(THIS) SCONST OVERRIDE;

    /**
     * SRichEdit::SetDefaultLeftIndent
     * @brief     设置缩进
     * @param     LONG lNewIndent -- 缩进字符数
     *
     * Describe   设置缩进
     */
    STDMETHOD_(void, SetDefaultLeftIndent)(THIS_ LONG lNewIndent) OVERRIDE;

    /**
     * SRichEdit::SetSaveSelection
     * @brief
     * @param     BOOL fSaveSelection
     * @return    返回BOOL
     *
     * Describe
     */
    STDMETHOD_(BOOL, SetSaveSelection)(THIS_ BOOL fSaveSelection) OVERRIDE;

    /**
     * SRichEdit::SetDefaultTextColor
     * @brief     默认文本颜色
     * @param     COLORREF cr -- 颜色
     * @return    返回COLORREF
     *
     * Describe   设置默认文本颜色
     */
    STDMETHOD_(COLORREF, SetDefaultTextColor)(THIS_ COLORREF cr) OVERRIDE;

    /**
     * SRichEdit::SetWindowText
     * @brief     设置窗口标题
     * @param     LPCWSTR lpszText -- 窗口标题
     *
     * Describe   设置窗口标题
     */
    STDMETHOD_(void, SetWindowText)(LPCTSTR lpszText) OVERRIDE;

    STDMETHOD_(int, GetWindowText)(THIS_ TCHAR *pBuf, int nBufLen, BOOL bRawText) OVERRIDE;

    STDMETHOD_(BOOL, SwndProc)(THIS_ UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lResult) OVERRIDE;

    STDMETHOD_(BOOL, CreateCaret)(THIS_ HBITMAP pBmp, int nWid, int nHeight) OVERRIDE;

  public:
    /**
     * SRichEdit::GetWindowText
     * @brief     获取窗口标题
     * @return    返回SStringT
     *
     * Describe   获取窗口标题
     */
    SStringT GetWindowText(BOOL bRawText = FALSE);

    /**
     * SRichEdit::SetSel
     * @brief     设置选中
     * @param     DWORD dwSelection --
     * @param     BOOL bNoScroll --
     *
     * Describe   设置选中
     */
    void SetSel(DWORD dwSelection, BOOL bNoScroll = FALSE);

    int GetWindowTextLength() const;

  protected:
    /**
     * SRichEdit::OnCreate
     * @brief    创建
     * @return   LRESULT
     *
     * Describe  此函数是消息响应函数
     */
    int OnCreate(LPVOID);
    /**
     * SRichEdit::OnDestroy
     * @brief    销毁窗口
     *
     * Describe  消息响应函数
     */
    void OnDestroy();
    /**
     * SRichEdit::OnPaint
     * @brief    绘制消息
     * @param    IRenderTarget * pRT -- 绘画设备上下文
     *
     * Describe  此函数是消息响应函数
     */
    void OnPaint(IRenderTarget *pRT);
    /**
     * SRichEdit::OnSetFocus
     * @brief    获得焦点
     *
     * Describe  此函数是消息响应函数
     */
    void OnSetFocus(SWND wndOld);
    /**
     * SRichEdit::OnKillFocus
     * @brief    失去焦点
     *
     * Describe  此函数是消息响应函数
     */
    void OnKillFocus(SWND wndFocus);
    /**
     * SRichEdit::OnTimer
     * @brief    定时器
     * @param    char idEvent idEvent
     *
     * Describe  定时器
     */
    void OnTimer(char idEvent);

    /**
     * SRichEdit::OnGetDlgCode
     * @brief    获取窗口消息码
     * @return   返回UINT
     *
     * Describe  获取窗口消息码
     */
    virtual UINT WINAPI OnGetDlgCode() const;

    /**
     * SRichEdit::OnScroll
     * @brief    滚动条事件
     * @param    BOOL bVertical -- 是否垂直滚动
     * @param    UINT uCode -- 消息码
     * @param    int nPos -- 位置
     * @return   返回BOOL
     *
     * Describe  滚动条事件
     */
    virtual BOOL OnScroll(BOOL bVertical, UINT uCode, int nPos);
    /**
     * SRichEdit::OnSetCursor
     * @brief    设置鼠标位置
     * @param    const CPoint &pt -- 坐标
     * @return   返回BOOL
     *
     * Describe  设置鼠标位置
     */
    virtual BOOL OnSetCursor(const CPoint &pt);

    virtual void OnScaleChanged(int nScale);

    virtual void OnRebuildFont();

    BOOL IsRichScale() const;

    void OnEnable(BOOL bEnable, UINT nStatus);
    /**
     * SRichEdit::InitDefaultCharFormat
     * @brief
     * @param    CHARFORMAT2W* pcf --
     * @param    IFont *pFont --
     * @return   返回HRESULT
     *
     * Describe
     */
    HRESULT InitDefaultCharFormat(CHARFORMAT2W *pcf, IFontS *pFont = NULL);
    /**
     * SRichEdit::InitDefaultParaFormat
     * @brief
     * @param    PARAFORMAT2* ppf
     * @return   返回HRESULT
     *
     * Describe
     */
    HRESULT InitDefaultParaFormat(PARAFORMAT2 *ppf);
    /**
     * SRichEdit::OnTxNotify
     * @brief
     * @param    DWORD iNotify --
     * @param    LPVOID pv  --
     * @return   返回HRESULT
     *
     * Describe
     */

    virtual HRESULT OnTxNotify(DWORD iNotify, LPVOID pv);

    /**
     * SRichEdit::OnLButtonDown
     * @brief    左键按下事件
     * @param    UINT nFlags -- 标志
     * @param    CPoint point -- 鼠标坐标
     *
     * Describe  此函数是消息响应函数
     */
    void OnLButtonDown(UINT nFlags, CPoint point);
    void OnLButtonUp(UINT nFlags, CPoint point);

    /**
     * SRichEdit::OnRButtonDown
     * @brief    右键按下事件
     * @param    UINT nFlags -- 标志
     * @param    CPoint point -- 鼠标坐标
     *
     * Describe  此函数是消息响应函数
     */
    void OnRButtonDown(UINT nFlags, CPoint point);

    /**
     * SRichEdit::OnMouseMove
     * @brief    鼠标移动
     * @param    UINT nFlags -- 标志
     * @param    CPoint point -- 鼠标坐标
     *
     * Describe  此函数是消息响应函数
     */
    void OnMouseMove(UINT nFlags, CPoint point);

    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    /**
     * SRichEdit::OnKeyDown
     * @brief    键盘按下事件
     * @param    UINT nChar -- 按键对应的码值
     * @param    UINT nRepCnt -- 重复次数
     * @param    UINT nFlags -- 标志
     *
     * Describe  此函数是消息响应函数
     */
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * SRichEdit::OnButtonClick
     * @brief    通用鼠标事件
     * @param    UINT uMsg -- msg
     * @param    WPARAM wParam -- wParam
     * @param    LPARAM lParam -- lParam
     * @return   返回LRESULT
     *
     * Describe  此函数是消息响应函数
     */
    LRESULT OnButtonClick(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * SRichEdit::OnChar
     * @brief    字符事件
     * @param    UINT nChar -- 按键对应的码值
     * @param    UINT nRepCnt -- 重复次数
     * @param    UINT nFlags -- 标志
     *
     * Describe  此函数是消息响应函数
     */
    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    /**
     * SRichEdit::OnSetFont
     * @brief    设置字体
     * @param    IFont * font -- 字体
     * @param    BOOL bRedraw -- 是否重绘
     *
     * Describe  设置字体
     */
    void OnSetFont(IFontS *font, BOOL bRedraw);
    /**
     * SRichEdit::OnSetText
     * @brief    设置文本
     * @param    UINT uMsg -- 消息码
     * @param    WPARAM wparam --
     * @param    LPARAM lparam --
     * @return   返回LRESULT
     *
     * Describe  设置文本
     */
    LRESULT OnSetText(UINT uMsg, WPARAM wparam, LPARAM lparam);
    /**
     * SRichEdit::OnSetCharFormat
     * @brief
     * @param    UINT uMsg -- 消息码
     * @param    WPARAM wparam --
     * @param    LPARAM lparam --
     * @return   返回LRESULT
     *
     * Describe
     */
    LRESULT OnSetCharFormat(UINT uMsg, WPARAM wParam, LPARAM lParam);
    /**
     * SRichEdit::OnSetParaFormat
     * @brief
     * @param    UINT uMsg -- 消息码
     * @param    WPARAM wparam --
     * @param    LPARAM lparam --
     * @return   返回LRESULT
     *
     * Describe
     */
    LRESULT OnSetParaFormat(UINT uMsg, WPARAM wParam, LPARAM lParam);
    /**
     * SRichEdit::OnSetReadOnly
     * @brief    设置只读
     * @param    UINT uMsg -- 消息码
     * @param    WPARAM wparam --
     * @param    LPARAM lparam --
     * @return   返回LRESULT
     *
     * Describe  设置只读
     */
    LRESULT OnSetReadOnly(UINT uMsg, WPARAM wParam, LPARAM lParam);
    /**
     * SRichEdit::OnSetLimitText
     * @brief    设置文本长度
     * @param    UINT uMsg -- 消息码
     * @param    WPARAM wparam --
     * @param    LPARAM lparam --
     * @return   返回LRESULT
     *
     * Describe  设置文本长度
     */
    LRESULT OnSetLimitText(UINT uMsg, WPARAM wParam, LPARAM lParam);
    /**
     * SRichEdit::OnNcCalcSize
     * @brief    计算非客户大小
     * @param    BOOL bCalcValidRects --
     * @param    LPARAM lparam --
     * @return   返回LRESULT
     *
     * Describe  消息响应函数
     */
    LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam);
    /**
     * SRichEdit::OnEnableDragDrop
     * @brief    是否允许拖拽
     * @param    BOOL bEnable
     *
     * Describe  是否允许拖拽
     */
    void OnEnableDragDrop(BOOL bEnable);

    LRESULT OnGetRect(UINT uMsg, WPARAM wp, LPARAM lp);

    BOOL OnTxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw);

    BOOL OnTxSetTimer(UINT idTimer, UINT uTimeout);
    void OnTxKillTimer(UINT idTimer);
    BOOL OnTimeout(IEvtArgs *e);

  protected:
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_NCCALCSIZE(OnNcCalcSize)
        MSG_WM_SETFOCUS_EX(OnSetFocus)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
        MSG_WM_TIMER_EX(OnTimer)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_RBUTTONDOWN(OnRButtonDown)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_CHAR(OnChar)
        MSG_WM_SETFONT_EX(OnSetFont)
        MESSAGE_HANDLER_EX(WM_LBUTTONDBLCLK, OnButtonClick)
        MESSAGE_HANDLER_EX(WM_SETTEXT, OnSetText)
        MESSAGE_HANDLER_EX(EM_SETPARAFORMAT, OnSetParaFormat)
        MESSAGE_HANDLER_EX(EM_SETCHARFORMAT, OnSetCharFormat)
        MESSAGE_HANDLER_EX(EM_SETREADONLY, OnSetReadOnly)
        MESSAGE_HANDLER_EX(EM_EXLIMITTEXT, OnSetLimitText)
        MESSAGE_HANDLER_EX(EM_GETRECT, OnGetRect)
    SOUI_MSG_MAP_END()

  protected:
#define ATTR_RE_STYLE(attr, style, txtBit, func)        \
    if (0 == strAttribName.CompareNoCase(attr))         \
    {                                                   \
        hRet = func(strValue, style, txtBit, bLoading); \
    }                                                   \
    else

    /**
     * SRichEdit::OnAttrTextColor
     * @brief    设置文本颜色
     * @param    const SStringW &  strValue -- 字符串
     * @param    BOOL bLoading -- 是否加载
     * @return   返回HRESULT
     *
     * Describe  设置文本颜色
     */
    HRESULT OnAttrTextColor(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrRTF(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrAlign(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrNotifyChange(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrPasswordChar(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrEnableDragdrop(const SStringW &strValue, BOOL bLoading);

    HRESULT OnAttrReStyle(const SStringW &strValue, DWORD dwStyle, DWORD txtBit, BOOL bLoading);
    HRESULT OnAttrReStyle2(const SStringW &strValue, DWORD dwStyle, DWORD txtBit, BOOL bLoading);

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"style", m_dwStyle, TRUE)
        ATTR_INT(L"maxBuf", m_cchTextMost, FALSE)
        ATTR_BOOL(L"transparent", m_fTransparent, TRUE)
        ATTR_BOOL(L"rich", m_fRich, TRUE)
        ATTR_BOOL(L"vertical", m_fVertical, TRUE)
        ATTR_BOOL(L"wordWrap", m_fWordWrap, TRUE)
        ATTR_BOOL(L"allowBeep", m_fAllowBeep, FALSE)
        ATTR_BOOL(L"autoWordSel", m_fEnableAutoWordSel, TRUE)
        ATTR_BOOL(L"vcenter", m_fSingleLineVCenter, TRUE)
        ATTR_BOOL(L"disableCaret", m_fDisableCaret, TRUE)
        ATTR_BOOL(L"wantTab", m_fWantTab, FALSE)
        ATTR_BOOL(L"autoSel", m_fAutoSel, FALSE)
        ATTR_CUSTOM(L"colorText", OnAttrTextColor)
        ATTR_CUSTOM(L"rtf", OnAttrRTF)
        ATTR_CUSTOM(L"align", OnAttrAlign)
        ATTR_CUSTOM(L"notifyChange", OnAttrNotifyChange)
        ATTR_RE_STYLE(L"wantReturn", ES_WANTRETURN, 0, OnAttrReStyle)
        ATTR_RE_STYLE(L"number", ES_NUMBER, 0, OnAttrReStyle)
        ATTR_RE_STYLE(L"upperCase", ES_UPPERCASE, 0, OnAttrReStyle)
        ATTR_RE_STYLE(L"lowerCase", ES_LOWERCASE, 0, OnAttrReStyle)
        ATTR_RE_STYLE(L"password", ES_PASSWORD, TXTBIT_USEPASSWORD, OnAttrReStyle)
        ATTR_RE_STYLE(L"readOnly", ES_READONLY, TXTBIT_READONLY, OnAttrReStyle)
        ATTR_RE_STYLE(L"multiLines", ES_MULTILINE, TXTBIT_MULTILINE, OnAttrReStyle)
        ATTR_RE_STYLE(L"autoHscroll", ES_AUTOHSCROLL, TXTBIT_SCROLLBARCHANGE, OnAttrReStyle2)
        ATTR_RE_STYLE(L"autoVscroll", ES_AUTOVSCROLL, TXTBIT_SCROLLBARCHANGE, OnAttrReStyle2)
        ATTR_RE_STYLE(L"hscrollBar", WS_HSCROLL, TXTBIT_SCROLLBARCHANGE, OnAttrReStyle2)
        ATTR_RE_STYLE(L"vscrollBar", WS_VSCROLL, TXTBIT_SCROLLBARCHANGE, OnAttrReStyle2)
        ATTR_CUSTOM(L"passwordChar", OnAttrPasswordChar)
        ATTR_CUSTOM(L"enableDragdrop", OnAttrEnableDragdrop)
    SOUI_ATTRS_END()
  protected:
    CHARFORMAT2W m_cfDef;   /**< Default character format  */
    PARAFORMAT2 m_pfDef;    /**< Default paragraph format  */
    DWORD m_cchTextMost;    /**< Maximize Characters       */
    TCHAR m_chPasswordChar; /**< Password character        */
    LONG m_lAccelPos;       /**< Accelerator position      */
    SIZEL m_sizelExtent;    /**< Extent array              */
    CRect m_rcInset;        /**< inset margin              */
    int m_nFontHeight;      /**< 单行文字输出高度          */
    DWORD m_dwStyle;

    UINT m_fEnableAutoWordSel : 1;              /**< enable Word style auto word selection?  */
    UINT m_fWordWrap : 1;                       /**< Whether control should word wrap */
    UINT m_fRich : 1;                           /**< Whether control is rich text */
    UINT m_fSaveSelection : 1;                  /**< Whether to save the selection when inactive */
    UINT m_fTransparent : 1;                    /**< Whether control is transparent */
    UINT m_fVertical : 1;                       /**< Whether control is layout following vertical */
    UINT m_fAllowBeep : 1;                      /**< Whether message beep is allowed in the control */
    UINT m_fWantTab : 1;                        /**< Whether control will deal with tab input */
    UINT m_fSingleLineVCenter : 1;              /**< Whether control that is single line will be vertical
                                                   centered */
    UINT m_fScrollPending : 1;                  /**< Whether scroll is activated by richedit or by panelex */
    UINT m_fEnableDragDrop : 1;                 /**< 允许在该控件中使用拖放 */
    UINT m_fAutoSel : 1;                        /**< 有焦点时自动全选 */
    UINT m_fNotifyChange : 1;                   /**< receive re_notify when data changed */
    UINT m_fDisableCaret : 1;                   /**< disable caret flag */
    BYTE m_byDbcsLeadByte;                      /**< DBCS输入时的中文头字节*/
    SStringW m_strRtfSrc;                       /**< 在XML中指定的RTF数据源*/
    STextHost *m_pTxtHost;                      /**< Host of Richedit*/
    SMap<UINT, SAutoRefPtr<ITimer>> m_mapTimer; /**< map of timer to id*/
};

SNSEND

#endif // __SRICHEDIT__H__