/**
 * @file  test_panel_gesture.cpp
 * @brief SPanel 接入 SGestureDetector 后的无头回归用例
 *
 * 覆盖：touch slop 门槛、X/Y 双轴同手势滚动、第一次越过阈值不产生位移、
 * WM_CANCELMODE 清理、无配对 DOWN 的 UP 不触发手势回调、非滚动面板不吞点击，
 * 以及 SGestureDetector::AxisBoth 与既有单轴语义互不干扰。
 *
 * 直接 SSendMessage 驱动控件自身消息映射（与 test_rubberband.cpp 一致），
 * 不依赖容器拦截链；速度倍率依赖真实时钟，故只断言位移方向与是否为 0。
 */
#include "test_soui_uitest_common.h"
#include <SApp.h>
#include <core/SGestureDetector.h>
#include <core/SPanel.h>

using namespace SOUI;

namespace
{
    /** 用自己的 SOUI_MSG_MAP 拦截 DOWN/MOVE/UP 并链式调用 SPanel，
     *  记录「未被手势吞掉」的点击，避免依赖真实滚动条皮肤与定时器。 */
    class GesturePanel : public SPanel
    {
        DEF_SOBJECT(SPanel, L"gesturepanel")

      public:
        int downHits;
        int moveHits;
        int upHits;
        int dragV;
        int dragH;
        int dragStarts;
        int flings;
        BOOL bandEnabled;

        GesturePanel()
            : downHits(0)
            , moveHits(0)
            , upHits(0)
            , dragV(0)
            , dragH(0)
            , dragStarts(0)
            , flings(0)
            , bandEnabled(FALSE)
        {
        }

        void SetBandEnabled(BOOL bEnable)
        {
            bandEnabled = bEnable;
        }

        void ResetCounters()
        {
            downHits = moveHits = upHits = 0;
            dragV = dragH = dragStarts = flings = 0;
        }

        void EnableBothBars()
        {
            ShowScrollBar(SSB_BOTH, TRUE);
        }

        /** CancelCaptureMode 在 SPanel 中是 protected，测试通过派生类转发。 */
        BOOL TryCancel(int reason)
        {
            return CancelCaptureMode(reason);
        }

      protected:
        virtual BOOL IsRubberBandSelEnabled() const
        {
            return bandEnabled;
        }

        // 常规事件映射拦截：先链式调用 SPanel，被手势吞掉就不跑自己的逻辑。
        void OnLButtonDown(UINT nFlags, CPoint pt)
        {
            if (__baseCls::OnLButtonDown(nFlags, pt))
                return;
            ++downHits;
        }
        void OnMouseMove(UINT nFlags, CPoint pt)
        {
            if (__baseCls::OnMouseMove(nFlags, pt))
                return;
            ++moveHits;
        }
        void OnLButtonUp(UINT nFlags, CPoint pt)
        {
            if (__baseCls::OnLButtonUp(nFlags, pt))
                return;
            ++upHits;
        }

        SOUI_MSG_MAP_BEGIN()
            MSG_WM_LBUTTONDOWN(OnLButtonDown)
            MSG_WM_MOUSEMOVE(OnMouseMove)
            MSG_WM_LBUTTONUP(OnLButtonUp)
        SOUI_MSG_MAP_END()
        virtual void OnDragScrollStart()
        {
            ++dragStarts;
        }
        virtual void OnDragScroll(BOOL bVert, int delta)
        {
            if (bVert)
                dragV += delta;
            else
                dragH += delta;
        }
        virtual void OnFlingScroll()
        {
            ++flings;
        }
    };

    class PanelGestureTest : public SouiTestFixture
    {
      public:
        GesturePanel *MakePanel(bool bScrollable)
        {
            SWindow *root = loadUi(L"<window width='300' height='200'/>", CRect(0, 0, 300, 200));
            if (!root)
                return NULL;
            GesturePanel *panel = new GesturePanel;
            panel->SetContainer(container());
            root->InsertChild(panel);
            panel->Move(CRect(0, 0, 300, 200));
            container()->SetRoot(root);
            if (bScrollable)
                panel->EnableBothBars();
            return panel;
        }

