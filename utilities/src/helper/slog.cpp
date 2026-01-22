#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <helper/slog.h>
#include <stdio.h>
#include <stdint.h>
#ifndef E_RANGE
#define E_RANGE 9944
#endif

SNSBEGIN

//////////////////////////////////////////////////////////////////////////
int SLog::s_logLevel = LOG_LEVEL_INFO;
bool SLog::s_enableEcho = true;
LogCallback SLog::gs_logCallback = NULL;

static bool IsConsoleProgram()
{
#ifdef _WIN32
    return GetConsoleWindow() != NULL;
#else
    return true;
#endif
}

SLog::SLog(const char *tag, int level, const char *filename, const char *funcname, int lineIndex, void *pAddr)
    : m_level(level)
    , m_pAddr(pAddr)
    , m_file(filename)
    , m_func(funcname)
    , m_line(lineIndex)
    , m_stream(m_logBuf, MAX_LOGLEN)
{
    assert(strlen(tag) < MAX_TAGLEN);
    strcpy(m_tag, tag);
}

SLog::~SLog()
{
    if (m_level < s_logLevel)
        return;

    if (gs_logCallback)
    {
        gs_logCallback(m_tag, m_logBuf, m_level, m_file, m_line, m_func, m_pAddr);
    }
    if (s_enableEcho)
    {
        SYSTEMTIME wtm;
        GetLocalTime(&wtm);
        const int kMaxLog = SLog::MAX_LOGLEN + 100;
        char *logbuf2 = (char *)malloc(kMaxLog + 1);
        uint32_t tid = (uint32_t)GetCurrentThreadId();
#ifdef _WIN32
        int nLen = _snprintf(logbuf2, kMaxLog, "tid=%u,%04d-%02d-%02d %02d:%02d:%02d %03dms %s,%s %s %s:%d\n", tid, wtm.wYear, wtm.wMonth, wtm.wDay, wtm.wHour, wtm.wMinute, wtm.wSecond, wtm.wMilliseconds, m_tag, m_logBuf, m_func, m_file, m_line);
#else
        int nLen = _snprintf(logbuf2, kMaxLog, "tid=%u,%04d-%02d-%02d %02d:%02d:%02d %03dms %s,%s %s %s:%d\n", tid, wtm.wYear, wtm.wMonth, wtm.wDay, wtm.wHour, wtm.wMinute, wtm.wSecond, wtm.wMilliseconds, m_tag, m_logBuf, m_func, m_file, m_line);
#endif //_WIN32
        if (nLen > 0)
        {
            logbuf2[nLen] = 0;
            if (IsConsoleProgram())
            {
                fwrite(logbuf2, 1, nLen, stdout);
                fflush(stdout);
            }
            else
            {
                OutputDebugStringA(logbuf2);
            }
        }
        free(logbuf2);
    }
}

SLogStream &SLog::stream()
{
    return m_stream;
}

void SLog::setLogLevel(int nLevel)
{
    s_logLevel = nLevel;
}

// setLogCallback is not thread safe.
// call this at the beginning of program please.
void SLog::setLogCallback(LogCallback logCallback)
{
    gs_logCallback = logCallback;
}

void SLog::setLogEcho(bool bEnable)
{
    s_enableEcho = bEnable;
}

//////////////////////////////////////////////////////////////////////////

SLogStream::SLogStream(SLogStream &)
{
}

SLogStream::SLogStream()
{
}

SLogStream &SLogStream::writeWString(const wchar_t *t, int nLen)
{
    DWORD dwLen = WideCharToMultiByte(CP_ACP, 0, t, nLen, NULL, 0, NULL, NULL);
    if (dwLen < SLog::MAX_LOGLEN)
    {
        char buf[SLog::MAX_LOGLEN];
        dwLen = WideCharToMultiByte(CP_ACP, 0, t, nLen, buf, dwLen, NULL, NULL);
        if (dwLen > 0)
        {
            buf[dwLen] = 0;
            writeFormat("%s", buf);
        }
    }
    return *this;
}

SLogStream &SLogStream::writeString(const char *t)
{
    writeFormat("%s", t);
    return *this;
}

SLogStream &SLogStream::writeBinary(const SLogBinary &t)
{
    writeFormat("%s", "\r\n\t[");
    for (int i = 0; i < t._len; i++)
    {
        if (i % 16 == 0)
        {
            writeFormat("%s", "\r\n\t");
            *this << (void *)(t._buf + i);
            writeFormat("%s", ": ");
        }
        writeFormat("%02x ", (unsigned char)t._buf[i]);
    }
    writeFormat("%s", "\r\n\t]\r\n\t");
    return *this;
}

