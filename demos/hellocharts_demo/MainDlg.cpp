#include "stdafx.h"
#include "MainDlg.h"
#include <time.h>
#include "hellocharts/view/SAbstractChartView.h"

using namespace SOUI;

CMainDlg::CMainDlg()
    : SHostDialog(_T("LAYOUT:dlg_main"))
    , m_pLineChart(NULL)
    , m_pColumnChart(NULL)
    , m_pPieChart(NULL)
    , m_pBubbleChart(NULL)
    , m_pRadarChart(NULL)
    , m_pComboChart(NULL)
    , m_bAnimationEnabled(TRUE)
    , m_currentPieScenario(0)
{
}

CMainDlg::~CMainDlg()
{
    // 无需手动删除listener对象，SOUI自动处理事件订阅
}

BOOL CMainDlg::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
{
    // Initialize charts (events are automatically handled by EVENT_MAP)
    m_pLineChart = FindChildByName2<SLineChartView>(L"line_chart");
    m_pColumnChart = FindChildByName2<SColumnChartView>(L"column_chart");
    m_pPieChart = FindChildByName2<SPieChartView>(L"pie_chart");
    m_pBubbleChart = FindChildByName2<SBubbleChartView>(L"bubble_chart");
    m_pRadarChart = FindChildByName2<SRadarChartView>(L"radar_chart");
    m_pComboChart = FindChildByName2<SComboChartView>(L"combo_chart");

    // Generate initial sample data for all charts
    GenerateSampleData();

    return 0;
}

void CMainDlg::OnClose()
{
    SHostWnd::DestroyWindow();
}

void CMainDlg::OnMaximize()
{
    SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}

void CMainDlg::OnRestore()
{
    SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}

void CMainDlg::OnMinimize()
{
    SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
    if(nType == SIZE_MAXIMIZED){
        FindChildByName2<SWindow>(L"btn_max")->SetVisible(FALSE);
        FindChildByName2<SWindow>(L"btn_restore")->SetVisible(TRUE);
    }else{
        FindChildByName2<SWindow>(L"btn_max")->SetVisible(TRUE);
        FindChildByName2<SWindow>(L"btn_restore")->SetVisible(FALSE);
    }
    SetMsgHandled(FALSE);
}

void CMainDlg::OnBtnGenerateData()
{
    GenerateSampleData();
}

void CMainDlg::OnBtnClearData()
{
    ClearChartData();
}

void CMainDlg::OnBtnToggleAnimation()
{
    m_bAnimationEnabled = !m_bAnimationEnabled;

    SWindow* pBtn = FindChildByName(L"btn_toggle_animation");
    if (pBtn)
    {
        SStringT text = m_bAnimationEnabled ? _T("Disable Animation") : _T("Enable Animation");
        pBtn->SetWindowText(text);
    }
}

void CMainDlg::OnBtnReplayAnimation()
{
    if (!m_bAnimationEnabled)
        return;

    // Replay animation for all charts
    if (m_pLineChart)
        m_pLineChart->StartDataAnimation(500);
    if (m_pColumnChart)
        m_pColumnChart->StartDataAnimation(500);
    if (m_pPieChart)
        m_pPieChart->StartDataAnimation(500);
    if (m_pBubbleChart)
        m_pBubbleChart->StartDataAnimation(800);
    if (m_pRadarChart)
        m_pRadarChart->StartDataAnimation(1000);
    if (m_pComboChart)
        m_pComboChart->StartDataAnimation(1200);

    // Update status
    SWindow* pStatus = FindChildByName(L"txt_status");
    if (pStatus)
    {
        pStatus->SetWindowText(_T("Animation replaying..."));
    }
}

void CMainDlg::GenerateSampleData()
{
    // Generate random data using traditional C++ rand() function
    srand((unsigned int)time(NULL));

    // Generate line chart data
    if (m_pLineChart)
    {
        GenerateLineChartData();
    }

    // Generate column chart data
    if (m_pColumnChart)
    {
        GenerateColumnChartData();
    }

    // Generate pie chart data
    if (m_pPieChart)
    {
        GeneratePieChartData();
    }

    // Generate bubble chart data
    if (m_pBubbleChart)
    {
        GenerateBubbleChartData();
    }

    // Generate radar chart data
    if (m_pRadarChart)
    {
        GenerateRadarChartData();
    }

    // Generate combo chart data
    if (m_pComboChart)
    {
        GenerateComboChartData();
    }

    // Update status
    SWindow* pStatus = FindChildByName(L"txt_status");
    if (pStatus)
    {
        pStatus->SetWindowText(_T("Sample data generated"));
    }
}

