// Scintilla source code edit control
/** @file ScintillaHost.h
 ** Host abstraction for ScintillaWin.
 ** Separates the HWND-bound operations of the Win32 platform layer from the
 ** rendering / event / notification concern, so that a headless instantiation
 ** (drawing to a caller-provided HDC and receiving forwarded messages) can reuse
 ** the same engine without a real native window.
 **/
// Copyright 2005 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be
// distributed.

#ifndef SCINTILLAHOST_H
#define SCINTILLAHOST_H

#include <windows.h>

#include <string>
#include <vector>

#include "Platform.h"
#include "Scintilla.h"
#include "ScintillaHostListener.h"

// SCNotification and PRectangle are provided by the headers above.

/// One autocomplete state snapshot delivered to the owner (via the headless
/// host's auto-complete callback) whenever the engine changes its candidate
/// list, current selection or visibility. `items`/`rcPopup` are copied by value,
/// so the owner may keep the struct beyond the callback call. rcPopup is a hint,
/// in the control's local coordinate space, of where the popup should appear
/// (the owner may compute its own from the caret position instead).
struct ScintillaAutoCompleteInfo {
	bool visible;    ///< engine wants the popup shown (a populated list)
	bool shown;      ///< the list is currently considered shown by the engine
	int count;       ///< number of candidate words in items
	int selection;   ///< currently selected index, or -1
	PRectangle rcPopup;                ///< suggested popup rect (control-local)
	std::vector<std::string> items;    ///< candidate words (UTF-8)

	ScintillaAutoCompleteInfo()
		: visible(false), shown(false), count(0), selection(-1), rcPopup(0.f,0.f,0.f,0.f) {}
};

/**
 * Interface implemented by a host that ScintillaWin uses for all operating
 * system specific window facilities. Two implementations are provided:
 * - ScintillaNativeHost: wraps a real HWND, reproducing the existing behavior.
 * - ScintillaHeadlessHost: no real window, draws to a caller supplied HDC and
 *   delivers notifications through callbacks.
 *
 * Rendering to an external HDC itself reuses the existing "OCX" path of
 * ScintillaWin::WndPaint, in which the caller passes a PAINTSTRUCT whose hdc and
 * rcPaint describe the target, so it is not duplicated here.
 */
class ScintillaHost {
public:
	virtual ~ScintillaHost() {}

	/// Whether this host operates on a real native window.
	virtual bool IsNative() const = 0;

	/// The window handle to use. For the headless host this must remain a
	/// stable non-null placeholder value (used only to make the engine's
	/// surface/auto-complete paths believe a window exists).
	virtual HWND MainHWND() const = 0;

	/// The window handle to own system services that need a real thread window
	/// (notably the clipboard). Native hosts use MainHWND; a headless host may
	/// have the DUI layer inject its real host HWND (ClipboardOwnerHwnd), instead
	/// of relying on the non-dereferenced placeholder or on NULL.
	virtual HWND ClipboardOwnerHwnd() const { return MainHWND(); }

	/// Client rectangle used for layout and drawing.
	virtual PRectangle GetClientRectangle() const = 0;


	/// Control's client-area origin (top-left) expressed in the IME HWND's client
	/// coordinate space. The engine calls this when positioning the floating IME
	/// composition/candidate window so that a control-local caret point is mapped
	/// into the host window's client space. Native hosts return 0,0.
	virtual void GetIMEWindowOffset(int *px, int *py) const {
		if (px) *px = 0;
		if (py) *py = 0;
	}

	/// Control identifier used in notifications.
	virtual int GetCtrlID() const = 0;
	virtual void SetCtrlID(int identifier) = 0;

	/// Request an area of the window to be repainted.
	virtual void InvalidateRectangle(PRectangle rc) = 0;

	/// Notification (SCN_*) delivered to the client. For the native host this
	/// sends WM_NOTIFY to the parent window, for the headless host it invokes
	/// the registered callback.
	virtual void NotifyParent(SCNotification scn) = 0;

	/// Change of content notification.
	virtual void NotifyChange(int ctrlID) = 0;

	/// Focus change notification.
	virtual void NotifyFocus(bool focus, int ctrlID) = 0;

	/// Double click handled by Scintilla; lets the container process it too.
	virtual void NotifyDoubleClick(int modifiers, int x, int y) = 0;

	/// Scroll bar manipulation. No-op for the headless host.
	virtual int SetScrollInfo(int nBar, const SCROLLINFO *lpsi, bool bRedraw) = 0;
	virtual bool GetScrollInfo(int nBar, SCROLLINFO *lpsi) = 0;

