#include <platform.h>
#include <gdi.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xcb.h>
#include <xcb/xcb_aux.h>
#include <math.h>

#include "sdc.h"
#include "SConnection.h"
using namespace SOUI;

typedef struct _GdiObj{
    int type;
    void *ptr;
    _GdiObj(int _type,void * _ptr):type(_type),ptr(_ptr){
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

HGDIOBJ InitGdiObj(int type,void *ptr){
    return new _GdiObj(type,ptr);
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
            cairo_pattern_t *pattern = (cairo_pattern_t*)br->lbHatch;
            cairo_set_source(ctx,pattern);
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
    cairo_region_t* rgn = (cairo_region_t* )GetGdiObjPtr(hRgn);
    int nrc = cairo_region_num_rectangles(rgn);
    for(int i=0;i<nrc;i++){
        cairo_rectangle_int_t rc ;
        cairo_region_get_rectangle(rgn,i,&rc);
        cairo_rectangle(ctx,rc.x,rc.y,rc.width,rc.height);
    }
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
    HBITMAP bmp = CreateDIBSection(hdc,pbmi,0,nullptr,0,0);
    if(bmp){
        UpdateDIBPixmap(bmp,pbmi->bmiHeader.biWidth,pbmi->bmiHeader.biHeight,pbmi->bmiHeader.biBitCount,pbmi->bmiHeader.biWidth*4,pjBits);
    }
    return bmp;
}

HBRUSH  CreateDIBPatternBrush(HGLOBAL h, UINT iUsage)
{
    return 0;
}

HBRUSH CreateDIBPatternBrushPt(const VOID *lpPackedDIB, UINT iUsage)
{
    return 0;
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
    if(hdc==0){
        //todo: get root window dc
        SConnection *conn = SConnMgr::instance()->getConnection();
        cairo_surface_t * surface = cairo_xcb_surface_create(conn->connection, conn->screen->root, xcb_aux_find_visual_by_id(conn->screen,conn->screen->root_visual), 10,10);
        HBITMAP bmp = InitGdiObj(OBJ_BITMAP,surface);
        hdc = new _SDC(conn->screen->root,bmp);
        return hdc;
    }else{
        return CreateDC(hdc->hwnd,0,0);
    }
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
            cairo_destroy(hdc->cairo);
            hdc->bmp = h;
            hdc->cairo = cairo_create((cairo_surface_t*)GetGdiObjPtr(h));
            break;
        }
    }
    return ret;
}

