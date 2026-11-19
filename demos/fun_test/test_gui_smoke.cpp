#include <souistd.h>
#include <string.h>
#include <core/SHostWnd.h>
#include <core/SModalViewSession.h>
#include <core/SWindowMgr.h>
#include <animation/SAnimation.h>
#include <commgr2.h>
#include <xml/SXml.h>
#include <gtest/gtest.h>

using namespace SOUI;

namespace {
struct ClickCounter {
	ClickCounter():clicks(0){}
    int clicks;

    BOOL onClick(IEvtArgs *)
    {
        ++clicks;
        return TRUE;
    }
};

struct HostClose {
    SHostWnd *host;

    ~HostClose()
    {
        if (host->m_hWnd)
            host->DestroyWindow();
    }
};

struct EventUnsubscribe {
    SWindow *window;
    DWORD eventId;
    const IEvtSlot *slot;

    ~EventUnsubscribe()
    {
        window->UnsubscribeEvent(eventId, slot);
    }
};

// Host whose layout comes from an in-memory XML string, so a reload can be
// observed without touching the resource package.  m_nBuild is stamped into the
// window text, m_bFail makes the next load fail.
class ReloadHost : public SHostWnd {
  public:
    ReloadHost()
        : m_nBuild(0)
        , m_bFail(FALSE)
    {
    }

    int m_nBuild;
    BOOL m_bFail;

  protected:
    virtual BOOL OnLoadLayoutFromResourceID(SXmlDoc &xmlDoc)
    {
        if (m_bFail)
            return FALSE;
        SStringA xml = SStringA().Format("<SOUI title='reload' translucent='0'>"
                                         "<root width='200' height='60' layout='vbox' colorBkgnd='#ffffff'>"
                                         "<text name='status' text='build %d' size='-2,-2'/>"
                                         "</root></SOUI>",
                                         m_nBuild);
        return xmlDoc.load_buffer(xml.c_str(), xml.GetLength(), xml_parse_default, enc_utf8);
    }
};

// Counts the EventReload notifications that reach the host container.
struct ReloadCounter {
    ReloadCounter()
        : count(0)
    {
    }
    int count;
};

BOOL OnHostEvent(IEvtArgs *evt, void *ctx)
{
    if (evt->GetID() == EventReload::EventID)
        ++((ReloadCounter *)ctx)->count;
    return FALSE; // keep default processing
}

// Counts EventExitModalView notifications and remembers the last exit code.
struct ModalExitCounter {
    ModalExitCounter()
        : count(0)
        , exitCode(-12345)
    {
    }
    int count;
    int exitCode;

    BOOL onExit(IEvtArgs *evt)
    {
        ++count;
        EventExitModalView *pEvt = sobj_cast<EventExitModalView>(evt);
        if (pEvt)
            exitCode = pEvt->exitCode;
        return TRUE;
    }
};

// Builds a standalone modal root from an in-memory layout, so a session can be
// exercised without a resource package.  Returns a window the caller owns
// (reference count 1).
SModalRoot *CreateTestModalRoot(const char *pszXml)
{
    SXmlDoc doc;
    if (!doc.LoadBuffer(pszXml, (int)strlen(pszXml), 0, enc_utf8))
        return NULL;
    SXmlNode node = doc.root().first_child();
    if (!node)
        return NULL;
    SModalRoot *pModal = (SModalRoot *)SApplication::getSingletonPtr()->CreateWindowByName(SModalRoot::GetClassName());
    if (!pModal)
        return NULL;
    if (!pModal->InitFromXml(&node))
    {
        pModal->Release();
        return NULL;
    }
    return pModal;
}

// Current reference count of a window.  AddRef() reports the value after the
// increment, so the measurement has to subtract one.
long RefCount(SWindow *pWnd)
{
    long nRefs = pWnd->AddRef();
    pWnd->Release();
    return nRefs - 1;
}

// Modal view whose exit animation is finished by the test instead of by the
// timeline, so the window lifetime between "end requested" and "exit finished"
// can be inspected.
class DeferredModalView : public SModalView {
    DEF_SOBJECT(SModalView, L"test_deferred_modalview")
  public:
    // Only the presence of an exit animation matters here: the frames are never
    // driven, the test ends the exit explicitly.
    void ArmExitAnimation()
    {
        m_aniExit.Attach(new SAnimation);
    }

