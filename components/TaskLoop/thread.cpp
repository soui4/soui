#include <windows.h>
#include <cassert>
#include "thread.h"
#include <interface/STaskLoop-i.h>
#ifndef _WIN32
#include <thread>
#include <functional>
#else
#include <process.h>
#endif


#ifdef _MSC_VER
// we have undocumented Win32 APIs to set thread name.
const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)
static void SetThreadName(const char *threadName)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName;
	info.dwThreadID = ::GetCurrentThreadId();
	info.dwFlags = 0;
#pragma warning(push)
#pragma warning(disable: 6320 6322)
	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR *)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}

#pragma warning(pop)
}
#else
#include <pthread.h>
static void SetThreadName(const char *threadName)
{
	pthread_setname_np(pthread_self(), threadName);
}

#endif


SNSBEGIN

#ifdef _WIN32
class ThreadPrivate
{
public:
	ThreadPrivate():_hthread(0), _threadID(-1)
	{

	}

	~ThreadPrivate(){
		if(_hthread){
			CloseHandle(_hthread);
		}
	}
	void join(){
		WaitForSingleObject(_hthread,INFINITE);
	}

	void clear(){
		if ( _hthread !=0 )
		{
				::CloseHandle( _hthread); // resource must be released
				_hthread = 0;
				_threadID = -1;
		}
	}

	bool begin(unsigned int (CALLBACK *proc)(void*), void *param){
		_hthread = (HANDLE)_beginthreadex(NULL, 0, proc, param, 0, &_threadID);
		return _hthread!=0;
	}
	tid_t get_id() const{
		return _threadID;
	}

	bool set_thread_priority(int priority){
		if(_hthread==0)
			return false;
		return SetThreadPriority(_hthread, priority);
	}

	HANDLE _hthread;
	unsigned int _threadID;
};

#else
enum{
	THREAD_PRIORITY_TIME_CRITICAL=0,
	THREAD_PRIORITY_NORMAL=3,
	THREAD_PRIORITY_IDLE =5,
};
class ThreadPrivate{
public:
ThreadPrivate():tid(-1){}

tid_t tid;
std::thread m_thread;

tid_t get_id() const{
	return tid;
}
void join(){
	m_thread.join();
	clear();
}
void clear(){
	tid=-1;
}

bool set_thread_priority(int priority) {
	if(tid==-1)
		return false;
    struct sched_param param;
    int policy = 0;
    pthread_getschedparam(tid, &policy, &param);
    param.sched_priority = priority;
    return pthread_setschedparam(tid, policy, &param) != 0;
}

unsigned int threadProc(unsigned int (CALLBACK *proc)(void*), void *param){
	tid = pthread_self();
	return proc(param);
}

bool begin(unsigned int (CALLBACK *proc)(void*), void *param){
		m_thread = std::thread(std::bind(&ThreadPrivate::threadProc,this,proc,param));
		return true;
	}

};
#endif

Thread::Thread() :
_stopping(false),
_runnable(NULL),
_name(),
_private(*(new ThreadPrivate())),
_start(false)
{
}

Thread::~Thread()
{
	waitForStop();
	delete &_private;
}


bool Thread::start(IRunnable *runnable, const std::string &name, ThreadPriority priority)
{
	assert(runnable);

	{
		SAutoLock autolock (_lock);

		if (_start)
		{
			return false;
		}

		_stopping = false;
		_runnable.Attach(runnable->clone());
		_name = name;
		_start = true;
		_priorityLevel = priority;

	   if(!_private.begin(&Thread::threadProc, this))
		{
			clear();
			return false;
		}
		setPriority(priority);
	}

	_startSem.wait(100);
	return true;
}

void Thread::stop()
{
	SAutoLock autolock (_lock);

	if (!_start)
	{
		return;
	}

	_stopping = true;
}

void Thread::waitForStop()
{
	// we must release lock before the wait function, to avoid deadlock
	{
		SAutoLock autolock (_lock);
		if (!_start)
		{
			return;
		}
	}
	_private.join();
	{
		SAutoLock autolock (_lock);
		clear();
	}
}

bool Thread::isStopping()
{
	SAutoLock autolock (_lock);
	return _stopping;
}

bool Thread::isStopped()
{
	SAutoLock autolock (_lock);
	return _stopping || (!_start);
}

std::string Thread::getName()
{
	SAutoLock autolock (_lock);
	return _name;
}

void Thread::setThreadName(const std::string &name)
{
	SetThreadName(name.c_str());
}

bool Thread::setPriority(ThreadPriority priorityLevel)
{
	int priority = 0;

	switch (priorityLevel)
	{
	case High:
		priority = THREAD_PRIORITY_TIME_CRITICAL;
		break;

	case Low:
		priority = THREAD_PRIORITY_IDLE;
		break;

	default:
		priority = THREAD_PRIORITY_NORMAL;
		break;
	}
	return _private.set_thread_priority(priority);
}

void Thread::clear()
{
	SAutoLock autolock (_lock);
	_stopping = false;
	_runnable = NULL;
	_name = "";
	_start = false;
}

long Thread::getThreadID() const
    {
        return _private.get_id();
    }

unsigned int Thread::threadProc(void *args)
{
	Thread *thread = static_cast<Thread *> (args);
	{
		SAutoLock autoLock(thread->_lock);
		thread->setThreadName(thread->_name);
		thread->setPriority(thread->_priorityLevel);
	}

	thread->_startSem.notify();
	thread->_runnable->run();

	return 0;
}

SNSEND