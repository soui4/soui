#ifndef __SATTRSTORAGE_CAPI_H__
#define __SATTRSTORAGE_CAPI_H__

#include "../SAttrStorage-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SAttrStorage Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IAttrStorage C API Macros */
#define IAttrStorage_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IAttrStorage_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IAttrStorage_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IAttrStorage_OnSetAttribute(This, strName, strValue, bHandled) \
    ((This)->lpVtbl->OnSetAttribute(This, strName, strValue, bHandled))

#define IAttrStorage_OnGetAttribute(This, strName, strValue) \
    ((This)->lpVtbl->OnGetAttribute(This, strName, strValue))

/* IAttrStorageFactory C API Macros */
#define IAttrStorageFactory_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IAttrStorageFactory_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IAttrStorageFactory_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IAttrStorageFactory_CreateAttrStorage(This, owner, ppAttrStorage) \
    ((This)->lpVtbl->CreateAttrStorage(This, owner, ppAttrStorage))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IAttrStorage Helper Functions */
static inline long IAttrStorage_AddRef_C(IAttrStorage* pThis)
{
    return IAttrStorage_AddRef(pThis);
}

static inline long IAttrStorage_Release_C(IAttrStorage* pThis)
{
    return IAttrStorage_Release(pThis);
}

static inline void IAttrStorage_OnSetAttribute_C(IAttrStorage* pThis, const IStringW* strName, const IStringW* strValue, BOOL bHandled)
{
    IAttrStorage_OnSetAttribute(pThis, strName, strValue, bHandled);
}

static inline BOOL IAttrStorage_OnGetAttribute_C(IAttrStorage* pThis, const IStringW* strName, IStringW* strValue)
{
    return IAttrStorage_OnGetAttribute(pThis, strName, strValue);
}

/* IAttrStorageFactory Helper Functions */
static inline long IAttrStorageFactory_AddRef_C(IAttrStorageFactory* pThis)
{
    return IAttrStorageFactory_AddRef(pThis);
}

static inline long IAttrStorageFactory_Release_C(IAttrStorageFactory* pThis)
{
    return IAttrStorageFactory_Release(pThis);
}

static inline HRESULT IAttrStorageFactory_CreateAttrStorage_C(IAttrStorageFactory* pThis, IWindow* owner, IAttrStorage** ppAttrStorage)
{
    return IAttrStorageFactory_CreateAttrStorage(pThis, owner, ppAttrStorage);
}

/*
 * Convenience macros for common attribute storage operations
 */

/* Attribute setting shortcuts */
#define IAttrStorage_SetAttribute(This, name, value) \
    IAttrStorage_OnSetAttribute(This, name, value, TRUE)

#define IAttrStorage_SetAttributeUnhandled(This, name, value) \
    IAttrStorage_OnSetAttribute(This, name, value, FALSE)

#define IAttrStorage_GetAttribute(This, name, value) \
    IAttrStorage_OnGetAttribute(This, name, value)

/* Safe attribute operations */
#define IAttrStorage_SafeSetAttribute(This, name, value, handled) \
    do { \
        if ((This) && (name) && (value)) { \
            IAttrStorage_OnSetAttribute(This, name, value, handled); \
        } \
    } while(0)

#define IAttrStorage_SafeGetAttribute(This, name, value) \
    ((This) && (name) && (value) ? IAttrStorage_OnGetAttribute(This, name, value) : FALSE)

/* Factory shortcuts */
#define IAttrStorageFactory_CreateStorage(This, owner, ppStorage) \
    IAttrStorageFactory_CreateAttrStorage(This, owner, ppStorage)

#define IAttrStorageFactory_SafeCreateStorage(This, owner, ppStorage) \
    do { \
        if ((This) && (owner) && (ppStorage)) { \
            if (FAILED(IAttrStorageFactory_CreateAttrStorage(This, owner, ppStorage))) { \
                *(ppStorage) = NULL; \
            } \
        } else if (ppStorage) { \
            *(ppStorage) = NULL; \
        } \
    } while(0)

/*
 * Attribute storage helper functions
 */
