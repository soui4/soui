#pragma once
///////////////////////////////////////////////////////////////////////////////
// CDC - The device context class

template <bool t_bManaged>
class CDCT
{
public:
	// Data members
	HDC m_hDC;

	// Constructor/destructor/operators
	CDCT(HDC hDC = NULL) : m_hDC(hDC)
	{
	}

	~CDCT()
	{
		if (t_bManaged && (m_hDC != NULL))
			::DeleteDC(Detach());
	}

	CDCT<t_bManaged>& operator =(HDC hDC)
	{
		Attach(hDC);
		return *this;
	}

	void Attach(HDC hDC)
	{
		if (t_bManaged && (m_hDC != NULL) && (m_hDC != hDC))
			::DeleteDC(m_hDC);
		m_hDC = hDC;
	}

	HDC Detach()
	{
		HDC hDC = m_hDC;
		m_hDC = NULL;
		return hDC;
	}

	operator HDC() const { return m_hDC; }

	bool IsNull() const { return (m_hDC == NULL); }

	// Operations
	HWND WindowFromDC() const
	{
		SASSERT(m_hDC != NULL);
		return ::WindowFromDC(m_hDC);
	}

	HPEN GetCurrentPen() const
	{
		SASSERT(m_hDC != NULL);
		return (HPEN)::GetCurrentObject(m_hDC, OBJ_PEN);
	}

	HBRUSH GetCurrentBrush() const
	{
		SASSERT(m_hDC != NULL);
		return (HBRUSH)::GetCurrentObject(m_hDC, OBJ_BRUSH);
	}

	HPALETTE GetCurrentPalette() const
	{
		SASSERT(m_hDC != NULL);
		return ((HPALETTE)::GetCurrentObject(m_hDC, OBJ_PAL));
	}

	HFONT GetCurrentFont() const
	{
		SASSERT(m_hDC != NULL);
		return ((HFONT)::GetCurrentObject(m_hDC, OBJ_FONT));
	}

	HBITMAP GetCurrentBitmap() const
	{
		SASSERT(m_hDC != NULL);
		return ((HBITMAP)::GetCurrentObject(m_hDC, OBJ_BITMAP));
	}

	HDC CreateDC(LPCTSTR lpszDriverName, LPCTSTR lpszDeviceName, LPCTSTR lpszOutput, const DEVMODE* lpInitData)
	{
		SASSERT(m_hDC == NULL);
		m_hDC = ::CreateDC(lpszDriverName, lpszDeviceName, lpszOutput, lpInitData);
		return m_hDC;
	}

	HDC CreateCompatibleDC(HDC hDC = NULL)
	{
		SASSERT(m_hDC == NULL);
		m_hDC = ::CreateCompatibleDC(hDC);
		return m_hDC;
	}

	BOOL DeleteDC()
	{
		if (m_hDC == NULL)
			return FALSE;
		BOOL bRet = ::DeleteDC(m_hDC);
		if (bRet)
			m_hDC = NULL;
		return bRet;
	}

	// Device-Context Functions
	int SaveDC()
	{
		SASSERT(m_hDC != NULL);
		return ::SaveDC(m_hDC);
	}

	BOOL RestoreDC(int nSavedDC)
	{
		SASSERT(m_hDC != NULL);
		return ::RestoreDC(m_hDC, nSavedDC);
	}

