﻿#pragma once

#include <interface/SRender-i.h>

#include <helper/SColor.h>
#include <helper/SAttrCracker.h>
#include <helper/obj-ref-impl.hpp>
#include <core/SkCanvas.h>
#include <core/SkBitmap.h>
#include <core/SkTypeface.h>
#include <string/tstring.h>
#include <string/strcpcvt.h>
#include <souicoll.h>
#include <core/SkShader.h>
SNSBEGIN

//////////////////////////////////////////////////////////////////////////
// SRenderFactory_Skia
class SRenderFactory_Skia : public TObjRefImpl<IRenderFactory>
{
public:
	SRenderFactory_Skia();

	~SRenderFactory_Skia();

	STDMETHOD_(IImgDecoderFactory *,GetImgDecoderFactory)(THIS) OVERRIDE;
	STDMETHOD_(void,SetImgDecoderFactory)(THIS_ IImgDecoderFactory *pImgDecoderFac) OVERRIDE;
	STDMETHOD_(BOOL,CreateRenderTarget)(THIS_ IRenderTarget ** ppRenderTarget,int nWid,int nHei) OVERRIDE;

	STDMETHOD_(BOOL,CreateRenderTarget2)(THIS_ IRenderTarget ** ppRenderTarget,HWND hWnd) OVERRIDE;

	STDMETHOD_(BOOL,CreateFont)(THIS_ IFontS ** ppFont, const LOGFONT *lf) OVERRIDE;

	STDMETHOD_(BOOL,CreateBitmap)(THIS_ IBitmapS ** ppBitmap) OVERRIDE;

	STDMETHOD_(BOOL,CreateRegion)(THIS_ IRegionS **ppRgn) OVERRIDE;

	STDMETHOD_(BOOL,CreatePath)(THIS_ IPathS ** ppPath) OVERRIDE;

	STDMETHOD_(BOOL,CreatePathEffect)(THIS_ REFGUID guidEffect,IPathEffect ** ppPathEffect) OVERRIDE;

	STDMETHOD_(HRESULT,CreateBlurMaskFilter)(THIS_ float radius, BlurStyle style,BlurFlags flag,IMaskFilter ** ppMaskFilter) OVERRIDE;

	STDMETHOD_(HRESULT,CreateEmbossMaskFilter)(THIS_ float direction[3], float ambient, float specular, float blurRadius,IMaskFilter ** ppMaskFilter) OVERRIDE;

    STDMETHOD_(IFontS *,GetDefFont)(CTHIS) OVERRIDE{
		return m_defFont;
	}
protected:
	SAutoRefPtr<IImgDecoderFactory> m_imgDecoderFactory;
	SAutoRefPtr<IFontS> m_defFont;
};


//////////////////////////////////////////////////////////////////////////
// TSkiaRenderObjImpl
template<class T, OBJTYPE ot>
class TSkiaRenderObjImpl : public TObjRefImpl<T>
{
public:
	TSkiaRenderObjImpl(IRenderFactory * pRenderFac):m_pRenderFactory(pRenderFac)
	{

	}

	virtual ~TSkiaRenderObjImpl(){}

	STDMETHOD_(IRenderFactory *,GetRenderFactory)(THIS) SCONST OVERRIDE
	{
		return m_pRenderFactory;
	}

	STDMETHOD_(OBJTYPE,ObjectType)(THIS) SCONST OVERRIDE
	{
		return ot;
	}

protected:
	IRenderFactory *m_pRenderFactory;

};


//////////////////////////////////////////////////////////////////////////
// SPen_Skia
class SPen_Skia : public TSkiaRenderObjImpl<IPenS,OT_PEN>
{
public:
	SPen_Skia(IRenderFactory * pRenderFac,int iStyle=PS_SOLID,COLORREF cr=0xFF000000,int cWidth=1);

	STDMETHOD_(int,GetWidth)(THIS) SCONST OVERRIDE;

	STDMETHOD_(void,SetWidth)(THIS_ int nWid) OVERRIDE;

	STDMETHOD_(int,GetStyle)(THIS) SCONST OVERRIDE;

	STDMETHOD_(void,SetStyle)(THIS_ int style) OVERRIDE;

	STDMETHOD_(COLORREF,GetColor)(THIS) SCONST OVERRIDE;

