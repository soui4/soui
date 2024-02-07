#ifndef __SRUNNABLE_I__H__
#define __SRUNNABLE_I__H__
#include <interface/obj-ref-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IRunnable
DECLARE_INTERFACE_(IRunnable, IObjRef)
{
    /**
     * @brief 增加引用计数
     * @return 新引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief 减少引用计数
     * @return 新引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief 释放对象
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief clone当前对象
     * @return IRunnable *--clone得到的新对象
     */
    STDMETHOD_(IRunnable *, clone)(CTHIS) SCONST PURE;

    /**
     * @brief 运行该对象
     * @return
     */
    STDMETHOD_(void, run)(THIS) PURE;

    /**
     * @brief 获取该Runnable的宿主
     * @return void *--宿主指针，对于FreeFunction对象，该值为NULL
     */
    STDMETHOD_(void *, getObject)(THIS) PURE;

    /**
     * @brief 获取runnable的描述
     * @return const char *--描述
     */
    STDMETHOD_(const char *, getClassInfo)(CTHIS) SCONST PURE;
};

SNSEND
#endif // __SRUNNABLE_I__H__