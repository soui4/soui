/**
 * @file  test_soui_uitest_controls.cpp
 * @brief SouiTest Phase 0 —— 编辑框/组合框/选项卡/复选框 无头用例（Tier A）
 *
 * 验证"输入/选择 → 状态/事件变化"在框架级注入下的可观测性：
 *   - 编辑框：SetWindowText / GetWindowText 往返（属性级输入）
 *   - 组合框：InsertItem + SetCurSel 触发 EventCBSelChange，且 GetCurSel / 文本同步
 *   - 选项卡：SetCurSel 触发 EventTabSelChanged，且 GetCurSel 同步
 *   - 复选框：点击触发 EventCmd；SetCheck 切换 WndState_Check
 *   - 列表框：XML items 建项，文本/数据/查找可用；SetCurSel 触发 EventLBSelChanged
 *   - 进度条：XML min/max/value 生效，SetValue/SetRange 把值钳制在区间内
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

// 列表框：XML <items> 建项后，文本/数据/查找均可读；SetCurSel(bNotify=TRUE) 触发选择事件。
TEST_F(SouiTestFixture, listbox_items_and_selchange)
{
    SWindow *root = loadUi(
        L"<window layout='vbox' width='200' height='160'>"
        L"<listbox name='lb' width='180' height='120'>"
        L"<items>"
        L"<item text='alpha' data='1'/>"
        L"<item text='beta' data='2'/>"
        L"<item text='gamma' data='3'/>"
        L"</items>"
        L"</listbox>"
        L"</window>");
    SListBox *lb = SouiTest::findByName<SListBox>(root, L"lb");
    ASSERT_TRUE(lb != NULL);

    ASSERT_EQ(lb->GetCount(), 3);
    EXPECT_EQ(lb->GetText(1), _T("beta"));
    EXPECT_EQ(lb->GetItemData(1), (LPARAM)2);
    // FindString 从 iFindAfter 之后开始环形查找，点击前缀即可命中。
    EXPECT_EQ(lb->FindString(-1, _T("gam")), 2);

    SouiTest::SignalSpy spy(lb, EventLBSelChanged::EventID);
    EXPECT_TRUE(lb->SetCurSel(2, TRUE));
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(lb->GetCurSel(), 2);

    // 选中项没变化时不重复发事件。
    EXPECT_FALSE(lb->SetCurSel(2, TRUE));
    EXPECT_EQ(spy.count(), 1);
}

// 进度条：XML 的 min/max/value 生效；SetValue 越界时钳制，SetRange 收窄时同步收敛当前值。
TEST_F(SouiTestFixture, progress_value_clamped_to_range)
{
    SWindow *root = loadUi(
        L"<window layout='vbox' width='200' height='80'>"
        L"<progress name='pg' min='10' max='90' value='50' width='150' height='12'/>"
        L"</window>");
    SProgress *pg = SouiTest::findByName<SProgress>(root, L"pg");
    ASSERT_TRUE(pg != NULL);

    int nMin = 0, nMax = 0;
    pg->GetRange(&nMin, &nMax);
    EXPECT_EQ(nMin, 10);
    EXPECT_EQ(nMax, 90);
    EXPECT_EQ(pg->GetValue(), 50);

    pg->SetValue(200);
    EXPECT_EQ(pg->GetValue(), 90);
    pg->SetValue(5);
    EXPECT_EQ(pg->GetValue(), 10);

    // 新区间不含当前值时，值收敛到新区间端点。
    pg->SetRange(20, 40);
    pg->GetRange(&nMin, &nMax);
    EXPECT_EQ(nMin, 20);
    EXPECT_EQ(nMax, 40);
    EXPECT_EQ(pg->GetValue(), 20);
}