static inline BOOL IAttrStorage_HasAttribute_C(IAttrStorage* pThis, const IStringW* strName)
{
    if (!pThis || !strName) return FALSE;
    
    // Create a temporary string to receive the value
    IStringW* tempValue = NULL;
    // Note: This would need proper string creation from factory
    // For now, we'll assume the caller provides a valid string object
    
    return FALSE; // Simplified implementation
}

static inline BOOL IAttrStorage_SetAttributeString_C(IAttrStorage* pThis, const IStringW* strName, LPCWSTR value, BOOL bHandled)
{
    if (!pThis || !strName || !value) return FALSE;
    
    // Create string from value
    // Note: This would need proper string creation from factory
    // For now, this is a placeholder implementation
    
    return FALSE; // Simplified implementation
}

static inline BOOL IAttrStorage_GetAttributeString_C(IAttrStorage* pThis, const IStringW* strName, LPWSTR buffer, int bufferSize)
{
    if (!pThis || !strName || !buffer || bufferSize <= 0) return FALSE;
    
    // Create temporary string to receive value
    // Note: This would need proper string creation and conversion
    // For now, this is a placeholder implementation
    
    buffer[0] = L'\0';
    return FALSE; // Simplified implementation
}

/*
 * Attribute storage state management
 */
typedef struct AttrStorageState {
    IAttrStorage* storage;
    IWindow* owner;
    BOOL isValid;
    int attributeCount;
} AttrStorageState;

static inline void AttrStorageState_Init(AttrStorageState* state, IAttrStorage* storage, IWindow* owner)
{
    if (state) {
        state->storage = storage;
        state->owner = owner;
        state->isValid = (storage != NULL);
        state->attributeCount = 0; // Would need to be tracked separately
    }
}

static inline void AttrStorageState_Cleanup(AttrStorageState* state)
{
    if (state) {
        if (state->storage) {
            IAttrStorage_SafeRelease(&state->storage);
        }
        memset(state, 0, sizeof(AttrStorageState));
    }
}

static inline BOOL AttrStorageState_SetAttribute(AttrStorageState* state, const IStringW* name, const IStringW* value, BOOL handled)
{
    if (!state || !state->isValid || !state->storage) return FALSE;
    
    IAttrStorage_OnSetAttribute(state->storage, name, value, handled);
    if (handled) {
        state->attributeCount++;
    }
    return TRUE;
}

static inline BOOL AttrStorageState_GetAttribute(AttrStorageState* state, const IStringW* name, IStringW* value)
{
    if (!state || !state->isValid || !state->storage) return FALSE;
    
    return IAttrStorage_OnGetAttribute(state->storage, name, value);
}

/*
 * Factory management helpers
 */
typedef struct AttrStorageFactory {
    IAttrStorageFactory* factory;
    BOOL isValid;
} AttrStorageFactory;

static inline void AttrStorageFactory_Init(AttrStorageFactory* mgr, IAttrStorageFactory* factory)
{
    if (mgr) {
        mgr->factory = factory;
        mgr->isValid = (factory != NULL);
        if (factory) {
            IAttrStorageFactory_SafeAddRef(factory);
        }
    }
}

static inline void AttrStorageFactory_Cleanup(AttrStorageFactory* mgr)
{
    if (mgr) {
        if (mgr->factory) {
            IAttrStorageFactory_SafeRelease(&mgr->factory);
        }
        memset(mgr, 0, sizeof(AttrStorageFactory));
    }
}

static inline IAttrStorage* AttrStorageFactory_CreateStorage(AttrStorageFactory* mgr, IWindow* owner)
{
    if (!mgr || !mgr->isValid || !mgr->factory || !owner) return NULL;
    
    IAttrStorage* storage = NULL;
    HRESULT hr = IAttrStorageFactory_CreateAttrStorage(mgr->factory, owner, &storage);
    
    return SUCCEEDED(hr) ? storage : NULL;
}

/*
 * Attribute batch operations
 */
typedef struct AttributePair {
    const IStringW* name;
    const IStringW* value;
    BOOL handled;
} AttributePair;

