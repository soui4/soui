///////////////////////////////////////////////////////////////////////
/** File Name: SSkin.cpp */
///////////////////////////////////////////////////////////////////////
#include "souistd.h"
#include "core/SSkin.h"
#include "helper/SDIBHelper.h"
#include "helper/SplitString.h"
#include <core/SGradient.h>
#include "core/Svg.h"
#include <src/nanosvg.h>

SNSBEGIN

typedef bool (*Fun_Colorize)(COLORREF &crTarget, COLORREF crRef);

/**
 * Colorizes an SVG image by modifying all colors in the image.
 * @param image The image to colorize.
 * @param fun The function to use to colorize the image.
 * @param color The color to colorize the image to.
 * @return True if the image was colorized successfully, false otherwise.
 * @see Fun_Colorize
 */
static void ColorizeSVG(NSVGimage *image, Fun_Colorize fun, COLORREF color)
{
    if (!image)
        return;

    // Process shapes
    NSVGshape *shape = image->shapes;
    while (shape)
    {
        // Process fill color
        if (shape->fill.type == NSVG_PAINT_COLOR)
        {
            fun((COLORREF &)shape->fill.color, color);
        }
        else if (shape->fill.type == NSVG_PAINT_LINEAR_GRADIENT || shape->fill.type == NSVG_PAINT_RADIAL_GRADIENT)
        {
            // Process gradient stops
            NSVGgradient *grad = shape->fill.gradient;
            if (grad)
            {
                int i;
                for (i = 0; i < grad->nstops; i++)
                {
                    fun((COLORREF &)grad->stops[i].color, color);
                }
            }
        }

        // Process stroke color
        if (shape->stroke.type == NSVG_PAINT_COLOR)
        {
            fun((COLORREF &)shape->stroke.color, color);
        }
        else if (shape->stroke.type == NSVG_PAINT_LINEAR_GRADIENT || shape->stroke.type == NSVG_PAINT_RADIAL_GRADIENT)
        {
            // Process gradient stops
            NSVGgradient *grad = shape->stroke.gradient;
            if (grad)
            {
                int i;
                for (i = 0; i < grad->nstops; i++)
                {
                    fun((COLORREF &)grad->stops[i].color, color);
                }
            }
        }

        shape = shape->next;
    }

    // Process text elements
    NSVGtext *text = image->texts;
    while (text)
    {
        fun((COLORREF &)text->fillColor, color);
        text = text->next;
    }
}

/** Helper function: draw the SVG nine-grid */
static void DrawSVG9Patch(IRenderTarget *pRT, ISvgObj *pSvg, LPCRECT pRect, LPCRECT prcSrc, LPCRECT prcMargin, BYTE byAlpha, int nScale)
{
    if (!pRT || !pSvg || !pRect || !prcMargin)
        return;

    float srcX = 0, srcY = 0, srcWidth, srcHeight;
    if (prcSrc)
    {
        srcX = (float)prcSrc->left;
        srcY = (float)prcSrc->top;
        srcWidth = (float)(prcSrc->right - prcSrc->left);
        srcHeight = (float)(prcSrc->bottom - prcSrc->top);
    }
    else
    {
        NSVGimage *pImg = (NSVGimage *)pSvg->GetPtr();
        srcWidth = pImg->width;
        srcHeight = pImg->height;
    }

    float marginL = (float)prcMargin->left;
    float marginT = (float)prcMargin->top;
    float marginR = (float)prcMargin->right;
    float marginB = (float)prcMargin->bottom;

    // Calculate the 9 parts of the source region
    float srcMidX = srcX + marginL;
    float srcMidY = srcY + marginT;
    float srcMidW = srcWidth - marginL - marginR;
    float srcMidH = srcHeight - marginT - marginB;

    float fScale = (float)nScale / 100;
    float dst_marginL = marginL * fScale;
    float dst_marginT = marginT * fScale;
    float dst_marginR = marginR * fScale;
    float dst_marginB = marginB * fScale;

    // Calculate the 9 parts of the target region
    float dstMidX = (float)pRect->left + dst_marginL;
    float dstMidY = (float)pRect->top + dst_marginT;
    float dstMidW = (float)(pRect->right - pRect->left) - dst_marginL - dst_marginR;
    float dstMidH = (float)(pRect->bottom - pRect->top) - dst_marginT - dst_marginB;

    // Draw the 9 parts
    // Top-left corner
    if (marginL > 0 && marginT > 0)
    {
        RECT rcSrc = { (LONG)srcX, (LONG)srcY, (LONG)(srcX + marginL), (LONG)(srcY + marginT) };
        RECT rcDst = { pRect->left, pRect->top, (LONG)(pRect->left + dst_marginL), (LONG)(pRect->top + dst_marginT) };
        IntersectRect(&rcDst, &rcDst, pRect);
        pRT->DrawSVG(pSvg, &rcDst, &rcSrc, byAlpha);
    }

    // Top edge
    if (marginT > 0 && dstMidW > 0)
    {
        RECT rcSrc = { (LONG)srcMidX, (LONG)srcY, (LONG)(srcMidX + srcMidW), (LONG)(srcY + marginT) };
        RECT rcDst = { (LONG)dstMidX, pRect->top, (LONG)(dstMidX + dstMidW), (LONG)(pRect->top + dst_marginT) };
        IntersectRect(&rcDst, &rcDst, pRect);
        pRT->DrawSVG(pSvg, &rcDst, &rcSrc, byAlpha);
    }

    // Top-right corner
    if (marginR > 0 && marginT > 0)
    {
        RECT rcSrc = { (LONG)(srcX + srcWidth - marginR), (LONG)srcY, (LONG)(srcX + srcWidth), (LONG)(srcY + marginT) };
        RECT rcDst = { (LONG)(pRect->right - dst_marginR), pRect->top, pRect->right, (LONG)(pRect->top + dst_marginT) };
        IntersectRect(&rcDst, &rcDst, pRect);
        pRT->DrawSVG(pSvg, &rcDst, &rcSrc, byAlpha);
    }

    // Left edge
    if (marginL > 0 && dstMidH > 0)
    {
        RECT rcSrc = { (LONG)srcX, (LONG)srcMidY, (LONG)(srcX + marginL), (LONG)(srcMidY + srcMidH) };
        RECT rcDst = { pRect->left, (LONG)dstMidY, (LONG)(pRect->left + dst_marginL), (LONG)(dstMidY + dstMidH) };
        IntersectRect(&rcDst, &rcDst, pRect);
        pRT->DrawSVG(pSvg, &rcDst, &rcSrc, byAlpha);
    }

    // Center
    if (dstMidW > 0 && dstMidH > 0)
    {
        RECT rcSrc = { (LONG)srcMidX, (LONG)srcMidY, (LONG)(srcMidX + srcMidW), (LONG)(srcMidY + srcMidH) };
        RECT rcDst = { (LONG)dstMidX, (LONG)dstMidY, (LONG)(dstMidX + dstMidW), (LONG)(dstMidY + dstMidH) };
        IntersectRect(&rcDst, &rcDst, pRect);
        pRT->DrawSVG(pSvg, &rcDst, &rcSrc, byAlpha);
    }

    // Right edge
    if (marginR > 0 && dstMidH > 0)
    {
        RECT rcSrc = { (LONG)(srcX + srcWidth - marginR), (LONG)srcMidY, (LONG)(srcX + srcWidth), (LONG)(srcMidY + srcMidH) };
        RECT rcDst = { (LONG)(pRect->right - dst_marginR), (LONG)dstMidY, pRect->right, (LONG)(dstMidY + dstMidH) };
        IntersectRect(&rcDst, &rcDst, pRect);
        pRT->DrawSVG(pSvg, &rcDst, &rcSrc, byAlpha);
    }

    // Bottom-left corner
    if (marginL > 0 && marginB > 0)
    {
        RECT rcSrc = { (LONG)srcX, (LONG)(srcY + srcHeight - marginB), (LONG)(srcX + marginL), (LONG)(srcY + srcHeight) };
        RECT rcDst = { pRect->left, (LONG)(pRect->bottom - dst_marginB), (LONG)(pRect->left + dst_marginL), pRect->bottom };
        IntersectRect(&rcDst, &rcDst, pRect);
        pRT->DrawSVG(pSvg, &rcDst, &rcSrc, byAlpha);
    }

    // Bottom edge
    if (marginB > 0 && dstMidW > 0)
    {
        RECT rcSrc = { (LONG)srcMidX, (LONG)(srcY + srcHeight - marginB), (LONG)(srcMidX + srcMidW), (LONG)(srcY + srcHeight) };
        RECT rcDst = { (LONG)dstMidX, (LONG)(pRect->bottom - dst_marginB), (LONG)(dstMidX + dstMidW), pRect->bottom };
        IntersectRect(&rcDst, &rcDst, pRect);
        pRT->DrawSVG(pSvg, &rcDst, &rcSrc, byAlpha);
    }

    // Bottom-right corner
    if (marginR > 0 && marginB > 0)
    {
        RECT rcSrc = { (LONG)(srcX + srcWidth - marginR), (LONG)(srcY + srcHeight - marginB), (LONG)(srcX + srcWidth), (LONG)(srcY + srcHeight) };
        RECT rcDst = { (LONG)(pRect->right - dst_marginR), (LONG)(pRect->bottom - dst_marginB), pRect->right, pRect->bottom };
        IntersectRect(&rcDst, &rcDst, pRect);
        pRT->DrawSVG(pSvg, &rcDst, &rcSrc, byAlpha);
    }
}

