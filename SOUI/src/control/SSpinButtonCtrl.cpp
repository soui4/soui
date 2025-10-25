﻿#include "include/souistd.h"
#include "control/SSpinButtonCtrl.h"

SNSBEGIN
SSpinButtonCtrl::SSpinButtonCtrl(void)
    : m_pDownSkin(GETBUILTINSKIN(SKIN_SYS_SPIN_DOWN))
    , m_pUpSkin(GETBUILTINSKIN(SKIN_SYS_SPIN_UP))
    , m_iActionBtn(ACTION_NULL)
    , m_nMin(0)
    , m_nMax(100)
    , m_nValue(0)
    , m_uStep(1)
    , m_bCircle(TRUE)
{
    GetEventSet()->addEvent(EVENTID(EventSpinValue2String));
}

SSpinButtonCtrl::~SSpinButtonCtrl(void)
{
}

void SSpinButtonCtrl::SetValue(int nValue)
{
    m_nValue = nValue;
    OnValueChanged();
}

void SSpinButtonCtrl::SetRange(int nMin, int nMax)
{
    m_nMin = nMin;
    m_nMax = nMax;
    if (m_nValue < m_nMin)
        m_nValue = m_nMin;
    if (m_nValue > nMax)
        m_nValue = nMax;
    OnValueChanged();
}

void SSpinButtonCtrl::SetStep(UINT nStep)
{
    m_uStep = nStep;
}

int SSpinButtonCtrl::GetValue() const
{
    return m_nValue;
}

IWindow *SSpinButtonCtrl::GetIBuddy() const
{
    return m_pBuddy;
}

void SSpinButtonCtrl::SetBuddy(IWindow *pBuddy)
{
    MemberFunctionSlot<SSpinButtonCtrl, IEvtArgs> slot = Subscriber(&SSpinButtonCtrl::OnBuddyChange, this);
    if(m_pBuddy){
        m_pBuddy->UnsubscribeEvent(EVT_RE_NOTIFY, &slot);
    }
    m_pBuddy = pBuddy;
    if(m_pBuddy){
        m_pBuddy->SubscribeEvent(EVT_RE_NOTIFY, &slot);
    }
}
void SSpinButtonCtrl::GetDesiredSize(SIZE *psz, int wid, int hei)
{
    CSize szRet;
    if (GetLayoutParam()->IsSpecifiedSize(Horz))
    {
        szRet.cx = GetLayoutParam()->GetSpecifiedSize(Horz).toPixelSize(GetScale());
    }
    else
    {
        szRet.cx = m_pDownSkin->GetSkinSize().cx;
    }
    if (GetLayoutParam()->IsSpecifiedSize(Vert))
    {
        szRet.cy = GetLayoutParam()->GetSpecifiedSize(Vert).toPixelSize(GetScale());
    }
    else
    {
        szRet.cy = m_pDownSkin->GetSkinSize().cy + m_pUpSkin->GetSkinSize().cy;
    }
    *psz = szRet;
}

HRESULT SSpinButtonCtrl::OnAttrValue(const SStringW &strValue, BOOL bLoading)
{
    m_nValue = _wtoi(strValue);
    if (!bLoading)
    {
        OnValueChanged();
    }
    return S_OK;
}

void SSpinButtonCtrl::OnValueChanged(bool bInit)
{
    SWindow *pBuddy = GetBuddy();
    if (pBuddy)
    {
        SStringT strValue = SStringT().Format(_T("%d"), m_nValue);
        EventSpinValue2String evt(this);
        evt.bInit = bInit;
        evt.nValue = m_nValue;
        evt.strValue = &strValue;
        FireEvent(evt);
        pBuddy->SetWindowText(strValue);
    }
}

void SSpinButtonCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
    SWindow::OnLButtonDown(nFlags, point);
    OnClick();
    Invalidate();
    SetTimer(1, 200);
}

void SSpinButtonCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
    SWindow::OnLButtonUp(nFlags, point);
    KillTimer(1);
    Invalidate();
}

void SSpinButtonCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
    CRect rcClient = GetClientRect();
    if (rcClient.PtInRect(point))
    {
        if (point.y > (rcClient.top + rcClient.Height() / 2))
            m_iActionBtn = ACTION_DOWN;
        else
            m_iActionBtn = ACTION_UP;
    }
    else
    {
        m_iActionBtn = ACTION_NULL;
    }
}

void SSpinButtonCtrl::OnColorize(COLORREF cr)
{
    __baseCls::OnColorize(cr);

    if (m_pUpSkin)
        m_pUpSkin->OnColorize(cr);
    if (m_pDownSkin)
        m_pDownSkin->OnColorize(cr);
}

void SSpinButtonCtrl::OnPaint(IRenderTarget *pRT)
{
    SWindow::OnPaint(pRT);
    CRect rcClient = GetClientRect();
    CRect rcUp = rcClient, rcDown = rcClient;
    rcUp.bottom = rcDown.top = rcClient.top + rcClient.Height() / 2;
    int iState = SState2Index::GetDefIndex(GetState(), true);
    if (m_iActionBtn == ACTION_UP)
    {
        m_pUpSkin->DrawByIndex(pRT, rcUp, iState);
        m_pDownSkin->DrawByIndex(pRT, rcDown, iState != 2 ? iState : 1);
    }
    else if (m_iActionBtn == ACTION_DOWN)
    {
        m_pUpSkin->DrawByIndex(pRT, rcUp, iState != 2 ? iState : 1);
        m_pDownSkin->DrawByIndex(pRT, rcDown, iState);
    }
    else
    {
        iState = iState != 2 ? iState : 0;
        m_pUpSkin->DrawByIndex(pRT, rcUp, iState);
        m_pDownSkin->DrawByIndex(pRT, rcDown, iState);
    }
}
void SSpinButtonCtrl::OnTimer(char cTimerId)
{
    if (m_iActionBtn == ACTION_NULL)
    {
        return;
    }

    OnClick();
}

BOOL SSpinButtonCtrl::OnBuddyChange(IEvtArgs *pEvt)
{
    EventRENotify *pEvt2 = sobj_cast<EventRENotify>(pEvt);
    if (pEvt2->iNotify != EN_CHANGE)
        return false;
    SWindow *pBuddy = GetBuddy();
    if (pBuddy)
    {
        int nValue = _ttoi(pBuddy->GetWindowText());
        if (nValue <= m_nMax && nValue >= m_nMin)
        {
            m_nValue = nValue;

            SStringT strValue = SStringT().Format(_T("%d"), m_nValue);
            EventSpinValue2String evt(this);
            evt.bInit = false;
            evt.nValue = m_nValue;
            evt.strValue = &strValue;
            FireEvent(evt);
        }
    }
    return TRUE;
}

void SSpinButtonCtrl::OnScaleChanged(int scale)
{
    __baseCls::OnScaleChanged(scale);
    GetScaleSkin(m_pUpSkin, scale);
    GetScaleSkin(m_pDownSkin, scale);
}

int SSpinButtonCtrl::OnCreate(void *)
{
    int nRet = __baseCls::OnCreate(NULL);
    if (nRet != 0)
        return nRet;
    OnValueChanged(true);
    if(m_strBuddy.IsEmpty()) 
        return 0;
    SWindow * pBuddy = GetParent()->FindChildByName(m_strBuddy);
    SetBuddy(pBuddy);
    return 0;
}

void SSpinButtonCtrl::OnDestroy()
{
    SetBuddy(NULL);
    __baseCls::OnDestroy();
}
void SSpinButtonCtrl::OnClick()
{
    SASSERT(m_iActionBtn != ACTION_NULL);

    if (m_iActionBtn == ACTION_DOWN)
    {
        m_nValue -= m_uStep;
        if (m_nValue < m_nMin)
        {
            if (m_bCircle)
                m_nValue = m_nMax;
            else
                m_nValue = m_nMin;
        }
    }
    else
    {
        m_nValue += m_uStep;
        if (m_nValue > m_nMax)
        {
            if (m_bCircle)
                m_nValue = m_nMin;
            else
                m_nValue = m_nMax;
        }
    }
    OnValueChanged();
}

SWindow *SSpinButtonCtrl::GetBuddy() const
{
    return GetParent()->FindChildByName(m_strBuddy);
}

SNSEND
