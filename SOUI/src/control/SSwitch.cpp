#include <souistd.h>
#include "control/SSwitch.h"

SNSBEGIN

SSwitch::SSwitch()
    : m_pSkin(GETBUILTINSKIN(SKIN_SYS_SWITCH_BG))
    , m_pSkinForce(GETBUILTINSKIN(SKIN_SYS_SWITCH))
    , m_animator(NULL)
{
    m_animator.Attach(SApplication::getSingletonPtr()->CreateValueAnimatorByName(SFloatAnimator::GetClassName()));
    m_animator->setDuration(200);
    m_animator->addListener(this);
    m_animator->addUpdateListener(this);
}

SSwitch::~SSwitch()
{
}

void SSwitch::onAnimationEnd(IValueAnimator *pAnimator)
{
    SLOGI2("switch") << "fraction:" << pAnimator->getAnimatedFraction();
    SetCheck(!pAnimator->GetID());
}

void SSwitch::onAnimationUpdate(IValueAnimator *pAnimator)
{
    Invalidate();
}

void SSwitch::OnPaint(IRenderTarget *pRT)
{
    CRect rcWnd = GetWindowRect();
    if (m_pSkin)
        m_pSkin->DrawByState(pRT, rcWnd, 0);
    if (!m_pSkinForce)
        return;
    SIZE skSize = m_pSkinForce->GetSkinSize();
    int dwSpace = rcWnd.Width() - skSize.cx;
    CRect rcDst;
    rcDst.top = 0;
    rcDst.bottom = skSize.cy;
    if (m_animator->isRunning())
    {
        float fac = m_animator->getAnimatedFraction();
        if (m_animator->GetID() == 0) // uncheck to check
        {
            rcDst.left = 0;
            rcDst.right = skSize.cx;
            rcDst.OffsetRect((int)(fac * (rcWnd.Width() - skSize.cx)), 0);
        }
        else // check to uncheck
        {
            rcDst.right = rcWnd.Width();
            rcDst.left = rcDst.right - skSize.cx;
            rcDst.OffsetRect(-(int)(fac * (rcWnd.Width() - skSize.cx)), 0);
        }
        rcDst.OffsetRect(rcWnd.TopLeft());

        BYTE alpha2 = (BYTE)(fac * 255);
        BYTE alpha1 = 255 - alpha2;

        DWORD dwState = GetState();
        m_pSkinForce->DrawByState2(pRT, rcDst, dwState, alpha1);
        dwState ^= WndState_Check;
        m_pSkinForce->DrawByState2(pRT, rcDst, dwState, alpha2);
    }
    else
    {
        rcDst.left = 0;
        rcDst.right = skSize.cx;
        if (IsChecked())
        {
            rcDst.OffsetRect(rcWnd.Width() - skSize.cx, 0);
        }
        rcDst.OffsetRect(rcWnd.TopLeft());
        m_pSkinForce->DrawByState(pRT, rcDst, GetState());
    }
}

void SSwitch::GetDesiredSize(SIZE *ret, int wid, int hei)
{
    if (m_pSkin)
        *ret = m_pSkin->GetSkinSize();
    else
        __baseCls::GetDesiredSize(ret, wid, hei);
}

void SSwitch::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (!(GetState() & WndState_PushDown))
        return;
    ReleaseCapture();
    ModifyState(0, WndState_PushDown, TRUE);

    m_animator->SetID(IsChecked());
    m_animator->start(GetContainer());
}

void SSwitch::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_SPACE)
    {
        m_animator->SetID(IsChecked());
        m_animator->start(GetContainer());
    }
}

HRESULT SSwitch::OnAttrAnimator(const SStringW &value, BOOL bLoading)
{
    m_animator.Attach(SApplication::getSingletonPtr()->LoadValueAnimator(S_CW2T(value)));
    m_animator->addListener(this);
    m_animator->addUpdateListener(this);
    return S_FALSE;
}

HRESULT SSwitch::OnAttrCheck(const SStringW &strValue, BOOL bLoading)
{
    SetCheck(STRINGASBOOL(strValue));
    return bLoading ? S_OK : S_FALSE;
}

void SSwitch::OnScaleChanged(int scale)
{
    SWindow::OnScaleChanged(scale);
    GetScaleSkin(m_pSkin, scale);
    GetScaleSkin(m_pSkinForce, scale);
}
SNSEND