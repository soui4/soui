#ifndef __SGRIDLAYOUTPARAMSTRUCT__H__
#define __SGRIDLAYOUTPARAMSTRUCT__H__

#include <core/SLayoutSize.h>

SNSBEGIN

enum GridGravity
{
    gUndef = -1, /**<Undefined*/
    gLeft = 0,   /**<Left align*/
    gTop = 0,    /**<Top align*/
    gCenter = 1, /**<Center align*/
    gRight = 2,  /**<Right align*/
    gBottom = 2, /**<Bottom align*/
    gFill = 3,   /**<Expand align*/
};

/**
 * Grid layout cell parameters
 * remark:
 * If multiple cells in the same row specify rowWeight, only the largest one takes effect; correspondingly, in the same column only the largest colWeight takes effect
 */
struct SGridLayoutParamStruct
{
    int nColSpan;               /**<Grid column span*/
    int nRowSpan;               /**<Grid row span*/
    GridGravity layoutGravityX; /**<Grid horizontal alignment*/
    GridGravity layoutGravityY; /**<Grid vertical alignment*/
    SLayoutSize width;          /**<Grid width*/
    SLayoutSize height;         /**<Grid height*/
    float fColWeight;           /**<Weight of grid width*/
    float fRowWeight;           /**<Weight of grid height*/
};

SNSEND
#endif /**< __SGRIDLAYOUTPARAMSTRUCT__H__ */