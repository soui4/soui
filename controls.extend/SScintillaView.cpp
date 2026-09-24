#include "stdafx.h"
#include "SScintillaView.h"
#include <ScintillaHeadless.h>
#include <ScintillaHostListener.h>
#include <SciLexer.h>
#include <helper/SColor.h>
#include <helper/slog.h>
#include <gdialpha.h>
#include <algorithm>
#define kLogTag "SScintillaView"

SNSBEGIN

// Case-insensitive string order for the parsed autocomplete candidates
// (VS2008/C++03: std::sort / std::lower_bound need a class, not a lambda).
namespace {
struct AccWordLess {
    bool operator()(const SStringA &a, const SStringA &b) const {
        return a.CompareNoCase(b.c_str()) < 0;
    }
};
}

// A dropdown that never activates when shown. SWP_NOACTIVATE lets us display the
// autocomplete list without stealing native focus from the editor; otherwise the
// editor's OnKillFocus makes the engine cancel the pending autocomplete and the
// list dismisses itself. Up/Down/Return/Escape are intercepted during the message
// loop's pre-translate phase and re-sent to the native dropdown window, which
// routes them to the focused listbox -- mirroring SPropertyGrid's
// SSearchDropdownList.
class SAutoCompleteDropDown : public SDropDownWnd {
    typedef SDropDownWnd __baseCls;

  public:
    SAutoCompleteDropDown(ISDropDownOwner *pOwner)
        : SDropDownWnd(pOwner)
    {
    }

    // Show (or reposition) without activating, then take mouse capture like the
    // combobox/search dropdown does.
    void Adjust(int x, int y, int cx, int cy)
    {
        SetWindowPos(HWND_TOPMOST, x, y, cx, cy, SWP_SHOWWINDOW | SWP_NOACTIVATE);
        SNativeWnd::SetCapture();
    }

  protected:
    BOOL WINAPI PreTranslateMessage(MSG *pMsg) OVERRIDE
    {
        if (SDropDownWnd::PreTranslateMessage(pMsg))
            return TRUE;
        // Mirror SSearchDropdownList::SDropdownList: intercept the mouse wheel
        // (so it scrolls the list instead of the editor underneath) and the
        // Up/Down/Return/Escape keys, re-sending them to this native dropdown so
        // they route to the focused listbox inside.
        if (pMsg->message == WM_MOUSEWHEEL ||
            ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP) &&
             (pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN || pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)))
        {
            SNativeWnd::SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
            return TRUE;
        }
        return FALSE;
    }
};

// Implements ScintillaHost's ScintillaHeadlessListener on behalf of SScintillaView
// so the control header does not need the Scintilla listener header / include path.
// The host forwards engine notifications here and pulls geometry on demand; all
// view-side handling lives behind SScintillaView's own methods (this struct is a
// friend of the view).
struct SScintillaHeadlessListenerImpl : public ScintillaHeadlessListener {
    SScintillaView *m_pView;
    explicit SScintillaHeadlessListenerImpl(SScintillaView *pView) : m_pView(pView) {}

    // Control client rectangle in its own (local) coordinate space -- the engine
    // paints with the viewport origin at the control top-left (see OnPaint), so
    // the rectangle is {0,0,w,h}.
    PRectangle GetClientRectangle() const override
    {
        CRect rc;
        m_pView->GetClientRect(&rc);
        return PRectangle(0, 0, rc.Width(), rc.Height());
    }

    void GetIMEWindowOffset(int *px, int *py) const override
    {
            
    CRect rc= m_pView->GetClientRect();
    m_pView->GetContainer()->FrameToHost(&rc);
    if(px) *px = rc.left;
    if(py) *py = rc.top;
    }

    void RequestTimer(int reason, int millis) override
    {
        if (millis > 0)
            m_pView->SetTimer(static_cast<char>(SScintillaView::kTimerBase + reason),
                              static_cast<UINT>(millis));
        else
            m_pView->KillTimer(static_cast<char>(SScintillaView::kTimerBase + reason));
    }

    void NotifyCaret(int x, int y, int height, bool shown) override
    {
        m_pView->OnScintillaCaret(x, y, height, shown);
    }

    void AutoCompleteNotify(const ScintillaAutoCompleteInfo &info) override
    {
        m_pView->ApplyAutoComplete(info);
    }

    void InvalidateRectangle(int left, int top, int right, int bottom) override
    {
        m_pView->OnScintillaInvalidate(CRect(left, top, right, bottom));
    }

    void Notify(const SCNotification &scn) override
    {
        m_pView->OnScintillaNotify(scn);
        m_pView->SyncScrollBars();
        // Painting and text edits repaint through the engine's InvalidateRectangle
        // callback (partial dirty rect), so no full-window Invalidate() here. A
        // periodic unconditional repaint flooded the message loop with WM_PAINT and
        // starved WM_TIMER.
    }
};

SScintillaView::SScintillaView()
    : m_host(NULL)
    , m_pListener(NULL)
    , m_sci(NULL)
    , m_bShowLineNumber(FALSE)
    , m_strLexer()
    , m_bAccPopup(FALSE)
    , m_pAccListBox(NULL)
    , m_pAccDropDown(NULL)
    , m_nAccCaretX(0)
    , m_nAccCaretY(0)
{
    memset(&m_lfAcc, 0, sizeof(m_lfAcc));
    m_bFocusable = TRUE;
    m_pListener = new SScintillaHeadlessListenerImpl(this);
    m_evtSet.addEvent(EVENTID(EventScintillaAccQuery));
}

SScintillaView::~SScintillaView()
{
    // The SDropDownWnd deletes itself (SHostWnd::OnFinalMessage) once destroyed;
    // the dropdown listbox is released here like SComboBox releases its list box.
    if (m_pAccListBox)
    {
        m_pAccListBox->SetOwner(NULL);
        m_pAccListBox->SSendMessage(WM_DESTROY);
        m_pAccListBox->Release();
        m_pAccListBox = NULL;
    }
    if (m_host)
    {
        Scintilla_DestroyHeadless(m_sci);
        delete m_host;
        m_host = NULL;
        m_sci = NULL;
    }
    delete m_pListener;
    m_pListener = NULL;
}

LRESULT SScintillaView::SendEditor(unsigned int uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!m_sci)
        return 0;
    return static_cast<LRESULT>(Scintilla_DirectFunction(m_sci, uMsg, wParam, lParam));
}

