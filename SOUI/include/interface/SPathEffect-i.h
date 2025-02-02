#ifndef __SPATHEFFECT_I__H__
#define __SPATHEFFECT_I__H__

#include <interface/obj-ref-i.h>
#include <SGuidDef.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IPathEffect
DECLARE_INTERFACE_(IPathEffect, IObjRef)
{
    //!添加引用
    /*!
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //!释放引用
    /*!
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //!释放对象
    /*!
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief 获取PathEffect的原始对象指针
     * @return void* 原始对象指针
     */
    STDMETHOD_(void *, GetRealPathEffect)(THIS) PURE;
};

// {14B1ECBA-7E3A-4dbc-A9DA-D40F2A9E5EC3}
DEFINE_SGUID(IID_ICornerPathEffect, 0x14b1ecba, 0x7e3a, 0x4dbc, 0xa9, 0xda, 0xd4, 0xf, 0x2a, 0x9e, 0x5e, 0xc3);
#undef INTERFACE
#define INTERFACE ICornerPathEffect
DECLARE_INTERFACE_(ICornerPathEffect, IPathEffect)
{
    DECLARE_CLASS_SIID(IID_ICornerPathEffect)
    //!添加引用
    /*!
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //!释放引用
    /*!
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //!释放对象
    /*!
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief 获取PathEffect的原始对象指针
     * @return void* 原始对象指针
     */
    STDMETHOD_(void *, GetRealPathEffect)(THIS) PURE;

    STDMETHOD_(void, Init)(THIS_ float radius) PURE;
};

// {517FB1CF-550E-4e07-822A-425131A32DE6}
DEFINE_SGUID(IID_IDashPathEffect, 0x517fb1cf, 0x550e, 0x4e07, 0x82, 0x2a, 0x42, 0x51, 0x31, 0xa3, 0x2d, 0xe6);
#undef INTERFACE
#define INTERFACE IDashPathEffect
DECLARE_INTERFACE_(IDashPathEffect, IPathEffect)
{
    DECLARE_CLASS_SIID(IID_IDashPathEffect)
    //!添加引用
    /*!
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //!释放引用
    /*!
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //!释放对象
    /*!
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief 获取PathEffect的原始对象指针
     * @return void* 原始对象指针
     */
    STDMETHOD_(void *, GetRealPathEffect)(THIS) PURE;

    STDMETHOD_(void, Init)(THIS_ float intervals[], int count, float phase) PURE;
};

// {90A2EE62-3BC7-45da-AE05-B51302CC4357}
DEFINE_SGUID(IID_IDiscretePathEffect, 0x90a2ee62, 0x3bc7, 0x45da, 0xae, 0x5, 0xb5, 0x13, 0x2, 0xcc, 0x43, 0x57);
#undef INTERFACE
#define INTERFACE IDiscretePathEffect
DECLARE_INTERFACE_(IDiscretePathEffect, IPathEffect)
{
    DECLARE_CLASS_SIID(IID_IDiscretePathEffect)
    //!添加引用
    /*!
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //!释放引用
    /*!
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //!释放对象
    /*!
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief 获取PathEffect的原始对象指针
     * @return void* 原始对象指针
     */
    STDMETHOD_(void *, GetRealPathEffect)(THIS) PURE;

    STDMETHOD_(void, Init)(THIS_ float segmentLength, float deviation) PURE;
};

SNSEND
#endif // __SPATHEFFECT_I__H__