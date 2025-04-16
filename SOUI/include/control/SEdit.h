#ifndef _SEDIT_H_
#define _SEDIT_H_

#include <control/SRichEdit.h>

SNSBEGIN

/**
 * @class SEdit
 * @brief Simple Edit Control
 * @details A simple edit control that supports cue text and color customization.
 */
class SOUI_EXP SEdit : public TCtrlProxy<IEdit, SRichEdit> {
    DEF_SOBJECT(SRichEdit, L"edit")

  public:
    /**
     * @brief Constructor
     */
    SEdit();

  public:
    /**
     * @brief Get the cue text
     * @param pStr Output string for the cue text
     */
    STDMETHOD_(void, GetCueText)(CTHIS_ IStringT *pStr) SCONST
    {
        SStringT str = GetCueText(FALSE);
        pStr->Copy(&str);
    }

    /**
     * @brief Get the cue text color
     * @return Cue text color
     */
    STDMETHOD_(COLORREF, GetCueColor)(CTHIS) SCONST
    {
        return m_crCue;
    }

  public:
    /**
     * @brief Get the cue text
     * @param bRawText Whether to get raw text
     * @return Cue text
     */
    SStringT GetCueText(BOOL bRawText = FALSE) const;

    SOUI_ATTRS_BEGIN()
        ATTR_COLOR(L"cueColor", m_crCue, TRUE)
        ATTR_I18NSTRT(L"cueText", m_strCue, TRUE)
    SOUI_ATTRS_END()

  protected:
    /**
     * @brief Handle language change event
     * @return HRESULT
     */
    virtual HRESULT OnLanguageChanged();

    /**
     * @brief Paint the control
     * @param pRT Rendering target handle
     * @details This function is a message handler.
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief Handle set focus event
     * @param wndOld Previous focus window handle
     * @details This function is a message handler.
     */
    void OnSetFocus(SWND wndOld);

    /**
     * @brief Get the alignment of the cue text
     * @return Alignment of the cue text
     */
    UINT GetCueTextAlign();

    /**
     * @brief Handle kill focus event
     * @param wndFocus New focus window handle
     * @details This function is a message handler.
     */
    void OnKillFocus(SWND wndFocus);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_SETFOCUS_EX(OnSetFocus)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
    SOUI_MSG_MAP_END()

  protected:
    COLORREF m_crCue; /**< Cue text color */
    STrText m_strCue; /**< Cue text */
};

SNSEND

#endif // _SEDIT_H_