/**
 * @file  test_soui_uitest_events.cpp
 * @brief 基于 SouiTest 的 SOUI 事件体系无头单元测试（Tier A）
 *
 * 覆盖 SWindow 事件机制的几条主干语义（依据 SOUI/src/core/Swnd.cpp + src/event/SEventSet.cpp）：
 *   1. 输入 -> 事件：WM_LBUTTONDOWN/UP 触发 EventMouseClick（clickId 区分按下/抬起），
 *      抬起落在窗口内再触发 EventCmd；WM_KEYDOWN/UP/CHAR 触发 EventKeyDown/Up/Char。
 *   2. 派发链：本窗口事件槽 -> 脚本处理器 -> owner 窗口 -> 容器 OnFireEvent。
 *   3. 冒泡控制：SEvent::fire 按【后订阅先执行】遍历；处理器返回 TRUE 且把
 *      SetBubbleUp(FALSE) 后，更早的订阅者与容器都不再收到该事件。
 *   4. 订阅生命周期：事件 ID 必须已在该窗口的事件集合中注册（不同控件注册各自的事件）；
 *      UnsubscribeEvent 后不再回调；SetEventMute(TRUE) 时 FireEvent 直接返回。
 *   5. 与布局协作：OnRelayout 触发 EventSwndPos 与 EventSwndSize，负载即新矩形/客户区尺寸。
 *
 * 说明：断言"负载内容"与"发送者身份"时使用 DataSpy / ArgsSpy 而非 SignalSpy::lastArgs()，
 * 因为事件对象是 FireEvent 调用方栈上的临时对象，返回后即析构。
 */
#include "test_soui_uitest_common.h"

#include <string>
#include <vector>

using namespace SOUI;

namespace
{
/**
 * @brief 可编程监听器：记录调用顺序，可选地在处理后将事件标记为"不再冒泡"。
 * @note  同时暴露 subscribed，用于断言 SubscribeEvent 是否成功（事件未注册时返回 FALSE）。
 */
class Probe
{
  public:
    Probe(SWindow *wnd, DWORD eventId, std::vector<std::string> *log, const char *tag, bool stopBubble = false)
        : subscribed(false)
        , m_wnd(wnd)
        , m_id(eventId)
        , m_log(log)
        , m_tag(tag)
        , m_stopBubble(stopBubble)
        , m_slot(Subscriber(&Probe::onEvent, this))
    {
        subscribed = m_wnd->SubscribeEvent(m_id, &m_slot) != FALSE;
    }

    ~Probe() { m_wnd->UnsubscribeEvent(m_id, &m_slot); }

    bool subscribed;

  private:
    BOOL onEvent(IEvtArgs *e)
    {
        m_log->push_back(m_tag);
        if (m_stopBubble)
            e->SetBubbleUp(FALSE);
        return TRUE;
    }

    SWindow *m_wnd;
    DWORD m_id;
    std::vector<std::string> *m_log;
    std::string m_tag;
    bool m_stopBubble;
    MemberFunctionSlot<Probe, IEvtArgs> m_slot;
};

// 最小订阅者：只计数，用于验证手动 SubscribeEvent / UnsubscribeEvent 的返回值语义。
struct Counter
{
    int n;

    Counter()
        : n(0)
    {
    }

    BOOL onEvent(IEvtArgs *)
    {
        ++n;
        return TRUE;
    }
};
} // namespace

// ---------------------------------------------------------------------------
// 1. 输入 -> 事件
// ---------------------------------------------------------------------------
// 一次点击在框架层是 DOWN + UP 两条 WM_，各自产生一条 EventMouseClick，
// clickId 与注入点的坐标都被原样带出。
TEST_F(SouiTestFixture, event_mouse_click_reports_kind_and_point)
{
    SWindow *root = loadUi(
        L"<window layout='vbox'>"
        L"  <button name='btn' text='OK' width='100' height='30'/>"
        L"</window>");
    SWindow *btn = SouiTest::findByName<SWindow>(root, L"btn");
    ASSERT_TRUE(btn != NULL);

    SouiTest::DataSpy<StEventMouseClick> spy(btn, EventMouseClick::EventID);
    CPoint pt = SouiTest::centerOf(btn);
    SouiTest::mouseClick(btn, pt);

    ASSERT_EQ(spy.count(), 2); // DOWN + UP
    EXPECT_EQ(spy.at(0).clickId, MOUSE_LBTN_DOWN);
    EXPECT_EQ(spy.at(1).clickId, MOUSE_LBTN_UP);
    EXPECT_EQ(spy.at(0).pt.x, pt.x);
    EXPECT_EQ(spy.at(0).pt.y, pt.y);
}

