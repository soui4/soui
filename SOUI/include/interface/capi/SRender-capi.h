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

/* ISvgObj C API Macros */
#define ISvgObj_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ISvgObj_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ISvgObj_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define ISvgObj_Load(This, strSvg) \
    ((This)->lpVtbl->Load(This, strSvg))

#define ISvgObj_Free(This) \
    ((This)->lpVtbl->Free(This))

#define ISvgObj_GetPtr(This) \
    ((This)->lpVtbl->GetPtr(This))

#define ISvgObj_GetWidth(This) \
    ((This)->lpVtbl->GetWidth(This))

#define ISvgObj_GetHeight(This) \
    ((This)->lpVtbl->GetHeight(This))

#define ISvgObj_Size(This) \
    ((This)->lpVtbl->Size(This))

/* IRenderObj C API Macros */
#define IRenderObj_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IRenderObj_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IRenderObj_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IRenderObj_GetRenderFactory(This) \
    ((This)->lpVtbl->GetRenderFactory(This))

#define IRenderObj_ObjectType(This) \
    ((This)->lpVtbl->ObjectType(This))

/* IBrushS C API Macros */
#define IBrushS_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IBrushS_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IBrushS_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IBrushS_GetRenderFactory(This) \
    ((This)->lpVtbl->GetRenderFactory(This))

#define IBrushS_ObjectType(This) \
    ((This)->lpVtbl->ObjectType(This))

#define IBrushS_GetBrushType(This) \
    ((This)->lpVtbl->GetBrushType(This))

/* IPenS C API Macros */
#define IPenS_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IPenS_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IPenS_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IPenS_GetRenderFactory(This) \
    ((This)->lpVtbl->GetRenderFactory(This))

#define IPenS_ObjectType(This) \
    ((This)->lpVtbl->ObjectType(This))

#define IPenS_GetWidth(This) \
    ((This)->lpVtbl->GetWidth(This))

#define IPenS_SetWidth(This, nWid) \
    ((This)->lpVtbl->SetWidth(This, nWid))

#define IPenS_GetStyle(This) \
    ((This)->lpVtbl->GetStyle(This))

#define IPenS_SetStyle(This, nStyle) \
    ((This)->lpVtbl->SetStyle(This, nStyle))

#define IPenS_GetColor(This) \
    ((This)->lpVtbl->GetColor(This))

#define IPenS_SetColor(This, cr) \
    ((This)->lpVtbl->SetColor(This, cr))

/* IBitmapS C API Macros */
#define IBitmapS_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IBitmapS_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IBitmapS_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IBitmapS_GetRenderFactory(This) \
    ((This)->lpVtbl->GetRenderFactory(This))

#define IBitmapS_ObjectType(This) \
    ((This)->lpVtbl->ObjectType(This))

#define IBitmapS_Init(This, nWid, nHei, pBits) \
    ((This)->lpVtbl->Init(This, nWid, nHei, pBits))

#define IBitmapS_Init2(This, pImgFrame) \
    ((This)->lpVtbl->Init2(This, pImgFrame))

#define IBitmapS_LoadFromFile(This, pszFileName) \
    ((This)->lpVtbl->LoadFromFile(This, pszFileName))

#define IBitmapS_LoadFromMemory(This, pBuf, szLen) \
    ((This)->lpVtbl->LoadFromMemory(This, pBuf, szLen))

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

#define IBitmapS_GetPixelBits(This) \
    ((This)->lpVtbl->GetPixelBits(This))

#define IBitmapS_Clone(This, ppClone) \
    ((This)->lpVtbl->Clone(This, ppClone))

#define IBitmapS_Scale(This, pOutput, nScale, filterLevel) \
    ((This)->lpVtbl->Scale(This, pOutput, nScale, filterLevel))

#define IBitmapS_Scale2(This, pOutput, nWid, nHei, filterLevel) \
    ((This)->lpVtbl->Scale2(This, pOutput, nWid, nHei, filterLevel))

