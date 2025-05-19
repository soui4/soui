#include "stdafx.h"
#include "SImgCanvas.h"


namespace SOUI
{
	SImgCanvas::SImgCanvas(void) :m_bVert(FALSE)
	{
	}

	SImgCanvas::~SImgCanvas(void)
	{
	}

	void SImgCanvas::OnPaint(IRenderTarget *pRT)
	{
		if (m_lstImg.IsEmpty())
		{
			SWindow::OnPaint(pRT);
		}
		else
		{
			IBitmapS * pBmp = m_lstImg.GetHead();

			CRect rcClient = GetClientRect();
			CSize szBmp(pBmp->Size());
			CRect rcAll = rcClient;
			CSize szAll = szBmp;

			CPoint ptOffset;
			if (m_bVert)
			{
				szAll.cy *= m_lstImg.GetCount();
				ptOffset.y = szBmp.cy;
			}
			else
			{
				szAll.cx *= m_lstImg.GetCount();
				ptOffset.x = szBmp.cx;
			}

			rcAll.DeflateRect((rcClient.Size() - szAll) / 2);
			CRect rcBmp(rcAll.TopLeft(), szBmp);


			SPOSITION pos = m_lstImg.GetHeadPosition();
			while (pos)
			{
				pBmp = m_lstImg.GetNext(pos);
				pRT->DrawBitmap(rcBmp, pBmp, 0, 0,0xFF);
				rcBmp.OffsetRect(ptOffset);
			}
			SAutoRefPtr<IPenS> pen, oldpen;
			pRT->CreatePen(PS_DASHDOT, RGBA(0, 0, 0, 128), 1, &pen);
			pRT->SelectObject(pen, (IRenderObj**)&oldpen);
			pRT->DrawRectangle(rcAll);
			pRT->SelectObject(oldpen,NULL);
		}
	}

	BOOL SImgCanvas::AddFile(LPCTSTR pszFileName)
	{
		SStringT strFileName = pszFileName;
		strFileName.Replace(_T("\\"), _T("/"));
		IBitmapS *pImg = SResLoadFromFile::LoadImage(strFileName);
		if (!pImg) return FALSE;

		m_lstImg.AddTail(pImg);
		if (GetLayoutParam()->IsWrapContent(Any) && GetParent())
		{
			//重新计算坐标
			RequestRelayout();
		}
		if (IsVisible(TRUE)) Invalidate();
		return TRUE;
	}

	CSize SImgCanvas::GetImgInfo()
	{
		if (m_lstImg.GetCount() == 0)
			return CSize(0, 0);

		IBitmapS * pBmp = m_lstImg.GetHead();
		return CSize(pBmp->Size());
	}

	CSize SImgCanvas::GetDesiredSize(LPCRECT pRcContainer)
	{
		CSize szRet;
		if (m_lstImg.GetCount() > 0) szRet = m_lstImg.GetHead()->Size();

		return szRet;
	}

	void SImgCanvas::Clear()
	{
		SPOSITION pos = m_lstImg.GetHeadPosition();
		while (pos)
		{
			IBitmapS *pbmp = m_lstImg.GetNext(pos);
			pbmp->Release();
		}
		m_lstImg.RemoveAll();
		Invalidate();
	}

	BOOL SImgCanvas::Save2File(LPCWSTR pszFileName, int nSplit)
	{
		SStringW strDesc = GETRENDERFACTORY->GetImgDecoderFactory()->GetDescription();
		if (strDesc != L"gdi+" || m_lstImg.IsEmpty())
			return FALSE;

		IBitmapS *pBmp = m_lstImg.GetHead();
		CSize szBmp = pBmp->Size();
		if (m_bVert) szBmp.cy *= m_lstImg.GetCount();
		else szBmp.cx *= m_lstImg.GetCount();

		CAutoRefPtr<IRenderTarget> pMemRT;
		GETRENDERFACTORY->CreateRenderTarget(&pMemRT, szBmp.cx, szBmp.cy);

		CRect rcDst(CPoint(), pBmp->Size());

		SPOSITION pos = m_lstImg.GetHeadPosition();
		while (pos)
		{
			pBmp = m_lstImg.GetNext(pos);
			pMemRT->DrawBitmap(rcDst, pBmp, 0, 0,0xff);
			if (m_bVert) rcDst.OffsetRect(0, rcDst.Height());
			else rcDst.OffsetRect(rcDst.Width(), 0);
		}

		IBitmapS * pCache = (IBitmapS*)pMemRT->GetCurrentObject(OT_BITMAP);
		if (nSplit == 1)
		{
			return pCache->Save2(pszFileName, Img_PNG);
		}
		else
		{
			CAutoRefPtr<IRenderTarget> pMemRT2;
			CSize szSub = szBmp;
			if (m_bVert) szSub.cy /= nSplit;
			else szSub.cx /= nSplit;

			GETRENDERFACTORY->CreateRenderTarget(&pMemRT2, szSub.cx, szSub.cy);

			SStringW strSaveName = pszFileName;
			int nDot = strSaveName.ReverseFind(L'.');
			if (nDot != -1) strSaveName = strSaveName.Left(nDot);

			CRect rcDst(CPoint(), szSub), rcSrc = rcDst;
			for (int i = 0; i < nSplit; i++)
			{
				pMemRT2->BitBlt(rcDst, pMemRT, rcSrc.left, rcSrc.top,0xFF);
				IBitmapS *pCacheSub = (IBitmapS*)pMemRT2->GetCurrentObject(OT_BITMAP);
				pCacheSub->Save2(SStringW().Format(L"%s_%d.png", strSaveName.c_str(), i + 1),Img_PNG);
				pMemRT2->ClearRect(rcDst, 0);
				if (m_bVert) rcSrc.OffsetRect(0, rcSrc.Height());
				else rcSrc.OffsetRect(rcSrc.Width(), 0);
			}
			return TRUE;
		}
	}

	void SImgCanvas::SetVertical(BOOL bVert)
	{
		m_bVert = bVert;
		Invalidate();
	}

}
