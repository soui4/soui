#ifndef _SEDIT_H_
#define _SEDIT_H_

#include <core/SPanel.h>
#include <core/SWnd.h>
#include <proxy/SPanelProxy.h>
#include <interface/SCtrls-i.h>
#include <event/SEvents.h>
#include <souicoll.h>

/*
 * The RichEdit-backed implementation needs SRichEdit.h / SCtrlProxy.h.
 * They MUST be included at file scope, BEFORE SNSBEGIN opens namespace SOUI:
 * those headers open the namespace themselves, so including them from inside
 * it would nest SOUI::SOUI and break every SNS:: qualified type used by the
 * attribute-table and message-map macros.
 */
#if !defined(SOUI_EDIT_LITE)
#include <control/SRichEdit.h>
#include <proxy/SCtrlProxy.h>
#else
/*
 * In the RichEdit-backed build, SouiCtrls.h pulls <richedit.h> in through
 * SRichEdit.h, so every translation unit sees the Win32 edit declarations
 * (TEXTRANGEA, EM_GETTEXTRANGE, EM_SETEVENTMASK, ENM_CHANGE, ...).  Some
 * existing code (e.g. controls.extend/ScintillaWnd.cpp) relies on that
 * transitive include.  Keep the same declarations visible in the lite build
 * so those translation units need no changes.  This is a declarations-only
 * header: it does NOT link the RichEdit engine.
 */
#include <richedit.h>
#endif

SNSBEGIN

/**
 * @class SEdit
 * @brief Edit Control (<edit>)
 * @details
 *  The public class name SEdit is shared by two implementations, selected at
 *  compile time through the SOUI_EDIT_LITE macro:
 *   - Default (RichEdit-backed): SEdit is a TCtrlProxy<IEdit, SRichEdit> that
 *     wraps the (large) RichEdit engine and adds cue-text support on top.
 *   - Lightweight (SOUI_EDIT_LITE defined, e.g. for resource-constrained
 *     FreeRTOS builds): SEdit is the self-contained plain-text implementation
 *     implemented in SEditLite.cpp.
 *  In both cases the XML tag <edit> and the public IEdit surface stay identical,
 *  so code that depends on SEdit (combo box, inplace edit, property grid, ...)
 *  never needs to change.
 */

#if defined(SOUI_EDIT_LITE)

#ifndef EM_SETEVENTMASK
#define EM_SETEVENTMASK 0x0445 // WM_USER + 69
#endif
#ifndef ENM_CHANGE
#define ENM_CHANGE 0x00000001
#endif

/**
 * @class SEdit
 * @brief Lightweight plain-text edit control (no RichEdit / ITextServices).
 * @details
 *  This is a self-contained single/multi-line plain-text editor implemented
 *  directly on top of SPanel + the DUI render target. It mirrors the XML
 *  attribute surface of SRichEdit (the RichEdit-backed <edit>) and implements
 *  the plain-text-relevant part of IRichEdit so that it can drop in as the
 *  backing implementation of the <edit> tag. It is meant for platforms/products
 *  that do not want to link the (large) RichEdit engine, e.g. resource
 *  constrained FreeRTOS builds.
 *
 *  Rich-text-only features (RTF load/save, rich text flag, char/paragraph
 *  formatting, vertical text, IME composition underline, drag & drop) are
 *  intentionally unsupported and degrade to no-ops. Clipboard is an in-process
 *  static buffer (no OS clipboard on bare metal).
 */
