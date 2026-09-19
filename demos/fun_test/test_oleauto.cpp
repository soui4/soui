/**
 * swinx OLE automation data APIs: BSTR management (alloc/realloc/free),
 * VARIANT init/clear/copy
 * and COM task memory. Assertions follow standard Win32/OLE semantics and
 * compile against real Windows (oleaut32/ole32) as well as swinx.
 *
 * Regression context: swinx's VariantCopy VT_BSTR branch and SafeArray BSTR
 * handling both depend on SysAllocStringByteLen keeping the byte length as
 * the authoritative size (embedded NULs included).
 */
#include <gtest/gtest.h>
#include <windows.h>
#include <oleauto.h>
#include <objbase.h>
#include <string.h>
#ifndef _WIN32
#include <shlobj.h> // swinx: CLSID_DragDropHelper / IID_IDragSourceHelper
#endif

// ------------------------------------------------------------------------
// BSTR
// ------------------------------------------------------------------------

TEST(swinx_bstr, alloc_string_len_and_bytes)
{
    BSTR s = SysAllocString(L"hello");
    ASSERT_TRUE(s != NULL);
    EXPECT_EQ(SysStringLen(s), 5u);
    EXPECT_EQ(SysStringByteLen(s), 10u);
    EXPECT_EQ(wcscmp(s, L"hello"), 0);
    // BSTR always terminates with a NUL after the length-prefixed content
    EXPECT_EQ(s[5], L'\0');
    SysFreeString(s);
}

TEST(swinx_bstr, alloc_null_yields_null)
{
    // documented: SysAllocString(NULL) returns NULL
    EXPECT_TRUE(SysAllocString(NULL) == NULL);
}

TEST(swinx_bstr, null_len_is_zero)
{
    // documented: length queries on a NULL BSTR return 0
    EXPECT_EQ(SysStringLen(NULL), 0u);
    EXPECT_EQ(SysStringByteLen(NULL), 0u);
}

TEST(swinx_bstr, alloc_len_truncates)
{
    BSTR s = SysAllocStringLen(L"abcdef", 3);
    ASSERT_TRUE(s != NULL);
    EXPECT_EQ(SysStringLen(s), 3u);
    EXPECT_EQ(SysStringByteLen(s), 6u);
    EXPECT_EQ(wcsncmp(s, L"abc", 3), 0);
    SysFreeString(s);
}

TEST(swinx_bstr, alloc_len_null_uninitialized_but_sized)
{
    // documented: SysAllocStringLen(NULL, n) allocates a string of n chars
    // with uninitialized content — only the length is asserted
    BSTR s = SysAllocStringLen(NULL, 4);
    ASSERT_TRUE(s != NULL);
    EXPECT_EQ(SysStringLen(s), 4u);
    EXPECT_EQ(SysStringByteLen(s), 8u);
    SysFreeString(s);
}

TEST(swinx_bstr, byte_len_keeps_embedded_nul)
{
    // SysAllocStringByteLen performs no ANSI->Unicode conversion: the byte
    // length is authoritative and embedded NULs are preserved verbatim
    const char raw[5] = {'a', 'b', '\0', 'c', 'd'};
    BSTR s = SysAllocStringByteLen(raw, 5);
    ASSERT_TRUE(s != NULL);
    EXPECT_EQ(SysStringByteLen(s), 5u);
    EXPECT_EQ(memcmp(s, raw, 5), 0);
    SysFreeString(s);
}

TEST(swinx_bstr, free_null_is_noop)
{
    SysFreeString(NULL);
    SUCCEED();
}

// ------------------------------------------------------------------------
// BSTR: SysReAllocString / SysReAllocStringLen
// ------------------------------------------------------------------------

TEST(swinx_bstr, realloc_string_replaces_content)
{
    BSTR s = SysAllocString(L"old");
    ASSERT_TRUE(s != NULL);

    EXPECT_TRUE(SysReAllocString(&s, L"a longer replacement") != 0);
    ASSERT_TRUE(s != NULL);
    EXPECT_EQ(SysStringLen(s), 20u);
    EXPECT_EQ(SysStringByteLen(s), 40u);
    EXPECT_EQ(wcscmp(s, L"a longer replacement"), 0);
    EXPECT_EQ(s[20], L'\0');
    SysFreeString(s);
}

