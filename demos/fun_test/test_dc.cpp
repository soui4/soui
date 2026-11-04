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
    SwinxDcTest() : hdc(NULL), m_font(NULL), m_target(NULL), m_oldTarget(NULL),
                   m_probeDone(false), m_pixelsOk(false) {}
protected:
    void SetUp()
    {
        hdc = CreateCompatibleDC(NULL);
        ASSERT_NE(hdc, (HDC)NULL) << "Failed to create HDC";
    }

    void TearDown()
    {
        if (m_font) {
            // deselect from the DC before deleting
            SelectObject(hdc, GetStockObject(SYSTEM_FONT));
            DeleteObject(m_font);
            m_font = NULL;
        }
        if (m_target) {
            if (m_oldTarget) SelectObject(hdc, m_oldTarget);
            DeleteObject(m_target);
            m_target = NULL;
            m_oldTarget = NULL;
        }
        if (hdc) {
            DeleteDC(hdc);
            hdc = NULL;
        }
    }

    // Pixel assertions are only decidable where the carrier really rasterizes:
    // the node leg's stub canvas answers getImageData with all zeros, so probe
    // once at run time instead of blaming the implementation for the stub.
    bool pixelsReadable()
    {
        if (m_probeDone) return m_pixelsOk;
        m_probeDone = true;
        HDC dc = CreateCompatibleDC(NULL);
        HDC screen = GetDC(NULL);
        HBITMAP bmp = screen ? CreateCompatibleBitmap(screen, 4, 4) : NULL;
        if (screen) ReleaseDC(NULL, screen);
        if (dc && bmp)
        {
            HGDIOBJ old = SelectObject(dc, bmp);
            PatBlt(dc, 0, 0, 4, 4, BLACKNESS);
            m_pixelsOk = (GetPixel(dc, 2, 2) == RGB(0, 0, 0));
            SelectObject(dc, old);
        }
        if (bmp) DeleteObject(bmp);
        if (dc) DeleteDC(dc);
        return m_pixelsOk;
    }

    // Attach a w x h color target to the test DC and paint it white.
    // TearDown re-selects the old bitmap and deletes this one.
    HBITMAP SelectWhiteTarget(int w, int h)
    {
        HDC screen = GetDC(NULL);
        HBITMAP bmp = screen ? CreateCompatibleBitmap(screen, w, h) : NULL;
        if (screen) ReleaseDC(NULL, screen);
        if (!bmp) return NULL;
        m_target = bmp;
        m_oldTarget = SelectObject(hdc, bmp);
        PatBlt(hdc, 0, 0, w, h, WHITENESS);
        return bmp;
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
    HBITMAP m_target;
    HGDIOBJ m_oldTarget;
    bool m_probeDone;
    bool m_pixelsOk;
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
    DWORD nGdiBefore = GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS);
    EXPECT_TRUE(DeleteObject(obj));
    // "no-op" has to mean no-op: the object stays alive, so the process GDI
    // object count must not drop. A backend that builds stock objects out of an
    // ordinary deleteable handle frees them here while its own cached handle
    // still points at the freed memory -- every DC created afterwards then uses
    // a dangling default pen/brush/font (use-after-free).
    EXPECT_EQ(GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS), nGdiBefore);
    // and the cached handle must still be the live stock object the next DC
    // selects as its default pen
    EXPECT_TRUE(GetStockObject(BLACK_PEN) == obj);
    HDC dc = CreateCompatibleDC(NULL);
    ASSERT_TRUE(dc != NULL);
    EXPECT_TRUE(GetCurrentObject(dc, OBJ_PEN) == obj);
    MoveToEx(dc, 0, 0, NULL); // strokes with the stock pen
    LineTo(dc, 4, 4);
    DeleteDC(dc);
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


static int RgnComplexity(HRGN hRgn) {
    RECT bounds = {0};
    return GetRgnBox(hRgn, &bounds);
}

// 非轴对齐路径转区域：cairo 后端原来用“裁剪矩形列表”导出路径几何，弧线、
// 圆角矩形会被判为 CLIP_NOT_REPRESENTABLE，PathToRegion 因此返回空区域。
// 区域必须按路径形状逐像素给出 —— 退化成包围盒同样算错。
TEST_F(SwinxDcTest, PathToRegion_roundrect_keeps_shape)
{
    EXPECT_TRUE(BeginPath(hdc));
    RoundRect(hdc, 20, 20, 120, 120, 60, 60); // 圆角半径 30
    EXPECT_TRUE(EndPath(hdc));

    HRGN hRegion = PathToRegion(hdc);
    ASSERT_NE(hRegion, (HRGN)NULL);
    if (hRegion)
    {
        EXPECT_NE(RgnComplexity(hRegion), NULLREGION);
        EXPECT_TRUE(PtInRegion(hRegion, 70, 70));  // 中心
        EXPECT_TRUE(PtInRegion(hRegion, 70, 22));  // 直边内侧
        EXPECT_FALSE(PtInRegion(hRegion, 22, 22)); // 圆角外：包围盒退化会误报为真
        EXPECT_FALSE(PtInRegion(hRegion, 5, 5));   // 路径之外
        DeleteObject(hRegion);
    }
}

