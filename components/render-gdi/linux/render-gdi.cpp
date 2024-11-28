// render-gdi.cpp : Defines the exported functions for the DLL application.
//

#include "render-gdi.h"
#include <math.h>
#include <tchar.h>
#include <algorithm>
#include "GradientFillHelper.h"

namespace SOUI
{


    //////////////////////////////////////////////////////////////////////////
    // SRenderFactory_GDI
    BOOL SRenderFactory_Gdi::CreateRenderTarget( IRenderTarget ** ppRenderTarget ,int nWid,int nHei)
    {
        *ppRenderTarget = new SRenderTarget_GDI(this, nWid, nHei);
        return TRUE;
    }

	BOOL SRenderFactory_Gdi::CreateRenderTarget2(THIS_ IRenderTarget ** ppRenderTarget,HWND hWnd)
	{
		RECT rc;
		::GetClientRect(hWnd,&rc);
		return CreateRenderTarget(ppRenderTarget,rc.right,rc.bottom);
	}

    BOOL SRenderFactory_Gdi::CreateFont( IFontS ** ppFont , const LOGFONT *lf)
    {
        *ppFont = new SFont_GDI(this,lf);
        return TRUE;
    }

    BOOL SRenderFactory_Gdi::CreateBitmap( IBitmapS ** ppBitmap )
    {
        *ppBitmap = new SBitmap_GDI(this);
        return TRUE;
    }

    BOOL SRenderFactory_Gdi::CreateRegion( IRegionS **ppRgn )
    {
        *ppRgn = new SRegion_GDI(this);
        return TRUE;
    }

	BOOL SRenderFactory_Gdi::CreatePath(IPathS ** ppPath)
	{
		return FALSE;
	}

	BOOL SRenderFactory_Gdi::CreatePathEffect(REFGUID guidEffect,IPathEffect ** ppPathEffect)
	{
		return FALSE;
	}

	void SRenderFactory_Gdi::SetImgDecoderFactory(IImgDecoderFactory *pImgDecoderFac)
	{
		m_imgDecoderFactory=pImgDecoderFac;
	}

	IImgDecoderFactory * SRenderFactory_Gdi::GetImgDecoderFactory()
	{
		return m_imgDecoderFactory;
	}


	//////////////////////////////////////////////////////////////////////////

	HPEN SPen_GDI::GetPen()
	{
		return m_hPen;
	}

	void SPen_GDI::SetColor(THIS_ COLORREF cr)
	{
		m_cr = cr;
		UpdatePen();
	}

	COLORREF SPen_GDI::GetColor(THIS) SCONST
	{
		return m_cr;
	}

	void SPen_GDI::SetStyle(THIS_ int nStyle)
	{
		m_style = nStyle;
		UpdatePen();
	}

	int SPen_GDI::GetStyle(THIS) SCONST
	{
		return m_style;
	}

	void SPen_GDI::SetWidth(THIS_ int nWid)
	{
		m_nWidth=nWid;
		UpdatePen();
	}

	int SPen_GDI::GetWidth(THIS) SCONST
	{
		return m_nWidth;
	}


	SPen_GDI::SPen_GDI(IRenderFactory * pRenderFac,int iStyle/*=PS_SOLID*/,COLORREF cr/*=0*/,int cWidth/*=1*/) :TGdiRenderObjImpl<IPenS,OT_PEN>(pRenderFac)
		,m_nWidth(cWidth),m_style(iStyle),m_cr(cr)
		,m_hPen(NULL)
	{
		UpdatePen();
	}

	SPen_GDI::~SPen_GDI()
	{
		DeleteObject(m_hPen);
	}

	void SPen_GDI::UpdatePen()
	{
		if(m_hPen)
		{
			DeleteObject(m_hPen);
			m_hPen=NULL;
		}
		m_hPen = ::CreatePen(m_style , m_nWidth, m_cr);
	}

	//////////////////////////////////////////////////////////////////////////

	const LOGFONT * SFont_GDI::LogFont()  SCONST
	{
		return &m_lf;
	}

	LPCTSTR SFont_GDI::FamilyName() SCONST
	{
		return m_lf.lfFaceName;
	}

	int SFont_GDI::TextSize() SCONST
	{
		return m_lf.lfHeight;
	}

	BOOL SFont_GDI::IsBold() SCONST
	{
		return m_lf.lfWeight == FW_BOLD;
	}

	BOOL SFont_GDI::IsItalic() SCONST
	{
		return m_lf.lfItalic;
	}

	BOOL SFont_GDI::IsUnderline() SCONST
	{
		return m_lf.lfUnderline;
	}

	BOOL SFont_GDI::IsStrikeOut() SCONST
	{
		return m_lf.lfStrikeOut;
	}

	BOOL SFont_GDI::UpdateFont(const LOGFONT *pLogFont)
	{
		if(!m_hFont) return FALSE;
		DeleteObject(m_hFont);
		memcpy(&m_lf,pLogFont,sizeof(LOGFONT));
		m_hFont = CreateFontIndirect(&m_lf);
		return TRUE;
	}

	void SFont_GDI::SetProp(IXmlNode *pXmlNode)
	{

	}

	SFont_GDI::SFont_GDI(IRenderFactory * pRenderFac,const LOGFONT * plf) 
		:TGdiRenderObjImpl<IFontS,OT_FONT>(pRenderFac)
		,m_hFont(NULL)
	{
		memcpy(&m_lf,plf,sizeof(LOGFONT));
		m_hFont=CreateFontIndirect(&m_lf);
	}

	SFont_GDI::~SFont_GDI()
	{
		if(m_hFont) 
			DeleteObject(m_hFont);
	}


	//////////////////////////////////////////////////////////////////////////

	SBrush_GDI::SBrush_GDI(IRenderFactory * pRenderFac,HBITMAP hBmp,TileMode tileMode) :TGdiRenderObjImpl<IBrushS,OT_BRUSH>(pRenderFac),m_brushType(Brush_Bitmap)
	{
        m_hBrush = CreatePatternBrush2(hBmp, (TILEMODE)tileMode);
	}

