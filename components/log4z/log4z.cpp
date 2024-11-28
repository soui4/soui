/*
 * Log4z License
 * -----------
 * 
 * Log4z is licensed under the terms of the MIT license reproduced below.
 * This means that Log4z is free software and can be used for both academic
 * and commercial purposes at absolutely no cost.
 * 
 * 
 * ===============================================================================
 * 
 * Copyright (C) 2010-2015 YaweiZhang <yawei.zhang@foxmail.com>.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * ===============================================================================
 * 
 * (end of COPYRIGHT)
 */

#include "log4z.h"
#include <helper/obj-ref-impl.hpp>
#include <windows.h>

#if defined (WIN32) || defined(_WIN64)
#include <io.h>
#include <shlwapi.h>
#include <process.h>
#pragma comment(lib, "shlwapi")
#pragma warning(disable:4996)
#else
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<pthread.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <semaphore.h>
#endif


#ifdef __APPLE__
#include <dispatch/dispatch.h>
#include <libproc.h>
#endif

#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <errno.h>
#include <stdio.h>
#if defined(WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <intrin.h>
#endif
#include <vector>
#include <map>
#include <list>
#include <queue>
#include <deque>
#include <stdint.h>
using namespace SOUI;

//////////////////////////////////////////////////////////////////////////
//! -----------------default logger config, can change on this.-----------
//////////////////////////////////////////////////////////////////////////
//! default logger output file.
static const char* const LOG4Z_DEFAULT_PATH = "./log/";

static const char* const LOG4Z_DEFAULT_NAME = "soui";
//! default log filter level
static const int LOG4Z_DEFAULT_LEVEL = SOUI::LOG_LEVEL_DEBUG;
//! default logger display
static const bool LOG4Z_DEFAULT_DISPLAY = false;
//! default logger output to file
static const bool LOG4Z_DEFAULT_OUTFILE = true;
//! default logger output file limit size, unit M byte.
static const int LOG4Z_DEFAULT_LIMITSIZE = 100;
//! default logger show suffix (file name and line number) 
static const bool LOG4Z_DEFAULT_SHOWSUFFIX = true;

static const int LOG4Z_MAX_LOG_INDEX = 5;

static const char *const LOG_STRING[]=
{
    "TRACE",
    "DEBUG",
    "INFO ",
    "WARN ",
    "ERROR",
    "ALARM",
    "FATAL",
};

#if defined (WIN32) || defined(_WIN64)
const static WORD LOG_COLOR[LOG_LEVEL_FATAL + 1] = {
    0,
    0,
    FOREGROUND_BLUE | FOREGROUND_GREEN,
    FOREGROUND_GREEN | FOREGROUND_RED,
    FOREGROUND_RED,
    FOREGROUND_GREEN,
    FOREGROUND_RED | FOREGROUND_BLUE };
#else

const static char LOG_COLOR[LOG_LEVEL_FATAL + 1][50] = {
    "\e[0m",
    "\e[0m",
    "\e[34m\e[1m",//hight blue
    "\e[33m", //yellow
    "\e[31m", //red
    "\e[32m", //green
    "\e[35m" };
#endif

//////////////////////////////////////////////////////////////////////////
//! Log4zFileHandler
//////////////////////////////////////////////////////////////////////////
class Log4zFileHandler
{
public:
    Log4zFileHandler(){ _file = NULL; }
    ~Log4zFileHandler(){ close(); }
    inline bool isOpen(){ return _file != NULL; }
    inline bool open(const char *path, const char * mod)
    {
        if (_file != NULL){fclose(_file);_file = NULL;}
		wchar_t path2[1000]={0},mod2[10]={0};
		MultiByteToWideChar(CP_UTF8,0,path,-1,path2,1000);
		MultiByteToWideChar(CP_UTF8,0,mod,-1,mod2,10);
        _file = _wfopen(path2, mod2);
        return _file != NULL;
    }
    inline void close()
    {
        if (_file != NULL){fclose(_file);_file = NULL;}
    }
    inline void write(const char * data, size_t len)
    {
        if (_file && len > 0)
        {
            if (fwrite(data, 1, len, _file) != len)
            {
                close();
            }
        }
    }
    inline void flush(){ if (_file) fflush(_file); }

    inline std::string readLine()
    {
        char buf[500] = { 0 };
        if (_file && fgets(buf, 500, _file) != NULL)
        {
            return std::string(buf);
        }
        return std::string();
    }
    inline const std::string readContent();

	inline size_t size() const{
		if(!_file) return 0;
		size_t pos = ftell(_file);
		fseek(_file,0,SEEK_END);
		size_t ret = ftell(_file);
		fseek(_file,pos,SEEK_SET);
		return ret;
	}
public:
    FILE *_file;
};


//////////////////////////////////////////////////////////////////////////
//! UTILITY
//////////////////////////////////////////////////////////////////////////
static void sleepMillisecond(unsigned int ms);
static tm timeToTm(time_t t);
static bool isSameDay(time_t t1, time_t t2);

static void fixPath(std::string &path);
static void trimLogConfig(std::string &str, std::string extIgnore = std::string());
static std::pair<std::string, std::string> splitPairString(const std::string & str, const std::string & delimiter);


static bool isDirectory(std::string path);
static bool createRecursionDir(std::string path);
static unsigned int getProcessID();
static std::string getProcessName();



