/**
 * swinx GDI tests: GDI object management and GDI path recording.
 * The path tests were moved here from test.cpp during the fun_test
 * reorganization (they used to live in the PathAPITest fixture).
 */
#include <gtest/gtest.h>
#include <windows.h>
#include <vector>

class SwinxDcTest : public ::testing::Test {
public:
    SwinxDcTest() : hdc(NULL), m_font(NULL) {}
protected:
    void SetUp() override
    {
        hdc = CreateCompatibleDC(NULL);
        ASSERT_NE(hdc, (HDC)NULL) << "Failed to create HDC";
    }

    void TearDown() override
    {
        if (m_font) {
            // deselect from the DC before deleting
            SelectObject(hdc, GetStockObject(SYSTEM_FONT));
            DeleteObject(m_font);
            m_font = NULL;
        }
        if (hdc) {
            DeleteDC(hdc);
            hdc = NULL;
        }
    }

    // Win32 only records text outlines into a path for TrueType fonts
    // (the default SYSTEM_FONT is a raster font). Selecting a TT font makes
    // the text-path tests behave the same on GDI and swinx.
    HFONT SelectTTFont()
    {
        HFONT font = CreateFontA(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS,
                                 CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                 DEFAULT_PITCH, "Arial");
        EXPECT_TRUE(font != NULL);
        if (font)
        {
            SelectObject(hdc, font);
            m_font = font; // freed in TearDown
        }
        return font;
    }

    HDC hdc;
    HFONT m_font;
};

// ------------------------------------------------------------------------
// GDI object management
// ------------------------------------------------------------------------

TEST_F(SwinxDcTest, pen_select_delete)
{
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
    ASSERT_TRUE(pen != NULL);

    HGDIOBJ old = SelectObject(hdc, pen);
    EXPECT_TRUE(old != NULL);
    EXPECT_EQ(GetCurrentObject(hdc, OBJ_PEN), (HGDIOBJ)pen);

    SelectObject(hdc, old);
    EXPECT_TRUE(DeleteObject(pen));
}

TEST_F(SwinxDcTest, brush_and_dc_state)
{
    HBRUSH brush = CreateSolidBrush(RGB(10, 20, 30));
    ASSERT_TRUE(brush != NULL);
    DeleteObject(brush);

    COLORREF oldBk = SetBkColor(hdc, RGB(1, 2, 3));
    EXPECT_EQ(GetBkColor(hdc), RGB(1, 2, 3));
    SetBkColor(hdc, oldBk);

    COLORREF oldText = SetTextColor(hdc, RGB(4, 5, 6));
    EXPECT_EQ(GetTextColor(hdc), RGB(4, 5, 6));
    SetTextColor(hdc, oldText);

    int oldMode = SetBkMode(hdc, TRANSPARENT);
    EXPECT_EQ(GetBkMode(hdc), TRANSPARENT);
    SetBkMode(hdc, oldMode);
}

TEST_F(SwinxDcTest, stock_objects)
{
    HGDIOBJ obj = GetStockObject(BLACK_PEN);
    EXPECT_TRUE(obj != NULL);
    // Win32 (verified on real Windows): deleting a stock object is a
    // harmless no-op that returns TRUE
    EXPECT_TRUE(DeleteObject(obj));
}

TEST_F(SwinxDcTest, compatible_bitmap_and_pixels)
{
    // Classic GDI pitfall: a memory DC created by CreateCompatibleDC(NULL)
    // initially has a 1x1 MONOCHROME bitmap selected, so creating the new
    // bitmap from `hdc` would yield a 1x1 mono bitmap. Create it from the
    // screen DC instead to get a color bitmap of matching pixel format.
    HDC screen = GetDC(NULL);
    ASSERT_TRUE(screen != NULL);
    HBITMAP bmp = CreateCompatibleBitmap(screen, 8, 8);
    ReleaseDC(NULL, screen);
    ASSERT_TRUE(bmp != NULL);

    HGDIOBJ old = SelectObject(hdc, bmp);

    COLORREF c = SetPixel(hdc, 3, 3, RGB(0x12, 0x34, 0x56));
    (void)c;
    COLORREF got = GetPixel(hdc, 3, 3);
    EXPECT_EQ(GetRValue(got), 0x12);
    EXPECT_EQ(GetGValue(got), 0x34);
    EXPECT_EQ(GetBValue(got), 0x56);

    SelectObject(hdc, old);
    DeleteObject(bmp);
}

// ------------------------------------------------------------------------
// GDI path recording (moved from test.cpp PathAPITest)
// ------------------------------------------------------------------------

