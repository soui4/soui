/**
 * @file  test_tab_drag.cpp
 * @brief STabCtrl 拖动切换页面（Android ViewPager 风格）无头回归用例
 *
 * 覆盖：属性默认关闭、阈值、左右翻页、回弹、边界 clamp、
 * EventTabSelChanging.bCancel、容器级触摸拦截链、拦截时子控件收到 CANCEL、
 * 子控件拒绝放弃捕获、requestDisallowInterceptTouchEvent、ACTION_CANCEL 回弹、
 * tabAlign 自动决定滑动方向、同轴向嵌套时内层优先与边界回退到外层，
 * 以及 SGestureDetector 本身的探针/回调语义。
 *
 * dragCommitDuration="0" 让提交/回弹同步完成，不依赖 timeline 泵；
 * dragFling="0" 排除释放速度的干扰，断言只取决于位移比例。
 */
#include "test_soui_uitest_common.h"
#include <SApp.h>
#include <control/STabCtrl.h>
#include <core/SGestureDetector.h>

#include <string>

using namespace SOUI;

namespace
{
    // 在 EventTabSelChanging 中把 bCancel 置 TRUE，模拟业务层禁止切页。
    struct TabSelCanceller
    {
        BOOL fired;
        TabSelCanceller()
            : fired(FALSE)
        {
        }
        BOOL OnSelChanging(IEvtArgs *pEvt)
        {
            EventTabSelChanging *p = static_cast<EventTabSelChanging *>(pEvt);
            fired = TRUE;
            p->bCancel = TRUE;
            return FALSE;
        }
    };

    /** 记录 CancelCaptureMode 调用，可模拟"子控件拒绝放弃手势"。 */
    class CancelSpyWindow : public SWindow
    {
        DEF_SOBJECT(SWindow, L"cancelspy")

      public:
        int m_cancelCalls;
        int m_lastReason;
        BOOL m_refuse;
        CancelSpyWindow()
            : m_cancelCalls(0)
            , m_lastReason(-1)
            , m_refuse(FALSE)
        {
        }
        virtual BOOL CancelCaptureMode(int reason)
        {
            ++m_cancelCalls;
            m_lastReason = reason;
            if (m_refuse)
                return FALSE;
            return SWindow::CancelCaptureMode(reason);
        }
    };

    /** 仅用于模拟迟到的动画回调，避免为测试给 STabCtrl 增加公开接口。
     *  通过成员指针访问受保护成员，不把基类对象强转成派生类型。 */
    class TabHook : public STabCtrl
    {
      public:
        static BOOL HasSlider(STabCtrl *p)
        {
            static STabSlider *STabCtrl::*pSlider = &TabHook::m_tabSlider;
            return (p->*pSlider) != NULL;
        }
        static void FinishSlider(STabCtrl *p)
        {
            static void (STabCtrl::*pFinish)() = &TabHook::OnSliderFinish;
            (p->*pFinish)();
        }
    };

    /** 模拟 fundholdings 的三页滑动容器：订阅者在 Changed 里同步复位到基准页。 */
    struct SwipeResetter
    {
        STabCtrl *tabs;
        int fired;
        BOOL sliderAtNotify;

        SwipeResetter()
            : tabs(NULL)
            , fired(0)
            , sliderAtNotify(TRUE)
        {
        }
        BOOL OnSelChanged(IEvtArgs *pEvt)
        {
            ++fired;
            if (fired == 1 && tabs)
                sliderAtNotify = TabHook::HasSlider(tabs);
            if (tabs)
                tabs->SetCurSel(0);
            return FALSE;
        }
    };

    class TabDragTest : public SouiTestFixture
    {
      public:
        STabCtrl *MakeTabs(const wchar_t *attrs)
        {
            std::wstring xml = L"<window layout='vbox' width='300' height='200'>"
                               L"<tabctrl name='tabs' size='300,200' tabHeight='0' ";
            xml += attrs;
            xml += L"><page title='P1'/><page title='P2'/></tabctrl></window>";
            SWindow *root = loadUi(xml.c_str(), CRect(0, 0, 300, 200));
            STabCtrl *tabs = (STabCtrl *)root->FindChildByName(L"tabs");
            EXPECT_TRUE(tabs != NULL);
            // SwndContainerImpl 默认不设置 root；容器级 DoFrameEvent 需要它。
            container()->SetRoot(root);
            return tabs;
        }

