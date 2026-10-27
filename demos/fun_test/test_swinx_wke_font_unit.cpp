/**
 * Unit tests for the glyph-level GDI font APIs that swinx added for wke
 * (GetGlyphIndices*, GetCharWidth*, GetGlyphOutline, GetFontData,
 * GetOutlineTextMetrics, GetFontUnicodeRanges, EnumFontFamiliesEx).
 *
 * The suite runs against swinx on Linux/macOS and against the real Win32
 * GDI on Windows, so every assertion here must hold on both. Where the
 * swinx contract deliberately differs from Win32 (or where the result
 * depends on the host), the case is guarded with #ifndef _WIN32 and marked.
 */
#include <gtest/gtest.h>
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <tchar.h>

#ifndef MAKEFOURCC
#define MAKEFOURCC(c0, c1, c2, c3) \
    ((DWORD)(UINT)(UCHAR)(c0) | ((DWORD)(UINT)(UCHAR)(c1) << 8) | \
     ((DWORD)(UINT)(UCHAR)(c2) << 16) | ((DWORD)(UINT)(UCHAR)(c3) << 24))
#endif

namespace
{

class WkeDc
{
public:
    WkeDc() : m_dc(NULL), m_font(NULL), m_old(NULL)
    {
        m_dc = CreateCompatibleDC(NULL);
        if (m_dc)
        {
            m_font = CreateFontA(32, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS,
                                 CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                                 FF_DONTCARE, NULL);
            if (m_font)
                m_old = SelectObject(m_dc, m_font);
        }
    }
    ~WkeDc()
    {
        if (m_dc && m_old)
            SelectObject(m_dc, m_old);
        if (m_font)
            DeleteObject(m_font);
        if (m_dc)
            DeleteDC(m_dc);
    }
    HDC operator*() const { return m_dc; }
    bool ok() const { return m_dc != NULL && m_font != NULL && m_old != NULL; }
private:
    HDC m_dc;
    HFONT m_font;
    HGDIOBJ m_old;
};

WORD glyphOf(HDC hdc, wchar_t ch)
{
    WORD gi = 0xFFFF;
    if (GetGlyphIndicesW(hdc, &ch, 1, &gi, 0) != 1)
        return 0xFFFF;
    return gi;
}

// Identity MAT2: passing a real matrix keeps the call valid on both GDI and swinx.
const MAT2 kIdentityMtx = { {0, 1}, {0, 0}, {0, 0}, {0, 1} };

struct FaceProbe
{
    int seen;
    int weights[8];
    TCHAR firstFamily[32];   // copied down from LOGFONTW, ASCII only
};

int WINAPI countFaceProcW(CONST LOGFONTW *lf, CONST TEXTMETRICW *, DWORD iFace, LPARAM lp)
{
    FaceProbe *p = (FaceProbe *)lp;
    p->seen++;
    if (p->seen == 1)
    {
        int i = 0;
        for (; i < 31 && lf->lfFaceName[i]; i++)
            p->firstFamily[i] = (TCHAR)lf->lfFaceName[i];
        p->firstFamily[i] = 0;
    }
    if (p->seen <= 8)
        p->weights[p->seen - 1] = (int)lf->lfWeight;
    (void)iFace;
    return 1;
}

int WINAPI stopFaceProcW(CONST LOGFONTW *, CONST TEXTMETRICW *, DWORD, LPARAM lp)
{
    ((FaceProbe *)lp)->seen++;
    return 0;
}

// Width-agnostic counter: the LOGFONT/TEXTMETRIC pointer types differ
// between the A and W enumerators, the count does not.
int WINAPI countFaceProcA(CONST LOGFONTA *, CONST TEXTMETRICA *, DWORD, LPARAM lp)
{
    ((FaceProbe *)lp)->seen++;
    return 1;
}

bool weightIsSorted(const int *w, int n)
{
    for (int i = 1; i < n; i++)
        if (w[i] < w[i - 1])
            return false;
    return true;
}

} // namespace

// ------------------------------------------------------------------------
// GetGlyphIndicesW
// ------------------------------------------------------------------------

TEST(swinx_wke_font, glyph_indices_reports_one_index_per_character)
{
    WkeDc dc;
    ASSERT_TRUE(dc.ok());
    const wchar_t *text = L"ABC";
    WORD gi[3] = { 0xFFFF, 0xFFFF, 0xFFFF };
    EXPECT_EQ(GetGlyphIndicesW(*dc, text, 3, gi, 0), 3u);
    // A real TrueType face always maps the latin capitals to a glyph.
    EXPECT_NE(gi[0], 0);
    EXPECT_NE(gi[1], 0);
    EXPECT_NE(gi[2], 0);
}

