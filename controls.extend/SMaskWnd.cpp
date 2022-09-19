#include "stdafx.h"
#include "SMaskWnd.h"

namespace SOUI
{
	SMaskWnd::SMaskWnd(void)
	{
		m_bLayeredWindow = TRUE;
	}

	SMaskWnd::~SMaskWnd(void)
	{
	}

	void SMaskWnd::OnCommitSurface(IRenderTarget *pRtDest, LPCRECT pRcDest, IRenderTarget *pRtSrc, LPCRECT pRcSrc, BYTE alpha)
	{
		if (m_bmpMaskTemp) {
			SAutoRefPtr<IRenderTarget> pRT;
			CRect rcWnd = GetWindowRect();
			GETRENDERFACTORY->CreateRenderTarget(&pRT, rcWnd.Width(), rcWnd.Height());
			pRT->OffsetViewportOrg(-rcWnd.left, -rcWnd.top);
			pRT->ClearRect(&rcWnd, 0);
			pRT->BitBlt(&rcWnd, pRtDest, pRcDest->left, pRcDest->top, SRCCOPY);
			pRT->AlphaBlend(&rcWnd, pRtSrc, pRcSrc, alpha);
			IBitmapS * pBmp = (IBitmapS*)pRT->GetCurrentObject(OT_BITMAP);
			MakeCacheApha(pBmp);
			pRT->SetXfermode(kSrcIn_Mode, NULL);
			pRT->AlphaBlend(&rcWnd, pRtSrc, &rcWnd, 255);
			pRtDest->AlphaBlend(pRcDest, pRT, &rcWnd, alpha);
		}
		else {
			__baseCls::OnCommitSurface(pRtDest, pRcDest, pRtSrc, pRcSrc, alpha);
		}
	}

	HRESULT SMaskWnd::OnAttrMask(const SStringW &strValue, BOOL bLoading)
	{
		SStringW strChannel = strValue.Right(2);
		m_iMaskChannel = -1;
		if (strChannel == L".a")
			m_iMaskChannel = 3;
		else if (strChannel == L".r")
			m_iMaskChannel = 2;
		else if (strChannel == L".g")
			m_iMaskChannel = 1;
		else if (strChannel == L".b")
			m_iMaskChannel = 0;

		IBitmapS *pImg = NULL;
		if (m_iMaskChannel == -1)
		{ // use alpha channel as default
			m_iMaskChannel = 2;
			pImg = LOADIMAGE2(strValue);
		}
		else
		{
			pImg = LOADIMAGE2(strValue.Left(strValue.GetLength() - 2));
		}
		if (!pImg)
		{
			return E_FAIL;
		}
		m_bmpMask = pImg;
		pImg->Release();
		MakeMaskTemp();
		return S_OK;
	}

	void SMaskWnd::MakeCacheApha(IBitmapS* cache)
	{
		//从mask的指定channel中获得alpha通道
		LPBYTE pBitCache = (LPBYTE)cache->LockPixelBits();
		LPBYTE pBitMask = (LPBYTE)m_bmpMaskTemp->LockPixelBits();
		LPBYTE pDst = pBitCache;
		LPBYTE pSrc = pBitMask + m_iMaskChannel;
		int nPixels = cache->Width() * cache->Height();
		for (int i = 0; i < nPixels; i++)
		{
			BYTE byAlpha = *pSrc;
			pSrc += 4;
			//源半透明，mask不透明时使用源的半透明属性
			if (pDst[3] == 0xff || (pDst[3] != 0xFF && byAlpha == 0))
			{                                       //源不透明,或者mask全透明
				*pDst++ = ((*pDst) * byAlpha) >> 8; //做premultiply
				*pDst++ = ((*pDst) * byAlpha) >> 8; //做premultiply
				*pDst++ = ((*pDst) * byAlpha) >> 8; //做premultiply
				*pDst++ = byAlpha;
			}
			else
			{
				*pDst += 4;
			}
		}
		cache->UnlockPixelBits(pBitCache);
		m_bmpMaskTemp->UnlockPixelBits(pBitMask);
	}

	void SMaskWnd::MakeMaskTemp()
	{
		if (m_bmpMask) {
			CRect rcWnd = GetWindowRect();
			CRect rccache(CPoint(0, 0), rcWnd.Size());
			m_bmpMaskTemp = nullptr;
			GETRENDERFACTORY->CreateBitmap(&m_bmpMaskTemp);
			m_bmpMaskTemp->Init(rccache.Width(), rccache.Height(), NULL);
			SAutoRefPtr<IRenderTarget> pRT;
			GETRENDERFACTORY->CreateRenderTarget(&pRT, rccache.Width(), rccache.Height());
			SAutoRefPtr<IRenderObj> pOldBmp;
			pRT->SelectObject(m_bmpMaskTemp, &pOldBmp);
			CRect rcmask(CPoint(0, 0), m_bmpMask->Size());
			pRT->DrawBitmapEx(&rccache, m_bmpMask, &rcmask, MAKELONG(EM_STRETCH, kHigh_FilterLevel), 255);
			pRT->SelectObject(pOldBmp, NULL);
		}
	}

	void SMaskWnd::OnSize(UINT nType, CSize size)
	{
		__baseCls::OnSize(nType, size);
		MakeMaskTemp();
	}

} // namespace SOUI