﻿#include <windows.h>
#include "TaskLoop.h"
#include <helper/obj-ref-impl.hpp>
#include <algorithm>
#include <cassert>
#include <deque>
#include <limits>
#ifdef _WIN32
#pragma comment(lib,"Winmm.lib")
#endif//_WIN32
SNSBEGIN

	STaskLoop::STaskLoop() :
		m_taskListLock(),
		m_runningLock(),
		m_thread(),
		m_itemsSem(),
		m_items(),
		m_hasRunningItem(false),
		m_runningItem(NULL,0),
		m_nextTaskID(0),
		m_nHeartBeatInterval(INFINITE)
	{
	}

	STaskLoop::~STaskLoop()
	{
		stop();
		if(m_nHeartBeatInterval!=INFINITE){
			#ifdef _WIN32
			timeEndPeriod(1);
			#endif//_WIN32
		}
	}

	void STaskLoop::start(const char * pszName,Priority priority)
	{
		{
			SAutoLock autoLock(m_taskListLock);
			m_items.clear();
			if(pszName) m_strName = pszName;
		}
		_start(this, &STaskLoop::runLoopProc,  priority);
	}

	void STaskLoop::stop()
	{
		int taskNum = getTaskCount();
		m_thread.stop();
		m_itemsSem.notify();
		m_thread.waitForStop();
	}

	BOOL STaskLoop::isRunning()
	{
		return !m_thread.isStopped();
	}

	long STaskLoop::postTask(const IRunnable *runnable, BOOL waitUntilDone, int priority)
	{
		if (m_thread.isStopped())
		{
			return -1;
		}
		SAutoRefPtr<IRunnable> pCloneRunnable;
		pCloneRunnable.Attach(runnable->clone());
		if (GetCurrentThreadId() == m_thread.getThreadID() && waitUntilDone)
		{
			pCloneRunnable->run();
			return -1;
		}

		SSemaphore semaphore;
		TaskItem item(pCloneRunnable,priority);

		if (waitUntilDone)
		{
			item.semaphore = &semaphore;
		}

		m_taskListLock.Enter();

		item.taskID = m_nextTaskID;
		m_nextTaskID = (m_nextTaskID + 1) & ((std::numeric_limits<long>::max)());
		std::list<TaskItem>::reverse_iterator it= m_items.rbegin();
		while(it != m_items.rend())
		{
			if(it->nPriority>=priority)
			{
				m_items.insert(it.base(),item);
				break;
			}
			it ++;
		}
		if(it==m_items.rend())
			m_items.push_front(item);

		m_taskListLock.Leave();
		m_itemsSem.notify();

		if (waitUntilDone)
		{
			int ret = semaphore.wait();

			if (ret == RETURN_ERROR)
			{
			}
		}

		return item.taskID;
	}

	void STaskLoop::runLoopProc()
	{
		unsigned int interval=INFINITE;
		{
			SAutoLock autoLock(m_csHeartBeatTask);
			interval = m_nHeartBeatInterval;
		}
		while (true)
		{
			if (m_thread.isStopping())
			{
				break;
			}
			m_itemsSem.wait(interval);
			{
				SAutoLock autoLock(m_taskListLock);
				SAutoLock autoRunningLock(m_runningLock);
				SAutoLock autoRuningInfoLock(m_runningInfoLock);
				m_hasRunningItem = false;
				m_runingItemInfo.clear();
				m_runningItem = TaskItem(NULL,0);
				if (!m_items.empty())
				{
					m_hasRunningItem = true;
					m_runningItem = m_items.front();
					m_runingItemInfo = m_runningItem.getRunnableInfo();
					m_items.pop_front();
				}
			}


			{
				SAutoLock autoRunningLock(m_runningLock);
				if (m_hasRunningItem)
				{
					TaskItem item = m_runningItem;
					item.runnable->run();
					if (item.semaphore)
					{
						//通知一个task执行完毕
						item.semaphore->notify();
					}
				}
				{
					SAutoLock autoRuningInfoLock(m_runningInfoLock);
					m_hasRunningItem = false;
					m_runingItemInfo.clear();
					m_runningItem = TaskItem(NULL,0);
				}
			}
			{
				SAutoLock autoLock(m_csHeartBeatTask);
				if(m_heartBeatTask){
					#ifdef _WIN32
					unsigned int tsNow = timeGetTime();
					#else
					unsigned int tsNow = GetTickCount();
					#endif
					unsigned int elapse = 0;
					if(m_tsTick == -1)
					{
						elapse = m_nHeartBeatInterval;
					}else{
						if(tsNow>=m_tsTick){
							elapse = tsNow-m_tsTick;
						}else{//time round
							elapse = (INFINITE-m_tsTick) + tsNow;
						}
					}
					if(elapse >= m_nHeartBeatInterval){
						interval = m_nHeartBeatInterval;//reset interval
						m_tsTick = tsNow;
						m_heartBeatTask->run();
					}else{
						interval = m_nHeartBeatInterval - elapse;
					}
				}
			}
		}// end of while

		SAutoLock autoLock(m_taskListLock);

		size_t itemsSize = m_items.size();
		while (itemsSize > 0)
		{
			TaskItem item = m_items.front();
			m_items.pop_front();
			itemsSize--;

			if (item.semaphore)
			{
				item.semaphore->notify();
			}
		}

		m_items.clear();
	}

	BOOL STaskLoop::getName(char * pszBuf, int nBufLen)
	{
		SAutoLock autoLock(m_taskListLock);
		if (m_strName.length() >= (size_t)nBufLen)
			return false;
		strcpy_s(pszBuf, nBufLen, m_strName.c_str());
		return true;
	}

	void STaskLoop::cancelTasksForObject(void *object)
	{
		if (object == NULL)
		{
			return;
		}

		{
			SAutoLock autoLock(m_taskListLock);
			std::list<TaskItem>::iterator iter = m_items.begin();

			while (iter != m_items.end())
			{
				TaskItem &item = *iter;

				if (item.runnable->getObject() == object)
				{
					iter = m_items.erase(iter);
				}
				else
				{
					++iter;
				}
			}
		}
		{
			SAutoLock autoLock(m_runningLock);
			SAutoLock autoLockRunningInfo(m_runningInfoLock);
			if(m_hasRunningItem)
			{//make sure the running item is not belong to the to be canceled object.
				m_hasRunningItem = false;
				m_runingItemInfo.clear();
				m_runningItem = TaskItem(NULL,0);
			}
		}
		{
			SAutoLock autoLock(m_csHeartBeatTask);
			if(m_heartBeatTask && m_heartBeatTask->getObject() == object){
				m_heartBeatTask=NULL;
				m_nHeartBeatInterval = INFINITE;
				#ifdef _WIN32
				timeEndPeriod(1);
				#endif//_WIN32
			}
		}
	}

	BOOL STaskLoop::cancelTask(long taskId)
	{
		SAutoLock autoLock(m_taskListLock);
		std::list<TaskItem>::iterator itemIt = m_items.begin();

		while (itemIt != m_items.end())
		{
			if (itemIt->taskID == taskId)
			{
				itemIt = m_items.erase(itemIt);
				return true;
			}
			else
			{
				++itemIt;
			}
		}
		return false;
	}

	int STaskLoop::getTaskCount() const 
	{
		SAutoLock autoLock(m_taskListLock);

		return (int)m_items.size();
	}

	BOOL STaskLoop::getRunningTaskInfo(char *buf, int bufLen)
	{
		SAutoLock autoLock(m_runningInfoLock);
		if(!m_hasRunningItem)
			return false;
		strcat_s(buf,bufLen-1,m_runingItemInfo.c_str());
		return true;
	}

	void STaskLoop::setHeartBeatTask(THIS_ IRunnable *pTask, int intervel)
	{
		SAutoLock autoLock(m_csHeartBeatTask);
		if(pTask){
			m_nHeartBeatInterval = intervel;
			m_heartBeatTask.Attach(pTask->clone());
			#ifdef _WIN32
			timeBeginPeriod(1);
			#endif//_WIN32
			m_tsTick = -1;
			m_itemsSem.notify();//stop previous wait.
		}else{
			m_heartBeatTask = NULL;
			m_nHeartBeatInterval=INFINITE;
			#ifdef _WIN32
			timeEndPeriod(1);
			#endif//_WIN32
		}
	}


	SOUI_COM_C BOOL SOUI_COM_API TASKLOOP::SCreateInstance(IObjRef **ppTaskLoop)
	{
		*ppTaskLoop = new STaskLoop();
		return TRUE;
	}

SNSEND

EXTERN_C BOOL TaskLoop_SCreateInstance(IObjRef **ppTaskLoop)
{
	return SOUI::TASKLOOP::SCreateInstance(ppTaskLoop);
}