	int GetDeviceCaps(int nIndex) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetDeviceCaps(m_hDC, nIndex);
	}

	UINT SetBoundsRect(LPCRECT lpRectBounds, UINT flags)
	{
		SASSERT(m_hDC != NULL);
		return ::SetBoundsRect(m_hDC, lpRectBounds, flags);
	}

	UINT GetBoundsRect(LPRECT lpRectBounds, UINT flags) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetBoundsRect(m_hDC, lpRectBounds, flags);
	}

	BOOL ResetDC(const DEVMODE* lpDevMode)
	{
		SASSERT(m_hDC != NULL);
		return ::ResetDC(m_hDC, lpDevMode) != NULL;
	}

	// Drawing-Tool Functions
	BOOL GetBrushOrg(LPPOINT lpPoint) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetBrushOrgEx(m_hDC, lpPoint);
	}

	BOOL SetBrushOrg(int x, int y, LPPOINT lpPoint = NULL)
	{
		SASSERT(m_hDC != NULL);
		return ::SetBrushOrgEx(m_hDC, x, y, lpPoint);
	}

	BOOL SetBrushOrg(POINT point, LPPOINT lpPointRet = NULL)
	{
		SASSERT(m_hDC != NULL);
		return ::SetBrushOrgEx(m_hDC, point.x, point.y, lpPointRet);
	}

	int EnumObjects(int nObjectType, int (CALLBACK* lpfn)(LPVOID, LPARAM), LPARAM lpData)
	{
		SASSERT(m_hDC != NULL);
#ifdef STRICT
		return ::EnumObjects(m_hDC, nObjectType, (GOBJENUMPROC)lpfn, lpData);
#else
		return ::EnumObjects(m_hDC, nObjectType, (GOBJENUMPROC)lpfn, (LPVOID)lpData);
#endif
	}

	// Type-safe selection helpers
	HPEN SelectPen(HPEN hPen)
	{
		SASSERT(m_hDC != NULL);
		SASSERT((hPen == NULL) || (::GetObjectType(hPen) == OBJ_PEN) || (::GetObjectType(hPen) == OBJ_EXTPEN));
		return (HPEN)::SelectObject(m_hDC, hPen);
	}

	HBRUSH SelectBrush(HBRUSH hBrush)
	{
		SASSERT(m_hDC != NULL);
		SASSERT((hBrush == NULL) || (::GetObjectType(hBrush) == OBJ_BRUSH));
		return (HBRUSH)::SelectObject(m_hDC, hBrush);
	}

	HFONT SelectFont(HFONT m_hTitleFont)
	{
		SASSERT(m_hDC != NULL);
		SASSERT((m_hTitleFont == NULL) || (::GetObjectType(m_hTitleFont) == OBJ_FONT));
		return (HFONT)::SelectObject(m_hDC, m_hTitleFont);
	}

	HBITMAP SelectBitmap(HBITMAP hBitmap)
	{
		SASSERT(m_hDC != NULL);
		SASSERT((hBitmap == NULL) || (::GetObjectType(hBitmap) == OBJ_BITMAP));
		return (HBITMAP)::SelectObject(m_hDC, hBitmap);
	}

	int SelectRgn(HRGN hRgn)       // special return for regions
	{
		SASSERT(m_hDC != NULL);
		SASSERT((hRgn == NULL) || (::GetObjectType(hRgn) == OBJ_REGION));
		return PtrToInt(::SelectObject(m_hDC, hRgn));
	}

	// Type-safe selection helpers for stock objects
	HPEN SelectStockPen(int nPen)
	{
		SASSERT(m_hDC != NULL);
		SASSERT((nPen == WHITE_PEN) || (nPen == BLACK_PEN) || (nPen == NULL_PEN) || (nPen == DC_PEN));
		return SelectPen((HPEN)::GetStockObject(nPen));
	}

	HBRUSH SelectStockBrush(int nBrush)
	{
		SASSERT(((nBrush >= WHITE_BRUSH) && (nBrush <= HOLLOW_BRUSH)) || (nBrush == DC_BRUSH));
		return SelectBrush((HBRUSH)::GetStockObject(nBrush));
	}

	HFONT SelectStockFont(int nFont)
	{
		SASSERT(((nFont >= OEM_FIXED_FONT) && (nFont <= SYSTEM_FIXED_FONT)) || (nFont == DEFAULT_GUI_FONT));
		return SelectFont((HFONT)::GetStockObject(nFont));
	}

	HPALETTE SelectStockPalette(int nPalette, BOOL bForceBackground)
	{
		SASSERT(nPalette == DEFAULT_PALETTE); // the only one supported
		return SelectPalette((HPALETTE)::GetStockObject(nPalette), bForceBackground);
	}

	// Color and Color Palette Functions
	COLORREF GetNearestColor(COLORREF crColor) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetNearestColor(m_hDC, crColor);
	}

	HPALETTE SelectPalette(HPALETTE hPalette, BOOL bForceBackground)
	{
		SASSERT(m_hDC != NULL);

		return ::SelectPalette(m_hDC, hPalette, bForceBackground);
	}

	UINT RealizePalette()
	{
		SASSERT(m_hDC != NULL);
		return ::RealizePalette(m_hDC);
	}

	void UpdateColors()
	{
		SASSERT(m_hDC != NULL);
		::UpdateColors(m_hDC);
	}

	// Drawing-Attribute Functions
	COLORREF GetBkColor() const
	{
		SASSERT(m_hDC != NULL);
		return ::GetBkColor(m_hDC);
	}

	int GetBkMode() const
	{
		SASSERT(m_hDC != NULL);
		return ::GetBkMode(m_hDC);
	}

	int GetPolyFillMode() const
	{
		SASSERT(m_hDC != NULL);
		return ::GetPolyFillMode(m_hDC);
	}

	int GetROP2() const
	{
		SASSERT(m_hDC != NULL);
		return ::GetROP2(m_hDC);
	}

	int GetStretchBltMode() const
	{
		SASSERT(m_hDC != NULL);
		return ::GetStretchBltMode(m_hDC);
	}

	COLORREF GetTextColor() const
	{
		SASSERT(m_hDC != NULL);
		return ::GetTextColor(m_hDC);
	}

	COLORREF SetBkColor(COLORREF crColor)
	{
		SASSERT(m_hDC != NULL);
		return ::SetBkColor(m_hDC, crColor);
	}

	int SetBkMode(int nBkMode)
	{
		SASSERT(m_hDC != NULL);
		return ::SetBkMode(m_hDC, nBkMode);
	}

	int SetPolyFillMode(int nPolyFillMode)
	{
		SASSERT(m_hDC != NULL);
		return ::SetPolyFillMode(m_hDC, nPolyFillMode);
	}

	int SetROP2(int nDrawMode)
	{
		SASSERT(m_hDC != NULL);
		return ::SetROP2(m_hDC, nDrawMode);
	}

	int SetStretchBltMode(int nStretchMode)
	{
		SASSERT(m_hDC != NULL);
		return ::SetStretchBltMode(m_hDC, nStretchMode);
	}

	COLORREF SetTextColor(COLORREF crColor)
	{
		SASSERT(m_hDC != NULL);
		return ::SetTextColor(m_hDC, crColor);
	}

	BOOL GetColorAdjustment(LPCOLORADJUSTMENT lpColorAdjust) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetColorAdjustment(m_hDC, lpColorAdjust);
	}

	BOOL SetColorAdjustment(const COLORADJUSTMENT* lpColorAdjust)
	{
		SASSERT(m_hDC != NULL);
		return ::SetColorAdjustment(m_hDC, lpColorAdjust);
	}

	// Mapping Functions
	int GetMapMode() const
	{
		SASSERT(m_hDC != NULL);
		return ::GetMapMode(m_hDC);
	}

	BOOL GetViewportOrg(LPPOINT lpPoint) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetViewportOrgEx(m_hDC, lpPoint);
	}

	int SetMapMode(int nMapMode)
	{
		SASSERT(m_hDC != NULL);
		return ::SetMapMode(m_hDC, nMapMode);
	}

	// Viewport Origin
	BOOL SetViewportOrg(int x, int y, LPPOINT lpPoint = NULL)
	{
		SASSERT(m_hDC != NULL);
		return ::SetViewportOrgEx(m_hDC, x, y, lpPoint);
	}

	BOOL SetViewportOrg(POINT point, LPPOINT lpPointRet = NULL)
	{
		SASSERT(m_hDC != NULL);
		return SetViewportOrg(point.x, point.y, lpPointRet);
	}

	BOOL OffsetViewportOrg(int nWidth, int nHeight, LPPOINT lpPoint = NULL)
	{
		SASSERT(m_hDC != NULL);
		return ::OffsetViewportOrgEx(m_hDC, nWidth, nHeight, lpPoint);
	}

	// Viewport Extent
	BOOL GetViewportExt(LPSIZE lpSize) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetViewportExtEx(m_hDC, lpSize);
	}

	BOOL SetViewportExt(int x, int y, LPSIZE lpSize = NULL)
	{
		SASSERT(m_hDC != NULL);
		return ::SetViewportExtEx(m_hDC, x, y, lpSize);
	}

	BOOL SetViewportExt(SIZE size, LPSIZE lpSizeRet = NULL)
	{
		SASSERT(m_hDC != NULL);
		return SetViewportExt(size.cx, size.cy, lpSizeRet);
	}

	BOOL ScaleViewportExt(int xNum, int xDenom, int yNum, int yDenom, LPSIZE lpSize = NULL)
	{
		SASSERT(m_hDC != NULL);
		return ::ScaleViewportExtEx(m_hDC, xNum, xDenom, yNum, yDenom, lpSize);
	}

	// Window Origin
	BOOL GetWindowOrg(LPPOINT lpPoint) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetWindowOrgEx(m_hDC, lpPoint);
	}

	BOOL SetWindowOrg(int x, int y, LPPOINT lpPoint = NULL)
	{
		SASSERT(m_hDC != NULL);
		return ::SetWindowOrgEx(m_hDC, x, y, lpPoint);
	}

	BOOL SetWindowOrg(POINT point, LPPOINT lpPointRet = NULL)
	{
		SASSERT(m_hDC != NULL);
		return SetWindowOrg(point.x, point.y, lpPointRet);
	}

	BOOL OffsetWindowOrg(int nWidth, int nHeight, LPPOINT lpPoint = NULL)
	{
		SASSERT(m_hDC != NULL);
		return ::OffsetWindowOrgEx(m_hDC, nWidth, nHeight, lpPoint);
	}

	// Window extent
	BOOL GetWindowExt(LPSIZE lpSize) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetWindowExtEx(m_hDC, lpSize);
	}

	BOOL SetWindowExt(int x, int y, LPSIZE lpSize = NULL)
	{
		SASSERT(m_hDC != NULL);
		return ::SetWindowExtEx(m_hDC, x, y, lpSize);
	}

	BOOL SetWindowExt(SIZE size, LPSIZE lpSizeRet = NULL)
	{
		SASSERT(m_hDC != NULL);
		return SetWindowExt(size.cx, size.cy, lpSizeRet);
	}

	BOOL ScaleWindowExt(int xNum, int xDenom, int yNum, int yDenom, LPSIZE lpSize = NULL)
	{
		SASSERT(m_hDC != NULL);
		return ::ScaleWindowExtEx(m_hDC, xNum, xDenom, yNum, yDenom, lpSize);
	}

	// Coordinate Functions
	BOOL DPtoLP(LPPOINT lpPoints, int nCount = 1) const
	{
		SASSERT(m_hDC != NULL);
		return ::DPtoLP(m_hDC, lpPoints, nCount);
	}

	BOOL DPtoLP(LPRECT lpRect) const
	{
		SASSERT(m_hDC != NULL);
		return ::DPtoLP(m_hDC, (LPPOINT)lpRect, 2);
	}

	BOOL DPtoLP(LPSIZE lpSize) const
	{
		SIZE sizeWinExt = { 0, 0 };
		if (!GetWindowExt(&sizeWinExt))
			return FALSE;
		SIZE sizeVpExt = { 0, 0 };
		if (!GetViewportExt(&sizeVpExt))
			return FALSE;
		lpSize->cx = ::MulDiv(lpSize->cx, abs(sizeWinExt.cx), abs(sizeVpExt.cx));
		lpSize->cy = ::MulDiv(lpSize->cy, abs(sizeWinExt.cy), abs(sizeVpExt.cy));
		return TRUE;
	}

	BOOL LPtoDP(LPPOINT lpPoints, int nCount = 1) const
	{
		SASSERT(m_hDC != NULL);
		return ::LPtoDP(m_hDC, lpPoints, nCount);
	}

	BOOL LPtoDP(LPRECT lpRect) const
	{
		SASSERT(m_hDC != NULL);
		return ::LPtoDP(m_hDC, (LPPOINT)lpRect, 2);
	}

	BOOL LPtoDP(LPSIZE lpSize) const
	{
		SIZE sizeWinExt = { 0, 0 };
		if (!GetWindowExt(&sizeWinExt))
			return FALSE;
		SIZE sizeVpExt = { 0, 0 };
		if (!GetViewportExt(&sizeVpExt))
			return FALSE;
		lpSize->cx = ::MulDiv(lpSize->cx, abs(sizeVpExt.cx), abs(sizeWinExt.cx));
		lpSize->cy = ::MulDiv(lpSize->cy, abs(sizeVpExt.cy), abs(sizeWinExt.cy));
		return TRUE;
	}

	// Special Coordinate Functions (useful for dealing with metafiles and OLE)
#define HIMETRIC_INCH   2540    // HIMETRIC units per inch

	void DPtoHIMETRIC(LPSIZE lpSize)
	{
		SASSERT(m_hDC != NULL);
		int nMapMode = GetMapMode();
		if ((nMapMode < MM_ISOTROPIC) && (nMapMode != MM_TEXT))
		{
			// when using a constrained map mode, map against physical inch
			SetMapMode(MM_HIMETRIC);
			DPtoLP(lpSize);
			SetMapMode(nMapMode);
		}
		else
		{
			// map against logical inch for non-constrained mapping modes
			int cxPerInch = GetDeviceCaps(LOGPIXELSX);
			int cyPerInch = GetDeviceCaps(LOGPIXELSY);
			SASSERT((cxPerInch != 0) && (cyPerInch != 0));
			lpSize->cx = ::MulDiv(lpSize->cx, HIMETRIC_INCH, cxPerInch);
			lpSize->cy = ::MulDiv(lpSize->cy, HIMETRIC_INCH, cyPerInch);
		}
	}

	void HIMETRICtoDP(LPSIZE lpSize)
	{
		SASSERT(m_hDC != NULL);
		int nMapMode = GetMapMode();
		if ((nMapMode < MM_ISOTROPIC) && (nMapMode != MM_TEXT))
		{
			// when using a constrained map mode, map against physical inch
			SetMapMode(MM_HIMETRIC);
			LPtoDP(lpSize);
			SetMapMode(nMapMode);
		}
		else
		{
			// map against logical inch for non-constrained mapping modes
			int cxPerInch = GetDeviceCaps(LOGPIXELSX);
			int cyPerInch = GetDeviceCaps(LOGPIXELSY);
			SASSERT((cxPerInch != 0) && (cyPerInch != 0));
			lpSize->cx = ::MulDiv(lpSize->cx, cxPerInch, HIMETRIC_INCH);
			lpSize->cy = ::MulDiv(lpSize->cy, cyPerInch, HIMETRIC_INCH);
		}
	}

	void LPtoHIMETRIC(LPSIZE lpSize)
	{
		LPtoDP(lpSize);
		DPtoHIMETRIC(lpSize);
	}

	void HIMETRICtoLP(LPSIZE lpSize)
	{
		HIMETRICtoDP(lpSize);
		DPtoLP(lpSize);
	}

	// Region Functions
	BOOL FillRgn(HRGN hRgn, HBRUSH hBrush)
	{
		SASSERT(m_hDC != NULL);
		return ::FillRgn(m_hDC, hRgn, hBrush);
	}

	BOOL FrameRgn(HRGN hRgn, HBRUSH hBrush, int nWidth, int nHeight)
	{
		SASSERT(m_hDC != NULL);
		return ::FrameRgn(m_hDC, hRgn, hBrush, nWidth, nHeight);
	}

	BOOL InvertRgn(HRGN hRgn)
	{
		SASSERT(m_hDC != NULL);
		return ::InvertRgn(m_hDC, hRgn);
	}

	BOOL PaintRgn(HRGN hRgn)
	{
		SASSERT(m_hDC != NULL);
		return ::PaintRgn(m_hDC, hRgn);
	}

	// Clipping Functions
	int GetClipBox(LPRECT lpRect) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetClipBox(m_hDC, lpRect);
	}

