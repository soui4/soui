#pragma once

#include "core/SSingletonMap.h"

SNSBEGIN

struct TIMERINFO
{
	SAutoRefPtr<IEvtSlot> pEvtSlot;
    BOOL bRepeat;
};

class STimerGenerator : public SSingletonMap<STimerGenerator, TIMERINFO, UINT_PTR> {
    SINGLETON2_TYPE(SINGLETON_TIMERGENERATOR)
  public:
    ~STimerGenerator();

    UINT SetTimer(IEvtSlot *pEvtSlot, UINT nElapse, BOOL bRepeat);

    void ClearTimer(UINT_PTR uID);

    static VOID CALLBACK _TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
};

SNSEND