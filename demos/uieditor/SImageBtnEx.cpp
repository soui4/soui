﻿#include "stdafx.h"
#include "SImageBtnEx.h"

void SOUI::SImageBtnEx::OnPaint( IRenderTarget *pRT )
{
	__baseCls::OnPaint(pRT);
	SPainter painter;

	BeforePaint(pRT, painter);

	if (m_pIcon)
	{
		SIZE sz = m_pIcon->GetSkinSize();
		CRect rc = GetWindowRect();
		rc.left += m_IconOffsetX;
		rc.top += m_IconOffsetY;
		rc.right = rc.left + sz.cx;
		rc.bottom = rc.top + sz.cy;
		m_pIcon->DrawByIndex(pRT, rc, 0);
	}

	if (!m_strText.IsEmpty())
	{
		SIZE szChar;
		pRT->MeasureText(m_strText, m_strText.GetLength(), &szChar);
		CRect chRc = GetWindowRect();
		chRc.left += (chRc.Width() - szChar.cx)/2 + m_TextOffsetX;
		chRc.top += (chRc.Height() - szChar.cy)/2 ;
		if (GetState() & WndState_PushDown) chRc.top++;
		chRc.right = chRc.left + szChar.cx;
		chRc.bottom = chRc.top + szChar.cy;
		pRT->DrawText(m_strText, m_strText.GetLength(), chRc, GetTextAlign());
	}

	AfterPaint(pRT, painter);
}