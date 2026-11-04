// fund_estimate.cpp : 盘中估值模块（行情抓取 + 估算）
//
// 口径（与 UI 设计稿一致）：
//   贡献ᵢ(pp)     = 最新季报占净值比例ᵢ(%) × 个股当日涨跌幅ᵢ(%) / 100
//   估算涨跌幅(%) = Σ 贡献ᵢ
//   估算净值      = 最新单位净值 × (1 + 估算涨跌幅/100)
// 未披露持仓（现金、债券等）按 0 计，因此估算偏保守，仅作参考。
#include "stdafx.h"
#include "fund_estimate.h"

#define kLogTag "fundest"

namespace {

// ---------- 简易 JSON 取值（只针对东财固定结构，避免引入额外依赖）----------

// 取对象内某个键的原始值文本（字符串去引号并反转义；数字原样返回）
std::string JsonField(const std::string &obj, const char *key) {
    std::string pat = std::string("\"") + key + "\"";
    size_t p = obj.find(pat);
    if (p == std::string::npos) return std::string();
    p = obj.find(':', p + pat.size());
    if (p == std::string::npos) return std::string();
    ++p;
    while (p < obj.size() && (unsigned char)obj[p] <= ' ') ++p;
    if (p >= obj.size()) return std::string();
    if (obj[p] == '"') {
        ++p;
        std::string out;
        while (p < obj.size() && obj[p] != '"') {
            if (obj[p] == '\\' && p + 1 < obj.size()) {
                char n = obj[p + 1];
                if (n == 'n') out += '\n';
                else if (n == 't') out += '\t';
                else out += n;
                p += 2;
                continue;
            }
            out += obj[p++];
        }
        return out;
    }
    size_t e = p;
    while (e < obj.size() && obj[e] != ',' && obj[e] != '}' && obj[e] != ']') ++e;
    std::string v = obj.substr(p, e - p);
    size_t b = v.find_first_not_of(" \t\r\n");
    size_t t = v.find_last_not_of(" \t\r\n");
    if (b == std::string::npos) return std::string();
    return v.substr(b, t - b + 1);
}

double JsonNumber(const std::string &obj, const char *key) {
    std::string s = JsonField(obj, key);
    if (s.empty() || s == "-" || s == "null") return 0;
    return std::atof(s.c_str());
}

// 从 p（指向 '['）开始，按括号配平拆出每个顶层 {...} 对象
std::vector<std::string> SplitObjectsAt(const std::string &json, size_t p) {
    std::vector<std::string> out;
    if (p == std::string::npos || p >= json.size() || json[p] != '[') return out;

    int depth = 0;
    bool inStr = false;
    std::string cur;
    for (size_t i = p + 1; i < json.size(); ++i) {
        char c = json[i];
        if (inStr) {
            cur += c;
            if (c == '\\' && i + 1 < json.size()) {
                cur += json[++i];
            } else if (c == '"') {
                inStr = false;
            }
            continue;
        }
        if (c == '"') { inStr = true; cur += c; continue; }
        if (c == '{') { ++depth; cur += c; continue; }
        if (c == '}') {
            --depth;
            cur += c;
            if (depth == 0) { out.push_back(cur); cur.clear(); }
            continue;
        }
        if (depth > 0) { cur += c; continue; }
        if (c == ']') break;
    }
    return out;
}

// 拆分 "diff":[{...},{...}] 里的每个对象
std::vector<std::string> SplitDiffObjects(const std::string &json) {
    size_t p = json.find("\"diff\"");
    if (p == std::string::npos) return std::vector<std::string>();
    return SplitObjectsAt(json, json.find('[', p));
}

// 截取 JS 中 "var <name> = [...]" 的数组文本（找不到返回空串）
std::string JsArrayText(const std::string &js, const char *name) {
    size_t p = js.find(name);
    if (p == std::string::npos) return std::string();
    p = js.find('[', p);
    if (p == std::string::npos) return std::string();

    int depth = 0;
    bool inStr = false;
    for (size_t i = p; i < js.size(); ++i) {
        char c = js[i];
        if (inStr) {
            if (c == '\\') { ++i; continue; }
            if (c == '"') inStr = false;
            continue;
        }
        if (c == '"') { inStr = true; continue; }
        if (c == '[') { ++depth; continue; }
        if (c == ']') {
            if (--depth == 0) return js.substr(p, i - p + 1);
        }
    }
    return std::string();
}

// 取 JS 中 "var <name> = \"...\";" 的字符串值
std::string JsStringVar(const std::string &js, const char *name) {
    size_t p = js.find(name);
    if (p == std::string::npos) return std::string();
    size_t b = js.find('"', p);
    if (b == std::string::npos) return std::string();
    size_t e = js.find('"', b + 1);
    if (e == std::string::npos) return std::string();
    return js.substr(b + 1, e - b - 1);
}

// 毫秒时间戳 -> 北京时间 yyyy-mm-dd
std::string MsToDate(double ms) {
    if (ms <= 0) return std::string();
    std::time_t t = (std::time_t)(ms / 1000.0 + 0.5) + 8 * 3600;  // 归一化到 UTC+8
    std::tm tmv;
#if defined(_WIN32)
    gmtime_s(&tmv, &t);
#else
    gmtime_r(&t, &tmv);
#endif
    char buf[16];
    fund_snprintf(buf, sizeof(buf), "%04d-%02d-%02d", tmv.tm_year + 1900, tmv.tm_mon + 1,
                  tmv.tm_mday);
    return std::string(buf);
}

// 股票代码 -> 东财 secid（1.=沪市，0.=深市/北交所）
std::string ToSecId(const std::string &code) {
    if (code.empty()) return std::string();
    char c = code[0];
    if (c == '6' || c == '9' || c == '5') return "1." + code;
    return "0." + code;
}

std::string Rt() {
    char buf[32];
    fund_snprintf(buf, sizeof(buf), "%.6f", (double)std::rand() / (double)RAND_MAX);
    return std::string(buf);
}

std::string NowHms() {
    std::time_t t = std::time(NULL);
    std::tm tmv;
#if defined(_WIN32)
    localtime_s(&tmv, &t);
#else
    localtime_r(&t, &tmv);
#endif
    char buf[16];
    fund_snprintf(buf, sizeof(buf), "%02d:%02d:%02d", tmv.tm_hour, tmv.tm_min, tmv.tm_sec);
    return std::string(buf);
}

// 按 |贡献| 降序（原先写成 lambda，VS2008 不支持）
bool ContribAbsDesc(const ContributionItem &a, const ContributionItem &b) {
    double aa = a.contrib < 0 ? -a.contrib : a.contrib;
    double bb = b.contrib < 0 ? -b.contrib : b.contrib;
    return aa > bb;
}

}  // namespace