        void Down(GesturePanel *p, int x, int y)
        {
            p->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
        }
        void Move(GesturePanel *p, int x, int y)
        {
            p->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x, y));
        }
        void Up(GesturePanel *p, int x, int y)
        {
            p->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(x, y));
        }
    };

    /** 纯探测器测试用的回调计数器。 */
    struct GestureRecorder : public IGestureListener
    {
        int down;
        int scroll;
        int fling;
        int up;
        int cancel;
        GestureRecorder()
            : down(0)
            , scroll(0)
            , fling(0)
            , up(0)
            , cancel(0)
        {
        }
        virtual void OnGestureDown(const CPoint &)
        {
            ++down;
        }
        virtual void OnGestureScroll(const CPoint &, const CPoint &, int, int)
        {
            ++scroll;
        }
        virtual void OnGestureFling(float, float)
        {
            ++fling;
        }
        virtual void OnGestureUp(const CPoint &, float, float)
        {
            ++up;
        }
        virtual void OnGestureCancel()
        {
            ++cancel;
        }
    };
} // namespace

// ---------------------------------------------------------------------------
// 阈值之内：不进入拖动，UP 必须照常回到派生类的 OnLButtonUp（点击语义不丢失）。
// ---------------------------------------------------------------------------
TEST_F(PanelGestureTest, below_touch_slop_keeps_click_semantics)
{
    GesturePanel *p = MakePanel(true);
    ASSERT_TRUE(p != NULL);

    Down(p, 100, 100);
    Move(p, 105, 100); // 5px < 8dp
    Up(p, 105, 100);

    EXPECT_EQ(0, p->dragStarts);
    EXPECT_EQ(0, p->dragV);
    EXPECT_EQ(0, p->dragH);
    EXPECT_EQ(1, p->upHits) << L"UP must still reach the derived OnLButtonUp";
}

// ---------------------------------------------------------------------------
// 常规消息映射拦截：派生类自己注册 DOWN/MOVE/UP，链式调用 SPanel 后只有
// 「未被吞掉」的事件才落到自己的逻辑；完成拖动时 MOVE / UP 都不落到派生逻辑。
// ---------------------------------------------------------------------------
TEST_F(PanelGestureTest, derived_message_map_chains_to_spanel)
{
    GesturePanel *p = MakePanel(true);
    ASSERT_TRUE(p != NULL);

    Down(p, 100, 100);
    EXPECT_EQ(1, p->downHits);
    Move(p, 105, 100); // 5px < 8dp：未越过阈值，派生 MOVE 生效
    EXPECT_EQ(1, p->moveHits);
    Up(p, 105, 100);
    EXPECT_EQ(1, p->upHits);

    p->ResetCounters();
    Down(p, 100, 100);
    Move(p, 100, 120); // 越过阈值：这次 MOVE 被手势吞掉
    EXPECT_EQ(0, p->moveHits);
    Up(p, 100, 120); // 拖动收尾的 UP 也被吞掉
    EXPECT_EQ(0, p->upHits);
    EXPECT_EQ(1, p->downHits) << L"DOWN itself is not consumed by a normal press";
    EXPECT_EQ(1, p->dragStarts);
}

// ---------------------------------------------------------------------------
// 越过阈值的那一次 MOVE 只负责启动拖动（旧行为），第二次 MOVE 才产生位移。
// ---------------------------------------------------------------------------
TEST_F(PanelGestureTest, threshold_move_only_arms_the_drag)
{
    GesturePanel *p = MakePanel(true);
    ASSERT_TRUE(p != NULL);

    Down(p, 100, 100);
    Move(p, 100, 120); // crosses touch slop
    EXPECT_EQ(1, p->dragStarts);
    EXPECT_EQ(0, p->dragV) << L"the threshold-crossing MOVE must not scroll";
    Move(p, 100, 140);
    EXPECT_GT(p->dragV, 0);
    EXPECT_EQ(0, p->dragH);
    Up(p, 100, 140);
    EXPECT_EQ(0, p->upHits) << L"a completed drag consumes its UP";
}

// ---------------------------------------------------------------------------
// AxisBoth：同一个手势里 X 与 Y 都要滚动，不能被"主轴主导"判据过滤掉。
// ---------------------------------------------------------------------------
TEST_F(PanelGestureTest, diagonal_drag_scrolls_both_axes)
{
    GesturePanel *p = MakePanel(true);
    ASSERT_TRUE(p != NULL);

    Down(p, 60, 60);
    Move(p, 80, 80);  // arm
    Move(p, 100, 100); // dx=20, dy=20
    Up(p, 100, 100);

    EXPECT_GT(p->dragV, 0);
    EXPECT_GT(p->dragH, 0);
}

