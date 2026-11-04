// fund_eastmoney.cpp
// 将 akshare.fund_portfolio_hold_em 的业务逻辑用 C++ 重写：
//   请求天天基金网 -> 提取 content(HTML) -> 解析季度标签与表格 -> 抽取持仓行
// 解析部分为纯 std::string 实现，不依赖 SOUI；仅 HTTP 部分使用 SOUI 的 IHttpClient。
#include "fund_eastmoney.h"

#include <Windows.h>
#include <Winhttp.h>

#include <souistd.h>
#include <interface/SHttpClient-i.h>
#include <commgr2.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <map>
#include <string>
#include <vector>

using namespace SOUI;

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
                        code = (int)strtol(ent.c_str() + 2, nullptr, 16);
                    else
                        code = (int)strtol(ent.c_str() + 1, nullptr, 10);
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
    for (char c : html) {
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
                std::snprintf(label, sizeof(label), "%d年%d季度股票投资明细", y, qn);
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
    auto a = findQuartersByLabel(h);
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
std::vector<std::vector<std::string>> parseTableRows(const std::string& tableHtml) {
    std::vector<std::vector<std::string>> rows;
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
    int code = -1, name = -1, pct = -1, shares = -1, mv = -1;
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
    if (!t.empty() && t.back() == '%') t.pop_back();
    return t;
}

// 规整数值字符串：去掉千分位逗号，并把 "1.30"/"0.00" 规整为 "1.3"/"0.0"，
// 与 akshare 的 pd.to_numeric 行为保持一致。无法解析为数字时（如 "—"）原样返回。
std::string normalizeNumber(const std::string& s) {
    std::string t;
    for (char c : s) {
        if (c == ',' || c == ' ' || c == '\t' || c == '\r' || c == '\n') continue;
        t += c;
    }
    if (t.empty()) return t;
    char* endp = nullptr;
    double d = std::strtod(t.c_str(), &endp);
    if (endp == t.c_str() || *endp != '\0') return t;  // 非纯数字
    if (std::fabs(d) < 1e-9) return "0.0";
    if (std::fabs(d - std::floor(d + 0.5)) < 1e-9 && std::fabs(d) < 1e15) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.0f.0", d);  // 整数值 -> "X.0"
        return std::string(buf);
    }
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.6f", d);
    std::string r(buf);
    size_t dot = r.find('.');
    if (dot != std::string::npos) {
        while (r.size() > dot + 1 && r.back() == '0') r.pop_back();
        if (r.back() == '.') r += '0';
    }
    return r;
}

const std::string& cellGet(const std::vector<std::string>& row, int idx) {
    static const std::string empty;
    if (idx < 0 || (size_t)idx >= row.size()) return empty;
    return row[idx];
}

// ---------- 会话 Cookie 引导（原生 WinHTTP）----------
// 部分网络环境下，东财 fundf10 接口会校验会话 Cookie：浏览器先访问基金详情页拿到
// ASP.NET_SessionId，再带着 Cookie 发起 AJAX 才不会 404。下面用原生 WinHTTP 访问基金页
// 取回 Set-Cookie，供后续请求通过 IHttpClient 的 Cookie 头回带（模拟浏览器会话）。
static std::wstring WidenUtf8(const std::string& s) {
    int n = ::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (n <= 0) return {};
    std::wstring w; w.resize((size_t)n);
    ::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], n);
    if (!w.empty() && w.back() == L'\0') w.pop_back();
    return w;
}
static std::string NarrowUtf8(const std::wstring& w) {
    int n = ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (n <= 0) return {};
    std::string s; s.resize((size_t)n);
    ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &s[0], n, nullptr, nullptr);
    if (!s.empty() && s.back() == '\0') s.pop_back();
    return s;
}

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
    int n = ::MultiByteToWideChar(codePage, 0, s.c_str(), -1, nullptr, 0);
    if (n <= 0) return s;
    std::wstring w; w.resize((size_t)n);
    ::MultiByteToWideChar(codePage, 0, s.c_str(), -1, &w[0], n);
    if (!w.empty() && w.back() == L'\0') w.pop_back();
    int m = ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (m <= 0) return s;
    std::string o; o.resize((size_t)m);
    ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &o[0], m, nullptr, nullptr);
    if (!o.empty() && o.back() == '\0') o.pop_back();
    return o;
}

