#ifndef SCINTILLAHOSTLISTENER_H
#define SCINTILLAHOSTLISTENER_H

// Scintilla type forward declarations used by the listener interface. This
// header is intentionally Scintilla-header-free so it can be included from a
// DUI window header without pulling in Platform.h/Scintilla.h (whose bare
// `Window`/`Point`/... classes conflict with SOUI::Window). This build does not
// define SCI_NAMESPACE, so all Scintilla types (PRectangle, SCNotification) are
// declared in the global namespace. Definitions live in the Scintilla headers;
// the concrete listener overrides are implemented in the .cpp where those
// headers are fully visible.
class PRectangle;
struct SCNotification;
struct ScintillaAutoCompleteInfo;

/**
 * Listener interface implemented by the owning DUI window and handed to a
 * ScintillaHeadlessHost. Consolidates all host-to-owner interactions (engine
 * notifications, timer, caret, autocomplete, invalidation) and the geometric
 * queries the host needs from the owner (client rectangle, IME window origin)
 * into one object, replacing the former set of loose function-pointer +
 * context callbacks.
 *
 * The owner must keep the object alive for as long as the host is in use (the
 * host holds a non-owning pointer). Geometric queries are pull-based: the host
 * requests fresh geometry whenever it needs it, so the owner does not have to
 * constantly push sized/moved updates.
 */
class ScintillaHeadlessListener {
public:
	virtual ~ScintillaHeadlessListener() {}

	/// Client rectangle of the control in its own local coordinate space
	/// (top-left == 0,0), used by the engine for layout and painting.
	virtual PRectangle GetClientRectangle() const = 0;

	/// Control's client-area origin (top-left) expressed in the IME HWND's
	/// client coordinate space (see ScintillaHost::GetIMEWindowOffset).
	virtual void GetIMEWindowOffset(int *px, int *py) const = 0;

	/// Runtime notifications (timer request, caret geometry change, autocomplete
	/// state change, and engine repaint hints). Subclasses override the relevant
	/// ones. Defaults are no-ops.
	virtual void RequestTimer(int reason, int millis) {}
	virtual void NotifyCaret(int x, int y, int height, bool shown) {}
	virtual void AutoCompleteNotify(const ScintillaAutoCompleteInfo &) {}
	virtual void InvalidateRectangle(int left, int top, int right, int bottom) {}
	/// Engine notification (SCN_*, ...). The SCNotification is a stack local
	/// filled by the host; copy what you need before returning.
	virtual void Notify(const SCNotification &scn) {}
};

#endif