#include <souistd.h>
#include <core/SHostWnd.h>
#include <commgr2.h>
#include <xml/SXml.h>
#include <layout/SFrameLayout.h>
#include <gtest/gtest.h>

using namespace SOUI;

namespace {
// Pump posted messages (e.g. the WM_CLOSE the float host window posts when
// the dock bar is docked back) so asynchronous native window teardown
// completes before the test exits.
void PumpMessages()
{
    MSG msg;
    for (int i = 0; i < 100; ++i)
    {
        if (!::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            break;
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }
}
} // namespace

// GUI test: create a real host window with a frame layout holding a dock
// bar, then float the dock bar into its own host window and dock it back.
// Keep the suite named window so the default non-interactive fun_test
// filter (-window.*) continues to exclude it.
TEST(window, gui_dockfloat_float_and_dock_roundtrip)
{
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFactory;
    ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
    ASSERT_TRUE(renderFactory);
    SApplication app(renderFactory, NULL);

    static const char xml[] =
        "<SOUI title='DockFloat' translucent='0'>"
        "<root width='640' height='480' layout='frame' colorBkgnd='#ffffff'>"
        "<dockbar name='dock_left' dock='left' width='120' text='DockLeft'/>"
        "<window name='main' dock='mainview'/>"
        "</root></SOUI>";
    SXmlDoc doc;
    ASSERT_TRUE(doc.LoadBuffer(xml, sizeof(xml) - 1, 0, enc_utf8));
    SXmlNode xmlRoot = doc.root().first_child();
    ASSERT_TRUE(xmlRoot);

    SHostWnd host;
    HWND hwnd = host.CreateEx(NULL, WS_POPUP, 0, 100, 100, 640, 480, &xmlRoot);
    ASSERT_TRUE(hwnd);
    host.ShowWindow(SW_SHOW);

    SWindow *pRoot = host.GetRoot();
    ASSERT_TRUE(pRoot);

    SDockBar *pDockLeft = sobj_cast<SDockBar>(host.FindChildByName(L"dock_left"));
    ASSERT_TRUE(pDockLeft);
    SWindow *pDockParent = pDockLeft->GetParent();
    ASSERT_TRUE(pDockParent);

    SFrameLayout *pFrameLayout = sobj_cast<SFrameLayout>(pRoot->GetLayout());
    ASSERT_TRUE(pFrameLayout);

    // Initial state: docked inside the frame layout.
    EXPECT_FALSE(pDockLeft->IsFloating());
    EXPECT_FALSE(pFrameLayout->IsChildFloating(pDockLeft));

    // Round 1: float/dock through the frame layout convenience methods.
    ASSERT_TRUE(pFrameLayout->FloatChild(pDockLeft, CPoint(300, 200)));
    EXPECT_TRUE(pDockLeft->IsFloating());
    EXPECT_TRUE(pFrameLayout->IsChildFloating(pDockLeft));
    EXPECT_NE(pDockLeft->GetParent(), pDockParent);

    pFrameLayout->DockChild(pDockLeft);
    EXPECT_FALSE(pDockLeft->IsFloating());
    EXPECT_FALSE(pFrameLayout->IsChildFloating(pDockLeft));
    EXPECT_EQ(pDockLeft->GetParent(), pDockParent);

    // Round 2: float/dock through the dock bar API directly.
    ASSERT_TRUE(pDockLeft->Float(CPoint(300, 200)));
    EXPECT_TRUE(pDockLeft->IsFloating());
    EXPECT_NE(pDockLeft->GetParent(), pDockParent);

    pDockLeft->Dock();
    EXPECT_FALSE(pDockLeft->IsFloating());
    EXPECT_EQ(pDockLeft->GetParent(), pDockParent);

    // Round 3: while floating, the close button hides the dock bar (keeping
    // the float state); re-showing it stays floating.
    ASSERT_TRUE(pDockLeft->Float(CPoint(300, 200)));
    EXPECT_TRUE(pDockLeft->IsFloating());

    pDockLeft->SetVisible(FALSE, TRUE);
    EXPECT_FALSE(pDockLeft->IsVisible(FALSE));
    EXPECT_TRUE(pDockLeft->IsFloating());

    pDockLeft->SetVisible(TRUE, TRUE);
    EXPECT_TRUE(pDockLeft->IsVisible(FALSE));
    EXPECT_TRUE(pDockLeft->IsFloating());

    // Clean up: dock it back so the float host is destroyed via WM_CLOSE.
    pDockLeft->Dock();
    EXPECT_FALSE(pDockLeft->IsFloating());

    // Let the posted WM_CLOSE destroy the float host window objects.
    PumpMessages();

    host.DestroyWindow();
}