// 从响应句柄读取 Set-Cookie，提取 "name=value" 片段（剔除 path/expires 等属性），拼成 Cookie 头值
static std::string ExtractSetCookie(HINTERNET hRequest) {
    DWORD dwSize = 0;
    if (!::WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_SET_COOKIE, WINHTTP_HEADER_NAME_BY_INDEX,
                               nullptr, &dwSize, nullptr)) {
        if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER) return "";
    }
    if (dwSize == 0) return "";
    std::wstring buf; buf.resize((size_t)dwSize / sizeof(wchar_t));
    if (!::WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_SET_COOKIE, WINHTTP_HEADER_NAME_BY_INDEX,
                               &buf[0], &dwSize, nullptr)) return "";
    std::string raw = NarrowUtf8(buf);

    std::string cookie;
    size_t pos = 0;
    while (pos < raw.size()) {
        size_t semi = raw.find(';', pos);
        std::string part = (semi == std::string::npos) ? raw.substr(pos) : raw.substr(pos, semi - pos);
        size_t a = part.find_first_not_of(" \t\r\n");
        size_t b = part.find_last_not_of(" \t\r\n");
        if (a == std::string::npos) { if (semi == std::string::npos) break; pos = semi + 1; continue; }
        part = part.substr(a, b - a + 1);
        size_t eq = part.find('=');
        if (eq != std::string::npos) {
            std::string name = part.substr(0, eq);
            for (char& ch : name) ch = (char)::tolower((unsigned char)ch);  // 属性名大小写不敏感
            if (name == "path" || name == "expires" || name == "domain" || name == "secure" ||
                name == "httponly" || name == "samesite" || name == "max-age" || name.empty()) {
                // 跳过属性，只保留 名称=值
            } else {
                if (!cookie.empty()) cookie += "; ";
                cookie += part;
            }
        }
        if (semi == std::string::npos) break;
        pos = semi + 1;
    }
    return cookie;
}