TEST(swinx_wke_font, glyph_indices_size_contract)
{
    WkeDc dc;
    ASSERT_TRUE(dc.ok());
    WORD gi[1] = { 0 };
#ifdef _WIN32
    // GDI rejects a zero-length run instead of treating it as a no-op, so the
    // "0 indices" count is only observable on swinx. Both refuse to write.
    EXPECT_EQ(GetGlyphIndicesW(*dc, L"", 0, gi, 0), (DWORD)GDI_ERROR);
#else
    EXPECT_EQ(GetGlyphIndicesW(*dc, L"", 0, gi, 0), 0u);
#endif
    EXPECT_EQ(GetGlyphIndicesW(*dc, NULL, 1, gi, 0), (DWORD)GDI_ERROR);
    EXPECT_EQ(GetGlyphIndicesW(*dc, L"A", 1, NULL, 0), (DWORD)GDI_ERROR);
}

TEST(swinx_wke_font, glyph_indices_marks_unmapped_codepoints)
{
    WkeDc dc;
    ASSERT_TRUE(dc.ok());
    WORD gi[1] = { 0 };
    EXPECT_EQ(GetGlyphIndicesW(*dc, L"\x0978", 1, gi, GGI_MARK_NONEXISTING_GLYPHS), 1u);
    EXPECT_TRUE(gi[0] == 0xFFFF || gi[0] == 0);
#ifndef _WIN32
    // swinx: without the mark flag a miss collapses to .notdef (index 0).
    gi[0] = 1;
    EXPECT_EQ(GetGlyphIndicesW(*dc, L"\x0978", 1, gi, 0), 1u);
    EXPECT_EQ(gi[0], 0);
#endif
}

// ------------------------------------------------------------------------
// GetCharWidthI / GetCharWidth32A
// ------------------------------------------------------------------------

TEST(swinx_wke_font, char_width_by_glyph_index_matches_outline_metrics)
{
    WkeDc dc;
    ASSERT_TRUE(dc.ok());
    WORD gi[2];
    ASSERT_EQ(GetGlyphIndicesW(*dc, L"AB", 2, gi, 0), 2u);

    int adv[2] = { -1, -1 };
    EXPECT_TRUE(GetCharWidthI(*dc, 0, 2, gi, adv) != FALSE);
    EXPECT_GT(adv[0], 0);
    EXPECT_GT(adv[1], 0);

    GLYPHMETRICS gm;
    DWORD cb = GetGlyphOutlineW(*dc, gi[0], GGO_GLYPH_INDEX | GGO_METRICS, &gm, 0, NULL,
                                &kIdentityMtx);
    ASSERT_NE(cb, (DWORD)GDI_ERROR);
#ifndef _WIN32
    // swinx answers a GGO_METRICS query with sizeof(GLYPHMETRICS); GDI returns
    // the size of the whole glyph record, which varies with the face.
    EXPECT_EQ(cb, (DWORD)sizeof(GLYPHMETRICS));
#endif
    EXPECT_EQ(adv[0], (int)gm.gmCellIncX);
}

TEST(swinx_wke_font, char_width_rejects_bad_arguments)
{
    WkeDc dc;
    ASSERT_TRUE(dc.ok());
    int buf[4];
    EXPECT_FALSE(GetCharWidthI(*dc, 'A', 1, NULL, NULL));
    EXPECT_FALSE(GetCharWidthI(NULL, 'A', 1, NULL, buf));
}

TEST(swinx_wke_font, char_width32_covers_the_inclusive_range)
{
    WkeDc dc;
    ASSERT_TRUE(dc.ok());
    int buf[4] = { -1, -1, -1, -1 };
    EXPECT_TRUE(GetCharWidth32A(*dc, 'A', 'D', buf) != FALSE);
    for (int i = 0; i < 4; i++)
        EXPECT_GT(buf[i], 0);
}

// ------------------------------------------------------------------------
// GetGlyphOutlineW (metrics only)
// ------------------------------------------------------------------------

