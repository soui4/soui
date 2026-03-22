#ifndef __SMASKFILTER_I__H__
#define __SMASKFILTER_I__H__

#include <interface/obj-ref-i.h>
#include <SGuidDef.h>

SNSBEGIN

typedef enum _BlurStyle
{
    kNone_BLurStyle = -1,
    kNormal_BlurStyle = 0, //!< 模糊内部和外部
    kSolid_BlurStyle,      //!< 固定内部，模糊外部
    kOuter_BlurStyle,      //!< 无内部，模糊外部
    kInner_BlurStyle,      //!< 模糊内部，无外部

    kLastEnum_BlurStyle = kInner_BlurStyle
} BlurStyle;

typedef enum _BlurFlags
{
    kNone_BlurFlag = 0x00,
    /** The blur layer's radius is not affected by transforms */
    kIgnoreTransform_BlurFlag = 0x01,
    /** Use a smoother, higher quality blur algorithm */
    kHighQuality_BlurFlag = 0x02,
    /** mask for all blur flags */
    kAll_BlurFlag = 0x03
} BlurFlags;

#undef INTERFACE
#define INTERFACE IMaskFilter
DECLARE_INTERFACE_(IMaskFilter, IObjRef)
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
     * @brief 获取MaskFilter的原始对象指针
     * @return void* 原始对象指针
     */
    STDMETHOD_(void *, GetPtr)(THIS) PURE;
};

// {A7C3F8D2-9B5E-4F1A-8D6C-3E5B7A2F9C1D}
DEFINE_SGUID(IID_IMaskFilterBlur, 0xa7c3f8d2, 0x9b5e, 0x4f1a, 0x8d, 0x6c, 0x3e, 0x5b, 0x7a, 0x2f, 0x9c, 0x1d);
#undef INTERFACE
#define INTERFACE IMaskFilterBlur
DECLARE_INTERFACE_(IMaskFilterBlur, IMaskFilter)
{
    DECLARE_CLASS_SIID(IID_IMaskFilterBlur)
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
     * @brief 获取ImageEffect的原始对象指针
     * @return void* 原始对象指针
     */
    STDMETHOD_(void *, GetPtr)(THIS) PURE;

    STDMETHOD_(void, Init)(THIS_ float fRadius, BlurStyle eStyle, BlurFlags eFlags) PURE;
};

// {B8D4E9F3-AC6F-4E2B-9E7D-4F6C8B3A0E2F}
DEFINE_SGUID(IID_IMaskFilterEmboss, 0xb8d4e9f3, 0xac6f, 0x4e2b, 0x9e, 0x7d, 0x4f, 0x6c, 0x8b, 0x3a, 0x0e, 0x2f);
#undef INTERFACE
#define INTERFACE IMaskFilterEmboss
DECLARE_INTERFACE_(IMaskFilterEmboss, IMaskFilter)
{
    DECLARE_CLASS_SIID(IID_IMaskFilterEmboss)
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
     * @brief 获取ImageEffect的原始对象指针
     * @return void* 原始对象指针
     */
    STDMETHOD_(void *, GetPtr)(THIS) PURE;

    /**
     * @brief 初始化
     * @param blurSigma 模糊sigma
     * @param direction 模糊方向
     * @param ambient 环境光
     * @param specular 镜面光
     */
    STDMETHOD_(void, Init)(THIS_ float blurSigma,float direction[3], float ambient, float specular) PURE;
};


SNSEND
#endif // __SMASKFILTER_I__H__