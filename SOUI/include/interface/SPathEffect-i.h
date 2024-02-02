#ifndef __SPATHEFFECT_I__H__
#define __SPATHEFFECT_I__H__

#include <interface/obj-ref-i.h>
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

#undef INTERFACE
#define INTERFACE ICornerPathEffect
DECLARE_INTERFACE_(ICornerPathEffect, IPathEffect)
{
	DECLARE_CLASS_SIID( 0x3C4A07C2,0x2617,0x432a,0xADFC,0x8921,0xD8B2E2BD)
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

#undef INTERFACE
#define INTERFACE IDashPathEffect
DECLARE_INTERFACE_(IDashPathEffect, IPathEffect)
{
	DECLARE_CLASS_SIID( 0x5FA8CF4C,0x8C83,0x4eac,0x82C0,0xEE16,0x0E04D78C)
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

#undef INTERFACE
#define INTERFACE IDashPathEffect
DECLARE_INTERFACE_(IDiscretePathEffect, IPathEffect)
{
	DECLARE_CLASS_SIID( 0x51D32163,0x594E,0x4fbe,0xB2A5,0x8A07,0xDEDDBEAB)
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