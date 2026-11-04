#ifndef EVENTS_H
#define EVENTS_H


// 自定义事件 ID（使用 SOUI 保留给外部事件的区间）
enum FundEvents {
    EVT_WATCH_RESULT = SOUI::EVT_EXTERNAL_BEGIN,  // 某只基金的持仓/净值抓取完成
    EVT_MARKET_RESULT,                            // 行情刷新完成（后台线程 -> UI 线程）
    EVT_INDEX_RESULT,                             // 大盘三大指数抓取完成（与基金列表无关）
};

#endif  // EVENTS_H