//////////////////////////////////////////////////////////////////////////
//! LockHelper
//////////////////////////////////////////////////////////////////////////
class LockHelper
{
public:
    LockHelper();
    virtual ~LockHelper();

public:
    void lock();
    void unLock();
private:
#if defined (WIN32) || defined(_WIN64)
    CRITICAL_SECTION _crit;
#else
    pthread_mutex_t  _crit;
#endif
};

//////////////////////////////////////////////////////////////////////////
//! AutoLock
//////////////////////////////////////////////////////////////////////////
class AutoLock
{
public:
    explicit AutoLock(LockHelper & lk):_lock(lk){_lock.lock();}
    ~AutoLock(){_lock.unLock();}
private:
    LockHelper & _lock;
};

//////////////////////////////////////////////////////////////////////////
//! SemHelper
//////////////////////////////////////////////////////////////////////////
class SemHelper
{
public:
    SemHelper();
    virtual ~SemHelper();
public:
    bool create(int initcount);
    bool wait(int timeout = 0);
    bool post();
private:
#if defined (WIN32) || defined(_WIN64)
    HANDLE _hSem;
#elif defined(__APPLE__)
    dispatch_semaphore_t _semid;
#else
    sem_t _semid;
    bool  _isCreate;
#endif

};



//////////////////////////////////////////////////////////////////////////
//! ThreadHelper
//////////////////////////////////////////////////////////////////////////
#if defined (WIN32) || defined(_WIN64)
static unsigned int WINAPI  threadProc(LPVOID lpParam);
#else
static void * threadProc(void * pParam);
#endif

class ThreadHelper
{
public:
    ThreadHelper(){_hThreadID = 0;}
    virtual ~ThreadHelper(){}
public:
    bool start();
    bool wait();
    virtual void run() = 0;
private:
    unsigned long long _hThreadID;
#ifndef WIN32
    pthread_t _phtreadID;
#endif
};

#if defined (WIN32) || defined(_WIN64)
unsigned int WINAPI  threadProc(LPVOID lpParam)
{
    ThreadHelper * p = (ThreadHelper *) lpParam;
    p->run();
    return 0;
}
#else
void * threadProc(void * pParam)
{
    ThreadHelper * p = (ThreadHelper *) pParam;
    p->run();
    return NULL;
}
#endif


//////////////////////////////////////////////////////////////////////////
//! LogData
//////////////////////////////////////////////////////////////////////////
struct LogData
{
    int    _level;    //log level
    uint64_t _time;        //create time
	DWORD _pid;
	tid_t _tid;
	std::string _content; //content
	std::string _filter;
	std::string _moduleName;
	std::string _file;
	std::string _func;
	int         _line;
};

//////////////////////////////////////////////////////////////////////////
//! LoggerInfo
//////////////////////////////////////////////////////////////////////////
struct LoggerInfo 
{
    //! attribute
    std::string _name;    // one logger one name.
    std::string _path;    //path for log file.
    int  _level;        //filter level
    BOOL _display;        //display to screen 
    BOOL _outfile;        //output to file
    unsigned int _limitsize; //limit file's size, unit Million byte.
    BOOL _enable;        //logger is enable 
    BOOL _fileLine;        //enable/disable the log's suffix.(file name:line number)

    //! runtime info
    unsigned int _curWriteLen;  //current file length
    Log4zFileHandler    _handle;        //file handle.
    
    LoggerInfo()
    {
        _enable = false; 
		_name = LOG4Z_DEFAULT_NAME;
        _path = LOG4Z_DEFAULT_PATH; 
        _level = LOG4Z_DEFAULT_LEVEL; 
        _display = LOG4Z_DEFAULT_DISPLAY; 
        _outfile = LOG4Z_DEFAULT_OUTFILE;

        _limitsize = LOG4Z_DEFAULT_LIMITSIZE;
        _fileLine = LOG4Z_DEFAULT_SHOWSUFFIX;
        _curWriteLen = 0;
    }
};

class OutoutFileBuilder : public TObjRefImpl<IOutputFileBuilder>
{
public:

	STDMETHOD_(BOOL,buildOutputFile)(THIS_ char *pszFileName,int nLen,const char * pszLogName,unsigned long pid,int curFileIndex) SCONST OVERRIDE
    {
        sprintf_s(pszFileName,nLen,"%s-%d.log",pszLogName,curFileIndex);
        return true;
    }

};

//////////////////////////////////////////////////////////////////////////
//! LogerManager
//////////////////////////////////////////////////////////////////////////
class LogerManager : public ThreadHelper, public TObjRefImpl<ILogMgr>
{
public:
    LogerManager();
    virtual ~LogerManager();
    
public:
	//! Config or overwrite configure
	//! Needs to be called before ILog4zManager::Start,, OR Do not call.
	STDMETHOD_(BOOL,config)(THIS_ const char * configPath) OVERRIDE;
	STDMETHOD_(BOOL,configFromString)(THIS_ const char * configContent) OVERRIDE;

	//! Start Log Thread. This method can only be called once by one process.
	STDMETHOD_(BOOL,start)(THIS) OVERRIDE;

	//! Default the method will be calling at process exit auto.
	//! Default no need to call and no recommended.
	STDMETHOD_(BOOL,stop)(THIS) OVERRIDE;

