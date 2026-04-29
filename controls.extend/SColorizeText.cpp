#include "stdafx.h"
#include "SColorizeText.h"

SNSBEGIN

SColorizeText::SColorizeText(void)
    : m_crKey(RGBA(255, 0, 0, 255))
    , m_bCaseSensitive(FALSE)
{
    m_strKey.SetOwner(this);
}

SColorizeText::~SColorizeText(void)
{
}

void SColorizeText::OnDrawLine(IRenderTarget *pRT, LPCTSTR pszBuf, int iBegin, int cchText, LPRECT pRect, UINT uFormat){
    if (uFormat & DT_CALCRECT)
    {
        __baseCls::OnDrawLine(pRT, pszBuf, iBegin, cchText, pRect, uFormat);
    }else{
        COLORREF crDef = pRT->GetTextColor();
        SArray<COLORIZEINFO> lstInfo;
        int iEnd = iBegin + cchText;
        int iBegin2 = iBegin;
        for (int i = 0; i < (int)m_lstColorizeInfo.GetCount(); i++)
        {
            COLORIZEINFO info = m_lstColorizeInfo[i];
            if (info.iBegin > iBegin2)
            {
                COLORIZEINFO infoDef = { iBegin2, info.iBegin, crDef };
                lstInfo.Add(infoDef);
            }
            lstInfo.Add(info);
            iBegin2 = info.iEnd;
            if(iBegin2 >= iEnd)
                break;
        }

        if (iBegin2 < iEnd)
        {
            COLORIZEINFO info = { iBegin2, iEnd, crDef };
            lstInfo.Add(info);
        }

        int x = pRect->left;
        int y = pRect->top;
        for (int i = 0; i < (int)lstInfo.GetCount(); i++)
        {
            const COLORIZEINFO &info = lstInfo[i];
            pRT->SetTextColor(info.cr);
            LPCTSTR p1 = pszBuf + info.iBegin;
            SIZE szSeg = { 0 };
            pRT->MeasureText(p1, info.iEnd - info.iBegin, &szSeg);
            pRT->TextOut(x, y, p1, info.iEnd - info.iBegin);
            x += szSeg.cx;
        }

        pRT->SetTextColor(crDef);
    }
}
void SColorizeText::OnContentChanged()
{
    __baseCls::OnContentChanged();
    UpdateTextColors();
}

void SColorizeText::ClearColorizeInfo()
{
    m_lstColorizeInfo.RemoveAll();
    Invalidate();
}

void SColorizeText::AddColorizeInfo(int iBegin, int iEnd, COLORREF cr)
{
    COLORIZEINFO info = { iBegin, iEnd, cr };
    if (!m_lstColorizeInfo.IsEmpty())
    { // 确保是按顺序设置
        if (iBegin < m_lstColorizeInfo[m_lstColorizeInfo.GetCount() - 1].iEnd)
            return;
    }
    m_lstColorizeInfo.Add(info);
}

void SColorizeText::UpdateTextColors()
{
    ClearColorizeInfo();
    SStringT strKey = m_strKey.GetText();
    if (strKey.IsEmpty())
        return;
    SStringT strText = GetWindowText();
    if (!m_bCaseSensitive)
    {
        strText.ToLower();
        strKey.ToLower();
    }
    int posStart = 0;
    while (posStart < strText.GetLength())
    {
        int pos = strText.Find(strKey, posStart);
        if (pos == -1)
            break;
        AddColorizeInfo(pos, pos + strKey.GetLength(), m_crKey);
        posStart = pos + strKey.GetLength();
    }
    Invalidate();
}

HRESULT SColorizeText::OnAttrKey(const SStringW &strValue, BOOL bLoading)
{
    SStringW strText = GETSTRING(strValue);
    SStringT strCvt2 = S_CW2T(strText);
    m_strKey.SetText(strCvt2);
    OnContentChanged();
    return bLoading ? S_OK : S_FALSE;
}

void SColorizeText::SetKeyText(LPCTSTR pszKey)
{
    m_strKey.SetText(pszKey);
    UpdateTextColors();
}

SNSEND