	SBrush_GDI::SBrush_GDI(IRenderFactory * pRenderFac,COLORREF cr) :TGdiRenderObjImpl<IBrushS,OT_BRUSH>(pRenderFac),m_cr(cr),m_brushType(Brush_Color)
	{
		m_hBrush = ::CreateSolidBrush(m_cr);
	}

    SBrush_GDI::SBrush_GDI(IRenderFactory* pRenderFac, const GradientItem* pGradients, int nCount, const GradientInfo* info, BYTE byAlpha, TileMode tileMode)
        : TGdiRenderObjImpl<IBrushS, OT_BRUSH>(pRenderFac), m_brushType(Brush_Shader)
    {
        m_tileMode = tileMode;
        m_byAlpha = byAlpha;
        m_gradInfo = *info;
        m_arrGradItem.SetCount(nCount);
        memcpy(m_arrGradItem.GetData(), pGradients, sizeof(GradientItem) * nCount);
        m_hBrush = ::CreateGradientBrush((PGRADIENTITEM)pGradients, nCount, (PGRADIENTINFO)info, byAlpha, (TILEMODE)tileMode);
    }


	SBrush_GDI::~SBrush_GDI()
	{
		if(m_hBrush) DeleteObject(m_hBrush);
	}

    //////////////////////////////////////////////////////////////////////////
    //  SBitmap_GDI
    HBITMAP SBitmap_GDI::CreateGDIBitmap( int nWid,int nHei,void ** ppBits )
    {
        BITMAPINFO bmi;
        memset(&bmi, 0, sizeof(bmi));
        bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth       = nWid;
        bmi.bmiHeader.biHeight      = -nHei; // top-down image 
        bmi.bmiHeader.biPlanes      = 1;
        bmi.bmiHeader.biBitCount    = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage   = 0;

        HDC hdc=GetDC(0);
        HBITMAP hBmp=CreateDIBSection(hdc,&bmi,DIB_RGB_COLORS,ppBits,0,0);
        ReleaseDC(0,hdc);
        return hBmp;
    }

    HRESULT SBitmap_GDI::Init( int nWid,int nHei ,const LPVOID pBits/*=NULL*/)
    {
        if(m_hBmp) DeleteObject(m_hBmp);
        LPVOID pBmpBits=NULL;
        m_hBmp = CreateGDIBitmap(nWid,nHei,&pBmpBits);
        if(m_hBmp)
        {
            if(UpdateDIBPixmap(m_hBmp,nWid,nHei,32,nWid*4,pBits))
                m_sz.cx=nWid,m_sz.cy=nHei;
        }
        return m_hBmp?S_OK:E_OUTOFMEMORY;
    }

    HRESULT SBitmap_GDI::Init2(IImgFrame *pFrame )
    {
        UINT nWid,nHei;
        pFrame->GetSize(&nWid,&nHei);

        if(m_hBmp) DeleteObject(m_hBmp);
        void * pBits=NULL;
        m_hBmp = CreateGDIBitmap(nWid,nHei,&pBits);
        if(!m_hBmp) return E_OUTOFMEMORY;
        if(!UpdateDIBPixmap(m_hBmp,nWid,nHei,32,nWid*4,pFrame->GetPixels()))
            m_sz.cx=nWid,m_sz.cy=nHei;
        return S_OK;
    }

    HRESULT SBitmap_GDI::LoadFromFile( LPCTSTR pszFileName)
    {
        SAutoRefPtr<IImgX> imgDecoder;
        GetRenderFactory()->GetImgDecoderFactory()->CreateImgX(&imgDecoder);
#ifdef _UNICODE
		if(imgDecoder->LoadFromFileW(pszFileName)==0) return S_FALSE;
#else
		if(imgDecoder->LoadFromFileA(pszFileName)==0) return S_FALSE;
#endif
        return ImgFromDecoder(imgDecoder);
    }

    HRESULT SBitmap_GDI::LoadFromMemory(LPBYTE pBuf,size_t szLen)
    {
        SAutoRefPtr<IImgX> imgDecoder;
        GetRenderFactory()->GetImgDecoderFactory()->CreateImgX(&imgDecoder);
        if(imgDecoder->LoadFromMemory(pBuf,szLen)==0) return S_FALSE;
        return ImgFromDecoder(imgDecoder);
    }

    HRESULT SBitmap_GDI::ImgFromDecoder(IImgX *imgDecoder)
    {
        IImgFrame *pFrame=imgDecoder->GetFrame(0);
        UINT nWid,nHei;
        pFrame->GetSize(&nWid,&nHei);
        m_sz.cx=nWid,m_sz.cy=nHei;
        
        if(m_hBmp) DeleteObject(m_hBmp);
        void * pBits=NULL;
        m_hBmp = CreateGDIBitmap(m_sz.cx,m_sz.cy,&pBits);

        if(!m_hBmp) return E_OUTOFMEMORY;
        UpdateDIBPixmap(m_hBmp,nWid,nHei,32,nWid*4,pFrame->GetPixels());
        return S_OK;
    }

    UINT SBitmap_GDI::Width() const
    {
        return m_sz.cx;
    }

    UINT SBitmap_GDI::Height() const
    {
        return m_sz.cy;
    }

    SIZE SBitmap_GDI::Size() const
    {
        return m_sz;
    }

    LPVOID SBitmap_GDI::LockPixelBits()
    {
        BITMAP bm;
        GetObject(m_hBmp,sizeof(bm),&bm);
        return bm.bmBits;
    }

    void SBitmap_GDI::UnlockPixelBits( LPVOID pBuf)
    {
        MarkPixmapDirty(m_hBmp);
    }

    const LPVOID SBitmap_GDI::GetPixelBits() const
    {
        BITMAP bm;
        GetObject(m_hBmp,sizeof(bm),&bm);
        return bm.bmBits;
    }

	HRESULT SBitmap_GDI::Clone(IBitmapS **ppClone) const 
	{
		HRESULT hr = E_UNEXPECTED;
		BOOL bOK = GetRenderFactory()->CreateBitmap(ppClone);
		if(bOK)
		{
			hr=(*ppClone)->Init(Width(),Height(),GetPixelBits());
			if(S_OK != hr)
			{
				(*ppClone)->Release();
				(*ppClone) = NULL;
			}
		}
		return hr;
	}

