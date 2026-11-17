#pragma once

#include <souistd.h>
#include "FluentCompat.h"
#include "FluentPopupGeometry.h"
#include <SApp.h>
#include <animation/SAlphaAnimation.h>
#include <animation/SAnimationSet.h>
#include <animation/STranslateAnimation.h>
#include <control/SComboBase.h>
#include <control/SDropDown.h>
#include <core/SRootWindow.h>
#include <helper/SMenuEx.h>
#include <res.mgr/SUiDef.h>
#include <map>
#include <memory>

namespace SNS
{
namespace FluentDemo
{
namespace PopupDetail
{
namespace Geometry = ::FluentPopupGeometry;

class EnterInterpolator : public SInterpolatorBase {
  public:
    float WINAPI getInterpolation(float input) const
    {
        return Geometry::EaseOut(input);
    }
};

inline SAutoRefPtr<IAnimation> CreateEnterAnimation(float fromX, float fromY)
{
    SAutoRefPtr<SAnimationSet> animation(new SAnimationSet(TRUE), FALSE);
    SAutoRefPtr<SAlphaAnimation> alpha(new SAlphaAnimation(), FALSE);
    alpha->init(0.0f, 1.0f);
    animation->addAnimation(alpha);
    SAutoRefPtr<STranslateAnimation> translation(new STranslateAnimation(), FALSE);
    SValueDescription x, y;
    // SLayoutSize::defUnit is mutable. Specify dp instead of inheriting it,
    // otherwise a px-authored demo would accidentally animate by four pixels.
    x.value.setSize(fromX, dp);
    y.value.setSize(fromY, dp);
    translation->init(x, SValueDescription(), y, SValueDescription());
    animation->addAnimation(translation);
    SAutoRefPtr<IInterpolator> curve(new EnterInterpolator(), FALSE);
    animation->setInterpolator(curve);
    animation->setDuration(Geometry::EnterDurationMs);
    animation->setFillAfter(FALSE);
    return SAutoRefPtr<IAnimation>(animation);
}

// Read the actual SRootWindow member through a pointer-to-member whose declaring
// class remains SRootWindow. Never cast a real root to this accessor subclass.
// Attribute storage is optional and Theme's delegate need not retain literals.
class RootAnimationAccess : public SRootWindow {
  public:
    static IAnimation *Enter(const SRootWindow &root)
    {
        SAutoRefPtr<IAnimation> SRootWindow::*member = &RootAnimationAccess::m_aniEnter;
        return root.*member;
    }
};

inline bool HasAuthoredEnter(const SRootWindow &root)
{
    if (RootAnimationAccess::Enter(root))
        return true;
    SStringW value;
    return root.GetAttribute(L"enterAnimation", &value) && !value.IsEmpty();
}

inline void CancelOwned(SWindow &root, SAutoRefPtr<IAnimation> &owned)
{
    // Never clear an unrelated animation, even if it replaced ours mid-enter.
    if (owned && root.GetAnimation() == owned)
        root.ClearAnimation();
    owned = NULL;
}

inline bool ApplyChrome(SWindow &root, int innerGapDp, const wchar_t *shadowSkin = L"fluent_popup_shadow")
{
    if (!GETSKIN(shadowSkin, root.GetScale()))
        return false; // No motion-only attachment masquerading as chrome.
    const int scale = root.GetScale() > 0 ? root.GetScale() : 100;
    const CRect old = root.GetStyle().GetPadding();
    const double pixelsToDp = 100.0 / scale;
    const SStringW padding = SStringW().Format(L"%gdp,%gdp,%gdp,%gdp", 18.0 + (old.left * pixelsToDp > innerGapDp ? old.left * pixelsToDp : innerGapDp), 12.0 + (old.top * pixelsToDp > innerGapDp ? old.top * pixelsToDp : innerGapDp), 18.0 + (old.right * pixelsToDp > innerGapDp ? old.right * pixelsToDp : innerGapDp), 24.0 + (old.bottom * pixelsToDp > innerGapDp ? old.bottom * pixelsToDp : innerGapDp));
    if (FAILED(root.SetAttribute(L"skin", shadowSkin)))
        return false;
    root.SetAttribute(L"colorBkgnd", L"#00000000");
    root.SetAttribute(L"colorBorder", L"#00000000");
    // The nine-slice already owns the panel border and per-pixel opacity.
    // Old ncSkin/uniform alpha would repaint the transparent shadow gutter.
    root.SetAttribute(L"ncSkin", L"");
    root.SetAttribute(L"alpha", L"255");
    const bool applied = SUCCEEDED(root.SetAttribute(L"padding", padding));
    if (applied)
        root.RequestRelayout();
    return applied;
}

inline bool KeyboardActive()
{
#ifdef _WIN32
    return (GetKeyState(VK_F4) & 0x8000) || (GetKeyState(VK_DOWN) & 0x8000) || (GetKeyState(VK_UP) & 0x8000) || (GetKeyState(VK_RETURN) & 0x8000) || (GetKeyState(VK_SPACE) & 0x8000);
#else
    return false;
#endif
}

inline bool IsKeyMessage(UINT message)
{
    return message == WM_KEYDOWN || message == WM_SYSKEYDOWN || message == WM_KEYUP || message == WM_SYSKEYUP;
}

inline bool SystemAllowsMotion()
{
#ifdef _WIN32
    // Some demos intentionally compile with an XP-era WINVER. Query the
    // documented action locally instead of raising their target or defining a
    // global SDK macro. Unsupported/failed queries conservatively skip motion.
    // Microsoft Learn: SystemParametersInfoA, SPI_GETCLIENTAREAANIMATION=0x1042.
    const UINT getClientAreaAnimation = 0x1042;
    BOOL enabled = FALSE;
    if (!SystemParametersInfo(getClientAreaAnimation, 0, &enabled, 0) || !enabled)
        return false;
    enabled = FALSE;
    return SystemParametersInfo(SPI_GETMENUANIMATION, 0, &enabled, 0) && enabled;
#else
    // swinx still gets chrome/placement, but not unverified native motion.
    return false;
#endif
}

struct Attachment
{
    Attachment()
        : refs(1)
        , host(NULL)
        , previous()
        , keyboard()
        , reduced(false)
        , dropdown(false)
        , active(true)
        , chrome(false)
        , fixedBodyWidthPx(-1)
        , showPending(false)
        , foreignEnterObserved(false)
        , parent(NULL)
        , parentItem(0)
        , owner(NULL)
        , explicitBuddy(false)
        , buddy(0, 0, 0, 0)
        , trackFlags(TPM_LEFTALIGN)
        , explicitMenuAnchor(false)
        , menuAnchor()
        , fromX(0.0f)
        , fromY(-Geometry::EnterDistanceDp)
    {
    }
    void AddRef()
    {
        ++refs;
    }
    void Release()
    {
        if (--refs == 0)
            delete this;
    }

