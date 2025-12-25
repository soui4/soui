/**
 * @file SPropertyAnimator-i.h
 * @brief SOUI属性动画接口定义 - 参考Android属性动画系统设计
 * @author SOUI group
 * @date 2025
 */

#ifndef __SPROPERTY_AMINATOR_I__H__
#define __SPROPERTY_AMINATOR_I__H__

#include <interface/obj-ref-i.h>


SNSBEGIN

typedef struct IWindow IWindow;
typedef struct _LAYOUTSIZE LAYOUTSIZE;

typedef enum _PROP_TYPE
{
    PROP_TYPE_UNKNOWN = 0,
    PROP_TYPE_BYTE,
    PROP_TYPE_SHORT,
    PROP_TYPE_COLORREF,
    PROP_TYPE_INT,
    PROP_TYPE_FLOAT,
    PROP_TYPE_LAYOUT_SIZE,
    PROP_TYPE_VARIANT,
} PROP_TYPE;

/**
 * @interface IPropertyValuesHolder
 * @brief 属性值持有者接口 - 类似Android PropertyValuesHolder
 */
#undef INTERFACE
#define INTERFACE IPropertyValuesHolder
DECLARE_INTERFACE_(IPropertyValuesHolder, IObjRef)
{
    /**
     * @brief 设置属性名称
     * @param propertyName 属性名称
     */
    STDMETHOD_(void, SetPropertyName)(THIS_ LPCWSTR propertyName) PURE;

    /**
     * @brief 获取属性名称
     * @return 属性名称
     */
    STDMETHOD_(LPCWSTR, GetPropertyName)(CTHIS) SCONST PURE;

    /**
     * @brief 设置字节值
     * @param values 值数组
     * @param count 值数量
     */
    STDMETHOD_(void, SetByteValues)(THIS_ const BYTE *values, int count) PURE;

    /**
     * @brief 设置短整型值
     * @param values 值数组
     * @param count 值数量
     */
    STDMETHOD_(void, SetShortValues)(THIS_ const short *values, int count) PURE;

    /**
     * @brief 颜色值
     * @param values 值数组
     * @param count 值数量
     */
    STDMETHOD_(void, SetColorRefValues)(THIS_ const COLORREF *values, int count) PURE;

    /**
     * @brief 设置浮点数值
     * @param values 值数组
     * @param count 值数量
     */
    STDMETHOD_(void, SetFloatValues)(THIS_ const float *values, int count) PURE;

    /**
     * @brief 设置整数值
     * @param values 值数组
     * @param count 值数量
     */
    STDMETHOD_(void, SetIntValues)(THIS_ const int *values, int count) PURE;

    /**
     * @brief 设置LAYOUTSIZE值
     * @param values 值数组
     * @param count 值数量
     */
    STDMETHOD_(void, SetLayoutSizeValues)(THIS_ const LAYOUTSIZE *values, int count) PURE;

    /**
     * @brief 设置位置值（用于位置相关属性）
     * @param values 值数组（具体类型依赖于布局类型）
     * @param count 值数量
     * @param valueSize 每个值的大小（字节）
     */
    STDMETHOD_(void, SetPositionValues)(THIS_ const void *values, int count, int valueSize) PURE;

    /**
     * @brief 获取当前动画值
     * @param fraction 动画进度(0.0-1.0)
     * @param pValue 输出值指针
     * @return 成功返回TRUE
     */
    STDMETHOD_(BOOL, GetAnimatedValue)(CTHIS_ float fraction, void *pValue) SCONST PURE;

    /**
     * @brief 获取值类型
     * @return 值类型
     */
    STDMETHOD_(PROP_TYPE, GetValueType)(CTHIS) SCONST PURE;

    /**
     * @brief 获取值占用内存大小
     * @return 值数量
     */
    STDMETHOD_(int, GetValueSize)(CTHIS) SCONST PURE;

    /**
     * @brief 获取起始值
     * @param pValue 输出值指针
     */
    STDMETHOD_(void, GetStartValue)(CTHIS_ void *pValue) SCONST PURE;

    /**
     * @brief 获取结束值
     * @param pValue 输出值指针
     */
    STDMETHOD_(void, GetEndValue)(CTHIS_ void *pValue) SCONST PURE;

    /**
     * @brief 将动画进度转换为索引
     * @param fraction 动画进度(0.0-1.0)
     * @param idx 索引值数组（输出）
     * @return 分段内动画进度（0.0-1.0）
     */
    STDMETHOD_(float, Fraction2Index)(CTHIS_ float fraction, int idx[2]) SCONST PURE;

    /**
     * @brief 通过索引获取值
     * @param index 索引值
     * @param pValue 输出值指针
     * @param valueSize 值大小
     * @return 成功返回TRUE
     */
    STDMETHOD_(BOOL, GetValueByIndex)(CTHIS_ int index, void *pValue, int valueSize) SCONST PURE;

    /**
     * @brief 获取关键帧数量
     * @return 关键帧数量
     */
    STDMETHOD_(int, GetKeyframeCount)(CTHIS) SCONST PURE;

    /**
     * @brief 设置关键帧权重
     * @param weights 权重数组
     * @param count 权重数量
     * @return 成功返回TRUE,失败返回FALSE(权重数量与关键帧数量不一致)
     * @note 权重数量必须与关键帧数量一致,不设置权重则默认为1.0
     */
    STDMETHOD_(BOOL, SetKeyFrameWeights)(THIS_ const float *weights, int count) PURE;

    /**
     * @brief 获取关键帧权重
     * @param weights 权重数组
     * @param count 权重数量
     * @return 成功返回TRUE,失败返回FALSE(权重数量与关键帧数量不一致)
     */
    STDMETHOD_(BOOL, GetKeyFrameWeights)(CTHIS_ float *weights, int count) SCONST PURE;
};


DECLARE_INTERFACE_(IPropertyAnimator, IValueAnimator)
{ 
    /**
     * @brief 获取动画目标
     * @return 动画目标
     */
    STDMETHOD_(IWindow *,GetTarget)(CTHIS) SCONST PURE;

    /**
     * @brief 设置属性值持有者
     * @param pHolder 属性值持有者
     */
    STDMETHOD_(void,SetPropertyValuesHolder)(THIS_ IPropertyValuesHolder *pHolder) PURE;
    /**
     * @brief 设置属性值持有者数组
     * @param pHolders 属性值持有者数组
     * @param count 属性值持有者数量
     */
    STDMETHOD_(void,SetPropertyValuesHolders)(THIS_ IPropertyValuesHolder **pHolders, int count) PURE; 
    /**
     * @brief 获取属性值持有者
     * @param propertyName 属性名称
     * @return 属性值持有者
     */
    STDMETHOD_(IPropertyValuesHolder *,GetPropertyValuesHolderByName)(CTHIS_ LPCWSTR propertyName) SCONST PURE;

    /**
     * @brief 获取属性值持有者
     * @param index 索引
     * @return 属性值持有者
     */
    STDMETHOD_(IPropertyValuesHolder *,GetPropertyValuesHolderByIndex)(CTHIS_ int index) SCONST PURE;

    /**
     * @brief 获取属性值持有者数量
     * @return 属性值持有者数量
     */
    STDMETHOD_(int,GetPropertyValuesHolderCount)(CTHIS) SCONST PURE;
};

SNSEND

#endif // __SPROPERTY_AMINATOR_I__H__