        /** 两层同轴向 tabctrl（都保持默认 tabAlign=top），复现 page_skia.xml 的嵌套。 */
        void MakeNestedTabs(const wchar_t *attrs, STabCtrl **ppOuter, STabCtrl **ppInner)
        {
            std::wstring xml = L"<window layout='vbox' width='300' height='200'>"
                               L"<tabctrl name='outer' size='300,200' tabHeight='0' ";
            xml += attrs;
            xml += L"><page title='O1'><tabctrl name='inner' size='300,200' tabHeight='0' ";
            xml += attrs;
            xml += L"><page title='I1'/><page title='I2'/></tabctrl></page>"
                   L"<page title='O2'/></tabctrl></window>";
            SWindow *root = loadUi(xml.c_str(), CRect(0, 0, 300, 200));
            *ppOuter = (STabCtrl *)root->FindChildByName(L"outer");
            *ppInner = (STabCtrl *)root->FindChildByName(L"inner");
            container()->SetRoot(root);
        }

        /** 给当前页插入一个可观察 CANCEL 的子窗口，覆盖 (200..300, 50..150)。 */
        CancelSpyWindow *AddSpy(STabCtrl *tabs)
        {
            STabPage *page = tabs->GetItem(0);
            EXPECT_TRUE(page != NULL);
            if (!page)
                return NULL;
            CancelSpyWindow *spy = new CancelSpyWindow;
            spy->SetContainer(container());
            page->InsertChild(spy);
            spy->Move(CRect(200, 50, 300, 150));
            return spy;
        }

        // 一次完整的按下 -> 跨过阈值 -> 继续移动 -> 松手（窗口自身作为命中目标）。
        void Drag(SWindow *wnd, int x0, int y0, int x1, int y1, int x2, int y2)
        {
            wnd->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x0, y0));
            wnd->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x1, y1));
            wnd->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x2, y2));
            wnd->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(x2, y2));
        }

        // 走真实容器派发，按下点落在页面子窗口上。
        void FrameDrag(int x0, int y0, int x1, int y1, int x2, int y2)
        {
            container()->DoFrameEvent(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x0, y0));
            container()->DoFrameEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x1, y1));
            container()->DoFrameEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x2, y2));
            container()->DoFrameEvent(WM_LBUTTONUP, 0, MAKELPARAM(x2, y2));
        }
    };

    const wchar_t *kDragOn = L"enableDragSwitch='1' dragCommitDuration='0' dragFling='0'";

    struct GestureRecorder : public IGestureListener
    {
        int down, scroll, fling, up, cancel;
        GestureRecorder()
            : down(0)
            , scroll(0)
            , fling(0)
            , up(0)
            , cancel(0)
        {
        }
        virtual void OnGestureDown(const CPoint &) { ++down; }
        virtual void OnGestureScroll(const CPoint &, const CPoint &, int, int) { ++scroll; }
        virtual void OnGestureFling(float, float) { ++fling; }
        virtual void OnGestureUp(const CPoint &, float, float) { ++up; }
        virtual void OnGestureCancel() { ++cancel; }
    };
} // namespace

// 默认关闭：即使完整拖动也不切页、不发事件。
TEST_F(TabDragTest, drag_disabled_by_default)
{
    STabCtrl *tabs = MakeTabs(L"dragCommitDuration='0' dragFling='0'");
    ASSERT_TRUE(tabs != NULL);
    SouiTest::SignalSpy spy(tabs, EventTabSelChanged::EventID);
    Drag(tabs, 250, 100, 200, 100, 50, 100);
    EXPECT_EQ(tabs->GetCurSel(), 0);
    EXPECT_EQ(spy.count(), 0);
}

