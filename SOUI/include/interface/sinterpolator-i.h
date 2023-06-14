#pragma once
#include <interface/sobject-i.h>
SNSBEGIN

#undef INTERFACE
#define INTERFACE IInterpolator
DECLARE_INTERFACE_(IInterpolator, IObject)
{
#ifdef __cplusplus
    /**
     * GetClassName
     * @brief    获得对象类型名称
     * @return   LPCWSTR -- 类名
     * Describe  静态函数
     */
    static LPCWSTR GetClassName()
    {
        return L"IInterpolator";
    }

    /**
     * GetClassType
     * @brief    获得对象类型
     * @return   int -- 类型
     * Describe  静态函数
     */
    static int GetClassType()
    {
        return Interpolator;
    }
#endif
#include <interface/SobjectApi.h>

    //----------------------------------------------------------------------------------

    /**
     * getInterpolation
     * @brief    计算出插值
     * @param    float input --  输入值[0,1]
     * @return   float -- 输出插值结果
     * Describe
     */
    STDMETHOD_(float, getInterpolation)(CTHIS_ float input) SCONST PURE;
};
SNSEND