	//pre-check the log filter. if filter out return false. 
	STDMETHOD_(BOOL,prePushLog)(THIS_ int level) OVERRIDE;
	//! Push log, thread safe.
	STDMETHOD_(BOOL,pushLog)(THIS_ int level, const char * filter, const char * log, const char * file , int line , const char * func , const void *pRetAddr) OVERRIDE;

	//! set logger's attribute, thread safe.
	STDMETHOD_(BOOL,enableLogger)(THIS_ BOOL enable) OVERRIDE;
	STDMETHOD_(BOOL,setLoggerName)(THIS_ const char *name) OVERRIDE;
	STDMETHOD_(BOOL,setLoggerPath)(THIS_ const char * path) OVERRIDE;
	STDMETHOD_(BOOL,setLoggerLevel)(THIS_ int nLevel) OVERRIDE;
	STDMETHOD_(BOOL,setLoggerFileLine)(THIS_ BOOL enable) OVERRIDE;
	STDMETHOD_(BOOL,setLoggerDisplay)(THIS_ BOOL enable) OVERRIDE;
	STDMETHOD_(BOOL,setLoggerOutFile)(THIS_ BOOL enable) OVERRIDE;
	STDMETHOD_(BOOL,setLoggerLimitsize)(THIS_ unsigned int limitsize) OVERRIDE;

	//设置LOG输出到文件的规则
	STDMETHOD_(void,setOutputFileBuilder)(THIS_ IOutputFileBuilder *pOutputFileBuilder) OVERRIDE;

	//! Log4z status statistics, thread safe.
	STDMETHOD_(BOOL,isLoggerEnable)(CTHIS) SCONST OVERRIDE;

protected:
    void showColorText(const char *text, int level = LOG_LEVEL_DEBUG);
    
    bool openLogger(LogData * log);
    bool closeLogger();
    bool popLog(LogData *& log);
    virtual void run();

private:

    //! thread status.
    bool        _runing;

    //! hot change name or path for one logger
    LockHelper _hotLock;

    //! the process info.
    unsigned int _pid;
    std::string _proName;
    
    LoggerInfo _loggerInfo;

    //! log queue
    std::list<LogData *> _logs;
    LockHelper    _logLock;

    //show color lock
    LockHelper _scLock;
    SAutoRefPtr<IOutputFileBuilder> m_pOutputFileBuilder;
};




//////////////////////////////////////////////////////////////////////////
//! Log4zFileHandler
//////////////////////////////////////////////////////////////////////////

const std::string Log4zFileHandler::readContent()
{
    std::string content;

    if (!_file)
    {
        return content;
    }
    char buf[BUFSIZ];
    size_t ret = 0;
    do  
    {
        ret = fread(buf, sizeof(char), BUFSIZ, _file);
        content.append(buf, ret);
    }
    while (ret == BUFSIZ);

    return content;
}




//////////////////////////////////////////////////////////////////////////
//! utility
//////////////////////////////////////////////////////////////////////////


void sleepMillisecond(unsigned int ms)
{
#if defined (WIN32) || defined(_WIN64)
    ::Sleep(ms);
#else
    usleep(1000*ms);
#endif
}

struct tm timeToTm(time_t t)
{
#if defined (WIN32) || defined(_WIN64)
#if _MSC_VER < 1400 //VS2003
    return * localtime(&t);
#else //vs2005->vs2013->
    struct tm tt = { 0 };
    localtime_s(&tt, &t);
    return tt;
#endif
#else //linux
    struct tm tt = { 0 };
    localtime_r(&t, &tt);
    return tt;
#endif
}

bool isSameDay(time_t t1, time_t t2)
{
    tm tm1 = timeToTm(t1);
    tm tm2 = timeToTm(t2);
    if ( tm1.tm_year == tm2.tm_year
        && tm1.tm_yday == tm2.tm_yday)
    {
        return true;
    }
    return false;
}


void fixPath(std::string &path)
{
    if (path.empty()){return;}
    for (std::string::iterator iter = path.begin(); iter != path.end(); ++iter)
    {
        if (*iter == '\\'){*iter = '/';}
    }
    if (path.at(path.length()-1) != '/'){path.append("/");}
}

static void trimLogConfig(std::string &str, std::string extIgnore)
{
    if (str.empty()){return;}
    extIgnore += "\r\n\t ";
    int length = (int)str.length();
    int posBegin = 0;
    int posEnd = 0;

    //trim utf8 file bom
    if (str.length() >= 3 
        && (unsigned char)str[0] == 0xef
        && (unsigned char)str[1] == 0xbb
        && (unsigned char)str[2] == 0xbf)
    {
        posBegin = 3;
    }

    //trim character 
    for (int i = posBegin; i<length; i++)
    {
        bool bCheck = false;
        for (int j = 0; j < (int)extIgnore.length(); j++)
        {
            if (str[i] == extIgnore[j])
            {
                bCheck = true;
            }
        }
        if (bCheck)
        {
            if (i == posBegin)
            {
                posBegin++;
            }
        }
        else
        {
            posEnd = i + 1;
        }
    }
    if (posBegin < posEnd)
    {
        str = str.substr(posBegin, posEnd-posBegin);
    }
    else
    {
        str.clear();
    }
}

//split
static std::pair<std::string, std::string> splitPairString(const std::string & str, const std::string & delimiter)
{
    std::string::size_type pos = str.find(delimiter.c_str());
    if (pos == std::string::npos)
    {
        return std::make_pair(str, "");
    }
    return std::make_pair(str.substr(0, pos), str.substr(pos+delimiter.length()));
}

