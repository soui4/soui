// fund_eastmoney.cpp
#include "stdafx.h"
#include "fund_eastmoney.h"

// 本文件日志统一使用 SOUI 的 SLOGI() 输出（参考 demos/demo 的日志用法）。
#define kLogTag "fundhold"

namespace {

// ---------- 基础字符串工具 ----------
size_t findCI(const std::string& s, const std::string& needle, size_t from) {
    if (needle.empty() || from >= s.size()) return std::string::npos;
    size_t nlen = needle.size();
    size_t last = s.size() - nlen;
    for (size_t i = from; i <= last; ++i) {
        bool eq = true;
        for (size_t j = 0; j < nlen; ++j) {
            if (tolower((unsigned char)s[i + j]) != tolower((unsigned char)needle[j])) {
                eq = false;
                break;
            }
        }
        if (eq) return i;
    }
    return std::string::npos;
}

std::string trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace((unsigned char)s[a])) ++a;
    while (b > a && std::isspace((unsigned char)s[b - 1])) --b;
    return s.substr(a, b - a);
}

// HTML 实体解码：&nbsp; &amp; &lt; &gt; &quot; &#NNN; &#xHH;
std::string decodeEntities(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    size_t i = 0, n = s.size();
    while (i < n) {
        if (s[i] == '&') {
            size_t end = s.find(';', i);
            if (end != std::string::npos) {
                std::string ent = s.substr(i + 1, end - i - 1);
                if (ent == "nbsp") out += ' ';
                else if (ent == "amp") out += '&';
                else if (ent == "lt") out += '<';
                else if (ent == "gt") out += '>';
                else if (ent == "quot") out += '"';
                else if (!ent.empty() && ent[0] == '#') {
                    int code = 0;
                    if (ent[1] == 'x' || ent[1] == 'X')
                        code = (int)strtol(ent.c_str() + 2, NULL, 16);
                    else
                        code = (int)strtol(ent.c_str() + 1, NULL, 10);
                    if (code > 0 && code < 0x110000) {
                        // 仅处理常见 ASCII/中文范围，避免复杂 UTF-8 编码
                        if (code < 128) out += (char)code;
                        else out += '?';
                    } else {
                        out += '?';
                    }
                } else {
                    out += '&';
                    out += ent;
                    out += ';';
                }
                i = end + 1;
                continue;
            }
        }
        out += s[i];
        ++i;
    }
    return out;
}

// 去掉所有 HTML 标签，再解码实体，返回可见文本
std::string cellText(const std::string& html) {
    std::string noTag;
    noTag.reserve(html.size());
    bool inTag = false;
    for (size_t i = 0; i < html.size(); ++i) {
        char c = html[i];
        if (c == '<') inTag = true;
        else if (c == '>') inTag = false;
        else if (!inTag) noTag += c;
    }
    return trim(decodeEntities(noTag));
}

// ---------- 响应解析 ----------
// 从 `var apidata={ content:"...." }` 中提取 content 字符串（双引号包裹，内部用单引号）
std::string extractContent(const std::string& raw) {
    size_t p = raw.find("content:");
    if (p == std::string::npos) return "";
    p += 8;  // skip "content:"（共 8 个字符，包含末尾的冒号）
    while (p < raw.size() && (raw[p] == ' ' || raw[p] == '\t')) ++p;
    if (p >= raw.size() || raw[p] != '"') return "";
    ++p;  // 进入 content 内容
    std::string res;
    while (p < raw.size()) {
        char c = raw[p];
        if (c == '\\' && p + 1 < raw.size()) {  // 转义字符：保留下一个字符
            res += raw[p + 1];
            p += 2;
            continue;
        }
        if (c == '"') break;  // 结束引号
        res += c;
        ++p;
    }
    return res;
}

// 查找所有季度标签 —— 方法1：按中文“YYYY年N季度股票投资明细”字面量匹配。
// 要求 content 为 UTF-8（东财返回 GBK 时，上层会先转 UTF-8）。
// 注意：“年”等中文是多字节，年份/季度数字都是 ASCII 单字节，不能用 h[ny+1] 直接取下个字符，
// 必须按字节跳过“年”的多字节后再取季度数字。
std::vector<std::string> findQuartersByLabel(const std::string& h) {
    std::vector<std::string> v;
    const std::string tail = "季度股票投资明细";
    size_t p = 0;
    while (true) {
        size_t q = findCI(h, tail, p);
        if (q == std::string::npos) break;
        size_t ny = h.rfind("年", q);
        if (ny == std::string::npos) { p = q + 1; continue; }
        // 季度数字：年 之后的第一个 ASCII 数字（跳过“年”自身的多字节）
        size_t ap = ny;
        while (ap < q && !std::isdigit((unsigned char)h[ap])) ++ap;
        char qd = (ap < q) ? h[ap] : 0;
        if (qd < '1' || qd > '4') { p = q + 1; continue; }
        // 年份：年 之前的 4 位 ASCII 数字（年份为 ASCII，单字节）
        size_t ys = ny;
        int cnt = 0;
        while (ys > 0 && std::isdigit((unsigned char)h[ys - 1]) && cnt < 4) {
            --ys;
            ++cnt;
        }
        if (cnt != 4 || h[ys] != '2') { p = q + 1; continue; }
        v.push_back(h.substr(ys, (q + tail.size()) - ys));
        p = q + tail.size();
    }
    return v;
}

