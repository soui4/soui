#ifndef __SCARET_CAPI_H__
#define __SCARET_CAPI_H__

#include "../scaret-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for scaret Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* ICaret C API Macros */
#define ICaret_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ICaret_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ICaret_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define ICaret_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define ICaret_GetObjectClass(This) \
    ((This)->lpVtbl->GetObjectClass(This))

#define ICaret_IsClass(This, lpszName) \
    ((This)->lpVtbl->IsClass(This, lpszName))

#define ICaret_GetClassInfo(This) \
    ((This)->lpVtbl->GetClassInfo(This))

#define ICaret_SetAttribute(This, strName, strValue, bLoading) \
    ((This)->lpVtbl->SetAttribute(This, strName, strValue, bLoading))

#define ICaret_SetAttributeA(This, pszAttr, pszValue, bLoading) \
    ((This)->lpVtbl->SetAttributeA(This, pszAttr, pszValue, bLoading))

#define ICaret_GetAttribute(This, strName, strValue) \
    ((This)->lpVtbl->GetAttribute(This, strName, strValue))

#define ICaret_GetAttributeA(This, pszAttr, strValue) \
    ((This)->lpVtbl->GetAttributeA(This, pszAttr, strValue))

#define ICaret_AfterAttribute(This) \
    ((This)->lpVtbl->AfterAttribute(This))

#define ICaret_InitFromXml(This, xmlNode) \
    ((This)->lpVtbl->InitFromXml(This, xmlNode))

#define ICaret_OnInitFinished(This, xmlNode) \
    ((This)->lpVtbl->OnInitFinished(This, xmlNode))

#define ICaret_Init(This, hBmp, nWid, nHei) \
    ((This)->lpVtbl->Init(This, hBmp, nWid, nHei))

#define ICaret_SetPosition(This, x, y) \
    ((This)->lpVtbl->SetPosition(This, x, y))

#define ICaret_SetVisible(This, bVisible, owner) \
    ((This)->lpVtbl->SetVisible(This, bVisible, owner))

#define ICaret_IsVisible(This) \
    ((This)->lpVtbl->IsVisible(This))

#define ICaret_Draw(This, pRT) \
    ((This)->lpVtbl->Draw(This, pRT))

#define ICaret_GetRect(This) \
    ((This)->lpVtbl->GetRect(This))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* ICaret Helper Functions */
static inline long ICaret_AddRef_C(ICaret* pThis)
{
    return ICaret_AddRef(pThis);
}

static inline long ICaret_Release_C(ICaret* pThis)
{
    return ICaret_Release(pThis);
}

static inline int ICaret_GetObjectType_C(const ICaret* pThis)
{
    return ICaret_GetObjectType(pThis);
}

static inline LPCWSTR ICaret_GetObjectClass_C(const ICaret* pThis)
{
    return ICaret_GetObjectClass(pThis);
}

static inline BOOL ICaret_IsClass_C(const ICaret* pThis, LPCWSTR lpszName)
{
    return ICaret_IsClass(pThis, lpszName);
}

static inline const IObjectInfo* ICaret_GetClassInfo_C(const ICaret* pThis)
{
    return ICaret_GetClassInfo(pThis);
}

static inline HRESULT ICaret_SetAttribute_C(ICaret* pThis, const IStringW* strName, const IStringW* strValue, BOOL bLoading)
{
    return ICaret_SetAttribute(pThis, strName, strValue, bLoading);
}

static inline HRESULT ICaret_SetAttributeA_C(ICaret* pThis, LPCSTR pszAttr, LPCSTR pszValue, BOOL bLoading)
{
    return ICaret_SetAttributeA(pThis, pszAttr, pszValue, bLoading);
}

static inline BOOL ICaret_GetAttribute_C(const ICaret* pThis, const IStringW* strName, IStringW* strValue)
{
    return ICaret_GetAttribute(pThis, strName, strValue);
}

static inline BOOL ICaret_GetAttributeA_C(const ICaret* pThis, LPCSTR pszAttr, IStringA* strValue)
{
    return ICaret_GetAttributeA(pThis, pszAttr, strValue);
}

static inline void ICaret_AfterAttribute_C(ICaret* pThis)
{
    ICaret_AfterAttribute(pThis);
}

