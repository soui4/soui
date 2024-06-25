#include "region.h"
#include <cairo/cairo.h>

int GetRegionComplexity(HRGN hRgn){
    if(!hRgn)
        return ERROR;
    if(cairo_region_is_empty((cairo_region_t*)hRgn))
        return NULLREGION;
    if(cairo_region_num_rectangles((cairo_region_t*)hRgn)==1)
        return SIMPLEREGION;
    return COMPLEXREGION;
}

int CombineRgn(HRGN hrgnDst, HRGN hrgnSrc1, HRGN hrgnSrc2, int iMode)
{
    switch(iMode){
        case RGN_AND:
        {
            cairo_region_t *empty = cairo_region_create();
            //clear dest region
            cairo_region_intersect((cairo_region_t*)hrgnDst,empty);
            cairo_region_union((cairo_region_t*)hrgnDst,(cairo_region_t*)hrgnSrc1);
            cairo_region_intersect((cairo_region_t*)hrgnDst,(cairo_region_t*)hrgnSrc2);
            cairo_region_destroy(empty);
        }
        break;
        case RGN_COPY:
        {
            cairo_region_t *empty = cairo_region_create();
            //clear dest region
            cairo_region_intersect((cairo_region_t*)hrgnDst,empty);
            cairo_region_union((cairo_region_t*)hrgnDst,(cairo_region_t*)hrgnSrc1);
            cairo_region_destroy(empty);
        }
        break;
        case RGN_DIFF:
        {
            cairo_region_t *empty = cairo_region_create();
            //clear dest region
            cairo_region_intersect((cairo_region_t*)hrgnDst,empty);
            cairo_region_union((cairo_region_t*)hrgnDst,(cairo_region_t*)hrgnSrc1);
            cairo_region_subtract((cairo_region_t*)hrgnDst,(cairo_region_t*)hrgnSrc2);
            cairo_region_destroy(empty);
        }
        break;
        case RGN_OR:
        {
            cairo_region_t *empty = cairo_region_create();
            //clear dest region
            cairo_region_intersect((cairo_region_t*)hrgnDst,empty);
            cairo_region_union((cairo_region_t*)hrgnDst,(cairo_region_t*)hrgnSrc1);
            cairo_region_union((cairo_region_t*)hrgnDst,(cairo_region_t*)hrgnSrc2);
            cairo_region_destroy(empty);
        }
        break;
        case RGN_XOR:
        {
            cairo_region_t *empty = cairo_region_create();
            //clear dest region
            cairo_region_intersect((cairo_region_t*)hrgnDst,empty);
            cairo_region_union((cairo_region_t*)hrgnDst,(cairo_region_t*)hrgnSrc1);
            cairo_region_xor((cairo_region_t*)hrgnDst,(cairo_region_t*)hrgnSrc2);
            cairo_region_destroy(empty);
        }
        break;
    }
    return GetRegionComplexity(hrgnDst);
}

HRGN CreateEllipticRgn(int nLeftRect, int nTopRect, int nRightRect, int nBottomRect)
{
    return 0;
}

HRGN CreateEllipticRgnIndirect(const RECT *lprc)
{
    return CreateEllipticRgn(lprc->left,lprc->top,lprc->right,lprc->bottom);
}

HRGN CreatePolygonRgn(const POINT *lppt, int cPoints, int fnPolyFillMode)
{
    return 0;
}

HRGN CreateRectRgn(int nLeftRect, int nTopRect, int nRightRect, int nBottomRect)
{
    cairo_rectangle_int_t rc={nLeftRect,nTopRect,nRightRect-nLeftRect,nBottomRect-nTopRect};
    cairo_region_t *ret = cairo_region_create_rectangle(&rc);
    return (HRGN)ret;
}

HRGN CreateRectRgnIndirect(const RECT *lprc)
{
    return CreateRectRgn(lprc->left,lprc->top,lprc->right,lprc->bottom);
}

HRGN CreateRoundRectRgn(int nLeftRect, int nTopRect, int nRightRect, int nBottomRect, int nWidthEllipse, int nHeightEllipse)
{
    return 0;
}

BOOL EqualRgn(HRGN hSrcRgn1, HRGN hSrcRgn2)
{
    return cairo_region_equal((cairo_region_t*)hSrcRgn1,(cairo_region_t*)hSrcRgn2);
}

HRGN ExtCreateRegion(const XFORM *lpXform, DWORD nCount, const RGNDATA *lpRgnData)
{
    int rcCnt = (nCount - sizeof(RGNDATAHEADER))/sizeof(RECT);
    if(rcCnt == 0)
        return 0;
    cairo_region_t * rgn = cairo_region_create();
    const RECT * prc=(const RECT*)lpRgnData->Buffer;
    for(int i=0;i<rcCnt;i++){
        cairo_rectangle_int_t rc={prc->left,prc->top,prc->right-prc->left,prc->bottom-prc->top};
        cairo_region_union_rectangle(rgn,&rc);
    }
    if(lpXform){
        //todo: only translate is availble.
        cairo_region_translate(rgn,lpXform->eDx,lpXform->eDy);
    }
    return (HRGN)rgn;
}


