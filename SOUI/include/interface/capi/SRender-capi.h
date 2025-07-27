#ifndef __SRENDER_CAPI_H__
#define __SRENDER_CAPI_H__

#include "../SRender-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SRender Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IRenderObj C API Macros */
#define IRenderObj_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IRenderObj_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IRenderObj_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IRenderObj_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

/* IBrushS C API Macros */
#define IBrushS_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IBrushS_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IBrushS_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IBrushS_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define IBrushS_GetBrushType(This) \
    ((This)->lpVtbl->GetBrushType(This))

/* IPenS C API Macros */
#define IPenS_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IPenS_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IPenS_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IPenS_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define IPenS_GetWidth(This) \
    ((This)->lpVtbl->GetWidth(This))

#define IPenS_GetStyle(This) \
    ((This)->lpVtbl->GetStyle(This))

#define IPenS_GetColor(This) \
    ((This)->lpVtbl->GetColor(This))

/* IBitmapS C API Macros */
#define IBitmapS_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IBitmapS_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IBitmapS_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IBitmapS_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define IBitmapS_Init(This, nWid, nHei, pBits) \
    ((This)->lpVtbl->Init(This, nWid, nHei, pBits))

#define IBitmapS_Init2(This, nWid, nHei) \
    ((This)->lpVtbl->Init2(This, nWid, nHei))

#define IBitmapS_LoadFromFile(This, pszFileName) \
    ((This)->lpVtbl->LoadFromFile(This, pszFileName))

#define IBitmapS_LoadFromMemory(This, pBuf, size) \
    ((This)->lpVtbl->LoadFromMemory(This, pBuf, size))

#define IBitmapS_Width(This) \
    ((This)->lpVtbl->Width(This))

#define IBitmapS_Height(This) \
    ((This)->lpVtbl->Height(This))

#define IBitmapS_Size(This) \
    ((This)->lpVtbl->Size(This))

#define IBitmapS_LockPixelBits(This) \
    ((This)->lpVtbl->LockPixelBits(This))

#define IBitmapS_UnlockPixelBits(This, pBuf) \
    ((This)->lpVtbl->UnlockPixelBits(This, pBuf))

#define IBitmapS_Clone(This, ppClone) \
    ((This)->lpVtbl->Clone(This, ppClone))

#define IBitmapS_Scale(This, ppOutput, nScale, filterLevel) \
    ((This)->lpVtbl->Scale(This, ppOutput, nScale, filterLevel))

#define IBitmapS_Scale2(This, ppOutput, nWid, nHei, filterLevel) \
    ((This)->lpVtbl->Scale2(This, ppOutput, nWid, nHei, filterLevel))

#define IBitmapS_Save(This, pszFileName, pFormat) \
    ((This)->lpVtbl->Save(This, pszFileName, pFormat))

/* IFontS C API Macros */
#define IFontS_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IFontS_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IFontS_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IFontS_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define IFontS_LogFont(This) \
    ((This)->lpVtbl->LogFont(This))

#define IFontS_FamilyName(This) \
    ((This)->lpVtbl->FamilyName(This))

#define IFontS_TextSize(This) \
    ((This)->lpVtbl->TextSize(This))

#define IFontS_IsBold(This) \
    ((This)->lpVtbl->IsBold(This))

#define IFontS_IsUnderline(This) \
    ((This)->lpVtbl->IsUnderline(This))

#define IFontS_IsItalic(This) \
    ((This)->lpVtbl->IsItalic(This))

#define IFontS_IsStrikeOut(This) \
    ((This)->lpVtbl->IsStrikeOut(This))

/* IRegionS C API Macros */
#define IRegionS_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IRegionS_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IRegionS_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IRegionS_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define IRegionS_CombineRect(This, lpRect, nCombineMode) \
    ((This)->lpVtbl->CombineRect(This, lpRect, nCombineMode))

#define IRegionS_CombineRoundRect(This, lpRect, ptConner, nCombineMode) \
    ((This)->lpVtbl->CombineRoundRect(This, lpRect, ptConner, nCombineMode))

#define IRegionS_CombineEllipse(This, lpRect, nCombineMode) \
    ((This)->lpVtbl->CombineEllipse(This, lpRect, nCombineMode))

#define IRegionS_CombinePolygon(This, pts, nCount, nPolygonMode, nCombineMode) \
    ((This)->lpVtbl->CombinePolygon(This, pts, nCount, nPolygonMode, nCombineMode))