void CMainDlg::GenerateLineChartData()
{
    // Create new line chart data
    SLineChartData* pData = new SLineChartData();

    // Configure X axis
    SAxis* pAxisX = new SAxis();
    pAxisX->SetName(_T("Time (hours)"));
    pAxisX->SetTextColor(RGBA(100, 100, 100, 255));
    pAxisX->SetLineColor(RGBA(150, 150, 150, 255));
    pAxisX->SetHasLines(TRUE);

    // Add X axis values (0, 2, 4, 6, 8)
    for (int i = 0; i <= 8; i += 2)
    {
        SStringT label;
        label.Format(_T("%dh"), i);
        SAxisValue* pValue = new SAxisValue((float)i, label);
        pAxisX->AddValue(pValue);
    }
    pData->SetAxisX(pAxisX);

    // Configure Y axis
    SAxis* pAxisY = new SAxis();
    pAxisY->SetName(_T("Value"));
    pAxisY->SetTextColor(RGBA(100, 100, 100, 255));
    pAxisY->SetLineColor(RGBA(150, 150, 150, 255));
    pAxisY->SetHasLines(TRUE);

    // Add Y axis values (0, 2, 4, 6, 8, 10)
    for (int i = 0; i <= 10; i += 2)
    {
        SStringT label;
        label.Format(_T("%.0f"), (float)i);
        SAxisValue* pValue = new SAxisValue((float)i, label);
        pAxisY->AddValue(pValue);
    }
    pData->SetAxisY(pAxisY);

    // Enable axes display
    pData->SetAxesEnabled(TRUE);
    
    // Create first line (blue)
    std::vector<SPointValue*> values1;
    for (int i = 0; i < 10; ++i)
    {
        float x = (float)i;
        // Generate random value between 1.0f and 10.0f
        float y = 1.0f + (float)(rand() % 900) / 100.0f;
        SPointValue* pValue = new SPointValue(x, y);
        values1.push_back(pValue);
    }
    
    SLine* pLine1 = new SLine(values1);
    pLine1->SetColor(RGBA(51, 181, 229,255));  // Blue
    pLine1->SetHasPoints(TRUE);
    pLine1->SetStrokeWidth(3);
    pLine1->SetPointRadius(4);
    pData->AddLine(pLine1);
    
    // Create second line (red)
    std::vector<SPointValue*> values2;
    for (int i = 0; i < 10; ++i)
    {
        float x = (float)i;
        // Generate random value between 1.0f and 10.0f
        float y = 1.0f + (float)(rand() % 900) / 100.0f;
        SPointValue* pValue = new SPointValue(x, y);
        values2.push_back(pValue);
    }
    
    SLine* pLine2 = new SLine(values2);
    pLine2->SetColor(RGBA(255, 87, 87,255));   // Red
    pLine2->SetHasPoints(TRUE);
    pLine2->SetStrokeWidth(3);
    pLine2->SetPointRadius(4);
    pData->AddLine(pLine2);
    
    // Create third line (green, filled)
    std::vector<SPointValue*> values3;
    for (int i = 0; i < 10; ++i)
    {
        float x = (float)i;
        // Generate random value between 0.5f and 5.0f
        float y = 0.5f + (float)(rand() % 450) / 100.0f;
        SPointValue* pValue = new SPointValue(x, y);
        values3.push_back(pValue);
    }
    
    SLine* pLine3 = new SLine(values3);
    pLine3->SetColor(RGBA(76, 175, 80,255));   // Green
    pLine3->SetHasPoints(TRUE);
    pLine3->SetFilled(TRUE);
    pLine3->SetAreaTransparency(128);
    pLine3->SetStrokeWidth(2);
    pLine3->SetPointRadius(3);
    pData->AddLine(pLine3);
    
    // Set chart data
    m_pLineChart->SetLineChartData(pData);
    
    // Start animation if enabled
    if (m_bAnimationEnabled)
    {
        m_pLineChart->StartDataAnimation(500);
    }
}

void CMainDlg::GenerateColumnChartData()
{
    // Create new column chart data
    SColumnChartData* pData = new SColumnChartData();

    // Configure X axis
    SAxis* pAxisX = new SAxis();
    pAxisX->SetName(_T("Categories"));
    pAxisX->SetTextColor(RGBA(100, 100, 100, 255));
    pAxisX->SetLineColor(RGBA(150, 150, 150, 255));
    pAxisX->SetHasLines(TRUE);

    // Add X axis values (Category A, B, C, D, E, F)
    SStringT categories[] = {_T("A"), _T("B"), _T("C"), _T("D"), _T("E"), _T("F")};
    for (int i = 0; i < 6; ++i)
    {
        SAxisValue* pValue = new SAxisValue((float)i, categories[i]);
        pAxisX->AddValue(pValue);
    }
    pData->SetAxisX(pAxisX);

    // Configure Y axis
    SAxis* pAxisY = new SAxis();
    pAxisY->SetName(_T("Values"));
    pAxisY->SetTextColor(RGBA(100, 100, 100, 255));
    pAxisY->SetLineColor(RGBA(150, 150, 150, 255));
    pAxisY->SetHasLines(TRUE);

    // Add Y axis values (0, 20, 40, 60, 80, 100)
    for (int i = 0; i <= 100; i += 20)
    {
        SStringT label;
        label.Format(_T("%d"), i);
        SAxisValue* pValue = new SAxisValue((float)i, label);
        pAxisY->AddValue(pValue);
    }
    pData->SetAxisY(pAxisY);

    // Enable axes display
    pData->SetAxesEnabled(TRUE);

    // Create 6 columns with single subcolumn each
    for (int i = 0; i < 6; ++i)
    {
        std::vector<SSubcolumnValue*> values;
        // Generate values between 20 and 100 for better visibility
        float value = 20.0f + (float)(rand() % 8000) / 100.0f;
        SSubcolumnValue* pValue = new SSubcolumnValue(value);
        values.push_back(pValue);

        SColumn* pColumn = new SColumn(values);

        // Set different colors for each column
        COLORREF colors[] = {
            RGBA(51, 181, 229, 255),   // Blue
            RGBA(255, 87, 87, 255),    // Red
            RGBA(76, 175, 80, 255),    // Green
            RGBA(255, 193, 7, 255),    // Yellow
            RGBA(156, 39, 176, 255),   // Purple
            RGBA(255, 152, 0, 255)     // Orange
        };
        pColumn->SetColor(colors[i % 6]);
        pColumn->SetHasLabels(TRUE);
        pColumn->SetStrokeWidth(2); // Add stroke for better visibility

        pData->AddColumn(pColumn);
    }

    // Configure chart properties for better display
    pData->SetFillRatio(0.75f); // 75% fill ratio for good spacing

    // Set chart data
    m_pColumnChart->SetColumnChartData(pData);

    // Start animation if enabled
    if (m_bAnimationEnabled)
    {
        m_pColumnChart->StartDataAnimation(500);
    }

    // TODO: Update statistics display
    // UpdateChartStatistics();
}

void CMainDlg::ClearChartData()
{
    // Clear line chart data
    if (m_pLineChart)
    {
        SLineChartData* pLineData = new SLineChartData();
        m_pLineChart->SetLineChartData(pLineData);
    }

    // Clear column chart data
    if (m_pColumnChart)
    {
        SColumnChartData* pColumnData = new SColumnChartData();
        m_pColumnChart->SetColumnChartData(pColumnData);
    }

    if(m_pPieChart)
    {
        SPieChartData* pPieData = new SPieChartData();
        m_pPieChart->SetPieChartData(pPieData);
    }
    if(m_pBubbleChart)
    {
        SBubbleChartData* pBubbleData = new SBubbleChartData();
        m_pBubbleChart->SetBubbleChartData(pBubbleData);
    }
    if(m_pRadarChart)
    {
        SRadarChartData* pRadarData = new SRadarChartData();
        m_pRadarChart->SetRadarChartData(pRadarData);
    }
    if(m_pComboChart)
    {
        SComboChartData* pComboData = new SComboChartData();
        m_pComboChart->SetComboChartData(pComboData);
    }
    // Update status
    SWindow* pStatus = FindChildByName(L"txt_status");
    if (pStatus)
    {
        pStatus->SetWindowText(_T("Chart data cleared"));
    }
}

