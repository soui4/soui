// render-gdi.cpp : Defines the exported functions for the DLL application.
//

#include "render-d2d.h"
#include <gdialpha.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <tchar.h>
#include <algorithm>
#include <atl.mini/SComHelper.h>
#include <wtl.mini/souimisc.h>

#pragma comment(lib,"Msimg32")
#pragma  comment(lib,"windowscodecs.lib")


#pragma warning(disable:4244 4018)

using namespace D2D1;

#ifndef _D2D1_1_H_
#define _D2D1_1_H_
//copy from d2d1_1.h hjx

#ifndef DX_DECLARE_INTERFACE
#define DX_DECLARE_INTERFACE(x) DECLSPEC_UUID(x) DECLSPEC_NOVTABLE
#endif

/// <summary>
/// Describes a point along a path.
/// </summary>
typedef struct D2D1_POINT_DESCRIPTION
{
	D2D1_POINT_2F point;
	D2D1_POINT_2F unitTangentVector;
	UINT32 endSegment;
	UINT32 endFigure;
	FLOAT lengthToEndSegment;

} D2D1_POINT_DESCRIPTION;

#define D2D1_DEFAULT_FLATTENING_TOLERANCE (0.25f)

/// <summary>
/// The ID2D1PathGeometry1 interface adds functionality to ID2D1PathGeometry. In
/// particular, it provides the path geometry-specific
/// ComputePointAndSegmentAtLength method.
/// </summary>
interface DX_DECLARE_INTERFACE("62baa2d2-ab54-41b7-b872-787e0106a421") ID2D1PathGeometry1  : public ID2D1PathGeometry
{

	STDMETHOD(ComputePointAndSegmentAtLength)(
		FLOAT length,
		UINT32 startSegment,
		_In_opt_ CONST D2D1_MATRIX_3X2_F *worldTransform,
		FLOAT flatteningTolerance,
		_Out_ D2D1_POINT_DESCRIPTION *pointDescription 
		) CONST PURE;

	COM_DECLSPEC_NOTHROW
		HRESULT
		ComputePointAndSegmentAtLength(
		FLOAT length,
		UINT32 startSegment,
		CONST D2D1_MATRIX_3X2_F &worldTransform,
		FLOAT flatteningTolerance,
		_Out_ D2D1_POINT_DESCRIPTION *pointDescription 
		) CONST  
	{
		return ComputePointAndSegmentAtLength(length, startSegment, &worldTransform, flatteningTolerance, pointDescription);
	}

	COM_DECLSPEC_NOTHROW
		HRESULT
		ComputePointAndSegmentAtLength(
		FLOAT length,
		UINT32 startSegment,
		_In_opt_ CONST D2D1_MATRIX_3X2_F *worldTransform,
		_Out_ D2D1_POINT_DESCRIPTION *pointDescription 
		) CONST  
	{
		return ComputePointAndSegmentAtLength(length, startSegment, worldTransform, D2D1_DEFAULT_FLATTENING_TOLERANCE, pointDescription);
	}

	COM_DECLSPEC_NOTHROW
		HRESULT
		ComputePointAndSegmentAtLength(
		FLOAT length,
		UINT32 startSegment,
		CONST D2D1_MATRIX_3X2_F &worldTransform,
		_Out_ D2D1_POINT_DESCRIPTION *pointDescription 
		) CONST  
	{
		return ComputePointAndSegmentAtLength(length, startSegment, &worldTransform, D2D1_DEFAULT_FLATTENING_TOLERANCE, pointDescription);
	}
}; // interface ID2D1PathGeometry1
#endif //_D2D1_1_H_

namespace SOUI
{
	class D2DColor
	{
	public:
		D2DColor(BYTE r,BYTE g,BYTE b,BYTE a=0xFF):a(a),b(b),g(g),r(r)
		{

		}

		D2DColor(COLORREF cr)
		{
			memcpy(this,&cr,4);
		}

		D2DColor(D2D1_COLOR_F cf)
		{
			r=(BYTE)(cf.r*255);
			g=(BYTE)(cf.g*255);
			b=(BYTE)(cf.b*255);
			a=(BYTE)(cf.a*255);
		}

		D2D1_COLOR_F toD2DColor() const
		{
			D2D1_COLOR_F ret;
			ret.r = float(r)/255;
			ret.g = float(g)/255;
			ret.b = float(b)/255;
			ret.a = float(a)/255;
			return ret;
		}
		operator const COLORREF() const
		{
			DWORD cr;
			memcpy(&cr,this,4);
			return cr;
		}

		void UpdateAlpha(BYTE alpha){
			float a2=alpha*1.0f/255;
			r = r*a2;
			g = g*a2;
			b = b*a2;
			a = a*a2;
		}
		DWORD r:8;
		DWORD g:8;
		DWORD b:8;
		DWORD a:8;
	};


	//////////////////////////////////////////////////////////////////////////
	// SRenderFactory_D2D
	static ID2D1Factory * toD2DFac(IRenderFactory *pFac)
	{
		SRenderFactory_D2D *pFacD2D=(SRenderFactory_D2D*)pFac;
		return pFacD2D->GetD2D1Factory();
	}

	static IDWriteFactory * toD2DWriteFac(IRenderFactory *pFac)
	{
		SRenderFactory_D2D *pFacD2D=(SRenderFactory_D2D*)pFac;
		return pFacD2D->GetDWriteFactory();
	}

	static IWICImagingFactory * toD2DWicImgFac(IRenderFactory*pFac)
	{
		SRenderFactory_D2D *pFacD2D=(SRenderFactory_D2D*)pFac;
		return pFacD2D->GetWicImgFactory();
	}

	static D2D1_RECT_F toRectF(LPCRECT pRc,BOOL bDec5=FALSE)
	{
		D2D1_RECT_F ret={(float)pRc->left,(float)pRc->top,(float)pRc->right,(float)pRc->bottom};
		if(bDec5){
			ret.left+=0.5f;
			ret.top +=0.5f;
			ret.right-=0.5f;
			ret.bottom-=0.5;
		}
		return ret;
	}

	static D2D1_POINT_2F toPointF(POINT pt){
		D2D1_POINT_2F ret = { (float)pt.x,(float)pt.y};
		return ret;
	}

	BOOL SRenderFactory_D2D::Init()
	{
		HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
		if(!SUCCEEDED(hr)) return FALSE;

		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(m_pDWriteFactory),
			reinterpret_cast<IUnknown **>(&m_pDWriteFactory)
			);
		if(!SUCCEEDED(hr)) return FALSE;

