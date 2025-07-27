#ifndef __STRANSLATOR_CAPI_H__
#define __STRANSLATOR_CAPI_H__

#include "../STranslator-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Translation constants
 */
#define TRANSLATOR_MAX_NAME_LEN TR_MAX_NAME_LEN

/*
 * C API Helper Macros for STranslator Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* ITranslator C API Macros */
#define ITranslator_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ITranslator_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ITranslator_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define ITranslator_Load(This, pData, uType) \
    ((This)->lpVtbl->Load(This, pData, uType))

#define ITranslator_GetName(This, szBuf) \
    ((This)->lpVtbl->GetName(This, szBuf))

#define ITranslator_GetNameA(This, str) \
    ((This)->lpVtbl->GetNameA(This, str))

#define ITranslator_NameEqual(This, pszName) \
    ((This)->lpVtbl->NameEqual(This, pszName))

#define ITranslator_NameEqualA(This, str) \
    ((This)->lpVtbl->NameEqualA(This, str))

#define ITranslator_guid(This) \
    ((This)->lpVtbl->guid(This))

#define ITranslator_tr(This, strSrc, strCtx, pszOut, nLen) \
    ((This)->lpVtbl->tr(This, strSrc, strCtx, pszOut, nLen))

#define ITranslator_getFontInfo(This, strFont) \
    ((This)->lpVtbl->getFontInfo(This, strFont))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* ITranslator Helper Functions */
static inline long ITranslator_AddRef_C(ITranslator* pThis)
{
    return ITranslator_AddRef(pThis);
}

static inline long ITranslator_Release_C(ITranslator* pThis)
{
    return ITranslator_Release(pThis);
}

static inline BOOL ITranslator_Load_C(ITranslator* pThis, LPVOID pData, UINT uType)
{
    return ITranslator_Load(pThis, pData, uType);
}

static inline void ITranslator_GetName_C(const ITranslator* pThis, wchar_t szBuf[TRANSLATOR_MAX_NAME_LEN])
{
    ITranslator_GetName(pThis, szBuf);
}

static inline void ITranslator_GetNameA_C(const ITranslator* pThis, IStringA* str)
{
    ITranslator_GetNameA(pThis, str);
}

static inline BOOL ITranslator_NameEqual_C(const ITranslator* pThis, LPCWSTR pszName)
{
    return ITranslator_NameEqual(pThis, pszName);
}

static inline BOOL ITranslator_NameEqualA_C(const ITranslator* pThis, const IStringA* str)
{
    return ITranslator_NameEqualA(pThis, str);
}

static inline GUID ITranslator_guid_C(const ITranslator* pThis)
{
    return ITranslator_guid(pThis);
}

static inline int ITranslator_tr_C(const ITranslator* pThis, const IStringW* strSrc, const IStringW* strCtx, wchar_t* pszOut, int nLen)
{
    return ITranslator_tr(pThis, strSrc, strCtx, pszOut, nLen);
}

static inline void ITranslator_getFontInfo_C(const ITranslator* pThis, IStringW* strFont)
{
    ITranslator_getFontInfo(pThis, strFont);
}

/*
 * Convenience macros for common translator operations
 */

/* Translation shortcuts */
#define ITranslator_Translate(This, src, ctx, out, len) \
    ITranslator_tr(This, src, ctx, out, len)

#define ITranslator_TranslateSimple(This, src, out, len) \
    ITranslator_tr(This, src, NULL, out, len)

#define ITranslator_GetTranslation(This, src, ctx, out, len) \
    ITranslator_tr(This, src, ctx, out, len)

/* Name operations shortcuts */
#define ITranslator_GetTranslatorName(This, buf) \
    ITranslator_GetName(This, buf)

#define ITranslator_IsNamed(This, name) \
    ITranslator_NameEqual(This, name)

#define ITranslator_CheckName(This, name) \
    ITranslator_NameEqual(This, name)

/* GUID operations shortcuts */
#define ITranslator_GetGUID(This) \
    ITranslator_guid(This)

#define ITranslator_GetId(This) \
    ITranslator_guid(This)

