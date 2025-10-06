#pragma once

#include "SAbstractChartView.h"
#include "../model/SBubbleChartData.h"
#include "../renderer/SBubbleChartRenderer.h"

SNSBEGIN

// Forward declarations
class SBubbleChartRenderer;

/**
 * @class SBubbleChartView
 * @brief Bubble chart view implementation
 */
class SBubbleChartView : public SAbstractChartView
{
    DEF_SOBJECT(SAbstractChartView, L"bubbleChart")

public:
    SBubbleChartView();
    virtual ~SBubbleChartView();

    // SAbstractChartView interface
    virtual SChartData* GetChartData() override;
    virtual void CallTouchListener() override;

    /**
     * @brief Get bubble chart data
     * @return Pointer to bubble chart data
     */
    SBubbleChartData* GetBubbleChartData() const { return m_pData; }

    /**
     * @brief Set bubble chart data
     * @param pData Pointer to bubble chart data
     */
    void SetBubbleChartData(SBubbleChartData* pData);

    /**
     * @brief Set value select listener
     * @param pListener Pointer to value select listener
     */
    void SetOnValueSelectListener(IBubbleChartOnValueSelectListener* pListener);

    /**
     * @brief Check if bubble labels are enabled
     * @return TRUE if bubble labels are enabled
     */
    BOOL IsBubbleLabelsEnabled() const { return m_bBubbleLabelsEnabled; }

    /**
     * @brief Set bubble labels enabled state
     * @param bEnabled Bubble labels enabled state
     */
    void SetBubbleLabelsEnabled(BOOL bEnabled);

    /**
     * @brief Get minimum bubble radius
     * @return Minimum bubble radius
     */
    int GetMinBubbleRadius() const { return m_minBubbleRadius; }

    /**
     * @brief Set minimum bubble radius
     * @param radius Minimum bubble radius
     */
    void SetMinBubbleRadius(int radius);

    /**
     * @brief Get maximum bubble radius
     * @return Maximum bubble radius
     */
    int GetMaxBubbleRadius() const { return m_maxBubbleRadius; }

    /**
     * @brief Set maximum bubble radius
     * @param radius Maximum bubble radius
     */
    void SetMaxBubbleRadius(int radius);

    /**
     * @brief Get bubble scale type
     * @return Bubble scale type (0=linear, 1=area)
     */
    int GetBubbleScale() const { return m_bubbleScale; }

    /**
     * @brief Set bubble scale type
     * @param scale Bubble scale type (0=linear, 1=area)
     */
    void SetBubbleScale(int scale);

protected:
    // SAbstractChartView interface
    virtual void InitializeRenderers() override;

    // Message handlers
    virtual void OnLButtonDown(UINT nFlags, CPoint point) override;

    /**
     * @brief Handle bubble value selection
     * @param bubbleIndex Selected bubble index
     * @param pValue Selected bubble value
     */
    void OnBubbleValueSelected(int bubbleIndex, SBubbleValue* pValue);

    /**
     * @brief Handle bubble value deselection
     */
    void OnBubbleValueDeselected();

private:
    SBubbleChartData* m_pData;
    SBubbleChartRenderer* m_pBubbleChartRenderer;
    IBubbleChartOnValueSelectListener* m_pOnValueSelectListener;
    
    // Chart properties
    BOOL m_bBubbleLabelsEnabled;
    int m_minBubbleRadius;
    int m_maxBubbleRadius;
    int m_bubbleScale;

    // Default values
    static const int DEFAULT_MIN_BUBBLE_RADIUS;
    static const int DEFAULT_MAX_BUBBLE_RADIUS;
    static const int DEFAULT_BUBBLE_SCALE;

    friend class SBubbleChartValueSelectListener;
};

/**
 * @class SBubbleChartValueSelectListener
 * @brief Default implementation of bubble chart value select listener
 */
class SBubbleChartValueSelectListener : public IBubbleChartOnValueSelectListener
{
public:
    SBubbleChartValueSelectListener(SBubbleChartView* pView) : m_pView(pView) {}
    virtual ~SBubbleChartValueSelectListener() {}

    virtual void OnValueSelected(int bubbleIndex, SBubbleValue* pValue) override
    {
        if (m_pView)
        {
            m_pView->OnBubbleValueSelected(bubbleIndex, pValue);
        }
    }

    virtual void OnValueDeselected() override
    {
        if (m_pView)
        {
            m_pView->OnBubbleValueDeselected();
        }
    }

private:
    SBubbleChartView* m_pView;
};

SNSEND