	HRESULT SBitmap_GDI::Scale(IBitmapS **ppOutput,int nScale,FilterLevel filterLevel) SCONST
	{
		int wid = MulDiv(Width(),nScale,100);
		int hei = MulDiv(Height(),nScale,100);
		return Scale2(ppOutput,wid,hei,filterLevel);
	}

	HRESULT SBitmap_GDI::Scale2(IBitmapS **pOutput,int nWid,int nHei,FilterLevel filterLevel) SCONST
	{
		if(nWid == Width() && nHei == Height())
		{
			return Clone(pOutput);
		}
		HRESULT hr = E_UNEXPECTED;
		IRenderTarget *pRT=NULL;
		if(GetRenderFactory()->CreateRenderTarget(&pRT,nWid,nHei))
		{
			RECT rcSrc = {0,0,(LONG)Width(),(LONG)Height()};
			RECT rcDst ={0,0,nWid,nHei};
			hr = pRT->DrawBitmapEx(&rcDst,this,&rcSrc,MAKELONG(EM_STRETCH,filterLevel),255);
			if(hr == S_OK)
			{
				*pOutput = (IBitmapS*)pRT->GetCurrentObject(OT_BITMAP);
				(*pOutput)->AddRef();
			}
			pRT->Release();
		}else
		{
			hr = E_OUTOFMEMORY;
		}
		return hr;
	}

	HRESULT SBitmap_GDI::Save(LPCWSTR pszFileName,const LPVOID pFormat) SCONST
	{
		LPBYTE pBits = (LPBYTE)GetPixelBits();
		return GetRenderFactory()->GetImgDecoderFactory()->SaveImage(pBits,Width(),Height(),pszFileName,pFormat);
	}

	HRESULT SBitmap_GDI::Save2(CTHIS_ LPCWSTR pszFileName, ImgFmt imgFmt) SCONST
	{
		LPBYTE pBits = (LPBYTE)GetPixelBits();
		return GetRenderFactory()->GetImgDecoderFactory()->SaveImage2(pBits,Width(),Height(),pszFileName,imgFmt);
	}


    //////////////////////////////////////////////////////////////////////////
    //	SRegion_GDI
    SRegion_GDI::SRegion_GDI( IRenderFactory *pRenderFac )
        :TGdiRenderObjImpl<IRegionS,OT_RGN>(pRenderFac)
    {
        m_hRgn = ::CreateRectRgn(0,0,0,0);
    }

	SRegion_GDI::~SRegion_GDI()
	{
		DeleteObject(m_hRgn);
	}


    /*
    int CombineRgn(  HRGN hrgnDest,      // handle to destination region
    HRGN hrgnSrc1,      // handle to source region
    HRGN hrgnSrc2,      // handle to source region
    int fnCombineMode   // region combining mode);
    
    RGN_AND Creates the intersection of the two combined regions. 
    RGN_COPY Creates a copy of the region identified by hrgnSrc1. 
    RGN_DIFF Combines the parts of hrgnSrc1 that are not part of hrgnSrc2. 
    RGN_OR Creates the union of two combined regions. 
    RGN_XOR Creates the union of two combined regions except for any overlapping areas. 
    
    注意 RGN_DIFF 和 RGN_COPY对于hrgnSrc1的使用差异
    */
    void SRegion_GDI::CombineRect( LPCRECT lprect,int nCombineMode )
    {
        HRGN hRgn=::CreateRectRgnIndirect(lprect);
        _CombineRgn(hRgn,nCombineMode);
        DeleteObject(hRgn);
    }

	void SRegion_GDI::CombinePolygon(const POINT * pts, int count, int nPolygonMode, int nCombineMode)
	{
		HRGN hRgn = ::CreatePolygonRgn(pts, count, nPolygonMode);
		_CombineRgn(hRgn, nCombineMode);
		DeleteObject(hRgn);
	}

    void SRegion_GDI::CombineRgn(const IRegionS * pRgnSrc,int nCombineMode)
    {
        const SRegion_GDI *pRgnSrcGdi = (const SRegion_GDI*)pRgnSrc;
        HRGN hRgn = pRgnSrcGdi->GetRegion();
        _CombineRgn(hRgn,nCombineMode);
    }


    void SRegion_GDI::_CombineRgn(HRGN hRgn,int nCombineMode)
    {
        if(nCombineMode == RGN_DIFF)
        {
            ::CombineRgn(m_hRgn,m_hRgn,hRgn,RGN_DIFF);
        }else
        {
            ::CombineRgn(m_hRgn,hRgn,m_hRgn,nCombineMode);
        }

    }

    BOOL SRegion_GDI::PtInRegion( POINT pt ) const
    {
        return ::PtInRegion(m_hRgn,pt.x,pt.y);
    }

    BOOL SRegion_GDI::RectInRegion( LPCRECT lprect ) const
    {
        SASSERT(lprect);
        return ::RectInRegion(m_hRgn,lprect);
    }

    void SRegion_GDI::GetRgnBox( LPRECT lprect ) const
    {
        SASSERT(lprect);
        ::GetRgnBox(m_hRgn,lprect);
    }

    BOOL SRegion_GDI::IsEmpty() const
    {
        RECT rcBox;
        GetRgnBox(&rcBox);
        return (rcBox.left == rcBox.right) || (rcBox.top== rcBox.bottom);
    }

    void SRegion_GDI::Offset( POINT pt )
    {
        ::OffsetRgn(m_hRgn,pt.x,pt.y);
    }

    HRGN SRegion_GDI::GetRegion() const
    {
        return m_hRgn;
    }

    void SRegion_GDI::Clear()
    {
        ::SetRectRgn(m_hRgn,0,0,0,0);
    }

	void SRegion_GDI::CombineRoundRect(LPCRECT lprect, POINT ptRadius, int nCombineMode)
	{
		HRGN hRgn = ::CreateRoundRectRgn(lprect->left,lprect->top,lprect->right,lprect->bottom,ptRadius.x*2,ptRadius.y*2);
		::CombineRgn(m_hRgn,hRgn,NULL,nCombineMode);
		DeleteObject(hRgn);
	}