// 位移未过阈值：只是一次点击，页面保持不变。
TEST_F(TabDragTest, below_threshold_keeps_selection)
{
    STabCtrl *tabs = MakeTabs(kDragOn);
    ASSERT_TRUE(tabs != NULL);
    SouiTest::SignalSpy spy(tabs, EventTabSelChanged::EventID);
    tabs->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(200, 100));
    tabs->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(196, 100));
    tabs->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(196, 100));
    EXPECT_EQ(tabs->GetCurSel(), 0);
    EXPECT_EQ(spy.count(), 0);
}

// 向左拖动超过 35% 页宽 -> 切到下一页，并发出一次 EventTabSelChanged。
TEST_F(TabDragTest, drag_left_switches_to_next_page)
{
    STabCtrl *tabs = MakeTabs(kDragOn);
    ASSERT_TRUE(tabs != NULL);
    SouiTest::SignalSpy spy(tabs, EventTabSelChanged::EventID);
    Drag(tabs, 250, 100, 200, 100, 50, 100);
    EXPECT_EQ(tabs->GetCurSel(), 1);
    EXPECT_EQ(spy.count(), 1);
}

// 向右拖动 -> 切到上一页。
TEST_F(TabDragTest, drag_right_switches_to_previous_page)
{
    STabCtrl *tabs = MakeTabs(kDragOn);
    ASSERT_TRUE(tabs != NULL);
    ASSERT_TRUE(tabs->SetCurSel(1));
    Drag(tabs, 50, 100, 100, 100, 250, 100);
    EXPECT_EQ(tabs->GetCurSel(), 0);
}

// 位移不足提交比例 -> 回弹，保持原页且不发切换事件。
TEST_F(TabDragTest, short_drag_snaps_back)
{
    STabCtrl *tabs = MakeTabs(kDragOn);
    ASSERT_TRUE(tabs != NULL);
    SouiTest::SignalSpy spy(tabs, EventTabSelChanged::EventID);
    Drag(tabs, 250, 100, 200, 100, 190, 100);
    EXPECT_EQ(tabs->GetCurSel(), 0);
    EXPECT_EQ(spy.count(), 0);
}

// 首页没有上一页：向边界方向拖动不应越界。
TEST_F(TabDragTest, boundary_page_is_clamped)
{
    STabCtrl *tabs = MakeTabs(kDragOn);
    ASSERT_TRUE(tabs != NULL);
    Drag(tabs, 50, 100, 100, 100, 250, 100);
    EXPECT_EQ(tabs->GetCurSel(), 0);
}

// EventTabSelChanging.bCancel 为 TRUE 时禁止切页（与 SetCurSel 语义一致）。
TEST_F(TabDragTest, selchanging_cancel_blocks_switch)
{
    STabCtrl *tabs = MakeTabs(kDragOn);
    ASSERT_TRUE(tabs != NULL);
    TabSelCanceller canceller;
    ASSERT_TRUE(tabs->GetEventSet()->subscribeEvent(
        EventTabSelChanging::EventID, Subscriber(&TabSelCanceller::OnSelChanging, &canceller)));
    Drag(tabs, 250, 100, 200, 100, 50, 100);
    EXPECT_TRUE(canceller.fired);
    EXPECT_EQ(tabs->GetCurSel(), 0);
}

// 按下落在页面子窗口时，容器的拦截链把手势交给 tabctrl（不是旁路副本）。
TEST_F(TabDragTest, intercept_route_from_page_press)
{
    STabCtrl *tabs = MakeTabs(kDragOn);
    ASSERT_TRUE(tabs != NULL);
    CPoint ptHit(250, 100);
    SWND hHit = tabs->GetRoot()->SwndFromPoint(ptHit);
    ASSERT_TRUE(hHit != 0);
    EXPECT_TRUE(hHit != tabs->GetSwnd()); // 证明命中的是页面/子窗口

    SouiTest::SignalSpy spy(tabs, EventTabSelChanged::EventID);
    FrameDrag(250, 100, 200, 100, 50, 100);
    EXPECT_EQ(tabs->GetCurSel(), 1);
    EXPECT_EQ(spy.count(), 1);
}