void CMainDlg::OnChartValueSelected(int lineIndex, int pointIndex, SPointValue* pValue)
{
    if (!pValue)
        return;
        
    SStringT status;
    status.Format(_T("Selected: Line %d, Point %d, Value (%.2f, %.2f)"), 
                  lineIndex, pointIndex, pValue->GetX(), pValue->GetY());
    
    SWindow* pStatus = FindChildByName(L"txt_status");
    if (pStatus)
    {
        pStatus->SetWindowText(status);
    }
}

void CMainDlg::OnChartValueDeselected()
{
    SWindow* pStatus = FindChildByName(L"txt_status");
    if (pStatus)
    {
        pStatus->SetWindowText(_T("No selection"));
    }
}

void CMainDlg::OnColumnValueSelected(int columnIndex, int subcolumnIndex, SSubcolumnValue* pValue)
{
    if (!pValue)
        return;

    // Update status bar
    SStringT status;
    status.Format(_T("Selected: Column %d, Value %.1f"), columnIndex + 1, pValue->GetY());

    SWindow* pStatus = FindChildByName(_T("txt_status"));
    if (pStatus)
    {
        pStatus->SetWindowText(status);
    }

    // Update selection info panel
    SStringT selectionInfo;
    selectionInfo.Format(_T("Column Chart Selection:\n\nColumn Index: %d\nSubcolumn Index: %d\nValue: %.2f\nColor: RGB(%d,%d,%d)\n\nClick elsewhere to deselect."),
                        columnIndex + 1,
                        subcolumnIndex + 1,
                        pValue->GetY(),
                        GetRValue(pValue->GetColor()),
                        GetGValue(pValue->GetColor()),
                        GetBValue(pValue->GetColor()));

    SWindow* pSelectionInfo = FindChildByName(_T("txt_selection_info"));
    if (pSelectionInfo)
    {
        pSelectionInfo->SetWindowText(selectionInfo);
    }
}

void CMainDlg::OnColumnValueDeselected()
{
    SWindow* pStatus = FindChildByName(_T("txt_status"));
    if (pStatus)
    {
        pStatus->SetWindowText(_T("Ready"));
    }

    // Clear selection info
    SWindow* pSelectionInfo = FindChildByName(_T("txt_selection_info"));
    if (pSelectionInfo)
    {
        pSelectionInfo->SetWindowText(_T("Click on chart elements to see selection details here."));
    }
}



void CMainDlg::GeneratePieChartData()
{
    // Create new pie chart data
    SPieChartData* pData = new SPieChartData();

    // Create test data with mixed large and small slices to test intelligent labeling
    struct SliceData {
        SStringT label;
        float minValue;
        float maxValue;
        COLORREF color;
    };

    // Mix of large, medium, and small slices for comprehensive testing
    SliceData sliceData[] = {
        // Large slices (will show labels inside)
        {_T("Technology"), 25.0f, 35.0f, RGBA(51, 181, 229, 255)},    // ~30% - Large
        {_T("Healthcare"), 20.0f, 25.0f, RGBA(255, 87, 87, 255)},     // ~22% - Large
        {_T("Finance"), 15.0f, 20.0f, RGBA(76, 175, 80, 255)},        // ~17% - Medium

        // Medium slices (will show percentage inside)
        {_T("Education"), 8.0f, 12.0f, RGBA(255, 193, 7, 255)},       // ~10% - Medium
        {_T("Retail"), 6.0f, 10.0f, RGBA(156, 39, 176, 255)},         // ~8% - Medium

        // Small slices (will show external labels or legend)
        {_T("Manufacturing"), 2.0f, 4.0f, RGBA(255, 152, 0, 255)},    // ~3% - Small
        {_T("Agriculture"), 1.5f, 3.0f, RGBA(96, 125, 139, 255)},     // ~2% - Small
        {_T("Mining"), 1.0f, 2.5f, RGBA(121, 85, 72, 255)},           // ~1.5% - Very Small
        {_T("Utilities"), 0.8f, 2.0f, RGBA(158, 158, 158, 255)},      // ~1.2% - Very Small
        {_T("Transportation"), 0.5f, 1.5f, RGBA(233, 30, 99, 255)},   // ~1% - Very Small
        {_T("Real Estate"), 0.3f, 1.0f, RGBA(103, 58, 183, 255)},     // ~0.6% - Tiny
        {_T("Media"), 0.2f, 0.8f, RGBA(255, 87, 34, 255)}             // ~0.5% - Tiny
    };

    int sliceCount = sizeof(sliceData) / sizeof(SliceData);

    for (int i = 0; i < sliceCount; ++i)
    {
        // Generate value within the specified range for each slice type
        float range = sliceData[i].maxValue - sliceData[i].minValue;
        // Generate random value between 0 and 1
        float randomValue = (float)(rand() % 100) / 100.0f;
        float value = sliceData[i].minValue + randomValue * range;

        SSliceValue* pValue = new SSliceValue(value, sliceData[i].color, sliceData[i].label);
        pData->AddValue(pValue);
    }

    // Configure pie chart properties
    pData->SetHasLabels(TRUE);
    pData->SetHasLabelLines(TRUE); // Enable label lines for small slices
    pData->SetCenterCircleRatio(0.0f); // Full pie (not donut)
    pData->SetSliceSpacing(1.0f); // Small spacing between slices

    m_pPieChart->SetPieChartData(pData);

    // Start animation if enabled
    if (m_bAnimationEnabled)
    {
        m_pPieChart->StartDataAnimation(500);
    }
}

