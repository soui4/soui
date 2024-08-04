#include <platform.h>
#include <gdi.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xcb.h>
#include <xcb/xcb_aux.h>
#include <math.h>
#include <png.h>

#include "sdc.h"
#include "SConnection.h"
using namespace SOUI;

typedef struct _GdiObj{
    int type;
    void *ptr;
    int nRef;
    fun_gdi_free cbFree;
    _GdiObj(int _type,void * _ptr, fun_gdi_free _cbFree):type(_type),ptr(_ptr),cbFree(_cbFree), nRef(1){
    }

    ~_GdiObj() {
    }
}* HGDIOBJ;

struct CairoColor{
    double r,g,b,a;
};

void RGBA2CairoColor(COLORREF crSrc,CairoColor *dst){
    dst->r = GetRValue(crSrc)/255.0;
    dst->g = GetGValue(crSrc)/255.0;
    dst->b = GetBValue(crSrc)/255.0;
    dst->a = GetAValue(crSrc)/255.0;
}

static void gdi_bmp_free(void* ptr) {
    cairo_surface_destroy((cairo_surface_t*)ptr);
}

static void gdi_pen_free(void* ptr) {
    delete (LOGPEN*)ptr;
}

struct PatternInfo {
    cairo_pattern_t* pattern;
    HBITMAP  bmp;
};

static void gdi_brush_free(void* ptr) {
    LOGBRUSH* plogbr = (LOGBRUSH*)ptr;
    if (plogbr->lbStyle == BS_PATTERN) {
        PatternInfo* info = (PatternInfo*)plogbr->lbHatch;
        cairo_pattern_destroy(info->pattern);
        DeleteObject(info->bmp);
        delete info;
    }
    delete plogbr;
}

static void gdi_font_free(void* ptr) {
    delete (LOGFONT*)ptr;
}

HGDIOBJ InitGdiObj(int type,void *ptr){
    fun_gdi_free cbFree=nullptr;
    switch (type) {
    case OBJ_BITMAP:cbFree = gdi_bmp_free; break;
    case OBJ_PEN: cbFree = gdi_pen_free; break;
    case OBJ_BRUSH:cbFree = gdi_brush_free; break;
    case OBJ_FONT:cbFree = gdi_font_free; break;
    }
    return new _GdiObj(type,ptr, cbFree);
}

HGDIOBJ InitGdiObj2(int type, void* ptr, fun_gdi_free cbFree) {
    return new _GdiObj(type, ptr, cbFree);
}

int GetObjectType(HGDIOBJ hgdiobj){
    return hgdiobj->type;
}

void* GetGdiObjPtr(HGDIOBJ hgdiobj){
    return hgdiobj->ptr;
}

void SetGdiObjPtr(HGDIOBJ hgdiObj, void* ptr) {
    hgdiObj->ptr = ptr;
}

int  RefGdiObj(HGDIOBJ hgdiObj) {
    return ++hgdiObj->nRef;
}

static bool IsNullPen(HPEN hpen) {
    LOGPEN* pen = (LOGPEN*)GetGdiObjPtr(hpen);
    return pen->lopnStyle == PS_NULL;
}

static bool ApplyPen(cairo_t * ctx,HPEN hpen){
    LOGPEN *pen = (LOGPEN*)GetGdiObjPtr(hpen);
    if(pen->lopnStyle == PS_NULL)
        return false;
    cairo_set_line_width(ctx,pen->lopnWidth);
    CairoColor cr;
    RGBA2CairoColor(pen->lopnColor,&cr);
    cairo_set_source_rgba(ctx,cr.r,cr.g,cr.b,cr.a);
    switch(pen->lopnStyle & PS_STYLE_MASK){
        case PS_DASH:
        {
            static double dashes_dash[] = {5.0, 5.0};
            cairo_set_dash(ctx,dashes_dash,ARRAYSIZE(dashes_dash),0.0);
        }
        break;
        case PS_DASHDOT:
        {
            static double dashes_dash[] = {5.0, 5.0, 1.0, 5.0};
            cairo_set_dash(ctx,dashes_dash,ARRAYSIZE(dashes_dash),0.0);
        }
        case PS_DASHDOTDOT:
        {
            static double dashes_dash[] = {5.0, 5.0, 1.0, 5.0, 1.0, 5.0};
            cairo_set_dash(ctx,dashes_dash,ARRAYSIZE(dashes_dash),0.0);
        }
        case PS_SOLID:
        break;
    }
    switch(pen->lopnStyle & PS_ENDCAP_MASK){
        case PS_ENDCAP_ROUND:
        cairo_set_line_cap(ctx,CAIRO_LINE_CAP_ROUND);
        break;
        case PS_ENDCAP_SQUARE:
        cairo_set_line_cap(ctx,CAIRO_LINE_CAP_SQUARE);
        break;
        case PS_ENDCAP_FLAT:
        default:
        cairo_set_line_cap(ctx,CAIRO_LINE_CAP_BUTT);
        break;
    }
    switch(pen->lopnStyle & PS_JOIN_MASK){
        case PS_JOIN_ROUND:
        cairo_set_line_join(ctx,CAIRO_LINE_JOIN_ROUND);
        break;
        case PS_JOIN_BEVEL:
        cairo_set_line_join(ctx,CAIRO_LINE_JOIN_BEVEL);
        break;
        case PS_JOIN_MITER:
        default:
        cairo_set_line_join(ctx,CAIRO_LINE_JOIN_MITER);
        break;
    }
    return true;
}

static bool IsNullBrush(HPEN hbr) {
    LOGBRUSH* br = (LOGBRUSH*)GetGdiObjPtr(hbr);
    return br->lbStyle == BS_NULL;
}

static bool ApplyBrush(cairo_t * ctx,HBRUSH hbr){
    LOGBRUSH *br = (LOGBRUSH*)GetGdiObjPtr(hbr);
    if(br->lbStyle == BS_NULL)
        return false;
    switch(br->lbStyle){
        case BS_SOLID:
        {
            CairoColor cr;
            RGBA2CairoColor(br->lbColor,&cr);
            cairo_set_source_rgba(ctx,cr.r,cr.g,cr.b,cr.a);
        }
        break;
        case BS_PATTERN:
        {
            PatternInfo* info = (PatternInfo*)br->lbHatch;
            cairo_set_source(ctx,info->pattern);
            break;
        }
    }
    return true;
}

static void ApplyFont(HDC hdc){
    if(hdc->hfont){
        LOGFONT *lf = (LOGFONT*)GetGdiObjPtr(hdc->hfont);
        cairo_select_font_face(hdc->cairo, lf->lfFaceName, 
            lf->lfItalic?CAIRO_FONT_SLANT_ITALIC:CAIRO_FONT_SLANT_NORMAL, 
            lf->lfWeight>400?CAIRO_FONT_WEIGHT_BOLD:CAIRO_FONT_WEIGHT_NORMAL);

        cairo_set_font_size(hdc->cairo, abs(lf->lfHeight));
    }
}


static void ApplyRegion(cairo_t * ctx, HRGN hRgn){
    cairo_reset_clip(ctx);
    DWORD dwCount = GetRegionData(hRgn, 0, nullptr);
    RGNDATA *pData = (RGNDATA*)malloc(dwCount);
    GetRegionData(hRgn, dwCount, pData);
    RECT* pRc = (RECT*)pData->Buffer;
    for (int i = 0; i < pData->rdh.nCount; i++) {
        cairo_rectangle(ctx, pRc->left,pRc->top,pRc->right-pRc->left,pRc->bottom-pRc->top);
        pRc++;
    }
    free(pData);
    cairo_clip(ctx);
}

HPEN ExtCreatePen(DWORD iPenStyle, DWORD cWidth, const LOGBRUSH *plbrush, DWORD cStyle, const DWORD *pstyle)
{
    return 0;
}

int GetObject(HGDIOBJ h, int c, LPVOID pv)
{
    if(!h->ptr)
        return 0;
    switch(h->type){
        case OBJ_BITMAP:
        if(c>=sizeof(BITMAP)){
            BITMAP * bm = (BITMAP*)pv;
            cairo_surface_t * pixmap = (cairo_surface_t*)h->ptr;
            bm->bmWidth = cairo_image_surface_get_width(pixmap);
            bm->bmHeight = cairo_image_surface_get_height(pixmap);
            bm->bmPlanes = 1;
            bm->bmBitsPixel=32;
            bm->bmWidthBytes = bm->bmWidth*4;
            bm->bmType = BI_RGB;
            bm->bmBits = cairo_image_surface_get_data(pixmap);
            return sizeof(BITMAP);
        }
        break;
    }
    return 0;
}

