#include <souistd.h>
#include <list>
#include <vector>
#include <helper/SCriticalSection.h>
#include <core/SMsgLoop.h>

#ifndef WM_SYSTIMER
#define WM_SYSTIMER 0x0118 //(caret blink)
#endif                     // WM_SYSTIMER

SNSBEGIN

template <class T>
BOOL RemoveElementFromArray(std::vector<T> &arr, T ele)
{
    typename std::vector<T>::iterator it = arr.begin();
    while (it != arr.end())
    {
        if (*it == ele)
        {
            arr.erase(it);
            return TRUE;
        }
        it++;
    }
    return FALSE;
}

class SMessageLoopPriv {
  public:
    SMessageLoopPriv(IMessageLoop *pParentLoop);
    virtual ~SMessageLoopPriv();

  public:
    // Message filter operations
    STDMETHOD_(BOOL, AddMessageFilter)(THIS_ IMsgFilter *pMessageFilter);

    STDMETHOD_(BOOL, RemoveMessageFilter)(THIS_ IMsgFilter *pMessageFilter);

    // Idle handler operations
    STDMETHOD_(BOOL, AddIdleHandler)(THIS_ IIdleHandler *pIdleHandler);

    STDMETHOD_(BOOL, RemoveIdleHandler)(THIS_ IIdleHandler *pIdleHandler);

    //  to change message filtering
    STDMETHOD_(BOOL, PreTranslateMessage)(THIS_ MSG *pMsg);

    //  to change idle processing
    STDMETHOD_(BOOL, OnIdle)(THIS_ int /*nIdleCount*/);

    STDMETHOD_(int, Run)(THIS);

    STDMETHOD_(BOOL, IsRunning)(THIS) const;

    STDMETHOD_(void, OnMsg)(THIS_ LPMSG pMsg);

    STDMETHOD_(void, Quit)(THIS_ int exitCode);

    STDMETHOD_(BOOL, PostTask)(THIS_ IRunnable *runable);

    STDMETHOD_(int, RemoveTasksForObject)(THIS_ void *pObj);

    STDMETHOD_(void, ExecutePendingTask)();

    STDMETHOD_(BOOL, PeekMsg)(THIS_ LPMSG pMsg, UINT wMsgFilterMin, UINT wMsgFilterMax, BOOL bRemove);

    STDMETHOD_(BOOL, WaitMsg)(THIS);

    STDMETHOD_(int, HandleMsg)(THIS);

  public:
    static BOOL IsIdleMessage(MSG *pMsg);

  protected:
    typedef std::vector<IMsgFilter *> MsgFilterVector;
    MsgFilterVector m_aMsgFilter;
    typedef std::vector<IIdleHandler *> IdleHandlerVector;
    IdleHandlerVector m_aIdleHandler;

    BOOL m_bRunning;
    BOOL m_bQuit;
    BOOL m_bDoIdle;
    int m_nIdleCount;

    SCriticalSection m_cs;
    typedef std::list<IRunnable *> RunnableList;
    RunnableList m_runnables;
    SCriticalSection m_csRunningQueue;
    RunnableList m_runningQueue;
    SAutoRefPtr<IMessageLoop> m_parentLoop;
    tid_t m_tid;
};

SMessageLoopPriv::SMessageLoopPriv(IMessageLoop *pParentLoop)
    : m_bRunning(FALSE)
    , m_tid(0)
    , m_parentLoop(pParentLoop)
    , m_bQuit(FALSE)
    , m_bDoIdle(FALSE)
    , m_nIdleCount(0)
{
}

SMessageLoopPriv::~SMessageLoopPriv()
{
}

void SMessageLoopPriv::OnMsg(LPMSG pMsg)
{
    if (!PreTranslateMessage(pMsg))
    {
        ::TranslateMessage(pMsg);
        ::DispatchMessage(pMsg);
    }
}

void SMessageLoopPriv::Quit(int exitCode)
{
    if (!m_bRunning)
        return;
    PostThreadMessage(m_tid, WM_QUIT, (WPARAM)exitCode, 0);
}

