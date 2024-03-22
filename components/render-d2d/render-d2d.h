#pragma once
#include <interface/SRender-i.h>

#include <helper/SColor.h>
#include <helper/obj-ref-impl.hpp>
#include <sobject/Sobject.hpp>
#include <string/tstring.h>
#include <string/strcpcvt.h>
#include <souicoll.h>
#include <atl.mini/SComCli.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

SNSBEGIN

//////////////////////////////////////////////////////////////////////////
// SRenderFactory_D2D
class SRenderFactory_D2D : public TObjRefImpl<IRenderFactory>
{
public:
	SRenderFactory_D2D()
	{
		Init();
	}

	~SRenderFactory_D2D()
	{
	}

	STDMETHOD_(IImgDecoderFactory *,GetImgDecoderFactory)(THIS) OVERRIDE;
	STDMETHOD_(void,SetImgDecoderFactory)(THIS_ IImgDecoderFactory *pImgDecoderFac) OVERRIDE;
	STDMETHOD_(BOOL,CreateRenderTarget)(THIS_ IRenderTarget ** ppRenderTarget,int nWid,int nHei) OVERRIDE;

	STDMETHOD_(BOOL,CreateRenderTarget2)(THIS_ IRenderTarget ** ppRenderTarget,HWND hWnd) OVERRIDE;

	STDMETHOD_(BOOL,CreateFont)(THIS_ IFontS ** ppFont, const LOGFONT *lf) OVERRIDE;

	STDMETHOD_(BOOL,CreateBitmap)(THIS_ IBitmapS ** ppBitmap) OVERRIDE;

	STDMETHOD_(BOOL,CreateRegion)(THIS_ IRegionS **ppRgn) OVERRIDE;

	STDMETHOD_(BOOL,CreatePath)(THIS_ IPathS ** ppPath) OVERRIDE;

	STDMETHOD_(HRESULT,CreateBlurMaskFilter)(THIS_ float radius, BlurStyle style,BlurFlags flag,IMaskFilter ** ppMaskFilter) OVERRIDE {return E_NOTIMPL;}

	STDMETHOD_(HRESULT,CreateEmbossMaskFilter)(THIS_ float direction[3], float ambient, float specular, float blurRadius,IMaskFilter ** ppMaskFilter) OVERRIDE {return E_NOTIMPL;}

	STDMETHOD_(BOOL,CreatePathEffect)(THIS_ REFGUID guidEffect,IPathEffect ** ppPathEffect) OVERRIDE;

public:
	ID2D1Factory * GetD2D1Factory(){return m_pD2DFactory;}
	IDWriteFactory * GetDWriteFactory(){return m_pDWriteFactory;}
	IWICImagingFactory *GetWicImgFactory(){return m_pWICImageFactory;}
protected:
	BOOL Init();

	SAutoRefPtr<IImgDecoderFactory> m_imgDecoderFactory;

	SComPtr<ID2D1Factory> m_pD2DFactory;
	SComPtr<IDWriteFactory> m_pDWriteFactory;
	SComPtr<IWICImagingFactory> m_pWICImageFactory;
};


//////////////////////////////////////////////////////////////////////////
// TD2DRenderObjImpl
template<class T, OBJTYPE ot>
class TD2DRenderObjImpl : public TObjRefImpl<T>
{
public:
	TD2DRenderObjImpl(IRenderFactory * pRenderFac):m_pRenderFactory(pRenderFac)
	{

	}

	virtual ~TD2DRenderObjImpl(){}

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
// SPen_D2D
class SPen_D2D : public TD2DRenderObjImpl<IPenS,OT_PEN>
{
public:
	SPen_D2D(IRenderFactory * pRenderFac,int iStyle=PS_SOLID,COLORREF cr=0,int cWidth=1);

	~SPen_D2D();

	STDMETHOD_(int,GetWidth)(THIS) SCONST OVERRIDE;

	STDMETHOD_(void,SetWidth)(THIS_ int nWid) OVERRIDE;

	STDMETHOD_(int,GetStyle)(THIS) SCONST OVERRIDE;

	STDMETHOD_(void,SetStyle)(THIS_ int nStyle) OVERRIDE;

	STDMETHOD_(COLORREF,GetColor)(THIS) SCONST OVERRIDE;

	STDMETHOD_(void,SetColor)(THIS_ COLORREF cr) OVERRIDE;

	SComPtr<ID2D1StrokeStyle> GetStrokeStyle(){return m_pen;}
	SComPtr<ID2D1Brush> GetColorBrush(ID2D1RenderTarget *pRT);
protected:
	void UpdatePen();