// 点击的最终产物是 EventCmd：一次点击一条，且发生在抬起之后。
TEST_F(SouiTestFixture, event_click_fires_mouse_click_then_command)
{
    SWindow *root = loadUi(
        L"<window layout='vbox'>"
        L"  <button name='btn' text='OK' width='100' height='30'/>"
        L"</window>");
    SWindow *btn = SouiTest::findByName<SWindow>(root, L"btn");
    ASSERT_TRUE(btn != NULL);

    SouiTest::DataSpy<StEventMouseClick> mc(btn, EventMouseClick::EventID);
    SouiTest::SignalSpy cmd(btn, EventCmd::EventID);
    SouiTest::mouseClick(btn);

    EXPECT_EQ(mc.count(), 2);
    EXPECT_EQ(cmd.count(), 1);
}

// EventCmd 的事件参数携带发送者身份（sender / id / name）。
TEST_F(SouiTestFixture, event_command_carries_sender_identity)
{
    SWindow *root = loadUi(
        L"<window layout='vbox'>"
        L"  <button name='btn' id='1001' text='OK' width='100' height='30'/>"
        L"</window>");
    SWindow *btn = SouiTest::findByName<SWindow>(root, L"btn");
    ASSERT_TRUE(btn != NULL);

    SouiTest::ArgsSpy spy(btn, EventCmd::EventID);
    SouiTest::mouseClick(btn);

    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.last().sender, static_cast<IObject *>(btn));
    EXPECT_EQ(spy.last().idFrom, 1001);
    EXPECT_STREQ(spy.last().nameFrom.c_str(), L"btn");
    // 快照取在回调入口：此时该事件还没有任何处理器计过数
    EXPECT_EQ(spy.last().handled, 0u);
}

// 键盘消息 -> EventKeyDown / EventKeyUp / EventChar（nChar 原样传递）。
TEST_F(SouiTestFixture, event_keyboard_pipeline_keydown_keyup_char)
{
    SWindow *root = loadUi(L"<window><window name='k' width='100' height='30'/></window>");
    SWindow *k = SouiTest::findByName<SWindow>(root, L"k");
    ASSERT_TRUE(k != NULL);

    SouiTest::DataSpy<StEventKeyDown> kd(k, EventKeyDown::EventID);
    SouiTest::DataSpy<StEventKeyUp> ku(k, EventKeyUp::EventID);
    SouiTest::DataSpy<StEventChar> ch(k, EventChar::EventID);

    SouiTest::keyClick(k, 'A');

    ASSERT_EQ(kd.count(), 1);
    ASSERT_EQ(ku.count(), 1);
    ASSERT_EQ(ch.count(), 1);
    EXPECT_EQ(kd.at(0).nChar, (UINT)'A');
    EXPECT_EQ(ku.at(0).nChar, (UINT)'A');
    EXPECT_EQ(ch.at(0).nChar, (UINT)'A');
}

// ---------------------------------------------------------------------------
// 2. 冒泡控制（SEvent::fire 后订阅先执行 + SetBubbleUp(FALSE) 中断）
// ---------------------------------------------------------------------------
// 后订阅的处理器先执行；它把 bubbleUp 置 FALSE 后，更早的订阅者收不到该事件。
TEST_F(SouiTestFixture, event_bubbleup_false_stops_remaining_subscribers)
{
    SWindow *root = loadUi(
        L"<window layout='vbox'>"
        L"  <button name='btn' text='OK' width='100' height='30'/>"
        L"</window>");
    SWindow *btn = SouiTest::findByName<SWindow>(root, L"btn");
    ASSERT_TRUE(btn != NULL);

    std::vector<std::string> log;
    Probe first(btn, EventCmd::EventID, &log, "first");
    Probe second(btn, EventCmd::EventID, &log, "second", /*stopBubble=*/true);
    ASSERT_TRUE(first.subscribed);
    ASSERT_TRUE(second.subscribed);

    btn->FireCommand();

    ASSERT_EQ(log.size(), 1u);
    EXPECT_EQ(log[0], "second");
}

// 默认 bubbleUp=TRUE：事件在窗口处理完之后仍会送达容器层。
TEST_F(SouiTestFixture, event_bubbleup_true_reaches_container)
{
    SWindow *root = loadUi(
        L"<window layout='vbox'>"
        L"  <button name='btn' text='OK' width='100' height='30'/>"
        L"</window>");
    SWindow *btn = SouiTest::findByName<SWindow>(root, L"btn");
    ASSERT_TRUE(btn != NULL);

    container()->ResetEventCounter();
    btn->FireCommand();

    EXPECT_EQ(container()->m_nFiredEvents, 1);
    EXPECT_EQ(container()->m_nLastEventId, (DWORD)EventCmd::EventID);
}