TEST(swinx_bstr, realloc_string_empty_source_is_valid)
{
    // an empty (non-NULL) source yields a valid empty BSTR, not NULL
    BSTR s = SysAllocString(L"old");
    ASSERT_TRUE(s != NULL);
    EXPECT_TRUE(SysReAllocString(&s, L"") != 0);
    ASSERT_TRUE(s != NULL);
    EXPECT_EQ(SysStringLen(s), 0u);
    EXPECT_EQ(s[0], L'\0');
    SysFreeString(s);
}

TEST(swinx_bstr, realloc_string_from_null_pointer_allocates)
{
    // a NULL *pbstr behaves like SysAllocString
    BSTR s = NULL;
    EXPECT_TRUE(SysReAllocString(&s, L"fresh") != 0);
    ASSERT_TRUE(s != NULL);
    EXPECT_EQ(SysStringLen(s), 5u);
    EXPECT_EQ(wcscmp(s, L"fresh"), 0);
    SysFreeString(s);
}

TEST(swinx_bstr, realloc_string_len_truncates_and_keeps_embedded_nul)
{
    BSTR s = SysAllocString(L"abcdef");
    ASSERT_TRUE(s != NULL);

    EXPECT_TRUE(SysReAllocStringLen(&s, L"XY", 2) != 0);
    ASSERT_TRUE(s != NULL);
    EXPECT_EQ(SysStringLen(s), 2u);
    EXPECT_EQ(SysStringByteLen(s), 4u);
    EXPECT_EQ(wcscmp(s, L"XY"), 0);
    EXPECT_EQ(s[2], L'\0');

    // documented: the source may contain embedded NULs and does not need to be
    // NUL-terminated - len characters are copied and a NUL is appended after
    const OLECHAR src[4] = {L'a', L'\0', L'b', L'c'};
    EXPECT_TRUE(SysReAllocStringLen(&s, src, 4) != 0);
    ASSERT_TRUE(s != NULL);
    EXPECT_EQ(SysStringLen(s), 4u);
    EXPECT_EQ(SysStringByteLen(s), 8u);
    EXPECT_EQ(s[0], L'a');
    EXPECT_EQ(s[1], L'\0');
    EXPECT_EQ(s[2], L'b');
    EXPECT_EQ(s[3], L'c');
    EXPECT_EQ(s[4], L'\0');
    SysFreeString(s);
}

TEST(swinx_bstr, realloc_string_len_null_source_is_sized)
{
    // documented: a NULL source allocates a string of len characters whose
    // contents are uninitialized - only length and termination are asserted
    BSTR s = SysAllocString(L"old");
    ASSERT_TRUE(s != NULL);

    EXPECT_TRUE(SysReAllocStringLen(&s, NULL, 7) != 0);
    ASSERT_TRUE(s != NULL);
    EXPECT_EQ(SysStringLen(s), 7u);
    EXPECT_EQ(SysStringByteLen(s), 14u);
    EXPECT_EQ(s[7], L'\0');
    SysFreeString(s);
}

TEST(swinx_bstr, realloc_string_len_from_null_pointer_allocates)
{
    BSTR s = NULL;
    EXPECT_TRUE(SysReAllocStringLen(&s, L"abcdef", 3) != 0);
    ASSERT_TRUE(s != NULL);
    EXPECT_EQ(SysStringLen(s), 3u);
    EXPECT_EQ(wcsncmp(s, L"abc", 3), 0);
    SysFreeString(s);
}

TEST(swinx_bstr, realloc_string_len_zero_is_valid_empty)
{
    BSTR s = SysAllocString(L"old");
    ASSERT_TRUE(s != NULL);
    EXPECT_TRUE(SysReAllocStringLen(&s, L"xyz", 0) != 0);
    ASSERT_TRUE(s != NULL);
    EXPECT_EQ(SysStringLen(s), 0u);
    EXPECT_EQ(s[0], L'\0');
    SysFreeString(s);
}

#ifndef _WIN32
// The three cases below are swinx-specific. Win32 documents a NULL pbstr as an
// access violation, a source aliasing *pbstr as undefined ("unexpected results
// may occur"), and says nothing about a NULL source for SysReAllocString (its
// psz is marked [in, optional] though). swinx follows Wine's conformance-tested
// behaviour instead: defensive NULL checks, a NULL source releasing the BSTR,
// and an allocate-before-release order that also makes aliasing safe.
TEST(swinx_bstr, realloc_string_null_source_releases)
{
    BSTR s = SysAllocString(L"old");
    ASSERT_TRUE(s != NULL);
    EXPECT_TRUE(SysReAllocString(&s, NULL) != 0);
    EXPECT_TRUE(s == NULL);
}

