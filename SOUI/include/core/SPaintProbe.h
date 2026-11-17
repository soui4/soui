///////////////////////////////////////////////////////////////////////
/** Class Name: SPaintProbe
 *
 *  Opt-in instrumentation for the paint dispatcher (SWindow::DispatchPaint /
 *  SHostWnd::_RedrawRegion).  It answers one question: **a frame that dirties
 *  region R ends up doing how much work, and on what**.
 *
 *  Why this exists (see soui-freertos/esp32s3/README.md 11.17):
 *  a moving child window invalidates old-rect + new-rect, the two are merged
 *  by RGN_OR, and the merged shape is then re-painted.  Whether that is
 *  "1 x |R| background + 1 x |B| child" (correct) or "the whole bounding box /
 *  every window in it" (wrong) cannot be told from outside, because the GDI
 *  clip makes the *rasterised* pixels invisible from the caller.  So we log,
 *  per WM_PAINT:
 *
 *    - the dirty region's bounding box and how many of its rows are fully
 *      covered (a 2-rect union and a single merged rect differ here),
 *    - how many windows entered DispatchPaint, how many of them actually
 *      painted their client, and the area they *asked* to paint,
 *    - stage timing: ClearRect / root dispatch / AfterPaint,
 *    - per-window cost, top N.
 *
 *  Zero cost when disabled: every entry point starts with `if (!On()) return;`
 *  and On() is one pointer test.  Header-only with function-local statics so
 *  SOUI and the firmware share one instance without touching any build file.
 */
///////////////////////////////////////////////////////////////////////

#ifndef __SPAINTPROBE__H__
#define __SPAINTPROBE__H__

#include <stdio.h>

namespace SOUI
{
namespace probe
{

typedef unsigned long long (*FnNowUs)();
typedef void (*FnLog)(const char *);

enum
{
    kMaxWin = 6 // per-frame detail: how many windows to report
};

struct SWinCost
{
    const wchar_t *pszClass;
    int x, y, w, h;
    unsigned us;
    long long px;
};

struct SState
{
    // --- wiring (set by the application before the UI runs) ---
    FnNowUs now_us;     // microsecond clock; NULL disables timing
    FnLog log;          // line sink; NULL disables the whole probe
    unsigned nEvery;    // emit the summary line every N WM_PAINTs
    unsigned nEveryWin; // emit the per-window detail every N frames (0 = never)
    unsigned nEmit;     // cumulative emitted lines (cache-only frames have no nFrame++)

    // --- per frame ---
    unsigned nFrame;
    unsigned long long tFrame; // Now() at frame start

    int bx, by, bw, bh; // dirty region bounding box
    int nRowsFull;      // rows of the bbox fully covered by the region

    unsigned usClear, usRoot, usAfter, usAll;

    int nVisit;     // DispatchPaint calls that passed the region test
    int nEnter;     // DispatchPaint entries (whole tree walk, incl. early outs)
    int nClient;    // _PaintClient calls (WM_ERASEBKGND + WM_PAINT sent)
    int nNonClient; // WM_NCPAINT calls
    int nLayered;

    // Self time of every DispatchPaint call, summed.  Deliberately excludes
    // _PaintChildren, which is just the sum of the children's self times --
    // otherwise the tree is counted once per level.
    unsigned usSetup;  // entry -> just before _PaintClient (region test, matrix)
    unsigned usClient; // _PaintClient
    unsigned usTail;   // nonclient + layered commit + matrix restore
    unsigned usBef;    // BeforePaint  (font/text-colour select) -- per window
    unsigned usAft;    // AfterPaint   (restore)
    unsigned usExcl;   // SaveClip + _ExcludeVideoCanvasFromPaint, once per frame
    unsigned usSave;   // the SaveClip half of usExcl (a cairo/DC save per frame)

    // Absolute stamps around SHostWnd::OnPrint, to separate "SOUI paint
    // dispatch" (usAll) from the platform's GetDC/present tail.
    unsigned long long tPrint, tPrePresent, tPostPresent;

    // --- the frame as a whole ---------------------------------------------
    // usGap is what no amount of paint optimisation can shorten: the wall
    // clock between two OnPrint entries.  If usGap >> onprint, the frame rate
    // is set by the message pump / animator tick, not by rendering.
    unsigned long long tPrevPrint, usGap;
    unsigned usLayout;    // GetRoot()->UpdateLayout()
    unsigned usZorder;    // BuildWndTreeZorder()
    unsigned zorderCalls; // cumulative: frames that rebuilt the z-order
    unsigned cacheOnly;   // cumulative: frames that only re-presented the canvas
    unsigned layoutDirty; // cumulative: frames whose root was layout-dirty on entry
    bool bRepaint;        // this frame took the _RedrawRegion branch

