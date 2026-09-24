#ifndef __SSCINTILLAVIEW_H__
#define __SSCINTILLAVIEW_H__

#include <core/SPanel.h>
#include <control/SDropDown.h>
#include <control/SListbox.h>
#include <event/SEvents.h>
#include <vector>

class ScintillaHeadlessHost;
struct SCNotification;
struct ScintillaAutoCompleteInfo;
SNSBEGIN

struct SScintillaHeadlessListenerImpl;
class SAutoCompleteDropDown;

#ifndef EVT_SCIVIEW_ACCQUERY
#define EVT_SCIVIEW_ACCQUERY (EVT_EXTERNAL_BEGIN+900)
#endif//EVT_SCIVIEW_ACCQUERY
// Fired by the control when it has no internal autocomplete list (the
// `autocompleteList` attribute is not configured). The business layer handles it
// and fills strCandidates with a space-separated candidate list (matching the
// engine's SCI_AUTOCSETSEPARATOR) to drive the popup. The event is dispatched
// synchronously via FireEvent, so the business handler writes into strCandidates
// during the call and the control reads it back immediately.
// Event ID is a fixed literal kept clear of the SOUI core enum (until
// EVT_EXTERNAL_BEGIN at 10000000); the extended control owns this namespace.
DEF_EVT_EXT(EventScintillaAccQuery, EVT_SCIVIEW_ACCQUERY, {
     LPCSTR strPrefix; /**< Current typed token prefix (read-only), utf8 */
    int nLenEntered;           /**< Characters already typed for the token    */
    IStringA *strCandidates;   /**< In/out: fill with space-separated candidates */
    BOOL bSorted;              /**< Indicated whether strCandidates was alphabet sorted */
})
/**
 * @class SScintillaView
 * @brief Windowless Scintilla editor control.
 * @details Hosts a headless Scintilla engine (no real HWND). The engine draws to
 * the SOUI render target inside OnPaint (SRichEdit style: temporary render
 * target + HDC + viewport origin moved to the control, no alpha blend), receives
 * mouse/keyboard/focus/size/scroll events forwarded through SendEditor, and
 * drives caret blink through the SWindow timer mechanism bridged via
 * ScintillaHeadlessHost.
 *
 * It derives from SPanel so it inherits SOUI's virtual scrollbar (skin based).
 * Engine scrollbar state is fed into SPanel via SyncScrollBars(), and scrollbar
 * interaction is forwarded back to the engine by overriding OnScroll(), the
 * same wrapping pattern SRichEdit uses for the rich edit text service.
 *
 * Scintilla headers are deliberately kept out of this header and included only
 * in the .cpp, so every translation unit that includes the control does not
 * pull in the Scintilla platform layer.
 *
 * IME composition/candidate window following is supported on Windows: the
 * control forwards WM_IME_STARTCOMPOSITION / WM_IME_ENDCOMPOSITION to the engine
 * and feeds the control origin into the headless host, so the engine positions
 * the floating IME window at the caret via the real host window's input context.
 */
class SScintillaView : public SPanel, public ISDropDownOwner {
    DEF_SOBJECT(SPanel, L"scintilla")
    enum{
        kTimerBase = 100,// SWindow timer-id base for engine fine tickers (each reason = base + reason).
        kTickReasonCount=5,// Scintilla TickReason count: tickCaret..tickPlatform.
    };
    friend struct SScintillaHeadlessListenerImpl;

  public:
    SScintillaView(void);
    virtual ~SScintillaView(void);

    /// Send a Scintilla command/message to the engine (SCI_* or a window message).
    LRESULT SendEditor(unsigned int uMsg, WPARAM wParam = 0, LPARAM lParam = 0);

    /// Convenience: replace the whole document text (expects UTF-8).
    void SetEditorText(const char *pSrc);

    /// Read back the whole document text (UTF-8), including user edits.
    SStringA GetEditorText();

    /// Commit the currently selected autocomplete candidate into the document
    /// (SCI_AUTOCSELECT + SCI_AUTOCCOMPLETE). nItem < 0 uses the dropdown list's
    /// current selection.
    void CompleteAutoComplete(int nItem = -1);

    // ISDropDownOwner
    SWindow *GetDropDownOwner() OVERRIDE;
    void OnCreateDropDown(SDropDownWnd *pDropDown) OVERRIDE;
    void OnDestroyDropDown(SDropDownWnd *pDropDown) OVERRIDE;

  protected:
    /// Called when the engine emits an SCN_* notification. Subclass to handle
    /// events such as SCN_MODIFIED. The SCNotification is only valid during the
    /// call, copy what you need.
    virtual void OnScintillaNotify(const SCNotification &scn);

    LRESULT OnCreate(LPVOID);
    void OnDestroy();
    void OnSize(UINT nType, CSize size);
    void OnPaint(IRenderTarget *pRT);
    void OnTimer(char cTimerID);
    void OnSetFocus(SWND wndOld);
    void OnKillFocus(SWND wndFocus);
    void OnLButtonDown(UINT nFlags, CPoint point);
    void OnLButtonUp(UINT nFlags, CPoint point);
    void OnLButtonDblClk(UINT nFlags, CPoint point);
    void OnMouseMove(UINT nFlags, CPoint point);
    BOOL OnSetCursor(const CPoint &pt);
    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    /// IME-composed text is delivered to the focused window as WM_IME_CHAR;
    /// forward the composed character to the engine.
    LRESULT OnImeChar(UINT msg,WPARAM wp,LPARAM lp);
    /// IME composition opened: feed the control origin into the headless host
    /// then forward so the engine positions the floating IME window at the caret.
    LRESULT OnImeStartComposition(UINT msg,WPARAM wp,LPARAM lp);
    /// IME composition closed: forward so the engine restores its caret.
    LRESULT OnImeEndComposition(UINT msg,WPARAM wp,LPARAM lp);