	STDMETHOD_(void,SetColor)(THIS_ COLORREF cr) OVERRIDE;

protected:

	int		m_style;
	int		m_nWidth;
	COLORREF	m_cr;
};

//////////////////////////////////////////////////////////////////////////
// SFont_Skia
class SFont_Skia: public TSkiaRenderObjImpl<IFontS,OT_FONT>
{
	friend class SRenderTarget_Skia;
	friend class SPath_Skia;
public:
	SFont_Skia(IRenderFactory * pRenderFac,const LOGFONT * plf);

	virtual ~SFont_Skia();

	STDMETHOD_(const LOGFONT *,LogFont)(THIS) SCONST OVERRIDE;

	STDMETHOD_(LPCTSTR,FamilyName)(THIS) SCONST OVERRIDE;

	STDMETHOD_(int,TextSize)(THIS) SCONST OVERRIDE;

	STDMETHOD_(BOOL,IsBold)(THIS) SCONST OVERRIDE;

	STDMETHOD_(BOOL,IsUnderline)(THIS) SCONST OVERRIDE;

	STDMETHOD_(BOOL,IsItalic)(THIS) SCONST OVERRIDE;

	STDMETHOD_(BOOL,IsStrikeOut)(THIS) SCONST OVERRIDE;

	STDMETHOD_(BOOL,UpdateFont)(THIS_ const LOGFONT *pLogFont) OVERRIDE;

	STDMETHOD_(void,SetProp)(THIS_ IXmlNode *pXmlNode) OVERRIDE;
private:
	SkTypeface *GetFont() const {return m_skFont;}
protected:
	SkTypeface *m_skFont;   //定义字体
	SkPaint     m_skPaint;  //定义文字绘制属性
	LOGFONT     m_lf;
	SAutoRefPtr<IMaskFilter> m_maskFilter;
	FillStyle	m_fillStyle;
	BOOL		m_bLcdText;
};

class SBrush_Skia : public TSkiaRenderObjImpl<IBrushS,OT_BRUSH>
{
public:
	SBrush_Skia(IRenderFactory * pRenderFac,COLORREF cr);
	SBrush_Skia(IRenderFactory * pRenderFac,SkBitmap bmp,TileMode xtm,TileMode ytm);
	SBrush_Skia(IRenderFactory * pRenderFac,const GradientItem *pGradients, int nCount, const GradientInfo *info, BYTE byAlpha,TileMode tileMode);

	~SBrush_Skia();

public:
	STDMETHOD_(BrushType,GetBrushType)(CTHIS) SCONST OVERRIDE{
		return m_brushType;
	}
	void InitPaint(SkPaint & paint,const SkRect & skrc);
	BOOL IsFullArc() const;
protected:
	BrushType m_brushType;

	SkColor m_cr;		//颜色画刷
	SkBitmap  m_bmp;	//位图画刷
	SkShader::TileMode m_xtm,m_ytm;
	//gradient info
	int m_nWid,m_nHei;
	GradientInfo m_gradInfo;
	BYTE	m_byAlpha;
	SArray<GradientItem> m_arrGradItem;

	SkShader::TileMode  m_tileMode;
};

//////////////////////////////////////////////////////////////////////////
// SBitmap_Skia
class SBitmap_Skia : public TSkiaRenderObjImpl<IBitmapS,OT_BITMAP>
{
public:
	SBitmap_Skia(IRenderFactory *pRenderFac);
	~SBitmap_Skia();

	STDMETHOD_(HRESULT,Init)(THIS_ int nWid,int nHei,const LPVOID pBits) OVERRIDE;

	STDMETHOD_(HRESULT,Init2)(THIS_ IImgFrame *pImgFrame) OVERRIDE;

	STDMETHOD_(HRESULT,LoadFromFile)(THIS_ LPCTSTR pszFileName) OVERRIDE;

	STDMETHOD_(HRESULT,LoadFromMemory)(THIS_ LPBYTE pBuf,size_t szLen) OVERRIDE;

	STDMETHOD_(UINT,Width)(THIS) SCONST OVERRIDE;

	STDMETHOD_(UINT,Height)(THIS) SCONST OVERRIDE;

	STDMETHOD_(SIZE,Size)(THIS) SCONST OVERRIDE;

	STDMETHOD_(LPVOID,LockPixelBits)(THIS) OVERRIDE;

