#ifndef __SOBJFACTORY_CAPI_H__
#define __SOBJFACTORY_CAPI_H__

#include "../SObjFactory-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Object factory constants
 */
#define MAX_OBJECT_NAME MAX_OBJNAME

/*
 * C API Helper Macros for SObjFactory Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IObjectFactory C API Macros */
#define IObjectFactory_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IObjectFactory_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IObjectFactory_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IObjectFactory_NewObject(This) \
    ((This)->lpVtbl->NewObject(This))

#define IObjectFactory_BaseClassName(This) \
    ((This)->lpVtbl->BaseClassName(This))

#define IObjectFactory_GetObjectInfo(This, info) \
    ((This)->lpVtbl->GetObjectInfo(This, info))

#define IObjectFactory_Clone(This) \
    ((This)->lpVtbl->Clone(This))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IObjectFactory Helper Functions */
static inline long IObjectFactory_AddRef_C(IObjectFactory* pThis)
{
    return IObjectFactory_AddRef(pThis);
}

static inline long IObjectFactory_Release_C(IObjectFactory* pThis)
{
    return IObjectFactory_Release(pThis);
}

static inline IObject* IObjectFactory_NewObject_C(const IObjectFactory* pThis)
{
    return IObjectFactory_NewObject(pThis);
}

static inline LPCWSTR IObjectFactory_BaseClassName_C(const IObjectFactory* pThis)
{
    return IObjectFactory_BaseClassName(pThis);
}

static inline void IObjectFactory_GetObjectInfo_C(const IObjectFactory* pThis, SObjectInfo* info)
{
    IObjectFactory_GetObjectInfo(pThis, info);
}

static inline IObjectFactory* IObjectFactory_Clone_C(const IObjectFactory* pThis)
{
    return IObjectFactory_Clone(pThis);
}

/*
 * Convenience macros for common object factory operations
 */

/* Object creation shortcuts */
#define IObjectFactory_CreateObject(This) \
    IObjectFactory_NewObject(This)

#define IObjectFactory_CreateInstance(This) \
    IObjectFactory_NewObject(This)

#define IObjectFactory_Instantiate(This) \
    IObjectFactory_NewObject(This)

/* Information access shortcuts */
#define IObjectFactory_GetClassName(This) \
    IObjectFactory_BaseClassName(This)

#define IObjectFactory_GetBaseClass(This) \
    IObjectFactory_BaseClassName(This)

#define IObjectFactory_GetInfo(This, info) \
    IObjectFactory_GetObjectInfo(This, info)

/* Factory management shortcuts */
#define IObjectFactory_Duplicate(This) \
    IObjectFactory_Clone(This)

#define IObjectFactory_Copy(This) \
    IObjectFactory_Clone(This)

/* Safe object factory operations */
#define IObjectFactory_SafeNewObject(This, ppObject) \
    do { \
        if ((This) && (ppObject)) { \
            *(ppObject) = IObjectFactory_NewObject(This); \
        } else if (ppObject) { \
            *(ppObject) = NULL; \
        } \
    } while(0)

#define IObjectFactory_SafeGetInfo(This, info) \
    do { \
        if ((This) && (info)) { \
            IObjectFactory_GetObjectInfo(This, info); \
        } else if (info) { \
            memset(info, 0, sizeof(SObjectInfo)); \
        } \
    } while(0)

#define IObjectFactory_SafeClone(This, ppClone) \
    do { \
        if ((This) && (ppClone)) { \
            *(ppClone) = IObjectFactory_Clone(This); \
        } else if (ppClone) { \
            *(ppClone) = NULL; \
        } \
    } while(0)

/*
 * SObjectInfo helper functions
 */
static inline void SObjectInfo_Init(SObjectInfo* info)
{
    if (info) {
        memset(info, 0, sizeof(SObjectInfo));
    }
}

static inline void SObjectInfo_SetName(SObjectInfo* info, LPCWSTR name)
{
    if (info && name) {
        wcsncpy_s(info->szName, MAX_OBJNAME, name, _TRUNCATE);
    }
}

static inline void SObjectInfo_SetAlias(SObjectInfo* info, LPCWSTR alias)
{
    if (info) {
        info->szAlise = alias;
    }
}

static inline void SObjectInfo_SetType(SObjectInfo* info, int type)
{
    if (info) {
        info->nType = type;
    }
}

static inline LPCWSTR SObjectInfo_GetName(const SObjectInfo* info)
{
    return info ? info->szName : NULL;
}

