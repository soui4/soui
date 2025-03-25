#ifndef __SASSERTFMT__H__
#define __SASSERTFMT__H__

#ifdef _DEBUG
#include <crtdbg.h>
#define SASSERT_FMTW(expr, format, ...) (void)((!!(expr)) || (1 != _CrtDbgReportW(_CRT_ASSERT, _CRT_WIDE(__FILE__), __LINE__, NULL, format, __VA_ARGS__)) || (_CrtDbgBreak(), 0))

#define SASSERT_FMTA(expr, format, ...) (void)((!!(expr)) || (1 != _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, format, __VA_ARGS__)) || (_CrtDbgBreak(), 0))

#define SASSERT_MSGW(expr, msg) (void)((!!(expr)) || (1 != _CrtDbgReportW(_CRT_ASSERT, _CRT_WIDE(__FILE__), __LINE__, NULL, msg)) || (_CrtDbgBreak(), 0))

#define SASSERT_MSGA(expr, msg) (void)((!!(expr)) || (1 != _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, msg)) || (_CrtDbgBreak(), 0))
#else
#define SASSERT_FMTW(expr, format, ...) \
    if (!(expr))                        \
        SSLOGFMTW(format, ##__VA_ARGS__);

#define SASSERT_FMTA(expr, format, ...) \
    if (!(expr))                        \
        SSLOGFMTW(format, ##__VA_ARGS__);

#define SASSERT_MSGA(expr, msg) \
    if (!(expr))                \
        SSLOGW() << msg;

#define SASSERT_MSGW(expr, msg) SASSERT_MSGA(expr, msg)
#endif

#ifdef _UNICODE
#define SASSERT_FMT SASSERT_FMTW
#define SASSERT_MSG SASSERT_MSGW
#else
#define SASSERT_FMT SASSERT_FMTA
#define SASSERT_MSG SASSERT_MSGA
#endif //_UNICODE
#endif // __SASSERTFMT__H__