static bool parseConfigLine(const std::string& line, int curLineNum, std::string & key, LoggerInfo & outInfo)
{
    std::pair<std::string, std::string> kv = splitPairString(line, "=");
    if (kv.first.empty())
    {
        return false;
    }

    trimLogConfig(kv.first);
    trimLogConfig(kv.second);
    if (kv.first.empty() || kv.first.at(0) == '#')
    {
        return true;
    }

    trimLogConfig(kv.first);
    trimLogConfig(kv.second);

	std::transform(kv.first.begin(), kv.first.end(), kv.first.begin(), ::tolower);
    //! path
    if (kv.first == "path")
    {
        outInfo._path = kv.second;
        return true;
    }
    else if (kv.first == "name")
    {
        outInfo._name = kv.second;
        return true;
    }
    std::transform(kv.second.begin(), kv.second.end(), kv.second.begin(), ::tolower);
    //! level
    if (kv.first == "level")
    {
        if (kv.second == "trace" || kv.second == "all")
        {
            outInfo._level = LOG_LEVEL_TRACE;
        }
        else if (kv.second == "debug")
        {
            outInfo._level = LOG_LEVEL_DEBUG;
        }
        else if (kv.second == "info")
        {
            outInfo._level = LOG_LEVEL_INFO;
        }
        else if (kv.second == "warn" || kv.second == "warning")
        {
            outInfo._level = LOG_LEVEL_WARN;
        }
        else if (kv.second == "error")
        {
            outInfo._level = LOG_LEVEL_ERROR;
        }
        else if (kv.second == "alarm")
        {
            outInfo._level = LOG_LEVEL_ALARM;
        }
        else if (kv.second == "fatal")
        {
            outInfo._level = LOG_LEVEL_FATAL;
        }
    }
    //! display
    else if (kv.first == "display")
    {
        if (kv.second == "false" || kv.second == "0")
        {
            outInfo._display = false;
        }
        else
        {
            outInfo._display = true;
        }
    }
    //! output to file
    else if (kv.first == "outfile")
    {
        if (kv.second == "false" || kv.second == "0")
        {
            outInfo._outfile = false;
        }
        else
        {
            outInfo._outfile = true;
        }
    }

    //! limit file size
    else if (kv.first == "limitsize")
    {
        outInfo._limitsize = atoi(kv.second.c_str());
    }
    //! display log in file line
    else if (kv.first == "fileline")
    {
        if (kv.second == "false" || kv.second == "0")
        {
            outInfo._fileLine = false;
        }
        else
        {
            outInfo._fileLine = true;
        }
    }
    //! enable/disable one logger
    else if (kv.first == "enable")
    {
        if (kv.second == "false" || kv.second == "0")
        {
            outInfo._enable = false;
        }
        else
        {
            outInfo._enable = true;
        }
    }
    return true;
}

static bool parseConfigFromString(std::string content, LoggerInfo & outInfo)
{

    std::string key;
    int curLine = 1;
    std::string line;
    std::string::size_type curPos = 0;
    if (content.empty())
    {
        return true;
    }
    do
    {
        std::string::size_type pos = std::string::npos;
        for (std::string::size_type i = curPos; i < content.length(); ++i)
        {
            //support linux/unix/windows LRCF
            if (content[i] == '\r' || content[i] == '\n')
            {
                pos = i;
                break;
            }
        }
        line = content.substr(curPos, pos - curPos);
        parseConfigLine(line, curLine, key, outInfo);
        curLine++;

        if (pos == std::string::npos)
        {
            break;
        }
        else
        {
            curPos = pos+1;
        }
    } while (1);
    return true;
}



bool isDirectory(std::string path)
{
#if defined (WIN32) || defined(_WIN64)
	wchar_t szPath[1000]={0};
	MultiByteToWideChar(CP_UTF8,0,path.c_str(),path.length(),szPath,1000);
    return PathIsDirectoryW(szPath) ? true : false;
#else
    DIR * pdir = opendir(path.c_str());
    if (pdir == NULL)
    {
        return false;
    }
    else
    {
        closedir(pdir);
        pdir = NULL;
        return true;
    }
#endif
}



bool createRecursionDir(std::string path)
{
    if (path.length() == 0) return true;
    std::string sub;
    fixPath(path);

    std::string::size_type pos = path.find('/');
    while (pos != std::string::npos)
    {
        std::string cur = path.substr(0, pos-0);
        if (cur.length() > 0 && !isDirectory(cur))
        {
            bool ret = false;
#if defined (WIN32) || defined(_WIN64)
			wchar_t szPath[1000]={0};
			MultiByteToWideChar(CP_UTF8,0,cur.c_str(),cur.length(),szPath,1000);
            ret = CreateDirectoryW(szPath, NULL) ? true : false;
#else
            ret = (mkdir(cur.c_str(), S_IRWXU|S_IRWXG|S_IRWXO) == 0);
#endif
            if (!ret)
            {
                return false;
            }
        }
        pos = path.find('/', pos+1);
    }

    return true;
}

unsigned int getProcessID()
{
#if defined (WIN32) || defined(_WIN64)
    return GetCurrentProcessId();
#else
    return getpid();
#endif
}


