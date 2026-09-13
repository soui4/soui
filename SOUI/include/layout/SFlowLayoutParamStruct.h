#ifndef __SFLOWLAYOUTPARAMSTRUCT__H__
#define __SFLOWLAYOUTPARAMSTRUCT__H__

#include <core/SLayoutSize.h>
#include <layout/SLinearLayoutParamStruct.h>
SNSBEGIN

/**
 * @struct SFlowLayoutParamStruct
 * @brief Flow layout parameter struct
 */
struct SFlowLayoutParamStruct
{
    SLayoutSize width;         /**< Width */
    SLayoutSize height;        /**< Height */
    SLayoutSize extend_left;   /**< Left expansion */
    SLayoutSize extend_top;    /**< Top expansion */
    SLayoutSize extend_right;  /**< Right expansion */
    SLayoutSize extend_bottom; /**< Bottom expansion */
    Gravity gravity;           /**< Alignment */

    SFlowLayoutParamStruct()
    {
        width.setWrapContent();
        height.setWrapContent();
        extend_left.setSize(0, dp);
        extend_top.setSize(0, dp);
        extend_right.setSize(0, dp);
        extend_bottom.setSize(0, dp);
        gravity = G_Undefined;
    }
};

SNSEND

#endif /**< __SFLOWLAYOUTPARAMSTRUCT__H__ */
