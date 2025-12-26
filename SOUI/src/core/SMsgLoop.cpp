#include "souistd.h"
#include "core/SMsgLoop.h"
#include "helper/slog.h"
#include "core/SNativeWnd.h"

#ifndef WM_SYSTIMER
#define WM_SYSTIMER 0x0118 //(caret blink)
#endif                     // WM_SYSTIMER

#define TM_POSTTASK -50

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

class SMessageLoopPriv {
  public:
    SMessageLoopPriv(IMessageLoop *pParentLoop = NULL)
        : m_parentLoop(pParentLoop)
    {
    }
    SArray<IMsgFilter *> m_aMsgFilter;
    SArray<IIdleHandler *> m_aIdleHandler;
    SList<IRunnable *> m_runnables;
    SList<IRunnable *> m_runningQueue;
    SAutoRefPtr<IMessageLoop> m_parentLoop;
    // Window handle for the message loop, used for handling WM_TIMER messages
    SNativeWnd m_msgWnd;
};

SMessageLoop::SMessageLoop(IMessageLoop *pParentLoop)
    : m_bRunning(FALSE)
    , m_tid(0)
    , m_bQuit(FALSE)
    , m_bDoIdle(FALSE)
    , m_nIdleCount(0)
{
    m_priv = new SMessageLoopPriv(pParentLoop);
}

SMessageLoop::~SMessageLoop()
{
    delete m_priv;
}

void SMessageLoop::OnMsg(LPMSG pMsg)
{
    if (!PreTranslateMessage(pMsg))
    {
        ::TranslateMessage(pMsg);
        ::DispatchMessage(pMsg);
    }
}

void SMessageLoop::Quit(int exitCode)
{
    if (!m_bRunning)
        return;
    PostThreadMessage(m_tid, WM_QUIT, (WPARAM)exitCode, 0);
}

int SMessageLoop::Run()
{
    m_bDoIdle = TRUE;
    m_nIdleCount = 0;
    m_tid = GetCurrentThreadId();
    m_priv->m_msgWnd.CreateNative(NULL, 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0);
    SASSERT(m_priv->m_msgWnd.IsWindow());
    m_bRunning = TRUE;
    m_bQuit = FALSE;
    int nRet = 0;
    do
    {
        BOOL bGetMsg = WaitMsg();
        if (m_bQuit)
            break;
        if (!bGetMsg)
        {
            continue;
        }
        nRet = HandleMsg();
    } while (!m_bQuit);

    {
        SAutoLock lock(m_cs);
        SPOSITION pos = m_priv->m_runnables.GetHeadPosition();
        while (pos)
        {
            IRunnable *pRunnable = m_priv->m_runnables.GetNext(pos);
            pRunnable->Release();
        }
        m_priv->m_runnables.RemoveAll();
    }
    m_bRunning = FALSE;
    m_priv->m_msgWnd.DestroyWindow();
    return nRet;
}

BOOL SMessageLoop::OnIdle(int nIdleCount)
{
    BOOL bContinue = !m_priv->m_aIdleHandler.IsEmpty();

    for (size_t i = 0; i < m_priv->m_aIdleHandler.GetCount(); i++)
    {
        IIdleHandler *pIdleHandler = m_priv->m_aIdleHandler[i];
        if (!pIdleHandler->OnIdle())
            bContinue = FALSE;
    }
    if (m_priv->m_parentLoop)
    {
        if (!m_priv->m_parentLoop->OnIdle(nIdleCount))
            bContinue = FALSE;
    }
    return bContinue;
}