static inline LPCWSTR SObjectInfo_GetAlias(const SObjectInfo* info)
{
    return info ? info->szAlise : NULL;
}

static inline int SObjectInfo_GetType(const SObjectInfo* info)
{
    return info ? info->nType : 0;
}

static inline BOOL SObjectInfo_HasName(const SObjectInfo* info)
{
    return info && info->szName[0] != L'\0';
}

static inline BOOL SObjectInfo_HasAlias(const SObjectInfo* info)
{
    return info && info->szAlise && info->szAlise[0] != L'\0';
}

static inline BOOL SObjectInfo_IsValid_C(const SObjectInfo* info)
{
    return ObjInfo_IsValid(info);
}

static inline void SObjectInfo_New_C(SObjectInfo* info, LPCWSTR name, int type, LPCWSTR alias)
{
    ObjInfo_New(info, name, type, alias);
}

/*
 * Object factory helper functions
 */
static inline BOOL IObjectFactory_IsValid_C(const IObjectFactory* pThis)
{
    return (pThis != NULL);
}

static inline BOOL IObjectFactory_CanCreateObject_C(const IObjectFactory* pThis)
{
    if (!pThis) return FALSE;
    
    // Try to get object info to verify factory is functional
    SObjectInfo info;
    IObjectFactory_GetObjectInfo(pThis, &info);
    return SObjectInfo_IsValid_C(&info);
}

static inline LPCWSTR IObjectFactory_GetObjectName_C(const IObjectFactory* pThis)
{
    if (!pThis) return NULL;
    
    static SObjectInfo info;
    IObjectFactory_GetObjectInfo(pThis, &info);
    return SObjectInfo_GetName(&info);
}

static inline int IObjectFactory_GetObjectType_C(const IObjectFactory* pThis)
{
    if (!pThis) return 0;
    
    SObjectInfo info;
    IObjectFactory_GetObjectInfo(pThis, &info);
    return SObjectInfo_GetType(&info);
}

static inline BOOL IObjectFactory_CompareClassName_C(const IObjectFactory* pThis, LPCWSTR className)
{
    if (!pThis || !className) return FALSE;
    
    LPCWSTR baseClassName = IObjectFactory_BaseClassName(pThis);
    return baseClassName ? (wcscmp(baseClassName, className) == 0) : FALSE;
}

static inline BOOL IObjectFactory_CompareObjectName_C(const IObjectFactory* pThis, LPCWSTR objectName)
{
    if (!pThis || !objectName) return FALSE;
    
    LPCWSTR name = IObjectFactory_GetObjectName_C(pThis);
    return name ? (wcscmp(name, objectName) == 0) : FALSE;
}

/*
 * Object creation with error handling
 */
static inline IObject* IObjectFactory_CreateObjectSafe_C(const IObjectFactory* pThis)
{
    if (!pThis || !IObjectFactory_CanCreateObject_C(pThis)) {
        return NULL;
    }
    
    return IObjectFactory_NewObject(pThis);
}

static inline BOOL IObjectFactory_CreateObjectEx_C(const IObjectFactory* pThis, IObject** ppObject, SObjectInfo* pInfo)
{
    if (!pThis || !ppObject) return FALSE;
    
    if (pInfo) {
        IObjectFactory_GetObjectInfo(pThis, pInfo);
    }
    
    *ppObject = IObjectFactory_NewObject(pThis);
    return (*ppObject != NULL);
}

/*
 * Factory comparison helpers
 */
static inline BOOL IObjectFactory_IsSameType_C(const IObjectFactory* pThis, const IObjectFactory* pOther)
{
    if (!pThis || !pOther) return FALSE;
    if (pThis == pOther) return TRUE;
    
    LPCWSTR className1 = IObjectFactory_BaseClassName(pThis);
    LPCWSTR className2 = IObjectFactory_BaseClassName(pOther);
    
    if (!className1 || !className2) return FALSE;
    
    return wcscmp(className1, className2) == 0;
}

static inline BOOL IObjectFactory_IsSameObject_C(const IObjectFactory* pThis, const IObjectFactory* pOther)
{
    if (!pThis || !pOther) return FALSE;
    if (pThis == pOther) return TRUE;
    
    SObjectInfo info1, info2;
    IObjectFactory_GetObjectInfo(pThis, &info1);
    IObjectFactory_GetObjectInfo(pOther, &info2);
    
    return (wcscmp(info1.szName, info2.szName) == 0) && (info1.nType == info2.nType);
}

