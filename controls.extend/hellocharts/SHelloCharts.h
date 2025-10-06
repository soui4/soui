#ifndef __SHELLOCHARTS_H__
#define __SHELLOCHARTS_H__

#include "view/SLineChartView.h"
#include "view/SColumnChartView.h"
#include "view/SPieChartView.h"
#include "view/SBubbleChartView.h"
#include "view/SRadarChartView.h"
#include "view/SComboChartView.h"

#include "model/SAxis.h"

SNSBEGIN

/**
 * @class SHelloChartsRegister
 * @brief Registration helper for HelloCharts controls
 */
class SHelloChartsRegister
{
public:
    /**
     * @brief Register all HelloCharts controls
     */
    static void RegisterControls(SApplication* pApp);
};

SNSEND

#endif // __SHELLOCHARTS_H__