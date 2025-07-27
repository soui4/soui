#ifndef __SSKINPOOL_CAPI_H__
#define __SSKINPOOL_CAPI_H__

#include "../SSkinPool-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SSkinPool Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* ISkinPool C API Macros */
#define ISkinPool_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ISkinPool_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ISkinPool_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define ISkinPool_GetSkin(This, strSkinName, nScale) \
    ((This)->lpVtbl->GetSkin(This, strSkinName, nScale))

#define ISkinPool_GetSkin_Default(This, strSkinName) \
    ((This)->lpVtbl->GetSkin(This, strSkinName, 100))

#define ISkinPool_LoadSkins(This, xmlNode) \
    ((This)->lpVtbl->LoadSkins(This, xmlNode))

#define ISkinPool_AddSkin(This, skin) \
    ((This)->lpVtbl->AddSkin(This, skin))

#define ISkinPool_RemoveSkin(This, skin) \
    ((This)->lpVtbl->RemoveSkin(This, skin))

#define ISkinPool_RemoveAll(This) \
    ((This)->lpVtbl->RemoveAll(This))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* ISkinPool Helper Functions */
static inline long ISkinPool_AddRef_C(ISkinPool* pThis)
{
    return ISkinPool_AddRef(pThis);
}

static inline long ISkinPool_Release_C(ISkinPool* pThis)
{
    return ISkinPool_Release(pThis);
}

static inline ISkinObj* ISkinPool_GetSkin_C(ISkinPool* pThis, LPCWSTR strSkinName, int nScale)
{
    return ISkinPool_GetSkin(pThis, strSkinName, nScale);
}

static inline int ISkinPool_LoadSkins_C(ISkinPool* pThis, IXmlNode* xmlNode)
{
    return ISkinPool_LoadSkins(pThis, xmlNode);
}

static inline BOOL ISkinPool_AddSkin_C(ISkinPool* pThis, ISkinObj* skin)
{
    return ISkinPool_AddSkin(pThis, skin);
}

static inline BOOL ISkinPool_RemoveSkin_C(ISkinPool* pThis, ISkinObj* skin)
{
    return ISkinPool_RemoveSkin(pThis, skin);
}

static inline void ISkinPool_RemoveAll_C(ISkinPool* pThis)
{
    ISkinPool_RemoveAll(pThis);
}

/*
 * Convenience macros for common skin pool operations
 */

/* Skin retrieval shortcuts */
#define ISkinPool_FindSkin(This, name) \
    ISkinPool_GetSkin_Default(This, name)

#define ISkinPool_GetSkinByName(This, name) \
    ISkinPool_GetSkin_Default(This, name)

#define ISkinPool_GetScaledSkin(This, name, scale) \
    ISkinPool_GetSkin(This, name, scale)

/* Skin management shortcuts */
#define ISkinPool_AddSkinObj(This, skin) \
    ISkinPool_AddSkin(This, skin)

#define ISkinPool_DeleteSkin(This, skin) \
    ISkinPool_RemoveSkin(This, skin)

#define ISkinPool_Clear(This) \
    ISkinPool_RemoveAll(This)

#define ISkinPool_ClearAll(This) \
    ISkinPool_RemoveAll(This)

/* XML loading shortcuts */
#define ISkinPool_LoadFromXml(This, xmlNode) \
    ISkinPool_LoadSkins(This, xmlNode)

#define ISkinPool_ImportSkins(This, xmlNode) \
    ISkinPool_LoadSkins(This, xmlNode)

/* Safe skin pool operations */
#define ISkinPool_SafeGetSkin(This, name, scale, ppSkin) \
    do { \
        if ((This) && (name) && (ppSkin)) { \
            *(ppSkin) = ISkinPool_GetSkin(This, name, scale); \
        } else if (ppSkin) { \
            *(ppSkin) = NULL; \
        } \
    } while(0)

#define ISkinPool_SafeAddSkin(This, skin) \
    ((This) && (skin) ? ISkinPool_AddSkin(This, skin) : FALSE)

#define ISkinPool_SafeRemoveSkin(This, skin) \
    ((This) && (skin) ? ISkinPool_RemoveSkin(This, skin) : FALSE)