// 查找所有季度标签 —— 方法2：编码无关兜底。
// 每个季度块都是 <h4>...</h4>，内部有 <font class='px12'>YYYY-MM-DD</font> 的截止日期（纯 ASCII），
// 由月份反推季度：03→Q1、06→Q2、09→Q3、12→Q4。即使 content 是 GBK 或中文已被转成 '?'，
// 该 ASCII 日期仍可正确解析，从而保证 quarters 不为 0。
std::vector<std::string> findQuartersByDate(const std::string& h) {
    std::vector<std::string> v;
    size_t p = 0;
    while (true) {
        size_t h4s = findCI(h, "<h4", p);
        if (h4s == std::string::npos) break;
        size_t h4e = findCI(h, "</h4>", h4s);
        if (h4e == std::string::npos) break;
        std::string block = h.substr(h4s, h4e + 5 - h4s);
        size_t d = 0;
        bool found = false;
        while (!found) {
            size_t s = block.find("px12'>", d);
            if (s == std::string::npos) break;
            s += 6;
            size_t e = block.find('<', s);
            if (e == std::string::npos) break;
            std::string date = block.substr(s, e - s);  // e.g. "2026-09-30"
            int y = 0, mo = 0, da = 0;
            if (sscanf(date.c_str(), "%d-%d-%d", &y, &mo, &da) == 3 &&
                y >= 2000 && mo >= 1 && mo <= 12) {
                int qn = (mo - 1) / 3 + 1;
                char label[64];
                fund_snprintf(label, sizeof(label), "%d年%d季度股票投资明细", y, qn);
                v.push_back(label);
                found = true;
            }
            d = e;
        }
        p = h4e + 5;
    }
    return v;
}

// 优先按中文标签（content 已是 UTF-8 时生效）；若返回 0（GBK 未转码成功 / 中文被转成 '?'），
// 回退到与编码无关的日期推导，确保 quarters 始终能解析出来。
std::vector<std::string> findQuarters(const std::string& h) {
    std::vector<std::string> a = findQuartersByLabel(h);
    if (!a.empty()) return a;
    return findQuartersByDate(h);
}

// 查找所有 <table>...</table> 块
std::vector<std::string> findTables(const std::string& h) {
    std::vector<std::string> v;
    size_t p = 0;
    while (true) {
        size_t s = findCI(h, "<table", p);
        if (s == std::string::npos) break;
        size_t e = findCI(h, "</table>", s);
        if (e == std::string::npos) break;
        v.push_back(h.substr(s, e + 8 - s));
        p = e + 8;
    }
    return v;
}

// 解析单个表格为行/列文本（首行为表头）
std::vector<std::vector<std::string> > parseTableRows(const std::string& tableHtml) {
    std::vector<std::vector<std::string> > rows;
    size_t p = 0;
    while (true) {
        size_t trS = findCI(tableHtml, "<tr", p);
        if (trS == std::string::npos) break;
        size_t trE = findCI(tableHtml, "</tr>", trS);
        if (trE == std::string::npos) break;
        std::string tr = tableHtml.substr(trS, trE + 5 - trS);

        std::vector<std::string> row;
        size_t c = 0;
        while (true) {
            size_t tdS = findCI(tr, "<td", c);
            if (tdS == std::string::npos) tdS = findCI(tr, "<th", c);
            if (tdS == std::string::npos) break;
            size_t closeTag = findCI(tr, "</td>", tdS);
            size_t closeTag2 = findCI(tr, "</th>", tdS);
            size_t cellE = (closeTag == std::string::npos)
                               ? closeTag2
                               : ((closeTag2 == std::string::npos) ? closeTag
                                                                  : smin(closeTag, closeTag2));
            if (cellE == std::string::npos) break;
            size_t gt = tr.find('>', tdS);
            if (gt == std::string::npos || gt >= cellE) break;
            std::string inner = tr.substr(gt + 1, cellE - gt - 1);
            row.push_back(cellText(inner));
            c = cellE + 5;
        }
        if (!row.empty()) rows.push_back(row);
        p = trE + 5;
    }
    return rows;
}

struct ColMap {
    int code, name, pct, shares, mv;

    ColMap() : code(-1), name(-1), pct(-1), shares(-1), mv(-1) {}
};

// 按表头文字映射列索引（兼容 "占净值 比例" 等带空格写法）。
// 若中文列名因编码问题匹配不上（GBK 未转码成功），则启用编码无关的位置兜底：
//   序号(0)、股票代码(1)、股票名称(2) 固定在前；
//   占净值比例 / 持股数 / 持仓市值 永远是表格最后三列（无论中间是否夹带最新价/涨跌幅/相关资讯）。
ColMap mapColumns(const std::vector<std::string>& hdr) {
    ColMap m;
    for (size_t i = 0; i < hdr.size(); ++i) {
        const std::string& s = hdr[i];
        if (s.find("股票代码") != std::string::npos) m.code = (int)i;
        else if (s.find("股票名称") != std::string::npos || s.find("名称") != std::string::npos) m.name = (int)i;
        else if (s.find("占净值") != std::string::npos) m.pct = (int)i;
        else if (s.find("持股数") != std::string::npos) m.shares = (int)i;
        else if (s.find("持仓市值") != std::string::npos || s.find("市值") != std::string::npos) m.mv = (int)i;
    }
    if (m.pct < 0 || m.shares < 0 || m.mv < 0) {
        if (hdr.size() >= 4) {
            if (m.code < 0) m.code = 1;
            if (m.name < 0) m.name = 2;
            m.pct = (int)hdr.size() - 3;
            m.shares = (int)hdr.size() - 2;
            m.mv = (int)hdr.size() - 1;
        }
    }
    return m;
}

std::string stripPct(const std::string& s) {
    std::string t = trim(s);
    if (!t.empty() && t[t.size() - 1] == '%') t.erase(t.size() - 1);
    return t;
}