// ---------------------------------------------------------------------------
// 水平单轴拖动也有效（旧实现是 dx>8 || dy>8，双轴模式必须保留）。
// ---------------------------------------------------------------------------
TEST_F(PanelGestureTest, horizontal_only_drag_scrolls_x)
{
    GesturePanel *p = MakePanel(true);
    ASSERT_TRUE(p != NULL);

    Down(p, 60, 60);
    Move(p, 80, 60);
    Move(p, 100, 60);
    Up(p, 100, 60);

    EXPECT_EQ(0, p->dragV);
    EXPECT_GT(p->dragH, 0);
}

// ---------------------------------------------------------------------------
// WM_CANCELMODE（容器抢手势 / 系统取消）必须清掉拖动状态且不起 fling。
// ---------------------------------------------------------------------------
TEST_F(PanelGestureTest, cancel_mode_aborts_gesture_and_allows_next_one)
{
    GesturePanel *p = MakePanel(true);
    ASSERT_TRUE(p != NULL);

    Down(p, 60, 60);
    Move(p, 60, 80);
    Move(p, 60, 100);
    p->SSendMessage(WM_CANCELMODE, 0, 0);
    EXPECT_EQ(0, p->flings);

    int vAfterCancel = p->dragV;
    Move(p, 60, 120);
    EXPECT_EQ(vAfterCancel, p->dragV) << L"no scroll callbacks after CANCELMODE";

    // The detector must be usable again for the next gesture.
    p->ResetCounters();
    Down(p, 60, 60);
    Move(p, 60, 80);
    Move(p, 60, 100);
    Up(p, 60, 100);
    EXPECT_EQ(1, p->dragStarts);
    EXPECT_GT(p->dragV, 0);
}

// ---------------------------------------------------------------------------
// 没有配对 DOWN 的 UP 不能触发手势收尾（detector 的 m_bDown 守卫）。
// ---------------------------------------------------------------------------
TEST_F(PanelGestureTest, stray_up_is_not_a_gesture)
{
    GesturePanel *p = MakePanel(true);
    ASSERT_TRUE(p != NULL);

    Up(p, 100, 100);
    EXPECT_EQ(1, p->upHits);
    EXPECT_EQ(0, p->dragStarts);
    EXPECT_EQ(0, p->flings);
}

// ---------------------------------------------------------------------------
// 没有滚动条也没有框选的面板不该被 8px 拖动吞掉点击（原有的 arm 门槛）。
// ---------------------------------------------------------------------------
TEST_F(PanelGestureTest, non_scrollable_panel_never_arms_gesture)
{
    GesturePanel *p = MakePanel(false);
    ASSERT_TRUE(p != NULL);

    Down(p, 60, 60);
    Move(p, 60, 120);
    Up(p, 60, 120);

    EXPECT_EQ(0, p->dragStarts);
    EXPECT_EQ(0, p->dragV);
    EXPECT_EQ(1, p->upHits);
}

// ---------------------------------------------------------------------------
// 轴相关否决：纵向滚动面板只在纵向接管时拒绝放弃手势，横向翻页必须让路；
// 反向同理。修复前只要有任意滚动条就无条件否决（旧的无轴向 reason）。
// ---------------------------------------------------------------------------
TEST_F(PanelGestureTest, scroll_veto_is_axis_aware)
{
    GesturePanel *p = MakePanel(false);
    ASSERT_TRUE(p != NULL);

    p->ShowScrollBar(SSB_VERT, TRUE);
    EXPECT_FALSE(p->TryCancel(CANCEL_REASON_SCROLL_V)) << L"vertical drag belongs to this panel";
    EXPECT_TRUE(p->TryCancel(CANCEL_REASON_SCROLL_H)) << L"a vertical list must yield a horizontal page switch";

    p->ShowScrollBar(SSB_VERT, FALSE);
    p->ShowScrollBar(SSB_HORZ, TRUE);
    EXPECT_FALSE(p->TryCancel(CANCEL_REASON_SCROLL_H));
    EXPECT_TRUE(p->TryCancel(CANCEL_REASON_SCROLL_V));

    // 两轴同时置位保持最保守语义：任一条可滚动方向都否决。
    EXPECT_FALSE(p->TryCancel(CANCEL_REASON_SCROLL_H | CANCEL_REASON_SCROLL_V));
}

