#ifndef __SINTERPOLATOR_I__H__
#define __SINTERPOLATOR_I__H__
#include <interface/sobject-i.h>
SNSBEGIN

#undef INTERFACE
#define INTERFACE IInterpolator
DECLARE_INTERFACE_(IInterpolator, IObject)
{
    DEF_OBJ_BASE(IInterpolator, Interpolator)
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
#endif // __SINTERPOLATOR_I__H__