    long refs;
    SHostWnd *host;
    MsgHandlerInfo previous;
    BoolFlagPtr keyboard;
    bool reduced;
    bool dropdown;
    bool active;
    bool chrome;
    int fixedBodyWidthPx;
    bool showPending;
    bool foreignEnterObserved;
    SHostWnd *parent;
    SWND parentItem;
    SWindow *owner;
    bool explicitBuddy;
    Geometry::Rect buddy;
    UINT trackFlags;
    bool explicitMenuAnchor;
    POINT menuAnchor;
    float fromX;
    float fromY;
    SAutoRefPtr<IAnimation> owned;
};

typedef SharedPtr<Attachment> AttachmentPtr;
typedef std::map<SHostWnd *, AttachmentPtr> AttachmentMap;

// The registry detects our handler even when another owner has chained above
// it. Context lifetime extends through reentrant old handlers and NCDESTROY.
// Like SOUI hosts, this registry is UI-thread-affine, not a global worker API.
inline AttachmentMap &Attachments()
{
    static AttachmentMap values;
    return values;
}

inline Geometry::Rect RectOf(const RECT &rect)
{
    return Geometry::Rect(rect.left, rect.top, rect.right, rect.bottom);
}

inline Geometry::Insets PaddingOf(const SWindow &root)
{
    const CRect padding = root.GetStyle().GetPadding();
    return Geometry::Insets(padding.left, padding.top, padding.right, padding.bottom);
}

inline bool ScreenRect(SWindow &window, Geometry::Rect &out)
{
    ISwndContainer *container = window.GetContainer();
    if (!container || !::IsWindow(container->GetHostHwnd()))
        return false;
    CRect rect = window.GetWindowRect();
    container->FrameToHost(&rect);
    CPoint origin = rect.TopLeft();
    if (!::ClientToScreen(container->GetHostHwnd(), &origin))
        return false;
    rect.MoveToXY(origin.x, origin.y);
    out = RectOf(rect);
    return out.Width() > 0 && out.Height() > 0;
}

inline Geometry::Rect WorkArea(const Geometry::Rect &anchor)
{
    const RECT rect = { anchor.left, anchor.top, anchor.right, anchor.bottom };
    MONITORINFO monitor = { sizeof(MONITORINFO) };
    if (GetMonitorInfo(MonitorFromRect(&rect, MONITOR_DEFAULTTONEAREST), &monitor))
        return RectOf(monitor.rcWork);
    // A missing monitor must not push a popup to (0,0) or shrink it to zero.
    return Geometry::Rect(-0x10000000, -0x10000000, 0x10000000, 0x10000000);
}

inline void AdjustPosition(Attachment &attachment, WINDOWPOS &position)
{
    if (!attachment.active || !attachment.chrome || (position.flags & SWP_NOSIZE) || (position.flags & SWP_NOMOVE) || position.cx <= 0 || position.cy <= 0)
        return;
    if (!attachment.dropdown && ::IsWindowVisible(attachment.host->m_hWnd))
        return;
    SWindow *root = attachment.host->GetRoot();
    const Geometry::Rect incoming(position.x, position.y, position.x + position.cx, position.y + position.cy);
    Geometry::Placement placement;
    if (attachment.dropdown)
    {
        Geometry::Rect buddy = attachment.buddy;
        if (!attachment.explicitBuddy && (!attachment.owner || !ScreenRect(*attachment.owner, buddy)))
            return;
        if (buddy.Width() <= 0 || buddy.Height() <= 0)
            return;
        placement = Geometry::PlaceCombo(buddy, position.cy, WorkArea(buddy), root->GetScale());
    }
    else if (attachment.parent && Attachments().count(attachment.parent))
    {
        const Geometry::Rect parent = RectOf(attachment.parent->GetWindowRect());
        Geometry::Rect trigger;
        SWindow *item = SWindowMgr::GetWindow(attachment.parentItem);
        const int triggerTop = item && ScreenRect(*item, trigger) ? trigger.top : incoming.top;
        placement = Geometry::PlaceSubmenu(parent, triggerTop, position.cx, position.cy, WorkArea(parent), root->GetScale(), PaddingOf(*attachment.parent->GetRoot()), PaddingOf(*root), incoming.left >= parent.left);
    }
    else
    {
        const Geometry::Alignment horizontal = attachment.trackFlags & TPM_RIGHTALIGN ? Geometry::AlignEnd : attachment.trackFlags & TPM_CENTERALIGN ? Geometry::AlignCenter : Geometry::AlignStart;
        const Geometry::Alignment vertical = attachment.trackFlags & TPM_BOTTOMALIGN ? Geometry::AlignEnd : attachment.trackFlags & TPM_VCENTERALIGN ? Geometry::AlignCenter : Geometry::AlignStart;
        Geometry::Rect unadjusted = incoming;
        Geometry::Rect anchor = incoming;
        if (attachment.explicitMenuAnchor)
        {
            const POINT point = attachment.menuAnchor;
            unadjusted = Geometry::MenuRectAt(point.x, point.y, position.cx, position.cy, horizontal, vertical);
            anchor = Geometry::Rect(point.x, point.y, point.x + 1, point.y + 1);
        }
        placement = Geometry::PlaceMenu(unadjusted, WorkArea(anchor), root->GetScale(), horizontal, vertical);
    }
    position.x = placement.bounds.left;
    position.y = placement.bounds.top;
    position.cx = placement.bounds.Width();
    position.cy = placement.bounds.Height();
    attachment.fromX = placement.fromX;
    attachment.fromY = placement.fromY;
}

inline void BeginEnter(Attachment &attachment)
{
    SWindow *root = attachment.host->GetRoot();
    if (!root || !attachment.active)
        return;
    const bool foreign = root->GetAnimation() && root->GetAnimation() != attachment.owned;
    attachment.foreignEnterObserved = attachment.foreignEnterObserved || foreign || HasAuthoredEnter(*static_cast<SRootWindow *>(root));
    if (!Geometry::ShouldEnter(attachment.reduced, *attachment.keyboard || KeyboardActive(), root->GetAnimation() != NULL, attachment.foreignEnterObserved, SystemAllowsMotion()))
        return;
    attachment.owned = CreateEnterAnimation(attachment.fromX, attachment.fromY);
    root->StartAnimation(attachment.owned, TRUE);
}

inline BOOL HandleMessage(const LPMSG message, LRESULT *result, void *context)
{
    Attachment *raw = static_cast<Attachment *>(context);
    AttachmentMap::iterator found = Attachments().find(raw->host);
    if (found == Attachments().end() || found->second.get() != raw)
        return FALSE;
    const AttachmentPtr attachment = found->second;
    const MsgHandlerInfo previous = attachment->previous;
    SHostWnd *host = attachment->host;
    if (IsKeyMessage(message->message))
    {
        *attachment->keyboard = true;
        attachment->showPending = false;
        CancelOwned(*host->GetRoot(), attachment->owned);
    }
    if (message->message == WM_WINDOWPOSCHANGING)
    {
        WINDOWPOS *position = reinterpret_cast<WINDOWPOS *>(message->lParam);
        AdjustPosition(*attachment, *position);
        if (position->flags & SWP_SHOWWINDOW)
            attachment->showPending = !::IsWindowVisible(message->hwnd);
        if (position->flags & SWP_HIDEWINDOW)
        {
            attachment->showPending = false;
            CancelOwned(*host->GetRoot(), attachment->owned);
        }
    }
    if ((message->message == WM_SHOWWINDOW && !message->wParam) || message->message == WM_CANCELMODE || message->message == WM_COMMAND || message->message == WM_DESTROY || message->message == WM_NCDESTROY)
    {
        attachment->showPending = false;
        CancelOwned(*host->GetRoot(), attachment->owned);
    }
    if (message->message == WM_NCDESTROY)
    {
        // Do not overwrite a handler subsequently installed above this one.
        const MsgHandlerInfo current = *host->GetMsgHandler();
        if (current.fun == HandleMessage && current.ctx == raw)
            host->SetMsgHandler(previous.fun, previous.ctx);
        Attachments().erase(host);
    }
    const BOOL handled = previous.fun ? previous.fun(message, result, previous.ctx) : FALSE;
    // SHostWnd installs an authored enterAnimation in WINDOWPOSCHANGING.
    // Starting in WINDOWPOSCHANGED allows its live animation (and the public
    // attribute, if stored) to take precedence. The actual configured member is
    // also checked; optional attribute storage is not an authorization oracle.
    AttachmentMap::iterator alive = Attachments().find(host);
    if (!handled && message->message == WM_WINDOWPOSCHANGED && alive != Attachments().end() && alive->second == attachment && attachment->showPending)
    {
        attachment->showPending = false;
        BeginEnter(*attachment);
    }
    return handled;
}

inline AttachmentPtr Attach(SHostWnd &host, int gapDp, BoolFlagPtr keyboard, bool reduced, const wchar_t *shadowSkin = L"fluent_popup_shadow")
{
    if (!host.IsWindow() || !host.GetRoot())
        return AttachmentPtr();
    AttachmentMap::iterator found = Attachments().find(&host);
    AttachmentPtr attachment;
    if (found != Attachments().end())
        attachment = found->second;
    else
    {
        if (!ApplyChrome(*host.GetRoot(), gapDp, shadowSkin))
            return AttachmentPtr();
        attachment = AttachmentPtr(new Attachment());
        attachment->host = &host;
        attachment->previous = *host.GetMsgHandler();
        attachment->chrome = true;
        Attachments()[&host] = attachment;
        host.SetMsgHandler(HandleMessage, attachment.get());
    }
    // Per-pixel alpha needs a layered host.  Use the framework entry point:
    // switching presentation at runtime also installs the dummy paint-pump
    // window and rebuilds the offscreen render target.  Writing the attribute
    // directly (plus ModifyStyleEx) produces a layered host that no invalidation
    // can reach, so the popup stays invisible while still swallowing clicks.
    host.SetTranslucent(true);
    attachment->keyboard = keyboard;
    attachment->reduced = reduced;
    attachment->active = true;
    if (reduced || *keyboard)
    {
        attachment->showPending = false;
        CancelOwned(*host.GetRoot(), attachment->owned);
    }
    return attachment;
}

inline bool PrepareMenuTree(IMenuEx &menu, BoolFlagPtr keyboard, bool reduced, UINT flags, SHostWnd *parent, SWND parentItem, const POINT *screenAnchor, int fixedWidthScale = 0, const wchar_t *shadowSkin = L"fluent_popup_shadow")
{
    // IMenuEx::GetHostWnd is also suitable for the demo2 MenuEx bridge.
    SHostWnd *host = static_cast<SHostWnd *>(menu.GetHostWnd());
    if (!host)
        return false;
    SWindow *root = host->GetRoot();
    int bodyWidth = -1;
    if (fixedWidthScale > 0)
    {
        root->SDispatchMessage(UM_SETSCALE, fixedWidthScale, 0);
        AttachmentMap::iterator existing = Attachments().find(host);
        if (existing == Attachments().end())
        {
            CSize desired;
            root->GetDesiredSize(&desired, -1, 10000);
            const CRect padding = root->GetStyle().GetPadding();
            bodyWidth = desired.cx - padding.left - padding.right;
        }
        else
            bodyWidth = existing->second->fixedBodyWidthPx;
    }
    const AttachmentPtr attachment = Attach(*host, 8, keyboard, reduced, shadowSkin);
    if (!attachment)
        return false;
    if (bodyWidth >= 0)
    {
        attachment->fixedBodyWidthPx = bodyWidth;
        const CRect padding = root->GetStyle().GetPadding();
        const SStringW width = SStringW().Format(L"%dpx", bodyWidth + padding.left + padding.right);
        root->SetAttribute(L"minWidth", width);
        root->SetAttribute(L"maxWidth", width);
        root->RequestRelayout();
    }
    attachment->parent = parent;
    attachment->parentItem = parentItem;
    attachment->trackFlags = flags;
    attachment->explicitMenuAnchor = screenAnchor != NULL;
    if (screenAnchor)
        attachment->menuAnchor = *screenAnchor;
    bool complete = true;
    SWindow *item = host->GetRoot()->GetWindow(GSW_FIRSTCHILD);
    for (int i = 0; i < menu.GetMenuItemCount(); ++i)
    {
        IMenuEx *child = menu.GetSubMenu(i);
        if (child)
            complete = PrepareMenuTree(*child, keyboard, reduced, TPM_LEFTALIGN, host, item ? item->GetSwnd() : 0, NULL, fixedWidthScale, shadowSkin) && complete;
        if (item)
            item = item->GetWindow(GSW_NEXTSIBLING);
    }
    return complete;
}
} // namespace PopupDetail

// Call after LoadMenu/LoadMenu2 and before TrackPopupMenu. Pass the SAME flags
// to both calls, notably wxDemo's TPM_BOTTOMALIGN. Repeated preparation does
// not add padding, widen a popup, or chain a second handler.
// Passing TrackPopupMenu's original (x,y) as screenAnchor also preserves the
// anchor if the framework has already flipped the rectangle against rcMonitor.
inline bool PrepareMenu(IMenuEx &menu, bool keyboard = false, bool reduced = false, UINT trackFlags = TPM_LEFTALIGN, const POINT *screenAnchor = NULL, const wchar_t *shadowSkin = L"fluent_popup_shadow")
{
    return PopupDetail::PrepareMenuTree(menu, MakeBoolFlag(keyboard), reduced, trackFlags, NULL, 0, screenAnchor, 0, shadowSkin);
}

inline bool PrepareMenu(SMenuEx &menu, bool keyboard = false, bool reduced = false, UINT trackFlags = TPM_LEFTALIGN, const POINT *screenAnchor = NULL, const wchar_t *shadowSkin = L"fluent_popup_shadow")
{
    return PrepareMenu(static_cast<IMenuEx &>(menu), keyboard, reduced, trackFlags, screenAnchor, shadowSkin);
}

// Only for the three fixed-width MenuEx resources (demo tray, demo2 and wx
// emoji). Measure the authored body at the SAME target scale as TrackPopupMenu;
// add gutter outside it. These call sites load a fresh menu on each opening.
// Do not reuse a prepared dp-authored menu at a different scale without
// reloading its authored constraints. Free-width menus use PrepareMenu.
inline bool PrepareFixedWidthMenu(IMenuEx &menu, int scale, bool keyboard = false, bool reduced = false, UINT trackFlags = TPM_LEFTALIGN, const POINT *screenAnchor = NULL, const wchar_t *shadowSkin = L"fluent_popup_shadow")
{
    return PopupDetail::PrepareMenuTree(menu, MakeBoolFlag(keyboard), reduced, trackFlags, NULL, 0, screenAnchor, scale > 0 ? scale : 100, shadowSkin);
}

// screenBuddyRect is already in screen coordinates. It need not be the owner's
// own window rect: MC's search control uses its buddy, not an empty search rect.
// Call after popup creation/insertion and BEFORE its framework size calculation.
inline bool PrepareDropdown(SDropDownWnd &popup, SWindow &owner, const RECT &screenBuddyRect, bool keyboard = false, bool reduced = false, const wchar_t *shadowSkin = L"fluent_popup_shadow")
{
    if (screenBuddyRect.right <= screenBuddyRect.left || screenBuddyRect.bottom <= screenBuddyRect.top)
        return false;
    const PopupDetail::AttachmentPtr attachment = PopupDetail::Attach(popup, 4, MakeBoolFlag(keyboard || PopupDetail::KeyboardActive()), reduced, shadowSkin);
    if (!attachment)
        return false;
    attachment->dropdown = true;
    attachment->owner = &owner;
    attachment->explicitBuddy = true;
    attachment->buddy = PopupDetail::RectOf(screenBuddyRect);
    return true;
}

inline BOOL OnComboDropdown(IEvtArgs *event, void *ownerHost, const wchar_t *shadowSkin = L"fluent_popup_shadow")
{
    EventCBDropdown *dropdown = sobj_cast<EventCBDropdown>(event);
    if (!dropdown || !dropdown->pDropDown)
        return FALSE;
    SDropDownWnd *popup = static_cast<SDropDownWnd *>(dropdown->pDropDown);
    SWindow *owner = popup->GetDropDownOwner();
    if (!owner)
        return FALSE;
    const MSG *message = ownerHost ? static_cast<SHostWnd *>(ownerHost)->GetCurrentMessage() : NULL;
    const bool keyboard = (message && PopupDetail::IsKeyMessage(message->message)) || PopupDetail::KeyboardActive();
    const PopupDetail::AttachmentPtr attachment = PopupDetail::Attach(*popup, 4, MakeBoolFlag(keyboard), false, shadowSkin);
    if (attachment)
    {
        attachment->dropdown = true;
        attachment->owner = owner;
        attachment->explicitBuddy = false;
        // SComboBase reads animateTime AFTER firing this event; only disable
        // its legacy whole-HWND slide, never alter dropHeight/listHeight/value.
        owner->SetAttribute(L"animateTime", L"0");
    }
    event->SetBubbleUp(TRUE);
    return FALSE; // Preserve list population, selection and application events.
}

} // namespace FluentDemo
} // namespace SNS