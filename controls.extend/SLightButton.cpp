#include "stdafx.h"
#include "SLightButton.h"
#include "matrix/SMatrix.h"

SNSBEGIN

SLightButton::SLightButton()
    : m_strText(_T(""))
    , m_crTextColor(RGBA(255, 255, 255, 255))
    , m_crAlarmColor(RGBA(255, 107, 107, 255))
    , m_crNormalColor(RGBA(10, 10, 10, 255))
    , m_crBorderOutColorStart(RGBA(255, 255, 255, 255))
    , m_crBorderOutColorEnd(RGBA(166, 166, 166, 255))
    , m_crBorderInColorStart(RGBA(166, 166, 166, 255))
    , m_crBorderInColorEnd(RGBA(255, 255, 255, 255))
    , m_crBgColor(RGBA(100, 184, 255, 255))
    , m_bShowRect(FALSE)
    , m_bShowOverlay(TRUE)
    , m_crOverlayColor(RGBA(255, 255, 255, 255))
    , m_bIsAlarm(FALSE)
    , m_cAlarmTimerID(0)
{
    m_bClipClient = TRUE;
}

SLightButton::~SLightButton()
{
    if (m_cAlarmTimerID != 0)
    {
        KillTimer(m_cAlarmTimerID);
    }
}

void SLightButton::OnPaint(IRenderTarget *pRT)
{
    SPainter p;
    BeforePaint(pRT, p);
    CRect rcClient = GetClientRect();
    
    if (m_bShowRect)
    {
        // 绘制矩形模式
        SAutoRefPtr<IBrushS> pBrush;
        pRT->CreateSolidColorBrush(m_crBgColor, &pBrush);
        pRT->SelectObject(pBrush, NULL);
        
        POINT pt = {5, 5};
        pRT->FillRoundRect(&rcClient, pt);
        
    }
    else
    {
        // 绘制圆形发光模式
        int nWidth = rcClient.Width();
        int nHeight = rcClient.Height();
        int nSide = smin(nWidth, nHeight);

        // 计算缩放比例
        float fScale = nSide / 200.0f;

        // 保存当前变换矩阵
        float fOldMatrix[9];
        pRT->GetTransform(fOldMatrix);

        // 创建新的变换矩阵：平移到中心，然后缩放
        SMatrix mtx;
        CPoint ptCenter = rcClient.CenterPoint();
        mtx.translate((float)ptCenter.x, (float)ptCenter.y);
        mtx.preScale(fScale, fScale);
        // 应用变换
        pRT->SetTransform(mtx.fMat);

        // 计算绘制区域（以原点为中心的200x200区域）
        CRect rcDraw(-100, -100, 100, 100);

        // 绘制外边框
        DrawBorderOut(pRT, rcDraw);

        // 绘制内边框
        DrawBorderIn(pRT, rcDraw);

        // 绘制背景
        DrawBg(pRT, rcDraw);

        // 绘制遮罩层
        if (m_bShowOverlay)
        {
            mtx.preRotate(-20);
            pRT->SetTransform(mtx.fMat);
            DrawOverlay(pRT, rcDraw);
        }
        // 恢复变换矩阵
        pRT->SetTransform(fOldMatrix);
    }
    // 绘制文字
    DrawText(pRT, rcClient);

    AfterPaint(pRT, p);
}

void SLightButton::DrawBorderOut(IRenderTarget *pRT, const CRect& rcDraw)
{
    // 创建渐变画刷绘制外边框
    GradientItem gradients[2];
    gradients[0].pos = 0.0f;
    gradients[0].cr = m_crBorderOutColorStart;
    gradients[1].pos = 1.0f;
    gradients[1].cr = m_crBorderOutColorEnd;

    GradientInfo gradInfo;
    gradInfo.type = linear;
    gradInfo.angle = 45.0f; // 45度角的线性渐变

    SAutoRefPtr<IBrushS> pBrush;
    pRT->CreateGradientBrush(gradients, 2, &gradInfo, 255, kClamp_TileMode, &pBrush);
    pRT->SelectObject(pBrush, NULL);

    // 绘制外边框椭圆
    CRect rcOuter = rcDraw;
    pRT->FillEllipse(&rcOuter);
}

