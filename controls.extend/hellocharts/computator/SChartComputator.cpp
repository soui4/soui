#include "stdafx.h"
#include "SChartComputator.h"
#include <algorithm>

SNSBEGIN

SChartComputator::SChartComputator()
    : m_chartMarginLeft(0)
    , m_chartMarginTop(0)
    , m_chartMarginRight(0)
    , m_chartMarginBottom(0)
    , m_minViewportZoom(1.0f)
    , m_maxViewportZoom(20.0f)
{
}

SChartComputator::~SChartComputator()
{
}

void SChartComputator::SetContentRect(int width, int height, int paddingLeft, int paddingTop, 
                                     int paddingRight, int paddingBottom)
{
    m_contentRect.SetRect(paddingLeft, paddingTop, 
                         width - paddingRight, height - paddingBottom);
}

CRect SChartComputator::GetContentRectMinusAllMargins() const
{
    CRect rect = m_contentRect;
    rect.left += m_chartMarginLeft;
    rect.top += m_chartMarginTop;
    rect.right -= m_chartMarginRight;
    rect.bottom -= m_chartMarginBottom;
    return rect;
}

void SChartComputator::SetChartAreaMargins(int left, int top, int right, int bottom)
{
    m_chartMarginLeft = left;
    m_chartMarginTop = top;
    m_chartMarginRight = right;
    m_chartMarginBottom = bottom;
}

void SChartComputator::GetChartAreaMargins(int* pLeft, int* pTop, int* pRight, int* pBottom) const
{
    if (pLeft) *pLeft = m_chartMarginLeft;
    if (pTop) *pTop = m_chartMarginTop;
    if (pRight) *pRight = m_chartMarginRight;
    if (pBottom) *pBottom = m_chartMarginBottom;
}

float SChartComputator::ComputeRawX(float rawX) const
{
    CRect chartRect = GetContentRectMinusAllMargins();
    float viewportWidth = m_currentViewport.GetWidth();
    if (viewportWidth == 0.0f) return (float)chartRect.left;
    
    float scale = (float)chartRect.Width() / viewportWidth;
    return (float)chartRect.left + (rawX - m_currentViewport.GetLeft()) * scale;
}

float SChartComputator::ComputeRawY(float rawY) const
{
    CRect chartRect = GetContentRectMinusAllMargins();
    float viewportHeight = m_currentViewport.GetHeight();
    if (viewportHeight == 0.0f) return (float)chartRect.bottom;
    
    float scale = (float)chartRect.Height() / viewportHeight;
    return (float)chartRect.bottom - (rawY - m_currentViewport.GetTop()) * scale;
}

float SChartComputator::ComputeRawDistanceX(float pixelX) const
{
    CRect chartRect = GetContentRectMinusAllMargins();
    float viewportWidth = m_currentViewport.GetWidth();
    if (chartRect.Width() == 0) return 0.0f;
    
    return (pixelX - (float)chartRect.left) * viewportWidth / (float)chartRect.Width() + m_currentViewport.GetLeft();
}

float SChartComputator::ComputeRawDistanceY(float pixelY) const
{
    CRect chartRect = GetContentRectMinusAllMargins();
    float viewportHeight = m_currentViewport.GetHeight();
    if (chartRect.Height() == 0) return 0.0f;
    
    return ((float)chartRect.bottom - pixelY) * viewportHeight / (float)chartRect.Height() + m_currentViewport.GetTop();
}

BOOL SChartComputator::IsInContentArea(float x, float y) const
{
    CRect chartRect = GetContentRectMinusAllMargins();
    return x >= chartRect.left && x <= chartRect.right && y >= chartRect.top && y <= chartRect.bottom;
}

int SChartComputator::GetChartWidth() const
{
    CRect chartRect = GetContentRectMinusAllMargins();
    return chartRect.Width();
}

int SChartComputator::GetChartHeight() const
{
    CRect chartRect = GetContentRectMinusAllMargins();
    return chartRect.Height();
}

SViewport SChartComputator::ConstrainViewport(const SViewport& viewport) const
{
    SViewport constrainedViewport = viewport;
    
    // Constrain to maximum viewport bounds
    if (constrainedViewport.GetLeft() < m_maximumViewport.GetLeft())
    {
        float diff = m_maximumViewport.GetLeft() - constrainedViewport.GetLeft();
        constrainedViewport.SetLeft(m_maximumViewport.GetLeft());
        constrainedViewport.SetRight(constrainedViewport.GetRight() + diff);
    }
    
    if (constrainedViewport.GetRight() > m_maximumViewport.GetRight())
    {
        float diff = constrainedViewport.GetRight() - m_maximumViewport.GetRight();
        constrainedViewport.SetRight(m_maximumViewport.GetRight());
        constrainedViewport.SetLeft(constrainedViewport.GetLeft() - diff);
    }
    
    if (constrainedViewport.GetTop() < m_maximumViewport.GetTop())
    {
        float diff = m_maximumViewport.GetTop() - constrainedViewport.GetTop();
        constrainedViewport.SetTop(m_maximumViewport.GetTop());
        constrainedViewport.SetBottom(constrainedViewport.GetBottom() + diff);
    }
    
    if (constrainedViewport.GetBottom() > m_maximumViewport.GetBottom())
    {
        float diff = constrainedViewport.GetBottom() - m_maximumViewport.GetBottom();
        constrainedViewport.SetBottom(m_maximumViewport.GetBottom());
        constrainedViewport.SetTop(constrainedViewport.GetTop() - diff);
    }
    
    // Constrain zoom level
    float currentZoom = m_maximumViewport.GetWidth() / constrainedViewport.GetWidth();
    if (currentZoom < m_minViewportZoom)
    {
        float scale = m_minViewportZoom / currentZoom;
        float centerX = (constrainedViewport.GetLeft() + constrainedViewport.GetRight()) / 2.0f;
        float centerY = (constrainedViewport.GetTop() + constrainedViewport.GetBottom()) / 2.0f;
        float newWidth = constrainedViewport.GetWidth() * scale;
        float newHeight = constrainedViewport.GetHeight() * scale;
        
        constrainedViewport.Set(centerX - newWidth / 2.0f, centerY - newHeight / 2.0f,
                               centerX + newWidth / 2.0f, centerY + newHeight / 2.0f);
    }
    else if (currentZoom > m_maxViewportZoom)
    {
        float scale = m_maxViewportZoom / currentZoom;
        float centerX = (constrainedViewport.GetLeft() + constrainedViewport.GetRight()) / 2.0f;
        float centerY = (constrainedViewport.GetTop() + constrainedViewport.GetBottom()) / 2.0f;
        float newWidth = constrainedViewport.GetWidth() * scale;
        float newHeight = constrainedViewport.GetHeight() * scale;
        
        constrainedViewport.Set(centerX - newWidth / 2.0f, centerY - newHeight / 2.0f,
                               centerX + newWidth / 2.0f, centerY + newHeight / 2.0f);
    }
    
    return constrainedViewport;
}

SNSEND