SLogStream::SLogStream(char *buf, int len)
{
    _begin = buf;
    _end = buf + len;
    _cur = _begin;
    buf[0] = 0;
}

SLogStream &SLogStream::writeLongLong(long long t)
{
#if defined(WIN32) || defined(_WIN64)
    writeFormat("%I64d", t);
#else
    writeFormat("%lld", t);
#endif
    return *this;
}

SLogStream &SLogStream::writeULongLong(unsigned long long t)
{
#if defined(WIN32) || defined(_WIN64)
    writeFormat("%I64u", t);
#else
    writeFormat("%llu", t);
#endif
    return *this;
}

SLogStream &SLogStream::writePointer(const void *t)
{
#if defined(WIN32) || defined(_WIN64)
    sizeof(t) == 8 ? writeFormat("0x%016I64x", (unsigned long long)t) : writeFormat("0x%08I64x", (unsigned long long)t);
#else
    sizeof(t) == 8 ? writeFormat("0x%016llx", (unsigned long long)t) : writeFormat("0x%08llx", (unsigned long long)t);
#endif
    return *this;
}

SLogStream &SLogStream::writeFormat(const wchar_t *fmt2, ...)
{
#ifndef _WIN32
    wchar_t *fmt = wcsdup(fmt2);
    wchar_t *p = wcsstr(fmt, L"%s");
    while (p)
    {
        p[1] = L'S';
        p = wcsstr(p + 2, L"%s");
    }
#else
    const wchar_t *fmt = fmt2;
#endif //_WIN32

    wchar_t logbuf[SNS::SLog::MAX_LOGLEN] = { 0 };
    va_list args;
    va_start(args, fmt2);
    int ret = vswprintf(logbuf, SNS::SLog::MAX_LOGLEN, fmt, args);
    va_end(args);
#ifndef _WIN32
    free(fmt);
#endif //_WIN32
    return writeWString(logbuf, ret);
}

SLogStream &SLogStream::writeFormat(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (_cur < _end)
    {
        int len = 0;
        int count = (int)(_end - _cur) - 1;
        len = vsnprintf(_cur, count, fmt, args);
        if (len == count || (len == -1 && errno == E_RANGE))
        {
            len = count;
            *(_end - 1) = '\0';
        }
        else if (len < 0)
        {
            *_cur = '\0';
            len = 0;
        }
        _cur += len;
    }
    va_end(args);

    return *this;
}

SLogStream &SLogStream::operator<<(const SLogBinary &binary)
{
    return writeBinary(binary);
}

SLogStream &SLogStream::operator<<(double t)
{
    return writeFormat("%.4lf", t);
}

SLogStream &SLogStream::operator<<(float t)
{
    return writeFormat("%.4f", t);
}

SLogStream &SLogStream::operator<<(unsigned long long t)
{
    return writeULongLong(t);
}

SLogStream &SLogStream::operator<<(long long t)
{
    return writeLongLong(t);
}

SLogStream &SLogStream::operator<<(unsigned long t)
{
    return writeULongLong(t);
}

SLogStream &SLogStream::operator<<(long t)
{
    return writeLongLong(t);
}

SLogStream &SLogStream::operator<<(unsigned int t)
{
    return writeFormat("%u", t);
}

SLogStream &SLogStream::operator<<(int t)
{
    return writeFormat("%d", t);
}

SLogStream &SLogStream::operator<<(unsigned short t)
{
    return writeFormat("%u", (unsigned int)t);
}

SLogStream &SLogStream::operator<<(short t)
{
    return writeFormat("%d", (int)t);
}

SLogStream &SLogStream::operator<<(unsigned char t)
{
    return writeFormat("%u", (unsigned int)t);
}

SLogStream &SLogStream::operator<<(char t)
{
    return writeFormat("%c", t);
}

SLogStream &SLogStream::operator<<(wchar_t t)
{
    return writeWString(&t, 1);
}

SLogStream &SLogStream::operator<<(bool t)
{
    return (t ? writeFormat("%s", "true") : writeFormat("%s", "false"));
}

SLogStream &SLogStream::operator<<(const wchar_t *t)
{
    return writeWString(t);
}

SLogStream &SLogStream::operator<<(const char *t)
{
    return writeString(t);
}

SLogStream &SLogStream::operator<<(const void *t)
{
    return writePointer(t);
}

SLogStream &SLogStream::operator<<(const POINT &pt)
{
    return writeFormat("{%d,%d}", pt.x, pt.y);
}

SLogStream &SLogStream::operator<<(const RECT &rc)
{
    return writeFormat("{%d,%d,%d,%d}", rc.left, rc.top, rc.right, rc.bottom);
}

SNSEND