// ---------------------------------------------------------------------------
// 评审修复回归：启用框选时任意轴都可能起手，而容器在越过阈值的同一次 MOVE
// 的探针阶段就发 CANCEL，此时 m_bRubberBandActive 还没置位，所以不能只按
// 「对应轴有没有滚动条」决定是否否决外层翻页。
// ---------------------------------------------------------------------------
TEST_F(PanelGestureTest, rubber_band_veto_is_axis_independent)
{
    GesturePanel *p = MakePanel(false);
    ASSERT_TRUE(p != NULL);

    p->ShowScrollBar(SSB_VERT, TRUE);
    EXPECT_TRUE(p->TryCancel(CANCEL_REASON_SCROLL_H)) << L"未启用框选：纵向列表让出横向翻页";

    p->SetBandEnabled(TRUE);
    EXPECT_FALSE(p->TryCancel(CANCEL_REASON_SCROLL_H)) << L"框选启用：横向拖动不能被外层抢走";
    EXPECT_FALSE(p->TryCancel(CANCEL_REASON_SCROLL_V));
    EXPECT_FALSE(p->TryCancel(CANCEL_REASON_SCROLL_H | CANCEL_REASON_SCROLL_V));
}

// ---------------------------------------------------------------------------
// SGestureDetector：AxisBoth 任一轴越界即启动，且探针绝不产生回调。
// ---------------------------------------------------------------------------
TEST(soui_panel_gesture_detector, axis_both_probe_accepts_either_axis)
{
    GestureRecorder rec;
    SGestureDetector detector(&rec);
    detector.SetAxis(SGestureDetector::AxisBoth);
    detector.SetTouchSlop(8);

    detector.OnInterceptTouchEvent(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 50));
    EXPECT_EQ(1, rec.down);

    EXPECT_TRUE(detector.OnInterceptTouchEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 61)));
    EXPECT_TRUE(detector.OnInterceptTouchEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(61, 50)));
    EXPECT_TRUE(detector.OnInterceptTouchEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(61, 61)));
    EXPECT_FALSE(detector.OnInterceptTouchEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(55, 55)));
    EXPECT_EQ(0, rec.scroll) << L"OnInterceptTouchEvent is a pure probe";
    EXPECT_FALSE(detector.IsDragging());
}

// ---------------------------------------------------------------------------
// 单轴语义不得被 AxisBoth 改动影响（STabCtrl 依赖"主轴主导"）。
// ---------------------------------------------------------------------------
TEST(soui_panel_gesture_detector, horizontal_axis_still_requires_dominant_x)
{
    GestureRecorder rec;
    SGestureDetector detector(&rec);
    detector.SetAxis(SGestureDetector::AxisHorizontal);
    detector.SetTouchSlop(8);

    detector.OnInterceptTouchEvent(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 50));
    EXPECT_FALSE(detector.OnInterceptTouchEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 70)));
    EXPECT_FALSE(detector.OnInterceptTouchEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(65, 90)));
    EXPECT_TRUE(detector.OnInterceptTouchEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(70, 55)));
}

// ---------------------------------------------------------------------------
// AxisBoth 的连续 OnTouchEvent：scroll 回调带增量，CANCEL 走 cancel 而非 up。
// ---------------------------------------------------------------------------
TEST(soui_panel_gesture_detector, axis_both_touch_stream_and_cancel)
{
    GestureRecorder rec;
    SGestureDetector detector(&rec);
    detector.SetAxis(SGestureDetector::AxisBoth);
    detector.SetTouchSlop(8);

    detector.OnTouchEvent(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 50));
    EXPECT_TRUE(detector.OnTouchEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(70, 70)));
    EXPECT_TRUE(detector.OnTouchEvent(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(90, 90)));
    EXPECT_EQ(2, rec.scroll);
    EXPECT_TRUE(detector.OnTouchEvent(WM_CANCELMODE, 0, 0));
    EXPECT_EQ(1, rec.cancel);
    EXPECT_EQ(0, rec.up);
    EXPECT_FALSE(detector.IsDown());
}
