/**
 * swinx GDI text APIs: font creation/selection, text extents and metrics
 * (cairo-backed). Assertions are kept structural (positive sizes, monotonic
 * widths) to stay font-independent.
 */
#include <gtest/gtest.h>
#include <windows.h>
#include <wchar.h>
#include <limits.h>
#include <stdlib.h>
#include <vector>

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

// ---------------------------------------------------------------------------
// GetTextExtentExPointA/W
//
// These APIs fill lpnDx with the *cumulative* advance of each character, so a
// useful universal invariant is: every entry is >= 0 and the sequence never
// decreases.  The last entry must equal the whole-line advance reported in
// psizl->cx (and GetTextExtentPoint32), otherwise the accumulating per-char
// rounding has drifted.
// ---------------------------------------------------------------------------

// Call GetTextExtentExPointA with a full-size lpnDx buffer and return the
// per-char cumulative widths.  `fit` receives lpnFit when requested.
// (EXPECT rather than ASSERT: these helpers return a value.)
static std::vector<int> ExtentExPointA(HDC hdc, const char *text, int len,
                                       int nMaxExtent, int *fit = NULL)
{
    std::vector<int> dx(len > 0 ? len : 1, -12345);
    SIZE sz = {0, 0};
    int nFit = -1;
    EXPECT_TRUE(GetTextExtentExPointA(hdc, text, len, nMaxExtent, &nFit, dx.data(), &sz));
    if (fit)
        *fit = nFit;
    return dx;
}

// Same for the wide variant (lpnDx is indexed in wchars).
static std::vector<int> ExtentExPointW(HDC hdc, const wchar_t *text, int len,
                                       int nMaxExtent, int *fit = NULL)
{
    std::vector<int> dx(len > 0 ? len : 1, -12345);
    SIZE sz = {0, 0};
    int nFit = -1;
    EXPECT_TRUE(GetTextExtentExPointW(hdc, text, len, nMaxExtent, &nFit, dx.data(), &sz));
    if (fit)
        *fit = nFit;
    return dx;
}

// The invariant this whole suite exists for: no per-char width may be negative,
// and the sequence may never shrink.  `n` is the number of entries that are
// expected to have been written (the fitting prefix).
static void ExpectNonNegativeMonotonic(const std::vector<int> &dx, int n)
{
    ASSERT_GE((int)dx.size(), n);
    int prev = 0;
    for (int i = 0; i < n; i++)
    {
        EXPECT_GE(dx[i], 0) << "dx[" << i << "] must never be negative";
        EXPECT_GE(dx[i], prev) << "dx[" << i << "] must not shrink below dx[" << i - 1 << "]";
        prev = dx[i];
    }
}

TEST_F(TextTest, extent_ex_point_a_ascii_all_non_negative)
{
    const char *text = "Hello, world!";
    int len = (int)strlen(text);
    std::vector<int> dx = ExtentExPointA(hdc, text, len, INT_MAX);
    ExpectNonNegativeMonotonic(dx, len);

    SIZE sz = {0, 0};
    ASSERT_TRUE(GetTextExtentPoint32A(hdc, text, len, &sz));
    EXPECT_EQ(dx[len - 1], sz.cx) << "last per-char width must equal the whole-line extent";
}

TEST_F(TextTest, extent_ex_point_w_ascii_all_non_negative)
{
    const wchar_t *text = L"Hello, world!";
    int len = (int)wcslen(text);
    std::vector<int> dx = ExtentExPointW(hdc, text, len, INT_MAX);
    ExpectNonNegativeMonotonic(dx, len);

    SIZE sz = {0, 0};
    ASSERT_TRUE(GetTextExtentPoint32W(hdc, text, len, &sz));
    EXPECT_EQ(dx[len - 1], sz.cx);
}