	int		m_style;
	int		m_nWidth;
	COLORREF	m_cr;

	SComPtr<ID2D1StrokeStyle> m_pen;
};

//////////////////////////////////////////////////////////////////////////
// SFont_D2D
class SFont_D2D: public TD2DRenderObjImpl<IFontS,OT_FONT>
{
public:
	SFont_D2D(IRenderFactory * pRenderFac,const LOGFONT * plf);
	~SFont_D2D();

		/**
	* LogFont
	* @brief    获得字体的LOGFONT
	* @return   const LOGFONT * -- 包含字体信息的LOGFONT*
	* Describe  
	*/    
	STDMETHOD_(const LOGFONT *,LogFont)(THIS) SCONST OVERRIDE;

	/**
	* FamilyName
	* @brief    获取字体名
	* @return   LPCTSTR -- 字体名
	* Describe  
	*/    
	STDMETHOD_(LPCTSTR,FamilyName)(THIS) SCONST OVERRIDE;

	/**
	* TextSize
	* @brief    获取字体大小
	* @return   int -- 字体大小
	* Describe  
	*/    
	STDMETHOD_(int,TextSize)(THIS) SCONST OVERRIDE;

	/**
	* IsBold
	* @brief    查询是否为粗体
	* @return   BOOL -- true为粗体，false正常
	* Describe  
	*/    
	STDMETHOD_(BOOL,IsBold)(THIS) SCONST OVERRIDE;

	/**
	* IsUnderline
	* @brief    查询字体下划线状态
	* @return   BOOL -- true有下划线，false正常
	* Describe  
	*/    
	STDMETHOD_(BOOL,IsUnderline)(THIS) SCONST OVERRIDE;

	/**
	* IsItalic
	* @brief    查询字体的斜体状态
	* @return   BOOL -- true为斜体，false正常
	* Describe  
	*/    
	STDMETHOD_(BOOL,IsItalic)(THIS) SCONST OVERRIDE;

	/**
	* StrikeOut
	* @brief    查询字体的删除线状态
	* @return   BOOL -- true有删除线
	* Describe  
	*/    
	STDMETHOD_(BOOL,IsStrikeOut)(THIS) SCONST OVERRIDE;

	STDMETHOD_(BOOL,UpdateFont)(THIS_ const LOGFONT *pLogFont) OVERRIDE;

	STDMETHOD_(void,SetProp)(THIS_ IXmlNode *pXmlNode) OVERRIDE;

	IDWriteTextFormat* GetFont(){return m_hFont;}
protected:
	BOOL CreateD2DFont();

	LOGFONT     m_lf;
	SComPtr<IDWriteTextFormat> m_hFont;
};

class SBrush_D2D : public TD2DRenderObjImpl<IBrushS,OT_BRUSH>
{
public:
	static SBrush_D2D * CreateSolidBrush(IRenderFactory * pRenderFac,COLORREF cr){
		return new SBrush_D2D(pRenderFac,cr);
	}

	static SBrush_D2D * CreateBitmapBrush(IRenderFactory * pRenderFac,IWICBitmap* hBmp)
	{
		return new SBrush_D2D(pRenderFac,hBmp);
	}

	static SBrush_D2D * CreateGradientBrush(IRenderFactory * pRenderFac,const GradientItem *pGradients, int nCount, const GradientInfo *info, BYTE byAlpha, TileMode tileMode){
		return new SBrush_D2D(pRenderFac,pGradients,nCount,info,byAlpha,tileMode);
	}
public:
	STDMETHOD_(BrushType,GetBrushType)(CTHIS) SCONST {
		return m_brushType;
	}


	BOOL IsBitmap(){return m_bmp!=NULL;}

	COLORREF GetColor() const{return m_cr;}
	IWICBitmap* GetBitmap() const{return m_bmp;}
	SComPtr<ID2D1Brush> toBrush(ID2D1RenderTarget *pRT,LPCRECT pRect) const;

protected:
	SBrush_D2D(IRenderFactory * pRenderFac,COLORREF cr);
	SBrush_D2D(IRenderFactory * pRenderFac,IWICBitmap *hBmp);
	SBrush_D2D(IRenderFactory * pRenderFac,const GradientItem *pGradients, int nCount, const GradientInfo *info, BYTE byAlpha,TileMode tileMode);

	~SBrush_D2D();

	BrushType m_brushType;