BOOL SMessageLoop::PreTranslateMessage(MSG *pMsg)
{
    // loop backwards
    for (int i = (int)m_priv->m_aMsgFilter.GetCount() - 1; i >= 0; i--)
    {
        IMsgFilter *pMessageFilter = m_priv->m_aMsgFilter[i];
        if (pMessageFilter != NULL && pMessageFilter->PreTranslateMessage(pMsg))
            return TRUE;
    }
    if (m_priv->m_parentLoop)
    {
        m_priv->m_parentLoop->PreTranslateMessage(pMsg);
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
    if (!pIdleHandler)
        return FALSE;
    return RemoveElementFromArray(m_priv->m_aIdleHandler, pIdleHandler);
}

BOOL SMessageLoop::AddIdleHandler(IIdleHandler *pIdleHandler)
{
    if (!pIdleHandler)
        return FALSE;
    if (m_priv->m_aIdleHandler.Find(pIdleHandler) != -1)
        return TRUE;
    m_priv->m_aIdleHandler.Add(pIdleHandler);
    return TRUE;
}

BOOL SMessageLoop::RemoveMessageFilter(IMsgFilter *pMessageFilter)
{
    return RemoveElementFromArray(m_priv->m_aMsgFilter, pMessageFilter);
}

BOOL SMessageLoop::AddMessageFilter(IMsgFilter *pMessageFilter)
{
    if (!pMessageFilter)
        return FALSE;
    if (m_priv->m_aMsgFilter.Find(pMessageFilter) != -1)
        return TRUE;
    m_priv->m_aMsgFilter.Add(pMessageFilter);
    return TRUE;
}

static void CALLBACK OnTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    KillTimer(hwnd, idEvent);
    tid_t tid = GetCurrentThreadId();
    PostThreadMessage(tid, WM_NULL, 0, 0);
}

BOOL SMessageLoop::PostTask(IRunnable *runable)
{
    SAutoLock lock(m_cs);
    if (m_tid == 0)
    {
        SSLOGW() << "msg loop not running now! pending task size:" << m_priv->m_runnables.GetCount();
    }
    m_priv->m_runnables.AddTail(runable->clone());
    if (m_priv->m_runnables.GetCount() > 5)
    {
        PostThreadMessage(m_tid, WM_NULL, 0, 0);
    }
    else if (m_bRunning)
    {
        m_priv->m_msgWnd.SetTimer(TM_POSTTASK, 100, OnTimer); // set max waitting time to 100ms.
    }
    return TRUE;
}

int SMessageLoop::RemoveTasksForObject(void *pObj)
{
    int nRet = 0;
    SAutoLock lock(m_cs);
    SPOSITION pos = m_priv->m_runnables.GetHeadPosition();
    while (pos)
    {
        SPOSITION pos2 = pos;
        IRunnable *p = m_priv->m_runnables.GetNext(pos);
        if (p->getObject() == pObj)
        {
            p->Release();
            m_priv->m_runnables.RemoveAt(pos2);
            nRet++;
        }
    }
    SAutoLock lock2(m_csRunningQueue);
    pos = m_priv->m_runningQueue.GetHeadPosition();
    while (pos)
    {
        SPOSITION pos2 = pos;
        IRunnable *p = m_priv->m_runningQueue.GetNext(pos);
        if (p->getObject() == pObj)
        {
            p->Release();
            m_priv->m_runningQueue.RemoveAt(pos2);
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
    m_priv->m_runningQueue.Swap(m_priv->m_runnables);
    m_cs.Leave();
    for (;;)
    {
        SAutoLock lock(m_csRunningQueue);
        if (m_priv->m_runningQueue.IsEmpty())
            break;
        IRunnable *pRunnable = m_priv->m_runningQueue.GetHead();
        m_priv->m_runningQueue.RemoveHead();
        pRunnable->run();
        pRunnable->Release();
    }
    if (m_priv->m_parentLoop)
    {
        m_priv->m_parentLoop->ExecutePendingTask();
    }
    if (m_bRunning)
    {
        m_priv->m_msgWnd.KillTimer(TM_POSTTASK);
    }
}

BOOL SMessageLoop::PeekMsg(THIS_ LPMSG pMsg, UINT wMsgFilterMin, UINT wMsgFilterMax, BOOL bRemove)
{
    return ::PeekMessage(pMsg, 0, wMsgFilterMin, wMsgFilterMax, bRemove ? PM_REMOVE : PM_NOREMOVE);
}

void SMessageLoop::RunIdle()
{
    MSG msg;
    while (!m_bQuit && m_bDoIdle && !PeekMsg(&msg, 0, 0, FALSE))
    {
        m_bDoIdle = OnIdle(m_nIdleCount++);
    }
}

BOOL SMessageLoop::WaitMsg(THIS)
{
    RunIdle();
    if (m_bQuit)
        return FALSE;
    return ::WaitMessage();
}

int SMessageLoop::HandleMsg(THIS)
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

SNSEND