	/// Default window procedure. No-op for the headless host.
	virtual sptr_t DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam) = 0;

	/// Engine requests the host to start (millis > 0) or cancel (millis <= 0) a
	/// fine timer. Default is a no-op: native hosts drive their own WM_TIMER.
	virtual void RequestTimer(int reason, int millis) const {}

	/// Engine reports a caret geometry/visibility change (x/y in control-local
	/// space). Default is a no-op: native hosts use the OS caret instead.
	virtual void NotifyCaret(int x, int y, int height, bool shown) const {}

	/// Does the host want to take over the autocomplete list box? If true the
	/// engine hands candidates/selection/visibility to the host (via
	/// AutoCompleteNotify) instead of creating its own native popup listbox.
	/// Default is false (engine auto-creates its listbox).
	virtual bool HandlesAutoComplete() const { return false; }

	/// Engine autocomplete state change (list shown/hidden, selection moved,
	/// content replaced). Default is a no-op: native hosts render the listbox
	/// themselves. Delivered as an event (not polled), like NotifyCaret.
	virtual void AutoCompleteNotify(const ScintillaAutoCompleteInfo &) const {}
};

/**
 * Host that operates on a real HWND. All operations map directly onto the
 * Win32 API so behavior is identical to the pre-refactor ScintillaWin.
 */
class ScintillaNativeHost : public ScintillaHost {
	HWND hwnd;
public:
	explicit ScintillaNativeHost(HWND hwnd_) : hwnd(hwnd_) {}

	bool IsNative() const override {
		return true;
	}

	HWND MainHWND() const override {
		return hwnd;
	}

	PRectangle GetClientRectangle() const override {
		RECT rc;
		::GetClientRect(hwnd, &rc);
		return PRectangle::FromInts(rc.left, rc.top, rc.right, rc.bottom);
	}

	int GetCtrlID() const override {
		return ::GetDlgCtrlID(hwnd);
	}

	void SetCtrlID(int identifier) override {
		::SetWindowLongPtr(hwnd, GWLP_ID, identifier);
	}

	void InvalidateRectangle(PRectangle rc) override {
		RECT rcw = { static_cast<LONG>(rc.left), static_cast<LONG>(rc.top),
			static_cast<LONG>(rc.right), static_cast<LONG>(rc.bottom) };
		::InvalidateRect(hwnd, &rcw, FALSE);
	}

	void NotifyParent(SCNotification scn) override {
		scn.nmhdr.hwndFrom = reinterpret_cast<void *>(hwnd);
		scn.nmhdr.idFrom = GetCtrlID();
		::SendMessage(::GetParent(hwnd), WM_NOTIFY, GetCtrlID(),
			reinterpret_cast<LPARAM>(&scn));
	}

	void NotifyChange(int /*ctrlID*/) override {
		::SendMessage(::GetParent(hwnd), WM_COMMAND,
			MAKELONG(GetCtrlID(), SCEN_CHANGE),
			(LPARAM)hwnd);
	}

	void NotifyFocus(bool focus, int /*ctrlID*/) override {
		::SendMessage(::GetParent(hwnd), WM_COMMAND,
			MAKELONG(GetCtrlID(), focus ? SCEN_SETFOCUS : SCEN_KILLFOCUS),
			(LPARAM)hwnd);
	}

	void NotifyDoubleClick(int modifiers, int x, int y) override {
		::SendMessage(hwnd, WM_LBUTTONDBLCLK,
			(modifiers & SCMOD_SHIFT) ? MK_SHIFT : 0,
			MAKELPARAM(x, y));
	}

	int SetScrollInfo(int nBar, const SCROLLINFO *lpsi, bool bRedraw) override {
		return ::SetScrollInfo(hwnd, nBar, lpsi, bRedraw ? TRUE : FALSE);
	}

	bool GetScrollInfo(int nBar, SCROLLINFO *lpsi) override {
		return ::GetScrollInfo(hwnd, nBar, lpsi) ? true : false;
	}

	sptr_t DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam) override {
		return ::DefWindowProc(hwnd, iMessage, wParam, lParam);
	}
};

/**
 * Host that needs no real window. Layout comes from a caller supplied client
 * rectangle, painting goes to a caller supplied HDC (via the OCX paint path
 * of ScintillaWin), and notifications are delivered through onNotify.
 *
 * The placeholder window handle is merely a stable non-null value; it is
 * never dereferenced. Win32 operations that cannot apply headless (scroll
 * bars, system caret, default window proc, timers) must be guarded by
 * IsNative() == false in ScintillaWin.
 */
class ScintillaHeadlessHost : public ScintillaHost {
	HWND m_hHost;
	int ctrlID;
	ScintillaHeadlessListener *listener;
	SCROLLINFO m_siVer;
	SCROLLINFO m_siHoz;
public:
	explicit ScintillaHeadlessHost(HWND hMain,
		int ctrlID_ = 0,
		ScintillaHeadlessListener *listener_ = NULL)
		:m_hHost(hMain),
		ctrlID(ctrlID_),
		listener(listener_) {}