	COLORREF	m_cr;
	SComPtr<IWICBitmap> m_bmp;
	//gradient info
	int m_nWid,m_nHei;
	GradientInfo m_gradInfo;
	BYTE	m_byAlpha;
	TileMode m_tileMode;
	SArray<GradientItem> m_arrGradItem;
};

//////////////////////////////////////////////////////////////////////////
// SBitmap_D2D
class SBitmap_D2D : public TD2DRenderObjImpl<IBitmapS,OT_BITMAP>
{
public:
	SBitmap_D2D(IRenderFactory *pRenderFac);
	virtual ~SBitmap_D2D()
	{
	}

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

		virtual void SetMaskFilter(IMaskFilter *pMaskFilter){}
		virtual IMaskFilter* GetMaskFilter(){return NULL;}
public:
	SComPtr<ID2D1Bitmap> toD2D1Bitmap(ID2D1RenderTarget *pRT) const;
	IWICBitmap* GetBitmap() {return m_bmp2;}
	HBITMAP     GetGdiBitmap();
protected:

	HRESULT ImgFromDecoder(IImgX *imgDecoder);
	SIZE        m_sz;
	SComPtr<IWICBitmap> m_bmp2;
	IWICBitmapLock* m_lock;
};

//////////////////////////////////////////////////////////////////////////
//	SRegion_D2D
class SRegion_D2D: public TD2DRenderObjImpl<IRegionS,OT_RGN>
{
	friend class SRenderTarget_D2D;
public:
	SRegion_D2D(IRenderFactory *pRenderFac,SComPtr<ID2D1Geometry> rgn=NULL);
	~SRegion_D2D();

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

	SComPtr<ID2D1Geometry> GetRegison2() {return m_hRgn;}

	void CombineGeometry(ID2D1Geometry * geometry, int nCombineMode);
	BOOL IsRect() const{return m_bRect;}
protected:
	ID2D1Geometry * GetRegion() const;
	BOOL IsRectGeometry(ID2D1Geometry* geo) const;
	SComPtr<ID2D1Geometry>    m_hRgn;
	BOOL		m_bRect;
};

class SPath_D2D : public TD2DRenderObjImpl<IPathS,OT_PATH>{
public:
	SPath_D2D(IRenderFactory *pRenderFac);

	ID2D1Geometry * GetPath() {return m_geometry;}

public:

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

	STDMETHOD_(void,addOval2)(THIS_ float left, float top, float right, float bottom, Direction dir/*= kCW_Direction*/) OVERRIDE;

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

	STDMETHOD_(BOOL,beginFigure)(THIS_ float x,float y,BOOL bFill)OVERRIDE;

	STDMETHOD_(BOOL,endFigure)(THIS_ BOOL bClose) OVERRIDE;

	STDMETHOD_(float,getLength)(CTHIS) SCONST OVERRIDE;

	STDMETHOD_(BOOL,getPosTan)(CTHIS_ float distance, fPoint *pos, fPoint *vec) SCONST OVERRIDE;

	STDMETHOD_(void,close)(THIS) OVERRIDE;

	STDMETHOD_(BOOL, hitTest)(CTHIS_ float x,float y) SCONST OVERRIDE;

	STDMETHOD_(BOOL, hitTestStroke)(CTHIS_ float x,float y,float strokeSize) SCONST OVERRIDE;

private:
	SComPtr<ID2D1PathGeometry>    m_path;
	SComPtr<ID2D1Geometry>		  m_geometry;
	SComPtr<ID2D1GeometrySink>	  m_sink;
	BOOL						  m_bFigureOpen;
	FillType					  m_fillType;
	D2D1_POINT_2F				  m_lastPt;
};

struct LayerInfo{
	SAutoRefPtr<IRegionS> rgn;
	bool bLayer;
};
class SClipStack : public TObjRefImpl<IObjRef>{
public:
	SClipStack(IRenderFactory* pRenderFactory,int nWid,int nHei);

	void PushClipRect(LPCRECT pRect ,UINT mode);

	void PushClipRegion(SAutoRefPtr<IRegionS> rgn ,UINT mode);

	bool PopClip();

	SAutoRefPtr<IRegionS> GetRegion() const;

	int GetLayerCount() const;

	SList<LayerInfo> * GetLayerStack(){return &m_stack;}
private:
	SAutoRefPtr<IRegionS> CombineRegion(IRegionS * rgn ,UINT mode);

