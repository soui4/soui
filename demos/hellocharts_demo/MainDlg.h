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

    // Chart value selection listeners
    class CLineChartValueSelectListener : public SOUI::ILineChartOnValueSelectListener
    {
    public:
        CLineChartValueSelectListener(CMainDlg* pDlg) : m_pDlg(pDlg) {}
        
        virtual void OnValueSelected(int lineIndex, int pointIndex, SOUI::SPointValue* pValue) override;
        virtual void OnValueDeselected() override;
        
    private:
        CMainDlg* m_pDlg;
    };

    class CColumnChartValueSelectListener : public SOUI::IColumnChartOnValueSelectListener
    {
    public:
        CColumnChartValueSelectListener(CMainDlg* pDlg) : m_pDlg(pDlg) {}
        virtual void OnValueSelected(int columnIndex, int subcolumnIndex, SOUI::SSubcolumnValue* pValue) override;
        virtual void OnValueDeselected() override;
    private:
        CMainDlg* m_pDlg;
    };

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

    // Pie chart value selection listener
    class CPieChartValueSelectListener : public SOUI::IPieChartOnValueSelectListener
    {
    public:
        CPieChartValueSelectListener(CMainDlg* pDlg) : m_pDlg(pDlg) {}
        virtual void OnValueSelected(int sliceIndex, SOUI::SSliceValue* pValue) override;
        virtual void OnValueDeselected() override;
    private:
        CMainDlg* m_pDlg;
    };

    // Bubble chart value selection listener
    class CBubbleChartValueSelectListener : public SOUI::IBubbleChartOnValueSelectListener
    {
    public:
        CBubbleChartValueSelectListener(CMainDlg* pDlg) : m_pDlg(pDlg) {}
        virtual void OnValueSelected(int bubbleIndex, SOUI::SBubbleValue* pValue) override;
        virtual void OnValueDeselected() override;
    private:
        CMainDlg* m_pDlg;
    };

    // Radar chart value selection listener
    class CRadarChartValueSelectListener : public SOUI::IRadarChartOnValueSelectListener
    {
    public:
        CRadarChartValueSelectListener(CMainDlg* pDlg) : m_pDlg(pDlg) {}
        virtual void OnValueSelected(int seriesIndex, SOUI::SRadarValue* pValue);
        virtual void OnValueDeselected();
    private:
        CMainDlg* m_pDlg;
    };

    // Combo chart value selection listener
    class CComboChartValueSelectListener : public SOUI::IComboChartOnValueSelectListener
    {
    public:
        CComboChartValueSelectListener(CMainDlg* pDlg) : m_pDlg(pDlg) {}
        virtual void OnValueSelected(int chartType, int valueIndex, int subValueIndex);
        virtual void OnValueDeselected();
    private:
        CMainDlg* m_pDlg;
    };

private:
    SOUI::SLineChartView* m_pLineChart;
    SOUI::SColumnChartView* m_pColumnChart;
    SOUI::SPieChartView* m_pPieChart;
    SOUI::SBubbleChartView* m_pBubbleChart;
    SOUI::SRadarChartView* m_pRadarChart;
    SOUI::SComboChartView* m_pComboChart;
    CLineChartValueSelectListener* m_pValueSelectListener;
    CColumnChartValueSelectListener* m_pColumnValueSelectListener;
    CPieChartValueSelectListener* m_pPieValueSelectListener;
    CBubbleChartValueSelectListener* m_pBubbleValueSelectListener;
    CRadarChartValueSelectListener* m_pRadarValueSelectListener;
    CComboChartValueSelectListener* m_pComboValueSelectListener;
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
    EVENT_MAP_END2(SHostDialog)

    BEGIN_MSG_MAP_EX(CMainDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_SIZE(OnSize)
        CHAIN_MSG_MAP(SHostDialog)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()
};