std::string getProcessName()
{
    std::string name = "MainLog";
    char buf[260] = {0};
#if defined (WIN32) || defined(_WIN64)
    if (GetModuleFileNameA(NULL, buf, 259) > 0)
    {
        name = buf;
    }
    std::string::size_type pos = name.rfind("\\");
    if (pos != std::string::npos)
    {
        name = name.substr(pos+1, std::string::npos);
    }
    pos = name.rfind(".");
    if (pos != std::string::npos)
    {
        name = name.substr(0, pos-0);
    }

#elif defined(__APPLE__)

    proc_name(getpid(), buf, 260);
    name = buf;
    return name;;
#else
    sprintf(buf, "/proc/%d/cmdline", (int)getpid());
    Log4zFileHandler i;
    i.open(buf, "rb");
    if (!i.isOpen())
    {
        return name;
    }
    name = i.readLine();
    i.close();

    std::string::size_type pos = name.rfind("/");
    if (pos != std::string::npos)
    {
        name = name.substr(pos+1, std::string::npos);
    }
#endif


    return name;
}






//////////////////////////////////////////////////////////////////////////
// LockHelper
//////////////////////////////////////////////////////////////////////////
LockHelper::LockHelper()
{
#if defined (WIN32) || defined(_WIN64)
    InitializeCriticalSection(&_crit);
#else
    //_crit = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&_crit, &attr);
    pthread_mutexattr_destroy(&attr);
#endif
}
LockHelper::~LockHelper()
{
#if defined (WIN32) || defined(_WIN64)
    DeleteCriticalSection(&_crit);
#else
    pthread_mutex_destroy(&_crit);
#endif
}

void LockHelper::lock()
{
#if defined (WIN32) || defined(_WIN64)
    EnterCriticalSection(&_crit);
#else
    pthread_mutex_lock(&_crit);
#endif
}
void LockHelper::unLock()
{
#if defined (WIN32) || defined(_WIN64)
    LeaveCriticalSection(&_crit);
#else
    pthread_mutex_unlock(&_crit);
#endif
}
//////////////////////////////////////////////////////////////////////////
// SemHelper
//////////////////////////////////////////////////////////////////////////
SemHelper::SemHelper()
{
#if defined (WIN32) || defined(_WIN64)
    _hSem = NULL;
#elif defined(__APPLE__)
    _semid = NULL;
#else
    _isCreate = false;
#endif

}
SemHelper::~SemHelper()
{
#if defined (WIN32) || defined(_WIN64)
    if (_hSem != NULL)
    {
        CloseHandle(_hSem);
        _hSem = NULL;
    }
#elif defined(__APPLE__)
    if (_semid)
    {
        dispatch_release(_semid);
        _semid = NULL;
    }
#else
    if (_isCreate)
    {
        _isCreate = false;
        sem_destroy(&_semid);
    }
#endif

}

bool SemHelper::create(int initcount)
{
    if (initcount < 0)
    {
        initcount = 0;
    }
#if defined (WIN32) || defined(_WIN64)
    if (initcount > 64)
    {
        return false;
    }
    _hSem = CreateSemaphore(NULL, initcount, 64, NULL);
    if (_hSem == NULL)
    {
        return false;
    }
#elif defined(__APPLE__)
    _semid = dispatch_semaphore_create(initcount);
    if (!_semid)
    {
        return false;
    }
#else
    if (sem_init(&_semid, 0, initcount) != 0)
    {
        return false;
    }
    _isCreate = true;
#endif

    return true;
}
bool SemHelper::wait(int timeout)
{
#if defined (WIN32) || defined(_WIN64)
    if (timeout <= 0)
    {
        timeout = INFINITE;
    }
    if (WaitForSingleObject(_hSem, timeout) != WAIT_OBJECT_0)
    {
        return false;
    }
#elif defined(__APPLE__)
    if (dispatch_semaphore_wait(_semid, dispatch_time(DISPATCH_TIME_NOW, timeout*1000)) != 0)
    {
        return false;
    }
#else
    if (timeout <= 0)
    {
        return (sem_wait(&_semid) == 0);
    }
    else
    {
        struct timeval tm;
        gettimeofday(&tm, NULL);
        long long endtime = tm.tv_sec *1000 + tm.tv_usec/1000 + timeout;
        do 
        {
            sleepMillisecond(50);
            int ret = sem_trywait(&_semid);
            if (ret == 0)
            {
                return true;
            }
            struct timeval tv_cur;
            gettimeofday(&tv_cur, NULL);
            if (tv_cur.tv_sec*1000 + tv_cur.tv_usec/1000 > endtime)
            {
                return false;
            }

            if (ret == -1 && errno == EAGAIN)
            {
                continue;
            }
            else
            {
                return false;
            }
        } while (true);
        return false;
    }
#endif
    return true;
}

bool SemHelper::post()
{
#if defined (WIN32) || defined(_WIN64)
    return ReleaseSemaphore(_hSem, 1, NULL) ? true : false;
#elif defined(__APPLE__)
    return dispatch_semaphore_signal(_semid) == 0;
#else
    return (sem_post(&_semid) == 0);
#endif

}

