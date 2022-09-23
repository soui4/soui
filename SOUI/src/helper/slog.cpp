#include <souistd.h>
#include <helper/slog.h>


SNSBEGIN

void Log::DefCallback(const char *tag, const char* pLogStr, int level, int loggerId,const char * file,int line,const char *fun,void *retAddr){
	SApplication *theApp = SApplication::getSingletonPtr();
	ILog4zManager *pLogMgr = theApp? theApp->GetLogManager():NULL;
	if (pLogMgr && pLogMgr->prePushLog(loggerId, level))                                    
	{                                                                                         
		pLogMgr->pushLog(loggerId, level, tag, pLogStr, file, line, fun, retAddr);                                                          
	}                                                                                         
	if(Log::s_enableEcho)                                                   
	{
		const int kMaxLog = Log::MAX_LOGLEN+100;
		char *logbuf2 = (char *)malloc(kMaxLog+1);                         
		int nLen = _snprintf_s(logbuf2, kMaxLog, _TRUNCATE, "%s,%s %s %s:%d\n", 
			tag,pLogStr, fun, file, line);                     
		logbuf2[nLen] = 0;                                                                    
		OutputDebugStringA(logbuf2);                                                          
		free(logbuf2);                                                                        
	}
}

//////////////////////////////////////////////////////////////////////////
int Log::s_logLevel = Log::LOG_INFO;
bool Log::s_enableEcho = true;
LogCallback Log::gs_logCallback = Log::DefCallback;
  
Log::Log(int nLoggerId,const char *tag, int level, const char* filename, const char* funcname, int lineIndex,void *pAddr)
:m_level(level)
,m_loggerId(nLoggerId)
,m_pAddr(pAddr)
,m_file(filename)
,m_func(funcname)
,m_line(lineIndex)
,m_stream(m_logBuf,MAX_LOGLEN)
{
	assert(strlen(tag)<MAX_TAGLEN);
	strcpy(m_tag,tag);
}

Log::~Log() {
	if(m_level < s_logLevel)
		return;

	if(gs_logCallback){
		gs_logCallback(m_tag,m_logBuf,m_level,m_loggerId,m_file,m_line,m_func,m_pAddr);
	}else if(s_enableEcho){
		const int kMaxLog = Log::MAX_LOGLEN+100;
		char *logbuf2 = (char *)malloc(kMaxLog+1);                         
		int nLen = _snprintf_s(logbuf2, kMaxLog, _TRUNCATE, "%s,%s %s %s:%d\n", 
			m_tag,m_logBuf, m_func, m_file, m_line);                     
		logbuf2[nLen] = 0;                                                                    
		OutputDebugStringA(logbuf2);                                                          
		free(logbuf2);                                                                        
	}
}

LogStream &Log::stream() {
	return m_stream;
}

void Log::setLogLevel(int nLevel) {
	s_logLevel = nLevel;
}

//setLogCallback is not thread safe.
//call this at the beginning of program please.
void Log::setLogCallback(LogCallback logCallback) {
	gs_logCallback = logCallback;
}

void Log::setLogEcho(bool bEnable)
{
	s_enableEcho = bEnable;
}

//////////////////////////////////////////////////////////////////////////
LogStream & LogStream::writeWString(const wchar_t *t)
{
#if defined(WIN32) || defined(_WIN64)
	DWORD dwLen = WideCharToMultiByte(CP_ACP, 0, t, -1, NULL, 0, NULL, NULL);
	if (dwLen < Log::MAX_LOGLEN)
	{
		char buf[ Log::MAX_LOGLEN];
		dwLen = WideCharToMultiByte(CP_ACP, 0, t, -1, buf, dwLen, NULL, NULL);
		if (dwLen > 0)
		{
			buf[dwLen] = 0;
			writeData("%s", buf);
		}
	}
#else
	// not support
#endif
	return *this;
}

LogStream & LogStream::writeString(const char *t)
{
	writeData("%s", t);
	return *this;
}

LogStream & LogStream::writeBinary(const LogBinary &t)
{
	writeData("%s", "\r\n\t[");
	for (int i = 0; i < t._len; i++)
	{
		if (i % 16 == 0)
		{
			writeData("%s", "\r\n\t");
			*this << (void *)(t._buf + i);
			writeData("%s", ": ");
		}
		writeData("%02x ", (unsigned char)t._buf[i]);
	}
	writeData("%s", "\r\n\t]\r\n\t");
	return *this;
}

LogStream::LogStream(char *buf, int len)
{
	_begin = buf;
	_end = buf + len;
	_cur = _begin;
	buf[0] = 0;
}

LogStream & LogStream::writeLongLong(long long t)
{
#if defined(WIN32) || defined(_WIN64)
	writeData("%I64d", t);
#else
	writeData("%lld", t);
#endif
	return *this;
}

LogStream & LogStream::writeULongLong(unsigned long long t)
{
#if defined(WIN32) || defined(_WIN64)
	writeData("%I64u", t);
#else
	writeData("%llu", t);
#endif
	return *this;
}

LogStream & LogStream::writePointer(const void *t)
{
#if defined(WIN32) || defined(_WIN64)
	sizeof(t) == 8 ? writeData("0x%016I64x", (unsigned long long)t)
		: writeData("0x%08I64x", (unsigned long long)t);
#else
	sizeof(t) == 8 ? writeData("0x%016llx", (unsigned long long)t)
		: writeData("0x%08llx", (unsigned long long)t);
#endif
	return *this;
}

LogStream & LogStream::writeData(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	if (_cur < _end)
	{
		int len = 0;
		int count = (int)(_end - _cur) - 1;
#if defined(WIN32) || defined(_WIN64)
		len = _vsnprintf(_cur, count, fmt, args);
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
#else
		len = vsnprintf(_cur, count, fmt, args);
		if (len < 0)
		{
			*_cur = '\0';
			len = 0;
		}
		else if (len >= count)
		{
			len = count;
			*(_end - 1) = '\0';
		}
#endif
		_cur += len;
	}
	va_end(args);

	return *this;
}

SNSEND