// 规整数值字符串：去掉千分位逗号，并把 "1.30"/"0.00" 规整为 "1.3"/"0.0"，
// 与 akshare 的 pd.to_numeric 行为保持一致。无法解析为数字时（如 "—"）原样返回。
std::string normalizeNumber(const std::string& s) {
    std::string t;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == ',' || c == ' ' || c == '\t' || c == '\r' || c == '\n') continue;
        t += c;
    }
    if (t.empty()) return t;
    char* endp = NULL;
    double d = std::strtod(t.c_str(), &endp);
    if (endp == t.c_str() || *endp != '\0') return t;  // 非纯数字
    if (std::fabs(d) < 1e-9) return "0.0";
    if (std::fabs(d - std::floor(d + 0.5)) < 1e-9 && std::fabs(d) < 1e15) {
        char buf[64];
        fund_snprintf(buf, sizeof(buf), "%.0f.0", d);  // 整数值 -> "X.0"
        return std::string(buf);
    }
    char buf[64];
    fund_snprintf(buf, sizeof(buf), "%.6f", d);
    std::string r(buf);
    size_t dot = r.find('.');
    if (dot != std::string::npos) {
        while (r.size() > dot + 1 && r[r.size() - 1] == '0') r.erase(r.size() - 1);
        if (r[r.size() - 1] == '.') r += '0';
    }
    return r;
}

const std::string& cellGet(const std::vector<std::string>& row, int idx) {
    static const std::string empty;
    if (idx < 0 || (size_t)idx >= row.size()) return empty;
    return row[idx];
}

// ---------- 会话 Cookie 引导（走 IHttpClient，平台无关）----------
// 部分网络环境下，东财 fundf10 接口会校验会话 Cookie：浏览器先访问基金详情页拿到
// ASP.NET_SessionId，再带着 Cookie 发起 AJAX 才不会 404。
//
// 取 Cookie 这件事以前是在本文件里直接用 WinHTTP 做的，等于把整个工具钉死在 Windows 上。
// 现在收敛成 IHttpClient::GetCookie()：Windows 后端用 WinHTTP，其它平台用 libcurl，
// 上层（本文件）只面对接口，天然跨平台。

// 判断字符串是否为合法 UTF-8（用于区分“东财返回的 GBK content”与“已经是 UTF-8 的 content”）。
static bool IsValidUtf8(const std::string& s) {
    size_t i = 0, n = s.size();
    while (i < n) {
        unsigned char c = (unsigned char)s[i];
        if (c < 0x80) { ++i; continue; }
        int extra;
        if ((c & 0xE0) == 0xC0) extra = 1;       // 110xxxxx -> 2 字节
        else if ((c & 0xF0) == 0xE0) extra = 2;  // 1110xxxx -> 3 字节
        else if ((c & 0xF8) == 0xF0) extra = 3;  // 11110xxx -> 4 字节
        else return false;                        // 非法首字节
        if (i + (size_t)extra >= n) return false;
        for (int k = 1; k <= extra; ++k) {
            unsigned char cc = (unsigned char)s[i + k];
            if ((cc & 0xC0) != 0x80) return false;  // 续字节必须是 10xxxxxx
        }
        i += (size_t)extra + 1;
    }
    return true;
}

// 将多字节字符串（指定代码页，如 936=GBK）转换为 UTF-8。转换失败则原样返回。
static std::string MbsToUtf8(const std::string& s, UINT codePage) {
    int n = ::MultiByteToWideChar(codePage, 0, s.c_str(), -1, NULL, 0);
    if (n <= 0) return s;
    std::wstring w; w.resize((size_t)n);
    ::MultiByteToWideChar(codePage, 0, s.c_str(), -1, &w[0], n);
    if (!w.empty() && w[w.size() - 1] == L'\0') w.erase(w.size() - 1);
    int m = ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, NULL, 0, NULL, NULL);
    if (m <= 0) return s;
    std::string o; o.resize((size_t)m);
    ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &o[0], m, NULL, NULL);
    if (!o.empty() && o[o.size() - 1] == '\0') o.erase(o.size() - 1);
    return o;
}

// 访问基金详情页，取回会话 Cookie。
// 走 IHttpClient::GetCookie()，不再直接调 WinHTTP/curl：
//   · Windows  -> components/httpclient/win/HttpClient.cpp（WinHTTP）
//   · 其它平台 -> components/httpclient/linux/HttpClient.cpp（libcurl）
// 这样本文件的调用点与平台无关，编译到 Linux/macOS 时不需要任何条件编译。
// 取不到（无会话校验的网络环境 / 请求失败）返回空串，调用方照常继续。
std::string fetchSessionCookie(IHttpClient *pHttp, const std::string& fundCode) {
    if (!pHttp) return "";
    const std::string url = "https://fundf10.eastmoney.com/ccmx_" + fundCode + ".html";
    SStringA strCookie;
    const BOOL bOk = pHttp->GetCookie(&strCookie, url.c_str());
    std::string cookie;
    if (bOk) cookie.assign(strCookie.c_str(), (size_t)strCookie.GetLength());
    SLOGI() << "[cookie] url=" << url.c_str() << " ok=" << (int)bOk
            << " len=" << (fund_uint64)cookie.size();
    return cookie;
}

// ---------- HTTP：统一走 IHttpClient（平台无关）----------
// 以前这一段是用 WinHTTP API 直接写的（WinHttpOpen/Connect/OpenRequest/QueryHeaders/...），
// 业务代码里散着 HINTERNET 句柄、WINHTTP_ACCESS_TYPE_* 常量、SECURITY_FLAG_* 之类的东西，
// 等于把整个工具钉死在 Windows 上。现在所有请求都收敛到 IHttpClient：
//   · Windows  -> components/httpclient/win（WinHTTP）
//   · 其它平台 -> components/httpclient/linux（libcurl）
// 平台差异由组件承担，本文件只面对接口，编译到 Linux/macOS 不需要任何条件编译。
//
// 日志统一走 SOUI 的 SLOGI()（参考 demos/demo），由 applog 的回调落盘，便于排查。