    /// SPanel override: a scrollbar command originated from SOUI's scrollbar.
    /// Forward it to the engine (as WM_VSCROLL/WM_HSCROLL) then reflect the
    /// engine's resulting scrollbar position back into the panel state.
    BOOL OnScroll(BOOL bVertical, UINT uCode, int nPos) OVERRIDE;

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_SIZE(OnSize)
        MSG_WM_SETFOCUS_EX(OnSetFocus)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
        MSG_WM_TIMER_EX(OnTimer)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_CHAR(OnChar)
        MESSAGE_HANDLER_EX(WM_IME_CHAR,OnImeChar)
        MESSAGE_HANDLER_EX(WM_IME_STARTCOMPOSITION,OnImeStartComposition)
        MESSAGE_HANDLER_EX(WM_IME_ENDCOMPOSITION,OnImeEndComposition)
    SOUI_MSG_MAP_END()

  protected:
    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"text", OnAttrSetText)
        ATTR_CUSTOM(L"showLineNumber", OnAttrShowLineNumber)
        ATTR_CUSTOM(L"autocompleteList", OnAttrAutocompleteList)
        ATTR_CUSTOM(L"lexer", OnAttrLexer)
    SOUI_ATTRS_END()

    ScintillaHeadlessHost *m_host;
    SScintillaHeadlessListenerImpl *m_pListener; // opaque engine listener adapter (owned)
    void *m_sci;
    SStringW m_strText;
    bool m_bShowLineNumber;
    SStringW m_strLexer; /**< lexer= attribute ("xml"/"cpp"/"text") for syntax highlight */

    // Autocomplete support. Candidates are owned by the engine; the engine hands
    // state to us through the ScintillaHeadlessListener::AutoCompleteNotify
    // callback, and a real SOUI dropdown window (SDropDownWnd) hosts an SListBox
    // that renders them (never in the editor's client area). The dropdown's
    // listbox owns Up/Down/Enter navigation (mirroring SComboBox); selection
    // changes push the engine preview (SCI_AUTOCSELECT) while committing does
    // SCI_AUTOCCOMPLETE.
    SStringA m_strAccAll;       // candidate words, space separated (UTF-8)
    std::vector<SStringA> m_vAccWords; // parsed candidates, sorted case-insensitively
    bool m_bAccPopup;           // dropdown window currently shown
    SListBox *m_pAccListBox;    // the dropdown list (created once, owned)
    SAutoCompleteDropDown *m_pAccDropDown; // the SOUI dropdown window (owned)
    int m_nAccCaretX;           // last caret x/y (control-local) for popup placement
    int m_nAccCaretY;
    LOGFONT m_lfAcc;            // editor font (kept for the height of a row)

    // Engine notification / geometry handling is provided by the opaque
    // listener adapter (SScintillaHeadlessListenerImpl) created in the .cpp;
    // these methods contain the view-side logic it forwards to.
    void OnScintillaInvalidate(const CRect &rc);
    /// Route events originating from the dropdown's listbox (selection preview /
    /// double-click to commit) back into the engine and the dropdown.
    STDMETHOD_(BOOL, FireEvent)(IEvtArgs *evt) OVERRIDE;
    /// Create the dropdown SListBox once and keep it detached until it is moved
    /// into the dropdown root by OnCreateDropDown (mirrors SComboBox::CreateListBox).
    BOOL CreateAccListBox();

    /// Engine caret changed; drive the SOUI SCaret (via SWindow::CreateCaret/
    /// ShowCaret/SetCaretPos) instead of a system caret, mirroring SRichEdit.
    void OnScintillaCaret(int x, int y, int height, bool shown);

    /// Copy the engine's scrollbar state (range/page/pos) into the SPanel state
    /// and show/hide the corresponding SOUI scrollbar.
    void SyncScrollBars();
    void UpdateScrollBarFromEngine(UINT nBar);
    /// Push the window's own SWindow font (family + size) down into the engine as
    /// the STYLE_DEFAULT style, so the control honors the XML `font=` attribute.
    void ApplyEditorFont();
    /// Enable/disable the engine's line-number margin (margin 1) according to
    /// m_bShowLineNumber, and force the margin cursor to a plain arrow.
    void ApplyLineNumberMargins();
    HRESULT OnAttrSetText(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrShowLineNumber(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrAutocompleteList(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrLexer(const SStringW &strValue, BOOL bLoading);
    /// Apply the `lexer=` attribute: configure the Scintilla lexer + lexer styles.
    void ApplyLexer();
    /// Compose the engine autocomplete: ask it to show a listbox for the current
    /// token (which routes to our popup via the host callback listbox).
    void TriggerAutoComplete();
    /// Apply an engine auto-complete snapshot to the dropdown (show/hide/update rows).
    void ApplyAutoComplete(const ScintillaAutoCompleteInfo &info);
    /// Compute the dropdown rectangle anchored below the caret (or above when near
    /// the bottom of the monitor), mirroring SComboBase::CalcPopupRect.
    BOOL CalcAccPopupRect(int nHeight, CRect &rcPopup);
};

SNSEND

#endif // __SSCINTILLAVIEW_H__