// 处理器中断冒泡后，容器层同样收不到该事件（FireEvent 在 evt->IsBubbleUp() 为假时提前 break）。
TEST_F(SouiTestFixture, event_bubbleup_false_blocks_container)
{
    SWindow *root = loadUi(
        L"<window layout='vbox'>"
        L"  <button name='btn' text='OK' width='100' height='30'/>"
        L"</window>");
    SWindow *btn = SouiTest::findByName<SWindow>(root, L"btn");
    ASSERT_TRUE(btn != NULL);

    std::vector<std::string> log;
    Probe stopper(btn, EventCmd::EventID, &log, "stopper", /*stopBubble=*/true);
    ASSERT_TRUE(stopper.subscribed);

    container()->ResetEventCounter();
    btn->FireCommand();

    EXPECT_EQ(log.size(), 1u);
    EXPECT_EQ(container()->m_nFiredEvents, 0); // 事件没有走到容器层
}

// owner 链：内部子控件（如组合框的下拉列表）通过 SetOwner 把事件转交给宿主控件。
// 普通控件的 owner 为空，事件不会自动冒泡给父控件。
TEST_F(SouiTestFixture, event_owner_chain_forwards_to_owner)
{
    SWindow *root = loadUi(
        L"<window>"
        L"  <window name='owner' pos='0,0' width='200' height='100'>"
        L"    <window name='child' pos='0,0' width='50' height='20'/>"
        L"  </window>"
        L"</window>");
    SWindow *owner = SouiTest::findByName<SWindow>(root, L"owner");
    SWindow *child = SouiTest::findByName<SWindow>(root, L"child");
    ASSERT_TRUE(owner != NULL);
    ASSERT_TRUE(child != NULL);

    // 默认没有 owner：父子关系不等于事件转发关系
    EXPECT_TRUE(child->GetOwner() == NULL);

    SouiTest::SignalSpy ownerSpy(owner, EventCmd::EventID);
    SouiTest::SignalSpy childSpy(child, EventCmd::EventID);

    child->SetOwner(owner);
    child->FireCommand();

    EXPECT_EQ(childSpy.count(), 1); // 自身处理器
    EXPECT_EQ(ownerSpy.count(), 1); // 转交给 owner
}

// ---------------------------------------------------------------------------
// 3. 订阅生命周期
// ---------------------------------------------------------------------------
// 事件 ID 必须已在该窗口的事件集合里注册：SWindow 构造只注册基础事件，
// 控件专有事件（如 EVT_CB_SELCHANGE）由控件自己 addEvent，否则订阅直接失败。
TEST_F(SouiTestFixture, event_subscribe_requires_registered_event)
{
    SWindow *root = loadUi(
        L"<window layout='vbox'>"
        L"  <window name='plain' width='100' height='30'/>"
        L"</window>");
    SWindow *plain = SouiTest::findByName<SWindow>(root, L"plain");
    ASSERT_TRUE(plain != NULL);

    std::vector<std::string> log;
    Probe base(plain, EventCmd::EventID, &log, "base");                    // 基础事件
    Probe comboSel(plain, EventCBSelChange::EventID, &log, "combo_sel");   // 组合框专有事件
    EXPECT_TRUE(base.subscribed);
    EXPECT_FALSE(comboSel.subscribed);
}

// UnsubscribeEvent 精确摘除对应 (函数, 对象) 的订阅；重复摘除返回 FALSE。
TEST_F(SouiTestFixture, event_unsubscribe_detaches_listener)
{
    SWindow *root = loadUi(
        L"<window layout='vbox'>"
        L"  <button name='btn' text='OK' width='100' height='30'/>"
        L"</window>");
    SWindow *btn = SouiTest::findByName<SWindow>(root, L"btn");
    ASSERT_TRUE(btn != NULL);

    Counter counter;
    MemberFunctionSlot<Counter, IEvtArgs> slot(Subscriber(&Counter::onEvent, &counter));

    ASSERT_TRUE(btn->SubscribeEvent(EventCmd::EventID, &slot));
    btn->FireCommand();
    EXPECT_EQ(counter.n, 1);

    ASSERT_TRUE(btn->UnsubscribeEvent(EventCmd::EventID, &slot));
    btn->FireCommand();
    EXPECT_EQ(counter.n, 1); // 已摘除，不再回调

    EXPECT_FALSE(btn->UnsubscribeEvent(EventCmd::EventID, &slot)); // 重复摘除失败
}