// 本次请求专有的头：Referer / Cookie 每次都不同，所以不能放进 SetHeader，
// 只能随请求下发。IHttpClient 的两端都接受 CRLF 分隔的多行头文本。
static std::string BuildRequestHeaders(const std::string &referer, const std::string &cookie) {
    std::string h;
    if (!referer.empty()) {
        h += "Referer: ";
        h += referer;
        h += "\r\n";
    }
    if (!cookie.empty()) {
        h += "Cookie: ";
        h += cookie;
        h += "\r\n";
    }
    return h;
}

// 单次 GET（走 IHttpClient）。返回响应体；outHttpCode 回传 HTTP 状态码
// （-1 表示网络层就失败了，连状态行都没拿到）。
static std::string HttpClientGet(SOUI::IHttpClient *pHttp, const std::string &url,
                                 const std::string &referer, const std::string &cookie,
                                 int *outHttpCode) {
    if (outHttpCode) *outHttpCode = -1;
    if (!pHttp) {
        // 启动早期客户端可能还没创建出来，当作"无网络结果"处理
        SLOGI() << "[HTTP] client 未创建，跳过 url=" << url.c_str();
        return std::string();
    }

    const std::string hdrs = BuildRequestHeaders(referer, cookie);
    SLOGI() << "[HTTP] GET " << url.c_str();
    SLOGI() << "[HTTP]   referer=" << (referer.empty() ? "(none)" : referer.c_str())
            << " cookie=" << (cookie.empty() ? "(none)" : cookie.c_str());

    // GET 没有请求体，postData 传 NULL/0。lpHeader 由后端按平台拼装：
    // Win 端走 WinHttpSendRequest 的附加头，libcurl 端进 CURLOPT_HTTPHEADER。
    SStringA res;
    const BOOL bOk = pHttp->Request2(&res, url.c_str(), Hr_Get, NULL, 0,
                                     hdrs.empty() ? NULL : hdrs.c_str());
    const int code = pHttp->GetHttpCode();
    if (outHttpCode) *outHttpCode = code;

    std::string body;
    if (res.GetLength() > 0) body.assign(res.c_str(), (size_t)res.GetLength());

    SLOGI() << "[HTTP] code=" << code << " ok=" << (int)bOk
            << " bytes=" << (fund_uint64)body.size()
            << " err=" << (int)pHttp->GetErrorCode();
    if (!body.empty()) {
        std::string preview = body;
        if (preview.size() > 300) preview = preview.substr(0, 300) + "...";
        SLOGI() << "[HTTP] preview: " << preview.c_str();
    }
    return body;
}

// 统计响应体里“季度段数”（取 中文标签命中数 与 表格数 的较大者），用于在不同请求策略间择优。
static int CountQuarters(const std::string &raw) {
    std::string c = extractContent(raw);
    if (c.empty()) c = raw;
    int q = (int)findQuartersByLabel(c).size();
    if (q == 0) q = (int)findQuartersByDate(c).size();
    int t = (int)findTables(c).size();
    return q > t ? q : t;
}

// 请求策略：带会话 Cookie / 不带。
// 以前还有"直连 vs 系统代理"这一维（共 4 条策略）。现在代理策略由 IHttpClient 后端
// 统一决定（Windows 后端用系统代理配置，libcurl 后端用环境变量），业务层不再介入：
// 少一半探测请求，自选加载明显更快。没有 Cookie 时"带 Cookie"那条自动跳过。
struct HttpVariant {
    bool withCookie;
    const char *name;
};
static const HttpVariant kHttpVariants[] = {
    {true, "cookie"},
    {false, "plain"},
};

// 等价于 akshare 的请求，但改为“多策略择优”：逐条策略取响应，返回“季度段数最多”的那个。
// 之所以不只认第一次 200：曾经出现过直连/无 Cookie 被缓存代理回了“陈旧的一季”响应，
// 而带会话 Cookie 能拿到全量三季 —— 只认第一个 200 会让另外两季整段丢失。
// outDiag 回传最终选用的策略名，便于日志排查。
std::string fetchRichest(SOUI::IHttpClient *pHttp, const std::string& url,
                         const std::string& referer, const std::string& cookie,
                         int* outHttpCode, std::string& outDiag) {
    std::string bestBody; int bestQ = -1; int bestCode = 0; std::string bestName;

    // 择优要跑多条策略，网络差时可能拖很久（都在后台线程，但会让"刷新估值"迟迟没结果）。
    // 给整体探测设个上限：一旦已经拿到可用结果就提前收工。这里用 time() 而不是
    // GetTickCount()，免得为几行判断引入平台相关调用（精度到秒，够用）。
    const std::time_t tStart = std::time(NULL);
    const int kProbeDeadlineSec = 12;

    const int nVariants = (int)(sizeof(kHttpVariants) / sizeof(kHttpVariants[0]));
    for (int vi = 0; vi < nVariants; ++vi) {
        const HttpVariant &v = kHttpVariants[vi];
        if (v.withCookie && cookie.empty()) continue;  // 没 Cookie 时这条与下一条重复
        if (bestQ > 0 && std::time(NULL) - tStart > kProbeDeadlineSec) {
            SLOGI() << "[fetch] 择优探测已超过 " << kProbeDeadlineSec << "s，跳过剩余策略";
            break;
        }
        int code = 0;
        std::string body = HttpClientGet(pHttp, url, referer,
                                         v.withCookie ? cookie : std::string(), &code);
        int q = (code == 200 && !body.empty()) ? CountQuarters(body) : 0;
        SLOGI() << "[fetch] strategy=" << v.name << " code=" << code
                << " body=" << (fund_uint64)body.size() << " quarters=" << q;
        if (q > bestQ || (q == bestQ && body.size() > bestBody.size())) {
            bestQ = q; bestBody.swap(body); bestCode = code; bestName = v.name;
        }
    }
    if (outHttpCode) *outHttpCode = bestCode;
    outDiag = bestName;
    return bestBody;
}