HPEN CreatePen(int iStyle, int cWidth, COLORREF color)
{
    LOGPEN logPen = {(UINT)iStyle,(LONG)cWidth,color};
    return CreatePenIndirect(&logPen);
}

HPEN CreatePenIndirect(const LOGPEN *plpen)
{
    LOGPEN *pData = new LOGPEN;
    memcpy(pData,plpen,sizeof(LOGPEN));
    return InitGdiObj(OBJ_PEN,pData);
}

HFONT CreateFontIndirect(const LOGFONT *lplf)
{
    LOGFONT *plog = new LOGFONT;
    memcpy(plog,lplf,sizeof(LOGFONT));
    return InitGdiObj(OBJ_FONT,plog);
}

HFONT CreateFont(int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight, 
DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut, DWORD iCharSet, 
DWORD iOutPrecision, DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily, LPCSTR pszFaceName)
{
    LOGFONT lf;
    lf.lfCharSet = iCharSet;
    lf.lfHeight = cHeight;
    lf.lfWidth = cWidth;
    lf.lfEscapement = cEscapement;
    lf.lfOrientation = cOrientation;
    lf.lfWeight = cWeight;
    lf.lfItalic = bItalic;
    lf.lfUnderline = bUnderline;
    lf.lfStrikeOut = bStrikeOut;
    lf.lfClipPrecision = iClipPrecision;
    lf.lfOutPrecision = iOutPrecision;
    lf.lfQuality = iQuality;
    lf.lfPitchAndFamily=iPitchAndFamily;
    strcpy_s(lf.lfFaceName,ARRAYSIZE(lf.lfFaceName),pszFaceName);
    return CreateFontIndirect(&lf);
}

HBITMAP CreateDIBitmap(HDC hdc, const BITMAPINFOHEADER *pbmih, DWORD flInit, const VOID *pjBits, const BITMAPINFO *pbmi, UINT iUsage)
{
    if (iUsage != DIB_RGB_COLORS)
        return nullptr;
    HBITMAP bmp = CreateDIBSection(hdc,pbmi,0,nullptr,0,0);
    if(bmp){
        UpdateDIBPixmap(bmp,pbmi->bmiHeader.biWidth,pbmi->bmiHeader.biHeight,pbmi->bmiHeader.biBitCount,pbmi->bmiHeader.biWidth*4,pjBits);
    }
    return bmp;
}

HBRUSH  CreateDIBPatternBrush(HGLOBAL h, UINT iUsage)
{
    //todo:hjx
    return nullptr;
}

HBRUSH CreateDIBPatternBrushPt(const VOID *lpPackedDIB, UINT iUsage)
{
    BITMAPINFO* pInfo = (BITMAPINFO*)lpPackedDIB;
    HBITMAP bmp = CreateDIBitmap(nullptr, &pInfo->bmiHeader, 1,pInfo+1, pInfo,iUsage);
    if (!bmp) {
        return nullptr;
    }
    HBRUSH ret =  CreatePatternBrush(bmp);
    DeleteObject(bmp);
    return ret;
}

HBRUSH CreatePatternBrush(HBITMAP bmp)
{
    LOGBRUSH* plog = new LOGBRUSH;
    plog->lbStyle = BS_PATTERN;
    PatternInfo* info = new PatternInfo;
    info->bmp = bmp;
    info->pattern = cairo_pattern_create_for_surface((cairo_surface_t*)GetGdiObjPtr(bmp));
    RefGdiObj(bmp);
    plog->lbHatch = (UINT_PTR)info;
    return InitGdiObj(OBJ_BRUSH, plog);

}

HBRUSH CreateSolidBrush(COLORREF color)
{
    LOGBRUSH *plog = new LOGBRUSH;
    plog->lbStyle = BS_SOLID;
    plog->lbColor = color;
    return InitGdiObj(OBJ_BRUSH,plog);
}

HBITMAP CreateDIBSection(HDC hdc, const BITMAPINFO *lpbmi, UINT usage, VOID **ppvBits, HANDLE hSection, DWORD offset)
{
    if(lpbmi->bmiHeader.biBitCount!=32)
        return 0;
    cairo_surface_t *ret = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,lpbmi->bmiHeader.biWidth,abs(lpbmi->bmiHeader.biHeight));
    if(!ret)
        return 0;
    if(ppvBits){
        *ppvBits = cairo_image_surface_get_data(ret);
    }
    return InitGdiObj(OBJ_BITMAP,ret);
}

BOOL   UpdateDIBPixmap(HBITMAP bmp,int wid,int hei,int bitsPixel,int stride,CONST VOID*pjBits){
    BITMAP bm={0};
    GetObject(bmp,sizeof(bm),&bm);
    if(!bm.bmBits)
        return FALSE;
    if(bm.bmWidth != wid || bm.bmHeight!=hei || bm.bmBitsPixel != bitsPixel)
        return FALSE;
    if(pjBits)
        memcpy(bm.bmBits,pjBits,hei*stride);
    else
        memset(bm.bmBits,0,hei*stride);
    MarkPixmapDirty(bmp);
    return TRUE;
}

void   MarkPixmapDirty(HBITMAP bmp){
    if(bmp && bmp->type == OBJ_BITMAP){
        cairo_surface_mark_dirty((cairo_surface_t*)bmp->ptr);
    }
}

HDC CreateCompatibleDC(HDC hdc)
{
    HWND hwnd = 0;
    if(hdc==0){
        SConnection *conn = SConnMgr::instance()->getConnection();
        hwnd = conn->screen->root;
    }else{
        hwnd = hdc->hwnd;
    }
    return new _SDC(hwnd);
}

BOOL DeleteDC(HDC hdc)
{
    delete hdc;
    return TRUE;
}

int SetBkMode(HDC hdc, int mode)
{
    int ret = hdc->bkMode;
    hdc->bkMode = mode;
    return ret;
}

int SetGraphicsMode(HDC hdc, int iMode)
{
    return 0;
}

HBITMAP CreateCompatibleBitmap(HDC hdc, int cx, int cy)
{
    BITMAPINFO bmi;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biWidth = cx;
    bmi.bmiHeader.biHeight = -cy;
    return CreateDIBSection(hdc,&bmi,0,nullptr,0,0);
}

HGDIOBJ SelectObject(HDC hdc, HGDIOBJ h)
{
    HGDIOBJ ret = 0;
    assert(h);
    switch(h->type){
        case OBJ_PEN:
        {
            ret = hdc->pen;
            hdc->pen = h;
            break;
        }
        case OBJ_BRUSH:
        {
            ret = hdc->brush;
            hdc->brush=h;
            break;
        }
        case OBJ_FONT:
        {
            ret = hdc->hfont;
            hdc->hfont = h;
            break;
        }
        case OBJ_BITMAP:
        {
            //recreate cairo_t object
            assert(h != hdc->bmp);
            ret = hdc->bmp;
            if (hdc->cairo)
            {
                cairo_destroy(hdc->cairo);
                hdc->cairo = nullptr;
            }
            hdc->bmp = h;
            if (GetGdiObjPtr(h))
            {
                hdc->cairo = cairo_create((cairo_surface_t*)GetGdiObjPtr(h));
                //todo:
                cairo_set_antialias(hdc->cairo, CAIRO_ANTIALIAS_GOOD);
            }
            break;
        }
    }
    return ret;
}

BOOL DeleteObject(HGDIOBJ hObj)
{
    if(!hObj)
        return FALSE;
    if (--hObj->nRef > 0)
        return TRUE;
    if (hObj->ptr && hObj->cbFree) {
        hObj->cbFree(hObj->ptr);
    }
    delete hObj;
    return TRUE;
}

int SaveDC(HDC hdc)
{
    return hdc->SaveState();
}

BOOL RestoreDC(HDC hdc, int nSavedDC)
{
    return hdc->RestoreState(nSavedDC);
}