EMarketSession detectMarketSession(std::time_t now) {
    std::tm tmv;
#if defined(_WIN32)
    localtime_s(&tmv, &now);
#else
    localtime_r(&now, &tmv);
#endif
    if (tmv.tm_wday == 0 || tmv.tm_wday == 6) return MS_CLOSED;

    int hm = tmv.tm_hour * 100 + tmv.tm_min;
    if (hm < 930) return MS_PREOPEN;
    if (hm <= 1130) return MS_TRADING;
    if (hm < 1300) return MS_LUNCH;
    if (hm <= 1500) return MS_TRADING;
    return MS_AFTERCLOSE;
}

const TCHAR *marketSessionLabel(EMarketSession s) {
    switch (s) {
        case MS_TRADING:    return _T("交易中");
        case MS_LUNCH:      return _T("午间休市");
        case MS_PREOPEN:    return _T("未开盘");
        case MS_AFTERCLOSE: return _T("已收盘");
        case MS_SIMULATED:  return _T("模拟盘中");
        case MS_CLOSED:
        default:            return _T("休市");
    }
}

bool isSessionActive(EMarketSession s) {
    return s == MS_TRADING || s == MS_SIMULATED;
}

bool fetchFundMeta(SOUI::IHttpClient *pHttp, const std::string &code, FundMeta &out,
                   std::string &err) {
    out = FundMeta();
    out.code = code;

    // 天天基金「基金档案」数据脚本：含 var fS_name、Data_netWorthTrend 等单位净值序列
    // （老的 fundgz.1234567.com.cn/js/{code}.js 实时估值接口已下线，返回 404 页面）
    std::string url = "https://fund.eastmoney.com/pingzhongdata/" + code + ".js?v=" + Rt();
    std::string body, diag;
    int httpCode = 0;
    if (!httpGetText(pHttp, url, "https://fund.eastmoney.com/" + code + ".html", body, httpCode,
                     diag)) {
        char codeBuf[16];
        fund_snprintf(codeBuf, sizeof(codeBuf), "%d", httpCode);
        err = "净值接口请求失败（HTTP " + std::string(codeBuf) + "）";
        SLOGI() << "[est] 净值接口失败 code=" << httpCode << " diag=" << diag.c_str();
        return false;
    }

    out.name = JsStringVar(body, "fS_name");

    // Data_netWorthTrend = [{"x":毫秒,"y":单位净值,"equityReturn":..},...]，取最后一项
    std::string arr = JsArrayText(body, "Data_netWorthTrend");
    std::vector<std::string> pts = SplitObjectsAt(arr, 0);
    if (!pts.empty()) {
        const std::string &last = pts.back();
        out.nav = JsonNumber(last, "y");
        out.navDate = MsToDate(JsonNumber(last, "x"));
    }

    // 货币基金没有单位净值走势，退化为按 1.0000 计（其净值恒为 1 元）
    if (!(out.nav > 0)) {
        std::string arr2 = JsArrayText(body, "Data_millionCopiesIncome");
        if (!arr2.empty()) {
            out.nav = 1.0;
            size_t b = arr2.rfind("],[");
            if (b != std::string::npos) {
                out.navDate = MsToDate(std::atof(arr2.c_str() + b + 3));
            }
        }
    }

    out.ok = out.nav > 0;
    SLOGI() << "[est] meta name=" << out.name.c_str() << " nav=" << out.nav
            << " date=" << out.navDate.c_str() << " points=" << (int)pts.size();
    if (!out.ok) err = "净值接口未返回有效净值（可能为新基金或代码有误）";
    return out.ok;
}

