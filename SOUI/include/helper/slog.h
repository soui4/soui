#ifndef __SLOG__H__
#define __SLOG__H__

#ifdef _MSC_VER
#if _MSC_VER <= 1400
#define RetAddr() NULL
#else
#include <intrin.h>
#define RetAddr() _ReturnAddress()
#endif
#else
#define RetAddr() __builtin_return_address(0)
#endif

#include <interface/slog-i.h>
#include <soui_exp.h>
#ifndef GETLOGMGR
#define GETLOGMGR() SNS::SApplication::getSingletonPtr() ? SNS::SApplication::getSingleton().GetLogManager() : NULL
#endif

SNSBEGIN

/**
 * @class SLogBinary
 * @brief 二进制日志数据类
 */
class SLogBinary {
  public:
    /**
     * @brief 构造函数
     * @param buf 二进制数据缓冲区
     * @param len 二进制数据长度
     */
    SLogBinary(const char *buf, int len)
        : _buf(buf)
        , _len(len)
    {
    }

    const char *_buf; ///< 二进制数据缓冲区
    int _len;         ///< 二进制数据长度
};

/**
 * @class SLogStream
 * @brief 日志流类，用于格式化日志输出
 */
class SOUI_EXP SLogStream {
  public:
    /**
     * @brief 构造函数
     * @param buf 缓冲区指针
     * @param len 缓冲区长度
     */
    SLogStream(char *buf, int len);

  public:
    /**
     * @brief 重载运算符<<，用于输出各种类型的数据
     */
    SLogStream &operator<<(const void *t);
    SLogStream &operator<<(const char *t);
    SLogStream &operator<<(const wchar_t *t);
    SLogStream &operator<<(bool t);
    SLogStream &operator<<(char t);
    SLogStream &operator<<(wchar_t t);
    SLogStream &operator<<(unsigned char t);
    SLogStream &operator<<(short t);
    SLogStream &operator<<(unsigned short t);
    SLogStream &operator<<(int t);
    SLogStream &operator<<(unsigned int t);
    SLogStream &operator<<(long t);
    SLogStream &operator<<(unsigned long t);
    SLogStream &operator<<(long long t);
    SLogStream &operator<<(unsigned long long t);
    SLogStream &operator<<(float t);
    SLogStream &operator<<(double t);
    SLogStream &operator<<(const SLogBinary &binary);

    /**
     * @brief 格式化输出日志
     * @param format 格式字符串
     * @param ... 可变参数列表
     */
    SLogStream &writeFormat(const char *format, ...);
    SLogStream &writeFormat(const wchar_t *format, ...);

  private:
    /**
     * @brief 写入长整型数据
     * @param t 长整型数据
     */
    SLogStream &writeLongLong(long long t);

    /**
     * @brief 写入无符号长整型数据
     * @param t 无符号长整型数据
     */
    SLogStream &writeULongLong(unsigned long long t);

    /**
     * @brief 写入指针数据
     * @param t 指针数据
     */
    SLogStream &writePointer(const void *t);

    /**
     * @brief 写入字符串数据
     * @param t 字符串数据
     */
    SLogStream &writeString(const char *t);

    /**
     * @brief 写入宽字符串数据
     * @param t 宽字符串数据
     * @param nLen 字符串长度（默认为-1，表示自动计算）
     */
    SLogStream &writeWString(const wchar_t *t, int nLen = -1);

    /**
     * @brief 写入二进制数据
     * @param t 二进制数据
     */
    SLogStream &writeBinary(const SLogBinary &t);

  private:
    SLogStream();
    SLogStream(SLogStream &);
    char *_begin; ///< 缓冲区起始地址
    char *_end;   ///< 缓冲区结束地址
    char *_cur;   ///< 当前写入位置
};

/**
 * @typedef LogCallback
 * @brief 日志回调函数类型
 * @param tag 日志标签
 * @param pLogStr 日志字符串
 * @param level 日志级别
 * @param file 文件名
 * @param line 行号
 * @param fun 函数名
 * @param retAddr 返回地址
 */
