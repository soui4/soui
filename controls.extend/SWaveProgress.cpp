#include "stdafx.h"
#include "SWaveProgress.h"
#include <helper/slog.h>
#define kLogTag "waveProgress"

SNSBEGIN

SWaveProgress::SWaveProgress(void)
    : m_crBackground(RGBA(240, 240, 240,255)),
      m_crForeground(RGBA(0, 162, 232,255)), 
      m_crWave(RGBA(100, 200, 255, 128)),
      m_crBorder(RGBA(0, 0, 0, 255)),
      m_fWaveSpeed(0.02f),
      m_fWaveHeight(0.1f),
      m_nWaveCount(2),
      m_nOffset(0)
{
}

SWaveProgress::~SWaveProgress(void)
{
}

void SWaveProgress::StartAnimation()
{
    GetContainer()->RegisterTimelineHandler(this);
}

void SWaveProgress::StopAnimation()
{
    GetContainer()->UnregisterTimelineHandler(this);
}

void SWaveProgress::OnDestroy()
{
    StopAnimation();
    __baseCls::OnDestroy();
}

void SWaveProgress::OnNextFrame()
{
    CRect rcClient;
    GetClientRect(&rcClient);
    m_nOffset += m_fWaveSpeed * rcClient.Width();
    if (m_nOffset >= rcClient.Width())
        m_nOffset = 0.0f;
    Invalidate();
}

void SWaveProgress::OnShowWindow(BOOL bShow, UINT nStatus)
{
    __baseCls::OnShowWindow(bShow, nStatus);
    if (IsVisible(TRUE))
        StartAnimation();
    else
        StopAnimation();
}

void SWaveProgress::OnSize(UINT nType, CSize size)
{
    __baseCls::OnSize(nType, size);
    int cx = size.cx;
    int cy = size.cy;
    if (cx <= 0 || cy <= 0)
        return;

    // 创建波浪路径，宽度为控件宽度的2倍
    m_path = NULL;
    GETRENDERFACTORY->CreatePath(&m_path);
    // 波浪的参数
    int waveHeight = m_fWaveHeight * cy; // 波浪高度
    int wavePathWidth = cx * 3;     // 波浪路径宽度为控件宽度的3倍

    // 波浪起点：从左侧外开始
    m_path->moveTo(-cx, 0);

    // 使用贝塞尔曲线绘制波浪，每个波浪由两个贝塞尔曲线段组成（一个波峰和一个波谷）
    int waveLength = cx / m_nWaveCount;    // 每个波浪的长度
    int waveCountTotal = m_nWaveCount * 3; // 总波浪数（路径宽度为3倍控件宽度）

    for (int i = 0; i < waveCountTotal; i++)
    {
        // 计算当前波浪的起点和终点
        int startX = -cx + i * waveLength;
        int endX = startX + waveLength;

        // 计算波浪的控制点
        int midX = (startX + endX) / 2;

        // 波峰控制点
        int control1X = startX + waveLength / 4;
        int control1Y =  - waveHeight;

        // 波谷控制点
        int control2X = midX + waveLength / 4;
        int control2Y =  waveHeight;

        // 波浪终点
        int endY = 0;

        // 使用三次贝塞尔曲线绘制波浪
        m_path->cubicTo(control1X, control1Y, control2X, control2Y, endX, endY);
    }

    // 闭合波浪路径：连接到右侧底部和左侧底部
    m_path->lineTo(2*cx, cy);
    m_path->lineTo(-cx, cy);
    m_path->close();
    CRect rcBounds;
    m_path->getBounds(&rcBounds);
    SLOGI() <<" wave bounds : "<<rcBounds;       
}

void SWaveProgress::OnPaint(IRenderTarget *pRT)
{
    CRect rcClient;
    GetClientRect(&rcClient);
    
    pRT->SaveClip(NULL);
    SAutoRefPtr<IRegionS> rgn;
    GETRENDERFACTORY->CreateRegion(&rgn);
    CRect rcClip = rcClient;
    rcClip.DeflateRect(1, 1);
    rgn->CombineEllipse(&rcClip, RGN_COPY);
    pRT->PushClipRegion(rgn, RGN_AND);

    // 绘制背景
    pRT->FillSolidRect(rcClient, m_crBackground);
    // 计算进度高度
    int nValue = GetValue();
    int nMin, nMax;
    GetRange(&nMin, &nMax);

    int nHeight = rcClient.Height() * (nValue - nMin) / (nMax - nMin);
    if (nHeight > 0 && m_path){

        float fProgress = (nValue - nMin) / (float)(nMax - nMin);
        fProgress = 1.0f - fProgress;
        int xOffset = m_nOffset;
        int yOffset = (int)(rcClient.Height() * fProgress);
        // 平移路径，实现波浪动画效果
        m_path->offset(rcClient.left +  xOffset, rcClient.top + yOffset);
        {
            // 创建前景画刷
            SAutoRefPtr<IBrushS> pBrush;
            pRT->CreateSolidColorBrush(m_crForeground, &pBrush);
            if (pBrush)
            {
                SAutoRefPtr<IBrushS> oldBrush;
                pRT->SelectObject(pBrush, (IRenderObj **)&oldBrush);
                pRT->FillPath(m_path);
                pRT->SelectObject(oldBrush);
            }
            CRect rcBounds;
            m_path->getBounds(&rcBounds);
        }
        {
            int waveOffset = rcClient.Width() / m_nWaveCount * 0.5f;
            m_path->offset(-waveOffset, 0);
            // 创建半透明画刷
            SAutoRefPtr<IBrushS> pBrush;
            pRT->CreateSolidColorBrush(m_crWave, &pBrush);
            if (pBrush)
            {
                SAutoRefPtr<IBrushS> oldBrush;
                pRT->SelectObject(pBrush, (IRenderObj **)&oldBrush);
                pRT->FillPath(m_path);
                pRT->SelectObject(oldBrush);
            }
            m_path->offset(waveOffset, 0);

        }
        // 恢复路径
        m_path->offset(-(rcClient.left +  xOffset), -(rcClient.top + yOffset));
    }
    // 恢复裁剪区域
    pRT->RestoreClip();
    // 绘制边框
    CRect rcBorder = rcClient;
    rcBorder.DeflateRect(1, 1);
    SAutoRefPtr<IPenS> pen, oldPen;
    pRT->CreatePen(PS_SOLID, m_crBorder, 2, &pen);
    pRT->SelectObject(pen, (IRenderObj **)&oldPen);
    pRT->DrawEllipse(&rcBorder);
    pRT->SelectObject(oldPen, NULL);

}

SNSEND