#define ISkinPool_SafeLoadSkins(This, xmlNode) \
    ((This) && (xmlNode) ? ISkinPool_LoadSkins(This, xmlNode) : 0)

#define ISkinPool_SafeRemoveAll(This) \
    do { \
        if (This) { \
            ISkinPool_RemoveAll(This); \
        } \
    } while(0)

/*
 * Skin pool helper functions
 */
static inline BOOL ISkinPool_HasSkin_C(ISkinPool* pThis, LPCWSTR strSkinName)
{
    if (!pThis || !strSkinName) return FALSE;
    
    ISkinObj* skin = ISkinPool_GetSkin_Default(pThis, strSkinName);
    return (skin != NULL);
}

static inline BOOL ISkinPool_HasSkinWithScale_C(ISkinPool* pThis, LPCWSTR strSkinName, int nScale)
{
    if (!pThis || !strSkinName) return FALSE;
    
    ISkinObj* skin = ISkinPool_GetSkin(pThis, strSkinName, nScale);
    return (skin != NULL);
}

static inline ISkinObj* ISkinPool_GetSkinSafe_C(ISkinPool* pThis, LPCWSTR strSkinName, int nScale)
{
    if (!pThis || !strSkinName) return NULL;
    
    return ISkinPool_GetSkin(pThis, strSkinName, nScale);
}

static inline int ISkinPool_LoadSkinsFromFile_C(ISkinPool* pThis, LPCWSTR xmlFilePath)
{
    if (!pThis || !xmlFilePath) return 0;
    
    // Note: This would require XML file loading functionality
    // For now, this is a placeholder implementation
    return 0;
}

static inline BOOL ISkinPool_ReplaceSkin_C(ISkinPool* pThis, ISkinObj* oldSkin, ISkinObj* newSkin)
{
    if (!pThis || !oldSkin || !newSkin) return FALSE;
    
    if (ISkinPool_RemoveSkin(pThis, oldSkin)) {
        return ISkinPool_AddSkin(pThis, newSkin);
    }
    return FALSE;
}

/*
 * Skin pool state management
 */
typedef struct SkinPoolState {
    ISkinPool* skinPool;
    int skinCount;
    BOOL isValid;
} SkinPoolState;

static inline void SkinPoolState_Init(SkinPoolState* state, ISkinPool* skinPool)
{
    if (state) {
        state->skinPool = skinPool;
        state->skinCount = 0; // Would need enumeration to get actual count
        state->isValid = (skinPool != NULL);
    }
}

static inline void SkinPoolState_Update(SkinPoolState* state)
{
    if (state && state->skinPool) {
        // Update skin count if enumeration is available
        state->isValid = TRUE;
    }
}

static inline BOOL SkinPoolState_AddSkin(SkinPoolState* state, ISkinObj* skin)
{
    if (!state || !state->isValid || !state->skinPool || !skin) return FALSE;
    
    if (ISkinPool_AddSkin(state->skinPool, skin)) {
        state->skinCount++;
        return TRUE;
    }
    return FALSE;
}

static inline BOOL SkinPoolState_RemoveSkin(SkinPoolState* state, ISkinObj* skin)
{
    if (!state || !state->isValid || !state->skinPool || !skin) return FALSE;
    
    if (ISkinPool_RemoveSkin(state->skinPool, skin)) {
        if (state->skinCount > 0) {
            state->skinCount--;
        }
        return TRUE;
    }
    return FALSE;
}

static inline void SkinPoolState_Clear(SkinPoolState* state)
{
    if (state && state->isValid && state->skinPool) {
        ISkinPool_RemoveAll(state->skinPool);
        state->skinCount = 0;
    }
}

/*
 * Skin collection management
 */
typedef struct SkinCollection {
    ISkinObj** skins;
    LPCWSTR* names;
    int* scales;
    int count;
    int capacity;
} SkinCollection;

static inline void SkinCollection_Init(SkinCollection* collection, int initialCapacity)
{
    if (collection) {
        collection->skins = initialCapacity > 0 ? 
            (ISkinObj**)malloc(sizeof(ISkinObj*) * initialCapacity) : NULL;
        collection->names = initialCapacity > 0 ? 
            (LPCWSTR*)malloc(sizeof(LPCWSTR) * initialCapacity) : NULL;
        collection->scales = initialCapacity > 0 ? 
            (int*)malloc(sizeof(int) * initialCapacity) : NULL;
        collection->count = 0;
        collection->capacity = initialCapacity;
    }
}