BOOL DeleteObject(HGDIOBJ hObj)
{
    if(!hObj)
        return FALSE;
    switch (hObj->type)
    {
    case OBJ_BITMAP:
        {
            cairo_surface_destroy((cairo_surface_t*)hObj->ptr);
        }
        break;
    case OBJ_REGION:
    {
        if(hObj->ptr) 
            cairo_region_destroy((cairo_region_t*)hObj->ptr);
    }
        break;
    case OBJ_PEN:
        delete (LOGPEN*)hObj->ptr;
        break;
    case OBJ_BRUSH:
        delete (LOGBRUSH*)hObj->ptr;
        break;
    case OBJ_FONT:
        delete (LOGFONT*)hObj->ptr;
        break;
    default:
        break;
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

int SelectClipRgn(HDC hdc, HRGN hrgn)
{

    CombineRgn(hdc->rgn,hrgn,nullptr,RGN_COPY);
    ApplyRegion(hdc->cairo, hdc->rgn);
    return RgnComplexity(hdc->rgn);
}

int ExtSelectClipRgn(HDC hdc, HRGN hrgn, int mode)
{
    int ret= CombineRgn(hdc->rgn,hrgn,hdc->rgn,mode);
    ApplyRegion(hdc->cairo, hdc->rgn);
    return ret;
}

int ExcludeClipRect(HDC hdc, int left, int top, int right, int bottom)
{
    HRGN hrgn = CreateRectRgn(left,top,right,bottom);
    int ret = CombineRgn(hdc->rgn,hdc->rgn,hrgn,RGN_DIFF);
    ApplyRegion(hdc->cairo, hdc->rgn);
    DeleteObject(hrgn);
    return ret;
}

int IntersectClipRect(HDC hdc, int left, int top, int right, int bottom)
{
    HRGN hrgn = CreateRectRgn(left,top,right,bottom);
    int ret = CombineRgn(hdc->rgn,hdc->rgn,hrgn,RGN_AND);
    ApplyRegion(hdc->cairo, hdc->rgn);
    DeleteObject(hrgn);
    return ret;
}

int  GetClipRgn(HDC hdc, HRGN hrgn)
{
    CombineRgn(hrgn,hdc->rgn,nullptr,RGN_COPY);
    return RgnComplexity(hrgn) == NULLREGION?0:1;
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

BOOL  AlphaBlend(HDC hdc, int x, int y, int wDst, int hDst, HDC hdcSrc, int x1, int y1, int wSrc, int hSrc, BLENDFUNCTION ftn)
{
    assert(hdc && hdcSrc);
    cairo_surface_t *src = (cairo_surface_t *)GetGdiObjPtr(hdcSrc->bmp);
    
    cairo_save(hdc->cairo);
    cairo_rectangle(hdc->cairo,x,y,wDst,hDst);
    cairo_clip(hdc->cairo);
    
    cairo_translate(hdc->cairo,x,y);
    double scale_x = wDst*1.0/wSrc;
    double scale_y = hDst*1.0/hSrc;
    cairo_scale(hdc->cairo,scale_x,scale_y);

    cairo_set_source_surface(hdc->cairo,src,-x1,-y1);
    
    //todo: draw alpha

    // cairo_pattern_t *pattern = cairo_pattern_create_for_surface(src);
    // cairo_set_source(hdc->cairo,pattern);
    // cairo_set_operator(hdc->cairo,CAIRO_OPERATOR_OVER);
    // cairo_set_source_rgba(hdc->cairo,1.0,1.0,1.0,0.5);

    cairo_rectangle(hdc->cairo,0,0,wDst/scale_x,hDst/scale_y);
    cairo_fill(hdc->cairo);

    // cairo_pattern_destroy(pattern);

    cairo_restore(hdc->cairo);
    return 0;
}

BOOL BitBlt(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, DWORD rop)
{
    assert(hdc && hdcSrc);
    cairo_surface_t *src = (cairo_surface_t *)GetGdiObjPtr(hdcSrc->bmp);

    cairo_save(hdc->cairo);
    cairo_rectangle(hdc->cairo,x,y,cx,cy);
    cairo_clip(hdc->cairo);
    cairo_rectangle(hdc->cairo,x,y,cx,cy);
    switch(rop){
        case SRCCOPY:
        cairo_set_source_surface(hdc->cairo,src,x-x1,y-y1);
        cairo_set_operator(hdc->cairo,CAIRO_OPERATOR_OVER);
        break;
        case SRCINVERT:
        cairo_set_source_surface(hdc->cairo,src,x-x1,y-y1);
        cairo_set_operator(hdc->cairo,CAIRO_OPERATOR_XOR);
        break;
        case SRCPAINT:
        cairo_set_source_surface(hdc->cairo,src,x-x1,y-y1);
        cairo_set_operator(hdc->cairo,CAIRO_OPERATOR_OVER);
        break;
        case SRCAND:
        cairo_set_source_surface(hdc->cairo,src,x-x1,y-y1);
        cairo_set_operator(hdc->cairo,CAIRO_OPERATOR_DEST_IN);
        break;
        case DSTINVERT:
        cairo_set_source_rgb(hdc->cairo,1.0,1.0,1.0);
        cairo_set_operator(hdc->cairo,CAIRO_OPERATOR_XOR);
        break;
    }
    cairo_fill(hdc->cairo);
    cairo_restore(hdc->cairo);
    return TRUE;
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
        pRect->bottom = pRect->top + ext.height;
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
    psizl->cy = ext.height;
    cairo_restore(hdc->cairo);

    return TRUE;
}

COLORREF GetSysColor(int i){
    switch(i){
        case COLOR_INFOBK:
        return RGBA(128,128,128,255);
    }
    return RGBA(255,255,255,255);
}

HGDIOBJ  GetStockObject(int i)
{
    switch(i){
        case NULL_BRUSH:
        {
            static LOGBRUSH log;
            log.lbStyle=BS_NULL;
            log.lbColor = RGBA(0,0,0,0);
            static _GdiObj br(OBJ_BRUSH,&log);
            return &br;
        }
        case BLACK_BRUSH:{
            static LOGBRUSH log;
            log.lbStyle=BS_SOLID;
            log.lbColor = RGBA(0,0,0,255);
            static _GdiObj br(OBJ_BRUSH,&log);
            return &br;
        }
        case WHITE_BRUSH:{
            static LOGBRUSH log;
            log.lbStyle=BS_SOLID;
            log.lbColor = RGBA(255,255,255,255);
            static _GdiObj br(OBJ_BRUSH,&log);
            return &br;
        }
        case NULL_PEN:
        {
            static LOGPEN log;
            log.lopnStyle=PS_NULL;
            log.lopnWidth = 0;
            static _GdiObj pen(OBJ_PEN,&log);
            return &pen;
        }
        case BLACK_PEN:
        {
            static LOGPEN log;
            log.lopnStyle=PS_SOLID;
            log.lopnWidth = 1;
            log.lopnColor = RGBA(0,0,0,255);
            static _GdiObj pen(OBJ_PEN,&log);
            return &pen;
        }
        case WHITE_PEN:
        {
            static LOGPEN log;
            log.lopnStyle=PS_SOLID;
            log.lopnWidth = 1;
            log.lopnColor = RGBA(255,255,255,255);
            static _GdiObj pen(OBJ_PEN,&log);
            return &pen;
        }
        case SYSTEM_FONT:
        case DEFAULT_GUI_FONT:
        {
            static LOGFONT lf={0};
            strcpy(lf.lfFaceName,"Arial");
            lf.lfHeight=20;
            lf.lfWeight=400;
            static _GdiObj font(OBJ_FONT,&lf);
            return &font;
        }
    }
    return HGDIOBJ(0);
}

BOOL  Rectangle(HDC hdc, int left, int top, int right, int bottom)
{
    cairo_save(hdc->cairo);
    LOGBRUSH *br = (LOGBRUSH*)GetGdiObjPtr(hdc->brush);
    if(br->lbStyle == BS_SOLID){
        CairoColor cr;
        RGBA2CairoColor(br->lbColor,&cr);
        cairo_set_source_rgba(hdc->cairo,cr.r,cr.g,cr.b,cr.a);
        cairo_rectangle(hdc->cairo,left,top,right-left,bottom-top);
        cairo_fill(hdc->cairo);
    }

    LOGPEN * pen= (LOGPEN *)GetGdiObjPtr(hdc->pen);
    if(pen->lopnStyle==PS_SOLID){
        CairoColor cr;
        RGBA2CairoColor(pen->lopnColor,&cr);
        cairo_set_source_rgba(hdc->cairo,cr.r,cr.g,cr.b,cr.a);
        cairo_set_line_width(hdc->cairo,pen->lopnWidth);
        cairo_rectangle(hdc->cairo,left,top,right-left,bottom-top);
        cairo_stroke(hdc->cairo);
    }
    cairo_restore(hdc->cairo);
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

BOOL  OffsetViewportOrgEx(HDC hdc, int x, int y, LPPOINT lppt)
{
    cairo_matrix_t mtx;
    cairo_get_matrix(hdc->cairo,&mtx);
    if(lppt){
        lppt->x = (int)floor(mtx.x0);
        lppt->y = (int)floor(mtx.y0);
    }

    cairo_translate(hdc->cairo,x,y);
    return TRUE;
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
    cairo_save(ctx);
    double x = (left+right)/2;
    double y = (top+bottom)/2;
    double scale_x = right-left;
    double scale_y = bottom-top;
    cairo_translate(ctx,x, y);
    cairo_scale(ctx,scale_x, scale_y);
    cairo_arc(ctx,0,0,1,0,2*M_PI);

    if(ApplyBrush(ctx,hdc->brush)){
        cairo_fill(ctx);
    }
    if(ApplyPen(ctx,hdc->pen)){
        cairo_stroke(ctx);
    }
    cairo_restore(ctx);
    return TRUE;
}

BOOL  Pie(HDC hdc, int left, int top, int right, int bottom, int xr1, int yr1, int xr2, int yr2)
{
    return 0;
}

BOOL  Arc(HDC hdc, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
    return 0;
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

HANDLE LoadImage(HINSTANCE hInst, LPCSTR name, UINT type, int cx, int cy, UINT fuLoad)
{
    return HANDLE();
}