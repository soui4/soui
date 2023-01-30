#pragma once
#include <interface/STimer-i.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

class SOUI_EXP STimer : public TObjRefImpl<ITimer>
{
public:
	STimer(IEvtSlot *pSlot);
	~STimer(void);

public:
	STDMETHOD_(BOOL,StartTimer)(THIS_ int nElapse,BOOL bRepeat) OVERRIDE;

	STDMETHOD_(void,KillTimer)(THIS) OVERRIDE;

private:
	UINT  m_uTimerId;
	SAutoRefPtr<IEvtSlot> m_evtSlot;
};

SNSEND