///////////////////////////////////////////////////////////////////////
/** SSkinImgList */
SSkinImgList::SSkinImgList()
    : m_nStates(1)
    , m_bTile(FALSE)
    , m_bVertical(FALSE)
    , m_filterLevel(kUndef_FilterLevel)
    , m_bAutoFit(TRUE)
    , m_tileMode(TM_Both)
    , m_bLazyLoad(FALSE)
    , m_bCacheSvg(TRUE)
{
}

SSkinImgList::~SSkinImgList()
{
}

SIZE SSkinImgList::GetImageSize(BOOL bRaw) const
{
    SIZE ret = { 0, 0 };
    if (GetSvg())
    {
        ret = GetSvg()->Size();
        if (!bRaw)
        {
            ret.cx = MulDiv(ret.cx, GetScale(), 100);
            ret.cy = MulDiv(ret.cy, GetScale(), 100);
        }
    }
    else if (GetImage())
    {
        ret = GetImage()->Size();
    }
    return ret;
}

SIZE SSkinImgList::GetSkinSize() const
{
    return _GetSkinSize(FALSE);
}

SIZE SSkinImgList::_GetSkinSize(BOOL bRaw) const
{
    SIZE ret = GetImageSize(bRaw);
    if (m_bVertical)
        ret.cy /= m_nStates;
    else
        ret.cx /= m_nStates;
    return ret;
}

int SSkinImgList::GetStates() const
{
    return m_nStates;
}

void SSkinImgList::LoadSrcImage() const
{
    if (m_strSrc.IsEmpty())
        return;
    SStringTList list;
    int segs = ParseResID(S_CW2T(m_strSrc), list);
    for (int i = 0; i < segs; i++)
    {
        list[i].MakeLower();
    }
    if (segs == 1)
        list.InsertAt(0, _T("file"));
    if (list[0] == _T("svg") || (list[0] == _T("file") && list[1].EndsWith(_T("svg"))))
    {
        m_pSvg.Attach(CreateSvgFromResId(S_CW2T(m_strSrc)));
        // The SVG source changed (including first load); the rasterization cache is invalidated and rebuilt on demand
        m_pCacheBmp = NULL;
        m_szPerStateCache.cx = m_szPerStateCache.cy = 0;
    }
    else
    {
        m_pImg.Attach(LOADIMAGE2(m_strSrc));
    }
}

void SSkinImgList::OnInitFinished(IXmlNode *xmlNode)
{
    __baseCls::OnInitFinished(xmlNode);
    if (!m_bLazyLoad && !m_strSrc.IsEmpty())
    {
        LoadSrcImage();
    }
}

IBitmapS *SSkinImgList::GetSvgCacheBitmap(IRenderTarget *pRT, int cxPerState, int cyPerState) const
{
    ISvgObj *pSvg = GetSvg();
    SASSERT(pSvg);
    if (cxPerState <= 0 || cyPerState <= 0)
        return NULL;

    // Cache hit: same per-state dimensions (skin's states/layout are immutable)
    if (m_pCacheBmp && m_szPerStateCache.cx == cxPerState && m_szPerStateCache.cy == cyPerState)
        return m_pCacheBmp;

    IRenderFactory *pRenderFactory = GETRENDERFACTORY;
    if (!pRenderFactory)
        return NULL;

    // Compute the entire bitmap size (all states laid out like the original SVG)
    int wholeCx, wholeCy;
    if (m_bVertical)
    {
        wholeCx = cxPerState;
        wholeCy = cyPerState * m_nStates;
    }
    else
    {
        wholeCx = cxPerState * m_nStates;
        wholeCy = cyPerState;
    }

    SAutoRefPtr<IRenderTarget> pMemRT;
    if (!pRenderFactory->CreateRenderTarget(&pMemRT, wholeCx, wholeCy))
        return NULL;

    // One-shot: rasterize the ENTIRE SVG to fill the whole bitmap (all states).
    // DrawSVG without prcSrc == draw the full SVG into rcDst.
    CRect rcDst(0, 0, wholeCx, wholeCy);
    pMemRT->BeginDraw();
    pMemRT->DrawSVG(pSvg, &rcDst);
    pMemRT->EndDraw();

    IBitmapS *pNewBmp = (IBitmapS *)pMemRT->GetCurrentObject(OT_BITMAP);
    if (pNewBmp)
    {
        m_pCacheBmp = pNewBmp;
        m_szPerStateCache.cx = cxPerState;
        m_szPerStateCache.cy = cyPerState;
    }
    return m_pCacheBmp;
}

void SSkinImgList::_DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const
{
    if (!GetImage() && !GetSvg())
        return;

    IBitmapS *pImg = GetImage();
    SIZE szSrc; // size of one state in the source image's coordinate space
    bool bFromCache = false;

    if (pImg)
    {
        // Regular bitmap: all states in one image
        szSrc = _GetSkinSize(FALSE);
    }
    else if (GetSvg())
    {
        if (m_bCacheSvg)
        {
            int cxPerState, cyPerState;
            if (m_bTile)
            {
                // Tile mode: cache at scaled skin size (natural tile size)
                SIZE sz = _GetSkinSize(FALSE);
                cxPerState = sz.cx;
                cyPerState = sz.cy;
            }
            else
            {
                // Stretch mode: cache at draw rect size per-state (high precision 1:1)
                cxPerState = rcDraw->right - rcDraw->left;
                cyPerState = rcDraw->bottom - rcDraw->top;
            }
            pImg = GetSvgCacheBitmap(pRT, cxPerState, cyPerState);
            if (pImg)
            {
                szSrc.cx = cxPerState;
                szSrc.cy = cyPerState;
                bFromCache = true;
            }
        }
    }

    if (pImg)
    {
        // Bitmap drawing path. Cache bitmap has the SAME multi-state layout as a regular
        // bitmap, so selecting a state via offset works identically for both.
        RECT rcSrc = { 0, 0, szSrc.cx, szSrc.cy };
        if (m_bVertical)
            OffsetRect(&rcSrc, 0, iState * szSrc.cy);
        else
            OffsetRect(&rcSrc, iState * szSrc.cx, 0);

        if (m_bTile)
        {
            if (m_tileMode == TM_Both)
            {
                SAutoRefPtr<IBrushS> brush, oldBrush;
                pRT->CreateBitmapBrush(pImg, kRepeat_TileMode, kRepeat_TileMode, &brush);
                pRT->SelectObject(brush, (IRenderObj **)&oldBrush);
                pRT->FillRectangle(rcDraw);
                pRT->SelectObject(oldBrush, NULL);
            }
            else
            {
                int wid = m_tileMode != TM_Vert ? szSrc.cx : rcDraw->right - rcDraw->left;
                int hei = m_tileMode != TM_Horz ? szSrc.cy : rcDraw->bottom - rcDraw->top;
                for (int y = rcDraw->top; y < rcDraw->bottom; y += hei)
                {
                    for (int x = rcDraw->left; x < rcDraw->right; x += wid)
                    {
                        RECT rcTile = { x, y, x + wid, y + hei };
                        pRT->DrawBitmapEx(&rcTile, pImg, &rcSrc, GetExpandMode(), byAlpha);
                    }
                }
            }
        }
        else
        {
            pRT->DrawBitmapEx(rcDraw, pImg, &rcSrc, GetExpandMode(), byAlpha);
        }
    }
    else if (GetSvg())
    {
        // Direct SVG drawing (cache disabled or failed)
        SIZE sz = _GetSkinSize(TRUE); // raw SVG coordinates
        RECT rcSrc = { 0, 0, sz.cx, sz.cy };
        if (m_bVertical)
            OffsetRect(&rcSrc, 0, iState * sz.cy);
        else
            OffsetRect(&rcSrc, iState * sz.cx, 0);
        if (m_bTile)
        {
            int wid = m_tileMode != TM_Vert ? sz.cx : rcDraw->right - rcDraw->left;
            int hei = m_tileMode != TM_Horz ? sz.cy : rcDraw->bottom - rcDraw->top;
            for (int y = rcDraw->top; y < rcDraw->bottom; y += hei)
            {
                for (int x = rcDraw->left; x < rcDraw->right; x += wid)
                {
                    RECT rcTile = { x, y, x + wid, y + hei };
                    pRT->DrawSVG(GetSvg(), &rcTile, &rcSrc, byAlpha);
                }
            }
        }
        else
        {
            pRT->DrawSVG(GetSvg(), rcDraw, &rcSrc, byAlpha);
        }
    }
}

UINT SSkinImgList::GetExpandMode() const
{
    if (m_bAutoFit)
        return MAKELONG(EM_STRETCH, m_filterLevel);
    else
        return MAKELONG(EM_NULL, m_filterLevel);
}

HRESULT SSkinImgList::OnAttrSrc(const SStringW &value, BOOL bLoading)
{
    m_strSrc = value;
    if (!bLoading)
    {
        LoadSrcImage();
    }
    return S_OK;
}

bool SSkinImgList::SetImage(IBitmapS *pImg)
{
    m_pImg = pImg;
    m_bLazyLoad = FALSE;
    m_pSvg = NULL;
    m_pCacheBmp = NULL;
    m_szPerStateCache.cx = m_szPerStateCache.cy = 0;
    return true;
}

