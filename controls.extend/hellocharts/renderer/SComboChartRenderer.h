#pragma once
#include "../SChart.h"
#include "SChartRenderer.h"
#include "SLineChartRenderer.h"
#include "SColumnChartRenderer.h"
#include "../model/SComboChartData.h"

SNSBEGIN

// Forward declarations
class SComboChartView;

/**
 * @class SComboChartRenderer
 * @brief Renderer for combo charts (combination of line and column charts)
 */
class SComboChartRenderer : public SChartRenderer
{
public:
    SComboChartRenderer(ISChart* pChart);
    virtual ~SComboChartRenderer();

    // SChartRenderer interface
    virtual void Draw(IRenderTarget* pRT);
    virtual void DrawUnclipped(IRenderTarget* pRT);
    virtual BOOL IsTouched(float touchX, float touchY);

    /**
     * @brief Select combo value
     * @param selectedValue Selected value
     */
    virtual void SelectValue(const SSelectedValue& selectedValue);

    /**
     * @brief Deselect combo value
     */
    virtual void DeselectValue();

    /**
     * @brief Get combo chart view
     * @return Combo chart view
     */
    SComboChartView* GetComboChartView() const { return m_pComboChartView; }

    /**
     * @brief Set combo chart view
     * @param pView Combo chart view
     */
    void SetComboChartView(SComboChartView* pView) { m_pComboChartView = pView; }

    /**
     * @brief Get line chart renderer
     * @return Line chart renderer
     */
    SLineChartRenderer* GetLineChartRenderer() const { return m_pLineChartRenderer; }

    /**
     * @brief Get column chart renderer
     * @return Column chart renderer
     */
    SColumnChartRenderer* GetColumnChartRenderer() const { return m_pColumnChartRenderer; }

    /**
     * @brief Check if selection is from line chart
     * @return TRUE if selection is from line chart
     */
    BOOL IsLineChartSelection() const { return m_bIsLineChartSelection; }

    /**
     * @brief Check if selection is from column chart
     * @return TRUE if selection is from column chart
     */
    BOOL IsColumnChartSelection() const { return !m_bIsLineChartSelection; }

private:
    /**
     * @brief Initialize sub-renderers
     */
    void InitializeSubRenderers();

    /**
     * @brief Draw secondary Y axis
     * @param pRT Render target
     */
    void DrawSecondaryYAxis(IRenderTarget* pRT);

    /**
     * @brief Get combo chart data
     * @return Combo chart data
     */
    SComboChartData* GetComboChartData() const;

    /**
     * @brief Setup line chart renderer with combo data
     */
    void SetupLineChartRenderer();

    /**
     * @brief Setup column chart renderer with combo data
     */
    void SetupColumnChartRenderer();

    /**
     * @brief Apply alpha to line chart data
     * @param alpha Alpha value (0.0 to 1.0)
     */
    void ApplyLineChartAlpha(float alpha);

    /**
     * @brief Apply alpha to column chart data
     * @param alpha Alpha value (0.0 to 1.0)
     */
    void ApplyColumnChartAlpha(float alpha);

private:
    /**
     * @brief Line chart adapter for combo chart
     */
    class SComboLineChartAdapter : public ISChart
    {
    public:
        SComboLineChartAdapter(SComboChartView* pComboView) : m_pComboView(pComboView) {}
        virtual SChartData* GetChartData();
        virtual SChartRenderer* GetChartRenderer();
        virtual void SetChartRenderer(SChartRenderer* pRenderer);
        virtual SAxesRenderer* GetAxesRenderer();
        virtual SChartComputator* GetChartComputator();
        virtual SChartTouchHandler* GetTouchHandler();
        virtual void AnimationDataUpdate(float scale);
        virtual void AnimationDataFinished();
        virtual void StartDataAnimation(DWORD duration);
        virtual void CancelDataAnimation();
        virtual BOOL IsDataAnimationRunning();
        virtual SViewport GetCurrentViewport();
        virtual void SetCurrentViewport(const SViewport& viewport);
        virtual SViewport GetMaximumViewport();
        virtual void SetMaximumViewport(const SViewport& viewport);
        virtual BOOL IsInteractive();
        virtual void SetInteractive(BOOL bInteractive);
        virtual BOOL IsZoomEnabled();
        virtual void SetZoomEnabled(BOOL bEnabled);
        virtual BOOL IsScrollEnabled();
        virtual void SetScrollEnabled(BOOL bEnabled);
        virtual SSelectedValue GetSelectedValue();
        virtual BOOL SelectValue(float x, float y);
        virtual void ClearSelection();
        virtual void OnChartDataChange();
        virtual void OnChartSizeChanged();
        virtual void CallTouchListener();
        virtual void GetRect(LPRECT pRect);
    private:
        SComboChartView* m_pComboView;
    };

    /**
     * @brief Column chart adapter for combo chart
     */
    class SComboColumnChartAdapter : public ISChart
    {
    public:
        SComboColumnChartAdapter(SComboChartView* pComboView) : m_pComboView(pComboView) {}
        virtual SChartData* GetChartData();
        virtual SChartRenderer* GetChartRenderer();
        virtual void SetChartRenderer(SChartRenderer* pRenderer);
        virtual SAxesRenderer* GetAxesRenderer();
        virtual SChartComputator* GetChartComputator();
        virtual SChartTouchHandler* GetTouchHandler();
        virtual void AnimationDataUpdate(float scale);
        virtual void AnimationDataFinished();
        virtual void StartDataAnimation(DWORD duration);
        virtual void CancelDataAnimation();
        virtual BOOL IsDataAnimationRunning();
        virtual SViewport GetCurrentViewport();
        virtual void SetCurrentViewport(const SViewport& viewport);
        virtual SViewport GetMaximumViewport();
        virtual void SetMaximumViewport(const SViewport& viewport);
        virtual BOOL IsInteractive();
        virtual void SetInteractive(BOOL bInteractive);
        virtual BOOL IsZoomEnabled();
        virtual void SetZoomEnabled(BOOL bEnabled);
        virtual BOOL IsScrollEnabled();
        virtual void SetScrollEnabled(BOOL bEnabled);
        virtual SSelectedValue GetSelectedValue();
        virtual BOOL SelectValue(float x, float y);
        virtual void ClearSelection();
        virtual void OnChartDataChange();
        virtual void OnChartSizeChanged();
        virtual void CallTouchListener();
        virtual void GetRect(LPRECT pRect);
    private:
        SComboChartView* m_pComboView;
    };

private:
    SComboChartView* m_pComboChartView;
    SLineChartRenderer* m_pLineChartRenderer;
    SColumnChartRenderer* m_pColumnChartRenderer;
    SComboLineChartAdapter* m_pLineChartAdapter;
    SComboColumnChartAdapter* m_pColumnChartAdapter;

    BOOL m_bIsLineChartSelection;
    BOOL m_bSubRenderersInitialized;
};


SNSEND