static inline void SkinCollection_Cleanup(SkinCollection* collection)
{
    if (collection) {
        if (collection->skins) {
            for (int i = 0; i < collection->count; i++) {
                if (collection->skins[i]) {
                    ISkinObj_SafeRelease(&collection->skins[i]);
                }
            }
            free(collection->skins);
        }
        if (collection->names) {
            free(collection->names);
        }
        if (collection->scales) {
            free(collection->scales);
        }
        memset(collection, 0, sizeof(SkinCollection));
    }
}

static inline BOOL SkinCollection_Add(SkinCollection* collection, ISkinObj* skin, LPCWSTR name, int scale)
{
    if (!collection || !skin || collection->count >= collection->capacity) return FALSE;
    
    ISkinObj_SafeAddRef(skin);
    collection->skins[collection->count] = skin;
    collection->names[collection->count] = name;
    collection->scales[collection->count] = scale;
    collection->count++;
    return TRUE;
}

static inline ISkinObj* SkinCollection_FindByName(SkinCollection* collection, LPCWSTR name)
{
    if (!collection || !name) return NULL;
    
    for (int i = 0; i < collection->count; i++) {
        if (collection->names[i] && wcscmp(collection->names[i], name) == 0) {
            return collection->skins[i];
        }
    }
    return NULL;
}

static inline BOOL SkinCollection_LoadToPool(SkinCollection* collection, ISkinPool* pool)
{
    if (!collection || !pool) return FALSE;
    
    int successCount = 0;
    for (int i = 0; i < collection->count; i++) {
        if (collection->skins[i] && ISkinPool_AddSkin(pool, collection->skins[i])) {
            successCount++;
        }
    }
    return (successCount == collection->count);
}

/*
 * Skin scale helpers
 */
#define SKIN_SCALE_100      100
#define SKIN_SCALE_125      125
#define SKIN_SCALE_150      150
#define SKIN_SCALE_200      200

#define ISkinPool_GetSkin100(This, name) \
    ISkinPool_GetSkin(This, name, SKIN_SCALE_100)

#define ISkinPool_GetSkin125(This, name) \
    ISkinPool_GetSkin(This, name, SKIN_SCALE_125)

#define ISkinPool_GetSkin150(This, name) \
    ISkinPool_GetSkin(This, name, SKIN_SCALE_150)

#define ISkinPool_GetSkin200(This, name) \
    ISkinPool_GetSkin(This, name, SKIN_SCALE_200)

/*
 * Reference counting helpers
 */
#define ISkinPool_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define ISkinPool_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Debugging helpers
 */
#ifdef _DEBUG
static inline void ISkinPool_DebugInfo_C(ISkinPool* pThis)
{
    if (!pThis) {
        printf("SkinPool: NULL\n");
        return;
    }
    
    printf("SkinPool: %p\n", pThis);
    // Note: Without enumeration capability, we can't show detailed skin info
    // This would require additional interface methods
}
#else
#define ISkinPool_DebugInfo_C(This) ((void)0)
#endif

/*
 * Batch operations
 */
static inline int ISkinPool_AddMultipleSkins_C(ISkinPool* pThis, ISkinObj** skins, int count)
{
    if (!pThis || !skins || count <= 0) return 0;
    
    int successCount = 0;
    for (int i = 0; i < count; i++) {
        if (skins[i] && ISkinPool_AddSkin(pThis, skins[i])) {
            successCount++;
        }
    }
    return successCount;
}

static inline int ISkinPool_RemoveMultipleSkins_C(ISkinPool* pThis, ISkinObj** skins, int count)
{
    if (!pThis || !skins || count <= 0) return 0;
    
    int successCount = 0;
    for (int i = 0; i < count; i++) {
        if (skins[i] && ISkinPool_RemoveSkin(pThis, skins[i])) {
            successCount++;
        }
    }
    return successCount;
}

#ifdef __cplusplus
}
#endif

#endif /* __SSKINPOOL_CAPI_H__ */