	void SRegion_GDI::CombineEllipse(LPCRECT lprect , int nCombineMode)
	{
		HRGN hRgn = ::CreateEllipticRgnIndirect(lprect);
		::CombineRgn(m_hRgn,hRgn,NULL,nCombineMode);
		DeleteObject(hRgn);
	}

    //////////////////////////////////////////////////////////////////////////
    //	SRenderTarget_GDI
    //////////////////////////////////////////////////////////////////////////
    
    SRenderTarget_GDI::SRenderTarget_GDI( IRenderFactory* pRenderFactory ,int nWid,int nHei)
        :m_hdc(NULL)
        ,m_curColor(0xFF000000)//默认黑色
        ,m_uGetDCFlag(0)
    {
		m_pRenderFactory = pRenderFactory;
                
        HDC hdc=::GetDC(0);
        m_hdc = CreateCompatibleDC(hdc);
        ::ReleaseDC(0,hdc);
        ::SetBkMode(m_hdc,TRANSPARENT);
        ::SetGraphicsMode(m_hdc,GM_ADVANCED);
        CreatePen(PS_SOLID,SColor(0,0,0).toCOLORREF(),1,&m_defPen);
        SelectObject(m_defPen,NULL);

        CreateSolidColorBrush(SColor(0,0,0).toCOLORREF(),&m_defBrush);
        SelectObject(m_defBrush,NULL);

        LOGFONT lf={0};
        lf.lfHeight=20;
        _tcscpy(lf.lfFaceName,_T("宋体"));
        pRenderFactory->CreateFont(&m_defFont,&lf);
        SelectObject(m_defFont,NULL);

        pRenderFactory->CreateBitmap(&m_defBmp);
        m_defBmp->Init(nWid,nHei,NULL);
        SelectObject(m_defBmp,NULL);
    }

    SRenderTarget_GDI::~SRenderTarget_GDI()
    {
        DeleteDC(m_hdc);
    }