void SScintillaView::SetEditorText(const char *pSrc)
{
    if (m_sci)
        SendEditor(SCI_SETTEXT, 0, reinterpret_cast<sptr_t>(pSrc));
}

SStringA SScintillaView::GetEditorText()
{
    SStringA strRet;
    if (m_sci)
    {
        int nLen = (int)SendEditor(SCI_GETTEXTLENGTH, 0, 0);
        if (nLen > 0)
        {
            char *pBuf = strRet.GetBufferSetLength(nLen);
            SendEditor(SCI_GETTEXT, nLen + 1, reinterpret_cast<sptr_t>(pBuf));
            strRet.ReleaseBuffer(nLen);
        }
    }
    return strRet;
}

void SScintillaView::OnScintillaNotify(const SCNotification &scn)
{
    // A character was inserted (or backspaced): ask the engine to refresh its
    // autocomplete list for the current token. Filtering on SCN_CHARADDED keeps
    // the popup in sync whether the change came from typing or deletion; the
    // engine (and our host callback listbox) drive the popup state. Candidates
    // come from the internal `autocompleteList` list when configured, otherwise
    // from the business layer through the EventScintillaAccQuery SOUI event.
    if (scn.nmhdr.code == SCN_CHARADDED && m_sci)
        TriggerAutoComplete();
}

void SScintillaView::UpdateScrollBarFromEngine(UINT nBar)
{
    SCROLLINFO si = { sizeof(si) };
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    if (!m_host || !m_host->GetScrollInfo(static_cast<int>(nBar), &si))
        return;

    const BOOL bVertical = (nBar == SB_VERT);
    const UINT wBar = bVertical ? SSB_VERT : SSB_HORZ;

    // While the thumb is being dragged SPanel drives the position through the
    // panel's own track state; do not toggle the bar's visibility or overhaul
    // its position in the middle of a drag (that made the bar flicker away and
    // could break capture).
    const bool bTracking = (bVertical ? m_sbVert : m_sbHorz).IsThumbTracking();

    // Scrollable when the content extent exceeds the page (view) size.
    const bool scrollable = (si.nPage != 0) && (si.nMax >= static_cast<int>(si.nPage));
    if (!scrollable)
    {
        if (!bTracking && HasScrollBar(bVertical))
            ShowScrollBar(wBar, FALSE);
        return;
    }

    // Show the SOUI scrollbar the first time the content actually scrolls, but
    // only when a scrollbar skin is configured -- without one GetSbWidth() would
    // hit an assert and nothing would be painted anyway. Never disclose a bar
    // that the user is currently dragging.
    if (!bTracking && !HasScrollBar(bVertical) && m_pSkinSb)
        ShowScrollBar(wBar, TRUE);

    // During a thumb drag the position lives in the panel's track state; leave
    // the engine-synced nPos alone so the thumb does not snap back.
    if (!bTracking)
    {
        SCROLLINFO *psi = bVertical ? (&m_siVer) : (&m_siHoz);
        psi->nMin = si.nMin;
        psi->nMax = si.nMax;
        psi->nPage = si.nPage;
        psi->nPos = si.nPos;
        const int nPosMax = psi->nMax - static_cast<int>(psi->nPage) + 1;
        if (psi->nPos > nPosMax)
            psi->nPos = nPosMax;
        if (psi->nPos < psi->nMin)
            psi->nPos = psi->nMin;
    }

    CRect rcSb = GetScrollBarRect(bVertical);
    InvalidateRect(rcSb);
}

// Copy the engine's scrollbar state into SPanel's state so the SOUI scrollbars
// track the engine. Called after any input that may scroll the document.
void SScintillaView::SyncScrollBars()
{
    if (!m_sci)
        return;
    UpdateScrollBarFromEngine(SB_VERT);
    UpdateScrollBarFromEngine(SB_HORZ);
}

// A scrollbar interaction originated from SOUI's scrollbar. Forward the command
// to the engine (WM_VSCROLL/WM_HSCROLL), then let SyncScrollBars() pull back the
// engine's resulting position. Mirrors SRichEdit::OnScroll.
BOOL SScintillaView::OnScroll(BOOL bVertical, UINT uCode, int nPos)
{
    if (!m_sci)
        return FALSE;

    // The engine reads the thumb track position back from GetScrollInfo (its
    // WM_VSCROLL ignores the wParam HiWord), so push the drag position into the
    // recorded state before forwarding, otherwise the engine keeps an old value.
    if (uCode == SB_THUMBTRACK && m_host)
    {
        SCROLLINFO si = { sizeof(si) };
        si.fMask = SIF_TRACKPOS;
        if (m_host->GetScrollInfo(bVertical ? SB_VERT : SB_HORZ, &si))
        {
            if (si.nTrackPos != nPos)
            {
                si.nTrackPos = nPos;
                m_host->SetScrollInfo(bVertical ? SB_VERT : SB_HORZ, &si, false);
            }
        }
    }

    SendEditor(bVertical ? WM_VSCROLL : WM_HSCROLL, MAKEWPARAM(uCode, nPos), 0);

    // The engine updated its internal top line / x offset and recorded the new
    // scrollbar state; reflect it into the panel. The dirty rect comes back
    // through the engine's InvalidateRectangle callback, so no full repaint here.
    SyncScrollBars();
    return TRUE;
}