// 行情/净值这类“单一结果”接口不需要择优：第一个 200 且非空就返回。
// 让它们也走多策略等于每次刷新多打一倍请求，网络稍差时界面就像“点了没反应”。
std::string fetchFirstOk(SOUI::IHttpClient *pHttp, const std::string& url,
                         const std::string& referer, const std::string& cookie,
                         int* outHttpCode, std::string& outDiag) {
    std::string lastBody;
    int lastCode = 0;
    const int nVariants = (int)(sizeof(kHttpVariants) / sizeof(kHttpVariants[0]));
    for (int vi = 0; vi < nVariants; ++vi) {
        const HttpVariant &v = kHttpVariants[vi];
        if (v.withCookie && cookie.empty()) continue;
        int code = 0;
        std::string body = HttpClientGet(pHttp, url, referer,
                                         v.withCookie ? cookie : std::string(), &code);
        SLOGI() << "[http] strategy=" << v.name << " code=" << code
                << " body=" << (fund_uint64)body.size();
        if (code == 200 && !body.empty()) {
            if (outHttpCode) *outHttpCode = code;
            outDiag = v.name;
            return body;
        }
        lastBody.swap(body);
        lastCode = code;
    }
    if (outHttpCode) *outHttpCode = lastCode;
    outDiag = "all-failed";
    return lastBody;
}

std::string makeRt() {
    static bool seeded = false;
    if (!seeded) { std::srand((unsigned)std::time(NULL)); seeded = true; }
    char buf[32];
    fund_snprintf(buf, sizeof(buf), "%f", (double)std::rand() / RAND_MAX);
    return std::string(buf);
}

// 从季度标签 "2026年3季度股票投资明细" 中提取季度序号（1/2/3/4）。
//
// 关键：本函数必须与“执行字符集”无关。
// 标签里的“年”在 UTF-8 下是 3 字节(E5 B9 B4)、在 GBK 下是 2 字节(C4 EA)，
// 因此不能直接 find 某个固定的 UTF-8 字节串（MSVC 中文环境下窄字面量是 GBK，
// 会匹配失败并让整个透视阶段丢弃全部记录）。这里改为纯 ASCII 规则：
//   1) 先定位 4 位 ASCII 年份（"2026"）；
//   2) 跳过其后所有 >=0x80 的非 ASCII 字节（不论 UTF-8 还是 GBK 的“年”）；
//   3) 遇到的第一个 '1'..'4' 即季度号；若先遇到其它 ASCII 数字则判定失败。
// 严禁从串首直接扫描 1-4：那样会命中“2026”里的 '2'，把 Q1/Q3 全误判成 Q2。
int quarterIndex(const std::string& q) {
    const size_t n = q.size();
    for (size_t i = 0; i + 4 <= n; ++i) {
        if (!std::isdigit((unsigned char)q[i])) continue;
        if (i > 0 && std::isdigit((unsigned char)q[i - 1])) continue;  // 必须从年份首位开始
        size_t j = i;
        while (j < n && std::isdigit((unsigned char)q[j])) ++j;
        if (j - i != 4) continue;  // 不是 4 位年份
        for (size_t k = j; k < n; ++k) {
            unsigned char c = (unsigned char)q[k];
            if (c >= '1' && c <= '4') return c - '0';
            if (c >= '0' && c <= '9') return 0;  // 年份后先遇到别的数字（如日期），格式不符
        }
        return 0;
    }
    return 0;
}

// 生成季度标签，如 "2026Q3"（纯 ASCII，仅用于日志/展示）。
// 这里刻意不拼接中文：中文字面量会随执行字符集（MSVC 中文环境=GBK，g++=UTF-8）
// 变化，而季度判定已改为使用数值 qNum，标签不再参与任何逻辑判断。
std::string quarterLabel(int year, int qn) {
    char buf[24];
    fund_snprintf(buf, sizeof(buf), "%dQ%d", year, qn);
    return std::string(buf);
}

// 从基金详情页里找“查看相关ETF”链接对应的 6 位 ETF 代码。
// 该链接只出现在 ETF 联接基金的页面；中文用显式 UTF-8 字节匹配，
// 避免 MSVC 按执行字符集(GBK)编译窄字面量后匹配不到 UTF-8 页面。
std::string extractRelatedEtfCode(const std::string& html) {
    static const char kMark[] =
        "\xE6\x9F\xA5\xE7\x9C\x8B\xE7\x9B\xB8\xE5\x85\xB3"  // 查看相关
        "ETF";
    size_t p = html.find(kMark);
    if (p == std::string::npos) return "";

    // 形如：<a ... href="http://fund.eastmoney.com/159755.html">查看相关ETF></a>
    size_t h = html.rfind("href=\"", p);
    if (h == std::string::npos) return "";
    h += 6;
    size_t e = html.find('"', h);
    if (e == std::string::npos) return "";
    std::string href = html.substr(h, e - h);

    size_t slash = href.rfind('/');
    if (slash == std::string::npos) return "";
    size_t dot = href.find('.', slash);
    if (dot == std::string::npos) return "";
    std::string code = href.substr(slash + 1, dot - slash - 1);
    if (code.size() != 6) return "";
    if (code.find_first_not_of("0123456789") != std::string::npos) return "";
    return code;
}

// 查询 ETF 联接基金对应的目标 ETF。
// 普通基金页面没有“查看相关ETF”，返回空，调用方保持原结果。
bool resolveTargetEtf(SOUI::IHttpClient *pHttp, const std::string& fundCode,
                      std::string& etfCode) {
    etfCode.clear();
    if (!pHttp || fundCode.empty()) return false;

    std::string url = "https://fund.eastmoney.com/" + fundCode + ".html";
    std::string body;
    int httpCode = 0;
    std::string diag;
    if (!httpGetText(pHttp, url, url, body, httpCode, diag)) {
        SLOGI() << "[fetch] 查询相关 ETF 失败 code=" << fundCode.c_str()
                << " httpCode=" << httpCode;
        return false;
    }
    etfCode = extractRelatedEtfCode(body);
    SLOGI() << "[fetch] 相关 ETF 查询 code=" << fundCode.c_str()
            << " etf=" << etfCode.c_str();
    return !etfCode.empty();
}

