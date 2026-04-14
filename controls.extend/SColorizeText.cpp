#include "stdafx.h"
#include "SColorizeText.h"

SNSBEGIN

SColorizeText::SColorizeText(void):m_crKey(RGBA(255,0,0,255)), m_bCaseSensitive(FALSE)
	{
	}

	SColorizeText::~SColorizeText(void)
	{
	}

	void SColorizeText::DrawText(IRenderTarget *pRT,LPCTSTR pszBuf,int cchText,LPRECT pRect,UINT uFormat)
	{
		if(uFormat & DT_CALCRECT)
		{
			__baseCls::DrawText(pRT,pszBuf,cchText,pRect,uFormat);
			return;
		}

		
		if(!(uFormat&DT_SINGLELINE)) return;

		if(cchText==-1) cchText=_tcslen(pszBuf);

		COLORREF crDef = pRT->GetTextColor();
		SArray<COLORIZEINFO> lstInfo;

		int iBegin=0;
		for(int i=0;i<(int)m_lstColorizeInfo.GetCount();i++)
		{
			COLORIZEINFO info = m_lstColorizeInfo[i];
			if(info.iBegin>iBegin)
			{
				COLORIZEINFO infoDef={iBegin,info.iBegin,crDef};
				lstInfo.Add(infoDef);
			}
			lstInfo.Add(info);
			iBegin = info.iEnd;
		}

		if(iBegin<cchText)
		{
			COLORIZEINFO info={iBegin,cchText,crDef};
			lstInfo.Add(info);
		}

		int x=pRect->left;
		int y=pRect->top;
		for(int i=0;i<(int)lstInfo.GetCount();i++)
		{
			const COLORIZEINFO & info = lstInfo[i];
			pRT->SetTextColor(info.cr);
			LPCTSTR p1=pszBuf+info.iBegin;
			SIZE szSeg={0};
			pRT->MeasureText(p1,info.iEnd-info.iBegin,&szSeg);
			pRT->TextOut(x,y,p1,info.iEnd-info.iBegin);
			x += szSeg.cx;
		}

		pRT->SetTextColor(crDef);
	}

	void SColorizeText::ClearColorizeInfo()
	{
		m_lstColorizeInfo.RemoveAll();
		Invalidate();
	}

	void SColorizeText::AddColorizeInfo(int iBegin,int iEnd,COLORREF cr)
	{
		COLORIZEINFO info={iBegin,iEnd,cr};
		if(!m_lstColorizeInfo.IsEmpty())
		{//确保是按顺序设置
			if(iBegin<m_lstColorizeInfo[m_lstColorizeInfo.GetCount()-1].iEnd)
				return;
		}
		m_lstColorizeInfo.Add(info);
	}

	void SColorizeText::UpdateTextColors()
	{
		ClearColorizeInfo();
		if (m_strKey.IsEmpty())
			return;
		SStringT strText = GetWindowText();
		if(!m_bCaseSensitive)
			strText.ToLower();
		int posStart = 0;
		while (posStart < strText.GetLength()) {
			int pos = strText.Find(m_strKey, posStart);
			if (pos == -1) break;
			AddColorizeInfo(pos, pos + m_strKey.GetLength(), m_crKey);
			posStart = pos + m_strKey.GetLength();
		}
		Invalidate();
	}

	void SColorizeText::SetKeyText(LPCTSTR pszKey)
	{
		m_strKey = pszKey;
		if (!m_bCaseSensitive) {
			m_strKey.ToLower();
		}
		UpdateTextColors();
	}

	void SColorizeText::SetWindowText(LPCTSTR pszText)
	{
		__baseCls::SetWindowText(pszText);
		UpdateTextColors();
	}

SNSEND