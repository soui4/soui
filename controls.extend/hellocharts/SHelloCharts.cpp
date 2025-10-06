#include "stdafx.h"
#include "SHelloCharts.h"

SNSBEGIN

void SHelloChartsRegister::RegisterControls(SApplication* pApp)
{
    // Register chart controls
    pApp->RegisterWindowClass<SLineChartView>();
    pApp->RegisterWindowClass<SColumnChartView>();
    pApp->RegisterWindowClass<SPieChartView>();
    pApp->RegisterWindowClass<SBubbleChartView>();
    pApp->RegisterWindowClass<SRadarChartView>();
    pApp->RegisterWindowClass<SComboChartView>();
}

SNSEND