/**
 * @file  test_gui_combobox.cpp
 * @brief SouiTest GUI 层用例：可编辑组合框在下拉列表中选择项后编辑框内容同步。
 *
 * 回归的缺陷：鼠标点击下拉列表项时，消息顺序是
 *   WM_LBUTTONUP -> SListBox::OnLButtonUp -> __baseCls::OnLButtonUp(SWindow)
 *               -> FireCommand() 发 EVT_CMD -> SComboBox::FireEvent 执行 CloseUp()
 *               -> 下拉窗口销毁（m_pDropDownWnd 置空）
 *               -> 回到 SListBox::OnLButtonUp 才 NotifySelChange() 发 EVT_LB_SELCHANGED
 * 而 SComboBox::FireEvent 原来用 "evt->IdFrom()==IDC_DROPDOWN_LIST && m_pDropDownWnd"
 * 作为进入条件，事件到达时下拉窗口已经销毁，于是 OnSelChanged() 被丢弃：
 * 列表选中项变了（GetCurSel 同步），但内嵌编辑框文本与 EVT_CB_SELCHANGE 都没有发生。
 *
 * 该用例需要真实宿主窗口（下拉列表是独立的弹出窗口），因此按仓库约定放在
 * test_gui_*.cpp（soui-gui 层，默认不注册，需 -DSOUI_ENABLE_GUI_SMOKE=ON）。
 */
#include <souistd.h>
#include <core/SHostWnd.h>
#include <control/SComboBox.h>
#include <commgr2.h>
#include <xml/SXml.h>
#include <SouiFactory.h>
#include <gtest/gtest.h>

using namespace SOUI;

namespace {

struct CbxHostGuard {
    SHostWnd *host;
    ~CbxHostGuard()
    {
        if (host->m_hWnd)
            host->DestroyWindow();
    }
};

struct SelChangeCounter {
    int count;
    SelChangeCounter()
        : count(0)
    {
    }
    BOOL onSelChange(IEvtArgs *)
    {
        ++count;
        return TRUE;
    }
};

struct SlotGuard {
    SWindow *window;
    DWORD eventId;
    const IEvtSlot *slot;
    ~SlotGuard()
    {
        window->UnsubscribeEvent(eventId, slot);
    }
};

BOOL LoadSysRes(SApplication &app, SComMgr2 &comMgr, SAutoRefPtr<IRenderFactory> &renderFactory)
{
    SAutoRefPtr<IImgDecoderFactory> imgDec;
    if (comMgr.CreateImgDecoder((IObjRef **)&imgDec) && imgDec)
        renderFactory->SetImgDecoderFactory(imgDec);

    HMODULE hSysRes = ::LoadLibrary(_T("soui-sys-resource"));
    if (!hSysRes)
        return FALSE;
    SouiFactory fac;
    SAutoRefPtr<IResProvider> sysRes;
    sysRes.Attach(fac.CreateResProvider(RES_PE));
    if (!sysRes || !sysRes->Init((WPARAM)hSysRes, 0))
        return FALSE;
    app.LoadSystemNamedResource(sysRes);
    return TRUE;
}

} // namespace

// 在弹出窗口客户区点击下拉列表的第 nIndex 项（列表与弹出窗口共享同一坐标系）
static void ClickDropdownItem(SListBox *lb, int nIndex)
{
    HWND hDrop = lb->GetContainer()->GetHostHwnd();
    ASSERT_TRUE(hDrop != NULL);
    CRect rcLb = lb->GetWindowRect();
    int nItemHei = lb->GetItemHeight();
    LPARAM pt = MAKELPARAM(rcLb.left + 10, rcLb.top + nItemHei * nIndex + nItemHei / 2);
    ::SendMessage(hDrop, WM_MOUSEMOVE, 0, pt);
    ::SendMessage(hDrop, WM_LBUTTONDOWN, MK_LBUTTON, pt);
    ::SendMessage(hDrop, WM_LBUTTONUP, 0, pt);
}

