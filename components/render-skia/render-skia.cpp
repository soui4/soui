#include <core/SkShader.h>
#include <core/SkDevice.h>
#include <effects/SkDashPathEffect.h>
#include <effects/SkGradientShader.h>
#include <effects/SkBlurMaskFilter.h>
#include <core/SkGraphics.h>
#include <core/SkPathMeasure.h>
#include <src/effects/SkBlurMask.h>

#include <gdialpha.h>

#include "drawtext-skia.h"

#include "render-skia.h"

#include "skia2rop2.h"
#include "PathEffect-Skia.h"

#include <tchar.h>
#include <algorithm>
#include <vector>
#include <map>
#include <windows.h>

#define getTotalClip internal_private_getTotalClip
// #include <vld.h>

#ifndef PI	//PI
#define PI         ((float)3.141592654f)
#endif

namespace SOUI
{
	//PS_SOLID
	const float  ps_solid[] ={1.0f,0.0f};
	const float  ps_dash[] ={5.0f,5.0f};
	const float  ps_dot[] ={1.0f,4.0f};
	const float  ps_dashdot[] ={4.0f,1.0f,1.0f,1.0f};
	const float  ps_dashdotdot[] ={4.0f,1.0f,1.0f,1.0f,1.0f,1.0f};

	const struct LineDashEffect
	{
		const float  *fDash;
		int		nCount;
	}LINEDASHEFFECT[] =
	{
		{ps_solid,ARRAYSIZE(ps_solid)},
		{ps_dash,ARRAYSIZE(ps_dash)},
		{ps_dot,ARRAYSIZE(ps_dot)},
		{ps_dashdot,ARRAYSIZE(ps_dashdot)},
		{ps_dashdotdot,ARRAYSIZE(ps_dashdotdot)},
	};

	bool String2Bool(const SStringW & value)
	{
		SASSERT(!value.IsEmpty());
		return !(value == L"false" || value == L"0");
	}

	SkIRect toSkIRect(LPCRECT pRc)
	{
		SkIRect rc={(int)pRc->left,(int)pRc->top,(int)pRc->right,(int)pRc->bottom};
		return rc;
	}

	SkRect toSkRect(LPCRECT pRc)
	{
		SkIRect rc = { (int)pRc->left,(int)pRc->top,(int)pRc->right,(int)pRc->bottom };
		return SkRect::Make(rc);
	}

	void SkRect2RECT(const SkRect & rcIn, RECT * rcOut)
	{
		rcOut->left = (int)rcIn.fLeft;
		rcOut->top = (int)rcIn.fTop;
		rcOut->right = (int)(rcIn.fRight + 0.5f);
		rcOut->bottom = (int)(rcIn.fBottom + 0.5f);
	}

	void InflateSkRect(SkRect *pRect,SkScalar dx,SkScalar dy)
	{
		pRect->fLeft -= dx;
		pRect->fRight += dx;
		pRect->fTop -= dy;
		pRect->fBottom += dy;
	}

	int RectWid(LPCRECT pRc){return pRc->right-pRc->left;}
	int RectHei(LPCRECT pRc){return pRc->bottom-pRc->top;}

#define PS_ENDCAP_ROUND     0x00000000
#define PS_ENDCAP_SQUARE    0x00000100
#define PS_ENDCAP_FLAT      0x00000200
#define PS_ENDCAP_MASK      0x00000F00

#define PS_JOIN_ROUND       0x00000000
#define PS_JOIN_BEVEL       0x00001000
#define PS_JOIN_MITER       0x00002000
#define PS_JOIN_MASK        0x0000F000

#define PS_COSMETIC         0x00000000
#define PS_GEOMETRIC        0x00010000
#define PS_TYPE_MASK        0x000F0000

	class SLineDashEffect
	{
	public:
		SLineDashEffect(int iStyle):pDashPathEffect(NULL)
		{
			iStyle = iStyle&PS_STYLE_MASK;
			if(iStyle>=PS_SOLID && iStyle<=PS_DASHDOTDOT)
			{
				const LineDashEffect *pEff=&LINEDASHEFFECT[iStyle];
				pDashPathEffect=SkDashPathEffect::Create(pEff->fDash,pEff->nCount,0.0f);
			}
		}
		~SLineDashEffect()
		{
			if(pDashPathEffect) pDashPathEffect->unref();
		}

		SkDashPathEffect * Get() const{return pDashPathEffect;}
	private:
		SkDashPathEffect * pDashPathEffect;
	};

	class SStrokeCap{
	public:
		SStrokeCap(int iStyle){
			iStyle = iStyle & PS_ENDCAP_MASK;
			switch(iStyle)
			{
			case PS_ENDCAP_ROUND: m_Cap = SkPaint::kRound_Cap;break;
			case PS_ENDCAP_SQUARE: m_Cap = SkPaint::kSquare_Cap;break;
			case PS_ENDCAP_FLAT:default: m_Cap = SkPaint::kDefault_Cap; break;
			}
		}

		SkPaint::Cap Get() const
		{
			return m_Cap;
		}
	private:
		SkPaint::Cap m_Cap;
	};

	class SStrokeJoin{
	public:
		SStrokeJoin(int iStyle){
			iStyle = iStyle & PS_JOIN_MASK;
			switch(iStyle)
			{
			case PS_JOIN_ROUND: m_Join = SkPaint::kRound_Join;break;
			case PS_JOIN_BEVEL: m_Join = SkPaint::kBevel_Join;break;
			case PS_JOIN_MITER:default: m_Join = SkPaint::kDefault_Join; break;

			}
		}
		SkPaint::Join Get() const
		{
			return m_Join;
		}
	private:
		SkPaint::Join m_Join;
	};
	//////////////////////////////////////////////////////////////////////////
	// SRenderFactory_Skia

	SRenderFactory_Skia::SRenderFactory_Skia()
	{
		SkGraphics::Init();
		SkGraphics::SetFontCacheCountLimit(500);//cache up to 500 font resource.
	}

	SRenderFactory_Skia::~SRenderFactory_Skia()
	{
		SkGraphics::Term();
	}

	void SRenderFactory_Skia::SetImgDecoderFactory(IImgDecoderFactory *pImgDecoderFac)
	{
		m_imgDecoderFactory=pImgDecoderFac;
	}

	IImgDecoderFactory * SRenderFactory_Skia::GetImgDecoderFactory()
	{
		return m_imgDecoderFactory;
	}

	BOOL SRenderFactory_Skia::CreateRenderTarget( IRenderTarget ** ppRenderTarget ,int nWid,int nHei)
	{
		*ppRenderTarget = new SRenderTarget_Skia(this, nWid, nHei);
		return TRUE;
	}

	BOOL SRenderFactory_Skia::CreateFont( IFontS ** ppFont , const LOGFONT *lf )
	{
		*ppFont = new SFont_Skia(this,lf);
		return TRUE;
	}

	BOOL SRenderFactory_Skia::CreateBitmap( IBitmapS ** ppBitmap )
	{
		*ppBitmap = new SBitmap_Skia(this);
		return TRUE;
	}

	BOOL SRenderFactory_Skia::CreateRegion( IRegionS **ppRgn )
	{
		*ppRgn = new SRegion_Skia(this);
		return TRUE;
	}

	BOOL SRenderFactory_Skia::CreatePath(IPathS ** ppPath)
	{
		*ppPath = new SPath_Skia(this);
		return TRUE;
	}

	BOOL SRenderFactory_Skia::CreatePathEffect(REFGUID guidEffect,IPathEffect ** ppPathEffect)
	{
		* ppPathEffect = NULL;

		if(guidEffect == __suidof(ICornerPathEffect))
		{
			*ppPathEffect = (IPathEffect*) new SPathEffect_Corner();
		}else if(guidEffect == __suidof(IDashPathEffect))
		{
			*ppPathEffect = (IPathEffect *) new SPathEffect_Dash();
		}else if(guidEffect == __suidof(IDiscretePathEffect))
		{
			*ppPathEffect = (IPathEffect *) new SPathEffect_Discrete();
		}
		return (*ppPathEffect) != NULL;
	}

	HRESULT SRenderFactory_Skia::CreateBlurMaskFilter(float radius, BlurStyle style,BlurFlags flag,IMaskFilter ** ppMaskFilter)
	{
		SkMaskFilter *pMaskFilter = SkBlurMaskFilter::Create(::SkBlurStyle(style),SkBlurMaskFilter::ConvertRadiusToSigma(radius),flag);
		if(!pMaskFilter)
			return E_OUTOFMEMORY;
		*ppMaskFilter = new SMaskFilter_Skia(pMaskFilter);
		return S_OK;
	}


	HRESULT SRenderFactory_Skia::CreateEmbossMaskFilter(float direction[3], float ambient, float specular, float blurRadius,IMaskFilter ** ppMaskFilter)
	{
		SkMaskFilter *pMaskFilter=SkBlurMaskFilter::CreateEmboss(SkBlurMask::ConvertRadiusToSigma(blurRadius),direction,ambient,specular);
		if(!pMaskFilter)
			return E_OUTOFMEMORY;
		*ppMaskFilter = new SMaskFilter_Skia(pMaskFilter);
		return S_OK;
	}

	BOOL SRenderFactory_Skia::CreateRenderTarget2(THIS_ IRenderTarget ** ppRenderTarget,HWND hWnd)
	{
		RECT rc;
		::GetClientRect(hWnd,&rc);
		return CreateRenderTarget(ppRenderTarget,rc.right,rc.bottom);
	}

	//----------------------------------------------------------------------------------------------
	//////////////////////////////////////////////////////////////////////////
	SPen_Skia::SPen_Skia(IRenderFactory * pRenderFac,int iStyle/*=PS_SOLID*/,COLORREF cr/*=0xFF000000*/,int cWidth/*=1*/) :TSkiaRenderObjImpl<IPenS,OT_PEN>(pRenderFac)
		,m_nWidth(cWidth),m_style(iStyle),m_cr(cr)
	{

	}

	void SPen_Skia::SetColor(THIS_ COLORREF cr)
	{
		m_cr = cr;
	}

	COLORREF SPen_Skia::GetColor(THIS) SCONST
	{
		return m_cr;
	}

	void SPen_Skia::SetStyle(THIS_ int style)
	{
		m_style=style;
	}

	int SPen_Skia::GetStyle(THIS) SCONST
	{
		return m_style;
	}

	void SPen_Skia::SetWidth(THIS_ int nWid)
	{
		m_nWidth=nWid;
	}

	int SPen_Skia::GetWidth(THIS) SCONST
	{
		return m_nWidth;
	}

	//////////////////////////////////////////////////////////////////////////
	// SRenderTarget_Skia

	SRenderTarget_Skia::SRenderTarget_Skia( IRenderFactory* pRenderFactory ,int nWid,int nHei)
		:m_SkCanvas(NULL)
		,m_curColor(0xFF000000)//默认黑色
		,m_hGetDC(0)
		,m_nGetDC(0)
		,m_bAntiAlias(true)
		,m_xferMode(kSrcOver_Mode)
		,m_lastSave(0)
	{
		m_ptOrg.fX=m_ptOrg.fY=0.0f;
		m_pRenderFactory = pRenderFactory;

		m_SkCanvas = new SkCanvas();
		m_paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
		m_paint.setAntiAlias(true);
		m_paint.setLCDRenderText(true);
		m_paint.setStyle(SkPaint::kStrokeAndFill_Style);

		CreatePen(PS_SOLID,SColor(0,0,0).toCOLORREF(),1,&m_defPen);
		SelectObject(m_defPen,NULL);

		CreateSolidColorBrush(SColor(0,0,0).toCOLORREF(),&m_defBrush);
		SelectObject(m_defBrush,NULL);

		LOGFONT lf={0};
		lf.lfHeight=20;
#ifdef _WIN32
		_tcscpy(lf.lfFaceName,_T("宋体"));
#else
		_tcscpy(lf.lfFaceName, _T("Arial"));
#endif
		pRenderFactory->CreateFont(&m_defFont,&lf);
		SelectObject(m_defFont,NULL);

		pRenderFactory->CreateBitmap(&m_defBmp);
		m_defBmp->Init(nWid,nHei,NULL);
		SelectObject(m_defBmp,NULL);
		SAutoRefPtr<IPenS> pPen;
		CreatePen(PS_SOLID,SColor(0,0,0).toCOLORREF(),1,&pPen);
		SelectObject(pPen,NULL);
	}