bool SSkinImgList::SetSvg(ISvgObj *pSvg)
{
    m_pSvg = pSvg;
    m_pImg = NULL;
    m_bLazyLoad = FALSE;
    m_pCacheBmp = NULL;
    m_szPerStateCache.cx = m_szPerStateCache.cy = 0;
    return true;
}

ISvgObj *SSkinImgList::GetSvg() const
{
    if (m_pSvg)
        return m_pSvg;
    if (m_pImg)
        return NULL;
    if (m_bLazyLoad && !m_strSrc.IsEmpty())
    {
        LoadSrcImage();
    }
    return m_pSvg;
}
IBitmapS *SSkinImgList::GetImage() const
{
    if (m_pImg)
        return m_pImg;
    if (m_pSvg)
        return NULL;
    if (m_bLazyLoad && !m_strSrc.IsEmpty())
    {
        LoadSrcImage();
    }
    return m_pImg;
}

void SSkinImgList::OnColorize(COLORREF cr)
{
    if (!m_bEnableColorize)
        return;
    if (cr == m_crColorize)
        return;
    m_crColorize = cr;

    IBitmapS *pImg = GetImage();
    if (pImg)
    {
        if (m_imgBackup)
        { // restore
            LPCVOID pSrc = m_imgBackup->GetPixelBits();
            LPVOID pDst = pImg->LockPixelBits();
            UINT nWid = pImg->Width();
            UINT nHei = pImg->Height();
            if (pSrc && pDst && nWid == m_imgBackup->Width() && nHei == m_imgBackup->Height())
            {
                memcpy(pDst, pSrc, (size_t)nWid * nHei * 4);
                pImg->UnlockPixelBits(pDst);
            }
            else
            {
                if (pDst)
                    pImg->UnlockPixelBits(pDst);
                m_imgBackup = NULL;
                return;
            }
        }
        else
        {
            if (S_OK != pImg->Clone(&m_imgBackup))
                return;
        }

        if (cr != 0)
            SDIBHelper::Colorize(pImg, cr);
        else
            m_imgBackup = NULL; // free backup
    }
    else if (GetSvg())
    {
        m_pCacheBmp = NULL; // reset cache.
        m_szPerStateCache.cx = m_szPerStateCache.cy = 0;
        NSVGimage *pImg = (NSVGimage *)GetSvg()->GetPtr();
        ColorizeSVG(pImg, SDIBHelper::Colorize, cr);
    }
}

void SSkinImgList::_Scale(ISkinObj *skinObj, int nScale)
{
    __baseCls::_Scale(skinObj, nScale);
    SSkinImgList *pRet = sobj_cast<SSkinImgList>(skinObj);
    pRet->m_nStates = m_nStates;
    pRet->m_bTile = m_bTile;
    pRet->m_tileMode = m_tileMode;
    pRet->m_bVertical = m_bVertical;
    pRet->m_filterLevel = m_filterLevel;
    pRet->m_bAutoFit = m_bAutoFit;
    pRet->m_state2Index = m_state2Index;
    pRet->m_bLazyLoad = FALSE;

    if (GetImage())
    {
        CSize szSkin = SSkinImgList::GetSkinSize(); // add SSkinImgList:: to avoid call function implemented in derived class
        szSkin.cx = MulDiv(szSkin.cx, nScale, GetScale());
        szSkin.cy = MulDiv(szSkin.cy, nScale, GetScale());
        if (m_bVertical)
        {
            szSkin.cy *= m_nStates;
        }
        else
        {
            szSkin.cx *= m_nStates;
        }

        if (m_imgBackup)
        {
            m_imgBackup->Scale2(&pRet->m_imgBackup, szSkin.cx, szSkin.cy, kHigh_FilterLevel);
        }
        IBitmapS *pImg = GetImage();
        if (pImg)
        {
            m_pImg->Scale2(&pRet->m_pImg, szSkin.cx, szSkin.cy, kHigh_FilterLevel);
        }
    }
    else if (GetSvg())
    {
        // SVG scaling is handled in DrawSVG method
        pRet->m_pSvg = m_pSvg;
    }
}

///////////////////////////////////////////////////////////////////////
/** SSkinImgCenter */
void SSkinImgCenter::_DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const
{
    if (!GetImage() && !GetSvg())
        return;
    SIZE szSkin = _GetSkinSize(GetSvg() != NULL);
    RECT rcSrc = { 0, 0, szSkin.cx, szSkin.cy };
    if (m_bVertical)
        OffsetRect(&rcSrc, 0, iState * szSkin.cy);
    else
        OffsetRect(&rcSrc, iState * szSkin.cx, 0);

    if (GetImage())
    {
        CRect rcTarget = *rcDraw;
        rcTarget.DeflateRect((rcTarget.Width() - szSkin.cx) / 2, (rcTarget.Height() - szSkin.cy) / 2);
        pRT->DrawBitmapEx(rcTarget, GetImage(), &rcSrc, GetExpandMode(), byAlpha);
    }
    else
    {
        CRect rcTarget = *rcDraw;
        szSkin.cx = MulDiv(szSkin.cx, GetScale(), 100);
        szSkin.cy = MulDiv(szSkin.cy, GetScale(), 100);
        rcTarget.DeflateRect((rcTarget.Width() - szSkin.cx) / 2, (rcTarget.Height() - szSkin.cy) / 2);
        pRT->DrawSVG(GetSvg(), &rcTarget, &rcSrc, byAlpha);
    }
}

///////////////////////////////////////////////////////////////////////
/** SSkinImgFrame */
SSkinImgFrame::SSkinImgFrame()
{
}
void SSkinImgFrame::_DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const
{
    IBitmapS *pImg = GetImage();
    bool bFromCache = false;
    SIZE szPerState;

    if (pImg)
    {
        szPerState = _GetSkinSize(FALSE);
    }
    else if (GetSvg() && m_bCacheSvg)
    {
        // Cache at scaled per-state size (natural corner size for 9-patch).
        // The cache bitmap contains ALL states arranged in the same layout as the SVG.
        szPerState = _GetSkinSize(FALSE);
        pImg = GetSvgCacheBitmap(pRT, szPerState.cx, szPerState.cy);
        if (pImg)
            bFromCache = true;
    }

    if (pImg)
    {
        // Compute source rect for the requested state (works for regular
        // bitmap and cache bitmap equally since both have the same layout).
        CPoint pt;
        if (IsVertical())
            pt.y = szPerState.cy * iState;
        else
            pt.x = szPerState.cx * iState;
        CRect rcSour(pt, szPerState);

        if (bFromCache)
        {
            // Cache bitmap was built at scaled per-state size, so margins
            // must be scaled to match the bitmap's pixel coordinate space.
            int nScale = GetScale();
            CRect rcMargin(MulDiv(m_rcMargin.left, nScale, 100), MulDiv(m_rcMargin.top, nScale, 100), MulDiv(m_rcMargin.right, nScale, 100), MulDiv(m_rcMargin.bottom, nScale, 100));
            pRT->DrawBitmap9Patch(rcDraw, pImg, &rcSour, &rcMargin, GetExpandMode(), byAlpha);
        }
        else
        {
            // Regular bitmap: margins are already in source bitmap pixel space
            pRT->DrawBitmap9Patch(rcDraw, pImg, &rcSour, &m_rcMargin, GetExpandMode(), byAlpha);
        }
    }
    else if (GetSvg())
    {
        // Direct SVG 9-patch (cache disabled or failed)
        SIZE sz = _GetSkinSize(TRUE);
        CPoint pt;
        if (IsVertical())
            pt.y = sz.cy * iState;
        else
            pt.x = sz.cx * iState;
        CRect rcSour(pt, sz);
        DrawSVG9Patch(pRT, GetSvg(), rcDraw, &rcSour, &m_rcMargin, byAlpha, GetScale());
    }
}

UINT SSkinImgFrame::GetExpandMode() const
{
    return MAKELONG(m_bTile ? EM_TILE : EM_STRETCH, m_filterLevel);
}

void SSkinImgFrame::_Scale(ISkinObj *skinObj, int nScale)
{
    SSkinImgList::_Scale(skinObj, nScale);
    SSkinImgFrame *pClone = sobj_cast<SSkinImgFrame>(skinObj);
    int nSrcScale = GetScale();
    if (GetSvg())
    {
        pClone->m_rcMargin = m_rcMargin;
    }
    else
    {
        pClone->m_rcMargin.left = MulDiv(m_rcMargin.left, nScale, nSrcScale);
        pClone->m_rcMargin.top = MulDiv(m_rcMargin.top, nScale, nSrcScale);
        pClone->m_rcMargin.right = MulDiv(m_rcMargin.right, nScale, nSrcScale);
        pClone->m_rcMargin.bottom = MulDiv(m_rcMargin.bottom, nScale, nSrcScale);
    }
}