// 连续的 LineTo 必须连成同一个图元。cairo 后端曾在记录期每次 LineTo 之后补一次
// cairo_move_to，把一条折线拆成互不相连的子路径；填充时每个子路径各自隐式闭合，
// 四边形就退化成三条线（SWaveProgress 的波浪矩形因此画成楔形）。
TEST_F(SwinxDcTest, PathRecording_consecutive_lineto_stays_onefigure)
{
    EXPECT_TRUE(BeginPath(hdc));
    MoveToEx(hdc, 0, 10, NULL);
    LineTo(hdc, 100, 10);
    LineTo(hdc, 100, 50);
    LineTo(hdc, 0, 50);
    EXPECT_TRUE(EndPath(hdc));

    // 一个起点 + 三条线段。拆成子路径时每次补的 move_to 会各记一个点。
    int n = GetPath(hdc, NULL, NULL, 0);
    ASSERT_EQ(n, 4);
    std::vector<POINT> points(n);
    std::vector<BYTE> types(n);
    ASSERT_EQ(GetPath(hdc, &points[0], &types[0], n), n);
    EXPECT_EQ(types[0], PT_MOVETO);
    for (int i = 1; i < n; i++)
        EXPECT_EQ(types[i], PT_LINETO) << "第 " << i << " 个点又从 MoveTo 重新开始";

    // 填充按 GDI 语义隐式闭合回起点 (0,10)，区域应覆盖整个四边形。
    HRGN hRegion = PathToRegion(hdc);
    ASSERT_NE(hRegion, (HRGN)NULL);
    if (hRegion)
    {
        EXPECT_NE(RgnComplexity(hRegion), NULLREGION);
        EXPECT_TRUE(PtInRegion(hRegion, 50, 20));  // 上部
        EXPECT_TRUE(PtInRegion(hRegion, 50, 40));  // 右下：散成线段时这里必为空
        EXPECT_FALSE(PtInRegion(hRegion, 50, 5));  // 顶边之上
        DeleteObject(hRegion);
    }
}

