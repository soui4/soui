/**
 * swinx time/tick APIs: GetTickCount monotonicity, QPC/QPF, local/system time.
 */
#include <gtest/gtest.h>
#include <windows.h>

TEST(swinx_time, get_tick_count_monotonic)
{
    DWORD t1 = GetTickCount();
    Sleep(60);
    DWORD t2 = GetTickCount();
    // allow scheduling slack around the sleep
    EXPECT_GE(t2 - t1, 40u);
}

TEST(swinx_time, get_tick_count64_not_behind_32bit)
{
    DWORD t32 = GetTickCount();
    ULONGLONG t64 = GetTickCount64();
    EXPECT_GE(t64, (ULONGLONG)t32);
}

TEST(swinx_time, query_performance_counter_monotonic)
{
    LARGE_INTEGER f = {0};
    ASSERT_TRUE(QueryPerformanceFrequency(&f));
    EXPECT_GT(f.QuadPart, 0);

    LARGE_INTEGER c1 = {0}, c2 = {0};
    ASSERT_TRUE(QueryPerformanceCounter(&c1));
    Sleep(20);
    ASSERT_TRUE(QueryPerformanceCounter(&c2));
    EXPECT_GE(c2.QuadPart, c1.QuadPart);

    // ~20ms elapsed (allow wide slack for scheduler)
    double ms = double(c2.QuadPart - c1.QuadPart) * 1000.0 / double(f.QuadPart);
    EXPECT_GE(ms, 5.0);
    EXPECT_LE(ms, 500.0);
}

static void checkSanity(const SYSTEMTIME &st)
{
    EXPECT_GE(st.wYear, 2020u);
    EXPECT_LE(st.wYear, 2100u);
    EXPECT_GE(st.wMonth, 1u);
    EXPECT_LE(st.wMonth, 12u);
    EXPECT_GE(st.wDay, 1u);
    EXPECT_LE(st.wDay, 31u);
    EXPECT_LT(st.wHour, 24u);
    EXPECT_LT(st.wMinute, 60u);
    EXPECT_LT(st.wSecond, 60u);
}

TEST(swinx_time, get_local_time_sanity)
{
    SYSTEMTIME st = {};
    GetLocalTime(&st);
    checkSanity(st);
}

TEST(swinx_time, get_system_time_sanity)
{
    SYSTEMTIME st = {};
    GetSystemTime(&st);
    checkSanity(st);
}

// ------------------------------------------------------------------------
// FILETIME conversion family. These were historically the most bug-prone
// area (epoch/unit errors in FileTime2TimeSpec), so every converter that is
// publicly declared gets a roundtrip or ordering assertion.
// ------------------------------------------------------------------------

static const FILETIME kFixedFileTime = {0x2b1a9c00, 0x01d8f0a1}; // arbitrary fixed instant

TEST(swinx_filetime, compare_file_time_ordering)
{
    FILETIME a = {100, 1};
    FILETIME b = {100, 1};
    FILETIME c = {200, 1}; // higher low part
    FILETIME d = {100, 2}; // higher high part

    EXPECT_EQ(CompareFileTime(&a, &b), 0);
    EXPECT_EQ(CompareFileTime(&a, &c), -1);
    EXPECT_EQ(CompareFileTime(&c, &a), 1);
    EXPECT_EQ(CompareFileTime(&a, &d), -1);
    EXPECT_EQ(CompareFileTime(&d, &a), 1);
    EXPECT_EQ(CompareFileTime(&kFixedFileTime, &kFixedFileTime), 0);
}

TEST(swinx_filetime, local_file_time_roundtrip)
{
    // the actual bias depends on the machine timezone, but the two-way
    // conversion must be an exact identity in both directions
    FILETIME local = {}, back = {};
    ASSERT_TRUE(FileTimeToLocalFileTime(&kFixedFileTime, &local));
    ASSERT_TRUE(LocalFileTimeToFileTime(&local, &back));
    EXPECT_EQ(back.dwLowDateTime, kFixedFileTime.dwLowDateTime);
    EXPECT_EQ(back.dwHighDateTime, kFixedFileTime.dwHighDateTime);

    // and the reverse order roundtrips too
    FILETIME utc2 = {}, back2 = {};
    ASSERT_TRUE(LocalFileTimeToFileTime(&local, &utc2));
    ASSERT_TRUE(FileTimeToLocalFileTime(&utc2, &back2));
    EXPECT_EQ(back2.dwLowDateTime, local.dwLowDateTime);
    EXPECT_EQ(back2.dwHighDateTime, local.dwHighDateTime);
}

TEST(swinx_filetime, dos_datetime_roundtrip)
{
    // 2022-12-21 12:34:56 in MS-DOS packed form; seconds are even so the
    // 2-second DOS resolution is lossless
    WORD fatdate = (WORD)(((2022 - 1980) << 9) | (12 << 5) | 21);
    WORD fattime = (WORD)((12 << 11) | (34 << 5) | (56 / 2));

    FILETIME ft = {};
    ASSERT_TRUE(DosDateTimeToFileTime(fatdate, fattime, &ft));

    WORD d2 = 0, t2 = 0;
    ASSERT_TRUE(FileTimeToDosDateTime(&ft, &d2, &t2));
    EXPECT_EQ(d2, fatdate);
    EXPECT_EQ(t2, fattime);

    // 12:34:58 packs differently from 12:34:56 and must stay distinct
    WORD fattime58 = (WORD)((12 << 11) | (34 << 5) | (58 / 2));
    FILETIME ft58 = {};
    ASSERT_TRUE(DosDateTimeToFileTime(fatdate, fattime58, &ft58));
    WORD d3 = 0, t3 = 0;
    ASSERT_TRUE(FileTimeToDosDateTime(&ft58, &d3, &t3));
    EXPECT_EQ(d3, fatdate);
    EXPECT_EQ(t3, fattime58);
    EXPECT_NE(CompareFileTime(&ft, &ft58), 0);
}