    // Pretends the enter animation is still playing, which is what a close request
    // arriving right after the dialog opened sees.
    void ArmEnterAnimation()
    {
        m_aniEnter.Attach(new SAnimation);
        m_aniState = AniState_Enter;
    }

    // Simulates the last animation frame: the view reports the end of the exit
    // animation, which is where the modal root completes the session.
    void FinishExit()
    {
        IAnimation *pAni = GetAnimation();
        if (!pAni)
            return;
        m_isAnimating = TRUE; // what the first animation frame would have done
        OnAnimationStop(pAni);
        // `this` may be destroyed by the call above: touch nothing here.
    }

    // Marks the exit animation as running (what the first animation frame does), so
    // a forced close has to cancel a live animation instead of just dropping one
    // that never started.
    void MarkRunning()
    {
        m_isAnimating = TRUE;
    }
};
} // namespace

// This creates a real native host window and sends input through its window
// procedure. Keep the suite named window so the default non-interactive
// fun_test filter continues to exclude it.
TEST(window, gui_smoke_dispatches_button_click)
{
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFactory;
    ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
    ASSERT_TRUE(renderFactory);
    SApplication app(renderFactory, NULL);

    static const char xml[] =
        "<SOUI title='GUI smoke' translucent='0'>"
        "<root width='240' height='80' layout='vbox' colorBkgnd='#ffffff'>"
        "<text name='status' text='Ready' size='-2,30'/>"
        "<button name='action' text='Click' size='100,30'/>"
        "</root></SOUI>";
    SXmlDoc doc;
    ASSERT_TRUE(doc.LoadBuffer(xml, sizeof(xml) - 1, 0, enc_utf8));
    SXmlNode xmlRoot = doc.root().first_child();
    ASSERT_TRUE(xmlRoot);

    SHostWnd host;
    HostClose close;
	close.host=&host;
    HWND hwnd = host.CreateEx(NULL, WS_POPUP, 0, 100, 100, 240, 80, &xmlRoot);
    ASSERT_TRUE(hwnd);
    host.ShowWindow(SW_SHOW);
    ASSERT_TRUE(IsWindowVisible(hwnd));
    SWindow *status = host.FindChildByName("status");
    SWindow *action = host.FindChildByName("action");
    ASSERT_TRUE(status);
    ASSERT_TRUE(action);
    EXPECT_EQ(status->GetWindowText(), _T("Ready"));

    ClickCounter counter;
	MemberFunctionSlot<ClickCounter,IEvtArgs> slot = Subscriber(&ClickCounter::onClick, &counter);
    ASSERT_TRUE(action->SubscribeEvent(EventCmd::EventID, &slot));
    EventUnsubscribe unsubscribe ={action, EventCmd::EventID, &slot};
    CRect rect = action->GetWindowRect();
    ASSERT_GT(rect.Width(), 0);
    ASSERT_GT(rect.Height(), 0);
    LPARAM point = MAKELPARAM((rect.left + rect.right) / 2,
                             (rect.top + rect.bottom) / 2);
    SendMessage(hwnd, WM_MOUSEMOVE, 0, point);
    SendMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, point);
    SendMessage(hwnd, WM_LBUTTONUP, 0, point);
    EXPECT_EQ(counter.clicks, 1);
}