	STDMETHOD_(void,UnlockPixelBits)(THIS_ LPVOID pBuf) OVERRIDE;

	STDMETHOD_(const LPVOID,GetPixelBits)(THIS) SCONST OVERRIDE;

	STDMETHOD_(HRESULT,Clone)(THIS_ IBitmapS **ppClone) SCONST OVERRIDE;

	STDMETHOD_(HRESULT,Scale)(THIS_ IBitmapS **pOutput,int nScale,FilterLevel filterLevel) SCONST OVERRIDE;

	STDMETHOD_(HRESULT,Scale2)(THIS_ IBitmapS **pOutput,int nWid,int nHei,FilterLevel filterLevel) SCONST OVERRIDE;

	STDMETHOD_(HRESULT,Save)(THIS_ LPCWSTR pszFileName,const LPVOID pFormat) SCONST OVERRIDE;
	STDMETHOD_(HRESULT, Save2)(CTHIS_ LPCWSTR pszFileName, ImgFmt imgFmt) SCONST OVERRIDE;

public:
	const SkBitmap & GetSkBitmap() const{return m_bitmap;}
	HBITMAP  GetGdiBitmap(){return m_hBmp;}
	#ifndef _WIN32
	void MarkDirty(){
		MarkPixmapDirty(m_hBmp);
	}
	#endif//_WIN32
protected:
	HBITMAP CreateGDIBitmap(int nWid,int nHei,void ** ppBits);

	HRESULT ImgFromDecoder(IImgX *imgDecoder);

	SkBitmap    m_bitmap;   //skia 管理的BITMAP
	HBITMAP     m_hBmp;     //标准的32位位图，和m_bitmap共享内存
};

//////////////////////////////////////////////////////////////////////////
//	SRegion_Skia
class SRegion_Skia: public TSkiaRenderObjImpl<IRegionS,OT_RGN>
{
	friend class SRenderTarget_Skia;
public:
	SRegion_Skia(IRenderFactory *pRenderFac);
	virtual ~SRegion_Skia();

	STDMETHOD_(void,CombineRect)(THIS_ LPCRECT lprect,int nCombineMode) OVERRIDE;

	STDMETHOD_(void,CombineRgn)(THIS_ const IRegionS * pRgnSrc,int nCombineMode ) OVERRIDE;

	STDMETHOD_(void,CombineRoundRect)(THIS_ LPCRECT lprect, POINT ptConner, int nCombineMode) OVERRIDE;

	STDMETHOD_(void,CombineEllipse)(THIS_ LPCRECT lprect , int nCombineMode) OVERRIDE;

	STDMETHOD_(void,CombinePolygon)(THIS_ const POINT *pts, int count, int nPolygonMode, int nCombineMode) OVERRIDE;

	STDMETHOD_(BOOL,PtInRegion)(THIS_ POINT pt) SCONST OVERRIDE;

	STDMETHOD_(BOOL,RectInRegion)(THIS_ LPCRECT lprect) SCONST OVERRIDE;

	STDMETHOD_(void,GetRgnBox)(THIS_ LPRECT lprect) SCONST OVERRIDE;

	STDMETHOD_(BOOL,IsEmpty)(THIS) SCONST OVERRIDE;

	STDMETHOD_(void,Offset)(THIS_ POINT pt) OVERRIDE;

	STDMETHOD_(void,Clear)(THIS) OVERRIDE;
protected:
	SkRegion GetRegion() const;

	void SetRegion(const SkRegion & rgn);

	static SkRegion::Op RGNMODE2SkRgnOP(UINT mode);


protected:
	SkRegion    m_rgn;
};

class SPath_Skia: public TSkiaRenderObjImpl<IPathS,OT_PATH>
{
	friend class SRenderTarget_Skia;
	friend class SPathMeasure_Skia;
public:
	SPath_Skia(IRenderFactory *pRenderFac);
	virtual ~SPath_Skia();

	STDMETHOD_(FillType,getFillType)(THIS) SCONST OVERRIDE;

	STDMETHOD_(void,setFillType)(THIS_ FillType ft) OVERRIDE;

	STDMETHOD_(void,reset)(THIS) OVERRIDE;

	STDMETHOD_(BOOL,isEmpty)(THIS) SCONST OVERRIDE;

	STDMETHOD_(void,getBounds)(CTHIS_ LPRECT prc) SCONST OVERRIDE;