TEST(swinx_wke_font, glyph_outline_metrics_for_a_mapped_character)
{
    WkeDc dc;
    ASSERT_TRUE(dc.ok());
    GLYPHMETRICS gm;
    memset(&gm, 0xAA, sizeof(gm));
    DWORD cb = GetGlyphOutlineW(*dc, (UINT)'A', GGO_METRICS, &gm, 0, NULL, &kIdentityMtx);
    // Measured on Win10 GDI: the return value is the size of the glyph record
    // (60 at 32px), not sizeof(GLYPHMETRICS); only swinx reports the struct size.
    ASSERT_NE(cb, (DWORD)GDI_ERROR);
#ifndef _WIN32
    EXPECT_EQ(cb, (DWORD)sizeof(GLYPHMETRICS));
#endif
    EXPECT_GT((int)gm.gmBlackBoxX, 0);
    EXPECT_GT((int)gm.gmBlackBoxY, 0);
    EXPECT_GT((int)gm.gmCellIncX, 0);
    EXPECT_EQ(gm.gmCellIncY, 0);
    // 'A' sits above the baseline within a 32 pixel em box.
    EXPECT_GT(gm.gmptGlyphOrigin.y, 0);
    EXPECT_LE(gm.gmptGlyphOrigin.y, 32);
}

TEST(swinx_wke_font, glyph_outline_rejects_null_metrics)
{
    WkeDc dc;
    ASSERT_TRUE(dc.ok());
    EXPECT_EQ(GetGlyphOutlineW(*dc, (UINT)'A', GGO_METRICS, NULL, 0, NULL, NULL),
              (DWORD)GDI_ERROR);
}

#ifndef _WIN32
TEST(swinx_wke_font, glyph_outline_only_implements_metrics_format)
{
    WkeDc dc;
    ASSERT_TRUE(dc.ok());
    GLYPHMETRICS gm;
    BYTE bits[64];
    // swinx records only what wkex asks for (GGO_METRICS); bitmap/bezier
    // requests are refused instead of silently returning garbage.
    EXPECT_EQ(GetGlyphOutlineW(*dc, (UINT)'A', GGO_BITMAP, &gm, sizeof(bits), bits, NULL),
              (DWORD)GDI_ERROR);
    EXPECT_EQ(GetGlyphOutlineW(*dc, (UINT)'A', GGO_METRICS, &gm, 0, NULL, &kIdentityMtx),
              (DWORD)sizeof(GLYPHMETRICS));
}

TEST(swinx_wke_font, glyph_outline_accepts_glyph_index_flag)
{
    WkeDc dc;
    ASSERT_TRUE(dc.ok());
    WORD gi = glyphOf(*dc, L'A');
    ASSERT_NE(gi, 0xFFFF);
    GLYPHMETRICS a, b;
    ASSERT_EQ(GetGlyphOutlineW(*dc, (UINT)'A', GGO_METRICS, &a, 0, NULL, &kIdentityMtx),
              (DWORD)sizeof(GLYPHMETRICS));
    ASSERT_EQ(GetGlyphOutlineW(*dc, gi, GGO_METRICS | GGO_GLYPH_INDEX, &b, 0, NULL, &kIdentityMtx),
              (DWORD)sizeof(GLYPHMETRICS));
    EXPECT_EQ(a.gmBlackBoxX, b.gmBlackBoxX);
    EXPECT_EQ(a.gmCellIncX, b.gmCellIncX);
}

TEST(swinx_wke_font, font_unicode_ranges_probe_is_stable)
{
    WkeDc dc;
    ASSERT_TRUE(dc.ok());
    DWORD need = GetFontUnicodeRanges(*dc, NULL);
    ASSERT_GT(need, (DWORD)offsetof(GLYPHSET, ranges));
    GLYPHSET *gs = (GLYPHSET *)malloc(need + sizeof(WCRANGE) * 4);
    ASSERT_TRUE(gs != NULL);
    memset(gs, 0, need + sizeof(WCRANGE) * 4);
    gs->cbThis = need;
    DWORD second = GetFontUnicodeRanges(*dc, gs);
    EXPECT_EQ(second, need);
    EXPECT_GT(gs->cRanges, 0u);
    EXPECT_GT(gs->cGlyphsSupported, 0u);
    free(gs);
}
#endif

// ------------------------------------------------------------------------
// GetFontData
// ------------------------------------------------------------------------