// ReloadLayout() rebuilds the window tree (root included) inside the same HWND
// and fires EventReload for the business layer.  When SOUI is built with
// SOUI_ENABLE_RELOAD one F5 press does the same (auto-repeat is ignored).
// A layout that cannot be loaded keeps the live tree.  Kept in the window.*
// suite like the smoke test above.
TEST(window, gui_smoke_reload_layout_from_resource)
{
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFactory;
    ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
    ASSERT_TRUE(renderFactory);
    SApplication app(renderFactory, NULL);

    ReloadHost host;
    HostClose close;
    close.host = &host;
    HWND hwnd = host.CreateEx(NULL, WS_POPUP, 0, 100, 100, 200, 60, NULL);
    ASSERT_TRUE(hwnd);
    host.ShowWindow(SW_SHOW);
    SWindow *status = host.FindChildByName("status");
    ASSERT_TRUE(status);
    EXPECT_EQ(status->GetWindowText(), _T("build 0"));

    ReloadCounter reloads;
    host.SetEventHandler(OnHostEvent, &reloads);

    // API path: the tree is rebuilt from the layout resource in place, the old
    // root is released and a new one takes its place.
    SWindow *rootBefore = host.GetRoot();
    host.m_nBuild = 1;
    EXPECT_TRUE(host.ReloadLayout());
    EXPECT_TRUE(host.GetRoot() != NULL);
    EXPECT_TRUE(host.GetRoot() != rootBefore);
    status = host.FindChildByName("status");
    ASSERT_TRUE(status);
    EXPECT_EQ(status->GetWindowText(), _T("build 1"));
    EXPECT_EQ(reloads.count, 1);

    // A layout that cannot be loaded must leave the live tree (and root) intact
    // and must not report a reload to the business layer.
    SWindow *rootAfter = host.GetRoot();
    host.m_bFail = TRUE;
    host.m_nBuild = 9;
    EXPECT_FALSE(host.ReloadLayout());
    host.m_bFail = FALSE;
    EXPECT_TRUE(host.GetRoot() == rootAfter);
    status = host.FindChildByName("status");
    ASSERT_TRUE(status);
    EXPECT_EQ(status->GetWindowText(), _T("build 1"));
    EXPECT_EQ(reloads.count, 1);

#if (SOUI_ENABLE_RELOAD)
    // Hot-key path: F5 rebuilds once, the auto-repeat flag (bit 30) is ignored.
    host.m_nBuild = 2;
    SendMessage(hwnd, WM_KEYDOWN, VK_F5, 0);
    status = host.FindChildByName("status");
    ASSERT_TRUE(status);
    EXPECT_EQ(status->GetWindowText(), _T("build 2"));
    EXPECT_EQ(reloads.count, 2);

    host.m_nBuild = 3;
    SendMessage(hwnd, WM_KEYDOWN, VK_F5, 0x40000000);
    status = host.FindChildByName("status");
    ASSERT_TRUE(status);
    EXPECT_EQ(status->GetWindowText(), _T("build 2"));
    EXPECT_EQ(reloads.count, 2);
#endif // SOUI_ENABLE_RELOAD
}