	/// Install the listener that receives engine notifications and provides the
	/// geometry the host queries on demand. May be (re)set any time; pass NULL to
	/// detach.
	void SetListener(ScintillaHeadlessListener *l) {
		listener = l;
	}

	/// Capture the scroll bar state the engine computes, so the owning DUI window
	/// can later poll it (via GetScrollInfo) to render its own scroll bars. Nothing
	/// is drawn here; the info is only recorded.
	int SetScrollInfo(int nBar, const SCROLLINFO *lpsi, bool /*bRedraw*/) override {
		if (!lpsi)
			return 0;
		SCROLLINFO *dst = (nBar == SB_VERT) ? &m_siVer
			: (nBar == SB_HORZ) ? &m_siHoz : NULL;
		if (!dst)
			return 0;
		// Merge only the fields requested by the caller's fMask. The engine
		// frequently updates a single field (e.g. SIF_POS via ChangeScrollPos),
		// so copying the whole struct would wipe the previously stored range/page.
		const DWORD mask = lpsi->fMask;
		if (mask & SIF_POS)
			dst->nPos = lpsi->nPos;
		if (mask & SIF_RANGE) {
			dst->nMin = lpsi->nMin;
			dst->nMax = lpsi->nMax;
		}
		if (mask & SIF_PAGE)
			dst->nPage = lpsi->nPage;
		if (mask & SIF_TRACKPOS)
			dst->nTrackPos = lpsi->nTrackPos;
		dst->cbSize = sizeof(SCROLLINFO);
		return 0;
	}

	bool GetScrollInfo(int nBar, SCROLLINFO *lpsi) override {
		if (!lpsi)
			return false;
		const SCROLLINFO &src = (nBar == SB_VERT) ? m_siVer : m_siHoz;
		if (src.cbSize == 0)
			return false; // never set by the engine yet
		// Honor the caller's fMask; src always holds the full last-known state.
		const DWORD mask = lpsi->fMask;
		if (mask & SIF_POS)
			lpsi->nPos = src.nPos;
		if (mask & SIF_RANGE) {
			lpsi->nMin = src.nMin;
			lpsi->nMax = src.nMax;
		}
		if (mask & SIF_PAGE)
			lpsi->nPage = src.nPage;
		if (mask & SIF_TRACKPOS)
			lpsi->nTrackPos = src.nTrackPos;
		return true;
	}

	/// Ask the owner to start (millis > 0) or cancel (millis <= 0) a timer.
	void RequestTimer(int reason, int millis) const override {
		if (listener)
			listener->RequestTimer(reason, millis);
	}

	/// Forward a caret geometry/visibility update to the owner.
	void NotifyCaret(int x, int y, int height, bool shown) const override {
		if (listener)
			listener->NotifyCaret(x, y, height, shown);
	}

	bool HandlesAutoComplete() const override {
		return listener != NULL;
	}

	void AutoCompleteNotify(const ScintillaAutoCompleteInfo &info) const override {
		if (listener)
			listener->AutoCompleteNotify(info);
	}

	bool IsNative() const override {
		return false;
	}

	HWND MainHWND() const override {
		return m_hHost;
	}

	void GetIMEWindowOffset(int *px, int *py) const override {
		if (px) *px = 0;
		if (py) *py = 0;
		if (listener)
			listener->GetIMEWindowOffset(px, py);
	}

	HWND ClipboardOwnerHwnd() const override {
		return MainHWND();
	}

	PRectangle GetClientRectangle() const override {
		return listener ? listener->GetClientRectangle() : PRectangle();
	}

	/// Forward a partial dirty rectangle to the owner so it can invalidate just
	/// that region and avoid a full-window repaint.
	void InvalidateRectangle(PRectangle rc) override {
		if (listener) {
			const int left = static_cast<int>(rc.left);
			const int top = static_cast<int>(rc.top);
			const int right = static_cast<int>(rc.right);
			const int bottom = static_cast<int>(rc.bottom);
			listener->InvalidateRectangle(left, top, right, bottom);
		}
	}

	int GetCtrlID() const override {
		return ctrlID;
	}

	void SetCtrlID(int identifier) override {
		ctrlID = identifier;
	}

	void NotifyParent(SCNotification scn) override {
		if (listener) {
			scn.nmhdr.hwndFrom = (void*)MainHWND();
			scn.nmhdr.idFrom = ctrlID;
			listener->Notify(scn);
		}
	}

	void NotifyChange(int /*ctrlID*/) override {
	}

	void NotifyFocus(bool /*focus*/, int /*ctrlID*/) override {
	}

	void NotifyDoubleClick(int /*modifiers*/, int /*x*/, int /*y*/) override {
	}

	sptr_t DefWndProc(unsigned int /*iMessage*/, uptr_t /*wParam*/, sptr_t /*lParam*/) override {
		return 0;
	}
};

#endif