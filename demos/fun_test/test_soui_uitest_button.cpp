/**
 * @file  test_soui_uitest_button.cpp
 * @brief SouiTest Phase 0 —— 按钮交互无头用例（Tier A / SSendMessage）
 *
 * 覆盖设计文档 Phase 0 的"按钮点击→EventCmd"路径，并在无显示环境下运行，
 * 归入 soui-headless 门禁。所有交互均通过 SouiTest::mouseClick / keyClick
 * 注入到 SOUI 框架级消息管线（SWindow::SSendMessage），不依赖原生窗口。
 */
#include "test_soui_uitest_common.h"

using namespace SOUI;

// 最小布局：状态文本 + 一个按钮。
static const wchar_t *kButtonUi =
    L"<window layout='vbox' width='200' height='80'>"
    L"<text name='status' text='Ready' height='30'/>"
    L"<button name='act' text='Go' width='80' height='30'/>"
    L"</window>";

// 框架级点击（SSendMessage WM_LBUTTONDOWN/UP）应触发按钮的 EventCmd 一次。
TEST_F(SouiTestFixture, button_click_fires_command)
{
    SWindow *root = loadUi(kButtonUi);
    SButton *act = SouiTest::findByName<SButton>(root, L"act");
    ASSERT_TRUE(act != NULL);

    SouiTest::SignalSpy spy(act, EventCmd::EventID);
    SouiTest::mouseClick(act);

    EXPECT_EQ(spy.count(), 1);
    SOUITEST_EXPECT_VISIBLE(act);
}

// 键盘激活：VK_SPACE 经 WM_KEYDOWN/UP 也应触发 EventCmd（与鼠标等价的输入路径）。
TEST_F(SouiTestFixture, button_keyboard_activates)
{
    SWindow *root = loadUi(kButtonUi);
    SButton *act = SouiTest::findByName<SButton>(root, L"act");
    ASSERT_TRUE(act != NULL);

    SouiTest::SignalSpy spy(act, EventCmd::EventID);
    SouiTest::keyClick(act, VK_SPACE);

    EXPECT_EQ(spy.count(), 1);
}

// 边界：UP 落点落在按钮矩形外时，SWindow::OnLButtonUp 的 PtInRect 判定不应触发命令。
// 此用例锁定 mouseClick 使用"宿主坐标系中心"这一坐标口径的正确性。
TEST_F(SouiTestFixture, button_click_outside_rect_does_not_fire)
{
    SWindow *root = loadUi(kButtonUi);
    SButton *act = SouiTest::findByName<SButton>(root, L"act");
    ASSERT_TRUE(act != NULL);

    CRect rc = SouiTest::rectOf(act);
    CPoint center = SouiTest::centerOf(act);
    CPoint outside(rc.left - 10, rc.top - 10); // 矩形左上之外

    SouiTest::SignalSpy spy(act, EventCmd::EventID);
    SouiTest::mousePress(act, center);   // 置 PushDown + SetCapture
    SouiTest::mouseRelease(act, outside); // 释放，但落点在外

    EXPECT_EQ(spy.count(), 0);
}