// The session owns its own references on the modal root, so a caller that follows
// the normal COM rule (Release() what CreateWindowByName() returned) must not end
// up with a window freed while the session is still tearing it down.
TEST(window, gui_smoke_modal_view_session_caller_release)
{
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFactory;
    ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
    ASSERT_TRUE(renderFactory);
    SApplication app(renderFactory, NULL);

    ReloadHost host;
    HostClose close;
    close.host = &host;
    HWND hwnd = host.CreateEx(NULL, WS_POPUP, 0, 100, 100, 200, 60, NULL);
    ASSERT_TRUE(hwnd);
    host.ShowWindow(SW_SHOW);

    SModalRoot *pModal = CreateTestModalRoot("<modalroot><modalview width='100' height='40'/></modalroot>");
    ASSERT_TRUE(pModal != NULL);
    SWND swnd = pModal->GetSwnd();

    ModalExitCounter exits;
    MemberFunctionSlot<ModalExitCounter, IEvtArgs> exitSlot = Subscriber(&ModalExitCounter::onExit, &exits);
    ASSERT_TRUE(pModal->SubscribeEvent(EventExitModalView::EventID, &exitSlot));

    ModalViewSessionID id = host.BeginModalViewSession(pModal);
    ASSERT_TRUE(id != 0);
    // The caller does not need its own reference any more: the session keeps the
    // window alive.  This is what used to crash on end.
    pModal->Release();
    EXPECT_EQ(RefCount(pModal), 2); // window tree + session

    // No exit animation in this layout, so the session finishes synchronously and
    // the modal window is gone once the call returns.
    EXPECT_TRUE(host.EndModalViewSession(id, IDOK));
    EXPECT_TRUE(SWindowMgr::GetWindow(swnd) == NULL);
    EXPECT_EQ(exits.count, 1);
    EXPECT_EQ(exits.exitCode, IDOK);
    EXPECT_EQ(host.GetLastModalViewSessionID(), 0u);
}

// The pattern used by the demos: create the modal root, begin the session, drop
// the creation reference right away, and do the same for every popup.  A leak or
// an extra release inside the session would surface on the second round.
TEST(window, gui_smoke_modal_view_session_repeat_after_caller_release)
{
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFactory;
    ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
    ASSERT_TRUE(renderFactory);
    SApplication app(renderFactory, NULL);

    ReloadHost host;
    HostClose close;
    close.host = &host;
    HWND hwnd = host.CreateEx(NULL, WS_POPUP, 0, 100, 100, 200, 60, NULL);
    ASSERT_TRUE(hwnd);
    host.ShowWindow(SW_SHOW);

    for (int round = 0; round < 3; ++round)
    {
        SModalRoot *pModal = CreateTestModalRoot("<modalroot><modalview width='100' height='40'/></modalroot>");
        ASSERT_TRUE(pModal != NULL);
        SWND swnd = pModal->GetSwnd();

        ModalViewSessionID id = host.BeginModalViewSession(pModal);
        ASSERT_TRUE(id != 0);
        pModal->Release(); // creation reference dropped right away, like the demos
        EXPECT_EQ(RefCount(pModal), 2); // window tree + session

        EXPECT_TRUE(host.EndModalViewSession(id, IDOK));
        EXPECT_TRUE(SWindowMgr::GetWindow(swnd) == NULL);
        EXPECT_EQ(host.GetLastModalViewSessionID(), 0u);
    }
}

// A caller that keeps its own reference only loses that reference: the session
// releases exactly what it owns, and the window dies with the caller's Release().
TEST(window, gui_smoke_modal_view_session_keeps_caller_reference)
{
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFactory;
    ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
    ASSERT_TRUE(renderFactory);
    SApplication app(renderFactory, NULL);

    ReloadHost host;
    HostClose close;
    close.host = &host;
    HWND hwnd = host.CreateEx(NULL, WS_POPUP, 0, 100, 100, 200, 60, NULL);
    ASSERT_TRUE(hwnd);
    host.ShowWindow(SW_SHOW);

    SModalRoot *pModal = CreateTestModalRoot("<modalroot><modalview width='100' height='40'/></modalroot>");
    ASSERT_TRUE(pModal != NULL);
    SWND swnd = pModal->GetSwnd();

    ModalViewSessionID id = host.BeginModalViewSession(pModal);
    ASSERT_TRUE(id != 0);
    // One for the caller, one for the window tree, one for the session.
    EXPECT_EQ(RefCount(pModal), 3);

    EXPECT_TRUE(host.EndModalViewSession(id, IDOK));
    // The caller's own reference is all that is left.
    EXPECT_TRUE(SWindowMgr::GetWindow(swnd) == pModal);
    EXPECT_EQ(RefCount(pModal), 1);

    pModal->Release();
    EXPECT_TRUE(SWindowMgr::GetWindow(swnd) == NULL);
}

