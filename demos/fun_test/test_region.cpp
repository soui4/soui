/**
 * swinx region APIs (Wine-derived WINEREGION implementation):
 * creation, bounds, point/rect membership, equality, combining, offset.
 */

#include <gtest/gtest.h>
#include <windows.h>
#include <vector>

TEST(swinx_region, create_rect_and_box)
{
    HRGN r = CreateRectRgn(10, 10, 100, 100);
    ASSERT_TRUE(r != NULL);

    RECT box = {0};
    INT type = GetRgnBox(r, &box);
    EXPECT_EQ(type, SIMPLEREGION);
    EXPECT_EQ(box.left, 10);
    EXPECT_EQ(box.top, 10);
    EXPECT_EQ(box.right, 100);
    EXPECT_EQ(box.bottom, 100);

    EXPECT_TRUE(DeleteObject(r));
}

TEST(swinx_region, create_rect_indirect)
{
    RECT rc = {1, 2, 30, 40};
    HRGN r = CreateRectRgnIndirect(&rc);
    ASSERT_TRUE(r != NULL);

    RECT box = {0};
    GetRgnBox(r, &box);
    EXPECT_EQ(box.left, 1);
    EXPECT_EQ(box.top, 2);
    EXPECT_EQ(box.right, 30);
    EXPECT_EQ(box.bottom, 40);

    EXPECT_TRUE(DeleteObject(r));
}

TEST(swinx_region, rect_in_region)
{
    HRGN r = CreateRectRgn(10, 10, 100, 100);
    ASSERT_TRUE(r != NULL);

    // fully inside
    RECT inside = {20, 20, 50, 50};
    EXPECT_TRUE(RectInRegion(r, &inside));
    // fully outside
    RECT outside = {200, 200, 210, 210};
    EXPECT_FALSE(RectInRegion(r, &outside));

    EXPECT_TRUE(DeleteObject(r));
}

TEST(swinx_region, equal_rgn)
{
    HRGN a = CreateRectRgn(0, 0, 50, 50);
    HRGN b = CreateRectRgn(0, 0, 50, 50);
    HRGN c = CreateRectRgn(5, 5, 50, 50);
    ASSERT_TRUE(a != NULL && b != NULL && c != NULL);

    EXPECT_TRUE(EqualRgn(a, b));
    EXPECT_FALSE(EqualRgn(a, c));

    DeleteObject(a);
    DeleteObject(b);
    DeleteObject(c);
}

TEST(swinx_region, combine_or_disjoint)
{
    HRGN r1 = CreateRectRgn(0, 0, 10, 10);
    HRGN r2 = CreateRectRgn(20, 20, 30, 30);
    HRGN dst = CreateRectRgn(0, 0, 0, 0);
    ASSERT_TRUE(r1 != NULL && r2 != NULL && dst != NULL);

    INT type = CombineRgn(dst, r1, r2, RGN_OR);
    EXPECT_EQ(type, COMPLEXREGION); // two disjoint rects

    RECT box = {0};
    GetRgnBox(dst, &box);
    EXPECT_EQ(box.left, 0);
    EXPECT_EQ(box.top, 0);
    EXPECT_EQ(box.right, 30);
    EXPECT_EQ(box.bottom, 30);

    DeleteObject(r1);
    DeleteObject(r2);
    DeleteObject(dst);
}

TEST(swinx_region, combine_and_overlap)
{
    HRGN r1 = CreateRectRgn(0, 0, 20, 20);
    HRGN r2 = CreateRectRgn(10, 10, 30, 30);
    HRGN dst = CreateRectRgn(0, 0, 0, 0);
    ASSERT_TRUE(r1 != NULL && r2 != NULL && dst != NULL);

    INT type = CombineRgn(dst, r1, r2, RGN_AND);
    EXPECT_EQ(type, SIMPLEREGION);

    RECT box = {0};
    GetRgnBox(dst, &box);
    EXPECT_EQ(box.left, 10);
    EXPECT_EQ(box.top, 10);
    EXPECT_EQ(box.right, 20);
    EXPECT_EQ(box.bottom, 20);

    DeleteObject(r1);
    DeleteObject(r2);
    DeleteObject(dst);
}