static void ClipRegion(cairo_t * ctx, cairo_region_t* rgn){
    int nrc = cairo_region_num_rectangles(rgn);
    for(int i=0;i<nrc;i++){
        cairo_rectangle_int_t rc ;
        cairo_region_get_rectangle(rgn,i,&rc);
        cairo_rectangle(ctx,rc.x,rc.y,rc.width,rc.height);
    }
    cairo_clip(ctx);
}

BOOL FillRgn(HDC hdc, HRGN hrgn, HBRUSH hbr)
{
    if(!hrgn)
        return FALSE;
    cairo_t * ctx = (cairo_t*)hdc;
    cairo_save(ctx);

    ClipRegion(ctx,(cairo_region_t *)hrgn);
    //todo:use hbrush color
    cairo_fill(ctx);

    cairo_restore(ctx);
    return TRUE;
}


BOOL FrameRgn(HDC hdc, HRGN hrgn, HBRUSH hbr, int nWidth, int nHeight)
{
    if(!hrgn)
        return FALSE;
    cairo_t * ctx = (cairo_t*)hdc;
    cairo_save(ctx);
    ClipRegion(ctx,(cairo_region_t *)hrgn);
    //todo:use hbrush color
    cairo_stroke(ctx);

    cairo_restore(ctx);
    return TRUE;
}

DWORD GetRegionData(HRGN hrgn, DWORD dwCount, PRGNDATA lpRgnData)
{
    int nrc = cairo_region_num_rectangles((cairo_region_t*)hrgn);
    int nRet = sizeof(RECT)*nrc + sizeof(RGNDATAHEADER);
    if(!lpRgnData)
        return nRet;
    if(dwCount<nRet)
        return 0;
    LPRECT prc = (LPRECT)lpRgnData->Buffer;
    for(int i=0;i<nrc;i++){
        cairo_rectangle_int_t rc ;
        cairo_region_get_rectangle((cairo_region_t*)hrgn,i,&rc);
        prc->left = rc.x;
        prc->top = rc.y;
        prc->right = rc.x + rc.width;
        prc->bottom = rc.y + rc.height;
        prc++;
    }
    return nRet;
}

int GetRgnBox(HRGN hrgn, LPRECT lprc)
{
    cairo_rectangle_int_t rc ;
    cairo_region_get_extents((cairo_region_t*)hrgn,&rc);
    lprc->left = rc.x;
    lprc->top = rc.y;
    lprc->right = rc.x + rc.width;
    lprc->bottom = rc.y + rc.height;
    return GetRegionComplexity(hrgn);
}

BOOL InvertRgn(HDC hdc, HRGN hrgn)
{
    if(!hrgn)
        return FALSE;
    cairo_t * ctx = (cairo_t*)hdc;
    cairo_save(ctx);

    ClipRegion(ctx,(cairo_region_t *)hrgn);
    //todo:
    //cairo_invert(ctx);

    cairo_restore(ctx);
    return TRUE;
}

int OffsetRgn(HRGN hrgn, int nXOffset, int nYOffset)
{
    cairo_region_translate((cairo_region_t*)hrgn,nXOffset,nYOffset);
    return 1;
}

BOOL PaintRgn(HDC hdc, HRGN hrgn)
{
    if(!hrgn)
        return FALSE;
    cairo_t * ctx = (cairo_t*)hdc;
    cairo_save(ctx);

    ClipRegion(ctx,(cairo_region_t *)hrgn);
    cairo_fill(ctx);

    cairo_restore(ctx);
    return TRUE;
}

BOOL PtInRegion(HRGN hrgn, int X, int Y)
{
    return cairo_region_contains_point((cairo_region_t*)hrgn,X,Y);
}

BOOL RectInRegion(HRGN hrgn, const RECT *lprc)
{
    cairo_rectangle_int_t rc={lprc->left,lprc->top,lprc->right-lprc->left,lprc->bottom-lprc->top};
    return cairo_region_contains_rectangle((cairo_region_t*)hrgn,&rc);
}

int SetPolyFillMode(HDC hdc, int iPolyFillMode)
{
    return 0;
}

BOOL SetRectRgn(HRGN hrgn, int left, int top, int right, int bottom)
{
    cairo_region_t *tmp = cairo_region_create();
    cairo_region_intersect((cairo_region_t*)hrgn,tmp);
    cairo_region_destroy(tmp);
    cairo_rectangle_int_t rc={left,top,right-left,bottom-top};
    tmp = cairo_region_create_rectangle(&rc);
    cairo_region_union((cairo_region_t*)hrgn,tmp);
    cairo_region_destroy(tmp);
    return 0;
}