// SetEventMute(TRUE)：FireEvent 在入口直接返回，事件完全不派发。
TEST_F(SouiTestFixture, event_mute_suppresses_dispatch)
{
    SWindow *root = loadUi(
        L"<window layout='vbox'>"
        L"  <button name='btn' text='OK' width='100' height='30'/>"
        L"</window>");
    SWindow *btn = SouiTest::findByName<SWindow>(root, L"btn");
    ASSERT_TRUE(btn != NULL);

    SouiTest::SignalSpy spy(btn, EventCmd::EventID);

    btn->SetEventMute(TRUE);
    btn->FireCommand();
    EXPECT_EQ(spy.count(), 0);

    btn->SetEventMute(FALSE);
    btn->FireCommand();
    EXPECT_EQ(spy.count(), 1);
}

// ---------------------------------------------------------------------------
// 4. 布局与事件的协作
// ---------------------------------------------------------------------------
// OnRelayout 会依次抛出 EventSwndPos（新窗口矩形）与 EventSwndSize（新客户区尺寸）。
TEST_F(SouiTestFixture, event_relayout_fires_pos_and_size)
{
    SWindow *root = loadUi(
        L"<window layout='vbox'>"
        L"  <window name='a' width='-2' height='-2'/>"
        L"</window>");

    SouiTest::DataSpy<StEventSwndPos> pos(root, EventSwndPos::EventID);
    SouiTest::DataSpy<StEventSwndSize> size(root, EventSwndSize::EventID);

    root->OnRelayout(CRect(0, 0, 400, 300));

    ASSERT_GE(pos.count(), 1);
    EXPECT_EQ(pos.at(pos.count() - 1).rcWnd.right, 400);
    EXPECT_EQ(pos.at(pos.count() - 1).rcWnd.bottom, 300);

    ASSERT_GE(size.count(), 1);
    EXPECT_EQ(size.at(size.count() - 1).szWnd.cx, 400);
    EXPECT_EQ(size.at(size.count() - 1).szWnd.cy, 300);
}

// ---------------------------------------------------------------------------
// 5. Spy 生命周期（回归：宿主先销毁、Spy 后析构）
// ---------------------------------------------------------------------------
// 三个 Spy 都只保存 SWindow*。若宿主控件先被销毁而 Spy 后析构，直接
// m_wnd->UnsubscribeEvent(...) 就是对已释放对象解引用（曾使 fun_test 在
// SignalSpy::~SignalSpy 处 SEGFAULT）。现在 Spy 构造时快照 SWND，析构时用
// SWindowMgr::GetWindow(swnd) == m_wnd 判活，宿主已销毁则跳过退订。
TEST_F(SouiTestFixture, event_spy_survives_host_destroyed_first)
{
    SWindow *root = loadUi(
        L"<window layout='vbox'>"
        L"  <button name='btn' text='OK' width='100' height='30'/>"
        L"</window>");
    SWindow *btn = SouiTest::findByName<SWindow>(root, L"btn");
    ASSERT_TRUE(btn != NULL);

    int nCmd = 0;
    {
        SouiTest::SignalSpy spy(btn, EventCmd::EventID);
        SouiTest::DataSpy<StEventMouseClick> mouseSpy(btn, EventMouseClick::EventID);
        SouiTest::ArgsSpy argsSpy(btn, EventCmd::EventID);

        btn->FireCommand();
        nCmd = spy.count();
        ASSERT_EQ(nCmd, 1);
        EXPECT_EQ(argsSpy.count(), 1);
        const int idFrom = btn->GetID(); // 销毁后不能再读宿主，先取快照
        const SWND swndBtn = btn->GetSwnd();

        // 宿主被销毁，三个 Spy 此刻仍然存活（m_wnd 已成悬垂指针）。
        root->DestroyAllChildren();

        // 自校验：确认宿主确实已释放（句柄已从 SWindowMgr 注销）。
        // 若这里仍非空，说明用例根本没触发"宿主先死"的场景，后面就是在空跑。
        EXPECT_TRUE(SWindowMgr::GetWindow(swndBtn) == NULL);

        // 销毁过程不应再产生 EventCmd / EventMouseClick。
        EXPECT_EQ(spy.count(), nCmd);
        EXPECT_EQ(mouseSpy.count(), 0);
        EXPECT_EQ(argsSpy.count(), 1);
        EXPECT_EQ(argsSpy.last().idFrom, idFrom);
    } // <-- 三个 Spy 在此析构：宿主已死，必须安全退订（曾在此崩溃）
}