#define IBitmapS_Save(This, pszFileName, pFormat) \
    ((This)->lpVtbl->Save(This, pszFileName, pFormat))

#define IBitmapS_Save2(This, pszFileName, imgFmt) \
    ((This)->lpVtbl->Save2(This, pszFileName, imgFmt))

#define IBitmapS_ToHBITMAP(This) \
    ((This)->lpVtbl->ToHBITMAP(This))

/* IFontS C API Macros */
#define IFontS_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IFontS_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IFontS_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IFontS_GetRenderFactory(This) \
    ((This)->lpVtbl->GetRenderFactory(This))

#define IFontS_ObjectType(This) \
    ((This)->lpVtbl->ObjectType(This))

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

#define IFontS_UpdateFont(This, pLogFont) \
    ((This)->lpVtbl->UpdateFont(This, pLogFont))

#define IFontS_SetProp(This, pXmlNode) \
    ((This)->lpVtbl->SetProp(This, pXmlNode))

/* IRegionS C API Macros */
#define IRegionS_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IRegionS_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IRegionS_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IRegionS_GetRenderFactory(This) \
    ((This)->lpVtbl->GetRenderFactory(This))

#define IRegionS_ObjectType(This) \
    ((This)->lpVtbl->ObjectType(This))

#define IRegionS_CombineRect(This, lprect, nCombineMode) \
    ((This)->lpVtbl->CombineRect(This, lprect, nCombineMode))

#define IRegionS_CombineRgn(This, pRgnSrc, nCombineMode) \
    ((This)->lpVtbl->CombineRgn(This, pRgnSrc, nCombineMode))

#define IRegionS_CombineRoundRect(This, lprect, ptConner, nCombineMode) \
    ((This)->lpVtbl->CombineRoundRect(This, lprect, ptConner, nCombineMode))

#define IRegionS_CombineEllipse(This, lprect, nCombineMode) \
    ((This)->lpVtbl->CombineEllipse(This, lprect, nCombineMode))

#define IRegionS_CombinePolygon(This, pts, count, nPolygonMode, nCombineMode) \
    ((This)->lpVtbl->CombinePolygon(This, pts, count, nPolygonMode, nCombineMode))

#define IRegionS_PtInRegion(This, pt) \
    ((This)->lpVtbl->PtInRegion(This, pt))

#define IRegionS_RectInRegion(This, lprect) \
    ((This)->lpVtbl->RectInRegion(This, lprect))

#define IRegionS_GetRgnBox(This, lprect) \
    ((This)->lpVtbl->GetRgnBox(This, lprect))

#define IRegionS_IsEmpty(This) \
    ((This)->lpVtbl->IsEmpty(This))

#define IRegionS_Offset(This, pt) \
    ((This)->lpVtbl->Offset(This, pt))

#define IRegionS_Clear(This) \
    ((This)->lpVtbl->Clear(This))

/* IPathS C API Macros */
#define IPathS_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IPathS_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IPathS_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IPathS_GetRenderFactory(This) \
    ((This)->lpVtbl->GetRenderFactory(This))

#define IPathS_ObjectType(This) \
    ((This)->lpVtbl->ObjectType(This))

#define IPathS_getFillType(This) \
    ((This)->lpVtbl->getFillType(This))

#define IPathS_setFillType(This, ft) \
    ((This)->lpVtbl->setFillType(This, ft))

#define IPathS_reset(This) \
    ((This)->lpVtbl->reset(This))

#define IPathS_isEmpty(This) \
    ((This)->lpVtbl->isEmpty(This))

#define IPathS_getBounds(This, prc) \
    ((This)->lpVtbl->getBounds(This, prc))

#define IPathS_moveTo(This, x, y) \
    ((This)->lpVtbl->moveTo(This, x, y))