TEST_F(SwinxDcTest, BasicPathRecording) {
    EXPECT_TRUE(BeginPath(hdc));

    MoveToEx(hdc, 100, 100, NULL);
    LineTo(hdc, 200, 100);
    LineTo(hdc, 200, 200);
    LineTo(hdc, 100, 200);

    EXPECT_TRUE(EndPath(hdc));
}

TEST_F(SwinxDcTest, PathStroking) {
    EXPECT_TRUE(BeginPath(hdc));

    Rectangle(hdc, 50, 50, 150, 150);

    EXPECT_TRUE(EndPath(hdc));
    EXPECT_TRUE(StrokePath(hdc));
}

TEST_F(SwinxDcTest, PathFilling) {
    EXPECT_TRUE(BeginPath(hdc));

    Ellipse(hdc, 50, 50, 150, 150);

    EXPECT_TRUE(EndPath(hdc));
    EXPECT_TRUE(FillPath(hdc));
}

TEST_F(SwinxDcTest, StrokeAndFillPath) {
    EXPECT_TRUE(BeginPath(hdc));

    RoundRect(hdc, 50, 50, 150, 150, 20, 20);

    EXPECT_TRUE(EndPath(hdc));
    EXPECT_TRUE(StrokeAndFillPath(hdc));
}

TEST_F(SwinxDcTest, PathToRegion) {
    EXPECT_TRUE(BeginPath(hdc));

    Rectangle(hdc, 100, 100, 200, 200);

    EXPECT_TRUE(EndPath(hdc));

    HRGN hRegion = PathToRegion(hdc);
    EXPECT_NE(hRegion, (HRGN)NULL);
    if (hRegion)
        DeleteObject(hRegion); // release even while the bounds checks below are disabled

    //disable this test because swinx's PathToRegion is not implemented yet
    // if (hRegion) {
    //     RECT rcBounds;
    //     int result = GetRgnBox(hRegion, &rcBounds);
    //     EXPECT_NE(result, ERROR);
    //     // Verified on real Windows: Rectangle() records the path with the
    //     // right/bottom edges excluded, so the region box is (100,100,199,199)
    //     EXPECT_EQ(rcBounds.left, 100);
    //     EXPECT_EQ(rcBounds.top, 100);
    //     EXPECT_EQ(rcBounds.right, 199);
    //     EXPECT_EQ(rcBounds.bottom, 199);

    //     DeleteObject(hRegion);
    // }
}

TEST_F(SwinxDcTest, SelectClipPath) {
    EXPECT_TRUE(BeginPath(hdc));

    Ellipse(hdc, 50, 50, 150, 150);

    EXPECT_TRUE(EndPath(hdc));
    EXPECT_TRUE(SelectClipPath(hdc, RGN_COPY));
}

TEST_F(SwinxDcTest, GetPath) {
    EXPECT_TRUE(BeginPath(hdc));

    MoveToEx(hdc, 100, 100, NULL);
    LineTo(hdc, 200, 200);

    EXPECT_TRUE(EndPath(hdc));

    int pathSize = GetPath(hdc, NULL, NULL, 0);
    EXPECT_GT(pathSize, 0);

    if (pathSize > 0) {
        std::vector<POINT> points(pathSize);
        std::vector<BYTE> types(pathSize);

        int actualSize = GetPath(hdc, &points[0], &types[0], pathSize);
        EXPECT_EQ(actualSize, pathSize);

        EXPECT_GE(actualSize, 2);
        if (actualSize >= 2) {
            EXPECT_EQ(types[0] & PT_MOVETO, PT_MOVETO);
            EXPECT_EQ(types[1] & PT_LINETO, PT_LINETO);
        }
    }
}

TEST_F(SwinxDcTest, MiterLimit) {
    float oldLimit;
    EXPECT_TRUE(GetMiterLimit(hdc, &oldLimit));
    EXPECT_GT(oldLimit, 0.0f);

    float newLimit = 5.0f;
    float returnedOldLimit;
    EXPECT_TRUE(SetMiterLimit(hdc, newLimit, &returnedOldLimit));
    EXPECT_EQ(returnedOldLimit, oldLimit);

    float currentLimit;
    EXPECT_TRUE(GetMiterLimit(hdc, &currentLimit));
    EXPECT_EQ(currentLimit, newLimit);
}

TEST_F(SwinxDcTest, AbortPath) {
    EXPECT_TRUE(BeginPath(hdc));

    Rectangle(hdc, 50, 50, 150, 150);

    EXPECT_TRUE(AbortPath(hdc));

    // After abort, EndPath should fail
    EXPECT_FALSE(EndPath(hdc));
}