///////////////////////////////////////////////////////////////////////
/** SSkinButton */
SSkinButton::SSkinButton()
    : m_nCornerRadius(2)
    , m_fCornerPercent(0.0)
{
    m_colors.m_crBorder[0] = RGB(0x70, 0x70, 0x70);
    m_colors.m_crBorder[1] = CR_INVALID; // Does not change the original effect
    m_colors.m_crBorder[2] = CR_INVALID;
    m_colors.m_crBorder[3] = CR_INVALID;

    m_colors.m_crUp[0] = (RGB(0xEE, 0xEE, 0xEE));
    m_colors.m_crDown[0] = (RGB(0xD6, 0xD6, 0xD6));
    m_colors.m_crUp[1] = (RGB(0xEE, 0xEE, 0xEE));
    m_colors.m_crDown[1] = (RGB(0xE0, 0xE0, 0xE0));
    m_colors.m_crUp[2] = (RGB(0xCE, 0xCE, 0xCE));
    m_colors.m_crDown[2] = (RGB(0xC0, 0xC0, 0xC0));
    m_colors.m_crUp[3] = (RGB(0x8E, 0x8E, 0x8E));
    m_colors.m_crDown[3] = (RGB(0x80, 0x80, 0x80));
}

void SSkinButton::_DrawByIndex(IRenderTarget *pRT, LPCRECT prcDraw, int iState, BYTE byAlpha) const
{
    int nCorner = m_nCornerRadius;
    if (m_fCornerPercent > 0.0)
    {
        int nW = prcDraw->right - prcDraw->left;
        int nH = prcDraw->bottom - prcDraw->top;
        nCorner = (nW < nH) ? (int)(nW * m_fCornerPercent) : (int)(nH * m_fCornerPercent);
    }
    // This is only needed when a gradient is required
    if (m_colors.m_crUp[iState] != m_colors.m_crDown[iState])
    {
        CRect rcDraw = *prcDraw;
        rcDraw.DeflateRect(1, 1);
        CPoint ptCorner(nCorner, nCorner);
        GradientItem gradients[2] = { m_colors.m_crUp[iState], 0.0f, m_colors.m_crDown[iState], 1.0f };
        pRT->DrawGradientRect(rcDraw, TRUE, ptCorner, gradients, 2, byAlpha);
    }
    else
    {
        SColor cr(m_colors.m_crDown[iState]);
        cr.updateAlpha(byAlpha);
        if (nCorner > 0)
            pRT->FillSolidRoundRect(prcDraw, CPoint(nCorner, nCorner), cr.toCOLORREF());
        else
            pRT->FillSolidRect(prcDraw, cr.toCOLORREF());
    }

    if (CR_INVALID == m_colors.m_crBorder[iState]) // Does not change the original effect
        iState = 0;
    // Draw border
    SAutoRefPtr<IPenS> pPen, pOldPen;
    pRT->CreatePen(PS_SOLID, m_colors.m_crBorder[iState], 1, &pPen);
    pRT->SelectObject(pPen, (IRenderObj **)&pOldPen);
    pRT->DrawRoundRect(prcDraw, CPoint(nCorner, nCorner));
    pRT->SelectObject(pOldPen, NULL);
}

int SSkinButton::GetStates() const
{
    return 4;
}

void SSkinButton::SetColors(COLORREF crUp[4], COLORREF crDown[4], COLORREF crBorder[4])
{
    memcpy(m_colors.m_crUp, crUp, 4 * sizeof(COLORREF));
    memcpy(m_colors.m_crDown, crDown, 4 * sizeof(COLORREF));
    memcpy(m_colors.m_crBorder, crBorder, 4 * sizeof(COLORREF));
}

void SSkinButton::OnColorize(COLORREF cr)
{
    if (!m_bEnableColorize)
        return;
    if (m_crColorize == cr)
        return;
    if (cr == 0)
    {
        memcpy(&m_colors, &m_colorsBackup, sizeof(BTNCOLORS));
        m_crColorize = 0;
    }
    else
    {
        if (m_crColorize != 0)
        { // Get data from the backup
            memcpy(&m_colors, &m_colorsBackup, sizeof(BTNCOLORS));
        }
        else
        { // Back up the data
            memcpy(&m_colorsBackup, &m_colors, sizeof(BTNCOLORS));
        }
        m_crColorize = cr;

        // Adjust color values
        for (int i = 0; i < 4; i++)
        {
            SDIBHelper::Colorize(m_colors.m_crBorder[i], m_crColorize);
            SDIBHelper::Colorize(m_colors.m_crDown[i], m_crColorize);
            SDIBHelper::Colorize(m_colors.m_crUp[i], m_crColorize);
        }
    }
}

ISkinObj *SSkinButton::Scale(int nScale)
{
    return NULL;
}

///////////////////////////////////////////////////////////////////////
/** SSkinGradation */
SSkinGradation::SSkinGradation()
    : m_bVert(TRUE)
    , m_crFrom(CR_INVALID)
    , m_crTo(CR_INVALID)
{
}

void SSkinGradation::_DrawByIndex(IRenderTarget *pRT, LPCRECT prcDraw, int iState, BYTE byAlpha) const
{
    GradientItem gradients[2] = { m_crFrom, 0.0f, m_crTo, 1.0f };
    pRT->DrawGradientRect(prcDraw, m_bVert, CPoint(), gradients, 2, byAlpha);
}

ISkinObj *SSkinGradation::Scale(int nScale)
{
    return NULL;
}

///////////////////////////////////////////////////////////////////////
SGradientDesc::SGradientDesc()
{
    // set default to linear gradient
    m_type = linear;
    m_angle = 0.0f;
    m_centerX = m_centerY = 0.5f;
    m_radius.setInvalid();
    m_ratio_radius = 0.5f;
    m_startAngle = 0.0f;
    m_gradient.Attach(new SGradient());
}

