#ifndef _SEDIT_H_
#define _SEDIT_H_

#ifdef _WIN32
#include <control/SRichEdit.h>

SNSBEGIN
/**
 * @class      SEdit
 * @brief      ��edit�ؼ�
 *
 * Describe
 */
class SOUI_EXP SEdit : public TCtrlProxy<IEdit, SRichEdit> {
    DEF_SOBJECT(SRichEdit, L"edit")
  public:
    /**
     * SEdit::SEdit
     * @brief    ���캯��
     *
     * Describe  ���캯��
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
     * @brief    ������Ϣ
     * @param    IRenderTarget * pRT -- �滭�豸������
     *
     * Describe  �˺�������Ϣ��Ӧ����
     */
    void OnPaint(IRenderTarget *pRT);
    /**
     * SEdit::OnSetFocus
     * @brief    ��ý���
     *
     * Describe  �˺�������Ϣ��Ӧ����
     */
    void OnSetFocus(SWND wndOld);
    UINT GetCueTextAlign();
    /**
     * SEdit::OnKillFocus
     * @brief    ʧȥ����
     *
     * Describe  �˺�������Ϣ��Ӧ����
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
#else
#include <core/SWnd.h>
#include <interface/SCtrl-i.h>
#include <proxy/SWindowProxy.h>
#include <proxy/SCtrlProxy.h>

SNSBEGIN

class SOUI_EXP SEdit : public TWindowProxy<IEdit> {
	DEF_SOBJECT(SWindow, L"edit")
public:
  void SetSel(int begin, int end, BOOL bScroll);
};

SNSEND
#endif//_WIN32

#endif//_SEDIT_H_