TEST(swinx_bstr, realloc_string_null_pointer_is_rejected)
{
    EXPECT_TRUE(SysReAllocString(NULL, L"x") == 0);
    EXPECT_TRUE(SysReAllocStringLen(NULL, L"x", 1) == 0);
}

TEST(swinx_bstr, realloc_string_self_alias_is_safe)
{
    BSTR s = SysAllocString(L"self alias");
    ASSERT_TRUE(s != NULL);
    EXPECT_TRUE(SysReAllocString(&s, s) != 0);
    ASSERT_TRUE(s != NULL);
    EXPECT_EQ(SysStringLen(s), 10u);
    EXPECT_EQ(wcscmp(s, L"self alias"), 0);
    SysFreeString(s);
}
#endif // !_WIN32

// ------------------------------------------------------------------------
// VARIANT
// ------------------------------------------------------------------------

TEST(swinx_variant, init_clear_i4)
{
    VARIANT v;
    VariantInit(&v);
    EXPECT_EQ(v.vt, VT_EMPTY);

    v.vt = VT_I4;
    v.lVal = 42;
    EXPECT_EQ(VariantClear(&v), S_OK);
    EXPECT_EQ(v.vt, VT_EMPTY);
}

TEST(swinx_variant, copy_i4)
{
    VARIANT src, dst;
    VariantInit(&src);
    VariantInit(&dst);

    src.vt = VT_I4;
    src.lVal = 1234;
    EXPECT_EQ(VariantCopy(&dst, &src), S_OK);
    EXPECT_EQ(dst.vt, VT_I4);
    EXPECT_EQ(dst.lVal, 1234);

    EXPECT_EQ(VariantClear(&dst), S_OK);
    EXPECT_EQ(VariantClear(&src), S_OK);
}

TEST(swinx_variant, copy_bstr_deep)
{
    VARIANT src, dst;
    VariantInit(&src);
    VariantInit(&dst);

    BSTR s = SysAllocString(L"variant bstr");
    ASSERT_TRUE(s != NULL);
    src.vt = VT_BSTR;
    src.bstrVal = s;

    EXPECT_EQ(VariantCopy(&dst, &src), S_OK);
    EXPECT_EQ(dst.vt, VT_BSTR);
    // deep copy: the destination owns an independent buffer
    EXPECT_TRUE(dst.bstrVal != src.bstrVal);
    EXPECT_EQ(wcscmp(dst.bstrVal, L"variant bstr"), 0);
    EXPECT_EQ(SysStringLen(dst.bstrVal), SysStringLen(src.bstrVal));

    // clearing both must not double-free (valgrind verifies)
    EXPECT_EQ(VariantClear(&src), S_OK);
    EXPECT_EQ(VariantClear(&dst), S_OK);
    EXPECT_EQ(src.vt, VT_EMPTY);
    EXPECT_EQ(dst.vt, VT_EMPTY);
}

TEST(swinx_variant, clear_empty_is_noop)
{
    VARIANT v;
    VariantInit(&v);
    EXPECT_EQ(VariantClear(&v), S_OK);
    EXPECT_EQ(v.vt, VT_EMPTY);
}

// ------------------------------------------------------------------------
// COM task memory
// ------------------------------------------------------------------------

TEST(swinx_cotaskmem, alloc_free_roundtrip)
{
    BYTE *p = (BYTE *)CoTaskMemAlloc(128);
    ASSERT_TRUE(p != NULL);
    for (int i = 0; i < 128; i++)
        p[i] = (BYTE)i;
    for (int i = 0; i < 128; i++)
        ASSERT_EQ(p[i], (BYTE)i) << "byte " << i;
    CoTaskMemFree(p);
}

TEST(swinx_cotaskmem, realloc_preserves_prefix)
{
    BYTE *p = (BYTE *)CoTaskMemAlloc(64);
    ASSERT_TRUE(p != NULL);
    for (int i = 0; i < 64; i++)
        p[i] = (BYTE)(i * 3);

    // realloc semantics: contents up to the smaller size are preserved
    BYTE *q = (BYTE *)CoTaskMemRealloc(p, 128);
    ASSERT_TRUE(q != NULL);
    for (int i = 0; i < 64; i++)
        ASSERT_EQ(q[i], (BYTE)(i * 3)) << "byte " << i;
    CoTaskMemFree(q);
}

