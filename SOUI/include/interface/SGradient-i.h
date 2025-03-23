#ifndef _GRADIENT_I_H_
#define _GRADIENT_I_H_

#include <interface/sobject-i.h>

SNSBEGIN

typedef struct _GradientItem
{
    COLORREF cr;
    float pos;
} GradientItem;

#undef INTERFACE
#define INTERFACE IGradient
DECLARE_INTERFACE_(IGradient, IObject)
{
    /**
     * @brief 添加引用
     * @return long -- 引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief 释放引用
     * @return long -- 引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief 释放对象
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief 获取渐变数据
     * @return const GradientItem* -- 渐变数据
     */
    STDMETHOD_(const GradientItem *, GetGradientData)(CTHIS) SCONST PURE;

    /**
     * @brief 获取渐变长度
     * @return int -- 渐变长度
     */
    STDMETHOD_(int, GetGradientLength)(CTHIS) SCONST PURE;
};

SNSEND
#endif //_GRADIENT_I_H_



