#ifndef __SINTERPOLATOR_CAPI_H__
#define __SINTERPOLATOR_CAPI_H__

#include "../sinterpolator-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Interpolator constants
 */
#define INTERPOLATOR_INPUT_MIN      0.0f
#define INTERPOLATOR_INPUT_MAX      1.0f

/*
 * C API Helper Macros for sinterpolator Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IInterpolator C API Macros */
#define IInterpolator_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IInterpolator_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IInterpolator_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IInterpolator_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define IInterpolator_GetObjectClass(This) \
    ((This)->lpVtbl->GetObjectClass(This))

#define IInterpolator_IsClass(This, lpszName) \
    ((This)->lpVtbl->IsClass(This, lpszName))

#define IInterpolator_GetClassInfo(This) \
    ((This)->lpVtbl->GetClassInfo(This))

#define IInterpolator_SetAttribute(This, strName, strValue, bLoading) \
    ((This)->lpVtbl->SetAttribute(This, strName, strValue, bLoading))

#define IInterpolator_SetAttributeA(This, pszAttr, pszValue, bLoading) \
    ((This)->lpVtbl->SetAttributeA(This, pszAttr, pszValue, bLoading))

#define IInterpolator_GetAttribute(This, strName, strValue) \
    ((This)->lpVtbl->GetAttribute(This, strName, strValue))

#define IInterpolator_GetAttributeA(This, pszAttr, strValue) \
    ((This)->lpVtbl->GetAttributeA(This, pszAttr, strValue))

#define IInterpolator_AfterAttribute(This) \
    ((This)->lpVtbl->AfterAttribute(This))

#define IInterpolator_InitFromXml(This, xmlNode) \
    ((This)->lpVtbl->InitFromXml(This, xmlNode))

#define IInterpolator_OnInitFinished(This, xmlNode) \
    ((This)->lpVtbl->OnInitFinished(This, xmlNode))

#define IInterpolator_getInterpolation(This, input) \
    ((This)->lpVtbl->getInterpolation(This, input))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IInterpolator Helper Functions */
static inline long IInterpolator_AddRef_C(IInterpolator* pThis)
{
    return IInterpolator_AddRef(pThis);
}

static inline long IInterpolator_Release_C(IInterpolator* pThis)
{
    return IInterpolator_Release(pThis);
}

static inline int IInterpolator_GetObjectType_C(const IInterpolator* pThis)
{
    return IInterpolator_GetObjectType(pThis);
}

static inline LPCWSTR IInterpolator_GetObjectClass_C(const IInterpolator* pThis)
{
    return IInterpolator_GetObjectClass(pThis);
}

static inline BOOL IInterpolator_IsClass_C(const IInterpolator* pThis, LPCWSTR lpszName)
{
    return IInterpolator_IsClass(pThis, lpszName);
}

static inline const IObjectInfo* IInterpolator_GetClassInfo_C(const IInterpolator* pThis)
{
    return IInterpolator_GetClassInfo(pThis);
}

static inline HRESULT IInterpolator_SetAttribute_C(IInterpolator* pThis, const IStringW* strName, const IStringW* strValue, BOOL bLoading)
{
    return IInterpolator_SetAttribute(pThis, strName, strValue, bLoading);
}

static inline HRESULT IInterpolator_SetAttributeA_C(IInterpolator* pThis, LPCSTR pszAttr, LPCSTR pszValue, BOOL bLoading)
{
    return IInterpolator_SetAttributeA(pThis, pszAttr, pszValue, bLoading);
}

static inline BOOL IInterpolator_GetAttribute_C(const IInterpolator* pThis, const IStringW* strName, IStringW* strValue)
{
    return IInterpolator_GetAttribute(pThis, strName, strValue);
}

static inline BOOL IInterpolator_GetAttributeA_C(const IInterpolator* pThis, LPCSTR pszAttr, IStringA* strValue)
{
    return IInterpolator_GetAttributeA(pThis, pszAttr, strValue);
}

static inline void IInterpolator_AfterAttribute_C(IInterpolator* pThis)
{
    IInterpolator_AfterAttribute(pThis);
}

