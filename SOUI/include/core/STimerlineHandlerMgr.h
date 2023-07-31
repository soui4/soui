#ifndef __STIMERLINEHANDLERMGR__H__
#define __STIMERLINEHANDLERMGR__H__

#include <interface/STimelineHandler-i.h>
#include <souicoll.h>

SNSBEGIN

class STimerlineHandlerMgr : public ITimelineHandler {
  public:
    STimerlineHandlerMgr(void);
    ~STimerlineHandlerMgr(void);

    bool RegisterTimelineHandler(ITimelineHandler *pHandler);

    bool UnregisterTimelineHandler(ITimelineHandler *pHandler);

    bool IsEmpty() const;

  public:
    STDMETHOD_(void, OnNextFrame)(THIS_) OVERRIDE;

  protected:
    typedef SMap<ITimelineHandler *, bool> TLMAP;
    TLMAP m_mapHandlers;
};

SNSEND

#endif // __STIMERLINEHANDLERMGR__H__