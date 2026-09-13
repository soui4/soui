/**
 * swinx GDI text APIs: font creation/selection, text extents and metrics
 * (cairo-backed). Assertions are kept structural (positive sizes, monotonic
 * widths) to stay font-independent.
 */
#include <gtest/gtest.h>
#include <windows.h>

class TextTest : public ::testing::Test {
protected:
    TextTest()
        : hdc(NULL)
        , font(NULL)
        , oldFont(NULL)
    {}
    void SetUp() override
    {
        hdc = CreateCompatibleDC(NULL);
        ASSERT_NE(hdc, (HDC)NULL);

        font = CreateFontA(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                           CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                           DEFAULT_PITCH, "sans-serif");
        ASSERT_TRUE(font != NULL);

        oldFont = SelectObject(hdc, font);
        ASSERT_TRUE(oldFont != NULL);
    }

    void TearDown() override
    {
        SelectObject(hdc, oldFont);
        DeleteObject(font);
        DeleteDC(hdc);
    }

    HDC hdc;
    HFONT font;
    HGDIOBJ oldFont;
};

TEST_F(TextTest, get_text_extent_basic)
{
    SIZE sz = {0, 0};
    ASSERT_TRUE(GetTextExtentPoint32A(hdc, "hello", 5, &sz));
    EXPECT_GT(sz.cx, 0);
    EXPECT_GT(sz.cy, 0);
}

TEST_F(TextTest, extent_grows_with_length)
{
    SIZE s1 = {0, 0}, s2 = {0, 0};
    ASSERT_TRUE(GetTextExtentPoint32A(hdc, "hello", 5, &s1));
    ASSERT_TRUE(GetTextExtentPoint32A(hdc, "hello world", 11, &s2));
    EXPECT_GT(s2.cx, s1.cx);
    // line height does not depend on the string
    EXPECT_EQ(s2.cy, s1.cy);
}

TEST_F(TextTest, extent_zero_length)
{
    SIZE sz = {0, 0};
    ASSERT_TRUE(GetTextExtentPoint32A(hdc, "abc", 0, &sz));
    EXPECT_EQ(sz.cx, 0);
}

TEST_F(TextTest, get_text_metrics)
{
    TEXTMETRICA tm;
    memset(&tm, 0, sizeof(tm));
    ASSERT_TRUE(GetTextMetricsA(hdc, &tm));
    EXPECT_GT(tm.tmHeight, 0);
    EXPECT_GT(tm.tmAscent, 0);
    EXPECT_GT(tm.tmDescent, 0);
    // ascent + descent should account for the full line height
    EXPECT_GE(tm.tmAscent + tm.tmDescent, tm.tmHeight - 1);
}

TEST_F(TextTest, select_object_restores)
{
    HFONT f2 = CreateFontA(-24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                           CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                           DEFAULT_PITCH, "sans-serif");
    ASSERT_TRUE(f2 != NULL);

    HGDIOBJ prev = SelectObject(hdc, f2);
    EXPECT_EQ(prev, (HGDIOBJ)font);
    EXPECT_EQ(GetCurrentObject(hdc, OBJ_FONT), (HGDIOBJ)f2);

    SelectObject(hdc, prev);
    EXPECT_EQ(GetCurrentObject(hdc, OBJ_FONT), (HGDIOBJ)font);

    DeleteObject(f2);
}