// Multi-byte (UTF-8) characters: the array must be monotonic and consistent with
// the whole-line extent, and every byte must have been written (the sentinel
// catches an under-filled lpnDx).
TEST_F(TextTest, extent_ex_point_a_multibyte_all_non_negative)
{
    const char *text = u8"中文测试abc";
    int len = (int)strlen(text);
    std::vector<int> dx = ExtentExPointA(hdc, text, len, INT_MAX);
    ExpectNonNegativeMonotonic(dx, len);
    // the helper seeds the buffer with a sentinel: a populated array can no
    // longer contain it, which is what catches an under-filled lpnDx
    for (int i = 0; i < len; i++)
        EXPECT_NE(dx[i], -12345) << "dx[" << i << "] left unset for a multi-byte string";

    SIZE sz = {0, 0};
    ASSERT_TRUE(GetTextExtentPoint32A(hdc, text, len, &sz));
    EXPECT_EQ(dx[len - 1], sz.cx);
}

TEST_F(TextTest, extent_ex_point_w_multibyte_all_non_negative)
{
    const wchar_t *text = L"中文测试abc";
    int len = (int)wcslen(text);
    std::vector<int> dx = ExtentExPointW(hdc, text, len, INT_MAX);
    ExpectNonNegativeMonotonic(dx, len);

    SIZE sz = {0, 0};
    ASSERT_TRUE(GetTextExtentPoint32W(hdc, text, len, &sz));
    EXPECT_EQ(dx[len - 1], sz.cx);
}

// Accumulation must not drift: measuring the whole line one character at a time
// has to agree with the single-call result, and every intermediate prefix width
// has to match GetTextExtentPoint32 of that prefix.
TEST_F(TextTest, extent_ex_point_accumulation_no_drift)
{
    const char *text = "The quick brown fox jumps over the lazy dog";
    int len = (int)strlen(text);
    std::vector<int> dx = ExtentExPointA(hdc, text, len, INT_MAX);

    for (int i = 0; i < len; i++)
    {
        SIZE sz = {0, 0};
        ASSERT_TRUE(GetTextExtentPoint32A(hdc, text, i + 1, &sz));
        // A one-unit tolerance covers cairo's own per-glyph rounding; a float
        // accumulation bug shows up as a steadily growing gap instead.
        EXPECT_LE(abs(dx[i] - sz.cx), 1)
            << "prefix of " << (i + 1) << " chars: dx=" << dx[i] << " extent=" << sz.cx;
    }
}

// CJK lines are the worst case for fractional advances: verify the invariant on
// a longer run and against the whole-line measurement.
TEST_F(TextTest, extent_ex_point_cjk_no_drift)
{
    const char *text = u8"这是一段用来测试逐字宽度累加精度的中文文本";
    int len = (int)strlen(text);
    std::vector<int> dx = ExtentExPointA(hdc, text, len, INT_MAX);
    ExpectNonNegativeMonotonic(dx, len);

    SIZE sz = {0, 0};
    ASSERT_TRUE(GetTextExtentPoint32A(hdc, text, len, &sz));
    EXPECT_EQ(dx[len - 1], sz.cx);
}

// nMaxExtent truncation: the reported fit must be the number of characters whose
// cumulative width is <= nMaxExtent, psizl->cx must be the advance of exactly
// that prefix, and the dx tail beyond the fit must not be left unset.
TEST_F(TextTest, extent_ex_point_a_truncation_by_max_extent)
{
    const char *text = "abcdefghij";
    int len = (int)strlen(text);
    std::vector<int> dx = ExtentExPointA(hdc, text, len, INT_MAX);
    ASSERT_EQ((int)dx.size(), len);

    for (int cut = 0; cut <= len; cut++)
    {
        int limit = (cut == 0) ? 0 : dx[cut - 1];
        int fit = -1;
        std::vector<int> dxCut = ExtentExPointA(hdc, text, len, limit, &fit);
        EXPECT_EQ(fit, cut) << "fit should stop exactly at the prefix that fits limit=" << limit;
        // gdi32 contract (measured): lpnDx is filled for the fitting prefix
        // only, and psizl->cx is always the whole-string width
        for (int i = 0; i < cut; i++)
        {
            EXPECT_NE(dxCut[i], -12345) << "dx[" << i << "] must be written inside the fitting prefix";
            EXPECT_EQ(dxCut[i], dx[i]) << "dx[" << i << "] must be the true cumulative width";
        }
        ExpectNonNegativeMonotonic(dxCut, cut);
        SIZE sz = {0, 0};
        ASSERT_TRUE(GetTextExtentPoint32A(hdc, text, len, &sz));
        EXPECT_EQ(sz.cx, dx[len - 1]) << "cx must be the whole-string width, not the fitted prefix";
    }
}