int  GetClipRgn(HDC hdc, HRGN hrgn)
{
    cairo_rectangle_list_t* rcList = cairo_copy_clip_rectangle_list(hdc->cairo);

    int size = FIELD_OFFSET(RGNDATA, Buffer)+ rcList->num_rectangles * sizeof(RECT);
	RGNDATA* pRgnData = (RGNDATA*)malloc(size);
    pRgnData->rdh.dwSize = size;
    pRgnData->rdh.iType = RDH_RECTANGLES;
    pRgnData->rdh.nCount = rcList->num_rectangles;

	RECT* pRc = (RECT*)pRgnData->Buffer;
	cairo_rectangle_t* pRcSrc = rcList->rectangles;
	for (int i = 0; i < rcList->num_rectangles; i++) {
		pRc->left = pRcSrc->x;
		pRc->top = pRcSrc->y;
		pRc->right = pRc->left + pRcSrc->width;
		pRc->bottom = pRc->top + pRcSrc->height;
        pRc++;
        pRcSrc++;
	}
	HRGN rgnSrc = ExtCreateRegion(NULL, pRgnData->rdh.nCount, pRgnData);
	free(pRgnData);    
    cairo_rectangle_list_destroy(rcList);
    CombineRgn(hrgn, rgnSrc, nullptr, RGN_COPY);
    DeleteObject(rgnSrc);
    return RgnComplexity(hrgn) == NULLREGION ? 0 : 1;
}

int SelectClipRgn(HDC hdc, HRGN hrgn)
{
    ApplyRegion(hdc->cairo, hrgn);
    return RgnComplexity(hrgn);
}

int ExtSelectClipRgn(HDC hdc, HRGN hrgn, int mode)
{
    POINT pt;
    GetViewportOrgEx(hdc, &pt);
    RECT rcClip;
    GetClipBox(hdc, &rcClip);

    HRGN rgnNow = CreateRectRgn(0, 0, 0, 0);
    GetClipRgn(hdc, rgnNow);
    int ret = CombineRgn(rgnNow, rgnNow, hrgn, mode);

    RECT rc3;
    GetRgnBox(rgnNow, &rc3);

    ApplyRegion(hdc->cairo, rgnNow);
    DeleteObject(rgnNow);
    return ret;
}

int ExcludeClipRect(HDC hdc, int left, int top, int right, int bottom)
{
    HRGN hrgn = CreateRectRgn(left,top,right,bottom);
    int ret = ExtSelectClipRgn(hdc, hrgn, RGN_DIFF);
    DeleteObject(hrgn);
    return ret;
}

int IntersectClipRect(HDC hdc, int left, int top, int right, int bottom)
{
    HRGN hrgn = CreateRectRgn(left, top, right, bottom);
    int ret = ExtSelectClipRgn(hdc, hrgn, RGN_AND);
    DeleteObject(hrgn);
    return ret;
}

HGDIOBJ  GetCurrentObject(HDC hdc, UINT type)
{
    switch(type){
        case OBJ_PEN:
        return hdc->pen;
        case OBJ_BRUSH:
        return hdc->brush;
        case OBJ_BITMAP:
        return hdc->bmp;
        case OBJ_FONT:
        return hdc->hfont;
    }
    return HGDIOBJ(0);
}

int  GetDIBits(HDC hdc, HBITMAP hbm, UINT start, UINT cLines, LPVOID lpvBits, LPBITMAPINFO lpbmi, UINT usage)
{
    return 0;
}

// 检查矩阵是否是单位矩阵
static int matrix_is_identity(const cairo_matrix_t *matrix) {
    static cairo_matrix_t identity_matrix={
        1.0,0.0,
        0.0,1.0,
        0.0,0.0
    };
    return matrix->xx == identity_matrix.xx &&
            matrix->xy == identity_matrix.xy &&
            matrix->yy == identity_matrix.yy &&
            matrix->yx == identity_matrix.yx &&
            matrix->x0 == identity_matrix.x0 &&
            matrix->y0 == identity_matrix.y0;
}

static bool matrix_inverse(double A[3][3], double A_inv[3][3]) {
    double det = A[0][0] * (A[1][1] * A[2][2] - A[1][2] * A[2][1]) - A[0][1] * (A[1][0] * A[2][2] - A[1][2] * A[2][0]) + A[0][2] * (A[1][0] * A[2][1] - A[1][1] * A[2][0]);

    if (det == 0) {
        //printf("矩阵不可逆，因为行列式为零。\n");
        return false;
    }

    A_inv[0][0] = (A[1][1] * A[2][2] - A[1][2] * A[2][1]) / det;
    A_inv[0][1] = (A[0][2] * A[2][1] - A[0][1] * A[2][2]) / det;
    A_inv[0][2] = (A[0][1] * A[1][2] - A[0][2] * A[1][1]) / det;
    A_inv[1][0] = (A[1][2] * A[2][0] - A[1][0] * A[2][2]) / det;
    A_inv[1][1] = (A[0][0] * A[2][2] - A[0][2] * A[2][0]) / det;
    A_inv[1][2] = (A[0][2] * A[1][0] - A[0][0] * A[1][2]) / det;
    A_inv[2][0] = (A[1][0] * A[2][1] - A[1][1] * A[2][0]) / det;
    A_inv[2][1] = (A[0][1] * A[2][0] - A[0][0] * A[2][1]) / det;
    A_inv[2][2] = (A[0][0] * A[1][1] - A[0][1] * A[1][0]) / det;
    return true;
}

static bool cairo_matrix_inverse(const cairo_matrix_t* src, cairo_matrix_t* inv) {
    double A[3][3];
    A[0][0] = src->xx;   A[0][1] = src->yx;   A[0][2] = 0;
    A[1][0] = src->xy;   A[1][1] = src->yy;   A[1][2] = 0;
    A[2][0] = src->x0;   A[2][1] = src->y0;   A[2][2] = 1;
    double A_inv[3][3];
    if (!matrix_inverse(A, A_inv))
        return false;
    inv->xx = A_inv[0][0]; inv->yx = A_inv[0][1];
    inv->xy = A_inv[1][0]; inv->yy = A_inv[1][1];
    inv->x0 = A_inv[2][0]; inv->y0 = A_inv[2][1];
    return true;
}

BOOL InvertRgn(HDC hdc, HRGN hrgn)
{
    if(!hrgn)
        return FALSE;
    cairo_t * ctx = hdc->cairo;
    cairo_save(ctx);
    ApplyRegion(ctx,hrgn);
    RECT rc;
    GetRgnBox(hrgn,&rc);
    cairo_set_source_rgb(ctx,1.0,1.0,1.0);
    cairo_set_operator(ctx,CAIRO_OPERATOR_XOR);
    cairo_rectangle(ctx,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top);
    cairo_fill(ctx);

    cairo_restore(ctx);
    return TRUE;
}

int GetClipBox(HDC hdc, LPRECT lprect)
{
    double x1, y1, x2, y2;
    cairo_clip_extents(hdc->cairo, &x1, &y1, &x2, &y2);
    lprect->left = x1;
    lprect->top = y1;
    lprect->right = x2;
    lprect->bottom = y2;
    return 0;
}


BOOL FillRgn(HDC hdc, HRGN hrgn, HBRUSH hbr)
{
    if(!hrgn || GetObjectType(hrgn)!=OBJ_REGION)
        return FALSE;
    cairo_t * ctx = hdc->cairo;
    cairo_save(ctx);
    ApplyRegion(ctx,hrgn);
    RECT rc;
    GetRgnBox(hrgn,&rc);
    cairo_rectangle(ctx,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top);
    ApplyBrush(ctx,hbr);
    cairo_fill(ctx);
    cairo_restore(ctx);
    return TRUE;
}


BOOL FrameRgn(HDC hdc, HRGN hrgn, HBRUSH hbr, int nWidth, int nHeight)
{
    if(!hrgn || GetObjectType(hrgn)!=OBJ_REGION)
        return FALSE;
    cairo_t * ctx = hdc->cairo;
    cairo_save(ctx);
    ApplyRegion(ctx,hrgn);
    ApplyPen(ctx,hdc->pen);
    cairo_stroke(ctx);
    cairo_restore(ctx);
    return TRUE;
}


BOOL PaintRgn(HDC hdc, HRGN hrgn)
{
    if(!hrgn)
        return FALSE;
    cairo_t * ctx = hdc->cairo;
    cairo_save(ctx);
    ApplyBrush(ctx,hdc->brush);
    ApplyRegion(ctx,hrgn);
    cairo_paint(ctx);
    cairo_restore(ctx);
    return TRUE;
}