int SMessageLoopPriv::Run()
{
    m_bRunning = TRUE;
    m_bDoIdle = TRUE;
    m_nIdleCount = 0;
    m_tid = GetCurrentThreadId();
    m_bQuit = FALSE;
    int nRet = 0;
    do
    {
        BOOL bGetMsg = WaitMsg();
        if (m_bQuit)
            break;
        if (!bGetMsg)
        {
            continue; // error, don't process
        }
        nRet = HandleMsg();
    } while (!m_bQuit);

    {
        SAutoLock lock(m_cs);
        RunnableList::iterator it = m_runnables.begin();
        while (it != m_runnables.end())
        {
            IRunnable *pRunnable = *it;
            pRunnable->Release();
            it++;
        }
        m_runnables.clear();
    }
    m_bRunning = FALSE;
    return nRet;
}

BOOL SMessageLoopPriv::OnIdle(int nIdleCount)
{
    BOOL bContinue = !m_aIdleHandler.empty();

    for (size_t i = 0; i < m_aIdleHandler.size(); i++)
    {
        IIdleHandler *pIdleHandler = m_aIdleHandler[i];
        if (!pIdleHandler->OnIdle())
            bContinue = FALSE;
    }
    if (m_parentLoop)
    {
        if (!m_parentLoop->OnIdle(nIdleCount))
            bContinue = FALSE;
    }
    return bContinue;
}

BOOL SMessageLoopPriv::PreTranslateMessage(MSG *pMsg)
{
    // loop backwards
    for (int i = (int)m_aMsgFilter.size() - 1; i >= 0; i--)
    {
        IMsgFilter *pMessageFilter = m_aMsgFilter[i];
        if (pMessageFilter != NULL && pMessageFilter->PreTranslateMessage(pMsg))
            return TRUE;
    }
    if (m_parentLoop)
    {
        m_parentLoop->PreTranslateMessage(pMsg);
    }
    return FALSE; // not translated
}

BOOL SMessageLoopPriv::IsIdleMessage(MSG *pMsg)
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

BOOL SMessageLoopPriv::RemoveIdleHandler(IIdleHandler *pIdleHandler)
{
    if (!pIdleHandler)
        return FALSE;
    return RemoveElementFromArray(m_aIdleHandler, pIdleHandler);
}

BOOL SMessageLoopPriv::AddIdleHandler(IIdleHandler *pIdleHandler)
{
    if (!pIdleHandler)
        return FALSE;
    m_aIdleHandler.push_back(pIdleHandler);
    return TRUE;
}

BOOL SMessageLoopPriv::RemoveMessageFilter(IMsgFilter *pMessageFilter)
{
    return RemoveElementFromArray(m_aMsgFilter, pMessageFilter);
}

BOOL SMessageLoopPriv::AddMessageFilter(IMsgFilter *pMessageFilter)
{
    m_aMsgFilter.push_back(pMessageFilter);
    return TRUE;
}

BOOL SMessageLoopPriv::PostTask(IRunnable *runable)
{
    SAutoLock lock(m_cs);
    if (m_tid == 0)
    {
        // SSLOGW() << "msg loop not running now! pending task size:" << m_runnables.GetCount();
    }
    m_runnables.push_back(runable->clone());
    if (m_runnables.size() > 5)
    {
        PostThreadMessage(m_tid, WM_NULL, 0, 0);
    }
    return TRUE;
}

int SMessageLoopPriv::RemoveTasksForObject(void *pObj)
{
    int nRet = 0;
    SAutoLock lock(m_cs);
    RunnableList::iterator it = m_runnables.begin();
    while (it != m_runnables.end())
    {
        RunnableList::iterator it2 = it++;
        IRunnable *p = *it;
        if (p->getObject() == pObj)
        {
            p->Release();
            m_runnables.erase(it2);
            nRet++;
        }
    }
    SAutoLock lock2(m_csRunningQueue);
    it = m_runningQueue.begin();
    while (it != m_runningQueue.end())
    {
        RunnableList::iterator it2 = it++;
        IRunnable *p = *it;
        if (p->getObject() == pObj)
        {
            p->Release();
            m_runningQueue.erase(it2);
            nRet++;
        }
    }
    return nRet;
}

BOOL SMessageLoopPriv::IsRunning(THIS) const
{
    return m_bRunning;
}