bool fetchQuotes(SOUI::IHttpClient *pHttp, const std::vector<std::string> &codes,
                 std::vector<StockQuote> &out, std::string &err) {
    out.clear();
    if (codes.empty()) { err = "无股票代码可查询"; return false; }

    out.reserve(codes.size());
    for (size_t i = 0; i < codes.size(); ++i) {
        StockQuote q;
        q.code = codes[i];
        out.push_back(q);
    }

    const size_t kBatch = 60;  // 单个请求的 secid 数量上限（URL 长度考虑）
    bool anyOk = false;
    std::string lastErr;

    for (size_t start = 0; start < codes.size(); start += kBatch) {
        size_t end = start + kBatch;
        if (end > codes.size()) end = codes.size();

        std::string secids;
        for (size_t i = start; i < end; ++i) {
            if (!secids.empty()) secids += ",";
            secids += ToSecId(codes[i]);
        }

        std::string url =
            "https://push2.eastmoney.com/api/qt/ulist.np/get?fltt=2&invt=2"
            "&fields=f2,f3,f12,f14,f18&secids=" + secids + "&_=" + Rt();

        std::string body, diag;
        int httpCode = 0;
        if (!httpGetText(pHttp, url, "https://quote.eastmoney.com/", body, httpCode, diag)) {
            char codeBuf[16];
            fund_snprintf(codeBuf, sizeof(codeBuf), "%d", httpCode);
            lastErr = "行情接口请求失败（HTTP " + std::string(codeBuf) + "）";
            SLOGI() << "[est] 行情接口失败 code=" << httpCode << " diag=" << diag.c_str();
            continue;
        }

        std::vector<std::string> objs = SplitDiffObjects(body);
        SLOGI() << "[est] 行情批次 " << (int)start << "~" << (int)end
                << " 条数=" << (int)objs.size();

        for (size_t k = 0; k < objs.size(); ++k) {
            const std::string &o = objs[k];
            std::string code = JsonField(o, "f12");
            if (code.empty()) continue;
            for (size_t i = start; i < end; ++i) {
                if (out[i].code != code) continue;
                out[i].name = JsonField(o, "f14");
                out[i].price = JsonNumber(o, "f2");
                out[i].chgPct = JsonNumber(o, "f3");
                out[i].preClose = JsonNumber(o, "f18");
                out[i].ok = out[i].price > 0;
                if (out[i].ok) anyOk = true;
                break;
            }
        }
    }

    if (!anyOk) {
        err = lastErr.empty() ? "未获取到任何有效行情" : lastErr;
        return false;
    }
    return true;
}

