#include "stdafx.h"
#include "SImgCanvas.h"
#include "SaveToIcon.h"

namespace SOUI
{
    SImgCanvas::SImgCanvas(void):m_bVert(FALSE)
    {
    }

    SImgCanvas::~SImgCanvas(void)
    {
    }

    void SImgCanvas::OnPaint(IRenderTarget *pRT)
    {
        if(m_lstImg.IsEmpty())
        {
            SWindow::OnPaint(pRT);
        }else
        {
            IBitmap * pBmp = m_lstImg.GetHead();
            
            CRect rcClient = GetClientRect();
            CSize szBmp(pBmp->Size());
            CRect rcAll = rcClient;
            CSize szAll = szBmp;

            CPoint ptOffset;
            if(m_bVert) 
            {
                szAll.cy *= m_lstImg.GetCount();
				szAll.cy += m_lstImg.GetCount()-1;//interval
                ptOffset.y=szBmp.cy+1;
            }
            else 
            {
                szAll.cx *= m_lstImg.GetCount();
				szAll.cx += m_lstImg.GetCount()-1;//interval
                ptOffset.x = szBmp.cx+1;
            }
            
            rcAll.DeflateRect((rcClient.Size()-szAll)/2);
            CRect rcBmp(rcAll.TopLeft(),szBmp);
            
			SAutoRefPtr<IPen> pen,oldpen;
			pRT->CreatePen(PS_DASHDOT,RGBA(255,0,255,128),1,&pen);
			pRT->SelectObject(pen,(IRenderObj**)&oldpen);
            
            SPOSITION pos = m_lstImg.GetHeadPosition();
            while(pos)
            {
                pBmp = m_lstImg.GetNext(pos);
                pRT->DrawBitmap(rcBmp,pBmp,0,0);
				if(m_bVert)
				{//draw horz line
					POINT pt[2]={rcBmp.left,rcBmp.bottom,rcBmp.right,rcBmp.bottom};
					pRT->DrawLines(pt,2);
				}else
				{
					POINT pt[2]={rcBmp.right,rcBmp.top,rcBmp.right,rcBmp.bottom};
					pRT->DrawLines(pt,2);
				}
                rcBmp.OffsetRect(ptOffset);
            }
            pRT->DrawRectangle(rcAll);
            pRT->SelectObject(oldpen);
        }
    }

	void SImgCanvas::OnRButtonDown(UINT nFlags, CPoint point)
	{
		__baseCls::OnRButtonDown(nFlags, point);
		if (m_lstImg.IsEmpty())
			return;
		IBitmap* pBmp = m_lstImg.GetHead();

		CRect rcClient = GetClientRect();
		CSize szBmp(pBmp->Size());
		CRect rcAll = rcClient;
		CSize szAll = szBmp;

		CPoint ptOffset;
		if (m_bVert)
		{
			szAll.cy *= m_lstImg.GetCount();
			szAll.cy += m_lstImg.GetCount() - 1;//interval
			ptOffset.y = szBmp.cy + 1;
		}
		else
		{
			szAll.cx *= m_lstImg.GetCount();
			szAll.cx += m_lstImg.GetCount() - 1;//interval
			ptOffset.x = szBmp.cx + 1;
		}

		rcAll.DeflateRect((rcClient.Size() - szAll) / 2);
		if (rcAll.PtInRect(point)) {
			point -= rcAll.TopLeft();
			int iIcon = -1;
			if (m_bVert) {
				iIcon = (point.y-1) / (szBmp.cy+1);
			}
			else
				iIcon = (point.x-1) / (szBmp.cx+1);
			SASSERT(iIcon >= 0 && iIcon < m_lstImg.GetCount());
			SPOSITION pos = m_lstImg.FindIndex(iIcon);
			if (pos) {
				IBitmapPtr bmp = m_lstImg.GetAt(pos);
				m_lstImg.RemoveAt(pos);
				bmp->Release();
			}
			Invalidate();
		}
	}

    BOOL SImgCanvas::AddFile(LPCWSTR pszFileName)
    {
        IBitmap *pImg=SResLoadFromFile::LoadImage(S_CW2T(pszFileName));
        if(!pImg) return FALSE;
        m_lstImg.AddTail(pImg);
        Invalidate();
        return TRUE;
    }

    void SImgCanvas::Clear()
    {
        SPOSITION pos = m_lstImg.GetHeadPosition();
        while(pos)
        {
            IBitmap *pbmp = m_lstImg.GetNext(pos);
            pbmp->Release();
        }
        m_lstImg.RemoveAll();
        Invalidate();
    }