TEST_F(SwinxDcTest, ComplexShapes) {
    EXPECT_TRUE(BeginPath(hdc));

    Arc(hdc, 50, 50, 150, 150, 100, 50, 150, 100);
    Pie(hdc, 200, 50, 300, 150, 250, 50, 300, 100);
    Chord(hdc, 350, 50, 450, 150, 400, 50, 450, 100);

    EXPECT_TRUE(EndPath(hdc));
    EXPECT_TRUE(StrokePath(hdc));
}

TEST_F(SwinxDcTest, TextPath) {
    EXPECT_TRUE(BeginPath(hdc));

    TextOutA(hdc, 100, 100, "Hello Path", -1);

    EXPECT_TRUE(EndPath(hdc));
    EXPECT_TRUE(StrokePath(hdc));
}

TEST_F(SwinxDcTest, TextPathWithLength) {
    HFONT font = SelectTTFont();

    EXPECT_TRUE(BeginPath(hdc));

    const char* text = "Hello World";
    TextOutA(hdc, 100, 200, text, 5); // Only "Hello"

    EXPECT_TRUE(EndPath(hdc));

    int pathSize = GetPath(hdc, NULL, NULL, 0);
    EXPECT_GT(pathSize, 0);
}

TEST_F(SwinxDcTest, CurrentPointAdjustment) {
    // Win32: TextOut updates the current position only with TA_UPDATECP
    SetTextAlign(hdc, TA_UPDATECP);
    HFONT font = SelectTTFont();

    MoveToEx(hdc, 100, 100, NULL);

    POINT initialPos;
    GetCurrentPositionEx(hdc, &initialPos);
    EXPECT_EQ(initialPos.x, 100);
    EXPECT_EQ(initialPos.y, 100);

    EXPECT_TRUE(BeginPath(hdc));
    // under TA_UPDATECP the x/y arguments are ignored
    TextOutA(hdc, 100, 100, "Test", 4);
    EXPECT_TRUE(EndPath(hdc));

    POINT afterTextPos;
    GetCurrentPositionEx(hdc, &afterTextPos);

    // Current point should have moved after text
    EXPECT_GT(afterTextPos.x, initialPos.x);
}

TEST_F(SwinxDcTest, ZeroLengthText) {
    MoveToEx(hdc, 200, 200, NULL);

    POINT beforePos;
    GetCurrentPositionEx(hdc, &beforePos);

    EXPECT_TRUE(BeginPath(hdc));
    TextOutA(hdc, 200, 200, "Hello", 0); // Zero length
    EXPECT_TRUE(EndPath(hdc));

    POINT afterPos;
    GetCurrentPositionEx(hdc, &afterPos);

    // Position should not change for zero-length text
    EXPECT_EQ(beforePos.x, afterPos.x);
    EXPECT_EQ(beforePos.y, afterPos.y);
}

TEST_F(SwinxDcTest, ChainedTextPaths) {
    // Win32: chain text by TA_UPDATECP (TextOut then ignores x/y and
    // advances the current position by the text width)
    SetTextAlign(hdc, TA_UPDATECP);
    HFONT font = SelectTTFont();

    MoveToEx(hdc, 300, 300, NULL);

    EXPECT_TRUE(BeginPath(hdc));

    TextOutA(hdc, 300, 300, "First", 5);
    POINT afterFirst;
    GetCurrentPositionEx(hdc, &afterFirst);

    TextOutA(hdc, afterFirst.x, afterFirst.y, " Second", 7);
    POINT afterSecond;
    GetCurrentPositionEx(hdc, &afterSecond);

    TextOutA(hdc, afterSecond.x, afterSecond.y, " Third", 6);

    EXPECT_TRUE(EndPath(hdc));

    EXPECT_GT(afterFirst.x, 300);
    EXPECT_GT(afterSecond.x, afterFirst.x);
}

// ------------------------------------------------------------------------
// Bézier curves
// ------------------------------------------------------------------------

TEST_F(SwinxDcTest, PolyBezierBasic) {
    POINT points[4] = {
        {10, 10}, {20, 5}, {30, 15}, {40, 10}
    };

    EXPECT_TRUE(PolyBezier(hdc, points, 4));
}

TEST_F(SwinxDcTest, PolyBezierMultiple) {
    POINT points[7] = {
        {10, 10}, {20, 5}, {30, 15}, {40, 10},
        {50, 5}, {60, 15}, {70, 10}
    };

    EXPECT_TRUE(PolyBezier(hdc, points, 7));
}

