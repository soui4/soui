#ifndef __SLINEARLAYOUTPARAMSTRUCT__H__
#define __SLINEARLAYOUTPARAMSTRUCT__H__

#include <core/SLayoutSize.h>
SNSBEGIN
enum Gravity
{
    G_Undefined = -1,
    G_Left = 0,
    G_Top = 0,
    G_Center = 1,
    G_Right = 2,
    G_Bottom = 2,
};

struct SLinearLayoutParamStruct
{
    SLayoutSize width;
    SLayoutSize height;
    SLayoutSize extend_left, extend_right; /**< Equivalent to Android's margin attribute */
    SLayoutSize extend_top, extend_bottom; /**< Equivalent to Android's margin attribute */
    float weight;
    Gravity gravity;
};
SNSEND
#endif /**< __SLINEARLAYOUTPARAMSTRUCT__H__ */