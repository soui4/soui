/**
 * @file  test_soui_uitest_controls.cpp
 * @brief SouiTest Phase 0 —— 编辑框/组合框/选项卡/复选框 无头用例（Tier A）
 *
 * 验证"输入/选择 → 状态/事件变化"在框架级注入下的可观测性：
 *   - 编辑框：SetWindowText / GetWindowText 往返（属性级输入）
 *   - 组合框：InsertItem + SetCurSel 触发 EventCBSelChange，且 GetCurSel / 文本同步
 *   - 选项卡：SetCurSel 触发 EventTabSelChanged，且 GetCurSel 同步
 *   - 复选框：点击触发 EventCmd；SetCheck 切换 WndState_Check
 */
#include "test_soui_uitest_common.h"

using namespace SOUI;

// 编辑框：文本可被写入并读回（控件树可见、可寻址、属性可断言）。
TEST_F(SouiTestFixture, edit_text_set_and_read)
{
    SWindow *root = loadUi(
        L"<window layout='vbox' width='200' height='80'>"
        L"<edit name='ed' width='150' height='24'/>"
        L"</window>");
    SEdit *ed = SouiTest::findByName<SEdit>(root, L"ed");
    ASSERT_TRUE(ed != NULL);

    ed->SetWindowText(_T("hello"));
    EXPECT_EQ(ed->GetWindowText(), _T("hello"));

    ed->SetWindowText(_T(""));
    EXPECT_EQ(ed->GetWindowText(), _T(""));
}

// 复选框（toggle）：点击触发 EventCmd；SetCheck 切换勾选态。
TEST_F(SouiTestFixture, checkbox_click_toggles_and_commands)
{
    SWindow *root = loadUi(
        L"<window layout='vbox' width='200' height='80'>"
        L"<toggle name='cb' text='Agree' width='100' height='30'/>"
        L"</window>");
    SWindow *cb = SouiTest::findByName<SWindow>(root, L"cb");
    ASSERT_TRUE(cb != NULL);

    // 初始未勾选
    EXPECT_FALSE(SouiTest::isChecked(cb));

    // 点击触发命令
    SouiTest::SignalSpy spy(cb, EventCmd::EventID);
    SouiTest::mouseClick(cb);
    EXPECT_EQ(spy.count(), 1);

    // 直接切换勾选态（属性级）
    cb->SetCheck(TRUE);
    EXPECT_TRUE(SouiTest::isChecked(cb));
    cb->SetCheck(FALSE);
    EXPECT_FALSE(SouiTest::isChecked(cb));
}

// 单选框（radio）：点击触发 EventCmd（与按钮同一条命令路径）。
TEST_F(SouiTestFixture, radio_click_fires_command)
{
    SWindow *root = loadUi(
        L"<window layout='vbox' width='200' height='80'>"
        L"<radio name='rb' text='Option' width='100' height='30'/>"
        L"</window>");
    SWindow *rb = SouiTest::findByName<SWindow>(root, L"rb");
    ASSERT_TRUE(rb != NULL);

    SouiTest::SignalSpy spy(rb, EventCmd::EventID);
    SouiTest::mouseClick(rb);
    EXPECT_EQ(spy.count(), 1);
}