TEST(swinx_cotaskmem, free_null_is_noop)
{
    CoTaskMemFree(NULL);
    SUCCEED();
}

// ------------------------------------------------------------------------
// CoGetMalloc / IMalloc (P1)
// ------------------------------------------------------------------------

TEST(swinx_commalloc, get_task_malloc_and_roundtrip)
{
    IMalloc *m = NULL;
    HRESULT hr = CoGetMalloc(MEMCTX_TASK, &m);
    EXPECT_TRUE(hr==S_OK || hr==S_FALSE);
    ASSERT_TRUE(m != NULL);

    // basic Alloc/Realloc/Free through the returned interface
    void *p = m->Alloc(32);
    ASSERT_TRUE(p != NULL);
    memset(p, 0xAB, 32);

    void *q = m->Realloc(p, 64);
    ASSERT_TRUE(q != NULL);
    BYTE *b = (BYTE *)q;
    for (int i = 0; i < 32; i++)
        ASSERT_EQ(b[i], 0xAB) << "byte " << i;
    m->Free(q);

    // CoTaskMem* is documented to sit on the same allocator as IMalloc
    void *t = CoTaskMemAlloc(16);
    ASSERT_TRUE(t != NULL);
    m->Free(t);
}

TEST(swinx_commalloc, invalid_context_rejected)
{
    IMalloc *m = NULL;
    // only MEMCTX_TASK is supported (Win32 documents E_INVALIDARG otherwise)
    HRESULT hr = CoGetMalloc(MEMCTX_SHARED, &m);
    EXPECT_FALSE(hr == S_OK || hr == S_FALSE);
}

// ------------------------------------------------------------------------
// IIDFromString (P1)
// ------------------------------------------------------------------------

// IIDFromString returns BOOL in swinx but HRESULT on real Windows
static bool IIDParse(const wchar_t *s, GUID *g)
{
#ifdef _WIN32
    // real Win32 IIDFromString takes LPOLESTR (non-const); the input here is const
    return SUCCEEDED(IIDFromString((LPOLESTR)s, g));
#else
    return IIDFromString(s, g) == TRUE;
#endif
}

static void GuidToString(const GUID &g, wchar_t *buf) // buf >= 39 chars
{
    swprintf(buf, 39, L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
             (unsigned)g.Data1, (unsigned)g.Data2, (unsigned)g.Data3,
             g.Data4[0], g.Data4[1], g.Data4[2], g.Data4[3],
             g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);
}

TEST(swinx_iid, parse_known_guid)
{
    // IID_IMalloc's well-known value
    GUID g;
    ASSERT_TRUE(IIDParse(L"{00000002-0000-0000-C000-000000000046}", &g));
    EXPECT_EQ(g.Data1, 0x00000002u);
    EXPECT_EQ(g.Data2, 0);
    EXPECT_EQ(g.Data3, 0);
    EXPECT_EQ(g.Data4[0], 0xC0);
    EXPECT_EQ(g.Data4[1], 0x00);
    EXPECT_EQ(g.Data4[7], 0x46);
}

TEST(swinx_iid, case_insensitive_parse)
{
    GUID a, b;
    ASSERT_TRUE(IIDParse(L"{12345678-9ABC-DEF0-1234-56789ABCDEF0}", &a));
    ASSERT_TRUE(IIDParse(L"{12345678-9abc-def0-1234-56789abcdef0}", &b));
    EXPECT_EQ(memcmp(&a, &b, sizeof(GUID)), 0);
}

TEST(swinx_iid, cocreateguid_roundtrip)
{
    GUID g;
    ASSERT_TRUE(CoCreateGuid(&g) == S_OK);

    wchar_t buf[39];
    GuidToString(g, buf);

    GUID p;
    ASSERT_TRUE(IIDParse(buf, &p));
    EXPECT_EQ(memcmp(&g, &p, sizeof(GUID)), 0);
}

TEST(swinx_iid, rejects_malformed)
{
    GUID g;
    EXPECT_FALSE(IIDParse(L"not-a-guid", &g));
    EXPECT_FALSE(IIDParse(L"{12345678-9ABC-DEF0-1234}", &g)); // truncated
    EXPECT_FALSE(IIDParse(L"{12345678-9ABC-DEF0-1234-56789ABCDEFZ}", &g)); // bad hex
}

// ------------------------------------------------------------------------
// VARIANT COM reference types & BYREF (swinx COM surface completion)
// ------------------------------------------------------------------------