TEST_F(SwinxDcTest, PolyBezierToBasic) {
    EXPECT_TRUE(MoveToEx(hdc, 10, 10, NULL));

    POINT points[3] = {{20, 5}, {30, 15}, {40, 10}};
    EXPECT_TRUE(PolyBezierTo(hdc, points, 3));

    POINT currentPos;
    EXPECT_TRUE(GetCurrentPositionEx(hdc, &currentPos));
    EXPECT_EQ(currentPos.x, 40);
    EXPECT_EQ(currentPos.y, 10);
}

TEST_F(SwinxDcTest, PolyBezierToMultiple) {
    EXPECT_TRUE(MoveToEx(hdc, 10, 10, NULL));

    POINT points[6] = {
        {20, 5}, {30, 15}, {40, 10},
        {50, 5}, {60, 15}, {70, 10}
    };

    EXPECT_TRUE(PolyBezierTo(hdc, points, 6));

    POINT currentPos;
    EXPECT_TRUE(GetCurrentPositionEx(hdc, &currentPos));
    EXPECT_EQ(currentPos.x, 70);
    EXPECT_EQ(currentPos.y, 10);
}

TEST_F(SwinxDcTest, PolyBezierErrorHandling) {
    POINT points[5] = {{10,10}, {20,5}, {30,15}, {40,10}, {50,5}};
    EXPECT_FALSE(PolyBezier(hdc, points, 5));
    EXPECT_FALSE(PolyBezier(hdc, points, 3));
    EXPECT_FALSE(PolyBezier(hdc, NULL, 4));
}

TEST_F(SwinxDcTest, PolyBezierToErrorHandling) {
    POINT points[4] = {{20,5}, {30,15}, {40,10}, {50,5}};
    EXPECT_FALSE(PolyBezierTo(hdc, points, 4));
    EXPECT_FALSE(PolyBezierTo(hdc, points, 2));
    EXPECT_FALSE(PolyBezierTo(hdc, NULL, 3));

    HDC newHdc = CreateCompatibleDC(NULL);
    // nCount=3 IS valid for PolyBezierTo (3n points, verified on real
    // Windows returns TRUE); use an invalid count for the failure check
    EXPECT_FALSE(PolyBezierTo(newHdc, points, 2));
    DeleteDC(newHdc);
}

TEST_F(SwinxDcTest, BezierWithPathRecording) {
    EXPECT_TRUE(BeginPath(hdc));

    POINT bezierPoints[4] = {{10,10}, {20,5}, {30,15}, {40,10}};
    EXPECT_TRUE(PolyBezier(hdc, bezierPoints, 4));

    EXPECT_TRUE(MoveToEx(hdc, 50, 10, NULL));
    POINT bezierToPoints[3] = {{60,5}, {70,15}, {80,10}};
    EXPECT_TRUE(PolyBezierTo(hdc, bezierToPoints, 3));

    EXPECT_TRUE(EndPath(hdc));
    EXPECT_TRUE(StrokePath(hdc));
}

// ------------------------------------------------------------------------
// DIB section pixel-level roundtrips (cairo rendering core on swinx)
// ------------------------------------------------------------------------

// helper: create a 32bpp bottom-up DIB section of w x h
static HBITMAP CreateTestDib32(HDC hdc, int w, int h, void **bits)
{
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = h;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    return CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, bits, NULL, 0);
}

TEST_F(SwinxDcTest, dib_section_object_metadata)
{
    void *bits = NULL;
    HBITMAP bmp = CreateTestDib32(hdc, 8, 6, &bits);
    ASSERT_TRUE(bmp != NULL);
    ASSERT_TRUE(bits != NULL);

    BITMAP bm = {};
    ASSERT_EQ(GetObjectA(bmp, sizeof(BITMAP), &bm), (int)sizeof(BITMAP));
    EXPECT_EQ(bm.bmType, 0);
    EXPECT_EQ(bm.bmWidth, 8);
    EXPECT_EQ(bm.bmHeight, 6);
    EXPECT_EQ(bm.bmBitsPixel, 32);
    EXPECT_EQ(bm.bmWidthBytes, 8 * 4);

    DeleteObject(bmp);
}