void SLightButton::DrawBorderIn(IRenderTarget *pRT, const CRect& rcDraw)
{
    // 创建渐变画刷绘制内边框
    GradientItem gradients[2];
    gradients[0].pos = 0.0f;
    gradients[0].cr = m_crBorderInColorStart;
    gradients[1].pos = 1.0f;
    gradients[1].cr = m_crBorderInColorEnd;

    GradientInfo gradInfo;
    gradInfo.type = linear;
    gradInfo.angle = 45.0f; // 45度角的线性渐变

    SAutoRefPtr<IBrushS> pBrush;
    pRT->CreateGradientBrush(gradients, 2, &gradInfo, 255, kClamp_TileMode, &pBrush);
    pRT->SelectObject(pBrush, NULL);

    // 绘制内边框椭圆
    CRect rcInner = rcDraw;
    rcInner.DeflateRect(10, 10);
    pRT->FillEllipse(&rcInner);
}

void SLightButton::DrawBg(IRenderTarget *pRT, const CRect& rcDraw)
{
    // 绘制背景圆形
    SAutoRefPtr<IBrushS> pBrush;
    pRT->CreateSolidColorBrush(m_crBgColor, &pBrush);
    pRT->SelectObject(pBrush, NULL);
    
    CRect rcBg = rcDraw;
    rcBg.DeflateRect(20, 20);
    pRT->FillEllipse(&rcBg);
}