LRESULT SScintillaView::OnCreate(LPVOID lp)
{
    // Initialize the SPanel scrollbar state. Premultiplied -1 means "no scrollbar
    // skin configured yet"; that is acceptable here (the bars just won't paint),
    // so do not fail window creation on it.
    (void)__baseCls::OnCreate(NULL);
    if (m_sci)
        return 0;

    m_host = new ScintillaHeadlessHost(GetContainer()->GetHostHwnd(), GetID(), m_pListener);
    CreateAccListBox();

    m_sci = Scintilla_CreateHeadless(m_host);
    if (!m_sci)
        return 0;

    m_host->SetCtrlID(GetID());

    // Minimal but usable default configuration.
    SendEditor(SCI_SETCODEPAGE, SC_CP_UTF8, 0);
    SendEditor(SCI_SETTABWIDTH, 4, 0);
    // The `autocompleteList` attribute is space separated; tell the engine the
    // auto-complete candidate separator is the space character so it splits the
    // list into individual candidates (the shared AutoComplete state machine).
    SendEditor(SCI_AUTOCSETSEPARATOR, 0x20, 0);
    // Prefix matching is case-insensitive (the internal `autocompleteList` is
    // sorted case-insensitively and SC_ORDER_PRESORTED skips the engine's sort),
    // so the engine's Select/Sorter must compare case-insensitively too;
    // otherwise the case-sensitive strncmp binary search can miss in mixed case.
    SendEditor(SCI_AUTOCSETIGNORECASE, 1, 0);
    SendEditor(SCI_STYLERESETDEFAULT, 0, 0);
    // Honor the window's own XML `font=` attribute (family + size), then reflect
    // the showLineNumber attribute on the line-number margin.
    ApplyEditorFont();
    ApplyLineNumberMargins();
    // Apply the `lexer=` attribute last so the lexer styles inherit the editor font.
    ApplyLexer();

    if (!m_strText.IsEmpty())
    {
        SStringA strU8 = S_CW2A(m_strText,CP_UTF8);
        SetEditorText(strU8.c_str());
    }

    SendEditor(SCI_SETBUFFEREDDRAW, 0, 0);
    // The caret is rendered by SOUI's own blazing SCaret (see OnScintillaCaret),
    // so keep the engine from drawing its own (fixed, non-blinking) caret bar.
    SendEditor(SCI_SETCARETSTYLE, CARETSTYLE_INVISIBLE, 0);
    return 0;
}

void SScintillaView::OnDestroy()
{
    if (m_host)
    {
        Scintilla_DestroyHeadless(m_sci);
        delete m_host;
        m_host = NULL;
        m_sci = NULL;
    }
    __baseCls::OnDestroy();
}

void SScintillaView::OnSize(UINT nType, CSize size)
{
    __baseCls::OnSize(nType, size);
    if (m_sci)
    {
        SendEditor(WM_SIZE, (WPARAM)nType, (LPARAM)MAKELONG(size.cx, size.cy));
    }
    SyncScrollBars();
}

void SScintillaView::OnPaint(IRenderTarget *pRT)
{
    CRect rcClient;
    GetClientRect(&rcClient);
    pRT->PushClipRect(&rcClient, RGN_AND);

    // Save drawing state like SRichEdit does.
    SAutoRefPtr<IRenderObj> pFont = pRT->GetCurrentObject(OT_FONT);
    COLORREF crText = pRT->GetTextColor();

    // Draw directly on the render target's own HDC. On GDI backends GetDC()
    // returns the very HDC of the render target (host coordinate space), so we
    // move the viewport origin to the control's top-left and hand the engine the
    // control-local client rectangle -- no offscreen buffer, no alpha blend.
    // Creating a temporary render target here would shift the origin (viewport/
    // world transform) and cost a full-screen copy on every paint.
    HDC hdc = pRT->GetDC(0);
    bool bPaint = (hdc != NULL) && (m_sci != NULL);
    // Only repaint the region the render target actually claims. SOUI calls
    // OnPaint for whatever dirty rectangle the container reclaimed (and the
    // PushClipRect above already clips to the client rect), so take the current
    // clip box and intersect it with the client rect. Repainting a sub-rect
    // makes Scintilla's View only lay-out/redraw the affected lines instead of
    // the whole document on every pass. Falls back to the whole client rect if
    // the clip is not available.
    CRect rcPaint(rcClient);
    if (bPaint)
    {
        CRect rcClip;
        if (pRT->GetClipBox(&rcClip) == S_OK)
            bPaint = rcPaint.IntersectRect(&rcClient, &rcClip);
    }
    if (bPaint)
    {
        POINT ptOld;
#ifdef _WIN32
        // Scintilla paints through plain GDI (Scintilla_PaintHeadless), which
        // trashes the alpha channel of the 32bpp target. Save the alpha bytes
        // of the paint rect and write them back afterwards, mirroring
        // SRichEdit::OnPaint. AlphaBackup resolves the rect against the DC's
        // current viewport origin, so it must run before SetViewportOrgEx
        // shifts that origin below; AlphaRestore touches raw bitmap bytes, so
        // its position relative to the origin shift does not matter.
        ALPHAINFO ai;
        CGdiAlpha::AlphaBackup(hdc, &rcPaint, ai);
#endif
        SetViewportOrgEx(hdc,rcClient.left,rcClient.top,&ptOld);
        // Engine paint rect must be expressed in control-local space, i.e. the
        // viewport origin that was just applied (client top-left becomes 0,0).
        CRect rc = rcPaint;
        rc.MoveToXY(rcPaint.left - rcClient.left, rcPaint.top - rcClient.top);
        const int nOldMode = ::SetGraphicsMode(hdc, GM_COMPATIBLE);
        Scintilla_PaintHeadless(m_sci, hdc, &rc);
        ::SetGraphicsMode(hdc, nOldMode);
        SetViewportOrgEx(hdc,ptOld.x,ptOld.y,NULL);
#ifdef _WIN32
        CGdiAlpha::AlphaRestore(ai);
#endif
    }
    pRT->ReleaseDC(hdc, &rcClient);

    pRT->SelectObject(pFont);
    pRT->SetTextColor(crText);

    pRT->PopClip();
}

void SScintillaView::OnTimer(char cTimerID)
{
    const int reason = cTimerID - kTimerBase;
    if (reason >= 0 && reason < kTickReasonCount && m_sci)
    {
        Scintilla_TickHeadless(m_sci, reason);
        // The engine marks its own dirty rect (tickCaret -> NotifyCaret, wrap ->
        // Redraw); no full-window repaint here.
    }
}

// Repaint only the region the engine marked dirty, letting Scintilla's view
// lay-out/redraw just the affected lines instead of repainting the whole
// window. Merges with any already-dirty rect the way the container does.
void SScintillaView::OnScintillaInvalidate(const CRect &rc)
{
    CRect rcClient;
    GetClientRect(&rcClient);
    CRect rcDirty(rc);
    rcDirty.OffsetRect(rcClient.left,rcClient.top);
    InvalidateRect(&rcDirty);
}