// DOWN 不拦截：子控件持有捕获；超阈值后父接管，且子控件收到 CANCEL。
TEST_F(TabDragTest, child_receives_cancel_on_intercept)
{
    STabCtrl *tabs = MakeTabs(kDragOn);
    ASSERT_TRUE(tabs != NULL);
    CancelSpyWindow *spy = AddSpy(tabs);
    ASSERT_TRUE(spy != NULL);

    container()->DoFrameEvent(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(250, 100));
    EXPECT_EQ(tabs->GetCapture(), spy->GetSwnd()); // ACTION_DOWN 未被拦截

    container()->DoFrameEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(200, 100));
    EXPECT_GE(spy->m_cancelCalls, 1);             // CancelCaptureMode + WM_CANCELMODE
    EXPECT_EQ(tabs->GetCapture(), tabs->GetSwnd()); // 捕获转交给拦截者

    container()->DoFrameEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 100));
    container()->DoFrameEvent(WM_LBUTTONUP, 0, MAKELPARAM(50, 100));
    EXPECT_EQ(tabs->GetCurSel(), 1);
}

// 子控件拒绝放弃捕获（SPanel 在有滚动条/框选时就是这样）：父不得抢手势。
TEST_F(TabDragTest, child_refusal_blocks_intercept)
{
    STabCtrl *tabs = MakeTabs(kDragOn);
    ASSERT_TRUE(tabs != NULL);
    CancelSpyWindow *spy = AddSpy(tabs);
    ASSERT_TRUE(spy != NULL);
    spy->m_refuse = TRUE;

    container()->DoFrameEvent(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(250, 100));
    container()->DoFrameEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(200, 100));
    // 拒绝后捕获仍归子控件（松手时 SWindow::OnLButtonUp 会正常释放，故在此处断言）。
    EXPECT_GE(spy->m_cancelCalls, 1);
    EXPECT_EQ(tabs->GetCapture(), spy->GetSwnd());

    container()->DoFrameEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 100));
    container()->DoFrameEvent(WM_LBUTTONUP, 0, MAKELPARAM(50, 100));
    EXPECT_EQ(tabs->GetCurSel(), 0);
}

// requestDisallowInterceptTouchEvent：子控件声明手势归自己，父连询问都不做。
TEST_F(TabDragTest, disallow_intercept_blocks_parent)
{
    STabCtrl *tabs = MakeTabs(kDragOn);
    ASSERT_TRUE(tabs != NULL);
    CancelSpyWindow *spy = AddSpy(tabs);
    ASSERT_TRUE(spy != NULL);

    container()->DoFrameEvent(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(250, 100));
    // 容器在 ACTION_DOWN 时清 disallow 标记，所以必须在 DOWN 之后请求。
    spy->RequestDisallowInterceptTouchEvent(TRUE);
    container()->DoFrameEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(200, 100));
    container()->DoFrameEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 100));
    container()->DoFrameEvent(WM_LBUTTONUP, 0, MAKELPARAM(50, 100));
    EXPECT_EQ(spy->m_cancelCalls, 0);
    EXPECT_EQ(tabs->GetCurSel(), 0);
}

// ACTION_CANCEL（WM_CANCELMODE）：跟手中的手势立即回弹，不发切换事件。
TEST_F(TabDragTest, cancel_mode_snaps_back)
{
    STabCtrl *tabs = MakeTabs(kDragOn);
    ASSERT_TRUE(tabs != NULL);
    SouiTest::SignalSpy spy(tabs, EventTabSelChanged::EventID);
    container()->DoFrameEvent(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(250, 100));
    container()->DoFrameEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(200, 100));
    container()->DoFrameEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(150, 100));
    container()->DoFrameEvent(WM_CANCELMODE, 0, 0);
    EXPECT_EQ(tabs->GetCurSel(), 0);
    EXPECT_EQ(spy.count(), 0);
    container()->DoFrameEvent(WM_LBUTTONUP, 0, MAKELPARAM(50, 100));
    EXPECT_EQ(tabs->GetCurSel(), 0);
}

