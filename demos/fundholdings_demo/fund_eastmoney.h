#pragma once
#include <string>
#include <vector>

// 基金持仓单条记录（等价于 akshare fund_portfolio_hold_em 返回的一行）
struct FundHolding {
    std::string quarter;  // 季度标签，如 "2026年3季度股票投资明细"
    std::string code;     // 股票代码
    std::string name;     // 股票名称
    std::string pct;      // 占净值比例(%)，数值字符串（已去掉 %）
    std::string shares;   // 持股数(万股)
    std::string mv;       // 持仓市值(万元)
};

// 持仓变化表的一行：按股票代码透视，列式为 Q1/Q2/Q3 各指标 + 环比变化。
// 与 gen_change_table.py 生成的 HTML 变化表对齐（占净值% / 持股(万) / 市值(万)，Q3-Q1 Δ）。
struct HoldingChange {
    std::string code;          // 股票代码
    std::string name;          // 股票名称
    bool hasQ[3] = {false, false, false};  // 是否持有 Q1/Q2/Q3
    double pct[3] = {0, 0, 0};             // 占净值比例(%)
    double shares[3] = {0, 0, 0};          // 持股数(万股)
    double mv[3] = {0, 0, 0};              // 持仓市值(万元)
    double lp = 0;                         // 最新季度占净值(%)，用于排序（取 Q3>Q2>Q1 中最新存在的）
    int lpQuarter = 0;                     // 最新季度序号 1/2/3
    double pctChg = 0, shChg = 0, mvChg = 0;  // Q3-Q1 变化量
    bool chgValid = false;                 // 是否同时含 Q3 与 Q1（可计算 Δ）
    std::string status;                    // 持有季度，如 "✓✓✓" / "✓——"
    std::string tag;                       // 变动标记，如 "Q3新进" / "Q3退出"（可多个，用 / 连接）
};

// 把 fetchFundHoldings 返回的扁平持仓列表（每条 = 某季度某股票），转换为按股票代码透视的
// 持仓变化表。成功返回 true，结果写入 out（按最新季度占净值降序、代码升序排列）。
bool buildHoldingsChange(const std::vector<FundHolding>& holdings,
                         std::vector<HoldingChange>& out);

// 等价于 akshare.fund_portfolio_hold_em(symbol, date)
// 内部流程：
//   1. 构造并发送 GET 请求到天天基金 FundArchivesDatas.aspx
//   2. 从 `var apidata={ content:"..." }` 响应中提取 content(HTML)
//   3. 解析 <h4> 季度标签与 <table> 表格
//   4. 按列名映射、抽取持仓行，并按季度升序排列
// 成功返回 true（out 可能为空），失败返回 false 并在 errMsg 中说明原因。
bool fetchFundHoldings(const std::string& fundCode,
                       const std::string& year,
                       std::vector<FundHolding>& out,
                       std::string& errMsg);
