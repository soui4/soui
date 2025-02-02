#ifndef _SEDIT_H_
#define _SEDIT_H_
#include <control/SRichEdit.h>

SNSBEGIN
/**
 * @class      SEdit
 * @brief      简单edit控件
 *
 * Describe
 */
class SOUI_EXP SEdit : public TCtrlProxy<IEdit, SRichEdit> {
    DEF_SOBJECT(SRichEdit, L"edit")
  public:
    /**
     * SEdit::SEdit
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    SEdit();

  public:
    STDMETHOD_(void, GetCueText)(CTHIS_ IStringT *pStr) SCONST
    {
        SStringT str = GetCueText(FALSE);
        pStr->Copy(&str);
    }

    STDMETHOD_(COLORREF, GetCueColor)(CTHIS) SCONST
    {
        return m_crCue;
    }

  public:
    SStringT GetCueText(BOOL bRawText = FALSE) const;

    SOUI_ATTRS_BEGIN()
        ATTR_COLOR(L"cueColor", m_crCue, TRUE)
        ATTR_I18NSTRT(L"cueText", m_strCue, TRUE)
    SOUI_ATTRS_END()

  protected:
    virtual HRESULT OnLanguageChanged();

    /**
     * SEdit::OnPaint
     * @brief    绘制消息
     * @param    IRenderTarget * pRT -- 绘画设备上下文
     *
     * Describe  此函数是消息响应函数
     */
    void OnPaint(IRenderTarget *pRT);
    /**
     * SEdit::OnSetFocus
     * @brief    获得焦点
     *
     * Describe  此函数是消息响应函数
     */
    void OnSetFocus(SWND wndOld);
    UINT GetCueTextAlign();
    /**
     * SEdit::OnKillFocus
     * @brief    失去焦点
     *
     * Describe  此函数是消息响应函数
     */
    void OnKillFocus(SWND wndFocus);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_SETFOCUS_EX(OnSetFocus)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
    SOUI_MSG_MAP_END()

    COLORREF m_crCue;
    STrText m_strCue;
};

SNSEND

#endif //_SEDIT_H_