// tabAlign=left 时自动改用垂直轴：上下拖动切页，水平拖动不响应。
TEST_F(TabDragTest, drag_axis_follows_tab_align)
{
    const wchar_t *attrs = L"enableDragSwitch='1' tabAlign='left' tabWidth='0' dragCommitDuration='0' dragFling='0'";
    STabCtrl *tabs = MakeTabs(attrs);
    ASSERT_TRUE(tabs != NULL);
    Drag(tabs, 100, 180, 100, 120, 100, 20);
    EXPECT_EQ(tabs->GetCurSel(), 1);

    STabCtrl *tabs2 = MakeTabs(attrs);
    ASSERT_TRUE(tabs2 != NULL);
    Drag(tabs2, 250, 100, 200, 100, 50, 100);
    EXPECT_EQ(tabs2->GetCurSel(), 0);
}

// 同轴向嵌套（两层都是默认 tabAlign=top）：水平拖动必须由最内层 tabctrl 响应。
// 复现 page_skia.xml 的两层 tabctrl；修复前父链自外向内询问，外层恒赢。
TEST_F(TabDragTest, nested_same_axis_inner_wins)
{
    STabCtrl *outer = NULL, *inner = NULL;
    MakeNestedTabs(kDragOn, &outer, &inner);
    ASSERT_TRUE(outer != NULL);
    ASSERT_TRUE(inner != NULL);

    CPoint ptHit(250, 100);
    SWND hHit = outer->GetRoot()->SwndFromPoint(ptHit);
    ASSERT_TRUE(hHit != 0);
    EXPECT_TRUE(hHit != outer->GetSwnd()); // 按下落在内层页面的子窗口上

    SouiTest::SignalSpy spyOuter(outer, EventTabSelChanged::EventID);
    SouiTest::SignalSpy spyInner(inner, EventTabSelChanged::EventID);
    FrameDrag(250, 100, 200, 100, 50, 100);
    EXPECT_EQ(inner->GetCurSel(), 1);
    EXPECT_EQ(outer->GetCurSel(), 0);
    EXPECT_EQ(spyInner.count(), 1);
    EXPECT_EQ(spyOuter.count(), 0);
}

// 内层处于边界方向时不拦截，手势回退给外层（ViewPager 的 canScroll 语义），
// 且不会出现"捕获已被抢走但画面不动"的吞手势现象。
TEST_F(TabDragTest, nested_inner_boundary_falls_back_to_outer)
{
    STabCtrl *outer = NULL, *inner = NULL;
    MakeNestedTabs(kDragOn, &outer, &inner);
    ASSERT_TRUE(outer != NULL);
    ASSERT_TRUE(inner != NULL);
    ASSERT_TRUE(inner->SetCurSel(1)); // 内层最后一页：向左拖没有下一页
    ASSERT_EQ(outer->GetCurSel(), 0); // 外层首页：向左拖有下一页

    FrameDrag(250, 100, 200, 100, 50, 100);
    EXPECT_EQ(inner->GetCurSel(), 1);
    EXPECT_EQ(outer->GetCurSel(), 1);
}

// SGestureDetector：OnInterceptTouchEvent 是纯探针，不产生任何手势回调。
TEST(soui_gesture_detector, intercept_probe_does_not_fire_callbacks)
{
    GestureRecorder rec;
    SGestureDetector detector(&rec);
    detector.SetTouchSlop(8);
    detector.OnInterceptTouchEvent(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(250, 100));
    EXPECT_EQ(rec.down, 1);
    EXPECT_FALSE(detector.OnInterceptTouchEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(245, 100)));
    EXPECT_TRUE(detector.OnInterceptTouchEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(200, 100)));
    EXPECT_EQ(rec.scroll, 0);
    EXPECT_FALSE(detector.IsDragging());
}