static inline BOOL IInterpolator_InitFromXml_C(IInterpolator* pThis, IXmlNode* xmlNode)
{
    return IInterpolator_InitFromXml(pThis, xmlNode);
}

static inline void IInterpolator_OnInitFinished_C(IInterpolator* pThis, IXmlNode* xmlNode)
{
    IInterpolator_OnInitFinished(pThis, xmlNode);
}

static inline float IInterpolator_getInterpolation_C(const IInterpolator* pThis, float input)
{
    return IInterpolator_getInterpolation(pThis, input);
}

/*
 * Convenience macros for common interpolator operations
 */

/* Interpolation shortcuts */
#define IInterpolator_Interpolate(This, input) \
    IInterpolator_getInterpolation(This, input)

#define IInterpolator_Calculate(This, input) \
    IInterpolator_getInterpolation(This, input)

#define IInterpolator_GetValue(This, input) \
    IInterpolator_getInterpolation(This, input)

#define IInterpolator_Evaluate(This, input) \
    IInterpolator_getInterpolation(This, input)

/* Common interpolation values */
#define IInterpolator_GetStart(This) \
    IInterpolator_getInterpolation(This, INTERPOLATOR_INPUT_MIN)

#define IInterpolator_GetEnd(This) \
    IInterpolator_getInterpolation(This, INTERPOLATOR_INPUT_MAX)

#define IInterpolator_GetMiddle(This) \
    IInterpolator_getInterpolation(This, 0.5f)

#define IInterpolator_GetQuarter(This) \
    IInterpolator_getInterpolation(This, 0.25f)

#define IInterpolator_GetThreeQuarter(This) \
    IInterpolator_getInterpolation(This, 0.75f)

/* Object information shortcuts */
#define IInterpolator_GetType(This) \
    IInterpolator_GetObjectType(This)

#define IInterpolator_GetClass(This) \
    IInterpolator_GetObjectClass(This)

#define IInterpolator_CheckClass(This, className) \
    IInterpolator_IsClass(This, className)

/* Safe interpolator operations */
#define IInterpolator_SafeGetInterpolation(This, input) \
    ((This) ? IInterpolator_getInterpolation(This, input) : 0.0f)

#define IInterpolator_SafeInterpolate(This, input, defaultValue) \
    ((This) ? IInterpolator_getInterpolation(This, input) : (defaultValue))

#define IInterpolator_SafeSetAttribute(This, name, value, loading) \
    ((This) && (name) && (value) ? IInterpolator_SetAttribute(This, name, value, loading) : E_INVALIDARG)

#define IInterpolator_SafeInitFromXml(This, xmlNode) \
    ((This) && (xmlNode) ? IInterpolator_InitFromXml(This, xmlNode) : FALSE)

/*
 * Interpolator helper functions
 */
static inline BOOL IInterpolator_IsValid_C(IInterpolator* pThis)
{
    return (pThis != NULL);
}

static inline BOOL IInterpolator_IsValidInput_C(float input)
{
    return (input >= INTERPOLATOR_INPUT_MIN && input <= INTERPOLATOR_INPUT_MAX);
}

static inline float IInterpolator_ClampInput_C(float input)
{
    if (input < INTERPOLATOR_INPUT_MIN) return INTERPOLATOR_INPUT_MIN;
    if (input > INTERPOLATOR_INPUT_MAX) return INTERPOLATOR_INPUT_MAX;
    return input;
}

static inline float IInterpolator_GetInterpolationSafe_C(const IInterpolator* pThis, float input)
{
    if (!pThis) return 0.0f;
    
    float clampedInput = IInterpolator_ClampInput_C(input);
    return IInterpolator_getInterpolation(pThis, clampedInput);
}

static inline float IInterpolator_GetInterpolationRange_C(const IInterpolator* pThis, float input, float minOutput, float maxOutput)
{
    if (!pThis) return minOutput;
    
    float normalizedValue = IInterpolator_GetInterpolationSafe_C(pThis, input);
    return minOutput + normalizedValue * (maxOutput - minOutput);
}

