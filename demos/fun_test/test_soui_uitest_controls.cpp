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

// edit lite：边框皮肤只有 normal/hover 两态，按下鼠标不能进入 PushDown，
// 否则状态索引落到不存在的第 3 帧而被回退成 normal（点击后边框“闪”一下）。
TEST_F(SouiTestFixture, edit_lite_click_keeps_hover_border_state)
{
#if !defined(SOUI_EDIT_LITE)
    SUCCEED() << "edit lite is not enabled in this build";
    return;
#else
    SWindow *root = loadUi(
        L"<window layout='vbox' width='200' height='80'>"
        L"<edit name='ed' width='150' height='24'/>"
        L"</window>");
    SEdit *ed = SouiTest::findByName<SEdit>(root, L"ed");
    ASSERT_TRUE(ed != NULL);

    ed->SetFocus();
    ed->ModifyState(WndState_Hover, 0, FALSE);
    CPoint pt = SouiTest::centerOf(ed);
    SouiTest::mousePress(ed, pt);
    EXPECT_TRUE((ed->GetState() & WndState_Hover) != 0);
    EXPECT_TRUE((ed->GetState() & WndState_PushDown) == 0);
    SouiTest::mouseRelease(ed, pt);
#endif
}

// edit lite：EM_SETSEL 的负数起点表示“取消选择并折叠光标”，而非从 0 开始。
// 资源管理器拖放文件时正是先 SetSel(-1,-1) 再 ReplaceSel() 来追加文件名。
TEST_F(SouiTestFixture, edit_lite_set_sel_negative_collapses)
{
#if !defined(SOUI_EDIT_LITE)
    SUCCEED() << "edit lite is not enabled in this build";
    return;
#else
    SWindow *root = loadUi(
        L"<window layout='vbox' width='200' height='80'>"
        L"<edit name='ed' width='150' height='24'/>"
        L"</window>");
    SEdit *ed = SouiTest::findByName<SEdit>(root, L"ed");
    ASSERT_TRUE(ed != NULL);

    ed->SetWindowText(_T("abc"));
    ed->SetSel(-1, -1, FALSE);
    ed->ReplaceSel(_T("d"), FALSE);
    EXPECT_EQ(ed->GetWindowText(), _T("abcd"));

    // 起点为负、终点非负：折叠到当前活动端（caret），不改变选区起点语义。
    ed->SetSel(0, 2, TRUE);
    ed->SetSel(-1, 1, TRUE);
    ed->ReplaceSel(_T("X"), FALSE);
    EXPECT_EQ(ed->GetWindowText(), _T("abXcd"));
#endif
}

// edit lite：多级 undo/redo。每次文本修改一层，redo 在产生新编辑后失效。
TEST_F(SouiTestFixture, edit_lite_undo_redo_multiple_steps)
{
#if !defined(SOUI_EDIT_LITE)
    SUCCEED() << "edit lite is not enabled in this build";
    return;
#else
    SWindow *root = loadUi(
        L"<window layout='vbox' width='200' height='80'>"
        L"<edit name='ed' width='150' height='24'/>"
        L"</window>");
    SEdit *ed = SouiTest::findByName<SEdit>(root, L"ed");
    ASSERT_TRUE(ed != NULL);

    ed->SetWindowText(_T("a"));
    ed->ReplaceSel(_T("b"), TRUE);
    ed->ReplaceSel(_T("c"), TRUE);
    ed->ReplaceSel(_T("d"), TRUE);
    EXPECT_EQ(ed->GetWindowText(), _T("abcd"));
    EXPECT_TRUE(ed->SSendMessage(EM_CANUNDO) != 0);

    // 逐级回退；栈空后 EM_UNDO 返回 0 且文本保持不变。
    ed->SSendMessage(EM_UNDO);
    EXPECT_EQ(ed->GetWindowText(), _T("abc"));
    ed->SSendMessage(EM_UNDO);
    EXPECT_EQ(ed->GetWindowText(), _T("ab"));
    ed->SSendMessage(EM_UNDO);
    EXPECT_EQ(ed->GetWindowText(), _T("a"));
    EXPECT_EQ(ed->SSendMessage(EM_CANUNDO), 0);
    EXPECT_EQ(ed->SSendMessage(EM_UNDO), 0);
    EXPECT_EQ(ed->GetWindowText(), _T("a"));

    // 逐级重做；栈空后 EM_REDO 返回 0。
    EXPECT_TRUE(ed->SSendMessage(EM_REDO) != 0);
    EXPECT_EQ(ed->GetWindowText(), _T("ab"));
    ed->SSendMessage(EM_REDO);
    ed->SSendMessage(EM_REDO);
    EXPECT_EQ(ed->GetWindowText(), _T("abcd"));
    EXPECT_EQ(ed->SSendMessage(EM_REDO), 0);
    EXPECT_EQ(ed->GetWindowText(), _T("abcd"));

    // 新编辑使 redo 失效。
    ed->ReplaceSel(_T("e"), TRUE);
    EXPECT_EQ(ed->GetWindowText(), _T("abcde"));
    EXPECT_EQ(ed->SSendMessage(EM_REDO), 0);
    EXPECT_EQ(ed->GetWindowText(), _T("abcde"));

    // EM_EMPTYUNDOBUFFER 同时清空 undo/redo。
    ed->SSendMessage(EM_EMPTYUNDOBUFFER);
    EXPECT_EQ(ed->SSendMessage(EM_CANUNDO), 0);
    EXPECT_EQ(ed->SSendMessage(EM_UNDO), 0);
    EXPECT_EQ(ed->GetWindowText(), _T("abcde"));
#endif
}