// 访问基金详情页，取回会话 Cookie（优先直连，失败时回退系统代理）
std::string fetchSessionCookie(const std::string& fundCode) {
    std::wstring host = WidenUtf8("fundf10.eastmoney.com");
    std::wstring path = WidenUtf8("/ccmx_" + fundCode + ".html");
    const DWORD proxyModes[2] = { WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY };
    for (int i = 0; i < 2; ++i) {
        HINTERNET hS = ::WinHttpOpen(L"Mozilla/5.0", proxyModes[i],
                                     WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (!hS) continue;
        HINTERNET hC = ::WinHttpConnect(hS, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (!hC) { ::WinHttpCloseHandle(hS); continue; }
        HINTERNET hR = ::WinHttpOpenRequest(hC, L"GET", path.c_str(), nullptr,
                                            WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                                            WINHTTP_FLAG_SECURE);
        if (!hR) { ::WinHttpCloseHandle(hC); ::WinHttpCloseHandle(hS); continue; }
        BOOL ok = ::WinHttpSendRequest(hR, WINHTTP_NO_ADDITIONAL_HEADERS, 0, nullptr, 0, 0, 0) &&
                  ::WinHttpReceiveResponse(hR, nullptr);
        std::string cookie;
        if (ok) cookie = ExtractSetCookie(hR);
        ::WinHttpCloseHandle(hR); ::WinHttpCloseHandle(hC); ::WinHttpCloseHandle(hS);
        if (!cookie.empty()) return cookie;
    }
    return "";
}

// ---------- HTTP（原生 WinHTTP，直连优先，便于日志与绕过代理）----------
// 日志统一走 SOUI 的 SLOGI()（参考 demos/demo）。需在 main.cpp 中通过
// SLog::setLogCallback 注册文件回调，把日志同时落盘到 fundholdings_demo.log 以便排查。

// 解析 https://host[:port]/path?query -> host / page(含 query) / port / https
static bool ParseUrlWin(const std::string& url, std::string& host, std::string& page,
                        WORD& port, bool& https) {
    std::string t = url;
    size_t p = t.find("://");
    if (p != std::string::npos) t = t.substr(p + 3);
    https = (url.compare(0, 5, "https") == 0);
    port = https ? (WORD)INTERNET_DEFAULT_HTTPS_PORT : 80;
    size_t slash = t.find('/');
    if (slash == std::string::npos) { host = t; page = "/"; }
    else { host = t.substr(0, slash); page = t.substr(slash); }
    size_t colon = host.find(':');
    if (colon != std::string::npos) {
        port = (WORD)atoi(host.substr(colon + 1).c_str());
        host = host.substr(0, colon);
    }
    return !host.empty();
}

// 单次原生 WinHTTP GET；返回 body；outHttpCode 传出状态码；全程写日志。
static std::string RawWinHttpGet(const std::string& url,
                                 const std::vector<std::pair<std::string, std::string>>& hdrs,
                                 int* outHttpCode,
                                 DWORD accessType) {
    std::string host, page; WORD port = 0; bool https = false;
    if (!ParseUrlWin(url, host, page, port, https)) { if (outHttpCode) *outHttpCode = -1; return ""; }
    std::wstring whost = WidenUtf8(host), wpage = WidenUtf8(page);
    SLOGI() << "[HTTP] access=" << (accessType == WINHTTP_ACCESS_TYPE_NO_PROXY ? "NO_PROXY" : "DEFAULT_PROXY")
            << " url=" << url.c_str();
    for (auto& kv : hdrs) SLOGI() << "[HTTP]   header " << kv.first.c_str() << ": " << kv.second.c_str();

    HINTERNET hS = ::WinHttpOpen(L"Mozilla/5.0", accessType, WINHTTP_NO_PROXY_NAME,
                                 WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hS) { SLOGI() << "[HTTP] WinHttpOpen failed, err=" << (int)::GetLastError();
               if (outHttpCode) *outHttpCode = -1; return ""; }
    HINTERNET hC = ::WinHttpConnect(hS, whost.c_str(), port, 0);
    if (!hC) { SLOGI() << "[HTTP] WinHttpConnect failed, err=" << (int)::GetLastError();
               ::WinHttpCloseHandle(hS); if (outHttpCode) *outHttpCode = -1; return ""; }
    HINTERNET hR = ::WinHttpOpenRequest(hC, L"GET", wpage.c_str(), nullptr, WINHTTP_NO_REFERER,
                                        WINHTTP_DEFAULT_ACCEPT_TYPES, https ? WINHTTP_FLAG_SECURE : 0);
    if (!hR) { SLOGI() << "[HTTP] WinHttpOpenRequest failed, err=" << (int)::GetLastError();
               ::WinHttpCloseHandle(hC); ::WinHttpCloseHandle(hS); if (outHttpCode) *outHttpCode = -1; return ""; }
    if (https) {
        DWORD f = SECURITY_FLAG_SECURE | SECURITY_FLAG_IGNORE_UNKNOWN_CA |
                  SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE | SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
                  SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
        ::WinHttpSetOption(hR, WINHTTP_OPTION_SECURITY_FLAGS, &f, sizeof(f));
    }
    std::wstring whead;
    for (auto& kv : hdrs) {
        if (!whead.empty()) whead += L"\r\n";
        whead += WidenUtf8(kv.first + ": " + kv.second);
    }
    if (!::WinHttpSendRequest(hR, whead.c_str(), (DWORD)whead.size(), nullptr, 0, 0, 0)) {
        SLOGI() << "[HTTP] WinHttpSendRequest failed, err=" << (int)::GetLastError();
        ::WinHttpCloseHandle(hR); ::WinHttpCloseHandle(hC); ::WinHttpCloseHandle(hS);
        if (outHttpCode) *outHttpCode = -1; return "";
    }
    if (!::WinHttpReceiveResponse(hR, nullptr)) {
        SLOGI() << "[HTTP] WinHttpReceiveResponse failed, err=" << (int)::GetLastError();
        ::WinHttpCloseHandle(hR); ::WinHttpCloseHandle(hC); ::WinHttpCloseHandle(hS);
        if (outHttpCode) *outHttpCode = -1; return "";
    }
    int code = -1;
    wchar_t codeBuf[16] = {0}; DWORD cds = sizeof(codeBuf);
    if (::WinHttpQueryHeaders(hR, WINHTTP_QUERY_STATUS_CODE, WINHTTP_HEADER_NAME_BY_INDEX,
                              codeBuf, &cds, nullptr))
        code = (int)wcstoul(codeBuf, nullptr, 10);
    if (outHttpCode) *outHttpCode = code;
    SLOGI() << "[HTTP] status=" << code;

    DWORD rhSize = 0;
    if (::WinHttpQueryHeaders(hR, WINHTTP_QUERY_RAW_HEADERS, WINHTTP_HEADER_NAME_BY_INDEX,
                              nullptr, &rhSize, WINHTTP_NO_HEADER_INDEX) && rhSize) {
        std::wstring rh; rh.resize((size_t)rhSize / sizeof(wchar_t));
        if (::WinHttpQueryHeaders(hR, WINHTTP_QUERY_RAW_HEADERS, WINHTTP_HEADER_NAME_BY_INDEX,
                                  &rh[0], &rhSize, WINHTTP_NO_HEADER_INDEX)) {
            std::string rhs = NarrowUtf8(rh);
            if (rhs.size() > 600) rhs = rhs.substr(0, 600) + "...";
            SLOGI() << "[HTTP] response-headers:\n" << rhs.c_str();
        }
    }
    std::string body; DWORD avail = 0;
    while (::WinHttpQueryDataAvailable(hR, &avail) && avail) {
        char buf[65536]; DWORD rd = 0;
        if (!::WinHttpReadData(hR, buf, avail > sizeof(buf) ? sizeof(buf) : avail, &rd)) break;
        body.append(buf, rd);
    }
    std::string preview = body;
    if (preview.size() > 300) preview = preview.substr(0, 300) + "...";
    SLOGI() << "[HTTP] body-bytes=" << (unsigned long long)body.size() << " preview:\n" << preview.c_str();
    ::WinHttpCloseHandle(hR); ::WinHttpCloseHandle(hC); ::WinHttpCloseHandle(hS);
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

// 等价于 akshare 的请求，但改为“多策略择优”：依次尝试 4 种组合
// （直连 NO_PROXY / 系统代理 DEFAULT_PROXY） × （带会话 Cookie / 不带），
// 返回“季度段数最多”的响应体。这样可同时规避两类问题：
//   1) 系统代理对该域名直接返回 404（历史上出现过）→ 该策略被跳过，选其它可用策略；
//   2) 直连被缓存代理回了“陈旧的一季”响应，而系统代理带 Cookie 能拿到全量三季
//      → 旧逻辑因直连已 200 就返回、永不试代理，故改为“都试、选最全”。
// outDiag 回传最终选用的策略名，便于日志排查。
std::string fetchRichest(const std::string& url, const std::string& referer,
                         const std::string& cookie, int* outHttpCode,
                         std::string& outDiag) {
    struct Variant { DWORD access; bool withCookie; const char* name; };
    static const Variant vars[] = {
        {WINHTTP_ACCESS_TYPE_NO_PROXY,      true,  "NO_PROXY+ck"},
        {WINHTTP_ACCESS_TYPE_NO_PROXY,      false, "NO_PROXY"},
        {WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, true,  "DEF_PROXY+ck"},
        {WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, false, "DEF_PROXY"},
    };
    std::string bestBody; int bestQ = -1; int bestCode = 0; std::string bestName;
    for (const auto& v : vars) {
        std::vector<std::pair<std::string, std::string>> hdrs = {
            {"Accept", "*/*"},
            {"Accept-Language", "zh-CN,zh;q=0.9,en;q=0.8"},
            {"Referer", referer},
            {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
                           "(KHTML, like Gecko) Chrome/138.0.0.0 Safari/537.36"},
            {"X-Requested-With", "XMLHttpRequest"},
        };
        if (v.withCookie && !cookie.empty()) hdrs.push_back({"Cookie", cookie});
        int code = 0;
        std::string body = RawWinHttpGet(url, hdrs, &code, v.access);
        int q = (code == 200 && !body.empty()) ? CountQuarters(body) : 0;
        SLOGI() << "[fetch] strategy=" << v.name << " code=" << code
                << " body=" << (unsigned long long)body.size() << " quarters=" << q;
        if (q > bestQ || (q == bestQ && body.size() > bestBody.size())) {
            bestQ = q; bestBody = std::move(body); bestCode = code; bestName = v.name;
        }
    }
    if (outHttpCode) *outHttpCode = bestCode;
    outDiag = bestName;
    return bestBody;
}

std::string makeRt() {
    static bool seeded = false;
    if (!seeded) { std::srand((unsigned)std::time(nullptr)); seeded = true; }
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%f", (double)std::rand() / RAND_MAX);
    return std::string(buf);
}

// 从季度标签 "2026年3季度股票投资明细" 中提取季度序号（1/2/3/4）。
// 注意“年”是多字节 UTF-8，年份也是 ASCII，但季度数字紧跟在“年”之后，
// 因此从“年”之后扫描第一个 ASCII 数字即可得到季度序号。
int quarterIndex(const std::string& q) {
    size_t ny = q.find("\xe5\xb9\xb4");  // UTF-8 的"年"
    if (ny == std::string::npos) {
        // 兜底：找不到"年"时直接返回 0（跳过该季度），
        // 严禁扫描整串找第一个 1-4 数字——否则会命中年份"2026"里的 '2'，
        // 把本该是 Q1/Q3 的记录全部误判成 Q2（历史"只显示 Q2"的根因之一）。
        return 0;
    }
    for (size_t i = ny + 1; i < q.size(); ++i) {
        if (q[i] >= '1' && q[i] <= '4') return q[i] - '0';
        if ((unsigned char)q[i] == 0xE5) break;  // 遇到下一个中文字节（"季"）停止
    }
    return 0;
}

}  // namespace

bool fetchFundHoldings(const std::string& fundCode,
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
    // 部分网络环境下东财会校验会话 Cookie：先用原生 WinHTTP 访问基金详情页取回
    // ASP.NET_SessionId；取不到不影响（沙箱/宽松网络无需 Cookie）。
    std::string cookie = fetchSessionCookie(fundCode);
    int httpCode = 0;
    std::string diag;
    std::string raw = fetchRichest(url, referer, cookie, &httpCode, diag);
    SLOGI() << "[fetch] 选用策略=" << diag.c_str() << " httpCode=" << httpCode
            << " rawBytes=" << (unsigned long long)raw.size();
    if (httpCode != 200) {
        if (httpCode == 404) {
            errMsg = "数据源返回 404。说明：该接口是 AJAX 专用接口，浏览器地址栏直接打开会返回 404"
                     "（缺 Referer/X-Requested-With 头），这是正常现象；本工具已自动带上这些头。"
                     "若仍报 404，通常是网络不可达或基金代码不存在。";
        } else if (httpCode > 0) {
            errMsg = "HTTP 请求失败（状态码 " + std::to_string(httpCode) + "），请检查网络或基金代码";
        } else {
            errMsg = "所有请求策略均失败（无返回），请检查网络或基金代码";
        }
        return false;
    }
    if (raw.empty()) {
        errMsg = "HTTP 200 但响应体为空，请稍后重试";
        return false;
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
            std::snprintf(tmp, sizeof(tmp), "%02X ", (unsigned char)content[i]);
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
            if (!conv2.empty()) conv = std::move(conv2);
        }
        if (!conv.empty() && IsValidUtf8(conv)) {
            SLOGI() << "[parse] content 非合法 UTF-8，已转 UTF-8(原长度=" << (unsigned long long)content.size()
                    << " 新长度=" << (unsigned long long)conv.size() << ")，用于正确显示中文";
            content = std::move(conv);
        } else {
            SLOGI() << "[parse] content 非合法 UTF-8，且 GBK/ACP 转 UTF-8 均失败，保留原字节（季度/列将用编码无关兜底解析）";
        }
    } else {
        SLOGI() << "[parse] content 已是合法 UTF-8，无需转码";
    }

    // 4) 解析：按“每个 <table> 关联其前面最近的 <h4> 截止日期”来判定季度。
    //    日期 "YYYY-MM-DD" 为纯 ASCII，与编码无关；且天然做到“表↔季度”一一对应，
    //    不会出现“季度标签数组”与“表格数组”按位置错位、导致只取到一个季度的问题。
    //    （历史上“只显示 Q2”正是源于：编码相关的中文标签识别少识别到一个季度后，
    //      quarters.size() 变 1，smin(1,3)=1，只处理第一个 table，其余两季整段丢失。）
    std::vector<std::pair<size_t, size_t>> tblSpans;
    {
        size_t p = 0;
        while (true) {
            size_t s = findCI(content, "<table", p);
            if (s == std::string::npos) break;
            size_t e = findCI(content, "</table>", s);
            if (e == std::string::npos) break;
            tblSpans.push_back({s, e + 8});
            p = e + 8;
        }
    }
    SLOGI() << "[parse] content-bytes=" << (unsigned long long)content.size()
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
        char label[64];
        std::snprintf(label, sizeof(label), "%d年%d季度股票投资明细", qYear, qn);

        auto rows = parseTableRows(content.substr(tblSpans[ti].first, tblSpans[ti].second - tblSpans[ti].first));
        if (rows.size() < 2) continue;  // 至少要有表头 + 一行数据
        ColMap cm = mapColumns(rows[0]);
        SLOGI() << "[parse]   table[" << (int)ti << "] 季度=" << qn
                << " 列映射 code=" << cm.code << " name=" << cm.name
                << " pct=" << cm.pct << " shares=" << cm.shares << " mv=" << cm.mv
                << " rows=" << (int)rows.size();
        for (size_t r = 1; r < rows.size(); ++r) {
            const auto& row = rows[r];
            FundHolding h;
            h.quarter = label;
            h.code = cellGet(row, cm.code);
            h.name = cellGet(row, cm.name);
            h.pct = normalizeNumber(stripPct(cellGet(row, cm.pct)));
            h.shares = normalizeNumber(cellGet(row, cm.shares));
            h.mv = normalizeNumber(cellGet(row, cm.mv));
            if (h.code.empty() && h.name.empty()) continue;
            out.push_back(std::move(h));
        }
    }
    SLOGI() << "[parse] 命中季度表格数=" << qHits;

    // 5) 等价于 akshare：按季度升序排列（Q1,Q2,Q3...）
    std::stable_sort(out.begin(), out.end(),
                     [](const FundHolding& a, const FundHolding& b) {
                         return a.quarter < b.quarter;
                     });

    if (out.empty()) errMsg = "解析完成但无持仓数据";
    return true;
}