	STDMETHOD_(void,moveTo)(THIS_ float x, float y) OVERRIDE;

	STDMETHOD_(void,rMoveTo)(THIS_ float dx, float dy) OVERRIDE;

	STDMETHOD_(void,lineTo)(THIS_ float x, float y) OVERRIDE;

	STDMETHOD_(void,rLineTo)(THIS_ float dx, float dy) OVERRIDE;

	STDMETHOD_(void,quadTo)(THIS_ float x1, float y1, float x2, float y2) OVERRIDE;

	STDMETHOD_(void,rQuadTo)(THIS_ float dx1, float dy1, float dx2, float dy2) OVERRIDE;

	STDMETHOD_(void,conicTo)(THIS_ float x1, float y1, float x2, float y2,
		float w) OVERRIDE;

	STDMETHOD_(void,rConicTo)(THIS_ float dx1, float dy1, float dx2, float dy2,
		float w) OVERRIDE;

	STDMETHOD_(void,cubicTo)(THIS_ float x1, float y1, float x2, float y2,
		float x3, float y3) OVERRIDE;
	STDMETHOD_(void,rCubicTo)(THIS_ float x1, float y1, float x2, float y2,
		float x3, float y3) OVERRIDE;

	STDMETHOD_(void,addRect)(THIS_ const RECT* rect, Direction dir /*= kCW_Direction*/) OVERRIDE;

	STDMETHOD_(void,addRect2)(THIS_ float left, float top, float right, float bottom,
		Direction dir/* = kCW_Direction*/) OVERRIDE;

	STDMETHOD_(void,addOval)(THIS_ const RECT * oval, Direction dir/*= kCW_Direction*/) OVERRIDE;

	STDMETHOD_(void,addOval2)(THIS_ float left, float top, float right, float bottom, Direction dir DEF_VAL(kCW_Direction)) OVERRIDE;

	STDMETHOD_(void,addCircle)(THIS_ float x, float y, float radius,
		Direction dir /*= kCW_Direction*/) OVERRIDE;

	STDMETHOD_(void,addArc)(THIS_ const RECT* oval, float startAngle, float sweepAngle) OVERRIDE;

	STDMETHOD_(void,addArc2)(THIS_ float left, float top, float right, float bottom,float startAngle, float sweepAngle) OVERRIDE;

	STDMETHOD_(void,addRoundRect)(THIS_ const RECT* rect, float rx, float ry,
		Direction dir/* = kCW_Direction*/) OVERRIDE;

	STDMETHOD_(void,addRoundRect2)(THIS_ float left, float top,float right,float bottom, float rx,float ry,
		Direction dir/* = kCW_Direction*/) OVERRIDE;

	STDMETHOD_(void,addPoly)(THIS_ const POINT pts[], int count, BOOL close) OVERRIDE;

	STDMETHOD_(void,offset)(THIS_ float dx, float dy) OVERRIDE;

	STDMETHOD_(void,transform)(THIS_ const IxForm * matrix) OVERRIDE;

	STDMETHOD_(BOOL,getLastPt)(THIS_ fPoint* lastPt) SCONST OVERRIDE;

	STDMETHOD_(void,addString)(THIS_ LPCTSTR pszText,int nLen, float x,float y, const IFontS *pFont) OVERRIDE;

	STDMETHOD_(IPathS *, clone)(CTHIS) SCONST OVERRIDE;

	STDMETHOD_(BOOL,beginFigure)(THIS_ float x,float y,BOOL bFill)OVERRIDE {
		moveTo(x,y);
		return TRUE;
	}

	STDMETHOD_(BOOL,endFigure)(THIS_ BOOL bClose) OVERRIDE{
		if(bClose){
			m_skPath.close();
		}
		return TRUE;
	}
	STDMETHOD_(float,getLength)(CTHIS) SCONST OVERRIDE;

	STDMETHOD_(BOOL,getPosTan)(CTHIS_ float distance, fPoint *pos, fPoint *vec) SCONST OVERRIDE;

	STDMETHOD_(void,close)(THIS) OVERRIDE;

	STDMETHOD_(BOOL, hitTest)(CTHIS_ float x,float y) SCONST OVERRIDE;

