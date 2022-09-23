#pragma once
#if _MSC_VER <= 1400
#define RetAddr() NULL
#else
#include <intrin.h>
#define RetAddr() _ReturnAddress()
#endif

#include <stdio.h>
#include <interface/slog-i.h>

#ifndef E_RANGE
#define E_RANGE 9944
#endif

SNSBEGIN
//! optimze from std::stringstream to Log4zStream
#if defined(WIN32) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

class LogBinary {
  public:
    LogBinary(const char *buf, int len)
    {
        _buf = buf;
        _len = len;
    }
    const char *_buf;
    int _len;
};

class SOUI_EXP LogStream {
  public:
    LogStream(char *buf, int len);
    int getCurrentLen()
    {
        return (int)(_cur - _begin);
    }

  private:
    LogStream &writeData(const char *ft, ...);
    LogStream &writeLongLong(long long t);
    LogStream &writeULongLong(unsigned long long t);
    LogStream &writePointer(const void *t);
    LogStream &writeString(const char *t);
    LogStream &writeWString(const wchar_t *t);
    LogStream &writeBinary(const LogBinary &t);

  public:
    LogStream &operator<<(const void *t)
    {
        return writePointer(t);
    }

    LogStream &operator<<(const char *t)
    {
        return writeString(t);
    }
#if defined(WIN32) || defined(_WIN64)
    LogStream &operator<<(const wchar_t *t)
    {
        return writeWString(t);
    }
#endif
    LogStream &operator<<(bool t)
    {
        return (t ? writeData("%s", "true") : writeData("%s", "false"));
    }

    LogStream &operator<<(char t)
    {
        return writeData("%c", t);
    }

    LogStream &operator<<(unsigned char t)
    {
        return writeData("%u", (unsigned int)t);
    }

    LogStream &operator<<(short t)
    {
        return writeData("%d", (int)t);
    }

    LogStream &operator<<(unsigned short t)
    {
        return writeData("%u", (unsigned int)t);
    }

    LogStream &operator<<(int t)
    {
        return writeData("%d", t);
    }

    LogStream &operator<<(unsigned int t)
    {
        return writeData("%u", t);
    }

    LogStream &operator<<(long t)
    {
        return writeLongLong(t);
    }

    LogStream &operator<<(unsigned long t)
    {
        return writeULongLong(t);
    }

    LogStream &operator<<(long long t)
    {
        return writeLongLong(t);
    }

    LogStream &operator<<(unsigned long long t)
    {
        return writeULongLong(t);
    }

    LogStream &operator<<(float t)
    {
        return writeData("%.4f", t);
    }

    LogStream &operator<<(double t)
    {
        return writeData("%.4lf", t);
    }

    LogStream &operator<<(const LogBinary &binary)
    {
        return writeBinary(binary);
    }

  private:
    LogStream()
    {
    }
    LogStream(LogStream &)
    {
    }
    char *_begin;
    char *_end;
    char *_cur;
};

#if defined(WIN32) || defined(_WIN64)
#pragma warning(pop)
#endif


	typedef void (*LogCallback)(const char *tag, const char* pLogStr, int level, int loggerId,const char * file,int line,const char *fun,void *retAddr);

	class SOUI_EXP Log
	{
	public:
		enum{
			MAX_TAGLEN = 100,
			MAX_LOGLEN = 1024,
		};
		enum LogPriority {
			/** For internal use only.  */
			LOG_UNKNOWN = 0,
			/** The default priority, for internal use only.  */
			LOG_DEFAULT, /* only for SetMinPriority() */
			/** Verbose logging. Should typically be disabled for a release apk. */
			LOG_VERBOSE,
			/** Debug logging. Should typically be disabled for a release apk. */
			LOG_DEBUG,
			/** Informational logging. Should typically be disabled for a release apk. */
			LOG_INFO,
			/** Warning logging. For use with recoverable failures. */
			LOG_WARN,
			/** Error logging. For use with unrecoverable failures. */
			LOG_ERROR,
			/** Fatal logging. For use when aborting. */
			LOG_FATAL,
		};
	public:
		Log(int nLoggerId,const char *tag, int level,const char * filename, const char *funcname,int lineIndex,void *pAddr);

		~Log();
		LogStream & stream();

		static void setLogLevel(int nLevel);
		static void setLogCallback(LogCallback logCallback);
		static void setLogEcho(bool bEnable);
	private:
		static void DefCallback(const char *tag, const char* pLogStr, int level, int loggerId,const char * file,int line,const char *fun,void *retAddr);

		char m_tag[100];
		int  m_level;
		int  m_loggerId;
		void * m_pAddr;
		const char * m_file,* m_func;
		int m_line;
		char m_logBuf[MAX_LOGLEN+1];
		LogStream m_stream;

		static bool s_enableEcho;
		static int s_logLevel;
		static LogCallback gs_logCallback;
	};

