#pragma once
#include <string>
#include <vector>
#include <interface/SHttpClient-i.h>

// 基金持仓单条记录（等价于 akshare fund_portfolio_hold_em 返回的一行）
struct FundHolding {
    std::string quarter;  // 季度标签，如 "2026Q3"（纯 ASCII，仅供日志/展示）
    int qNum;             // 季度序号 1/2/3/4：解析阶段由截止日期直接得到，
                          // 数值天然与编码无关，是透视（buildHoldingsChange）的唯一依据。
                          // 注意：不要用 quarter 字符串反推季度——中文字面量在 MSVC 下
                          // 会按执行字符集(GBK)编码，与解析用的 UTF-8 字节不匹配。
    std::string code;     // 股票代码
    std::string name;     // 股票名称
    std::string pct;      // 占净值比例(%)，数值字符串（已去掉 %）
    std::string shares;   // 持股数(万股)
    std::string mv;       // 持仓市值(万元)

    FundHolding() : qNum(0) {}
};

// 持仓变化表的一行：按股票代码透视，列式为 Q1/Q2/Q3 各指标 + 环比变化。
// 与 gen_change_table.py 生成的 HTML 变化表对齐（占净值% / 持股(万) / 市值(万)，Q3-Q1 Δ）。
struct HoldingChange {
    std::string code;  // 股票代码
    std::string name;  // 股票名称
    bool hasQ[3];      // 是否持有 Q1/Q2/Q3
    double pct[3];     // 占净值比例(%)
    double shares[3];  // 持股数(万股)
    double mv[3];      // 持仓市值(万元)
    double lp;         // 最新季度占净值(%)，用于排序（取 Q3>Q2>Q1 中最新存在的）
    int lpQuarter;     // 最新季度序号 1/2/3
    double pctChg;     // Q3-Q1 占净值变化量
    double shChg;      // Q3-Q1 持股变化量
    double mvChg;      // Q3-Q1 市值变化量
    bool chgValid;     // 是否同时含 Q3 与 Q1（可计算 Δ）
    std::string status;// 持有季度，如 "✓✓✓" / "✓——"
    std::string tag;   // 变动标记，如 "Q3新进" / "Q3退出"（可多个，用 / 连接）

    // VS2008 不支持类内成员初始化，统一放到构造函数里
    HoldingChange() : lp(0), lpQuarter(0), pctChg(0), shChg(0), mvChg(0), chgValid(false) {
        for (int i = 0; i < 3; ++i) {
            hasQ[i] = false;
            pct[i] = 0;
            shares[i] = 0;
            mv[i] = 0;
        }
    }
};

// 把 fetchFundHoldings 返回的扁平持仓列表（每条 = 某季度某股票），转换为按股票代码透视的
// 持仓变化表。成功返回 true，结果写入 out（按最新季度占净值降序、代码升序排列）。
bool buildHoldingsChange(const std::vector<FundHolding>& holdings,
                         std::vector<HoldingChange>& out);

// 对 IHttpClient 做一次性公共配置（UA / Accept / Accept-Language / X-Requested-With）。
// 创建客户端之后调一次即可。这些头在两端都会进入每个请求：
// Windows 后端走 WinHttpAddRequestHeaders，libcurl 后端走 CURLOPT_HTTPHEADER。
// 必须调，否则 FundArchivesDatas.aspx 会因缺少 X-Requested-With 直接返回 404。
void configureHttpClient(SOUI::IHttpClient *pHttp);

// 等价于 akshare.fund_portfolio_hold_em(symbol, date)
// 内部流程：
//   1. 通过 IHttpClient 访问基金详情页，取回会话 Cookie（平台无关：
//      Windows 后端是 WinHTTP，其它平台是 libcurl，本文件不再直接调 WinHTTP）
//   2. 构造并发送 GET 请求到天天基金 FundArchivesDatas.aspx
//   3. 从 `var apidata={ content:"..." }` 响应中提取 content(HTML)
//   4. 解析 <h4> 季度标签与 <table> 表格
//   5. 按列名映射、抽取持仓行，并按季度升序排列
// pHttp 可为 NULL（例如启动早期还没创建出来），此时跳过 Cookie 引导，
// 在不需要会话校验的网络环境下照常工作。
// 成功返回 true（out 可能为空），失败返回 false 并在 errMsg 中说明原因。
bool fetchFundHoldings(SOUI::IHttpClient *pHttp,
                       const std::string& fundCode,
                       const std::string& year,
                       std::vector<FundHolding>& out,
                       std::string& errMsg);

// 通用 HTTP GET（走 IHttpClient，与持仓接口共用同一套策略逻辑：带/不带会话 Cookie，
// 第一个 200 即返回）。供行情、净值等 JSON 接口复用。
// 成功（HTTP 200 且 body 非空）返回 true；diag 回传最终选用的策略名，便于日志排查。
bool httpGetText(SOUI::IHttpClient *pHttp, const std::string& url, const std::string& referer,
                 std::string& body, int& httpCode, std::string& diag);
