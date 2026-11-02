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
 * A slot returning TRUE marks the event "handled" (bubble-up = false) and that
 * single fact does two things:
 *    - stops the remaining slots in stage 1  (SEvent::fire breaks, SWnd.cpp:71)
 *    - stops the owner / container tail       (SWindow::FireEvent breaks, SWnd.cpp:2258)
 * A slot returning FALSE leaves bubble-up true, so the event keeps travelling
 * until something handles it or it reaches the container.
 *
 * These two behaviours are the part of the event system that is pure
 * bookkeeping and therefore testable without a UI: this file pins them with
 * executable assertions so a future change to the dispatch order or the
 * interrupt rule has to break a red test on purpose (closes risk 15, event
 * side). It complements test_event_slot_identity.cpp, which pins the slot
 * equality contract and SNotifyCenter; together they cover the event system's
 * pure-bookkeeping surface.
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
    BOOL handledSlot(IEvtArgs *, void *)
    {
        return TRUE; // marks the event handled -> bubble-up stops
    }
    BOOL countAndHandle(IEvtArgs *, void *ctx)
    {
        ++*(int *)ctx;
        return TRUE; // handles -> interrupt
    }
    BOOL countAndIgnore(IEvtArgs *, void *ctx)
    {
        ++*(int *)ctx;
        return FALSE; // does not handle -> event keeps bubbling
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
// Within-stage interrupt: the latest subscriber runs first (SEvent::fire walks
// slots newest-first) and, when it handles the event, earlier subscribers are
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
    FreeFunctionSlot late(countAndHandle, &lateCalls);   // runs FIRST, handles
    ASSERT_TRUE(w.SubscribeEvent(EventCmd::EventID, &early));
    ASSERT_TRUE(w.SubscribeEvent(EventCmd::EventID, &late));

    EventCmd evt(&w);
    EXPECT_TRUE(w.FireEvent(evt));
    EXPECT_EQ(1, lateCalls) << L"latest subscriber runs first";
    EXPECT_EQ(0, earlyCalls) << L"handled by the later slot => earlier skipped (interrupt)";
    EXPECT_EQ(0, container.m_nFiredEvents) << L"handled => no bubble to container";
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