//////////////////////////////////////////////////////////////////////////
//! ThreadHelper
//////////////////////////////////////////////////////////////////////////
bool ThreadHelper::start()
{
#if defined (WIN32) || defined(_WIN64)
    unsigned long long ret = _beginthreadex(NULL, 0, threadProc, (void *) this, 0, NULL);

    if (ret == -1 || ret == 0)
    {
        std::cout << "log4z: create log4z thread error! \r\n" <<std::endl;
        return false;
    }
    _hThreadID = ret;
#else
    int ret = pthread_create(&_phtreadID, NULL, threadProc, (void*)this);
    if (ret != 0)
    {
        std::cout <<"log4z: create log4z thread error! \r\n" << std::endl;
        return false;
    }
#endif
    return true;
}

bool ThreadHelper::wait()
{
#if defined (WIN32) || defined(_WIN64)
    if (WaitForSingleObject((HANDLE)_hThreadID, INFINITE) != WAIT_OBJECT_0)
    {
        return false;
    }
#else
    if (pthread_join(_phtreadID, NULL) != 0)
    {
        return false;
    }
#endif
    return true;
}

//////////////////////////////////////////////////////////////////////////
//! LogerManager
//////////////////////////////////////////////////////////////////////////
LogerManager::LogerManager()
{
    _runing = false;

    _pid = getProcessID();
    _proName = getProcessName();
    _loggerInfo._enable = true;
    _loggerInfo._name = "soui4";
    m_pOutputFileBuilder.Attach(new OutoutFileBuilder());
}

LogerManager::~LogerManager()
{
    stop();
}


void LogerManager::showColorText(const char *text, int level)
{
    if (level <= LOG_LEVEL_DEBUG || level > LOG_LEVEL_FATAL)
    {
        printf("%s", text);
        return;
    }
#ifndef WIN32
    printf("%s%s\e[0m", LOG_COLOR[level], text);
#else

    AutoLock l(_scLock);
    HANDLE hStd = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStd == INVALID_HANDLE_VALUE) return;
    CONSOLE_SCREEN_BUFFER_INFO oldInfo;
    if (!GetConsoleScreenBufferInfo(hStd, &oldInfo))
    {
        return;
    }
    else 
    {
        SetConsoleTextAttribute(hStd, LOG_COLOR[level]);
        printf("%s", text);
        SetConsoleTextAttribute(hStd, oldInfo.wAttributes);
    }
#endif
    return;
}

//! read configure and create with overwriting
BOOL LogerManager::config(const char* configPath)
{
    Log4zFileHandler f;
    f.open(configPath, "rb");
    if (!f.isOpen())
    {
        std::cout << " !!! !!! !!! !!!" << std::endl;
        std::cout << " !!! !!! log4z load config file error. filename=" << configPath << " !!! !!! " << std::endl;
        std::cout << " !!! !!! !!! !!!" << std::endl;
        return false;
    }
    return parseConfigFromString(f.readContent().c_str(),_loggerInfo);
}

//! read configure and create with overwriting
BOOL LogerManager::configFromString(const char* configContent)
{
    return parseConfigFromString(configContent,_loggerInfo);
}


BOOL LogerManager::start()
{
    if (_runing)
    {
        return FALSE;
    }
    return ThreadHelper::start();
}
BOOL LogerManager::stop()
{
    if (_runing == true)
    {
        _runing = FALSE;
        wait();
        return TRUE;
    }
    return FALSE;
}

BOOL LogerManager::prePushLog(int level)
{
    if (!_runing || !_loggerInfo._enable)
    {
        return false;
    }
    if (level < _loggerInfo._level)
    {
        return false;
    }
    return true;
}


BOOL LogerManager::pushLog(int level, const char * filter, const char * log, const char * file, int line, const char *func, const void * pRetAddr)
{
	if(!prePushLog(level))
		return FALSE;
    //create log data
    LogData * pLog = new LogData;
    pLog->_level = level;
    pLog->_filter = filter;
	pLog->_content = log;
	if(file != NULL) pLog->_file = file;
	pLog->_func = func;
	pLog->_line = line;

    //append precise time to log
    {
#if defined (WIN32) || defined(_WIN64)
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        unsigned long long now = ft.dwHighDateTime;
        now <<= 32;
        now |= ft.dwLowDateTime;
        now /=10;
        now -=11644473600000000ULL;
        now /=1000;
        pLog->_time = now;
#else
        struct timeval tm;
        gettimeofday(&tm, NULL);
        pLog->_time = tm.tv_sec*1000+tm.tv_usec/1000;
#endif
    }
    
#if defined (WIN32) || defined(_WIN64)
	char szPath[MAX_PATH]={0};
	char *pModuleName = szPath;
    if(pRetAddr)
    {
        HMODULE hMod = 0;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS| GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCSTR)pRetAddr,&hMod);
        GetModuleFileNameA(hMod,szPath,MAX_PATH);
        pModuleName = strrchr(szPath,'\\')+1;
    }
	pLog->_moduleName = pModuleName;
    pLog->_pid = GetCurrentProcessId();
    pLog->_tid = GetCurrentThreadId();
#endif


    
    AutoLock l(_logLock);
    _logs.push_back(pLog);
    return true;
}


BOOL LogerManager::enableLogger(BOOL enable)
{
    _loggerInfo._enable = enable;
    return TRUE;
}
BOOL LogerManager::setLoggerLevel(int level)
{
    if (level < LOG_LEVEL_TRACE || level >LOG_LEVEL_FATAL) return FALSE;
    _loggerInfo._level = level;
    return TRUE;
}
BOOL LogerManager::setLoggerDisplay(BOOL enable)
{
    _loggerInfo._display = enable;
    return TRUE;
}
BOOL LogerManager::setLoggerOutFile(BOOL enable)
{
    _loggerInfo._outfile = enable;
    return TRUE;
}

