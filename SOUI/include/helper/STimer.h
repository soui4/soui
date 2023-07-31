#ifndef __STIMER__H__
#define __STIMER__H__
#include <interface/STimer-i.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

class SOUI_EXP STimer : public TObjRefImpl<ITimer> {
  public:
    STimer(IEvtSlot *pSlot);
    ~STimer(void);

  public:
    STDMETHOD_(BOOL, StartTimer)(THIS_ int nElapse, BOOL bRepeat, LPARAM uData DEF_VAL(0)) OVERRIDE;

    STDMETHOD_(void, KillTimer)(THIS) OVERRIDE;

  private:
    UINT m_uTimerId;
    SAutoRefPtr<IEvtSlot> m_evtSlot;
};

SNSEND
#endif // __STIMER__H__