typedef void (*LogCallback)(const char *tag, const char *pLogStr, int level, const char *file, int line, const char *fun, void *retAddr);

/**
 * @class Log
 * @brief 日志记录类
 */
class SOUI_EXP Log {
  public:
    enum
    {
        MAX_TAGLEN = 100,  ///< 最大标签长度
        MAX_LOGLEN = 1024, ///< 最大日志长度
    };

  public:
    /**
     * @brief 构造函数
     * @param tag 日志标签
     * @param level 日志级别
     * @param filename 文件名
     * @param funcname 函数名
     * @param lineIndex 行号
     * @param pAddr 返回地址
     */
    Log(const char *tag, int level, const char *filename, const char *funcname, int lineIndex, void *pAddr);

    /**
     * @brief 析构函数
     */
    ~Log();

    /**
     * @brief 获取日志流对象
     * @return 日志流对象引用
     */
    SLogStream &stream();

    /**
     * @brief 设置日志级别
     * @param nLevel 日志级别
     */
    static void setLogLevel(int nLevel);

    /**
     * @brief 设置日志回调函数
     * @param logCallback 日志回调函数指针
     */
    static void setLogCallback(LogCallback logCallback);

    /**
     * @brief 设置日志回显功能
     * @param bEnable 是否启用回显
     */
    static void setLogEcho(bool bEnable);

  private:
    /**
     * @brief 默认日志回调函数
     * @param tag 日志标签
     * @param pLogStr 日志字符串
     * @param level 日志级别
     * @param file 文件名
     * @param line 行号
     * @param fun 函数名
     * @param retAddr 返回地址
     */
    static void DefCallback(const char *tag, const char *pLogStr, int level, const char *file, int line, const char *fun, void *retAddr);

    char m_tag[MAX_TAGLEN];        ///< 日志标签
    int m_level;                   ///< 日志级别
    void *m_pAddr;                 ///< 返回地址
    const char *m_file;            ///< 文件名
    const char *m_func;            ///< 函数名
    int m_line;                    ///< 行号
    char m_logBuf[MAX_LOGLEN + 1]; ///< 日志缓冲区
    SLogStream m_stream;           ///< 日志流对象

    static bool s_enableEcho;          ///< 是否启用日志回显
    static int s_logLevel;             ///< 当前日志级别
    static LogCallback gs_logCallback; ///< 日志回调函数指针
};

SNSEND

#ifdef _WIN32
#define FUNCNAME __FUNCTION__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif //_WIN32

/**
 * @def SLOG
 * @brief 流式输出日志宏
 * @param tag 日志标签
 * @param level 日志级别
 */
#define SLOG(tag, level) SNS::Log(tag, level, __FILE__, FUNCNAME, __LINE__, RetAddr()).stream()

/**
 * @def SLOG_FMT
 * @brief 格式化输出日志宏
 * @param tag 日志标签
 * @param level 日志级别
 * @param logformat 格式字符串
 * @param ... 可变参数列表
 */
#define SLOG_FMT(tag, level, logformat, ...) SLOG(tag, level).writeFormat(logformat, ##__VA_ARGS__)

// 流式输出日志，当kLogTag有效时使用，否则编译失败，kLogTag可以是当前定义的宏，也可以是当前对象的成员变量。
#define SLOGD() SLOG(kLogTag, SNS::LOG_LEVEL_DEBUG)
#define SLOGI() SLOG(kLogTag, SNS::LOG_LEVEL_INFO)
#define SLOGW() SLOG(kLogTag, SNS::LOG_LEVEL_WARN)
#define SLOGE() SLOG(kLogTag, SNS::LOG_LEVEL_ERROR)
#define SLOGF() SLOG(kLogTag, SNS::LOG_LEVEL_FATAL)