static inline BOOL ICaret_InitFromXml_C(ICaret* pThis, IXmlNode* xmlNode)
{
    return ICaret_InitFromXml(pThis, xmlNode);
}

static inline void ICaret_OnInitFinished_C(ICaret* pThis, IXmlNode* xmlNode)
{
    ICaret_OnInitFinished(pThis, xmlNode);
}

static inline BOOL ICaret_Init_C(ICaret* pThis, HBITMAP hBmp, int nWid, int nHei)
{
    return ICaret_Init(pThis, hBmp, nWid, nHei);
}

static inline void ICaret_SetPosition_C(ICaret* pThis, int x, int y)
{
    ICaret_SetPosition(pThis, x, y);
}

static inline BOOL ICaret_SetVisible_C(ICaret* pThis, BOOL bVisible, SWND owner)
{
    return ICaret_SetVisible(pThis, bVisible, owner);
}

static inline BOOL ICaret_IsVisible_C(const ICaret* pThis)
{
    return ICaret_IsVisible(pThis);
}

static inline void ICaret_Draw_C(ICaret* pThis, IRenderTarget* pRT)
{
    ICaret_Draw(pThis, pRT);
}

static inline RECT ICaret_GetRect_C(const ICaret* pThis)
{
    return ICaret_GetRect(pThis);
}

/*
 * Convenience macros for common caret operations
 */

/* Caret initialization shortcuts */
#define ICaret_InitDefault(This, width, height) \
    ICaret_Init(This, NULL, width, height)

#define ICaret_InitWithBitmap(This, bitmap, width, height) \
    ICaret_Init(This, bitmap, width, height)

/* Position management shortcuts */
#define ICaret_MoveTo(This, x, y) \
    ICaret_SetPosition(This, x, y)

#define ICaret_Move(This, x, y) \
    ICaret_SetPosition(This, x, y)

#define ICaret_SetPos(This, x, y) \
    ICaret_SetPosition(This, x, y)

/* Visibility management shortcuts */
#define ICaret_Show(This, owner) \
    ICaret_SetVisible(This, TRUE, owner)

#define ICaret_Hide(This, owner) \
    ICaret_SetVisible(This, FALSE, owner)

#define ICaret_Toggle(This, owner) \
    ICaret_SetVisible(This, !ICaret_IsVisible(This), owner)

#define ICaret_IsShown(This) \
    ICaret_IsVisible(This)

#define ICaret_IsHidden(This) \
    (!ICaret_IsVisible(This))

/* Drawing shortcuts */
#define ICaret_Render(This, renderTarget) \
    ICaret_Draw(This, renderTarget)

#define ICaret_Paint(This, renderTarget) \
    ICaret_Draw(This, renderTarget)

/* Rectangle shortcuts */
#define ICaret_GetBounds(This) \
    ICaret_GetRect(This)

#define ICaret_GetRectangle(This) \
    ICaret_GetRect(This)

/* Object information shortcuts */
#define ICaret_GetType(This) \
    ICaret_GetObjectType(This)

#define ICaret_GetClass(This) \
    ICaret_GetObjectClass(This)

#define ICaret_CheckClass(This, className) \
    ICaret_IsClass(This, className)

/* Safe caret operations */
#define ICaret_SafeInit(This, bitmap, width, height) \
    ((This) && (width) > 0 && (height) > 0 ? ICaret_Init(This, bitmap, width, height) : FALSE)

#define ICaret_SafeSetPosition(This, x, y) \
    do { \
        if (This) { \
            ICaret_SetPosition(This, x, y); \
        } \
    } while(0)

#define ICaret_SafeSetVisible(This, visible, owner) \
    ((This) ? ICaret_SetVisible(This, visible, owner) : FALSE)

#define ICaret_SafeDraw(This, renderTarget) \
    do { \
        if ((This) && (renderTarget)) { \
            ICaret_Draw(This, renderTarget); \
        } \
    } while(0)

#define ICaret_SafeGetRect(This, pRect) \
    do { \
        if ((This) && (pRect)) { \
            *(pRect) = ICaret_GetRect(This); \
        } \
    } while(0)