// Drive the SOUI SCaret owned by this window, mirroring SRichEdit's STextHost
// TxCreateCaret/TxShowCaret/TxSetCaretPos. The engine reports update on focus
// gain/loss, caret movement and scrolling; SCaret itself handles the blinking.
void SScintillaView::OnScintillaCaret(int x, int y, int height, bool shown)
{
    if (shown && height > 0)
    {
        // The engine reports the caret in its own client space (0,0 == control
        // top-left), but SCaret is drawn on the render target in the window's
        // coordinate space, so add the control's client top-left offset.
        CRect rcClient;
        GetClientRect(&rcClient);
        m_nAccCaretX = x;
        m_nAccCaretY = y;
        // Create once, then refresh dimensions/position each update (CreateCaret
        // re-inits the existing SCaret). A null bitmap makes SCaret draw its own
        // color caret of the given size.
        SWindow::CreateCaret(NULL, 2, height);
        SWindow::ShowCaret(TRUE);
        SWindow::SetCaretPos(rcClient.left + x, rcClient.top + y);
    }
    else
    {
        SWindow::ShowCaret(FALSE);
    }
}

void SScintillaView::OnSetFocus(SWND wndOld)
{
    __baseCls::OnSetFocus(wndOld);
    if (m_sci)
    {
        SendEditor(WM_SETFOCUS, 0, 0);
    }
}

void SScintillaView::OnKillFocus(SWND wndFocus)
{
    // While the autocomplete dropdown is up it owns SWindow/native focus (its
    // listbox steals it in OnCreateDropDown). Forwarding WM_KILLFOCUS to the
    // engine then makes it cancel the pending autocomplete, which immediately
    // hides the dropdown we just showed. With a dropdown active keep the engine
    // believing it stays focused; OnDestroyDropDown refocuses this control so the
    // engine gets its WM_SETFOCUS back.
    if (m_sci && !m_pAccDropDown)
    {
        SendEditor(WM_KILLFOCUS, 0, 0);
    }
    __baseCls::OnKillFocus(wndFocus);
}

void SScintillaView::OnLButtonDown(UINT nFlags, CPoint point)
{
    CRect rcClient = GetClientRect();
    CPoint ptLocal = point - rcClient.TopLeft();
    SetFocus();
    SetCapture();
    if (m_sci)
    {
        SendEditor(WM_LBUTTONDOWN, (WPARAM)nFlags, (LPARAM)MAKELPARAM(ptLocal.x, ptLocal.y));
    }
}

void SScintillaView::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (m_sci)
    {
        point -= GetClientRect().TopLeft();
        SendEditor(WM_LBUTTONUP, (WPARAM)nFlags, (LPARAM)MAKELPARAM(point.x, point.y));
    }
    ReleaseCapture();
}

void SScintillaView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    if (m_sci)
    {
        point -= GetClientRect().TopLeft();
        // The engine has no WM_LBUTTONDBLCLK branch; double-click word selection is
        // decided internally from consecutive WM_LBUTTONDOWN presses within the
        // double-click time. SOUI swallows the second press into OnLButtonDblClk, so
        // re-deliver it as a WM_LBUTTONDOWN to let the engine select the word.
        SendEditor(WM_LBUTTONDOWN, (WPARAM)nFlags, (LPARAM)MAKELPARAM(point.x, point.y));
    }
}

void SScintillaView::OnMouseMove(UINT nFlags, CPoint point)
{
    // Drag-selection preview: the engine updates the selection internally on
    // every move but does not mark a dirty rect for live dragging, so repaint
    // here (mirrors SRichEdit redrawing as the selection grows).
    if (m_sci)
    {
        point -= GetClientRect().TopLeft();
        SendEditor(WM_MOUSEMOVE, (WPARAM)nFlags, (LPARAM)MAKELPARAM(point.x, point.y));
    }
    Invalidate();
}

BOOL SScintillaView::OnSetCursor(const CPoint &pt)
{
    if (!m_sci || !GetContainer())
    {
        ::SetCursor(GETRESPROVIDER->LoadCursor(IDC_ARROW));
        return TRUE;
    }
    const UINT uHit = OnNcHitTest(pt);
    if (uHit == HTVSCROLL || uHit == HTHSCROLL)
    {
        ::SetCursor(GETRESPROVIDER->LoadCursor(IDC_ARROW));
        return TRUE;
    }

    // Control-local coordinate (engine client rect is {0,0,w,h}); fall back to
    // (0,0) when the cursor position could not be resolved, matching the prior
    // behaviour.
    CPoint ptPos = pt - GetClientRect().TopLeft();
    // Standard cursor resource IDs (INT_PTR values of MAKEINTRESOURCE(IDC_*)).
    const int CID_ARROW = 32512;
    const int CID_IBEAM = 32513;
    const int CID_WAIT = 32514;
    const int CID_UPARROW = 32516;
    const int CID_SIZEWE = 32644;
    const int CID_HAND = 32649;

    const int nCursor = Scintilla_CursorForPointHeadless(m_sci, ptPos.x, ptPos.y);
    int nStdId = CID_ARROW;
    switch (nCursor)
    {
    case 1: // Window::cursorText
        nStdId = CID_IBEAM;
        break;
    case 2: // Window::cursorArrow
        nStdId = CID_ARROW;
        break;
    case 3: // Window::cursorUp
        nStdId = CID_UPARROW;
        break;
    case 4: // Window::cursorWait
        nStdId = CID_WAIT;
        break;
    case 5: // Window::cursorHoriz
    case 7: // Window::cursorReverseArrow -> horizontal arrow (nearest on macOS)
        nStdId = CID_SIZEWE;
        break;
    case 8: // Window::cursorHand
        nStdId = CID_HAND;
        break;
    default:
        nStdId = CID_ARROW;
        break;
    }
    ::SetCursor(GETRESPROVIDER->LoadCursor(MAKEINTRESOURCE(nStdId)));
    return TRUE;
}

BOOL SScintillaView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    if (m_sci)
    {
        pt -= GetClientRect().TopLeft();
        SendEditor(WM_MOUSEWHEEL, (WPARAM)MAKEWPARAM(nFlags, zDelta), (LPARAM)MAKELPARAM(pt.x, pt.y));
    }
    SyncScrollBars();
    // The engine scrolls and reports its dirty rect through InvalidateRectangle;
    // no explicit repaint needed here.
    return TRUE;
}

