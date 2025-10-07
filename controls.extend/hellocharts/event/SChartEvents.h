#ifndef __SCHARTEVENTS_H__
#define __SCHARTEVENTS_H__

#include <event/SEvents.h>

SNSBEGIN


// 前向声明
class SPointValue;
class SSubcolumnValue;
class SSliceValue;
class SBubbleValue;
class SRadarValue;

#ifndef EVT_CHART_BEGIN
#define EVT_CHART_BEGIN (EVT_EXTERNAL_BEGIN + 2000)
#endif//EVT_CHART_BEGIN

enum{
    EVT_CHART_VIEWPORT_CHANGED = EVT_CHART_BEGIN,
    EVT_CHART_SELECTION_CHANGED,
    EVT_CHART_DATA_ANIMATION_START,
    EVT_CHART_DATA_ANIMATION_END,
    EVT_CHART_DATA_ANIMATION_UPDATE,
    EVT_CHART_LINE_VALUE_SELECT,
    EVT_CHART_COLUMN_VALUE_SELECT,
    EVT_CHART_PIE_VALUE_SELECT,
    EVT_CHART_BUBBLE_VALUE_SELECT,
    EVT_CHART_RADAR_VALUE_SELECT,
    EVT_CHART_COMBO_VALUE_SELECT,
};

DEF_EVT_EXT(SChartViewportChangedEvent, EVT_CHART_VIEWPORT_CHANGED,{
    SViewport viewport;
});

DEF_EVT_EXT(SChartSelectionChangedEvent, EVT_CHART_SELECTION_CHANGED,{
    SSelectedValue selectedValue;
})
DEF_EVT_EXT(SChartDataAnimationStartEvent, EVT_CHART_DATA_ANIMATION_START,{
    DWORD dwDuration;
});
DEF_EVT_EXT(SChartDataAnimationUpdateEvent, EVT_CHART_DATA_ANIMATION_UPDATE,{
    float scale;
});
DEF_EVT_EXT(SChartDataAnimationEndEvent, EVT_CHART_DATA_ANIMATION_END,{
    int dummy;
});

DEF_EVT_EXT(SLineChartValueSelectEvent, EVT_CHART_LINE_VALUE_SELECT,{
    int lineIndex;
    int pointIndex;
    SPointValue* pValue;
});

DEF_EVT_EXT(SColumnChartValueSelectEvent, EVT_CHART_COLUMN_VALUE_SELECT,{
    int columnIndex;
    int subcolumnIndex;
    SSubcolumnValue* pValue;
});

DEF_EVT_EXT(SPieChartValueSelectEvent, EVT_CHART_PIE_VALUE_SELECT,{
    int sliceIndex;
    SSliceValue* pValue;
});

DEF_EVT_EXT(SBubbleChartValueSelectEvent, EVT_CHART_BUBBLE_VALUE_SELECT,{
    int bubbleIndex;
    SBubbleValue* pValue;
});

DEF_EVT_EXT(SRadarChartValueSelectEvent, EVT_CHART_RADAR_VALUE_SELECT,{
    int valueIndex;
    int axisIndex;
    SRadarValue* pValue;
});

DEF_EVT_EXT(SComboChartValueSelectEvent, EVT_CHART_COMBO_VALUE_SELECT,{
    int chartType;  // 0=线图, 1=柱状图
    int valueIndex;
    void* pValue;   // SPointValue* 或 SSubcolumnValue*
});


SNSEND

#endif // __SCHARTEVENTS_H__