void SMessageLoopPriv::ExecutePendingTask()
{
    m_cs.Enter();
    m_runningQueue.swap(m_runnables);
    m_cs.Leave();
    for (;;)
    {
        SAutoLock lock(m_csRunningQueue);
        if (m_runningQueue.empty())
            break;
        IRunnable *pRunnable = m_runningQueue.front();
        m_runningQueue.pop_front();
        pRunnable->run();
        pRunnable->Release();
    }
    if (m_parentLoop)
    {
        m_parentLoop->ExecutePendingTask();
    }
}

BOOL SMessageLoopPriv::PeekMsg(THIS_ LPMSG pMsg, UINT wMsgFilterMin, UINT wMsgFilterMax, BOOL bRemove)
{
    return ::PeekMessage(pMsg, 0, wMsgFilterMin, wMsgFilterMax, bRemove ? PM_REMOVE : PM_NOREMOVE);
}

BOOL SMessageLoopPriv::WaitMsg(THIS)
{
    MSG msg;
    while (!m_bQuit && m_bDoIdle && !PeekMsg(&msg, 0, 0, FALSE))
    {
        m_bDoIdle = OnIdle(m_nIdleCount++);
    }
    if (m_bQuit)
        return FALSE;
    return ::WaitMessage();
}

int SMessageLoopPriv::HandleMsg(THIS)
{
    MSG msg = { 0 };
    while (PeekMsg(&msg, 0, 0, TRUE) && !m_bQuit)
    {
        if (msg.message == WM_QUIT)
        {
            m_bQuit = TRUE;
            break;
        }
        ExecutePendingTask();
        OnMsg(&msg);
        m_bDoIdle = IsIdleMessage(&msg);
        if (m_bDoIdle)
        {
            m_nIdleCount = 0;
        }
    }
    return (int)msg.wParam;
}

//////////////////////////////////////////////////////////////////////////
SMessageLoop::SMessageLoop(IMessageLoop *pParentLoop)
{
    m_priv = new SMessageLoopPriv(pParentLoop);
}

SMessageLoop::~SMessageLoop()
{
    delete m_priv;
}

BOOL SMessageLoop::AddMessageFilter(THIS_ IMsgFilter *pMessageFilter)
{
    return m_priv->AddMessageFilter(pMessageFilter);
}

BOOL SMessageLoop::RemoveMessageFilter(THIS_ IMsgFilter *pMessageFilter)
{
    return m_priv->RemoveMessageFilter(pMessageFilter);
}

BOOL SMessageLoop::AddIdleHandler(THIS_ IIdleHandler *pIdleHandler)
{
    return m_priv->AddIdleHandler(pIdleHandler);
}

BOOL SMessageLoop::RemoveIdleHandler(THIS_ IIdleHandler *pIdleHandler)
{
    return m_priv->RemoveIdleHandler(pIdleHandler);
}

BOOL SMessageLoop::PreTranslateMessage(THIS_ MSG *pMsg)
{
    return m_priv->PreTranslateMessage(pMsg);
}

BOOL SMessageLoop::OnIdle(THIS_ int nIdleCount)
{
    return m_priv->OnIdle(nIdleCount);
}

int SMessageLoop::Run(THIS)
{
    return m_priv->Run();
}

BOOL SMessageLoop::IsRunning(THIS) const
{
    return m_priv->IsRunning();
}

void SMessageLoop::OnMsg(THIS_ LPMSG pMsg)
{
    m_priv->OnMsg(pMsg);
}

void SMessageLoop::Quit(THIS_ int exitCode)
{
    m_priv->Quit(exitCode);
}

BOOL SMessageLoop::PostTask(THIS_ IRunnable *runable)
{
    return m_priv->PostTask(runable);
}

int SMessageLoop::RemoveTasksForObject(THIS_ void *pObj)
{
    return m_priv->RemoveTasksForObject(pObj);
}

void SMessageLoop::ExecutePendingTask()
{
    m_priv->ExecutePendingTask();
}

BOOL SMessageLoop::PeekMsg(THIS_ LPMSG pMsg, UINT wMsgFilterMin, UINT wMsgFilterMax, BOOL bRemove)
{
    return m_priv->PeekMsg(pMsg, wMsgFilterMin, wMsgFilterMax, bRemove);
}

BOOL SMessageLoop::WaitMsg(THIS)
{
    return m_priv->WaitMsg();
}

int SMessageLoop::HandleMsg(THIS)
{
    return m_priv->HandleMsg();
}

SNSEND