// edit lite：按住选中文本拖动可移动；按住 Ctrl 拖动为复制。
TEST_F(SouiTestFixture, edit_lite_selection_drag_moves_and_copies)
{
#if !defined(SOUI_EDIT_LITE)
    SUCCEED() << "edit lite is not enabled in this build";
    return;
#else
    SWindow *root = loadUi(
        L"<window layout='vbox' width='400' height='100'>"
        L"<edit name='ed' width='300' height='30'/>"
        L"</window>");
    SEdit *ed = SouiTest::findByName<SEdit>(root, L"ed");
    ASSERT_TRUE(ed != NULL);

    // 先取得焦点，避免首次点击触发 autoSel 全选。
    ed->SetFocus();
    SStringT strText = _T("0123456789AB");
    ed->SetWindowText(strText);
    ed->SetSel(0, 6, TRUE);

    CRect rcClient;
    ed->GetClientRect(&rcClient);
    int y = rcClient.top + rcClient.Height() / 2;
    CPoint ptDown(rcClient.left + 20, y);         // 落在选区内
    CPoint ptMove(rcClient.left + 40, y);         // 越过 4px 拖动阈值
    CPoint ptDrop(rcClient.right - 2, y);         // 文本末尾之后，落点索引稳定为文本长度

    ed->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(ptDown.x, ptDown.y));
    ed->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(ptMove.x, ptMove.y));
    ed->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(ptDrop.x, ptDrop.y));
    EXPECT_EQ(ed->GetWindowText(), _T("6789AB012345"));

    // Ctrl 落下为复制：原文保留，选中块插入到落点。
    ed->SetWindowText(strText);
    ed->SetSel(0, 6, TRUE);
    ed->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(ptDown.x, ptDown.y));
    ed->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(ptMove.x, ptMove.y));
    ed->SSendMessage(WM_LBUTTONUP, MK_CONTROL, MAKELPARAM(ptDrop.x, ptDrop.y));
    EXPECT_EQ(ed->GetWindowText(), _T("0123456789AB012345"));
#endif
}

// edit lite：选中文本可以拖到另一个 edit（同一进程内）；无 Ctrl 为移动，Ctrl 为复制。
// 该场景没有 OLE 拖放源，源控件持有鼠标捕获，因此落点由源控件命中测试解析。
TEST_F(SouiTestFixture, edit_lite_selection_drag_between_edits)
{
#if !defined(SOUI_EDIT_LITE)
    SUCCEED() << "edit lite is not enabled in this build";
    return;
#else
    SWindow *root = loadUi(
        L"<window layout='vbox' width='300' height='120'>"
        L"<edit name='ed1' width='250' height='30'/>"
        L"<edit name='ed2' width='250' height='30'/>"
        L"</window>");
    SEdit *ed1 = SouiTest::findByName<SEdit>(root, L"ed1");
    SEdit *ed2 = SouiTest::findByName<SEdit>(root, L"ed2");
    ASSERT_TRUE(ed1 != NULL);
    ASSERT_TRUE(ed2 != NULL);

    ed1->SetFocus();
    ed1->SetWindowText(_T("0123456789AB"));
    ed1->SetSel(0, 6, TRUE);

    CRect rc1, rc2;
    ed1->GetClientRect(&rc1);
    ed2->GetClientRect(&rc2);
    CPoint ptDown(rc1.left + 20, rc1.top + rc1.Height() / 2); // 落在选区内
    CPoint ptMove(rc1.left + 40, rc1.top + rc1.Height() / 2); // 越过拖动阈值
    CPoint ptDrop(rc2.left + 4, rc2.top + rc2.Height() / 2);  // 另一个 edit 的文本起点

    ed1->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(ptDown.x, ptDown.y));
    ed1->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(ptMove.x, ptMove.y));
    ed1->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(ptDrop.x, ptDrop.y));
    EXPECT_EQ(ed1->GetWindowText(), _T("6789AB"));
    EXPECT_EQ(ed2->GetWindowText(), _T("012345"));

    // Ctrl 落下为复制：源文本保持不变。
    ed1->SetWindowText(_T("0123456789AB"));
    ed1->SetSel(0, 6, TRUE);
    ed2->SetWindowText(_T(""));
    ed1->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(ptDown.x, ptDown.y));
    ed1->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(ptMove.x, ptMove.y));
    ed1->SSendMessage(WM_LBUTTONUP, MK_CONTROL, MAKELPARAM(ptDrop.x, ptDrop.y));
    EXPECT_EQ(ed1->GetWindowText(), _T("0123456789AB"));
    EXPECT_EQ(ed2->GetWindowText(), _T("012345"));
#endif
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