	BOOL SImgCanvas::Save2IconFile(LPCWSTR pszFileName)
	{
		CRGBA2ICON iconfile;
		if (m_size.GetCount() && m_lstImg.GetCount() == 1)
		{
			
			SPOSITION pos = m_size.GetHeadPosition();			
			while (pos)
			{
				IBitmap *_bitmap=NULL;				
				int szBmp = m_size.GetNext(pos);
				m_lstImg.GetHead()->Scale2(&_bitmap,szBmp, szBmp, kHigh_FilterLevel);
				iconfile.AddBitmapToIco(_bitmap);
			}
		}
		else
		{
			SPOSITION pos = m_lstImg.GetHeadPosition();
			while (pos)
			{
				IBitmap* pBmp = m_lstImg.GetNext(pos);
				iconfile.AddBitmapToIco(pBmp);
			}
		}
		return iconfile.SaveIconFile(S_CW2T(pszFileName));
	}

	void SImgCanvas::PushIconSize(int size)
	{
		if (m_size.Find(size))
			return;
		m_size.AddTail(size);
	}

    BOOL SImgCanvas::Save2File(LPCWSTR pszFileName)
    {
        IBitmap *pBmp = m_lstImg.GetHead();
        CSize szBmp = pBmp->Size();
        if(m_bVert) szBmp.cy *= m_lstImg.GetCount();
        else szBmp.cx *= m_lstImg.GetCount();
        
        SAutoRefPtr<IRenderTarget> pMemRT;
        GETRENDERFACTORY->CreateRenderTarget(&pMemRT,szBmp.cx,szBmp.cy);
        
        CRect rcDst(CPoint(),pBmp->Size());
        
        SPOSITION pos = m_lstImg.GetHeadPosition();
        while(pos)
        {
            pBmp = m_lstImg.GetNext(pos);
            pMemRT->DrawBitmap(rcDst,pBmp,0,0);
            if(m_bVert) rcDst.OffsetRect(0,rcDst.Height());
            else rcDst.OffsetRect(rcDst.Width(),0);
        }
        
        IBitmap * pCache = (IBitmap*)pMemRT->GetCurrentObject(OT_BITMAP);
        return pCache->Save2(pszFileName, Img_PNG);
    }

    void SImgCanvas::SetVertical(BOOL bVert)
    {
        m_bVert = bVert;
        Invalidate();
    }

	void SImgCanvas::Split(int nSplit)
	{
		IBitmap *pBmp = m_lstImg.GetHead();
		CSize szBmp = pBmp->Size();
		CSize szImg = szBmp;
		if(m_bVert)
		{
			szBmp.cy *= m_lstImg.GetCount();
		}else
		{
			szBmp.cx *= m_lstImg.GetCount();
		}
		SAutoRefPtr<IRenderTarget> pMemRT;

		GETRENDERFACTORY->CreateRenderTarget(&pMemRT,szBmp.cx,szBmp.cy);
		{//render img
			SPOSITION pos = m_lstImg.GetHeadPosition();
			CRect rcDst(CPoint(),szImg);
			while(pos)
			{
				IBitmap *pImg = m_lstImg.GetNext(pos);
				pMemRT->DrawBitmap(rcDst,pImg,0,0);
				if(m_bVert)
					rcDst.OffsetRect(0,szImg.cy);
				else
					rcDst.OffsetRect(szImg.cx,0);
				pImg->Release();
			}
			m_lstImg.RemoveAll();
		}


		CSize szSub = szBmp;
		if(m_bVert) szSub.cy/=nSplit;
		else szSub.cx/=nSplit;


		{
			CRect rcDst(CPoint(),szSub),rcSrc=rcDst;
			for(int i=0;i< nSplit; i++)
			{
				SAutoRefPtr<IRenderTarget> pMemRT2;
				GETRENDERFACTORY->CreateRenderTarget(&pMemRT2,szSub.cx,szSub.cy);
				pMemRT2->ClearRect(&rcDst,0);
				pMemRT2->AlphaBlend(rcDst,pMemRT,rcSrc,255);
				IBitmap *pCache = (IBitmap*)pMemRT2->GetCurrentObject(OT_BITMAP);
				m_lstImg.AddTail(pCache);
				pCache->AddRef();
				if(m_bVert) rcSrc.OffsetRect(0,szSub.cy);
				else rcSrc.OffsetRect(szSub.cx,0);
			}
		}
		Invalidate();
	}

	void SImgCanvas::SaveSplits(LPCWSTR pszFileName)
	{
		int i=0;
		SPOSITION pos = m_lstImg.GetHeadPosition();
		while(pos)
		{
			IBitmap *pBmp = m_lstImg.GetNext(pos);
			SStringW strName = pszFileName;
			strName += SStringW().Format(L"_%d.png",i++);
            pBmp->Save2(strName, Img_PNG);
		}
	}
}