// With an exit animation the session ends asynchronously: EndModalViewSession()
// returns before the window is destroyed, and the window has to stay alive (and
// stay in the tree) until the animation reports its end.
TEST(window, gui_smoke_modal_view_session_exit_animation)
{
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFactory;
    ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
    ASSERT_TRUE(renderFactory);
    SApplication app(renderFactory, NULL);
    ASSERT_TRUE(SApplication::getSingletonPtr()->RegisterWindowClass<DeferredModalView>());

    ReloadHost host;
    HostClose close;
    close.host = &host;
    HWND hwnd = host.CreateEx(NULL, WS_POPUP, 0, 100, 100, 200, 60, NULL);
    ASSERT_TRUE(hwnd);
    host.ShowWindow(SW_SHOW);

    SModalRoot *pModal =
        CreateTestModalRoot("<modalroot><test_deferred_modalview width='100' height='40'/></modalroot>");
    ASSERT_TRUE(pModal != NULL);
    SWND swnd = pModal->GetSwnd();
    DeferredModalView *pView = (DeferredModalView *)pModal->GetModalView();
    ASSERT_TRUE(pView != NULL);
    pView->ArmExitAnimation();

    ModalExitCounter exits;
    MemberFunctionSlot<ModalExitCounter, IEvtArgs> exitSlot = Subscriber(&ModalExitCounter::onExit, &exits);
    ASSERT_TRUE(pModal->SubscribeEvent(EventExitModalView::EventID, &exitSlot));

    ModalViewSessionID id = host.BeginModalViewSession(pModal);
    ASSERT_TRUE(id != 0);
    pModal->Release(); // the session alone has to keep the window alive from here

    EXPECT_TRUE(host.EndModalViewSession(id, IDCANCEL));
    // The session left the stack, but the window is still in the tree, waiting for
    // its exit animation; the exit event has not been reported yet.
    EXPECT_EQ(host.GetLastModalViewSessionID(), 0u);
    EXPECT_FALSE(host.EndModalViewSession(id, IDCANCEL)); // a second end is ignored
    EXPECT_TRUE(SWindowMgr::GetWindow(swnd) == pModal);
    EXPECT_TRUE(pModal->GetParent() != NULL);
    EXPECT_EQ(exits.count, 0);

    // Finish the exit: the host removes the window from the tree and destroys it.
    pView->FinishExit();
    EXPECT_TRUE(SWindowMgr::GetWindow(swnd) == NULL);
    EXPECT_EQ(exits.count, 1);
    EXPECT_EQ(exits.exitCode, IDCANCEL);

    // Nothing is left in flight, so the host can rebuild its layout again.
    host.m_nBuild = 7;
    EXPECT_TRUE(host.ReloadLayout());
    SWindow *status = host.FindChildByName("status");
    ASSERT_TRUE(status);
    EXPECT_EQ(status->GetWindowText(), _T("build 7"));
}

// A forced close (bNoAnimation = TRUE) skips the exit animation entirely: the exit
// event and the removal of the window both happen inside EndModalViewSession().
TEST(window, gui_smoke_modal_view_session_force_close)
{
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFactory;
    ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
    ASSERT_TRUE(renderFactory);
    SApplication app(renderFactory, NULL);

    ReloadHost host;
    HostClose close;
    close.host = &host;
    HWND hwnd = host.CreateEx(NULL, WS_POPUP, 0, 100, 100, 200, 60, NULL);
    ASSERT_TRUE(hwnd);
    host.ShowWindow(SW_SHOW);

    SModalRoot *pModal = CreateTestModalRoot("<modalroot><modalview width='100' height='40'/></modalroot>");
    ASSERT_TRUE(pModal != NULL);
    SWND swnd = pModal->GetSwnd();

    ModalExitCounter exits;
    MemberFunctionSlot<ModalExitCounter, IEvtArgs> exitSlot = Subscriber(&ModalExitCounter::onExit, &exits);
    ASSERT_TRUE(pModal->SubscribeEvent(EventExitModalView::EventID, &exitSlot));

    ModalViewSessionID id = host.BeginModalViewSession(pModal);
    ASSERT_TRUE(id != 0);
    pModal->Release();

    EXPECT_TRUE(host.EndModalViewSession(id, IDOK, TRUE));
    // Everything already happened inside the call above.
    EXPECT_TRUE(SWindowMgr::GetWindow(swnd) == NULL);
    EXPECT_EQ(exits.count, 1);
    EXPECT_EQ(exits.exitCode, IDOK);
    EXPECT_EQ(host.GetLastModalViewSessionID(), 0u);

    // Nothing is left to close.
    EXPECT_FALSE(host.EndModalViewSession(id, IDOK, TRUE));
}