    HRESULT SRenderTarget_GDI::CreatePen( int iStyle,COLORREF cr,int cWidth,IPenS ** ppPen )
    {
		*ppPen = new SPen_GDI(m_pRenderFactory,iStyle,cr,cWidth);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::CreateSolidColorBrush( COLORREF cr,IBrushS ** ppBrush )
    {
        *ppBrush = new SBrush_GDI(m_pRenderFactory,cr);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::CreateBitmapBrush( IBitmapS *pBmp,TileMode xtm,TileMode ytm,IBrushS ** ppBrush )
    {
        SBitmap_GDI *pBmpGdi = (SBitmap_GDI*)pBmp;
		(xtm),(ytm);
        *ppBrush = new SBrush_GDI(m_pRenderFactory, pBmpGdi->GetBitmap(), xtm);
        return S_OK;
    }

	HRESULT SRenderTarget_GDI::CreateGradientBrush(THIS_ const GradientItem *pGradients, int nCount, const GradientInfo *info, BYTE byAlpha,TileMode tileMode, IBrushS * *ppBrush)
	{
        if(info->type == sweep)
            return E_NOTIMPL;
        *ppBrush = new SBrush_GDI(m_pRenderFactory, pGradients, nCount, info, byAlpha, tileMode);
        return S_OK;
	}

    HRESULT SRenderTarget_GDI::Resize( SIZE sz )
    {
        HBITMAP hBmp=CreateCompatibleBitmap(m_hdc,0,0);
        ::SelectObject(m_hdc,hBmp);
        m_curBmp->Init(sz.cx,sz.cy,NULL);
        ::SelectObject(m_hdc,m_curBmp->GetBitmap());
        ::DeleteObject(hBmp);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::PushClipRect( LPCRECT pRect ,UINT mode/*=RGN_AND*/)
    {
        HRGN hRgn=::CreateRectRgnIndirect(pRect);
        ::SaveDC(m_hdc);
        ::ExtSelectClipRgn(m_hdc,hRgn,mode);
        ::DeleteObject(hRgn);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::PopClip()
    {
        ::RestoreDC(m_hdc,-1);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::ExcludeClipRect( LPCRECT pRc )
    {
        ::ExcludeClipRect(m_hdc,pRc->left,pRc->top,pRc->right,pRc->bottom);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::SaveClip( int *pnState )
    {
        int nState=::SaveDC(m_hdc);
        if(pnState) *pnState=nState;
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::RestoreClip( int nState/*=-1*/ )
    {
        ::RestoreDC(m_hdc,nState);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::PushClipRegion( IRegionS *pRegion ,UINT mode/*=RGN_AND*/)
    {
        SRegion_GDI *pRgnGDI=(SRegion_GDI*)pRegion;
        ::SaveDC(m_hdc);
        ::ExtSelectClipRgn(m_hdc, pRgnGDI->GetRegion(),mode);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::GetClipRegion( IRegionS **ppRegion )
    {
        SRegion_GDI *pRgn=new SRegion_GDI(m_pRenderFactory);
        ::GetClipRgn(m_hdc,pRgn->GetRegion());
        *ppRegion = pRgn;
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::GetClipBox(LPRECT prc)
    {
        ::GetClipBox(m_hdc,prc);
        return S_OK;
    }


    HRESULT SRenderTarget_GDI::BitBlt( LPCRECT pRcDest,IRenderTarget *pRTSour,int xSrc,int ySrc,DWORD dwRop/*=SRCCOPY*/)
    {
		switch(dwRop)
		{
		case kSrcCopy:
		case kDstInvert:
		case kSrcInvert:
		case kSrcAnd:
			break;
		default:
			return E_INVALIDARG;
		}
        SRenderTarget_GDI *pRTSrc_GDI=(SRenderTarget_GDI*)pRTSour;

        ::BitBlt(m_hdc,pRcDest->left,pRcDest->top,pRcDest->right-pRcDest->left,pRcDest->bottom-pRcDest->top,pRTSrc_GDI->m_hdc,xSrc,ySrc,dwRop);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::DrawText( LPCTSTR pszText,int cchLen,LPRECT pRc,UINT uFormat)
    {
        if(uFormat & DT_CALCRECT)
        {
            int nRet = ::DrawText(m_hdc,pszText,cchLen,pRc,uFormat);
			if(!nRet)
			{
				pRc->right = pRc->left;
				pRc->bottom = pRc->top;
			}
            return S_OK;
        }
        
        if(cchLen == 0) return S_OK;

        {
            ::DrawText(m_hdc,pszText,cchLen,pRc,uFormat);
        }

        return S_OK;
    }

    HRESULT SRenderTarget_GDI::MeasureText( LPCTSTR pszText,int cchLen, SIZE *psz )
    {
        if(cchLen<0) cchLen = _tcslen(pszText);
        ::GetTextExtentPoint32(m_hdc,pszText,cchLen,psz);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::DrawRectangle(LPCRECT pRect)
    {
		if(!m_curPen) return E_INVALIDARG;

		RECT rcBuf = *pRect;
		if (m_curPen->GetWidth() == 1)
		{
			rcBuf.right--;
			rcBuf.bottom--;
		}
		else
		{
			::InflateRect(&rcBuf, -m_curPen->GetWidth() / 2, -m_curPen->GetWidth() / 2);
		}
        HGDIOBJ oldBr=::SelectObject(m_hdc,GetStockObject(NULL_BRUSH));
        ::Rectangle(m_hdc, rcBuf.left, rcBuf.top, rcBuf.right, rcBuf.bottom);
        ::SelectObject(m_hdc,oldBr);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::FillRectangle(LPCRECT pRect)
    {
        BYTE byAlpha=0xFF;
        if(m_curBrush->GetBrushType() == Brush_Color) byAlpha = GetAValue(m_curBrush->GetColor());
        
        HGDIOBJ oldPen=::SelectObject(m_hdc,GetStockObject(NULL_PEN));
        ::Rectangle(m_hdc,pRect->left,pRect->top,pRect->right,pRect->bottom);
        ::SelectObject(m_hdc,oldPen);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::DrawRoundRect( LPCRECT pRect,POINT pt )
    {
        HGDIOBJ oldBr=::SelectObject(m_hdc,GetStockObject(NULL_BRUSH));
        ::RoundRect(m_hdc,pRect->left,pRect->top,pRect->right,pRect->bottom,pt.x*2,pt.y*2);
        ::SelectObject(m_hdc,oldBr);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::FillRoundRect( LPCRECT pRect,POINT pt )
    {
        BYTE byAlpha=0xFF;
        if(m_curBrush->GetBrushType() == Brush_Color) byAlpha = GetAValue(m_curBrush->GetColor());
        HGDIOBJ oldPen=::SelectObject(m_hdc,GetStockObject(NULL_PEN));
        ::RoundRect(m_hdc,pRect->left,pRect->top,pRect->right,pRect->bottom,pt.x*2,pt.y*2);
        ::SelectObject(m_hdc,oldPen);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::FillSolidRoundRect(LPCRECT pRect,POINT pt,COLORREF cr)
    {
        HBRUSH br=::CreateSolidBrush(cr);
		HGDIOBJ oldObj=::SelectObject(m_hdc,br);
		HGDIOBJ oldPen = ::SelectObject(m_hdc, GetStockObject(NULL_PEN));
		::RoundRect(m_hdc,pRect->left,pRect->top,pRect->right,pRect->bottom,pt.x*2,pt.y*2);
		::SelectObject(m_hdc, oldPen);
		::SelectObject(m_hdc,oldObj);
        ::DeleteObject(br);
        return S_OK;    
    }

    HRESULT SRenderTarget_GDI::DrawLines(LPPOINT pPt,size_t nCount)
    {
        RECT rc={100000,100000,0,0};
        for(size_t i=0;i<nCount;i++)
        {
            if(pPt[i].x<rc.left) rc.left=pPt[i].x;
            if(pPt[i].x>rc.right) rc.right=pPt[i].x;
            if(pPt[i].y<rc.top) rc.top=pPt[i].y;
            if(pPt[i].y>rc.bottom) rc.bottom=pPt[i].y;
        }
        rc.left -= 1;
        rc.top -=1;
        int nPenWidth = 1;
        if(m_curPen) nPenWidth = m_curPen->GetWidth();
        rc.bottom+=nPenWidth;
        rc.right+=nPenWidth;

        ::Polyline(m_hdc,pPt,(int)nCount);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::TextOut( int x, int y, LPCTSTR lpszString, int nCount)
    {
        if(nCount<0) nCount = _tcslen(lpszString);
		SIZE sz;
		MeasureText(lpszString,nCount,&sz);
		int escape = m_curFont->LogFont()->lfEscapement;
		if(escape != 0){
			//calc bound rect
			RECT rc;
			if(escape == 900){
				RECT rc2 = {x, y-sz.cx, x+sz.cy, y};
				rc = rc2;
			}else if(escape == 1800){
				RECT rc2 = {x-sz.cx, y-sz.cy, x, y};
				rc = rc2;
			}else if(escape == 2700){
				RECT rc2 = {x-sz.cy, y, x, y+sz.cx};
				rc = rc2;
			}else{
				RECT rc2 = {0,0, (LONG)m_curBmp->Width(),(LONG)m_curBmp->Height()};
				rc = rc2;
			}
			::TextOut(m_hdc,x,y,lpszString,nCount);
		}else{
			RECT rc={x,y,x+sz.cx,y+sz.cy};
			::TextOut(m_hdc,x,y,lpszString,nCount);
		}
		return S_OK;
    }

    HRESULT SRenderTarget_GDI::DrawIconEx( int xLeft, int yTop, HICON hIcon, int cxWidth,int cyWidth,UINT diFlags )
    {
        ::DrawIconEx(m_hdc, xLeft, yTop, hIcon, cxWidth, cyWidth, 0, NULL, diFlags);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::DrawBitmap(LPCRECT pRcDest,const IBitmapS *pBitmap,int xSrc,int ySrc,BYTE byAlpha/*=0xFF*/ )
    {
        SBitmap_GDI *pBmp = (SBitmap_GDI*)pBitmap;
        HBITMAP bmp=pBmp->GetBitmap();
        HDC hmemdc=CreateCompatibleDC(m_hdc);
        ::SelectObject(hmemdc,bmp);
        SIZE szBmp = pBmp->Size();
        
        BLENDFUNCTION bf={ AC_SRC_OVER,0,byAlpha,AC_SRC_ALPHA};
        int nWid= (std::min)(pRcDest->right-pRcDest->left,szBmp.cx);
        int nHei= (std::min)(pRcDest->bottom-pRcDest->top,szBmp.cy);
        
        BOOL bOK=::AlphaBlend(m_hdc,pRcDest->left,pRcDest->top,nWid,nHei,
                   hmemdc,xSrc,ySrc,nWid,nHei,bf);
        DeleteDC(hmemdc);
        
        return bOK?S_OK:E_FAIL;
    }

    HRESULT SRenderTarget_GDI::AlphaBlend( LPCRECT pRcDest,IRenderTarget *pRTSrc,LPCRECT pRcSrc,BYTE byAlpha )
    {
        BLENDFUNCTION bf={ AC_SRC_OVER,0,byAlpha,AC_SRC_ALPHA};
        SRenderTarget_GDI *pRTGdiSrc=(SRenderTarget_GDI*)pRTSrc;
        BOOL bOK=::AlphaBlend(m_hdc,pRcDest->left,pRcDest->top,pRcDest->right-pRcDest->left,pRcDest->bottom-pRcDest->top,
                              pRTGdiSrc->m_hdc,pRcSrc->left,pRcSrc->top,pRcSrc->right-pRcSrc->left,pRcSrc->bottom-pRcSrc->top,
                              bf);
        return bOK?S_OK:E_FAIL;
    }

    HRESULT SRenderTarget_GDI::DrawBitmapEx( LPCRECT pRcDest,const IBitmapS *pBitmap,LPCRECT pRcSrc,UINT expendMode, BYTE byAlpha/*=0xFF*/ )
    {
        SBitmap_GDI *pBmp = (SBitmap_GDI*)pBitmap;
        HBITMAP bmp=pBmp->GetBitmap();
        return ::DrawBitmapEx(m_hdc,pRcDest,bmp,pRcSrc,expendMode,byAlpha)?S_OK:S_FALSE;
    }


    HRESULT SRenderTarget_GDI::DrawBitmap9Patch( LPCRECT pRcDest,const IBitmapS *pBitmap,LPCRECT pRcSrc,LPCRECT pRcSourMargin,UINT expendMode,BYTE byAlpha/*=0xFF*/ )
    {
        SBitmap_GDI *pBmp = (SBitmap_GDI*)pBitmap;
        HBITMAP bmp=pBmp->GetBitmap();
        return ::DrawBitmap9Patch(m_hdc,pRcDest,bmp,pRcSrc,pRcSourMargin,expendMode,byAlpha)?S_OK:S_FALSE;
    }

    IRenderObj * SRenderTarget_GDI::GetCurrentObject( OBJTYPE uType )
    {
        IRenderObj *pRet=NULL;
        switch(uType)
        {
        case OT_BITMAP: 
            pRet=m_curBmp;
            break;
        case OT_PEN:
            pRet=m_curPen;
            break;
        case OT_BRUSH:
            pRet=m_curBrush;
            break;
        case OT_FONT:
            pRet=m_curFont;
            break;
        }
        return pRet;
    }

    HRESULT SRenderTarget_GDI::SelectDefaultObject(OBJTYPE objType, IRenderObj ** ppOldObj)
    {
        IRenderObj *pDefObj = NULL;
        switch(objType)
        {
        case OT_BITMAP: pDefObj = m_defBmp;break;
        case OT_PEN: pDefObj = m_defPen;break;
        case OT_BRUSH: pDefObj = m_defBrush;break;
        case OT_FONT: pDefObj = m_defFont;break;
        default:return E_INVALIDARG;
        }
        if(pDefObj == GetCurrentObject(objType)) 
            return S_FALSE;
        return SelectObject(pDefObj,ppOldObj);
    }

    HRESULT SRenderTarget_GDI::SelectObject( IRenderObj *pObj,IRenderObj ** ppOldObj /*= NULL*/ )
    {
        SAutoRefPtr<IRenderObj> pRet;
        switch(pObj->ObjectType())
        {
        case OT_BITMAP: 
            pRet=m_curBmp;
            m_curBmp=(SBitmap_GDI*)pObj;
            ::SelectObject(m_hdc,m_curBmp->GetBitmap());
            break;
        case OT_PEN:
            pRet=m_curPen;
            m_curPen=(SPen_GDI*)pObj;
            ::SelectObject(m_hdc,m_curPen->GetPen());
            break;
        case OT_BRUSH:
            pRet=m_curBrush;
            m_curBrush=(SBrush_GDI*)pObj;
            ::SelectObject(m_hdc,m_curBrush->GetBrush());
            break;
        case OT_FONT:
            pRet=m_curFont;
            m_curFont=(SFont_GDI*)pObj;
            ::SelectObject(m_hdc,m_curFont->GetFont());
            break;
        }
        if(pRet && ppOldObj)
        {//由调用者调用Release释放该RenderObj
            pRet->AddRef();
            *ppOldObj = pRet;
        }
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::OffsetViewportOrg( int xOff, int yOff, LPPOINT lpPoint )
    {
        ::OffsetViewportOrgEx(m_hdc, xOff, yOff, lpPoint);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::GetViewportOrg( LPPOINT lpPoint )
    {
        ::GetViewportOrgEx(m_hdc, lpPoint);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::SetViewportOrg( POINT pt )
    {
        ::SetViewportOrgEx(m_hdc, pt.x, pt.y,NULL);
        return S_OK;
    }

    HDC SRenderTarget_GDI::GetDC( UINT uFlag )
    {
        m_uGetDCFlag = uFlag;
        return m_hdc;
    }

    void SRenderTarget_GDI::ReleaseDC( HDC hdc ,LPCRECT pRc)
    {
        if(hdc == m_hdc)
        {
            m_uGetDCFlag =0;
        }
    }


    HRESULT SRenderTarget_GDI::DrawGradientRect(THIS_ LPCRECT pRect,  BOOL bVert, POINT ptRoundCorner, const GradientItem *pGradients, int nCount, BYTE byAlpha)
    {
        GradientInfo info;
        info.type = linear;
        info.angle=bVert?90.0f:0.0f;
        return DrawGradientRectEx(pRect,ptRoundCorner,pGradients,nCount,&info,byAlpha);
    }

	HRESULT SRenderTarget_GDI::DrawGradientRectEx(THIS_ LPCRECT pRect, POINT ptRoundCorner, const GradientItem *pGradients, int nCount, const GradientInfo *info, BYTE byAlpha){
        HBRUSH hbr = ::CreateGradientBrush((const GRADIENTITEM*)pGradients,nCount,(const GRADIENTINFO*)info,byAlpha,kTileMode_Repeat);
        if(!hbr)
            return E_INVALIDARG;
        if(ptRoundCorner.x==0 && ptRoundCorner.y==0)
            ::FillRect(m_hdc,pRect,hbr);
        else{
            HPEN hOldPen = ::SelectObject(m_hdc,GetStockObject(NULL_PEN));
            HBRUSH hOldBr = ::SelectObject(m_hdc,hbr);
            ::RoundRect(m_hdc,pRect->left,pRect->top,pRect->right,pRect->bottom,ptRoundCorner.x,ptRoundCorner.y);
            ::SelectObject(m_hdc,hOldBr);
            ::SelectObject(m_hdc,hOldPen);
        }
        DeleteObject(hbr);
        return S_OK;
	}

    //通过一个内存位图来填充位置的alpha值
    HRESULT SRenderTarget_GDI::FillSolidRect( LPCRECT pRect,COLORREF cr )
    {
        HBRUSH br=::CreateSolidBrush(cr);
        ::FillRect(m_hdc,pRect,br);
        ::DeleteObject(br);
        return S_OK;    
    }

    HRESULT SRenderTarget_GDI::ClearRect( LPCRECT pRect,COLORREF cr )
    {
        HBRUSH br = CreateSolidBrush(cr);
        FillRect(m_hdc, pRect,br);
        DeleteObject(br);
        return S_OK;    
    }

    HRESULT SRenderTarget_GDI::InvertRect(LPCRECT pRect)
    {
        return ::InvertRect(m_hdc,pRect)?S_OK:E_UNEXPECTED;
    }

    HRESULT SRenderTarget_GDI::DrawEllipse( LPCRECT pRect )
    {
		if(!m_curPen) return E_INVALIDARG;
		RECT rcBuf = *pRect;
		::InflateRect(&rcBuf,m_curPen->GetWidth()/2,m_curPen->GetWidth()/2);
        ::SelectObject(m_hdc, (HBRUSH)GetStockObject(NULL_BRUSH));
        ::Ellipse(m_hdc,pRect->left,pRect->top,pRect->right,pRect->bottom);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::FillEllipse( LPCRECT pRect )
    {
        BYTE byAlpha=0xFF;
        if(m_curBrush->GetBrushType() == Brush_Color) byAlpha = GetAValue(m_curBrush->GetColor());
		HGDIOBJ oldPen =::SelectObject(m_hdc,GetStockObject(NULL_PEN));
        ::Ellipse(m_hdc,pRect->left,pRect->top,pRect->right,pRect->bottom);
		::SelectObject(m_hdc,oldPen);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::FillSolidEllipse(LPCRECT pRect,COLORREF cr)
    {
		HBRUSH br=::CreateSolidBrush(cr);
		HGDIOBJ oldObj=::SelectObject(m_hdc,br);
		HGDIOBJ oldPen = ::SelectObject(m_hdc, GetStockObject(NULL_PEN));
		::Ellipse(m_hdc,pRect->left,pRect->top,pRect->right,pRect->bottom);
		::SelectObject(m_hdc, oldPen);
		::SelectObject(m_hdc,oldObj);
		::DeleteObject(br);
		return S_OK;    
    }

    const float PI = 3.1415926f;

    HRESULT SRenderTarget_GDI::DrawArc( LPCRECT pRect,float startAngle,float sweepAngle,BOOL useCenter )
    {
		if(!m_curPen) return E_INVALIDARG;

		RECT rcBuf = *pRect;
		::InflateRect(&rcBuf,m_curPen->GetWidth()/2,m_curPen->GetWidth()/2);

        HGDIOBJ oldBr=::SelectObject(m_hdc,GetStockObject(NULL_BRUSH));
        POINT ptCenter = {(pRect->left+pRect->right)/2,(pRect->top+pRect->bottom)/2};
        int   a=ptCenter.x-pRect->left,b=ptCenter.y-pRect->top;
        POINT pt1,pt2;
        float startAngle2 =startAngle*PI/180.0f;
        float endAngle2 = (startAngle+sweepAngle)*PI/180.0f;
        pt1.x=ptCenter.x+(int)(a*cos(startAngle2));
        pt1.y=ptCenter.y+(int)(b*sin(startAngle2));
        pt2.x=ptCenter.x+(int)(a*cos(endAngle2));
        pt2.y=ptCenter.y+(int)(b*sin(endAngle2));
		if (useCenter)
			::Pie(m_hdc, pRect->left, pRect->top, pRect->right, pRect->bottom, pt2.x, pt2.y, pt1.x, pt1.y);
        else
			::Arc(m_hdc,pRect->left,pRect->top,pRect->right,pRect->bottom,pt2.x,pt2.y,pt1.x,pt1.y);
        ::SelectObject(m_hdc,oldBr);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::DrawArc2(LPCRECT pRect, float startAngle, float sweepAngle, int width) {
        return E_NOTIMPL;
    }

    HRESULT SRenderTarget_GDI::FillArc( LPCRECT pRect,float startAngle,float sweepAngle )
    {
        BYTE byAlpha=0xFF;
        if(m_curBrush->GetBrushType() == Brush_Color) byAlpha = GetAValue(m_curBrush->GetColor());

        HGDIOBJ oldPen=::SelectObject(m_hdc,GetStockObject(NULL_PEN));
        POINT ptCenter = {(pRect->left+pRect->right)/2,(pRect->top+pRect->bottom)/2};
        int   a=ptCenter.x-pRect->left,b=ptCenter.y-pRect->top;
        POINT pt1,pt2;
        float startAngle2 =startAngle*PI/180.0f;
        float endAngle2 = (startAngle+sweepAngle)*PI/180.0f;
        pt1.x=ptCenter.x+(int)(a*cos(startAngle2));
        pt1.y=ptCenter.y+(int)(b*sin(startAngle2));
        pt2.x=ptCenter.x+(int)(a*cos(endAngle2));
        pt2.y=ptCenter.y+(int)(b*sin(endAngle2));
        ::Pie(m_hdc,pRect->left,pRect->top,pRect->right,pRect->bottom,pt2.x,pt2.y,pt1.x,pt1.y);
        ::SelectObject(m_hdc,oldPen);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::SetTransform(const float matrix[9], float oldMatrix[9])
    {
		XFORM xForm = { matrix[kMScaleX],matrix[kMSkewY],
			matrix[kMSkewX],matrix[kMScaleY],
			matrix[kMTransX],matrix[kMTransY] };
        if(oldMatrix)
        {
			GetTransform(oldMatrix);
        }
        return ::SetWorldTransform(m_hdc,&xForm)?S_OK:E_FAIL;
    }

    HRESULT SRenderTarget_GDI::GetTransform( float matrix[9]) const
    {
		SASSERT(matrix);
		XFORM xForm;
		if(!::GetWorldTransform(m_hdc,&xForm))
			return E_FAIL;

		matrix[kMScaleX]=xForm.eM11; 
		matrix[kMSkewX]=xForm.eM21;
		matrix[kMTransX]=xForm.eDx;
		matrix[kMSkewY]=xForm.eM12;		
		matrix[kMScaleY]=xForm.eM22;
		matrix[kMTransY] = xForm.eDy;

		matrix[kMPersp0] = 0.0f;
		matrix[kMPersp1] = 0.0f;
		matrix[kMPersp2] = 1.0f;
		return S_OK;
	}

	COLORREF SRenderTarget_GDI::GetPixel( int x, int y )
	{
		if(!m_curBmp) return CR_INVALID;
		const COLORREF *pBits = (const COLORREF*)m_curBmp->GetPixelBits();
		POINT pt;
		GetViewportOrg(&pt);
		x += pt.x;
		y += pt.y;
		if(x<0 || x >= (int)m_curBmp->Width() || y<0 || y>= (int)m_curBmp->Height())
			return CR_INVALID;

		return pBits[y*m_curBmp->Width()+x];
	}

	COLORREF SRenderTarget_GDI::SetPixel( int x, int y, COLORREF cr )
	{
		if(!m_curBmp) return CR_INVALID;
		COLORREF *pBits = (COLORREF*)m_curBmp->LockPixelBits();
		POINT pt;
		GetViewportOrg(&pt);
		x += pt.x;
		y += pt.y;
		
		if(x >= (int)m_curBmp->Width() || y>= (int)m_curBmp->Height())
			return CR_INVALID;

		COLORREF crRet = pBits[y*m_curBmp->Width()+x];
	
		pBits[y*m_curBmp->Width()+x] = cr;
		
		m_curBmp->UnlockPixelBits(pBits);

		return crRet;
	}

	HRESULT SRenderTarget_GDI::DrawGradientPath(THIS_ const IPathS* path, const GradientItem *pGradients, int nCount,const GradientInfo *info, BYTE byAlpha)
	{
		return E_NOTIMPL;
	}

	HRESULT SRenderTarget_GDI::CreateRegion(IRegionS ** ppRegion)
	{
		return m_pRenderFactory->CreateRegion(ppRegion)?S_OK:E_OUTOFMEMORY;
	}

	HRESULT SRenderTarget_GDI::PushClipPath(const IPathS * path, UINT mode, BOOL doAntiAlias /*= false*/)
	{
		return E_NOTIMPL;
	}

	HRESULT SRenderTarget_GDI::DrawPath(const IPathS * path,IPathEffect * pathEffect)
	{
		return E_NOTIMPL;
	}

	HRESULT SRenderTarget_GDI::FillPath(const IPathS * path)
	{
		return E_NOTIMPL;
	}

	HRESULT SRenderTarget_GDI::SetXfermode(int mode,int *pOldMode)
	{
		switch (mode)
		{
		case kSrcCopy:
		case kDstInvert:
		case kSrcInvert:
		case kSrcAnd:
			break;
		default:
			return E_INVALIDARG;
		}
		int nOldMode = ::SetROP2(m_hdc,mode);
		if(pOldMode) *pOldMode = nOldMode;
		return S_OK;
	}

	BOOL SRenderTarget_GDI::SetAntiAlias(BOOL bAntiAlias)
	{
        Antialias mode = ::SetAntialiasMode(m_hdc, bAntiAlias ? ANTIALIAS_GOOD:ANTIALIAS_DEFAULT);
		return mode!= ANTIALIAS_DEFAULT;
	}

	BOOL SRenderTarget_GDI::GetAntiAlias() SCONST
	{
		return ::GetAntialiasMode(m_hdc)!= ANTIALIAS_DEFAULT;
	}

	COLORREF SRenderTarget_GDI::GetTextColor()
	{
		return m_curColor.toCOLORREF();
	}

	COLORREF SRenderTarget_GDI::SetTextColor(COLORREF color)
	{
		COLORREF crOld=m_curColor.toCOLORREF();
		m_curColor.setRGB(color);
		::SetTextColor(m_hdc,color);
		return crOld;
	}


    //////////////////////////////////////////////////////////////////////////
    namespace RENDER_GDI
    {
        BOOL SCreateInstance(IObjRef ** ppRenderFactory)
        {
            *ppRenderFactory = new SRenderFactory_Gdi;
            return TRUE;
        }
    }
}

EXTERN_C BOOL Render_Gdi_SCreateInstance(IObjRef ** ppRenderFactory)
{
	return SOUI::RENDER_GDI::SCreateInstance(ppRenderFactory);
}



