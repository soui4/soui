#ifndef COMPAT_H
#define COMPAT_H

// compat.h : C++98 / VS2008 兼容层
//
// 本工程原先用了少量 C++11 语法（auto、range-for、lambda、nullptr、
// 类内成员初始化、std::function、std::to_string 等）。
// VS2008 只支持到 C++98/03，因此这里集中提供等效的替代品：
//   - 64 位整数别名：VS2008 对 long long 的支持不完整，改用 __int64
//   - fund_snprintf：VS2008 没有 C99 的 snprintf / std::snprintf
//   - fund_swprintf：同上，宽字符版本
//   - FUND_I64_FMT：64 位整数的 printf 格式串（MSVC 用 %I64d）
//
// 注意：本文件必须保存为「UTF-8 带 BOM」，否则 MSVC 按 CP936 解析中文注释
// 会把某些字节当成续行符，造成后续代码整片语法错误。

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cwchar>

#ifdef _MSC_VER
typedef __int64 fund_int64;
typedef unsigned __int64 fund_uint64;
#define FUND_I64_FMT _T("%I64d")
#define FUND_U64_FMT _T("%I64u")
#else
typedef long long fund_int64;
typedef unsigned long long fund_uint64;
#define FUND_I64_FMT _T("%lld")
#define FUND_U64_FMT _T("%llu")
#endif

// snprintf 包装：VS2008 上转到 _vsnprintf
// （_vsnprintf 截断时返回 -1 且不保证结尾 0，这里统一补齐）
static inline int fund_snprintf(char *buf, size_t n, const char *fmt, ...)
{
    va_list ap;
    int r;
    if (buf == NULL || n == 0)
        return -1;
    buf[0] = '\0';
    va_start(ap, fmt);
#ifdef _MSC_VER
    r = _vsnprintf(buf, n, fmt, ap);
    if (r < 0 || (size_t)r >= n)
    {
        buf[n - 1] = '\0';
    }
#else
    r = vsnprintf(buf, n, fmt, ap);
#endif
    va_end(ap);
    return r;
}

// 宽字符版本
static inline int fund_swprintf(wchar_t *buf, size_t n, const wchar_t *fmt, ...)
{
    va_list ap;
    int r;
    if (buf == NULL || n == 0)
        return -1;
    buf[0] = L'\0';
    va_start(ap, fmt);
#ifdef _MSC_VER
    r = _vsnwprintf(buf, n, fmt, ap);
    if (r < 0 || (size_t)r >= n)
    {
        buf[n - 1] = L'\0';
    }
#else
    r = vswprintf(buf, n, fmt, ap);
#endif
    va_end(ap);
    return r;
}

#endif// COMPAT_H