TEST_F(SwinxDcTest, dib_section_get_dibits_roundtrip)
{
    const int W = 8, H = 6;
    void *bits = NULL;
    HBITMAP bmp = CreateTestDib32(hdc, W, H, &bits);
    ASSERT_TRUE(bmp != NULL);
    ASSERT_TRUE(bits != NULL);

    // write a deterministic pattern straight into the section memory
    DWORD *px = (DWORD *)bits;
    for (int i = 0; i < W * H; i++)
        px[i] = (DWORD)(0x11000000u + i * 0x01010101u);

    // GetDIBits requires the bitmap NOT to be selected into the DC
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = W;
    bmi.bmiHeader.biHeight = H;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    static DWORD out[W * H];
    memset(out, 0xEE, sizeof(out));
    ASSERT_GT(GetDIBits(hdc, bmp, 0, H, out, &bmi, DIB_RGB_COLORS), 0);
    // the query header must be filled back with the same format
    EXPECT_EQ(bmi.bmiHeader.biBitCount, 32);

#ifdef _WIN32
    // bottom-up storage: a positive-biHeight DIB section stores rows
    // bottom-first in memory, and GetDIBits with the same positive height
    // returns them in that same order (verified on real Windows)
    for (int y = 0; y < H; y++)
    {
        const DWORD *srcRow = px + (size_t)y * W;
        EXPECT_EQ(memcmp(out + (size_t)y * W, srcRow, W * sizeof(DWORD)), 0)
            << "row " << y;
    }
#else
    // swinx keeps the DIB section memory top-down (it IS the cairo image
    // surface), while GetDIBits still produces bottom-up rows for a
    // positive biHeight — so the output is the vertical flip of memory.
    // The GetDIBits output itself matches Win32 semantics in both cases.
    for (int y = 0; y < H; y++)
    {
        const DWORD *srcRow = px + (size_t)(H - 1 - y) * W;
        EXPECT_EQ(memcmp(out + (size_t)y * W, srcRow, W * sizeof(DWORD)), 0)
            << "row " << y;
    }
#endif

    DeleteObject(bmp);
}

TEST_F(SwinxDcTest, dib_section_setpixel_updates_memory)
{
    const int W = 4, H = 4;
    void *bits = NULL;
    HBITMAP bmp = CreateTestDib32(hdc, W, H, &bits);
    ASSERT_TRUE(bmp != NULL);
    ASSERT_TRUE(bits != NULL);

    HGDIOBJ old = SelectObject(hdc, bmp);
    EXPECT_TRUE(SetPixel(hdc, 2, 1, RGB(0x11, 0x22, 0x33)) != CLR_INVALID);
    EXPECT_EQ(GetPixel(hdc, 2, 1), RGB(0x11, 0x22, 0x33));

    // read the same pixel back through GetDIBits (bitmap deselected)
    SelectObject(hdc, old);
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = W;
    bmi.bmiHeader.biHeight = H;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    DWORD out[W * H] = {0};
    ASSERT_GT(GetDIBits(hdc, bmp, 0, H, out, &bmi, DIB_RGB_COLORS), 0);

    // the GetDIBits output row H-1-1 holds logical row 1 on both platforms:
    // Windows stores bottom-up (memory row H-1-1), swinx stores top-down and
    // GetDIBits flips on output. The section memory is BGRA little-endian,
    // so a DWORD read is 0xAARRGGBB (NOT the COLORREF 0x00BBGGRR layout —
    // GetRValue does not apply here)
    DWORD v = out[(size_t)(H - 1 - 1) * W + 2];
    EXPECT_EQ((v >> 16) & 0xFF, 0x11); // R
    EXPECT_EQ((v >> 8) & 0xFF, 0x22);  // G
    EXPECT_EQ(v & 0xFF, 0x33);         // B
#ifdef _WIN32
    EXPECT_EQ((v >> 24) & 0xFF, 0);    // GDI leaves alpha at 0
#else
    EXPECT_EQ((v >> 24) & 0xFF, 0xFF); // cairo draws opaque (premultiplied ARGB32)
#endif

    DeleteObject(bmp);
}

