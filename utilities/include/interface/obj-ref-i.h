#ifndef __OBJ_REF_I__H__
#define __OBJ_REF_I__H__

#include <sdef.h>
#include <objbase.h>
#ifdef _WIN32
#include <sguid.h>
#else
#include <guiddef.h>
#endif // _WIN32

#undef INTERFACE
#define INTERFACE IObjRef
DECLARE_INTERFACE(IObjRef)
{
    /**
     * @brief Increases the reference count.
     * @return The new reference count.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decreases the reference count.
     * @return The new reference count.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Called when the reference count reaches zero and the object is about to be released.
     * @details This method is intended to perform any necessary cleanup before the object is deleted.
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;
};

#endif // __OBJ_REF_I__H__