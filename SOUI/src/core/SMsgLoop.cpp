#include "souistd.h"
#include "core/SMsgLoop.h"

#ifndef WM_SYSTIMER
#define WM_SYSTIMER 0x0118 //(caret blink)
#endif                     // WM_SYSTIMER

SNSBEGIN

template <class T>
BOOL RemoveElementFromArray(SArray<T> &arr, T ele)
{
    int idx = arr.Find(ele);
    if (idx == -1)
        return FALSE;
    arr.RemoveAt(idx);
    return TRUE;
}

SMessageLoop::SMessageLoop(IMessageLoop * pParentLoop)
    : m_bRunning(FALSE)
    , m_tid(0)
	, m_parentLoop(pParentLoop)
{
}

SMessageLoop::~SMessageLoop()
{
}

void SMessageLoop::OnMsg(LPMSG pMsg)
{
    if (!PreTranslateMessage(pMsg))
    {
        ::TranslateMessage(pMsg);
        ::DispatchMessage(pMsg);
    }
}

void SMessageLoop::Quit()
{
    if (!m_bRunning)
        return;
    m_bQuit = TRUE;
}

int SMessageLoop::Run()
{
    BOOL bDoIdle = TRUE;
    int nIdleCount = 0;
    BOOL bRet;

    m_bRunning = TRUE;
    m_tid = GetCurrentThreadId();
    m_bQuit = FALSE;
    for (;!m_bQuit;)
    {
        while (bDoIdle && !::PeekMessage(&m_msg, NULL, 0, 0, PM_NOREMOVE))
        {
            if (!OnIdle(nIdleCount++))
                bDoIdle = FALSE;
            if (m_bQuit)
                goto exit_loop;
        }

		bRet = OnWaitMessage();
		if(!bRet){
			SSLOGD() << "OnWaitMessage returned FALSE (error)";
			continue; // error, don't process
		}
		
        while(PeekMessage(&m_msg,NULL,0,0,PM_REMOVE))
		{
			if(m_msg.message == WM_QUIT)
			{
				m_bQuit = TRUE;
				break;
			}
			ExecutePendingTask();
			OnMsg(&m_msg);
			bDoIdle = IsIdleMessage(&m_msg);
			if(bDoIdle){
				nIdleCount = 0;
			}
		}
    }

exit_loop:
{
    SAutoLock lock(m_cs);
    SPOSITION pos = m_runnables.GetHeadPosition();
    while (pos)
    {
        IRunnable *pRunnable = m_runnables.GetNext(pos);
        pRunnable->Release();
    }
    m_runnables.RemoveAll();
}
    m_bRunning = FALSE;
    return (int)m_msg.wParam;
}

BOOL SMessageLoop::OnIdle(int nIdleCount)
{
	BOOL bContinue=FALSE;//default set to don't continue
    for (size_t i = 0; i < m_aIdleHandler.GetCount(); i++)
    {
        IIdleHandler *pIdleHandler = m_aIdleHandler[i];
        if (pIdleHandler != NULL)
            if(pIdleHandler->OnIdle())
				bContinue=TRUE;
    }
	if(m_parentLoop){
		if(m_parentLoop->OnIdle(nIdleCount))
			bContinue = TRUE;
	}
    return bContinue;
}

BOOL SMessageLoop::PreTranslateMessage(MSG *pMsg)
{
    // loop backwards
    for (int i = (int)m_aMsgFilter.GetCount() - 1; i >= 0; i--)
    {
        IMsgFilter *pMessageFilter = m_aMsgFilter[i];
        if (pMessageFilter != NULL && pMessageFilter->PreTranslateMessage(pMsg))
            return TRUE;
    }
	if(m_parentLoop){
		m_parentLoop->PreTranslateMessage(pMsg);
	}
    return FALSE; // not translated
}

BOOL SMessageLoop::IsIdleMessage(MSG *pMsg)
{
    // These messages should NOT cause idle processing
    switch (pMsg->message)
    {
    case WM_MOUSEMOVE:
    case WM_NCMOUSEMOVE:
    case WM_PAINT:
    case WM_SYSTIMER: // WM_SYSTIMER (caret blink)
        return FALSE;
    }

    return TRUE;
}

BOOL SMessageLoop::RemoveIdleHandler(IIdleHandler *pIdleHandler)
{
    return RemoveElementFromArray(m_aIdleHandler, pIdleHandler);
}

BOOL SMessageLoop::AddIdleHandler(IIdleHandler *pIdleHandler)
{
    m_aIdleHandler.Add(pIdleHandler);
    return TRUE;
}

BOOL SMessageLoop::RemoveMessageFilter(IMsgFilter *pMessageFilter)
{
    return RemoveElementFromArray(m_aMsgFilter, pMessageFilter);
}

BOOL SMessageLoop::AddMessageFilter(IMsgFilter *pMessageFilter)
{
    m_aMsgFilter.Add(pMessageFilter);
    return TRUE;
}

BOOL SMessageLoop::PostTask(IRunnable *runable)
{
    SAutoLock lock(m_cs);
    if (m_tid == 0)
        return FALSE;
    m_runnables.AddTail(runable->clone());
    if (m_runnables.GetCount() > 5)
    {
        PostThreadMessage(m_tid, WM_NULL, 0, 0);
    }
    return TRUE;
}

int SMessageLoop::RemoveTasksForObject(void *pObj)
{
    int nRet = 0;
    SAutoLock lock(m_cs);
    SPOSITION pos = m_runnables.GetHeadPosition();
    while (pos)
    {
        SPOSITION pos2 = pos;
        IRunnable *p = m_runnables.GetNext(pos);
        if (p->getObject() == pObj)
        {
            p->Release();
            m_runnables.RemoveAt(pos2);
            nRet++;
        }
    }
    SAutoLock lock2(m_csRunningQueue);
    pos = m_runningQueue.GetHeadPosition();
    while (pos)
    {
        SPOSITION pos2 = pos;
        IRunnable *p = m_runningQueue.GetNext(pos);
        if (p->getObject() == pObj)
        {
            p->Release();
            m_runningQueue.RemoveAt(pos2);
            nRet++;
        }
    }
    return nRet;
}

BOOL SMessageLoop::IsRunning(THIS) const
{
    return m_bRunning;
}

void SMessageLoop::ExecutePendingTask()
{
	m_cs.Enter();
	m_runningQueue.Swap(m_runnables);
	m_cs.Leave();
	for (;;)
	{
		SAutoLock lock(m_csRunningQueue);
		if (m_runningQueue.IsEmpty())
			break;
		IRunnable *pRunnable = m_runningQueue.GetHead();
		m_runningQueue.RemoveHead();
		pRunnable->run();
		pRunnable->Release();
	}
	if(m_parentLoop){
		m_parentLoop->ExecutePendingTask();
	}
}

BOOL SMessageLoop::OnWaitMessage()
{
	return WaitMessage();
}

SNSEND