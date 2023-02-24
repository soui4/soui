#include "souistd.h"
#include "helper/STimerGenerator.h"

SNSBEGIN
//////////////////////////////////////////////////////////////////////////
//  SScriptTimer
template <>
STimerGenerator *SSingleton<STimerGenerator>::ms_Singleton = NULL;

STimerGenerator::~STimerGenerator()
{
    SPOSITION pos = m_mapNamedObj->GetStartPosition();
    while (pos)
    {
        SMap<UINT_PTR, TIMERINFO>::CPair *p = m_mapNamedObj->GetNext(pos);
        ::KillTimer(NULL, p->m_key);
    }
}

VOID CALLBACK STimerGenerator::_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    TIMERINFO ti;
    if (STimerGenerator::getSingleton().GetKeyObject(idEvent, ti))
    {
        if (!ti.bRepeat)
        {
            STimerGenerator::getSingleton().ClearTimer(idEvent);
        }
		EventTimer evt(NULL);
		evt.uID = (UINT)idEvent;
		evt.uData = ti.uData;
        ti.pEvtSlot->Run(&evt);
    }
    else
    {
        STimerGenerator::getSingleton().ClearTimer(idEvent);
    }
}

void STimerGenerator::ClearTimer(UINT_PTR uID)
{
    ::KillTimer(NULL, uID);
    RemoveKeyObject(uID);
}

UINT STimerGenerator::SetTimer(IEvtSlot *pEvtSlot, UINT nElapse, BOOL bRepeat,LPARAM uData)
{
    UINT_PTR uID = ::SetTimer(NULL, 0, nElapse, _TimerProc);
    if (uID != 0)
    {
		TIMERINFO ti = { pEvtSlot, bRepeat,uData };
        AddKeyObject(uID, ti);
    }
    return (UINT)uID;
}

SNSEND