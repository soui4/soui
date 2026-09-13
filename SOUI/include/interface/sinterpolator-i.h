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
     * @brief    Compute the interpolation
     * @param    float input --  input value [0,1]
     * @return   float -- output interpolation result
     * Describe
     */
    STDMETHOD_(float, getInterpolation)(CTHIS_ float input) SCONST PURE;
};
SNSEND
#endif /**< __SINTERPOLATOR_I__H__ */