static void ApplySrcMatrix(cairo_t* dst, cairo_t* src_ctx) {
    //todo: if src dc has a transform matrix, apply it's inverse matrix to dst cairo context. result has not been verified right now.
    cairo_matrix_t mtx;
    cairo_get_matrix(src_ctx, &mtx);
    if (!matrix_is_identity(&mtx)) {
        cairo_matrix_t inv;
        if (cairo_matrix_inverse(&mtx, &inv)) {
            cairo_transform(dst, &inv);
        }
    }
}

BOOL  AlphaBlend(HDC hdc, int x, int y, int wDst, int hDst, HDC hdcSrc, int x1, int y1, int wSrc, int hSrc, BLENDFUNCTION ftn)
{
    assert(hdc && hdcSrc);
    cairo_surface_t *src = (cairo_surface_t *)GetGdiObjPtr(hdcSrc->bmp);
    cairo_save(hdc->cairo);

    ApplySrcMatrix(hdc->cairo, hdcSrc->cairo);

    cairo_rectangle(hdc->cairo,x,y,wDst,hDst);
    cairo_clip(hdc->cairo);
    
    cairo_translate(hdc->cairo,x,y);
    double scale_x = wDst*1.0/wSrc;
    double scale_y = hDst*1.0/hSrc;
    cairo_scale(hdc->cairo,scale_x,scale_y);

    cairo_set_source_surface(hdc->cairo,src,-x1,-y1);
    cairo_paint_with_alpha(hdc->cairo, ftn.SourceConstantAlpha*1.0/255.0);

    cairo_restore(hdc->cairo);
    return 0;
}

BOOL BitBlt(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, DWORD rop)
{
    assert(hdc && hdcSrc);
    cairo_surface_t *src = (cairo_surface_t *)GetGdiObjPtr(hdcSrc->bmp);

    cairo_save(hdc->cairo);
    ApplySrcMatrix(hdc->cairo, hdcSrc->cairo);

    cairo_rectangle(hdc->cairo,x,y,cx,cy);
    cairo_clip(hdc->cairo);
    switch(rop){
        case SRCCOPY:
        cairo_set_source_surface(hdc->cairo,src,x1,y1);
        cairo_set_operator(hdc->cairo,CAIRO_OPERATOR_OVER);
        break;
        case SRCINVERT:
        cairo_set_source_surface(hdc->cairo,src,x1,y1);
        cairo_set_operator(hdc->cairo,CAIRO_OPERATOR_XOR);
        break;
        case SRCPAINT:
        cairo_set_source_surface(hdc->cairo,src,x1,y1);
        cairo_set_operator(hdc->cairo,CAIRO_OPERATOR_OVER);
        break;
        case SRCAND:
        cairo_set_source_surface(hdc->cairo,src,x1,y1);
        cairo_set_operator(hdc->cairo,CAIRO_OPERATOR_DEST_IN);
        break;
        case DSTINVERT:
        cairo_set_source_rgb(hdc->cairo,1.0,1.0,1.0);
        cairo_set_operator(hdc->cairo,CAIRO_OPERATOR_XOR);
        break;
    }
    cairo_rectangle(hdc->cairo,0,0,cx,cy);
    cairo_fill(hdc->cairo);
    cairo_restore(hdc->cairo);
    return TRUE;
}

//todo: to test
BOOL  StretchBlt(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, int cx2, int cy2, DWORD rop)
{
    assert(hdc && hdcSrc);
    cairo_surface_t *src = (cairo_surface_t *)GetGdiObjPtr(hdcSrc->bmp);

    cairo_save(hdc->cairo);
    ApplySrcMatrix(hdc->cairo, hdcSrc->cairo);

    cairo_rectangle(hdc->cairo,x,y,cx,cy);
    cairo_clip(hdc->cairo);

    cairo_translate(hdc->cairo,x,y);
    double scale_x = cx*1.0/cx2;
    double scale_y = cy*1.0/cy2;
    cairo_scale(hdc->cairo,scale_x,scale_y);

    switch(rop){
        case SRCCOPY:
        cairo_set_source_surface(hdc->cairo,src,x1,y1);
        cairo_set_operator(hdc->cairo,CAIRO_OPERATOR_OVER);
        break;
        case SRCINVERT:
        cairo_set_source_surface(hdc->cairo,src,x1,y1);
        cairo_set_operator(hdc->cairo,CAIRO_OPERATOR_XOR);
        break;
        case SRCPAINT:
        cairo_set_source_surface(hdc->cairo,src,x1,y1);
        cairo_set_operator(hdc->cairo,CAIRO_OPERATOR_OVER);
        break;
        case SRCAND:
        cairo_set_source_surface(hdc->cairo,src,x1,y1);
        cairo_set_operator(hdc->cairo,CAIRO_OPERATOR_DEST_IN);
        break;
        case DSTINVERT:
        cairo_set_source_rgb(hdc->cairo,1.0,1.0,1.0);
        cairo_set_operator(hdc->cairo,CAIRO_OPERATOR_XOR);
        break;
    }
    cairo_rectangle(hdc->cairo,0,0,cx,cy);
    cairo_fill(hdc->cairo);
    cairo_restore(hdc->cairo);
    return TRUE;
}

//todo: to test
INT StretchDIBits(HDC hdc, INT x_dst, INT y_dst, INT width_dst, INT height_dst, INT x_src, INT y_src, INT width_src, INT height_src, const void *bits, const BITMAPINFO *bmi, UINT coloruse, DWORD rop)
{
    HBITMAP bmp = CreateDIBitmap(hdc,&bmi->bmiHeader,1,bits,bmi,coloruse);
    if(!bmp)
    {
        return 0;
    }
    HDC memdc = CreateCompatibleDC(hdc);
    SelectObject(memdc,bmp);
    BOOL ret = StretchBlt(hdc,x_dst,y_dst,width_dst,height_dst,memdc,x_src,y_src,width_src,height_src,rop);
    DeleteDC(memdc);
    DeleteObject(bmp);
    return ret;
}

void SetStretchBltMode(HDC hdc, int mode)
{
    //todo:hjx
}