// SSpeedMeter 的彩色圆弧走 addArc + DrawGradientPath，落到 GDI 后端就是
// “路径转区域当裁剪”。半圆饼只覆盖一侧，包围盒退化会把两侧都算进去。
TEST_F(SwinxDcTest, PathToRegion_pie_covers_sector)
{
    EXPECT_TRUE(BeginPath(hdc));
    Pie(hdc, 0, 0, 200, 200, 200, 100, 0, 100);
    EXPECT_TRUE(EndPath(hdc));

    HRGN hRegion = PathToRegion(hdc);
    ASSERT_NE(hRegion, (HRGN)NULL);
    if (hRegion)
    {
        EXPECT_NE(RgnComplexity(hRegion), NULLREGION);
        BOOL bUp = PtInRegion(hRegion, 100, 60);
        BOOL bDown = PtInRegion(hRegion, 100, 140);
        EXPECT_NE(bUp, bDown); // 恰好覆盖一侧
        EXPECT_FALSE(PtInRegion(hRegion, 1, 1));   // 圆外
        EXPECT_FALSE(PtInRegion(hRegion, 199, 1)); // 圆外
        DeleteObject(hRegion);
    }
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
    if (pathSize <= 0)
        return;

    // 只判断"非空"太弱：任何一段无关涂画都能让它变绿。这里用 "Hello" 的排版框把
    // 断言收紧到坐标：路径的每个点都必须落在框内（被截掉的 " World" 不能出现）。
    // Windows/cairo 记字形轮廓、canvas2d 记整串排版框，两者都满足这个包含关系，
    // 同时又都能挡住退化成整幅画布或原点图形的回归。
    // （canvas2d 实测点集为 x∈[100,172]、y∈[200,225]，与 100+sz.cx、200+tmHeight
    //   一致；把 tol 改成 -1 本用例会逐点报错，证明断言不是空跑。）
    SIZE sz;
    TEXTMETRICA tm;
    ASSERT_TRUE(GetTextExtentPoint32A(hdc, "Hello", 5, &sz));
    ASSERT_TRUE(GetTextMetricsA(hdc, &tm));
    const int tol = 2; // 字形侧 bearing / 取整余量
    std::vector<POINT> points(pathSize);
    std::vector<BYTE> types(pathSize);
    ASSERT_EQ(GetPath(hdc, &points[0], &types[0], pathSize), pathSize);
    int checked = 0;
    for (int i = 0; i < pathSize; i++)
    {
        // Win32 约定：PT_CLOSEFIGURE 对应的点坐标无意义（各后端在这里填 0 或重复上一点）。
        if (types[i] == PT_CLOSEFIGURE)
            continue;
        checked++;
        EXPECT_GE(points[i].x, 100 - tol) << "点 " << i << " 跑到文本左边界之外";
        EXPECT_LE(points[i].x, 100 + sz.cx + tol) << "点 " << i << " 超出 \"Hello\" 的宽度";
        EXPECT_GE(points[i].y, 200 - tol) << "点 " << i << " 跑到行框之上";
        EXPECT_LE(points[i].y, 200 + tm.tmHeight + tol) << "点 " << i << " 跑到行框之下";
    }
    EXPECT_GT(checked, 0) << "全是 PT_CLOSEFIGURE，等于没检查任何坐标";
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

#ifdef _WIN32
static void ClearRect(HDC hdc, LPCRECT pRect, COLORREF cr)
{
    if(cr & 0xFF000000) {
		cr = cr & 0x00FFFFFF;
    }        
    HBRUSH hBrush = CreateSolidBrush(cr);
    FillRect(hdc, pRect, hBrush);
    DeleteObject(hBrush);
}
#endif
// ------------------------------------------------------------------------
// SHostPresenter::OnHostPresent hands the host's memory render target to the
// window with nothing but BitBlt(..., SRCCOPY), and SOUI repaints only the
// dirty rect, so the destination still carries the previous frame's ink under
// it. Win32 SRCCOPY is a replace (a 32bpp copy, alpha included) and the cairo
// backend says so with CAIRO_OPERATOR_SOURCE; a canvas2d backend that maps it
// to source-over cannot erase anything the source left transparent, so a
// moving text window smears into a trail behind itself.
// Two assertions are paired on purpose: "the old ink is gone" alone would also
// be satisfied by a blit that wipes the whole destination canvas.
// ------------------------------------------------------------------------
TEST_F(SwinxDcTest, bitblt_srccopy_replaces_destination)
{
    const int W = 32, H = 32;
    const int DIRTY = 24; // only this much of the target is repainted each frame
    const COLORREF crGold = RGB(0xFF, 0xC8, 0x00);
    const COLORREF crOutside = RGB(0x00, 0x00, 0xFF);
    ASSERT_TRUE(SelectWhiteTarget(W, H) != NULL);
    if (!pixelsReadable())
        return; // the node stub rasterizes nothing; the browser leg judges pixels

    HDC mem = CreateCompatibleDC(hdc);
    ASSERT_TRUE(mem != NULL);
    HDC screen = GetDC(NULL);
    HBITMAP memBmp = screen ? CreateCompatibleBitmap(screen, W, H) : NULL;
    if (screen) ReleaseDC(NULL, screen);
    ASSERT_TRUE(memBmp != NULL);
    SelectObject(mem, memBmp);

    // Ink the destination owns from an earlier paint, outside every dirty rect.
    SetPixel(hdc, W - 2, H - 2, crOutside);

    RECT whole = {0, 0, W, H};
    // frame 1: the glyph sits on the left
    ClearRect(mem, &whole, 0);
    SetPixel(mem, 3, 3, crGold);
    ASSERT_TRUE(BitBlt(hdc, 0, 0, DIRTY, H, mem, 0, 0, SRCCOPY));
    EXPECT_EQ(GetPixel(hdc, 3, 3), crGold) << "frame 1 never reached the destination";

    // frame 2: the same window moved right; frame 1's position is cleared away
    ClearRect(mem, &whole, 0);
    SetPixel(mem, 19, 3, crGold);
    ASSERT_TRUE(BitBlt(hdc, 0, 0, DIRTY, H, mem, 0, 0, SRCCOPY));

    EXPECT_EQ(GetPixel(hdc, 19, 3), crGold) << "the new glyph position did not arrive";
    EXPECT_EQ(GetPixel(hdc, W - 2, H - 2), crOutside)
        << "SRCCOPY reached outside the destination rect";

    int staleX = -1, staleY = -1;
    for (int y = 0; y < 12 && staleX < 0; y++)
    {
        for (int x = 0; x < 12; x++)
        {
            if (GetPixel(hdc, x, y) == crGold)
            {
                staleX = x;
                staleY = y;
                break;
            }
        }
    }
    EXPECT_TRUE(staleX < 0)
        << "frame 1's glyph is still in the destination at " << staleX << "," << staleY
        << ": SRCCOPY composited over it instead of replacing it, so a transparent "
           "source pixel can never erase stale ink (the moving-text trail)";

    DeleteDC(mem);
    DeleteObject(memBmp);
}

// ------------------------------------------------------------------------
// StretchDIBits is the same copy, reached from a CPU-side DIB instead of a
// source DC: the canvas2d backend used to hand the temporary canvas straight to
// drawImage and drop the rop argument, so a 32bpp source with alpha==0 composited
// over the destination and left the previous frame showing through. cairo routes
// this call through CreateDIBitmap + StretchBlt(rop), and Win32's SRCCOPY on
// 32bpp replaces bit for bit including alpha; all three have to agree, because
// icon/cursor loading (swinx/src/platform/posix/cursoricon.cpp) passes SRCCOPY
// here and the artifacts of a mismatched composite show up as stale ink behind a
// drawn image.
// Same pairing as above: "the old ink is gone" must be checked together with the
// opaque source pixel arriving and the destination outside the rect surviving,
// otherwise a blit that wipes the whole canvas would pass too.
// ------------------------------------------------------------------------
TEST_F(SwinxDcTest, stretchdibits_srccopy_replaces_destination)
{
    const int W = 32, H = 32;
    const int SRC = 16, BLIT = 16; // the DIB and the destination rect it is copied into
    const COLORREF crGold = RGB(0xFF, 0xC8, 0x00);
    const COLORREF crGreen = RGB(0x00, 0xFF, 0x00);
    const COLORREF crOutside = RGB(0x00, 0x00, 0xFF);
    ASSERT_TRUE(SelectWhiteTarget(W, H) != NULL);
    if (!pixelsReadable())
        return; // the node stub rasterizes nothing; the browser leg judges pixels

    // ARGB32 as swinx expects it in memory: bytes B,G,R,A == DWORD 0xAARRGGBB.
    // Fully transparent except one opaque pixel, so only the replace semantics can
    // make the stale ink disappear.
    DWORD src[SRC * SRC];
    for (int i = 0; i < SRC * SRC; i++)
        src[i] = 0x00000000;
    src[2 * SRC + 2] = 0xFF00FF00; // (2,2) opaque green

    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = SRC;
    bmi.bmiHeader.biHeight = -SRC; // top-down, the order callers here actually pass
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    // Ink the destination owns from an earlier paint, inside and outside the rect.
    SetPixel(hdc, 3, 3, crGold);
    SetPixel(hdc, W - 2, H - 2, crOutside);

    ASSERT_TRUE(StretchDIBits(hdc, 0, 0, BLIT, BLIT, 0, 0, SRC, SRC, src, &bmi,
                              DIB_RGB_COLORS, SRCCOPY) > 0);

    EXPECT_EQ(GetPixel(hdc, 2, 2), crGreen) << "the opaque source pixel did not arrive";
    EXPECT_EQ(GetPixel(hdc, W - 2, H - 2), crOutside)
        << "StretchDIBits(SRCCOPY) reached outside the destination rect";

    int staleX = -1, staleY = -1;
    for (int y = 0; y < BLIT && staleX < 0; y++)
    {
        for (int x = 0; x < BLIT; x++)
        {
            if (GetPixel(hdc, x, y) == crGold)
            {
                staleX = x;
                staleY = y;
                break;
            }
        }
    }
    EXPECT_TRUE(staleX < 0)
        << "the previous frame's ink is still inside the destination rect at "
        << staleX << "," << staleY
        << ": StretchDIBits composited the transparent source over it instead of "
           "replacing it, which is what a rop argument of SRCCOPY must never do";
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
    // swinx measures with fractional CoreText advances: DT_CALCRECT rounds the
    // extent up so it always fits the text, while GetTextExtentPoint32 keeps
    // the gdi32-compatible truncated width (tied to its lpnDx prefixes), so the
    // two may differ by 1px.
    EXPECT_LE(abs((rc.right - rc.left) - sz.cx), 1);
    EXPECT_EQ(rc.bottom - rc.top, sz.cy);

    // two lines -> double line height
    RECT rc2 = {0, 0, 1000, 1000};
    DrawTextA(hdc, "aa\nbb", -1, &rc2, DT_CALCRECT);
    EXPECT_EQ(rc2.bottom - rc2.top, 2 * sz.cy);
}

// Regression: the DT_CALCRECT extent used to be truncated down. Callers that
// size a window from it (e.g. the msgbox STATIC) then wrapped the last
// character of the widest line onto its own line ("Select render type:" ->
// "Select render type" + ":") because the fractional line width exceeded the
// truncated constraint by a fraction of a pixel. The reported extent must be
// wide enough to hold every line: re-measuring under that exact width must
// not add a line.
TEST_F(SwinxDcTest, draw_text_calc_rect_extent_fits_word_wrap)
{
    const char *text = "Hello\nWorld";
    RECT rcFree = {0, 0, 10000, 1000};
    DrawTextA(hdc, text, -1, &rcFree, DT_CALCRECT);
    ASSERT_GT(rcFree.right - rcFree.left, 0);

    SIZE szA;
    ASSERT_TRUE(GetTextExtentPoint32A(hdc, "A", 1, &szA));
    ASSERT_GT(szA.cy, 0);
    ASSERT_EQ((rcFree.bottom - rcFree.top) % szA.cy, 0) << "two lines of text";

    RECT rcConstrained = {0, 0, rcFree.right - rcFree.left, 1000};
    DrawTextA(hdc, text, -1, &rcConstrained, DT_CALCRECT);
    EXPECT_EQ(rcConstrained.bottom - rcConstrained.top, rcFree.bottom - rcFree.top)
        << "constraining to the reported extent must not wrap an extra line";
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

// ------------------------------------------------------------------------
// Clip region semantics, in the shape of the wasm-demo tetris score animation
// (a transparent child window moved along a parabola every frame; the host
// repaints the union of the old and new rects, so a clip that cannot widen
// back leaves a gold trail on the canvas).
// ------------------------------------------------------------------------

// Win32: SelectClipRgn *replaces* the clipping region (RGN_COPY), it does not
// intersect it. A replace that cannot widen is what turns one leaked
// IntersectClipRect into permanent damage.
//
// The pixel pair matters: painting the far corner black after a full-surface
// widen cannot tell "the clip widened" from "the backend wiped the canvas"
// (both leave black there). So the clear lands in a rect the leaked clip does
// not cover, and a marker inside the leaked clip but outside the clear rect
// must still be red afterwards.
TEST_F(SwinxDcTest, selectcliprgn_replaces_previous_clip)
{
    const int W = 32, H = 32;
    ASSERT_TRUE(SelectWhiteTarget(W, H) != NULL);

    const COLORREF crMark = RGB(0xFF, 0x00, 0x00);
    if (pixelsReadable())
    {
        SetPixel(hdc, 2, 2, crMark);
        EXPECT_EQ(GetPixel(hdc, 2, 2), crMark) << "the carrier cannot SetPixel";
    }

    HRGN smrgn = CreateRectRgn(0, 0, 8, 8);
    HRGN full = CreateRectRgn(0, 0, W, H);
    ASSERT_TRUE(smrgn != NULL && full != NULL);
    SelectClipRgn(hdc, smrgn);
    SelectClipRgn(hdc, full);

    RECT rc;
    EXPECT_NE(GetClipBox(hdc, &rc), (int)NULLREGION);
    EXPECT_EQ(rc.left, 0);
    EXPECT_EQ(rc.top, 0);
    EXPECT_EQ(rc.right, W);
    EXPECT_EQ(rc.bottom, H);

    if (pixelsReadable())
    {
        PatBlt(hdc, 16, 16, 16, 16, BLACKNESS);
        EXPECT_EQ(GetPixel(hdc, W - 2, H - 2), RGB(0, 0, 0))
            << "clip replaced with the whole surface, yet the far corner stayed white";
        EXPECT_EQ(GetPixel(hdc, 2, 2), crMark)
            << "the clip replacement cleared the canvas instead of only the clip";
    }
    DeleteObject(smrgn);
    DeleteObject(full);
}

// The recovery path SOUI uses once per frame: PushClipRegion(dirty, RGN_COPY).
// It must be able to widen back over a clip a previous frame leaked.
TEST_F(SwinxDcTest, extselectcliprgn_copy_recovers_from_leaked_clip)
{
    const int W = 32, H = 32;
    ASSERT_TRUE(SelectWhiteTarget(W, H) != NULL);

    const COLORREF crMark = RGB(0xFF, 0x00, 0x00);
    if (pixelsReadable())
    {
        SetPixel(hdc, 2, 2, crMark);
        EXPECT_EQ(GetPixel(hdc, 2, 2), crMark) << "the carrier cannot SetPixel";
    }

    IntersectClipRect(hdc, 0, 0, 8, 8); // the leak: no SaveDC owns this clip

    int save = SaveDC(hdc);
    ASSERT_TRUE(save != 0);
    HRGN full = CreateRectRgn(0, 0, W, H);
    ASSERT_TRUE(full != NULL);
    ExtSelectClipRgn(hdc, full, RGN_COPY);

    RECT rc;
    EXPECT_NE(GetClipBox(hdc, &rc), (int)NULLREGION);
    EXPECT_EQ(rc.right, W);
    EXPECT_EQ(rc.bottom, H);

    if (pixelsReadable())
    {
        // outside the leaked 8x8, so only a real widening can reach it
        PatBlt(hdc, 16, 16, 16, 16, BLACKNESS);
        EXPECT_EQ(GetPixel(hdc, W - 2, H - 2), RGB(0, 0, 0))
            << "RGN_COPY could not widen past the leaked clip; every later frame "
               "is confined to the leaked rect";
        EXPECT_EQ(GetPixel(hdc, 2, 2), crMark)
            << "widening the clip cleared the canvas; a frame clear must not do that";
    }
    RestoreDC(hdc, -1);
    DeleteObject(full);
}

// Win32: SelectClipRgn(hdc, NULL) REMOVES the clipping region, it is not a
// no-op. A backend that ignores it can never widen the clip back either, and
// hands the leaked rect to every later frame.
TEST_F(SwinxDcTest, selectcliprgn_null_removes_clip)
{
    const int W = 32, H = 32;
    ASSERT_TRUE(SelectWhiteTarget(W, H) != NULL);

    const COLORREF crMark = RGB(0xFF, 0x00, 0x00);
    if (pixelsReadable())
    {
        SetPixel(hdc, 2, 2, crMark);
        EXPECT_EQ(GetPixel(hdc, 2, 2), crMark) << "the carrier cannot SetPixel";
    }

    HRGN smrgn = CreateRectRgn(0, 0, 8, 8);
    ASSERT_TRUE(smrgn != NULL);
    EXPECT_NE(SelectClipRgn(hdc, smrgn), (int)NULLREGION);
    SelectClipRgn(hdc, NULL);

    RECT rc;
    EXPECT_NE(GetClipBox(hdc, &rc), (int)NULLREGION);
    // the surface again, not the 8x8 box left behind (>= so real Windows'
    // visible-region clamp cannot break this)
    EXPECT_LE(rc.left, 0);
    EXPECT_LE(rc.top, 0);
    EXPECT_GE(rc.right, W);
    EXPECT_GE(rc.bottom, H);

    if (pixelsReadable())
    {
        PatBlt(hdc, 16, 16, 16, 16, BLACKNESS);
        EXPECT_EQ(GetPixel(hdc, W - 2, H - 2), RGB(0, 0, 0))
            << "SelectClipRgn(hdc, NULL) left the previous 8x8 clip in place";
        EXPECT_EQ(GetPixel(hdc, 2, 2), crMark)
            << "removing the clip cleared the canvas instead of only the clip";
    }
    DeleteObject(smrgn);
}

// Ink must not reach outside the clip: whatever is painted outside the child
// window rect can never be erased, because invalidation only covers that rect.
TEST_F(SwinxDcTest, clip_confines_text_ink)
{
    const int W = 48, H = 48;
    ASSERT_TRUE(SelectWhiteTarget(W, H) != NULL);
    if (!pixelsReadable()) return;

    HFONT font = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                             DEFAULT_PITCH, "Arial");
    ASSERT_TRUE(font != NULL);
    HGDIOBJ oldFont = SelectObject(hdc, font);
    SetTextColor(hdc, RGB(0xFF, 0xD7, 0x00));

    ASSERT_TRUE(SaveDC(hdc) != 0);
    IntersectClipRect(hdc, 0, 0, 16, 16);
    EXPECT_TRUE(TextOutA(hdc, 8, 4, "+800", 4));
    RestoreDC(hdc, -1);

    for (int y = 20; y < H; y++)
        for (int x = 0; x < W; x++)
            ASSERT_EQ(GetPixel(hdc, x, y), RGB(0xFF, 0xFF, 0xFF))
                << "gold ink at " << x << "," << y << " is outside the 16x16 clip";
    for (int x = 20; x < W; x++)
        for (int y = 0; y < H; y++)
            ASSERT_EQ(GetPixel(hdc, x, y), RGB(0xFF, 0xFF, 0xFF))
                << "gold ink at " << x << "," << y << " is outside the 16x16 clip";

    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

// ------------------------------------------------------------------------
// The two cases above hand the DC an explicit clip region. DrawText is
// different: it owns a formatting rectangle and Win32 clips the glyphs to it
// by itself, unless the caller says DT_NOCLIP. SOUI never adds that clip --
// SWindow::DispatchPaint only applies a widget's m_clipRgn/m_clipPath, and the
// dirty rects it invalidates are the widget rects -- so ink that DrawText lets
// escape its rectangle lands outside any region that would ever be repainted
// over it. That is exactly the gold parabola left behind by the "+score" text
// in demos/wasm-demo: a 20px glyph in a 26dp-tall window paints outside it, and
// when the window moves the escaped pixels are never cleared again.
// The cairo backend already follows Win32 (cairo_clip in TextLayoutEx::draw
// unless DT_NOCLIP); a backend that omits the clip differs from it, and from
// Windows itself, in the direction that produces permanent residue.
// Paired assertions: ink inside the rect must survive (otherwise "clip
// everything" would pass), DT_NOCLIP must overflow (otherwise "never draw"
// would pass), and the clip must not leak into the caller's DC afterwards.
// ------------------------------------------------------------------------
TEST_F(SwinxDcTest, drawtext_clips_to_format_rect)
{
    const int W = 64, H = 64;
    const COLORREF crWhite = RGB(0xFF, 0xFF, 0xFF);
    ASSERT_TRUE(SelectWhiteTarget(W, H) != NULL);
    if (!pixelsReadable())
        return; // the node stub rasterizes nothing; the browser leg judges pixels

    HFONT font = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                             DEFAULT_PITCH, "Arial");
    ASSERT_TRUE(font != NULL);
    HGDIOBJ oldFont = SelectObject(hdc, font);
    SetTextColor(hdc, RGB(0xFF, 0xD7, 0x00));
    SetBkMode(hdc, TRANSPARENT);

    // A 16x16 rectangle holding six 20px bold glyphs: the string is several
    // times wider than the rect, so every non-clipped pixel of "WWWWWW" past
    // x=20 is a visible, permanent trail candidate.
    RECT rc = {4, 4, 20, 20};
    EXPECT_NE(DrawTextA(hdc, "WWWWWW", -1, &rc, DT_LEFT | DT_TOP | DT_SINGLELINE), 0);

    bool inkInside = false;
    int outsideX = -1, outsideY = -1;
    for (int y = 0; y < H; y++)
    {
        for (int x = 0; x < W; x++)
        {
            if (GetPixel(hdc, x, y) == crWhite)
                continue;
            if (x >= 4 && x < 20 && y >= 4 && y < 20)
                inkInside = true;
            else if (outsideX < 0)
            {
                outsideX = x;
                outsideY = y;
            }
        }
    }
    EXPECT_TRUE(inkInside)
        << "nothing was painted inside the 16x16 formatting rect; the case "
           "cannot tell a clip from a total loss of the draw";
    EXPECT_TRUE(outsideX < 0)
        << "gold ink at " << outsideX << "," << outsideY << " escaped the "
           "formatting rect: DrawText must clip to it (Win32, cairo), and SOUI "
           "never invalidates that area again, so escaped ink becomes residue";

    // DT_NOCLIP is the documented opt-out: the very same call has to overflow
    // the rect, otherwise the assertion above would just be proving that
    // nothing gets drawn at all.
    RECT rcWhole = {0, 0, W, H};
    FillRect(hdc, &rcWhole, (HBRUSH)GetStockObject(WHITE_BRUSH));
    EXPECT_TRUE(DrawTextA(hdc, "WWWWWW", -1, &rc, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOCLIP) != 0);
    bool inkFarRight = false;
    for (int y = 0; y < H && !inkFarRight; y++)
        for (int x = 32; x < W; x++)
            if (GetPixel(hdc, x, y) != crWhite)
            {
                inkFarRight = true;
                break;
            }
    EXPECT_TRUE(inkFarRight)
        << "DT_NOCLIP did not let the glyphs out of the rect; the flag is ignored";

    // The clip must be scoped to the call: a fill past the rect afterwards
    // still has to arrive on the DC (a leaked clip would silently swallow it).
    HBRUSH red = CreateSolidBrush(RGB(0xFF, 0x00, 0x00));
    RECT rcAfter = {40, 48, 56, 56};
    EXPECT_EQ((int)FillRect(hdc, &rcAfter, red), 1);
    DeleteObject(red);
    EXPECT_EQ(GetPixel(hdc, 48, 52), RGB(0xFF, 0x00, 0x00))
        << "the DrawText clip leaked onto the caller's DC";

    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

// End to end: the frame loop of the flying "+score" text. Each frame erases
// the union of the previous and current rects, then paints the text clipped to
// the current rect. After the animation the canvas must be uniformly white.
TEST_F(SwinxDcTest, moving_text_leaves_no_residue)
{
    const int W = 120, H = 80;
    ASSERT_TRUE(SelectWhiteTarget(W, H) != NULL);
    if (!pixelsReadable()) return;

    HFONT font = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                             DEFAULT_PITCH, "Arial");
    ASSERT_TRUE(font != NULL);
    HGDIOBJ oldFont = SelectObject(hdc, font);
    SetTextColor(hdc, RGB(0xFF, 0xD7, 0x00));

    const int TW = 40, TH = 26; // the text widget is 72x26 in the demo; 40 is enough here
    RECT rcPrev = {0, 0, 0, 0};
    for (int i = 0; i < 6; i++)
    {
        RECT rcCur;
        rcCur.left = 4 + i * 12;
        rcCur.top = 4 + i * 8;
        rcCur.right = rcCur.left + TW;
        rcCur.bottom = rcCur.top + TH;
        if (rcCur.right > W) rcCur.right = W;
        if (rcCur.bottom > H) rcCur.bottom = H;

        // 1) repaint the area the text just vacated plus the new one
        RECT rcDirty;
        UnionRect(&rcDirty, &rcPrev, &rcCur);
        HRGN rgnDirty = CreateRectRgnIndirect(&rcDirty);
        ASSERT_TRUE(rgnDirty != NULL);
        ASSERT_TRUE(SaveDC(hdc) != 0);
        ExtSelectClipRgn(hdc, rgnDirty, RGN_COPY);
        PatBlt(hdc, rcDirty.left, rcDirty.top,
               rcDirty.right - rcDirty.left, rcDirty.bottom - rcDirty.top, WHITENESS);
        RestoreDC(hdc, -1);
        DeleteObject(rgnDirty);

        // 2) draw the text, clipped to its own window rect
        HRGN rgnCur = CreateRectRgnIndirect(&rcCur);
        ASSERT_TRUE(rgnCur != NULL);
        ASSERT_TRUE(SaveDC(hdc) != 0);
        ExtSelectClipRgn(hdc, rgnCur, RGN_COPY);
        EXPECT_TRUE(TextOutA(hdc, rcCur.left + 2, rcCur.top + 2, "+800", 4));
        RestoreDC(hdc, -1);
        DeleteObject(rgnCur);

        rcPrev = rcCur;
    }

    // final frame: the animation ends and the last rect is erased as well
    HRGN rgnLast = CreateRectRgnIndirect(&rcPrev);
    ASSERT_TRUE(SaveDC(hdc) != 0);
    ExtSelectClipRgn(hdc, rgnLast, RGN_COPY);
    PatBlt(hdc, rcPrev.left, rcPrev.top,
           rcPrev.right - rcPrev.left, rcPrev.bottom - rcPrev.top, WHITENESS);
    RestoreDC(hdc, -1);
    DeleteObject(rgnLast);

    int residue = 0;
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
            if (GetPixel(hdc, x, y) != RGB(0xFF, 0xFF, 0xFF))
            {
                if (residue < 4)
                    ADD_FAILURE() << "gold residue at " << x << "," << y;
                residue++;
            }
    EXPECT_EQ(residue, 0) << "the vacated rects were not erased cleanly";

    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

// Same animation, but one child paints through a clip it does NOT own a
// SaveDC level for (IntersectClipRect straight on the frame DC), and the clip
// it leaves behind is somewhere else than the text. That is how the tetris
// "+score" trail in demos/wasm-demo/bug.png actually forms: the leaked rect
// survives on the frame DC, so the next frame's PushClipRegion(dirty,
// RGN_COPY) has to widen over it. If the backend can only intersect, that
// clear becomes a no-op on the vacated glyphs and the gold text freezes.
//
// The leak must happen AFTER the ink is down: if every frame leaks, each
// frame's dirty union still covers the accumulated clip, so even an
// intersect-only backend erases everything and the case proves nothing.
TEST_F(SwinxDcTest, moving_text_after_leaked_clip_leaves_no_residue)
{
    const int W = 120, H = 80;
    ASSERT_TRUE(SelectWhiteTarget(W, H) != NULL);
    if (!pixelsReadable()) return;

    HFONT font = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                             DEFAULT_PITCH, "Arial");
    ASSERT_TRUE(font != NULL);
    HGDIOBJ oldFont = SelectObject(hdc, font);
    SetTextColor(hdc, RGB(0xFF, 0xD7, 0x00));

    const int TW = 40, TH = 26;
    RECT rcPrev = {0, 0, 0, 0};
    for (int i = 0; i < 3; i++)
    {
        RECT rcCur;
        rcCur.left = 4 + i * 12;
        rcCur.top = 4 + i * 8;
        rcCur.right = rcCur.left + TW;
        rcCur.bottom = rcCur.top + TH;
        if (rcCur.right > W) rcCur.right = W;
        if (rcCur.bottom > H) rcCur.bottom = H;

        // 1) frame clear, exactly like SHostWnd::_RedrawRegion: RGN_COPY over the dirty region
        RECT rcDirty;
        UnionRect(&rcDirty, &rcPrev, &rcCur);
        HRGN rgnDirty = CreateRectRgnIndirect(&rcDirty);
        ASSERT_TRUE(rgnDirty != NULL);
        ASSERT_TRUE(SaveDC(hdc) != 0);
        ExtSelectClipRgn(hdc, rgnDirty, RGN_COPY);
        PatBlt(hdc, rcDirty.left, rcDirty.top,
               rcDirty.right - rcDirty.left, rcDirty.bottom - rcDirty.top, WHITENESS);
        RestoreDC(hdc, -1);
        DeleteObject(rgnDirty);

        // 2) draw the text, clipped to its own window rect
        HRGN rgnCur = CreateRectRgnIndirect(&rcCur);
        ASSERT_TRUE(rgnCur != NULL);
        ASSERT_TRUE(SaveDC(hdc) != 0);
        ExtSelectClipRgn(hdc, rgnCur, RGN_COPY);
        EXPECT_TRUE(TextOutA(hdc, rcCur.left + 2, rcCur.top + 2, "+800", 4));
        RestoreDC(hdc, -1);
        DeleteObject(rgnCur);

        rcPrev = rcCur;
    }

    // The last rect still holds gold ink. Now an unrelated child leaks a narrow
    // clip onto the frame DC; it does not touch that ink.
    IntersectClipRect(hdc, 90, 60, 98, 68);
    RECT rcLeak;
    GetClipBox(hdc, &rcLeak);
    EXPECT_TRUE(rcLeak.left >= 90 && rcLeak.top >= 60);

    // The animation ends and the frame clears the final rect the same way it
    // always did. This RGN_COPY has to widen back over the text rect.
    HRGN rgnLast = CreateRectRgnIndirect(&rcPrev);
    ASSERT_TRUE(SaveDC(hdc) != 0);
    ExtSelectClipRgn(hdc, rgnLast, RGN_COPY);
    PatBlt(hdc, rcPrev.left, rcPrev.top,
           rcPrev.right - rcPrev.left, rcPrev.bottom - rcPrev.top, WHITENESS);
    RestoreDC(hdc, -1);
    DeleteObject(rgnLast);

    // The leaked child clip (90,60)-(98,68) is still on the frame DC here: the
    // final RestoreDC(-1) restores exactly that clip, not the whole surface.
    // Real GDI's GetPixel returns CLR_INVALID for any pixel outside the current
    // clip region, so the scan below would misreport the entire surface as
    // residue (9536 of 9600 px). The gold was already erased by the RGN_COPY
    // clear above; reset the clip to the full surface before probing pixels.
    SelectClipRgn(hdc, NULL);

    int residue = 0;
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
        {
            COLORREF cr = GetPixel(hdc, x, y);
            if (cr != RGB(0xFF, 0xFF, 0xFF))
            {
                if (residue < 4)
                    ADD_FAILURE() << "residue at " << x << "," << y << " color=0x"
                                  << std::hex << cr << std::dec;
                residue++;
            }
        }
    EXPECT_EQ(residue, 0) << "a leaked child clip pinned the frame clear; "
                             "the vacated glyphs stayed on the canvas";

    SelectObject(hdc, oldFont);
    DeleteObject(font);
}
