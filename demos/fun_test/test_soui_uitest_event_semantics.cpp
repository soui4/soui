/*
 * test_soui_uitest_event_semantics.cpp -- event dispatch semantics that the
 * behaviour-level cases in test_soui_uitest_events.cpp do not pin down.
 *
 * The merged event suite covers the observable "what happened" side: a click
 * fires MouseClick then Command, a stopper hides later subscribers, the owner
 * chain forwards, mute suppresses. This file complements it with the rules
 * that are easy to break while refactoring and invisible from the outside:
 *
 *   1. FireEvent's return value is HandleCount() > 0 -- not "some slot ran"
 *      and not "a slot returned TRUE". Handled and consumed are independent.
 *   2. HandleCount is bumped once per *handling* slot, so a window with three
 *      handlers answers 3 while still falling through to the container.
 *   3. The parent chain is not a dispatch path: a child without an owner
 *      reaches the container directly, and its parent sees nothing.
 *   4. The container is the last resort for *any* event, including one the
 *      window never registered in its own event set (no slot can subscribe to
 *      it, but the container still gets it). This is what makes a plain
 *      `<window>` able to observe control events it did not declare.
 *   5. SetEventMute is a counter: nested mute needs the same number of
 *      unmutes. A boolean would unmute early on the first release.
 *
 * Source of truth: SOUI/src/core/SWnd.cpp:2246 (FireEvent),
 * SOUI/src/event/SEventSet.cpp:54 (SEvent::fire), :110 (setMutedState).
 */

#include "test_soui_uitest_common.h"

using namespace SOUI;
using SouiTest::findByName;

namespace
{
    /// Free-function probe: FreeFunctionSlot::Equal compares (function, ctx),
    /// so several probes can coexist on one event -- unlike std::function slots.
    struct Probe
    {
        std::vector<std::string> *log;
        const char *tag;
        BOOL handled;
        int stopBubble; ///< -1 = leave alone, 0 = SetBubbleUp(FALSE)

        MemberFunctionSlot<Probe, IEvtArgs> slot;

        Probe(std::vector<std::string> *l, const char *t, BOOL h, int stop = -1)
            : log(l)
            , tag(t)
            , handled(h)
            , stopBubble(stop)
            , slot(Subscriber(&Probe::onEvent, this))
        {
        }

        BOOL onEvent(IEvtArgs *e)
        {
            if (log)
                log->push_back(tag);
            if (stopBubble >= 0)
                e->SetBubbleUp(stopBubble ? TRUE : FALSE);
            return handled;
        }
    };

    /// Free-function slot used where C++11 would allow an inline lambda.
    BOOL alwaysTrueSlot(IEvtArgs *, void *)
    {
        return TRUE;
    }
} // namespace

// ---------------------------------------------------------------------------
// Return value == HandleCount() > 0
// ---------------------------------------------------------------------------
TEST_F(SouiTestFixture, event_return_value_is_handle_count_not_slot_result)
{
    SWindow *root = loadUi(L"<window><window name='w' width='100' height='30'/></window>");
    SWindow *w = findByName<SWindow>(root, L"w");
    ASSERT_TRUE(w != NULL);

    // Handled + consumed => TRUE, and exactly one handler counted.
    {
        std::vector<std::string> log;
        Probe p(&log, "handled", TRUE, /*stop=*/0);
        ASSERT_TRUE(w->SubscribeEvent(EventCmd::EventID, &p.slot));

        container()->ResetEventCounter();
        EventCmd evt(w);
        EXPECT_TRUE(w->FireEvent(evt));
        EXPECT_EQ(1u, evt.HandleCount());
        EXPECT_EQ(0, container()->m_nFiredEvents);
        ASSERT_TRUE(w->UnsubscribeEvent(EventCmd::EventID, &p.slot));
    }

    // Not handled + consumed => FALSE even though a slot did run and it chose
    // to stop propagation. "Swallowed" without handling is not "handled".
    {
        std::vector<std::string> log;
        Probe p(&log, "swallow", FALSE, /*stop=*/0);
        ASSERT_TRUE(w->SubscribeEvent(EventCmd::EventID, &p.slot));

        container()->ResetEventCounter();
        EventCmd evt(w);
        EXPECT_FALSE(w->FireEvent(evt));
        EXPECT_EQ(0u, evt.HandleCount());
        EXPECT_EQ(1u, log.size()) << L"the slot did run; it just did not claim the event";
        EXPECT_EQ(0, container()->m_nFiredEvents);
        ASSERT_TRUE(w->UnsubscribeEvent(EventCmd::EventID, &p.slot));
    }
}