void SScintillaView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    // Esc while the autocomplete dropdown is up dismisses it. The dropdown's
    // message filter normally re-sends the key to itself (EndDropDown), but that
    // round-trip depends on filter ordering and whether the message reaches the
    // focused editor -- handle it here so Esc always exits the popup.
    if (nChar == VK_ESCAPE && m_pAccDropDown && m_bAccPopup)
    {
        m_pAccDropDown->EndDropDown(IDCANCEL);
        return;
    }
    // Keyboard navigation of the autocomplete popup (Up/Down/PageUp/PageDown/
    // Home/End/Enter/Tab/Escape) is owned by the engine's shared AutoComplete
    // state machine; we only forward the key so it behaves identically to the
    // native HWND popup.
    if (m_sci)
    {
        SendEditor(WM_KEYDOWN, (WPARAM)nChar, (LPARAM)MAKELONG(nRepCnt, nFlags));
    }
    SyncScrollBars();
}

void SScintillaView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (m_sci)
    {
        SendEditor(WM_CHAR, (WPARAM)nChar, (LPARAM)MAKELONG(nRepCnt, nFlags));
    }
    SyncScrollBars();
}

LRESULT SScintillaView::OnImeChar(UINT msg, WPARAM wp, LPARAM lp) {
    UINT nChar = (UINT)wp;
    // IME-composed text arrives as WM_IME_CHAR with the composed character in
    // wParam; hand it to the engine which inserts it (AddWString) into the doc.
    if (m_sci)
    {
        SendEditor(WM_IME_CHAR, nChar, 0);
    }
    SyncScrollBars();
    return 0;
}

LRESULT SScintillaView::OnImeStartComposition(UINT msg, WPARAM wp, LPARAM lp) {
    if (m_sci)
        SendEditor(WM_IME_STARTCOMPOSITION, wp, lp);
    return 0;
}

LRESULT SScintillaView::OnImeEndComposition(UINT msg, WPARAM wp, LPARAM lp) {
    if (m_sci)
        SendEditor(WM_IME_ENDCOMPOSITION, wp, lp);
    return 0;
}

HRESULT SScintillaView::OnAttrSetText(const SStringW &strValue, BOOL bLoading)
{
    m_strText = strValue;
    if (!bLoading && m_sci)
    {
        SStringA strA = S_CW2A(m_strText,CP_UTF8);
        SetEditorText(strA.c_str());
    }
    return S_OK;
}

void SScintillaView::ApplyEditorFont()
{
    if (!m_sci)
        return;
    SAutoRefPtr<IRenderTarget> pRT;
    GETRENDERFACTORY->CreateRenderTarget(&pRT);
    BeforePaintEx(pRT);
    IFontPtr pFont = (IFontPtr)pRT->GetCurrentObject(OT_FONT);
    if (!pFont)
        return;
    const LOGFONT *plf = pFont->LogFont();
    if (plf && plf->lfFaceName[0] != 0)
    {
        memcpy(&m_lfAcc, plf, sizeof(m_lfAcc));
        SendEditor(SCI_STYLESETFONT, STYLE_DEFAULT, reinterpret_cast<LPARAM>(plf->lfFaceName));
    }
    HDC hdc = ::GetDC(GetHostHwnd());
    int dpiY = ::GetDeviceCaps(hdc, LOGPIXELSY);
    ::ReleaseDC(GetHostHwnd(),hdc);
    int fontSize = abs(pFont->TextSize());
    fontSize = MulDiv(fontSize,72,dpiY);
    SendEditor(SCI_STYLESETSIZE, STYLE_DEFAULT, fontSize);
    // Re-resolve all styles that inherit from STYLE_DEFAULT with the new font.
    SendEditor(SCI_STYLECLEARALL, 0, 0);
}

void SScintillaView::ApplyLineNumberMargins()
{
    if (!m_sci)
        return;
    if (m_bShowLineNumber)
    {
        SendEditor(SCI_SETMARGINTYPEN, 1, SC_MARGIN_NUMBER);
        SendEditor(SCI_SETMARGINWIDTHN, 1, 32);
        // A margin defaults to the reverse-arrow cursor (which mapped to a SIZEWE
        // "<->" on the DUI side). Give the line-number margin a plain arrow so the
        // mouse does not change shape over it when pressed.
        SendEditor(SCI_SETMARGINCURSORN, 1, SC_CURSORARROW);
    }
    else
    {
        SendEditor(SCI_SETMARGINWIDTHN, 1, 0);
    }
}

HRESULT SScintillaView::OnAttrShowLineNumber(const SStringW &strValue, BOOL bLoading)
{
    m_bShowLineNumber = (strValue != L"0");
    if (!bLoading && m_sci)
        ApplyLineNumberMargins();
    return S_OK;
}

HRESULT SScintillaView::OnAttrAutocompleteList(const SStringW &strValue, BOOL bLoading)
{
    SStringA strA = S_CW2A(strValue, CP_UTF8);
    m_strAccAll = strA;
    // Pre-parse the space-separated candidates once (XML attribute load) into a
    // case-insensitively sorted vector, so TriggerAutoComplete can binary-search
    // and range-filter by prefix instead of re-splitting the raw string each time.
    m_vAccWords.clear();
    const char *p = strA.c_str();
    while (p && *p)
    {
        const char *sp = strchr(p, ' ');
        const int len = sp ? static_cast<int>(sp - p) : static_cast<int>(strlen(p));
        if (len > 0)
            m_vAccWords.push_back(SStringA(p, len));
        p = sp ? sp + 1 : NULL;
    }
    std::sort(m_vAccWords.begin(), m_vAccWords.end(), AccWordLess());
    (void)bLoading;
    return S_OK;
}

HRESULT SScintillaView::OnAttrLexer(const SStringW &strValue, BOOL bLoading)
{
    m_strLexer = strValue;
    if (!bLoading && m_sci)
        ApplyLexer();
    return S_OK;
}