TEST(swinx_region, combine_diff)
{
    HRGN r1 = CreateRectRgn(0, 0, 20, 20);
    HRGN r2 = CreateRectRgn(10, 10, 30, 30);
    HRGN dst = CreateRectRgn(0, 0, 0, 0);
    ASSERT_TRUE(r1 != NULL && r2 != NULL && dst != NULL);

    // r1 - r2 leaves an L-shape (two rects) => complex
    INT type = CombineRgn(dst, r1, r2, RGN_DIFF);
    EXPECT_EQ(type, COMPLEXREGION);

    RECT box = {0};
    GetRgnBox(dst, &box);
    EXPECT_EQ(box.left, 0);
    EXPECT_EQ(box.top, 0);
    EXPECT_EQ(box.right, 20);
    EXPECT_EQ(box.bottom, 20);

    DeleteObject(r1);
    DeleteObject(r2);
    DeleteObject(dst);
}

TEST(swinx_region, combine_or_identical)
{
    HRGN r1 = CreateRectRgn(5, 5, 25, 25);
    HRGN r2 = CreateRectRgn(5, 5, 25, 25);
    HRGN dst = CreateRectRgn(0, 0, 0, 0);
    ASSERT_TRUE(r1 != NULL && r2 != NULL && dst != NULL);

    INT type = CombineRgn(dst, r1, r2, RGN_OR);
    EXPECT_EQ(type, SIMPLEREGION);
    EXPECT_TRUE(EqualRgn(dst, r1));

    DeleteObject(r1);
    DeleteObject(r2);
    DeleteObject(dst);
}

TEST(swinx_region, offset_rgn)
{
    HRGN r = CreateRectRgn(10, 10, 100, 100);
    ASSERT_TRUE(r != NULL);

    EXPECT_EQ(OffsetRgn(r, 5, 7), SIMPLEREGION);

    RECT box = {0};
    GetRgnBox(r, &box);
    EXPECT_EQ(box.left, 15);
    EXPECT_EQ(box.top, 17);
    EXPECT_EQ(box.right, 105);
    EXPECT_EQ(box.bottom, 107);

    EXPECT_TRUE(DeleteObject(r));
}

TEST(swinx_region, pt_in_region)
{
    // Win32 rectangle regions include left/top and exclude right/bottom
    HRGN r = CreateRectRgn(10, 10, 50, 50);
    ASSERT_TRUE(r != NULL);

    EXPECT_TRUE(PtInRegion(r, 30, 30));  // well inside
    EXPECT_TRUE(PtInRegion(r, 10, 10));  // left/top corner is inclusive
    EXPECT_FALSE(PtInRegion(r, 5, 30));  // left of the region
    EXPECT_FALSE(PtInRegion(r, 60, 60)); // right/below the region
    EXPECT_FALSE(PtInRegion(r, 50, 50)); // right/bottom corner is exclusive
    EXPECT_FALSE(PtInRegion(r, 30, 50)); // bottom edge is exclusive

    EXPECT_TRUE(DeleteObject(r));
}