// 	int GetClipRgn(CRgn& region) const
// 	{
// 		SASSERT(m_hDC != NULL);
// 		if (region.IsNull())
// 			region.CreateRectRgn(0, 0, 0, 0);
// 
// 		int nRet = ::GetClipRgn(m_hDC, region);
// 		if (nRet != 1)
// 			region.DeleteObject();
// 
// 		return nRet;
// 	}

	BOOL PtVisible(int x, int y) const
	{
		SASSERT(m_hDC != NULL);
		return ::PtVisible(m_hDC, x, y);
	}

	BOOL PtVisible(POINT point) const
	{
		SASSERT(m_hDC != NULL);
		return ::PtVisible(m_hDC, point.x, point.y);
	}

	BOOL RectVisible(LPCRECT lpRect) const
	{
		SASSERT(m_hDC != NULL);
		return ::RectVisible(m_hDC, lpRect);
	}

	int SelectClipRgn(HRGN hRgn)
	{
		SASSERT(m_hDC != NULL);
		return ::SelectClipRgn(m_hDC, (HRGN)hRgn);
	}

	int ExcludeClipRect(int x1, int y1, int x2, int y2)
	{
		SASSERT(m_hDC != NULL);
		return ::ExcludeClipRect(m_hDC, x1, y1, x2, y2);
	}

	int ExcludeClipRect(LPCRECT lpRect)
	{
		SASSERT(m_hDC != NULL);
		return ::ExcludeClipRect(m_hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	}

	int ExcludeUpdateRgn(HWND hWnd)
	{
		SASSERT(m_hDC != NULL);
		return ::ExcludeUpdateRgn(m_hDC, hWnd);
	}

	int IntersectClipRect(int x1, int y1, int x2, int y2)
	{
		SASSERT(m_hDC != NULL);
		return ::IntersectClipRect(m_hDC, x1, y1, x2, y2);
	}

	int IntersectClipRect(LPCRECT lpRect)
	{
		SASSERT(m_hDC != NULL);
		return ::IntersectClipRect(m_hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	}

	int OffsetClipRgn(int x, int y)
	{
		SASSERT(m_hDC != NULL);
		return ::OffsetClipRgn(m_hDC, x, y);
	}

	int OffsetClipRgn(SIZE size)
	{
		SASSERT(m_hDC != NULL);
		return ::OffsetClipRgn(m_hDC, size.cx, size.cy);
	}

	int SelectClipRgn(HRGN hRgn, int nMode)
	{
		SASSERT(m_hDC != NULL);
		return ::ExtSelectClipRgn(m_hDC, hRgn, nMode);
	}

	// Line-Output Functions
	BOOL GetCurrentPosition(LPPOINT lpPoint) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetCurrentPositionEx(m_hDC, lpPoint);
	}

	BOOL MoveTo(int x, int y, LPPOINT lpPoint = NULL)
	{
		SASSERT(m_hDC != NULL);
		return ::MoveToEx(m_hDC, x, y, lpPoint);
	}

	BOOL MoveTo(POINT point, LPPOINT lpPointRet = NULL)
	{
		SASSERT(m_hDC != NULL);
		return MoveTo(point.x, point.y, lpPointRet);
	}

	BOOL LineTo(int x, int y)
	{
		SASSERT(m_hDC != NULL);
		return ::LineTo(m_hDC, x, y);
	}

	BOOL LineTo(POINT point)
	{
		SASSERT(m_hDC != NULL);
		return LineTo(point.x, point.y);
	}

	BOOL Arc(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
	{
		SASSERT(m_hDC != NULL);
		return ::Arc(m_hDC, x1, y1, x2, y2, x3, y3, x4, y4);
	}

	BOOL Arc(LPCRECT lpRect, POINT ptStart, POINT ptEnd)
	{
		SASSERT(m_hDC != NULL);
		return ::Arc(m_hDC, lpRect->left, lpRect->top,
			lpRect->right, lpRect->bottom, ptStart.x, ptStart.y,
			ptEnd.x, ptEnd.y);
	}

	BOOL Polyline(const POINT* lpPoints, int nCount)
	{
		SASSERT(m_hDC != NULL);
		return ::Polyline(m_hDC, lpPoints, nCount);
	}

	BOOL AngleArc(int x, int y, int nRadius, float fStartAngle, float fSweepAngle)
	{
		SASSERT(m_hDC != NULL);
		return ::AngleArc(m_hDC, x, y, nRadius, fStartAngle, fSweepAngle);
	}

	BOOL ArcTo(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
	{
		SASSERT(m_hDC != NULL);
		return ::ArcTo(m_hDC, x1, y1, x2, y2, x3, y3, x4, y4);
	}

	BOOL ArcTo(LPCRECT lpRect, POINT ptStart, POINT ptEnd)
	{
		SASSERT(m_hDC != NULL);
		return ArcTo(lpRect->left, lpRect->top, lpRect->right,
			lpRect->bottom, ptStart.x, ptStart.y, ptEnd.x, ptEnd.y);
	}

	int GetArcDirection() const
	{
		SASSERT(m_hDC != NULL);
		return ::GetArcDirection(m_hDC);
	}

	int SetArcDirection(int nArcDirection)
	{
		SASSERT(m_hDC != NULL);
		return ::SetArcDirection(m_hDC, nArcDirection);
	}

	BOOL PolyDraw(const POINT* lpPoints, const BYTE* lpTypes, int nCount)
	{
		SASSERT(m_hDC != NULL);
		return ::PolyDraw(m_hDC, lpPoints, lpTypes, nCount);
	}

	BOOL PolylineTo(const POINT* lpPoints, int nCount)
	{
		SASSERT(m_hDC != NULL);
		return ::PolylineTo(m_hDC, lpPoints, nCount);
	}

	BOOL PolyPolyline(const POINT* lpPoints,
		const DWORD* lpPolyPoints, int nCount)
	{
		SASSERT(m_hDC != NULL);
		return ::PolyPolyline(m_hDC, lpPoints, lpPolyPoints, nCount);
	}

	BOOL PolyBezier(const POINT* lpPoints, int nCount)
	{
		SASSERT(m_hDC != NULL);
		return ::PolyBezier(m_hDC, lpPoints, nCount);
	}

	BOOL PolyBezierTo(const POINT* lpPoints, int nCount)
	{
		SASSERT(m_hDC != NULL);
		return ::PolyBezierTo(m_hDC, lpPoints, nCount);
	}

	// Simple Drawing Functions
	BOOL FillRect(LPCRECT lpRect, HBRUSH hBrush)
	{
		SASSERT(m_hDC != NULL);
		return ::FillRect(m_hDC, lpRect, hBrush);
	}

	BOOL FillRect(LPCRECT lpRect, int nColorIndex)
	{
		SASSERT(m_hDC != NULL);
		return ::FillRect(m_hDC, lpRect, (HBRUSH)LongToPtr(nColorIndex + 1));
	}

	BOOL FrameRect(LPCRECT lpRect, HBRUSH hBrush)
	{
		SASSERT(m_hDC != NULL);
		return ::FrameRect(m_hDC, lpRect, hBrush);
	}

	BOOL InvertRect(LPCRECT lpRect)
	{
		SASSERT(m_hDC != NULL);
		return ::InvertRect(m_hDC, lpRect);
	}

	BOOL DrawIcon(int x, int y, HICON hIcon)
	{
		SASSERT(m_hDC != NULL);
		return ::DrawIcon(m_hDC, x, y, hIcon);
	}

	BOOL DrawIcon(POINT point, HICON hIcon)
	{
		SASSERT(m_hDC != NULL);
		return ::DrawIcon(m_hDC, point.x, point.y, hIcon);
	}

	BOOL DrawIconEx(int x, int y, HICON hIcon, int cxWidth, int cyWidth, UINT uStepIfAniCur = 0, HBRUSH hbrFlickerFreeDraw = NULL, UINT uFlags = DI_NORMAL)
	{
		SASSERT(m_hDC != NULL);
		return ::DrawIconEx(m_hDC, x, y, hIcon, cxWidth, cyWidth, uStepIfAniCur, hbrFlickerFreeDraw, uFlags);
	}

	BOOL DrawIconEx(POINT point, HICON hIcon, SIZE size, UINT uStepIfAniCur = 0, HBRUSH hbrFlickerFreeDraw = NULL, UINT uFlags = DI_NORMAL)
	{
		SASSERT(m_hDC != NULL);
		return ::DrawIconEx(m_hDC, point.x, point.y, hIcon, size.cx, size.cy, uStepIfAniCur, hbrFlickerFreeDraw, uFlags);
	}

	BOOL DrawState(POINT pt, SIZE size, HBITMAP hBitmap, UINT nFlags, HBRUSH hBrush = NULL)
	{
		SASSERT(m_hDC != NULL);
		return ::DrawState(m_hDC, hBrush, NULL, (LPARAM)hBitmap, 0, pt.x, pt.y, size.cx, size.cy, nFlags | DST_BITMAP);
	}

	BOOL DrawState(POINT pt, SIZE size, HICON hIcon, UINT nFlags, HBRUSH hBrush = NULL)
	{
		SASSERT(m_hDC != NULL);
		return ::DrawState(m_hDC, hBrush, NULL, (LPARAM)hIcon, 0, pt.x, pt.y, size.cx, size.cy, nFlags | DST_ICON);
	}

	BOOL DrawState(POINT pt, SIZE size, LPCTSTR lpszText, UINT nFlags, BOOL bPrefixText = TRUE, int nTextLen = 0, HBRUSH hBrush = NULL)
	{
		SASSERT(m_hDC != NULL);
		return ::DrawState(m_hDC, hBrush, NULL, (LPARAM)lpszText, (WPARAM)nTextLen, pt.x, pt.y, size.cx, size.cy, nFlags | (bPrefixText ? DST_PREFIXTEXT : DST_TEXT));
	}

	BOOL DrawState(POINT pt, SIZE size, DRAWSTATEPROC lpDrawProc, LPARAM lData, UINT nFlags, HBRUSH hBrush = NULL)
	{
		SASSERT(m_hDC != NULL);
		return ::DrawState(m_hDC, hBrush, lpDrawProc, lData, 0, pt.x, pt.y, size.cx, size.cy, nFlags | DST_COMPLEX);
	}

	// Ellipse and Polygon Functions
	BOOL Chord(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
	{
		SASSERT(m_hDC != NULL);
		return ::Chord(m_hDC, x1, y1, x2, y2, x3, y3, x4, y4);
	}

	BOOL Chord(LPCRECT lpRect, POINT ptStart, POINT ptEnd)
	{
		SASSERT(m_hDC != NULL);
		return ::Chord(m_hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, ptStart.x, ptStart.y, ptEnd.x, ptEnd.y);
	}

	void DrawFocusRect(LPCRECT lpRect)
	{
		SASSERT(m_hDC != NULL);
		::DrawFocusRect(m_hDC, lpRect);
	}

	BOOL Ellipse(int x1, int y1, int x2, int y2)
	{
		SASSERT(m_hDC != NULL);
		return ::Ellipse(m_hDC, x1, y1, x2, y2);
	}

	BOOL Ellipse(LPCRECT lpRect)
	{
		SASSERT(m_hDC != NULL);
		return ::Ellipse(m_hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	}

	BOOL Pie(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
	{
		SASSERT(m_hDC != NULL);
		return ::Pie(m_hDC, x1, y1, x2, y2, x3, y3, x4, y4);
	}

	BOOL Pie(LPCRECT lpRect, POINT ptStart, POINT ptEnd)
	{
		SASSERT(m_hDC != NULL);
		return ::Pie(m_hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, ptStart.x, ptStart.y, ptEnd.x, ptEnd.y);
	}

	BOOL Polygon(const POINT* lpPoints, int nCount)
	{
		SASSERT(m_hDC != NULL);
		return ::Polygon(m_hDC, lpPoints, nCount);
	}

	BOOL PolyPolygon(const POINT* lpPoints, const INT* lpPolyCounts, int nCount)
	{
		SASSERT(m_hDC != NULL);
		return ::PolyPolygon(m_hDC, lpPoints, lpPolyCounts, nCount);
	}

	BOOL Rectangle(int x1, int y1, int x2, int y2)
	{
		SASSERT(m_hDC != NULL);
		return ::Rectangle(m_hDC, x1, y1, x2, y2);
	}

	BOOL Rectangle(LPCRECT lpRect)
	{
		SASSERT(m_hDC != NULL);
		return ::Rectangle(m_hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	}

	BOOL RoundRect(int x1, int y1, int x2, int y2, int x3, int y3)
	{
		SASSERT(m_hDC != NULL);
		return ::RoundRect(m_hDC, x1, y1, x2, y2, x3, y3);
	}

	BOOL RoundRect(LPCRECT lpRect, POINT point)
	{
		SASSERT(m_hDC != NULL);
		return ::RoundRect(m_hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, point.x, point.y);
	}

	// Bitmap Functions
	BOOL PatBlt(int x, int y, int nWidth, int nHeight, DWORD dwRop)
	{
		SASSERT(m_hDC != NULL);
		return ::PatBlt(m_hDC, x, y, nWidth, nHeight, dwRop);
	}

	BOOL BitBlt(int x, int y, int nWidth, int nHeight, HDC hSrcDC,
		int xSrc, int ySrc, DWORD dwRop)
	{
		SASSERT(m_hDC != NULL);
		return ::BitBlt(m_hDC, x, y, nWidth, nHeight, hSrcDC, xSrc, ySrc, dwRop);
	}

	BOOL StretchBlt(int x, int y, int nWidth, int nHeight, HDC hSrcDC, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, DWORD dwRop)
	{
		SASSERT(m_hDC != NULL);
		return ::StretchBlt(m_hDC, x, y, nWidth, nHeight, hSrcDC, xSrc, ySrc, nSrcWidth, nSrcHeight, dwRop);
	}

	COLORREF GetPixel(int x, int y) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetPixel(m_hDC, x, y);
	}

	COLORREF GetPixel(POINT point) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetPixel(m_hDC, point.x, point.y);
	}

	COLORREF SetPixel(int x, int y, COLORREF crColor)
	{
		SASSERT(m_hDC != NULL);
		return ::SetPixel(m_hDC, x, y, crColor);
	}

	COLORREF SetPixel(POINT point, COLORREF crColor)
	{
		SASSERT(m_hDC != NULL);
		return ::SetPixel(m_hDC, point.x, point.y, crColor);
	}

	BOOL FloodFill(int x, int y, COLORREF crColor)
	{
		SASSERT(m_hDC != NULL);
		return ::FloodFill(m_hDC, x, y, crColor);
	}

	BOOL ExtFloodFill(int x, int y, COLORREF crColor, UINT nFillType)
	{
		SASSERT(m_hDC != NULL);
		return ::ExtFloodFill(m_hDC, x, y, crColor, nFillType);
	}

	BOOL MaskBlt(int x, int y, int nWidth, int nHeight, HDC hSrcDC, int xSrc, int ySrc, HBITMAP hMaskBitmap, int xMask, int yMask, DWORD dwRop)
	{
		SASSERT(m_hDC != NULL);
		return ::MaskBlt(m_hDC, x, y, nWidth, nHeight, hSrcDC, xSrc, ySrc, hMaskBitmap, xMask, yMask, dwRop);
	}

	BOOL PlgBlt(LPPOINT lpPoint, HDC hSrcDC, int xSrc, int ySrc, int nWidth, int nHeight, HBITMAP hMaskBitmap, int xMask, int yMask)
	{
		SASSERT(m_hDC != NULL);
		return ::PlgBlt(m_hDC, lpPoint, hSrcDC, xSrc, ySrc, nWidth, nHeight, hMaskBitmap, xMask, yMask);
	}

	BOOL SetPixelV(int x, int y, COLORREF crColor)
	{
		SASSERT(m_hDC != NULL);
		return ::SetPixelV(m_hDC, x, y, crColor);
	}

	BOOL SetPixelV(POINT point, COLORREF crColor)
	{
		SASSERT(m_hDC != NULL);
		return ::SetPixelV(m_hDC, point.x, point.y, crColor);
	}

#if !defined(_ATL_NO_MSIMG)
	BOOL TransparentBlt(int x, int y, int nWidth, int nHeight, HDC hSrcDC, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, UINT crTransparent)
	{
		SASSERT(m_hDC != NULL);
		return ::TransparentBlt(m_hDC, x, y, nWidth, nHeight, hSrcDC, xSrc, ySrc, nSrcWidth, nSrcHeight, crTransparent);
	}

	BOOL GradientFill(const PTRIVERTEX pVertices, DWORD nVertices, void* pMeshElements, DWORD nMeshElements, DWORD dwMode)
	{
		SASSERT(m_hDC != NULL);
		return ::GradientFill(m_hDC, pVertices, nVertices, pMeshElements, nMeshElements, dwMode);
	}

	BOOL GradientFillRect(RECT& rect, COLORREF clr1, COLORREF clr2, bool bHorizontal)
	{
		SASSERT(m_hDC != NULL);

		TRIVERTEX arrTvx[2] = { { 0 },{ 0 } };

		arrTvx[0].x = rect.left;
		arrTvx[0].y = rect.top;
		arrTvx[0].Red = MAKEWORD(0, GetRValue(clr1));
		arrTvx[0].Green = MAKEWORD(0, GetGValue(clr1));
		arrTvx[0].Blue = MAKEWORD(0, GetBValue(clr1));
		arrTvx[0].Alpha = 0;

		arrTvx[1].x = rect.right;
		arrTvx[1].y = rect.bottom;
		arrTvx[1].Red = MAKEWORD(0, GetRValue(clr2));
		arrTvx[1].Green = MAKEWORD(0, GetGValue(clr2));
		arrTvx[1].Blue = MAKEWORD(0, GetBValue(clr2));
		arrTvx[1].Alpha = 0;

		GRADIENT_RECT gr = { 0, 1 };

		return ::GradientFill(m_hDC, arrTvx, 2, &gr, 1, bHorizontal ? GRADIENT_FILL_RECT_H : GRADIENT_FILL_RECT_V);
	}

	BOOL AlphaBlend(int x, int y, int nWidth, int nHeight, HDC hSrcDC, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, BLENDFUNCTION bf)
	{
		SASSERT(m_hDC != NULL);
		return ::AlphaBlend(m_hDC, x, y, nWidth, nHeight, hSrcDC, xSrc, ySrc, nSrcWidth, nSrcHeight, bf);
	}
#endif //  !defined(_ATL_NO_MSIMG)

	// Extra bitmap functions
	// Helper function for painting a disabled toolbar or menu bitmap
	// This function can take either an HBITMAP (for SS) or a DC with 
	//           the bitmap already painted (for cmdbar)
	BOOL DitherBlt(int x, int y, int nWidth, int nHeight, HDC hSrcDC, HBITMAP hBitmap, int xSrc, int ySrc,
		HBRUSH hBrushBackground = ::GetSysColorBrush(COLOR_3DFACE),
		HBRUSH hBrush3DEffect = ::GetSysColorBrush(COLOR_3DHILIGHT),
		HBRUSH hBrushDisabledImage = ::GetSysColorBrush(COLOR_3DSHADOW))
	{
		SASSERT((m_hDC != NULL) || (hBitmap != NULL));
		SASSERT((nWidth > 0) && (nHeight > 0));

		// Create a generic DC for all BitBlts
		CDCT<false> dc = (hSrcDC != NULL) ? hSrcDC : ::CreateCompatibleDC(m_hDC);
		SASSERT(dc.m_hDC != NULL);
		if (dc.m_hDC == NULL)
			return FALSE;

		// Create a DC for the monochrome DIB section
		CDCT<true> dcBW = ::CreateCompatibleDC(m_hDC);
		SASSERT(dcBW.m_hDC != NULL);
		if (dcBW.m_hDC == NULL)
		{
			if (hSrcDC == NULL)
				dc.DeleteDC();
			return FALSE;
		}

		// Create the monochrome DIB section with a black and white palette
		struct RGBBWBITMAPINFO
		{
			BITMAPINFOHEADER bmiHeader;
			RGBQUAD bmiColors[2];
		};

		RGBBWBITMAPINFO rgbBWBitmapInfo =
		{
			{ sizeof(BITMAPINFOHEADER), nWidth, nHeight, 1, 1, BI_RGB, 0, 0, 0, 0, 0 },
		{ { 0x00, 0x00, 0x00, 0x00 },{ 0xFF, 0xFF, 0xFF, 0x00 } }
		};

		VOID* pbitsBW;
		CBitmap bmpBW = ::CreateDIBSection(dcBW, (LPBITMAPINFO)&rgbBWBitmapInfo, DIB_RGB_COLORS, &pbitsBW, NULL, 0);
		SASSERT(bmpBW.m_hBitmap != NULL);
		if (bmpBW.m_hBitmap == NULL)
		{
			if (hSrcDC == NULL)
				dc.DeleteDC();
			return FALSE;
		}

		// Attach the monochrome DIB section and the bitmap to the DCs
		HBITMAP hbmOldBW = dcBW.SelectBitmap(bmpBW);
		HBITMAP hbmOldDC = NULL;
		if (hBitmap != NULL)
			hbmOldDC = dc.SelectBitmap(hBitmap);

		// Block: Dark gray removal: we want (128, 128, 128) pixels to become black and not white
		{
			CDCT<true> dcTemp1 = ::CreateCompatibleDC(m_hDC);
			CDCT<true> dcTemp2 = ::CreateCompatibleDC(m_hDC);
			CBitmap bmpTemp1;
			bmpTemp1.CreateCompatibleBitmap(dc, nWidth, nHeight);
			CBitmap bmpTemp2;
			bmpTemp2.CreateBitmap(nWidth, nHeight, 1, 1, NULL);
			HBITMAP hOldBmp1 = dcTemp1.SelectBitmap(bmpTemp1);
			HBITMAP hOldBmp2 = dcTemp2.SelectBitmap(bmpTemp2);
			// Let's copy our image, it will be altered
			dcTemp1.BitBlt(0, 0, nWidth, nHeight, dc, xSrc, ySrc, SRCCOPY);

			// All dark gray pixels will become white, the others black
			dcTemp1.SetBkColor(RGB(128, 128, 128));
			dcTemp2.BitBlt(0, 0, nWidth, nHeight, dcTemp1, 0, 0, SRCCOPY);
			// Do an XOR to set to black these white pixels
			dcTemp1.BitBlt(0, 0, nWidth, nHeight, dcTemp2, 0, 0, SRCINVERT);

			// BitBlt the bitmap into the monochrome DIB section
			// The DIB section will do a true monochrome conversion
			// The magenta background being closer to white will become white
			dcBW.BitBlt(0, 0, nWidth, nHeight, dcTemp1, 0, 0, SRCCOPY);

			// Cleanup
			dcTemp1.SelectBitmap(hOldBmp1);
			dcTemp2.SelectBitmap(hOldBmp2);
		}

		// Paint the destination rectangle using hBrushBackground
		if (hBrushBackground != NULL)
		{
			RECT rc = { x, y, x + nWidth, y + nHeight };
			FillRect(&rc, hBrushBackground);
		}

		// BitBlt the black bits in the monochrome bitmap into hBrush3DEffect color in the destination DC
		// The magic ROP comes from the Charles Petzold's book
		HBRUSH hOldBrush = SelectBrush(hBrush3DEffect);
		BitBlt(x + 1, y + 1, nWidth, nHeight, dcBW, 0, 0, 0xB8074A);

		// BitBlt the black bits in the monochrome bitmap into hBrushDisabledImage color in the destination DC
		SelectBrush(hBrushDisabledImage);
		BitBlt(x, y, nWidth, nHeight, dcBW, 0, 0, 0xB8074A);

		SelectBrush(hOldBrush);
		dcBW.SelectBitmap(hbmOldBW);
		dc.SelectBitmap(hbmOldDC);

		if (hSrcDC == NULL)
			dc.DeleteDC();

		return TRUE;
	}

	// Text Functions
	BOOL TextOut(int x, int y, LPCTSTR lpszString, int nCount = -1)
	{
		SASSERT(m_hDC != NULL);
		if (nCount == -1)
			nCount = lstrlen(lpszString);
		return ::TextOut(m_hDC, x, y, lpszString, nCount);
	}

	BOOL ExtTextOut(int x, int y, UINT nOptions, LPCRECT lpRect, LPCTSTR lpszString, UINT nCount = -1, LPINT lpDxWidths = NULL)
	{
		SASSERT(m_hDC != NULL);
		if (nCount == -1)
			nCount = lstrlen(lpszString);
		return ::ExtTextOut(m_hDC, x, y, nOptions, lpRect, lpszString, nCount, lpDxWidths);
	}

	SIZE TabbedTextOut(int x, int y, LPCTSTR lpszString, int nCount = -1, int nTabPositions = 0, LPINT lpnTabStopPositions = NULL, int nTabOrigin = 0)
	{
		SASSERT(m_hDC != NULL);
		if (nCount == -1)
			nCount = lstrlen(lpszString);
		LONG lRes = ::TabbedTextOut(m_hDC, x, y, lpszString, nCount, nTabPositions, lpnTabStopPositions, nTabOrigin);
		SIZE size = { GET_X_LPARAM(lRes), GET_Y_LPARAM(lRes) };
		return size;
	}

	int DrawText(LPCTSTR lpstrText, int cchText, LPRECT lpRect, UINT uFormat)
	{
		SASSERT(m_hDC != NULL);
		SASSERT((uFormat & DT_MODIFYSTRING) == 0);
		return ::DrawText(m_hDC, lpstrText, cchText, lpRect, uFormat);
	}

	int DrawText(LPTSTR lpstrText, int cchText, LPRECT lpRect, UINT uFormat)
	{
		SASSERT(m_hDC != NULL);
		return ::DrawText(m_hDC, lpstrText, cchText, lpRect, uFormat);
	}

	int DrawTextEx(LPTSTR lpstrText, int cchText, LPRECT lpRect, UINT uFormat, LPDRAWTEXTPARAMS lpDTParams = NULL)
	{
		SASSERT(m_hDC != NULL);
		return ::DrawTextEx(m_hDC, lpstrText, cchText, lpRect, uFormat, lpDTParams);
	}

	// Note - ::DrawShadowText() is present only if comctl32.dll version 6 is loaded
	int DrawShadowText(LPCWSTR lpstrText, int cchText, LPRECT lpRect, DWORD dwFlags, COLORREF clrText, COLORREF clrShadow, int xOffset, int yOffset)
	{
		SASSERT(m_hDC != NULL);
		SASSERT(lpRect != NULL);
		return ::DrawShadowText(m_hDC, lpstrText, cchText, lpRect, dwFlags, clrText, clrShadow, xOffset, yOffset);
	}

	BOOL GetTextExtent(LPCTSTR lpszString, int nCount, LPSIZE lpSize) const
	{
		SASSERT(m_hDC != NULL);
		if (nCount == -1)
			nCount = lstrlen(lpszString);
		return ::GetTextExtentPoint32(m_hDC, lpszString, nCount, lpSize);
	}

	BOOL GetTextExtentExPoint(LPCTSTR lpszString, int cchString, LPSIZE lpSize, int nMaxExtent, LPINT lpnFit = NULL, LPINT alpDx = NULL)
	{
		SASSERT(m_hDC != NULL);
		return ::GetTextExtentExPoint(m_hDC, lpszString, cchString, nMaxExtent, lpnFit, alpDx, lpSize);
	}

	DWORD GetTabbedTextExtent(LPCTSTR lpszString, int nCount = -1, int nTabPositions = 0, LPINT lpnTabStopPositions = NULL) const
	{
		SASSERT(m_hDC != NULL);
		if (nCount == -1)
			nCount = lstrlen(lpszString);
		return ::GetTabbedTextExtent(m_hDC, lpszString, nCount, nTabPositions, lpnTabStopPositions);
	}

	BOOL GrayString(HBRUSH hBrush, BOOL(CALLBACK* lpfnOutput)(HDC, LPARAM, int), LPARAM lpData, int nCount, int x, int y, int nWidth, int nHeight)
	{
		SASSERT(m_hDC != NULL);
		return ::GrayString(m_hDC, hBrush, (GRAYSTRINGPROC)lpfnOutput, lpData, nCount, x, y, nWidth, nHeight);
	}

	UINT GetTextAlign() const
	{
		SASSERT(m_hDC != NULL);
		return ::GetTextAlign(m_hDC);
	}

	UINT SetTextAlign(UINT nFlags)
	{
		SASSERT(m_hDC != NULL);
		return ::SetTextAlign(m_hDC, nFlags);
	}

	int GetTextFace(LPTSTR lpszFacename, int nCount) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetTextFace(m_hDC, nCount, lpszFacename);
	}

	int GetTextFaceLen() const
	{
		SASSERT(m_hDC != NULL);
		return ::GetTextFace(m_hDC, 0, NULL);
	}

#ifdef _OLEAUTO_H_
	BOOL GetTextFace(BSTR& bstrFace) const
	{
		USES_CONVERSION;
		SASSERT(m_hDC != NULL);
		SASSERT(bstrFace == NULL);

		int nLen = GetTextFaceLen();
		if (nLen == 0)
			return FALSE;

		ATL::CTempBuffer<TCHAR, _WTL_STACK_ALLOC_THRESHOLD> buff;
		LPTSTR lpszText = buff.Allocate(nLen);
		if (lpszText == NULL)
			return FALSE;

		if (!GetTextFace(lpszText, nLen))
			return FALSE;

		bstrFace = ::SysAllocString(T2OLE(lpszText));
		return (bstrFace != NULL) ? TRUE : FALSE;
	}
#endif

#ifdef __ATLSTR_H__
	int GetTextFace(ATL::CString& strFace) const
	{
		SASSERT(m_hDC != NULL);

		int nLen = GetTextFaceLen();
		if (nLen == 0)
			return 0;

		LPTSTR lpstr = strFace.GetBufferSetLength(nLen);
		if (lpstr == NULL)
			return 0;
		int nRet = GetTextFace(lpstr, nLen);
		strFace.ReleaseBuffer();
		return nRet;
	}
#endif // __ATLSTR_H__

	BOOL GetTextMetrics(LPTEXTMETRIC lpMetrics) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetTextMetrics(m_hDC, lpMetrics);
	}

	int SetTextJustification(int nBreakExtra, int nBreakCount)
	{
		SASSERT(m_hDC != NULL);
		return ::SetTextJustification(m_hDC, nBreakExtra, nBreakCount);
	}

	int GetTextCharacterExtra() const
	{
		SASSERT(m_hDC != NULL);
		return ::GetTextCharacterExtra(m_hDC);
	}

	int SetTextCharacterExtra(int nCharExtra)
	{
		SASSERT(m_hDC != NULL);
		return ::SetTextCharacterExtra(m_hDC, nCharExtra);
	}

	// Advanced Drawing
	BOOL DrawEdge(LPRECT lpRect, UINT nEdge, UINT nFlags)
	{
		SASSERT(m_hDC != NULL);
		return ::DrawEdge(m_hDC, lpRect, nEdge, nFlags);
	}

	BOOL DrawFrameControl(LPRECT lpRect, UINT nType, UINT nState)
	{
		SASSERT(m_hDC != NULL);
		return ::DrawFrameControl(m_hDC, lpRect, nType, nState);
	}

	// Scrolling Functions
	BOOL ScrollDC(int dx, int dy, LPCRECT lpRectScroll, LPCRECT lpRectClip, HRGN hRgnUpdate, LPRECT lpRectUpdate)
	{
		SASSERT(m_hDC != NULL);
		return ::ScrollDC(m_hDC, dx, dy, lpRectScroll, lpRectClip, hRgnUpdate, lpRectUpdate);
	}

	// Font Functions
	BOOL GetCharWidth(UINT nFirstChar, UINT nLastChar, LPINT lpBuffer) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetCharWidth(m_hDC, nFirstChar, nLastChar, lpBuffer);
	}

	// GetCharWidth32 is not supported under Win9x
	BOOL GetCharWidth32(UINT nFirstChar, UINT nLastChar, LPINT lpBuffer) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetCharWidth32(m_hDC, nFirstChar, nLastChar, lpBuffer);
	}

	DWORD SetMapperFlags(DWORD dwFlag)
	{
		SASSERT(m_hDC != NULL);
		return ::SetMapperFlags(m_hDC, dwFlag);
	}

	BOOL GetAspectRatioFilter(LPSIZE lpSize) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetAspectRatioFilterEx(m_hDC, lpSize);
	}

	BOOL GetCharABCWidths(UINT nFirstChar, UINT nLastChar, LPABC lpabc) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetCharABCWidths(m_hDC, nFirstChar, nLastChar, lpabc);
	}

	DWORD GetFontData(DWORD dwTable, DWORD dwOffset, LPVOID lpData, DWORD cbData) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetFontData(m_hDC, dwTable, dwOffset, lpData, cbData);
	}

	int GetKerningPairs(int nPairs, LPKERNINGPAIR lpkrnpair) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetKerningPairs(m_hDC, nPairs, lpkrnpair);
	}

	UINT GetOutlineTextMetrics(UINT cbData, LPOUTLINETEXTMETRIC lpotm) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetOutlineTextMetrics(m_hDC, cbData, lpotm);
	}

	DWORD GetGlyphOutline(UINT nChar, UINT nFormat, LPGLYPHMETRICS lpgm, DWORD cbBuffer, LPVOID lpBuffer, const MAT2* lpmat2) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetGlyphOutline(m_hDC, nChar, nFormat, lpgm, cbBuffer, lpBuffer, lpmat2);
	}

	BOOL GetCharABCWidths(UINT nFirstChar, UINT nLastChar, LPABCFLOAT lpABCF) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetCharABCWidthsFloat(m_hDC, nFirstChar, nLastChar, lpABCF);
	}

	BOOL GetCharWidth(UINT nFirstChar, UINT nLastChar, float* lpFloatBuffer) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetCharWidthFloat(m_hDC, nFirstChar, nLastChar, lpFloatBuffer);
	}

	// Printer/Device Escape Functions
	int Escape(int nEscape, int nCount, LPCSTR lpszInData, LPVOID lpOutData)
	{
		SASSERT(m_hDC != NULL);
		return ::Escape(m_hDC, nEscape, nCount, lpszInData, lpOutData);
	}

	int Escape(int nEscape, int nInputSize, LPCSTR lpszInputData,
		int nOutputSize, LPSTR lpszOutputData)
	{
		SASSERT(m_hDC != NULL);
		return ::ExtEscape(m_hDC, nEscape, nInputSize, lpszInputData, nOutputSize, lpszOutputData);
	}

	int DrawEscape(int nEscape, int nInputSize, LPCSTR lpszInputData)
	{
		SASSERT(m_hDC != NULL);
		return ::DrawEscape(m_hDC, nEscape, nInputSize, lpszInputData);
	}

	// Escape helpers
	int StartDoc(LPCTSTR lpszDocName)  // old Win3.0 version
	{
		DOCINFO di = { 0 };
		di.cbSize = sizeof(DOCINFO);
		di.lpszDocName = lpszDocName;
		return StartDoc(&di);
	}

	int StartDoc(LPDOCINFO lpDocInfo)
	{
		SASSERT(m_hDC != NULL);
		return ::StartDoc(m_hDC, lpDocInfo);
	}

	int StartPage()
	{
		SASSERT(m_hDC != NULL);
		return ::StartPage(m_hDC);
	}

	int EndPage()
	{
		SASSERT(m_hDC != NULL);
		return ::EndPage(m_hDC);
	}

	int SetAbortProc(BOOL(CALLBACK* lpfn)(HDC, int))
	{
		SASSERT(m_hDC != NULL);
		return ::SetAbortProc(m_hDC, (ABORTPROC)lpfn);
	}

	int AbortDoc()
	{
		SASSERT(m_hDC != NULL);
		return ::AbortDoc(m_hDC);
	}

	int EndDoc()
	{
		SASSERT(m_hDC != NULL);
		return ::EndDoc(m_hDC);
	}

	// MetaFile Functions
	BOOL PlayMetaFile(HMETAFILE hMF)
	{
		SASSERT(m_hDC != NULL);
		if (::GetDeviceCaps(m_hDC, TECHNOLOGY) == DT_METAFILE)
		{
			// playing metafile in metafile, just use core windows API
			return ::PlayMetaFile(m_hDC, hMF);
		}

		// for special playback, lParam == pDC
		return ::EnumMetaFile(m_hDC, hMF, EnumMetaFileProc, (LPARAM)this);
	}

	BOOL PlayMetaFile(HENHMETAFILE hEnhMetaFile, LPCRECT lpBounds)
	{
		SASSERT(m_hDC != NULL);
		return ::PlayEnhMetaFile(m_hDC, hEnhMetaFile, lpBounds);
	}

	BOOL AddMetaFileComment(UINT nDataSize, const BYTE* pCommentData) // can be used for enhanced metafiles only
	{
		SASSERT(m_hDC != NULL);
		return ::GdiComment(m_hDC, nDataSize, pCommentData);
	}

	// Special handling for metafile playback
	static int CALLBACK EnumMetaFileProc(HDC hDC, HANDLETABLE* pHandleTable, METARECORD* pMetaRec, int nHandles, LPARAM lParam)
	{
		CDCT<false>* pDC = (CDCT<false>*)lParam;

		switch (pMetaRec->rdFunction)
		{
		case META_SETMAPMODE:
			pDC->SetMapMode((int)(short)pMetaRec->rdParm[0]);
			break;
		case META_SETWINDOWEXT:
			pDC->SetWindowExt((int)(short)pMetaRec->rdParm[1], (int)(short)pMetaRec->rdParm[0]);
			break;
		case META_SETWINDOWORG:
			pDC->SetWindowOrg((int)(short)pMetaRec->rdParm[1], (int)(short)pMetaRec->rdParm[0]);
			break;
		case META_SETVIEWPORTEXT:
			pDC->SetViewportExt((int)(short)pMetaRec->rdParm[1], (int)(short)pMetaRec->rdParm[0]);
			break;
		case META_SETVIEWPORTORG:
			pDC->SetViewportOrg((int)(short)pMetaRec->rdParm[1], (int)(short)pMetaRec->rdParm[0]);
			break;
		case META_SCALEWINDOWEXT:
			pDC->ScaleWindowExt((int)(short)pMetaRec->rdParm[3], (int)(short)pMetaRec->rdParm[2],
				(int)(short)pMetaRec->rdParm[1], (int)(short)pMetaRec->rdParm[0]);
			break;
		case META_SCALEVIEWPORTEXT:
			pDC->ScaleViewportExt((int)(short)pMetaRec->rdParm[3], (int)(short)pMetaRec->rdParm[2],
				(int)(short)pMetaRec->rdParm[1], (int)(short)pMetaRec->rdParm[0]);
			break;
		case META_OFFSETVIEWPORTORG:
			pDC->OffsetViewportOrg((int)(short)pMetaRec->rdParm[1], (int)(short)pMetaRec->rdParm[0]);
			break;
		case META_SAVEDC:
			pDC->SaveDC();
			break;
		case META_RESTOREDC:
			pDC->RestoreDC((int)(short)pMetaRec->rdParm[0]);
			break;
		case META_SETBKCOLOR:
			pDC->SetBkColor(*(UNALIGNED COLORREF*)&pMetaRec->rdParm[0]);
			break;
		case META_SETTEXTCOLOR:
			pDC->SetTextColor(*(UNALIGNED COLORREF*)&pMetaRec->rdParm[0]);
			break;

			// need to watch out for SelectObject(HFONT), for custom font mapping
		case META_SELECTOBJECT:
		{
			HGDIOBJ hObject = pHandleTable->objectHandle[pMetaRec->rdParm[0]];
			UINT nObjType = ::GetObjectType(hObject);
			if (nObjType == 0)
			{
				// object type is unknown, determine if it is a font
				HFONT hStockFont = (HFONT)::GetStockObject(SYSTEM_FONT);
				HFONT hFontOld = (HFONT)::SelectObject(pDC->m_hDC, hStockFont);
				HGDIOBJ hObjOld = ::SelectObject(pDC->m_hDC, hObject);
				if (hObjOld == hStockFont)
				{
					// got the stock object back, so must be selecting a font
					pDC->SelectFont((HFONT)hObject);
					break;  // don't play the default record
				}
				else
				{
					// didn't get the stock object back, so restore everything
					::SelectObject(pDC->m_hDC, hFontOld);
					::SelectObject(pDC->m_hDC, hObjOld);
				}
				// and fall through to PlayMetaFileRecord...
			}
			else if (nObjType == OBJ_FONT)
			{
				// play back as CDCHandle::SelectFont(HFONT)
				pDC->SelectFont((HFONT)hObject);
				break;  // don't play the default record
			}
		}
		// fall through...

		default:
			::PlayMetaFileRecord(hDC, pHandleTable, pMetaRec, nHandles);
			break;
		}

		return 1;
	}

	// Path Functions
	BOOL AbortPath()
	{
		SASSERT(m_hDC != NULL);
		return ::AbortPath(m_hDC);
	}

	BOOL BeginPath()
	{
		SASSERT(m_hDC != NULL);
		return ::BeginPath(m_hDC);
	}

	BOOL CloseFigure()
	{
		SASSERT(m_hDC != NULL);
		return ::CloseFigure(m_hDC);
	}

	BOOL EndPath()
	{
		SASSERT(m_hDC != NULL);
		return ::EndPath(m_hDC);
	}

	BOOL FillPath()
	{
		SASSERT(m_hDC != NULL);
		return ::FillPath(m_hDC);
	}

	BOOL FlattenPath()
	{
		SASSERT(m_hDC != NULL);
		return ::FlattenPath(m_hDC);
	}

	BOOL StrokeAndFillPath()
	{
		SASSERT(m_hDC != NULL);
		return ::StrokeAndFillPath(m_hDC);
	}

	BOOL StrokePath()
	{
		SASSERT(m_hDC != NULL);
		return ::StrokePath(m_hDC);
	}

	BOOL WidenPath()
	{
		SASSERT(m_hDC != NULL);
		return ::WidenPath(m_hDC);
	}

	BOOL GetMiterLimit(PFLOAT pfMiterLimit) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetMiterLimit(m_hDC, pfMiterLimit);
	}

	BOOL SetMiterLimit(float fMiterLimit)
	{
		SASSERT(m_hDC != NULL);
		return ::SetMiterLimit(m_hDC, fMiterLimit, NULL);
	}

	int GetPath(LPPOINT lpPoints, LPBYTE lpTypes, int nCount) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetPath(m_hDC, lpPoints, lpTypes, nCount);
	}

	BOOL SelectClipPath(int nMode)
	{
		SASSERT(m_hDC != NULL);
		return ::SelectClipPath(m_hDC, nMode);
	}

	// Misc Helper Functions
	static HBRUSH PASCAL GetHalftoneBrush()
	{
		HBRUSH halftoneBrush = NULL;
		WORD grayPattern[8] = { 0 };
		for (int i = 0; i < 8; i++)
			grayPattern[i] = (WORD)(0x5555 << (i & 1));
		HBITMAP grayBitmap = CreateBitmap(8, 8, 1, 1, &grayPattern);
		if (grayBitmap != NULL)
		{
			halftoneBrush = ::CreatePatternBrush(grayBitmap);
			DeleteObject(grayBitmap);
		}
		return (halftoneBrush);
	}

	void DrawDragRect(LPCRECT lpRect, SIZE size, LPCRECT lpRectLast, SIZE sizeLast, HBRUSH hBrush = NULL, HBRUSH hBrushLast = NULL)
	{
		// first, determine the update region and select it
		CRgn rgnOutside;
		rgnOutside.CreateRectRgnIndirect(lpRect);
		RECT rect = *lpRect;
		::InflateRect(&rect, -size.cx, -size.cy);
		::IntersectRect(&rect, &rect, lpRect);
		CRgn rgnInside;
		rgnInside.CreateRectRgnIndirect(&rect);
		CRgn rgnNew;
		rgnNew.CreateRectRgn(0, 0, 0, 0);
		rgnNew.CombineRgn(rgnOutside, rgnInside, RGN_XOR);

		HBRUSH hBrushOld = NULL;
		CBrush brushHalftone;
		if (hBrush == NULL)
			brushHalftone = hBrush = CDCHandle::GetHalftoneBrush();
		if (hBrushLast == NULL)
			hBrushLast = hBrush;

		CRgn rgnLast;
		CRgn rgnUpdate;
		if (lpRectLast != NULL)
		{
			// find difference between new region and old region
			rgnLast.CreateRectRgn(0, 0, 0, 0);
			rgnOutside.SetRectRgn(lpRectLast->left, lpRectLast->top, lpRectLast->right, lpRectLast->bottom);
			rect = *lpRectLast;
			::InflateRect(&rect, -sizeLast.cx, -sizeLast.cy);
			::IntersectRect(&rect, &rect, lpRectLast);
			rgnInside.SetRectRgn(rect.left, rect.top, rect.right, rect.bottom);
			rgnLast.CombineRgn(rgnOutside, rgnInside, RGN_XOR);

			// only diff them if brushes are the same
			if (hBrush == hBrushLast)
			{
				rgnUpdate.CreateRectRgn(0, 0, 0, 0);
				rgnUpdate.CombineRgn(rgnLast, rgnNew, RGN_XOR);
			}
		}
		if ((hBrush != hBrushLast) && (lpRectLast != NULL))
		{
			// brushes are different -- erase old region first
			SelectClipRgn(rgnLast);
			GetClipBox(&rect);
			hBrushOld = SelectBrush(hBrushLast);
			PatBlt(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, PATINVERT);
			SelectBrush(hBrushOld);
			hBrushOld = NULL;
		}

		// draw into the update/new region
		SelectClipRgn(rgnUpdate.IsNull() ? rgnNew : rgnUpdate);
		GetClipBox(&rect);
		hBrushOld = SelectBrush(hBrush);
		PatBlt(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, PATINVERT);

		// cleanup DC
		if (hBrushOld != NULL)
			SelectBrush(hBrushOld);
		SelectClipRgn(NULL);
	}

	void FillSolidRect(LPCRECT lpRect, COLORREF clr)
	{
		SASSERT(m_hDC != NULL);

		COLORREF clrOld = ::SetBkColor(m_hDC, clr);
		SASSERT(clrOld != CLR_INVALID);
		if (clrOld != CLR_INVALID)
		{
			::ExtTextOut(m_hDC, 0, 0, ETO_OPAQUE, lpRect, NULL, 0, NULL);
			::SetBkColor(m_hDC, clrOld);
		}
	}

	void FillSolidRect(int x, int y, int cx, int cy, COLORREF clr)
	{
		SASSERT(m_hDC != NULL);

		RECT rect = { x, y, x + cx, y + cy };
		FillSolidRect(&rect, clr);
	}

	void Draw3dRect(LPCRECT lpRect, COLORREF clrTopLeft, COLORREF clrBottomRight)
	{
		Draw3dRect(lpRect->left, lpRect->top, lpRect->right - lpRect->left,
			lpRect->bottom - lpRect->top, clrTopLeft, clrBottomRight);
	}

	void Draw3dRect(int x, int y, int cx, int cy, COLORREF clrTopLeft, COLORREF clrBottomRight)
	{
		FillSolidRect(x, y, cx - 1, 1, clrTopLeft);
		FillSolidRect(x, y, 1, cy - 1, clrTopLeft);
		FillSolidRect(x + cx, y, -1, cy, clrBottomRight);
		FillSolidRect(x, y + cy, cx, -1, clrBottomRight);
	}

	// DIB support
	int SetDIBitsToDevice(int x, int y, DWORD dwWidth, DWORD dwHeight, int xSrc, int ySrc, UINT uStartScan, UINT cScanLines, CONST VOID* lpvBits, CONST BITMAPINFO* lpbmi, UINT uColorUse)
	{
		SASSERT(m_hDC != NULL);
		return ::SetDIBitsToDevice(m_hDC, x, y, dwWidth, dwHeight, xSrc, ySrc, uStartScan, cScanLines, lpvBits, lpbmi, uColorUse);
	}

	int StretchDIBits(int x, int y, int nWidth, int nHeight, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, CONST VOID* lpvBits, CONST BITMAPINFO* lpbmi, UINT uColorUse, DWORD dwRop)
	{
		SASSERT(m_hDC != NULL);
		return ::StretchDIBits(m_hDC, x, y, nWidth, nHeight, xSrc, ySrc, nSrcWidth, nSrcHeight, lpvBits, lpbmi, uColorUse, dwRop);
	}

	UINT GetDIBColorTable(UINT uStartIndex, UINT cEntries, RGBQUAD* pColors) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetDIBColorTable(m_hDC, uStartIndex, cEntries, pColors);
	}

	UINT SetDIBColorTable(UINT uStartIndex, UINT cEntries, CONST RGBQUAD* pColors)
	{
		SASSERT(m_hDC != NULL);
		return ::SetDIBColorTable(m_hDC, uStartIndex, cEntries, pColors);
	}

	// OpenGL support