// SGestureDetector：OnTouchEvent 连续驱动 scroll，CANCEL 走 cancel 而非 up/fling。
TEST(soui_gesture_detector, touch_stream_and_cancel)
{
    GestureRecorder rec;
    SGestureDetector detector(&rec);
    detector.SetTouchSlop(8);
    detector.OnTouchEvent(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(250, 100));
    detector.OnTouchEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(200, 100));
    EXPECT_TRUE(detector.IsDragging());
    EXPECT_EQ(rec.scroll, 1);
    detector.OnTouchEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 100));
    EXPECT_EQ(rec.scroll, 2);
    detector.OnTouchEvent(WM_CANCELMODE, 0, 0);
    EXPECT_EQ(rec.cancel, 1);
    EXPECT_EQ(rec.up, 0);
    EXPECT_EQ(rec.fling, 0);
    EXPECT_FALSE(detector.IsDragging());
}

// ---------------------------------------------------------------------------
// 拖动中丢失捕获（被别的窗口/系统取走，没有 UP）：必须立即取消拖动，
// 否则存活的 slider 会一直把两个页面都藏着，直到下一次 DOWN。
// 提交动画用非 0 时长，保证 slider 在取消前仍然存活。
// ---------------------------------------------------------------------------
namespace
{
    const wchar_t *kDragAnimating = L"enableDragSwitch='1' dragCommitDuration='300' dragFling='0'";

    // 按下并越过阈值，让 STabSlider 创建出来（此时两个页面都被隐藏）。
    void ArmDrag(STabCtrl *tabs)
    {
        tabs->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(250, 100));
        tabs->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(150, 100));
    }
} // namespace

TEST_F(TabDragTest, capture_loss_cancels_drag)
{
    STabCtrl *tabs = MakeTabs(kDragAnimating);
    ASSERT_TRUE(tabs != NULL);
    SouiTest::SignalSpy spy(tabs, EventTabSelChanged::EventID);

    ArmDrag(tabs);
    ASSERT_TRUE(tabs->GetItem(0) != NULL);
    EXPECT_TRUE(TabHook::HasSlider(tabs));
    EXPECT_FALSE(tabs->GetItem(0)->IsVisible()) << L"an active drag hides both pages";

    tabs->ReleaseCapture(); // 触发 OnCaptureChanged(FALSE)
    EXPECT_FALSE(TabHook::HasSlider(tabs));
    EXPECT_TRUE(tabs->GetItem(0)->IsVisible()) << L"cancel must restore the current page";
    EXPECT_EQ(tabs->GetCurSel(), 0);
    EXPECT_EQ(spy.count(), 0);

    // 丢失捕获后的 UP 不能再切页。
    tabs->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(50, 100));
    EXPECT_EQ(tabs->GetCurSel(), 0);
}

