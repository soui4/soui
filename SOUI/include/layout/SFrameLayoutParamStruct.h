#ifndef __SFRAMELAYOUTPARAMSTRUCT__H__
#define __SFRAMELAYOUTPARAMSTRUCT__H__

#include <core/SLayoutSize.h>
#include <layout/SLinearLayoutParamStruct.h>
SNSBEGIN

typedef enum _DockPosition
{
    DockNone = 0,
    DockLeft,
    DockTop,
    DockRight,
    DockBottom,
    DockMainView
} DockPosition;

typedef enum _DockMode
{
    DockMode_None = 0,
    DockMode_Left = 1 << 0,
    DockMode_Top = 1 << 1,
    DockMode_Right = 1 << 2,
    DockMode_Bottom = 1 << 3,
    DockMode_Any = DockMode_Left | DockMode_Top | DockMode_Right | DockMode_Bottom
} DockMode;

struct SFrameLayoutParamStruct
{
    DockPosition dockPos;
    SLayoutSize width;
    SLayoutSize height;
    SLayoutSize extend_left, extend_right;
    SLayoutSize extend_top, extend_bottom;
    SStringW strDockRelativeTo;
    float weight;
    Gravity gravity;
};

SNSEND
#endif