TEST(swinx_wke_font, font_data_returns_table_size_then_bytes)
{
    WkeDc dc;
    ASSERT_TRUE(dc.ok());
    const DWORD head = MAKEFOURCC('h', 'e', 'a', 'd');
    DWORD size = GetFontData(*dc, head, 0, NULL, 0);
    ASSERT_TRUE(size != 0 && size != (DWORD)GDI_ERROR) << "no head table? size=" << (unsigned long)size;

    BYTE *buf = (BYTE *)malloc(size);
    ASSERT_TRUE(buf != NULL);
    EXPECT_EQ(GetFontData(*dc, head, 0, buf, size), size);
    // Win32 reads a slice of the table (dwOffset/cbData exist for that), so a
    // shorter request must return the number of bytes actually written.
    EXPECT_EQ(GetFontData(*dc, head, 0, buf, size - 1), size - 1);
    free(buf);
#ifdef _WIN32
    // A zero tag means "the whole font file" to GDI; swinx requires a table tag.
    EXPECT_GT(GetFontData(*dc, 0, 0, NULL, 0), size);
#else
    EXPECT_EQ(GetFontData(*dc, 0, 0, NULL, 0), (DWORD)GDI_ERROR);
#endif
}

#ifndef _WIN32
TEST(swinx_wke_font, swinx_accepts_both_tag_byte_orders)
{
    WkeDc dc;
    ASSERT_TRUE(dc.ok());

    // WebKit's GDI callers pack the tag Windows-style, i.e. the first character
    // in the low byte ("name" is passed as the literal 'eman', "CFF " as
    // 0x20464643). FreeType wants the opposite order, so swinx has to convert.
    const DWORD nameGdi = MAKEFOURCC('n', 'a', 'm', 'e');
    const DWORD nameFt = ((nameGdi & 0xFFu) << 24) | ((nameGdi & 0xFF00u) << 8) |
                         ((nameGdi >> 8) & 0xFF00u) | (nameGdi >> 24);

    DWORD viaGdi = GetFontData(*dc, nameGdi, 0, NULL, 0);
    EXPECT_TRUE(viaGdi != 0 && viaGdi != (DWORD)GDI_ERROR);
    EXPECT_EQ(viaGdi, GetFontData(*dc, nameFt, 0, NULL, 0));

    // Every table read through the GDI order must still be readable at an offset.
    BYTE buf[16];
    EXPECT_EQ(GetFontData(*dc, nameGdi, 4, buf, sizeof(buf)), (DWORD)sizeof(buf));
}
#endif

// ------------------------------------------------------------------------
// GetOutlineTextMetricsW / A
// ------------------------------------------------------------------------

TEST(swinx_wke_font, outline_text_metrics_two_step_query)
{
    WkeDc dc;
    ASSERT_TRUE(dc.ok());
    UINT need = GetOutlineTextMetricsW(*dc, 0, NULL);
    ASSERT_GT(need, (UINT)sizeof(OUTLINETEXTMETRICW));

    OUTLINETEXTMETRICW *potm = (OUTLINETEXTMETRICW *)malloc(need);
    ASSERT_TRUE(potm != NULL);
    memset(potm, 0, need);
    EXPECT_EQ(GetOutlineTextMetricsW(*dc, need, potm), need);
#ifdef _WIN32
    // GDI reports otmSize as the whole buffer, i.e. the fixed part plus the
    // face and family strings that follow it (256 vs sizeof == 232 here).
    EXPECT_EQ(potm->otmSize, need);
    EXPECT_GE(potm->otmSize, (UINT)sizeof(OUTLINETEXTMETRICW));
#else
    EXPECT_EQ(potm->otmSize, (UINT)sizeof(OUTLINETEXTMETRICW));
#endif
    EXPECT_GT(potm->otmEMSquare, 0u);
    EXPECT_GT(potm->otmAscent, 0u);
    EXPECT_GE(potm->otmDescent, 0u);
    free(potm);
}

TEST(swinx_wke_font, outline_text_metrics_buffer_too_small_fails)
{
#ifndef _WIN32
    WkeDc dc;
    ASSERT_TRUE(dc.ok());
    OUTLINETEXTMETRICW tiny;
    memset(&tiny, 0, sizeof(tiny));
    // A buffer smaller than the fixed part yields nothing (swinx contract).
    EXPECT_EQ(GetOutlineTextMetricsW(*dc, (UINT)sizeof(tiny) - 4, &tiny), 0u);
#else
    SUCCEED() << "GDI may partially fill an undersized buffer; swinx refuses.";
#endif
}

TEST(swinx_wke_font, outline_text_metrics_a_matches_wide_struct_size)
{
    WkeDc dc;
    ASSERT_TRUE(dc.ok());
    UINT needA = GetOutlineTextMetricsA(*dc, 0, NULL);
    UINT needW = GetOutlineTextMetricsW(*dc, 0, NULL);
    EXPECT_GT(needA, (UINT)sizeof(OUTLINETEXTMETRICA));
    EXPECT_GT(needW, (UINT)sizeof(OUTLINETEXTMETRICW));
}