// Forcing a session that is already closing drops its running exit animation and
// finishes it on the spot, instead of refusing the request as a plain second end
// call does.
TEST(window, gui_smoke_modal_view_session_force_finish_running_exit)
{
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFactory;
    ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
    ASSERT_TRUE(renderFactory);
    SApplication app(renderFactory, NULL);
    ASSERT_TRUE(SApplication::getSingletonPtr()->RegisterWindowClass<DeferredModalView>());

    ReloadHost host;
    HostClose close;
    close.host = &host;
    HWND hwnd = host.CreateEx(NULL, WS_POPUP, 0, 100, 100, 200, 60, NULL);
    ASSERT_TRUE(hwnd);
    host.ShowWindow(SW_SHOW);

    SModalRoot *pModal =
        CreateTestModalRoot("<modalroot><test_deferred_modalview width='100' height='40'/></modalroot>");
    ASSERT_TRUE(pModal != NULL);
    SWND swnd = pModal->GetSwnd();
    DeferredModalView *pView = (DeferredModalView *)pModal->GetModalView();
    ASSERT_TRUE(pView != NULL);
    pView->ArmExitAnimation();

    ModalExitCounter exits;
    MemberFunctionSlot<ModalExitCounter, IEvtArgs> exitSlot = Subscriber(&ModalExitCounter::onExit, &exits);
    ASSERT_TRUE(pModal->SubscribeEvent(EventExitModalView::EventID, &exitSlot));

    ModalViewSessionID id = host.BeginModalViewSession(pModal);
    ASSERT_TRUE(id != 0);
    pModal->Release();

    // Start an ordinary (animated) close: the window has to stay alive.
    EXPECT_TRUE(host.EndModalViewSession(id, IDCANCEL));
    EXPECT_TRUE(SWindowMgr::GetWindow(swnd) == pModal);
    EXPECT_EQ(exits.count, 0);
    pView->MarkRunning(); // the exit animation is mid-flight by now

    // Force it: the animation is dropped, the session finishes right now, and the
    // exit code of the forced call is the one that gets reported.
    EXPECT_TRUE(host.EndModalViewSession(id, IDOK, TRUE));
    EXPECT_TRUE(SWindowMgr::GetWindow(swnd) == NULL);
    EXPECT_EQ(exits.count, 1);
    EXPECT_EQ(exits.exitCode, IDOK);
    EXPECT_EQ(host.GetLastModalViewSessionID(), 0u);

    // The host is idle again, so a reload is allowed once more.
    host.m_nBuild = 6;
    EXPECT_TRUE(host.ReloadLayout());
    SWindow *status = host.FindChildByName("status");
    ASSERT_TRUE(status);
    EXPECT_EQ(status->GetWindowText(), _T("build 6"));
}