/* Font operations shortcuts */
#define ITranslator_GetFont(This, font) \
    ITranslator_getFontInfo(This, font)

#define ITranslator_GetFontInfo(This, font) \
    ITranslator_getFontInfo(This, font)

/* Safe translator operations */
#define ITranslator_SafeLoad(This, data, type) \
    ((This) && (data) ? ITranslator_Load(This, data, type) : FALSE)

#define ITranslator_SafeTranslate(This, src, ctx, out, len) \
    ((This) && (src) && (out) && (len) > 0 ? ITranslator_tr(This, src, ctx, out, len) : 0)

#define ITranslator_SafeGetName(This, buf) \
    do { \
        if ((This) && (buf)) { \
            ITranslator_GetName(This, buf); \
        } else if (buf) { \
            buf[0] = L'\0'; \
        } \
    } while(0)

#define ITranslator_SafeNameEqual(This, name) \
    ((This) && (name) ? ITranslator_NameEqual(This, name) : FALSE)

#define ITranslator_SafeGetFontInfo(This, font) \
    do { \
        if ((This) && (font)) { \
            ITranslator_getFontInfo(This, font); \
        } \
    } while(0)

/*
 * Translator helper functions
 */
static inline BOOL ITranslator_IsValid_C(const ITranslator* pThis)
{
    return (pThis != NULL);
}

static inline BOOL ITranslator_HasName_C(const ITranslator* pThis, LPCWSTR name)
{
    if (!pThis || !name) return FALSE;
    
    return ITranslator_NameEqual(pThis, name);
}

static inline BOOL ITranslator_IsGuidEqual_C(const ITranslator* pThis, const GUID* guid)
{
    if (!pThis || !guid) return FALSE;
    
    GUID thisGuid = ITranslator_guid(pThis);
    return memcmp(&thisGuid, guid, sizeof(GUID)) == 0;
}

static inline int ITranslator_GetTranslationLength_C(const ITranslator* pThis, const IStringW* strSrc, const IStringW* strCtx)
{
    if (!pThis || !strSrc) return 0;
    
    // Get required buffer length by passing NULL buffer
    return ITranslator_tr(pThis, strSrc, strCtx, NULL, 0);
}

static inline wchar_t* ITranslator_AllocAndTranslate_C(const ITranslator* pThis, const IStringW* strSrc, const IStringW* strCtx)
{
    if (!pThis || !strSrc) return NULL;
    
    int requiredLen = ITranslator_GetTranslationLength_C(pThis, strSrc, strCtx);
    if (requiredLen <= 0) return NULL;
    
    wchar_t* buffer = (wchar_t*)malloc((requiredLen + 1) * sizeof(wchar_t));
    if (!buffer) return NULL;
    
    int actualLen = ITranslator_tr(pThis, strSrc, strCtx, buffer, requiredLen + 1);
    if (actualLen <= 0) {
        free(buffer);
        return NULL;
    }
    
    return buffer;
}

static inline BOOL ITranslator_TranslateToBuffer_C(const ITranslator* pThis, const IStringW* strSrc, const IStringW* strCtx, wchar_t** ppBuffer, int* pBufferLen)
{
    if (!pThis || !strSrc || !ppBuffer || !pBufferLen) return FALSE;
    
    int requiredLen = ITranslator_GetTranslationLength_C(pThis, strSrc, strCtx);
    if (requiredLen <= 0) return FALSE;
    
    if (*ppBuffer == NULL || *pBufferLen < requiredLen + 1) {
        if (*ppBuffer) {
            free(*ppBuffer);
        }
        *ppBuffer = (wchar_t*)malloc((requiredLen + 1) * sizeof(wchar_t));
        if (!*ppBuffer) {
            *pBufferLen = 0;
            return FALSE;
        }
        *pBufferLen = requiredLen + 1;
    }
    
    int actualLen = ITranslator_tr(pThis, strSrc, strCtx, *ppBuffer, *pBufferLen);
    return (actualLen > 0);
}

/*
 * Translator state management
 */
typedef struct TranslatorState {
    ITranslator* translator;
    wchar_t name[TRANSLATOR_MAX_NAME_LEN];
    GUID guid;
    BOOL isValid;
    BOOL isLoaded;
} TranslatorState;

