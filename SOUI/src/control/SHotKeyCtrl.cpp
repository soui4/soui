#include "souistd.h"
#include "control/SHotKeyCtrl.h"

SNSBEGIN

SHotKeyCtrl::SHotKeyCtrl(void)
{
    m_wInvalidModifier = Mod_None;
    m_wInvalidComb = HKCOMB_NONE;
    m_bInSetting = FALSE;
    m_bFocusable = TRUE;
    m_evtSet.addEvent(EVENTID(EventSetHotKey));
}

SHotKeyCtrl::~SHotKeyCtrl(void)
{
}

void SHotKeyCtrl::OnLButtonDown(UINT nFlags, CPoint pt)
{
    __baseCls::OnLButtonDown(nFlags, pt);
    GetContainer()->OnSetSwndFocus(m_swnd);
}

void SHotKeyCtrl::OnPaint(IRenderTarget *pRT)
{
    SPainter painter;
    BeforePaint(pRT, painter);
    CRect rcClient;
    GetTextRect(&rcClient);
    SStringT str = GetWindowText();
    UINT uAlign = GetTextAlign();
    pRT->DrawText(str, str.GetLength(), &rcClient, uAlign);
    AfterPaint(pRT, painter);
}

void SHotKeyCtrl::UpdateCaret(IRenderTarget *pRT)
{
    SStringT str = GetWindowText();
    SIZE szTxt;
    pRT->MeasureText(str, str.GetLength(), &szTxt);

    CRect rcClient;
    GetTextRect(&rcClient);

    UINT uAlign = GetTextAlign();
    int x = rcClient.left + szTxt.cx;
    int y = rcClient.top + (rcClient.Height() - szTxt.cy) / 2;
    if (uAlign & DT_CENTER)
        x += (rcClient.Width() - szTxt.cx) / 2;
    else if (uAlign & DT_RIGHT)
        x = rcClient.right;

    SetCaretPos(x, y);
}

UINT SHotKeyCtrl::GetTextAlign()
{
    UINT uAlign = SWindow::GetTextAlign();
    uAlign &= ~DT_BOTTOM;
    uAlign |= DT_VCENTER | DT_SINGLELINE;
    return uAlign;
}

void SHotKeyCtrl::OnSetFocus(SWND wndOld)
{
    IRenderTarget *pRT = GetRenderTarget(NULL, GRT_NODRAW);
    BeforePaintEx(pRT);
    SIZE szTxt;
    pRT->MeasureText(_T("A"), 1, &szTxt);

    CreateCaret(NULL, 1, szTxt.cy);

    UpdateCaret(pRT);

    ReleaseRenderTarget(pRT);

    GetContainer()->EnableIME(FALSE);
    ShowCaret(TRUE);

    __baseCls::OnSetFocus(wndOld);
}

void SHotKeyCtrl::OnKillFocus(SWND wndFocus)
{
    ShowCaret(FALSE);
    GetContainer()->EnableIME(TRUE);
    __baseCls::OnKillFocus(wndFocus);
}

void SHotKeyCtrl::UpdateModifier()
{
    BOOL bAlt = GetKeyState(VK_MENU) & 0x8000;
    BOOL bCtrl = GetKeyState(VK_CONTROL) & 0x8000;
    BOOL bShift = GetKeyState(VK_SHIFT) & 0x8000;

    WORD wCombKey = 0;
    WORD wCombMask = HKCOMB_NONE;
    if (!bAlt && !bCtrl && !bShift)
        wCombKey = Mod_None;
    else if (bAlt && !bCtrl && !bShift)
        wCombKey = Mod_Alt, wCombMask = HKCOMB_A;
    else if (!bAlt && bCtrl && !bShift)
        wCombKey = Mod_Ctrl, wCombMask = HKCOMB_C;
    else if (!bAlt && !bCtrl && bShift)
        wCombKey = Mod_Shift, wCombMask = HKCOMB_S;
    else if (bAlt && bCtrl && !bShift)
        wCombKey = Mod_CA, wCombMask = HKCOMB_CA;
    else if (bAlt && !bCtrl && bShift)
        wCombKey = Mod_SA, wCombMask = HKCOMB_SA;
    else if (!bAlt && bCtrl && bShift)
        wCombKey = Mod_SC, wCombMask = HKCOMB_SC;
    else
        wCombKey = Mod_SCA, wCombMask = HKCOMB_SCA;
    if (wCombMask & m_wInvalidComb)
        m_wModifier = m_wInvalidModifier;
    else
        m_wModifier = wCombKey;

    EventSetHotKey ev(this);
    ev.wModifiers = m_wModifier;
    ev.vKey = m_wVK;
    FireEvent(ev);
}

void SHotKeyCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (!m_bInSetting)
    {
        m_bInSetting = TRUE;
        m_wVK = 0;
        m_wModifier = m_wInvalidModifier;
    }
    SStringT strKey = GetKeyName(nChar);
    if (!strKey.IsEmpty())
    {
        m_wVK = nChar;
    }
    UpdateModifier();
    IRenderTarget *pRT = GetRenderTarget(NULL, GRT_NODRAW);
    BeforePaintEx(pRT);
    UpdateCaret(pRT);
    ReleaseRenderTarget(pRT);
    Invalidate();
}

void SHotKeyCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (!m_bInSetting)
        return;

    IRenderTarget *pRT = GetRenderTarget(NULL, GRT_NODRAW);
    BeforePaintEx(pRT);

    if (nChar == m_wVK)
    {
        m_bInSetting = FALSE;
    }
    else if (m_wVK == 0 && (GetKeyState(VK_SHIFT) & 0x8000) == 0 && (GetKeyState(VK_MENU) & 0x8000) == 0 && (GetKeyState(VK_CONTROL) & 0x8000) == 0)
    {
        m_bInSetting = FALSE;
        UpdateModifier();
        UpdateCaret(pRT);
        Invalidate();
    }
    else if (nChar == VK_SHIFT || nChar == VK_MENU || nChar == VK_CONTROL)
    {
        UpdateModifier();
        UpdateCaret(pRT);
        Invalidate();
    }
    ReleaseRenderTarget(pRT);
}

void SHotKeyCtrl::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (GetKeyState(VK_MENU) & 0x8000 || nChar == VK_F10)
        OnKeyDown(nChar, nRepCnt, nFlags);
    else
        SetMsgHandled(FALSE);
}

void SHotKeyCtrl::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_MENU || GetKeyState(VK_MENU) & 0x8000 || nChar == VK_F10)
        OnKeyUp(nChar, nRepCnt, nFlags);
    else
        SetMsgHandled(FALSE);
}

void SHotKeyCtrl::SetRule(WORD wInvalidComp, WORD wModifier)
{
    m_wInvalidComb = wInvalidComp;
    m_wInvalidModifier = wModifier;
}

void SHotKeyCtrl::SetHotKey(WORD vKey, WORD wModifiers)
{
    m_wVK = vKey;
    m_wModifier = wModifiers;
    IRenderTarget *pRT = GetRenderTarget(NULL, GRT_NODRAW);
    BeforePaintEx(pRT);
    UpdateCaret(pRT);
    ReleaseRenderTarget(pRT);
    Invalidate();
}

void SHotKeyCtrl::GetHotKey(WORD *wKey, WORD *wModifers) const
{
    if (wKey)
        *wKey = m_wVK;
    if (wModifers)
        *wModifers = m_wModifier;
}

SStringT SHotKeyCtrl::GetWindowText(BOOL bRawText)
{
    return FormatHotkey();
}

HRESULT SHotKeyCtrl::OnAttrInvalidComb(const SStringW &value, BOOL bLoading)
{
    SStringWList arrComb;
    SplitString(value, L'|', arrComb);
    for (UINT i = 0; i < arrComb.GetCount(); i++)
    {
        DWORD dwKey = TranslateAccelKey(S_CW2T(arrComb[i]));
        switch (HIWORD(dwKey))
        {
        case Mod_Alt:
            m_wInvalidComb |= HKCOMB_A;
            break;
        case Mod_Ctrl:
            m_wInvalidComb |= HKCOMB_C;
            break;
        case Mod_Shift:
            m_wInvalidComb |= HKCOMB_S;
            break;
        case Mod_CA:
            m_wInvalidComb |= HKCOMB_CA;
            break;
        case Mod_SA:
            m_wInvalidComb |= HKCOMB_SA;
            break;
        case Mod_SC:
            m_wInvalidComb |= HKCOMB_SC;
            break;
        case Mod_SCA:
            m_wInvalidComb |= HKCOMB_SCA;
            break;
        }
    }
    return bLoading ? S_OK : S_FALSE;
}

HRESULT SHotKeyCtrl::OnAttrInvalidSysKey(const SStringW &value, BOOL bLoading)
{
    SStringWList arrComb;
    SplitString(value, L'|', arrComb);
    for (UINT i = 0; i < arrComb.GetCount(); i++)
    {
        DWORD dwKey = TranslateAccelKey(S_CW2T(arrComb[i]));
        switch (HIWORD(dwKey))
        {
        case Mod_Alt:
            m_wInvalidComb = HKCOMB_A | HKCOMB_CA | HKCOMB_SA | HKCOMB_SCA;
            break;
        case Mod_Ctrl:
            m_wInvalidComb = HKCOMB_C | HKCOMB_CA | HKCOMB_SC | HKCOMB_SCA;
            break;
        case Mod_Shift:
            m_wInvalidComb = HKCOMB_S | HKCOMB_SA | HKCOMB_SC | HKCOMB_SCA;
            break;
        }
    }
    return bLoading ? S_OK : S_FALSE;
}

HRESULT SHotKeyCtrl::OnAttrInvalidModifier(const SStringW &value, BOOL bLoading)
{
    DWORD dwKey = TranslateAccelKey(S_CW2T(value));
    m_wInvalidModifier = HIWORD(dwKey);
    return S_FALSE;
}

HRESULT SHotKeyCtrl::OnAttrHotKey(const SStringW &value, BOOL bLoading)
{
    DWORD dwKey = TranslateAccelKey(S_CW2T(value));
    m_wModifier = HIWORD(dwKey);
    m_wVK = LOWORD(dwKey);
    return bLoading ? S_OK : S_FALSE;
}

SNSEND