void CMainDlg::OnPieValueSelected(int sliceIndex, SSliceValue* pValue)
{
    if (!pValue)
        return;

    // Update status bar
    SStringT status;
    status.Format(_T("Selected: %s, Value %.1f"), pValue->GetLabel().GetBuffer(), pValue->GetValue());

    SWindow* pStatus = FindChildByName(_T("txt_status"));
    if (pStatus)
    {
        pStatus->SetWindowText(status);
    }

    // Update selection info panel
    SStringT selectionInfo;
    selectionInfo.Format(_T("Pie Chart Selection:\n\nSlice: %s\nIndex: %d\nValue: %.2f\nPercentage: %.1f%%\nColor: RGB(%d,%d,%d)\n\nClick elsewhere to deselect."),
                        pValue->GetLabel().GetBuffer(),
                        sliceIndex + 1,
                        pValue->GetValue(),
                        pValue->GetValue() * 100.0f / GetTotalPieValue(), // Calculate percentage
                        GetRValue(pValue->GetColor()),
                        GetGValue(pValue->GetColor()),
                        GetBValue(pValue->GetColor()));

    SWindow* pSelectionInfo = FindChildByName(_T("txt_selection_info"));
    if (pSelectionInfo)
    {
        pSelectionInfo->SetWindowText(selectionInfo);
    }
}

void CMainDlg::OnPieValueDeselected()
{
    SWindow* pStatus = FindChildByName(_T("txt_status"));
    if (pStatus)
    {
        pStatus->SetWindowText(_T("Ready"));
    }

    // Clear selection info
    SWindow* pSelectionInfo = FindChildByName(_T("txt_selection_info"));
    if (pSelectionInfo)
    {
        pSelectionInfo->SetWindowText(_T("Click on chart elements to see selection details here."));
    }
}

float CMainDlg::GetTotalPieValue()
{
    if (!m_pPieChart)
        return 1.0f;

    SPieChartData* pData = m_pPieChart->GetPieChartData();
    if (!pData)
        return 1.0f;

    float total = 0.0f;
    for (size_t i = 0; i < pData->GetValueCount(); ++i)
    {
        SSliceValue* pValue = pData->GetValue(i);
        if (pValue)
        {
            total += pValue->GetValue();
        }
    }

    return total > 0.0f ? total : 1.0f;
}

void CMainDlg::OnTabChartSelChanged(IEvtArgs *pEvt)
{
    EventTabSelChanged *e2= sobj_cast<EventTabSelChanged>(pEvt);
    STabCtrl *pTab = sobj_cast<STabCtrl>(e2->Sender());
    SWindow *pPage = pTab->GetItem(e2->uNewSel);
    SAbstractChartView *pChart = sobj_cast<SAbstractChartView>(pPage->GetWindow(GSW_FIRSTCHILD));
    if(pChart){
        pChart->StartDataAnimation(500);
    }
}

void CMainDlg::GenerateBubbleChartData()
{
    if (!m_pBubbleChart)
        return;

    // Create new bubble chart data
    SBubbleChartData* pData = new SBubbleChartData();

    // Generate sample bubble data representing different companies
    struct BubbleInfo {
        SStringT label;
        float x; // Revenue (millions)
        float y; // Profit margin (%)
        float z; // Market cap (billions)
        COLORREF color;
    };

    BubbleInfo bubbles[] = {
        // Tech companies
        {_T("Apple"), 365.0f, 25.0f, 2800.0f, RGBA(51, 181, 229, 255)},
        {_T("Microsoft"), 168.0f, 35.0f, 2400.0f, RGBA(255, 87, 87, 255)},
        {_T("Google"), 257.0f, 22.0f, 1600.0f, RGBA(76, 175, 80, 255)},
        {_T("Amazon"), 469.0f, 6.0f, 1500.0f, RGBA(255, 193, 7, 255)},

        // Traditional companies
        {_T("Walmart"), 559.0f, 2.8f, 400.0f, RGBA(156, 39, 176, 255)},
        {_T("ExxonMobil"), 285.0f, 8.0f, 350.0f, RGBA(255, 152, 0, 255)},
        {_T("Berkshire"), 245.0f, 12.0f, 550.0f, RGBA(96, 125, 139, 255)},

        // Emerging companies
        {_T("Tesla"), 53.0f, 5.0f, 800.0f, RGBA(121, 85, 72, 255)},
        {_T("Netflix"), 25.0f, 18.0f, 200.0f, RGBA(158, 158, 158, 255)},
        {_T("Zoom"), 4.0f, 27.0f, 120.0f, RGBA(233, 30, 99, 255)},

        // Small companies
        {_T("Startup A"), 2.0f, 15.0f, 5.0f, RGBA(103, 58, 183, 255)},
        {_T("Startup B"), 1.5f, 8.0f, 3.0f, RGBA(255, 87, 34, 255)},
        {_T("Startup C"), 0.8f, 12.0f, 2.0f, RGBA(0, 150, 136, 255)}
    };

    for (int i = 0; i < 13; ++i)
    {
        SBubbleValue* pValue = new SBubbleValue(bubbles[i].x, bubbles[i].y, bubbles[i].z,
                                               bubbles[i].color, bubbles[i].label);
        pData->AddValue(pValue);
    }

    // Configure bubble chart properties
    pData->SetHasBubbleLabels(TRUE);
    pData->SetMinBubbleRadius(8);
    pData->SetMaxBubbleRadius(60);
    pData->SetBubbleScale(1); // Area scale for better visual representation

    // Configure axes
    SAxis* pAxisX = new SAxis();
    pAxisX->SetName(_T("Revenue (Millions $)"));
    pAxisX->SetTextColor(RGBA(100, 100, 100, 255));
    pAxisX->SetLineColor(RGBA(150, 150, 150, 255));
    pAxisX->SetHasLines(TRUE);
    pData->SetAxisX(pAxisX);

    SAxis* pAxisY = new SAxis();
    pAxisY->SetName(_T("Profit Margin (%)"));
    pAxisY->SetTextColor(RGBA(100, 100, 100, 255));
    pAxisY->SetLineColor(RGBA(150, 150, 150, 255));
    pAxisY->SetHasLines(TRUE);
    pData->SetAxisY(pAxisY);

    // Add axis values for better reference
    for (int i = 0; i <= 600; i += 100)
    {
        SStringT label;
        label.Format(_T("%d"), i);
        pAxisX->AddValue(new SAxisValue((float)i, label));
    }

    for (int i = 0; i <= 40; i += 10)
    {
        SStringT label;
        label.Format(_T("%d%%"), i);
        pAxisY->AddValue(new SAxisValue((float)i, label));
    }

    pData->SetAxesEnabled(TRUE);

    m_pBubbleChart->SetBubbleChartData(pData);

    // Start animation if enabled
    if (m_bAnimationEnabled)
    {
        m_pBubbleChart->StartDataAnimation(800);
    }
}

