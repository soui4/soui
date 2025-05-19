#ifndef __STIMERLINEHANDLERMGR__H__
#define __STIMERLINEHANDLERMGR__H__

#include <interface/STimelineHandler-i.h>
#include <souicoll.h>

SNSBEGIN

/**
 * @class STimerlineHandlerMgr
 * @brief Manages a collection of timeline handlers.
 *
 * This class manages multiple timeline handlers, allowing them to be registered and unregistered.
 * It implements the ITimelineHandler interface and forwards the OnNextFrame notification to all registered handlers.
 */
class STimerlineHandlerMgr : public ITimelineHandler {
  public:
    /**
     * @brief Constructor.
     */
    STimerlineHandlerMgr(void);

    /**
     * @brief Destructor.
     */
    ~STimerlineHandlerMgr(void);

    /**
     * @brief Registers a timeline handler.
     * @param pHandler Pointer to the ITimelineHandler to register.
     * @return TRUE if registration is successful, FALSE otherwise.
     */
    bool RegisterTimelineHandler(ITimelineHandler *pHandler);

    /**
     * @brief Unregisters a timeline handler.
     * @param pHandler Pointer to the ITimelineHandler to unregister.
     * @return TRUE if unregistration is successful, FALSE otherwise.
     */
    bool UnregisterTimelineHandler(ITimelineHandler *pHandler);

    /**
     * @brief Checks if the manager is empty (no handlers registered).
     * @return TRUE if no handlers are registered, FALSE otherwise.
     */
    bool IsEmpty() const;

  public:
    /**
     * @brief Called when the next frame is ready.
     * @param THIS_ Pointer to the current instance.
     * This method forwards the OnNextFrame notification to all registered timeline handlers.
     */
    STDMETHOD_(void, OnNextFrame)(THIS_) OVERRIDE;

  protected:
    typedef SMap<ITimelineHandler *, bool> TLMAP; // Map type for storing timeline handlers.
    TLMAP m_mapHandlers;                          // Map of registered timeline handlers.
};

SNSEND

#endif // __STIMERLINEHANDLERMGR__H__