#if !defined(_ATL_NO_OPENGL)
	int ChoosePixelFormat(CONST PIXELFORMATDESCRIPTOR* ppfd)
	{
		SASSERT(m_hDC != NULL);
		return ::ChoosePixelFormat(m_hDC, ppfd);
	}

	int DescribePixelFormat(int iPixelFormat, UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd)
	{
		SASSERT(m_hDC != NULL);
		return ::DescribePixelFormat(m_hDC, iPixelFormat, nBytes, ppfd);
	}

	int GetPixelFormat() const
	{
		SASSERT(m_hDC != NULL);
		return ::GetPixelFormat(m_hDC);
	}

	BOOL SetPixelFormat(int iPixelFormat, CONST PIXELFORMATDESCRIPTOR* ppfd)
	{
		SASSERT(m_hDC != NULL);
		return ::SetPixelFormat(m_hDC, iPixelFormat, ppfd);
	}

	BOOL SwapBuffers()
	{
		SASSERT(m_hDC != NULL);
		return ::SwapBuffers(m_hDC);
	}

	HGLRC wglCreateContext()
	{
		SASSERT(m_hDC != NULL);
		return ::wglCreateContext(m_hDC);
	}

	HGLRC wglCreateLayerContext(int iLayerPlane)
	{
		SASSERT(m_hDC != NULL);
		return ::wglCreateLayerContext(m_hDC, iLayerPlane);
	}

	BOOL wglMakeCurrent(HGLRC hglrc)
	{
		SASSERT(m_hDC != NULL);
		return ::wglMakeCurrent(m_hDC, hglrc);
	}

	BOOL wglUseFontBitmaps(DWORD dwFirst, DWORD dwCount, DWORD listBase)
	{
		SASSERT(m_hDC != NULL);
		return ::wglUseFontBitmaps(m_hDC, dwFirst, dwCount, listBase);
	}

	BOOL wglUseFontOutlines(DWORD dwFirst, DWORD dwCount, DWORD listBase, FLOAT deviation, FLOAT extrusion, int format, LPGLYPHMETRICSFLOAT lpgmf)
	{
		SASSERT(m_hDC != NULL);
		return ::wglUseFontOutlines(m_hDC, dwFirst, dwCount, listBase, deviation, extrusion, format, lpgmf);
	}

	BOOL wglDescribeLayerPlane(int iPixelFormat, int iLayerPlane, UINT nBytes, LPLAYERPLANEDESCRIPTOR plpd)
	{
		SASSERT(m_hDC != NULL);
		return ::wglDescribeLayerPlane(m_hDC, iPixelFormat, iLayerPlane, nBytes, plpd);
	}

	int wglSetLayerPaletteEntries(int iLayerPlane, int iStart, int cEntries, CONST COLORREF* pclr)
	{
		SASSERT(m_hDC != NULL);
		return ::wglSetLayerPaletteEntries(m_hDC, iLayerPlane, iStart, cEntries, pclr);
	}

	int wglGetLayerPaletteEntries(int iLayerPlane, int iStart, int cEntries, COLORREF* pclr)
	{
		SASSERT(m_hDC != NULL);
		return ::wglGetLayerPaletteEntries(m_hDC, iLayerPlane, iStart, cEntries, pclr);
	}

	BOOL wglRealizeLayerPalette(int iLayerPlane, BOOL bRealize)
	{
		SASSERT(m_hDC != NULL);
		return ::wglRealizeLayerPalette(m_hDC, iLayerPlane, bRealize);
	}

	BOOL wglSwapLayerBuffers(UINT uPlanes)
	{
		SASSERT(m_hDC != NULL);
		return ::wglSwapLayerBuffers(m_hDC, uPlanes);
	}