// 流式输出日志，每条日志手动指定tag
#define SLOGD2(tag) SLOG(tag, SNS::LOG_LEVEL_DEBUG)
#define SLOGI2(tag) SLOG(tag, SNS::LOG_LEVEL_INFO)
#define SLOGW2(tag) SLOG(tag, SNS::LOG_LEVEL_WARN)
#define SLOGE2(tag) SLOG(tag, SNS::LOG_LEVEL_ERROR)
#define SLOGF2(tag) SLOG(tag, SNS::LOG_LEVEL_FATAL)

// SOUI4内部使用的日志输出，自动将TAG定义为soui4
#define kSoui4Tag "soui4"
#define SSLOGD()  SLOG(kSoui4Tag, SNS::LOG_LEVEL_DEBUG)
#define SSLOGI()  SLOG(kSoui4Tag, SNS::LOG_LEVEL_INFO)
#define SSLOGW()  SLOG(kSoui4Tag, SNS::LOG_LEVEL_WARN)
#define SSLOGE()  SLOG(kSoui4Tag, SNS::LOG_LEVEL_ERROR)
#define SSLOGF()  SLOG(kSoui4Tag, SNS::LOG_LEVEL_FATAL)

// 格式化输出日志，当kLogTag有效时使用，否则编译失败，kLogTag可以是当前定义的宏，也可以是当前对象的成员变量。
#define SLOGFMTD(logformat, ...) SLOG_FMT(kLogTag, SNS::LOG_LEVEL_DEBUG, logformat, ##__VA_ARGS__)
#define SLOGFMTI(logformat, ...) SLOG_FMT(kLogTag, SNS::LOG_LEVEL_INFO, logformat, ##__VA_ARGS__)
#define SLOGFMTW(logformat, ...) SLOG_FMT(kLogTag, SNS::LOG_LEVEL_WARN, logformat, ##__VA_ARGS__)
#define SLOGFMTE(logformat, ...) SLOG_FMT(kLogTag, SNS::LOG_LEVEL_ERROR, logformat, ##__VA_ARGS__)
#define SLOGFMTF(logformat, ...) SLOG_FMT(kLogTag, SNS::LOG_LEVEL_FATAL, logformat, ##__VA_ARGS__)

// 格式化输出日志，每条日志手动指定tag
#define SLOGFMTD2(tag, logformat, ...) SLOG_FMT(tag, SNS::LOG_LEVEL_DEBUG, logformat, ##__VA_ARGS__)
#define SLOGFMTI2(tag, logformat, ...) SLOG_FMT(tag, SNS::LOG_LEVEL_INFO, logformat, ##__VA_ARGS__)
#define SLOGFMTW2(tag, logformat, ...) SLOG_FMT(tag, SNS::LOG_LEVEL_WARN, logformat, ##__VA_ARGS__)
#define SLOGFMTE2(tag, logformat, ...) SLOG_FMT(tag, SNS::LOG_LEVEL_ERROR, logformat, ##__VA_ARGS__)
#define SLOGFMTF2(tag, logformat, ...) SLOG_FMT(tag, SNS::LOG_LEVEL_FATAL, logformat, ##__VA_ARGS__)

#define SSLOGFMTD(logformat, ...) SLOG_FMT(kSoui4Tag, SNS::LOG_LEVEL_DEBUG, logformat, ##__VA_ARGS__)
#define SSLOGFMTI(logformat, ...) SLOG_FMT(kSoui4Tag, SNS::LOG_LEVEL_INFO, logformat, ##__VA_ARGS__)
#define SSLOGFMTW(logformat, ...) SLOG_FMT(kSoui4Tag, SNS::LOG_LEVEL_WARN, logformat, ##__VA_ARGS__)
#define SSLOGFMTE(logformat, ...) SLOG_FMT(kSoui4Tag, SNS::LOG_LEVEL_ERROR, logformat, ##__VA_ARGS__)
#define SSLOGFMTF(logformat, ...) SLOG_FMT(kSoui4Tag, SNS::LOG_LEVEL_FATAL, logformat, ##__VA_ARGS__)

#endif // __SLOG__H__