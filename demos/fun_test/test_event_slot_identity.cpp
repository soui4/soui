/*
 * test_event_slot_identity.cpp -- the subscription contract and the global
 * event map, tested without a control tree.
 *
 * Two things live here because they are pure bookkeeping, not UI behaviour:
 *
 *   1. IEvtSlot::Equal decides whether a second subscribe() is a duplicate and
 *      which slot unsubscribe() removes. Each slot type answers differently,
 *      and one of those answers is a documented limitation:
 *
 *        FreeFunctionSlot   : equal iff same function AND same ctx
 *        MemberFunctionSlot : equal iff same member AND same object
 *        StdFunctionSlot    : ALWAYS equal to any other std::function slot
 *
 *      The last one means "one std::function subscriber per event per window".
 *      It is easy to read that comment and move on; the case below makes the
 *      consequence executable, so a future change either keeps it or has to
 *      update a red test on purpose.
 *
 *   2. SNotifyCenter is the process-wide handler list used by
 *      TAutoEventMapReg. Nothing in the gate covered its register/unregister
 *      pairing or RunOnUISync, although both are used by every component that
 *      opts into the notify center.
 */

#include <souistd.h>
#include <core/SWnd.h>
#include <event/SEvents.h>
#include <event/SEventSet.h>
#include <event/SNotifyCenter.h>
#include <gtest/gtest.h>

#include "common.h"
#include "test_container.h"

using namespace SOUI;

namespace
{
    BOOL alwaysTrue(IEvtArgs *, void *)
    {
        return TRUE;
    }
    BOOL alwaysFalse(IEvtArgs *, void *)
    {
        return FALSE;
    }

    /// A plain receiver used to prove MemberFunctionSlot compares the object.
    struct Receiver
    {
        int hits;

        Receiver()
            : hits(0)
        {
        }

        BOOL onEvent(IEvtArgs *)
        {
            hits++;
            return TRUE;
        }
    };

    /// Free function slot that counts calls through its context pointer.
    /// C++03 has no lambdas, and the test must stay compilable by VS2008.
    BOOL countCalls(IEvtArgs *, void *ctx)
    {
        ++*(int *)ctx;
        return TRUE;
    }

#if ENABLE_STDFUNCTOR
    /// std::function-compatible functor used where a lambda would be natural.
    struct CountFunctor
    {
        int *pCount;
        BOOL ret;

        CountFunctor(int *p, BOOL r)
            : pCount(p)
            , ret(r)
        {
        }

        BOOL operator()(IEvtArgs *) const
        {
            ++*pCount;
            return ret;
        }
    };

    /// RunOnUISync 的闭包：C++03 下用函数对象代替 lambda。
    struct SetFlag
    {
        int *pFlag;

        explicit SetFlag(int *p)
            : pFlag(p)
        {
        }

        void operator()() const { *pFlag = 1; }
    };
#endif // ENABLE_STDFUNCTOR
} // namespace

// ---------------------------------------------------------------------------
// A window that is not attached to a container refuses to dispatch at all.
// ---------------------------------------------------------------------------
// FireEvent() returns before the slot loop when GetContainer() is null: the
// container is the dispatch root, not an optional extra stage. Tested here
// because every fixture in the suite already has a container.
TEST(soui_event_slot, without_container_no_slot_runs)
{
    HeadlessApp app; // SWindow construction needs the app-level singletons
    SWindow orphan;
    ASSERT_TRUE(orphan.GetContainer() == NULL);

    int calls = 0;
    FreeFunctionSlot slot(countCalls, &calls);
    ASSERT_TRUE(orphan.SubscribeEvent(EventCmd::EventID, &slot));

    EventCmd evt(&orphan);
    EXPECT_FALSE(orphan.FireEvent(evt));
    EXPECT_EQ(0, calls) << L"the refusal happens before any slot is consulted";
}

