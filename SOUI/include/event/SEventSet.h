#ifndef __SEVENTSET__H__
#define __SEVENTSET__H__

#include "SEventSlot.h"

SNSBEGIN

/**
 * @class SEvent
 * @brief Represents an event object
 */
class SOUI_EXP SEvent {
  public:
    /**
     * @brief Constructor
     * @param dwEventID Event ID
     * @param pszEventName Event name
     */
    SEvent(DWORD dwEventID, LPCWSTR pszEventName);

    /**
     * @brief Destructor
     */
    virtual ~SEvent();

    /**
     * @brief Get event ID
     * @return Event ID
     */
    DWORD GetID();

    /**
     * @brief Get event name
     * @return Event name
     */
    SStringW GetName() const;

    /**
     * @brief Get script handler
     * @return Script handler string
     */
    SStringA GetScriptHandler() const;

    /**
     * @brief Set script handler
     * @param strScriptHandler Script handler string
     */
    void SetScriptHandler(const SStringA &strScriptHandler);

    /**
     * @brief Subscribe event
     * @param slot Event slot object
     * @return TRUE on success, FALSE on failure
     */
    BOOL subscribe(const IEvtSlot *slot);

    /**
     * @brief Unsubscribe event
     * @param slot Event slot object
     * @return TRUE on success, FALSE on failure
     */
    BOOL unsubscribe(const IEvtSlot *slot);

    /**
     * @brief Fire event
     * @param args Event argument object
     */
    void fire(IEvtArgs *args);

  protected:
    /**
     * @brief Find event slot object
     * @param slot Event slot object
     * @return Index of the found event slot, or -1 if not found
     */
    int findSlotFunctor(const IEvtSlot *slot);

    DWORD m_dwEventID;           /**<  Event ID */
    SStringW m_strEventName;     /**<  Event name */
    SStringA m_strScriptHandler; /**<  Script handler string */

    SArray<IEvtSlot *> m_evtSlots; /**<  Event slot array */
};

/**
 * @class SEventSet
 * @brief Represents a set of event objects
 */
class SOUI_EXP SEventSet {
    friend class SWindow;

  public:
    /**
     * @brief Constructor
     */
    SEventSet(void);

    /**
     * @brief Destructor
     */
    virtual ~SEventSet(void);

    /**
     * @brief Add a new event to the event set
     * @param dwEventID Event ID
     * @param pszEventHandlerName Event handler name
     * @return TRUE on success, FALSE on failure
     */
    BOOL addEvent(DWORD dwEventID, LPCWSTR pszEventHandlerName);

    /**
     * @brief Remove the event with the specified ID
     * @param dwEventID Event ID
     * @return TRUE on success, FALSE on failure
     */
    BOOL removeEvent(DWORD dwEventID);

    /**
     * @brief Remove all event objects
     */
    void removeAllEvents(void);

    /**
     * @brief Check whether the event set contains the event with the specified ID
     * @param dwEventID Event ID
     * @return TRUE if exists, FALSE otherwise
     */
    BOOL isEventPresent(DWORD dwEventID);

    /**
     * @brief Set the script handler for the event
     * @param strEventName Event name
     * @param strScriptHandler Script handler string
     * @return TRUE on success, FALSE on failure
     */
    BOOL setEventScriptHandler(const SStringW &strEventName, const SStringA strScriptHandler);

    /**
     * @brief Get the script handler of the event
     * @param strEventName event name
     * @return script handler string
     */
    SStringA getEventScriptHandler(const SStringW &strEventName) const;

    /**
     * @brief Subscribe to event
     * @param dwEventID event ID
     * @param subscriber event slot object
     * @return Returns TRUE on success, FALSE on failure
     */
    BOOL subscribeEvent(DWORD dwEventID, const IEvtSlot &subscriber)
    {
        return subscribeEvent(dwEventID, &subscriber);
    }

    /**
     * @brief Subscribe to event
     * @param dwEventID event ID
     * @param subscriber event slot object pointer
     * @return Returns TRUE on success, FALSE on failure
     */
    BOOL subscribeEvent(DWORD dwEventID, const IEvtSlot *subscriber);

#if __cplusplus >= 201103L
    /**
     * @brief Subscribe to event (using standard function callback)
     * @param dwEventID event ID
     * @param eventCallback standard function callback
     * @return Returns TRUE on success, FALSE on failure
     */
    BOOL subscribeEvent(DWORD dwEventID, const StdFunCallback &eventCallback);
#endif