    // WM_PAINT is BeginPaint -> OnPrint -> EndPaint.  `paint` in the 3s report
    // is that whole message; these two split off the parts OnPrint cannot see:
    // usBegin is GetDC + ExtSelectClipRgn + GetClipBox, usEnd is EndPaint ->
    // ReleaseDC -> commitCanvas -> blitCanvas (+ SPI).
    unsigned usBegin, usEnd;
    // Split of the OnPrint body that usAll cannot see: everything before
    // _RedrawRegion's FrameBegin() stamp (region swap, BeginDraw, BeforePaint,
    // PushClipRegion -> cairo_clip) and the presenter copy that follows it
    // (SHostPresenter::OnHostPresent -> BitBlt memRT -> window canvas).
    unsigned usPre;     // OnPrint entry .. FrameBegin
    unsigned usPreRgn;  // the region swap / CreateRegion part of usPre
    unsigned usPreDraw; // BeginDraw + BeforePaint part of usPre
    unsigned usPreClip; // PushClipRegion (cairo_clip) part of usPre
    unsigned usPresent; // UpdatePresenter (the blit to the window canvas)

    long long pxAsked; // sum of area(winRect & bbox) over painted windows

    int nWin;
    SWinCost win[kMaxWin]; // kept sorted by cost, descending
};

inline SState &S()
{
    static SState s = SState();
    return s;
}

inline bool On()
{
    return S().log != NULL;
}

inline unsigned long long Now()
{
    SState &s = S();
    return s.now_us ? s.now_us() : 0ULL;
}

/** First thing in SHostWnd::OnPrint: stamps the frame period (usGap) and
 *  resets the two stages that run *before* _RedrawRegion, so FrameBegin()
 *  cannot clobber them. */
inline void PrintBegin()
{
    SState &s = S();
    s.tPrint = Now();
    s.usGap = s.tPrevPrint ? (s.tPrint - s.tPrevPrint) : 0ULL;
    s.tPrevPrint = s.tPrint;
    s.usLayout = s.usZorder = 0;
}

inline void PaintPhase(unsigned begin, unsigned end)
{
    S().usBegin = begin;
    S().usEnd = end;
}

/** UpdatePresenter half of the frame: memRT -> window canvas copy. */
inline void Present(unsigned us)
{
    S().usPresent = us;
}

/** Split of usPre, stamped right after the render target clip is installed. */
inline void PreDraw(unsigned usRgn, unsigned usDraw, unsigned usClip)
{
    SState &s = S();
    s.usPreRgn = usRgn;
    s.usPreDraw = usDraw;
    s.usPreClip = usClip;
}

inline void Layout(unsigned us, bool bDirty)
{
    SState &s = S();
    s.usLayout = us;
    if (bDirty)
        ++s.layoutDirty;
}

inline void Zorder(unsigned us, bool bRepaint)
{
    SState &s = S();
    s.usZorder = us;
    s.bRepaint = bRepaint;
    if (bRepaint)
        ++s.zorderCalls;
    else
        ++s.cacheOnly;
}

inline void FrameBegin(int l, int t, int r, int b)
{
    SState &s = S();
    ++s.nFrame;
    s.tFrame = Now();
    s.bx = l;
    s.by = t;
    s.bw = r - l;
    s.bh = b - t;
    s.nRowsFull = -1;
    s.usClear = s.usRoot = s.usAfter = s.usAll = 0;
    s.nVisit = s.nEnter = s.nClient = s.nNonClient = s.nLayered = 0;
    s.usSetup = s.usClient = s.usTail = 0;
    s.usBef = s.usAft = s.usExcl = s.usSave = 0;
    s.usPre = (unsigned)(s.tFrame - s.tPrint);
    s.usPresent = 0;
    // tPrint is stamped by OnPrint *before* this runs -- do not clear it here.
    s.tPrePresent = s.tPostPresent = 0;
    s.pxAsked = 0;
    s.nWin = 0;
}

inline void Stage(unsigned &slot)
{
    SState &s = S();
    slot = s.now_us ? (unsigned)(Now() - s.tFrame) : 0u;
}

/** Rows of the bbox that the region covers end to end (lower bound of |R|). */
inline void RowsFull(int n)
{
    S().nRowsFull = n;
}

inline void Visit()
{
    ++S().nVisit;
}

inline void Enter()
{
    ++S().nEnter;
}

inline void Self(unsigned setup, unsigned client, unsigned tail)
{
    SState &s = S();
    s.usSetup += setup;
    s.usClient += client;
    s.usTail += tail;
}

inline void SaveRestore(unsigned bef, unsigned aft)
{
    SState &s = S();
    s.usBef += bef;
    s.usAft += aft;
}

/** Split of usExcl: the render target's SaveClip() alone. */
inline void SaveClip(unsigned us)
{
    S().usSave = us;
}

inline void NonClient()
{
    ++S().nNonClient;
}

inline void Layered()
{
    ++S().nLayered;
}

inline void WinPaint(const wchar_t *pszClass, int x, int y, int w, int h, unsigned us, long long px)
{
    SState &s = S();
    ++s.nClient;
    if (w > 0 && h > 0)
        s.pxAsked += px;

    // keep the kMaxWin most expensive windows, insertion sort (N is tiny)
    int i = s.nWin < kMaxWin ? s.nWin++ : kMaxWin;
    int pos = (i < kMaxWin) ? i : kMaxWin - 1;
    if (i == kMaxWin && us <= s.win[kMaxWin - 1].us)
        return;
    while (pos > 0 && s.win[pos - 1].us < us)
    {
        s.win[pos] = s.win[pos - 1];
        --pos;
    }
    SWinCost &d = s.win[pos];
    d.pszClass = pszClass;
    d.x = x;
    d.y = y;
    d.w = w;
    d.h = h;
    d.us = us;
    d.px = px;
}

inline void Emit()
{
    SState &s = S();
    if (!s.log)
        return;
    if (s.tPostPresent < s.tFrame) // present stamp belongs to an older frame
        return;
    if (s.nEvery && (++s.nEmit % s.nEvery) != 0)
        return;

    char buf[240];
    // The region's exact area is not exposed by IRegionS; `rf` (fully covered
    // rows of the bbox) gives a lower bound and, more to the point, tells a
    // 2-rect union (rf < bh) apart from one merged rect (rf == bh).
    _snprintf(buf, sizeof(buf), "f=%u box=%d,%d,%dx%d rf=%d/%d v=%d cli=%d nc=%d ly=%d px=%ld t=%u c%u r%u a%u", s.nFrame, s.bx, s.by, s.bw, s.bh, s.nRowsFull, s.bh, s.nVisit, s.nClient, s.nNonClient, s.nLayered, (long)s.pxAsked, s.usAll, s.usClear, s.usRoot, s.usAfter);
    s.log(buf);
    // Where the root dispatch went: self time of every window in the walk.
    _snprintf(buf, sizeof(buf), "  walk=%d setup=%u client=%u tail=%u bef=%u aft=%u save=%u excl=%u (sum=%u of %u)", s.nEnter, s.usSetup, s.usClient, s.usTail, s.usBef, s.usAft, s.usSave, s.usExcl, s.usSetup + s.usClient + s.usTail + s.usBef + s.usAft + s.usExcl, s.usRoot);
    s.log(buf);
    if (s.tPrint && s.tPostPresent)
    {
        // cycle = the real frame period; onprint - present = the platform tail
        // (commitCanvas + blitCanvas + SPI + EndPaint); cycle - onprint is time
        // not spent painting at all (pump idle, animator tick, other tasks).
        _snprintf(buf, sizeof(buf), "  cycle=%lums onprint=%lums present=%lums dispatch=%lums", (long)(s.usGap / 1000), (long)((s.tPostPresent - s.tPrint) / 1000), (long)((s.tPostPresent - s.tPrePresent) / 1000), (long)(s.usAll / 1000));
        s.log(buf);
        _snprintf(buf, sizeof(buf), "  pre: layout=%luus zorder=%luus branch=%s zc=%u/%u ld=%u/%u", (long)s.usLayout, (long)s.usZorder, s.bRepaint ? "repaint" : "cache-only", s.zorderCalls, s.nFrame, s.layoutDirty, s.nFrame);
        s.log(buf);
        // begin + onprint + end == the WM_PAINT message time the 3s report calls
        // `paint`.  Everything unaccounted for in a frame is outside WM_PAINT.
        _snprintf(buf, sizeof(buf), "  wmpaint: begin=%luus pre=%luus (rgn=%luus draw=%luus clip=%luus) print=%luus present=%luus end=%luus", (long)s.usBegin, (long)s.usPre, (long)s.usPreRgn, (long)s.usPreDraw, (long)s.usPreClip, (long)(s.tPostPresent - s.tPrint), (long)s.usPresent, (long)s.usEnd);
        s.log(buf);
    }

    for (int i = 0; i < s.nWin; ++i)
    {
        // Gate on emitted lines (nEmit), not on nFrame: Emit() is called only
        // on every nEvery-th frame, so a nFrame-based test can never agree with
        // it and the detail block would stay silent.
        if (!s.nEveryWin || (s.nEmit % s.nEveryWin) != 0)
            break;
        const SWinCost &d = s.win[i];
        char cls[24];
        int k = 0;
        if (d.pszClass)
            for (; d.pszClass[k] && k < (int)sizeof(cls) - 1; ++k)
                cls[k] = (d.pszClass[k] < 128) ? (char)d.pszClass[k] : '?';
        cls[k] = 0;
        _snprintf(buf, sizeof(buf), "  %s %d,%d,%dx%d %ldus %ldpx", cls, d.x, d.y, d.w, d.h, (long)d.us, (long)d.px);
        s.log(buf);
    }
}

} // namespace probe
} // namespace SOUI

#endif // __SPAINTPROBE__H__