// Minimal counting IUnknown implementation used to verify that VariantClear/
// VariantCopy balance interface reference counts. Compiled against real
// Windows oleaut32 as well as swinx — semantics must match.
namespace
{
class CountingUnknown : public IUnknown
{
public:
    CountingUnknown() : m_cRef(1) {}
    STDMETHOD_(ULONG, AddRef)() override { return (ULONG)(++m_cRef); }
    STDMETHOD_(ULONG, Release)() override { return (ULONG)(--m_cRef); }
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv) override
    {
        if (!ppv)
            return E_INVALIDARG;
        *ppv = NULL;
        if (IsEqualGUID(riid, IID_IUnknown))
        {
            *ppv = (IUnknown *)this;
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }
    LONG m_cRef;
};
} // namespace

TEST(swinx_variant, clear_unknown_releases_refcount)
{
    CountingUnknown *unk = new CountingUnknown(); // ref=1 (creation)
    VARIANT v;
    VariantInit(&v);
    v.vt = VT_UNKNOWN;
    v.punkVal = (IUnknown *)unk;
    EXPECT_EQ(v.vt, VT_UNKNOWN);

    EXPECT_EQ(VariantClear(&v), S_OK);
    EXPECT_EQ(unk->m_cRef, 0); // released exactly once by Clear
    delete unk; // Release() only counts references; the test owns the allocation
}

TEST(swinx_variant, clear_dispatch_releases_refcount)
{
    CountingUnknown *unk = new CountingUnknown(); // ref=1
    VARIANT v;
    VariantInit(&v);
    v.vt = VT_DISPATCH;
    v.pdispVal = (IDispatch *)unk;

    EXPECT_EQ(VariantClear(&v), S_OK);
    EXPECT_EQ(unk->m_cRef, 0);
    delete unk; // the test owns the allocation
}

TEST(swinx_variant, copy_unknown_addrefs)
{
    VARIANT src, dst;
    VariantInit(&src);
    VariantInit(&dst);

    CountingUnknown *unk = new CountingUnknown(); // ref=1
    src.vt = VT_UNKNOWN;
    src.punkVal = (IUnknown *)unk;

    EXPECT_EQ(VariantCopy(&dst, &src), S_OK);
    EXPECT_EQ(dst.vt, VT_UNKNOWN);
    EXPECT_EQ(dst.punkVal, src.punkVal); // same object, new reference
    EXPECT_EQ(unk->m_cRef, 2);

    EXPECT_EQ(VariantClear(&dst), S_OK); // ref=1
    EXPECT_EQ(VariantClear(&src), S_OK); // ref=0
    EXPECT_EQ(unk->m_cRef, 0);
    delete unk; // the test owns the allocation
}

TEST(swinx_variant, clear_safearray_destroys)
{
    SAFEARRAYBOUND bound = {4, 0};
    SAFEARRAY *psa = SafeArrayCreate(VT_I4, 1, &bound);
    ASSERT_TRUE(psa != NULL);

    VARIANT v;
    VariantInit(&v);
    // A SafeArray VARIANT must carry its element type: VT_ARRAY | VT_I4.
    // A bare VT_ARRAY (no type) is rejected by real oleaut32
    // (DISP_E_ARRAYISLOCKED); swinx accepts both, but the typed form is
    // the only portable one.
    v.vt = VT_ARRAY | VT_I4;
    v.parray = psa;

    EXPECT_EQ(VariantClear(&v), S_OK);
    // psa is now destroyed; cannot safely dereference — Clear returned S_OK
    SUCCEED();
}

TEST(swinx_variant, clear_byref_leaves_pointee)
{
    BSTR s = SysAllocString(L"byref");
    ASSERT_TRUE(s != NULL);

    VARIANT v;
    VariantInit(&v);
    v.vt = VT_BSTR | VT_BYREF;
    v.pbstrVal = &s;

    EXPECT_EQ(VariantClear(&v), S_OK);
    EXPECT_EQ(v.vt, VT_EMPTY);
    // the pointee is caller-owned: Clear must not free it
    EXPECT_EQ(SysStringLen(s), 5u);
    SysFreeString(s);
}