	SRenderTarget_Skia::~SRenderTarget_Skia()
	{
		if(m_SkCanvas) delete m_SkCanvas;
	}

	void SRenderTarget_Skia::EndDraw(THIS) {
		#ifndef _WIN32
		m_curBmp->MarkDirty();
		#endif//_WIN32
	}

	HRESULT SRenderTarget_Skia::CreatePen( int iStyle,COLORREF cr,int cWidth,IPenS ** ppPen )
	{
		*ppPen = new SPen_Skia(m_pRenderFactory,iStyle,cr,cWidth);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::CreateSolidColorBrush( COLORREF cr,IBrushS ** ppBrush )
	{
		*ppBrush = new SBrush_Skia(m_pRenderFactory,cr);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::CreateBitmapBrush( IBitmapS *pBmp,TileMode xtm,TileMode ytm, IBrushS ** ppBrush )
	{
		SBitmap_Skia *pBmpSkia = (SBitmap_Skia*)pBmp;
		*ppBrush = new SBrush_Skia(m_pRenderFactory,pBmpSkia->GetSkBitmap(),xtm,ytm);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::CreateGradientBrush(THIS_ const GradientItem *pGradients, int nCount, const GradientInfo *info, BYTE byAlpha,TileMode tileMode,IBrushS * *ppBrush)
	{
		*ppBrush = new SBrush_Skia(m_pRenderFactory,pGradients,nCount,info,byAlpha,tileMode);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::CreateRegion(IRegionS ** ppRegion)
	{
		return m_pRenderFactory->CreateRegion(ppRegion)?S_OK:E_OUTOFMEMORY;
	}

	HRESULT SRenderTarget_Skia::Resize( SIZE sz )
	{
		m_curBmp->Init(sz.cx,sz.cy,NULL);
		delete m_SkCanvas;
		m_SkCanvas = new SkCanvas(m_curBmp->GetSkBitmap());
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::PushClipRect( LPCRECT pRect ,UINT mode/*=RGN_AND*/)
	{
		SkRect skrc=toSkRect(pRect);
		skrc.offset(m_ptOrg);

		m_SkCanvas->save();

		m_SkCanvas->clipRect(skrc,SRegion_Skia::RGNMODE2SkRgnOP(mode));
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::PushClipRegion( IRegionS *pRegion ,UINT mode/*=RGN_AND*/)
	{
		SRegion_Skia * rgn_skia=(SRegion_Skia*)pRegion;
		SkRegion rgn=rgn_skia->GetRegion();
		rgn.translate((int)m_ptOrg.fX,(int)m_ptOrg.fY);

		m_SkCanvas->save();

		m_SkCanvas->clipRegion(rgn,SRegion_Skia::RGNMODE2SkRgnOP(mode));

		return S_OK;
	}

	HRESULT SRenderTarget_Skia::PopClip()
	{
		m_SkCanvas->restore();
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::ExcludeClipRect( LPCRECT pRc )
	{
		SkRect skrc=toSkRect(pRc);
		skrc.offset(m_ptOrg);
		m_SkCanvas->clipRect(skrc,SkRegion::kDifference_Op);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::SaveClip( int *pnState )
	{
		m_lastSave=m_SkCanvas->save();
		if(pnState) *pnState=m_lastSave;
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::RestoreClip( int nState/*=-1*/ )
	{
		if(nState == -1)
			nState = m_lastSave;
		m_SkCanvas->restoreToCount(nState);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::GetClipRegion( IRegionS **ppRegion )
	{
		SRegion_Skia *pRgn=new SRegion_Skia(m_pRenderFactory);
		SkRegion rgn = m_SkCanvas->getTotalClip();
		//需要将rect的viewOrg还原
		rgn.translate((int)-m_ptOrg.fX,(int)-m_ptOrg.fY);
		pRgn->SetRegion(rgn);
		*ppRegion = pRgn;
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::GetClipBox(LPRECT prc)
	{
		SkRect skrc;
		m_SkCanvas->getClipBounds(&skrc);
		//需要将rect的viewOrg还原
		skrc.offset(-m_ptOrg);

		prc->left=(LONG)skrc.fLeft;
		prc->top=(LONG)skrc.fTop;
		prc->right=(LONG)skrc.fRight;
		prc->bottom=(LONG)skrc.fBottom;
		//需要4周缩小一个单位才是和GDI相同的剪裁区
		::InflateRect(prc,-1,-1);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::BitBlt( LPCRECT pRcDest,IRenderTarget *pRTSour,int xSrc,int ySrc,DWORD dwRop/*=SRCCOPY*/)
	{
		SkPaint paint=m_paint;
		paint.setStyle(SkPaint::kFill_Style);
		SetPaintXferMode(paint,dwRop);

		SRenderTarget_Skia *pRtSourSkia=(SRenderTarget_Skia*)pRTSour;
		const SkBitmap  &  bmpSrc=pRtSourSkia->m_curBmp->GetSkBitmap();
		POINT ptSourViewport;
		pRtSourSkia->GetViewportOrg(&ptSourViewport);
		xSrc += ptSourViewport.x;
		ySrc += ptSourViewport.y;


		SkIRect isrc={xSrc,ySrc,xSrc + RectWid(pRcDest),ySrc+RectHei(pRcDest)};
		SkRect skrc=toSkRect(pRcDest);
		skrc.offset(m_ptOrg);
		m_SkCanvas->drawBitmapRect(bmpSrc,&isrc,skrc,&paint);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::DrawText( LPCTSTR pszText,int cchLen,LPRECT pRc,UINT uFormat)
	{
		if(cchLen<0) cchLen= (int)_tcslen(pszText);
		if(cchLen==0)
		{
			if(uFormat & DT_CALCRECT)
			{
				pRc->right=pRc->left;
				pRc->bottom=pRc->top;
			}
			return S_OK;
		}

		SStringW strW=S_CT2W(SStringT(pszText,cchLen));
		SkPaint     txtPaint = m_paint;
		SFont_Skia *pFont = m_curFont;
		txtPaint.setTypeface(pFont->GetFont());
		txtPaint.setTextSize(SkIntToScalar(abs(pFont->TextSize())));
		txtPaint.setUnderlineText(!!pFont->IsUnderline());
		txtPaint.setStrikeThruText(!!pFont->IsStrikeOut());
		txtPaint.setStyle(SkPaint::Style(pFont->m_fillStyle));
		txtPaint.setLCDRenderText(!!pFont->m_bLcdText);
		if(pFont->m_maskFilter)
		{
			SkMaskFilter *maskFilter=(SkMaskFilter*)pFont->m_maskFilter->GetPtr();
			txtPaint.setMaskFilter(maskFilter);
		}

		if(uFormat & DT_CENTER)
			txtPaint.setTextAlign(SkPaint::kCenter_Align);
		else if(uFormat & DT_RIGHT)
			txtPaint.setTextAlign(SkPaint::kRight_Align);
		else
			txtPaint.setTextAlign(SkPaint::kLeft_Align);
		SkRect skrc=toSkRect(pRc);
		skrc.offset(m_ptOrg);
		if(uFormat & DT_CALCRECT)
		{
			skrc=DrawText_Skia(m_SkCanvas,strW,strW.GetLength(),skrc,txtPaint,uFormat);
			pRc->left=(int)skrc.fLeft;
			pRc->top=(int)skrc.fTop;
			pRc->right=(int)skrc.fRight;
			pRc->bottom=(int)skrc.fBottom;
		}else if(m_curFont->LogFont()->lfEscapement!=0){
			//calc draw size
			SkRect skrcContent=DrawText_Skia(m_SkCanvas,strW,strW.GetLength(),skrc,txtPaint,uFormat|DT_CALCRECT);
			if(uFormat&DT_CENTER){
				skrc.fLeft += (skrc.width()-skrcContent.width())/2;
			}else if(uFormat&DT_RIGHT){
				skrc.fLeft += skrc.width()-skrcContent.width();
			}
			skrc.fRight = skrc.fLeft+skrcContent.width();
			if(uFormat&DT_VCENTER){
				skrc.fTop+=(skrc.height()-skrcContent.height())/2;
			}else if(uFormat&DT_BOTTOM){
				skrc.fTop+=skrc.height()-skrcContent.height();
			}
			skrc.fBottom=skrc.fTop+skrcContent.height();

			SkMatrix oldMtx = m_SkCanvas->getTotalMatrix();
			SkScalar fx = skrc.fLeft;
			SkScalar fy = skrc.fTop;
			SkMatrix mtx;
			float degree = (float)m_curFont->LogFont()->lfEscapement;
			degree /= 10;
			degree = 360.f-degree;//change to clockwise.
			mtx.setRotate(degree);
			mtx.preTranslate(-fx,-fy);
			mtx.postTranslate(fx,fy);
			mtx.postConcat(oldMtx);
			m_SkCanvas->setMatrix(mtx);
			skrc=DrawText_Skia(m_SkCanvas,strW,strW.GetLength(),skrc,txtPaint,uFormat);
			m_SkCanvas->setMatrix(oldMtx);
		}else{
			DrawText_Skia(m_SkCanvas,strW,strW.GetLength(),skrc,txtPaint,uFormat);
		}
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::MeasureText( LPCTSTR pszText,int cchLen, SIZE *psz )
	{
		SkPaint     txtPaint = m_paint;
		txtPaint.setTypeface(m_curFont->GetFont());
		txtPaint.setTextSize(SkIntToScalar(abs(m_curFont->TextSize())));
		SStringW strW=S_CT2W(SStringT(pszText,cchLen));
		psz->cx = (int)txtPaint.measureText(strW,strW.GetLength()*sizeof(wchar_t));

		SkPaint::FontMetrics metrics;
		txtPaint.getFontMetrics(&metrics);
		psz->cy = (int)(metrics.fBottom-metrics.fTop);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::DrawRectangle(LPCRECT pRect)
	{
		SkPaint paint=m_paint;
		paint.setColor(SColor(m_curPen->GetColor()).toARGB());
		SLineDashEffect skDash(m_curPen->GetStyle());
		paint.setPathEffect(skDash.Get());
		SStrokeCap strokeCap(m_curPen->GetStyle());
		paint.setStrokeCap(strokeCap.Get());
		SStrokeJoin strokeJoin(m_curPen->GetStyle());
		paint.setStrokeJoin(strokeJoin.Get());
		if(m_bAntiAlias)
		{
			paint.setAntiAlias(true);
			paint.setStrokeWidth((SkScalar)m_curPen->GetWidth()-0.5f);
		}else
		{
			paint.setAntiAlias(false);
			paint.setStrokeWidth((SkScalar)m_curPen->GetWidth());
		}
		paint.setStyle(SkPaint::kStroke_Style);

		SkRect skrc=toSkRect(pRect);
		skrc.offset(m_ptOrg);
		InflateSkRect(&skrc,-0.5f,-0.5f);
		m_SkCanvas->drawRect(skrc,paint);
		return S_OK;
	}


	HRESULT SRenderTarget_Skia::FillRectangle(LPCRECT pRect)
	{
		SkPaint paint=m_paint;
		paint.setStyle(SkPaint::kFill_Style);

		SkRect skrc=toSkRect(pRect);
		skrc.offset(m_ptOrg);
		m_curBrush->InitPaint(paint,skrc);
		InflateSkRect(&skrc,-0.5f,-0.5f);
		m_SkCanvas->drawRect(skrc,paint);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::DrawRoundRect( LPCRECT pRect,POINT pt )
	{
		SkPaint paint=m_paint;
		SetPaintXferMode(paint,m_xferMode);
		paint.setColor(SColor(m_curPen->GetColor()).toARGB());
		SLineDashEffect skDash(m_curPen->GetStyle());
		paint.setPathEffect(skDash.Get());
		paint.setStyle(SkPaint::kStroke_Style);
		if(m_bAntiAlias)
		{
			paint.setStrokeWidth((SkScalar)m_curPen->GetWidth()-0.5f);
		}else
		{
			paint.setStrokeWidth((SkScalar)m_curPen->GetWidth());
		}

		SkRect skrc=toSkRect(pRect);
		InflateSkRect(&skrc,-0.5f,-0.5f);//要缩小0.5显示效果才和GDI一致。
		skrc.offset(m_ptOrg);
		m_SkCanvas->drawRoundRect(skrc,(SkScalar)pt.x,(SkScalar)pt.y,paint);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::FillRoundRect( LPCRECT pRect,POINT pt )
	{
		SkPaint paint=m_paint;

		paint.setStyle(SkPaint::kFill_Style);

		SkRect skrc=toSkRect(pRect);
		InflateSkRect(&skrc,-0.5f,-0.5f);//要缩小0.5显示效果才和GDI一致。
		skrc.offset(m_ptOrg);
		m_curBrush->InitPaint(paint,skrc);

		m_SkCanvas->drawRoundRect(skrc,(SkScalar)pt.x,(SkScalar)pt.y,paint);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::FillSolidRoundRect(LPCRECT pRect,POINT pt,COLORREF cr)
	{
		SkPaint paint=m_paint;

		paint.setFilterLevel(SkPaint::kNone_FilterLevel);
		paint.setColor(SColor(cr).toARGB());
		paint.setStyle(SkPaint::kFill_Style);

		SkRect skrc=toSkRect(pRect);
		InflateSkRect(&skrc,-0.5f,-0.5f);//要缩小0.5显示效果才和GDI一致。
		skrc.offset(m_ptOrg);

		m_SkCanvas->drawRoundRect(skrc,(SkScalar)pt.x,(SkScalar)pt.y,paint);
		return S_OK;
	}


	HRESULT SRenderTarget_Skia::DrawLines(LPPOINT pPt,size_t nCount)
	{
		SkPoint *pts=new SkPoint[nCount];
		for(size_t i=0; i<nCount; i++ )
		{
			pts[i].fX = (SkScalar)pPt[i].x;
			pts[i].fY = (SkScalar)pPt[i].y;
		}
		SkPoint::Offset(pts, (int)nCount,m_ptOrg);

		SkPaint paint=m_paint;
		if(m_bAntiAlias)
		{
			SkScalar wid = (SkScalar)m_curPen->GetWidth();
			if(wid>1.0f) wid-=0.5f;
			paint.setStrokeWidth(wid);
		}else
		{
			paint.setStrokeWidth((SkScalar)m_curPen->GetWidth());
		}

		paint.setColor(SColor(m_curPen->GetColor()).toARGB());
		SLineDashEffect skDash(m_curPen->GetStyle());
		paint.setPathEffect(skDash.Get());
		SStrokeCap strokeCap(m_curPen->GetStyle());
		paint.setStrokeCap(strokeCap.Get());
		SStrokeJoin strokeJoin(m_curPen->GetStyle());
		paint.setStrokeJoin(strokeJoin.Get());
		paint.setStyle(SkPaint::kStroke_Style);
		m_SkCanvas->drawPoints(SkCanvas::kPolygon_PointMode,nCount,pts,paint);
		delete []pts;

		return S_OK;
	}

	HRESULT SRenderTarget_Skia::TextOut( int x, int y, LPCTSTR lpszString, int nCount)
	{
		if(nCount<0) nCount= (int)_tcslen(lpszString);
		SStringW strW=S_CT2W(SStringT(lpszString,nCount));
		SkPaint     txtPaint =m_paint;
		txtPaint.setStyle(SkPaint::kStrokeAndFill_Style);
		txtPaint.setTypeface(m_curFont->GetFont());
		txtPaint.setTextSize(SkIntToScalar(abs(m_curFont->TextSize())));
		txtPaint.setUnderlineText(!!m_curFont->IsUnderline());
		txtPaint.setStrikeThruText(!!m_curFont->IsStrikeOut());

		SkPaint::FontMetrics metrics;
		txtPaint.getFontMetrics(&metrics);
		SkScalar fx = m_ptOrg.fX + x;
		SkScalar fy = m_ptOrg.fY + y;
		
		SkMatrix oldMtx = m_SkCanvas->getTotalMatrix();
		if(m_curFont->LogFont()->lfEscapement!=0){
			SkMatrix mtx;
			float degree = (float)m_curFont->LogFont()->lfEscapement;
			degree /= 10;
			degree = 360.f-degree;//change to clockwise.
			mtx.setRotate(degree);
			mtx.preTranslate(-fx,-fy);
			mtx.postTranslate(fx,fy);
			mtx.postConcat(oldMtx);
			m_SkCanvas->setMatrix(mtx);
		}
		fy -= metrics.fTop;
		m_SkCanvas->drawText((LPCWSTR)strW,strW.GetLength()*2,fx,fy,txtPaint);
		if(m_curFont->LogFont()->lfEscapement!=0){
			m_SkCanvas->setMatrix(oldMtx);
		}
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::DrawIconEx( int xLeft, int yTop, HICON hIcon, int cxWidth,int cyWidth,UINT diFlags )
	{
		HDC hdc=GetDC(0);
#ifdef _WIN32
		ICONINFO ii={0};
		::GetIconInfo(hIcon,&ii);
		SASSERT(ii.hbmColor);
		BITMAP bm;
		::GetObject(ii.hbmColor,sizeof(bm),&bm);

		ALPHAINFO ai;
		RECT rc={xLeft,yTop,xLeft+cxWidth,yTop+cyWidth};
		if(bm.bmBitsPixel!=32)
		{
			CGdiAlpha::AlphaBackup(hdc,&rc,ai);
		}
		BOOL bRet=::DrawIconEx(hdc,xLeft,yTop,hIcon,cxWidth,cyWidth,0,NULL,diFlags);
		if(bm.bmBitsPixel!=32)
		{
			CGdiAlpha::AlphaRestore(ai);
		}
		if(ii.hbmColor) DeleteObject(ii.hbmColor);
		if(ii.hbmMask) DeleteObject(ii.hbmMask);
#else
		BOOL bRet=::DrawIconEx(hdc,xLeft,yTop,hIcon,cxWidth,cyWidth,0,NULL,diFlags);
#endif//_WIN32
		ReleaseDC(hdc,NULL);
		return bRet?S_OK:S_FALSE;
	}

	HRESULT SRenderTarget_Skia::DrawBitmap(LPCRECT pRcDest,const IBitmapS *pBitmap,int xSrc,int ySrc,BYTE byAlpha/*=0xFF*/ )
	{
		SBitmap_Skia *pBmp = (SBitmap_Skia*)pBitmap;
		const SkBitmap & bmp=pBmp->GetSkBitmap();

		SIZE szBmp = pBmp->Size();
		int nWid= (std::min)(pRcDest->right-pRcDest->left,szBmp.cx);
		int nHei= (std::min)(pRcDest->bottom-pRcDest->top,szBmp.cy);

		RECT rcSrc={xSrc,ySrc,xSrc+nWid,ySrc+nHei};
		RECT rcDst={pRcDest->left,pRcDest->top,pRcDest->left+nWid,pRcDest->top+nHei};

		SkRect skrcSrc= toSkRect(&rcSrc);
		SkRect skrcDst = toSkRect(&rcDst);

		skrcDst.offset(m_ptOrg);

		SkPaint paint=m_paint;
		BYTE oldAlpha = paint.getAlpha();
		paint.setAlpha(byAlpha);
		m_SkCanvas->drawBitmapRectToRect(bmp,&skrcSrc,skrcDst,&paint);
		paint.setAlpha(oldAlpha);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::AlphaBlend( LPCRECT pRcDest,IRenderTarget *pRTSrc,LPCRECT pRcSrc,BYTE byAlpha )
	{
		IBitmapS *pBmp=(IBitmapS*) pRTSrc->GetCurrentObject(OT_BITMAP);
		if(!pBmp) return S_FALSE;
		RECT rcSrc = *pRcSrc;
		POINT ptSrcOrg;
		pRTSrc->GetViewportOrg(&ptSrcOrg);
		SRenderTarget_Skia * pRTSrc_Skia=(SRenderTarget_Skia*)pRTSrc;
		SkMatrix mtx = pRTSrc_Skia->m_SkCanvas->getTotalMatrix();
		if(!mtx.isIdentity())
		{
			SkRect rc=toSkRect(&rcSrc);
			mtx.mapRect(&rc);
			SkRect2RECT(rc,&rcSrc);
		}
		OffsetRect(&rcSrc,ptSrcOrg.x,ptSrcOrg.y);
		return DrawBitmapEx(pRcDest,pBmp,&rcSrc,MAKELONG(EM_STRETCH,kUndef_FilterLevel),byAlpha);
	}

	HRESULT SRenderTarget_Skia::DrawBitmapEx( LPCRECT pRcDest,const IBitmapS *pBitmap,LPCRECT pRcSrc,UINT expendMode, BYTE byAlpha/*=0xFF*/ )
	{
		UINT expendModeLow = LOWORD(expendMode);

		const SkMatrix & m = m_SkCanvas->getTotalMatrix();

		if(m.isIdentity() && ( expendModeLow == EM_NULL || (RectWid(pRcDest)==RectWid(pRcSrc) && RectHei(pRcDest)==RectHei(pRcSrc))))
			return DrawBitmap(pRcDest,pBitmap,pRcSrc->left,pRcSrc->top,byAlpha);

		SBitmap_Skia *pBmp = (SBitmap_Skia*)pBitmap;
		const SkBitmap & bmp=pBmp->GetSkBitmap();

		RECT rcSour={0,0,bmp.width(),bmp.height()};
		if(!pRcSrc) pRcSrc = &rcSour;
		SkRect rcSrc = toSkRect(pRcSrc);
		SkRect rcDest= toSkRect(pRcDest);
		rcDest.offset(m_ptOrg);

		SkPaint paint=m_paint;
		BYTE oldAlpha=paint.getAlpha();
		paint.setAlpha(byAlpha);

		SkPaint::FilterLevel fl = (SkPaint::FilterLevel)HIWORD(expendMode);//SkPaint::kNone_FilterLevel;
		if(SOUI::FilterLevel(fl) != SOUI::kUndef_FilterLevel)
			paint.setFilterLevel(fl);

		if(expendModeLow == EM_STRETCH)
		{
			m_SkCanvas->drawBitmapRectToRect(bmp,&rcSrc,rcDest,&paint);
		}else
		{
			PushClipRect(pRcDest,RGN_AND);

			SkIRect rcSrc = toSkIRect(pRcSrc);
			SkRect rcSubDest={0.0f,0.0f,(float)rcSrc.width(),(float)rcSrc.height()};
			for(float y=rcDest.fTop;y<rcDest.fBottom;y+=rcSrc.height())
			{
				rcSubDest.offsetTo(rcDest.fLeft,y);               
				for(float x=rcDest.fLeft;x<rcDest.fRight;x += rcSrc.width())
				{
					m_SkCanvas->drawBitmapRect(bmp,&rcSrc,rcSubDest,&paint);
					rcSubDest.offset((float)rcSrc.width(),0.0f);
				}
			}

			PopClip();
		}
		paint.setAlpha(oldAlpha);
		return S_OK;

	}


	HRESULT SRenderTarget_Skia::DrawBitmap9Patch( LPCRECT pRcDest,const IBitmapS *pBitmap,LPCRECT pRcSrc,LPCRECT pRcSourMargin,UINT expendMode,BYTE byAlpha/*=0xFF*/ )
	{
		LONG xDest[4] = {pRcDest->left,pRcDest->left+pRcSourMargin->left,pRcDest->right-pRcSourMargin->right,pRcDest->right};
		LONG xSrc[4] = {pRcSrc->left,pRcSrc->left+pRcSourMargin->left,pRcSrc->right-pRcSourMargin->right,pRcSrc->right};
		LONG yDest[4] = {pRcDest->top,pRcDest->top+pRcSourMargin->top,pRcDest->bottom-pRcSourMargin->bottom,pRcDest->bottom};
		LONG ySrc[4] = {pRcSrc->top,pRcSrc->top+pRcSourMargin->top,pRcSrc->bottom-pRcSourMargin->bottom,pRcSrc->bottom};

		//首先保证九宫分割正常
		if(!(xSrc[0] <= xSrc[1] && xSrc[1] <= xSrc[2] && xSrc[2] <= xSrc[3])) return S_FALSE;
		if(!(ySrc[0] <= ySrc[1] && ySrc[1] <= ySrc[2] && ySrc[2] <= ySrc[3])) return S_FALSE;

		//调整目标位置
		int nDestWid=pRcDest->right-pRcDest->left;
		int nDestHei=pRcDest->bottom-pRcDest->top;

		if((pRcSourMargin->left + pRcSourMargin->right) > nDestWid)
		{//边缘宽度大于目标宽度的处理
			if(pRcSourMargin->left >= nDestWid)
			{//只绘制左边部分
				xSrc[1] = xSrc[2] = xSrc[3] = xSrc[0]+nDestWid;
				xDest[1] = xDest[2] = xDest[3] = xDest[0]+nDestWid;
			}else if(pRcSourMargin->right >= nDestWid)
			{//只绘制右边部分
				xSrc[0] = xSrc[1] = xSrc[2] = xSrc[3]-nDestWid;
				xDest[0] = xDest[1] = xDest[2] = xDest[3]-nDestWid;
			}else
			{//先绘制左边部分，剩余的用右边填充
				int nRemain=xDest[3]-xDest[1];
				xSrc[2] = xSrc[3]-nRemain;
				xDest[2] = xDest[3]-nRemain;
			}
		}

		if(pRcSourMargin->top + pRcSourMargin->bottom > nDestHei)
		{
			if(pRcSourMargin->top >= nDestHei)
			{//只绘制上边部分
				ySrc[1] = ySrc[2] = ySrc[3] = ySrc[0]+nDestHei;
				yDest[1] = yDest[2] = yDest[3] = yDest[0]+nDestHei;
			}else if(pRcSourMargin->bottom >= nDestHei)
			{//只绘制下边部分
				ySrc[0] = ySrc[1] = ySrc[2] = ySrc[3]-nDestHei;
				yDest[0] = yDest[1] = yDest[2] = yDest[3]-nDestHei;
			}else
			{//先绘制左边部分，剩余的用右边填充
				int nRemain=yDest[3]-yDest[1];
				ySrc[2] = ySrc[3]-nRemain;
				yDest[2] = yDest[3]-nRemain;
			}
		}

		//定义绘制模式
		UINT mode[3][3]={
			{EM_NULL,expendMode,EM_NULL},
			{expendMode,expendMode,expendMode},
			{EM_NULL,expendMode,EM_NULL}
		};

		for(int y=0;y<3;y++)
		{
			if(ySrc[y] == ySrc[y+1]) continue;
			for(int x=0;x<3;x++)
			{
				if(xSrc[x] == xSrc[x+1]) continue;
				RECT rcSrc = {xSrc[x],ySrc[y],xSrc[x+1],ySrc[y+1]};
				RECT rcDest ={xDest[x],yDest[y],xDest[x+1],yDest[y+1]};
				DrawBitmapEx(&rcDest,pBitmap,&rcSrc,mode[y][x],byAlpha);
			}
		}

		return S_OK;
	}

	IRenderObj * SRenderTarget_Skia::GetCurrentObject( OBJTYPE uType )
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


	HRESULT SRenderTarget_Skia::SelectDefaultObject(OBJTYPE objType,IRenderObj ** ppOldObj /*= NULL*/)
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

	HRESULT SRenderTarget_Skia::SelectObject( IRenderObj *pObj,IRenderObj ** ppOldObj /*= NULL*/ )
	{
		SAutoRefPtr<IRenderObj> pRet;
		switch(pObj->ObjectType())
		{
		case OT_BITMAP: 
			pRet=m_curBmp;
			m_curBmp=(SBitmap_Skia*)pObj;
			//重新生成clip
			SASSERT(m_SkCanvas);
			delete m_SkCanvas;
			m_SkCanvas = new SkCanvas(m_curBmp->GetSkBitmap());
			break;
		case OT_PEN:
			pRet=m_curPen;
			m_curPen=(SPen_Skia*)pObj;
			break;
		case OT_BRUSH:
			pRet=m_curBrush;
			m_curBrush=(SBrush_Skia*)pObj;
			break;
		case OT_FONT:
			pRet=m_curFont;
			m_curFont=(SFont_Skia*)pObj;
			break;
		}
		if(pRet && ppOldObj)
		{//由调用者调用Release释放该RenderObj
			pRet->AddRef();
			*ppOldObj = pRet;
		}
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::OffsetViewportOrg( int xOff, int yOff, LPPOINT lpPoint )
	{
		SkPoint ptOff={(SkScalar)xOff,(SkScalar)yOff};
		if(lpPoint)
		{
			lpPoint->x = (LONG)m_ptOrg.fX;
			lpPoint->y = (LONG)m_ptOrg.fY;
		}
		m_ptOrg.offset(ptOff.fX,ptOff.fY);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::GetViewportOrg( LPPOINT lpPoint )
	{
		if(lpPoint)
		{
			lpPoint->x = (LONG)m_ptOrg.fX;
			lpPoint->y = (LONG)m_ptOrg.fY;
		}
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::SetViewportOrg( POINT pt )
	{
		m_ptOrg.fX = SkIntToScalar(pt.x);
		m_ptOrg.fY = SkIntToScalar(pt.y);
		return S_OK;
	}


	HDC SRenderTarget_Skia::GetDC( UINT uFlag )
	{
		if(m_hGetDC) 
		{
			m_nGetDC++;
			return m_hGetDC;
		}

		HBITMAP bmp=m_curBmp->GetGdiBitmap();//bmp可能为NULL
		HDC hdc_desk = ::GetDC(0);
		m_hGetDC = CreateCompatibleDC(hdc_desk);
		::ReleaseDC(0,hdc_desk);

		::SelectObject(m_hGetDC,bmp);	
		if(m_SkCanvas->isClipEmpty())
		{
			::IntersectClipRect(m_hGetDC,0,0,0,0);
		}else if(m_SkCanvas->isClipRect())
		{
			SkRect rcClip;
			m_SkCanvas->getClipBounds(&rcClip);
			RECT rc = {(int)rcClip.left(),(int)rcClip.top(),(int)rcClip.right(),(int)rcClip.bottom()};
			::InflateRect(&rc,-1,-1);//注意需要向内缩小一个象素
			::IntersectClipRect(m_hGetDC,rc.left,rc.top,rc.right,rc.bottom);
		}else
		{
			SkRegion rgn = m_SkCanvas->getTotalClip();
			SkRegion::Iterator it(rgn);
			int nCount=0;
			for(;!it.done();it.next())
			{
				nCount++;
			}
			it.rewind();

			int nSize=sizeof(RGNDATAHEADER)+nCount*sizeof(RECT);
			RGNDATA *rgnData=(RGNDATA*)malloc(nSize);
			memset(rgnData,0,nSize);
			rgnData->rdh.dwSize= sizeof(RGNDATAHEADER);
			rgnData->rdh.iType = RDH_RECTANGLES;
			rgnData->rdh.nCount=nCount;
			rgnData->rdh.rcBound.right=m_curBmp->Width();
			rgnData->rdh.rcBound.bottom=m_curBmp->Height();

			nCount=0;
			LPRECT pRc=(LPRECT)rgnData->Buffer;
			for(;!it.done();it.next())
			{
				SkIRect skrc=it.rect();
				RECT rc = {skrc.fLeft,skrc.fTop,skrc.fRight,skrc.fBottom};
				pRc[nCount++]= rc;
			}

			HRGN hRgn=ExtCreateRegion(NULL,nSize,rgnData);
			free(rgnData);
			::SelectClipRgn(m_hGetDC,hRgn);
			DeleteObject(hRgn);
		}

		::SetGraphicsMode(m_hGetDC,GM_ADVANCED);
		::SetViewportOrgEx(m_hGetDC,(int)m_ptOrg.x(),(int)m_ptOrg.y(),NULL);

		SkMatrix mtx=m_SkCanvas->getTotalMatrix();
		XFORM xForm = { mtx.get(kMScaleX),mtx.get(kMSkewY),
			mtx.get(kMSkewX),mtx.get(kMScaleY),
			mtx.get(kMTransX),mtx.get(kMTransY) };
		::SetWorldTransform(m_hGetDC,&xForm);

		m_nGetDC = 1;
		return m_hGetDC;
	}

	void SRenderTarget_Skia::ReleaseDC( HDC hdc ,LPCRECT pRc)
	{
		if(hdc == m_hGetDC)
		{
			if(--m_nGetDC == 0){
				DeleteDC(hdc);
				m_hGetDC = 0;
			}
		}
	}

	static bool fequal(float a,float b)
	{
		return fabs(a-b)< 0.0000001f;
	}

	static SkShader * CreateShader(const SkRect &skrc,const GradientInfo * info,const GradientItem *pGradients,int nCount,BYTE byAlpha,SkShader::TileMode tileMode){
		SkColor stack_colors[3];
		SkScalar stack_pos[3];
		SkColor *skColors= nCount<=3?stack_colors:new SkColor[nCount];
		SkScalar *pos = nCount<=3?stack_pos: new SkScalar[nCount];
		for(int i=0;i<nCount;i++){
			skColors[i]=SColor(pGradients[i].cr,byAlpha).toARGB();
			pos[i]=pGradients[i].pos;
		}

		SkScalar wid = skrc.width();
		SkScalar hei = skrc.height();
		SkScalar halfWid = wid/2;
		SkScalar halfHei = hei/2;
		SkShader * pShader = NULL;
		if(info->type == linear)
		{
			SkPoint skPts[2];
			if(fequal(info->angle,90.0f) || fequal(info->angle,270.0f))
			{//90度
				skPts[0].set(halfWid,0.0f);
				skPts[1].set(halfWid,hei);
			}else if(fequal(info->angle,0.0f) || fequal(info->angle,180.0f))
			{//水平方向
				skPts[0].set(0.f ,halfHei);
				skPts[1].set(wid,halfHei);
			}else
			{//其它角度

				float angleInRadians = PI*info->angle/180;
				double tanAngle = tan(angleInRadians);

				SkPoint pt1a,pt2a;//与左右两条边相交的位置
				SkPoint pt1b,pt2b;//与上下两条边相关的位置

				pt1a.fX=-halfWid,pt2a.fX=halfWid;
				pt1b.fY=-halfHei,pt2b.fY=halfHei;

				pt1a.fY= (SkScalar)(-halfWid*tanAngle);
				pt2a.fY = -pt1a.fY;

				pt1b.fX = (SkScalar)(-halfHei/tanAngle);
				pt2b.fX = -pt1b.fX;

				if(pt2a.fY > halfHei)
				{//using pt1a,pt2a
					skPts[0] = pt1a;
					skPts[1] = pt2a;
				}else
				{//using pt1b,pt2b
					skPts[0]=pt1b;
					skPts[1]=pt2b;
				}
				skPts[0].offset(halfWid,halfHei);
                skPts[1].offset(halfWid,halfHei);
			}
			SkPoint::Offset(skPts,2,skrc.fLeft,skrc.fTop);
			pShader = SkGradientShader::CreateLinear(skPts, skColors, pos,nCount,tileMode);
		}else if(info->type == radial)
		{
			SkPoint skCenter;
			skCenter.set(wid*info->radial.centerX,hei*info->radial.centerY);
			SkPoint::Offset(&skCenter,1,skrc.fLeft,skrc.fTop);
			pShader = SkGradientShader::CreateRadial(skCenter,info->radial.radius ,skColors, pos,nCount,tileMode);
		}else if(info->type==sweep)
		{
			SkScalar centerX = skrc.fLeft + skrc.width() * info->sweep.centerX;
			SkScalar centerY = skrc.fTop+skrc.height()*info->sweep.centerY;
			pShader = SkGradientShader::CreateSweep(centerX,centerY, skColors, pos,nCount);
		}
		if(nCount>3){
			delete []skColors;
			delete []pos;
		}
		return pShader;
	}


	HRESULT SRenderTarget_Skia::DrawGradientRectEx(THIS_ LPCRECT pRect, POINT ptRoundCorner, const GradientItem *pGradients, int nCount, const GradientInfo *info, BYTE byAlpha)
	{
		SkRect skrc = toSkRect(pRect);
		skrc.offset(m_ptOrg);
		SkShader *pShader = CreateShader(skrc,info,pGradients,nCount,byAlpha,SkShader::kRepeat_TileMode);
		if(!pShader)
		{
			return E_INVALIDARG;
		}

		SkPaint paint=m_paint;
		paint.setShader(pShader)->unref();
		if(ptRoundCorner.x==0 && ptRoundCorner.y==0)
			m_SkCanvas->drawRect(skrc,paint);
		else
			m_SkCanvas->drawRoundRect(skrc,(SkScalar)ptRoundCorner.x,(SkScalar)ptRoundCorner.y,paint);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::DrawGradientPath(THIS_ const IPathS* path, const GradientItem *pGradients, int nCount, const GradientInfo *info,BYTE byAlpha)
	{
		RECT rc={0};
		path->getBounds(&rc);
		SkRect skrc = toSkRect(&rc);
		skrc.offset(m_ptOrg);
		SkShader *pShader = CreateShader(skrc,info,pGradients,nCount,byAlpha,SkShader::kClamp_TileMode);
		if(!pShader)
		{
			return E_INVALIDARG;
		}

		const SPath_Skia * path2 = (const SPath_Skia *)path;
		SkPaint paint=m_paint;

		SLineDashEffect skDash(m_curPen->GetStyle());
		paint.setPathEffect(skDash.Get());
		SStrokeCap strokeCap(m_curPen->GetStyle());
		paint.setStrokeCap(strokeCap.Get());
		SStrokeJoin strokeJoin(m_curPen->GetStyle());
		paint.setStrokeJoin(strokeJoin.Get());
		paint.setStyle(SkPaint::kStroke_Style);
		if(m_bAntiAlias)
		{
			paint.setStrokeWidth((SkScalar)m_curPen->GetWidth()-0.5f);
		}else
		{
			paint.setStrokeWidth((SkScalar)m_curPen->GetWidth());
		}
		paint.setShader(pShader)->unref();

		SkPath skPath;
		path2->m_skPath.offset(m_ptOrg.fX,m_ptOrg.fY,&skPath);
		m_SkCanvas->drawPath(skPath,paint);

		return S_OK;
	}

	HRESULT SRenderTarget_Skia::DrawGradientRect(THIS_ LPCRECT pRect,  BOOL bVert, POINT ptRoundCorner, const GradientItem *pGradients, int nCount, BYTE byAlpha){
		GradientInfo info={linear,{bVert?90.0f:0.0f}};
		return DrawGradientRectEx(pRect,ptRoundCorner,pGradients,nCount,&info,byAlpha);
	}

	HRESULT SRenderTarget_Skia::FillSolidRect( LPCRECT pRect,COLORREF cr )
	{
		SkPaint paint=m_paint;
		paint.setStyle(SkPaint::kFill_Style);
		paint.setColor(SColor(cr).toARGB());

		SkRect skrc=toSkRect(pRect);
		skrc.offset(m_ptOrg);
		m_SkCanvas->drawRect(skrc,paint);
		return S_OK;    
	}

	HRESULT SRenderTarget_Skia::ClearRect( LPCRECT pRect,COLORREF cr )
	{
		SkPaint paint=m_paint;
		paint.setStyle(SkPaint::kFill_Style);
		paint.setColor(SColor(cr).toARGB());
		paint.setXfermodeMode(SkXfermode::kSrc_Mode);

		SkRect skrc=toSkRect(pRect);
		skrc.offset(m_ptOrg);
		m_SkCanvas->drawRect(skrc,paint);
		return S_OK;    
	}

	HRESULT SRenderTarget_Skia::InvertRect(LPCRECT pRect)
	{
		SkPaint paint=m_paint;
		paint.setStyle(SkPaint::kFill_Style);
		paint.setXfermode(new ProcXfermode(ProcXfermode::Rop2_Invert));
		SkRect skrc = toSkRect(pRect);
		skrc.offset(m_ptOrg);
		m_SkCanvas->drawRect(skrc,paint);
		return S_OK;  
	}

	HRESULT SRenderTarget_Skia::DrawEllipse( LPCRECT pRect )
	{
		SkPaint paint=m_paint;
		paint.setColor(SColor(m_curPen->GetColor()).toARGB());
		SLineDashEffect skDash(m_curPen->GetStyle());
		paint.setPathEffect(skDash.Get());
		paint.setStyle(SkPaint::kStroke_Style);
		if(m_bAntiAlias)
		{
			paint.setStrokeWidth((SkScalar)m_curPen->GetWidth()-0.5f);
		}else
		{
			paint.setStrokeWidth((SkScalar)m_curPen->GetWidth());
		}

		SkRect skrc = toSkRect(pRect);
		skrc.offset(m_ptOrg);
		m_SkCanvas->drawOval(skrc,paint);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::FillEllipse( LPCRECT pRect )
	{
		SkPaint paint=m_paint;
		paint.setStyle(SkPaint::kFill_Style);
		SkRect skrc=toSkRect(pRect);
		skrc.offset(m_ptOrg);
		m_curBrush->InitPaint(paint,skrc);
		m_SkCanvas->drawOval(skrc,paint);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::FillSolidEllipse(LPCRECT pRect,COLORREF cr)
	{
		SkPaint paint=m_paint;
		paint.setFilterLevel(SkPaint::kNone_FilterLevel);
		paint.setColor(SColor(cr).toARGB());
		paint.setStyle(SkPaint::kFill_Style);

		SkRect skrc=toSkRect(pRect);
		skrc.offset(m_ptOrg);
		m_SkCanvas->drawOval(skrc,paint);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::DrawArc( LPCRECT pRect,float startAngle,float sweepAngle,BOOL useCenter )
	{
		SkPaint paint=m_paint;
		paint.setColor(SColor(m_curPen->GetColor()).toARGB());
		SLineDashEffect skDash(m_curPen->GetStyle());
		paint.setPathEffect(skDash.Get());
		SStrokeCap strokeCap(m_curPen->GetStyle());
		paint.setStrokeCap(strokeCap.Get());
		paint.setStyle(SkPaint::kStroke_Style);
		if(m_bAntiAlias)
		{
			paint.setStrokeWidth((SkScalar)m_curPen->GetWidth()-0.5f);
		}else
		{
			paint.setStrokeWidth((SkScalar)m_curPen->GetWidth());
		}

		SkRect skrc = toSkRect(pRect);
		skrc.offset(m_ptOrg);
		m_SkCanvas->drawArc(skrc,startAngle,sweepAngle,!!useCenter,paint);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::DrawArc2(LPCRECT pRect, float startAngle, float sweepAngle, int width)
	{
		SkPaint paint = m_paint;
		paint.setStrokeCap(SkPaint::kButt_Cap);
		paint.setStyle(SkPaint::kStroke_Style);
		if (m_bAntiAlias)
		{
			paint.setStrokeWidth((SkScalar)width - 0.5f);
		}
		else
		{
			paint.setStrokeWidth((SkScalar)width);
		}
		
		SkRect skrc = toSkRect(pRect);
		skrc.offset(m_ptOrg);
		m_SkCanvas->translate(skrc.centerX(), skrc.centerY());
		m_SkCanvas->rotate(startAngle);
		SkRect skrc2 = skrc;
		skrc2.offset(-skrc.centerX(), -skrc.centerY());
		m_curBrush->InitPaint(paint, skrc2);
		
		if (!m_curBrush->IsFullArc()) {
			SkMatrix matrix;
			matrix.setScale(sweepAngle / 360.0f, 1.0f);
			paint.getShader()->setLocalMatrix(&matrix);
		}
		m_SkCanvas->drawArc(skrc2, 0, sweepAngle, false, paint);
		m_SkCanvas->rotate(-startAngle);
		m_SkCanvas->translate(-skrc.centerX(), -skrc.centerY());
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::FillArc( LPCRECT pRect,float startAngle,float sweepAngle )
	{
		SkPaint paint=m_paint;
		paint.setStyle(SkPaint::kFill_Style);
		SkRect skrc=toSkRect(pRect);
		skrc.offset(m_ptOrg);
		m_curBrush->InitPaint(paint,skrc);
		m_SkCanvas->drawArc(skrc,startAngle, sweepAngle,true,paint);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::SetTransform(const float matrix[9], float oldMatrix[9])
	{
		SASSERT(matrix);
		if(oldMatrix) GetTransform(oldMatrix);
		SkMatrix m;
		m.setAll(matrix[kMScaleX], matrix[kMSkewX],matrix[kMTransX],
			matrix[kMSkewY],matrix[kMScaleY],matrix[kMTransY],
			matrix[kMPersp0], matrix[kMPersp1], matrix[kMPersp2]
		);
		m.preTranslate(-m_ptOrg.fX, -m_ptOrg.fY);
		m.postTranslate(m_ptOrg.fX, m_ptOrg.fY);
		m_SkCanvas->setMatrix(m);
		if(m.isIdentity()){
			m_paint.setFilterLevel(SkPaint::kNone_FilterLevel);
		}else{
			m_paint.setFilterLevel(SkPaint::kHigh_FilterLevel);
		}
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::GetTransform(float matrix[9]) const
	{
		SASSERT(matrix);
		const SkMatrix & m = m_SkCanvas->getTotalMatrix();
		matrix[kMScaleX] = m.getScaleX();
		matrix[kMSkewX] = m.getSkewX();
		matrix[kMTransX] = m.getTranslateX();
		matrix[kMSkewY] = m.getSkewY();
		matrix[kMScaleY] = m.getScaleY();
		matrix[kMTransY] = m.getTranslateY();
		matrix[kMPersp0] = m.getPerspX();
		matrix[kMPersp1] = m.getPerspY();
		matrix[kMPersp2] = m.get(SkMatrix::kMPersp2);

		return S_OK;
	}

	COLORREF SRenderTarget_Skia::GetPixel( int x, int y )
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

	COLORREF SRenderTarget_Skia::SetPixel( int x, int y, COLORREF cr )
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

	HRESULT SRenderTarget_Skia::PushClipPath(const IPathS * path, UINT mode, BOOL doAntiAlias /*= false*/)
	{
		const SPath_Skia * path2 = (const SPath_Skia *)path;
		m_SkCanvas->clipPath(path2->m_skPath,SRegion_Skia::RGNMODE2SkRgnOP(mode),!!doAntiAlias);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::DrawPath(const IPathS * path, IPathEffect * pathEffect)
	{
		const SPath_Skia * path2 = (const SPath_Skia *)path;

		SkPaint paint=m_paint;
		paint.setColor(SColor(m_curPen->GetColor()).toARGB());
		SLineDashEffect skDash(m_curPen->GetStyle());
		paint.setPathEffect(skDash.Get());
		SStrokeCap strokeCap(m_curPen->GetStyle());
		paint.setStrokeCap(strokeCap.Get());
		SStrokeJoin strokeJoin(m_curPen->GetStyle());
		paint.setStrokeJoin(strokeJoin.Get());
		paint.setStyle(SkPaint::kStroke_Style);
		if(m_bAntiAlias)
		{
			paint.setStrokeWidth((SkScalar)m_curPen->GetWidth()-0.5f);
		}else
		{
			paint.setStrokeWidth((SkScalar)m_curPen->GetWidth());
		}

		if(pathEffect!=NULL)
		{
			SkPathEffect * skPathEffect = (SkPathEffect*)pathEffect->GetRealPathEffect();
			paint.setPathEffect(skPathEffect);
		}
		SkPath skPath;
		path2->m_skPath.offset(m_ptOrg.fX,m_ptOrg.fY,&skPath);
		m_SkCanvas->drawPath(skPath,paint);

		return S_OK;
	}

	HRESULT SRenderTarget_Skia::FillPath(const IPathS * path)
	{
		const SPath_Skia * path2 = (const SPath_Skia *)path;

		SkPaint paint=m_paint;
		paint.setStyle(SkPaint::kFill_Style);

		SkPath skPath;
		path2->m_skPath.offset(m_ptOrg.fX,m_ptOrg.fY,&skPath);
		const SkRect &rcBound = skPath.getBounds();
		m_curBrush->InitPaint(paint,rcBound);

		m_SkCanvas->drawPath(skPath,paint);

		return S_OK;
	}

	HRESULT SRenderTarget_Skia::SetXfermode(int mode,int *pOldMode/* =NULL */)
	{
		if(pOldMode) *pOldMode = m_xferMode;
		m_xferMode = mode;
		SetPaintXferMode(m_paint,m_xferMode);
		return S_OK;
	}

	bool SRenderTarget_Skia::SetPaintXferMode(SkPaint & paint,int nRopMode)
	{
		bool bRet = true;
		switch(nRopMode)
		{
		case kSrcCopy:
			paint.setXfermodeMode(SkXfermode::kSrc_Mode);
			break;
		case kDstInvert:
			paint.setXfermode(new ProcXfermode(ProcXfermode::Rop2_Invert));
			break;
		case kSrcInvert:
			paint.setXfermode(new ProcXfermode(ProcXfermode::Rop2_Xor));
			break;
		case kSrcAnd:
			paint.setXfermode(new ProcXfermode(ProcXfermode::Rop2_And));
			break;
		default:
			if(nRopMode >= kClear_Mode && nRopMode<=kLuminosity_Mode)
				paint.setXfermodeMode((SkXfermode::Mode)nRopMode);
			else
				bRet = false;
			break;
		}
		return bRet;
	}

	BOOL SRenderTarget_Skia::SetAntiAlias(BOOL bAntilias)
	{
		BOOL bRet = m_bAntiAlias;
		m_bAntiAlias = !!bAntilias;
		m_paint.setAntiAlias(m_bAntiAlias);
		return bRet;
	}

	BOOL SRenderTarget_Skia::GetAntiAlias(THIS) SCONST
	{
		return m_bAntiAlias;
	}

	COLORREF SRenderTarget_Skia::GetTextColor()
	{
		return m_curColor.toCOLORREF();
	}

	COLORREF SRenderTarget_Skia::SetTextColor(COLORREF color)
	{
		COLORREF crOld=m_curColor.toCOLORREF();
		m_curColor.setRGB(color);
		m_paint.setColor(m_curColor.toARGB());
		return crOld;
	}

	IMaskFilter* SRenderTarget_Skia::GetMaskFilter()
	{
		return m_curMaskFilter;
	}

	void SRenderTarget_Skia::SetMaskFilter(IMaskFilter *pMaskFilter)
	{
		m_curMaskFilter = pMaskFilter;
		SMaskFilter_Skia *pMaskFilter2 = (SMaskFilter_Skia*)pMaskFilter;
		if(pMaskFilter2)
		{
			m_paint.setMaskFilter(pMaskFilter2->m_maskFilter);
		}
		else
		{
			m_paint.setMaskFilter(NULL);
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// SBitmap_Skia
	static int s_cBmp = 0;
	SBitmap_Skia::SBitmap_Skia( IRenderFactory *pRenderFac ) :TSkiaRenderObjImpl<IBitmapS,OT_BITMAP>(pRenderFac),m_hBmp(0)
	{
		//         STRACE(L"bitmap new; objects = %d",++s_cBmp);
	}

	SBitmap_Skia::~SBitmap_Skia()
	{
		m_bitmap.reset();
		if(m_hBmp) DeleteObject(m_hBmp);
		//         STRACE(L"bitmap delete objects = %d",--s_cBmp);
	}

	HBITMAP SBitmap_Skia::CreateGDIBitmap( int nWid,int nHei,void ** ppBits )
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

	HRESULT SBitmap_Skia::Init( int nWid,int nHei ,const LPVOID pBits/*=NULL*/)
	{
		m_bitmap.reset();
		m_bitmap.setInfo(SkImageInfo::Make(nWid,nHei,kN32_SkColorType,kPremul_SkAlphaType));
		if(m_hBmp) DeleteObject(m_hBmp);

		LPVOID pBmpBits=NULL;
		m_hBmp=CreateGDIBitmap(nWid,nHei,&pBmpBits);
		if(!m_hBmp) return E_OUTOFMEMORY;
		if(pBits)
		{
			memcpy(pBmpBits,pBits,nWid*nHei*4);
		}else
		{
			memset(pBmpBits,0,nWid*nHei*4);
		}
		m_bitmap.setPixels(pBmpBits);
		return S_OK;
	}

	HRESULT SBitmap_Skia::Init2( IImgFrame *pFrame )
	{
		UINT uWid=0,uHei =0;
		pFrame->GetSize(&uWid,&uHei);

		if(m_hBmp) DeleteObject(m_hBmp);
		m_bitmap.reset();
		m_bitmap.setInfo(SkImageInfo::Make(uWid, uHei,kN32_SkColorType,kPremul_SkAlphaType));
		void * pBits=NULL;
		m_hBmp = CreateGDIBitmap(uWid,uHei,&pBits);

		if(!m_hBmp) return E_OUTOFMEMORY;
		m_bitmap.setPixels(pBits);

		memcpy(m_bitmap.getPixels(),pFrame->GetPixels(), m_bitmap.rowBytes() * uHei);
		return S_OK;
	}

	HRESULT SBitmap_Skia::LoadFromFile( LPCTSTR pszFileName)
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

	HRESULT SBitmap_Skia::LoadFromMemory(LPBYTE pBuf,size_t szLen)
	{
		SAutoRefPtr<IImgX> imgDecoder;
		GetRenderFactory()->GetImgDecoderFactory()->CreateImgX(&imgDecoder);
		if(imgDecoder->LoadFromMemory(pBuf,szLen)==0) return S_FALSE;
		return ImgFromDecoder(imgDecoder);
	}

	HRESULT SBitmap_Skia::ImgFromDecoder(IImgX *imgDecoder)
	{
		IImgFrame *pFrame=imgDecoder->GetFrame(0);
		UINT uWid=0,uHei =0;
		pFrame->GetSize(&uWid,&uHei);

		if(m_hBmp) DeleteObject(m_hBmp);
		m_bitmap.reset();
		m_bitmap.setInfo(SkImageInfo::Make(uWid, uHei,kN32_SkColorType,kPremul_SkAlphaType));
		void * pBits=NULL;
		m_hBmp = CreateGDIBitmap(uWid,uHei,&pBits);

		if(!m_hBmp) return E_OUTOFMEMORY;
		m_bitmap.setPixels(pBits);

		memcpy(m_bitmap.getPixels(),pFrame->GetPixels(), m_bitmap.rowBytes() *uHei);
		return S_OK;
	}

	UINT SBitmap_Skia::Width()  const
	{
		return m_bitmap.width();
	}

	UINT SBitmap_Skia::Height() const
	{
		return m_bitmap.height();
	}

	SIZE SBitmap_Skia::Size() const
	{
		SIZE sz={m_bitmap.width(),m_bitmap.height()};
		return sz;
	}

	LPVOID SBitmap_Skia::LockPixelBits()
	{
		return m_bitmap.getPixels();
	}

	void SBitmap_Skia::UnlockPixelBits( LPVOID pBuf)
	{
		BITMAP bm;
		GetObject(m_hBmp,sizeof(bm),&bm);
		memcpy(bm.bmBits,pBuf,Width()*Height()*4);
#ifndef _WIN32
		MarkDirty();
#endif
	}

	const LPVOID SBitmap_Skia::GetPixelBits() const
	{
		return m_bitmap.getPixels();
	}

	HRESULT SBitmap_Skia::Clone(IBitmapS **ppClone) const 
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

	HRESULT SBitmap_Skia::Scale(IBitmapS **ppOutput,int nScale,FilterLevel filterLevel) SCONST
	{
		int wid = MulDiv(Width(),nScale,100);
		int hei = MulDiv(Height(),nScale,100);
		return Scale2(ppOutput,wid,hei,filterLevel);
	}

	HRESULT SBitmap_Skia::Scale2(IBitmapS **pOutput,int nWid,int nHei,FilterLevel filterLevel) SCONST
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

	HRESULT SBitmap_Skia::Save(LPCWSTR pszFileName,const LPVOID pFormat) SCONST
	{
		LPBYTE pBits = (LPBYTE)GetPixelBits();
		return GetRenderFactory()->GetImgDecoderFactory()->SaveImage(pBits,Width(),Height(),pszFileName,pFormat);
	}

	HRESULT SBitmap_Skia::Save2(CTHIS_ LPCWSTR pszFileName, ImgFmt imgFmt) SCONST
	{
		LPBYTE pBits = (LPBYTE)GetPixelBits();
		return GetRenderFactory()->GetImgDecoderFactory()->SaveImage2(pBits,Width(),Height(),pszFileName,imgFmt);
	}

	//////////////////////////////////////////////////////////////////////////
	static int s_cRgn =0;
	SRegion_Skia::SRegion_Skia( IRenderFactory *pRenderFac )
		:TSkiaRenderObjImpl<IRegionS,OT_RGN>(pRenderFac)
	{
		//         STRACE(L"region new; objects = %d",++s_cRgn);
	}

	SRegion_Skia::~SRegion_Skia()
	{
		//         STRACE(L"region delete; objects = %d",--s_cRgn);
	}

	void SRegion_Skia::CombineRect( LPCRECT lprect,int nCombineMode )
	{
		m_rgn.op(toSkIRect(lprect),RGNMODE2SkRgnOP(nCombineMode));
	}

	void SRegion_Skia::CombineRoundRect(LPCRECT lprect, POINT ptRadius, int nCombineMode)
	{
		SkPath skPath;
		skPath.addRoundRect(toSkRect(lprect),SkScalar(ptRadius.x),SkScalar(ptRadius.y));
		SkRegion clip;
		clip.setRect(lprect->left,lprect->top,lprect->right,lprect->bottom);
		SkRegion rgn;
		rgn.setPath(skPath,clip);

		m_rgn.op(rgn,RGNMODE2SkRgnOP(nCombineMode));
	}

	void SRegion_Skia::CombineEllipse(LPCRECT lprect , int nCombineMode)
	{
		SkPath skPath;
		skPath.addOval(toSkRect(lprect));
		SkRegion clip;
		clip.setRect(lprect->left,lprect->top,lprect->right,lprect->bottom);
		SkRegion rgn;
		rgn.setPath(skPath,clip);

		m_rgn.op(rgn,RGNMODE2SkRgnOP(nCombineMode));
	}

	void SRegion_Skia::CombinePolygon(const POINT * pts, int count, int nPolygonMode, int nCombineMode)
	{
		SkPoint *spts = new SkPoint[count];
		RECT rc = { 0 };
		for (int i = 0; i < count; i++)
		{
			if (rc.left > pts[i].x) rc.left = pts[i].x;
			if (rc.right < pts[i].x) rc.right = pts[i].x;
			if (rc.top > pts[i].y) rc.top = pts[i].y;
			if (rc.bottom < pts[i].y) rc.bottom = pts[i].y;
			spts[i].iset(pts[i].x, pts[i].y);
		}
		SkPath skPath;
		skPath.addPoly(spts, count, true);
		delete[]spts;

		SkRegion clip;
		clip.setRect(rc.left, rc.top, rc.right, rc.bottom);
		SkRegion rgn;
		rgn.setPath(skPath, clip);

		m_rgn.op(rgn, RGNMODE2SkRgnOP(nCombineMode));

	}

	void SRegion_Skia::CombineRgn(const IRegionS * pRgnSrc,int nCombineMode)
	{
		const SRegion_Skia * pRgnSrc2 = (const SRegion_Skia*)pRgnSrc;
		m_rgn.op(pRgnSrc2->GetRegion(),RGNMODE2SkRgnOP(nCombineMode));
	}

	BOOL SRegion_Skia::PtInRegion( POINT pt ) const
	{
		return m_rgn.contains(pt.x,pt.y);
	}

	BOOL SRegion_Skia::RectInRegion( LPCRECT lprect ) const
	{
		SASSERT(lprect);
		return m_rgn.intersects(toSkIRect(lprect));
	}

	void SRegion_Skia::GetRgnBox( LPRECT lprect ) const
	{
		SASSERT(lprect);
		SkIRect rc=m_rgn.getBounds();
		lprect->left=rc.left();
		lprect->top=rc.top();
		lprect->right=rc.right();
		lprect->bottom=rc.bottom();
	}

	BOOL SRegion_Skia::IsEmpty() const
	{
		return m_rgn.isEmpty();
	}

	void SRegion_Skia::Offset( POINT pt )
	{
		m_rgn.translate(pt.x,pt.y);
	}

	SkRegion SRegion_Skia::GetRegion() const
	{
		return m_rgn;
	}

	void SRegion_Skia::SetRegion( const SkRegion & rgn )
	{
		m_rgn=rgn;
	}

	SkRegion::Op SRegion_Skia::RGNMODE2SkRgnOP( UINT mode )
	{
		SkRegion::Op op = SkRegion::kReplace_Op;
		switch(mode)
		{
		case RGN_COPY: op = SkRegion::kReplace_Op;break;
		case RGN_AND: op = SkRegion::kIntersect_Op;break;
		case RGN_OR: op = SkRegion::kUnion_Op;break;
		case RGN_DIFF: op = SkRegion::kDifference_Op;break;
		case RGN_XOR: op = SkRegion::kXOR_Op;break;
		default:SASSERT(FALSE);break;
		}
		return op;
	}

	void SRegion_Skia::Clear()
	{
		m_rgn.setEmpty();
	}


	//////////////////////////////////////////////////////////////////////////
	// SFont_Skia

	SStringT FindPropValue(const SStringT & strProp, const SStringT & strKey)
	{
		SStringT strValue;
		int nPos1 = strProp.Find(strKey);
		if(nPos1!=-1)
		{
			nPos1+=strKey.GetLength();
			if(nPos1<strProp.GetLength() && strProp[nPos1]==_T(':'))
			{
				nPos1++;
				int nPos2=strProp.FindChar(_T(','),nPos1);
				if(nPos2==-1) nPos2=strProp.GetLength();
				strValue=strProp.Mid(nPos1,nPos2-nPos1);
			}
		}
		return strValue;
	}

	static int s_cFont =0;
	SFont_Skia::SFont_Skia( IRenderFactory * pRenderFac,const LOGFONT * plf) 
		:TSkiaRenderObjImpl<IFontS,OT_FONT>(pRenderFac)
		,m_skFont(NULL)
		,m_fillStyle(kStrokeAndFill_Style)
		,m_bLcdText(TRUE)
	{
		memcpy(&m_lf,plf,sizeof(LOGFONT));
#ifdef UNICODE
		SStringA strFace=S_CT2A(plf->lfFaceName,CP_UTF8);
#else
		SStringA strFace=S_CT2A(plf->lfFaceName,CP_ACP);
#endif
		BYTE style=SkTypeface::kNormal;
		if(plf->lfItalic) style |= SkTypeface::kItalic;
		if(plf->lfWeight == FW_BOLD) style |= SkTypeface::kBold;

		m_skFont=SkTypeface::CreateFromName(strFace,(SkTypeface::Style)style,plf->lfCharSet);
		//         STRACE(L"font new: objects = %d", ++s_cFont);
	}

	SFont_Skia::~SFont_Skia()
	{
		if(m_skFont) m_skFont->unref();
		//         STRACE(L"font delete: objects = %d", --s_cFont);
	}


	void SFont_Skia::SetProp(THIS_ IXmlNode *pXmlNode)
	{
		if(!pXmlNode)
			return;
		BlurStyle blurStyle = kNone_BLurStyle;
		BlurFlags blurFlags = kNone_BlurFlag;
		float blurRadius = 0.0f;

		IXmlAttr *pAttr = pXmlNode->FirstAttribute();
		while(pAttr)
		{
			LPCWSTR attrName = pAttr->Name();
			LPCWSTR attrValue = pAttr->Value();
			if(wcsicmp(attrName,L"blurStyle")==0)
			{
				if(wcsicmp(attrValue,L"normal") == 0)
					blurStyle = kNormal_BlurStyle;
				else if(wcsicmp(attrValue,L"solid") == 0)
					blurStyle = kSolid_BlurStyle;
				else if(wcsicmp(attrValue,L"outer") == 0)
					blurStyle = kOuter_BlurStyle;
				else if(wcsicmp(attrValue,L"inner") == 0)
					blurStyle = kInner_BlurStyle;
			}else if(wcsicmp(attrName,L"blurRadius")==0)
			{
				blurRadius = (float)_wtof(attrValue);
			}else if(wcsicmp(attrName,L"blurFlags") == 0)
			{
				if(wcsicmp(attrValue,L"ignoreTransform") == 0)
					blurFlags = kIgnoreTransform_BlurFlag;
				else if(wcsicmp(attrValue,L"HighQuality") == 0)
					blurFlags = kHighQuality_BlurFlag;
				else if(wcsicmp(attrValue,L"All") == 0)
					blurFlags = kAll_BlurFlag;
			}
			else if(wcsicmp(attrName,L"style")==0)
			{
				if(wcsicmp(attrValue,L"strokeAndFill")==0)
					m_fillStyle = kStrokeAndFill_Style;
				else if(wcsicmp(attrValue,L"fill")!=0)
					m_fillStyle = kFill_Style;
				else if(wcsicmp(attrValue,L"stroke")==0)
					m_fillStyle =kStroke_Style;
			}else if(wcsicmp(attrName,L"lcdText")==0)
			{
				m_bLcdText = String2Bool(attrValue);
			}

			pAttr = pAttr->Next();
		}
		
		if(blurStyle != SOUI::kNone_BLurStyle && blurRadius > 0.0f)
		{
			GetRenderFactory()->CreateBlurMaskFilter(blurRadius,blurStyle,blurFlags,&m_maskFilter);
		}
	}

	BOOL SFont_Skia::UpdateFont(const LOGFONT *plf)
	{
		if(!m_skFont) return FALSE;

		memcpy(&m_lf,plf,sizeof(LOGFONT));

#ifdef UNICODE
		SStringA strFace=S_CT2A(plf->lfFaceName,CP_UTF8);
#else
		SStringA strFace=S_CT2A(plf->lfFaceName,CP_ACP);
#endif
		BYTE style=SkTypeface::kNormal;
		if(plf->lfItalic) style |= SkTypeface::kItalic;
		if(plf->lfWeight == FW_BOLD) style |= SkTypeface::kBold;
		m_skFont->unref();
		m_skFont=SkTypeface::CreateFromName(strFace,(SkTypeface::Style)style,plf->lfCharSet);

		return TRUE;
	}

	BOOL SFont_Skia::IsStrikeOut() const
	{
		return m_lf.lfStrikeOut;
	}

	BOOL SFont_Skia::IsItalic() const
	{
		return m_lf.lfItalic;
	}

	BOOL SFont_Skia::IsUnderline() const
	{
		return m_lf.lfUnderline;
	}

	BOOL SFont_Skia::IsBold() const
	{
		return m_lf.lfWeight == FW_BOLD;
	}

	int SFont_Skia::TextSize() const
	{
		return m_lf.lfHeight;
	}

	LPCTSTR SFont_Skia::FamilyName() const
	{
		return m_lf.lfFaceName;
	}

	const LOGFONT * SFont_Skia::LogFont() const
	{
		return &m_lf;
	}

	//////////////////////////////////////////////////////////////////////////

	SBrush_Skia::SBrush_Skia(IRenderFactory * pRenderFac,const GradientItem *pGradients, int nCount, const GradientInfo *info, BYTE byAlpha,TileMode tileMode) 
		:TSkiaRenderObjImpl<IBrushS,OT_BRUSH>(pRenderFac),m_brushType(Brush_Shader)
	{
		m_tileMode = (SkShader::TileMode)tileMode;
		m_byAlpha = byAlpha;
		m_gradInfo = *info;
		m_arrGradItem.SetCount(nCount);
		memcpy(m_arrGradItem.GetData(),pGradients,sizeof(GradientItem)*nCount);
	}

	SBrush_Skia::SBrush_Skia(IRenderFactory * pRenderFac,SkBitmap bmp,TileMode xtm,TileMode ytm)
		:TSkiaRenderObjImpl<IBrushS,OT_BRUSH>(pRenderFac),m_brushType(Brush_Bitmap)
	{
		m_bmp = bmp;
		m_xtm = (SkShader::TileMode)xtm;
		m_ytm = (SkShader::TileMode)ytm;
	}

	SBrush_Skia::SBrush_Skia(IRenderFactory * pRenderFac,COLORREF cr) 
		:TSkiaRenderObjImpl<IBrushS,OT_BRUSH>(pRenderFac),m_brushType(Brush_Color)
	{
		SColor tmp(cr);
		m_cr = tmp.toARGB();
	}

	SBrush_Skia::~SBrush_Skia()
	{
	}

	void SBrush_Skia::InitPaint(SkPaint & paint,const SkRect & skrc)
	{
		if(m_brushType == Brush_Color)
		{
			paint.setFilterLevel(SkPaint::kNone_FilterLevel);
			paint.setColor(m_cr);
		}else if(m_brushType == Brush_Bitmap){
			SkMatrix mtx;
			mtx.setTranslate(skrc.fLeft,skrc.fTop);
			paint.setShader(SkShader::CreateBitmapShader(m_bmp,m_xtm,m_ytm,&mtx))->unref();
		}else//if(m_brushType == Brush_Shader)
		{
			SkShader *pShader = CreateShader(skrc,&m_gradInfo,m_arrGradItem.GetData(),m_arrGradItem.GetCount(),m_byAlpha,m_tileMode);
			if(!pShader)
				return;
			paint.setShader(pShader)->unref();
		}
	}


	BOOL SBrush_Skia::IsFullArc() const
	{
		if (m_gradInfo.type == sweep)
			return m_gradInfo.sweep.bFullArc;
		else
			return FALSE;
	}
	//////////////////////////////////////////////////////////////////////////
	static int s_cPath =0;
	SPath_Skia::SPath_Skia(IRenderFactory *pRenderFac)
		:TSkiaRenderObjImpl<IPathS,OT_PATH>(pRenderFac)
	{
		//         STRACE(L"path new; objects = %d",++s_cPath);
	}

	SPath_Skia::~SPath_Skia()
	{
		//         STRACE(L"path delete; objects = %d",--s_cPath);

	}

	FillType SPath_Skia::getFillType() const
	{
		return (FillType)m_skPath.getFillType();
	}

	void SPath_Skia::setFillType(FillType ft)
	{
		m_skPath.setFillType((SkPath::FillType)ft);
	}

	void SPath_Skia::reset()
	{
		m_skPath.reset();
	}

	BOOL SPath_Skia::isEmpty() const
	{
		return m_skPath.isEmpty();
	}

	void SPath_Skia::getBounds(LPRECT prc) const
	{
		const SkRect &rc = m_skPath.getBounds();
		SkRect2RECT(rc,prc);
	}

	void SPath_Skia::moveTo(float x, float y)
	{
		m_skPath.moveTo(x,y);
	}

	void SPath_Skia::rMoveTo(float dx, float dy)
	{
		m_skPath.rMoveTo(dx,dy);
	}

	void SPath_Skia::lineTo(float x, float y)
	{
		m_skPath.lineTo(x,y);
	}

	void SPath_Skia::rLineTo(float dx, float dy)
	{
		m_skPath.rLineTo(dx,dy);
	}

	void SPath_Skia::quadTo(float x1, float y1, float x2, float y2)
	{
		m_skPath.quadTo(x1,y1,x2,y2);
	}

	void SPath_Skia::rQuadTo(float dx1, float dy1, float dx2, float dy2)
	{
		m_skPath.rQuadTo(dx1,dy1,dx2,dy2);
	}

	void SPath_Skia::conicTo(float x1, float y1, float x2, float y2, float w)
	{
		m_skPath.conicTo(x1,y1,x2,y2,w);
	}

	void SPath_Skia::rConicTo(float dx1, float dy1, float dx2, float dy2, float w)
	{
		m_skPath.rConicTo(dx1,dy1,dx2,dy2,w);
	}

	void SPath_Skia::cubicTo(float x1, float y1, float x2, float y2, float x3, float y3)
	{
		m_skPath.cubicTo(x1,y1,x2,y2,x3,y3);
	}

	void SPath_Skia::rCubicTo(float dx1, float dy1, float dx2, float dy2, float dx3, float dy3)
	{
		m_skPath.rCubicTo(dx1,dy1,dx2,dy2,dx3,dy3);
	}

	void SPath_Skia::close()
	{
	}

	void SPath_Skia::addRect(const RECT* rect, Direction dir /*= kCW_Direction*/)
	{
		SkRect skRc = toSkRect(rect);
		m_skPath.addRect(skRc,(SkPath::Direction)dir);
	}

	void SPath_Skia::addRect2(float left, float top, float right, float bottom, Direction dir /*= kCW_Direction*/)
	{
		m_skPath.addRect(left,top,right,bottom,(SkPath::Direction)dir);
	}

	void SPath_Skia::addOval(const RECT * oval, Direction dir /*= kCW_Direction*/)
	{
		SkRect skRc = toSkRect(oval);
		m_skPath.addOval(skRc,(SkPath::Direction)dir);
	}

	void SPath_Skia::addOval2(THIS_ float left, float top, float right, float bottom, Direction dir)
	{
		SkRect skRc = {left,top,right,bottom};
		m_skPath.addOval(skRc,(SkPath::Direction)dir);
	}

	void SPath_Skia::addCircle(float x, float y, float radius, Direction dir /*= kCW_Direction*/)
	{
		m_skPath.addCircle(x,y,radius,(SkPath::Direction)dir);
	}

	void SPath_Skia::addArc(const RECT* oval, float startAngle, float sweepAngle)
	{
		SkRect skRc = toSkRect(oval);
		m_skPath.addArc(skRc,startAngle,sweepAngle);
	}

	void SPath_Skia::addArc2(THIS_ float left, float top, float right, float bottom,float startAngle, float sweepAngle)
	{
		SkRect skRc = {left,top,right,bottom};
		m_skPath.addArc(skRc,startAngle,sweepAngle);
	}

	void SPath_Skia::addRoundRect(const RECT* rect, float rx, float ry, Direction dir /*= kCW_Direction*/)
	{
		SkRect skRc = toSkRect(rect);
		m_skPath.addRoundRect(skRc,rx,ry,(SkPath::Direction)dir);
	}

	void SPath_Skia::addRoundRect2(float left, float top,float right,float bottom, float rx,float ry, Direction dir /*= kCW_Direction*/)
	{
		SkRect skRc = {left,top,right,bottom};
		m_skPath.addRoundRect(skRc,rx,ry,(SkPath::Direction)dir);
	}

	void SPath_Skia::addPoly(const POINT pts[], int count, BOOL close)
	{
		SkPoint *skPts = new SkPoint[count];
		for(int i=0;i<count;i++)
		{
			skPts[i].fX = (float)pts[i].x;
			skPts[i].fY = (float)pts[i].y;
		}

		m_skPath.addPoly(skPts,count,!!close);
		delete []skPts;
	}

	void SPath_Skia::offset(float dx, float dy)
	{
		m_skPath.offset(dx,dy);
	}

	void SPath_Skia::transform(const IxForm * xForm)
	{
		SkMatrix mat;
		mat.setAll(xForm->fMat[kMScaleX],
			xForm->fMat[kMSkewX], 
			xForm->fMat[kMTransX],
			xForm->fMat[kMSkewY],
			xForm->fMat[kMScaleY], 
			xForm->fMat[kMTransY],
			xForm->fMat[kMPersp0], 
			xForm->fMat[kMPersp1], 
			xForm->fMat[kMPersp2]);
		m_skPath.transform(mat);
	}

	BOOL SPath_Skia::getLastPt(fPoint* lastPt) const
	{
		SkPoint pt;
		bool bRet = m_skPath.getLastPt(&pt);
		if(lastPt)
		{
			lastPt->fX = pt.fX;
			lastPt->fY = pt.fY;
		}
		return bRet;
	}

	void SPath_Skia::addString(LPCTSTR pszText,int nLen, float x,float y, const IFontS *pFont)
	{
		const SFont_Skia *pFontSkia = (const SFont_Skia *)pFont;
		if(nLen < 0) nLen = (int)_tcslen(pszText);
		SkPaint paint;
		paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
		paint.setTypeface(pFontSkia->GetFont());
		const LOGFONT *plf = pFont->LogFont();
		paint.setTextSize(SkIntToScalar(abs(plf->lfHeight)));
		paint.setUnderlineText(!!plf->lfUnderline);
		paint.setStrikeThruText(!!plf->lfStrikeOut);

		SkPath path;
#ifdef _UNICODE
		paint.getTextPath(pszText,nLen*2,0.0f,0.0f,&path);
#else
		SStringW str=S_CT2W(SStringT(pszText,nLen));
		paint.getTextPath((LPCWSTR)str,str.GetLength()*2,0.0f,0.0f,&path);
#endif
		SkRect rcBound=path.getBounds();
		path.offset(-rcBound.fLeft,-rcBound.fTop);
		m_skPath.addPath(path,x,y,SkPath::kAppend_AddPathMode);
	}

	static void addLine(std::vector<SkPoint>& segmentPoints, std::vector<float>& lengths,
		const SkPoint& toPoint) {
			if (segmentPoints.empty()) {
				segmentPoints.push_back(SkPoint::Make(0, 0));
				lengths.push_back(0);
			}
			else if (segmentPoints.back() == toPoint) {
				return; // Empty line
			}
			float length = lengths.back() + SkPoint::Distance(segmentPoints.back(), toPoint);
			segmentPoints.push_back(toPoint);
			lengths.push_back(length);
	}
	typedef SkPoint(*bezierCalculation)(float t, const SkPoint* points);


	static float cubicCoordinateCalculation(float t, float p0, float p1, float p2, float p3) {
		float oneMinusT = 1 - t;
		float oneMinusTSquared = oneMinusT * oneMinusT;
		float oneMinusTCubed = oneMinusTSquared * oneMinusT;
		float tSquared = t * t;
		float tCubed = tSquared * t;
		return (oneMinusTCubed * p0) + (3 * oneMinusTSquared * t * p1)
			+ (3 * oneMinusT * tSquared * p2) + (tCubed * p3);
	}

	static SkPoint cubicBezierCalculation(float t, const SkPoint* points) {
		float x = cubicCoordinateCalculation(t, points[0].x(), points[1].x(),
			points[2].x(), points[3].x());
		float y = cubicCoordinateCalculation(t, points[0].y(), points[1].y(),
			points[2].y(), points[3].y());
		return SkPoint::Make(x, y);
	}

	static float quadraticCoordinateCalculation(float t, float p0, float p1, float p2) {
		float oneMinusT = 1 - t;
		return oneMinusT * ((oneMinusT * p0) + (t * p1)) + t * ((oneMinusT * p1) + (t * p2));
	}

	static SkPoint quadraticBezierCalculation(float t, const SkPoint* points) {
		float x = quadraticCoordinateCalculation(t, points[0].x(), points[1].x(), points[2].x());
		float y = quadraticCoordinateCalculation(t, points[0].y(), points[1].y(), points[2].y());
		return SkPoint::Make(x, y);
	}

	// Subdivide a section of the Bezier curve, set the mid-point and the mid-t value.
	// Returns true if further subdivision is necessary as defined by errorSquared.
	static bool subdividePoints(const SkPoint* points, bezierCalculation bezierFunction,
		float t0, const SkPoint &p0, float t1, const SkPoint &p1,
		float& midT, SkPoint &midPoint, float errorSquared) {
			midT = (t1 + t0) / 2;
			float midX = (p1.x() + p0.x()) / 2;
			float midY = (p1.y() + p0.y()) / 2;

			midPoint = (*bezierFunction)(midT, points);
			float xError = midPoint.x() - midX;
			float yError = midPoint.y() - midY;
			float midErrorSquared = (xError * xError) + (yError * yError);
			return midErrorSquared > errorSquared;
	}

	static void addBezier(const SkPoint* points,
		bezierCalculation bezierFunction, std::vector<SkPoint>& segmentPoints,
		std::vector<float>& lengths, float errorSquared, bool doubleCheckDivision) {
			typedef std::map<float, SkPoint> PointMap;
			PointMap tToPoint;

			tToPoint[0] = (*bezierFunction)(0, points);
			tToPoint[1] = (*bezierFunction)(1, points);

			PointMap::iterator iter = tToPoint.begin();
			PointMap::iterator next = iter;
			++next;
			while (next != tToPoint.end()) {
				bool needsSubdivision = true;
				SkPoint midPoint;
				do {
					float midT;
					needsSubdivision = subdividePoints(points, bezierFunction, iter->first,
						iter->second, next->first, next->second, midT, midPoint, errorSquared);
					if (!needsSubdivision && doubleCheckDivision) {
						SkPoint quarterPoint;
						float quarterT;
						needsSubdivision = subdividePoints(points, bezierFunction, iter->first,
							iter->second, midT, midPoint, quarterT, quarterPoint, errorSquared);
						if (needsSubdivision) {
							// Found an inflection point. No need to double-check.
							doubleCheckDivision = false;
						}
					}
					if (needsSubdivision) {
						next = tToPoint.insert(iter, PointMap::value_type(midT, midPoint));
					}
				} while (needsSubdivision);
				iter = next;
				next++;
			}

			// Now that each division can use linear interpolation with less than the allowed error
			for (iter = tToPoint.begin(); iter != tToPoint.end(); ++iter) {
				addLine(segmentPoints, lengths, iter->second);
			}
	}

	static void addMove(std::vector<SkPoint>& segmentPoints, std::vector<float>& lengths,
		const SkPoint& point) {
			float length = 0;
			if (!lengths.empty()) {
				length = lengths.back();
			}
			segmentPoints.push_back(point);
			lengths.push_back(length);
	}

	static void createVerbSegments(SkPath::Verb verb, const SkPoint* points,
		std::vector<SkPoint>& segmentPoints, std::vector<float>& lengths, float errorSquared) {
			switch (verb) {
		case SkPath::kMove_Verb:
			addMove(segmentPoints, lengths, points[0]);
			break;
		case SkPath::kClose_Verb:
			addLine(segmentPoints, lengths, points[0]);
			break;
		case SkPath::kLine_Verb:
			addLine(segmentPoints, lengths, points[1]);
			break;
		case SkPath::kQuad_Verb:
			addBezier(points, quadraticBezierCalculation, segmentPoints, lengths,
				errorSquared, false);
			break;
		case SkPath::kCubic_Verb:
			addBezier(points, cubicBezierCalculation, segmentPoints, lengths,
				errorSquared, true);
			break;
		default:
			// Leave element as NULL, Conic sections are not supported.
			break;
			}
	}

	IPathS * SPath_Skia::clone(CTHIS) const
	{
		SPath_Skia * ret = new SPath_Skia(GetRenderFactory());
		ret->m_skPath = m_skPath;
		return ret;
	}

	float SPath_Skia::getLength(CTHIS) const
	{
		SkPathMeasure pathMeasure;
		pathMeasure.setPath(&m_skPath,false);
		return pathMeasure.getLength();
	}

	BOOL SPath_Skia::getPosTan(CTHIS_ float distance, fPoint *pos, fPoint *vec) const
	{
		SkPathMeasure pathMeasure;
		pathMeasure.setPath(&m_skPath,false);
		return pathMeasure.getPosTan(distance,(SkPoint*)pos,(SkPoint*)vec);
	}

	BOOL SPath_Skia::hitTest(CTHIS_ float x,float y) const
	{
		return m_skPath.contains(x,y);
	}

	BOOL SPath_Skia::hitTestStroke(CTHIS_ float x,float y,float strokeSize) const
	{
		return FALSE;
	}

	//--------------------------------------------------------------------------------
	SMaskFilter_Skia::SMaskFilter_Skia(SkMaskFilter *maskFilter):m_maskFilter(maskFilter)
	{
	}

	SMaskFilter_Skia::~SMaskFilter_Skia()
	{
		if(m_maskFilter)
		{
			m_maskFilter->unref();
		}
	}

	void* SMaskFilter_Skia::GetPtr()
	{
		return m_maskFilter;
	}

	namespace RENDER_SKIA
	{
		BOOL SCreateInstance( IObjRef ** ppRenderFactory )
		{
			*ppRenderFactory = new SRenderFactory_Skia;
			return TRUE;
		}
	}

}//end of namespace SOUI


EXTERN_C BOOL Render_Skia_SCreateInstance(IObjRef ** ppRenderFactory)
{
	return SOUI::RENDER_SKIA::SCreateInstance(ppRenderFactory);
}

static FontFallback s_fontFallback=NULL;
SkTypeface *SkiaFontFallback(SkTypeface *font, const wchar_t *text, size_t len)
{
	if(!s_fontFallback)
		return NULL;
	SkString curFontName;
	font->getFamilyName(&curFontName);

	char fontName[100]={0};
	int nCharSet = DEFAULT_CHARSET;
	if(!s_fontFallback(curFontName.c_str(),text,len,fontName,&nCharSet))
		return NULL;
	return SkTypeface::CreateFromName(fontName,font->style(),nCharSet);
}


EXTERN_C void Render_Skia_SetFontFallback(FontFallback fontFallback)
{
	s_fontFallback = fontFallback;
	if(fontFallback){
		SkCanvas::SetFontFallback(SkiaFontFallback);
	}else{
		SkCanvas::SetFontFallback(NULL);
	}
}