	STDMETHOD_(BOOL, hitTestStroke)(CTHIS_ float x,float y,float strokeSize) SCONST OVERRIDE;

protected:
	SkPath      m_skPath;
};

class SMaskFilter_Skia : public TObjRefImpl<IMaskFilter>
{
	friend class SRenderTarget_Skia;
public:
	SMaskFilter_Skia(SkMaskFilter *maskFilter);

	~SMaskFilter_Skia();

	STDMETHOD_(void*,GetPtr)(THIS) OVERRIDE;
protected:
	SkMaskFilter * m_maskFilter;
};

//////////////////////////////////////////////////////////////////////////
//	SRenderTarget_Skia
//////////////////////////////////////////////////////////////////////////
class SRenderTarget_Skia: public TObjRefImpl<IRenderTarget>
{
public:
	SRenderTarget_Skia(IRenderFactory* pRenderFactory,int nWid,int nHei);
	~SRenderTarget_Skia();
	STDMETHOD_(void, BeginDraw)(THIS) OVERRIDE{}
	STDMETHOD_(void, EndDraw)(THIS) OVERRIDE;
	STDMETHOD_(BOOL,IsOffscreen)(CTHIS) SCONST {return TRUE;}

	STDMETHOD_(HRESULT,CreatePen)(THIS_ int iStyle,COLORREF cr,int cWidth,IPenS ** ppPen) OVERRIDE;
	STDMETHOD_(HRESULT,CreateSolidColorBrush)(THIS_ COLORREF cr,IBrushS ** ppBrush) OVERRIDE;
	STDMETHOD_(HRESULT,CreateBitmapBrush)(THIS_ IBitmapS *pBmp,TileMode xtm,TileMode ytm, IBrushS ** ppBrush ) OVERRIDE;
	STDMETHOD_(HRESULT,CreateGradientBrush)(THIS_ const GradientItem *pGradients, int nCount, const GradientInfo *info, BYTE byAlpha,TileMode tileMode, IBrushS * *ppBrush) OVERRIDE;
	STDMETHOD_(HRESULT,CreateRegion)(THIS_ IRegionS ** ppRegion ) OVERRIDE;

	STDMETHOD_(HRESULT,Resize)(THIS_ SIZE sz) OVERRIDE;

	STDMETHOD_(HRESULT,OffsetViewportOrg)(THIS_ int xOff, int yOff, LPPOINT lpPoint/*=NULL*/) OVERRIDE;
	STDMETHOD_(HRESULT,GetViewportOrg)(THIS_ LPPOINT lpPoint) OVERRIDE;
	STDMETHOD_(HRESULT,SetViewportOrg)(THIS_ POINT pt) OVERRIDE;

	STDMETHOD_(HRESULT,PushClipRect)(THIS_ LPCRECT pRect,UINT mode/*=RGN_AND*/) OVERRIDE;
	STDMETHOD_(HRESULT,PushClipRegion)(THIS_ IRegionS *pRegion,UINT mode/*=RGN_AND*/) OVERRIDE;
	STDMETHOD_(HRESULT,PopClip)(THIS) OVERRIDE;

	STDMETHOD_(HRESULT,ExcludeClipRect)(THIS_ LPCRECT pRc) OVERRIDE;

	STDMETHOD_(HRESULT,SaveClip)(THIS_ int *pnState) OVERRIDE;
	STDMETHOD_(HRESULT,RestoreClip)(THIS_ int nState/*=-1*/) OVERRIDE;

	STDMETHOD_(HRESULT,GetClipRegion)(THIS_ IRegionS **ppRegion) OVERRIDE;
	STDMETHOD_(HRESULT,GetClipBox)(THIS_ LPRECT prc) OVERRIDE;

	STDMETHOD_(HRESULT,DrawText)(THIS_ LPCTSTR pszText,int cchLen,LPRECT pRc,UINT uFormat) OVERRIDE;
	STDMETHOD_(HRESULT,MeasureText)(THIS_ LPCTSTR pszText,int cchLen, SIZE *psz) OVERRIDE;
	STDMETHOD_(HRESULT,TextOut)(THIS_ int x,int y, LPCTSTR lpszString,int nCount) OVERRIDE;