/*
 * Interpolator state management
 */
typedef struct InterpolatorState {
    IInterpolator* interpolator;
    LPCWSTR className;
    int objectType;
    float lastInput;
    float lastOutput;
    BOOL isValid;
} InterpolatorState;

static inline void InterpolatorState_Init(InterpolatorState* state, IInterpolator* interpolator)
{
    if (state) {
        state->interpolator = interpolator;
        if (interpolator) {
            state->className = IInterpolator_GetObjectClass(interpolator);
            state->objectType = IInterpolator_GetObjectType(interpolator);
            state->lastInput = 0.0f;
            state->lastOutput = IInterpolator_getInterpolation(interpolator, 0.0f);
            state->isValid = TRUE;
        } else {
            memset(state, 0, sizeof(InterpolatorState));
        }
    }
}

static inline void InterpolatorState_Update(InterpolatorState* state)
{
    if (state && state->interpolator) {
        InterpolatorState_Init(state, state->interpolator);
    }
}

static inline float InterpolatorState_GetInterpolation(InterpolatorState* state, float input)
{
    if (!state || !state->interpolator) return 0.0f;
    
    state->lastInput = input;
    state->lastOutput = IInterpolator_getInterpolation(state->interpolator, input);
    return state->lastOutput;
}

/*
 * Interpolation curve sampling
 */
typedef struct InterpolationSample {
    float input;
    float output;
} InterpolationSample;

static inline void IInterpolator_SampleCurve_C(const IInterpolator* pThis, InterpolationSample* samples, int sampleCount)
{
    if (!pThis || !samples || sampleCount <= 0) return;
    
    for (int i = 0; i < sampleCount; i++) {
        float input = (float)i / (float)(sampleCount - 1);
        samples[i].input = input;
        samples[i].output = IInterpolator_getInterpolation(pThis, input);
    }
}

static inline float IInterpolator_GetDerivative_C(const IInterpolator* pThis, float input, float epsilon)
{
    if (!pThis) return 0.0f;
    
    if (epsilon <= 0.0f) epsilon = 0.001f;
    
    float input1 = IInterpolator_ClampInput_C(input - epsilon);
    float input2 = IInterpolator_ClampInput_C(input + epsilon);
    
    float output1 = IInterpolator_getInterpolation(pThis, input1);
    float output2 = IInterpolator_getInterpolation(pThis, input2);
    
    return (output2 - output1) / (input2 - input1);
}

/*
 * Reference counting helpers
 */
#define IInterpolator_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IInterpolator_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Debugging helpers
 */
#ifdef _DEBUG
static inline void IInterpolator_DebugInfo_C(IInterpolator* pThis)
{
    if (!pThis) {
        printf("Interpolator: NULL\n");
        return;
    }
    
    printf("Interpolator: %p\n", pThis);
    printf("  Class Name: %ls\n", IInterpolator_GetObjectClass(pThis));
    printf("  Object Type: %d\n", IInterpolator_GetObjectType(pThis));
    printf("  Start Value: %f\n", IInterpolator_GetStart(pThis));
    printf("  Middle Value: %f\n", IInterpolator_GetMiddle(pThis));
    printf("  End Value: %f\n", IInterpolator_GetEnd(pThis));
}

static inline void InterpolatorState_DebugInfo(const InterpolatorState* state)
{
    if (!state) {
        printf("InterpolatorState: NULL\n");
        return;
    }
    
    printf("InterpolatorState: %p\n", state);
    printf("  Interpolator: %p\n", state->interpolator);
    printf("  Class Name: %ls\n", state->className);
    printf("  Object Type: %d\n", state->objectType);
    printf("  Last Input: %f\n", state->lastInput);
    printf("  Last Output: %f\n", state->lastOutput);
    printf("  Valid: %s\n", state->isValid ? "Yes" : "No");
}
#else
#define IInterpolator_DebugInfo_C(This) ((void)0)
#define InterpolatorState_DebugInfo(state) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SINTERPOLATOR_CAPI_H__ */
