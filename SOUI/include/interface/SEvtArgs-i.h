#ifndef __SEVTARGS_I__H__
#define __SEVTARGS_I__H__

#include <interface/sobject-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IEvtArgs
DECLARE_INTERFACE_(IEvtArgs, IObject)
{
    DEF_OBJ_BASE(IEvtArgs, Event)
#include <interface/SobjectApi.h>

    //-------------------------------------------------------------------------------------------

    /**
     * @brief Get the event sender object
     * @return IObject *--event sender object
     */
    STDMETHOD_(IObject *, Sender)(THIS) PURE;

    /**
     * @brief Get the event sender object ID
     * @return int--event sender object ID
     */
    STDMETHOD_(int, IdFrom)(CTHIS) SCONST PURE;

    /**
     * @brief Set the event sender object ID
     * @param id int--sender object ID
     * @return void
     */
    STDMETHOD_(void, SetIdFrom)(THIS_ int id) PURE;

    /**
     * @brief Get the event sender object Name
     * @return LPCWSTR--event sender object Name
     */
    STDMETHOD_(LPCWSTR, NameFrom)(CTHIS) SCONST PURE;

    /**
     * @brief Set the event sender object Name
     * @param name LPCWSTR--event sender object Name
     * @return void
     */
    STDMETHOD_(void, SetNameFrom)(THIS_ LPCWSTR name) PURE;

    /**
     * @brief Get whether the event needs bubble propagation
     * @return BOOL TRUE--bubble propagation
     */
    STDMETHOD_(BOOL, IsBubbleUp)(CTHIS) SCONST PURE;

    /**
     * @brief Set the event's bubble propagation
     * @param bBubbleUp BOOL, FALSE--stop propagation
     * @return void
     */
    STDMETHOD_(void, SetBubbleUp)(THIS_ BOOL bBubbleUp) PURE;

    /**
     * @brief Get the event handling count
     * @return UINT event handling count
     */
    STDMETHOD_(UINT, HandleCount)(CTHIS) SCONST PURE;

    /**
     * @brief Increment the event handling count
     * @return void
     */
    STDMETHOD_(void, IncreaseHandleCount)(THIS) PURE;

    /**
     * @brief Get the pointer to the data structure associated with the event object
     * @return LPVOID pointer to the data structure associated with the event object
     * @remark Different event objects contain different data; obtain the pointer and cast it to the structure pointer. Mainly used by the C interface.
     */
    STDMETHOD_(LPVOID, Data)(THIS) PURE;
};

/**
@brief
    Defines abstract interface which will be used when constructing various
    functor objects that bind slots to signals (or in CEGUI terms, handlers to
    events).
*/
#undef INTERFACE
#define INTERFACE IEvtSlot
DECLARE_INTERFACE_(IEvtSlot, IObjRef)
{
    /** Add reference */
    /**
     * @return long - current reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /** Release reference */
    /**
     * @return long - current reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /** Release object */
    /**
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Execute event
     * @param pArg IEvtArgs* event object
     * @return BOOL TRUE--the event was handled and the event object's usage count is incremented, FALSE--event not handled
     */
    STDMETHOD_(BOOL, Run)(THIS_ IEvtArgs * pArg) PURE;

    /**
     * @brief Clone an event slot
     * @return IEvtSlot * cloned object
     */
    STDMETHOD_(IEvtSlot *, Clone)(CTHIS) SCONST PURE;

    /**
     * @brief Check whether two IEvtSlot are equal
     * @param sour IEvtSlot * IEvtSlot to compare
     * @return BOOL TRUE--equal, FALSE--not equal
     */
    STDMETHOD_(BOOL, Equal)(CTHIS_ const IEvtSlot *sour) SCONST PURE;

    /**
     * @brief Get the IEvtSlot type
     * @return UINT IEvtSlot type
     */
    STDMETHOD_(UINT, GetSlotType)(CTHIS) SCONST PURE;
};

/**
 * @brief Event handling callback
 * @param pEvt IEvtArgs * event object
 * @param Ctx void * event handling context
 * @return BOOL TRUE--event handled, increment handling count
 */
typedef BOOL (*FunCallback)(IEvtArgs *pEvt, void *Ctx);

#define EventArgs IEvtArgs
SNSEND
#endif /**< __SEVTARGS_I__H__ */
