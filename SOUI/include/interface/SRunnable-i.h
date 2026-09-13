#ifndef __SRUNNABLE_I__H__
#define __SRUNNABLE_I__H__
#include <interface/obj-ref-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IRunnable
DECLARE_INTERFACE_(IRunnable, IObjRef)
{
    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Clone current object
     * @return IRunnable *--newly cloned object
     */
    STDMETHOD_(IRunnable *, clone)(CTHIS) SCONST PURE;

    /**
     * @brief Run this object
     * @return
     */
    STDMETHOD_(void, run)(THIS) PURE;

    /**
     * @brief Get the host of this Runnable
     * @return void *--host pointer, NULL for FreeFunction objects
     */
    STDMETHOD_(void *, getObject)(THIS) PURE;

    /**
     * @brief Get runnable description
     * @return const char *--description
     */
    STDMETHOD_(const char *, getClassInfo)(CTHIS) SCONST PURE;
};

SNSEND
#endif /**< __SRUNNABLE_I__H__ */