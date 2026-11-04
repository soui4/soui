// applog.cpp - 把 SLog 的日志同时送进内存队列（供界面「请求日志」显示）与 log4z。
// 公共头（含 windows.h / helper/slog.h）统一由 stdafx.h 提供。
#include "stdafx.h"
#include "applog.h"

// ---------------------------------------------------------------- 临界区实现
struct CLogLock::Impl {
    CRITICAL_SECTION cs;
};

CLogLock::CLogLock() : m_impl(new Impl) {
    ::InitializeCriticalSection(&m_impl->cs);
}

CLogLock::~CLogLock() {
    ::DeleteCriticalSection(&m_impl->cs);
    delete m_impl;
    m_impl = NULL;
}

void CLogLock::Enter() {
    ::EnterCriticalSection(&m_impl->cs);
}

void CLogLock::Leave() {
    ::LeaveCriticalSection(&m_impl->cs);
}

namespace {

// 日志正文里既有 UTF-8（股票名/基金名来自网络）又有本地字符集（MSVC 中文环境下的
// 窄字面量是 GBK）。这里先按 UTF-8 试解码，失败再按本地代码页处理，两种来源都能正常显示。
bool LooksUtf8(const std::string &s) {
    size_t i = 0, n = s.size();
    while (i < n) {
        unsigned char c = (unsigned char)s[i];
        if (c < 0x80) {
            ++i;
            continue;
        }
        int extra;
        if ((c & 0xE0) == 0xC0) extra = 1;
        else if ((c & 0xF0) == 0xE0) extra = 2;
        else if ((c & 0xF8) == 0xF0) extra = 3;
        else return false;
        if (i + (size_t)extra >= n) return false;
        for (int k = 1; k <= extra; ++k)
            if (((unsigned char)s[i + k] & 0xC0) != 0x80) return false;
        i += (size_t)extra + 1;
    }
    return true;
}

// 纯手工 UTF-8 解码（不依赖 MultiByteToWideChar，非 Windows 平台也能用）
std::wstring DecodeUtf8(const std::string &s) {
    std::wstring out;
    out.reserve(s.size());
    size_t i = 0, n = s.size();
    while (i < n) {
        unsigned char c = (unsigned char)s[i];
        if (c < 0x80) {
            out += (wchar_t)c;
            ++i;
            continue;
        }
        int extra = 0;
        unsigned int cp = 0;
        if ((c & 0xE0) == 0xC0) {
            extra = 1;
            cp = c & 0x1F;
        } else if ((c & 0xF0) == 0xE0) {
            extra = 2;
            cp = c & 0x0F;
        } else if ((c & 0xF8) == 0xF0) {
            extra = 3;
            cp = c & 0x07;
        } else {
            out += L'?';
            ++i;
            continue;
        }
        if (i + (size_t)extra >= n) {
            out += L'?';
            break;
        }
        for (int k = 1; k <= extra; ++k) cp = (cp << 6) | ((unsigned char)s[i + k] & 0x3F);
        i += (size_t)extra + 1;
        if (cp <= 0xFFFF) {
            out += (wchar_t)cp;
        } else {  // 补充平面：拆成代理对
            cp -= 0x10000;
            out += (wchar_t)(0xD800 + (cp >> 10));
            out += (wchar_t)(0xDC00 + (cp & 0x3FF));
        }
    }
    return out;
}

std::wstring ToWide(const std::string &s) {
    if (s.empty()) return std::wstring();
    if (LooksUtf8(s)) return DecodeUtf8(s);
#ifdef _WIN32
    int n = ::MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), NULL, 0);
    if (n > 0) {
        std::wstring w;
        w.resize((size_t)n);
        ::MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), &w[0], n);
        return w;
    }
#endif
    std::wstring w;
    w.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) w += (wchar_t)(unsigned char)s[i];
    return w;
}

// 级别缩写，和 log4z 的 T/D/I/W/E/A/F 一致
const wchar_t *LevelTag(int level) {
    switch (level) {
        case SNS::LOG_LEVEL_TRACE: return L"T";
        case SNS::LOG_LEVEL_DEBUG: return L"D";
        case SNS::LOG_LEVEL_INFO: return L"I";
        case SNS::LOG_LEVEL_WARN: return L"W";
        case SNS::LOG_LEVEL_ERROR: return L"E";
        case SNS::LOG_LEVEL_ALARM: return L"A";
        case SNS::LOG_LEVEL_FATAL: return L"F";
        default: return L"?";
    }
}

