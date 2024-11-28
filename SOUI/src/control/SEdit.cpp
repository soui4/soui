#include <souistd.h>
#include <control/SEdit.h>

SNSBEGIN


SEdit::SEdit()
: m_crCue(RGBA(0xcc, 0xcc, 0xcc, 0xff))
, m_strCue(this)
{
	m_fRich = 0;
	m_fAutoSel = TRUE;
}

void SEdit::OnKillFocus(SWND wndFocus)
{
	SRichEdit::OnKillFocus(wndFocus);
	if (!m_strCue.GetText(FALSE).IsEmpty() && GetWindowTextLength() == 0)
		Invalidate();
}

void SEdit::OnSetFocus(SWND wndOld)
{
	SRichEdit::OnSetFocus(wndOld);
	if (!m_strCue.GetText(FALSE).IsEmpty() && GetWindowTextLength() == 0)
		Invalidate();
}

UINT SEdit::GetCueTextAlign()
{
	UINT algin = SWindow::GetTextAlign();
	algin &= ~(DT_CENTER | DT_RIGHT);
	if (m_dwStyle & ES_CENTER)
		algin |= DT_CENTER;
	else if (m_dwStyle & ES_RIGHT)
		algin |= DT_RIGHT;
	return algin;
}

void SEdit::OnPaint(IRenderTarget *pRT)
{
	SRichEdit::OnPaint(pRT);
	SStringT strCue = GetCueText();
	if (!strCue.IsEmpty() && GetWindowTextLength() == 0 && !IsFocused())
	{
		SPainter painter;
		BeforePaint(pRT, painter);
		COLORREF crOld = pRT->SetTextColor(m_crCue);

		CRect rc;
		GetClientRect(&rc);
		CRect rcInsetPixel = GetStyle().GetPadding();
		rc.DeflateRect(rcInsetPixel.left, rcInsetPixel.top, rcInsetPixel.right, rcInsetPixel.bottom);
		pRT->DrawText(strCue, strCue.GetLength(), &rc, GetCueTextAlign());

		pRT->SetTextColor(crOld);
		AfterPaint(pRT, painter);
	}
}

SStringT SEdit::GetCueText(BOOL bRawText) const
{
	return m_strCue.GetText(bRawText);
}

HRESULT SEdit::OnLanguageChanged()
{
	HRESULT hr = __baseCls::OnLanguageChanged();
	m_strCue.TranslateText();
	return hr;
}

SNSEND