	SList<LayerInfo> m_stack;
	SAutoRefPtr<IRenderFactory> m_fac;
};

//////////////////////////////////////////////////////////////////////////
//	SRenderTarget_D2D
//////////////////////////////////////////////////////////////////////////
class SRenderTarget_D2D: public TObjRefImpl<IRenderTarget>
{
public:
	SRenderTarget_D2D(IRenderFactory* pRenderFactory,int nWid,int nHei);
	SRenderTarget_D2D(IRenderFactory* pRenderFactory,HWND hWnd);
	~SRenderTarget_D2D();

	STDMETHOD_(void, BeginDraw)(THIS) OVERRIDE;
	STDMETHOD_(void, EndDraw)(THIS) OVERRIDE;
	STDMETHOD_(BOOL,IsOffscreen)(CTHIS) SCONST;

	STDMETHOD_(HRESULT,CreatePen)(THIS_ int iStyle,COLORREF cr,int cWidth,IPenS ** ppPen) OVERRIDE;
	STDMETHOD_(HRESULT,CreateSolidColorBrush)(THIS_ COLORREF cr,IBrushS ** ppBrush) OVERRIDE;
	STDMETHOD_(HRESULT,CreateBitmapBrush)(THIS_ IBitmapS *pBmp,TileMode xtm,TileMode ytm, IBrushS ** ppBrush ) OVERRIDE;
	STDMETHOD_(HRESULT, CreateGradientBrush)(THIS_ const GradientItem *pGradients, int nCount, const GradientInfo *info, BYTE byAlpha, TileMode tileMode, IBrushS **ppBrush) OVERRIDE;
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
	STDMETHOD_(HRESULT,DrawArc2)(THIS_ LPCRECT pRect,float startAngle,float sweepAngle,int width) OVERRIDE;
	STDMETHOD_(HRESULT,FillArc)(THIS_ LPCRECT pRect,float startAngle,float sweepAngle) OVERRIDE;

	STDMETHOD_(HRESULT,DrawLines)(THIS_ LPPOINT pPt,size_t nCount) OVERRIDE;
	STDMETHOD_(HRESULT, DrawGradientRect)
		(THIS_ LPCRECT pRect, BOOL bVert, POINT ptRoundCorner, const GradientItem *pGradients, int nCount, BYTE byAlpha DEF_VAL(0xFF)) OVERRIDE;
	STDMETHOD_(HRESULT, DrawGradientRectEx)
		(THIS_ LPCRECT pRect, POINT ptRoundCorner, const GradientItem *pGradients, int nCount, const GradientInfo *info, BYTE byAlpha DEF_VAL(0xFF)) OVERRIDE;
	STDMETHOD_(HRESULT, DrawGradientPath)
		(THIS_ const IPathS* path, const GradientItem *pGradients, int nCount, const GradientInfo *info, BYTE byAlpha DEF_VAL(0xFF)) OVERRIDE;

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
	STDMETHOD_(void,SetMaskFilter)(THIS_ IMaskFilter *pMaskFilter) OVERRIDE{}
	STDMETHOD_(IMaskFilter *,GetMaskFilter)(THIS) OVERRIDE{return NULL;}
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

protected:
	HWND					   m_hWnd;
	SComPtr<ID2D1RenderTarget> m_rt;
	SComPtr<ID2D1GdiInteropRenderTarget> m_rtGdi;
	SAutoRefPtr<SClipStack>    m_clipStack;
	int						   m_saveState;
	SIZE					   m_canvsSize;
	SColor            m_curColor;
	SAutoRefPtr<SBitmap_D2D> m_curBmp;
	SAutoRefPtr<SPen_D2D> m_curPen;
	SAutoRefPtr<SBrush_D2D> m_curBrush;
	SAutoRefPtr<SFont_D2D> m_curFont;
	POINT               m_ptOrg;
	D2D1::Matrix3x2F			m_mtx;
	//注意保存4个默认的GDI对象
	SAutoRefPtr<IPenS> m_defPen;
	SAutoRefPtr<IBrushS> m_defBrush;
	SAutoRefPtr<IFontS> m_defFont;
	SAutoRefPtr<IRenderFactory> m_pRenderFactory;
	BOOL	m_bAntiAlias;
	int  m_cDrawing;
	HBITMAP m_hBmp;
};

namespace RENDER_D2D
{
	SOUI_COM_C BOOL SOUI_COM_API SCreateInstance(IObjRef ** ppRenderFactory);
}

SNSEND

EXTERN_C BOOL Render_D2D_SCreateInstance(IObjRef ** ppRenderFactory);
