#ifndef __SANILAYOUTPARAMSTRUCT__H__
#define __SANILAYOUTPARAMSTRUCT__H__

#include <layout/SLayoutSize.h>
#include <interface/SValueAnimator-i.h>

SNSBEGIN

enum AnchorPosType
{
    APT_Invalid=-1, /**<无效*/
    APT_Left_Top=0, /**<左上角*/
    APT_Center_Top, /**<上中*/
    APT_Right_Top, /**<右上角*/

    APT_Left_Center, /**<左中*/
    APT_Center_Center, /**<中中*/
    APT_Right_Center, /**<右中*/

    APT_Left_Bottom, /**<左下角*/
    APT_Center_Bottom, /**<下中*/
    APT_Right_Bottom, /**<右下角*/
};

struct AnchorPos
{
    int type;
    SLayoutSize x,y; /**<坐标值*/
    float fOffsetX, fOffsetY; /**< 窗口坐标偏移量, x += fOffsetX * width, y += fOffsetY * height  */
};

struct SAnchorLayoutParamStruct
{
    AnchorPos pos;
    SLayoutSize width;  /**<使用width属性定义的宽 nCount==0 时有效*/
    SLayoutSize height; /**<使用height属性定义的高 nCount==0 时有效*/
};
SNSEND

#endif // __SANILAYOUTPARAMSTRUCT__H__