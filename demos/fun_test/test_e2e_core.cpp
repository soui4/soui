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
}