#define IRegionS_PtInRegion(This, pt) \
    ((This)->lpVtbl->PtInRegion(This, pt))

#define IRegionS_RectInRegion(This, lpRect) \
    ((This)->lpVtbl->RectInRegion(This, lpRect))

#define IRegionS_GetRgnBox(This, lpRect) \
    ((This)->lpVtbl->GetRgnBox(This, lpRect))

#define IRegionS_IsEmpty(This) \
    ((This)->lpVtbl->IsEmpty(This))

#define IRegionS_Offset(This, ptOffset) \
    ((This)->lpVtbl->Offset(This, ptOffset))

#define IRegionS_Clear(This) \
    ((This)->lpVtbl->Clear(This))

/* IPathS C API Macros */
#define IPathS_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IPathS_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IPathS_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IPathS_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define IPathS_MoveTo(This, x, y) \
    ((This)->lpVtbl->MoveTo(This, x, y))

#define IPathS_LineTo(This, x, y) \
    ((This)->lpVtbl->LineTo(This, x, y))

#define IPathS_CubicTo(This, x1, y1, x2, y2, x3, y3) \
    ((This)->lpVtbl->CubicTo(This, x1, y1, x2, y2, x3, y3))

#define IPathS_QuadTo(This, x1, y1, x2, y2) \
    ((This)->lpVtbl->QuadTo(This, x1, y1, x2, y2))

#define IPathS_ArcTo(This, oval, startAngle, sweepAngle, forceMoveTo) \
    ((This)->lpVtbl->ArcTo(This, oval, startAngle, sweepAngle, forceMoveTo))

#define IPathS_Close(This) \
    ((This)->lpVtbl->Close(This))

#define IPathS_Reset(This) \
    ((This)->lpVtbl->Reset(This))

#define IPathS_Offset(This, dx, dy) \
    ((This)->lpVtbl->Offset(This, dx, dy))

#define IPathS_IsEmpty(This) \
    ((This)->lpVtbl->IsEmpty(This))

#define IPathS_GetBounds(This, prc) \
    ((This)->lpVtbl->GetBounds(This, prc))

#define IPathS_AddRect(This, left, top, right, bottom, dir) \
    ((This)->lpVtbl->AddRect(This, left, top, right, bottom, dir))

#define IPathS_AddOval(This, oval, dir) \
    ((This)->lpVtbl->AddOval(This, oval, dir))

#define IPathS_AddCircle(This, x, y, radius, dir) \
    ((This)->lpVtbl->AddCircle(This, x, y, radius, dir))

#define IPathS_AddArc(This, oval, startAngle, sweepAngle) \
    ((This)->lpVtbl->AddArc(This, oval, startAngle, sweepAngle))

#define IPathS_AddRoundRect(This, rect, rx, ry, dir) \
    ((This)->lpVtbl->AddRoundRect(This, rect, rx, ry, dir))

#define IPathS_AddPoly(This, pts, count, close) \
    ((This)->lpVtbl->AddPoly(This, pts, count, close))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IRenderObj Helper Functions */
static inline long IRenderObj_AddRef_C(IRenderObj* pThis)
{
    return IRenderObj_AddRef(pThis);
}

static inline long IRenderObj_Release_C(IRenderObj* pThis)
{
    return IRenderObj_Release(pThis);
}

static inline void IRenderObj_OnFinalRelease_C(IRenderObj* pThis)
{
    IRenderObj_OnFinalRelease(pThis);
}

static inline OBJTYPE IRenderObj_GetObjectType_C(IRenderObj* pThis)
{
    return IRenderObj_GetObjectType(pThis);
}

/* IBitmapS Helper Functions */
static inline HRESULT IBitmapS_Init_C(IBitmapS* pThis, int nWid, int nHei, const LPVOID pBits)
{
    return IBitmapS_Init(pThis, nWid, nHei, pBits);
}

static inline HRESULT IBitmapS_LoadFromFile_C(IBitmapS* pThis, LPCWSTR pszFileName)
{
    return IBitmapS_LoadFromFile(pThis, pszFileName);
}

static inline UINT IBitmapS_Width_C(IBitmapS* pThis)
{
    return IBitmapS_Width(pThis);
}

static inline UINT IBitmapS_Height_C(IBitmapS* pThis)
{
    return IBitmapS_Height(pThis);
}

#ifdef __cplusplus
}
#endif

#endif /* __SRENDER_CAPI_H__ */
