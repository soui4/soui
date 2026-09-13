/**
 * swinx memory management tests (Global/Local/Heap allocators).
 * LocalHandle exercises the HeapLock/HeapUnlock pairing fixed recently
 * (the entry point used to HeapUnlock without a matching HeapLock).
 */
#include <gtest/gtest.h>
#include <windows.h>
#include <string.h>

TEST(swinx_memory, global_alloc_fixed)
{
    HGLOBAL h = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, 100);
    ASSERT_TRUE(h != NULL);

    EXPECT_EQ(GlobalSize(h), 100u);

    char *ptr = (char *)GlobalLock(h);
    ASSERT_TRUE(ptr != NULL);
    for (int i = 0; i < 100; i++)
        EXPECT_EQ(ptr[i], 0); // GMEM_ZEROINIT
    memset(ptr, 0xAB, 100);
    GlobalUnlock(h);

    EXPECT_TRUE(GlobalFree(h) == NULL);
}

TEST(swinx_memory, global_alloc_moveable)
{
    HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, 100);
    ASSERT_TRUE(h != NULL);

    char *ptr = (char *)GlobalLock(h);
    ASSERT_TRUE(ptr != NULL);
    memcpy(ptr, "moveable", 9);
    GlobalUnlock(h);

    // realloc preserving contents
    HGLOBAL h2 = GlobalReAlloc(h, 200, GMEM_MOVEABLE);
    ASSERT_TRUE(h2 != NULL);

    EXPECT_GE(GlobalSize(h2), 100u);
    ptr = (char *)GlobalLock(h2);
    ASSERT_TRUE(ptr != NULL);
    EXPECT_STREQ(ptr, "moveable");
    GlobalUnlock(h2);

    EXPECT_TRUE(GlobalFree(h2) == NULL);
}

TEST(swinx_memory, local_alloc_lock_handle)
{
    HLOCAL h = LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, 64);
    ASSERT_TRUE(h != NULL);

    char *ptr = (char *)LocalLock(h);
    ASSERT_TRUE(ptr != NULL);
    memcpy(ptr, "local handle test", 18);

    // LocalHandle maps a locked pointer back to its handle
    HLOCAL h2 = LocalHandle(ptr);
    EXPECT_EQ(h2, h);

    LocalUnlock(h);
    EXPECT_TRUE(LocalFree(h) == NULL);
}

TEST(swinx_memory, heap_create_alloc)
{
    HANDLE hHeap = HeapCreate(0, 0, 0);
    ASSERT_TRUE(hHeap != NULL);

    void *p1 = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 128);
    ASSERT_TRUE(p1 != NULL);
    for (int i = 0; i < 128; i++)
        EXPECT_EQ(((char *)p1)[i], 0);

    void *p2 = HeapAlloc(hHeap, 0, 64);
    ASSERT_TRUE(p2 != NULL);

    EXPECT_TRUE(HeapFree(hHeap, 0, p1));
    EXPECT_TRUE(HeapFree(hHeap, 0, p2));

    EXPECT_TRUE(HeapDestroy(hHeap));
}
