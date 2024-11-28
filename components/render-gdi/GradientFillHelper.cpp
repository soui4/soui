#include "GradientFillHelper.h"

#ifdef _WIN32
#include <Wingdi.h>
#pragma comment(lib,"msimg32.lib")
#endif

namespace SOUI
{

#ifndef GetAValue
#define GetAValue(rgb)   (LOBYTE((rgb) >> 24))
#endif//GetAValue

void GradientFillRect(HDC hdc, const RECT *rcFill,const GradientItem * items, int nCount,BOOL bVert,BYTE byAlpha)
{
	TRIVERTEX        vert[10] ;
	GRADIENT_RECT  grect[10];
	TRIVERTEX * pVert = nCount<=10?vert:(TRIVERTEX * )malloc(sizeof(TRIVERTEX)*nCount);
	GRADIENT_RECT *gRect=nCount<=10?grect:(GRADIENT_RECT*)malloc(sizeof(GRADIENT_RECT)*nCount);

	int x = rcFill->left;
	int y = rcFill->top;
	int nWid = rcFill->right-rcFill->left;
	int nHei = rcFill->bottom-rcFill->top;
	
	for(int i=0;i<nCount;i++){
		pVert [i] .Red    = GetRValue(items[i].cr)<<8;
		pVert [i] .Green  = GetGValue(items[i].cr)<<8;
		pVert [i] .Blue   = GetBValue(items[i].cr)<<8;
		pVert [i] .Alpha  = ((GetAValue(items[i].cr)*(255-byAlpha))/255)<<8; 

		if(bVert){
			pVert [i] .x      = i%2==0?rcFill->left:rcFill->right;
			pVert [i] .y      = y + (long)(nHei * items[i].pos);
		}else{
			pVert [i] .x      = x + (long)(nWid * items[i].pos);
			pVert [i] .y      = i%2==0?rcFill->top:rcFill->bottom;
		}
		gRect[i].UpperLeft=i;
		gRect[i].LowerRight=i+1;
	}
	GradientFill(hdc,pVert,nCount,gRect,nCount-1,bVert?GRADIENT_FILL_RECT_V:GRADIENT_FILL_RECT_H);
	if(pVert != vert){
		free(pVert);
		free(gRect);
	}
	
}

void GradientFillRectH(HDC hdc, const RECT *rcFill, COLORREF crLeft, COLORREF crRight)
{
	GradientItem items[]={crLeft,0.0f,crRight,1.0f};;
    GradientFillRect(hdc, rcFill, items,2,FALSE,0xff);
}

void GradientFillRectV( HDC hdc,const RECT *rcFill, COLORREF crTop, COLORREF crBottom)
{
	GradientItem items[]={crTop,0.0f,crBottom,1.0f};;
	GradientFillRect(hdc, rcFill, items,2,TRUE,0xff);
}

}//namespace DuiEngine