// A reload rebuilds the tree the modal sessions live in, so it closes them all
// first, newest first and without an exit animation, while they still can report
// their end. A session that is already closing is finished on the spot.
TEST(window, gui_smoke_reload_closes_modal_views)
{
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFactory;
    ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
    ASSERT_TRUE(renderFactory);
    SApplication app(renderFactory, NULL);
    ASSERT_TRUE(SApplication::getSingletonPtr()->RegisterWindowClass<DeferredModalView>());

    ReloadHost host;
    HostClose close;
    close.host = &host;
    HWND hwnd = host.CreateEx(NULL, WS_POPUP, 0, 100, 100, 200, 60, NULL);
    ASSERT_TRUE(hwnd);
    host.ShowWindow(SW_SHOW);

    // Two ordinary sessions plus a third one that is already closing when the
    // reload arrives (its exit animation is still in flight).
    static const char *layouts[3] = {
        "<modalroot><modalview width='100' height='40'/></modalroot>",
        "<modalroot><modalview width='100' height='40'/></modalroot>",
        "<modalroot><test_deferred_modalview width='100' height='40'/></modalroot>",
    };
    SWND swnds[3] = {0};
    ModalExitCounter exits[3];
    MemberFunctionSlot<ModalExitCounter, IEvtArgs> slot0 = Subscriber(&ModalExitCounter::onExit, &exits[0]);
    MemberFunctionSlot<ModalExitCounter, IEvtArgs> slot1 = Subscriber(&ModalExitCounter::onExit, &exits[1]);
    MemberFunctionSlot<ModalExitCounter, IEvtArgs> slot2 = Subscriber(&ModalExitCounter::onExit, &exits[2]);
    typedef MemberFunctionSlot<ModalExitCounter, IEvtArgs> ExitSlot;
    ExitSlot *slots[3] = {&slot0, &slot1, &slot2};

    ModalViewSessionID lastID = 0;
    for (int i = 0; i < 3; ++i)
    {
        SModalRoot *pModal = CreateTestModalRoot(layouts[i]);
        ASSERT_TRUE(pModal != NULL);
        swnds[i] = pModal->GetSwnd();
        ASSERT_TRUE(pModal->SubscribeEvent(EventExitModalView::EventID, slots[i]));
        lastID = host.BeginModalViewSession(pModal);
        ASSERT_TRUE(lastID != 0);
        if (i == 2)
        {
            DeferredModalView *pView = (DeferredModalView *)pModal->GetModalView();
            ASSERT_TRUE(pView != NULL);
            pView->ArmExitAnimation();
        }
        pModal->Release();
    }

    // Start closing the top-most one: it leaves the stack and waits for its exit
    // animation, so the host is left with a session in flight.
    ASSERT_TRUE(host.EndModalViewSession(0, IDCANCEL));
    EXPECT_TRUE(SWindowMgr::GetWindow(swnds[2]) != NULL);
    EXPECT_EQ(exits[2].count, 0);

    // The reload no longer refuses: every session is closed synchronously (the one
    // in flight is finished on the spot), newest first.
    host.m_nBuild = 5;
    EXPECT_TRUE(host.ReloadLayout());
    for (int i = 0; i < 3; ++i)
    {
        EXPECT_TRUE(SWindowMgr::GetWindow(swnds[i]) == NULL);
        EXPECT_EQ(exits[i].count, 1);
        EXPECT_EQ(exits[i].exitCode, IDCANCEL);
    }
    EXPECT_EQ(host.GetLastModalViewSessionID(), 0u);
    SWindow *status = host.FindChildByName("status");
    ASSERT_TRUE(status);
    EXPECT_EQ(status->GetWindowText(), _T("build 5"));
}