void CMainDlg::OnBubbleValueSelected(int bubbleIndex, SBubbleValue* pValue)
{
    if (!pValue)
        return;

    // Update status bar
    SStringT status;
    status.Format(_T("Selected: %s"), pValue->GetLabel().GetBuffer());

    SWindow* pStatus = FindChildByName(_T("txt_status"));
    if (pStatus)
    {
        pStatus->SetWindowText(status);
    }

    // Update selection info panel
    SStringT selectionInfo;
    selectionInfo.Format(_T("Bubble Chart Selection:\n\nCompany: %s\nIndex: %d\nRevenue: $%.1fM\nProfit Margin: %.1f%%\nMarket Cap: $%.1fB\nColor: RGB(%d,%d,%d)\n\nBubble size represents market capitalization.\nClick elsewhere to deselect."),
                        pValue->GetLabel().GetBuffer(),
                        bubbleIndex + 1,
                        pValue->GetX(),
                        pValue->GetY(),
                        pValue->GetZ(),
                        GetRValue(pValue->GetColor()),
                        GetGValue(pValue->GetColor()),
                        GetBValue(pValue->GetColor()));

    SWindow* pSelectionInfo = FindChildByName(_T("txt_selection_info"));
    if (pSelectionInfo)
    {
        pSelectionInfo->SetWindowText(selectionInfo);
    }
}

void CMainDlg::OnBubbleValueDeselected()
{
    SWindow* pStatus = FindChildByName(_T("txt_status"));
    if (pStatus)
    {
        pStatus->SetWindowText(_T("Ready"));
    }

    // Clear selection info
    SWindow* pSelectionInfo = FindChildByName(_T("txt_selection_info"));
    if (pSelectionInfo)
    {
        pSelectionInfo->SetWindowText(_T("Click on chart elements to see selection details here."));
    }
}



void CMainDlg::GenerateRadarChartData()
{
    if (!m_pRadarChart)
        return;

    // Create new radar chart data
    SRadarChartData* pData = new SRadarChartData();

    // Define skill categories for comparison
    pData->AddAxisLabel(_T("Speed"));
    pData->AddAxisLabel(_T("Strength"));
    pData->AddAxisLabel(_T("Intelligence"));
    pData->AddAxisLabel(_T("Endurance"));
    pData->AddAxisLabel(_T("Agility"));
    pData->AddAxisLabel(_T("Accuracy"));

    // Create different character profiles for comparison
    struct CharacterProfile {
        SStringT name;
        float speed;
        float strength;
        float intelligence;
        float endurance;
        float agility;
        float accuracy;
        COLORREF color;
    };

    CharacterProfile profiles[] = {
        // Warrior: High strength and endurance, low intelligence
        {_T("Warrior"), 60.0f, 95.0f, 40.0f, 90.0f, 55.0f, 70.0f, RGBA(255, 87, 87, 255)},

        // Mage: High intelligence, low strength and endurance
        {_T("Mage"), 45.0f, 30.0f, 98.0f, 50.0f, 60.0f, 85.0f, RGBA(51, 181, 229, 255)},

        // Archer: High accuracy and agility, balanced other stats
        {_T("Archer"), 75.0f, 60.0f, 70.0f, 65.0f, 90.0f, 95.0f, RGBA(76, 175, 80, 255)},

        // Assassin: High speed and agility, low strength
        {_T("Assassin"), 95.0f, 45.0f, 75.0f, 60.0f, 98.0f, 80.0f, RGBA(156, 39, 176, 255)},

        // Paladin: Balanced high stats, jack of all trades
        {_T("Paladin"), 70.0f, 80.0f, 75.0f, 85.0f, 65.0f, 75.0f, RGBA(255, 193, 7, 255)}
    };

    for (int i = 0; i < 5; ++i)
    {
        SRadarValue* pValue = new SRadarValue(profiles[i].name, profiles[i].color);

        // Add axis values in the same order as axis labels
        pValue->AddAxisValue(profiles[i].speed);
        pValue->AddAxisValue(profiles[i].strength);
        pValue->AddAxisValue(profiles[i].intelligence);
        pValue->AddAxisValue(profiles[i].endurance);
        pValue->AddAxisValue(profiles[i].agility);
        pValue->AddAxisValue(profiles[i].accuracy);

        pData->AddValue(pValue);
    }

    // Configure radar chart properties
    pData->SetHasRadarLabels(TRUE);
    pData->SetHasWebLines(TRUE);
    pData->SetWebRings(5);
    pData->SetMaxValue(100.0f); // All stats are 0-100

    m_pRadarChart->SetRadarChartData(pData);

    // Start animation if enabled
    if (m_bAnimationEnabled)
    {
        m_pRadarChart->StartDataAnimation(1000);
    }
}

void CMainDlg::OnRadarValueSelected(int seriesIndex, SRadarValue* pValue)
{
    if (!pValue)
        return;

    // Update status bar
    SStringT status;
    status.Format(_T("Selected: %s"), pValue->GetLabel().GetBuffer());

    SWindow* pStatus = FindChildByName(_T("txt_status"));
    if (pStatus)
    {
        pStatus->SetWindowText(status);
    }

    // Update selection info panel
    SStringT selectionInfo;
    selectionInfo.Format(_T("Radar Chart Selection:\n\nCharacter: %s\nSeries: %d\n\nAttributes:\nSpeed: %.0f\nStrength: %.0f\nIntelligence: %.0f\nEndurance: %.0f\nAgility: %.0f\nAccuracy: %.0f\n\nColor: RGB(%d,%d,%d)\n\nClick elsewhere to deselect."),
                        pValue->GetLabel().GetBuffer(),
                        seriesIndex + 1,
                        pValue->GetAxisValue(0), // Speed
                        pValue->GetAxisValue(1), // Strength
                        pValue->GetAxisValue(2), // Intelligence
                        pValue->GetAxisValue(3), // Endurance
                        pValue->GetAxisValue(4), // Agility
                        pValue->GetAxisValue(5), // Accuracy
                        GetRValue(pValue->GetColor()),
                        GetGValue(pValue->GetColor()),
                        GetBValue(pValue->GetColor()));

    SWindow* pSelectionInfo = FindChildByName(_T("txt_selection_info"));
    if (pSelectionInfo)
    {
        pSelectionInfo->SetWindowText(selectionInfo);
    }
}

