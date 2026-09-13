/**
 * utilities 库 helper 测试（当前覆盖 helper/STls.h 线程局部存储）。
 *
 * 与 test_kernel.cpp 的 swinx_tls 区分：那里测的是 Win32 API
 * （TlsAlloc/TlsSetValue/TlsGetValue/TlsFree 的 swinx 兼容实现），
 * 这里是 utilities 自己实现的 STls（对标 SDL3 的 SDL_TLS*）：
 *   - ID 懒分配（首次 Set 才分配全局槽位；Get 不分配）
 *   - 按线程隔离（子线程看不到主线程的值），线程退出前手动 Cleanup
 *   - 析构回调 + Cleanup 的语义（含 Cleanup 后存储仍可复用）
 *   - 多 ID 独立，且超过分配块大小(TLS_ALLOC_CHUNKSIZE=4)后走扩容路径
 *   - 参数校验（id 为 NULL）与空 Cleanup 的幂等
 */
#include <gtest/gtest.h>
#include <windows.h>
#include <helper/STls.h>

namespace
{

// 子线程探针：主线程 Set 过的值，子线程应该看不到。
struct TlsProbe
{
    SOUI::STlsId *id;
    void *seenBeforeSet;
    void *valueToSet;
    void *seenAfterSet;
};

static DWORD WINAPI tlsProbeThread(LPVOID param)
{
    TlsProbe *p = (TlsProbe *)param;
    p->seenBeforeSet = SOUI::STls::Get(p->id); // 新线程：应为 NULL
    SOUI::STls::Set(p->id, p->valueToSet, NULL);
    p->seenAfterSet = SOUI::STls::Get(p->id);
    SOUI::STls::Cleanup(); // 契约：线程退出前清理本线程的存储
    return 0;
}

static int s_destructorCalls = 0;
static void *s_lastDestroyed = NULL;

static void countingDestructor(void *value)
{
    s_destructorCalls++;
    s_lastDestroyed = value;
}

} // namespace

TEST(utilities_tls, lazy_id_alloc_and_roundtrip)
{
    static SOUI::STlsId id = 0; // 契约：ID 必须静态存活，初值 0

    // 未 Set 过：ID 仍为 0，Get 返回 NULL 且不触发分配
    EXPECT_EQ((LONG)id, 0);
    EXPECT_TRUE(SOUI::STls::Get(&id) == NULL);
    EXPECT_EQ((LONG)id, 0);

    int payload = 0x1234;
    EXPECT_TRUE(SOUI::STls::Set(&id, &payload, NULL));
    EXPECT_NE((LONG)id, 0); // 首次 Set 才分配全局槽位
    EXPECT_EQ(SOUI::STls::Get(&id), (void *)&payload);

    // value 传 NULL 表示清除本线程该槽位的值
    EXPECT_TRUE(SOUI::STls::Set(&id, NULL, NULL));
    EXPECT_TRUE(SOUI::STls::Get(&id) == NULL);

    SOUI::STls::Cleanup();
}

TEST(utilities_tls, per_thread_isolation)
{
    static SOUI::STlsId id = 0;
    int mainValue = 1;
    int threadValue = 2;

    ASSERT_TRUE(SOUI::STls::Set(&id, &mainValue, NULL));
    ASSERT_EQ(SOUI::STls::Get(&id), (void *)&mainValue);

    TlsProbe probe = {&id, NULL, &threadValue, NULL};
    HANDLE th = CreateThread(NULL, 0, tlsProbeThread, &probe, 0, NULL);
    ASSERT_TRUE(th != NULL);
    ASSERT_EQ(WaitForSingleObject(th, 5000), (DWORD)WAIT_OBJECT_0);
    CloseHandle(th);

    EXPECT_TRUE(probe.seenBeforeSet == NULL); // 子线程看不到主线程的值
    EXPECT_EQ(probe.seenAfterSet, (void *)&threadValue);
    EXPECT_EQ(SOUI::STls::Get(&id), (void *)&mainValue); // 主线程的值不受影响

    SOUI::STls::Cleanup();
}

TEST(utilities_tls, destructor_invoked_by_cleanup)
{
    static SOUI::STlsId id = 0;
    int payload = 7;
    ASSERT_TRUE(SOUI::STls::Set(&id, &payload, countingDestructor));

    int before = s_destructorCalls;
    SOUI::STls::Cleanup();

    EXPECT_EQ(s_destructorCalls - before, 1);
    EXPECT_EQ(s_lastDestroyed, (void *)&payload);
    EXPECT_TRUE(SOUI::STls::Get(&id) == NULL); // 存储随 Cleanup 释放

    // Cleanup 之后仍可继续使用：Set 会重建本线程的存储
    EXPECT_TRUE(SOUI::STls::Set(&id, &payload, NULL));
    EXPECT_EQ(SOUI::STls::Get(&id), (void *)&payload);

    SOUI::STls::Cleanup();
}

TEST(utilities_tls, destructor_runs_even_for_null_value)
{
    // Set(id, NULL, dtor) 的语义是"注册析构函数但把值清空"：Cleanup 仍会回调一次，
    // 因此 STlsDestructor 必须能接受 NULL 实参（与所对标的 SDL3 SDL_TLSCleanup 一致）。
    static SOUI::STlsId id = 0;
    ASSERT_TRUE(SOUI::STls::Set(&id, NULL, countingDestructor));

    int before = s_destructorCalls;
    s_lastDestroyed = (void *)0x1; // 哨兵：区分"回调未发生"与"回调收到 NULL"
    SOUI::STls::Cleanup();

    EXPECT_EQ(s_destructorCalls - before, 1);
    EXPECT_TRUE(s_lastDestroyed == NULL);
}

TEST(utilities_tls, multiple_ids_are_independent)
{
    // TLS_ALLOC_CHUNKSIZE = 4：用 6 个 ID 强制走 per-thread 数组的扩容(realloc)路径
    enum
    {
        N = 6
    };
    static SOUI::STlsId ids[N] = {0};
    int values[N];

    for (int i = 0; i < N; i++)
    {
        values[i] = i;
        ASSERT_TRUE(SOUI::STls::Set(&ids[i], &values[i], NULL)) << "id " << i;
    }

    // 懒分配出来的槽位应互不相同
    for (int i = 0; i < N; i++)
        for (int j = i + 1; j < N; j++)
            EXPECT_NE((LONG)ids[i], (LONG)ids[j]);

    for (int i = 0; i < N; i++)
        EXPECT_EQ(SOUI::STls::Get(&ids[i]), (void *)&values[i]) << "id " << i;

    SOUI::STls::Cleanup();

    // Cleanup 后本线程存储已释放：所有槽位读回 NULL
    for (int i = 0; i < N; i++)
        EXPECT_TRUE(SOUI::STls::Get(&ids[i]) == NULL) << "id " << i;
}

TEST(utilities_tls, null_id_and_empty_cleanup)
{
    EXPECT_TRUE(SOUI::STls::Get(NULL) == NULL);
    EXPECT_FALSE(SOUI::STls::Set(NULL, (void *)1, NULL));

    // 契约：未使用过 TLS 的线程调用 Cleanup 是 no-op；重复调用同样安全
    SOUI::STls::Cleanup();
    SOUI::STls::Cleanup();
}