// 变动标记用的中文片段（显式 UTF-8 字节，理由同上）
const char* const kUtf8XinJin = "\xE6\x96\xB0\xE8\xBF\x9B";  // 新进
const char* const kUtf8TuiChu = "\xE9\x80\x80\xE5\x87\xBA";  // 退出
const char* const kTagSep = " / ";

}  // namespace

// 下面三个小函数原先是在函数体里就地写的 lambda，VS2008 不支持 lambda，改为文件级函数。
static void AppendHoldingTag(HoldingChange& rec, const std::string& t) {
    if (!rec.tag.empty()) rec.tag += kTagSep;
    rec.tag += t;
}

static bool HoldingByQuarter(const FundHolding& a, const FundHolding& b) {
    return a.qNum < b.qNum;
}

static bool HoldingChangeDesc(const HoldingChange& a, const HoldingChange& b) {
    if (std::fabs(a.lp - b.lp) > 1e-9) return a.lp > b.lp;
    return a.code < b.code;
}

// 对 IHttpClient 做一次性公共配置（创建之后调一次即可）。
// 走 SetHeader 而不是每次请求都拼进 lpHeader，好处是常量头只出现一次：
// Windows 后端会把它 WinHttpAddRequestHeaders 到每个 request 上，
// libcurl 后端会把它拼进 CURLOPT_HTTPHEADER，两端等价。
// 注意：SetHeader 会一直累积且同键不覆盖，所以只能在这里放"全局不变"的头；
// 每次请求可能不同的 Referer / Cookie 必须走 lpHeader（见 HttpClientGet）。
void configureHttpClient(SOUI::IHttpClient *pHttp) {
    if (!pHttp) return;
    // 东财站点按 UA/Accept 做简单筛选，保持与浏览器一致
    pHttp->SetHeader("User-Agent",
                     "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
                     "(KHTML, like Gecko) Chrome/138.0.0.0 Safari/537.36");
    pHttp->SetHeader("Accept", "*/*");
    pHttp->SetHeader("Accept-Language", "zh-CN,zh;q=0.9,en;q=0.8");
    // FundArchivesDatas.aspx 是 AJAX 专用接口，缺这个头会直接 404
    pHttp->SetHeader("X-Requested-With", "XMLHttpRequest");
}

