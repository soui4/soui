#pragma once
#include "fund_eastmoney.h"

// 交易时段：用于决定是否自动刷新行情，以及界面徽标文案
enum EMarketSession {
    MS_CLOSED = 0,  // 休市（周末/非交易日）
    MS_PREOPEN,     // 未开盘（09:30 前）
    MS_TRADING,     // 交易中（09:30-11:30 / 13:00-15:00）
    MS_LUNCH,       // 午间休市（11:30-13:00）
    MS_AFTERCLOSE,  // 已收盘（15:00 后）
    MS_SIMULATED,   // 模拟盘中（用户手动开启，用于非交易时段演示）
};

// 个股实时行情（东方财富 push2 接口单条）
struct StockQuote {
    std::string code;  // 6 位代码
    std::string name;  // 名称（UTF-8）
    double price;      // 最新价
    double chgPct;     // 当日涨跌幅(%)
    double preClose;   // 昨收
    bool ok;           // 是否取到有效行情

    StockQuote() : price(0), chgPct(0), preClose(0), ok(false) {}
};

// 大盘指数实时行情（东方财富 push2 接口单条）
// 字段口径对齐 akshare.stock_zh_index_spot_em(symbol="沪深重要指数") 的 rename 表：
//   f2 最新价 / f3 涨跌幅 / f4 涨跌额 / f12 代码 / f14 名称 / f18 昨收
// 区别是这里不拉全量 clist，只按 secid 取沪深三大指数，一次请求即可完成。
struct IndexQuote {
    std::string code;  // 指数代码，如 "000001"
    std::string name;  // 接口返回的名称（UTF-8）；为空时由界面按序号显示内置名称
    double price;      // 最新点位
    double chgPct;     // 涨跌幅(%)
    double chgAmt;     // 涨跌额(点)
    double preClose;   // 昨收
    bool ok;           // 是否取到有效行情

    IndexQuote() : price(0), chgPct(0), chgAmt(0), preClose(0), ok(false) {}
};

// 三大指数的固定清单（纯 ASCII：secid 前缀 1=沪市、0=深市，与 ToSecId 口径一致）。
// 中文显示名放在界面侧用 _T() 定义，避免窄字符串字面量在 MSVC 下被按执行字符集转码。
struct IndexSpec {
    const char *code;   // 6 位代码
    const char *secid;  // push2 secid
};

const IndexSpec *majorIndexSpecs();  // 静态数组，长度为 majorIndexCount()
int majorIndexCount();

// 批量抓取三大指数实时行情。out 恒为 3 项、顺序与 majorIndexSpecs() 一致；
// 未取到的项 ok=false（界面显示占位符）。至少一条有效时返回 true。
bool fetchMajorIndexes(SOUI::IHttpClient *pHttp, std::vector<IndexQuote> &out,
                       std::string &err);

// 基金基础信息（净值接口返回）
struct FundMeta {
    std::string code;     // 基金代码
    std::string name;     // 基金名称（UTF-8）
    std::string navDate;  // 净值日期，如 "2026-11-05"
    double nav;           // 最新单位净值
    bool ok;

    FundMeta() : nav(0), ok(false) {}
};

// 单只持仓对估算涨跌的贡献
struct ContributionItem {
    std::string code;  // 股票代码
    std::string name;  // 股票名称（UTF-8）
    double weight;     // 权重：最新季报占净值比例(%)
    double chgPct;     // 当日涨跌幅(%)
    double price;      // 最新价
    double contrib;    // 贡献(pp) = weight * chgPct / 100
    bool quoted;       // 是否取到行情（未取到按 0 计，界面标注）

    ContributionItem() : weight(0), chgPct(0), price(0), contrib(0), quoted(false) {}
};

// 一次估算的完整结果
struct FundEstimate {
    bool valid;                          // 是否可用于展示
    int quarter;                         // 权重所用季度 1/2/3
    double estPct;                       // 估算涨跌幅(%) = Σ贡献
    double coverPct;                     // 持仓覆盖度(%) = Σ权重
    int holdCount;                       // 参与计算的持仓只数
    double lastNav;                      // 最新净值
    std::string navDate;                 // 净值日期
    double estNav;                       // 估算净值 = lastNav * (1 + estPct/100)
    std::string updatedAt;               // 行情时间，如 "16:41:02"
    std::vector<ContributionItem> items; // 按 |贡献| 降序
    int bestIdx;                         // 最大拉升在 items 中的下标
    int worstIdx;                        // 最大拖累在 items 中的下标

    FundEstimate()
        : valid(false), quarter(0), estPct(0), coverPct(0), holdCount(0), lastNav(0), estNav(0),
          bestIdx(-1), worstIdx(-1) {}
};

// 判断当前交易时段（本地时间；不含节假日表，节假日按工作日处理）
EMarketSession detectMarketSession(std::time_t now);

// 时段中文文案（如 "交易中" / "午间休市"）
const TCHAR *marketSessionLabel(EMarketSession s);

// 是否处于"行情有效"的时段（交易中或模拟盘中）
bool isSessionActive(EMarketSession s);

// 抓取基金最新净值与名称（天天基金 pingzhongdata 档案脚本）
// pHttp 由调用方注入（CMainDlg 里创建的 IHttpClient），本模块不直接触碰任何平台网络 API。
bool fetchFundMeta(SOUI::IHttpClient *pHttp, const std::string &code, FundMeta &out,
                   std::string &err);

// 批量抓取个股行情（东方财富 push2 接口，一次请求最多 60 只）
bool fetchQuotes(SOUI::IHttpClient *pHttp, const std::vector<std::string> &codes,
                 std::vector<StockQuote> &out, std::string &err);

// 计算盘中估值：
//   贡献ᵢ = 权重ᵢ(最新季报占净值%) × 涨跌幅ᵢ(%) / 100
//   估算涨跌幅 = Σ贡献ᵢ ；估算净值 = 最新净值 × (1 + 估算涨跌幅/100)
// 未取到行情的个股按 0 涨跌计入（不剔除权重，保持仓位口径不变）
bool computeEstimate(const std::vector<HoldingChange> &rows,
                     const std::vector<StockQuote> &quotes, const FundMeta &meta,
                     FundEstimate &out);
