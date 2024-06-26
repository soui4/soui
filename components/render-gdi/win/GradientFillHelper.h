#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <interface/SRender-i.h>

namespace SOUI
{

void GradientFillRectV(HDC hdc,const RECT *rcFill, COLORREF crTop, COLORREF crBottom);

void GradientFillRectH(HDC hdc,const RECT *rcFill, COLORREF crLeft, COLORREF crRight);

void GradientFillRect(HDC hdc, const RECT *rcFill,const GradientItem * items, int nCount,BOOL bVert,BYTE byAlpha);
}//namespace SOUI