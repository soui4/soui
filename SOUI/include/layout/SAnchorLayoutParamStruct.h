#ifndef __SANILAYOUTPARAMSTRUCT__H__
#define __SANILAYOUTPARAMSTRUCT__H__

#include <core/SLayoutSize.h>
#include <interface/SValueAnimator-i.h>

SNSBEGIN

enum AnchorPosType
{
    APT_Invalid = -1, /**<Invalid*/
    APT_Left_Top = 0, /**<Top-left corner*/
    APT_Center_Top,   /**<Top-center*/
    APT_Right_Top,    /**<Top-right corner*/

    APT_Left_Center,   /**<Middle-left*/
    APT_Center_Center, /**<Center*/
    APT_Right_Center,  /**<Middle-right*/

    APT_Left_Bottom,   /**<Bottom-left corner*/
    APT_Center_Bottom, /**<Bottom-center*/
    APT_Right_Bottom,  /**<Bottom-right corner*/
};

struct AnchorPos
{
    int type;
    SLayoutSize x, y;         /**<Coordinate value*/
    float fOffsetX, fOffsetY; /**< Window coordinate offset, x += fOffsetX * width, y += fOffsetY * height  */
};

struct SAnchorLayoutParamStruct
{
    AnchorPos pos;
    SLayoutSize width;  /**<Width defined by the width attribute, valid when nCount==0*/
    SLayoutSize height; /**<Height defined by the height attribute, valid when nCount==0*/
};
SNSEND

#endif /**< __SANILAYOUTPARAMSTRUCT__H__ */