TEST_F(TabDragTest, hiding_tabctrl_cancels_drag)
{
    STabCtrl *tabs = MakeTabs(kDragAnimating);
    ASSERT_TRUE(tabs != NULL);
    SouiTest::SignalSpy spy(tabs, EventTabSelChanged::EventID);

    ArmDrag(tabs);
    ASSERT_TRUE(tabs->GetItem(0) != NULL);
    EXPECT_FALSE(tabs->GetItem(0)->IsVisible());

    tabs->SetVisible(FALSE);
    EXPECT_FALSE(TabHook::HasSlider(tabs));
    EXPECT_TRUE(tabs->GetItem(0)->IsVisible());
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(TabDragTest, disabling_tabctrl_cancels_drag)
{
    STabCtrl *tabs = MakeTabs(kDragAnimating);
    ASSERT_TRUE(tabs != NULL);
    SouiTest::SignalSpy spy(tabs, EventTabSelChanged::EventID);

    ArmDrag(tabs);
    ASSERT_TRUE(tabs->GetItem(0) != NULL);
    EXPECT_FALSE(tabs->GetItem(0)->IsVisible());

    tabs->EnableWindow(FALSE);
    EXPECT_FALSE(TabHook::HasSlider(tabs));
    EXPECT_TRUE(tabs->GetItem(0)->IsVisible());
    EXPECT_EQ(spy.count(), 0);
}

// ---------------------------------------------------------------------------
// 页面列表在提交动画结束前被清空：迟到的 OnSliderFinish 不能解引用空 slider，
// 也不能用 -1 的当前页下标越界访问 m_lstPages。
// ---------------------------------------------------------------------------
TEST_F(TabDragTest, slider_finish_after_pages_removed)
{
    STabCtrl *tabs = MakeTabs(kDragAnimating);
    ASSERT_TRUE(tabs != NULL);

    ArmDrag(tabs);
    EXPECT_TRUE(TabHook::HasSlider(tabs));

    tabs->RemoveAllItems();
    EXPECT_EQ(tabs->GetItemCount(), 0);

    TabHook::FinishSlider(tabs); // 迟到的动画结束回调
    EXPECT_FALSE(TabHook::HasSlider(tabs));

    TabHook::FinishSlider(tabs); // 已无 slider：再次回调也必须安全
    EXPECT_FALSE(TabHook::HasSlider(tabs));
}

// ---------------------------------------------------------------------------
// 评审修复回归 ①：拦截者声明消费轴，容器据此生成 CancelCaptureMode reason。
// ---------------------------------------------------------------------------
TEST_F(TabDragTest, intercept_axis_matches_tab_align)
{
    STabCtrl *hori = MakeTabs(L"enableDragSwitch='1' dragCommitDuration='0'");
    ASSERT_TRUE(hori != NULL);
    SWindow *pBase = hori;
    EXPECT_EQ(CANCEL_REASON_SCROLL_H, pBase->GetTouchInterceptAxis());

    STabCtrl *vert = MakeTabs(L"enableDragSwitch='1' tabAlign='left' dragCommitDuration='0'");
    ASSERT_TRUE(vert != NULL);
    EXPECT_EQ(CANCEL_REASON_SCROLL_V, ((SWindow *)vert)->GetTouchInterceptAxis());
}

// ---------------------------------------------------------------------------
// 评审修复回归 ②：45° 拖动没有可靠的「主轴」，reason 必须跟随后者声明的轴。
// ---------------------------------------------------------------------------
TEST_F(TabDragTest, cancel_reason_uses_declared_axis_on_diagonal_drag)
{
    STabCtrl *tabs = MakeTabs(L"enableDragSwitch='1' tabAlign='left' dragCommitDuration='0' dragFling='0'");
    ASSERT_TRUE(tabs != NULL);
    CancelSpyWindow *spy = AddSpy(tabs);
    ASSERT_TRUE(spy != NULL);

    // DOWN (250,100) -> MOVE (230,80)：|dx| == |dy|，旧的猜轴逻辑恒选水平；
    // tabAlign=left 的 tabctrl 消费垂直轴，必须发 SCROLL_V。
    container()->DoFrameEvent(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(250, 100));
    container()->DoFrameEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(230, 80));
    EXPECT_EQ(CANCEL_REASON_SCROLL_V, spy->m_lastReason);
}

// ---------------------------------------------------------------------------
// 评审修复回归 ③：订阅者在 EventTabSelChanged 里同步 SetCurSel() 复位页面时，
// 通知必须晚于 CommitDrag，否则 drag slider 会被提前 Stop、跟手补间被吞掉。
// ---------------------------------------------------------------------------
TEST_F(TabDragTest, selchanged_reentrant_setcursel_after_commit)
{
    STabCtrl *tabs = MakeTabs(kDragOn);
    ASSERT_TRUE(tabs != NULL);
    SwipeResetter resetter;
    resetter.tabs = tabs;
    ASSERT_TRUE(tabs->GetEventSet()->subscribeEvent(
        EventTabSelChanged::EventID, Subscriber(&SwipeResetter::OnSelChanged, &resetter)));

    Drag(tabs, 250, 100, 200, 100, 50, 100);
    EXPECT_EQ(2, resetter.fired) << L"切页 + 复位各通知一次";
    EXPECT_FALSE(resetter.sliderAtNotify) << L"Changed 必须在提交之后发出";
    EXPECT_EQ(0, tabs->GetCurSel());
    EXPECT_FALSE(TabHook::HasSlider(tabs));
}

