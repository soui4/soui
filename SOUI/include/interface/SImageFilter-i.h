#ifndef __SIMAGEEFFECT_I__H__
#define __SIMAGEEFFECT_I__H__

#include <interface/obj-ref-i.h>
#include <SGuidDef.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IImageFilter
DECLARE_INTERFACE_(IImageFilter, IObjRef)
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
     * @brief 获取ImageEffect的原始对象指针
     * @return void* 原始对象指针
     */
    STDMETHOD_(void *, GetPtr)(THIS) PURE;
};

// {F8E7D6C5-B4A3-4291-8E7F-D6C5B4A32910}
DEFINE_SGUID(IID_IDropShadowImageFilter, 0xf8e7d6c5, 0xb4a3, 0x4291, 0x8e, 0x7f, 0xd6, 0xc5, 0xb4, 0xa3, 0x29, 0x10);
#undef INTERFACE
#define INTERFACE IDropShadowImageFilter
DECLARE_INTERFACE_(IDropShadowImageFilter, IImageFilter)
{
    DECLARE_CLASS_SIID(IID_IDropShadowImageFilter)
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
     * @param dx 水平偏移
     * @param dy 垂直偏移
     * @param sigmaX 水平方向的模糊半径
     * @param sigmaY 垂直方向的模糊半径
     * @param color 颜色
     * @param input 输入图像
     */
    STDMETHOD_(void, Init)(THIS_ float dx, float dy, float sigmaX, float sigmaY, COLORREF color, IImageFilter *input DEF_VAL(NULL)) PURE;
};

// {A7C3F8D2-9B4E-4F1A-8D6C-3E5B7A2F9C1D}
DEFINE_SGUID(IID_IBlurImageFilter, 0xa7c3f8d2, 0x9b4e, 0x4f1a, 0x8d, 0x6c, 0x3e, 0x5b, 0x7a, 0x2f, 0x9c, 0x1d);
#undef INTERFACE
#define INTERFACE IBlurImageFilter
DECLARE_INTERFACE_(IBlurImageFilter, IImageFilter)
{
    DECLARE_CLASS_SIID(IID_IBlurImageFilter)
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
     * @brief 获取ImageEffect的初始化参数
     * @param sigmaX 水平方向的模糊半径
     * @param sigmaY 垂直方向的模糊半径
     * @param input 输入图像
     */
    STDMETHOD_(void, Init)(THIS_ float sigmaX, float sigmaY, IImageFilter *input DEF_VAL(NULL)) PURE;
};

// {D8C7B6A5-F4E3-4271-8D7C-B6A5F4E32710}
DEFINE_SGUID(IID_IComposeImageFilter, 0xd8c7b6a5, 0xf4e3, 0x4271, 0x8d, 0x7c, 0xb6, 0xa5, 0xf4, 0xe3, 0x27, 0x10);
#undef INTERFACE
#define INTERFACE IComposeImageFilter
DECLARE_INTERFACE_(IComposeImageFilter, IImageFilter)
{
    DECLARE_CLASS_SIID(IID_IComposeImageFilter)
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
     * @param outer 外层图像
     * @param inner 内层图像
     */
    STDMETHOD_(void, Init)(THIS_ IImageFilter * outer, IImageFilter * inner DEF_VAL(NULL)) PURE;
};

// {C7B6A594-E3D2-4160-9C6B-A594E3D21600}
DEFINE_SGUID(IID_IMergeImageFilter, 0xc7b6a594, 0xe3d2, 0x4160, 0x9c, 0x6b, 0xa5, 0x94, 0xe3, 0xd2, 0x16, 0x00);
#undef INTERFACE
#define INTERFACE IMergeImageFilter
DECLARE_INTERFACE_(IMergeImageFilter, IImageFilter)
{
    DECLARE_CLASS_SIID(IID_IMergeImageFilter)
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
     * @param filters 图像数组
     * @param count 图像数量
     */
    STDMETHOD_(void, Init)(THIS_ IImageFilter * filters[], int count) PURE;
};

// {B6A59483-D2C1-405F-8B5A-9483D2C10501}
DEFINE_SGUID(IID_ITileImageFilter, 0xb6a59483, 0xd2c1, 0x405f, 0x8b, 0x5a, 0x94, 0x83, 0xd2, 0xc1, 0x05, 0x01);
#undef INTERFACE
#define INTERFACE ITileImageFilter
DECLARE_INTERFACE_(ITileImageFilter, IImageFilter)
{
    DECLARE_CLASS_SIID(IID_ITileImageFilter)
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
     * @brief 获取ImageEffect的初始化参数
     * @param srcRect 源图像区域
     * @param dstRect 目标图像区域
     * @param input 输入图像
     */
    STDMETHOD_(void, Init)(THIS_ RECT srcRect, RECT dstRect, IImageFilter * input DEF_VAL(NULL)) PURE;
};

// {A5948372-C1B0-4F4E-9A49-8372C1B04E02}
DEFINE_SGUID(IID_IOffsetImageFilter, 0xa5948372, 0xc1b0, 0x4f4e, 0x9a, 0x49, 0x83, 0x72, 0xc1, 0xb0, 0x4e, 0x02);
#undef INTERFACE
#define INTERFACE IOffsetImageFilter
DECLARE_INTERFACE_(IOffsetImageFilter, IImageFilter)
{
    DECLARE_CLASS_SIID(IID_IOffsetImageFilter)
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
     * @brief 获取ImageEffect的初始化参数
     * @param dx 水平偏移
     * @param dy 垂直偏移
     * @param input 输入图像
     */
    STDMETHOD_(void, Init)(THIS_ float dx, float dy, IImageFilter *input DEF_VAL(NULL)) PURE;
};

SNSEND
#endif // __SIMAGEEFFECT_I__H__