/*
 * Caret helper functions
 */
static inline BOOL ICaret_IsValid_C(ICaret* pThis)
{
    return (pThis != NULL);
}

static inline BOOL ICaret_InitSimple_C(ICaret* pThis, int width, int height)
{
    if (!pThis || width <= 0 || height <= 0) return FALSE;
    
    return ICaret_Init(pThis, NULL, width, height);
}

static inline void ICaret_MoveBy_C(ICaret* pThis, int deltaX, int deltaY)
{
    if (!pThis) return;
    
    RECT currentRect = ICaret_GetRect(pThis);
    ICaret_SetPosition(pThis, currentRect.left + deltaX, currentRect.top + deltaY);
}

static inline BOOL ICaret_IsAtPosition_C(ICaret* pThis, int x, int y)
{
    if (!pThis) return FALSE;
    
    RECT rect = ICaret_GetRect(pThis);
    return (rect.left == x && rect.top == y);
}

static inline BOOL ICaret_ContainsPoint_C(ICaret* pThis, int x, int y)
{
    if (!pThis) return FALSE;
    
    RECT rect = ICaret_GetRect(pThis);
    return (x >= rect.left && x < rect.right && y >= rect.top && y < rect.bottom);
}

/*
 * Caret state management
 */
typedef struct CaretState {
    ICaret* caret;
    RECT rect;
    BOOL isVisible;
    SWND owner;
    BOOL isValid;
} CaretState;

static inline void CaretState_Init(CaretState* state, ICaret* caret)
{
    if (state) {
        state->caret = caret;
        if (caret) {
            state->rect = ICaret_GetRect(caret);
            state->isVisible = ICaret_IsVisible(caret);
            state->owner = NULL; // Can't get owner directly
            state->isValid = TRUE;
        } else {
            memset(state, 0, sizeof(CaretState));
        }
    }
}

static inline void CaretState_Update(CaretState* state)
{
    if (state && state->caret) {
        CaretState_Init(state, state->caret);
    }
}

static inline BOOL CaretState_SetPosition(CaretState* state, int x, int y)
{
    if (!state || !state->caret) return FALSE;
    
    ICaret_SetPosition(state->caret, x, y);
    state->rect = ICaret_GetRect(state->caret);
    return TRUE;
}

static inline BOOL CaretState_SetVisible(CaretState* state, BOOL visible, SWND owner)
{
    if (!state || !state->caret) return FALSE;
    
    BOOL result = ICaret_SetVisible(state->caret, visible, owner);
    if (result) {
        state->isVisible = visible;
        state->owner = owner;
    }
    return result;
}

/*
 * Reference counting helpers
 */
#define ICaret_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define ICaret_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Debugging helpers
 */
#ifdef _DEBUG
static inline void ICaret_DebugInfo_C(ICaret* pThis)
{
    if (!pThis) {
        printf("Caret: NULL\n");
        return;
    }
    
    RECT rect = ICaret_GetRect(pThis);
    printf("Caret: %p\n", pThis);
    printf("  Class Name: %ls\n", ICaret_GetObjectClass(pThis));
    printf("  Object Type: %d\n", ICaret_GetObjectType(pThis));
    printf("  Visible: %s\n", ICaret_IsVisible(pThis) ? "Yes" : "No");
    printf("  Position: (%d, %d)\n", rect.left, rect.top);
    printf("  Size: %dx%d\n", rect.right - rect.left, rect.bottom - rect.top);
}

static inline void CaretState_DebugInfo(const CaretState* state)
{
    if (!state) {
        printf("CaretState: NULL\n");
        return;
    }
    
    printf("CaretState: %p\n", state);
    printf("  Caret: %p\n", state->caret);
    printf("  Valid: %s\n", state->isValid ? "Yes" : "No");
    printf("  Visible: %s\n", state->isVisible ? "Yes" : "No");
    printf("  Owner: %p\n", state->owner);
    printf("  Rect: (%d,%d,%d,%d)\n", state->rect.left, state->rect.top, state->rect.right, state->rect.bottom);
}
#else
#define ICaret_DebugInfo_C(This) ((void)0)
#define CaretState_DebugInfo(state) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SCARET_CAPI_H__ */