void SLightButton::DrawText(IRenderTarget *pRT, const CRect& rcDraw)
{
    if (m_strText.IsEmpty())
        return;
        
    // 设置文字颜色
    COLORREF cr = pRT->SetTextColor(m_crTextColor);
    
    // 计算文字区域
    CRect rcText = rcDraw;
    rcText.DeflateRect(30, 30);
    
    // 绘制文字
    pRT->DrawText(m_strText, m_strText.GetLength(), &rcText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // 恢复文字颜色
    pRT->SetTextColor(cr);
}

void SLightButton::DrawOverlay(IRenderTarget *pRT, const CRect& rcDraw)
{
    SAutoRefPtr<IPathS> smallCircle, bigCircle;
    GETRENDERFACTORY->CreatePath(&smallCircle);
    GETRENDERFACTORY->CreatePath(&bigCircle);
    int radius = 80;
    radius -= 1;
    smallCircle->addOval2(-radius, -radius, radius, radius);
    radius *= 2;
    bigCircle->addOval2(-radius, -radius + 140, radius, radius+140);
    smallCircle->op(bigCircle, kDifference_PathOp, smallCircle);

    // 绘制遮罩层（高光效果）
    GradientItem gradients[2];
    gradients[0].pos = 0.0f;
    gradients[0].cr = RGBA(GetRValue(m_crOverlayColor), GetGValue(m_crOverlayColor), GetBValue(m_crOverlayColor), 100);
    gradients[1].pos = 1.0f;
    gradients[1].cr = RGBA(GetRValue(m_crOverlayColor), GetGValue(m_crOverlayColor), GetBValue(m_crOverlayColor), 30);;
    GradientInfo info;
    info.type = linear;
    info.angle = -90.0f;
    SAutoRefPtr<IBrushS> pBrush, oldBrush;
    pRT->CreateGradientBrush(gradients, 2, &info, 255, kClamp_TileMode, &pBrush);
    pRT->SelectObject(pBrush, (IRenderObj**)&oldBrush);
    pRT->FillPath(smallCircle);
    pRT->SelectObject(oldBrush, NULL);
}

void SLightButton::OnTimer(char cTimerID)
{
    if (cTimerID == m_cAlarmTimerID)
    {
        UpdateAlarmState();
    }else{
        SetMsgHandled(FALSE);
    }
    
}

// 设置文本
void SLightButton::SetText(const SStringT& strText)
{
    m_strText = strText;
    Invalidate();
}

// 设置文本颜色
void SLightButton::SetTextColor(COLORREF crTextColor)
{
    m_crTextColor = crTextColor;
    Invalidate();
}

// 设置报警颜色
void SLightButton::SetAlarmColor(COLORREF crAlarmColor)
{
    m_crAlarmColor = crAlarmColor;
    Invalidate();
}

// 设置正常颜色
void SLightButton::SetNormalColor(COLORREF crNormalColor)
{
    m_crNormalColor = crNormalColor;
    Invalidate();
}

// 设置外边框渐变颜色
void SLightButton::SetBorderOutColorStart(COLORREF crColor)
{
    m_crBorderOutColorStart = crColor;
    Invalidate();
}

void SLightButton::SetBorderOutColorEnd(COLORREF crColor)
{
    m_crBorderOutColorEnd = crColor;
    Invalidate();
}

// 设置里边框渐变颜色
void SLightButton::SetBorderInColorStart(COLORREF crColor)
{
    m_crBorderInColorStart = crColor;
    Invalidate();
}

void SLightButton::SetBorderInColorEnd(COLORREF crColor)
{
    m_crBorderInColorEnd = crColor;
    Invalidate();
}

// 设置背景色
void SLightButton::SetBgColor(COLORREF crBgColor)
{
    m_crBgColor = crBgColor;
    Invalidate();
}

// 设置显示矩形
void SLightButton::SetShowRect(BOOL bShowRect)
{
    m_bShowRect = bShowRect;
    Invalidate();
}

// 设置是否显示遮罩层
void SLightButton::SetShowOverlay(BOOL bShowOverlay)
{
    m_bShowOverlay = bShowOverlay;
    Invalidate();
}

// 设置遮罩层颜色
void SLightButton::SetOverlayColor(COLORREF crOverlayColor)
{
    m_crOverlayColor = crOverlayColor;
    Invalidate();
}

// 预设颜色方案
void SLightButton::SetGreen()
{
    m_crTextColor = RGBA(255, 255, 255, 255);
    SetBgColor(RGBA(0, 166, 0, 255));
}

void SLightButton::SetRed()
{
    m_crTextColor = RGBA(255, 255, 255, 255);
    SetBgColor(RGBA(166, 0, 0, 255));
}

void SLightButton::SetYellow()
{
    m_crTextColor = RGBA(0, 0, 0, 255);
    SetBgColor(RGBA(255, 255, 0, 255));
}

void SLightButton::SetBlack()
{
    m_crTextColor = RGBA(255, 255, 255, 255);
    SetBgColor(RGBA(10, 10, 10, 255));
}

void SLightButton::SetGray()
{
    m_crTextColor = RGBA(255, 255, 255, 255);
    SetBgColor(RGBA(129, 129, 129, 255));
}

void SLightButton::SetBlue()
{
    m_crTextColor = RGBA(255, 255, 255, 255);
    SetBgColor(RGBA(0, 0, 166, 255));
}

void SLightButton::SetLightBlue()
{
    m_crTextColor = RGBA(255, 255, 255, 255);
    SetBgColor(RGBA(100, 184, 255, 255));
}

void SLightButton::SetLightRed()
{
    m_crTextColor = RGBA(255, 255, 255, 255);
    SetBgColor(RGBA(255, 107, 107, 255));
}

void SLightButton::SetLightGreen()
{
    m_crTextColor = RGBA(255, 255, 255, 255);
    SetBgColor(RGBA(24, 189, 155, 255));
}

// 报警控制
void SLightButton::StartAlarm()
{
    if (m_cAlarmTimerID == 0)
    {
        m_cAlarmTimerID = 1;
        SetTimer(m_cAlarmTimerID, 500);
    }
}

void SLightButton::StopAlarm()
{
    if (m_cAlarmTimerID != 0)
    {
        KillTimer(m_cAlarmTimerID);
        m_cAlarmTimerID = 0;
        m_bIsAlarm = FALSE;
        // 恢复正常颜色
        m_crTextColor = RGBA(255, 255, 255, 255);
        m_crBgColor = m_crNormalColor;
        Invalidate();
    }
}

// 更新报警状态
void SLightButton::UpdateAlarmState()
{
    if (m_bIsAlarm)
    {
        m_crTextColor = RGBA(255, 255, 255, 255);
        m_crBgColor = m_crNormalColor;
    }
    else
    {
        m_crTextColor = RGBA(255, 255, 255, 255);
        m_crBgColor = m_crAlarmColor;
    }

    Invalidate();
    m_bIsAlarm = !m_bIsAlarm;
}

SNSEND