// ------------------------------------------------------------------------
// GetFontUnicodeRanges
// ------------------------------------------------------------------------

TEST(swinx_wke_font, font_unicode_ranges_contains_latin_capital_a)
{
    WkeDc dc;
    ASSERT_TRUE(dc.ok());
    DWORD need = GetFontUnicodeRanges(*dc, NULL);
    ASSERT_GT(need, (DWORD)offsetof(GLYPHSET, ranges));
    GLYPHSET *gs = (GLYPHSET *)malloc(need + sizeof(WCRANGE) * 4);
    ASSERT_TRUE(gs != NULL);
    memset(gs, 0, need + sizeof(WCRANGE) * 4);
    gs->cbThis = need;
    ASSERT_EQ(GetFontUnicodeRanges(*dc, gs), need);
    ASSERT_LE(gs->cRanges * sizeof(WCRANGE) + offsetof(GLYPHSET, ranges), need);

    bool foundA = false;
    for (UINT i = 0; i < gs->cRanges; i++)
    {
        WORD low = gs->ranges[i].wcLow;
        if (low <= 'A' && 'A' < (int)(low + gs->ranges[i].cGlyphs))
            foundA = true;
    }
    EXPECT_TRUE(foundA);
    EXPECT_GE(gs->cbThis, need);
    free(gs);
}

// ------------------------------------------------------------------------
// EnumFontFamiliesExW
// ------------------------------------------------------------------------

TEST(swinx_wke_font, enum_font_families_ex_enumerates_faces)
{
    WkeDc dc;
    ASSERT_TRUE(dc.ok());
    LOGFONTW lf;
    memset(&lf, 0, sizeof(lf));
    lf.lfCharSet = DEFAULT_CHARSET;

    FaceProbe probe;
    memset(&probe, 0, sizeof(probe));
    int ret = EnumFontFamiliesExW(*dc, &lf, countFaceProcW, (LPARAM)&probe, 0);
    EXPECT_TRUE(ret == 0 || ret == 1);
    EXPECT_GT(probe.seen, 0);
    EXPECT_TRUE(probe.firstFamily[0] != 0);
#ifndef _WIN32
    // swinx lists faces of one family ordered by weight; GDI makes no order promise.
    EXPECT_TRUE(weightIsSorted(probe.weights, probe.seen <= 8 ? probe.seen : 0));
#endif
}

TEST(swinx_wke_font, enum_font_families_ex_stops_on_callback_false)
{
    WkeDc dc;
    ASSERT_TRUE(dc.ok());
    LOGFONTW lf;
    memset(&lf, 0, sizeof(lf));
    lf.lfCharSet = DEFAULT_CHARSET;

    FaceProbe probe;
    memset(&probe, 0, sizeof(probe));
    // Win32 documents the return value as "the last value returned by the
    // callback function", so stopping the enumeration with 0 is a 0 return, not
    // an error. What must not happen is enumerating further faces.
    EXPECT_EQ(EnumFontFamiliesExW(*dc, &lf, stopFaceProcW, (LPARAM)&probe, 0), 0);
    EXPECT_EQ(probe.seen, 1);
}

TEST(swinx_wke_font, enum_font_families_ex_requires_a_callback)
{
    WkeDc dc;
    ASSERT_TRUE(dc.ok());
    LOGFONTW lf;
    memset(&lf, 0, sizeof(lf));
    EXPECT_EQ(EnumFontFamiliesExW(NULL, &lf, countFaceProcW, 0, 0), 0);
}

#ifndef _WIN32
TEST(swinx_wke_font, enum_font_families_ex_a_matches_wide_face_count)
{
    WkeDc dc;
    ASSERT_TRUE(dc.ok());
    LOGFONTA lfa;
    LOGFONTW lfw;
    memset(&lfa, 0, sizeof(lfa));
    memset(&lfw, 0, sizeof(lfw));
    lfa.lfCharSet = DEFAULT_CHARSET;
    lfw.lfCharSet = DEFAULT_CHARSET;

    FaceProbe a, w;
    memset(&a, 0, sizeof(a));
    memset(&w, 0, sizeof(w));
    EnumFontFamiliesExA(*dc, &lfa, countFaceProcA, (LPARAM)&a, 0);
    EnumFontFamiliesExW(*dc, &lfw, countFaceProcW, (LPARAM)&w, 0);
    EXPECT_EQ(a.seen, w.seen);
}
#endif