static inline void TranslatorState_Init(TranslatorState* state, ITranslator* translator)
{
    if (state) {
        state->translator = translator;
        if (translator) {
            ITranslator_GetName(translator, state->name);
            state->guid = ITranslator_guid(translator);
            state->isValid = TRUE;
            state->isLoaded = TRUE; // Assume loaded if we have a translator
        } else {
            memset(state, 0, sizeof(TranslatorState));
        }
    }
}

static inline void TranslatorState_Update(TranslatorState* state)
{
    if (state && state->translator) {
        TranslatorState_Init(state, state->translator);
    }
}

static inline BOOL TranslatorState_LoadData(TranslatorState* state, LPVOID pData, UINT uType)
{
    if (!state || !state->translator || !pData) return FALSE;
    
    BOOL result = ITranslator_Load(state->translator, pData, uType);
    state->isLoaded = result;
    return result;
}

static inline int TranslatorState_Translate(TranslatorState* state, const IStringW* strSrc, const IStringW* strCtx, wchar_t* pszOut, int nLen)
{
    if (!state || !state->isValid || !state->isLoaded || !state->translator) return 0;
    
    return ITranslator_tr(state->translator, strSrc, strCtx, pszOut, nLen);
}

/*
 * Translation cache helpers
 */
typedef struct TranslationCache {
    struct {
        const IStringW* src;
        const IStringW* ctx;
        wchar_t* translation;
        int length;
    } *entries;
    int count;
    int capacity;
} TranslationCache;

static inline void TranslationCache_Init(TranslationCache* cache, int initialCapacity)
{
    if (cache) {
        cache->entries = initialCapacity > 0 ? 
            malloc(sizeof(*cache->entries) * initialCapacity) : NULL;
        cache->count = 0;
        cache->capacity = initialCapacity;
    }
}

static inline void TranslationCache_Cleanup(TranslationCache* cache)
{
    if (cache) {
        for (int i = 0; i < cache->count; i++) {
            if (cache->entries[i].translation) {
                free(cache->entries[i].translation);
            }
        }
        if (cache->entries) {
            free(cache->entries);
        }
        memset(cache, 0, sizeof(TranslationCache));
    }
}

static inline wchar_t* TranslationCache_Find(TranslationCache* cache, const IStringW* src, const IStringW* ctx)
{
    if (!cache || !src) return NULL;
    
    for (int i = 0; i < cache->count; i++) {
        if (cache->entries[i].src == src && cache->entries[i].ctx == ctx) {
            return cache->entries[i].translation;
        }
    }
    return NULL;
}

static inline BOOL TranslationCache_Add(TranslationCache* cache, const IStringW* src, const IStringW* ctx, const wchar_t* translation)
{
    if (!cache || !src || !translation || cache->count >= cache->capacity) return FALSE;
    
    int len = wcslen(translation);
    wchar_t* copy = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
    if (!copy) return FALSE;
    
    wcscpy_s(copy, len + 1, translation);
    
    cache->entries[cache->count].src = src;
    cache->entries[cache->count].ctx = ctx;
    cache->entries[cache->count].translation = copy;
    cache->entries[cache->count].length = len;
    cache->count++;
    
    return TRUE;
}

/*
 * Reference counting helpers
 */
#define ITranslator_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define ITranslator_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Debugging helpers
 */
#ifdef _DEBUG
static inline void ITranslator_DebugInfo_C(const ITranslator* pThis)
{
    if (!pThis) {
        printf("Translator: NULL\n");
        return;
    }
    
    wchar_t name[TRANSLATOR_MAX_NAME_LEN];
    ITranslator_GetName(pThis, name);
    GUID guid = ITranslator_guid(pThis);
    
    printf("Translator: %p\n", pThis);
    printf("  Name: %ls\n", name);
    printf("  GUID: {%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}\n",
           guid.Data1, guid.Data2, guid.Data3,
           guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
           guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}
#else
#define ITranslator_DebugInfo_C(This) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STRANSLATOR_CAPI_H__ */
