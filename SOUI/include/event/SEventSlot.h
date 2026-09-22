#ifndef __SEVENTSLOT__H__
#define __SEVENTSLOT__H__

#include <interface/SEvtArgs-i.h>
#include <helper/obj-ref-impl.hpp>

#if __cplusplus >= 201103L
#define ENABLE_STDFUNCTOR 1
#else
#define ENABLE_STDFUNCTOR 0
#endif

#if ENABLE_STDFUNCTOR
#include <functional>
#endif

SNSBEGIN

/**
 * @enum _SLOTTYPE
 * @brief Defines the types of event slots
 */
enum _SLOTTYPE
{
    SLOT_FUN,        /**<  Free function slot */
    SLOT_STDFUNCTOR, /**<  Standard function object slot */
    SLOT_MEMBER,     /**<  Member function slot */
    SLOT_USER        /**<  User-defined slot */
};

/**
 * @class FreeFunctionSlot
 * @brief Slot class invoked via free function pointer callback
 */
class FreeFunctionSlot : public TObjRefImpl<IEvtSlot> {
  public:
    /**
     * @brief Constructor
     * @param func free function pointer
     * @param ctx context pointer
     */
    FreeFunctionSlot(FunCallback func, void *ctx)
        : d_function(func)
        , d_ctx(ctx)
    {
    }

    /**
     * @brief Run the slot function
     * @param pArg event argument object
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, Run)(THIS_ IEvtArgs *pArg) OVERRIDE
    {
        return d_function(pArg, d_ctx);
    }

    /**
     * @brief Clone the slot function object
     * @return pointer to the cloned slot function object
     */
    STDMETHOD_(IEvtSlot *, Clone)(THIS) SCONST OVERRIDE
    {
        return new FreeFunctionSlot(d_function, d_ctx);
    }

    /**
     * @brief Compare whether two slot function objects are equal
     * @param sour source slot function object pointer
     * @return Returns TRUE if equal, FALSE otherwise
     */
    STDMETHOD_(BOOL, Equal)(THIS_ const IEvtSlot *sour) SCONST OVERRIDE
    {
        if (sour->GetSlotType() != SLOT_FUN)
            return FALSE;
        const FreeFunctionSlot *psour = static_cast<const FreeFunctionSlot *>(sour);
        SASSERT(psour);
        return psour->d_function == d_function && psour->d_ctx == d_ctx;
    }

    /**
     * @brief Get the slot function type
     * @return slot function type
     */
    STDMETHOD_(UINT, GetSlotType)(THIS) SCONST OVERRIDE
    {
        return SLOT_FUN;
    }

  private:
    FunCallback d_function; /**<  Free function pointer */
    void *d_ctx;            /**<  Context pointer */
};

#if ENABLE_STDFUNCTOR
typedef std::function<BOOL(IEvtArgs *)> StdFunCallback;

/**
 * @class StdFunctionSlot
 * @brief Slot class invoked via standard function object callback
 */
class StdFunctionSlot : public TObjRefImpl<IEvtSlot> {
  public:
    /**
     * @brief Constructor
     * @param fun standard function object
     */
    StdFunctionSlot(const StdFunCallback &fun)
        : d_function(fun)
    {
    }

    /**
     * @brief Run the slot function
     * @param pArg event argument object
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, Run)(THIS_ IEvtArgs *pArg) OVERRIDE
    {
        return d_function(pArg);
    }

    /**
     * @brief Clone the slot function object
     * @return pointer to the cloned slot function object
     */
    STDMETHOD_(IEvtSlot *, Clone)(THIS) SCONST OVERRIDE
    {
        return new StdFunctionSlot(d_function);
    }

    /**
     * @brief Compare whether two slot function objects are equal
     * @param sour source slot function object pointer
     * @return Returns TRUE if equal, FALSE otherwise
     */
    STDMETHOD_(BOOL, Equal)(THIS_ const IEvtSlot *sour) SCONST OVERRIDE
    {
        SASSERT(sour);
        if (sour->GetSlotType() != SLOT_STDFUNCTOR)
            return FALSE;
        /** Note: as we don't know how to compare two std::function objects, here just return TRUE. */
        /** Such a behavior will make it impossible to subscribe an event with two different slots. */
        return TRUE;
    }

    /**
     * @brief Get the slot function type
     * @return slot function type
     */
    STDMETHOD_(UINT, GetSlotType)(THIS) SCONST OVERRIDE
    {
        return SLOT_STDFUNCTOR;
    }

  private:
    StdFunCallback d_function; /**<  Standard function object */
};
#endif /**< ENABLE_STDFUNCTOR */

/**
 * @class MemberFunctionSlot
 * @brief Slot class template invoked via class member function callback
 * @tparam T type
 * @tparam A event argument type
 */
template <typename T, typename A>
class MemberFunctionSlot : public TObjRefImpl<IEvtSlot> {
  public:
    /**
     * @brief Member function slot type
     */
    typedef BOOL (T::*MemberFunctionType)(A *);

    /**
     * @brief Constructor
     * @param func member function pointer
     * @param obj object pointer
     */
    MemberFunctionSlot(MemberFunctionType func, T *obj)
        : d_function(func)
        , d_object(obj)
    {
    }

    /**
     * @brief Run the slot function
     * @param pArg event argument object
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, Run)(THIS_ IEvtArgs *pArg) OVERRIDE
    {
        return (d_object->*d_function)(static_cast<A *>(pArg));
    }

    /**
     * @brief Clone the slot function object
     * @return pointer to the cloned slot function object
     */
    STDMETHOD_(IEvtSlot *, Clone)(THIS) SCONST OVERRIDE
    {
        return new MemberFunctionSlot(d_function, d_object);
    }

    /**
     * @brief Compare whether two slot function objects are equal
     * @param sour source slot function object pointer
     * @return Returns TRUE if equal, FALSE otherwise
     */
    STDMETHOD_(BOOL, Equal)(THIS_ const IEvtSlot *sour) SCONST OVERRIDE
    {
        SASSERT(sour);
        if (sour->GetSlotType() != GetSlotType())
            return FALSE;
        const MemberFunctionSlot *psour = static_cast<const MemberFunctionSlot *>(sour);
        SASSERT(psour);
        return psour->d_function == d_function && psour->d_object == d_object;
    }

    /**
     * @brief Get the slot function type
     * @return slot function type
     */
    STDMETHOD_(UINT, GetSlotType)(THIS) SCONST OVERRIDE
    {
        return SLOT_MEMBER;
    }

  private:
    MemberFunctionType d_function; /**<  Member function pointer */
    T *d_object;                   /**<  Object pointer */
};

template <typename T, typename A = IEvtArgs>
class MemberFunctionSlot;

/**
 * @brief Create a member function slot object
 * @tparam T type
 * @tparam A event argument type
 * @param pFn member function pointer
 * @param pObject object pointer
 * @return member function slot object
 */
template <class T, class A>
MemberFunctionSlot<T, A> Subscriber(BOOL (T::*pFn)(A *), T *pObject)
{
    return MemberFunctionSlot<T, A>(pFn, pObject);
}

/**
 * @brief Create a free function slot object
 * @param pFn free function pointer
 * @param ctx context pointer
 * @return free function slot object
 */
inline FreeFunctionSlot Subscriber(FunCallback pFn, void *ctx)
{
    return FreeFunctionSlot(pFn, ctx);
}

SNSEND
#endif /**< __SEVENTSLOT__H__ */