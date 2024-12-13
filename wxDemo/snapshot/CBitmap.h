#pragma once
template <bool t_bManaged>
class CBitmapT
{
public:
	// Data members
	HBITMAP m_hBitmap;

	// Constructor/destructor/operators
	CBitmapT(HBITMAP hBitmap = NULL) : m_hBitmap(hBitmap)
	{ }

	~CBitmapT()
	{
		if (t_bManaged && (m_hBitmap != NULL))
			DeleteObject();
	}

	CBitmapT<t_bManaged>& operator =(HBITMAP hBitmap)
	{
		Attach(hBitmap);
		return *this;
	}

	void Attach(HBITMAP hBitmap)
	{
		if (t_bManaged && (m_hBitmap != NULL) && (m_hBitmap != hBitmap))
			::DeleteObject(m_hBitmap);
		m_hBitmap = hBitmap;
	}

	HBITMAP Detach()
	{
		HBITMAP hBitmap = m_hBitmap;
		m_hBitmap = NULL;
		return hBitmap;
	}

	operator HBITMAP() const { return m_hBitmap; }

	bool IsNull() const { return (m_hBitmap == NULL); }


	HBITMAP LoadOEMBitmap(UINT nIDBitmap) // for OBM_/OCR_/OIC_
	{
		SASSERT(m_hBitmap == NULL);
		m_hBitmap = ::LoadBitmap(NULL, MAKEINTRESOURCE(nIDBitmap));
		return m_hBitmap;
	}

	HBITMAP CreateBitmap(int nWidth, int nHeight, UINT nPlanes, UINT nBitsPerPixel, const void* lpBits)
	{
		SASSERT(m_hBitmap == NULL);
		m_hBitmap = ::CreateBitmap(nWidth, nHeight, nPlanes, nBitsPerPixel, lpBits);
		return m_hBitmap;
	}

	HBITMAP CreateBitmapIndirect(LPBITMAP lpBitmap)
	{
		SASSERT(m_hBitmap == NULL);
		m_hBitmap = ::CreateBitmapIndirect(lpBitmap);
		return m_hBitmap;
	}

	HBITMAP CreateCompatibleBitmap(HDC hDC, int nWidth, int nHeight)
	{
		SASSERT(m_hBitmap == NULL);
		m_hBitmap = ::CreateCompatibleBitmap(hDC, nWidth, nHeight);
		return m_hBitmap;
	}

	HBITMAP CreateDiscardableBitmap(HDC hDC, int nWidth, int nHeight)
	{
		SASSERT(m_hBitmap == NULL);
		m_hBitmap = ::CreateDiscardableBitmap(hDC, nWidth, nHeight);
		return m_hBitmap;
	}

	BOOL DeleteObject()
	{
		SASSERT(m_hBitmap != NULL);
		BOOL bRet = ::DeleteObject(m_hBitmap);
		if (bRet)
			m_hBitmap = NULL;
		return bRet;
	}

	// Attributes
	int GetBitmap(BITMAP* pBitMap) const
	{
		SASSERT(m_hBitmap != NULL);
		return ::GetObject(m_hBitmap, sizeof(BITMAP), pBitMap);
	}

	bool GetBitmap(BITMAP& bm) const
	{
		SASSERT(m_hBitmap != NULL);
		return (::GetObject(m_hBitmap, sizeof(BITMAP), &bm) == sizeof(BITMAP));
	}

	bool GetSize(SIZE& size) const
	{
		SASSERT(m_hBitmap != NULL);
		BITMAP bm = { 0 };
		if (!GetBitmap(&bm))
			return false;
		size.cx = bm.bmWidth;
		size.cy = bm.bmHeight;
		return true;
	}

	DWORD GetBitmapBits(DWORD dwCount, LPVOID lpBits) const
	{
		SASSERT(m_hBitmap != NULL);
		return ::GetBitmapBits(m_hBitmap, dwCount, lpBits);
	}

	DWORD SetBitmapBits(DWORD dwCount, const void* lpBits)
	{
		SASSERT(m_hBitmap != NULL);
		return ::SetBitmapBits(m_hBitmap, dwCount, lpBits);
	}

	BOOL GetBitmapDimension(LPSIZE lpSize) const
	{
		SASSERT(m_hBitmap != NULL);
		return ::GetBitmapDimensionEx(m_hBitmap, lpSize);
	}

	BOOL SetBitmapDimension(int nWidth, int nHeight, LPSIZE lpSize = NULL)
	{
		SASSERT(m_hBitmap != NULL);
		return ::SetBitmapDimensionEx(m_hBitmap, nWidth, nHeight, lpSize);
	}

	// DIB support
	HBITMAP CreateDIBitmap(HDC hDC, CONST BITMAPINFOHEADER* lpbmih, DWORD dwInit, CONST VOID* lpbInit, CONST BITMAPINFO* lpbmi, UINT uColorUse)
	{
		SASSERT(m_hBitmap == NULL);
		m_hBitmap = ::CreateDIBitmap(hDC, lpbmih, dwInit, lpbInit, lpbmi, uColorUse);
		return m_hBitmap;
	}

	HBITMAP CreateDIBSection(HDC hDC, CONST BITMAPINFO* lpbmi, UINT uColorUse, VOID** ppvBits, HANDLE hSection, DWORD dwOffset)
	{
		SASSERT(m_hBitmap == NULL);
		m_hBitmap = ::CreateDIBSection(hDC, lpbmi, uColorUse, ppvBits, hSection, dwOffset);
		return m_hBitmap;
	}

	int GetDIBits(HDC hDC, UINT uStartScan, UINT cScanLines, LPVOID lpvBits, LPBITMAPINFO lpbmi, UINT uColorUse) const
	{
		SASSERT(m_hBitmap != NULL);
		return ::GetDIBits(hDC, m_hBitmap, uStartScan, cScanLines, lpvBits, lpbmi, uColorUse);
	}

	int SetDIBits(HDC hDC, UINT uStartScan, UINT cScanLines, CONST VOID* lpvBits, CONST BITMAPINFO* lpbmi, UINT uColorUse)
	{
		SASSERT(m_hBitmap != NULL);
		return ::SetDIBits(hDC, m_hBitmap, uStartScan, cScanLines, lpvBits, lpbmi, uColorUse);
	}
};

typedef CBitmapT<false>   CBitmapHandle;
typedef CBitmapT<true>    CBitmap;