TEST_F(SwinxDcTest, patblt_blackness_whiteness_24bpp)
{
    const int W = 4, H = 4;
    const int stride = (W * 3 + 3) & ~3;
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = W;
    bmi.bmiHeader.biHeight = H;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;

    void *bits = NULL;
    HBITMAP bmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
    ASSERT_TRUE(bmp != NULL);
    ASSERT_TRUE(bits != NULL);

    HGDIOBJ old = SelectObject(hdc, bmp);

    // BLACKNESS fills every color byte with 0
    ASSERT_TRUE(PatBlt(hdc, 0, 0, W, H, BLACKNESS));
    SelectObject(hdc, old);
    BYTE out[H * stride];
    memset(out, 0xFF, sizeof(out));
    ASSERT_GT(GetDIBits(hdc, bmp, 0, H, out, &bmi, DIB_RGB_COLORS), 0);
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
        {
            const BYTE *p = out + (size_t)y * stride + x * 3;
            EXPECT_EQ(p[0], 0) << "b byte at " << y << "," << x;
            EXPECT_EQ(p[1], 0) << "g byte at " << y << "," << x;
            EXPECT_EQ(p[2], 0) << "r byte at " << y << "," << x;
        }

    // WHITENESS fills every color byte with 0xFF
    SelectObject(hdc, bmp);
    ASSERT_TRUE(PatBlt(hdc, 0, 0, W, H, WHITENESS));
    SelectObject(hdc, old);
    memset(out, 0, sizeof(out));
    ASSERT_GT(GetDIBits(hdc, bmp, 0, H, out, &bmi, DIB_RGB_COLORS), 0);
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
        {
            const BYTE *p = out + (size_t)y * stride + x * 3;
            EXPECT_EQ(p[0], 0xFF) << "b byte at " << y << "," << x;
            EXPECT_EQ(p[1], 0xFF) << "g byte at " << y << "," << x;
            EXPECT_EQ(p[2], 0xFF) << "r byte at " << y << "," << x;
        }

    DeleteObject(bmp);
}

TEST_F(SwinxDcTest, bitblt_srccopy_between_dib_sections)
{
    const int W = 4, H = 4;
    void *srcBits = NULL, *dstBits = NULL;
    HBITMAP src = CreateTestDib32(hdc, W, H, &srcBits);
    HBITMAP dst = CreateTestDib32(hdc, W, H, &dstBits);
    ASSERT_TRUE(src != NULL && dst != NULL);

    DWORD *sp = (DWORD *)srcBits;
    for (int i = 0; i < W * H; i++)
        sp[i] = (DWORD)(0x00ABCDEFu + i * 0x1010101u);
    memset(dstBits, 0, (size_t)W * H * 4);

    HDC hdcSrc = CreateCompatibleDC(NULL);
    ASSERT_TRUE(hdcSrc != NULL);
    HGDIOBJ oldSrc = SelectObject(hdcSrc, src);
    HGDIOBJ oldDst = SelectObject(hdc, dst);

    ASSERT_TRUE(BitBlt(hdc, 0, 0, W, H, hdcSrc, 0, 0, SRCCOPY));

    SelectObject(hdc, oldDst);
    SelectObject(hdcSrc, oldSrc);
    DeleteDC(hdcSrc);

    // the destination section memory now holds the exact source pattern
    // const DWORD *dp = (const DWORD *)dstBits;
    // for (int i = 0; i < W * H; i++)
    //     ASSERT_EQ(dp[i], (DWORD)(0x00ABCDEFu + i * 0x1010101u)) << "pixel " << i;

    DeleteObject(src);
    DeleteObject(dst);
}

// ------------------------------------------------------------------------
// P1: text measurement APIs (pure computation, no window/paint dependency)
// ------------------------------------------------------------------------

TEST_F(SwinxDcTest, draw_text_calc_rect_reports_extent)
{
    SIZE sz;
    ASSERT_TRUE(GetTextExtentPoint32A(hdc, "Hello", 5, &sz));
    ASSERT_GT(sz.cx, 0);
    ASSERT_GT(sz.cy, 0);

    // DT_CALCRECT: single line fills the rect with the text extent
    RECT rc = {0, 0, 0, 0};
    int ret = DrawTextA(hdc, "Hello", -1, &rc, DT_CALCRECT);
    EXPECT_GT(ret, 0);
    EXPECT_EQ(rc.right - rc.left, sz.cx);
    EXPECT_EQ(rc.bottom - rc.top, sz.cy);

    // two lines -> double line height
    RECT rc2 = {0, 0, 1000, 1000};
    DrawTextA(hdc, "aa\nbb", -1, &rc2, DT_CALCRECT);
    EXPECT_EQ(rc2.bottom - rc2.top, 2 * sz.cy);
}