static bool IsAlpha(TCHAR c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool IsNumber(TCHAR c)
{
    return c >= '0' && c <= '9';
}

static bool IsHex(TCHAR c)
{
    return IsNumber(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static bool IsDigit(TCHAR c)
{
    return IsNumber(c) || c == '.' || c == ',';
}

static LPCTSTR SkipWord(LPCTSTR p)
{
    if (IsAlpha(*p))
    {
        while (*p)
        {
            p = CharNext(p);
            if (!IsAlpha(*p))
                break;
        }
    }
    return p;
}

static LPCTSTR SkipNumber(LPCTSTR p)
{
    if (*p && *(p + 1) && (_tcsncmp(p, _T("0x"), 2) == 0 || _tcsncmp(p, _T("0X"), 2) == 0))
    { // test for hex number
        p = p + 2;
        while (*p)
        {
            if (!IsHex(*p))
                break;
            p++;
        }
        return p;
    }
    else
    {
        while (*p)
        {
            if (!IsDigit(*p))
                break;
            p++;
        }
        return p;
    }
}

static LPCTSTR WordNext(LPCTSTR pszBuf, bool bWordbreak)
{
    SASSERT(pszBuf);
    LPCTSTR p = CharNext(pszBuf);
    if (!bWordbreak)
        return p;
    LPCTSTR pWord = SkipWord(pszBuf);
    if (pWord > pszBuf)
        return pWord;
    LPCTSTR pNum = SkipNumber(pszBuf);
    if (pNum > pszBuf)
        return pNum;
    return p;
}

static LPCSTR nextChar(LPCSTR p){
    int len = mbtowc(nullptr,p,MB_CUR_MAX);
    assert(len>0);
    return p + len;
}

static SIZE OnMeasureText(HDC hdc,LPCSTR pszBuf,int cchText){
    cairo_text_extents_t ext;
    int i=0;
    char word[6];
    LPCSTR p = pszBuf;
    LPCSTR pEnd = p+cchText;

    SIZE ret = {0,0};
    while(p<pEnd){
        LPCSTR next = nextChar(p);
        assert(next-p<=5);
        memcpy(word,p,(next-p));
        word[next-p]=0;
        cairo_text_extents(hdc->cairo,word,&ext);
        ret.cx += ext.width;
        ret.cy = std::max(ret.cy,(LONG)ext.height);
        p = next;
    }
    return ret;
}

static void DrawSingleLine(HDC hdc, LPCTSTR pszBuf, int iBegin, int cchText, LPRECT pRect, UINT uFormat)
{
    char *buf=(char*)malloc(cchText+1);
    memcpy(buf,pszBuf,cchText);
    buf[cchText]=0;

    cairo_text_extents_t ext;
    cairo_text_extents(hdc->cairo,buf,&ext);
    if(uFormat & DT_CALCRECT){
        pRect->right = pRect->left + ext.width;
        pRect->bottom = pRect->top + ext.height-ext.y_bearing;
    }else{
        switch(uFormat&(DT_LEFT|DT_CENTER|DT_RIGHT)){
            case DT_LEFT:
            cairo_move_to(hdc->cairo,pRect->left,pRect->top+ext.height);
            break;
            case DT_CENTER:
            cairo_move_to(hdc->cairo,pRect->left + (pRect->right-pRect->left - ext.width)/2,pRect->top + ext.height);
            break;
            case DT_RIGHT:
            cairo_move_to(hdc->cairo,pRect->right - ext.width,pRect->top + ext.height);
            break;
        }
        cairo_show_text(hdc->cairo,buf);
    }
    free(buf);
}


#define kDrawText_LineInterval 0

void DrawMultiLine(HDC hdc, LPCTSTR pszBuf, int cchText, LPRECT pRect, UINT uFormat)
{
    int i = 0, nLine = 1;
    if (cchText == -1)
        cchText = (int)_tcslen(pszBuf);
    LPCTSTR p1 = pszBuf;
    POINT pt = { pRect->left, pRect->top };
    SIZE szWord = OnMeasureText(hdc, _T("A"), 1);
    int nLineHei = szWord.cy;
    int nRight = pRect->right;
    int nLineWid = pRect->right - pRect->left;
    pRect->right = pRect->left;

    LPCTSTR pLineHead = p1, pLineTail = p1;

    LPCTSTR pPrev = NULL;
    while (i < cchText)
    {
        LPCTSTR p2 = WordNext(p1, uFormat&DT_WORDBREAK);
        SASSERT(p2 > p1);
        if ((*p1 == _T('\n') && p2))
        {
            if (pLineTail > pLineHead && !(uFormat & DT_CALCRECT))
            {
                RECT rcText={pRect->left, pt.y, nRight, pt.y + nLineHei};
                DrawSingleLine(hdc, pszBuf, (int)(pLineHead - pszBuf), (int)(pLineTail - pLineHead), &rcText, uFormat);
            }
            pt.y += nLineHei + kDrawText_LineInterval;
            pt.x = pRect->left;
            nLine++;
            i += (int)(p2 - p1);
            p1 = p2;
            pLineHead = p2;
            continue;
        }
        if (uFormat & DT_WORDBREAK && *p1 == 0x20 && pt.x == pRect->left && (!pPrev || *pPrev != 0x20))
        { // skip the first space for a new line.
            i += (int)(p2 - p1);
            pPrev = p1;
            p1 = p2;
            pLineTail = pLineHead = p2;
            continue;
        }
        szWord = OnMeasureText(hdc, p1, (int)(p2 - p1));
        if (pt.x + szWord.cx > nRight)
        { //检测到一行超过边界时还要保证当前行不为空

            if (pLineTail > pLineHead)
            {
                if (!(uFormat & DT_CALCRECT))
                {
                    RECT rcText = {pRect->left, pt.y, nRight, pt.y + nLineHei};
                    DrawSingleLine(hdc, pszBuf, (int)(pLineHead - pszBuf), (int)(pLineTail - pLineHead), &rcText, uFormat);
                }
                // 显示多行文本时，如果下一行文字的高度超过了文本框，则不再输出下一行文字内容。
                if (pt.y + nLineHei + kDrawText_LineInterval > pRect->bottom)
                { //将绘制限制在有效区。
                    pLineHead = pLineTail;
                    break;
                }

                pLineHead = p1;

                pt.y += nLineHei + kDrawText_LineInterval;
                pt.x = pRect->left;
                nLine++;

                continue;
            }
            else
            { // word is too long to draw in a single line
                LPCTSTR p3 = p1;
                SIZE szChar;
                szWord.cx = 0;
                while (p3 < p2)
                {
                    LPCTSTR p4 = CharNext(p3);
                    szChar = OnMeasureText(hdc, p3, (int)(p4 - p3));
                    if (szWord.cx + szChar.cx > nLineWid)
                    {
                        if (p3 == p1)
                        { // a line will contain at least one char.
                            p2 = p4;
                            szWord.cx = szChar.cx;
                        }
                        else
                        {
                            p2 = p3;
                        }
                        break;
                    }
                    szWord.cx += szChar.cx;
                    p3 = p4;
                }
            }
        }
        pt.x += szWord.cx;
        if (pt.x > pRect->right && uFormat & DT_CALCRECT)
            pRect->right = pt.x;
        i += (int)(p2 - p1);
        pPrev = p1;
        pLineTail = p1 = p2;
    }

    if (uFormat & DT_CALCRECT)
    {
        if (pRect->bottom > pt.y + nLineHei)
            pRect->bottom = pt.y + nLineHei;
    }
    else if (pLineTail > pLineHead)
    {
        RECT rcText={pRect->left, pt.y, nRight, pt.y + nLineHei};
        DrawSingleLine(hdc, pszBuf, (int)(pLineHead - pszBuf), (int)(pLineTail - pLineHead), &rcText, uFormat);
    }
}

int DrawText(HDC hdc, LPCSTR pszBuf, int cchText, LPRECT pRect, UINT uFormat)
{
    if (cchText < 0)
        cchText = strlen(pszBuf);

    cairo_save(hdc->cairo);
    ApplyFont(hdc);
    CairoColor cr;
    RGBA2CairoColor(hdc->crText, &cr);
    cairo_set_source_rgba(hdc->cairo, cr.r, cr.g, cr.b, cr.a);
    cairo_matrix_t mtx = { 0 };
    cairo_get_matrix(hdc->cairo,&mtx);
    if (uFormat & DT_SINGLELINE)
    {
        DrawSingleLine(hdc, pszBuf, 0, cchText, pRect, uFormat);
    }
    else
    {
        if (uFormat & (DT_VCENTER | DT_BOTTOM) && !(uFormat & DT_CALCRECT))
        {
            DrawMultiLine(hdc, pszBuf, cchText, pRect, uFormat | DT_CALCRECT);
            SIZE szTxt = {pRect->right-pRect->left,pRect->bottom-pRect->top};
            RECT rcText = *pRect;
            switch (uFormat & (DT_VCENTER | DT_BOTTOM))
            {
            case DT_VCENTER:
                InflateRect(&rcText,0, -(rcText.bottom-rcText.top - szTxt.cy) / 2);
                break;
            case DT_BOTTOM:
                InflateRect(&rcText,0, -(rcText.bottom-rcText.top - szTxt.cy));
                break;
            }
            DrawMultiLine(hdc, pszBuf, cchText, &rcText, uFormat);
        }
        else
        {
            DrawMultiLine(hdc, pszBuf, cchText, pRect, uFormat);
        }
    }
    cairo_restore(hdc->cairo);

    return TRUE;
}


BOOL  TextOut(HDC hdc, int x, int y, LPCSTR lpString, int c)
{
    cairo_save(hdc->cairo);
    ApplyFont(hdc);
    CairoColor cr;
    RGBA2CairoColor(hdc->crText,&cr);
    cairo_set_source_rgba(hdc->cairo, cr.r,cr.g,cr.b,cr.a); 

    cairo_text_extents_t ext2;
    cairo_text_extents(hdc->cairo,"A",&ext2);
    cairo_move_to(hdc->cairo,x,y-ext2.y_bearing);

    if(c>0){
        char *buf = (char*)malloc(c+1);
        memcpy(buf,lpString,c);
        buf[c]=0;
        cairo_show_text(hdc->cairo,buf);
        free(buf);
    }else{
        cairo_show_text(hdc->cairo,lpString);
    }
    cairo_restore(hdc->cairo);
    return TRUE;
}


BOOL GetTextExtentPoint32(HDC hdc, LPCSTR lpString, int c, LPSIZE psizl)
{
    cairo_save(hdc->cairo);
    ApplyFont(hdc);
    cairo_text_extents_t ext;
    if(c<0){
        cairo_text_extents(hdc->cairo,lpString,&ext);
    }else{
        char *buf = (char*)malloc(c+1);
        memcpy(buf,lpString,c);
        buf[c]=0;
        cairo_text_extents(hdc->cairo,buf,&ext);
        free(buf);
    }
    psizl->cx = ext.width;
    psizl->cy = ext.height - ext.y_bearing;
    cairo_restore(hdc->cairo);

    return TRUE;
}


/* the various registry keys that are used to store parameters */
enum parameter_key
{
    COLORS_KEY,
    DESKTOP_KEY,
    KEYBOARD_KEY,
    MOUSE_KEY,
    METRICS_KEY,
    SOUND_KEY,
    VERSION_KEY,
    SHOWSOUNDS_KEY,
    KEYBOARDPREF_KEY,
    SCREENREADER_KEY,
    AUDIODESC_KEY,
    NB_PARAM_KEYS
};
struct sysparam_entry
{
};

struct sysparam_rgb_entry
{
    enum parameter_key base_key;
    const char        *regval;
    COLORREF              val;
};

static struct sysparam_rgb_entry system_colors[] =
{
#define RGB_ENTRY(name,val,reg) { COLORS_KEY, reg , (val) }
    RGB_ENTRY( COLOR_SCROLLBAR, RGB(212, 208, 200), "Scrollbar" ),
    RGB_ENTRY( COLOR_BACKGROUND, RGB(58, 110, 165), "Background" ),
    RGB_ENTRY( COLOR_ACTIVECAPTION, RGB(10, 36, 106), "ActiveTitle" ),
    RGB_ENTRY( COLOR_INACTIVECAPTION, RGB(128, 128, 128), "InactiveTitle" ),
    RGB_ENTRY( COLOR_MENU, RGB(212, 208, 200), "Menu" ),
    RGB_ENTRY( COLOR_WINDOW, RGB(255, 255, 255), "Window" ),
    RGB_ENTRY( COLOR_WINDOWFRAME, RGB(0, 0, 0), "WindowFrame" ),
    RGB_ENTRY( COLOR_MENUTEXT, RGB(0, 0, 0), "MenuText" ),
    RGB_ENTRY( COLOR_WINDOWTEXT, RGB(0, 0, 0), "WindowText" ),
    RGB_ENTRY( COLOR_CAPTIONTEXT, RGB(255, 255, 255), "TitleText" ),
    RGB_ENTRY( COLOR_ACTIVEBORDER, RGB(212, 208, 200), "ActiveBorder" ),
    RGB_ENTRY( COLOR_INACTIVEBORDER, RGB(212, 208, 200), "InactiveBorder" ),
    RGB_ENTRY( COLOR_APPWORKSPACE, RGB(128, 128, 128), "AppWorkSpace" ),
    RGB_ENTRY( COLOR_HIGHLIGHT, RGB(10, 36, 106), "Hilight" ),
    RGB_ENTRY( COLOR_HIGHLIGHTTEXT, RGB(255, 255, 255), "HilightText" ),
    RGB_ENTRY( COLOR_BTNFACE, RGB(212, 208, 200), "ButtonFace" ),
    RGB_ENTRY( COLOR_BTNSHADOW, RGB(128, 128, 128), "ButtonShadow" ),
    RGB_ENTRY( COLOR_GRAYTEXT, RGB(128, 128, 128), "GrayText" ),
    RGB_ENTRY( COLOR_BTNTEXT, RGB(0, 0, 0), "ButtonText" ),
    RGB_ENTRY( COLOR_INACTIVECAPTIONTEXT, RGB(212, 208, 200), "InactiveTitleText" ),
    RGB_ENTRY( COLOR_BTNHIGHLIGHT, RGB(255, 255, 255), "ButtonHilight" ),
    RGB_ENTRY( COLOR_3DDKSHADOW, RGB(64, 64, 64), "ButtonDkShadow" ),
    RGB_ENTRY( COLOR_3DLIGHT, RGB(212, 208, 200), "ButtonLight" ),
    RGB_ENTRY( COLOR_INFOTEXT, RGB(0, 0, 0), "InfoText" ),
    RGB_ENTRY( COLOR_INFOBK, RGB(255, 255, 225), "InfoWindow" ),
    RGB_ENTRY( COLOR_ALTERNATEBTNFACE, RGB(181, 181, 181), "ButtonAlternateFace" ),
    RGB_ENTRY( COLOR_HOTLIGHT, RGB(0, 0, 200), "HotTrackingColor" ),
    RGB_ENTRY( COLOR_GRADIENTACTIVECAPTION, RGB(166, 202, 240), "GradientActiveTitle" ),
    RGB_ENTRY( COLOR_GRADIENTINACTIVECAPTION, RGB(192, 192, 192), "GradientInactiveTitle" ),
    RGB_ENTRY( COLOR_MENUHILIGHT, RGB(10, 36, 106), "MenuHilight" ),
    RGB_ENTRY( COLOR_MENUBAR, RGB(212, 208, 200), "MenuBar" )
#undef RGB_ENTRY
};

COLORREF GetSysColor(int i){
    if(i>=0 && i<ARRAYSIZE(system_colors))
        return system_colors[i].val;
    return RGBA(255,255,255,255);
}

HGDIOBJ  GetStockObject(int i)
{
    switch(i){
    case NULL_BITMAP:
    {
        static _GdiObj bmp(OBJ_BITMAP, nullptr,nullptr);
        return &bmp;
    }
        case NULL_BRUSH:
        {
            static LOGBRUSH log;
            log.lbStyle=BS_NULL;
            log.lbColor = RGBA(0,0,0,0);
            static _GdiObj br(OBJ_BRUSH,&log, nullptr);
            return &br;
        }
        case BLACK_BRUSH:{
            static LOGBRUSH log;
            log.lbStyle=BS_SOLID;
            log.lbColor = RGBA(0,0,0,255);
            static _GdiObj br(OBJ_BRUSH,&log, nullptr);
            return &br;
        }
        case WHITE_BRUSH:{
            static LOGBRUSH log;
            log.lbStyle=BS_SOLID;
            log.lbColor = RGBA(255,255,255,255);
            static _GdiObj br(OBJ_BRUSH,&log, nullptr);
            return &br;
        }
        case NULL_PEN:
        {
            static LOGPEN log;
            log.lopnStyle=PS_NULL;
            log.lopnWidth = 0;
            static _GdiObj pen(OBJ_PEN,&log, nullptr);
            return &pen;
        }
        case BLACK_PEN:
        {
            static LOGPEN log;
            log.lopnStyle=PS_SOLID;
            log.lopnWidth = 1;
            log.lopnColor = RGBA(0,0,0,255);
            static _GdiObj pen(OBJ_PEN,&log, nullptr);
            return &pen;
        }
        case WHITE_PEN:
        {
            static LOGPEN log;
            log.lopnStyle=PS_SOLID;
            log.lopnWidth = 1;
            log.lopnColor = RGBA(255,255,255,255);
            static _GdiObj pen(OBJ_PEN,&log, nullptr);
            return &pen;
        }
        case SYSTEM_FONT:
        case DEFAULT_GUI_FONT:
        {
            static LOGFONT lf={0};
            strcpy(lf.lfFaceName,"Arial");
            lf.lfHeight=20;
            lf.lfWeight=400;
            static _GdiObj font(OBJ_FONT,&lf, nullptr);
            return &font;
        }
    }
    return HGDIOBJ(0);
}

BOOL  Rectangle(HDC hdc, int left, int top, int right, int bottom)
{

    if(!IsNullBrush(hdc->brush)){
        cairo_save(hdc->cairo);
        ApplyBrush(hdc->cairo, hdc->brush);
        cairo_rectangle(hdc->cairo,left,top,right-left,bottom-top);
        cairo_fill(hdc->cairo);
        cairo_restore(hdc->cairo);
    }
    if (!IsNullPen(hdc->pen)) {
        cairo_save(hdc->cairo);
        ApplyPen(hdc->cairo, hdc->pen);
        cairo_rectangle(hdc->cairo, left, top, right - left, bottom - top);
        cairo_stroke(hdc->cairo);
        cairo_restore(hdc->cairo);
    }
    return TRUE;
}

void drawRoundRect(cairo_t* cr, double x, double y, double width, double height, double rx, double ry) {
    double degrees = M_PI / 180.0;

    double kappa = 0.5522848;
    double ox = (width / 2) * kappa;
    double oy = (height / 2) * kappa;
    double xe = x + width;
    double ye = y + height;
    double xm = x + width / 2;
    double ym = y + height / 2;

    cairo_new_sub_path(cr);
    cairo_move_to(cr, x, ym);
    cairo_curve_to(cr, x, ym - ry * oy, xm - rx * ox, y, xm, y);
    cairo_curve_to(cr, xm + rx * ox, y, xe, ym - ry * oy, xe, ym);
    cairo_curve_to(cr, xe, ym + ry * oy, xm + rx * ox, ye, xm, ye);
    cairo_curve_to(cr, xm - rx * ox, ye, x, ym + ry * oy, x, ym);
    cairo_close_path(cr);
}

BOOL  RoundRect(HDC hdc, int left, int top, int right, int bottom, int width, int height)
{
    cairo_t *ctx = hdc->cairo;
    cairo_save(ctx);
    drawRoundRect(ctx, left, top, right-left, bottom-top, width,height);
    if(ApplyBrush(ctx,hdc->brush)){
        cairo_fill(ctx);
    }
    if(ApplyPen(ctx,hdc->pen))
    {
        cairo_stroke(ctx);
    }
    cairo_restore(ctx);
    return TRUE;
}

BOOL  Polyline(HDC hdc, const POINT *apt, int cpt)
{
    cairo_t *ctx = hdc->cairo;
    cairo_save(ctx);
    ApplyPen(ctx,hdc->pen);

    for(int i=0;i<cpt-1;i++){
        cairo_move_to(ctx,apt[i].x,apt[i].y);
        cairo_line_to(ctx,apt[i+1].x,apt[i+1].y);
    }
    cairo_restore(ctx);
    return 0;
}

BOOL  SetViewportOrgEx(HDC hdc, int x, int y, LPPOINT lppt)
{
    cairo_matrix_t mtx;
    cairo_get_matrix(hdc->cairo,&mtx);
    if(lppt){
        lppt->x = (int)floor(mtx.x0);
        lppt->y = (int)floor(mtx.y0);
    }

    mtx.x0=x;
    mtx.y0=y;

    cairo_set_matrix(hdc->cairo,&mtx);
    return TRUE;
}

BOOL GetViewportOrgEx(HDC hdc, LPPOINT lpPoint) 
{
    if (!lpPoint)
        return FALSE;
    cairo_matrix_t mtx;
    cairo_get_matrix(hdc->cairo, &mtx);
    lpPoint->x = (int)floor(mtx.x0);
    lpPoint->y = (int)floor(mtx.y0);
    return TRUE;
}

BOOL  OffsetViewportOrgEx(HDC hdc, int x, int y, LPPOINT lppt)
{
    cairo_matrix_t mtx;
    cairo_get_matrix(hdc->cairo,&mtx);
    if(lppt){
        lppt->x = (int)floor(mtx.x0);
        lppt->y = (int)floor(mtx.y0);
    }
    return SetViewportOrgEx(hdc, x + mtx.x0, y + mtx.y0, lppt);
}

int  FillRect(HDC hdc, const RECT *lprc, HBRUSH hbr)
{
    LOGBRUSH *plog =(LOGBRUSH*)GetGdiObjPtr(hbr);
    CairoColor cr;
    RGBA2CairoColor(plog->lbColor,&cr);
    cairo_set_source_rgba(hdc->cairo,cr.r,cr.g,cr.b,cr.a);
    cairo_rectangle(hdc->cairo,lprc->left,lprc->top,lprc->right-lprc->left,lprc->bottom-lprc->top);
    cairo_fill(hdc->cairo);
    return 1;
}

int  FrameRect(HDC hdc, const RECT *lprc, HBRUSH hbr)
{
    cairo_t * ctx = hdc->cairo;
    cairo_save(ctx);
    ApplyPen(ctx,hdc->pen);
    cairo_rectangle(hdc->cairo,lprc->left,lprc->top,lprc->right-lprc->left,lprc->bottom-lprc->top);
    cairo_stroke(ctx);
    cairo_restore(ctx);
    return TRUE;
}

BOOL  InvertRect(HDC hdc, const RECT *lprc)
{
    cairo_t * ctx = hdc->cairo;
    cairo_save(ctx);
    cairo_set_source_rgb(ctx,1.0,1.0,1.0);
    cairo_set_operator(ctx,CAIRO_OPERATOR_XOR);
    cairo_rectangle(ctx,lprc->left,lprc->top,lprc->right-lprc->left,lprc->bottom-lprc->top);
    cairo_fill(ctx);

    cairo_restore(ctx);
    return TRUE;
}

BOOL  Ellipse(HDC hdc, int left, int top, int right, int bottom)
{
    cairo_t *ctx = hdc->cairo;
    double x = (left + right) / 2;
    double y = (top + bottom) / 2;
    double scale_x = (right - left) / 2;
    double scale_y = (bottom - top) / 2;
    if(!IsNullBrush(hdc->brush)){
        cairo_save(ctx);
        cairo_translate(ctx, x, y);
        cairo_scale(ctx, scale_x, scale_y);
        cairo_move_to(ctx, 1, 0);
        cairo_arc(ctx, 0, 0, 1, 0, 2 * M_PI);
        cairo_restore(ctx);
        cairo_save(ctx);
        ApplyBrush(ctx, hdc->brush);
        cairo_fill(ctx);
        cairo_restore(ctx);
    }
    if(!IsNullPen(hdc->pen)){
        cairo_save(ctx);
        cairo_translate(ctx, x, y);
        cairo_scale(ctx, scale_x, scale_y);
        cairo_move_to(ctx, 1, 0);
        cairo_arc(ctx, 0, 0, 1, 0, 2 * M_PI);
        cairo_restore(ctx);
        cairo_save(ctx);
        ApplyPen(ctx, hdc->pen);
        cairo_stroke(ctx);        
        cairo_restore(ctx);
    }

    return TRUE;
}

BOOL  Pie(HDC hdc, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
    double wid = x2 - x1;
    double hei = y2 - y1;
    if (wid == 0 || hei == 0)
        return FALSE;
    double cx = (x1 + x2) / 2;
    double cy = (y1 + y2) / 2;
    double dx3 = double(x3 - cx) / wid;
    double dx4 = double(x4 - cx) / wid;
    double dy3 = double(y3 - cy) / hei;
    double dy4 = double(y4 - cy) / hei;
    double arc1 = atan2(dy3, dx3);
    double arc2 = atan2(dy4, dx4);

    if (!IsNullBrush(hdc->brush)) {
        cairo_save(hdc->cairo);
        cairo_translate(hdc->cairo, cx, cy);
        cairo_scale(hdc->cairo, wid, hei);
        cairo_move_to(hdc->cairo, 0, 0);
        cairo_line_to(hdc->cairo, dx3, dx4);
        cairo_arc(hdc->cairo, 0, 0, 0.5, arc1, arc2);
        cairo_close_path(hdc->cairo);
        cairo_restore(hdc->cairo);
        cairo_save(hdc->cairo);
        ApplyBrush(hdc->cairo, hdc->brush);
        cairo_fill(hdc->cairo);
        cairo_restore(hdc->cairo);
    }
    if (!IsNullPen(hdc->pen)) {
        cairo_save(hdc->cairo);
        cairo_translate(hdc->cairo, cx, cy);
        cairo_scale(hdc->cairo, wid, hei);
        cairo_move_to(hdc->cairo, 0, 0);
        cairo_line_to(hdc->cairo, dx3, dx4);
        cairo_arc(hdc->cairo, 0, 0, 0.5, arc1, arc2);
        cairo_close_path(hdc->cairo);
        cairo_restore(hdc->cairo);
        cairo_save(hdc->cairo);
        ApplyPen(hdc->cairo, hdc->pen);
        cairo_stroke(hdc->cairo);
        cairo_restore(hdc->cairo);
    }
    return TRUE;
}

BOOL  Arc(HDC hdc, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
    double wid = x2 - x1;
    double hei = y2 - y1;
    if (wid == 0 || hei == 0)
        return FALSE;

    cairo_save(hdc->cairo);
    double cx = (x1 + x2) / 2;
    double cy = (y1 + y2) / 2;
    cairo_translate(hdc->cairo,cx, cy);
    cairo_scale(hdc->cairo, wid, hei);
    
    double dx3 = double(x3 - cx) / wid;
    double dx4 = double(x4 - cx) / wid;

    double dy3 = double(y3 - cy) / hei;
    double dy4 = double(y4 - cy) / hei;

    double arc1 = atan2(dy3, dx3);
    double arc2 = atan2(dy4, dx4);
    cairo_move_to(hdc->cairo, dx3, dx4);
    cairo_arc(hdc->cairo, 0, 0, 0.5, arc1,arc2);
    cairo_restore(hdc->cairo);
    cairo_save(hdc->cairo);
    if (ApplyPen(hdc->cairo, hdc->pen)){
        cairo_stroke(hdc->cairo);
    }
    cairo_restore(hdc->cairo);
    return TRUE;
}

BOOL  GetWorldTransform(HDC hdc, LPXFORM lpxf)
{
    cairo_matrix_t mtx;
    cairo_get_matrix(hdc->cairo,&mtx);
    lpxf->eM11 = mtx.xx;
    lpxf->eM12 = mtx.xy;
    lpxf->eM21 = mtx.yy;
    lpxf->eM22 = mtx.yx;
    lpxf->eDx = mtx.x0;
    lpxf->eDy = mtx.y0;
    return TRUE;
}

BOOL  SetWorldTransform(HDC hdc, const XFORM *lpxf)
{
    cairo_matrix_t mtx;
    mtx.xx =lpxf->eM11; 
    mtx.xy =lpxf->eM12;
    mtx.yy =lpxf->eM21;
    mtx.yx =lpxf->eM22;
    mtx.x0 =lpxf->eDx ;
    mtx.y0 =lpxf->eDy ;
    cairo_set_matrix(hdc->cairo,&mtx);
    return TRUE;
}

int  SetROP2(HDC hdc, int rop2)
{
    return 0;
}

COLORREF  SetTextColor(HDC hdc, COLORREF color)
{
    COLORREF ret = hdc->crText;
    hdc->crText = color;
    return ret;
}

HBITMAP CreateBitmap(
    int nWidth,         // bitmap width, in pixels
    int nHeight,        // bitmap height, in pixels
    UINT cPlanes,       // number of color planes
    UINT cBitsPerPel,   // number of bits to identify color
    CONST VOID* lpvBits // color data array
) {
    if (cBitsPerPel != 32 || cPlanes!=1)
        return nullptr;
    cairo_surface_t* ret = cairo_image_surface_create_for_data((unsigned char*)lpvBits,CAIRO_FORMAT_ARGB32, nWidth, nHeight, nWidth*4);
    if (ret) {
        return InitGdiObj(OBJ_BITMAP, ret);
    }
    else
    {
        return nullptr;
    }
}

static double color16_to_double(USHORT v) {
    return v * 1.0 / 0xffff;
}

static void NormalizeRect(RECT* prc) {
    if (prc->left > prc->right) {
        std::swap(prc->left, prc->right);
    }
    if (prc->top > prc->bottom) {
        std::swap(prc->top, prc->bottom);
    }
}

BOOL GradientFill(HDC hdc, TRIVERTEX* pVertices, ULONG nVertices, void* pMesh, ULONG nMeshElements, DWORD dwMode)
{
    if (dwMode == GRADIENT_FILL_TRIANGLE)
        return FALSE;
    PGRADIENT_RECT pGradientRect = (PGRADIENT_RECT)pMesh;
    //fill rect horz
    for (ULONG i = 0; i < nMeshElements; i++) {
        if (pGradientRect[i].UpperLeft >= nVertices || pGradientRect[i].LowerRight >= nVertices)
            return FALSE;
        TRIVERTEX* vertix0 = pVertices + pGradientRect[i].UpperLeft;
        TRIVERTEX* vertix1 = pVertices + pGradientRect[i].LowerRight;
        cairo_pattern_t* gradient = dwMode == GRADIENT_FILL_RECT_H ?
            cairo_pattern_create_linear(vertix0->x, vertix0->y, vertix1->x, vertix0->y) ://horz gradient
            cairo_pattern_create_linear(vertix0->x, vertix0->y, vertix0->x, vertix1->y); //vert gradient

        cairo_pattern_add_color_stop_rgba(gradient, 0, color16_to_double(vertix0->Red),
            color16_to_double(vertix0->Green),
            color16_to_double(vertix0->Blue),
            color16_to_double(vertix0->Alpha));
        cairo_pattern_add_color_stop_rgba(gradient, 1, color16_to_double(vertix1->Red),
            color16_to_double(vertix1->Green),
            color16_to_double(vertix1->Blue),
            color16_to_double(vertix1->Alpha));
        cairo_set_source(hdc->cairo, gradient);
        RECT rc = { vertix0->x, vertix0->y, vertix1->x, vertix1->y };
        NormalizeRect(&rc);
        cairo_rectangle(hdc->cairo, rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top);
        cairo_fill(hdc->cairo);
        cairo_pattern_destroy(gradient);
    }
    return TRUE;
}


int GetDeviceCaps(HDC hdc, int cap)
{
    if(cap == BITSPIXEL) //todo:hjx
        return 32;
    if(cap == PLANES)
        return 1;
    return 0;
}

struct _IconObj {
    BOOL fIcon;
    DWORD xHotspot;
    DWORD yHotspot;
    HBITMAP hbmMask;
    HBITMAP hbmColor;
};

BOOL GetIconInfo(HICON hIcon,
    PICONINFO piconinfo
) {
    if (!hIcon)
        return FALSE;
    piconinfo->fIcon = hIcon->fIcon;
    piconinfo->xHotspot = hIcon->xHotspot;
    piconinfo->yHotspot = hIcon->xHotspot;
    piconinfo->hbmColor = hIcon->hbmColor;
    if (piconinfo->hbmColor)
        RefGdiObj(piconinfo->hbmColor);
    piconinfo->hbmMask = hIcon->hbmMask;
    if(piconinfo->hbmMask)
        RefGdiObj(piconinfo->hbmMask);
    return TRUE;
}

HICON CreateIconIndirect(PICONINFO piconinfo)
{
    _IconObj* icon = new _IconObj;
    icon->fIcon = piconinfo->fIcon;
    icon->xHotspot = piconinfo->xHotspot;
    icon->yHotspot = piconinfo->xHotspot;
    icon->hbmColor = piconinfo->hbmColor;
    if(icon->hbmColor)
        RefGdiObj(icon->hbmColor);
    icon->hbmMask = piconinfo->hbmMask;
    if(icon->hbmMask)
        RefGdiObj(icon->hbmMask);
    return icon;
}

BOOL DrawIcon(HDC hDC, int X, int Y, HICON hIcon)
{
    return DrawIconEx(hDC, X, Y, hIcon, -1, -1, 0, NULL, DI_NORMAL);
}

BOOL DrawIconEx(HDC hDC, int xLeft, int yTop, HICON hIcon, int cxWidth, int cyWidth, UINT istepIfAniCur, HBRUSH hbrFlickerFreeDraw, UINT diFlags)
{
    if (!hIcon || !hIcon->hbmColor)
        return FALSE;
    BITMAP bm;
    GetObject(hIcon->hbmColor, sizeof(bm), &bm);
    if (bm.bmBitsPixel != 32)
        return FALSE;
    if (cxWidth < 0) cxWidth = bm.bmWidth;
    if (cyWidth < 0) cyWidth = bm.bmHeight;
    HDC memdc = CreateCompatibleDC(hDC);
    HGDIOBJ oldBmp = SelectObject(memdc, hIcon->hbmColor);
    BLENDFUNCTION bf;
    bf.BlendOp = AC_SRC_ALPHA;
    bf.SourceConstantAlpha = 255;
    AlphaBlend(hDC, xLeft, yTop, cxWidth, cyWidth, memdc, 0, 0, bm.bmWidth, bm.bmHeight, bf);
    SelectObject(memdc, oldBmp);
    DeleteDC(memdc);

    return TRUE;
}

BOOL DestroyIcon(HICON hIcon)
{
    if (!hIcon)
        return FALSE;
    if (hIcon->hbmColor)
        DeleteObject(hIcon->hbmColor);
    if (hIcon->hbmMask)
        DeleteObject(hIcon->hbmMask);
    delete hIcon;
    return TRUE;
}
