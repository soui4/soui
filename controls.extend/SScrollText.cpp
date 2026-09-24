#include "stdafx.h"
#include "SScrollText.h"

SNSBEGIN

SScrollText::SScrollText(void)
    : m_nSpeed(20)
    , m_nOffset(0)
    , m_nScrollWidth(0)
    , m_nRollType(0)
    , m_nNextInterval(0)
    , m_nStayTime(0)
    , m_nLineHeight(0)
    , m_nLineCount(0)
    , m_nCurLine(0)
    , m_nVPos(0)
    , m_nHOff(0)
    , m_nHTick(0)
    , m_bMultiline(FALSE)
    , m_bVDwell(FALSE)
{
}

SScrollText::~SScrollText(void)
{
}

void SScrollText::OnPaint(IRenderTarget *pRT)
{
    SPainter painter;
    BeforePaint(pRT, painter);
    CRect rcClient = GetClientRect();
    if (m_bMultiline)
    {
        pRT->PushClipRect(&rcClient, RGN_AND);

        if (!m_bVDwell && m_nVPos != 0)
        {
            // 切换中：当前行向上移出，下一行在同行空间从下方移入
            int next = (m_nCurLine + 1) % m_nLineCount;

            CRect rcOut = rcClient;
            rcOut.top -= m_nVPos;
            rcOut.bottom -= m_nVPos;
            rcOut.left -= m_nHOff;
            pRT->DrawText(m_vLines[m_nCurLine], m_vLines[m_nCurLine].GetLength(), &rcOut,
                          DT_SINGLELINE | DT_VCENTER);

            int offIn = m_nVPos - m_nLineHeight;
            CRect rcIn = rcClient;
            rcIn.top -= offIn;
            rcIn.bottom -= offIn;
            pRT->DrawText(m_vLines[next], m_vLines[next].GetLength(), &rcIn,
                          DT_SINGLELINE | DT_VCENTER);
        }
        else
        {
            // 停留：仅显示当前一行
            CRect rcText = rcClient;
            rcText.left -= m_nHOff;
            pRT->DrawText(m_vLines[m_nCurLine], m_vLines[m_nCurLine].GetLength(), &rcText,
                          DT_SINGLELINE | DT_VCENTER);
        }

        pRT->PopClip();
    }
    else if (m_nScrollWidth == 0)
    {
        pRT->DrawText(m_strText.GetText(), m_strText.GetText().GetLength(), &rcClient,
                      DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    }
    else
    {
        pRT->PushClipRect(&rcClient, RGN_AND);

        CRect rcText = rcClient;
        rcText.left -= m_nOffset;
        pRT->DrawText(m_strText.GetText(), m_strText.GetText().GetLength(), &rcText,
                      DT_SINGLELINE | DT_VCENTER);
        if (m_nRollType == 0)
        {
            if (m_nScrollWidth - m_nOffset < rcClient.Width())
            {
                rcText.left += m_nScrollWidth;
                //                 pRT->SetTextColor(RGBA(0,0,0,255));
                pRT->DrawText(m_strText.GetText(), m_strText.GetText().GetLength(), &rcText,
                              DT_SINGLELINE | DT_VCENTER);
            }
        }

        pRT->PopClip();
    }
    AfterPaint(pRT, painter);
}

void SScrollText::OnSize(UINT nType, CSize size)
{
    __baseCls::OnSize(nType, size);
    UpdateScrollInfo(size);
}

void SScrollText::OnShowWindow(BOOL bShow, UINT nStatus)
{
    __baseCls::OnShowWindow(bShow, nStatus);
    if (IsScrolling())
    {
        if (IsVisible(TRUE))
        {
            GetContainer()->RegisterTimelineHandler(this);
            m_nNextInterval = m_bMultiline ? m_nStayTime : m_nSpeed;
        }
        else
        {
            GetContainer()->UnregisterTimelineHandler(this);
        }
    }
}

void SScrollText::SetWindowText(const SStringT &strText)
{
    __baseCls::SetWindowText(strText);
    UpdateScrollInfo(GetClientRect().Size()); //重新计算滚动长度
}

void SScrollText::UpdateScrollInfo(CSize size)
{
    SAutoRefPtr<IRenderTarget> pRT;
    GETRENDERFACTORY->CreateRenderTarget(&pRT, 0, 0);
    BeforePaintEx(pRT);

    // 按换行符拆分多行
    m_vLines.RemoveAll();
    m_awLineWidth.RemoveAll();
    const SStringT &strText = m_strText.GetText();
    int len = strText.GetLength();
    int start = 0;
    while (start < len)
    {
        int end = strText.FindChar(_T('\n'), start);
        if (end == -1)
            end = len;
        SStringT line = strText.Mid(start, end - start);
        m_vLines.Add(line);
        SIZE szl;
        pRT->MeasureText(line, line.GetLength(), &szl);
        m_awLineWidth.Add(szl.cx);
        start = end + 1;
    }

    m_nLineCount = (int)m_vLines.GetCount();
    m_bMultiline = m_nLineCount > 1;
    if (!m_bMultiline)
    {
        // 单行：沿用原水平/静态逻辑
        int lineW = m_nLineCount > 0 ? m_awLineWidth[0] : 0;
        m_nOffset = 0;
        m_nScrollWidth = 0;
        if (lineW - size.cx > 0)
        {
            m_nScrollWidth = lineW;
            if (m_nRollType == 1)
            {
                m_nOffset = -size.cx;
            }
        }
        if (m_nScrollWidth > 0)
        {
            if (IsVisible(TRUE))
            {
                GetContainer()->RegisterTimelineHandler(this);
                m_nNextInterval = m_nSpeed;
            }
            else
            {
                GetContainer()->UnregisterTimelineHandler(this);
            }
        }
        return;
    }

    // 多行：垂直滚动 + 停留期水平展示
    SIZE sz;
    pRT->MeasureText(strText, strText.GetLength(), &sz);
    m_nLineHeight = sz.cy;
    m_nScrollWidth = 0;
    m_nCurLine = 0;
    m_nVPos = 0;
    m_nHOff = 0;
    m_nHTick = 0;
    m_bVDwell = TRUE;

    if (m_nLineCount > 1 && m_nLineHeight > 0)
    {
        if (IsVisible(TRUE))
        {
            GetContainer()->RegisterTimelineHandler(this);
            m_nNextInterval = m_nStayTime;
        }
        else
        {
            GetContainer()->UnregisterTimelineHandler(this);
        }
    }
    else
    {
        GetContainer()->UnregisterTimelineHandler(this);
    }
}

bool SScrollText::IsScrolling() const
{
    if (m_bMultiline)
        return m_nLineCount > 1 && m_nLineHeight > 0;
    return m_nScrollWidth > 0;
}

void SScrollText::OnNextFrame()
{
    if (m_bMultiline)
        OnNextFrameMulti();
    else
        OnNextFrameHoz();
}

void SScrollText::OnNextFrameHoz()
{
    m_nNextInterval -= 10;
    if (m_nNextInterval < 0)
    {
        m_nNextInterval = m_nSpeed;
        if (m_nScrollWidth > 0)
        {
            m_nOffset++;
            if (m_nOffset > m_nScrollWidth)
            {
                if (m_nRollType == 0)
                {
                    m_nOffset = 0;
                }
                else if (m_nRollType == 1)
                {
                    m_nOffset = -GetClientRect().Width();
                }
            }
            Invalidate();
        }
    }
}

void SScrollText::OnNextFrameMulti()
{
    if (m_nLineCount <= 1 || m_nLineHeight <= 0)
        return;
    m_nNextInterval -= 10;
    if (m_nNextInterval >= 0)
    {
        // 停留期间：当前行若超宽则水平滚动展示完整内容
        if (m_bVDwell)
            StepHozReveal();
        return;
    }

    if (m_bVDwell)
    {
        // 停留结束，开始垂直滚动到下一行
        m_bVDwell = FALSE;
        m_nNextInterval = m_nSpeed;
    }
    else
    {
        // 逐像素向上滚动到下一行
        m_nVPos++;
        if (m_nVPos >= m_nLineHeight)
        {
            m_nVPos = 0;
            m_nCurLine++;
            if (m_nCurLine >= m_nLineCount)
            {
                if (m_nRollType == 0)
                {
                    m_nCurLine = 0;
                }
                else
                {
                    // 非衔接：到达末行后停留在末行，不再滚动
                    m_nCurLine = m_nLineCount - 1;
                    m_nNextInterval = 0x7FFFFFFF;
                    Invalidate();
                    return;
                }
            }
            // 新行完全显示，进入停留阶段
            m_bVDwell = TRUE;
            m_nNextInterval = m_nStayTime;
            m_nHOff = 0;
            m_nHTick = 0;
        }
        else
        {
            m_nNextInterval = m_nSpeed;
        }
    }
    Invalidate();
}

void SScrollText::StepHozReveal()
{
    m_nHTick -= 10;
    if (m_nHTick >= 0)
        return;
    m_nHTick = m_nSpeed;

    int clientW = GetClientRect().Width();
    int lineW = m_awLineWidth[m_nCurLine];
    if (lineW <= clientW)
        return;
    int maxOff = lineW - clientW;
    if (m_nHOff < maxOff)
    {
        m_nHOff++;
        Invalidate();
    }
}

void SScrollText::OnDestroy()
{
    GetContainer()->UnregisterTimelineHandler(this);
    SStatic::OnDestroy();
}

void SScrollText::OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer)
{
    if (pOldContainer)
        pOldContainer->UnregisterTimelineHandler(this);
    if (pNewContainer)
        pNewContainer->RegisterTimelineHandler(this);
    SWindow::OnContainerChanged(pOldContainer, pNewContainer);
}

SNSEND