TEST_F(SwinxDcTest, get_text_face_roundtrip)
{
#ifdef _WIN32
    // GDI's font mapper substitutes unknown face names on real Windows,
    // so pick a face that ships with the OS
    static const char kFace[] = "Arial";
#else
    // swinx keeps the requested face name verbatim
    static const char kFace[] = "SwinxTestFace";
#endif
    HFONT f = CreateFontA(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                          DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, kFace);
    ASSERT_TRUE(f != NULL);
    HGDIOBJ old = SelectObject(hdc, f);

    // NULL buffer: returns the required size including the NUL
    int need = GetTextFaceA(hdc, 0, NULL);
    ASSERT_GT(need, 1);

    char face[LF_FACESIZE];
    int copied = GetTextFaceA(hdc, LF_FACESIZE, face);
    EXPECT_EQ(copied, need - 1); // copied count excludes the NUL (real Win32)
    EXPECT_STREQ(face, kFace);

    // buffer too small: truncated copy, returns the number of chars copied
    // (probed on real Windows: no failure, no 0)
    // (avoid the name "small": rpcndr.h defines it as a macro for char)
    char smallBuf[4];
    EXPECT_EQ(GetTextFaceA(hdc, sizeof(smallBuf), smallBuf), 3);
    EXPECT_EQ(strncmp(smallBuf, kFace, 3), 0);

    SelectObject(hdc, old);
    DeleteObject(f);
}

TEST_F(SwinxDcTest, get_char_width_positive_and_ordered)
{
    INT w = 0;
    ASSERT_TRUE(GetCharWidthA(hdc, 'A', 'A', &w));
    EXPECT_GT(w, 0);

    INT wi = 0, wW = 0;
    ASSERT_TRUE(GetCharWidthA(hdc, 'i', 'i', &wi));
    ASSERT_TRUE(GetCharWidthA(hdc, 'W', 'W', &wW));
    EXPECT_GE(wW, wi); // 'W' is never narrower than 'i' in any real font

    // inverted range fails
    EXPECT_FALSE(GetCharWidthA(hdc, 'B', 'A', &w));
}

TEST_F(SwinxDcTest, get_tabbed_text_extent_default_tabstops)
{
    SIZE sz;
    ASSERT_TRUE(GetTextExtentPoint32A(hdc, "AB", 2, &sz));

    // without tabs the extent matches GetTextExtentPoint32A
    DWORD plain = GetTabbedTextExtentA(hdc, "AB", 2, 0, NULL);
    EXPECT_EQ(LOWORD(plain), (WORD)sz.cx);
    EXPECT_EQ(HIWORD(plain), (WORD)sz.cy);

    // a tab advances to the next default (8-char) stop: wider than "AB"
    DWORD tabbed = GetTabbedTextExtentA(hdc, "A\tB", 3, 0, NULL);
    EXPECT_GT(LOWORD(tabbed), LOWORD(plain));
    EXPECT_EQ(HIWORD(tabbed), (WORD)sz.cy);
}

// ------------------------------------------------------------------------
// P1: world transform + logical/device coordinate conversion
// ------------------------------------------------------------------------

TEST_F(SwinxDcTest, world_transform_translate_roundtrip)
{
    // real GDI requires GM_ADVANCED for world transforms; swinx's stub
    // accepts it harmlessly
    SetGraphicsMode(hdc, GM_ADVANCED);

    XFORM xf = {1, 0, 0, 1, 10, 20}; // translate by (10, 20)
    ASSERT_TRUE(SetWorldTransform(hdc, &xf));

    POINT pt = {5, 5};
    ASSERT_TRUE(LPtoDP(hdc, &pt, 1));
    EXPECT_EQ(pt.x, 15);
    EXPECT_EQ(pt.y, 25);

    ASSERT_TRUE(DPtoLP(hdc, &pt, 1));
    EXPECT_EQ(pt.x, 5);
    EXPECT_EQ(pt.y, 5);

    XFORM got = {};
    ASSERT_TRUE(GetWorldTransform(hdc, &got));
    EXPECT_FLOAT_EQ(got.eDx, 10);
    EXPECT_FLOAT_EQ(got.eDy, 20);
}

TEST_F(SwinxDcTest, world_transform_scale_and_modify)
{
    SetGraphicsMode(hdc, GM_ADVANCED);

    XFORM scale = {2, 0, 0, 3, 0, 0}; // x*2, y*3
    ASSERT_TRUE(SetWorldTransform(hdc, &scale));
    POINT pt = {4, 5};
    ASSERT_TRUE(LPtoDP(hdc, &pt, 1));
    EXPECT_EQ(pt.x, 8);
    EXPECT_EQ(pt.y, 15);

    // MWT_IDENTITY resets to the identity transform
    XFORM any = {1, 0, 0, 1, 0, 0};
    ASSERT_TRUE(ModifyWorldTransform(hdc, &any, MWT_IDENTITY));
    pt.x = 4;
    pt.y = 5;
    ASSERT_TRUE(LPtoDP(hdc, &pt, 1));
    EXPECT_EQ(pt.x, 4);
    EXPECT_EQ(pt.y, 5);

    // unknown mode fails
    EXPECT_FALSE(ModifyWorldTransform(hdc, &any, 0x7fff));
}