#define IPathS_rMoveTo(This, dx, dy) \
    ((This)->lpVtbl->rMoveTo(This, dx, dy))

#define IPathS_lineTo(This, x, y) \
    ((This)->lpVtbl->lineTo(This, x, y))

#define IPathS_rLineTo(This, dx, dy) \
    ((This)->lpVtbl->rLineTo(This, dx, dy))

#define IPathS_quadTo(This, x1, y1, x2, y2) \
    ((This)->lpVtbl->quadTo(This, x1, y1, x2, y2))

#define IPathS_rQuadTo(This, dx1, dy1, dx2, dy2) \
    ((This)->lpVtbl->rQuadTo(This, dx1, dy1, dx2, dy2))

#define IPathS_conicTo(This, x1, y1, x2, y2, w) \
    ((This)->lpVtbl->conicTo(This, x1, y1, x2, y2, w))

#define IPathS_rConicTo(This, dx1, dy1, dx2, dy2, w) \
    ((This)->lpVtbl->rConicTo(This, dx1, dy1, dx2, dy2, w))

#define IPathS_cubicTo(This, x1, y1, x2, y2, x3, y3) \
    ((This)->lpVtbl->cubicTo(This, x1, y1, x2, y2, x3, y3))

#define IPathS_rCubicTo(This, x1, y1, x2, y2, x3, y3) \
    ((This)->lpVtbl->rCubicTo(This, x1, y1, x2, y2, x3, y3))

#define IPathS_addRect(This, rect, dir) \
    ((This)->lpVtbl->addRect(This, rect, dir))

#define IPathS_addRect2(This, left, top, right, bottom, dir) \
    ((This)->lpVtbl->addRect2(This, left, top, right, bottom, dir))

#define IPathS_addOval(This, oval, dir) \
    ((This)->lpVtbl->addOval(This, oval, dir))

#define IPathS_addOval2(This, left, top, right, bottom, dir) \
    ((This)->lpVtbl->addOval2(This, left, top, right, bottom, dir))

#define IPathS_addCircle(This, x, y, radius, dir) \
    ((This)->lpVtbl->addCircle(This, x, y, radius, dir))

#define IPathS_addArc(This, oval, startAngle, sweepAngle) \
    ((This)->lpVtbl->addArc(This, oval, startAngle, sweepAngle))

#define IPathS_addArc2(This, left, top, right, bottom, startAngle, sweepAngle) \
    ((This)->lpVtbl->addArc2(This, left, top, right, bottom, startAngle, sweepAngle))

#define IPathS_addRoundRect(This, rect, rx, ry, dir) \
    ((This)->lpVtbl->addRoundRect(This, rect, rx, ry, dir))

#define IPathS_addRoundRect2(This, left, top, right, bottom, rx, ry, dir) \
    ((This)->lpVtbl->addRoundRect2(This, left, top, right, bottom, rx, ry, dir))

#define IPathS_addPoly(This, pts, count, close) \
    ((This)->lpVtbl->addPoly(This, pts, count, close))

#define IPathS_offset(This, dx, dy) \
    ((This)->lpVtbl->offset(This, dx, dy))

#define IPathS_transform(This, matrix) \
    ((This)->lpVtbl->transform(This, matrix))

#define IPathS_getLastPt(This, lastPt) \
    ((This)->lpVtbl->getLastPt(This, lastPt))

#define IPathS_addString(This, pszText, nLen, x, y, pFont) \
    ((This)->lpVtbl->addString(This, pszText, nLen, x, y, pFont))

#define IPathS_clone(This) \
    ((This)->lpVtbl->clone(This))

#define IPathS_beginFigure(This, x, y, bFill) \
    ((This)->lpVtbl->beginFigure(This, x, y, bFill))

#define IPathS_endFigure(This, bClose) \
    ((This)->lpVtbl->endFigure(This, bClose))

#define IPathS_getLength(This) \
    ((This)->lpVtbl->getLength(This))

