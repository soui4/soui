#include "stdafx.h"
#include "SRoundImage.h"

namespace SOUI
{
	SRoundImage::SRoundImage():m_cornerSize(0)
	{
	}


	SRoundImage::~SRoundImage()
	{
	}

	SAutoRefPtr<IBitmapS> SRoundImage::GetRoundImage() const
	{
		if (!m_pSkin) return NULL;
		SAutoRefPtr<IRenderTarget> pRT; 
		CRect rc = GetClientRect();
		GETRENDERFACTORY->CreateRenderTarget(&pRT, rc.Width(), rc.Height());
		rc.MoveToXY(0, 0);
		pRT->ClearRect(&rc, 0);
		SAutoRefPtr<IBrushS> br;
		pRT->CreateSolidColorBrush(RGBA(0xBA, 0xB3, 0x99, 0xFF), &br);
		pRT->SelectObject(br,NULL);
		if(m_cornerSize==0){
			pRT->FillEllipse(&rc);
		}else{
			pRT->FillRoundRect(&rc,CPoint(m_cornerSize,m_cornerSize));
		}
		pRT->SetXfermode(kSrcIn_Mode,NULL);
		m_pSkin->DrawByIndex(pRT, rc, 0);
		return (IBitmapS*)pRT->GetCurrentObject(OT_BITMAP);
	}

	void SRoundImage::OnPaint(IRenderTarget * pRT)
	{
		if (!m_img) return;
		CRect rc = GetClientRect();
		pRT->DrawBitmap(&rc, m_img, 0, 0,0xff);
	}

	void SRoundImage::OnSize(UINT nType, CSize size)
	{
		__baseCls::OnSize(nType, size);
		m_img = GetRoundImage();
	}

	HRESULT SRoundImage::OnAttrCornerSize(const SStringW & value,BOOL bLoading)
	{
		m_cornerSize = _wtoi(value);
		if(!bLoading){
			m_img = GetRoundImage();
			return S_OK;
		}
		return S_FALSE;
	}

}