class SOUI_EXP SEdit
    : public IEdit
    , public TPanelProxy<IRichEdit> {
    friend class SApplication;

  public:
    DEF_SOBJECT(SPanel, L"edit")

    SEdit();
    virtual ~SEdit();

    // ----- IWindow text overrides (own storage) -----
    STDMETHOD_(void, SetWindowText)(LPCTSTR lpszText) OVERRIDE;
    STDMETHOD_(int, GetWindowText)(THIS_ TCHAR *pBuf, int nBufLen, BOOL bRawText) SCONST OVERRIDE;
    SStringT GetWindowText(BOOL bRawText = FALSE) SCONST OVERRIDE;

    int GetWindowTextLength() const;

    // ----- IRichEdit: plain-text relevant -----
    STDMETHOD_(void, SetSel)(THIS_ long nStartChar, long nEndChar, BOOL bNoScroll) OVERRIDE;

    /**
     * @brief Set the selection range (packed DWORD form).
     * @details Mirrors SRichEdit::SetSel(DWORD, BOOL). SRichEdit exposes this
     *  2-argument convenience overload in addition to IRichEdit::SetSel(long,
     *  long, BOOL), and existing controls (SMaskEdit / SDateTimeEdit) call it.
     *  Keeping it here lets those controls compile unchanged under SOUI_EDIT_LITE.
     */
    void SetSel(DWORD dwSelection, BOOL bNoScroll = FALSE);

    STDMETHOD_(void, ReplaceSel)(THIS_ LPCTSTR pszText, BOOL bCanUndo DEF_VAL(TRUE)) OVERRIDE;
    STDMETHOD_(BOOL, GetWordWrap)(THIS) SCONST OVERRIDE;
    STDMETHOD_(void, SetWordWrap)(THIS_ BOOL fWordWrap) OVERRIDE;
    STDMETHOD_(BOOL, GetReadOnly)(THIS) SCONST OVERRIDE;
    STDMETHOD_(BOOL, SetReadOnly)(THIS_ BOOL bReadOnly) OVERRIDE;
    STDMETHOD_(LONG, GetLimitText)(THIS) SCONST OVERRIDE;
    STDMETHOD_(BOOL, SetLimitText)(THIS_ int nLength) OVERRIDE;
    STDMETHOD_(WORD, GetDefaultAlign)(THIS) SCONST OVERRIDE;
    STDMETHOD_(void, SetDefaultAlign)(THIS_ WORD wNewAlign) OVERRIDE;
    STDMETHOD_(BOOL, GetRichTextFlag)(THIS) SCONST OVERRIDE;
    STDMETHOD_(void, SetRichTextFlag)(THIS_ BOOL fRich) OVERRIDE;
    STDMETHOD_(LONG, GetDefaultLeftIndent)(THIS) SCONST OVERRIDE;
    STDMETHOD_(void, SetDefaultLeftIndent)(THIS_ LONG lNewIndent) OVERRIDE;
    STDMETHOD_(BOOL, SetSaveSelection)(THIS_ BOOL fSaveSelection) OVERRIDE;
    STDMETHOD_(COLORREF, SetDefaultTextColor)(THIS_ COLORREF cr) OVERRIDE;

    // ----- IRichEdit: rich-text only -> no-op -----
    STDMETHOD_(DWORD, SaveRtf)(THIS_ LPCTSTR pszFileName) OVERRIDE;
    STDMETHOD_(DWORD, LoadRtf)(THIS_ LPCTSTR pszFileName) OVERRIDE;

    // ----- members shared with SEdit.cpp (so the proxy layer is unchanged) -----
    DWORD m_dwStyle;
    UINT m_fRich : 1;
    UINT m_fAutoSel : 1;

  protected:
    // ----- message handlers -----
    int OnCreate(LPVOID lpCreateStruct);
    void OnDestroy();
    void OnPaint(IRenderTarget *pRT);
    void OnSetFocus(SWND wndOld);
    void OnKillFocus(SWND wndFocus);
    void OnSize(UINT nType, CSize size);
    void OnLButtonDown(UINT nFlags, CPoint point);
    void OnLButtonDblClk(UINT nFlags, CPoint point);
    void OnLButtonUp(UINT nFlags, CPoint point);
    void OnMouseMove(UINT nFlags, CPoint point);
    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    LRESULT OnImeChar(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnGetSel(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnSetSel(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnReplaceSel(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnSetEventMask(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OnSetFont(IFontS *font, BOOL bRedraw);
    void OnEnable(BOOL bEnable, UINT nStatus);

    // ----- virtual overrides (chained to SPanel by default) -----
    virtual UINT WINAPI OnGetDlgCode() const OVERRIDE;
    virtual BOOL OnScroll(BOOL bVertical, UINT uCode, int nPos) OVERRIDE;
    virtual int GetScrollLineSize(BOOL bVertical) OVERRIDE;
    virtual void OnScaleChanged(int nScale) OVERRIDE;
    virtual void OnRebuildFont() OVERRIDE;
    virtual HRESULT OnLanguageChanged() OVERRIDE;

  public:
    // ----- cue text (hint text), kept in the control itself for the lite build -----
    STDMETHOD_(void, GetCueText)(CTHIS_ IStringT *pStr) SCONST OVERRIDE;
    STDMETHOD_(COLORREF, GetCueColor)(CTHIS) SCONST OVERRIDE
    {
        return m_crCue;
    }
    SStringT GetCueText(BOOL bRawText = FALSE) const;
    UINT GetCueTextAlign();

    // ----- IEdit identity / lifetime -----
    // IEdit (via ICtrl/IWindow/IObject/IObjRef) re-declares AddRef/Release/
    // OnFinalRelease/ToIWindow as pure. Because SEdit inherits IEdit directly
    // (rather than through TCtrlProxy<IEdit, ...>), it must supply the same
    // forwarding overrides TCtrlProxy would -- otherwise SEdit stays abstract.
    // Forward to the SPanel-backed implementation so the refcount and the
    // window identity match the RichEdit-backed proxy exactly.
    STDMETHOD_(long, AddRef)(THIS) OVERRIDE
    {
        return TPanelProxy<IRichEdit>::AddRef();
    }
    STDMETHOD_(long, Release)(THIS) OVERRIDE
    {
        return TPanelProxy<IRichEdit>::Release();
    }
    STDMETHOD_(void, OnFinalRelease)(THIS) OVERRIDE
    {
        return TPanelProxy<IRichEdit>::OnFinalRelease();
    }
    STDMETHOD_(IWindow *, ToIWindow)(THIS) OVERRIDE
    {
        return (IWindow *)this;
    }

    STDMETHOD_(HRESULT, QueryInterface)(REFGUID id, IObjRef **ppRet) OVERRIDE
    {
        if (id == IEdit::GetIID())
        {
            *ppRet = (IEdit *)this;
            AddRef();
            return S_OK;
        }
        return TPanelProxy<IRichEdit>::QueryInterface(id, ppRet);
    }

  protected:
    // ----- attribute handlers -----
    HRESULT OnAttrTextColor(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrRtf(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrRich(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrWordWrap(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrAlign(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrNotifyChange(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrWantReturn(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrNumber(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrUpperCase(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrLowerCase(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrPassword(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrReadOnly(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrMultiLines(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrAutoHscroll(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrAutoVscroll(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrHscrollBar(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrVscrollBar(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrPasswordChar(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrEnableDragdrop(const SStringW &strValue, BOOL bLoading);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_SETFOCUS_EX(OnSetFocus)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
        MSG_WM_SIZE(OnSize)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_CHAR(OnChar)
        MESSAGE_HANDLER_EX(WM_IME_CHAR, OnImeChar)
        MESSAGE_HANDLER_EX(EM_GETSEL, OnGetSel)
        MESSAGE_HANDLER_EX(EM_SETSEL, OnSetSel)
        MESSAGE_HANDLER_EX(EM_REPLACESEL, OnReplaceSel)
        MESSAGE_HANDLER_EX(EM_SETEVENTMASK, OnSetEventMask)
        MSG_WM_SETFONT_EX(OnSetFont)
    SOUI_MSG_MAP_END()

    SOUI_ATTRS_BEGIN()
        ATTR_COLOR(L"cueColor", m_crCue, TRUE)
        ATTR_I18NSTRT(L"cueText", m_strCue, TRUE)
        ATTR_INT(L"style", m_dwStyle, TRUE)
        ATTR_INT(L"maxBuf", m_cchTextMost, FALSE)
        ATTR_BOOL(L"transparent", m_fTransparent, TRUE)
        ATTR_BOOL(L"vertical", m_fVertical, TRUE)
        ATTR_BOOL(L"allowBeep", m_fAllowBeep, FALSE)
        ATTR_BOOL(L"autoWordSel", m_fEnableAutoWordSel, TRUE)
        ATTR_BOOL(L"vcenter", m_fSingleLineVCenter, TRUE)
        ATTR_BOOL(L"disableCaret", m_fDisableCaret, TRUE)
        ATTR_BOOL(L"wantTab", m_fWantTab, FALSE)
        ATTR_BOOL(L"autoSel", m_fAutoSel, FALSE)
        ATTR_CUSTOM(L"colorText", OnAttrTextColor)
        ATTR_CUSTOM(L"rtf", OnAttrRtf)
        ATTR_CUSTOM(L"rich", OnAttrRich)
        ATTR_CUSTOM(L"wordWrap", OnAttrWordWrap)
        ATTR_CUSTOM(L"align", OnAttrAlign)
        ATTR_CUSTOM(L"notifyChange", OnAttrNotifyChange)
        ATTR_CUSTOM(L"wantReturn", OnAttrWantReturn)
        ATTR_CUSTOM(L"number", OnAttrNumber)
        ATTR_CUSTOM(L"upperCase", OnAttrUpperCase)
        ATTR_CUSTOM(L"lowerCase", OnAttrLowerCase)
        ATTR_CUSTOM(L"password", OnAttrPassword)
        ATTR_CUSTOM(L"readOnly", OnAttrReadOnly)
        ATTR_CUSTOM(L"multiLines", OnAttrMultiLines)
        ATTR_CUSTOM(L"autoHscroll", OnAttrAutoHscroll)
        ATTR_CUSTOM(L"autoVscroll", OnAttrAutoVscroll)
        ATTR_CUSTOM(L"hscrollBar", OnAttrHscrollBar)
        ATTR_CUSTOM(L"vscrollBar", OnAttrVscrollBar)
        ATTR_CUSTOM(L"passwordChar", OnAttrPasswordChar)
        ATTR_CUSTOM(L"enableDragdrop", OnAttrEnableDragdrop)
    SOUI_ATTRS_END()

  protected:
    // ----- editing state -----
    SStringT m_strEditText; /**< full text, '\n' separates lines */
    int m_nCaret;           /**< caret character index */
    int m_nAnchor;          /**< selection anchor (for shift select) */
    DWORD m_cchTextMost;    /**< max text length */
    TCHAR m_chPasswordChar; /**< password mask char (0 = '*') */

    LONG m_lAccelPos;     /**< accelerator position (unused, kept for parity) */
    COLORREF m_crText;    /**< default text color */
    SStringW m_strRtfSrc; /**< rtf= attribute source (ignored) */

    // ----- behavior flags -----
    UINT m_fEnableAutoWordSel : 1;
    UINT m_fWordWrap : 1;
    UINT m_fTransparent : 1;
    UINT m_fVertical : 1;
    UINT m_fAllowBeep : 1;
    UINT m_fWantTab : 1;
    UINT m_fSingleLineVCenter : 1;
    UINT m_fScrollPending : 1;
    UINT m_fEnableDragDrop : 1;
    UINT m_fNotifyChange : 1;
    UINT m_fDisableCaret : 1;
    UINT m_fReadOnly : 1;
    UINT m_fMultiLine : 1;
    UINT m_fNumber : 1;
    UINT m_fUpperCase : 1;
    UINT m_fLowerCase : 1;
    UINT m_fPassword : 1;
    UINT m_fWantReturn : 1;
    UINT m_fAutoHScroll : 1;
    UINT m_fAutoVScroll : 1;
    UINT m_fHScrollBar : 1;
    UINT m_fVScrollBar : 1;
    UINT m_fSaveSelection : 1;

    // ----- cue text (hint text) -----
    COLORREF m_crCue; /**< Cue text color */
    STrText m_strCue; /**< Cue text */

    // ----- layout / scroll cache -----
    int m_nLineHeight;
    int m_nTotalHeight;
    int m_nContentWidth;
    int m_nScrollY;
    int m_nScrollX;
    BOOL m_bLayoutDirty;
    SArray<int> m_arrLineStart; /**< visual line start char indices */
    SArray<int> m_arrLineEnd;   /**< visual line end (exclusive, excludes '\n') */

    // ----- drag select -----
    BOOL m_bDragSel;

    // ----- simple undo -----
    SStringT m_strUndoText;
    int m_nUndoCaret;
    BOOL m_bUndoValid;

  protected:
    void RecalcLayout(IRenderTarget *pRT);
    void UpdateScrollBar();
    void OnEditAttrChanged();
    void UpdateCaretPos();
    int LineFromChar(int nChar) const;
    void InsertChar(TCHAR ch);
    void EnsureLayout();
    void EnsureCaretVisible();
    int CharFromPoint(CPoint pt, IRenderTarget *pRT);
    int ChartX(IRenderTarget *pRT, int nLine, int iChar);
    int GetLineLeft(IRenderTarget *pRT, int nLine, int nInnerW);
    void FireTextChanged();
    void DeleteSelection();
    void InsertText(LPCTSTR pszText);
    void CopySel();
    void CutSel();
    void PasteSel();
    void SelectAll();
    void PushUndo();

    COLORREF GetTextColor() const;
};

#else /* !SOUI_EDIT_LITE */

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
    STDMETHOD_(void, GetCueText)(CTHIS_ IStringT *pStr) SCONST OVERRIDE;

    /**
     * @brief Get the cue text color
     * @return Cue text color
     */
    STDMETHOD_(COLORREF, GetCueColor)(CTHIS) SCONST OVERRIDE
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

#endif /* SOUI_EDIT_LITE */

SNSEND

#endif /**< _SEDIT_H_ */
