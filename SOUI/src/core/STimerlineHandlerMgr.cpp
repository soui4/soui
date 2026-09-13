#include <souistd.h>
#include <core/STimerlineHandlerMgr.h>

SNSBEGIN
STimerlineHandlerMgr::STimerlineHandlerMgr(void)
{
}

STimerlineHandlerMgr::~STimerlineHandlerMgr(void)
{
}

bool STimerlineHandlerMgr::RegisterTimelineHandler(ITimelineHandler *pHandler)
{
    if (m_mapHandlers.Lookup(pHandler))
        return false;
    m_mapHandlers[pHandler] = false;
    return true;
}

bool STimerlineHandlerMgr::UnregisterTimelineHandler(ITimelineHandler *pHandler)
{
    return m_mapHandlers.RemoveKey(pHandler);
}

bool STimerlineHandlerMgr::RegisterValueAnimator(IValueAnimator *pAnimator)
{
    for (SPOSITION pos = m_lstAnimators.GetHeadPosition(); pos;)
    {
        IValueAnimator *p = m_lstAnimators.GetNext(pos);
        if (p == pAnimator)
            return false;
    }
    m_lstAnimators.AddTail(pAnimator);
    return true;
}

bool STimerlineHandlerMgr::UnregisterValueAnimator(IValueAnimator *pAnimator)
{
    for (SPOSITION pos = m_lstAnimators.GetHeadPosition(); pos;)
    {
        SPOSITION pos2 = pos;
        IValueAnimator *p = m_lstAnimators.GetNext(pos);
        if (p == pAnimator)
        {
            m_lstAnimators.RemoveAt(pos2);
            return true;
        }
    }
    return false;
}

void STimerlineHandlerMgr::OnNextFrame()
{
    ITimelineHandler **pHandlers = new ITimelineHandler *[m_mapHandlers.GetCount()];
    // collect all handlers.
    SPOSITION pos = m_mapHandlers.GetStartPosition();
    int idx = 0;
    while (pos)
    {
        ITimelineHandler *p = m_mapHandlers.GetNextKey(pos);
        pHandlers[idx++] = p;
    }

    // run handles.
    for (int i = 0; i < idx; i++)
    {
        TLMAP::CPair *p = m_mapHandlers.Lookup(pHandlers[i]);
        if (!p)
            continue;
        // make sure the handler is valid before run.
        pHandlers[i]->OnNextFrame();
    }
    delete[] pHandlers;
    SList<SAutoRefPtr<IValueAnimator>> lstAnimators;
    lstAnimators.Copy(m_lstAnimators);
    pos = lstAnimators.GetHeadPosition();
    while (pos)
    {
        IValueAnimator *p = lstAnimators.GetNext(pos);
        p->GetTimelineHandler()->OnNextFrame();
    }
}

bool STimerlineHandlerMgr::IsEmpty() const
{
    return m_mapHandlers.IsEmpty() && m_lstAnimators.IsEmpty();
}

SNSEND