void CMainDlg::OnRadarValueDeselected()
{
    SWindow* pStatus = FindChildByName(_T("txt_status"));
    if (pStatus)
    {
        pStatus->SetWindowText(_T("Ready"));
    }

    // Clear selection info
    SWindow* pSelectionInfo = FindChildByName(_T("txt_selection_info"));
    if (pSelectionInfo)
    {
        pSelectionInfo->SetWindowText(_T("Click on chart elements to see selection details here."));
    }
}



void CMainDlg::GenerateComboChartData()
{
    if (!m_pComboChart)
        return;

    // Create new combo chart data
    SComboChartData* pData = new SComboChartData();

    // Generate line chart data (temperature trend)
    SLineChartData* pLineData = new SLineChartData();
   // Temperature data for 12 months
    float temperatures[] = {-2.0f, 1.0f, 8.0f, 15.0f, 22.0f, 28.0f,
                           32.0f, 30.0f, 25.0f, 18.0f, 10.0f, 3.0f};
    std::vector<SPointValue*> values;
    for (int i = 0; i < ARRAYSIZE(temperatures); ++i)
    {
        SPointValue* pPoint = new SPointValue((float)i, temperatures[i]);
        pPoint->SetColor(RGBA(255, 87, 87, 255));
        values.push_back(pPoint);
    }
    SLine* pTempLine = new SLine(values);
    pTempLine->SetColor(RGBA(255, 87, 87, 255));
    pTempLine->SetStrokeWidth(3);
    pTempLine->SetHasPoints(TRUE);
    pTempLine->SetAreaTransparency(50);
    pTempLine->SetFilled(FALSE);
    pLineData->AddLine(pTempLine);

    // Configure line chart axes
    SAxis* pLineAxisX = new SAxis();
    pLineAxisX->SetName(_T("Month"));
    pLineAxisX->SetTextColor(RGBA(100, 100, 100, 255));
    pLineAxisX->SetLineColor(RGBA(150, 150, 150, 255));
    pLineAxisX->SetHasLines(TRUE);

    SStringT months[] = {_T("Jan"), _T("Feb"), _T("Mar"), _T("Apr"), _T("May"), _T("Jun"),
                        _T("Jul"), _T("Aug"), _T("Sep"), _T("Oct"), _T("Nov"), _T("Dec")};
    for (int i = 0; i < 12; ++i)
    {
        pLineAxisX->AddValue(new SAxisValue((float)i, months[i]));
    }

    SAxis* pLineAxisY = new SAxis();
    pLineAxisY->SetName(_T("Temperature (°C)"));
    pLineAxisY->SetTextColor(RGBA(255, 87, 87, 255));
    pLineAxisY->SetLineColor(RGBA(255, 87, 87, 255));
    pLineAxisY->SetHasLines(TRUE);

    for (int i = -10; i <= 40; i += 10)
    {
        SStringT label;
        label.Format(_T("%d°C"), i);
        pLineAxisY->AddValue(new SAxisValue((float)i, label));
    }

    pLineData->SetAxisX(pLineAxisX);
    pLineData->SetAxisY(pLineAxisY);
    pLineData->SetAxesEnabled(TRUE);

    // Generate column chart data (rainfall)
    SColumnChartData* pColumnData = new SColumnChartData();

    // Rainfall data for 12 months
    float rainfall[] = {45.0f, 38.0f, 52.0f, 68.0f, 85.0f, 120.0f,
                       135.0f, 128.0f, 95.0f, 72.0f, 58.0f, 48.0f};

    for (int i = 0; i < 12; ++i)
    {
        std::vector<SSubcolumnValue*> values;
        SSubcolumnValue* pValue = new SSubcolumnValue(rainfall[i]);
        pValue->SetColor(RGBA(51, 181, 229, 200)); // Semi-transparent blue
        values.push_back(pValue);
        SColumn* pColumn = new SColumn(values);
        pColumnData->AddColumn(pColumn);
    }

    // Configure column chart axes (reuse X axis, create separate Y axis)
    SAxis* pColumnAxisX = new SAxis(*pLineAxisX); // Copy X axis
    SAxis* pColumnAxisY = new SAxis();
    pColumnAxisY->SetName(_T("Rainfall (mm)"));
    pColumnAxisY->SetTextColor(RGBA(51, 181, 229, 255));
    pColumnAxisY->SetLineColor(RGBA(51, 181, 229, 255));
    pColumnAxisY->SetHasLines(FALSE); // Don't show grid lines for secondary axis

    for (int i = 0; i <= 150; i += 30)
    {
        SStringT label;
        label.Format(_T("%dmm"), i);
        pColumnAxisY->AddValue(new SAxisValue((float)i, label));
    }

    pColumnData->SetAxisX(pColumnAxisX);
    pColumnData->SetAxisY(pColumnAxisY);
    pColumnData->SetAxesEnabled(TRUE);

    // Set up combo chart
    pData->SetLineChartData(pLineData);
    pData->SetColumnChartData(pColumnData);
    pData->SetComboType(0); // Columns behind lines
    pData->SetUseSeparateYAxes(TRUE);
    pData->SetSecondaryAxisY(new SAxis(*pColumnAxisY));
    pData->SetSecondaryAxisOnRight(TRUE);
    pData->SetLineChartAlpha(1.0f);
    pData->SetColumnChartAlpha(0.7f);

    // Use the line chart's axes as primary
    pData->SetAxisX(new SAxis(*pLineAxisX));
    pData->SetAxisY(new SAxis(*pLineAxisY));
    pData->SetAxesEnabled(TRUE);

    m_pComboChart->SetComboChartData(pData);

    // Start animation if enabled
    if (m_bAnimationEnabled)
    {
        m_pComboChart->StartDataAnimation(1200);
    }
}

