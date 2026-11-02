/**
 * @file  test_gui_uitest_comparison.cpp
 * @brief SouiTest Phase 0 —— Tier A（框架级/无头）与 Tier B（原生窗口）等价性对照
 *
 * 设计文档 R1 风险：SSendMessage(WM_LBUTTONDOWN/UP) 是否等价于原生 SendMessage 所触发的
 * 完整处理链。本用例在同一按钮 XML 上分别用两条 Tier 驱动并断言都触发一次 EventCmd，
 * 从而把 SSendMessage 的等价子集锁定下来。
 *
 * 仅当 SOUI_ENABLE_GUI_SMOKE=ON（有显示环境）时编译并注册；归入 soui-gui（非 soui-headless），
 * 因此不会污染无头门禁。
 */
#include "test_soui_uitest_common.h"

#ifdef SOUI_ENABLE_GUI_SMOKE
#include <core/SHostWnd.h>
#include <commgr2.h>
#include <xml/SXml.h>

using namespace SOUI;

// 注意必须用数组而非指针：下面按 sizeof(kButtonXml) - 1 取长度，
// 若声明成 const char *，sizeof 得到的是指针大小（8），只会解析 7 个字节。
static const char kButtonXml[] =
    "<SOUI title='cmp' translucent='0'>"
    "<root width='240' height='80' layout='vbox' colorBkgnd='#ffffff'>"
    "<text name='status' text='Ready' size='-2,30'/>"
    "<button name='action' text='Click' size='100,30'/>"
    "</root></SOUI>";

// 同一"按钮点击"路径，Tier A 与 Tier B 都应恰好触发一次 EventCmd。
TEST(window_uitest, button_click_tierA_matches_tierB)
{
    // ---- Tier A：框架级 / 无头（复用 SouiTestFixture 基座）----
    {
        funtest::HeadlessApp app;
        funtest::TestContainer container;
        SWindow root;
        ASSERT_TRUE(funtest::CreateRootFromXml(root, &container,
            L"<window layout='vbox' width='240' height='80'>"
            L"<text name='status' text='Ready' height='30'/>"
            L"<button name='action' text='Click' width='100' height='30'/>"
            L"</window>"));
        root.SetVisible(TRUE);
        root.OnRelayout(CRect(0, 0, 240, 80));

        SButton *action = root.FindChildByName2<SButton>(L"action");
        ASSERT_TRUE(action != NULL);
        // Spy 的生命周期必须短于宿主控件：先让 spyA 析构（自动退订），
        // 再销毁控件树。反过来写虽然 SouiTest 的 Spy 也能安全处理（靠 SWND 判活），
        // 但那是兜底，测试本身应示范正确用法。
        {
            SouiTest::SignalSpy spyA(action, EventCmd::EventID);
            SouiTest::mouseClick(action);
            EXPECT_EQ(spyA.count(), 1);
        }
        root.DestroyAllChildren();
    }

    // ---- Tier B：原生窗口 / 需显示（对齐 test_gui_smoke.cpp）----
    {
        SComMgr2 comMgr;
        SAutoRefPtr<IRenderFactory> renderFactory;
        ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
        SApplication app(renderFactory, NULL);

        SXmlDoc doc;
        ASSERT_TRUE(doc.LoadBuffer(kButtonXml, sizeof(kButtonXml) - 1, 0, enc_utf8));
        SXmlNode xmlRoot = doc.root().first_child();
        ASSERT_TRUE(xmlRoot);

        SHostWnd host;
        HWND hwnd = host.CreateEx(NULL, WS_POPUP, 0, 100, 100, 240, 80, &xmlRoot);
        ASSERT_TRUE(hwnd);
        host.ShowWindow(SW_SHOW);
        ASSERT_TRUE(IsWindowVisible(hwnd));

        SWindow *action = host.FindChildByName(L"action");
        ASSERT_TRUE(action != NULL);

        CPoint center = SouiTest::centerOf(action);
        {
            SouiTest::SignalSpy spyB(action, EventCmd::EventID);
            SouiTest::nativeMouseClick(hwnd, center);
            EXPECT_EQ(spyB.count(), 1);
        }
        // 同上：spyB 已析构，再销毁原生宿主窗口。
        if (host.m_hWnd)
            host.DestroyWindow();
    }
}
#endif // SOUI_ENABLE_GUI_SMOKE
