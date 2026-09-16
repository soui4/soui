#include <souistd.h>
#include <core/SHostWnd.h>
#include <commgr2.h>
#include <xml/SXml.h>
#include <gtest/gtest.h>

using namespace SOUI;

namespace {
struct ClickCounter {
    int clicks = 0;

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
    HostClose close{&host};
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
    auto slot = Subscriber(&ClickCounter::onClick, &counter);
    ASSERT_TRUE(action->SubscribeEvent(EventCmd::EventID, &slot));
    EventUnsubscribe unsubscribe{action, EventCmd::EventID, &slot};
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