#endif // !defined(_ATL_NO_OPENGL)

	COLORREF GetDCPenColor() const
	{
		SASSERT(m_hDC != NULL);
		return ::GetDCPenColor(m_hDC);
	}

	COLORREF SetDCPenColor(COLORREF clr)
	{
		SASSERT(m_hDC != NULL);
		return ::SetDCPenColor(m_hDC, clr);
	}

	COLORREF GetDCBrushColor() const
	{
		SASSERT(m_hDC != NULL);
		return ::GetDCBrushColor(m_hDC);
	}

	COLORREF SetDCBrushColor(COLORREF clr)
	{
		SASSERT(m_hDC != NULL);
		return ::SetDCBrushColor(m_hDC, clr);
	}

	DWORD GetFontUnicodeRanges(LPGLYPHSET lpgs) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetFontUnicodeRanges(m_hDC, lpgs);
	}

	DWORD GetGlyphIndices(LPCTSTR lpstr, int cch, LPWORD pgi, DWORD dwFlags) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetGlyphIndices(m_hDC, lpstr, cch, pgi, dwFlags);
	}

	BOOL GetTextExtentPointI(LPWORD pgiIn, int cgi, LPSIZE lpSize) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetTextExtentPointI(m_hDC, pgiIn, cgi, lpSize);
	}

	BOOL GetTextExtentExPointI(LPWORD pgiIn, int cgi, int nMaxExtent, LPINT lpnFit, LPINT alpDx, LPSIZE lpSize) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetTextExtentExPointI(m_hDC, pgiIn, cgi, nMaxExtent, lpnFit, alpDx, lpSize);
	}

	BOOL GetCharWidthI(UINT giFirst, UINT cgi, LPWORD pgi, LPINT lpBuffer) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetCharWidthI(m_hDC, giFirst, cgi, pgi, lpBuffer);
	}

	BOOL GetCharABCWidthsI(UINT giFirst, UINT cgi, LPWORD pgi, LPABC lpabc) const
	{
		SASSERT(m_hDC != NULL);
		return ::GetCharABCWidthsI(m_hDC, giFirst, cgi, pgi, lpabc);
	}

	BOOL ColorCorrectPalette(HPALETTE hPalette, DWORD dwFirstEntry, DWORD dwNumOfEntries)
	{
		SASSERT(m_hDC != NULL);
		return ::ColorCorrectPalette(m_hDC, hPalette, dwFirstEntry, dwNumOfEntries);
	}
};

typedef CDCT<false>   CDCHandle;
typedef CDCT<true>    CDC;