bool computeEstimate(const std::vector<HoldingChange> &rows, const std::vector<StockQuote> &quotes,
                     const FundMeta &meta, FundEstimate &out) {
    out = FundEstimate();

    // 以最新存在的季度作为权重口径（Q3 > Q2 > Q1）
    int q = 0;
    for (size_t i = 0; i < rows.size(); ++i)
        if (rows[i].lpQuarter > q) q = rows[i].lpQuarter;
    if (q <= 0 || rows.empty()) return false;

    std::map<std::string, const StockQuote *> qm;
    for (size_t i = 0; i < quotes.size(); ++i) qm[quotes[i].code] = &quotes[i];

    double sumW = 0;
    for (size_t i = 0; i < rows.size(); ++i) {
        const HoldingChange &r = rows[i];
        if (!r.hasQ[q - 1]) continue;
        ContributionItem it;
        it.code = r.code;
        it.name = r.name;
        it.weight = r.pct[q - 1];
        std::map<std::string, const StockQuote *>::const_iterator p = qm.find(r.code);
        if (p != qm.end() && p->second->ok) {
            it.chgPct = p->second->chgPct;
            it.price = p->second->price;
            it.quoted = true;
        }
        it.contrib = it.weight * it.chgPct / 100.0;
        out.estPct += it.contrib;
        sumW += it.weight;
        out.items.push_back(it);
    }
    if (out.items.empty()) return false;

    out.quarter = q;
    out.coverPct = sumW;
    out.holdCount = (int)out.items.size();
    out.lastNav = meta.nav;
    out.navDate = meta.navDate;
    out.estNav = (meta.nav > 0) ? meta.nav * (1.0 + out.estPct / 100.0) : 0;
    out.updatedAt = NowHms();

    // 按 |贡献| 降序，便于"贡献度分解"面板展示
    std::stable_sort(out.items.begin(), out.items.end(), ContribAbsDesc);

    for (size_t i = 0; i < out.items.size(); ++i) {
        if (out.items[i].contrib > 0 &&
            (out.bestIdx < 0 || out.items[i].contrib > out.items[out.bestIdx].contrib))
            out.bestIdx = (int)i;
        if (out.items[i].contrib < 0 &&
            (out.worstIdx < 0 || out.items[i].contrib < out.items[out.worstIdx].contrib))
            out.worstIdx = (int)i;
    }

    out.valid = true;
    SLOGI() << "[est] 估算完成 季度=Q" << out.quarter << " 只数=" << out.holdCount
            << " 覆盖=" << out.coverPct << "% estPct=" << out.estPct
            << " estNav=" << out.estNav;
    return true;
}

// ---------------------------------------------------------------- 大盘三大指数
// 对应 akshare「沪深重要指数」里最常用的三只：上证指数 / 深证成指 / 创业板指。
// secid 前缀沿用 push2 约定（1=沪市，0=深市），与个股 ToSecId 一致。
static const IndexSpec kMajorIndexes[] = {
    {"000001", "1.000001"},  // 上证指数
    {"399001", "0.399001"},  // 深证成指
    {"399006", "0.399006"},  // 创业板指
};

const IndexSpec *majorIndexSpecs() {
    return kMajorIndexes;
}

int majorIndexCount() {
    return (int)(sizeof(kMajorIndexes) / sizeof(kMajorIndexes[0]));
}

bool fetchMajorIndexes(SOUI::IHttpClient *pHttp, std::vector<IndexQuote> &out,
                       std::string &err) {
    out.clear();
    const int n = majorIndexCount();
    for (int i = 0; i < n; ++i) {
        IndexQuote q;
        q.code = kMajorIndexes[i].code;
        out.push_back(q);
    }

    std::string secids;
    for (int i = 0; i < n; ++i) {
        if (!secids.empty()) secids += ",";
        secids += kMajorIndexes[i].secid;
    }

    // 与 fetchQuotes 复用同一个 push2 ulist.np/get：一次请求取回三只指数。
    // fields 的语义取自 akshare stock_zh_index_spot_em 的字段映射表。
    std::string url =
        "https://push2.eastmoney.com/api/qt/ulist.np/get?fltt=2&invt=2"
        "&fields=f2,f3,f4,f12,f14,f18&secids=" + secids + "&_=" + Rt();
    std::string body, diag;
    int httpCode = 0;
    if (!httpGetText(pHttp, url, "https://quote.eastmoney.com/center/hszs.html", body,
                     httpCode, diag)) {
        char codeBuf[16];
        fund_snprintf(codeBuf, sizeof(codeBuf), "%d", httpCode);
        err = "指数行情请求失败（HTTP " + std::string(codeBuf) + "）";
        SLOGI() << "[index] 大盘指数请求失败 code=" << httpCode << " diag=" << diag.c_str();
        return false;
    }

    std::vector<std::string> objs = SplitDiffObjects(body);
    int okCount = 0;
    for (size_t k = 0; k < objs.size(); ++k) {
        const std::string &o = objs[k];
        std::string code = JsonField(o, "f12");
        if (code.empty()) continue;
        for (int i = 0; i < n; ++i) {
            if (out[i].code != code) continue;
            std::string name = JsonField(o, "f14");
            if (!name.empty()) out[i].name = name;
            out[i].price = JsonNumber(o, "f2");
            out[i].chgPct = JsonNumber(o, "f3");
            out[i].chgAmt = JsonNumber(o, "f4");
            out[i].preClose = JsonNumber(o, "f18");
            out[i].ok = out[i].price > 0;
            if (out[i].ok) okCount++;
            break;
        }
    }

    SLOGI() << "[index] 三大指数 返回=" << (int)objs.size() << " 有效=" << okCount;
    if (okCount == 0) {
        err = "未获取到有效的大盘指数行情";
        return false;
    }
    return true;
}
