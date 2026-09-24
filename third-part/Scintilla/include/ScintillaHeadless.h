// Scintilla source code edit control
/** @file ScintillaHeadless.h
 ** Public headless (no native window) entry points for ScintillaWin.
 ** Lets a host render the editor to a caller supplied HDC and forward mouse,
 ** keyboard, focus, size, scroll and timer events without a real HWND, and
 ** receive SCN_* notifications through a callback.
 **/
// Based on ScintillaWin.cxx, which defines these symbols.

#ifndef SCINTILLAHEADLESS_H
#define SCINTILLAHEADLESS_H

#include "Scintilla.h"
#include "ScintillaHost.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Create a headless editor bound to the given host. The host must be supplied
/// by the caller (with its client rectangle, notification and timer callbacks)
/// and must outlive the returned editor.
void *Scintilla_CreateHeadless(ScintillaHeadlessHost *sciHost);

/// Destroy an editor created by Scintilla_CreateHeadless.
void Scintilla_DestroyHeadless(void *sci);

/// Paint the whole editor to hdc within rc.
sptr_t Scintilla_PaintHeadless(void *sci, HDC hdc, const RECT *rc);

/// Drive one engine fine ticker (e.g. caret blink) after the owning headless
/// window's own timer fired. The window must invalidate separately to repaint.
sptr_t Scintilla_TickHeadless(void *sci, int reason);

/// Return the engine's Window::Cursor value for a control-local point (ibeam
/// over text, reverse-arrow over the margin, hand over hotspots, arrow over the
/// selection) without setting the platform cursor. The caller maps the returned
/// value onto its own cursor and applies it.
int Scintilla_CursorForPointHeadless(void *sci, int x, int y);

/// Force the engine to push its current caret geometry to the headless owner
/// (UpdateSystemCaret / NotifyCaret). Use after an operation such as scrolling
/// where the engine may not have issued the caret notification on its own; the
/// owner keeps its SOUI SCaret in sync with the actual caret position.
void Scintilla_UpdateCaretHeadless(void *sci);

/// Direct call into the editor (equivalent to a native SendMessage(SCI_*),
/// used because a headless editor has no window to send messages to).
sptr_t Scintilla_DirectFunction(void *sci, unsigned int iMessage, uptr_t wParam, sptr_t lParam);

#ifdef __cplusplus
}
#endif

#endif