GradientInfo SGradientDesc::GetGradientInfo(int nScale, int wid, int hei) const
{
    GradientInfo ret;
    ret.type = m_type;
    switch (ret.type)
    {
    case linear:
        ret.angle = m_angle;
        break;
    case radial:
        if (m_radius.isValid())
            ret.radial.radius = (float)m_radius.toPixelSize(nScale);
        else
            ret.radial.radius = m_ratio_radius * smax(wid, hei);
        ret.sweep.centerX = m_centerX;
        ret.sweep.centerY = m_centerY;
        break;
    case sweep:
        ret.sweep.centerX = m_centerX;
        ret.sweep.centerY = m_centerY;
        ret.sweep.startAngle = m_startAngle;
        break;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
SSkinGradation2::SSkinGradation2()
{
    m_ptCorner = SPoint::Make(0.f, 0.f);
    m_szCorner[0].setInvalid();
    m_szCorner[1].setInvalid();
    m_bEnableScale = false;
    m_gradient.Attach(new SGradient());
}

void SSkinGradation2::_DrawByIndex(IRenderTarget *pRT, LPCRECT prcDraw, int iState, BYTE byAlpha) const
{
    CRect rc(prcDraw);
    CPoint ptCorner;
    if (m_szCorner[0].isValid() && m_szCorner[1].isValid())
    {
        ptCorner.x = m_szCorner[0].toPixelSize(GetScale());
        ptCorner.y = m_szCorner[1].toPixelSize(GetScale());
    }
    else
    {
        ptCorner.x = (int)(rc.Width() / 2 * m_ptCorner.fX);
        ptCorner.y = (int)(rc.Height() / 2 * m_ptCorner.fY);
    }
    GradientInfo info = GetGradientInfo(GetScale(), prcDraw->right - prcDraw->left, prcDraw->bottom - prcDraw->top);
    pRT->DrawGradientRectEx(prcDraw, ptCorner, m_gradient->GetGradientData(), m_gradient->GetGradientLength(), &info, GetAlpha());
}

ISkinObj *SSkinGradation2::Scale(int nScale)
{
    return NULL;
}

void SSkinGradation2::OnInitFinished(THIS_ IXmlNode *xmlNode)
{
    m_gradient->OnInitFinished(xmlNode);
}

///////////////////////////////////////////////////////////////////////
/** SSkinScrollbar */
SSkinScrollbar::SSkinScrollbar()
    : m_nMargin(0)
    , m_bHasGripper(FALSE)
    , m_bHasInactive(FALSE)
{
}

CRect SSkinScrollbar::GetPartRect(int nSbCode, int nState, BOOL bVertical) const
{
    CSize sz = GetSkinSize();
    if (GetSvg() && GetScale() != 100)
    {
        // restore sz to original size
        sz.cx = MulDiv(sz.cx, 100, GetScale());
        sz.cy = MulDiv(sz.cy, 100, GetScale());
    }
    CSize szFrame(sz.cx / 9, sz.cx / 9);
    if (nSbCode == SB_CORNOR)
    {
        return CRect(CPoint(szFrame.cx * 8, 0), szFrame);
    }
    else if (nSbCode == SB_THUMBGRIPPER)
    {
        return CRect(CPoint(szFrame.cx * 8, (1 + (bVertical ? 0 : 1)) * szFrame.cy), szFrame);
    }
    else
    {
        if (nState == SBST_INACTIVE && !m_bHasInactive)
        {
            nState = SBST_NORMAL;
        }
        CRect rcRet;
        int iPart = -1;
        switch (nSbCode)
        {
        case SB_LINEUP:
            iPart = 0;
            break;
        case SB_LINEDOWN:
            iPart = 1;
            break;
        case SB_THUMBTRACK:
            iPart = 2;
            break;
        case SB_PAGEUP:
        case SB_PAGEDOWN:
            iPart = 3;
            break;
        }
        if (!bVertical)
            iPart += 4;

        return CRect(CPoint(szFrame.cx * iPart, szFrame.cy * nState), szFrame);
    }
}

void SSkinScrollbar::_DrawByState(IRenderTarget *pRT, LPCRECT prcDraw, DWORD dwState, BYTE byAlpha) const
{
    if (!GetImage() && !GetSvg())
        return;
    int nSbCode = LOWORD(dwState);
    int nState = LOBYTE(HIWORD(dwState));
    BOOL bVertical = HIBYTE(HIWORD(dwState));
    CRect rcMargin(0, 0, 0, 0);
    if (bVertical)
        rcMargin.top = m_nMargin, rcMargin.bottom = m_nMargin;
    else
        rcMargin.left = m_nMargin, rcMargin.right = m_nMargin;

    CRect rcSour = GetPartRect(nSbCode, nState, bVertical);

    if (GetImage())
    {
        pRT->DrawBitmap9Patch(prcDraw, GetImage(), &rcSour, &rcMargin, m_bTile ? EM_TILE : EM_STRETCH, byAlpha);

        if (nSbCode == SB_THUMBTRACK && m_bHasGripper)
        {
            rcSour = GetPartRect(SB_THUMBGRIPPER, 0, bVertical);
            CRect rcDraw = *prcDraw;

            if (bVertical)
                rcDraw.top += (rcDraw.Height() - rcSour.Height()) / 2, rcDraw.bottom = rcDraw.top + rcSour.Height();
            else
                rcDraw.left += (rcDraw.Width() - rcSour.Width()) / 2, rcDraw.right = rcDraw.left + rcSour.Width();
            pRT->DrawBitmap9Patch(&rcDraw, GetImage(), &rcSour, &rcMargin, m_bTile ? EM_TILE : EM_STRETCH, byAlpha);
        }
    }
    else
    {
        // SVG supports 9-patch now
        DrawSVG9Patch(pRT, GetSvg(), prcDraw, &rcSour, &rcMargin, byAlpha, GetScale());

        if (nSbCode == SB_THUMBTRACK && m_bHasGripper)
        {
            rcSour = GetPartRect(SB_THUMBGRIPPER, 0, bVertical);
            CRect rcDraw = *prcDraw;

            if (bVertical)
                rcDraw.top += (rcDraw.Height() - rcSour.Height()) / 2, rcDraw.bottom = rcDraw.top + rcSour.Height();
            else
                rcDraw.left += (rcDraw.Width() - rcSour.Width()) / 2, rcDraw.right = rcDraw.left + rcSour.Width();
            DrawSVG9Patch(pRT, GetSvg(), &rcDraw, &rcSour, &rcMargin, byAlpha, GetScale());
        }
    }
}

void SSkinScrollbar::_Scale(ISkinObj *skinObj, int nScale)
{
    __baseCls::_Scale(skinObj, nScale);

    SSkinScrollbar *pRet = sobj_cast<SSkinScrollbar>(skinObj);
    pRet->m_nMargin = MulDiv(m_nMargin, nScale, GetScale());
    pRet->m_bHasInactive = m_bHasInactive;
    pRet->m_bHasGripper = m_bHasGripper;
}

int SSkinScrollbar::GetIdealSize() const
{
    if (GetImage())
        return GetImage()->Width() / 9;
    else if (GetSvg())
    {
        int ret = GetSvg()->GetWidth() / 9;
        return ret * GetScale() / 100;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////
/** SSkinColor */
SSkinColorRect::SSkinColorRect()
    : m_nRadius(0)
    , m_fCornerPercent(0.0)
    , m_nBorderWidth(0)
{
    m_crStates[0] = RGBA(255, 255, 255, 255);
    m_crStates[1] = CR_INVALID;
    m_crStates[2] = CR_INVALID;
    m_crStates[3] = CR_INVALID;
    m_crBorders[0] = m_crBorders[1] = m_crBorders[2] = m_crBorders[3] = CR_INVALID;
}

SSkinColorRect::~SSkinColorRect()
{
}

void SSkinColorRect::_DrawByIndex(IRenderTarget *pRT, LPCRECT prcDraw, int iState, BYTE byAlpha) const
{
    if (iState > 3)
        return;

    int nCorner = m_nRadius;
    if (m_fCornerPercent > 0.0)
    {
        int nW = prcDraw->right - prcDraw->left;
        int nH = prcDraw->bottom - prcDraw->top;
        nCorner = (nW < nH) ? (int)(nW * m_fCornerPercent) : (int)(nH * m_fCornerPercent);
    }
    int iBgColor = iState;
    if (m_crStates[iBgColor] == CR_INVALID)
        iBgColor = 0;
    if (m_crStates[iBgColor] != CR_INVALID)
    {
        SColor cr(m_crStates[iBgColor]);
        cr.updateAlpha(byAlpha);
        if (nCorner > 0)
            pRT->FillSolidRoundRect(prcDraw, CPoint(nCorner, nCorner), cr.toCOLORREF());
        else
            pRT->FillSolidRect(prcDraw, cr.toCOLORREF());
    }
    int iBorderColor = iState;
    if (m_crBorders[iBorderColor] == CR_INVALID)
        iBorderColor = 0;
    if (m_crBorders[iBorderColor] != CR_INVALID && m_nBorderWidth > 0)
    {
        SAutoRefPtr<IPenS> pen, oldPen;
        pRT->CreatePen(PS_SOLID, m_crBorders[iBorderColor], m_nBorderWidth, (IPenS **)&pen);
        pRT->SelectObject(pen, (IRenderObj **)&oldPen);
        if (nCorner > 0)
            pRT->DrawRoundRect(prcDraw, CPoint(nCorner, nCorner));
        else
            pRT->DrawRectangle(prcDraw);
        pRT->SelectObject(oldPen, NULL);
    }
}

int SSkinColorRect::GetStates() const
{
    int nStates = 4;
    for (int i = 3; i >= 0; i--)
    {
        if (m_crStates[i] == CR_INVALID)
            nStates--;
        else
            break;
    }
    return nStates;
}

ISkinObj *SSkinColorRect::Scale(int nScale)
{
    return NULL;
}

///////////////////////////////////////////////////////////////////////

SSkinShape::SSkinShape()
    : m_shape(rectangle)
    , m_bEnableCache(FALSE)
{
    m_cacheSize.cx = m_cacheSize.cy = 0;
}

void SSkinShape::OnInitFinished(IXmlNode *pNode)
{
    __baseCls::OnInitFinished(pNode);
    SXmlNode xmlNode(pNode);
    SXmlNode xmlSolid = xmlNode.child(SShapeSolid::GetClassName());
    if (xmlSolid)
    {
        if (!m_solid)
            m_solid.Attach(new SShapeSolid());
        m_solid->InitFromXml(&xmlSolid);
    }
    SXmlNode xmlBitmap = xmlNode.child(SShapeBitmap::GetClassName());
    if (xmlBitmap)
    {
        if (!m_bitmap)
            m_bitmap.Attach(new SShapeBitmap());
        m_bitmap->InitFromXml(&xmlBitmap);
    }

    SXmlNode xmlGrident = xmlNode.child(SGradientBrush::GetClassName());
    if (xmlGrident)
    {
        if (!m_gradient)
            m_gradient.Attach(new SGradientBrush());
        m_gradient->InitFromXml(&xmlGrident);
    }
    SXmlNode xmlSize = xmlNode.child(SShapeSize::GetClassName());
    if (xmlSize)
    {
        if (!m_shapeSize)
            m_shapeSize.Attach(new SShapeSize());
        m_shapeSize->InitFromXml(&xmlSize);
    }
    SXmlNode xmlStoke = xmlNode.child(SStroke::GetClassName());
    if (xmlStoke)
    {
        if (!m_stroke)
            m_stroke.Attach(new SStroke());
        m_stroke->InitFromXml(&xmlStoke);
    }
    SXmlNode xmlConner = xmlNode.child(SCornerSize::GetClassName());
    if (xmlConner)
    {
        if (!m_cornerSize)
            m_cornerSize.Attach(new SCornerSize());
        m_cornerSize->InitFromXml(&xmlConner);
    }
    SXmlNode xmlRatioConner = xmlNode.child(SRatioCornerSize::GetClassName());
    if (xmlRatioConner)
    {
        if (!m_ratioCornerSize)
            m_ratioCornerSize.Attach(new SRatioCornerSize());
        m_ratioCornerSize->InitFromXml(&xmlRatioConner);
    }

    SXmlNode xmlRing = xmlNode.child(SShapeRing::GetClassName());
    if (xmlRing)
    {
        if (!m_ringParam)
            m_ringParam.Attach(new SShapeRing());
        m_ringParam->InitFromXml(&xmlRing);
    }

    SXmlNode xmlPolygon = xmlNode.child(SShapePolygon::GetClassName());
    if (xmlPolygon)
    {
        if (!m_polygon)
            m_polygon.Attach(new SShapePolygon());
        m_polygon->InitFromXml(&xmlPolygon);
    }

    SXmlNode xmlShadow = xmlNode.child(SShapeShadow::GetClassName());
    if (xmlShadow)
    {
        if (!m_shadow)
            m_shadow.Attach(new SShapeShadow());
        m_shadow->InitFromXml(&xmlShadow);
    }

    SXmlNode xmlBlur = xmlNode.child(SShapeBlur::GetClassName());
    if (xmlBlur)
    {
        if (!m_blur)
            m_blur.Attach(new SShapeBlur());
        m_blur->InitFromXml(&xmlBlur);
    }
}

void SSkinShape::SShapeShadow::OnInitFinished(IXmlNode *pNode)
{
    if (m_color != CR_INVALID)
    {
        GETRENDERFACTORY->CreateImageFilter(IID_IDropShadowImageFilter, (IImageFilter **)&m_filter);
        if (m_filter)
        {
            IDropShadowImageFilter *pShadowFilter = (IDropShadowImageFilter *)(IImageFilter *)m_filter;
            pShadowFilter->Init(m_dx, m_dy, m_sigmaX, m_sigmaY, m_color);
        }
    }
}
void SSkinShape::SShapeBlur::OnInitFinished(IXmlNode *pNode)
{

    GETRENDERFACTORY->CreateImageFilter(IID_IBlurImageFilter, (IImageFilter **)&m_filter);
    if (m_filter)
    {
        IBlurImageFilter *pBlurFilter = (IBlurImageFilter *)(IImageFilter *)m_filter;
        pBlurFilter->Init(m_sigmaX, m_sigmaY, NULL);
    }
}

void SSkinShape::_Scale(ISkinObj *pObj, int nScale)
{
    __baseCls::_Scale(pObj, nScale);

    SSkinShape *pRet = sobj_cast<SSkinShape>(pObj);
    SASSERT(pRet);
    pRet->m_solid = m_solid;
    pRet->m_bitmap = m_bitmap;
    pRet->m_gradient = m_gradient;
    pRet->m_shape = m_shape;
    pRet->m_shapeSize = m_shapeSize;
    pRet->m_cornerSize = m_cornerSize;
    pRet->m_stroke = m_stroke;
    pRet->m_ringParam = m_ringParam;
    pRet->m_shadow = m_shadow;
    pRet->m_blur = m_blur;
    pRet->m_polygon = m_polygon;
    pRet->m_bEnableCache = m_bEnableCache;
    pRet->m_cacheBitmap = NULL;
    pRet->m_cacheSize.cx = pRet->m_cacheSize.cy = 0;
}

SIZE SSkinShape::GetSkinSize() const
{
    if (!m_shapeSize)
        return CSize();

    return CSize(m_shapeSize->m_width.toPixelSize(GetScale()), m_shapeSize->m_height.toPixelSize(GetScale()));
}

void SSkinShape::_DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const
{
    int nWidth = rcDraw->right - rcDraw->left;
    int nHeight = rcDraw->bottom - rcDraw->top;

    if (nWidth <= 0 || nHeight <= 0)
        return;

    if (m_bEnableCache)
    {
        if (!m_cacheBitmap || (m_cacheSize.cx != nWidth || m_cacheSize.cy != nHeight))
        {
            m_cacheBitmap = NULL;
            RECT rcMem = { 0, 0, nWidth, nHeight };
            IRenderFactory *pRenderFactory = GETRENDERFACTORY;
            if (pRenderFactory)
            {
                SAutoRefPtr<IRenderTarget> pMemRT;
                if (pRenderFactory->CreateRenderTarget(&pMemRT, nWidth, nHeight))
                {
                    pMemRT->BeginDraw();
                    pMemRT->ClearRect(&rcMem, RGBA(0, 0, 0, 0));
                    DrawShapeInternal(pMemRT, &rcMem, byAlpha);
                    pMemRT->EndDraw();
                    IBitmapS *pNewBitmap = (IBitmapS *)pMemRT->GetCurrentObject(OT_BITMAP);
                    if (pNewBitmap)
                    {
                        m_cacheBitmap = pNewBitmap;
                        m_cacheSize.cx = nWidth;
                        m_cacheSize.cy = nHeight;
                    }
                }
            }
        }
        if (m_cacheBitmap)
        {
            RECT rcSrc = { 0, 0, nWidth, nHeight };
            pRT->DrawBitmapEx(rcDraw, m_cacheBitmap, &rcSrc, MAKELONG(EM_NULL, kLow_FilterLevel), byAlpha);
        }
    }
    else
    {
        DrawShapeInternal(pRT, rcDraw, byAlpha);
    }
}

void SSkinShape::DrawShapeInternal(IRenderTarget *pRT, LPCRECT rcDraw, BYTE byAlpha) const
{
    CRect rcDest(rcDraw);
    SAutoRefPtr<IBrushS> pBrush, oldBrush;
    if (m_solid)
    {
        pBrush.Attach(m_solid->CreateBrush(pRT, byAlpha));
    }
    else if (m_gradient != NULL)
    {
        pBrush.Attach(m_gradient->CreateBrush(pRT, GetScale(), byAlpha, rcDraw->right - rcDraw->left, rcDraw->bottom - rcDraw->top));
    }
    else if (m_bitmap)
    {
        pBrush.Attach(m_bitmap->CreateBrush(pRT, byAlpha));
    }

    SAutoRefPtr<IPenS> pPen, oldPen;
    if (m_stroke != NULL)
    {
        int nPenWidth = m_stroke->m_width.toPixelSize(GetScale());
        SColor color(m_stroke->m_color, byAlpha);
        pRT->CreatePen(m_stroke->GetStyle(), color.toCOLORREF(), nPenWidth, &pPen);
        rcDest.DeflateRect(nPenWidth / 2, nPenWidth / 2);
    }

    CPoint ptCorner = GetCornerSize(rcDest);
    POINT pts[SShapePolygon::MAX_POINTS + 1];
    IImageFilter *pOldImageFilter = pRT->GetImageFilter();
    IImageFilter *pShadowFilter = NULL;
    IImageFilter *pBlurFilter = NULL;
    if (m_shadow)
        pShadowFilter = m_shadow->GetFilter();
    if (m_blur)
        pBlurFilter = m_blur->GetFilter();
    if (pShadowFilter || pBlurFilter)
    {
        if (pBlurFilter && pBlurFilter)
        {
            SAutoRefPtr<IComposeImageFilter> pFilterCompose;
            GETRENDERFACTORY->CreateImageFilter(IID_IComposeImageFilter, (IImageFilter **)&pFilterCompose);
            pFilterCompose->Init(pShadowFilter, pBlurFilter);
            pRT->SetImageFilter(pFilterCompose);
        }
        else if (pBlurFilter)
        {
            pRT->SetImageFilter(pBlurFilter);
        }
        else
        {
            pRT->SetImageFilter(pShadowFilter);
        }
    }
    if (pBrush)
    {
        pRT->SelectObject(pBrush, (IRenderObj **)&oldBrush);
        switch (m_shape)
        {
        case rectangle:
            if (ptCorner.x != 0 || ptCorner.y != 0)
                pRT->FillRoundRect(&rcDest, ptCorner);
            else
                pRT->FillRectangle(&rcDest);
            break;
        case oval:
            pRT->FillEllipse(&rcDest);
            break;
        case ring: // do nothing
            break;
        case polygon:
            if (m_polygon)
            {
                int wid = rcDraw->right - rcDraw->left;
                int hei = rcDraw->bottom - rcDraw->top;
                for (int i = 0; i < m_polygon->GetPoints(); i++)
                {
                    pts[i].x = (int)(m_polygon->m_points[i].fX * wid) + rcDraw->left;
                    pts[i].y = (int)(m_polygon->m_points[i].fY * hei) + rcDraw->top;
                }
                if (m_polygon->IsClosed())
                    pRT->FillPolygon(pts, m_polygon->GetPoints());
            }
            break;
        }
        pRT->SelectObject(oldBrush, NULL);
    }
    pRT->SetImageFilter(pOldImageFilter);
    if (pPen)
    {
        pRT->SelectObject(pPen, (IRenderObj **)&oldPen);
        ptCorner = GetCornerSize(rcDest);
        switch (m_shape)
        {
        case rectangle:
            if (ptCorner.x != 0 || ptCorner.y != 0)
                pRT->DrawRoundRect(&rcDest, ptCorner);
            else
                pRT->DrawRectangle(&rcDest);
            break;
        case oval:
            pRT->DrawEllipse(&rcDest);
            break;
        case ring:
        {
            POINT ptCenter = { (rcDest.left + rcDest.right) / 2, (rcDest.top + rcDest.bottom) / 2 };
            int nRadius = smin(rcDest.right - rcDest.left, rcDest.bottom - rcDest.top) / 2;
            RECT rcRing = { ptCenter.x - nRadius, ptCenter.y - nRadius, ptCenter.x + nRadius, ptCenter.y + nRadius };
            float startAngle = 0;
            float endAngle = 360;
            if (m_ringParam)
            {
                startAngle = m_ringParam->m_startAngle;
                endAngle = m_ringParam->m_sweepAngle;
            }
            pRT->DrawArc(&rcRing, startAngle, endAngle, false);
        }
        break;
        case polygon:
            if (m_polygon)
            {
                int ptCount = m_polygon->GetPoints();
                if (m_polygon->IsClosed())
                {
                    pts[ptCount++] = pts[0];
                }
                pRT->DrawPolygon(pts, ptCount);
            }
            break;
        }
        pRT->SelectObject(oldPen, NULL);
    }
}

int SSkinShape::GetStates() const
{
    return 1;
}

POINT SSkinShape::GetCornerSize(const CRect &rc) const
{
    if (m_cornerSize)
        return m_cornerSize->GetConner(GetScale());
    if (m_ratioCornerSize)
        return m_ratioCornerSize->GetConner(rc);
    return CPoint();
}

SSkinShape::SStroke::SStroke()
    : m_color(CR_INVALID)
    , m_style(PS_SOLID)
    , m_endStyle(0)
    , m_joinStyle(0)
{
}

int SSkinShape::SStroke::GetStyle() const
{
    int ret = m_style | m_endStyle | m_joinStyle;
    if (m_endStyle || m_joinStyle)
    {
        ret |= PS_GEOMETRIC;
    }
    return ret;
}

IBrushS *SSkinShape::SShapeSolid::CreateBrush(IRenderTarget *pRT, BYTE byAlpha)
{
    if (m_crSolid == CR_INVALID)
        return NULL;
    SColor color(m_crSolid, byAlpha);
    IBrushS *ret = NULL;
    pRT->CreateSolidColorBrush(color.toCOLORREF(), &ret);
    return ret;
}

IBrushS *SSkinShape::SShapeBitmap::CreateBrush(IRenderTarget *pRT, BYTE byAlpha)
{
    if (!m_pImg)
        return NULL;
    IBrushS *ret = NULL;
    pRT->CreateBitmapBrush(m_pImg, m_tileX, m_tileY, &ret);
    return ret;
}

IBrushS *SSkinShape::SGradientBrush::CreateBrush(IRenderTarget *pRT, int nScale, BYTE byAlpha, int wid, int hei) const
{
    if (m_gradient->GetGradientLength() < 2)
        return NULL;
    IBrushS *ret = NULL;
    GradientInfo info = GetGradientInfo(nScale, wid, hei);
    pRT->CreateGradientBrush(m_gradient->GetGradientData(), m_gradient->GetGradientLength(), &info, byAlpha, kRepeat_TileMode, &ret);
    return ret;
}

void SSkinShape::SGradientBrush::OnInitFinished(THIS_ IXmlNode *xmlNode)
{
    m_gradient->OnInitFinished(xmlNode);
}

HRESULT SSkinShape::SCornerSize::OnAttrRadius(const SStringW strValue, BOOL bLoading)
{
    SStringWList values;
    size_t nValues = SplitString(strValue, L',', values);
    if (nValues == 1)
    {
        m_radiusX = m_radiusY = GETLAYOUTSIZE(values[0]);
        return S_OK;
    }
    else if (nValues == 2)
    {
        m_radiusX = GETLAYOUTSIZE(values[0]);
        m_radiusY = GETLAYOUTSIZE(values[1]);
        return S_OK;
    }
    return E_INVALIDARG;
}

HRESULT SSkinShape::SRatioCornerSize::OnAttrRadius(const SStringW strValue, BOOL bLoading)
{
    SStringWList values;
    size_t nValues = SplitString(strValue, L',', values);
    if (nValues == 1)
    {
        swscanf_s(strValue.c_str(), L"%f", &m_radius.fX);
        m_radius.fY = m_radius.fX;
        return S_OK;
    }
    else if (nValues == 2)
    {
        swscanf_s(values[0].c_str(), L"%f", &m_radius.fX);
        swscanf_s(values[1].c_str(), L"%f", &m_radius.fY);
        return S_OK;
    }
    return E_INVALIDARG;
}

void SSkinShape::SShapePolygon::OnInitFinished(IXmlNode *pNode)
{
    SXmlNode xmlNode(pNode);
    SXmlNode xmlPoint = xmlNode.child(L"point");
    int i = 0;
    while (xmlPoint)
    {
        SPoint pt;
        SStringW value = xmlPoint.attribute(L"value").as_string();
        if (swscanf_s(value.c_str(), L"%f,%f", &pt.fX, &pt.fY) == 2)
        {
            m_points[i++] = pt;
        }
        xmlPoint = xmlPoint.next_sibling(L"point");
    }
    m_ptCount = i;
}

///////////////////////////////////////////////////////////////////////
/** SSKinGroup */
int SSKinGroup::GetStates() const
{
    return 4;
}

void SSKinGroup::_DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const
{
    if ((int)iState >= GetStates())
        return;
    ISkinObj *pSkin = m_skins[iState];
    if (!pSkin)
        return;
    pSkin->DrawByIndex2(pRT, rcDraw, 0, byAlpha);
}

SIZE SSKinGroup::GetSkinSize() const
{
    for (int i = 0; i < GetStates(); i++)
    {
        if (m_skins[i])
            return m_skins[i]->GetSkinSize();
    }
    return CSize();
}

void SSKinGroup::_Scale(ISkinObj *skinObj, int nScale)
{
    __baseCls::_Scale(skinObj, nScale);
    SSKinGroup *pRet = sobj_cast<SSKinGroup>(skinObj);
    for (int i = 0; i < ARRAYSIZE(m_skins); i++)
    {
        if (!m_skins[i])
            continue;
        pRet->m_skins[i].Attach(m_skins[i]->Scale(nScale));
    }
}

///////////////////////////////////////////////////////////////////////
/** SSKinGroup2 */

int SSKinGroup2::GetStates() const
{
    return (int)m_skins.GetCount();
}

void SSKinGroup2::OnInitFinished(IXmlNode *pNode)
{
    __baseCls::OnInitFinished(pNode);
    SXmlNode xmlNode(pNode);
    SXmlNode xmlItems = xmlNode.child(L"items");
    if (xmlItems)
    {
        SXmlNode xmlItem = xmlItems.first_child();
        while (xmlItem)
        {
            ISkinObj *pSkin = SApplication::getSingleton().CreateSkinByName(xmlItem.name());
            if (pSkin)
            {
                pSkin->InitFromXml(&xmlItem);
                m_skins.Add(pSkin);
                pSkin->Release();
            }
            xmlItem = xmlItem.next_sibling();
        }
    }
}

void SSKinGroup2::_DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const
{
    if (iState < 0 || iState >= (int)m_skins.GetCount())
        return;
    ISkinObj *pSkin = m_skins[iState];
    if (!pSkin)
        return;
    pSkin->DrawByIndex2(pRT, rcDraw, 0, byAlpha);
}

SIZE SSKinGroup2::GetSkinSize() const
{
    for (UINT i = 0; i < m_skins.GetCount(); i++)
    {
        if (m_skins[i])
            return m_skins[i]->GetSkinSize();
    }
    return CSize();
}

void SSKinGroup2::_Scale(ISkinObj *skinObj, int nScale)
{
    __baseCls::_Scale(skinObj, nScale);
    SSKinGroup2 *pRet = sobj_cast<SSKinGroup2>(skinObj);
    for (UINT i = 0; i < m_skins.GetCount(); i++)
    {
        if (!m_skins[i])
            continue;
        SAutoRefPtr<ISkinObj> skinPtr;
        skinPtr.Attach(m_skins[i]->Scale(nScale));
        pRet->m_skins.Add(skinPtr);
    }
}

/** The memory in IBitmap is in RGBA format; in .9, the alpha channel == 0 or 255 is used to determine how to stretch */
HRESULT SSkinImgFrame2::OnAttrSrc(const SStringW &strValue, BOOL bLoading)
{
    IBitmapS *pImg = LOADIMAGE2(strValue);
    if (!pImg)
        return E_FAIL;
    int nWid = pImg->Width();
    int nHei = pImg->Height();
    if (nWid < 2 || nHei < 2)
        return E_INVALIDARG;
    LPBYTE pBuf = (LPBYTE)pImg->LockPixelBits();

    int left, right, top, bottom;
    // Detect the left,right positions defined in the first scan line
    LPBYTE p = pBuf;
    int i = 1;
    while (i < nWid - 1 && p[3] == 0)
        i++, p += 4;
    left = i - 1;

    while (i < nWid - 1 && p[3] != 0)
        i++, p += 4;
    right = i - 1;

    // Detect the top,bottom positions defined in the first column
    i = 1;
    p = pBuf + (nWid * 4);
    while (i < nHei - 1 && p[3] == 0)
        i++, p += nWid * 4;
    top = i - 1;
    while (i < nHei - 1 && p[3] != 0)
        i++, p += nWid * 4;
    bottom = i - 1;

    m_rcMargin.left = left;
    m_rcMargin.right = nWid - 2 - right;
    m_rcMargin.top = top;
    m_rcMargin.bottom = nHei - 2 - bottom;

    HRESULT hRet = S_OK;
    IBitmapS *pImgCenter = NULL;
    if (!pImg->GetRenderFactory()->CreateBitmap(&pImgCenter))
    {
        return E_OUTOFMEMORY;
    }
    hRet = pImgCenter->Init(nWid - 2, nHei - 2, NULL);
    if (hRet != S_OK)
    {
        return hRet;
    }
    LPBYTE pBuf2 = (LPBYTE)pImgCenter->LockPixelBits();

    LPBYTE pSrc = pBuf + (nWid * 4);
    LPBYTE pDst = pBuf2;

    i = 1;
    pSrc += 4;
    while (i < nHei - 1)
    {
        memcpy(pDst, pSrc, (nWid - 2) * 4);
        pDst += (nWid - 2) * 4;
        pSrc += nWid * 4;
        i++;
    }

    pImg->UnlockPixelBits(pBuf);
    pImgCenter->UnlockPixelBits(pBuf2);

    SetImage(pImgCenter);
    pImgCenter->Release();
    pImg->Release();

    return hRet;
}

///////////////////////////////////////////////////////////////////////
/** SSkinTreeLines */
SSkinTreeLines::SSkinTreeLines()
    : m_crLine(RGBA(128, 128, 128, 255))
    , m_crCross(RGBA(0, 0, 0, 255))
    , m_nLineWidth(1)
    , m_nBoxSize(12)
{
}

SIZE SSkinTreeLines::GetSkinSize() const
{
// Return a suitable default size
#define kDefTreeNodeSize 16
    int wid = MulDiv(kDefTreeNodeSize, GetScale(), 100);
    return CSize(wid, wid);
}

int SSkinTreeLines::GetStates() const
{
    // Return 10 states
    return 10;
}

void SSkinTreeLines::_DrawByIndex(IRenderTarget *pRT, LPCRECT prcDraw, int iState, BYTE byAlpha) const
{
    SAutoRefPtr<IPenS> pPen, pPenCross, pPenDash;
    HRESULT hr = S_OK;

    // Create the pen and brush for the line color
    hr = pRT->CreatePen(PS_SOLID, m_crCross, m_nLineWidth, &pPenCross);
    if (FAILED(hr))
        return;
    hr = pRT->CreatePen(PS_SOLID, m_crLine, m_nLineWidth, &pPen);
    if (FAILED(hr))
        return;
    hr = pRT->CreatePen(PS_DOT, m_crLine, m_nLineWidth, &pPenDash);
    if (FAILED(hr))
        return;

    // Save the currently used object
    SAutoRefPtr<IRenderObj> pOldPen;
    pRT->SelectObject(pPen, &pOldPen);

    BOOL oldAntiAlias = pRT->SetAntiAlias(FALSE);
    // Calculate the center point
    CRect rcDraw(prcDraw);
    int centerX = (prcDraw->left + prcDraw->right) / 2;
    int centerY = (prcDraw->top + prcDraw->bottom) / 2;
    // Define the box size
    int boxSize = m_nBoxSize;
    CRect rcBox(centerX - boxSize / 2, centerY - boxSize / 2, centerX + boxSize / 2, centerY + boxSize / 2);
    rcBox.OffsetRect(-m_nLineWidth / 2, -m_nLineWidth / 2);
    const int kHalfLineLength = MulDiv(2, GetScale(), 100);
    // Define the drawing logic for the 10 states
    switch (iState)
    {
    case 0:
    {
        // The box has a + sign in the middle and a connected dashed line on the right
        // Draw the box
        pRT->DrawRectangle(&rcBox);
        // Draw the + sign
        pRT->SelectObject(pPenCross, NULL);
        pRT->DrawLine(CPoint(centerX, centerY - kHalfLineLength), CPoint(centerX, centerY + kHalfLineLength));
        pRT->DrawLine(CPoint(centerX - kHalfLineLength, centerY), CPoint(centerX + kHalfLineLength, centerY));
        // Draw the connected dashed line on the right
        pRT->SelectObject(pPenDash, NULL);
        pRT->DrawLine(CPoint(centerX + boxSize / 2, centerY), CPoint(rcDraw.right, centerY));
        break;
    }
    case 1:
    {
        // The box has a + sign in the middle and connected dashed lines on the top, right, and bottom
        // Draw the box
        pRT->DrawRectangle(&rcBox);
        // Draw the + sign
        pRT->SelectObject(pPenCross, NULL);
        pRT->DrawLine(CPoint(centerX, centerY - kHalfLineLength), CPoint(centerX, centerY + kHalfLineLength));
        pRT->DrawLine(CPoint(centerX - kHalfLineLength, centerY), CPoint(centerX + kHalfLineLength, centerY));
        // Draw the connected dashed lines on the top and right
        pRT->SelectObject(pPenDash, NULL);
        pRT->DrawLine(CPoint(centerX, rcDraw.top), CPoint(centerX, centerY - boxSize / 2));    // top line
        pRT->DrawLine(CPoint(centerX + boxSize / 2, centerY), CPoint(rcDraw.right, centerY));  // right line
        pRT->DrawLine(CPoint(centerX, centerY + boxSize / 2), CPoint(centerX, rcDraw.bottom)); // bottom line
        break;
    }
    case 2:
    {
        // The box has a + sign in the middle and connected dashed lines on the top and right
        // Draw the box
        pRT->DrawRectangle(&rcBox);
        // Draw the + sign
        pRT->SelectObject(pPenCross, NULL);
        pRT->DrawLine(CPoint(centerX, centerY - kHalfLineLength), CPoint(centerX, centerY + kHalfLineLength));
        pRT->DrawLine(CPoint(centerX - kHalfLineLength, centerY), CPoint(centerX + kHalfLineLength, centerY));
        // Draw the connected dashed lines on the top and right
        pRT->SelectObject(pPenDash, NULL);
        pRT->DrawLine(CPoint(centerX, rcDraw.top), CPoint(centerX, centerY - boxSize / 2));   // top line
        pRT->DrawLine(CPoint(centerX + boxSize / 2, centerY), CPoint(rcDraw.right, centerY)); // right line
        break;
    }
    case 3:
    {
        // The box has a - sign in the middle and a connected dashed line on the right
        // Draw the box
        pRT->DrawRectangle(&rcBox);
        // Draw the - sign
        pRT->SelectObject(pPenCross, NULL);
        pRT->DrawLine(CPoint(centerX - kHalfLineLength, centerY), CPoint(centerX + kHalfLineLength, centerY));
        // Draw the connected dashed line on the right
        pRT->SelectObject(pPenDash, NULL);
        pRT->DrawLine(CPoint(centerX + boxSize / 2, centerY), CPoint(rcDraw.right, centerY)); // right line
        break;
    }
    case 4:
    {
        // The box has a - sign in the middle and connected dashed lines on the top, right, and bottom
        // Draw the box
        pRT->DrawRectangle(&rcBox);
        // Draw the - sign
        pRT->SelectObject(pPenCross, NULL);
        pRT->DrawLine(CPoint(centerX - kHalfLineLength, centerY), CPoint(centerX + kHalfLineLength, centerY));
        // Draw the connected dashed lines on the top, right, and bottom
        pRT->SelectObject(pPenDash, NULL);
        pRT->DrawLine(CPoint(centerX, rcDraw.top), CPoint(centerX, centerY - boxSize / 2));    // top line
        pRT->DrawLine(CPoint(centerX + boxSize / 2, centerY), CPoint(rcDraw.right, centerY));  // right line
        pRT->DrawLine(CPoint(centerX, centerY + boxSize / 2), CPoint(centerX, rcDraw.bottom)); // bottom line
        break;
    }
    case 5:
    {
        // The box has a - sign in the middle and connected dashed lines on the top and right
        // Draw the box
        pRT->DrawRectangle(&rcBox);
        // Draw the - sign
        pRT->SelectObject(pPenCross, NULL);
        pRT->DrawLine(CPoint(centerX - kHalfLineLength, centerY), CPoint(centerX + kHalfLineLength, centerY));
        // Draw the connected dashed lines on the top and right
        pRT->SelectObject(pPenDash, NULL);
        pRT->DrawLine(CPoint(centerX, rcDraw.top), CPoint(centerX, centerY - boxSize / 2));   // top line
        pRT->DrawLine(CPoint(centerX + boxSize / 2, centerY), CPoint(rcDraw.right, centerY)); // right line
        break;
    }
    case 6:
    {
        // Vertical dashed line in the middle
        // Draw the vertical dashed line
        pRT->SelectObject(pPenDash, NULL);
        pRT->DrawLine(CPoint(centerX, rcDraw.top), CPoint(centerX, rcDraw.bottom));
        break;
    }
    case 7:
    {
        // Vertical dashed line in the middle + dashed line to the right
        // Draw the vertical dashed line
        pRT->SelectObject(pPenDash, NULL);
        pRT->DrawLine(CPoint(centerX, rcDraw.top), CPoint(centerX, rcDraw.bottom));
        // Draw the dashed line to the right
        pRT->DrawLine(CPoint(centerX, centerY), CPoint(rcDraw.right, centerY));
        break;
    }
    case 8:
    {
        // Connected dashed lines going up and to the right from the center point
        // Draw the dashed line going up
        pRT->SelectObject(pPenDash, NULL);
        pRT->DrawLine(CPoint(centerX, rcDraw.top), CPoint(centerX, centerY));
        // Draw the dashed line to the right
        pRT->DrawLine(CPoint(centerX, centerY), CPoint(rcDraw.right, centerY));
        break;
    }
    case 9:
    {
        // Connected dashed lines going down and to the right from the center point
        // Draw the dashed line going down
        pRT->SelectObject(pPenDash, NULL);
        pRT->DrawLine(CPoint(centerX, centerY), CPoint(centerX, rcDraw.bottom));
        // Draw the dashed line to the right
        pRT->DrawLine(CPoint(centerX, centerY), CPoint(rcDraw.right, centerY));
        break;
    }
    }

    // Restore the original object
    if (pOldPen)
        pRT->SelectObject(pOldPen, NULL);
    pRT->SetAntiAlias(oldAntiAlias);
}

void SSkinTreeLines::_Scale(ISkinObj *skinObj, int nScale)
{
    __baseCls::_Scale(skinObj, nScale);
    SSkinTreeLines *pRet = sobj_cast<SSkinTreeLines>(skinObj);
    if (pRet)
    {
        pRet->m_nLineWidth = MulDiv(m_nLineWidth, nScale, 100);
        pRet->m_nBoxSize = MulDiv(m_nBoxSize, nScale, 100);
    }
}
SNSEND