#define SLOG(tag,level,loggerId) SOUI::Log(loggerId,tag,level,__FILE__,__FUNCTION__,__LINE__,RetAddr()).stream()
#define SLOG_FMT(tag,level, loggerId,logformat, ...)                                    \
	do                                                                                  \
	{                                                                                   \
		if(sizeof(logformat[0])==sizeof(char)) {                                        \
			char logbuf[SOUI::Log::MAX_LOGLEN]={0};                                     \
			int nLen = _snprintf(logbuf, SOUI::Log::MAX_LOGLEN,                         \
			(const char*)logformat, ##__VA_ARGS__);                                     \
			SLOG(tag,level,loggerId)<<logbuf;                                           \
		}else                                                                           \
		{                                                                               \
			wchar_t logbuf[SOUI::Log::MAX_LOGLEN]={0};                                  \
			int nLen = _snwprintf(logbuf, SOUI::Log::MAX_LOGLEN,                        \
			(const wchar_t*)logformat, ##__VA_ARGS__);                                  \
			SLOG(tag,level,loggerId)<<logbuf;                                           \
		}                                                                               \
	}while(false);


//流式输出日志，当kLogTag有效时使用，否则编译失败，kLogTag可以是当前定义的宏，也可以是当前对象的成员变量。
#define SLOGD() SLOG(kLogTag,SOUI::Log::LOG_DEBUG,0)
#define SLOGI() SLOG(kLogTag,SOUI::Log::LOG_INFO,0)
#define SLOGW() SLOG(kLogTag,SOUI::Log::LOG_WARN,0)
#define SLOGE() SLOG(kLogTag,SOUI::Log::LOG_ERROR,0)
#define SLOGF() SLOG(kLogTag,SOUI::Log::LOG_FATAL,0)

//流式输出日志，每条日志手动指定tag
#define SLOGD2(tag) SLOG(tag,SOUI::Log::LOG_DEBUG,0)
#define SLOGI2(tag) SLOG(tag,SOUI::Log::LOG_INFO,0)
#define SLOGW2(tag) SLOG(tag,SOUI::Log::LOG_WARN,0)
#define SLOGE2(tag) SLOG(tag,SOUI::Log::LOG_ERROR,0)
#define SLOGF2(tag) SLOG(tag,SOUI::Log::LOG_FATAL,0)

//格式化输出日志，当kLogTag有效时使用，否则编译失败，kLogTag可以是当前定义的宏，也可以是当前对象的成员变量。
#define SLOGFMTD(logformat, ...) SLOG_FMT(kLogTag,SOUI::Log::LOG_DEBUG,0,logformat,##__VA_ARGS__)
#define SLOGFMTI(logformat, ...) SLOG_FMT(kLogTag,SOUI::Log::LOG_INFO,0,logformat,##__VA_ARGS__)
#define SLOGFMTW(logformat, ...) SLOG_FMT(kLogTag,SOUI::Log::LOG_WARN,0,logformat,##__VA_ARGS__)
#define SLOGFMTE(logformat, ...) SLOG_FMT(kLogTag,SOUI::Log::LOG_ERROR,0,logformat,##__VA_ARGS__)
#define SLOGFMTF(logformat, ...) SLOG_FMT(kLogTag,SOUI::Log::LOG_FATAL,0,logformat,##__VA_ARGS__)

//格式化输出日志，每条日志手动指定tag
#define SLOGFMTD2(tag,logformat, ...) SLOG_FMT(tag,SOUI::Log::LOG_DEBUG,0,logformat,##__VA_ARGS__)
#define SLOGFMTI2(tag,logformat, ...) SLOG_FMT(tag,SOUI::Log::LOG_INFO,0,logformat,##__VA_ARGS__)
#define SLOGFMTW2(tag,logformat, ...) SLOG_FMT(tag,SOUI::Log::LOG_WARN,0,logformat,##__VA_ARGS__)
#define SLOGFMTE2(tag,logformat, ...) SLOG_FMT(tag,SOUI::Log::LOG_ERROR,0,logformat,##__VA_ARGS__)
#define SLOGFMTF2(tag,logformat, ...) SLOG_FMT(tag,SOUI::Log::LOG_FATAL,0,logformat,##__VA_ARGS__)

//SOUI4内部使用的日志输出，自动将TAG定义为soui4
#define kSoui4Tag "soui4"
#define SSLOGD() SLOG(kSoui4Tag,SOUI::Log::LOG_DEBUG,0)
#define SSLOGI() SLOG(kSoui4Tag,SOUI::Log::LOG_INFO,0)
#define SSLOGW() SLOG(kSoui4Tag,SOUI::Log::LOG_WARN,0)
#define SSLOGE() SLOG(kSoui4Tag,SOUI::Log::LOG_ERROR,0)
#define SSLOGF() SLOG(kSoui4Tag,SOUI::Log::LOG_FATAL,0)

#define SSLOGFMTD(logformat, ...) SLOG_FMT(kSoui4Tag,SOUI::Log::LOG_DEBUG,0,logformat,##__VA_ARGS__)
#define SSLOGFMTI(logformat, ...) SLOG_FMT(kSoui4Tag,SOUI::Log::LOG_INFO,0,logformat,##__VA_ARGS__)
#define SSLOGFMTW(logformat, ...) SLOG_FMT(kSoui4Tag,SOUI::Log::LOG_WARN,0,logformat,##__VA_ARGS__)
#define SSLOGFMTE(logformat, ...) SLOG_FMT(kSoui4Tag,SOUI::Log::LOG_ERROR,0,logformat,##__VA_ARGS__)
#define SSLOGFMTF(logformat, ...) SLOG_FMT(kSoui4Tag,SOUI::Log::LOG_FATAL,0,logformat,##__VA_ARGS__)

SNSEND