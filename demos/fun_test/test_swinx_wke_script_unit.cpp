/**
 * Unit tests for the Uniscribe (usp10) and enhanced-metafile APIs swinx
 * added for wke. wke's WebKit port asks "which font will actually draw this
 * string" through ScriptStringAnalyse(SSA_FALLBACK|SSA_METAFILE) +
 * ScriptStringOut, then replays the recorded EMF; these tests cover that
 * path plus the itemize/shape/place/xcP primitives around it.
 *
 * Cases that only hold on swinx (deliberate contract choices, or behaviour
 * that depends on the host font stack) are guarded with #ifndef _WIN32.
 */
#include <gtest/gtest.h>
#include <windows.h>
#include <usp10.h>
#include <string.h>

namespace
{

class WkeScriptDc
{
public:
    WkeScriptDc() : m_dc(NULL), m_font(NULL), m_old(NULL)
    {
        m_dc = CreateCompatibleDC(NULL);
        if (m_dc)
        {
            m_font = CreateFontA(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS,
                                 CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                                 FF_DONTCARE, NULL);
            if (m_font)
                m_old = SelectObject(m_dc, m_font);
        }
    }
    ~WkeScriptDc()
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

struct EmfFontProbe
{
    int records;
    int fontRecords;
    int matched;
    char expect[32];
    WCHAR lastFace[32];   // face name of the last font record, so two string
                          //口径 of the same text can be compared directly
};

static bool sameFace(const WCHAR *a, const WCHAR *b)
{
    for (int i = 0; i < 32; ++i)
    {
        if (a[i] != b[i])
            return false;
        if (a[i] == 0)
            return true;
    }
    return true;
}

int WINAPI emfEnumProc(HDC, LPHANDLETABLE, CONST ENHMETARECORD *pRecord, int, LPARAM lp)
{
    EmfFontProbe *p = (EmfFontProbe *)lp;
    p->records++;
    if (pRecord->iType == EMR_EXTCREATEFONTINDIRECTW)
    {
        const EMREXTCREATEFONTINDIRECTW *rec =
            (const EMREXTCREATEFONTINDIRECTW *)pRecord;
        p->fontRecords++;
        const WCHAR *face = rec->elfw.elfLogFont.lfFaceName;
        int i = 0;
        for (; i < 31 && face[i]; ++i)
            p->lastFace[i] = face[i];
        p->lastFace[i] = 0;
        if (face[0])
        {
            // LOGFONTW face name: compare the first ASCII character only,
            // WCHAR width differs between swinx (4) and Win32 (2).
            if (p->expect[0] == 0 ||
                (char)face[0] == p->expect[0])
                p->matched++;
        }
    }
    return TRUE;
}

int WINAPI emfStopProc(HDC, LPHANDLETABLE, CONST ENHMETARECORD *, int, LPARAM lp)
{
    (*(int *)lp)++;
    return FALSE;
}

HFONT makeFont(const char *face, int height)
{
    return CreateFontA(height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                       DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS,
                       ANTIALIASED_QUALITY, FF_DONTCARE, face);
}

} // namespace

// ------------------------------------------------------------------------
// Enhanced metafile: record a font selection, replay it
// ------------------------------------------------------------------------

TEST(swinx_wke_emf, records_selected_font_and_replays_it)
{
    HDC ref = CreateCompatibleDC(NULL);
    ASSERT_TRUE(ref != NULL);
    HDC rec = CreateEnhMetaFileW(ref, NULL, NULL, NULL);
    ASSERT_TRUE(rec != NULL);

    HFONT font = makeFont("Arial", 20);
    ASSERT_TRUE(font != NULL);
    SelectObject(rec, font);

    HENHMETAFILE hmf = CloseEnhMetaFile(rec);
    ASSERT_TRUE(hmf != NULL);

    EmfFontProbe probe;
    memset(&probe, 0, sizeof(probe));
    probe.expect[0] = 'A';
    EXPECT_TRUE(EnumEnhMetaFile(NULL, hmf, emfEnumProc, &probe, NULL) != FALSE);
    EXPECT_GE(probe.fontRecords, 1);
    EXPECT_GE(probe.matched, 1);

    EXPECT_TRUE(DeleteEnhMetaFile(hmf) != FALSE);
    DeleteObject(font);
    DeleteDC(ref);
}

TEST(swinx_wke_emf, enum_stops_when_callback_returns_false)
{
    HDC ref = CreateCompatibleDC(NULL);
    ASSERT_TRUE(ref != NULL);
    HDC rec = CreateEnhMetaFileW(ref, NULL, NULL, NULL);
    ASSERT_TRUE(rec != NULL);
    HFONT font = makeFont("Arial", 20);
    ASSERT_TRUE(font != NULL);
    SelectObject(rec, font);
    HENHMETAFILE hmf = CloseEnhMetaFile(rec);
    ASSERT_TRUE(hmf != NULL);

    int calls = 0;
    // Win32 and swinx both stop the enumeration when the callback returns 0.
    EXPECT_EQ(EnumEnhMetaFile(NULL, hmf, emfStopProc, (LPVOID)&calls, NULL), 0);
    EXPECT_GE(calls, 1);
    EXPECT_TRUE(DeleteEnhMetaFile(hmf) != FALSE);
    DeleteObject(font);
    DeleteDC(ref);
}

TEST(swinx_wke_emf, argument_validation)
{
    EXPECT_FALSE(EnumEnhMetaFile(NULL, NULL, emfEnumProc, NULL, NULL));
    EXPECT_FALSE(DeleteEnhMetaFile(NULL));
    EXPECT_EQ(CloseEnhMetaFile(NULL), (HENHMETAFILE)NULL);
}

#ifndef _WIN32
TEST(swinx_wke_emf, create_is_memory_only_in_swinx)
{
    HDC ref = CreateCompatibleDC(NULL);
    ASSERT_TRUE(ref != NULL);
    // swinx deliberately does not implement on-disk EMF files.
    EXPECT_EQ(CreateEnhMetaFileW(ref, L"/tmp/swinx_emf_probe.emf", NULL, NULL), (HDC)NULL);
    EXPECT_EQ(CreateEnhMetaFileW(ref, NULL, NULL, L"description"), (HDC)NULL);
    DeleteDC(ref);
}
#endif

// ------------------------------------------------------------------------
// ScriptItemize
// ------------------------------------------------------------------------

TEST(swinx_wke_script, itemize_covers_the_whole_string)
{
    const WCHAR text[] = { 'H', 'e', 'l', 'l', 'o' };
    SCRIPT_ITEM items[8];
    memset(items, 0, sizeof(items));
    int nItems = 0;
    HRESULT hr = ScriptItemize(text, 5, 8, NULL, NULL, items, &nItems);
    EXPECT_TRUE(hr == S_OK) << "hr=0x" << (unsigned long)hr;
    EXPECT_GE(nItems, 1);
    EXPECT_LT(nItems, 8);
    EXPECT_EQ(items[0].iCharPos, 0);
    // Uniscribe reserves one slot past the reported items and writes a sentinel
    // whose iCharPos is the string length; pcItems counts the items only.
    EXPECT_EQ(items[nItems].iCharPos, 5);
}

TEST(swinx_wke_script, itemize_reports_out_of_memory_for_a_two_slot_buffer)
{
    const WCHAR text[] = { 'H', 'i' };
    SCRIPT_ITEM items[1];
    int nItems = 0;
    // One run plus the terminating sentinel needs two slots. The two
    // implementations disagree on how to say so: swinx answers E_OUTOFMEMORY
    // and reports the required count, the system USC answers E_INVALIDARG and
    // reports nothing.
    HRESULT hr = ScriptItemize(text, 2, 1, NULL, NULL, items, &nItems);
#ifdef _WIN32
    EXPECT_EQ(hr, E_INVALIDARG);
    EXPECT_EQ(nItems, 0);
#else
    EXPECT_EQ(hr, E_OUTOFMEMORY);
    EXPECT_GE(nItems, 2);
#endif
}

TEST(swinx_wke_script, itemize_argument_validation)
{
    const WCHAR text[] = { 'A' };
    SCRIPT_ITEM items[4];
    int nItems = 0;
    EXPECT_EQ(ScriptItemize(NULL, 1, 4, NULL, NULL, items, &nItems), E_INVALIDARG);
    EXPECT_EQ(ScriptItemize(text, -1, 4, NULL, NULL, items, &nItems), E_INVALIDARG);
#ifndef _WIN32
    // swinx requires every output pointer and treats the degenerate all-NULL
    // call as an empty, successful itemisation.
    EXPECT_EQ(ScriptItemize(text, 1, 4, NULL, NULL, items, NULL), E_INVALIDARG);
    EXPECT_EQ(ScriptItemize(NULL, 0, 0, NULL, NULL, NULL, &nItems), S_OK);
    EXPECT_EQ(nItems, 0);
#else
    // The system USC does the opposite: it tolerates a NULL pcItems and rejects
    // the all-NULL form, so neither quirk is asserted as a shared contract.
    (void)nItems;
#endif
}

// ------------------------------------------------------------------------
// ScriptShape / ScriptPlace / cache
// ------------------------------------------------------------------------

TEST(swinx_wke_script, shape_then_place_produces_one_glyph_and_one_advance)
{
    WkeScriptDc dc;
    ASSERT_TRUE(dc.ok());

    const WCHAR text[] = { 'H', 'e', 'l', 'l', 'o' };
    SCRIPT_ITEM items[4];
    memset(items, 0, sizeof(items));
    int nItems = 0;
    ASSERT_TRUE(ScriptItemize(text, 5, 4, NULL, NULL, items, &nItems) == S_OK);
    ASSERT_GE(nItems, 1);

    WORD glyphs[8], clusters[8];
    SCRIPT_VISATTR va[8];
    memset(glyphs, 0, sizeof(glyphs));
    memset(clusters, 0, sizeof(clusters));
    memset(va, 0, sizeof(va));
    int nGlyphs = 0;
    // psCache is a required in/out parameter: the system USC rejects a NULL one
    // with E_INVALIDARG even though swinx happens to tolerate it.
    SCRIPT_CACHE cache = NULL;
    HRESULT hr = ScriptShape(*dc, &cache, text, 5, 8, &items[0].a, glyphs, clusters, va, &nGlyphs);
    ASSERT_TRUE(hr == S_OK) << "hr=0x" << (unsigned long)hr;
    EXPECT_GT(nGlyphs, 0);
    EXPECT_LE(nGlyphs, 8);

    int advances[8];
    GOFFSET go[8];
    ABC abc;
    memset(advances, 0, sizeof(advances));
    memset(go, 0, sizeof(go));
    memset(&abc, 0, sizeof(abc));
    hr = ScriptPlace(*dc, &cache, glyphs, nGlyphs, va, &items[0].a, advances, go, &abc);
    ASSERT_TRUE(hr == S_OK) << "hr=0x" << (unsigned long)hr;
    for (int i = 0; i < nGlyphs; i++)
        EXPECT_GE(advances[i], 0);

    EXPECT_EQ(ScriptFreeCache(&cache), S_OK);
    EXPECT_TRUE(cache == NULL);
}

TEST(swinx_wke_script, shape_and_place_ask_for_a_font_dc)
{
    const WCHAR text[] = { 'A' };
    SCRIPT_ANALYSIS sa;
    memset(&sa, 0, sizeof(sa));
    sa.eScript = SCRIPT_UNDEFINED;
    WORD glyphs[2], clusters[2];
    SCRIPT_VISATTR va[2];
    int nGlyphs = 0;
#ifdef _WIN32
    // Documented USC behaviour: a NULL DC is a bad argument, not a cache miss.
    EXPECT_EQ(ScriptShape(NULL, NULL, text, 1, 2, &sa, glyphs, clusters, va, &nGlyphs),
              E_INVALIDARG);
#else
    // swinx reports "no font selected yet" so the caller can select one and retry.
    EXPECT_EQ(ScriptShape(NULL, NULL, text, 1, 2, &sa, glyphs, clusters, va, &nGlyphs),
              E_PENDING);
#endif

    int advance[2];
    GOFFSET go[2];
    ABC abc;
    WORD gi[1] = { 1 };
#ifdef _WIN32
    EXPECT_EQ(ScriptPlace(NULL, NULL, gi, 1, NULL, &sa, advance, go, &abc), E_INVALIDARG);
#else
    EXPECT_EQ(ScriptPlace(NULL, NULL, gi, 1, NULL, &sa, advance, go, &abc), E_PENDING);
#endif
}

TEST(swinx_wke_script, shape_and_place_argument_validation)
{
    WkeScriptDc dc;
    ASSERT_TRUE(dc.ok());
    const WCHAR text[] = { 'A' };
    SCRIPT_ANALYSIS sa;
    memset(&sa, 0, sizeof(sa));
    WORD glyphs[2], clusters[2];
    SCRIPT_VISATTR va[2];
    int nGlyphs = 0;
    SCRIPT_CACHE cache = NULL;
    EXPECT_EQ(ScriptShape(*dc, &cache, NULL, 1, 2, &sa, glyphs, clusters, va, &nGlyphs),
              E_INVALIDARG);
    EXPECT_EQ(ScriptShape(*dc, &cache, text, 0, 0, &sa, glyphs, clusters, va, &nGlyphs), S_OK);
    EXPECT_EQ(nGlyphs, 0);

    WORD gi[1] = { 1 };
    int advance[1];
    GOFFSET go[1];
    ABC abc;
    EXPECT_EQ(ScriptPlace(*dc, &cache, NULL, 1, NULL, &sa, advance, go, &abc), E_INVALIDARG);
    // An empty glyph run is a no-op in swinx, not an error -- but the output
    // pointers are still required, so hand over real storage. The system USC
    // rejects cGlyphs == 0 outright, so only the swinx side asserts the no-op.
#ifdef _WIN32
    EXPECT_EQ(ScriptPlace(*dc, &cache, gi, 0, NULL, &sa, advance, go, &abc), E_INVALIDARG);
#else
    EXPECT_EQ(ScriptPlace(*dc, &cache, gi, 0, NULL, &sa, advance, go, &abc), S_OK);
#endif
    EXPECT_EQ(ScriptFreeCache(&cache), S_OK);
}

// ------------------------------------------------------------------------
// ScriptGetFontProperties
// ------------------------------------------------------------------------

TEST(swinx_wke_script, font_properties_reports_the_struct_size)
{
    WkeScriptDc dc;
    ASSERT_TRUE(dc.ok());
    SCRIPT_FONTPROPERTIES sfp;
    memset(&sfp, 0, sizeof(sfp));
    sfp.cBytes = (int)sizeof(sfp);
    SCRIPT_CACHE cache = NULL;
    HRESULT hr = ScriptGetFontProperties(*dc, &cache, &sfp);
    ASSERT_TRUE(hr == S_OK) << "hr=0x" << (unsigned long)hr;
    EXPECT_EQ(sfp.cBytes, (int)sizeof(sfp));

    EXPECT_EQ(ScriptFreeCache(&cache), S_OK);

    EXPECT_EQ(ScriptGetFontProperties(*dc, &cache, NULL), E_INVALIDARG);
    // A NULL DC is a cache miss once a cache slot is supplied -- same answer
    // from the system USC and swinx. (With psc == NULL the argument check runs
    // first and both report E_INVALIDARG instead.)
    EXPECT_EQ(ScriptGetFontProperties(NULL, &cache, &sfp), E_PENDING);
}

// ------------------------------------------------------------------------
// ScriptXtoCP
// ------------------------------------------------------------------------

TEST(swinx_wke_script, x_to_cp_maps_an_advance_position)
{
    // Synthesised linear run: two clusters / two glyphs, 8 units each.
    const int cChars = 2, cGlyphs = 2;
    WORD clusters[2] = { 0, 1 };
    WORD glyphs[2] = { 1, 1 };
    SCRIPT_VISATTR va[2];
    memset(va, 0, sizeof(va));
    int advances[2] = { 8, 8 };
    SCRIPT_ANALYSIS sa;
    memset(&sa, 0, sizeof(sa));
    sa.eScript = SCRIPT_UNDEFINED;

    int cp = -1, trailing = -1;
    HRESULT hr = ScriptXtoCP(3, cChars, cGlyphs, clusters, va, advances, &sa, &cp, &trailing);
    ASSERT_TRUE(hr == S_OK) << "hr=0x" << (unsigned long)hr;
    EXPECT_EQ(cp, 0);

    hr = ScriptXtoCP(12, cChars, cGlyphs, clusters, va, advances, &sa, &cp, &trailing);
    ASSERT_TRUE(hr == S_OK) << "hr=0x" << (unsigned long)hr;
    EXPECT_EQ(cp, 1);
#ifndef _WIN32
    // swinx reports "before/after the caret" from the advance midpoint.
    EXPECT_EQ(trailing, 1);
#endif
}

// ------------------------------------------------------------------------
// ScriptStringAnalyse / Out / Free — the wke font-fallback probe
// ------------------------------------------------------------------------

TEST(swinx_wke_script, string_analysis_argument_validation)
{
    WkeScriptDc dc;
    ASSERT_TRUE(dc.ok());
    const WCHAR text[] = { 'A' };
    SCRIPT_STRING_ANALYSIS ssa = NULL;
#ifdef _WIN32
    // With flags == 0 no glyph is produced, so the system USC accepts a NULL DC
    // and analyses the string; swinx requires the DC and reports E_INVALIDARG.
    EXPECT_EQ(ScriptStringAnalyse(NULL, text, 1, 0, -1, 0, 0, NULL, NULL, NULL, NULL, NULL, &ssa),
              S_OK);
    if (ssa)
        EXPECT_EQ(ScriptStringFree(&ssa), S_OK);
#else
    EXPECT_EQ(ScriptStringAnalyse(NULL, text, 1, 0, -1, 0, 0, NULL, NULL, NULL, NULL, NULL, &ssa),
              E_INVALIDARG);
#endif
    EXPECT_EQ(ScriptStringAnalyse(*dc, text, 1, 0, -1, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL),
              E_INVALIDARG);
    EXPECT_EQ(ScriptStringAnalyse(*dc, NULL, 1, 0, -1, 0, 0, NULL, NULL, NULL, NULL, NULL, &ssa),
              E_INVALIDARG);
    EXPECT_EQ(ScriptStringAnalyse(*dc, text, -1, 0, -1, 0, 0, NULL, NULL, NULL, NULL, NULL, &ssa),
              E_INVALIDARG);
    // iCharset = -1 marks the buffer as wide characters; any other value is the
    // ANSI口径 and then cString counts bytes, not characters of the buffer.
    // The SDK also requires at least one character.
    EXPECT_EQ(ScriptStringAnalyse(*dc, text, 0, -1, -1, 0, 0, NULL, NULL, NULL, NULL, NULL,
                              &ssa),
              E_INVALIDARG);
    EXPECT_EQ(ScriptStringFree(NULL), E_INVALIDARG);
}

TEST(swinx_wke_script, string_analysis_roundtrip)
{
    WkeScriptDc dc;
    ASSERT_TRUE(dc.ok());
    const WCHAR text[] = { 'A', 'B' };
    SCRIPT_STRING_ANALYSIS ssa = NULL;
    HRESULT hr = ScriptStringAnalyse(*dc, text, 2, -1, -1,
                                     SSA_GLYPHS, 0,
                                     NULL, NULL, NULL, NULL, NULL, &ssa);
    ASSERT_TRUE(hr == S_OK) << "hr=0x" << (unsigned long)hr;
    ASSERT_TRUE(ssa != NULL);
    EXPECT_EQ(ScriptStringOut(ssa, 0, 24, 0, NULL, 0, 2, FALSE), S_OK);
    EXPECT_EQ(ScriptStringFree(&ssa), S_OK);
    EXPECT_TRUE(ssa == NULL);
}

#ifndef _WIN32
TEST(swinx_wke_script, fallback_probe_records_the_chosen_font_into_the_emf)
{
    // This is exactly what wke/WebKit's FontCacheWin::getFontDataForCharacters
    // does on non-Windows: analyse with SSA_FALLBACK|SSA_METAFILE against a
    // recording DC, let ScriptStringOut select the fallback font, then read the
    // selected LOGFONT back out of the recorded EMF.
    HDC ref = CreateCompatibleDC(NULL);
    ASSERT_TRUE(ref != NULL);
    HDC rec = CreateEnhMetaFileW(ref, NULL, NULL, NULL);
    ASSERT_TRUE(rec != NULL);
    HFONT base = makeFont("Arial", 20);
    ASSERT_TRUE(base != NULL);
    SelectObject(rec, base);

    const WCHAR cjk[] = { 0x4E2D, 0x6587 };
    SCRIPT_STRING_ANALYSIS ssa = NULL;
    ASSERT_EQ(ScriptStringAnalyse(rec, cjk, 2, -1, -1,
                                  SSA_GLYPHS | SSA_FALLBACK | SSA_METAFILE, 0,
                                  NULL, NULL, NULL, NULL, NULL, &ssa), S_OK);
    ASSERT_TRUE(ssa != NULL);
    ASSERT_EQ(ScriptStringOut(ssa, 0, 20, 0, NULL, 0, 2, FALSE), S_OK);
    ASSERT_EQ(ScriptStringFree(&ssa), S_OK);

    HENHMETAFILE hmf = CloseEnhMetaFile(rec);
    ASSERT_TRUE(hmf != NULL);
    EmfFontProbe probe;
    memset(&probe, 0, sizeof(probe));
    ASSERT_TRUE(EnumEnhMetaFile(NULL, hmf, emfEnumProc, &probe, NULL) != FALSE);
    // Two records at least: the base font and the font selected by the fallback.
    EXPECT_GE(probe.fontRecords, 2);
    EXPECT_EQ(probe.records, probe.fontRecords);
    EXPECT_TRUE(DeleteEnhMetaFile(hmf) != FALSE);
    DeleteObject(base);
    DeleteDC(ref);
}

// Runs the fallback probe above for one pString口径 and reports the records.
// swinx-only: Win32 reads a non -1 iCharset through the host ANSI code page,
// swinx uses its UTF-8口径, so only the swinx result is predictable here.
static void probeStringCharset(const void *pString, int cString, int iCharset,
                               EmfFontProbe *probe)
{
    memset(probe, 0, sizeof(*probe));
    HDC ref = CreateCompatibleDC(NULL);
    ASSERT_TRUE(ref != NULL);
    HDC rec = CreateEnhMetaFileW(ref, NULL, NULL, NULL);
    ASSERT_TRUE(rec != NULL);
    HFONT base = makeFont("Arial", 20);
    ASSERT_TRUE(base != NULL);
    SelectObject(rec, base);

    SCRIPT_STRING_ANALYSIS ssa = NULL;
    ASSERT_EQ(ScriptStringAnalyse(rec, pString, cString, -1, iCharset,
                                  SSA_GLYPHS | SSA_FALLBACK | SSA_METAFILE, 0,
                                  NULL, NULL, NULL, NULL, NULL, &ssa), S_OK);
    ASSERT_TRUE(ssa != NULL);
    ASSERT_EQ(ScriptStringOut(ssa, 0, 20, 0, NULL, 0, cString, FALSE), S_OK);
    ASSERT_EQ(ScriptStringFree(&ssa), S_OK);

    HENHMETAFILE hmf = CloseEnhMetaFile(rec);
    ASSERT_TRUE(hmf != NULL);
    EXPECT_TRUE(EnumEnhMetaFile(NULL, hmf, emfEnumProc, probe, NULL) != FALSE);
    EXPECT_TRUE(DeleteEnhMetaFile(hmf) != FALSE);
    DeleteObject(base);
    DeleteDC(ref);
}

TEST(swinx_wke_script, string_analysis_utf8_matches_the_wchar_result)
{
    // 中文 in UTF-8 is E4 B8 AD E6 96 87: six bytes for two characters, and the
    // ANSI口径 counts bytes, which is why cString differs from the WCHAR case.
    const char cjkUtf8[] = { (char)0xE4, (char)0xB8, (char)0xAD,
                              (char)0xE6, (char)0x96, (char)0x87 };
    const WCHAR cjkWide[] = { 0x4E2D, 0x6587 };

    EmfFontProbe wide, ansi;
    probeStringCharset(cjkWide, 2, -1, &wide);
    probeStringCharset(cjkUtf8, 6, ANSI_CHARSET, &ansi);

    // Reading the UTF-8 bytes one per character would land on Latin-1 values that
    // the base font already covers, so no fallback would be selected and only one
    // font record would exist. Two records plus the same family is the proof that
    // both口径 produced the same two code points.
    EXPECT_GE(wide.fontRecords, 2);
    EXPECT_GE(ansi.fontRecords, 2);
    EXPECT_TRUE(wide.lastFace[0] != 0);
    EXPECT_TRUE(sameFace(wide.lastFace, ansi.lastFace));
}

TEST(swinx_wke_script, string_analysis_undecodable_bytes_still_analyse)
{
    // Win32 conversion semantics without MB_ERR_INVALID_CHARS: bad bytes are
    // replaced and the analysis succeeds instead of failing the whole string.
    // probeStringCharset asserts the two HRESULTs.
    const char bad[] = { (char)0xFF, (char)0xFE, 'A' };
    EmfFontProbe probe;
    probeStringCharset(bad, 3, ANSI_CHARSET, &probe);
    EXPECT_EQ(probe.records, probe.fontRecords);
}
#endif