TEST(swinx_variant, copy_byref_variant_dereferences)
{
    // SOUI SwndAccessible.cpp accValidateNavStart pattern: a
    // VT_VARIANT|VT_BYREF argument is repeatedly dereference-copied
    // (VariantCopy(pvar, pvar->pvarVal)) until a plain type shows up.
    VARIANT inner, outer, dst;
    VariantInit(&inner);
    VariantInit(&outer);
    VariantInit(&dst);

    inner.vt = VT_I4;
    inner.lVal = 7;
    outer.vt = VT_VARIANT | VT_BYREF;
    outer.pvarVal = &inner;

    EXPECT_EQ(VariantCopy(&dst, &outer), S_OK);
#ifdef _WIN32
    // Real oleaut32 keeps the byref pointer intact: VariantCopy does NOT
    // implicitly dereference VT_VARIANT|VT_BYREF. (On non-Windows, swinx adds
    // that dereference so SOUI's accValidateNavStart loop terminates.)
    EXPECT_EQ(dst.vt, (VARTYPE)(VT_VARIANT | VT_BYREF));
#else
    // swinx additionally dereferences VT_VARIANT|VT_BYREF, so the copy yields
    // the plain inner type — this is what accValidateNavStart relies on.
    EXPECT_EQ(dst.vt, VT_I4);
    EXPECT_EQ(dst.lVal, 7);
#endif
    EXPECT_EQ(VariantClear(&dst), S_OK);
}

TEST(swinx_variant, copy_byref_bstr_is_shallow)
{
    BSTR s = SysAllocString(L"ptr");
    ASSERT_TRUE(s != NULL);

    VARIANT src, dst;
    VariantInit(&src);
    VariantInit(&dst);
    src.vt = VT_BSTR | VT_BYREF;
    src.pbstrVal = &s;

    EXPECT_EQ(VariantCopy(&dst, &src), S_OK);
    EXPECT_EQ(dst.vt, VT_BSTR | VT_BYREF);
    EXPECT_EQ(dst.pbstrVal, &s); // pointer copied verbatim, no allocation
    EXPECT_EQ(VariantClear(&dst), S_OK);
    EXPECT_EQ(SysStringLen(s), 3u); // pointee untouched
    SysFreeString(s);
}

TEST(swinx_variant, copy_safearray_sets_dest_vt)
{
    // regression: the VT_ARRAY branch used to leave dst.vt at VT_EMPTY
    SAFEARRAYBOUND bound = {2, 0};
    SAFEARRAY *psa = SafeArrayCreate(VT_I4, 1, &bound);
    ASSERT_TRUE(psa != NULL);

    VARIANT src, dst;
    VariantInit(&src);
    VariantInit(&dst);
    // Typed SafeArray VARIANT: VT_ARRAY | VT_I4. A bare VT_ARRAY is rejected
    // by real oleaut32, so the element type must be carried along.
    src.vt = VT_ARRAY | VT_I4;
    src.parray = psa;

    EXPECT_EQ(VariantCopy(&dst, &src), S_OK);
    // VariantCopy preserves the full VARTYPE (array modifier + element type)
    EXPECT_EQ(dst.vt, VT_ARRAY | VT_I4);
    EXPECT_TRUE(dst.parray != NULL && dst.parray != src.parray);

    EXPECT_EQ(VariantClear(&dst), S_OK);
    EXPECT_EQ(VariantClear(&src), S_OK);
}

#ifndef _WIN32
// swinx-defined NULL handling (real oleaut32 makes no guarantee)
TEST(swinx_variant, null_guards)
{
    EXPECT_EQ(VariantClear(NULL), E_POINTER);

    VARIANT dst;
    VariantInit(&dst);
    EXPECT_EQ(VariantCopy(&dst, NULL), E_POINTER);
    EXPECT_EQ(VariantCopy(NULL, &dst), E_POINTER);
}
#endif

#ifndef _WIN32
// swinx CoCreateInstance accepts IID_IUnknown (SOUI SAxContainer pattern);
// on Windows the real ole32/registry path makes this request unreliable in
// a test environment, so the baseline only covers swinx.
TEST(swinx_cocreate, dragdrop_helper_iunknown_riid)
{
    IUnknown *unk = NULL;
    HRESULT hr = CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
                                  IID_IUnknown, (void **)&unk);
    EXPECT_EQ(hr, S_OK);
    ASSERT_TRUE(unk != NULL);
    unk->Release(); // must not crash: refcount balanced through QI

    // the original riid path still works
    IDragSourceHelper *helper = NULL;
    hr = CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
                          IID_IDragSourceHelper, (void **)&helper);
    EXPECT_EQ(hr, S_OK);
    if (helper)
        helper->Release();
}
#endif