static inline BOOL IAttrStorage_SetMultipleAttributes_C(IAttrStorage* pThis, const AttributePair* attributes, int count)
{
    if (!pThis || !attributes || count <= 0) return FALSE;
    
    for (int i = 0; i < count; i++) {
        if (attributes[i].name && attributes[i].value) {
            IAttrStorage_OnSetAttribute(pThis, attributes[i].name, attributes[i].value, attributes[i].handled);
        }
    }
    
    return TRUE;
}

static inline int IAttrStorage_GetMultipleAttributes_C(IAttrStorage* pThis, const IStringW** names, IStringW** values, int count)
{
    if (!pThis || !names || !values || count <= 0) return 0;
    
    int successCount = 0;
    for (int i = 0; i < count; i++) {
        if (names[i] && values[i]) {
            if (IAttrStorage_OnGetAttribute(pThis, names[i], values[i])) {
                successCount++;
            }
        }
    }
    
    return successCount;
}

/*
 * Reference counting helpers
 */
#define IAttrStorage_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IAttrStorage_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

#define IAttrStorageFactory_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IAttrStorageFactory_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Debugging helpers
 */
#ifdef _DEBUG
static inline void IAttrStorage_DebugDump_C(IAttrStorage* pThis)
{
    if (!pThis) {
        printf("AttrStorage: NULL\n");
        return;
    }
    
    printf("AttrStorage: %p\n", pThis);
    // Note: Without enumeration capability, we can't dump all attributes
    // This would require additional interface methods
}
#else
#define IAttrStorage_DebugDump_C(This) ((void)0)
#endif

/*
 * Common attribute name constants (if available)
 */
// Note: These would typically be defined elsewhere in the SOUI framework
// #define ATTR_NAME_WIDTH     L"width"
// #define ATTR_NAME_HEIGHT    L"height"
// #define ATTR_NAME_VISIBLE   L"visible"
// #define ATTR_NAME_ENABLED   L"enabled"

/*
 * Attribute validation helpers
 */
static inline BOOL IAttrStorage_ValidateAttributeName_C(const IStringW* name)
{
    if (!name) return FALSE;
    
    // Basic validation - name should not be empty
    // Note: This would need proper string length checking
    // For now, just check for NULL
    return TRUE;
}

static inline BOOL IAttrStorage_ValidateAttributeValue_C(const IStringW* value)
{
    if (!value) return FALSE;
    
    // Basic validation - value can be empty but not NULL
    return TRUE;
}

static inline BOOL IAttrStorage_ValidateAttributePair_C(const IStringW* name, const IStringW* value)
{
    return IAttrStorage_ValidateAttributeName_C(name) && 
           IAttrStorage_ValidateAttributeValue_C(value);
}

/*
 * Error handling helpers
 */
typedef enum AttrStorageError {
    ATTR_ERROR_NONE = 0,
    ATTR_ERROR_NULL_POINTER,
    ATTR_ERROR_INVALID_NAME,
    ATTR_ERROR_INVALID_VALUE,
    ATTR_ERROR_NOT_FOUND,
    ATTR_ERROR_ACCESS_DENIED,
    ATTR_ERROR_FACTORY_FAILED
} AttrStorageError;

static inline AttrStorageError IAttrStorage_GetLastError_C(void)
{
    // This would need to be implemented with thread-local storage
    // For now, return no error
    return ATTR_ERROR_NONE;
}

static inline LPCWSTR AttrStorageError_ToString(AttrStorageError error)
{
    switch (error) {
        case ATTR_ERROR_NONE:           return L"No error";
        case ATTR_ERROR_NULL_POINTER:   return L"Null pointer";
        case ATTR_ERROR_INVALID_NAME:   return L"Invalid attribute name";
        case ATTR_ERROR_INVALID_VALUE:  return L"Invalid attribute value";
        case ATTR_ERROR_NOT_FOUND:      return L"Attribute not found";
        case ATTR_ERROR_ACCESS_DENIED:  return L"Access denied";
        case ATTR_ERROR_FACTORY_FAILED: return L"Factory creation failed";
        default:                        return L"Unknown error";
    }
}

#ifdef __cplusplus
}
#endif

#endif /* __SATTRSTORAGE_CAPI_H__ */