TEST_F(SouiTestFixture, event_handle_count_counts_every_handling_slot)
{
    SWindow *root = loadUi(L"<window><window name='w' width='100' height='30'/></window>");
    SWindow *w = findByName<SWindow>(root, L"w");
    ASSERT_TRUE(w != NULL);

    std::vector<std::string> log;
    Probe first(&log, "first", TRUE);
    Probe second(&log, "second", TRUE);
    Probe third(&log, "third", TRUE);
    ASSERT_TRUE(w->SubscribeEvent(EventCmd::EventID, &first.slot));
    ASSERT_TRUE(w->SubscribeEvent(EventCmd::EventID, &second.slot));
    ASSERT_TRUE(w->SubscribeEvent(EventCmd::EventID, &third.slot));

    container()->ResetEventCounter();
    EventCmd evt(w);
    // The container always answers FALSE, and it is reached because nobody
    // stopped propagation -- so the caller sees FALSE despite three handlers.
    EXPECT_FALSE(w->FireEvent(evt));
    EXPECT_EQ(3u, evt.HandleCount());
    EXPECT_EQ(1, container()->m_nFiredEvents);

    ASSERT_EQ(3u, log.size());
    EXPECT_EQ("third", log[0]) << L"reverse subscription order";
    EXPECT_EQ("second", log[1]);
    EXPECT_EQ("first", log[2]);
}

// ---------------------------------------------------------------------------
// Parent chain is not a dispatch path
// ---------------------------------------------------------------------------
TEST_F(SouiTestFixture, event_child_without_owner_never_reaches_parent)
{
    SWindow *root = loadUi(
        L"<window>"
        L"  <window name='parent' pos='0,0' width='200' height='100'>"
        L"    <window name='child' pos='0,0' width='50' height='20'/>"
        L"  </window>"
        L"</window>");
    SWindow *parent = findByName<SWindow>(root, L"parent");
    SWindow *child = findByName<SWindow>(root, L"child");
    ASSERT_TRUE(parent != NULL);
    ASSERT_TRUE(child != NULL);
    ASSERT_TRUE(child->GetOwner() == NULL);

    SouiTest::SignalSpy parentSpy(parent, EventCmd::EventID);
    SouiTest::SignalSpy childSpy(child, EventCmd::EventID);

    container()->ResetEventCounter();
    EventCmd evt(child);
    child->FireEvent(evt);

    EXPECT_EQ(1, childSpy.count());
    EXPECT_EQ(0, parentSpy.count()) << L"no parent bubbling in SOUI: only the owner chain exists";
    EXPECT_EQ(1, container()->m_nFiredEvents) << L"the container is reached directly, not through the parent";
}

// ---------------------------------------------------------------------------
// The container sees events the window never registered
// ---------------------------------------------------------------------------
TEST_F(SouiTestFixture, event_container_sees_events_absent_from_window_event_set)
{
    SWindow *root = loadUi(L"<window><window name='plain' width='100' height='30'/></window>");
    SWindow *plain = findByName<SWindow>(root, L"plain");
    ASSERT_TRUE(plain != NULL);

    // A control-only event: SWindow does not declare it, so no slot can attach...
    FreeFunctionSlot slot(alwaysTrueSlot, NULL);
    EXPECT_FALSE(plain->SubscribeEvent(EventLBSelChanged::EventID, &slot));

    // ...but dispatch still falls through to the container.
    container()->ResetEventCounter();
    EventLBSelChanged evt(plain);
    evt.nNewSel = 0;
    plain->FireEvent(evt);
    EXPECT_EQ(1, container()->m_nFiredEvents);
    EXPECT_EQ((DWORD)EventLBSelChanged::EventID, container()->m_nLastEventId);
}

// ---------------------------------------------------------------------------
// Mute is a counter
// ---------------------------------------------------------------------------
TEST_F(SouiTestFixture, event_mute_is_reference_counted)
{
    SWindow *root = loadUi(L"<window><window name='w' width='100' height='30'/></window>");
    SWindow *w = findByName<SWindow>(root, L"w");
    ASSERT_TRUE(w != NULL);

    SouiTest::SignalSpy spy(w, EventCmd::EventID);
    container()->ResetEventCounter();

    w->SetEventMute(TRUE);
    w->SetEventMute(TRUE); // nested: two levels held
    w->FireCommand();
    EXPECT_EQ(0, spy.count());
    EXPECT_EQ(0, container()->m_nFiredEvents) << L"muted at the FireEvent entry: nothing dispatches";

    w->SetEventMute(FALSE); // one level released, still muted
    w->FireCommand();
    EXPECT_EQ(0, spy.count()) << L"a boolean flag would already be unmuted here";
    EXPECT_EQ(0, container()->m_nFiredEvents);

    w->SetEventMute(FALSE); // fully released
    w->FireCommand();
    EXPECT_EQ(1, spy.count());
    EXPECT_EQ(1, container()->m_nFiredEvents);
}
