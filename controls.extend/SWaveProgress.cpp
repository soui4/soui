#include "stdafx.h"
#include "SWaveProgress.h"

SNSBEGIN

SWaveProgress::SWaveProgress(void)
    : m_crBackground(RGBA(240, 240, 240,255)),
      m_crForeground(RGBA(0, 162, 232,255)), 
      m_crWave(RGBA(100, 200, 255, 128)),
      m_crBorder(RGBA(0, 0, 0, 255)),
      m_fWaveSpeed(0.01f),
      m_fWaveHeight(0.1f),
      m_nWaveCount(2),
      m_fPhase(0.0f),
      m_fPhase2(0.0f)
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
    int cx = rcClient.Width();
    if (cx <= 0 || m_nWaveCount <= 0)
        return;

    // 相位以波长为单位循环，水面平移量始终小于一个波长：路径向两侧各多画了
    // 两个波长，所以循环点时左右两侧都还有完整的波浪，看不出拼接缝。
    float fPeriod = (float)cx / m_nWaveCount;
    float fStep = m_fWaveSpeed * fPeriod;
    if (fStep <= 0.0f)
        fStep = 0.5f;

    m_fPhase += fStep;
    while (m_fPhase >= fPeriod)
        m_fPhase -= fPeriod;

    // 第二层波反向慢速漂移，和前景波错开相位，避免两条曲线永远对称。
    m_fPhase2 -= fStep * 0.6f;
    while (m_fPhase2 < 0.0f)
        m_fPhase2 += fPeriod;

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

void SWaveProgress::BuildWavePath(IPathS *pPath, int cx, int cy, float fAmpScale)
{
    if (!pPath || cx <= 0 || cy <= 0 || m_nWaveCount <= 0)
        return;

    float fPeriod = (float)cx / m_nWaveCount;
    float fAmp = m_fWaveHeight * cy * fAmpScale;
    // 左右各多出两个波长，相位在 [0, 一个波长) 内平移时整条水面都有波浪覆盖。
    float fStart = -2.0f * fPeriod;
    float fEnd = cx + 2.0f * fPeriod;

    pPath->moveTo(fStart, 0);
    float x = fStart;
    while (x < fEnd)
    {
        // 一个波长由两段三次贝塞尔组成（先波峰后波谷），控制点取在 1/4、3/4 处。
        pPath->cubicTo(x + fPeriod * 0.25f, -fAmp, x + fPeriod * 0.75f, fAmp, x + fPeriod, 0.0f);
        x += fPeriod;
    }
    // 水面以下闭合到控件底部之外，垂直平移最多是一个控件高度，多留一点余量。
    pPath->lineTo(fEnd, cy + fAmp * 2.0f);
    pPath->lineTo(fStart, cy + fAmp * 2.0f);
    pPath->close();
}

void SWaveProgress::OnSize(UINT nType, CSize size)
{
    __baseCls::OnSize(nType, size);
    int cx = size.cx;
    int cy = size.cy;
    if (cx <= 0 || cy <= 0)
        return;

    float fPeriod = (float)cx / (m_nWaveCount > 0 ? m_nWaveCount : 1);
    m_fPhase -= fPeriod * (float)(int)(m_fPhase / fPeriod);
    if (m_fPhase < 0.0f)
        m_fPhase += fPeriod;
    m_fPhase2 = fPeriod * 0.5f;

    m_pathFront = NULL;
    m_pathBack = NULL;
    GETRENDERFACTORY->CreatePath(&m_pathFront);
    GETRENDERFACTORY->CreatePath(&m_pathBack);
    // 前景波高、慢一档且反向漂移的后景波低一些，两层水面才有纵深。
    BuildWavePath(m_pathFront, cx, cy, 1.0f);
    BuildWavePath(m_pathBack, cx, cy, 0.65f);
}

void SWaveProgress::OnPaint(IRenderTarget *pRT)
{
    CRect rcClient;
    GetClientRect(&rcClient);
    if (rcClient.Width() <= 0 || rcClient.Height() <= 0)
        return;

    pRT->SaveClip(NULL);
    SAutoRefPtr<IRegionS> rgn;
    GETRENDERFACTORY->CreateRegion(&rgn);
    CRect rcClip = rcClient;
    rcClip.DeflateRect(1, 1);
    rgn->CombineEllipse(&rcClip, RGN_COPY);
    pRT->PushClipRegion(rgn, RGN_AND);

    // 绘制背景
    pRT->FillSolidRect(rcClient, m_crBackground);

    int nValue = GetValue();
    int nMin, nMax;
    GetRange(&nMin, &nMax);
    int nSpan = nMax - nMin;
    if (nSpan <= 0)
        nSpan = 1;

    if (nValue > nMin && m_pathFront && m_pathBack)
    {
        // 水面高度：value 越大水面越高，y 偏移越小。
        float fYOffset = rcClient.Height() * (1.0f - (nValue - nMin) / (float)nSpan);

        // 先画后景波，再让前景波盖在上面；后景波较矮且相位错开，露出的部分就是
        // 一层浅色水纹，比两层同形波叠在一起更耐看。
        struct WaveLayer
        {
            IPathS *pPath;
            float fPhase;
            COLORREF cr;
        };
        WaveLayer layers[2];
        layers[0].pPath = m_pathBack;
        layers[0].fPhase = m_fPhase2;
        layers[0].cr = m_crWave;
        layers[1].pPath = m_pathFront;
        layers[1].fPhase = m_fPhase;
        layers[1].cr = m_crForeground;

        for (int i = 0; i < 2; i++)
        {
            SAutoRefPtr<IBrushS> pBrush;
            pRT->CreateSolidColorBrush(layers[i].cr, &pBrush);
            if (!pBrush)
                continue;
            SAutoRefPtr<IBrushS> oldBrush;
            pRT->SelectObject(pBrush, (IRenderObj **)&oldBrush);
            layers[i].pPath->offset(rcClient.left + layers[i].fPhase, rcClient.top + fYOffset);
            pRT->FillPath(layers[i].pPath);
            layers[i].pPath->offset(-(rcClient.left + layers[i].fPhase), -(rcClient.top + fYOffset));
            pRT->SelectObject(oldBrush);
        }
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
