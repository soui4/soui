/*
 * test_event_dispatch.cpp -- SWindow::FireEvent four-stage dispatch and the
 * bubble-up / interrupt semantics, exercised with a headless container
 * (no host window, no real window tree).
 *
 * SWindow::FireEvent (SOUI/src/core/SWnd.cpp:2246) is a four-stage chain:
 *
 *    1. subscribed C++ slots         (SEventSet::FireEvent -> SEvent::fire)
 *    2. script handler (if a script module is wired to the window)
 *    3. GetOwner()->FireEvent        (bubbling to the owner window)
 *    4. GetContainer()->OnFireEvent  (bubbling to the host container)
 *
 * Stopping the chain is an explicit act: a handler must call
 * SetBubbleUp(false). The EVENT_HANDLER family of macros (SEventCrack.h) call
 * SetBubbleUp(false) *before* invoking the user handler, which is what makes a
 * handler "handle" an event. A slot's return value alone does NOT stop the
 * chain: in SEvent::fire a slot returning TRUE only increments HandleCount, and
 * both the slot-interrupt branch and the owner/container-skip branch are gated
 * purely on IsBubbleUp() (SWnd.cpp:2258). So a bare "return TRUE" free slot
 * leaves bubble-up true and the event keeps travelling until something turns it
 * off or it reaches the container.
 *
 * These four tests pin that pure-bookkeeping contract with a headless container
 * (no host window, no real window tree): the stage-1 interrupt (newest-first
 * slot that turns off bubble-up skips earlier slots AND the container), the
 * full bubble to the container when nothing stops it, and the refuse-to-dispatch
 * precondition. They complement test_event_slot_identity.cpp (slot equality +
 * SNotifyCenter); together they cover the event system's pure-bookkeeping
 * surface.
 *
 * Note on stages 2 and 3: the script-handler stage needs a script module wired
 * to the window and the owner stage needs a real owner window. Both share the
 * very same SWindow::FireEvent code path that stage 1 + stage 4 exercise here,
 * so the container tail proven below is the canary for "the bubble chain runs".
 */

#include <souistd.h>
#include <core/SWnd.h>
#include <event/SEvents.h>
#include <event/SEventSet.h>
#include <gtest/gtest.h>

#include "common.h"
#include "test_container.h"

using namespace SOUI;

namespace
{
    // A free slot "handles" an event the same way a real SOUI event-map handler
    // does: it explicitly turns off bubble-up (EVENT_HANDLER et al. call
    // SetBubbleUp(false) before invoking the user handler). The return value
    // alone would only bump HandleCount and would NOT stop the chain, so without
    // SetBubbleUp(false) the event would keep bubbling to the container.
    BOOL handledSlot(IEvtArgs *e, void *)
    {
        e->SetBubbleUp(false); // stop propagation to owner / container
        return TRUE;           // counts as handled
    }
    BOOL countAndHandle(IEvtArgs *e, void *ctx)
    {
        ++*(int *)ctx;
        e->SetBubbleUp(false); // stops bubble-up -> interrupts earlier subscribers
        return TRUE;           // counts as handled
    }
    BOOL countAndIgnore(IEvtArgs *e, void *ctx)
    {
        ++*(int *)ctx;
        return FALSE; // does not turn off bubble-up -> event keeps travelling
    }
}

// ---------------------------------------------------------------------------
// Interrupt: a handled event never reaches the container (stage 4 is skipped).
// ---------------------------------------------------------------------------
TEST(soui_event_dispatch, handled_event_does_not_bubble_to_container)
{
    HeadlessApp app;
    TestContainer container;
    SWindow w;
    w.SetContainer(&container);

    container.ResetEventCounter();
    FreeFunctionSlot slot(handledSlot, NULL);
    ASSERT_TRUE(w.SubscribeEvent(EventCmd::EventID, &slot));

    EventCmd evt(&w);
    EXPECT_TRUE(w.FireEvent(evt)) << L"handled => FireEvent returns TRUE (HandleCount > 0)";
    EXPECT_EQ(0, container.m_nFiredEvents)
        << L"a handled event must not bubble to the container (interrupt)";
}

// ---------------------------------------------------------------------------
// Bubble: an unhandled event travels all the way to the container tail.
// ---------------------------------------------------------------------------
TEST(soui_event_dispatch, unhandled_event_bubbles_to_container)
{
    HeadlessApp app;
    TestContainer container;
    SWindow w;
    w.SetContainer(&container);

    container.ResetEventCounter();
    int slotCalls = 0;
    FreeFunctionSlot slot(countAndIgnore, &slotCalls);
    ASSERT_TRUE(w.SubscribeEvent(EventCmd::EventID, &slot));

    EventCmd evt(&w);
    EXPECT_FALSE(w.FireEvent(evt))
        << L"container OnFireEvent returns FALSE => whole dispatch returns FALSE";
    EXPECT_EQ(1, slotCalls) << L"stage 1 (subscription slot) must have run";
    EXPECT_EQ(1, container.m_nFiredEvents)
        << L"unhandled event must bubble to the container (stage 4)";
}

// ---------------------------------------------------------------------------
// Within-stage interrupt: SEvent::fire walks slots newest-first, so the latest
// subscriber runs first. When it turns off bubble-up, earlier subscribers are
// skipped and the container is never reached.
// ---------------------------------------------------------------------------
TEST(soui_event_dispatch, later_subscriber_interrupts_earlier)
{
    HeadlessApp app;
    TestContainer container;
    SWindow w;
    w.SetContainer(&container);

    container.ResetEventCounter();
    int earlyCalls = 0, lateCalls = 0;
    FreeFunctionSlot early(countAndIgnore, &earlyCalls); // runs AFTER late
    FreeFunctionSlot late(countAndHandle, &lateCalls);   // runs FIRST, turns off bubble-up
    ASSERT_TRUE(w.SubscribeEvent(EventCmd::EventID, &early));
    ASSERT_TRUE(w.SubscribeEvent(EventCmd::EventID, &late));

    EventCmd evt(&w);
    EXPECT_TRUE(w.FireEvent(evt));
    EXPECT_EQ(1, lateCalls) << L"latest subscriber runs first";
    EXPECT_EQ(0, earlyCalls) << L"later slot turned off bubble-up => earlier skipped (interrupt)";
    EXPECT_EQ(0, container.m_nFiredEvents) << L"bubble-up off => no bubble to container";
}

// ---------------------------------------------------------------------------
// Precondition: a window with no container refuses to dispatch at all, so the
// four-stage chain never starts. Pins the GetContainer() guard at SWnd.cpp:2249.
// ---------------------------------------------------------------------------
TEST(soui_event_dispatch, no_container_refuses_to_dispatch)
{
    HeadlessApp app;
    SWindow orphan;
    ASSERT_TRUE(orphan.GetContainer() == NULL);

    int calls = 0;
    FreeFunctionSlot slot(countAndIgnore, &calls);
    ASSERT_TRUE(orphan.SubscribeEvent(EventCmd::EventID, &slot));

    EventCmd evt(&orphan);
    EXPECT_FALSE(orphan.FireEvent(evt));
    EXPECT_EQ(0, calls) << L"dispatch is refused before any slot is consulted";
}