#define IPathS_getPosTan(This, distance, pos, vec) \
    ((This)->lpVtbl->getPosTan(This, distance, pos, vec))

#define IPathS_close(This) \
    ((This)->lpVtbl->close(This))

#define IPathS_hitTest(This, x, y) \
    ((This)->lpVtbl->hitTest(This, x, y))

#define IPathS_hitTestStroke(This, x, y, strokeSize) \
    ((This)->lpVtbl->hitTestStroke(This, x, y, strokeSize))

#define IPathS_op(This, other, op, out) \
    ((This)->lpVtbl->op(This, other, op, out))

/* IRenderTarget C API Macros */
#define IRenderTarget_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IRenderTarget_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IRenderTarget_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IRenderTarget_BeginDraw(This) \
    ((This)->lpVtbl->BeginDraw(This))

#define IRenderTarget_EndDraw(This) \
    ((This)->lpVtbl->EndDraw(This))

#define IRenderTarget_IsOffscreen(This) \
    ((This)->lpVtbl->IsOffscreen(This))

#define IRenderTarget_CreatePen(This, iStyle, cr, cWidth, ppPen) \
    ((This)->lpVtbl->CreatePen(This, iStyle, cr, cWidth, ppPen))

#define IRenderTarget_CreateSolidColorBrush(This, cr, ppBrush) \
    ((This)->lpVtbl->CreateSolidColorBrush(This, cr, ppBrush))

#define IRenderTarget_CreateBitmapBrush(This, pBmp, xtm, ytm, ppBrush) \
    ((This)->lpVtbl->CreateBitmapBrush(This, pBmp, xtm, ytm, ppBrush))

#define IRenderTarget_CreateGradientBrush(This, pGradients, nCount, info, byAlpha, tileMode, ppBrush) \
    ((This)->lpVtbl->CreateGradientBrush(This, pGradients, nCount, info, byAlpha, tileMode, ppBrush))

#define IRenderTarget_CreateRegion(This, ppRegion) \
    ((This)->lpVtbl->CreateRegion(This, ppRegion))

#define IRenderTarget_Resize(This, sz) \
    ((This)->lpVtbl->Resize(This, sz))

#define IRenderTarget_OffsetViewportOrg(This, xOff, yOff, lpPoint) \
    ((This)->lpVtbl->OffsetViewportOrg(This, xOff, yOff, lpPoint))

#define IRenderTarget_GetViewportOrg(This, lpPoint) \
    ((This)->lpVtbl->GetViewportOrg(This, lpPoint))

#define IRenderTarget_SetViewportOrg(This, pt) \
    ((This)->lpVtbl->SetViewportOrg(This, pt))

#define IRenderTarget_PushClipRect(This, pRect, mode) \
    ((This)->lpVtbl->PushClipRect(This, pRect, mode))

#define IRenderTarget_PushClipRegion(This, pRegion, mode) \
    ((This)->lpVtbl->PushClipRegion(This, pRegion, mode))

#define IRenderTarget_PopClip(This) \
    ((This)->lpVtbl->PopClip(This))

#define IRenderTarget_ExcludeClipRect(This, pRc) \
    ((This)->lpVtbl->ExcludeClipRect(This, pRc))

#define IRenderTarget_SaveClip(This, pnState) \
    ((This)->lpVtbl->SaveClip(This, pnState))

#define IRenderTarget_RestoreClip(This, nState) \
    ((This)->lpVtbl->RestoreClip(This, nState))

#define IRenderTarget_GetClipRegion(This, ppRegion) \
    ((This)->lpVtbl->GetClipRegion(This, ppRegion))

#define IRenderTarget_GetClipBox(This, prc) \
    ((This)->lpVtbl->GetClipBox(This, prc))

#define IRenderTarget_DrawText(This, pszText, cchLen, pRc, uFormat) \
    ((This)->lpVtbl->DrawText(This, pszText, cchLen, pRc, uFormat))