/**
 * 可编辑组合框（dropDown=0）：点击下拉列表第二项。
 * 期望：列表选中项=1、内嵌编辑框文本="item1"、EVT_CB_SELCHANGE 恰好触发一次。
 */
TEST(window, combobox_editable_edit_text_syncs_on_dropdown_select)
{
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFactory;
    ASSERT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
    ASSERT_TRUE(renderFactory);
    SApplication app(renderFactory, NULL);
    ASSERT_TRUE(LoadSysRes(app, comMgr, renderFactory));

    static const char xml[] = "<SOUI title='cbx' translucent='0'>"
                              "<root width='300' height='60' layout='vbox' colorBkgnd='#ffffff'>"
                              "<combobox name='cbx' size='-2,30' dropDown='0' dropHeight='120' curSel='0' animateTime='0'>"
                              "<dropdownStyle><root colorBkgnd='#ffffff'/></dropdownStyle>"
                              "<liststyle itemHeight='24'/>"
                              "<editstyle/>"
                              "<items><item text='item0'/><item text='item1'/><item text='item2'/></items>"
                              "</combobox>"
                              "</root></SOUI>";
    SXmlDoc doc;
    ASSERT_TRUE(doc.LoadBuffer(xml, sizeof(xml) - 1, 0, enc_utf8));
    SXmlNode xmlRoot = doc.root().first_child();
    ASSERT_TRUE(xmlRoot);

    SHostWnd host;
    CbxHostGuard hostGuard;
    hostGuard.host = &host;
    HWND hwnd = host.CreateEx(NULL, WS_POPUP, 0, 100, 100, 300, 60, &xmlRoot);
    ASSERT_TRUE(hwnd);
    host.ShowWindow(SW_SHOW);

    SComboBox *cbx = (SComboBox *)host.FindChildByName("cbx");
    ASSERT_TRUE(cbx != NULL);
    ASSERT_EQ(cbx->GetCount(), 3);
    EXPECT_EQ(S_CT2A(cbx->GetWindowText(), CP_UTF8), SStringA("item0"));

    SelChangeCounter counter;
    MemberFunctionSlot<SelChangeCounter, IEvtArgs> slot = Subscriber(&SelChangeCounter::onSelChange, &counter);
    ASSERT_TRUE(cbx->SubscribeEvent(EventCBSelChange::EventID, &slot));
    SlotGuard slotGuard = { cbx, EventCBSelChange::EventID, &slot };

    // 打开下拉列表：列表被挂到独立的弹出窗口上
    cbx->DropDown();
    SListBox *lb = cbx->GetListBox();
    ASSERT_TRUE(lb != NULL);
    ASSERT_TRUE(lb->GetContainer()->GetHostHwnd() != hwnd);

    ClickDropdownItem(lb, 1);

    EXPECT_EQ(cbx->GetCurSel(), 1);
    EXPECT_EQ(S_CT2A(cbx->GetWindowText(), CP_UTF8), SStringA("item1"));
    EXPECT_EQ(counter.count, 1);
    // 点击列表项后下拉窗口应已关闭、列表被挂回宿主容器
    EXPECT_TRUE(lb->GetContainer()->GetHostHwnd() == hwnd);

    // 程序化 SetCurSel 仍只同步一次，不应因上面的改动产生重复事件
    cbx->SetCurSel(2);
    EXPECT_EQ(S_CT2A(cbx->GetWindowText(), CP_UTF8), SStringA("item2"));
    EXPECT_EQ(counter.count, 2);

    // 再次下拉并选择第一项，验证重复打开后的同步依旧正确
    cbx->DropDown();
    ASSERT_TRUE(lb->GetContainer()->GetHostHwnd() != hwnd);
    ClickDropdownItem(lb, 0);
    EXPECT_EQ(cbx->GetCurSel(), 0);
    EXPECT_EQ(S_CT2A(cbx->GetWindowText(), CP_UTF8), SStringA("item0"));
    EXPECT_EQ(counter.count, 3);
}