void LogerManager::setOutputFileBuilder(IOutputFileBuilder *pOutputFileBuilder)
{
	AutoLock l(_logLock);
    m_pOutputFileBuilder = pOutputFileBuilder;
}

BOOL LogerManager::setLoggerLimitsize(unsigned int limitsize)
{
    if (limitsize == 0 ) {limitsize = (unsigned int)-1;}
    _loggerInfo._limitsize = limitsize;
    return TRUE;
}
BOOL LogerManager::setLoggerFileLine( BOOL enable)
{
    _loggerInfo._fileLine = enable;
    return TRUE;
}

BOOL LogerManager::setLoggerName(const char * name)
{
	AutoLock l(_hotLock);
	_loggerInfo._name = name;
	return TRUE;
}

BOOL LogerManager::setLoggerPath(const char * path)
{
    std::string copyPath;
    if (path == NULL || strlen(path) == 0) 
    {
        copyPath = LOG4Z_DEFAULT_PATH;
    }
    else
    {
         copyPath = path;
    }
    
    {
        char ch = copyPath.at(copyPath.length() - 1);
        if (ch != '\\' && ch != '/')
        {
            copyPath.append("/");
        }
    }

    AutoLock l(_hotLock);
	_loggerInfo._path = copyPath;
    return TRUE;
}


BOOL LogerManager::isLoggerEnable() const
{
    return _loggerInfo._enable;
}

bool LogerManager::openLogger(LogData * pLog)
{
    LoggerInfo * pLogger = &_loggerInfo;
    if (!pLogger->_enable || !pLogger->_outfile || pLog->_level < pLogger->_level)
    {
        return false;
    }

    bool needChangeFile = pLogger->_curWriteLen > pLogger->_limitsize * 1024 * 1024;
    if ( needChangeFile)
    {
        if (pLogger->_handle.isOpen())
        {
            pLogger->_handle.close();
        }

		std::string name = pLogger->_name;
		char buf[MAX_PATH];
		m_pOutputFileBuilder->buildOutputFile(buf,MAX_PATH,name.c_str(),_pid,LOG4Z_MAX_LOG_INDEX);        		
		std::string path = pLogger->_path+buf;
		remove(path.c_str());
		for(int i=LOG4Z_MAX_LOG_INDEX;i>0;i--)//max to 5 log index.
		{
			m_pOutputFileBuilder->buildOutputFile(buf,MAX_PATH,name.c_str(),_pid,i-1);        		
			std::string pathOld = pLogger->_path+buf;
			m_pOutputFileBuilder->buildOutputFile(buf,MAX_PATH,name.c_str(),_pid,i);        		
			std::string pathNew = pLogger->_path+buf;
			rename(pathOld.c_str(),pathNew.c_str());
		}
    }
    if (!pLogger->_handle.isOpen())
    {
        pLogger->_curWriteLen = 0;

        std::string name;
        std::string path;
        _hotLock.lock();
        name = pLogger->_name;
        path = pLogger->_path;
        _hotLock.unLock();
		if (!isDirectory(path))
		{
			createRecursionDir(path);
		}
        char buf[MAX_PATH] = { 0 };        
        m_pOutputFileBuilder->buildOutputFile(buf,MAX_PATH,name.c_str(),_pid,0);        
        path += buf;
		pLogger->_handle.open(path.c_str(), "ab");
		if (!pLogger->_handle.isOpen())
		{
			pLogger->_outfile = false;
			return false;
		}
		pLogger->_curWriteLen = pLogger->_handle.size();
	}
    return true;
}
bool LogerManager::closeLogger()
{
    if (_loggerInfo._handle.isOpen())
    {
        _loggerInfo._handle.close();
        return true;
    }
    return false;
}
bool LogerManager::popLog(LogData *& log)
{
    AutoLock l(_logLock);
    if (_logs.empty())
    {
        return false;
    }
    log = _logs.front();
    _logs.pop_front();
    return true;
}

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

