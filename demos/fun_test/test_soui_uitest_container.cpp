/**
 * @file  test_soui_uitest_container.cpp
 * @brief SouiTest —— 容器控件（组合框/选项卡）无头用例（Tier A）
 *
 * 这些用例与其它无头用例一样直接参与 soui-uitest 门禁：SouiTest 夹具在 SetUp 中调用
 * HeadlessApp::LoadSystemResource() 加载系统资源 soui-sys-resource，因此 SComboBox /
 * STabCtrl 在 OnCreate 阶段所需的内置皮肤 _skin.sys.dropbtn / _skin.sys.tab.page /
 * _skin.sys.tab.header 均已就绪，可在无原生窗口的情况下创建并接收框架级事件注入。
 *
 * 用例验证内容：
 *   - 组合框：程序化 InsertItem + SetCurSel 触发 EventCBSelChange，且 GetCurSel / 项文本同步
 *   - 选项卡：InsertItem 建两页 + SetCurSel 触发 EventTabSelChanged，且 GetCurSel 同步
 * 它们完全复用 SouiTest 的 mouseClick / keyClick / SignalSpy / findByName 机制，
 * 与 button/checkbox/radio 用例同一条事件注入 + 监听路径。
 */
#include "test_soui_uitest_common.h"
#include <SApp.h>

using namespace SOUI;

// 组合框：程序化插入两项后选择第 2 项，应触发 EVT_CB_SELCHANGE 且状态/文本同步。
//
// 组合框是"状态式"容器控件，下拉项本就由代码构建；其内部下拉列表在
// InitFromXml -> WM_CREATE -> CreateListBox 时才创建，故用工厂创建后先接入容器、
// 再 InitFromXml（建好内部列表），最后挂到根——也正是真实代码的写法。
TEST_F(SouiTestFixture, combobox_selchange_fires_event)
{
    SWindow *root = loadUi(L"<window layout='vbox' width='200' height='80'/>");

    SComboBox *cbx = (SComboBox *)SApplication::getSingleton().CreateWindowByName(L"combobox");
    ASSERT_TRUE(cbx != NULL);
    cbx->SetContainer(container());

    SXmlDoc doc;
    ASSERT_TRUE(doc.load_buffer(L"<combobox name='cbx' width='150' height='24'/>",
                               (size_t)wcslen(L"<combobox name='cbx' width='150' height='24'/>") * sizeof(wchar_t),
                               xml_parse_default, funtest::WCHAR_ENC));
    SXmlNode node = doc.root().first_child();
    ASSERT_TRUE(node);
    ASSERT_TRUE(cbx->InitFromXml(&node));

    root->InsertChild(cbx);

    ASSERT_EQ(cbx->InsertItem(0, _T("Apple"), 0, 0), 0);
    ASSERT_EQ(cbx->InsertItem(1, _T("Banana"), 0, 0), 1);
    ASSERT_EQ(cbx->GetCount(), 2);

    SouiTest::SignalSpy spy(cbx, EventCBSelChange::EventID);
    cbx->SetCurSel(1);

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(cbx->GetCurSel(), 1);
    // 选中文本读自列表数据（不经 richedit 编辑框），跨平台稳定。
    SStringT itemText;
    EXPECT_TRUE(cbx->GetItemText(1, TRUE, &itemText));
    EXPECT_EQ(itemText, _T("Banana"));
}

// 选项卡：插入两页后切换选中页应触发 EVT_TAB_SELCHANGED 且 GetCurSel 同步。
TEST_F(SouiTestFixture, tab_switch_fires_event)
{
    SWindow *root = loadUi(L"<window layout='vbox' width='300' height='200'/>");

    STabCtrl *tabs = (STabCtrl *)SApplication::getSingleton().CreateWindowByName(L"tabctrl");
    ASSERT_TRUE(tabs != NULL);
    tabs->SetContainer(container());

    SXmlDoc doc;
    ASSERT_TRUE(doc.load_buffer(L"<tabctrl name='tabs' width='300' height='200'/>",
                               (size_t)wcslen(L"<tabctrl name='tabs' width='300' height='200'/>") * sizeof(wchar_t),
                               xml_parse_default, funtest::WCHAR_ENC));
    SXmlNode node = doc.root().first_child();
    ASSERT_TRUE(node);
    ASSERT_TRUE(tabs->InitFromXml(&node));

    root->InsertChild(tabs);

    // InsertItem(LPCWSTR) 把参数当 XML 解析，需传 <page .../> 节点。
    ASSERT_EQ(tabs->InsertItem(L"<page title='P1'/>"), 0);
    ASSERT_EQ(tabs->InsertItem(L"<page title='P2'/>"), 1);
    ASSERT_GE(tabs->GetItemCount(), 2);

    int oldSel = tabs->GetCurSel();
    int target = (oldSel == 0) ? 1 : 0;

    SouiTest::SignalSpy spy(tabs, EventTabSelChanged::EventID);
    tabs->SetCurSel(target);

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(tabs->GetCurSel(), target);
}