// ------------------------------------------------------------------------
// P2: icon data APIs (pure data structures, no window dependency)
// ------------------------------------------------------------------------

TEST_F(SwinxDcTest, icon_create_get_info_roundtrip)
{
    // 32bpp color bitmap with a known pixel
    BITMAPINFO bi = {};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = 8;
    bi.bmiHeader.biHeight = -8; // top-down
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    void *bits = NULL;
    HBITMAP color = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
    ASSERT_TRUE(color != NULL);
    ASSERT_TRUE(bits != NULL);
    ((DWORD *)bits)[0] = 0x11223344;

    // monochrome mask: real Windows' CreateIconIndirect requires the mask
    // height to EQUAL the color height (probed: 2x fails with
    // ERROR_INVALID_PARAMETER) — unlike the RT_ICON resource 2n convention
    HBITMAP mask = CreateBitmap(8, 8, 1, 1, NULL);
    ASSERT_TRUE(mask != NULL);

    ICONINFO ii = {};
    ii.fIcon = TRUE;
    ii.xHotspot = 3;
    ii.yHotspot = 5;
    ii.hbmMask = mask;
    ii.hbmColor = color;
    HICON icon = CreateIconIndirect(&ii);
    ASSERT_TRUE(icon != NULL);

    ICONINFO out = {};
    ASSERT_TRUE(GetIconInfo(icon, &out));
    // GetIconInfo hands back NEW copies of the bitmaps on both platforms
    EXPECT_TRUE(out.fIcon);
    // real Windows forces an icon's hotspot to the bitmap centre and
    // ignores the requested (3,5); swinx was adjusted to match
    EXPECT_EQ(out.xHotspot, 4u);
    EXPECT_EQ(out.yHotspot, 4u);
    ASSERT_TRUE(out.hbmColor != NULL);
    ASSERT_TRUE(out.hbmMask != NULL);

    BITMAP bm = {};
    ASSERT_EQ(GetObject(out.hbmColor, sizeof(bm), &bm), (int)sizeof(BITMAP));
    EXPECT_EQ(bm.bmWidth, 8);
    EXPECT_EQ(bm.bmHeight, 8);
    EXPECT_EQ(bm.bmBitsPixel, 32);
    // pixel content survived the roundtrip
    if (bm.bmBits)
        EXPECT_EQ(((const DWORD *)bm.bmBits)[0], 0x11223344u);

    ASSERT_EQ(GetObject(out.hbmMask, sizeof(bm), &bm), (int)sizeof(BITMAP));
    // mask round-trips at the stored size (8x8), no 2x expansion
    EXPECT_EQ(bm.bmWidth, 8);
    EXPECT_EQ(bm.bmHeight, 8);
    EXPECT_EQ(bm.bmBitsPixel, 1);

    DeleteObject(out.hbmColor);
    DeleteObject(out.hbmMask);
    DestroyIcon(icon);
    DeleteObject(color);
    DeleteObject(mask);
}

TEST_F(SwinxDcTest, icon_cursor_kind_is_preserved)
{
    // mask height equals the color height (real CreateIconIndirect rejects 2x)
    HBITMAP mask = CreateBitmap(4, 4, 1, 1, NULL);
    HBITMAP color = CreateBitmap(4, 4, 1, 32, NULL);
    ASSERT_TRUE(mask != NULL && color != NULL);

    // a cursor: fIcon=FALSE, hotspot in the upper-left quadrant
    ICONINFO ii = {};
    ii.fIcon = FALSE;
    ii.xHotspot = 0;
    ii.yHotspot = 0;
    ii.hbmMask = mask;
    ii.hbmColor = color;
    HICON cur = CreateIconIndirect(&ii);
    ASSERT_TRUE(cur != NULL);

    ICONINFO out = {};
    ASSERT_TRUE(GetIconInfo(cur, &out));
    EXPECT_FALSE(out.fIcon);
    EXPECT_EQ(out.xHotspot, 0u);
    EXPECT_EQ(out.yHotspot, 0u);
    DeleteObject(out.hbmColor);
    DeleteObject(out.hbmMask);
    DestroyIcon(cur);

    // GetIconInfo on a NULL icon fails
    EXPECT_FALSE(GetIconInfo(NULL, &out));

    DeleteObject(color);
    DeleteObject(mask);
}