	STDMETHOD_(HRESULT,DrawRectangle)(THIS_ LPCRECT pRect) OVERRIDE;
	STDMETHOD_(HRESULT,FillRectangle)(THIS_ LPCRECT pRect) OVERRIDE;
	STDMETHOD_(HRESULT,FillSolidRect)(THIS_ LPCRECT pRect,COLORREF cr) OVERRIDE;
	STDMETHOD_(HRESULT,DrawRoundRect)(THIS_ LPCRECT pRect,POINT pt) OVERRIDE;
	STDMETHOD_(HRESULT,FillRoundRect)(THIS_ LPCRECT pRect,POINT pt) OVERRIDE;
	STDMETHOD_(HRESULT,FillSolidRoundRect)(THIS_ LPCRECT pRect,POINT pt,COLORREF cr) OVERRIDE;
	STDMETHOD_(HRESULT,ClearRect)(THIS_ LPCRECT pRect,COLORREF cr) OVERRIDE;
	STDMETHOD_(HRESULT,InvertRect)(THIS_ LPCRECT pRect) OVERRIDE;
	STDMETHOD_(HRESULT,DrawEllipse)(THIS_ LPCRECT pRect) OVERRIDE;
	STDMETHOD_(HRESULT,FillEllipse)(THIS_ LPCRECT pRect) OVERRIDE;
	STDMETHOD_(HRESULT,FillSolidEllipse)(THIS_ LPCRECT pRect,COLORREF cr) OVERRIDE;

	STDMETHOD_(HRESULT,DrawArc)(THIS_ LPCRECT pRect,float startAngle,float sweepAngle,BOOL useCenter) OVERRIDE;
	STDMETHOD_(HRESULT,DrawArc2)(THIS_ LPCRECT pRect, float startAngle, float sweepAngle, int width) OVERRIDE;
	STDMETHOD_(HRESULT,FillArc)(THIS_ LPCRECT pRect,float startAngle,float sweepAngle) OVERRIDE;