// 从指定代码直接抓取股票持仓。ETF 联接基金的回退逻辑由下方 fetchFundHoldings 负责，
// 这里只负责一次完整的“请求 + 解析”，避免回退分支与主流程重复代码。
static bool fetchFundHoldingsFromSource(SOUI::IHttpClient *pHttp,
                                        const std::string& fundCode,
                                        const std::string& year,
                                        std::vector<FundHolding>& out,
                                        std::string& errMsg) {
    out.clear();
    errMsg.clear();
    SLOGI() << "[fetch] fundCode=" << fundCode.c_str() << " year=" << year.c_str();

    // 1) 构造请求 URL（等价于 akshare 的 FundArchivesDatas.aspx 参数）
    std::string url = "https://fundf10.eastmoney.com/FundArchivesDatas.aspx"
                      "?type=jjcc&code=" + fundCode +
                      "&rt=" + makeRt() +
                      "&topline=100&year=" + year + "&month=";
    std::string referer = "https://fundf10.eastmoney.com/ccmx_" + fundCode + ".html";

    // 2) HTTP GET
    // 部分网络环境下东财会校验会话 Cookie：先用 IHttpClient 访问基金详情页取回
    // ASP.NET_SessionId（平台无关）；取不到不影响（沙箱/宽松网络无需 Cookie）。
    std::string cookie = fetchSessionCookie(pHttp, fundCode);
    int httpCode = 0;
    std::string diag;
    std::string raw = fetchRichest(pHttp, url, referer, cookie, &httpCode, diag);
    SLOGI() << "[fetch] 选用策略=" << diag.c_str() << " httpCode=" << httpCode
            << " rawBytes=" << (fund_uint64)raw.size();
    if (httpCode != 200) {
        if (httpCode == 404) {
            errMsg = "数据源返回 404。说明：该接口是 AJAX 专用接口，浏览器地址栏直接打开会返回 404"
                     "（缺 Referer/X-Requested-With 头），这是正常现象；本工具已自动带上这些头。"
                     "若仍报 404，通常是网络不可达或基金代码不存在。";
        } else if (httpCode > 0) {
            char codeBuf[16];
            fund_snprintf(codeBuf, sizeof(codeBuf), "%d", httpCode);
            errMsg = std::string("HTTP 请求失败（状态码 ") + codeBuf + "），请检查网络或基金代码";
        } else {
            errMsg = "所有请求策略均失败（无返回），请检查网络或基金代码";
        }
        return false;
    }
    if (raw.empty()) {
        errMsg = "HTTP 200 但响应体为空，请稍后重试";
        return false;
    }

    // 3) 提取 content(HTML)
    std::string content = extractContent(raw);
    if (content.empty()) {
        errMsg = "响应中未找到 content 字段";
        return false;
    }

    // 诊断：把 content 前 64 字节按十六进制打印，便于确认真实编码
    // （GBK 的“年”=C4 EA；UTF-8 的“年”=E5 B9 B4）。
    {
        std::string hex;
        size_t upto = (content.size() < 64) ? content.size() : 64;
        char tmp[8];
        for (size_t i = 0; i < upto; ++i) {
            fund_snprintf(tmp, sizeof(tmp), "%02X ", (unsigned char)content[i]);
            hex += tmp;
        }
        SLOGI() << "[parse] content 前64字节(hex): " << hex.c_str();
    }

    // 东财 FundArchivesDatas.aspx 的 content 通常为 GBK 编码；本工具后续所有解析
    // （季度标签、表头列名）均按 UTF-8 字面量匹配，因此若 content 不是合法 UTF-8，
    // 先按 GBK(代码页 936) 转 UTF-8，失败再按系统默认代码页(CP_ACP) 尝试。
    // 已是 UTF-8 的（如沙箱缓存）则保持不变。即便转码失败，下方的日期兜底也能解析出 quarters。
    if (!IsValidUtf8(content)) {
        std::string conv = MbsToUtf8(content, 936);
        if (!IsValidUtf8(conv)) {
            std::string conv2 = MbsToUtf8(content, ::GetACP());
            if (!conv2.empty()) conv.swap(conv2);
        }
        if (!conv.empty() && IsValidUtf8(conv)) {
            SLOGI() << "[parse] content 非合法 UTF-8，已转 UTF-8(原长度=" << (fund_uint64)content.size()
                    << " 新长度=" << (fund_uint64)conv.size() << ")，用于正确显示中文";
            content.swap(conv);
        } else {
            SLOGI() << "[parse] content 非合法 UTF-8，且 GBK/ACP 转 UTF-8 均失败，保留原字节（季度/列将用编码无关兜底解析）";
        }
    } else {
        SLOGI() << "[parse] content 已是合法 UTF-8，无需转码";
    }

    // 4) 解析：按“每个 <table> 关联其前面最近的 <h4> 截止日期”来判定季度。
    //    日期 "YYYY-MM-DD" 为纯 ASCII，与编码无关；且天然做到“表与季度”一一对应，
    //    不会出现“季度标签数组”与“表格数组”按位置错位、导致只取到一个季度的问题。
    //    （历史上“只显示 Q2”正是源于：编码相关的中文标签识别少识别到一个季度后，
    //      quarters.size() 变 1，smin(1,3)=1，只处理第一个 table，其余两季整段丢失。）
    std::vector<std::pair<size_t, size_t> > tblSpans;
    {
        size_t p = 0;
        while (true) {
            size_t s = findCI(content, "<table", p);
            if (s == std::string::npos) break;
            size_t e = findCI(content, "</table>", s);
            if (e == std::string::npos) break;
            tblSpans.push_back(std::make_pair(s, e + 8));
            p = e + 8;
        }
    }
    SLOGI() << "[parse] content-bytes=" << (fund_uint64)content.size()
            << " tables=" << (int)tblSpans.size();
    if (tblSpans.empty()) {
        errMsg = "未解析到任何持仓表格";
        return false;
    }

    int qHits = 0;
    for (size_t ti = 0; ti < tblSpans.size(); ++ti) {
        size_t tStart = tblSpans[ti].first;
        // 找该 table 之前最近的 </h4>，再找其 <h4 起点，从块内 px12'>日期 推季度
        int qn = 0, qYear = 0;
        size_t h4e = content.rfind("</h4>", tStart);
        if (h4e != std::string::npos) {
            size_t h4s = content.rfind("<h4", h4e);
            if (h4s != std::string::npos) {
                std::string block = content.substr(h4s, h4e + 5 - h4s);
                size_t ds = block.find("px12'>");
                if (ds != std::string::npos) {
                    ds += 6;
                    size_t de = block.find('<', ds);
                    std::string date = block.substr(ds, (de == std::string::npos ? block.size() : de) - ds);
                    int y = 0, mo = 0, da = 0;
                    if (sscanf(date.c_str(), "%d-%d-%d", &y, &mo, &da) == 3 &&
                        y >= 2000 && mo >= 1 && mo <= 12) {
                        qn = (mo - 1) / 3 + 1;
                        qYear = y;
                    }
                }
            }
        }
        if (qn < 1 || qn > 3) {
            SLOGI() << "[parse]   table[" << (int)ti << "] 无法从前置 h4 判定季度（非持仓表？），跳过";
            continue;
        }
        ++qHits;
        std::string label = quarterLabel(qYear, qn);  // 显式 UTF-8 构造，与执行字符集无关

        std::vector<std::vector<std::string> > rows =
            parseTableRows(content.substr(tblSpans[ti].first, tblSpans[ti].second - tblSpans[ti].first));
        if (rows.size() < 2) continue;  // 至少要有表头 + 一行数据
        ColMap cm = mapColumns(rows[0]);
        SLOGI() << "[parse]   table[" << (int)ti << "] 季度=" << qn
                << " 列映射 code=" << cm.code << " name=" << cm.name
                << " pct=" << cm.pct << " shares=" << cm.shares << " mv=" << cm.mv
                << " rows=" << (int)rows.size();
        for (size_t r = 1; r < rows.size(); ++r) {
            const std::vector<std::string>& row = rows[r];
            FundHolding h;
            h.quarter = label;
            h.qNum = qn;  // 季度号以数值下发，透视阶段不再解析字符串（编码无关）
            h.code = cellGet(row, cm.code);
            h.name = cellGet(row, cm.name);
            h.pct = normalizeNumber(stripPct(cellGet(row, cm.pct)));
            h.shares = normalizeNumber(cellGet(row, cm.shares));
            h.mv = normalizeNumber(cellGet(row, cm.mv));
            if (h.code.empty() && h.name.empty()) continue;
            out.push_back(h);
        }
    }
    SLOGI() << "[parse] 命中季度表格数=" << qHits;

    // 5) 等价于 akshare：按季度升序排列（Q1,Q2,Q3...）。用数值季度号排序，
    //    不再比较 quarter 字符串（字符串比较依赖编码，且格式稍有差异就会排错）。
    std::stable_sort(out.begin(), out.end(), HoldingByQuarter);

    if (out.empty()) errMsg = "解析完成但无持仓数据";
    return true;
}