void SScintillaView::ApplyLexer()
{
    if (!m_sci)
        return;
    // Table for (style, foreground, bold). Small linear scan; this runs only when
    // the attribute is applied / at creation, never per keystroke.
    struct StyleEntry {
        int style;
        COLORREF fore;
        int bold;
    };
    // A palette that stays readable on a light editor background.
    const SStringW &lex = m_strLexer;
    if (lex.IsEmpty() || lex.CompareNoCase(L"text") == 0 || lex.CompareNoCase(L"none") == 0)
    {
        SendEditor(SCI_SETLEXER, SCLEX_NULL, 0);
        return;
    }
    // The XML lexer keeps its styles in 5 bits; cpp stays within 32 styles too,
    // so a shared lower style-bits setting is safe before lexing starts.
    if (static_cast<int>(SendEditor(SCI_GETSTYLEBITS, 0, 0)) < 5)
        SendEditor(SCI_SETSTYLEBITS, 5, 0);

    if (lex.CompareNoCase(L"xml") == 0)
    {
        SendEditor(SCI_SETLEXERLANGUAGE, 0, reinterpret_cast<LPARAM>("xml"));
        SendEditor(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(""));
        static const StyleEntry s_styles[] = {
            // tag name / </name> : blue bold
            {SCE_H_TAG, RGB(0, 0, 255), 1},
            {SCE_H_TAGUNKNOWN, RGB(0, 0, 255), 1},
            {SCE_H_TAGEND, RGB(0, 0, 255), 1},
            {SCE_H_XMLSTART, RGB(0, 0, 255), 1},
            {SCE_H_XMLEND, RGB(0, 0, 255), 1},
            // attribute name : red
            {SCE_H_ATTRIBUTE, RGB(255, 0, 0), 0},
            {SCE_H_ATTRIBUTEUNKNOWN, RGB(255, 0, 0), 0},
            // attribute value / tag body : dark red
            {SCE_H_DOUBLESTRING, RGB(128, 0, 64), 0},
            {SCE_H_SINGLESTRING, RGB(128, 0, 64), 0},
            {SCE_H_VALUE, RGB(128, 0, 64), 0},
            {SCE_H_NUMBER, RGB(128, 0, 64), 0},
            // <!-- --> and <![CDATA[ ]] : green
            {SCE_H_COMMENT, RGB(0, 128, 0), 0},
            {SCE_H_CDATA, RGB(0, 128, 0), 0},
        };
        for (size_t i = 0; i < ARRAYSIZE(s_styles); ++i)
        {
            SendEditor(SCI_STYLESETFORE, s_styles[i].style, s_styles[i].fore);
            SendEditor(SCI_STYLESETBOLD, s_styles[i].style, s_styles[i].bold);
        }
        return;
    }
    // Default: C / C++ / C# family.
    SendEditor(SCI_SETLEXERLANGUAGE, 0, reinterpret_cast<LPARAM>("cpp"));
    SendEditor(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(
        "alignas alignof and and_eq asm auto bitand bitor bool break case catch "
        "char char16_t char32_t class compl const constexpr const_cast continue "
        "decltype default delete do double dynamic_cast else enum explicit export "
        "extern false float for friend goto if inline int long mutable namespace "
        "new noexcept not not_eq nullptr operator or or_eq private protected public "
        "register reinterpret_cast return short signed sizeof static static_assert "
        "static_cast struct switch template this thread_local throw true try typedef "
        "typeid typename union unsigned using virtual void volatile wchar_t while "
        "xor xor_eq"));
    // Preprocessor branch (keyword list 3): #if/#else/#endif/...
    SendEditor(SCI_SETKEYWORDS, 3, reinterpret_cast<LPARAM>(
        "if elif else endif define undef include line error pragma assumed import "
        "ifdef ifndef"));
    static const StyleEntry s_styles[] = {
        {SCE_C_DEFAULT, RGB(0, 0, 0), 0},
        {SCE_C_IDENTIFIER, RGB(0, 0, 0), 0},
        {SCE_C_OPERATOR, RGB(0, 0, 0), 0},
        // keywords : blue bold
        {SCE_C_WORD, RGB(0, 0, 255), 1},
        // numbers : brown
        {SCE_C_NUMBER, RGB(128, 128, 0), 0},
        // strings / chars : dark red
        {SCE_C_STRING, RGB(165, 21, 21), 0},
        {SCE_C_CHARACTER, RGB(165, 21, 21), 0},
        {SCE_C_STRINGEOL, RGB(165, 21, 21), 0},
        {SCE_C_STRINGRAW, RGB(165, 21, 21), 0},
        // comments : green (doc comments bold)
        {SCE_C_COMMENT, RGB(0, 128, 0), 0},
        {SCE_C_COMMENTLINE, RGB(0, 128, 0), 0},
        {SCE_C_COMMENTDOC, RGB(0, 128, 0), 1},
        {SCE_C_COMMENTLINEDOC, RGB(0, 128, 0), 1},
        {SCE_C_COMMENTDOCKEYWORD, RGB(0, 0, 128), 1},
        // preprocessor (#include/#define/...) : purple
        {SCE_C_PREPROCESSOR, RGB(128, 0, 128), 0},
    };
    for (size_t i = 0; i < ARRAYSIZE(s_styles); ++i)
    {
        SendEditor(SCI_STYLESETFORE, s_styles[i].style, s_styles[i].fore);
        SendEditor(SCI_STYLESETBOLD, s_styles[i].style, s_styles[i].bold);
    }
}