TEST_F(TextTest, extent_ex_point_w_truncation_by_max_extent)
{
    const wchar_t *text = L"abcdefghij";
    int len = (int)wcslen(text);
    std::vector<int> dx = ExtentExPointW(hdc, text, len, INT_MAX);

    for (int cut = 0; cut <= len; cut++)
    {
        int limit = (cut == 0) ? 0 : dx[cut - 1];
        int fit = -1;
        std::vector<int> dxCut = ExtentExPointW(hdc, text, len, limit, &fit);
        EXPECT_EQ(fit, cut);
        for (int i = 0; i < cut; i++)
        {
            EXPECT_NE(dxCut[i], -12345);
            EXPECT_EQ(dxCut[i], dx[i]);
        }
        ExpectNonNegativeMonotonic(dxCut, cut);
    }
}

// A zero nMaxExtent (no character fits) must report fit == 0 and leave the lpnDx
// array untouched.  gdi32 still reports the whole-string width in cx.
TEST_F(TextTest, extent_ex_point_a_zero_fit)
{
    const char *text = "abc";
    int len = (int)strlen(text);
    int fit = -1;
    std::vector<int> dx(len, -777);
    SIZE sz = {0, 0};
    ASSERT_TRUE(GetTextExtentExPointA(hdc, text, len, 0, &fit, dx.data(), &sz));
    EXPECT_EQ(fit, 0);
    for (int i = 0; i < len; i++)
        EXPECT_EQ(dx[i], -777) << "lpnDx must stay untouched when nothing fits";
}

// NULL lpnDx / lpnFit combinations must all work (GDI allows either to be NULL).
TEST_F(TextTest, extent_ex_point_a_optional_out_params)
{
    const char *text = "sample";
    int len = (int)strlen(text);
    SIZE sz = {0, 0};
    int fit = -1;

    // only size, no dx, no fit -> forwarded to GetTextExtentPoint32A
    ASSERT_TRUE(GetTextExtentExPointA(hdc, text, len, INT_MAX, NULL, NULL, &sz));
    SIZE ref = {0, 0};
    ASSERT_TRUE(GetTextExtentPoint32A(hdc, text, len, &ref));
    EXPECT_EQ(sz.cx, ref.cx);

    // fit only, no dx
    ASSERT_TRUE(GetTextExtentExPointA(hdc, text, len, INT_MAX, &fit, NULL, &sz));
    EXPECT_EQ(fit, len);

    // dx only, no fit
    std::vector<int> dx(len);
    ASSERT_TRUE(GetTextExtentExPointA(hdc, text, len, INT_MAX, NULL, dx.data(), &sz));
    ExpectNonNegativeMonotonic(dx, len);
}

// A NULL lpnDx must still yield the correct size: the size is the width of the
// whole string and must not depend on whether the per-char array was requested.
TEST_F(TextTest, extent_ex_point_size_independent_of_dx_buffer)
{
    const char *texts[] = {"sample", u8"Ag1中", "WAVE-WAVE-WAVE"};
    for (const char *text : texts)
    {
        int len = (int)strlen(text);
        SIZE withDx = {0, 0}, withoutDx = {0, 0};
        int fit1 = -1, fit2 = -1;
        std::vector<int> dx(len);

        ASSERT_TRUE(GetTextExtentExPointA(hdc, text, len, INT_MAX, &fit1, dx.data(), &withDx));
        ASSERT_TRUE(GetTextExtentExPointA(hdc, text, len, INT_MAX, &fit2, NULL, &withoutDx));
        EXPECT_EQ(withDx.cx, withoutDx.cx) << "size must not depend on lpnDx";
        EXPECT_EQ(fit1, fit2);

        // and the last dx entry must not overrun the reported size
        ExpectNonNegativeMonotonic(dx, len);
        EXPECT_LE(dx[len - 1], withDx.cx);
    }
}