/*
 * Reference counting helpers
 */
#define IObjectFactory_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IObjectFactory_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Object factory state management
 */
typedef struct ObjectFactoryState {
    IObjectFactory* factory;
    SObjectInfo objectInfo;
    LPCWSTR baseClassName;
    BOOL isValid;
    BOOL canCreate;
} ObjectFactoryState;

static inline void ObjectFactoryState_Init(ObjectFactoryState* state, IObjectFactory* factory)
{
    if (state) {
        state->factory = factory;
        if (factory) {
            IObjectFactory_GetObjectInfo(factory, &state->objectInfo);
            state->baseClassName = IObjectFactory_BaseClassName(factory);
            state->isValid = TRUE;
            state->canCreate = IObjectFactory_CanCreateObject_C(factory);
        } else {
            memset(&state->objectInfo, 0, sizeof(SObjectInfo));
            state->baseClassName = NULL;
            state->isValid = FALSE;
            state->canCreate = FALSE;
        }
    }
}

static inline void ObjectFactoryState_Update(ObjectFactoryState* state)
{
    if (state && state->factory) {
        ObjectFactoryState_Init(state, state->factory);
    }
}

static inline IObject* ObjectFactoryState_CreateObject(ObjectFactoryState* state)
{
    if (!state || !state->isValid || !state->canCreate) return NULL;
    
    return IObjectFactory_NewObject(state->factory);
}

static inline IObjectFactory* ObjectFactoryState_Clone(ObjectFactoryState* state)
{
    if (!state || !state->isValid) return NULL;
    
    return IObjectFactory_Clone(state->factory);
}

/*
 * Debugging helpers
 */
#ifdef _DEBUG
static inline void IObjectFactory_DebugInfo_C(const IObjectFactory* pThis)
{
    if (!pThis) {
        printf("ObjectFactory: NULL\n");
        return;
    }
    
    SObjectInfo info;
    IObjectFactory_GetObjectInfo(pThis, &info);
    LPCWSTR baseClassName = IObjectFactory_BaseClassName(pThis);
    
    printf("ObjectFactory: %p\n", pThis);
    printf("  Object Name: %ls\n", info.szName);
    printf("  Object Alias: %ls\n", info.szAlise ? info.szAlise : L"(none)");
    printf("  Object Type: %d\n", info.nType);
    printf("  Base Class: %ls\n", baseClassName ? baseClassName : L"(none)");
    printf("  Can Create: %s\n", IObjectFactory_CanCreateObject_C(pThis) ? "Yes" : "No");
}
#else
#define IObjectFactory_DebugInfo_C(This) ((void)0)
#endif

/*
 * Object factory collection helpers
 */
typedef struct ObjectFactoryCollection {
    IObjectFactory** factories;
    int count;
    int capacity;
} ObjectFactoryCollection;

static inline void ObjectFactoryCollection_Init(ObjectFactoryCollection* collection, int initialCapacity)
{
    if (collection) {
        collection->factories = initialCapacity > 0 ? 
            (IObjectFactory**)malloc(sizeof(IObjectFactory*) * initialCapacity) : NULL;
        collection->count = 0;
        collection->capacity = initialCapacity;
    }
}

static inline void ObjectFactoryCollection_Cleanup(ObjectFactoryCollection* collection)
{
    if (collection) {
        for (int i = 0; i < collection->count; i++) {
            if (collection->factories[i]) {
                IObjectFactory_SafeRelease(&collection->factories[i]);
            }
        }
        if (collection->factories) {
            free(collection->factories);
        }
        memset(collection, 0, sizeof(ObjectFactoryCollection));
    }
}

static inline BOOL ObjectFactoryCollection_Add(ObjectFactoryCollection* collection, IObjectFactory* factory)
{
    if (!collection || !factory || collection->count >= collection->capacity) return FALSE;
    
    IObjectFactory_SafeAddRef(factory);
    collection->factories[collection->count++] = factory;
    return TRUE;
}

static inline IObjectFactory* ObjectFactoryCollection_FindByName(ObjectFactoryCollection* collection, LPCWSTR name)
{
    if (!collection || !name) return NULL;
    
    for (int i = 0; i < collection->count; i++) {
        if (IObjectFactory_CompareObjectName_C(collection->factories[i], name)) {
            return collection->factories[i];
        }
    }
    return NULL;
}

#ifdef __cplusplus
}
#endif

#endif /* __SOBJFACTORY_CAPI_H__ */