void SScintillaView::TriggerAutoComplete()
{
    // Only offer suggestions while the user has typed at least one character of
    // the current token; with an empty prefix there is nothing to match against.
    if (!m_sci)
        return;
    // lenEntered (SCI_AUTOCSHOW wParam) is the number of characters the user has
    // ALREADY typed for the current token, NOT the candidate-list length. The
    // engine uses it as startLen to compute the current word
    // (RangeText(posStart-startLen, caret)) and Select() it against the list.
    const int pos = static_cast<int>(SendEditor(SCI_GETCURRENTPOS, 0, 0));
    const int wordStart = static_cast<int>(SendEditor(SCI_WORDSTARTPOSITION, pos, 0));
    const int lenEntered = pos - wordStart;
    if (lenEntered <= 0)
        return;
    // Fetch the typed prefix. Cap the range at the buffer size; a longer token is
    // an edge case where the first 255 chars still drive the prefix match.
    char buf[256] = {0};
    Sci_TextRange tr;
    tr.chrg.cpMin = wordStart;
    tr.chrg.cpMax = pos > wordStart + 255 ? wordStart + 255 : pos;
    tr.lpstrText = buf;
    SendEditor(SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&tr));
    const char *prefix = buf;
    const int prefixLen = static_cast<int>(strlen(prefix));
    if (prefixLen <= 0)
        return;

    BOOL bSorted = FALSE;
    SStringA strList;
    if (!m_vAccWords.empty())
    {
        // Internal `autocompleteList` path: candidates were pre-parsed and sorted
        // case-insensitively at XML attribute load; binary-search the first word
        // >= prefix, then collect the contiguous words starting with prefix.
        const SStringA strPrefix(prefix);
        std::vector<SStringA>::const_iterator it =
            std::lower_bound(m_vAccWords.begin(), m_vAccWords.end(), strPrefix,
                             AccWordLess());
        for (; it != m_vAccWords.end(); ++it)
        {
            if (it->GetLength() < prefixLen ||
                it->Mid(0, prefixLen).CompareNoCase(prefix) != 0)
                break; // sorted order keeps all prefix matches contiguous
            if (!strList.IsEmpty())
                strList += ' ';
            strList += *it;
        }
        bSorted = TRUE;
    }
    else
    {
        // No internal list configured: fire a synchronous SOUI event so the
        // business layer can supply candidates. The handler fills
        // strCandidates (space separated) during FireEvent; we read it back
        // immediately and feed it to the engine.
        EventScintillaAccQuery evt(this);
        evt.strPrefix = prefix;
        evt.nLenEntered = lenEntered;
        evt.bSorted = FALSE;
        evt.strCandidates = &strList;
        FireEvent(&evt);
        bSorted = evt.bSorted;
    }
    // No candidate matches the typed prefix (or the business layer provided none):
    // nothing to offer, keep the popup closed instead of feeding the full list back.
    if (strList.IsEmpty())
        return;
    if(!bSorted){
        SendEditor(SCI_AUTOCSETORDER,SC_ORDER_PERFORMSORT);
    }else{
        SendEditor(SCI_AUTOCSETORDER,SC_ORDER_PRESORTED);
    }
    SendEditor(SCI_AUTOCSHOW, (WPARAM)lenEntered, (LPARAM)strList.c_str());
}

SWindow *SScintillaView::GetDropDownOwner()
{
    return this;
}

BOOL SScintillaView::CreateAccListBox()
{
    // Create the dropdown list once, detached from the editor's tree. It is moved
    // into the dropdown root by OnCreateDropDown (and back out by OnDestroyDropDown).
    SListBox *pListBox = sobj_cast<SListBox>(CreateChildByName(SListBox::GetClassName()));
    if (!pListBox)
        return FALSE;
    m_pAccListBox = pListBox;
    // Default visual: fill the whole dropdown and chain selection events to us.
    m_pAccListBox->SetContainer(GetContainer());
    m_pAccListBox->SetAttribute(L"pos", L"0,0,-0,-0", TRUE);
    // Default styling mirrors SSearchDropdownList::CreateListBox: theme background
    // and border, a 1px margin, and hover tracking on the rows.
    m_pAccListBox->GetStyle().m_crBg = GETCOLOR(SNamedColor::THEME_COLOR);
    m_pAccListBox->GetStyle().m_crBorder = GETCOLOR(SNamedColor::THEME_BORDER);
    m_pAccListBox->SetAttribute(L"margin", L"1,1,1,1");
    m_pAccListBox->SetAttribute(L"hotTrack", L"1", TRUE);
    m_pAccListBox->SetOwner(this); // chain notify events back to the editor control
    m_pAccListBox->SetVisible(FALSE);
    m_pAccListBox->SetID(IDC_DROPDOWN_LIST);
    m_pAccListBox->SSendMessage(UM_SETSCALE, GetScale());
    m_pAccListBox->SSendMessage(WM_CREATE);
    return TRUE;
}

void SScintillaView::OnCreateDropDown(SDropDownWnd *pDropDown)
{
    SWindow *pRoot = pDropDown->GetRoot();
    if (!pRoot || !m_pAccListBox)
        return;
    GetContainer()->OnDropdownState(pDropDown, TRUE);
    pRoot->InsertChild(m_pAccListBox);
    pRoot->UpdateChildrenPosition();
    pRoot->SDispatchMessage(UM_SETSCALE, GetScale(), 0);
    m_pAccListBox->SetVisible(TRUE);
    m_pAccListBox->SetFocus();
    //SLOGI()<<"OnCreateDropDown,m_hWnd="<<pDropDown->m_hWnd;
}

void SScintillaView::OnDestroyDropDown(SDropDownWnd *pDropDown)
{
    //SLOGI()<<"OnDestroyDropDown,m_hWnd="<<pDropDown->m_hWnd;
    GetContainer()->OnDropdownState(pDropDown, FALSE);
    // IDOK means the user accepted the current row (Enter, committed by the
    // completion path); any other dismissal cancels the engine's being-pending
    // autocomplete (Escape, clicking outside, or no more matches).
    if (pDropDown->GetExitCode() == IDOK)
        CompleteAutoComplete(-1);
    else
        SendEditor(SCI_AUTOCCANCEL, 0, 0);

    if (m_pAccListBox && pDropDown->GetRoot())
    {
        pDropDown->GetRoot()->RemoveChild(m_pAccListBox);
        m_pAccListBox->SetVisible(FALSE);
        m_pAccListBox->SetContainer(GetContainer());
    }
    m_pAccDropDown = NULL;
    m_bAccPopup = FALSE;
}

BOOL SScintillaView::CalcAccPopupRect(int nHeight, CRect &rcPopup)
{
    // Anchor below the caret line. The caret is control-local; build the caret
    // line rect in frame coordinates, then convert to screen with the same chain
    // as SComboBase::CalcPopupRect / SSearchDropdownList::AdjustDropdownList:
    // frame -> host-client (FrameToHost) -> screen (ClientToScreen).
    const int w = 220;
    int rowH = (int)SendEditor(SCI_TEXTHEIGHT, 0, 0);
    if (rowH <= 0)
        rowH = 16;

    CRect rcBuddy;
    GetClientRect(&rcBuddy);
    rcBuddy.left += m_nAccCaretX;
    rcBuddy.top += m_nAccCaretY;
    rcBuddy.right = rcBuddy.left + w;
    rcBuddy.bottom = rcBuddy.top + rowH;
    GetContainer()->FrameToHost(&rcBuddy);
    HWND hHost = GetHostHwnd();
    if (hHost)
    {
        ::ClientToScreen(hHost, (LPPOINT)&rcBuddy);
        ::ClientToScreen(hHost, ((LPPOINT)&rcBuddy) + 1);
    }

    HMONITOR hMon = MonitorFromWindow(hHost, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(MONITORINFO) };
    GetMonitorInfo(hMon, &mi);

    // Anchor at the caret x, but pull the popup back so it never overflows the
    // right (or left) edge of the monitor.
    int x = rcBuddy.left;
    if (x + w > mi.rcMonitor.right)
        x = mi.rcMonitor.right - w;
    if (x < mi.rcMonitor.left)
        x = mi.rcMonitor.left;

    if (rcBuddy.bottom + nHeight <= mi.rcMonitor.bottom)
    {
        rcPopup = CRect(x, rcBuddy.bottom, x + w, rcBuddy.bottom + nHeight);
        return TRUE;
    }
    else
    {
        rcPopup = CRect(x, rcBuddy.top - nHeight, x + w, rcBuddy.top);
        return FALSE;
    }
}