void LogerManager::run()
{
    _runing = true;
    pushLog(LOG_LEVEL_ALARM, "logger", "-----------------  log4z thread started!   ----------------------------", __FILE__, __LINE__ , __FUNCTION__,RetAddr());
	if (_loggerInfo._enable)
	{
		std::stringstream ss;
		ss  <<" name=" <<_loggerInfo._name
			<<" path=" <<_loggerInfo._path
			<<" level=" << _loggerInfo._level
			<<" display=" << _loggerInfo._display;
		pushLog(LOG_LEVEL_ALARM, "logger", ss.str().c_str(), __FILE__, __LINE__ , __FUNCTION__,RetAddr());
	}


	char *pszBuf = new char[LOG4Z_LOG_BUF_SIZE];

    LogData * pLog = NULL;
    int needFlush = 0;
    time_t lastCheckUpdate = time(NULL);
    while (true)
    {
        while(popLog(pLog))
        {
            //discard
            if (!_loggerInfo._enable || pLog->_level <_loggerInfo._level  )
            {
                delete pLog;
                pLog = NULL;
                continue;
            }

			int nContentLen = 0;
			//format log
			{
				tm tt = timeToTm(pLog->_time/1000);
				if (pLog->_file.empty() || !_loggerInfo._fileLine)
				{
#if defined (WIN32) || defined(_WIN64)

					int ret = _snprintf_s(pszBuf, LOG4Z_LOG_BUF_SIZE, _TRUNCATE, "pid=%u tid=%u %d-%02d-%02d %02d:%02d:%02d.%03d %s %s %s \"%s\"\r\n",
						pLog->_pid, pLog->_tid,
						tt.tm_year + 1900, tt.tm_mon + 1, tt.tm_mday, tt.tm_hour, tt.tm_min, tt.tm_sec, (int)(pLog->_time%1000),
						LOG_STRING[pLog->_level], pLog->_moduleName.c_str(), pLog->_filter.c_str(), pLog->_content.c_str());
					if (ret == -1)
					{
						ret = LOG4Z_LOG_BUF_SIZE - 1;
					}
					nContentLen = ret;
#else
					int ret = snprintf(pszBuf, LOG4Z_LOG_BUF_SIZE, "%d-%02d-%02d %02d:%02d:%02d.%03d %s %s %s\r\n",
						tt.tm_year + 1900, tt.tm_mon + 1, tt.tm_mday, tt.tm_hour, tt.tm_min, tt.tm_sec, (int)(pLog->_time%1000),
						LOG_STRING[pLog->_level],pLog->_filter.c_str(), pLog->_content.c_str());
					if (ret == -1)
					{
						ret = 0;
					}
					if (ret >= LOG4Z_LOG_BUF_SIZE)
					{
						ret = LOG4Z_LOG_BUF_SIZE-1;
					}

					nContentLen = ret;
#endif
				}
				else
				{
#if defined (WIN32) || defined(_WIN64)
					const char * pNameBegin = strrchr(pLog->_file.c_str(),'\\');
#else
					const char * pNameBegin = strrchr(pLog->_file.c_str(),'/');
#endif
					if(!pNameBegin) pNameBegin = pLog->_file.c_str();
					else pNameBegin ++;            

#if defined (WIN32) || defined(_WIN64)
					int ret = _snprintf_s(pszBuf, LOG4Z_LOG_BUF_SIZE, _TRUNCATE, "pid=%u tid=%u %d-%02d-%02d %02d:%02d:%02d.%03d %s %s %s \"%s\" %s (%s):%d\r\n",
						pLog->_pid, pLog->_tid,
						tt.tm_year + 1900, tt.tm_mon + 1, tt.tm_mday, tt.tm_hour, tt.tm_min, tt.tm_sec, (int)(pLog->_time%1000),
						LOG_STRING[pLog->_level], pLog->_moduleName.c_str(), pLog->_filter.c_str(), pLog->_content.c_str(), pLog->_func.c_str(), pNameBegin, pLog->_line);
					if (ret == -1)
					{
						ret = LOG4Z_LOG_BUF_SIZE - 1;
					}
					nContentLen = ret;
#else
					int ret = snprintf(pszBuf, LOG4Z_LOG_BUF_SIZE, "%d-%02d-%02d %02d:%02d:%02d.%03d %s %s %s (%s):%d %s\r\n",
						tt.tm_year + 1900, tt.tm_mon + 1, tt.tm_mday, tt.tm_hour, tt.tm_min, tt.tm_sec, (int)(pLog->_time%1000),
						LOG_STRING[pLog->_level], pLog->_filter.c_str(), pLog->_content.c_str(), pNameBegin, pLog->_line, pLog->_func.c_str());
					if (ret == -1)
					{
						ret = 0;
					}
					if (ret >= LOG4Z_LOG_BUF_SIZE)
					{
						ret = LOG4Z_LOG_BUF_SIZE-1;
					}

					nContentLen = ret;
#endif
				}

			}

            if (_loggerInfo._display)
            {
                showColorText(pszBuf, pLog->_level);
#if defined (WIN32) || defined(_WIN64)
				OutputDebugStringA(pszBuf);
#endif
            }

            if (_loggerInfo._outfile)
            {
                if (!openLogger(pLog))
                {
                    delete pLog;
                    pLog = NULL;
                    continue;
                }

                _loggerInfo._handle.write(pszBuf, nContentLen);
                _loggerInfo._curWriteLen += (unsigned int)nContentLen;
                needFlush ++;
            }

            delete pLog;
            pLog = NULL;
        }
		if (_loggerInfo._enable && needFlush > 0)
		{
			_loggerInfo._handle.flush();
			needFlush = 0;
		}
		if(!_loggerInfo._enable && _loggerInfo._handle.isOpen())
		{
			_loggerInfo._handle.close();
		}

		//! delay. 
        sleepMillisecond(100);

        //! quit
        if (!_runing && _logs.empty())
        {
            break;
        }
    }

	closeLogger();
	delete []pszBuf;
}


SNSBEGIN
	namespace LOG4Z{
		SOUI_COM_C BOOL SOUI_COM_API SCreateInstance(IObjRef **ppLogMgr)
		{
			*ppLogMgr = new LogerManager;
			return TRUE;
		}
	}

SNSEND

EXTERN_C BOOL Log4z_SCreateInstance(IObjRef **ppLogMgr)
{
	return SOUI::LOG4Z::SCreateInstance(ppLogMgr);
}