    /**
     * @brief Subscribe to event (template function)
     * @tparam T object type
     * @tparam A event argument type
     * @param pFn member function pointer
     * @param pObject object pointer
     * @return Returns TRUE on success, FALSE on failure
     */
    template <typename T, typename A>
    BOOL subscribeEvent(BOOL (T::*pFn)(A *), T *pObject)
    {
        return subscribeEvent(A::EventID, Subscriber(pFn, pObject));
    }

    /**
     * @brief Subscribe to event (template function)
     * @tparam A event argument type
     * @param pFn function pointer
     * @return Returns TRUE on success, FALSE on failure
     */
    template <typename A>
    BOOL subscribeEvent(BOOL (*pFn)(A *))
    {
        return subscribeEvent(A::EventID, Subscriber(pFn));
    }

    /**
     * @brief Subscribe to event (template function)
     * @tparam T object type
     * @param pFn member function pointer
     * @param pObject object pointer
     * @return Returns TRUE on success, FALSE on failure
     */
    template <typename T>
    BOOL subscribeEvent(DWORD dwEventID, BOOL (T::*pFn)(IEvtArgs *), T *pObject)
    {
        return subscribeEvent(dwEventID, Subscriber(pFn, pObject));
    }

    /**
     * @brief Unsubscribe from event
     * @param dwEventID event ID
     * @param subscriber event slot object pointer
     * @return Returns TRUE on success, FALSE on failure
     */
    BOOL unsubscribeEvent(DWORD dwEventID, const IEvtSlot *subscriber);

    /**
     * @brief Unsubscribe from event
     * @param dwEventID event ID
     * @param subscriber event slot object
     * @return Returns TRUE on success, FALSE on failure
     */
    BOOL unsubscribeEvent(DWORD dwEventID, const IEvtSlot &subscriber)
    {
        return unsubscribeEvent(dwEventID, &subscriber);
    }

    /**
     * @brief Unsubscribe from event (template function)
     * @tparam T object type
     * @tparam A event argument type
     * @param pFn member function pointer
     * @param pObject object pointer
     * @return Returns TRUE on success, FALSE on failure
     */
    template <typename T, typename A>
    BOOL unsubscribeEvent(BOOL (T::*pFn)(A *), T *pObject)
    {
        return unsubscribeEvent(A::EventID, Subscriber(pFn, pObject));
    }

    /**
     * @brief Unsubscribe from event (template function)
     * @tparam A event argument type
     * @param pFn function pointer
     * @return Returns TRUE on success, FALSE on failure
     */
    template <typename A>
    BOOL unsubscribeEvent(BOOL (*pFn)(A *))
    {
        return unsubscribeEvent(A::EventID, Subscriber(pFn));
    }

    /**
     * @brief Unsubscribe from event (template function)
     * @tparam T object type
     * @param pFn member function pointer
     * @param pObject object pointer
     * @return Returns TRUE on success, FALSE on failure
     */
    template <typename T>
    BOOL unsubscribeEvent(DWORD dwEventID, BOOL (T::*pFn)(IEvtArgs *), T *pObject)
    {
        return unsubscribeEvent(dwEventID, Subscriber(pFn, pObject));
    }

    /**
     * @brief Fire event
     * @param args Event argument object
     */
    void FireEvent(IEvtArgs *args);

    /**
     * @brief Check whether the event set is muted
     * @return Returns TRUE if muted, FALSE otherwise
     */
    BOOL isMuted(void) const
    {
        return m_nMuted > 0;
    }

    /**
     * @brief Set the mute state of the event set
     * @param setting TRUE to mute, FALSE to unmute
     */
    void setMutedState(BOOL setting);

  protected:
    /**
     * @brief Get the event object
     * @param dwEventID event ID
     * @return pointer to the event object, NULL if not found
     */
    SEvent *GetEventObject(const DWORD dwEventID);

    SArray<SEvent *> m_evtArr; /**<  Event array */
    int m_nMuted;              /**<  Mute state count */
};

SNSEND

#endif /**< __SEVENTSET__H__ */