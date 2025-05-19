#ifndef __SEVENTSLOT__H__
#define __SEVENTSLOT__H__

#include <interface/SEvtArgs-i.h>
#include <helper/obj-ref-impl.hpp>

#if !defined(_WIN32) || _MSC_VER >= 1700
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
 * @brief 定义了事件槽的类型
 */
enum _SLOTTYPE
{
    SLOT_FUN,        ///< 自由函数槽
    SLOT_STDFUNCTOR, ///< 标准函数对象槽
    SLOT_MEMBER,     ///< 成员函数槽
    SLOT_USER        ///< 用户自定义槽
};

/**
 * @class FreeFunctionSlot
 * @brief 通过自由函数指针回调的槽函数类
 */
class FreeFunctionSlot : public TObjRefImpl<IEvtSlot> {
  public:
    /**
     * @brief 构造函数
     * @param func 自由函数指针
     * @param ctx 上下文指针
     */
    FreeFunctionSlot(FunCallback func, void *ctx)
        : d_function(func)
        , d_ctx(ctx)
    {
    }

    /**
     * @brief 运行槽函数
     * @param pArg 事件参数对象
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, Run)(THIS_ IEvtArgs *pArg) OVERRIDE
    {
        return d_function(pArg, d_ctx);
    }

    /**
     * @brief 克隆槽函数对象
     * @return 克隆的槽函数对象指针
     */
    STDMETHOD_(IEvtSlot *, Clone)(THIS) SCONST OVERRIDE
    {
        return new FreeFunctionSlot(d_function, d_ctx);
    }

    /**
     * @brief 比较两个槽函数对象是否相等
     * @param sour 源槽函数对象指针
     * @return 相等返回TRUE，不相等返回FALSE
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
     * @brief 获取槽函数类型
     * @return 槽函数类型
     */
    STDMETHOD_(UINT, GetSlotType)(THIS) SCONST OVERRIDE
    {
        return SLOT_FUN;
    }

  private:
    FunCallback d_function; ///< 自由函数指针
    void *d_ctx;            ///< 上下文指针
};

#if ENABLE_STDFUNCTOR
typedef std::function<BOOL(IEvtArgs *)> StdFunCallback;

/**
 * @class StdFunctionSlot
 * @brief 通过标准函数对象回调的槽函数类
 */
class StdFunctionSlot : public TObjRefImpl<IEvtSlot> {
  public:
    /**
     * @brief 构造函数
     * @param fun 标准函数对象
     */
    StdFunctionSlot(const StdFunCallback &fun)
        : d_function(fun)
    {
    }

    /**
     * @brief 运行槽函数
     * @param pArg 事件参数对象
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, Run)(THIS_ IEvtArgs *pArg) OVERRIDE
    {
        return d_function(pArg);
    }

    /**
     * @brief 克隆槽函数对象
     * @return 克隆的槽函数对象指针
     */
    STDMETHOD_(IEvtSlot *, Clone)(THIS) SCONST OVERRIDE
    {
        return new StdFunctionSlot(d_function);
    }

    /**
     * @brief 比较两个槽函数对象是否相等
     * @param sour 源槽函数对象指针
     * @return 相等返回TRUE，不相等返回FALSE
     */
    STDMETHOD_(BOOL, Equal)(THIS_ const IEvtSlot *sour) SCONST OVERRIDE
    {
        SASSERT(sour);
        if (sour->GetSlotType() != SLOT_STDFUNCTOR)
            return FALSE;
        // Note: as we don't know how to compare two std::function objects, here just return TRUE.
        // Such a behavior will make it impossible to subscribe an event with two different slots.
        return TRUE;
    }

    /**
     * @brief 获取槽函数类型
     * @return 槽函数类型
     */
    STDMETHOD_(UINT, GetSlotType)(THIS) SCONST OVERRIDE
    {
        return SLOT_STDFUNCTOR;
    }

  private:
    StdFunCallback d_function; ///< 标准函数对象
};
#endif // ENABLE_STDFUNCTOR

/**
 * @class MemberFunctionSlot
 * @brief 通过类成员函数回调的槽函数类模板
 * @tparam T 类型
 * @tparam A 事件参数类型
 */
template <typename T, typename A>
class MemberFunctionSlot : public TObjRefImpl<IEvtSlot> {
  public:
    /**
     * @brief 成员函数槽类型
     */
    typedef BOOL (T::*MemberFunctionType)(A *);

    /**
     * @brief 构造函数
     * @param func 成员函数指针
     * @param obj 对象指针
     */
    MemberFunctionSlot(MemberFunctionType func, T *obj)
        : d_function(func)
        , d_object(obj)
    {
    }

    /**
     * @brief 运行槽函数
     * @param pArg 事件参数对象
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, Run)(THIS_ IEvtArgs *pArg) OVERRIDE
    {
        return (d_object->*d_function)(static_cast<A *>(pArg));
    }

    /**
     * @brief 克隆槽函数对象
     * @return 克隆的槽函数对象指针
     */
    STDMETHOD_(IEvtSlot *, Clone)(THIS) SCONST OVERRIDE
    {
        return new MemberFunctionSlot(d_function, d_object);
    }

    /**
     * @brief 比较两个槽函数对象是否相等
     * @param sour 源槽函数对象指针
     * @return 相等返回TRUE，不相等返回FALSE
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
     * @brief 获取槽函数类型
     * @return 槽函数类型
     */
    STDMETHOD_(UINT, GetSlotType)(THIS) SCONST OVERRIDE
    {
        return SLOT_MEMBER;
    }

  private:
    MemberFunctionType d_function; ///< 成员函数指针
    T *d_object;                   ///< 对象指针
};

template <typename T, typename A = IEvtArgs>
class MemberFunctionSlot;

/**
 * @brief 创建成员函数槽对象
 * @tparam T 类型
 * @tparam A 事件参数类型
 * @param pFn 成员函数指针
 * @param pObject 对象指针
 * @return 成员函数槽对象
 */
template <class T, class A>
MemberFunctionSlot<T, A> Subscriber(BOOL (T::*pFn)(A *), T *pObject)
{
    return MemberFunctionSlot<T, A>(pFn, pObject);
}

/**
 * @brief 创建自由函数槽对象
 * @param pFn 自由函数指针
 * @param ctx 上下文指针
 * @return 自由函数槽对象
 */
inline FreeFunctionSlot Subscriber(FunCallback pFn, void *ctx)
{
    return FreeFunctionSlot(pFn, ctx);
}

SNSEND
#endif // __SEVENTSLOT__H__