bool fetchFundHoldings(SOUI::IHttpClient *pHttp,
                       const std::string& fundCode,
                       const std::string& year,
                       std::vector<FundHolding>& out,
                       std::string& errMsg) {
    out.clear();
    errMsg.clear();

    std::string firstErr;
    const bool ok = fetchFundHoldingsFromSource(pHttp, fundCode, year, out, firstErr);
    if (ok && !out.empty()) return true;

    // 没有股票持仓时，检查是否为 ETF 联接基金。联接基金主要持有目标 ETF，
    // 天天基金的股票持仓接口会返回空 content；此时改用目标 ETF 的股票持仓。
    std::string etfCode;
    if (resolveTargetEtf(pHttp, fundCode, etfCode) && etfCode != fundCode) {
        SLOGI() << "[fetch] 联接基金改用目标 ETF code=" << etfCode.c_str();
        std::vector<FundHolding> etfHoldings;
        std::string etfErr;
        const bool etfOk =
            fetchFundHoldingsFromSource(pHttp, etfCode, year, etfHoldings, etfErr);
        if (etfOk && !etfHoldings.empty()) {
            out.swap(etfHoldings);
            errMsg.clear();
            return true;
        }
        SLOGI() << "[fetch] 目标 ETF 仍无持仓 code=" << etfCode.c_str()
                << " err=" << etfErr.c_str();
    }

    errMsg = firstErr;
    return ok;
}

// 将扁平的（季度×股票）持仓列表，透视成“按股票”的持仓变化表。
// 与 gen_change_table.py 的逻辑对齐：latest_pct 取 Q3>Q2>Q1 中最新存在的占净值比例；
// Δ = Q3 - Q1（仅当二者都存在时有效）；持有季度用 √/— 表示；变动标记含 Q2/Q3 新进与退出。
bool buildHoldingsChange(const std::vector<FundHolding>& holdings,
                         std::vector<HoldingChange>& out) {
    out.clear();
    SLOGI() << "[pivot] 输入扁平记录=" << (int)holdings.size();
    if (holdings.empty()) return true;

    int nSkipped = 0;
    std::map<std::string, HoldingChange> m;
    for (size_t hi = 0; hi < holdings.size(); ++hi) {
        const FundHolding& h = holdings[hi];
        // 优先用解析阶段写入的数值季度号（编码无关）；仅在缺失时才回退解析标签
        int qi = h.qNum;
        if (qi < 1 || qi > 4) qi = quarterIndex(h.quarter);
        if (qi < 1 || qi > 3) {
            ++nSkipped;
            if (nSkipped <= 3) {
                SLOGI() << "[pivot] 跳过无法判定季度的记录 code=" << h.code.c_str()
                        << " qNum=" << h.qNum << " quarter=" << h.quarter.c_str();
            }
            continue;
        }
        int idx = qi - 1;
        HoldingChange& rec = m[h.code];
        rec.code = h.code;
        if (!h.name.empty()) rec.name = h.name;  // 同一代码名称一致，取首个非空
        rec.hasQ[idx] = true;
        rec.pct[idx] = std::strtod(h.pct.c_str(), NULL);
        rec.shares[idx] = std::strtod(h.shares.c_str(), NULL);
        rec.mv[idx] = std::strtod(h.mv.c_str(), NULL);
    }

    for (std::map<std::string, HoldingChange>::iterator it = m.begin(); it != m.end(); ++it) {
        HoldingChange& rec = it->second;
        // 最新季度占净值（Q3>Q2>Q1 优先）
        for (int i = 2; i >= 0; --i) {
            if (rec.hasQ[i]) { rec.lp = rec.pct[i]; rec.lpQuarter = i + 1; break; }
        }
        // 持有季度状态
        rec.status.clear();
        for (int i = 0; i < 3; ++i) rec.status += rec.hasQ[i] ? "\xe2\x9c\x93" : "\xe2\x80\x94";
        // 变动标记（与 Python 脚本一致）。中文片段用显式 UTF-8 字节，
        // 避免 MSVC 按执行字符集(GBK)编码后与界面侧 CP_UTF8 转换不一致导致乱码。
        if (rec.hasQ[1] && !rec.hasQ[0]) AppendHoldingTag(rec, std::string("Q2") + kUtf8XinJin);
        if (rec.hasQ[2] && !rec.hasQ[1]) AppendHoldingTag(rec, std::string("Q3") + kUtf8XinJin);
        if (rec.hasQ[0] && !rec.hasQ[1]) AppendHoldingTag(rec, std::string("Q2") + kUtf8TuiChu);
        if (rec.hasQ[1] && !rec.hasQ[2]) AppendHoldingTag(rec, std::string("Q3") + kUtf8TuiChu);
        // Q3 - Q1 变化量
        if (rec.hasQ[2] && rec.hasQ[0]) {
            rec.pctChg = rec.pct[2] - rec.pct[0];
            rec.shChg = rec.shares[2] - rec.shares[0];
            rec.mvChg = rec.mv[2] - rec.mv[0];
            rec.chgValid = true;
        }
        out.push_back(rec);
    }

    // 排序：最新季度占净值降序，再按代码升序
    std::stable_sort(out.begin(), out.end(), HoldingChangeDesc);
    SLOGI() << "[pivot] 输出股票数=" << (int)out.size() << " 跳过=" << nSkipped;
    return true;
}

// 通用 HTTP GET：复用持仓接口的策略逻辑，供行情/净值等 JSON 接口使用。
// 这些接口是"单一结果"，用 fetchFirstOk（第一个 200 就收工），不做季度择优。
bool httpGetText(SOUI::IHttpClient *pHttp, const std::string& url, const std::string& referer,
                 std::string& body, int& httpCode, std::string& diag) {
    body = fetchFirstOk(pHttp, url, referer, std::string(), &httpCode, diag);
    return httpCode == 200 && !body.empty();
}