void CMainDlg::OnComboValueSelected(int chartType, int valueIndex, int subValueIndex)
{
    // Update status bar
    SStringT status;
    SStringT chartTypeName = (chartType == 0) ? _T("Temperature Line") : _T("Rainfall Column");
    status.Format(_T("Selected: %s"), chartTypeName.GetBuffer());

    SWindow* pStatus = FindChildByName(_T("txt_status"));
    if (pStatus)
    {
        pStatus->SetWindowText(status);
    }

    // Update selection info panel
    SStringT selectionInfo;

    if (chartType == 0) // Line chart (temperature)
    {
        SStringT months[] = {_T("Jan"), _T("Feb"), _T("Mar"), _T("Apr"), _T("May"), _T("Jun"),
                            _T("Jul"), _T("Aug"), _T("Sep"), _T("Oct"), _T("Nov"), _T("Dec")};
        float temperatures[] = {-2.0f, 1.0f, 8.0f, 15.0f, 22.0f, 28.0f,
                               32.0f, 30.0f, 25.0f, 18.0f, 10.0f, 3.0f};

        if (valueIndex >= 0 && valueIndex < 12)
        {
            selectionInfo.Format(_T("Combo Chart Selection:\n\nChart Type: Temperature Line\nMonth: %s\nPoint Index: %d\nTemperature: %.1f°C\n\nThis line shows the average monthly temperature throughout the year.\n\nClick elsewhere to deselect."),
                                months[valueIndex].GetBuffer(),
                                valueIndex + 1,
                                temperatures[valueIndex]);
        }
    }
    else // Column chart (rainfall)
    {
        SStringT months[] = {_T("Jan"), _T("Feb"), _T("Mar"), _T("Apr"), _T("May"), _T("Jun"),
                            _T("Jul"), _T("Aug"), _T("Sep"), _T("Oct"), _T("Nov"), _T("Dec")};
        float rainfall[] = {45.0f, 38.0f, 52.0f, 68.0f, 85.0f, 120.0f,
                           135.0f, 128.0f, 95.0f, 72.0f, 58.0f, 48.0f};

        if (valueIndex >= 0 && valueIndex < 12)
        {
            selectionInfo.Format(_T("Combo Chart Selection:\n\nChart Type: Rainfall Column\nMonth: %s\nColumn Index: %d\nRainfall: %.0fmm\n\nThis column shows the average monthly rainfall throughout the year.\n\nClick elsewhere to deselect."),
                                months[valueIndex].GetBuffer(),
                                valueIndex + 1,
                                rainfall[valueIndex]);
        }
    }

    SWindow* pSelectionInfo = FindChildByName(_T("txt_selection_info"));
    if (pSelectionInfo)
    {
        pSelectionInfo->SetWindowText(selectionInfo);
    }
}

void CMainDlg::OnComboValueDeselected()
{
    SWindow* pStatus = FindChildByName(_T("txt_status"));
    if (pStatus)
    {
        pStatus->SetWindowText(_T("Ready"));
    }

    // Clear selection info
    SWindow* pSelectionInfo = FindChildByName(_T("txt_selection_info"));
    if (pSelectionInfo)
    {
        pSelectionInfo->SetWindowText(_T("Click on chart elements to see selection details here."));
    }
}



void CMainDlg::OnBtnTestPieScenarios()
{
    if (!m_pPieChart)
        return;

    // Cycle through different test scenarios
    m_currentPieScenario = (m_currentPieScenario + 1) % 4;
    GenerateTestPieData(m_currentPieScenario);

    // Update status to show current scenario
    SWindow* pStatus = FindChildByName(_T("txt_status"));
    if (pStatus)
    {
        SStringT statusText;
        switch (m_currentPieScenario)
        {
        case 0:
            statusText = _T("Scenario 1: Mixed Large & Small Slices");
            break;
        case 1:
            statusText = _T("Scenario 2: Many Small Slices (Legend Mode)");
            break;
        case 2:
            statusText = _T("Scenario 3: Few Large Slices Only");
            break;
        case 3:
            statusText = _T("Scenario 4: Extreme Small Slices Test");
            break;
        }
        pStatus->SetWindowText(statusText);
    }
}

void CMainDlg::GenerateTestPieData(int scenario)
{
    if (!m_pPieChart)
        return;

    SPieChartData* pData = new SPieChartData();

    switch (scenario)
    {
    case 0: // Mixed large and small slices
        {
            struct SliceInfo { SStringT label; float value; COLORREF color; };
            SliceInfo slices[] = {
                {_T("Technology"), 35.0f, RGBA(51, 181, 229, 255)},
                {_T("Healthcare"), 25.0f, RGBA(255, 87, 87, 255)},
                {_T("Finance"), 15.0f, RGBA(76, 175, 80, 255)},
                {_T("Education"), 10.0f, RGBA(255, 193, 7, 255)},
                {_T("Retail"), 8.0f, RGBA(156, 39, 176, 255)},
                {_T("Manufacturing"), 3.0f, RGBA(255, 152, 0, 255)},
                {_T("Agriculture"), 2.0f, RGBA(96, 125, 139, 255)},
                {_T("Others"), 2.0f, RGBA(121, 85, 72, 255)}
            };

            for (int i = 0; i < 8; ++i)
            {
                SSliceValue* pValue = new SSliceValue(slices[i].value, slices[i].color, slices[i].label);
                pData->AddValue(pValue);
            }
        }
        break;

    case 1: // Many small slices (should trigger legend mode)
        {
            SStringT labels[] = {_T("Tech"), _T("Health"), _T("Finance"), _T("Edu"), _T("Retail"),
                               _T("Mfg"), _T("Agri"), _T("Mining"), _T("Util"), _T("Trans"),
                               _T("Real Estate"), _T("Media"), _T("Tourism"), _T("Food"), _T("Energy")};
            COLORREF colors[] = {
                RGBA(51, 181, 229, 255), RGBA(255, 87, 87, 255), RGBA(76, 175, 80, 255),
                RGBA(255, 193, 7, 255), RGBA(156, 39, 176, 255), RGBA(255, 152, 0, 255),
                RGBA(96, 125, 139, 255), RGBA(121, 85, 72, 255), RGBA(158, 158, 158, 255),
                RGBA(233, 30, 99, 255), RGBA(103, 58, 183, 255), RGBA(255, 87, 34, 255),
                RGBA(0, 150, 136, 255), RGBA(205, 220, 57, 255), RGBA(255, 235, 59, 255)
            };

            // Create many small slices
            float values[] = {20.0f, 15.0f, 12.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.5f, 3.0f, 2.5f, 2.0f, 1.5f, 1.0f, 0.5f};

            for (int i = 0; i < 15; ++i)
            {
                SSliceValue* pValue = new SSliceValue(values[i], colors[i], labels[i]);
                pData->AddValue(pValue);
            }
        }
        break;

    case 2: // Few large slices only
        {
            struct SliceInfo { SStringT label; float value; COLORREF color; };
            SliceInfo slices[] = {
                {_T("Primary Sector"), 45.0f, RGBA(51, 181, 229, 255)},
                {_T("Secondary Sector"), 35.0f, RGBA(255, 87, 87, 255)},
                {_T("Tertiary Sector"), 20.0f, RGBA(76, 175, 80, 255)}
            };

            for (int i = 0; i < 3; ++i)
            {
                SSliceValue* pValue = new SSliceValue(slices[i].value, slices[i].color, slices[i].label);
                pData->AddValue(pValue);
            }
        }
        break;

    case 3: // Extreme small slices test
        {
            SStringT labels[] = {_T("Major"), _T("Medium"), _T("Tiny1"), _T("Tiny2"), _T("Tiny3"),
                               _T("Micro1"), _T("Micro2"), _T("Micro3"), _T("Micro4"), _T("Nano")};
            COLORREF colors[] = {
                RGBA(51, 181, 229, 255), RGBA(255, 87, 87, 255), RGBA(76, 175, 80, 255),
                RGBA(255, 193, 7, 255), RGBA(156, 39, 176, 255), RGBA(255, 152, 0, 255),
                RGBA(96, 125, 139, 255), RGBA(121, 85, 72, 255), RGBA(158, 158, 158, 255),
                RGBA(233, 30, 99, 255)
            };

            // One large, one medium, many tiny slices
            float values[] = {70.0f, 20.0f, 2.0f, 1.8f, 1.5f, 1.2f, 1.0f, 0.8f, 0.5f, 0.2f};

            for (int i = 0; i < 10; ++i)
            {
                SSliceValue* pValue = new SSliceValue(values[i], colors[i], labels[i]);
                pData->AddValue(pValue);
            }
        }
        break;
    }

    // Configure pie chart properties
    pData->SetHasLabels(TRUE);
    pData->SetHasLabelLines(TRUE);
    pData->SetCenterCircleRatio(0.0f);
    pData->SetSliceSpacing(1.0f);

    m_pPieChart->SetPieChartData(pData);

    // Start animation if enabled
    if (m_bAnimationEnabled)
    {
        m_pPieChart->StartDataAnimation(500);
    }
}