// A close request that arrives while the enter animation is still playing finishes
// that animation instead of animating an exit on top of it.
TEST(window, gui_smoke_modal_view_closes_while_entering)
{
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFactory;
    ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
    ASSERT_TRUE(renderFactory);
    SApplication app(renderFactory, NULL);
    ASSERT_TRUE(SApplication::getSingletonPtr()->RegisterWindowClass<DeferredModalView>());

    ReloadHost host;
    HostClose close;
    close.host = &host;
    HWND hwnd = host.CreateEx(NULL, WS_POPUP, 0, 100, 100, 200, 60, NULL);
    ASSERT_TRUE(hwnd);
    host.ShowWindow(SW_SHOW);

    SModalRoot *pModal =
        CreateTestModalRoot("<modalroot><test_deferred_modalview width='100' height='40'/></modalroot>");
    ASSERT_TRUE(pModal != NULL);
    SWND swnd = pModal->GetSwnd();
    DeferredModalView *pView = (DeferredModalView *)pModal->GetModalView();
    ASSERT_TRUE(pView != NULL);

    ModalExitCounter exits;
    MemberFunctionSlot<ModalExitCounter, IEvtArgs> exitSlot = Subscriber(&ModalExitCounter::onExit, &exits);
    ASSERT_TRUE(pModal->SubscribeEvent(EventExitModalView::EventID, &exitSlot));

    ModalViewSessionID id = host.BeginModalViewSession(pModal);
    ASSERT_TRUE(id != 0);
    pModal->Release();

    // The dialog is closed while its enter animation is still playing. No exit
    // animation is armed, so finishing the enter animation is enough and the
    // session completes inside the call.
    pView->ArmEnterAnimation();
    EXPECT_TRUE(host.EndModalViewSession(id, IDOK));
    EXPECT_TRUE(SWindowMgr::GetWindow(swnd) == NULL);
    EXPECT_EQ(exits.count, 1);
    EXPECT_EQ(exits.exitCode, IDOK);
    EXPECT_EQ(host.GetLastModalViewSessionID(), 0u);
}

// Closing a dialog that is still opening has to switch the view to the exit
// animation; a repeated close must then be ignored (the state really is "exiting"),
// instead of restarting the exit animation.
TEST(window, gui_smoke_modal_view_exit_after_entering)
{
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFactory;
    ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
    ASSERT_TRUE(renderFactory);
    SApplication app(renderFactory, NULL);
    ASSERT_TRUE(SApplication::getSingletonPtr()->RegisterWindowClass<DeferredModalView>());

    ReloadHost host;
    HostClose close;
    close.host = &host;
    HWND hwnd = host.CreateEx(NULL, WS_POPUP, 0, 100, 100, 200, 60, NULL);
    ASSERT_TRUE(hwnd);
    host.ShowWindow(SW_SHOW);

    SModalRoot *pModal =
        CreateTestModalRoot("<modalroot><test_deferred_modalview width='100' height='40'/></modalroot>");
    ASSERT_TRUE(pModal != NULL);
    SWND swnd = pModal->GetSwnd();
    DeferredModalView *pView = (DeferredModalView *)pModal->GetModalView();
    ASSERT_TRUE(pView != NULL);

    ModalExitCounter exits;
    MemberFunctionSlot<ModalExitCounter, IEvtArgs> exitSlot = Subscriber(&ModalExitCounter::onExit, &exits);
    ASSERT_TRUE(pModal->SubscribeEvent(EventExitModalView::EventID, &exitSlot));

    ModalViewSessionID id = host.BeginModalViewSession(pModal);
    ASSERT_TRUE(id != 0);
    pModal->Release();

    pView->ArmEnterAnimation();
    pView->ArmExitAnimation();
    EXPECT_TRUE(host.EndModalViewSession(id, IDCANCEL));
    // The enter animation was dropped and the exit one is now in flight.
    EXPECT_TRUE(SWindowMgr::GetWindow(swnd) == pModal);
    EXPECT_EQ(exits.count, 0);
    EXPECT_FALSE(host.EndModalViewSession(id, IDCANCEL)); // not restarting the exit

    pView->FinishExit();
    EXPECT_TRUE(SWindowMgr::GetWindow(swnd) == NULL);
    EXPECT_EQ(exits.count, 1);
    EXPECT_EQ(exits.exitCode, IDCANCEL);
}