	STDMETHOD_(HRESULT,DrawLines)(THIS_ LPPOINT pPt,size_t nCount) OVERRIDE;
    STDMETHOD_(HRESULT, DrawLine)(THIS_ POINT pt1, POINT pt2) OVERRIDE{
		POINT pts[2] = {pt1,pt2};
		return DrawLines(pts,2);
	}
	STDMETHOD_(HRESULT,DrawGradientRect)(THIS_ LPCRECT pRect,  BOOL bVert, POINT ptRoundCorner, const GradientItem *pGradients, int nCount, BYTE byAlpha DEF_VAL(0xFF)) OVERRIDE;
	STDMETHOD_(HRESULT, DrawGradientRectEx)
		(THIS_ LPCRECT pRect, POINT ptRoundCorner, const GradientItem *pGradients, int nCount, const GradientInfo *info, BYTE byAlpha DEF_VAL(0xFF)) OVERRIDE;
	STDMETHOD_(HRESULT, DrawGradientPath)
		(THIS_ const IPathS* path, const GradientItem *pGradients, int nCount, const GradientInfo *info,BYTE byAlpha DEF_VAL(0xFF)) OVERRIDE;
	STDMETHOD_(HRESULT,DrawIconEx)(THIS_ int xLeft, int yTop, HICON hIcon, int cxWidth,int cyWidth,UINT diFlags) OVERRIDE;
	STDMETHOD_(HRESULT,DrawBitmap)(THIS_ LPCRECT pRcDest,const IBitmapS *pBitmap,int xSrc,int ySrc,BYTE byAlpha/*=0xFF*/) OVERRIDE;
	STDMETHOD_(HRESULT,DrawBitmapEx)(THIS_ LPCRECT pRcDest,const IBitmapS *pBitmap,LPCRECT pRcSrc,UINT expendMode, BYTE byAlpha/*=0xFF*/) OVERRIDE;
	STDMETHOD_(HRESULT,DrawBitmap9Patch)(THIS_ LPCRECT pRcDest,const IBitmapS *pBitmap,LPCRECT pRcSrc,LPCRECT pRcSourMargin,UINT expendMode,BYTE byAlpha/*=0xFF*/) OVERRIDE;
	STDMETHOD_(HRESULT,BitBlt)(THIS_ LPCRECT pRcDest,IRenderTarget *pRTSour,int xSrc,int ySrc,DWORD dwRop/*=kSrcCopy*/) OVERRIDE;
	STDMETHOD_(HRESULT,AlphaBlend)(THIS_ LPCRECT pRcDest,IRenderTarget *pRTSrc,LPCRECT pRcSrc,BYTE byAlpha) OVERRIDE;
	STDMETHOD_(IRenderObj *,GetCurrentObject)(THIS_ OBJTYPE uType) OVERRIDE;
	STDMETHOD_(HRESULT,SelectDefaultObject)(THIS_ OBJTYPE objType, IRenderObj ** pOldObj/* = NULL*/) OVERRIDE;
	STDMETHOD_(HRESULT,SelectObject)(THIS_ IRenderObj *pObj,IRenderObj ** pOldObj/* = NULL*/) OVERRIDE;
	STDMETHOD_(COLORREF,GetTextColor)(THIS) OVERRIDE;
	STDMETHOD_(COLORREF,SetTextColor)(THIS_ COLORREF color) OVERRIDE;
	STDMETHOD_(void,SetMaskFilter)(THIS_ IMaskFilter *pMaskFilter) OVERRIDE;
	STDMETHOD_(IMaskFilter *,GetMaskFilter)(THIS) OVERRIDE;
	STDMETHOD_(HDC,GetDC)(THIS_ UINT uFlag) OVERRIDE;
	STDMETHOD_(void,ReleaseDC)(THIS_ HDC hdc,LPCRECT pRc) OVERRIDE;
	STDMETHOD_(HRESULT,SetTransform)(THIS_ const float matrix[9], float oldMatrix[9]/*=NULL*/) OVERRIDE;
	STDMETHOD_(HRESULT,GetTransform)(THIS_ float matrix[9]) SCONST OVERRIDE;
	STDMETHOD_(COLORREF,GetPixel)(THIS_ int x, int y) OVERRIDE;
	STDMETHOD_(COLORREF,SetPixel)(THIS_ int x, int y, COLORREF cr) OVERRIDE;
	STDMETHOD_(HRESULT,PushClipPath)(THIS_ const IPathS * path, UINT mode, BOOL doAntiAlias = false) OVERRIDE;
	STDMETHOD_(HRESULT,DrawPath)(THIS_ const IPathS * path,IPathEffect * pathEffect=NULL) OVERRIDE;
	STDMETHOD_(HRESULT,FillPath)(THIS_ const IPathS * path) OVERRIDE;
	STDMETHOD_(HRESULT,SetXfermode)(THIS_ int mode,int *pOldMode=NULL) OVERRIDE;
	STDMETHOD_(BOOL,SetAntiAlias)(THIS_ BOOL bAntiAlias) OVERRIDE;
	STDMETHOD_(BOOL,GetAntiAlias)(THIS) SCONST OVERRIDE;
public:
	SkCanvas *GetCanvas(){return m_SkCanvas;}

protected:
	bool SetPaintXferMode(SkPaint & paint,int nRopMode);
protected:
	SkCanvas *m_SkCanvas;
	int		  m_lastSave;
	SColor            m_curColor;
	SAutoRefPtr<SBitmap_Skia> m_curBmp;
	SAutoRefPtr<SPen_Skia> m_curPen;
	SAutoRefPtr<SBrush_Skia> m_curBrush;
	SAutoRefPtr<SFont_Skia> m_curFont;

	SkPoint         m_ptOrg;
	SkPaint			m_paint;

	//注意保存4个默认的RenderObject对象
	SAutoRefPtr<IBitmapS> m_defBmp;
	SAutoRefPtr<IPenS> m_defPen;
	SAutoRefPtr<IBrushS> m_defBrush;
	SAutoRefPtr<IFontS> m_defFont;
	SAutoRefPtr<IMaskFilter> m_curMaskFilter;

	SAutoRefPtr<IRenderFactory> m_pRenderFactory;

	HDC m_hGetDC;
	int m_nGetDC;

	bool			m_bAntiAlias;
	SList<int>		m_lstLayerId;	//list to save layer ids
	int				m_xferMode;
};

namespace RENDER_SKIA
{
	SOUI_COM_C BOOL SOUI_COM_API SCreateInstance(IObjRef ** ppRenderFactory);
}
SNSEND

EXTERN_C BOOL SOUI_COM_API Render_Skia_SCreateInstance(IObjRef ** ppRenderFactory);
EXTERN_C void SOUI_COM_API Render_Skia_SetFontFallback(FontFallback fontFallback);
