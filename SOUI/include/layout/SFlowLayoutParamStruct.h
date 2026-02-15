#ifndef __SFLOWLAYOUTPARAMSTRUCT__H__
#define __SFLOWLAYOUTPARAMSTRUCT__H__

#include <layout/SLayoutSize.h>
#include <layout/SLinearLayoutParamStruct.h>
SNSBEGIN

/**
 * @struct SFlowLayoutParamStruct
 * @brief 流式布局参数结构体
 */
struct SFlowLayoutParamStruct
{
    SLayoutSize width;         // 宽度
    SLayoutSize height;        // 高度
    SLayoutSize extend_left;   // 左侧扩展
    SLayoutSize extend_top;    // 顶部扩展
    SLayoutSize extend_right;  // 右侧扩展
    SLayoutSize extend_bottom; // 底部扩展
    Gravity gravity;           // 对齐方式

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

#endif // __SFLOWLAYOUTPARAMSTRUCT__H__