// ---------------------------------------------------------------------------
// std::function slots are indistinguishable -> one per event per window
// ---------------------------------------------------------------------------
#if ENABLE_STDFUNCTOR
TEST(soui_event_slot, std_function_slots_are_indistinguishable)
{
    int a = 0, b = 0;
    StdFunctionSlot s1(CountFunctor(&a, TRUE));
    StdFunctionSlot s2(CountFunctor(&b, FALSE));

    EXPECT_EQ((UINT)SLOT_STDFUNCTOR, s1.GetSlotType());
    EXPECT_TRUE(s1.Equal(&s2))
        << L"documented limitation: std::function has no equality, so Equal() answers TRUE";

    HeadlessApp app;
    SWindow w;
    EXPECT_TRUE(w.SubscribeEvent(EventSwndSize::EventID, &s1));
    EXPECT_FALSE(w.SubscribeEvent(EventSwndSize::EventID, &s2))
        << L"consequence: the second std::function subscriber is rejected as a duplicate";
}

TEST(soui_event_slot, std_function_slot_rejects_other_slot_types)
{
    int a = 0;
    StdFunctionSlot stdSlot(CountFunctor(&a, TRUE));
    FreeFunctionSlot freeSlot(alwaysTrue, NULL);

    EXPECT_FALSE(stdSlot.Equal(&freeSlot)) << L"type must be checked before the always-TRUE shortcut";
    EXPECT_FALSE(freeSlot.Equal(&stdSlot));
}
#endif // ENABLE_STDFUNCTOR

// ---------------------------------------------------------------------------
// Free-function / member-function slots compare by identity, not by type
// ---------------------------------------------------------------------------
TEST(soui_event_slot, free_function_slots_compare_by_function_and_context)
{
    int ctx1 = 1, ctx2 = 2;
    FreeFunctionSlot s1(alwaysTrue, &ctx1);
    FreeFunctionSlot s2(alwaysTrue, &ctx2);
    FreeFunctionSlot s3(alwaysFalse, &ctx1);

    EXPECT_FALSE(s1.Equal(&s2)) << L"same target, different ctx => different subscriber";
    EXPECT_FALSE(s1.Equal(&s3)) << L"same ctx, different target => different subscriber";
    EXPECT_TRUE(s1.Equal(&s1));
}

TEST(soui_event_slot, member_function_slots_compare_by_object)
{
    Receiver r1, r2;
    MemberFunctionSlot<Receiver, IEvtArgs> s1(Subscriber(&Receiver::onEvent, &r1));
    MemberFunctionSlot<Receiver, IEvtArgs> s2(Subscriber(&Receiver::onEvent, &r2));

    EXPECT_FALSE(s1.Equal(&s2)) << L"same member on a different object => different subscriber";
    EXPECT_TRUE(s1.Equal(&s1));
}

// ---------------------------------------------------------------------------
// SNotifyCenter: global handler map + UI-thread handoff
// ---------------------------------------------------------------------------
TEST(soui_event_notify_center, event_map_registration_dedupes_and_unregisters)
{
    HeadlessApp app;
    app.app->EnableNotifyCenter(TRUE);
    SNotifyCenter *nc = SNotifyCenter::getSingletonPtr();
    ASSERT_TRUE(nc != NULL);

    Receiver r1, r2;
    MemberFunctionSlot<Receiver, IEvtArgs> slot1(Subscriber(&Receiver::onEvent, &r1));
    MemberFunctionSlot<Receiver, IEvtArgs> slot2(Subscriber(&Receiver::onEvent, &r2));

    EXPECT_TRUE(nc->RegisterEventMap(&slot1));
    EXPECT_FALSE(nc->RegisterEventMap(&slot1)) << L"the same slot must not be added twice";
    EXPECT_TRUE(nc->RegisterEventMap(&slot2)) << L"a different object is a different handler";

    EXPECT_TRUE(nc->UnregisterEventMap(&slot1));
    EXPECT_FALSE(nc->UnregisterEventMap(&slot1)) << L"already removed";
    EXPECT_TRUE(nc->UnregisterEventMap(&slot2));

    app.app->EnableNotifyCenter(FALSE);
}

#if ENABLE_STDFUNCTOR
TEST(soui_event_notify_center, run_on_ui_sync_executes_before_returning)
{
    HeadlessApp app;
    app.app->EnableNotifyCenter(TRUE);
    SNotifyCenter *nc = SNotifyCenter::getSingletonPtr();
    ASSERT_TRUE(nc != NULL);

    int order = 0;
    nc->RunOnUISync(SetFlag(&order));
    EXPECT_EQ(1, order) << L"RunOnUISync must have run the closure before returning";

    app.app->EnableNotifyCenter(FALSE);
}
#endif // ENABLE_STDFUNCTOR