void SScintillaView::ApplyAutoComplete(const ScintillaAutoCompleteInfo &info)
{
    if (!info.visible || info.count <= 0 || !m_pAccListBox)
    {
        // Engine has no (more) candidates: dismiss the dropdown if it is up.
        if (m_pAccDropDown && m_bAccPopup)
        {
            m_pAccDropDown->EndDropDown(IDCANCEL); // OnDestroyDropDown cancels
        }
        return;
    }

    // (Re)create the SOUI dropdown window once; it self-deletes on destroy.
    if (!m_pAccDropDown)
    {
        SASSERT(!m_bAccPopup);
        m_pAccDropDown = new SAutoCompleteDropDown(this);
        GetContainer()->EnableHostPrivateUiDef(TRUE);
        m_pAccDropDown->Create(CRect(0, 0, 100, 100), 0);
        GetContainer()->EnableHostPrivateUiDef(FALSE);
        m_pAccDropDown->GetRoot()->SDispatchMessage(UM_SETSCALE, GetScale(), 0);
    }

    // Refresh the candidate rows. Suppress events so refill does not echo back a
    // preview loop into the engine; the user's Up/Down/click still fire real ones.
    m_pAccListBox->DeleteAll();
    int nSel = -1;
    for (size_t i = 0; i < info.items.size(); ++i)
    {
        SStringT str = S_CA2T(info.items[i].c_str(), CP_UTF8);
        const int idx = m_pAccListBox->AddString(str);
        if (static_cast<int>(i) == info.selection)
            nSel = idx;
    }
    if (nSel < 0 && info.selection >= 0 && info.selection < info.count)
        nSel = info.selection;
    m_pAccListBox->SetCurSel(nSel, FALSE);

    // Use the listbox's real item height so the popup matches its content; the
    // engine's SCI_TEXTHEIGHT is the text row height and can differ from the
    // SOUI listbox row (item padding). Fall back to the engine height only when
    // the listbox has no cached height yet.
    int rowH = m_pAccListBox ? m_pAccListBox->GetItemHeight() : 0;
    if (rowH <= 0)
        rowH = (int)SendEditor(SCI_TEXTHEIGHT, 0, 0);
    if (rowH <= 0)
        rowH = 16;
    // Cap the visible rows (like SSearchDropdownList caps maxDropHeight); the
    // listbox scrolls internally when there are more candidates.
    int nVisible = info.count;
    if (nVisible > 10)
        nVisible = 10;
    const int nHeight = rowH * nVisible + 2; // + border

    CRect rcPopup;
    CalcAccPopupRect(nHeight, rcPopup);
    // Show (or reposition) without activating and take mouse capture, exactly
    // like SSearchDropdownList::AdjustDropdownList. SWP_NOACTIVATE keeps native
    // focus on the editor so further typing keeps refining the list through the
    // engine, and SetCapture routes clicks/wheel back into the dropdown.
    m_pAccDropDown->Adjust(
        rcPopup.left, rcPopup.top, rcPopup.Width(), rcPopup.Height());
    m_bAccPopup = TRUE;
}

BOOL SScintillaView::FireEvent(IEvtArgs *evt)
{
    if (evt->IdFrom() == IDC_DROPDOWN_LIST && m_pAccDropDown)
    {
        if (evt->GetID() == EventLBSelChanged::EventID)
        {
            // User moved the selection (Up/Down or single click): preview the
            // candidate in the editor without committing.
            const int sel = m_pAccListBox ? m_pAccListBox->GetCurSel() : -1;
            if (sel >= 0 && m_sci)
            {
                SStringT str = m_pAccListBox->GetText(sel, TRUE);
                SStringA strA = S_CT2A(str, CP_UTF8);
                SendEditor(SCI_AUTOCSELECT, 0, (LPARAM)strA.c_str());
            }
            return TRUE;
        }
        if (evt->GetID() == EventLBDbClick::EventID)
        {
            // Double-click a row commits it (closes with IDOK, then completion
            // happens in OnDestroyDropDown like Enter does).
            if (m_pAccDropDown && m_bAccPopup)
                m_pAccDropDown->EndDropDown(IDOK);
            return TRUE;
        }
    }
    return SPanel::FireEvent(evt);
}

void SScintillaView::CompleteAutoComplete(int nItem)
{
    if (!m_sci || !m_pAccListBox)
        return;
    if (nItem < 0)
        nItem = m_pAccListBox->GetCurSel();
    if (nItem < 0 || nItem >= m_pAccListBox->GetCount())
        return;
    // Select then complete through the engine's shared AutoComplete state machine,
    // exactly like the native HWND listbox.
    SStringT str = m_pAccListBox->GetText(nItem, TRUE);
    SStringA strA = S_CT2A(str, CP_UTF8);
    SendEditor(SCI_AUTOCSELECT, 0, (LPARAM)strA.c_str());
    SendEditor(SCI_AUTOCCOMPLETE, 0, 0);
    // Completion moves the engine caret via SetEmptySelection, which now pushes
    // the new caret geometry through NotifyCaret; no explicit pull needed.
    // Commit ends the dropdown with IDOK (the engine hides autocomplete on its own,
    // but close explicitly so focus returns to the editor).
    if (m_pAccDropDown && m_bAccPopup)
    {
        m_pAccDropDown->EndDropDown(IDOK);
        m_bAccPopup = FALSE;
    }
}

SNSEND