TEST(swinx_region, get_region_data_size_and_header)
{
    HRGN r = CreateRectRgn(3, 5, 70, 90);
    ASSERT_TRUE(r != NULL);

    // first call with NULL buffer returns the required size
    DWORD need = GetRegionData(r, 0, NULL);
    ASSERT_GT(need, 0u);
    ASSERT_EQ(need, (DWORD)(sizeof(RGNDATAHEADER) + sizeof(RECT)));

    std::vector<unsigned char> buf(need);
    RGNDATA *data = (RGNDATA *)&buf[0];
    EXPECT_EQ(GetRegionData(r, need, data), need);

    EXPECT_EQ(data->rdh.dwSize, (DWORD)sizeof(RGNDATAHEADER));
    EXPECT_EQ(data->rdh.iType, (DWORD)RDH_RECTANGLES);
    EXPECT_EQ(data->rdh.nCount, (DWORD)1);
    EXPECT_EQ(data->rdh.nRgnSize, (DWORD)sizeof(RECT));
    EXPECT_EQ(data->rdh.rcBound.left, 3);
    EXPECT_EQ(data->rdh.rcBound.top, 5);
    EXPECT_EQ(data->rdh.rcBound.right, 70);
    EXPECT_EQ(data->rdh.rcBound.bottom, 90);

    // the single rectangle must be the region itself
    RECT *rects = (RECT *)data->Buffer;
    EXPECT_EQ(rects[0].left, 3);
    EXPECT_EQ(rects[0].top, 5);
    EXPECT_EQ(rects[0].right, 70);
    EXPECT_EQ(rects[0].bottom, 90);

    // a too-small buffer signals failure by returning 0
    EXPECT_EQ(GetRegionData(r, need - 1, data), 0u);

    EXPECT_TRUE(DeleteObject(r));
}

TEST(swinx_region, combine_xor)
{
    HRGN a = CreateRectRgn(0, 0, 100, 100);
    HRGN b = CreateRectRgn(50, 0, 150, 100);
    ASSERT_TRUE(a != NULL && b != NULL);

    // XOR keeps the two symmetric difference strips (2 rects -> complex)
    HRGN x = CreateRectRgn(0, 0, 0, 0);
    ASSERT_TRUE(x != NULL);
    INT type = CombineRgn(x, a, b, RGN_XOR);
    EXPECT_EQ(type, COMPLEXREGION);

    RECT box = {0};
    GetRgnBox(x, &box);
    EXPECT_EQ(box.left, 0);
    EXPECT_EQ(box.top, 0);
    EXPECT_EQ(box.right, 150);
    EXPECT_EQ(box.bottom, 100);

    // points inside only one of the two rects belong to the XOR result
    EXPECT_TRUE(PtInRegion(x, 25, 50));  // only in A
    EXPECT_TRUE(PtInRegion(x, 125, 50)); // only in B
    // the intersection must be excluded
    EXPECT_FALSE(PtInRegion(x, 75, 50));

    EXPECT_TRUE(DeleteObject(x));
    EXPECT_TRUE(DeleteObject(a));
    EXPECT_TRUE(DeleteObject(b));
}

// 迁自 test.cpp（原 soui.region_and）：包含关系的 RGN_OR 不改变包围盒
TEST(swinx_region, combine_or_contained_keeps_box)
{
    RECT rc1 = { 0,0,600,400 };
    RECT rc2 = { 10,10,56,18 };
    RECT rcCom = { 0,0,600,400 };
    HRGN rgn1 = CreateRectRgnIndirect(&rc1);
    HRGN rgn2 = CreateRectRgnIndirect(&rc2);
    ASSERT_TRUE(rgn1 != NULL && rgn2 != NULL);
    CombineRgn(rgn1, rgn1, rgn2, RGN_OR);
    DeleteObject(rgn2);
    RECT rc3;
    GetRgnBox(rgn1, &rc3);
    DeleteObject(rgn1);
    EXPECT_EQ(memcmp(&rc3, &rcCom, sizeof(RECT)), 0);
}

// 迁自 test.cpp（原 soui.region_diff）：整列左条被减去后剩余区域仍为单个矩形
TEST(swinx_region, combine_diff_rect_remainder)
{
    RECT rc1 = { 0,0,600,400 };
    RECT rc2 = { 0,0,100,400 };
    RECT rcCom = { 100,0,600,400 };
    HRGN rgn1 = CreateRectRgnIndirect(&rc1);
    HRGN rgn2 = CreateRectRgnIndirect(&rc2);
    ASSERT_TRUE(rgn1 != NULL && rgn2 != NULL);
    CombineRgn(rgn1, rgn1, rgn2, RGN_DIFF);
    DeleteObject(rgn2);
    RECT rc3;
    GetRgnBox(rgn1, &rc3);
    DeleteObject(rgn1);
    EXPECT_EQ(memcmp(&rc3, &rcCom, sizeof(RECT)), 0);
}