		// Create WIC factory.
		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory,
			reinterpret_cast<void **>(&m_pWICImageFactory)
			);
		if(!SUCCEEDED(hr)) return FALSE;
		return TRUE;
	}

	BOOL SRenderFactory_D2D::CreateRenderTarget( IRenderTarget ** ppRenderTarget ,int nWid,int nHei)
	{
		*ppRenderTarget = new SRenderTarget_D2D(this, nWid, nHei);
		return TRUE;
	}

	BOOL SRenderFactory_D2D::CreateRenderTarget2(THIS_ IRenderTarget ** ppRenderTarget,HWND hWnd)
	{
		*ppRenderTarget = new SRenderTarget_D2D(this, hWnd);
		return TRUE;
	}

	BOOL SRenderFactory_D2D::CreateFont( IFontS ** ppFont , const LOGFONT *lf)
	{
		*ppFont = new SFont_D2D(this,lf);
		return TRUE;
	}

	BOOL SRenderFactory_D2D::CreateBitmap( IBitmapS ** ppBitmap )
	{
		*ppBitmap = new SBitmap_D2D(this);
		return TRUE;
	}

	BOOL SRenderFactory_D2D::CreateRegion( IRegionS **ppRgn )
	{
		*ppRgn = new SRegion_D2D(this);
		return TRUE;
	}

	BOOL SRenderFactory_D2D::CreatePath(IPathS ** ppPath)
	{
		*ppPath = new SPath_D2D(this);
		return TRUE;
	}

	BOOL SRenderFactory_D2D::CreatePathEffect(REFGUID guidEffect,IPathEffect ** ppPathEffect)
	{
		return FALSE;
	}

	void SRenderFactory_D2D::SetImgDecoderFactory(IImgDecoderFactory *pImgDecoderFac)
	{
		m_imgDecoderFactory=pImgDecoderFac;
	}

	IImgDecoderFactory * SRenderFactory_D2D::GetImgDecoderFactory()
	{
		return m_imgDecoderFactory;
	}

	//////////////////////////////////////////////////////////////////////////

	void SPen_D2D::SetColor(THIS_ COLORREF cr)
	{
		m_cr = cr;
		UpdatePen();
	}

	STDMETHODIMP_(COLORREF) SPen_D2D::GetColor(THIS) SCONST
	{
		return m_cr;
	}

	void SPen_D2D::SetStyle(THIS_ int nStyle)
	{
		m_style = nStyle;
		UpdatePen();
	}

	int SPen_D2D::GetStyle(THIS) SCONST
	{
		return m_style;
	}

	void SPen_D2D::SetWidth(THIS_ int nWid)
	{
		m_nWidth=nWid;
		UpdatePen();
	}

	int SPen_D2D::GetWidth(THIS) SCONST
	{
		return m_nWidth;
	}


	SPen_D2D::SPen_D2D(IRenderFactory * pRenderFac,int iStyle/*=PS_SOLID*/,COLORREF cr/*=0*/,int cWidth/*=1*/) :TD2DRenderObjImpl<IPenS,OT_PEN>(pRenderFac)
		,m_nWidth(cWidth),m_style(iStyle),m_cr(cr)
	{
		UpdatePen();
	}

	SPen_D2D::~SPen_D2D()
	{
	}

	void SPen_D2D::UpdatePen()
	{
		m_pen = NULL;
		ID2D1Factory *pFac = toD2DFac(GetRenderFactory());
		D2D1_STROKE_STYLE_PROPERTIES prop=D2D1::StrokeStyleProperties(
			D2D1_CAP_STYLE_SQUARE,
			D2D1_CAP_STYLE_SQUARE,
			D2D1_CAP_STYLE_FLAT,
			D2D1_LINE_JOIN_ROUND,
			0.0f,
			D2D1_DASH_STYLE_SOLID,
			0.0f);
		switch(m_style&PS_STYLE_MASK)
		{
		case PS_SOLID:
			prop.dashStyle = D2D1_DASH_STYLE_SOLID;
			break;
		case PS_DASH:
			prop.dashStyle = D2D1_DASH_STYLE_DASH;
			break;
		case PS_DASHDOT:
			prop.dashStyle = D2D1_DASH_STYLE_DASH_DOT;
			break;
		case PS_DASHDOTDOT:
			prop.dashStyle = D2D1_DASH_STYLE_DASH_DOT_DOT;
			break;
		case PS_DOT:
			prop.dashStyle = D2D1_DASH_STYLE_DOT;
			break;
		}
		switch(m_style & PS_ENDCAP_MASK){
			case PS_ENDCAP_ROUND:
				prop.startCap = prop.endCap= D2D1_CAP_STYLE_ROUND;
				break;
			case PS_ENDCAP_SQUARE:
				prop.startCap = prop.endCap= D2D1_CAP_STYLE_SQUARE;
				break;
			case PS_ENDCAP_FLAT:
				prop.startCap = prop.endCap= D2D1_CAP_STYLE_FLAT;
				break;
		}
		switch(m_style & PS_JOIN_MASK){
			case PS_JOIN_ROUND:
				prop.lineJoin = D2D1_LINE_JOIN_ROUND;
				break;
			case PS_JOIN_BEVEL:
				prop.lineJoin = D2D1_LINE_JOIN_BEVEL;
				break;
			case PS_JOIN_MITER:
				prop.lineJoin = D2D1_LINE_JOIN_MITER;
				break;
		}
		pFac->CreateStrokeStyle(prop,NULL,0,(ID2D1StrokeStyle**)&m_pen);
	}

	SComPtr<ID2D1Brush> SPen_D2D::GetColorBrush(ID2D1RenderTarget *pRT)
	{
		SComPtr<ID2D1SolidColorBrush> br;
		D2DColor color(m_cr);
		pRT->CreateSolidColorBrush(color.toD2DColor(),&br);
		return SComPtr<ID2D1Brush>(br);
	}

	//////////////////////////////////////////////////////////////////////////

	const LOGFONT * SFont_D2D::LogFont()  SCONST
	{
		return &m_lf;
	}

	LPCTSTR SFont_D2D::FamilyName() SCONST
	{
		return m_lf.lfFaceName;
	}

	int SFont_D2D::TextSize() SCONST
	{
		return m_lf.lfHeight;
	}

	BOOL SFont_D2D::IsBold() SCONST
	{
		return m_lf.lfWeight == FW_BOLD;
	}

	BOOL SFont_D2D::IsItalic() SCONST
	{
		return m_lf.lfItalic;
	}

	BOOL SFont_D2D::IsUnderline() SCONST
	{
		return m_lf.lfUnderline;
	}

	BOOL SFont_D2D::IsStrikeOut() SCONST
	{
		return m_lf.lfStrikeOut;
	}

	BOOL SFont_D2D::UpdateFont(const LOGFONT *pLogFont)
	{
		if(!m_hFont) return FALSE;
		m_hFont=NULL;
		memcpy(&m_lf,pLogFont,sizeof(LOGFONT));
		CreateD2DFont();
		return TRUE;
	}

	void SFont_D2D::SetProp(IXmlNode *pXmlNode)
	{

	}

	SFont_D2D::SFont_D2D(IRenderFactory * pRenderFac,const LOGFONT * plf) 
		:TD2DRenderObjImpl<IFontS,OT_FONT>(pRenderFac)
		,m_hFont(NULL)
	{
		memcpy(&m_lf,plf,sizeof(LOGFONT));
		CreateD2DFont();
	}

	SFont_D2D::~SFont_D2D()
	{
		m_hFont=NULL;
	}

	static DWRITE_FONT_WEIGHT ConvertFontWeight(LONG lfWeight)
	{
		if(lfWeight == 0){
			return DWRITE_FONT_WEIGHT_NORMAL;
		}
		else if (lfWeight <= FW_THIN)
		{
			return DWRITE_FONT_WEIGHT_THIN;
		}
		else if (lfWeight <= FW_EXTRALIGHT)
		{
			return DWRITE_FONT_WEIGHT_EXTRA_LIGHT;
		}
		else if (lfWeight <= FW_LIGHT)
		{
			return DWRITE_FONT_WEIGHT_LIGHT;
		}
		else if (lfWeight <= FW_NORMAL)
		{
			return DWRITE_FONT_WEIGHT_NORMAL;
		}
		else if (lfWeight <= FW_MEDIUM)
		{
			return DWRITE_FONT_WEIGHT_MEDIUM;
		}
		else if (lfWeight <= FW_SEMIBOLD)
		{
			return DWRITE_FONT_WEIGHT_SEMI_BOLD;
		}
		else if (lfWeight <= FW_BOLD)
		{
			return DWRITE_FONT_WEIGHT_BOLD;
		}
		else if (lfWeight <= FW_EXTRABOLD)
		{
			return DWRITE_FONT_WEIGHT_EXTRA_BOLD;
		}
		else
		{
			return DWRITE_FONT_WEIGHT_BLACK;
		}
	}
	BOOL SFont_D2D::CreateD2DFont()
	{
		IDWriteFactory *pFac = toD2DWriteFac(GetRenderFactory());
		SASSERT(!m_hFont);
		SStringW strFaceName = S_CT2W(m_lf.lfFaceName);
		DWRITE_FONT_WEIGHT fontWeight=(DWRITE_FONT_WEIGHT)m_lf.lfWeight;
		DWRITE_FONT_STYLE  fontStyle = DWRITE_FONT_STYLE_NORMAL;
		if(m_lf.lfItalic) fontStyle = DWRITE_FONT_STYLE_ITALIC;
		DWRITE_FONT_STRETCH fontStetch =DWRITE_FONT_STRETCH_NORMAL;
		HRESULT hr  = pFac->CreateTextFormat(strFaceName.c_str(),NULL,
			ConvertFontWeight(m_lf.lfWeight),
			m_lf.lfItalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			abs(m_lf.lfHeight),
			L"",
			&m_hFont);
		SASSERT(SUCCEEDED(hr));
		return hr==S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	SBrush_D2D::SBrush_D2D(IRenderFactory * pRenderFac,COLORREF cr) :TD2DRenderObjImpl<IBrushS,OT_BRUSH>(pRenderFac),m_cr(cr),m_brushType(Brush_Color)
	{
	}

	SBrush_D2D::SBrush_D2D(IRenderFactory * pRenderFac,IWICBitmap *hBmp) :TD2DRenderObjImpl<IBrushS,OT_BRUSH>(pRenderFac),m_cr(CR_INVALID),m_brushType(Brush_Bitmap)
	{
		m_bmp=hBmp;
	}

	SBrush_D2D::SBrush_D2D(IRenderFactory * pRenderFac,const GradientItem *pGradients, int nCount, const GradientInfo *info, BYTE byAlpha,TileMode tileMode) :TD2DRenderObjImpl<IBrushS,OT_BRUSH>(pRenderFac),m_brushType(Brush_Shader)
	{
		m_tileMode = tileMode;
		m_byAlpha = byAlpha;
		m_gradInfo = *info;
		m_arrGradItem.SetCount(nCount);
		memcpy(m_arrGradItem.GetData(),pGradients,sizeof(GradientItem)*nCount);
	}

	SBrush_D2D::~SBrush_D2D()
	{
		
	}
	const float PI = 3.1415926f;

	SComPtr<ID2D1Brush> SBrush_D2D::toBrush(ID2D1RenderTarget *pRT,LPCRECT pRect) const
	{
		switch(m_brushType){
			case Brush_Color:
				{
					D2DColor crD2d(m_cr);
					SComPtr<ID2D1SolidColorBrush> br;
					pRT->CreateSolidColorBrush(crD2d.toD2DColor(),&br);
					return SComPtr<ID2D1Brush>(br);
				}
				break;
			case Brush_Bitmap:
				{
					SComPtr<ID2D1BitmapBrush> br;
					SComPtr<ID2D1Bitmap> bmp;
					pRT->CreateBitmapFromWicBitmap(m_bmp,&bmp);
					pRT->CreateBitmapBrush(bmp,&br);
					return SComPtr<ID2D1Brush>(br);
				}
				break;
			case Brush_Shader:
				{
					SComPtr<ID2D1GradientStopCollection> collection;
					int nCount = (int)m_arrGradItem.GetCount();
					D2D1_GRADIENT_STOP* gradientStops = new D2D1_GRADIENT_STOP [nCount];
					for (int i=0;i<nCount;i++)
					{
						gradientStops[i].position = m_arrGradItem[i].pos;
						D2DColor cr(m_arrGradItem[i].cr);
						cr.UpdateAlpha(m_byAlpha);
						gradientStops[i].color = cr.toD2DColor();
					}
					HRESULT hr = pRT->CreateGradientStopCollection (gradientStops, nCount, &collection);
					delete [] gradientStops;
					if(!collection) 
						break;
					switch(m_gradInfo.type)
					{
						case linear:
							{
								SComPtr<ID2D1LinearGradientBrush> brush;
								D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES properties;
								CRect rc(pRect);
								D2D1_POINT_2F skPts[2];
								float halfWid = rc.Width()/2;
								float halfHei = rc.Height()/2;
								if(m_gradInfo.angle==90.0f || m_gradInfo.angle==270.0f)
								{//90度
									skPts[0].x=skPts[1].x=halfWid;
									skPts[0].y=0.0f;
									skPts[1].y=rc.Height();
								}else if(m_gradInfo.angle==0.0f || m_gradInfo.angle==180.0f)
								{//水平方向
									skPts[0].y=skPts[1].y=halfHei;
									skPts[0].x = 0.0f;
									skPts[1].x = rc.Width();
								}else
								{//其它角度

									float angleInRadians = PI*m_gradInfo.angle/180;
									double tanAngle = tan(angleInRadians);

									D2D1_POINT_2F pt1a,pt2a;//与左右两条边相交的位置
									D2D1_POINT_2F pt1b,pt2b;//与上下两条边相关的位置

									pt1a.x=-halfWid,pt2a.x=halfWid;
									pt1b.y=-halfHei,pt2b.y=halfHei;

									pt1a.y= (float)(-halfWid*tanAngle);
									pt2a.y = -pt1a.y;

									pt1b.x = (float)(-halfHei/tanAngle);
									pt2b.x = -pt1b.x;

									if(pt2a.y > halfHei)
									{//using pt1a,pt2a
										skPts[0] = pt1a;
										skPts[1] = pt2a;
									}else
									{//using pt1b,pt2b
										skPts[0]=pt1b;
										skPts[1]=pt2b;
									}
								}
								skPts[0].x+=rc.left;
								skPts[1].x+=rc.left;
								skPts[0].y+=rc.top;
								skPts[1].y+=rc.top;
								properties.startPoint = skPts[0];
								properties.endPoint = skPts[1];
								if (SUCCEEDED (hr = pRT->CreateLinearGradientBrush (properties, collection, &brush)))
									return (ID2D1Brush*)brush;
							}
						break;
						case radial:
							{
								SComPtr<ID2D1RadialGradientBrush> brush;
								D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES properties;
								properties.center.x = m_gradInfo.radial.centerX*(pRect->right-pRect->left);
								properties.center.y = m_gradInfo.radial.centerY*(pRect->bottom-pRect->top);
								properties.gradientOriginOffset.x = properties.gradientOriginOffset.y = 0.0f;
								properties.radiusX = properties.radiusY = m_gradInfo.radial.radius;
								if(SUCCEEDED(hr=pRT->CreateRadialGradientBrush(properties,collection,&brush)))
									return (ID2D1Brush*)brush;
							}
							break;
						default:
							break;
					}
				}
			default:
				break;
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	//  SBitmap_D2D

	SBitmap_D2D::SBitmap_D2D(IRenderFactory *pRenderFac) :TD2DRenderObjImpl<IBitmapS,OT_BITMAP>(pRenderFac),m_lock(NULL)
	{
		m_sz.cx=m_sz.cy=0;
	}

	HRESULT SBitmap_D2D::Init( int nWid,int nHei ,const LPVOID pBits/*=NULL*/)
	{
		m_bmp2 = NULL;
		m_sz.cx = nWid;
		m_sz.cy = nHei;
		if(nWid == 0) nWid = 1;
		if(nHei == 0) nHei = 1;
		ID2D1Factory *pFac = toD2DFac(GetRenderFactory());
		IWICImagingFactory *pFac2 = toD2DWicImgFac(GetRenderFactory());
		HRESULT hr=E_FAIL;
		if(pBits)
			hr=pFac2->CreateBitmapFromMemory(nWid,nHei,GUID_WICPixelFormat32bppPBGRA,nWid*4,nWid*nHei*4,(BYTE*)pBits,&m_bmp2);
		else
			hr=pFac2->CreateBitmap(nWid,nHei,GUID_WICPixelFormat32bppPBGRA,WICBitmapCacheOnLoad,&m_bmp2);
		if(!SUCCEEDED(hr)){
			m_sz.cx = m_sz.cy = 0;
		}
		return hr;
	}

	HRESULT SBitmap_D2D::Init2(IImgFrame *pFrame )
	{
		HRESULT hr = S_OK;
		UINT nWid,nHei;
		pFrame->GetSize(&nWid,&nHei);
		do{
			HRESULT hr = Init(nWid,nHei,NULL);
			if(hr!=S_OK) break;
			WICRect rc={0,0,(int)nWid,(int)nHei};
			IWICBitmapLock *lock=NULL;
			hr=m_bmp2->Lock(&rc,WICBitmapLockWrite,&lock);
			if(hr!=S_OK) break;
			RECT rc2={0,0,(LONG)nWid,(LONG)nHei};
			UINT stride=0;
			lock->GetStride(&stride);
			UINT bufSize=0;
			BYTE *pBuf=NULL;
			lock->GetDataPointer(&bufSize,&pBuf);
			memcpy(pBuf,pFrame->GetPixels(),nHei*stride);
			lock->Release();
			m_sz.cx = nWid;
			m_sz.cy = nHei;
		}while(false);
		return hr;
	}

	HRESULT SBitmap_D2D::LoadFromFile( LPCTSTR pszFileName)
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

	HRESULT SBitmap_D2D::LoadFromMemory(LPBYTE pBuf,size_t szLen)
	{
		SAutoRefPtr<IImgX> imgDecoder;
		GetRenderFactory()->GetImgDecoderFactory()->CreateImgX(&imgDecoder);
		if(imgDecoder->LoadFromMemory(pBuf,szLen)==0) return S_FALSE;
		return ImgFromDecoder(imgDecoder);
	}

	HRESULT SBitmap_D2D::ImgFromDecoder(IImgX *imgDecoder)
	{
		IImgFrame *pFrame=imgDecoder->GetFrame(0);
		return Init2(pFrame);
	}

	UINT SBitmap_D2D::Width() const
	{
		return m_sz.cx;
	}

	UINT SBitmap_D2D::Height() const
	{
		return m_sz.cy;
	}

	SIZE SBitmap_D2D::Size() const
	{
		return m_sz;
	}

	LPVOID SBitmap_D2D::LockPixelBits()
	{
		if(!m_bmp2)
			return NULL;
		if(m_lock)
		{
			m_lock->AddRef();
		}else
		{
			WICRect rc={0,0,m_sz.cx,m_sz.cy};
			HRESULT hr=m_bmp2->Lock(&rc,WICBitmapLockWrite,&m_lock);
			if(hr!=S_OK)
				return NULL;
		}
		BYTE *pBuf=NULL;
		UINT uSize=0;
		m_lock->GetDataPointer(&uSize,&pBuf);
		return pBuf;
	}

	void SBitmap_D2D::UnlockPixelBits( LPVOID pBuf)
	{
		SASSERT(m_lock);
		if(m_lock->Release()==0)
		{
			m_lock=NULL;
		}
	}

	const LPVOID SBitmap_D2D::GetPixelBits() const
	{
		WICRect rc={0,0,m_sz.cx,m_sz.cy};
		SComPtr<IWICBitmapLock> lock;
		HRESULT hr=m_bmp2->Lock(&rc,WICBitmapLockRead,&lock);
		if(hr!=S_OK)
			return NULL;
		BYTE *pBuf=NULL;
		UINT uSize=0;
		lock->GetDataPointer(&uSize,&pBuf);
		return pBuf;
	}

	HRESULT SBitmap_D2D::Clone(IBitmapS **ppClone) const 
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

	HRESULT SBitmap_D2D::Scale(IBitmapS **ppOutput,int nScale,FilterLevel filterLevel) SCONST
	{
		int wid = MulDiv(Width(),nScale,100);
		int hei = MulDiv(Height(),nScale,100);
		return Scale2(ppOutput,wid,hei,filterLevel);
	}

	HRESULT SBitmap_D2D::Scale2(IBitmapS **pOutput,int nWid,int nHei,FilterLevel filterLevel) SCONST
	{
		if(nWid == Width() && nHei == Height())
		{
			return Clone(pOutput);
		}
		HRESULT hr = E_UNEXPECTED;
		IRenderTarget *pRT=NULL;
		if(GetRenderFactory()->CreateRenderTarget(&pRT,nWid,nHei))
		{
			RECT rcSrc = {0,0,(long)Width(),(long)Height()};
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

	HRESULT SBitmap_D2D::Save(LPCWSTR pszFileName,const LPVOID pFormat) SCONST
	{
		LPBYTE pBits = (LPBYTE)GetPixelBits();
		return GetRenderFactory()->GetImgDecoderFactory()->SaveImage(pBits,Width(),Height(),pszFileName,pFormat);
	}

	HRESULT SBitmap_D2D::Save2(CTHIS_ LPCWSTR pszFileName, ImgFmt imgFmt) SCONST
	{
		LPBYTE pBits = (LPBYTE)GetPixelBits();
		return GetRenderFactory()->GetImgDecoderFactory()->SaveImage2(pBits,Width(),Height(),pszFileName,imgFmt);
	}

	SComPtr<ID2D1Bitmap> SBitmap_D2D::toD2D1Bitmap(ID2D1RenderTarget *pRT) const
	{
		SComPtr<ID2D1Bitmap> ret;
		pRT->CreateBitmapFromWicBitmap(m_bmp2,&ret);
		return ret;
	}

	static HBITMAP CreateGDIBitmap( int nWid,int nHei,void ** ppBits )
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

		HDC hdc=GetDC(NULL);
		//LPVOID pBits=NULL;
		HBITMAP hBmp=CreateDIBSection(hdc,&bmi,DIB_RGB_COLORS,ppBits,0,0);
		ReleaseDC(NULL,hdc);
		return hBmp;
	}

	HBITMAP SBitmap_D2D::GetGdiBitmap()
	{
		const LPBYTE pSrcBits = (const LPBYTE)GetPixelBits();
		LPVOID pDstBits = NULL;
		HBITMAP hRet= CreateGDIBitmap(m_sz.cx,m_sz.cy,&pDstBits);
		memcpy(pDstBits,pSrcBits,m_sz.cx*4*m_sz.cy);
		return hRet;
	}

	//////////////////////////////////////////////////////////////////////////
	//	SRegion_D2D
	SRegion_D2D::SRegion_D2D( IRenderFactory *pRenderFac ,SComPtr<ID2D1Geometry> rgn)
		:TD2DRenderObjImpl<IRegionS,OT_RGN>(pRenderFac)
	{
		if(!rgn)
		{
			ID2D1Factory *pFac = toD2DFac(GetRenderFactory());
			D2D1_RECT_F rc={0};
			SComPtr<ID2D1RectangleGeometry> rgn;
			pFac->CreateRectangleGeometry(&rc,&rgn);
			m_hRgn = rgn;
			m_bRect=TRUE;
		}else
		{
			m_hRgn = rgn;
		}
	}

	SRegion_D2D::~SRegion_D2D()
	{
		m_hRgn=NULL;
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
	void SRegion_D2D::CombineRect( LPCRECT lprect,int nCombineMode )
	{
		SComPtr<ID2D1RectangleGeometry> hRgn; 
		ID2D1Factory *pFac = toD2DFac(GetRenderFactory());
		pFac->CreateRectangleGeometry(toRectF(lprect),&hRgn);
		CombineGeometry((ID2D1Geometry*)hRgn,nCombineMode);
		if(m_bRect){
			if(nCombineMode==RGN_COPY || nCombineMode== RGN_AND)
				m_bRect = TRUE;
			else if(nCombineMode == RGN_OR)
				m_bRect = IsRectGeometry(m_hRgn);
			else
				m_bRect = FALSE;
		}
	}

	void SRegion_D2D::CombinePolygon(const POINT * pts, int count, int nPolygonMode, int nCombineMode)
	{
		if(count<3) return;
		ID2D1Factory *pFac = toD2DFac(GetRenderFactory());
		SComPtr<ID2D1PathGeometry> hRgn; 
		pFac->CreatePathGeometry(&hRgn);
		SComPtr<ID2D1GeometrySink> sink;
		hRgn->Open(&sink);
		sink->BeginFigure(Point2F(pts[count-1].x,pts[count-1].y),D2D1_FIGURE_BEGIN_FILLED);
		for(int i=0;i<count;i++)
		{
			sink->AddLine(Point2F(pts[i].x,pts[i].y));
		}
		sink->EndFigure(D2D1_FIGURE_END_CLOSED);
		sink->Close();

		CombineGeometry(hRgn, nCombineMode);
		m_bRect = FALSE;
	}

	void SRegion_D2D::CombineRgn(const IRegionS * pRgnSrc,int nCombineMode)
	{
		const SRegion_D2D *pRgnSrc2 = (const SRegion_D2D*)pRgnSrc;
		if(pRgnSrc2->IsRect())
		{
			CRect rcBox;
			pRgnSrc2->GetRgnBox(&rcBox);
			CombineRect(&rcBox,nCombineMode);
		}else{
			CombineGeometry(pRgnSrc2->GetRegion(),nCombineMode);
			m_bRect = FALSE;
		}
	}


	void SRegion_D2D::CombineGeometry(ID2D1Geometry* hRgn,int nCombineMode)
	{
		SComPtr<ID2D1PathGeometry> pIPathGeo;
		ID2D1Factory *pFac = toD2DFac(GetRenderFactory());
		pFac->CreatePathGeometry(&pIPathGeo);

		SComPtr<ID2D1GeometrySink> pSink;
		HRESULT hr = pIPathGeo->Open(&pSink);
		if(hr!=S_OK){
			return;
		}
		if(nCombineMode == RGN_COPY){
			hr=hRgn->CombineWithGeometry(hRgn,D2D1_COMBINE_MODE_UNION,NULL,NULL,pSink);
		}else
		{
			D2D1_COMBINE_MODE d2dCombineMode;
			switch(nCombineMode)
			{
			case RGN_OR:d2dCombineMode=D2D1_COMBINE_MODE_UNION;break;
			case RGN_AND:d2dCombineMode=D2D1_COMBINE_MODE_INTERSECT;break;
			case RGN_DIFF:
				d2dCombineMode=D2D1_COMBINE_MODE_EXCLUDE;break;
			case RGN_XOR:
				d2dCombineMode=D2D1_COMBINE_MODE_XOR;break;
			}
			hr=m_hRgn->CombineWithGeometry(hRgn,d2dCombineMode,NULL,NULL,pSink);
		}
		pSink->Close();

		m_hRgn = pIPathGeo;
	}

	BOOL SRegion_D2D::PtInRegion( POINT pt ) const
	{
		BOOL bInRegin=FALSE;
		m_hRgn->FillContainsPoint(Point2F((float)pt.x,(float)pt.y),NULL,0,&bInRegin);
		return bInRegin;
	}

	BOOL SRegion_D2D::RectInRegion( LPCRECT lprect ) const
	{
		D2D1_GEOMETRY_RELATION relation=D2D1_GEOMETRY_RELATION_UNKNOWN;
		SComPtr<ID2D1RectangleGeometry> pRcGeo;
		ID2D1Factory *pFac = toD2DFac(GetRenderFactory());
		pFac->CreateRectangleGeometry(toRectF(lprect),&pRcGeo);
		m_hRgn->CompareWithGeometry(pRcGeo,NULL,0,&relation);
		return relation!=D2D1_GEOMETRY_RELATION_DISJOINT;
	}

	void SRegion_D2D::GetRgnBox( LPRECT lprect ) const
	{
		D2D1_RECT_F rc={0};
		HRESULT hr = m_hRgn->GetBounds(NULL,&rc);
		if(SUCCEEDED(hr)){
			lprect->left=(int)rc.left;
			lprect->top=(int)rc.top;
			lprect->right=(int)rc.right;
			lprect->bottom=(int)rc.bottom;
		}else{
			memset(lprect,0,sizeof(RECT));
		}
	}

	BOOL SRegion_D2D::IsEmpty() const
	{
		RECT rcBox;
		GetRgnBox(&rcBox);
		return (rcBox.left == rcBox.right) || (rcBox.top== rcBox.bottom);
	}

	void SRegion_D2D::Offset( POINT pt )
	{
		ID2D1Factory *pFac = toD2DFac(GetRenderFactory());
		SComPtr<ID2D1TransformedGeometry> hrgn2;
		HRESULT hr = pFac->CreateTransformedGeometry(
			m_hRgn,
			D2D1::Matrix3x2F::Translation(pt.x,pt.y),
			&hrgn2
			);
		m_hRgn = hrgn2;
	}

	ID2D1Geometry* SRegion_D2D::GetRegion() const
	{
		return m_hRgn;
	}

	void SRegion_D2D::Clear()
	{
		ID2D1Factory *pFac = toD2DFac(GetRenderFactory());
		D2D1_RECT_F rc={0};
		SComPtr<ID2D1RectangleGeometry> rgn;
		pFac->CreateRectangleGeometry(&rc,&rgn);
		m_hRgn = rgn;
		m_bRect = TRUE;
	}

	void SRegion_D2D::CombineRoundRect(LPCRECT lprect, POINT ptRadius, int nCombineMode)
	{
		if(ptRadius.x == 0 || ptRadius.y == 0)
			CombineRect(lprect,nCombineMode);
		else{
			ID2D1Factory *pFac = toD2DFac(GetRenderFactory());
			SComPtr<ID2D1RoundedRectangleGeometry> hrgn;
			D2D1_ROUNDED_RECT rc={toRectF(lprect),(float)ptRadius.x,(float)ptRadius.y};
			pFac->CreateRoundedRectangleGeometry(rc,&hrgn);
			CombineGeometry(hrgn,nCombineMode);
			m_bRect = FALSE;
		}
	}

	void SRegion_D2D::CombineEllipse(LPCRECT lprect , int nCombineMode)
	{
		ID2D1Factory *pFac = toD2DFac(GetRenderFactory());
		SComPtr<ID2D1EllipseGeometry> hrgn;
		D2D1_ELLIPSE ellipse;
		ellipse.point.x = (lprect->left+lprect->right)/2;
		ellipse.point.y = (lprect->top+lprect->bottom)/2;
		ellipse.radiusX = (lprect->right-lprect->left)/2;
		ellipse.radiusY = (lprect->bottom-lprect->top)/2;
		pFac->CreateEllipseGeometry(ellipse,&hrgn);
		CombineGeometry(hrgn,nCombineMode);
		m_bRect = FALSE;
	}

	BOOL SRegion_D2D::IsRectGeometry(ID2D1Geometry* geo) const
	{
		D2D1_RECT_F rcBox;
		geo->GetBounds(NULL,&rcBox);
		ID2D1Factory *pFac = toD2DFac(GetRenderFactory());
		D2D1_RECT_F rc={0};
		SComPtr<ID2D1RectangleGeometry> rgn;
		pFac->CreateRectangleGeometry(rcBox,&rgn);
		SRegion_D2D rgn2(m_pRenderFactory,(ID2D1Geometry*)rgn);
		rgn2.CombineGeometry(geo,RGN_DIFF);
		return rgn2.IsEmpty();
	}

	//////////////////////////////////////////////////////////////////////////
	//	SRenderTarget_D2D
	//////////////////////////////////////////////////////////////////////////

	SRenderTarget_D2D::SRenderTarget_D2D( IRenderFactory* pRenderFactory ,int nWid,int nHei)
		:m_curColor(0xFF000000)//默认黑色
		,m_bAntiAlias(TRUE)
		,m_hWnd(NULL)
		,m_cDrawing(0)
		,m_hBmp(NULL)
	{
		m_pRenderFactory = pRenderFactory;

		SRenderFactory_D2D *pFactoryD2D=(SRenderFactory_D2D*)pRenderFactory;
		pFactoryD2D->CreateBitmap((IBitmapS**)&m_curBmp);
		m_curBmp->Init(nWid,nHei,NULL);

		ID2D1Factory *pD2DFac = pFactoryD2D->GetD2D1Factory();
		IWICBitmap * pBmp = m_curBmp->GetBitmap();

		D2D1_RENDER_TARGET_TYPE targetType = D2D1_RENDER_TARGET_TYPE_SOFTWARE;;
		D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat (DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);

		HRESULT hr = pD2DFac->CreateWicBitmapRenderTarget(pBmp,D2D1::RenderTargetProperties (targetType, pixelFormat,0.0,0.0,D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE),(ID2D1RenderTarget**)&m_rt);
		m_ptOrg.x=m_ptOrg.y=0;
		m_mtx = Matrix3x2F::Identity();
		SetAntiAlias(m_bAntiAlias);

		CreatePen(PS_SOLID,SColor(0,0,0).toCOLORREF(),1,&m_defPen);

		CreateSolidColorBrush(SColor(0,0,0).toCOLORREF(),&m_defBrush);

		LOGFONT lf={0};
		lf.lfHeight=20;
		_tcscpy(lf.lfFaceName,_T("宋体"));
		pRenderFactory->CreateFont(&m_defFont,&lf);

		SelectObject(m_defBrush,NULL);
		SelectObject(m_defPen,NULL);
		SelectObject(m_defFont,NULL);

		m_canvsSize.cx = nWid;
		m_canvsSize.cy = nHei;
		m_clipStack.Attach(new SClipStack(m_pRenderFactory,nWid,nHei));
	}

	SRenderTarget_D2D::SRenderTarget_D2D(IRenderFactory* pRenderFactory,HWND hWnd)
		:m_pRenderFactory(pRenderFactory)
		,m_hWnd(hWnd)
		,m_bAntiAlias(TRUE)
		,m_curColor(0xFF000000)//默认黑色
		,m_cDrawing(0)
		,m_hBmp(NULL)
	{
		SRenderFactory_D2D *pFactoryD2D=(SRenderFactory_D2D*)pRenderFactory;
		ID2D1Factory *pD2DFac = pFactoryD2D->GetD2D1Factory();

		CRect rc;
		GetClientRect (m_hWnd, &rc);
		D2D1_SIZE_U size = D2D1::SizeU (static_cast<UINT32> (rc.Width()), static_cast<UINT32>(rc.Height()));
		D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat (DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);

		ID2D1HwndRenderTarget *rt = NULL;
		HRESULT hr = pD2DFac->CreateHwndRenderTarget(D2D1::RenderTargetProperties (D2D1_RENDER_TARGET_TYPE_HARDWARE, pixelFormat,0.0,0.0,D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE),
			D2D1::HwndRenderTargetProperties (m_hWnd, size, D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS),
			&rt);
		if (SUCCEEDED (hr))
		{
			rt->SetDpi (96, 96);
			m_rt = rt;
			rt->Release();
		}
		SetAntiAlias(m_bAntiAlias);

		m_ptOrg.x=m_ptOrg.y=0;
		m_mtx = Matrix3x2F::Identity();

		CreatePen(PS_SOLID,SColor(0,0,0).toCOLORREF(),1,&m_defPen);

		CreateSolidColorBrush(SColor(0,0,0).toCOLORREF(),&m_defBrush);

		LOGFONT lf={0};
		lf.lfHeight=20;
		_tcscpy(lf.lfFaceName,_T("宋体"));
		pRenderFactory->CreateFont(&m_defFont,&lf);

		SelectObject(m_defBrush,NULL);
		SelectObject(m_defPen,NULL);
		SelectObject(m_defFont,NULL);

		m_canvsSize = rc.Size();
		m_clipStack.Attach(new SClipStack(m_pRenderFactory,rc.right,rc.bottom));

	}

	SRenderTarget_D2D::~SRenderTarget_D2D()
	{
	}

	HRESULT SRenderTarget_D2D::CreatePen( int iStyle,COLORREF cr,int cWidth,IPenS ** ppPen )
	{
		*ppPen = new SPen_D2D(m_pRenderFactory,iStyle,cr,cWidth);
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::CreateSolidColorBrush( COLORREF cr,IBrushS ** ppBrush )
	{
		*ppBrush = SBrush_D2D::CreateSolidBrush(m_pRenderFactory,cr);
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::CreateBitmapBrush( IBitmapS *pBmp,TileMode xtm,TileMode ytm,IBrushS ** ppBrush )
	{
		SBitmap_D2D *pBmp2 = (SBitmap_D2D*)pBmp;
		*ppBrush = SBrush_D2D::CreateBitmapBrush(m_pRenderFactory,pBmp2->GetBitmap());
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::CreateGradientBrush(THIS_ const GradientItem *pGradients, int nCount, const GradientInfo *info, BYTE byAlpha, TileMode tileMode, IBrushS **ppBrush)
	{
		*ppBrush = SBrush_D2D::CreateGradientBrush(m_pRenderFactory,pGradients,nCount,info,byAlpha,tileMode);
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::Resize( SIZE sz )
	{
		if(IsOffscreen()){
			SASSERT(m_cDrawing==0);
			m_rt=NULL;
			m_curBmp=NULL;

			m_pRenderFactory->CreateBitmap((IBitmapS**)&m_curBmp);
			m_curBmp->Init(sz.cx,sz.cy,NULL);

			ID2D1Factory *pD2DFac = toD2DFac(m_pRenderFactory);
			IWICBitmap * pBmp = m_curBmp->GetBitmap();

			D2D1_RENDER_TARGET_TYPE targetType = D2D1_RENDER_TARGET_TYPE_SOFTWARE;;
			D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat (DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);

			HRESULT hr = pD2DFac->CreateWicBitmapRenderTarget(pBmp,D2D1::RenderTargetProperties (targetType, pixelFormat,0.0,0.0,D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE),(ID2D1RenderTarget**)&m_rt);
		}else{
			D2D1_SIZE_U size;
			size.width = sz.cx;
			size.height = sz.cy;
			
			ID2D1Factory *pD2DFac = toD2DFac(m_pRenderFactory);

			ID2D1HwndRenderTarget *rt=NULL;
			D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat (DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
			HRESULT hr = pD2DFac->CreateHwndRenderTarget(D2D1::RenderTargetProperties (D2D1_RENDER_TARGET_TYPE_HARDWARE, pixelFormat,0.0,0.0,D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE),
				D2D1::HwndRenderTargetProperties (m_hWnd, size, D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS),
				&rt);
			if (SUCCEEDED (hr))
			{
				m_rt=rt;
				rt->SetDpi (96, 96);
				rt->Release();
			}
		}
		SASSERT(m_rt);
		SetAntiAlias(m_bAntiAlias);

		m_ptOrg = CPoint(0,0);
		m_mtx = Matrix3x2F::Identity();
		m_canvsSize = sz;
		m_clipStack.Attach(new SClipStack(m_pRenderFactory,sz.cx,sz.cy));
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::PushClipRect( LPCRECT pRect ,UINT mode/*=RGN_AND*/)
	{
		if(mode == RGN_AND || mode==RGN_COPY)
		{
			m_rt->PushAxisAlignedClip(toRectF(pRect),D2D1_ANTIALIAS_MODE_ALIASED);
			CRect rc(pRect);
			rc.OffsetRect(m_ptOrg);
			m_clipStack->PushClipRect(&rc,mode);
			return S_OK;
		}else if(mode == RGN_DIFF){
			SAutoRefPtr<IRegionS> rgn;
			this->CreateRegion(&rgn);
			rgn->CombineRect(CRect(0,0,m_canvsSize.cx,m_canvsSize.cy),RGN_COPY);
			rgn->CombineRect(pRect,RGN_DIFF);
			return PushClipRegion(rgn,RGN_AND);
		}
		return E_INVALIDARG;
	}

	HRESULT SRenderTarget_D2D::PopClip()
	{
		if(m_clipStack->GetLayerCount()<2)
			return E_UNEXPECTED;
		bool bLayer = m_clipStack->PopClip();
		if(bLayer){
			m_rt->PopLayer();
		}else{
			m_rt->PopAxisAlignedClip();
		}
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::ExcludeClipRect( LPCRECT pRect )
	{
		return PushClipRect(pRect,RGN_DIFF);
	}

	HRESULT SRenderTarget_D2D::SaveClip( int *pnState )
	{
		m_saveState = m_clipStack->GetLayerCount();
		if(pnState) *pnState=m_saveState;
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::RestoreClip( int nState/*=-1*/ )
	{
		if(nState==-1)
			nState = m_saveState;
		while(m_clipStack->GetLayerCount()>nState){
			bool bLayer = m_clipStack->PopClip();
			if(bLayer){
				m_rt->PopLayer();
			}else{
				m_rt->PopAxisAlignedClip();
			}
		}
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::PushClipRegion( IRegionS *pRegion ,UINT mode/*=RGN_AND*/)
	{
		if(mode != RGN_AND && mode != RGN_COPY)
			return E_INVALIDARG;
		SRegion_D2D *pRegionD2D=(SRegion_D2D*)(IRegionS*)(pRegion);
		SASSERT(pRegionD2D);
		if(pRegionD2D->IsRect())
		{
			CRect rcBox;
			pRegionD2D->GetRgnBox(&rcBox);
			return PushClipRect(&rcBox,mode);
		}

		SComPtr<ID2D1Layer> pLayer;
		m_rt->CreateLayer(&pLayer);
		ID2D1Geometry *pPath = ((SRegion_D2D*)(IRegionS*)pRegion)->GetRegion();
		m_rt->PushLayer(D2D1::LayerParameters(D2D1::InfiniteRect(),pPath,D2D1_ANTIALIAS_MODE_PER_PRIMITIVE),pLayer);

		m_clipStack->PushClipRegion(pRegion,RGN_AND);
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::GetClipRegion( IRegionS **ppRegion )
	{
		SRegion_D2D *pRgn=new SRegion_D2D(m_pRenderFactory);
		SAutoRefPtr<IRegionS> rgn = m_clipStack->GetRegion();
		pRgn->CombineRgn(rgn,RGN_COPY);
		pRgn->Offset(-CPoint(m_ptOrg));
		*ppRegion = pRgn;
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::GetClipBox(LPRECT prc)
	{
		SAutoRefPtr<IRegionS> rgn = m_clipStack->GetRegion();
		rgn->GetRgnBox(prc);
		::OffsetRect(prc,-m_ptOrg.x,-m_ptOrg.y);
		return S_OK;
	}


	HRESULT SRenderTarget_D2D::BitBlt( LPCRECT pRcDest,IRenderTarget *pRTSour,int xSrc,int ySrc,DWORD dwRop/*=SRCCOPY*/)
	{
		if(!pRTSour->IsOffscreen())
			return E_INVALIDARG;

		SRenderTarget_D2D *pRTGdiSrc=(SRenderTarget_D2D*)pRTSour;
		bool bDrawing = pRTGdiSrc->m_cDrawing>0;
		if(bDrawing)
			pRTGdiSrc->m_rt->EndDraw();
		SBitmap_D2D *pSrc = pRTGdiSrc->m_curBmp;
		CRect rcDst(pRcDest);
		CRect rcSrc(xSrc,ySrc,xSrc+rcDst.Width(),ySrc+rcDst.Height());
		rcSrc.OffsetRect(pRTGdiSrc->m_ptOrg);
		m_rt->DrawBitmap(pSrc->toD2D1Bitmap(m_rt),toRectF(pRcDest),1.0f,D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,toRectF(&rcSrc));
		if(bDrawing)
			pRTGdiSrc->m_rt->BeginDraw();
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::DrawText( LPCTSTR pszText,int cchLen,LPRECT pRc,UINT uFormat)
	{
		if(cchLen<0) cchLen = (int)_tcslen(pszText);
		ID2D1Factory *pFac = toD2DFac(m_pRenderFactory);
		IDWriteFactory *pWriteFac = toD2DWriteFac(m_pRenderFactory);
		SFont_D2D *pFont = m_curFont;
		const LOGFONT * lf = pFont->LogFont();
		IDWriteTextFormat *txtFormat = pFont->GetFont();
		if(uFormat & DT_SINGLELINE) txtFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
		else txtFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);

		if(uFormat&DT_CENTER) txtFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		else if(uFormat&DT_RIGHT) txtFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
		else txtFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

		if(uFormat&DT_VCENTER) txtFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		else if(uFormat&DT_BOTTOM) txtFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
		else txtFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

		SComPtr<IDWriteInlineObject> signObj;
		if(uFormat & (DT_END_ELLIPSIS|DT_PATH_ELLIPSIS|DT_WORD_ELLIPSIS))
		{
			DWRITE_TRIMMING trimming;
			trimming.granularity = (uFormat&DT_WORD_ELLIPSIS)?DWRITE_TRIMMING_GRANULARITY_WORD:DWRITE_TRIMMING_GRANULARITY_CHARACTER;
			trimming.delimiter = 0;
			trimming.delimiterCount = 0;
			pWriteFac->CreateEllipsisTrimmingSign(txtFormat,(IDWriteInlineObject**)&signObj);
			txtFormat->SetTrimming(&trimming, signObj);
		}
		SComPtr<IDWriteTextLayout> layout;
#ifdef _UNICODE
		pWriteFac->CreateTextLayout(pszText,cchLen,txtFormat,RectWidth(pRc),RectHeight(pRc),&layout);
#else
		SStringW str=S_CA2W(SStringA(pszText,cchLen));
		pWriteFac->CreateTextLayout(str.c_str(),str.GetLength(),txtFormat,RectWidth(pRc),RectHeight(pRc),&layout);
#endif
		if(uFormat & DT_CALCRECT)
		{
			DWRITE_TEXT_METRICS metrics;
			layout->GetMetrics(&metrics);
			pRc->right = pRc->left + (int)ceil(metrics.widthIncludingTrailingWhitespace);
			pRc->bottom = pRc->top + (int)ceil(metrics.height);
		}else if(cchLen>0){
			DWRITE_TEXT_RANGE range;
			range.startPosition=0;
			range.length = cchLen;
			layout->SetStrikethrough(lf->lfStrikeOut,range);
			layout->SetUnderline(lf->lfUnderline,range);
			SBrush_D2D *pBr = SBrush_D2D::CreateSolidBrush(m_pRenderFactory,m_curColor.toCOLORREF());
			D2D1_POINT_2F pt={ (float)pRc->left,(float)pRc->top};
			m_rt->PushAxisAlignedClip(toRectF(pRc),D2D1_ANTIALIAS_MODE_ALIASED);
			m_rt->DrawTextLayout(pt,layout,pBr->toBrush(m_rt,NULL));
			m_rt->PopAxisAlignedClip();
			pBr->Release();
		}
		if(signObj)
		{
			DWRITE_TRIMMING trimming;
			trimming.granularity = DWRITE_TRIMMING_GRANULARITY_NONE;
			trimming.delimiter = 0;
			trimming.delimiterCount = 0;
			txtFormat->SetTrimming(&trimming, NULL);
		}

		return S_OK;
	}

	HRESULT SRenderTarget_D2D::MeasureText( LPCTSTR pszText,int cchLen, SIZE *psz )
	{
		RECT rc={0,0,10000000,10000000};
		DrawText(pszText,cchLen,&rc,DT_SINGLELINE|DT_CALCRECT);
		psz->cx = rc.right;
		psz->cy = rc.bottom;
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::DrawRectangle(LPCRECT pRect)
	{
		if(!m_curPen) return E_INVALIDARG;
		{
			SPen_D2D *pen=(SPen_D2D*)(IPenS*)m_curPen;
			SComPtr<ID2D1Brush> br = pen->GetColorBrush(m_rt);			
			m_rt->DrawRectangle(toRectF(pRect,TRUE),br,m_curPen->GetWidth(),pen->GetStrokeStyle());
		}
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::FillRectangle(LPCRECT pRect)
	{
		SComPtr<ID2D1Brush> br = m_curBrush->toBrush(m_rt,pRect);
		m_rt->FillRectangle(toRectF(pRect,TRUE),br);
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::DrawRoundRect( LPCRECT pRect,POINT pt )
	{
		SPen_D2D *pen=(SPen_D2D*)(IPenS*)m_curPen;
		SComPtr<ID2D1Brush> br = pen->GetColorBrush(m_rt);			
		D2D1_ROUNDED_RECT rcRound={toRectF(pRect,TRUE),(float)pt.x,(float)pt.y};
		m_rt->DrawRoundedRectangle(rcRound,br,m_curPen->GetWidth(),pen->GetStrokeStyle());
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::FillRoundRect( LPCRECT pRect,POINT pt )
	{
		SComPtr<ID2D1Brush> br = m_curBrush->toBrush(m_rt,pRect);
		D2D1_ROUNDED_RECT rcRound={toRectF(pRect,TRUE),(float)pt.x,(float)pt.y};
		m_rt->FillRoundedRectangle(rcRound,br);
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::FillSolidRoundRect(LPCRECT pRect,POINT pt,COLORREF cr)
	{		
		SComPtr<ID2D1SolidColorBrush> br;
		D2DColor color(cr);
		m_rt->CreateSolidColorBrush(color.toD2DColor(),&br);
		D2D1_ROUNDED_RECT rcRound={toRectF(pRect,TRUE),(float)pt.x,(float)pt.y};
		m_rt->FillRoundedRectangle(rcRound,br);
		return S_OK;    
	}

	HRESULT SRenderTarget_D2D::DrawLines(LPPOINT pts,size_t nCount)
	{
		SComPtr<ID2D1PathGeometry> path;
		ID2D1Factory *fac = toD2DFac(m_pRenderFactory);
		fac->CreatePathGeometry(&path);
		SComPtr<ID2D1GeometrySink> sink;
		path->Open(&sink);
		sink->BeginFigure(Point2F(pts[0].x,pts[0].y),D2D1_FIGURE_BEGIN_HOLLOW);
		for(int i=1;i<nCount;i++)
		{
			sink->AddLine(Point2F(pts[i].x,pts[i].y));
		}
		sink->EndFigure(D2D1_FIGURE_END_OPEN);
		sink->Close();

		SPen_D2D *pen=m_curPen;
		m_rt->DrawGeometry(path,pen->GetColorBrush(m_rt),pen->GetWidth(),pen->GetStrokeStyle());
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::TextOut( int x, int y, LPCTSTR lpszString, int nCount)
	{
		if(nCount<0) nCount = (int)_tcslen(lpszString);
		SIZE sz;
		MeasureText(lpszString,nCount,&sz);
		RECT rc={x,y,x+sz.cx,y+sz.cy};
		return DrawText(lpszString,nCount,&rc,DT_LEFT|DT_SINGLELINE);
	}

	HRESULT SRenderTarget_D2D::DrawIconEx( int xLeft, int yTop, HICON hIcon, int cxWidth,int cyWidth,UINT diFlags )
	{
		IWICImagingFactory *imgFac = toD2DWicImgFac(m_pRenderFactory);
		SComPtr<IWICBitmap> bmp;
		// 获取图标信息
		ICONINFO iconInfo;
		GetIconInfo(hIcon, &iconInfo);

		HRESULT hr = imgFac->CreateBitmapFromHBITMAP(iconInfo.hbmColor,NULL,
			WICBitmapUsePremultipliedAlpha,
			&bmp);
		if(hr == S_OK){
			SComPtr<ID2D1Bitmap> pBitmap ;
			hr = m_rt->CreateBitmapFromWicBitmap(bmp,&pBitmap);
			if(hr==S_OK){
				m_rt->DrawBitmap(pBitmap, D2D1::RectF(xLeft, yTop, xLeft+cxWidth,yTop+cyWidth));
			}
		}
		if(iconInfo.hbmColor){
			DeleteObject(iconInfo.hbmColor);
		}
		if(iconInfo.hbmMask){
			DeleteObject(iconInfo.hbmMask);
		}

		return hr;
	}

	HRESULT SRenderTarget_D2D::DrawBitmap(LPCRECT pRcDest,const IBitmapS *pBitmap,int xSrc,int ySrc,BYTE byAlpha/*=0xFF*/ )
	{
		SBitmap_D2D *pBmp = (SBitmap_D2D*)pBitmap;
		SComPtr<ID2D1Bitmap> bmp=pBmp->toD2D1Bitmap(m_rt);
		m_rt->DrawBitmap(bmp,toRectF(pRcDest),float(byAlpha)/255.f,D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,RectF(xSrc,ySrc,xSrc+RectWidth(pRcDest),ySrc+RectHeight(pRcDest)));
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::AlphaBlend( LPCRECT pRcDest,IRenderTarget *pRTSrc,LPCRECT pRcSrc,BYTE byAlpha )
	{
		if(!pRTSrc->IsOffscreen())
			return E_INVALIDARG;
		SRenderTarget_D2D *pRTGdiSrc=(SRenderTarget_D2D*)pRTSrc;
		bool bDrawing = pRTGdiSrc->m_cDrawing>0;
		if(bDrawing)
			pRTGdiSrc->m_rt->EndDraw();
		SBitmap_D2D *pSrc = pRTGdiSrc->m_curBmp;
		CRect rcSrc(pRcSrc);
		rcSrc.OffsetRect(pRTGdiSrc->m_ptOrg);
		m_rt->DrawBitmap(pSrc->toD2D1Bitmap(m_rt),toRectF(pRcDest),float(byAlpha)/255.f,D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,toRectF(&rcSrc));
		if(bDrawing)
			pRTGdiSrc->m_rt->BeginDraw();

		return S_OK;
	}

	HRESULT SRenderTarget_D2D::DrawBitmapEx( LPCRECT pRcDest,const IBitmapS *pBitmap,LPCRECT pRcSrc,UINT expendMode, BYTE byAlpha/*=0xFF*/ )
	{
		UINT expandModeLow = LOWORD(expendMode);
		FilterLevel  quality = (FilterLevel)HIWORD(expendMode);

		if(expandModeLow == EM_NULL)
			return DrawBitmap(pRcDest,pBitmap,pRcSrc->left,pRcSrc->top,byAlpha);

		SBitmap_D2D *pBmp = (SBitmap_D2D*)pBitmap;

		BLENDFUNCTION bf={ AC_SRC_OVER,0,byAlpha,AC_SRC_ALPHA};
		SComPtr<ID2D1Bitmap> bmp = pBmp->toD2D1Bitmap(m_rt);

		D2D1_BITMAP_INTERPOLATION_MODE mode = quality<kMedium_FilterLevel?D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR:D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
		if(expandModeLow == EM_STRETCH)
		{
			m_rt->DrawBitmap(bmp,toRectF(pRcDest),float(byAlpha)/255.f,mode,toRectF(pRcSrc));
		}else
		{//tile mode
			PushClipRect(pRcDest,RGN_AND);

			CRect rcDest(pRcDest);
			CRect rcSrc(pRcSrc);
			CRect rcSubDest(0,0,rcSrc.Width(),rcSrc.Height()); 
			for(int y=rcDest.top;y<rcDest.bottom;y+=rcSrc.Height())
			{
				rcSubDest.OffsetRect(rcDest.left,y);               
				for(int x=rcDest.left;x<rcDest.right;x += rcSrc.Width())
				{
					m_rt->DrawBitmap(bmp,toRectF(rcSubDest),float(byAlpha)/255.f,mode,toRectF(rcSrc));
					rcSubDest.OffsetRect(rcSrc.Width(),0);
				}
			}
			PopClip();
		}
		return S_OK;

	}


	HRESULT SRenderTarget_D2D::DrawBitmap9Patch( LPCRECT pRcDest,const IBitmapS *pBitmap,LPCRECT pRcSrc,LPCRECT pRcSourMargin,UINT expendMode,BYTE byAlpha/*=0xFF*/ )
	{
		int xDest[4] = {pRcDest->left,pRcDest->left+pRcSourMargin->left,pRcDest->right-pRcSourMargin->right,pRcDest->right};
		int xSrc[4] = {pRcSrc->left,pRcSrc->left+pRcSourMargin->left,pRcSrc->right-pRcSourMargin->right,pRcSrc->right};
		int yDest[4] = {pRcDest->top,pRcDest->top+pRcSourMargin->top,pRcDest->bottom-pRcSourMargin->bottom,pRcDest->bottom};
		int ySrc[4] = {pRcSrc->top,pRcSrc->top+pRcSourMargin->top,pRcSrc->bottom-pRcSourMargin->bottom,pRcSrc->bottom};

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

	IRenderObj * SRenderTarget_D2D::GetCurrentObject( OBJTYPE uType )
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

	HRESULT SRenderTarget_D2D::SelectDefaultObject(OBJTYPE objType, IRenderObj ** ppOldObj)
	{
		IRenderObj *pDefObj = NULL;
		switch(objType)
		{
		case OT_PEN: pDefObj = m_defPen;break;
		case OT_BRUSH: pDefObj = m_defBrush;break;
		case OT_FONT: pDefObj = m_defFont;break;
		default:return E_INVALIDARG;
		}
		if(pDefObj == GetCurrentObject(objType)) 
			return S_FALSE;
		return SelectObject(pDefObj,ppOldObj);
	}

	HRESULT SRenderTarget_D2D::SelectObject( IRenderObj *pObj,IRenderObj ** ppOldObj /*= NULL*/ )
	{
		HRESULT hr = S_OK;
		SAutoRefPtr<IRenderObj> pRet;
		switch(pObj->ObjectType())
		{
		case OT_BITMAP: 
			if(IsOffscreen()){
				ID2D1Factory *pD2DFac = toD2DFac(m_pRenderFactory);
				IWICBitmap * pBmp = m_curBmp->GetBitmap();

				pRet=m_curBmp;
				m_curBmp=(SBitmap_D2D*)pObj;
				m_rt=NULL;

				D2D1_RENDER_TARGET_TYPE targetType = D2D1_RENDER_TARGET_TYPE_SOFTWARE;
				D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat (DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
				pD2DFac->CreateWicBitmapRenderTarget(pBmp,D2D1::RenderTargetProperties (targetType, pixelFormat,0.0,0.0, D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE),(ID2D1RenderTarget**)&m_rt);
			}else
			{
				SASSERT(FALSE);
				hr = E_INVALIDARG;
			}
			break;
		case OT_PEN:
			pRet=m_curPen;
			m_curPen=(SPen_D2D*)pObj;
			break;
		case OT_BRUSH:
			pRet=m_curBrush;
			m_curBrush=(SBrush_D2D*)pObj;
			break;
		case OT_FONT:
			pRet=m_curFont;
			m_curFont=(SFont_D2D*)pObj;
			break;
		}
		if(pRet && ppOldObj)
		{//由调用者调用Release释放该RenderObj
			pRet->AddRef();
			*ppOldObj = pRet;
		}
		return hr;
	}

	HRESULT SRenderTarget_D2D::OffsetViewportOrg( int xOff, int yOff, LPPOINT lpPoint )
	{
		if(lpPoint)
		{
			*lpPoint=m_ptOrg;
		}
		POINT ptOrg = m_ptOrg;
		ptOrg.x+=xOff;
		ptOrg.y+=yOff;
		return SetViewportOrg(ptOrg);
	}

	HRESULT SRenderTarget_D2D::GetViewportOrg( LPPOINT lpPoint )
	{
		if(lpPoint)
		{
			*lpPoint=m_ptOrg;
		}
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::SetViewportOrg( POINT pt )
	{
		m_ptOrg = pt;
		if(m_mtx.IsIdentity())
		{
			m_rt->SetTransform(Matrix3x2F::Translation(pt.x,pt.y));
		}else
		{
			Matrix3x2F mtx = Matrix3x2F::Translation(m_ptOrg.x,m_ptOrg.y)*Matrix3x2F::Translation(-m_ptOrg.x,-m_ptOrg.y)*m_mtx*Matrix3x2F::Translation(m_ptOrg.x,m_ptOrg.y);
			m_rt->SetTransform(mtx);
		}
		return S_OK;
	}

	HDC SRenderTarget_D2D::GetDC( UINT uFlag )
	{
		if(m_cDrawing || !IsOffscreen()){
			if(m_rtGdi){
				SASSERT(FALSE);
			}
			m_rtGdi = m_rt;
			HDC hdc=0;
			if(!m_cDrawing)
				m_rt->BeginDraw();
			HRESULT hr = m_rtGdi->GetDC(D2D1_DC_INITIALIZE_MODE_COPY,&hdc);
			if(hdc){
				::SetViewportOrgEx(hdc,m_ptOrg.x,m_ptOrg.y,NULL);
			}
			return hdc;
		}else{
			SAutoRefPtr<SBitmap_D2D> bmp2=m_curBmp;
			HBITMAP bmp=bmp2->GetGdiBitmap();//bmp可能为NULL
			HDC hdc_desk = ::GetDC(NULL);
			HDC hdc = CreateCompatibleDC(hdc_desk);
			::ReleaseDC(NULL,hdc_desk);
			::SelectObject(hdc,bmp);
			::SetViewportOrgEx(hdc,m_ptOrg.x,m_ptOrg.y,NULL);
			m_hBmp = bmp;
			return hdc;
		}
	}

	void SRenderTarget_D2D::ReleaseDC( HDC hdc ,LPCRECT pRc)
	{
		//update bitmap data to m_curBmp
		if(m_cDrawing || !IsOffscreen()){
			if(hdc){
				if(pRc){
					CRect rc(pRc);
					rc.OffsetRect(m_ptOrg);
					m_rtGdi->ReleaseDC(&rc);
				}else{
					m_rtGdi->ReleaseDC(NULL);
				}
			}
			if(!m_cDrawing)
				m_rt->EndDraw();
			m_rtGdi=NULL;
		}else{
			DeleteDC(hdc);
			LPVOID pBits = m_curBmp->LockPixelBits();
			if(pBits){
				BITMAP bm;
				GetObject(m_hBmp,sizeof(bm),&bm);
				memcpy(pBits,bm.bmBits,bm.bmWidth*4*bm.bmHeight);
				m_curBmp->UnlockPixelBits(pBits);
			}
			::DeleteObject(m_hBmp);
			m_hBmp=NULL;
		}
	}

	HRESULT SRenderTarget_D2D::DrawGradientRect
		(LPCRECT pRect, BOOL bVert, POINT ptRoundCorner, const GradientItem *pGradients, int nCount, BYTE byAlpha){
			GradientInfo info;
			info.type = linear;
			info.angle = bVert?90.0f:0.0f;
			return DrawGradientRectEx(pRect,ptRoundCorner,pGradients,nCount,&info,byAlpha);
	}

	HRESULT SRenderTarget_D2D::DrawGradientRectEx
		(LPCRECT pRect, POINT ptRoundCorner, const GradientItem *pGradients, int nCount, const GradientInfo *info, BYTE byAlpha){
			ID2D1Factory *fac = toD2DFac(m_pRenderFactory);
			if(pRect->left == pRect->right || pRect->top == pRect->bottom)
				return S_OK;

			SComPtr<ID2D1Geometry> path;
			HRESULT hr=E_FAIL;
			if(ptRoundCorner.x != 0 || ptRoundCorner.y!=0){
				D2D1_ROUNDED_RECT rect;
				rect.rect = toRectF(pRect);
				rect.radiusX=ptRoundCorner.x;
				rect.radiusY=ptRoundCorner.y;
				SComPtr<ID2D1RoundedRectangleGeometry> path2;
				hr = fac->CreateRoundedRectangleGeometry(rect,&path2);
				path = path2;
			}else{
				SComPtr<ID2D1RectangleGeometry> path2;
				D2D1_RECT_F rect = toRectF(pRect);
				hr = fac->CreateRectangleGeometry(rect,&path2);
				path = path2;
			}
			if(!path)
				return hr;
			SAutoRefPtr<SBrush_D2D> br;
			hr = CreateGradientBrush(pGradients,nCount,info,byAlpha,kClamp_TileMode,(IBrushS**)&br);
			SComPtr<ID2D1Brush> d2dBr = br->toBrush(m_rt,pRect);
			if(d2dBr){
				m_rt->FillGeometry (path, d2dBr);
			}
			return hr;
	}

	HRESULT SRenderTarget_D2D::DrawGradientPath(THIS_ const IPathS* path, const GradientItem *pGradients, int nCount, const GradientInfo *info, BYTE byAlpha)
	{
		SPath_D2D *path2 = (SPath_D2D*)path;
		SAutoRefPtr<SBrush_D2D> br;
		CreateGradientBrush(pGradients,nCount,info,byAlpha,kClamp_TileMode,(IBrushS**)&br);
		CRect rcBox;
		path->getBounds(&rcBox);
		SComPtr<ID2D1Brush> d2dBr = br->toBrush(m_rt,&rcBox);
		SPen_D2D *pen=m_curPen;
		m_rt->DrawGeometry(path2->GetPath(),d2dBr,pen->GetWidth(),pen->GetStrokeStyle());
		return S_OK;
	}


	//通过一个内存位图来填充位置的alpha值
	HRESULT SRenderTarget_D2D::FillSolidRect( LPCRECT pRect,COLORREF cr )
	{
		SBrush_D2D *pBr = SBrush_D2D::CreateSolidBrush(m_pRenderFactory,cr);
		D2D1_MATRIX_3X2_F  mtx;
		m_rt->GetTransform(&mtx);
		m_rt->FillRectangle(toRectF(pRect),pBr->toBrush(m_rt,pRect));
		pBr->Release();
		return S_OK;    
	}

	HRESULT SRenderTarget_D2D::ClearRect( LPCRECT pRect,COLORREF cr )
	{
		int nWid=pRect->right-pRect->left;
		int nHei=pRect->bottom-pRect->top;
		if(nWid == 0 || nHei == 0)
			return S_OK;

		m_rt->PushAxisAlignedClip(toRectF(pRect),D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		D2DColor color(cr);
		m_rt->Clear(color.toD2DColor());
		m_rt->PopAxisAlignedClip();

		return S_OK;    
	}

	HRESULT SRenderTarget_D2D::InvertRect(LPCRECT pRect)
	{
		SComQIPtr<ID2D1GdiInteropRenderTarget> gdiRt(m_rt);
		if(!gdiRt)
			return E_FAIL;
		HDC hdc=0;
		gdiRt->GetDC(D2D1_DC_INITIALIZE_MODE_COPY,&hdc);
		if(hdc){
			::InvertRect(hdc,pRect);
			gdiRt->ReleaseDC(pRect);
		}
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::DrawEllipse( LPCRECT pRect )
	{
		if(!m_curPen) return E_INVALIDARG;
		D2D1_ELLIPSE shape={{(float)(pRect->left+pRect->right)/2,(float)(pRect->top+pRect->bottom)/2},(float)RectWidth(pRect)/2,(float)RectHeight(pRect)/2};
		m_rt->FillEllipse(shape,m_curBrush->toBrush(m_rt,pRect));
		m_rt->DrawEllipse(shape,m_curPen->GetColorBrush(m_rt),m_curPen->GetWidth(),m_curPen->GetStrokeStyle());
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::FillEllipse( LPCRECT pRect )
	{
		D2D1_ELLIPSE shape={{(float)(pRect->left+pRect->right)/2,(float)(pRect->top+pRect->bottom)/2},(float)RectWidth(pRect)/2,(float)RectHeight(pRect)/2};
		m_rt->FillEllipse(shape,m_curBrush->toBrush(m_rt,pRect));
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::FillSolidEllipse(LPCRECT pRect,COLORREF cr)
	{
		D2D1_ELLIPSE shape={{(float)(pRect->left+pRect->right)/2,(float)(pRect->top+pRect->bottom)/2},(float)RectWidth(pRect)/2,(float)RectHeight(pRect)/2};
		SBrush_D2D *pBr = SBrush_D2D::CreateSolidBrush(m_pRenderFactory,cr);
		m_rt->FillEllipse(shape,pBr->toBrush(m_rt,pRect));
		pBr->Release();
		return S_OK;    
	}


	HRESULT SRenderTarget_D2D::DrawArc( LPCRECT pRect,float startAngle,float sweepAngle,BOOL useCenter )
	{
		SPath_D2D path(m_pRenderFactory);
		path.addArc(pRect,startAngle,sweepAngle);
		return DrawPath(&path,NULL);
	}

	HRESULT SRenderTarget_D2D::DrawArc2(LPCRECT pRect, float startAngle, float sweepAngle, int width) {
		return E_NOTIMPL;
	}

	HRESULT SRenderTarget_D2D::FillArc( LPCRECT pRect,float startAngle,float sweepAngle )
	{
		SPath_D2D path(m_pRenderFactory);
		path.addArc(pRect,startAngle,sweepAngle);
		return FillPath(&path);
	}

	HRESULT SRenderTarget_D2D::SetTransform(const float matrix[9], float oldMatrix[9])
	{

		Matrix3x2F xForm (matrix[kMScaleX],matrix[kMSkewY],
			matrix[kMSkewX],matrix[kMScaleY],
			matrix[kMTransX],matrix[kMTransY]);
		if(oldMatrix)
		{
			GetTransform(oldMatrix);
		}
		m_mtx = xForm;
		if(m_ptOrg.x!=0 || m_ptOrg.y!=0){
			xForm = Matrix3x2F::Translation(m_ptOrg.x,m_ptOrg.y)*Matrix3x2F::Translation(-m_ptOrg.x,-m_ptOrg.y)*xForm*Matrix3x2F::Translation(m_ptOrg.x,m_ptOrg.y);
		}
		m_rt->SetTransform(&xForm);
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::GetTransform( float matrix[9]) const
	{
		SASSERT(matrix);
		matrix[kMScaleX]=m_mtx._11; 
		matrix[kMSkewX]=m_mtx._21;
		matrix[kMTransX]=m_mtx._31;
		matrix[kMSkewY]=m_mtx._12;		
		matrix[kMScaleY]=m_mtx._22;
		matrix[kMTransY] =m_mtx._32;

		matrix[kMPersp0] = 0.0f;
		matrix[kMPersp1] = 0.0f;
		matrix[kMPersp2] = 1.0f;
		return S_OK;
	}

	COLORREF SRenderTarget_D2D::GetPixel( int x, int y )
	{
		SComQIPtr<ID2D1GdiInteropRenderTarget> gdiRt(m_rt);
		if(gdiRt){
			HDC hdc=0;
			gdiRt->GetDC(D2D1_DC_INITIALIZE_MODE_COPY,&hdc);
			COLORREF crRet = ::GetPixel(hdc,x,y);
			RECT rcUpdate={0,0,0,0};
			gdiRt->ReleaseDC(&rcUpdate);
			return crRet;
		}
		return CR_INVALID;
	}

	COLORREF SRenderTarget_D2D::SetPixel( int x, int y, COLORREF cr )
	{
		SComQIPtr<ID2D1GdiInteropRenderTarget> gdiRt(m_rt);
		if(gdiRt){
			HDC hdc=0;
			gdiRt->GetDC(D2D1_DC_INITIALIZE_MODE_COPY,&hdc);
			RECT rcUpdate={x,y,x+1,y+1};
			COLORREF crRet = ::GetPixel(hdc,x,y);
			gdiRt->ReleaseDC(&rcUpdate);
			return crRet;
		}
		return CR_INVALID;
	}

	HRESULT SRenderTarget_D2D::CreateRegion(IRegionS ** ppRegion)
	{
		return m_pRenderFactory->CreateRegion(ppRegion)?S_OK:E_OUTOFMEMORY;
	}

	HRESULT SRenderTarget_D2D::PushClipPath(const IPathS * path, UINT mode, BOOL doAntiAlias /*= false*/)
	{
		SPath_D2D *path2 = (SPath_D2D*)path;
		SRegion_D2D region(m_pRenderFactory,path2->GetPath());
		return PushClipRegion(&region,mode);
	}

	HRESULT SRenderTarget_D2D::DrawPath(const IPathS * path,IPathEffect * pathEffect)
	{
		SPath_D2D *path2 = (SPath_D2D*)path;
		SPen_D2D *pen=m_curPen;
		m_rt->DrawGeometry(path2->GetPath(),pen->GetColorBrush(m_rt),pen->GetWidth(),pen->GetStrokeStyle());
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::FillPath(const IPathS * path)
	{
		SPath_D2D *path2 = (SPath_D2D*)path;
		RECT rcBox;
		path->getBounds(&rcBox);
		SComPtr<ID2D1Brush> br = m_curBrush->toBrush(m_rt,&rcBox);
		m_rt->FillGeometry(path2->GetPath(),br);
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::SetXfermode(int mode,int *pOldMode)
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
		return E_NOTIMPL;
	}

	BOOL SRenderTarget_D2D::SetAntiAlias(BOOL bAntiAlias)
	{
		m_bAntiAlias = bAntiAlias;
		if(bAntiAlias)
			m_rt->SetAntialiasMode (D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		else
			m_rt->SetAntialiasMode (D2D1_ANTIALIAS_MODE_ALIASED);
		return TRUE;
	}

	BOOL SRenderTarget_D2D::GetAntiAlias() SCONST
	{
		return m_bAntiAlias;
	}

	COLORREF SRenderTarget_D2D::GetTextColor()
	{
		return m_curColor.toCOLORREF();
	}

	COLORREF SRenderTarget_D2D::SetTextColor(COLORREF color)
	{
		COLORREF crOld=m_curColor.toCOLORREF();
		m_curColor.setRGB(color);
		return crOld;
	}
	
	void SRenderTarget_D2D::BeginDraw(THIS)
	{
		if(0==m_cDrawing++)
			m_rt->BeginDraw();
	}

	void SRenderTarget_D2D::EndDraw(THIS)
	{
		if(--m_cDrawing==0)
		{	
			HRESULT hr = m_rt->EndDraw();
			if (hr == D2DERR_RECREATE_TARGET)
			{
				Resize(m_canvsSize);//recreate render target.
			}else if(hr==S_OK){
				m_rt->Flush();
			}
		}
	}

	BOOL SRenderTarget_D2D::IsOffscreen(CTHIS) SCONST
	{
		return m_hWnd==NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	namespace RENDER_D2D
	{
		BOOL SCreateInstance(IObjRef ** ppRenderFactory)
		{
			*ppRenderFactory = new SRenderFactory_D2D;
			return TRUE;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	SClipStack::SClipStack(IRenderFactory* pRenderFactory,int nWid,int nHei)
	{
		m_fac = pRenderFactory;
		SAutoRefPtr<IRegionS> rgnInit;
		m_fac->CreateRegion((IRegionS**)&rgnInit);
		RECT rc={0,0,nWid,nHei};
		rgnInit->CombineRect(&rc,RGN_COPY);
		LayerInfo info={rgnInit,false};
		m_stack.AddTail(info);
	}

	void SClipStack::PushClipRect(LPCRECT pRect ,UINT mode)
	{
		SRegion_D2D rgn(m_fac);
		rgn.CombineRect(pRect,RGN_COPY);
		SAutoRefPtr<IRegionS> rgn2 = CombineRegion(&rgn,mode);
		LayerInfo layer={rgn2,false};
		m_stack.AddTail(layer);
	}

	SAutoRefPtr<IRegionS> SClipStack::CombineRegion(IRegionS * rgn ,UINT mode)
	{
		LayerInfo &layer = m_stack.GetTail();
		SAutoRefPtr<IRegionS> rgnTmp;
		m_fac->CreateRegion(&rgnTmp);
		rgnTmp->CombineRgn(layer.rgn,RGN_COPY);
		rgnTmp->CombineRgn(rgn,mode);
		return rgnTmp;
	}

	void SClipStack::PushClipRegion(SAutoRefPtr<IRegionS> rgn ,UINT mode)
	{
		SAutoRefPtr<IRegionS> rgn2 = CombineRegion(rgn,mode);
		LayerInfo layer={rgn2,true};
		m_stack.AddTail(layer);
	}

	bool SClipStack::PopClip()
	{
		SASSERT(m_stack.GetCount()>=2);
		return m_stack.RemoveTail().bLayer;
	}

	SAutoRefPtr<IRegionS> SClipStack::GetRegion() const
	{
		return m_stack.GetTail().rgn;
	}

	int SClipStack::GetLayerCount() const
	{
		return (int)m_stack.GetCount();
	}


	//////////////////////////////////////////////////////////////////////////
	SPath_D2D::SPath_D2D(IRenderFactory *pRenderFac)
		:TD2DRenderObjImpl<IPathS,OT_PATH>(pRenderFac)
		,m_fillType(kWinding_FillType)
		,m_bFigureOpen(FALSE)
	{
		reset();
	}

	FillType SPath_D2D::getFillType(THIS) const
	{
		return m_fillType;
	}

	void SPath_D2D::setFillType(THIS_ FillType ft)
	{
		m_fillType =ft;
		if(m_sink){
			D2D1_FILL_MODE fillMode = ft == kWinding_FillType?D2D1_FILL_MODE_WINDING:D2D1_FILL_MODE_ALTERNATE;
			m_sink->SetFillMode(fillMode);
		}
	}

	void SPath_D2D::reset(THIS)
	{
		ID2D1Factory *pFac = toD2DFac(GetRenderFactory());
		m_sink=NULL;
		m_path=NULL;
		HRESULT hr = pFac->CreatePathGeometry(&m_path);
		m_bFigureOpen = FALSE;
		hr = m_path->Open(&m_sink);
		if(m_sink){
			D2D1_FILL_MODE fillMode = m_fillType == kWinding_FillType?D2D1_FILL_MODE_WINDING:D2D1_FILL_MODE_ALTERNATE;
			m_sink->SetFillMode(fillMode);
		}
		m_geometry = m_path;
	}

	BOOL SPath_D2D::isEmpty(THIS) const
	{
		UINT32 nSegCount=0;
		m_path->GetSegmentCount(&nSegCount);
		return nSegCount==0;
	}

	void SPath_D2D::getBounds(CTHIS_ LPRECT prc) const
	{
		D2D1_RECT_F rc;
		HRESULT hr = m_path->GetBounds(NULL,&rc);
		if(SUCCEEDED(hr)){
			prc->left = rc.left;
			prc->top = rc.top;
			prc->right = rc.right;
			prc->bottom = rc.bottom;
		}
	}

	void SPath_D2D::moveTo(THIS_ float x, float y)
	{
		if(m_bFigureOpen)
			endFigure(FALSE);
		beginFigure(x,y,TRUE);
	}

	void SPath_D2D::rMoveTo(THIS_ float dx, float dy)
	{
		moveTo(m_lastPt.x+dx,m_lastPt.y+dy);
	}

	void SPath_D2D::lineTo(THIS_ float x, float y)
	{
		if(!m_bFigureOpen)
			return;
		D2D1_POINT_2F pt={x,y};
		m_sink->AddLine(pt);
		m_lastPt = pt;
	}

	void SPath_D2D::rLineTo(THIS_ float dx, float dy)
	{
		lineTo(m_lastPt.x+dx,m_lastPt.y+dy);
	}

	void SPath_D2D::quadTo(THIS_ float x1, float y1, float x2, float y2)
	{
		if(!m_bFigureOpen)
			return;
		D2D1_QUADRATIC_BEZIER_SEGMENT bezier;
		bezier.point1.x = x1;
		bezier.point1.y = y1;
		bezier.point2.x = x2;
		bezier.point2.y = y2;
		m_sink->AddQuadraticBezier(&bezier);
		m_lastPt = bezier.point2;
	}

	void SPath_D2D::rQuadTo(THIS_ float dx1, float dy1, float dx2, float dy2)
	{
		quadTo(m_lastPt.x+dx1,m_lastPt.y+dy1,m_lastPt.x+dx2,m_lastPt.y+dy2);
	}

	void SPath_D2D::conicTo(THIS_ float x1, float y1, float x2, float y2, float w)
	{
		if(!m_bFigureOpen)
			return;

		D2D1_POINT_2F controlPoint={x1,y1};  // 控制点
		D2D1_POINT_2F endPoint={x2,y2};  // 结束点

		D2D1_POINT_2F diff = {controlPoint.x - m_lastPt.x,controlPoint.y - m_lastPt.y};
		endPoint.x = m_lastPt.x + diff.x*(1.0f-w);
		endPoint.y = m_lastPt.y + diff.y*(1.0f-w);

		// 添加贝塞尔曲线
		m_sink->AddBezier(D2D1::BezierSegment(controlPoint, controlPoint, endPoint));
	}

	void SPath_D2D::rConicTo(THIS_ float dx1, float dy1, float dx2, float dy2, float w)
	{
		conicTo(m_lastPt.x+dx1,m_lastPt.y+dy1,m_lastPt.x+dx2,m_lastPt.y+dy2,2);
	}

	void SPath_D2D::cubicTo(THIS_ float x1, float y1, float x2, float y2, float x3, float y3)
	{
		if(!m_bFigureOpen)
			return;
		D2D1_BEZIER_SEGMENT bezier;
		bezier.point1.x = x1;
		bezier.point1.y = y1;
		bezier.point2.x = x2;
		bezier.point2.y = y2;
		bezier.point3.x = x3;
		bezier.point3.y = y3;
		m_sink->AddBezier(&bezier);
		m_lastPt = bezier.point3;
	}

	void SPath_D2D::rCubicTo(THIS_ float dx1, float dy1, float dx2, float dy2, float dx3, float dy3)
	{
		cubicTo(m_lastPt.x+dx1,m_lastPt.y+dy1,m_lastPt.x+dx2,m_lastPt.y+dy2,m_lastPt.x+dx3,m_lastPt.y+dy3);
	}

	static inline double radians (double degrees) { return degrees * M_PI / 180; }

	void SPath_D2D::close(THIS)
	{
		if(m_bFigureOpen)
			endFigure(FALSE);
		m_sink->Close();
		m_sink=NULL;
	}

	void SPath_D2D::addRect(THIS_ const RECT* rect, Direction dir /*= kCW_Direction*/)
	{
		addRect2(rect->left,rect->top,rect->right,rect->bottom,dir);
	}

	void SPath_D2D::addRect2(THIS_ float left, float top, float right, float bottom, Direction dir/* = kCW_Direction*/)
	{
		if(!m_sink)
			return;
		D2D1_POINT_2F points[4] = {
			{right, top},
			{right, bottom},
			{left, bottom},
			{left, top}
		};
		if (m_bFigureOpen && (m_lastPt.x !=left || m_lastPt.y!=top))
		{
			endFigure(FALSE);
		}
		if (!m_bFigureOpen)
		{
			beginFigure(left,top,TRUE);
		}
		m_sink->AddLine (points[0]);
		m_sink->AddLine (points[1]);
		m_sink->AddLine (points[2]);
		m_sink->AddLine (points[3]);
		m_lastPt = points[3];
	}

	void SPath_D2D::addOval(THIS_ const RECT * oval, Direction dir/*= kCW_Direction*/)
	{
		addOval2(oval->left,oval->top,oval->right,oval->bottom,dir);
	}

	void SPath_D2D::addOval2(THIS_ float left, float top, float right, float bottom, Direction dir/*= kCW_Direction*/)
	{
		if(!m_sink)
			return;
		D2D1_RECT_F r = {left,top,right,bottom};
		D2D1_POINT_2F ptTop = {r.left,r.top};
		ptTop.x += (r.right-r.left) / 2.;
		D2D1_POINT_2F ptBottom={r.left,r.bottom};
		ptBottom.x += (r.right-r.left) / 2.;
		if (m_bFigureOpen && (m_lastPt.x !=r.left || m_lastPt.y!=r.top))
		{
			endFigure(FALSE);
		}
		if (!m_bFigureOpen)
		{
			beginFigure(ptTop.x,ptTop.y,TRUE);
		}
		D2D1_ARC_SEGMENT arc = D2D1::ArcSegment (ptBottom, D2D1::SizeF ((r.right-r.left)/2.f, (r.bottom-r.top)/2.f), 180.f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL);
		m_sink->AddArc (arc);
		arc.point = ptTop;
		m_sink->AddArc (arc);
		m_lastPt = ptTop;
	}

	void SPath_D2D::addCircle(THIS_ float x, float y, float radius, Direction dir /*= kCW_Direction*/)
	{
		addOval2(x-radius,y-radius,x+radius,y+radius,dir);
	}

	void SPath_D2D::addArc(THIS_ const RECT* oval, float startAngle, float sweepAngle)
	{
		addArc2(oval->left,oval->top,oval->right,oval->bottom,startAngle,sweepAngle);
	}


	void SPath_D2D::addArc2(THIS_ float left, float top, float right, float bottom,float _startAngle, float sweepAngle)
	{
		if(!m_bFigureOpen)
			return;

		bool clockwise = sweepAngle>0;
		double startAngle = radians (_startAngle);
		double endAngle = radians (_startAngle+sweepAngle);
		D2D1_RECT_F rc={left,top,right,bottom};

		D2D1_POINT_2F center = {(right-left)/2.0f,(bottom-top)/2.0f};
		if (center.x != center.y)
		{
			startAngle = atan2 (sin (startAngle) * center.x, cos (startAngle) * center.y);
			endAngle = atan2 (sin (endAngle) * center.x, cos (endAngle) * center.y);
		}
		D2D1_POINT_2F start;
		start.x = rc.left + center.x + center.x * cos (startAngle);
		start.y = rc.top + center.y + center.y * sin (startAngle);

		if(start.x != m_lastPt.x || start.y!=m_lastPt.y)
		{
			lineTo(start.x,start.y);
		}

		double sweepangle = endAngle - startAngle;
		if (clockwise) {
			// sweepangle positive
			while (sweepangle < 0.0)
				sweepangle += 2 * M_PI;
			while (sweepangle > 2 * M_PI)
				sweepangle -= 2 * M_PI;
		} else {
			// sweepangle negative
			while (sweepangle > 0.0)
				sweepangle -= 2 * M_PI;
			while (sweepangle < -2 * M_PI)
				sweepangle += 2 * M_PI;
		}

		D2D1_POINT_2F endPoint;
		endPoint.x = rc.left + center.x + center.x * cos (endAngle);
		endPoint.y = rc.top + center.y + center.y * sin (endAngle);

		D2D1_ARC_SEGMENT arc;
		arc.size.width = (rc.right-rc.left)/2;
		arc.size.height = (rc.bottom-rc.top)/2;
		arc.rotationAngle = 0;
		arc.sweepDirection = clockwise ? D2D1_SWEEP_DIRECTION_CLOCKWISE : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
		arc.point = endPoint;
		arc.arcSize = fabs(sweepangle) <= M_PI ? D2D1_ARC_SIZE_SMALL : D2D1_ARC_SIZE_LARGE;
		m_sink->AddArc (arc);
		m_lastPt = endPoint;
	}

	void SPath_D2D::addRoundRect(THIS_ const RECT* rect, float rx, float ry, Direction dir/* = kCW_Direction*/)
	{
		addRoundRect2(rect->left,rect->top,rect->right,rect->bottom,rx,ry,dir);
	}

	void SPath_D2D::addRoundRect2(THIS_ float left, float top,float right,float bottom, float rx,float ry, Direction dir/* = kCW_Direction*/)
	{
		if(!m_sink)
			return;

		if(rx==0 && ry==0)
			return addRect2(left,top,right,bottom,dir);

		if (m_bFigureOpen)
		{
			endFigure(FALSE);
		}
		beginFigure(right-rx,top,TRUE);
		addArc2 (right - 2.0 * rx, top, right, top + 2.0 * ry, 270., 90.);
		addArc2 (right - 2.0 * rx, bottom - 2.0 *ry, right, bottom, 0., 90.);
		addArc2 (left, bottom - 2.0 * ry, left + 2.0 * rx, bottom, 90., 90.);
		addArc2 (left, top, left + 2.0 * rx, top + 2.0 * ry, 180., 90.);
		lineTo(right-rx,top);
	}

	void SPath_D2D::addPoly(THIS_ const POINT pts[], int count, BOOL bClose)
	{
		moveTo(pts[0].x,pts[0].y);
		for(int i=1;i<count;i++)
		{
			D2D1_POINT_2F pt = toPointF(pts[i]);
			lineTo(pts[i].x,pts[i].y);
		}
		if(bClose){
			lineTo(pts[0].x,pts[0].y);
		}
	}

	void SPath_D2D::offset(THIS_ float dx, float dy)
	{
		if(m_sink)
			return;
		ID2D1Factory *fac = toD2DFac(m_pRenderFactory);
		SComPtr<ID2D1TransformedGeometry>  tg;
		if (!SUCCEEDED (fac->CreateTransformedGeometry (m_geometry, Matrix3x2F::Translation(dx,dy), &tg)))
			return;
		m_geometry = tg;
	}

	void SPath_D2D::transform(THIS_ const IxForm * form)
	{
		if(m_sink)
			return;

		ID2D1Factory *fac = toD2DFac(m_pRenderFactory);
		const float *matrix = form->fMat;
		Matrix3x2F xForm (matrix[kMScaleX],matrix[kMSkewY],
			matrix[kMSkewX],matrix[kMScaleY],
			matrix[kMTransX],matrix[kMTransY]);

		SComPtr<ID2D1TransformedGeometry>  tg;
		if (!SUCCEEDED (fac->CreateTransformedGeometry (m_geometry, xForm, &tg)))
			return;
		m_geometry = tg;
	}

	BOOL SPath_D2D::getLastPt(THIS_ fPoint* lastPt) const
	{
		lastPt->fX = m_lastPt.x;
		lastPt->fY = m_lastPt.y;
		return TRUE;
	}

	void SPath_D2D::addString(THIS_ LPCTSTR pszText,int nLen, float x,float y, const IFontS *pFont)
	{
		/*
		ID2D1Factory *fac = toD2DFac(GetRenderFactory());
		IDWriteFactory *pWriteFac = toD2DWriteFac(GetRenderFactory());
		SFont_D2D * pFontD2D = (SFont_D2D*)pFont;
		if(nLen<0) nLen = _tcslen(pszText);
		SStringW str=S_CT2W(SStringT(pszText,nLen));
		SComPtr<IDWriteTextLayout> textLayout; // 文字布局的指针
		pWriteFac->CreateTextLayout(str.c_str(),str.GetLength(),pFontD2D->GetFont(),-1,-1,&textLayout);
		if(textLayout){
		SComPtr<ID2D1GeometrySink> sink;
		m_path->Open(&sink);
		if(sink){
		textLayout->Draw()
		sink->Close();
		}					
		}
		*/
	}


	IPathS * SPath_D2D::clone(CTHIS) const
	{
		return NULL;
	}

	BOOL SPath_D2D::beginFigure(THIS_ float x,float y,BOOL bFill)
	{
		if(!m_sink)
			return FALSE;
		if(m_bFigureOpen)
			return FALSE;
		m_lastPt.x = x;
		m_lastPt.y = y;
		m_sink->BeginFigure(m_lastPt,bFill?D2D1_FIGURE_BEGIN_FILLED:D2D1_FIGURE_BEGIN_HOLLOW);
		m_bFigureOpen=TRUE;
		return TRUE;
	}

	BOOL SPath_D2D::endFigure(THIS_ BOOL bClose)
	{
		if(!m_bFigureOpen)
			return FALSE;
		m_sink->EndFigure(bClose?D2D1_FIGURE_END_CLOSED:D2D1_FIGURE_END_OPEN);
		m_bFigureOpen = FALSE;
		return TRUE;
	}

	float SPath_D2D::getLength(CTHIS) const
	{
		float pathLength=0;
		HRESULT hr = m_path->ComputeLength(NULL, &pathLength);
		return pathLength;
	}

	BOOL SPath_D2D::getPosTan(CTHIS_ float distance, fPoint *pos, fPoint *vec) const
	{
		D2D1_POINT_DESCRIPTION pointDescription;
		SComQIPtr<ID2D1PathGeometry1> path2(m_path);
		if(!path2)
			return FALSE;
		HRESULT hr = path2->ComputePointAndSegmentAtLength(distance,0,NULL,&pointDescription);
		if(!SUCCEEDED(hr))
			return FALSE;
		pos->fX = pointDescription.point.x;
		pos->fY = pointDescription.point.y;

		vec->fX = pointDescription.unitTangentVector.x;
		vec->fY = pointDescription.unitTangentVector.y;
		return TRUE;
	}

	BOOL SPath_D2D::hitTest(CTHIS_ float x,float y) const
	{
		BOOL result = FALSE;
		D2D1_POINT_2F pt={x,y};
		m_path->FillContainsPoint (pt, NULL, &result);
		return result;
	}

	BOOL SPath_D2D::hitTestStroke(CTHIS_ float x,float y,float strokeSize) const
	{
		BOOL result = FALSE;
		D2D1_POINT_2F pt={x,y};
		m_path->StrokeContainsPoint (pt,strokeSize, NULL,NULL,D2D1_DEFAULT_FLATTENING_TOLERANCE, &result);
		return result;

	}

}

EXTERN_C BOOL Render_D2D_SCreateInstance(IObjRef ** ppRenderFactory)
{
	return SOUI::RENDER_D2D::SCreateInstance(ppRenderFactory);
}