// 将扁平的（季度×股票）持仓列表，透视成“按股票”的持仓变化表。
// 与 gen_change_table.py 的逻辑对齐：latest_pct 取 Q3>Q2>Q1 中最新存在的占净值比例；
// Δ = Q3 - Q1（仅当二者都存在时有效）；持有季度用 ✓/— 表示；变动标记含 Q2/Q3 新进与退出。
bool buildHoldingsChange(const std::vector<FundHolding>& holdings,
                         std::vector<HoldingChange>& out) {
    out.clear();
    if (holdings.empty()) return true;

    std::map<std::string, HoldingChange> m;
    for (const auto& h : holdings) {
        int qi = quarterIndex(h.quarter);
        if (qi < 1 || qi > 3) continue;
        int idx = qi - 1;
        HoldingChange& rec = m[h.code];
        rec.code = h.code;
        if (!h.name.empty()) rec.name = h.name;  // 同一代码名称一致，取首个非空
        rec.hasQ[idx] = true;
        rec.pct[idx] = std::strtod(h.pct.c_str(), nullptr);
        rec.shares[idx] = std::strtod(h.shares.c_str(), nullptr);
        rec.mv[idx] = std::strtod(h.mv.c_str(), nullptr);
    }

    for (auto& kv : m) {
        HoldingChange& rec = kv.second;
        // 最新季度占净值（Q3>Q2>Q1 优先）
        for (int i = 2; i >= 0; --i) {
            if (rec.hasQ[i]) { rec.lp = rec.pct[i]; rec.lpQuarter = i + 1; break; }
        }
        // 持有季度状态
        rec.status.clear();
        for (int i = 0; i < 3; ++i) rec.status += rec.hasQ[i] ? "\xe2\x9c\x93" : "\xe2\x80\x94";
        // 变动标记（与 Python 脚本一致）
        auto addTag = [&rec](const char* t) {
            if (!rec.tag.empty()) rec.tag += " / ";
            rec.tag += t;
        };
        if (rec.hasQ[1] && !rec.hasQ[0]) addTag("Q2新进");
        if (rec.hasQ[2] && !rec.hasQ[1]) addTag("Q3新进");
        if (rec.hasQ[0] && !rec.hasQ[1]) addTag("Q2退出");
        if (rec.hasQ[1] && !rec.hasQ[2]) addTag("Q3退出");
        // Q3 - Q1 变化量
        if (rec.hasQ[2] && rec.hasQ[0]) {
            rec.pctChg = rec.pct[2] - rec.pct[0];
            rec.shChg = rec.shares[2] - rec.shares[0];
            rec.mvChg = rec.mv[2] - rec.mv[0];
            rec.chgValid = true;
        }
        out.push_back(std::move(rec));
    }

    // 排序：最新季度占净值降序，再按代码升序
    std::stable_sort(out.begin(), out.end(),
                     [](const HoldingChange& a, const HoldingChange& b) {
                         if (std::fabs(a.lp - b.lp) > 1e-9) return a.lp > b.lp;
                         return a.code < b.code;
                     });
    return true;
}
