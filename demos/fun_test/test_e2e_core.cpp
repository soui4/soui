#include <souistd.h>
#include <core/SWnd.h>
#include <gtest/gtest.h>
#include "common.h"

using namespace SOUI;

// A headless core path: indexed UI resource -> application resource manager ->
// XML window factory -> named controls -> observable state change.
TEST(soui_core_e2e, resource_to_window_state)
{
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFactory;
    ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
    ASSERT_TRUE(renderFactory);

    SApplication app(renderFactory, NULL);
    SouiFactory factory;
    SAutoRefPtr<IResProvider> provider(factory.CreateResProvider(RES_FILE), FALSE);
    ASSERT_TRUE(provider);
    SStringT resourceDir = getSourceDir() + _T("/e2e-res");
    ASSERT_TRUE(provider->Init((LPARAM)resourceDir.c_str(), 0));
    app.AddResProvider(provider, NULL);

    SWindow root;
    ASSERT_TRUE(root.InitFromResId(_T("layout:XML_CORE_E2E")));
    ASSERT_EQ(root.GetChildrenCount(), 2u);

    SWindow *status = root.FindChildByName("status");
    SWindow *action = root.FindChildByName("action");
    ASSERT_TRUE(status);
    ASSERT_TRUE(action);
    EXPECT_EQ(status->GetWindowText(), _T("Ready"));
    EXPECT_EQ(action->GetWindowText(), _T("Continue"));

    status->SetWindowText(_T("Completed"));
    EXPECT_EQ(status->GetWindowText(), _T("Completed"));
    root.DestroyAllChildren();
}

// Inline templates: a <template> child node of a window registers its body on that
// window and is marked with userdata so it creates no window. A "t:name" child
// instantiates the template, replacing {{attr}} placeholders with the reference
// node attributes. Window-level templates take precedence over the global pool.
TEST(soui_core_e2e, inline_template_instantiation)
{
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFactory;
    ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
    ASSERT_TRUE(renderFactory);

    SApplication app(renderFactory, NULL);
    SouiFactory factory;
    SAutoRefPtr<IResProvider> provider(factory.CreateResProvider(RES_FILE), FALSE);
    ASSERT_TRUE(provider);
    SStringT resourceDir = getSourceDir() + _T("/e2e-res");
    ASSERT_TRUE(provider->Init((LPARAM)resourceDir.c_str(), 0));
    app.AddResProvider(provider, NULL);

    SWindow root;
    ASSERT_TRUE(root.InitFromResId(_T("layout:XML_CORE_TEMPLATE_E2E")));
    // The <template> node must not create a window: the root only holds the panel.
    ASSERT_EQ(root.GetChildrenCount(), 1u);

    SWindow *panel = root.FindChildByName("panel");
    ASSERT_TRUE(panel);
    // The two "t:" references instantiate the template with different attributes.
    ASSERT_EQ(panel->GetChildrenCount(), 2u);

    SWindow *first = panel->FindChildByName("first");
    SWindow *second = panel->FindChildByName("second");
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    EXPECT_EQ(first->GetWindowText(), _T("One"));
    EXPECT_EQ(second->GetWindowText(), _T("Two"));

    root.DestroyAllChildren();
}

// Regression: SFrameLayout used to collect children by WndState_Invisible
// (IsVisible(TRUE)). When a hidden branch is shown, UpdateLayout runs before the
// ParentShow broadcast clears the state bits, so every dock panel was treated as
// invisible and kept a zero rect until the window was resized. The layout must
// filter by the m_bVisible attribute (IsVisible(FALSE)) instead, matching
// GetNextLayoutChild used by the other layouts.
TEST(soui_core_e2e, frame_layout_lays_out_after_show)
{
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFactory;
    ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
    ASSERT_TRUE(renderFactory);

    SApplication app(renderFactory, NULL);
    SouiFactory factory;
    SAutoRefPtr<IResProvider> provider(factory.CreateResProvider(RES_FILE), FALSE);
    ASSERT_TRUE(provider);
    SStringT resourceDir = getSourceDir() + _T("/e2e-res");
    ASSERT_TRUE(provider->Init((LPARAM)resourceDir.c_str(), 0));
    app.AddResProvider(provider, NULL);

    SWindow root;
    ASSERT_TRUE(root.InitFromResId(_T("layout:XML_FRAME_E2E")));
    SWindow *frameHost = root.FindChildByName("frame_host");
    SWindow *pTop = root.FindChildByName("p_top");
    SWindow *pLeft = root.FindChildByName("p_left");
    SWindow *pMain = root.FindChildByName("p_main");
    ASSERT_TRUE(frameHost);
    ASSERT_TRUE(pTop);
    ASSERT_TRUE(pLeft);
    ASSERT_TRUE(pMain);

    // Simulate a tab page being hidden: SetVisible(FALSE) broadcasts ParentShow
    // and marks the whole subtree with WndState_Invisible.
    root.SetVisible(FALSE);
    // While hidden, the page itself still gets a rect (as STabCtrl does), but the
    // subtree is not laid out.
    root.OnRelayout(CRect(0, 0, 400, 300));

    // Show the branch again: UpdateLayout runs while the state bits are still set,
    // then the broadcast clears them. Panels must end up with correct rects.
    root.SetVisible(TRUE);

    CRect rcTop, rcLeft, rcMain;
    pTop->GetWindowRect(&rcTop);
    pLeft->GetWindowRect(&rcLeft);
    pMain->GetWindowRect(&rcMain);
    EXPECT_EQ(rcTop, CRect(0, 0, 400, 30));
    EXPECT_EQ(rcLeft, CRect(0, 30, 70, 300));
    EXPECT_EQ(rcMain, CRect(70, 30, 400, 300));

    root.DestroyAllChildren();
}