#define IRenderTarget_MeasureText(This, pszText, cchLen, psz) \
    ((This)->lpVtbl->MeasureText(This, pszText, cchLen, psz))

#define IRenderTarget_GetFontMetrics(This, ptm) \
    ((This)->lpVtbl->GetFontMetrics(This, ptm))

#define IRenderTarget_TextOut(This, x, y, lpszString, nCount) \
    ((This)->lpVtbl->TextOut(This, x, y, lpszString, nCount))

#define IRenderTarget_DrawRectangle(This, pRect) \
    ((This)->lpVtbl->DrawRectangle(This, pRect))

#define IRenderTarget_FillRectangle(This, pRect) \
    ((This)->lpVtbl->FillRectangle(This, pRect))

#define IRenderTarget_FillSolidRect(This, pRect, cr) \
    ((This)->lpVtbl->FillSolidRect(This, pRect, cr))

#define IRenderTarget_DrawRoundRect(This, pRect, pt) \
    ((This)->lpVtbl->DrawRoundRect(This, pRect, pt))

#define IRenderTarget_FillRoundRect(This, pRect, pt) \
    ((This)->lpVtbl->FillRoundRect(This, pRect, pt))

#define IRenderTarget_FillSolidRoundRect(This, pRect, pt, cr) \
    ((This)->lpVtbl->FillSolidRoundRect(This, pRect, pt, cr))

#define IRenderTarget_ClearRect(This, pRect, cr) \
    ((This)->lpVtbl->ClearRect(This, pRect, cr))

#define IRenderTarget_InvertRect(This, pRect) \
    ((This)->lpVtbl->InvertRect(This, pRect))

#define IRenderTarget_DrawEllipse(This, pRect) \
    ((This)->lpVtbl->DrawEllipse(This, pRect))

#define IRenderTarget_FillEllipse(This, pRect) \
    ((This)->lpVtbl->FillEllipse(This, pRect))

#define IRenderTarget_FillSolidEllipse(This, pRect, cr) \
    ((This)->lpVtbl->FillSolidEllipse(This, pRect, cr))

#define IRenderTarget_DrawArc(This, pRect, startAngle, sweepAngle, useCenter) \
    ((This)->lpVtbl->DrawArc(This, pRect, startAngle, sweepAngle, useCenter))

#define IRenderTarget_DrawArc2(This, pRect, startAngle, sweepAngle, width) \
    ((This)->lpVtbl->DrawArc2(This, pRect, startAngle, sweepAngle, width))

#define IRenderTarget_FillArc(This, pRect, startAngle, sweepAngle) \
    ((This)->lpVtbl->FillArc(This, pRect, startAngle, sweepAngle))

#define IRenderTarget_DrawLines(This, pPt, nCount) \
    ((This)->lpVtbl->DrawLines(This, pPt, nCount))

#define IRenderTarget_DrawLine(This, pt1, pt2) \
    ((This)->lpVtbl->DrawLine(This, pt1, pt2))

#define IRenderTarget_MoveToEx(This, pt, lpPoint) \
    ((This)->lpVtbl->MoveToEx(This, pt, lpPoint))

#define IRenderTarget_LineTo(This, pt) \
    ((This)->lpVtbl->LineTo(This, pt))

#define IRenderTarget_GetCurrentPositionEx(This, lpPoint) \
    ((This)->lpVtbl->GetCurrentPositionEx(This, lpPoint))

#define IRenderTarget_DrawPolygon(This, pPt, nCount) \
    ((This)->lpVtbl->DrawPolygon(This, pPt, nCount))

#define IRenderTarget_FillPolygon(This, pPt, nCount) \
    ((This)->lpVtbl->FillPolygon(This, pPt, nCount))

