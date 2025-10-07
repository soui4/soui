#pragma once

#include <hellocharts/SHelloCharts.h>
#include <time.h>


class CMainDlg : public SHostDialog
{
public:
    CMainDlg();
    ~CMainDlg();

    void OnClose();
    void OnMaximize();
    void OnRestore();
    void OnMinimize();
    void OnSize(UINT nType, CSize size);

    // Chart event handlers
    void OnBtnGenerateData();
    void OnBtnClearData();
    void OnBtnToggleAnimation();
    void OnBtnReplayAnimation();
    void OnBtnTestPieScenarios();

protected:
    BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

    // Chart event handlers (using SOUI event system)
    BOOL OnLineChartValueSelect(IEvtArgs* pEvt);
    BOOL OnColumnChartValueSelect(IEvtArgs* pEvt);
    BOOL OnPieChartValueSelect(IEvtArgs* pEvt);
    BOOL OnBubbleChartValueSelect(IEvtArgs* pEvt);
    BOOL OnRadarChartValueSelect(IEvtArgs* pEvt);
    BOOL OnComboChartValueSelect(IEvtArgs* pEvt);

    void OnChartValueSelected(int lineIndex, int pointIndex, SOUI::SPointValue* pValue);
    void OnChartValueDeselected();
    void OnColumnValueSelected(int columnIndex, int subcolumnIndex, SOUI::SSubcolumnValue* pValue);
    void OnColumnValueDeselected();
    void OnPieValueSelected(int sliceIndex, SOUI::SSliceValue* pValue);
    void OnPieValueDeselected();
    void OnBubbleValueSelected(int bubbleIndex, SOUI::SBubbleValue* pValue);
    void OnBubbleValueDeselected();
    void OnRadarValueSelected(int seriesIndex, SOUI::SRadarValue* pValue);
    void OnRadarValueDeselected();
    void OnComboValueSelected(int chartType, int valueIndex, int subValueIndex);
    void OnComboValueDeselected();

    // Generate sample data
    void GenerateSampleData();
    void GenerateLineChartData();
    void GenerateColumnChartData();
    void GeneratePieChartData();
    void GenerateTestPieData(int scenario); // Test different pie chart scenarios
    void GenerateBubbleChartData();
    void GenerateRadarChartData();
    void GenerateComboChartData();
    void ClearChartData();
    void UpdateChartStatistics();
    float GetTotalPieValue();

    void OnTabChartSelChanged(IEvtArgs* pEvt);

private:
    SOUI::SLineChartView* m_pLineChart;
    SOUI::SColumnChartView* m_pColumnChart;
    SOUI::SPieChartView* m_pPieChart;
    SOUI::SBubbleChartView* m_pBubbleChart;
    SOUI::SRadarChartView* m_pRadarChart;
    SOUI::SComboChartView* m_pComboChart;
    BOOL m_bAnimationEnabled;
    int m_currentPieScenario; // Current pie chart test scenario

    EVENT_MAP_BEGIN()
        EVENT_NAME_COMMAND(L"btn_close", OnClose)
        EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
        EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
        EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
        EVENT_NAME_COMMAND(L"btn_generate_data", OnBtnGenerateData)
        EVENT_NAME_COMMAND(L"btn_clear_data", OnBtnClearData)
        EVENT_NAME_COMMAND(L"btn_toggle_animation", OnBtnToggleAnimation)
        EVENT_NAME_COMMAND(L"btn_replay_animation", OnBtnReplayAnimation)
        EVENT_NAME_COMMAND(L"btn_test_pie", OnBtnTestPieScenarios)

        // Chart value selection events
        EVENT_NAME_HANDLER(L"line_chart", EVT_CHART_LINE_VALUE_SELECT, OnLineChartValueSelect)
        EVENT_NAME_HANDLER(L"column_chart", EVT_CHART_COLUMN_VALUE_SELECT, OnColumnChartValueSelect)
        EVENT_NAME_HANDLER(L"pie_chart", EVT_CHART_PIE_VALUE_SELECT, OnPieChartValueSelect)
        EVENT_NAME_HANDLER(L"bubble_chart", EVT_CHART_BUBBLE_VALUE_SELECT, OnBubbleChartValueSelect)
        EVENT_NAME_HANDLER(L"radar_chart", EVT_CHART_RADAR_VALUE_SELECT, OnRadarChartValueSelect)
        EVENT_NAME_HANDLER(L"combo_chart", EVT_CHART_COMBO_VALUE_SELECT, OnComboChartValueSelect)

        EVENT_NAME_HANDLER(L"chart_tabs", EventTabSelChanged::EventID, OnTabChartSelChanged)
    EVENT_MAP_END2(SHostDialog)

    BEGIN_MSG_MAP_EX(CMainDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_SIZE(OnSize)
        CHAIN_MSG_MAP(SHostDialog)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()
};