// 取本地时间（时/分/秒/毫秒）。VS2008 没有 <chrono>，这里按平台各取一套。
void LocalClock(int *pHour, int *pMin, int *pSec, int *pMs) {
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    *pHour = st.wHour;
    *pMin = st.wMinute;
    *pSec = st.wSecond;
    *pMs = st.wMilliseconds;
}

}  // namespace

CLogSink::CLogSink() : m_total(0), m_prev(NULL) {
}

CLogSink::~CLogSink() {
}

CLogSink &CLogSink::Instance() {
    static CLogSink s_sink;
    return s_sink;
}

void CLogSink::Install() {
    // setLogCallback 返回的是安装前的回调（即 SAppCfg::SetLog 装上的 log4z 回调）。
    // 保存下来，在 OnLog 里继续转交，保证日志照常落盘。
    SNS::LogCallback prev = SNS::SLog::setLogCallback(&CLogSink::OnLog);
    // 重复 Install 时 prev 会是自己：此时必须断开，
    // 否则 OnLog 转交给自己就是无限递归。
    m_prev = (prev == &CLogSink::OnLog) ? NULL : prev;
}

void CLogSink::Uninstall() {
    SNS::SLog::setLogCallback(m_prev);
    m_prev = NULL;
}

void CLogSink::Clear() {
    CLogAutoLock lk(m_lock);
    m_lines.clear();
    // m_total 不回退：界面侧用「已显示到第几条」判断增量，
    // 回退会导致清空后新日志编号与旧记录冲突。
}

fund_uint64 CLogSink::Count() const {
    CLogAutoLock lk(m_lock);
    return m_total;
}

std::vector<std::wstring> CLogSink::Snapshot(fund_uint64 from, size_t maxN) const {
    std::vector<std::wstring> out;
    CLogAutoLock lk(m_lock);
    if (maxN == 0 || m_lines.empty()) return out;

    const fund_uint64 first = m_total - (fund_uint64)m_lines.size();
    if (from < first) from = first;  // 请求的行已被环形队列裁掉，退到仍保留的最早一行
    if (from >= m_total) return out;

    size_t idx = (size_t)(from - first);
    size_t n = (size_t)(m_total - from);
    if (n > maxN) {  // 新增太多时只取最近的 maxN 行，避免一次塞入上千行拖慢界面
        idx += n - maxN;
        n = maxN;
    }
    out.reserve(n);
    for (size_t i = 0; i < n; ++i) out.push_back(m_lines[idx + i]);
    return out;
}

void CLogSink::AppendLine(const std::wstring &line) {
    CLogAutoLock lk(m_lock);
    m_lines.push_back(line);
    ++m_total;
    while (m_lines.size() > (size_t)kMaxLines) m_lines.pop_front();
}

// 注意：这里跑在产生日志的那个线程上（UI 线程或工作线程）。
// 只做内存操作 + 转交原回调，不碰窗口，因此不会阻塞界面。
void CLogSink::OnLog(const char *tag, const char *logStr, int level, const char *file, int line,
                     const char *fun, void *retAddr) {
    // 先入内存队列，再交给 log4z 落盘：即使落盘比较慢，界面也能立刻看到这条日志
    CLogSink &sink = Instance();

    // 时间戳（含毫秒），一次日志只取一次，多行正文共用同一前缀
    std::wstring prefix;
    {
        int hh = 0, mm = 0, ss = 0, ms = 0;
        LocalClock(&hh, &mm, &ss, &ms);

        wchar_t head[64];
        fund_swprintf(head, 64, L"%02d:%02d:%02d.%03d ", hh, mm, ss, ms);
        prefix = head;
        prefix += L"[";
        prefix += LevelTag(level);
        prefix += L"] ";
        if (tag && *tag) {
            prefix += L"(";
            prefix += ToWide(std::string(tag));
            prefix += L") ";
        }
    }

    // 正文可能含换行（HTTP 响应头/响应体预览就是多行的），按物理行拆开逐条入队
    const std::string body = logStr ? logStr : "";
    size_t p = 0;
    while (p <= body.size()) {
        size_t q = body.find('\n', p);
        if (q == std::string::npos) q = body.size();
        std::string one = body.substr(p, q - p);
        while (!one.empty() && one[one.size() - 1] == '\r') one.erase(one.size() - 1);
        if (!one.empty()) sink.AppendLine(prefix + ToWide(one));
        if (q == body.size()) break;
        p = q + 1;
    }

    // 转交安装前的回调（log4z），落盘/滚动文件等行为保持原样
    if (sink.m_prev) sink.m_prev(tag, logStr, level, file, line, fun, retAddr);
}