#define IRenderTarget_DrawGradientRect(This, pRect, bVert, ptRoundCorner, pGradients, nCount, byAlpha) \
    ((This)->lpVtbl->DrawGradientRect(This, pRect, bVert, ptRoundCorner, pGradients, nCount, byAlpha))

#define IRenderTarget_DrawGradientRectEx(This, pRect, ptRoundCorner, pGradients, nCount, info, byAlpha) \
    ((This)->lpVtbl->DrawGradientRectEx(This, pRect, ptRoundCorner, pGradients, nCount, info, byAlpha))

#define IRenderTarget_DrawGradientPath(This, path, pGradients, nCount, info, byAlpha, pRcBox) \
    ((This)->lpVtbl->DrawGradientPath(This, path, pGradients, nCount, info, byAlpha, pRcBox))

#define IRenderTarget_DrawIconEx(This, xLeft, yTop, hIcon, cxWidth, cyWidth, diFlags) \
    ((This)->lpVtbl->DrawIconEx(This, xLeft, yTop, hIcon, cxWidth, cyWidth, diFlags))

#define IRenderTarget_DrawBitmap(This, pRcDest, pBitmap, xSrc, ySrc, byAlpha) \
    ((This)->lpVtbl->DrawBitmap(This, pRcDest, pBitmap, xSrc, ySrc, byAlpha))

#define IRenderTarget_DrawBitmapEx(This, pRcDest, pBitmap, pRcSrc, expendMode, byAlpha) \
    ((This)->lpVtbl->DrawBitmapEx(This, pRcDest, pBitmap, pRcSrc, expendMode, byAlpha))

#define IRenderTarget_DrawBitmap9Patch(This, pRcDest, pBitmap, pRcSrc, pRcSourMargin, expendMode, byAlpha) \
    ((This)->lpVtbl->DrawBitmap9Patch(This, pRcDest, pBitmap, pRcSrc, pRcSourMargin, expendMode, byAlpha))

#define IRenderTarget_BitBlt(This, pRcDest, pRTSour, xSrc, ySrc, dwRop) \
    ((This)->lpVtbl->BitBlt(This, pRcDest, pRTSour, xSrc, ySrc, dwRop))

#define IRenderTarget_AlphaBlend(This, pRcDest, pRTSrc, pRcSrc, byAlpha) \
    ((This)->lpVtbl->AlphaBlend(This, pRcDest, pRTSrc, pRcSrc, byAlpha))

#define IRenderTarget_GetCurrentObject(This, uType) \
    ((This)->lpVtbl->GetCurrentObject(This, uType))

#define IRenderTarget_SelectDefaultObject(This, objType, pOldObj) \
    ((This)->lpVtbl->SelectDefaultObject(This, objType, pOldObj))

#define IRenderTarget_SelectObject(This, pObj, pOldObj) \
    ((This)->lpVtbl->SelectObject(This, pObj, pOldObj))

#define IRenderTarget_GetTextColor(This) \
    ((This)->lpVtbl->GetTextColor(This))

#define IRenderTarget_SetTextColor(This, color) \
    ((This)->lpVtbl->SetTextColor(This, color))

#define IRenderTarget_SetMaskFilter(This, pMaskFilter) \
    ((This)->lpVtbl->SetMaskFilter(This, pMaskFilter))

#define IRenderTarget_GetMaskFilter(This) \
    ((This)->lpVtbl->GetMaskFilter(This))

#define IRenderTarget_GetDC(This, uFlag) \
    ((This)->lpVtbl->GetDC(This, uFlag))

#define IRenderTarget_ReleaseDC(This, hdc, pRc) \
    ((This)->lpVtbl->ReleaseDC(This, hdc, pRc))

#define IRenderTarget_SetTransform(This, matrix, oldMatrix) \
    ((This)->lpVtbl->SetTransform(This, matrix, oldMatrix))

#define IRenderTarget_GetTransform(This, matrix) \
    ((This)->lpVtbl->GetTransform(This, matrix))