void CMainDlg::UpdateChartStatistics()
{
    if (!m_pColumnChart)
        return;

    SColumnChartData* pData = m_pColumnChart->GetColumnChartData();
    if (!pData || pData->GetColumnCount() == 0)
    {
        return;
    }

    // Simple statistics display
    SStringT statsInfo;
    statsInfo.Format(_T("Column Chart loaded with %d columns.\nClick on columns to see details."),
                    (int)pData->GetColumnCount());

    SWindow* pInfo = FindChildByName(_T("txt_selection_info"));
    if (pInfo)
    {
        pInfo->SetWindowText(statsInfo);
    }
}

//////////////////////////////////////////////////////////////////////////
// Chart Event Handlers (using SOUI event system)

BOOL CMainDlg::OnLineChartValueSelect(IEvtArgs * e)
{
    SLineChartValueSelectEvent* pEvt = sobj_cast<SLineChartValueSelectEvent>(e);
    if (pEvt)
    {
        if (pEvt->pValue && pEvt->lineIndex >= 0 && pEvt->pointIndex >= 0)
        {
            // 选中事件
            OnChartValueSelected(pEvt->lineIndex, pEvt->pointIndex, pEvt->pValue);
        }
        else
        {
            // 取消选中事件
            OnChartValueDeselected();
        }
    }
    return TRUE;
}

BOOL CMainDlg::OnColumnChartValueSelect(IEvtArgs * e)
{
    SColumnChartValueSelectEvent* pEvt = sobj_cast<SColumnChartValueSelectEvent>(e);
    if (pEvt)
    {
        if (pEvt->pValue && pEvt->columnIndex >= 0 && pEvt->subcolumnIndex >= 0)
        {
            // 选中事件
            OnColumnValueSelected(pEvt->columnIndex, pEvt->subcolumnIndex, pEvt->pValue);
        }
        else
        {
            // 取消选中事件
            OnColumnValueDeselected();
        }
    }
    return TRUE;
}

BOOL CMainDlg::OnPieChartValueSelect(IEvtArgs * e)
{
    SPieChartValueSelectEvent* pEvt = sobj_cast<SPieChartValueSelectEvent>(e);
    if (pEvt)
    {
        if (pEvt->pValue && pEvt->sliceIndex >= 0)
        {
            // 选中事件
            OnPieValueSelected(pEvt->sliceIndex, pEvt->pValue);
        }
        else
        {
            // 取消选中事件
            OnPieValueDeselected();
        }
    }
    return TRUE;
}

BOOL CMainDlg::OnBubbleChartValueSelect(IEvtArgs * e)
{
    SBubbleChartValueSelectEvent* pEvt = sobj_cast<SBubbleChartValueSelectEvent>(e);
    if (pEvt)
    {
        if (pEvt->pValue && pEvt->bubbleIndex >= 0)
        {
            // 选中事件
            OnBubbleValueSelected(pEvt->bubbleIndex, pEvt->pValue);
        }
        else
        {
            // 取消选中事件
            OnBubbleValueDeselected();
        }
    }
    return TRUE;
}

BOOL CMainDlg::OnRadarChartValueSelect(IEvtArgs * e)
{
    SRadarChartValueSelectEvent* pEvt = sobj_cast<SRadarChartValueSelectEvent>(e);
    if (pEvt)
    {
        if (pEvt->pValue && pEvt->valueIndex >= 0)
        {
            // 选中事件
            OnRadarValueSelected(pEvt->valueIndex, pEvt->pValue);
        }
        else
        {
            // 取消选中事件
            OnRadarValueDeselected();
        }
    }
    return TRUE;
}

BOOL CMainDlg::OnComboChartValueSelect(IEvtArgs * e)
{
    SComboChartValueSelectEvent* pEvt = sobj_cast<SComboChartValueSelectEvent>(e);
    if (pEvt)
    {
        if (pEvt->pValue && pEvt->chartType >= 0 && pEvt->valueIndex >= 0)
        {
            // 选中事件
            OnComboValueSelected(pEvt->chartType, pEvt->valueIndex, 0);
        }
        else
        {
            // 取消选中事件
            OnComboValueDeselected();
        }
    }
    return TRUE;
}