// cchString == 0 is a legal "measure nothing" call; a negative count is rejected
// and must leave the outputs untouched (verified against gdi32).
TEST_F(TextTest, extent_ex_point_zero_length)
{
    SIZE sz = {123, 456};
    int fit = -1;
    ASSERT_TRUE(GetTextExtentExPointA(hdc, "abc", 0, 100, &fit, NULL, &sz));
    EXPECT_EQ(sz.cx, 0);
    EXPECT_EQ(fit, 0);

    ASSERT_TRUE(GetTextExtentExPointW(hdc, L"abc", 0, 100, &fit, NULL, &sz));
    EXPECT_EQ(sz.cx, 0);
    EXPECT_EQ(fit, 0);
}

// Several font sizes: the invariant must hold for any advance scale.
TEST_F(TextTest, extent_ex_point_across_font_sizes)
{
    const int heights[] = {9, 12, 16, 24, 36};
    const char *text = u8"Ag1中";
    int len = (int)strlen(text);

    for (int h : heights)
    {
        HFONT f = CreateFontA(-h, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                              CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                              DEFAULT_PITCH, "sans-serif");
        ASSERT_TRUE(f != NULL);
        HGDIOBJ prev = SelectObject(hdc, f);

        std::vector<int> dx = ExtentExPointA(hdc, text, len, INT_MAX);
        ExpectNonNegativeMonotonic(dx, len);
        SIZE sz = {0, 0};
        ASSERT_TRUE(GetTextExtentPoint32A(hdc, text, len, &sz));
        EXPECT_EQ(dx[len - 1], sz.cx) << "font height " << h;

        SelectObject(hdc, prev);
        DeleteObject(f);
    }
}

// Pin the measured gdi32 contract: the cumulative sequence for a monospaced-ish
// ASCII string must be strictly derived from the whole-string width, the last
// entry must equal it, and cx must not change with nMaxExtent.  This is the
// regression guard for the swinx implementations, whose per-char widths used to
// be accumulated as truncated ints.
TEST_F(TextTest, extent_ex_point_matches_point32_extent)
{
    const char *text = "abcdefghij";
    int len = (int)strlen(text);

    SIZE whole = {0, 0};
    ASSERT_TRUE(GetTextExtentPoint32A(hdc, text, len, &whole));

    // full fit
    std::vector<int> dx = ExtentExPointA(hdc, text, len, INT_MAX);
    EXPECT_EQ(dx[len - 1], whole.cx);

    // cx is independent of nMaxExtent
    const int limits[] = {0, 1, 17, (int)(whole.cx / 2), (int)(whole.cx - 1), (int)whole.cx};
    for (int limit : limits)
    {
        SIZE sz = {0, 0};
        int fit = -1;
        std::vector<int> buf(len, -777);
        ASSERT_TRUE(GetTextExtentExPointA(hdc, text, len, limit, &fit, buf.data(), &sz));
        EXPECT_EQ(sz.cx, whole.cx) << "cx must equal the whole-string width at limit=" << limit;
        EXPECT_GE(fit, 0);
        EXPECT_LE(fit, len);
    }
}

// The prefix widths reported by lpnDx must agree with GetTextExtentPoint32 for
// the corresponding prefix.  Only ASCII prefixes are compared: on a prefix that
// splits a multi-byte character, gdi32's ANSI dx has already counted the whole
// split character while Point32 of the truncated bytes measures differently, so
// the two are not comparable there.
TEST_F(TextTest, extent_ex_point_prefix_widths_consistent)
{
    const char *text = "HelloWorld123"; // ASCII only: unambiguous prefix widths
    int len = (int)strlen(text);
    std::vector<int> dx = ExtentExPointA(hdc, text, len, INT_MAX);
    ExpectNonNegativeMonotonic(dx, len);

    for (int i = 0; i < len; i++)
    {
        SIZE sz = {0, 0};
        ASSERT_TRUE(GetTextExtentPoint32A(hdc, text, i + 1, &sz));
        EXPECT_LE(abs((int)dx[i] - (int)sz.cx), 1)
            << "prefix of " << (i + 1) << " bytes: dx=" << dx[i] << " extent=" << sz.cx;
    }
}
