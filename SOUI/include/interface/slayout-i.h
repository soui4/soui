/*
SOUI窗口布局接口
*/
#ifndef __SLAYOUT_I__H__
#define __SLAYOUT_I__H__

#include <interface/sobject-i.h>
#include <interface/sproperty-values-holder-i.h>
SNSBEGIN

typedef struct IWindow IWindow;

typedef enum ORIENTATION
{
    Horz,
    Vert,
    Any,
    Both,
} ORIENTATION;

enum
{
    SIZE_UNDEF = -3,
    SIZE_WRAP_CONTENT = -1,
    SIZE_MATCH_PARENT = -2,
    SIZE_SPEC = 0,
};

/**
 * @enum Unit
 * @brief 布局大小单位枚举
 */
typedef enum _Unit
{
    unknow = -1, // 未知单位
    px = 0,      // 像素
    dp,          // 设备独立像素
    dip = dp,    // 设备独立像素（别名）
    sp           // 缩放像素
}Unit;

typedef struct _LAYOUTSIZE
{
    float fSize; // 大小值
    Unit unit;   // 大小单位
}LAYOUTSIZE;


/**
 * @brief 动画状态枚举
 */
typedef enum _ANI_STATE{
    ANI_START=0,    /**< 动画开始状态 */
    ANI_PROGRESS,   /**< 动画进行中状态 */
    ANI_END,        /**< 动画结束状态 */
}ANI_STATE;



#undef INTERFACE
#define INTERFACE ILayoutParam
DECLARE_INTERFACE_(ILayoutParam, IObject)
{
    DEF_OBJ_BASE(ILayoutParam, LayoutParam)
#include <interface/SobjectApi.h>

    //----------------------------------------------------------

    /**
     * @brief 清空数据
     * @return
     */
    STDMETHOD_(void, Clear)(THIS) PURE;

    /**
     * @brief 布局充满父窗口标志
     * @param orientation ORIENTATION--布局方向
     * @return TRUE--布局充满父窗口
     */
    STDMETHOD_(BOOL, IsMatchParent)(CTHIS_ ORIENTATION orientation) SCONST PURE;

    /**
     * @brief 布局适应窗口内容标志
     * @param orientation ORIENTATION--布局方向
     * @return TRUE--适应窗口内容标志
     */
    STDMETHOD_(BOOL, IsWrapContent)(CTHIS_ ORIENTATION orientation) SCONST PURE;

    /**
     * @brief 布局指定大小标志
     * @param orientation ORIENTATION--布局方向
     * @return TRUE--指定大小
     */
    STDMETHOD_(BOOL, IsSpecifiedSize)(CTHIS_ ORIENTATION orientation) SCONST PURE;

    /**
     * @brief 获取指定的布局大小
     * @param orientation ORIENTATION--布局方向
     * @return SLayoutSize--布局大小
     */
    STDMETHOD_(BOOL, GetSpecifiedSize)(CTHIS_ ORIENTATION orientation, LAYOUTSIZE *pLayoutSize) SCONST PURE;

    /**
     * @brief 设定布局大小
     * @param orientation ORIENTATION--布局方向
     * @param layoutSize SLayoutSize--布局大小
     * @return
     */
    STDMETHOD_(void, SetSpecifiedSize)(THIS_ ORIENTATION orientation, const LAYOUTSIZE *layoutSize) PURE;

    /**
     * @brief 设定布局适应父窗口大小
     * @param orientation ORIENTATION--布局方向
     * @return
     */
    STDMETHOD_(void, SetMatchParent)(THIS_ ORIENTATION orientation) PURE;

    /**
     * @brief 设定布局适应内容
     * @param orientation ORIENTATION--布局方向
     * @return
     */
    STDMETHOD_(void, SetWrapContent)(THIS_ ORIENTATION orientation) PURE;

    /**
     * @brief 更新属性动画器状态
     * @param pHolder IPropertyValuesHolder*--属性值持有者
     * @param fraction float--动画进度（0.0-1.0）
     * @param state ANI_STATE--动画状态（ANI_START/ANI_PROGRESS/ANI_END）
     * @note 此方法在动画过程中被调用，用于处理动画状态变化和触发布局更新
     */
    STDMETHOD_(BOOL, SetAnimatorValue)(THIS_ IPropertyValuesHolder *pHolder, float fraction, ANI_STATE state) PURE;

    /**
     * @brief 获取布局结构体数据
     * @return void*布局结构体
     */
    STDMETHOD_(void *, GetRawData)(THIS) PURE;

    /**
     * @brief Clone布局参数
     * @return ILayoutParam *--布局参数对象
     */
    STDMETHOD_(ILayoutParam *, Clone)(CTHIS) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE ILayout
DECLARE_INTERFACE_(ILayout, IObject)
{
    DEF_OBJ_BASE(ILayout, Layout)
#include <interface/SobjectApi.h>
    //------------------------------------------------------------------------

    /**
     * @brief 判断当前布局类型和布局参数是否匹配
     * @param pLayoutParam const ILayoutParam *--布局参数
     * @return TRUE--匹配
     */
    STDMETHOD_(BOOL, IsParamAcceptable)(CTHIS_ const ILayoutParam *pLayoutParam) SCONST PURE;

    /**
     * @brief 布局指定控件的子窗口
     * @param pParent IWindow *--布局控件
     * @return
     */
    STDMETHOD_(void, LayoutChildren)(THIS_ IWindow * pParent) PURE;

    /**
     * @brief 创建和当前布局匹配的布局参数对象
     * @return ILayoutParam *--布局参数对象
     */
    STDMETHOD_(ILayoutParam *, CreateLayoutParam)(CTHIS) SCONST PURE;

    /**
     * @brief 计算窗口大小
     * @param pParent const IWindow*--目标窗口
     * @param nWidth int--父窗口宽度
     * @param nHeight int--父窗口高度
     * @return SIZE--目标窗口大小
     */
    STDMETHOD_(SIZE, MeasureChildren)
    (CTHIS_ const IWindow *pParent, int nWidth, int nHeight) SCONST PURE;
};

SNSEND
#endif // __SLAYOUT_I__H__