#define IRenderTarget_GetPixel(This, x, y) \
    ((This)->lpVtbl->GetPixel(This, x, y))

#define IRenderTarget_SetPixel(This, x, y, cr) \
    ((This)->lpVtbl->SetPixel(This, x, y, cr))

#define IRenderTarget_PushClipPath(This, path, mode, doAntiAlias) \
    ((This)->lpVtbl->PushClipPath(This, path, mode, doAntiAlias))

#define IRenderTarget_DrawPath(This, path, pathEffect) \
    ((This)->lpVtbl->DrawPath(This, path, pathEffect))

#define IRenderTarget_FillPath(This, path) \
    ((This)->lpVtbl->FillPath(This, path))

#define IRenderTarget_SetXfermode(This, mode, pOldMode) \
    ((This)->lpVtbl->SetXfermode(This, mode, pOldMode))

#define IRenderTarget_SetAntiAlias(This, bAntiAlias) \
    ((This)->lpVtbl->SetAntiAlias(This, bAntiAlias))

#define IRenderTarget_GetAntiAlias(This) \
    ((This)->lpVtbl->GetAntiAlias(This))

#define IRenderTarget_SetImageFilter(This, pImageEffect) \
    ((This)->lpVtbl->SetImageFilter(This, pImageEffect))

#define IRenderTarget_GetImageFilter(This) \
    ((This)->lpVtbl->GetImageFilter(This))

#define IRenderTarget_DrawSVG(This, pSvg, pRect, prcSrc, byAlpha) \
    ((This)->lpVtbl->DrawSVG(This, pSvg, pRect, prcSrc, byAlpha))

/* IRenderFactory C API Macros */
#define IRenderFactory_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IRenderFactory_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IRenderFactory_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IRenderFactory_GetImgDecoderFactory(This) \
    ((This)->lpVtbl->GetImgDecoderFactory(This))

#define IRenderFactory_SetImgDecoderFactory(This, pImgDecoderFac) \
    ((This)->lpVtbl->SetImgDecoderFactory(This, pImgDecoderFac))

#define IRenderFactory_CreateRenderTarget(This, ppRenderTarget, nWid, nHei) \
    ((This)->lpVtbl->CreateRenderTarget(This, ppRenderTarget, nWid, nHei))

#define IRenderFactory_CreateRenderTarget2(This, ppRenderTarget, hWnd) \
    ((This)->lpVtbl->CreateRenderTarget2(This, ppRenderTarget, hWnd))

#define IRenderFactory_CreateFont(This, ppFont, lf) \
    ((This)->lpVtbl->CreateFont(This, ppFont, lf))

#define IRenderFactory_CreateBitmap(This, ppBitmap) \
    ((This)->lpVtbl->CreateBitmap(This, ppBitmap))

#define IRenderFactory_CreateRegion(This, ppRgn) \
    ((This)->lpVtbl->CreateRegion(This, ppRgn))

#define IRenderFactory_CreatePath(This, ppPath) \
    ((This)->lpVtbl->CreatePath(This, ppPath))

#define IRenderFactory_CreatePathEffect(This, guidEffect, ppPathEffect) \
    ((This)->lpVtbl->CreatePathEffect(This, guidEffect, ppPathEffect))

#define IRenderFactory_CreateMaskFilter(This, guidEffect, ppMaskFilter) \
    ((This)->lpVtbl->CreateMaskFilter(This, guidEffect, ppMaskFilter))

#define IRenderFactory_CreateImageFilter(This, guidEffect, ppImageFilter) \
    ((This)->lpVtbl->CreateImageFilter(This, guidEffect, ppImageFilter))

#define IRenderFactory_GetDefFont(This) \
    ((This)->lpVtbl->GetDefFont(This))

#ifdef __cplusplus
}
#endif

#endif /* __SRENDER_CAPI_H__ */
