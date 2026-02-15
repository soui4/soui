#ifndef __SFLOWLAYOUT__H__
#define __SFLOWLAYOUT__H__

#include <interface/slayout-i.h>
#include <layout/SFlowLayoutParamStruct.h>
#include <layout/SLinearLayout.h>
#include <sobject/Sobject.hpp>
SNSBEGIN

/**
 * @class SFlowLayoutParam
 * @brief 流式布局参数类
 */
class SOUI_EXP SFlowLayoutParam
    : public TObjRefImpl<SObjectImpl<ILayoutParam>>
    , public SFlowLayoutParamStruct {
    DEF_SOBJECT(SObjectImpl<ILayoutParam>, L"FlowLayoutParam")

    friend class SFlowLayout;

  public:
    /**
     * @brief 构造函数
     */
    SFlowLayoutParam();

    /**
     * @brief 清除布局参数
     */
    STDMETHOD_(void, Clear)(THIS) OVERRIDE;

    /**
     * @brief 检查是否匹配父容器大小
     * @param orientation 方向（水平或垂直）
     * @return 如果匹配父容器大小返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, IsMatchParent)(THIS_ ORIENTATION orientation) SCONST OVERRIDE;

    /**
     * @brief 检查是否包裹内容大小
     * @param orientation 方向（水平或垂直）
     * @return 如果包裹内容大小返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, IsWrapContent)(THIS_ ORIENTATION orientation) SCONST OVERRIDE;

    /**
     * @brief 检查是否指定大小
     * @param orientation 方向（水平或垂直）
     * @return 如果指定大小返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, IsSpecifiedSize)(THIS_ ORIENTATION orientation) SCONST OVERRIDE;

    /**
     * @brief 获取指定大小
     * @param orientation 方向（水平或垂直）
     * @return 指定大小
     */
    STDMETHOD_(BOOL, GetSpecifiedSize)(THIS_ ORIENTATION orientation, LAYOUTSIZE *pLayoutSize) SCONST OVERRIDE;

    /**
     * @brief 设置匹配父容器大小
     * @param orientation 方向（水平或垂直）
     */
    STDMETHOD_(void, SetMatchParent)(THIS_ ORIENTATION orientation) OVERRIDE;

    /**
     * @brief 设置包裹内容大小
     * @param orientation 方向（水平或垂直）
     */
    STDMETHOD_(void, SetWrapContent)(THIS_ ORIENTATION orientation) OVERRIDE;

    /**
     * @brief 设置指定大小
     * @param orientation 方向（水平或垂直）
     * @param layoutSize 指定大小
     */
    STDMETHOD_(void, SetSpecifiedSize)(THIS_ ORIENTATION orientation, const LAYOUTSIZE *pLayoutSize) OVERRIDE;

    /**
     * @brief 获取原始数据指针
     * @return 原始数据指针
     */
    STDMETHOD_(void *, GetRawData)(THIS) OVERRIDE;

    /**
     * @brief 克隆布局参数
     * @return 克隆的布局参数对象指针
     */
    STDMETHOD_(ILayoutParam *, Clone)(THIS) SCONST OVERRIDE;

    /**
     * @brief 更新属性动画器状态
     * @param pHolder IPropertyValuesHolder*--属性值持有者
     * @param fraction float--动画进度（0.0-1.0）
     * @param state ANI_STATE--动画状态（ANI_START/ANI_PROGRESS/ANI_END）
     */
    STDMETHOD_(BOOL, SetAnimatorValue)(THIS_ IPropertyValuesHolder *pHolder, float fraction, ANI_STATE state) OVERRIDE;

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"width", OnAttrWidth)                      // 宽度
        ATTR_CUSTOM(L"height", OnAttrHeight)                    // 高度
        ATTR_CUSTOM(L"size", OnAttrSize)                        // 大小
        ATTR_GRAVITY(L"layout_gravity", gravity, FALSE)         // 对齐方式
        ATTR_CUSTOM(L"extend", OnAttrExtend)                    // 扩展属性
        ATTR_LAYOUTSIZE(L"extend_left", extend_left, FALSE)     // 左侧扩展
        ATTR_LAYOUTSIZE(L"extend_top", extend_top, FALSE)       // 顶部扩展
        ATTR_LAYOUTSIZE(L"extend_right", extend_right, FALSE)   // 右侧扩展
        ATTR_LAYOUTSIZE(L"extend_bottom", extend_bottom, FALSE) // 底部扩展
    SOUI_ATTRS_BREAK()

  protected:
    /**
     * @brief 处理大小属性
     * @param strValue 属性值字符串
     * @param bLoading 是否正在加载
     * @return 处理结果
     */
    HRESULT OnAttrSize(const SStringW &strValue, BOOL bLoading);

    /**
     * @brief 处理宽度属性
     * @param strValue 属性值字符串
     * @param bLoading 是否正在加载
     * @return 处理结果
     */
    HRESULT OnAttrWidth(const SStringW &strValue, BOOL bLoading);

    /**
     * @brief 处理高度属性
     * @param strValue 属性值字符串
     * @param bLoading 是否正在加载
     * @return 处理结果
     */
    HRESULT OnAttrHeight(const SStringW &strValue, BOOL bLoading);

    /**
     * @brief 处理扩展属性
     * @param strValue 属性值字符串
     * @param bLoading 是否正在加载
     * @return 处理结果
     */
    HRESULT OnAttrExtend(const SStringW &strValue, BOOL bLoading);
};

/**
 * @class SFlowLayout
 * @brief 流式布局类 - 支持水平和垂直两种流式布局方式
 *
 * 水平方向：元素从左到右排列，当达到容器宽度时换行到下一行
 * 垂直方向：元素从上到下排列，当达到容器高度时换到下一列
 */
class SOUI_EXP SFlowLayout : public TObjRefImpl<SObjectImpl<ILayout>> {
    DEF_SOBJECT_EX(SObjectImpl<ILayout>, L"flowLayout", L"flow")

  public:
    /**
     * @brief 构造函数
     */
    SFlowLayout(void);

    /**
     * @brief 析构函数
     */
    ~SFlowLayout(void);

    /**
     * @brief 检查布局参数是否可接受
     * @param pLayoutParam 布局参数对象指针
     * @return 如果可接受返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, IsParamAcceptable)(THIS_ const ILayoutParam *pLayoutParam) SCONST OVERRIDE;

    /**
     * @brief 布局子窗口
     * @param pParent 父窗口指针
     */
    STDMETHOD_(void, LayoutChildren)(THIS_ IWindow *pParent) OVERRIDE;

    /**
     * @brief 创建布局参数对象
     * @param pOwner IWindow*--布局参数的拥有者窗口对象
     * @return 布局参数对象指针
     */
    STDMETHOD_(ILayoutParam *, CreateLayoutParam)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief 测量子窗口大小
     * @param pParent 父窗口指针
     * @param nWidth 宽度
     * @param nHeight 高度
     * @return 测量结果大小
     */
    STDMETHOD_(SIZE, MeasureChildren)
    (THIS_ const IWindow *pParent, int nWidth, int nHeight) SCONST OVERRIDE;

    SOUI_ATTRS_BEGIN()
        ATTR_ENUM_BEGIN(L"orientation", ORIENTATION, FALSE) // 方向
            ATTR_ENUM_VALUE(L"horizontal", Horz)            // 水平方向：从左到右排列，超宽时换行
            ATTR_ENUM_VALUE(L"vertical", Vert)              // 垂直方向：从上到下排列，超高时换列
        ATTR_ENUM_END(m_orientation)
        ATTR_GRAVITY(L"gravity", m_gravity, FALSE)                    // 对齐方式
        ATTR_LAYOUTSIZE(L"xInterval", m_xInterval, FALSE)             // 水平间距
        ATTR_LAYOUTSIZE(L"yInterval", m_yInterval, FALSE)             // 垂直间距
        ATTR_LAYOUTSIZE(L"interval", m_xInterval = m_yInterval, TRUE) // 间距（水平和垂直相同）
    SOUI_ATTRS_BREAK()

  protected:
    ORIENTATION m_orientation; // 方向：水平或垂直
    Gravity m_gravity;         // 对齐方式
    SLayoutSize m_xInterval;   // 水平间距
    SLayoutSize m_yInterval;   // 垂直间距
};

/**
 * @class SHFlowBox
 * @brief 水平流式布局类 - 元素从左到右排列，超宽时换行
 */
class SOUI_EXP SHFlowBox : public SFlowLayout {
    DEF_SOBJECT(SFlowLayout, L"hflow")

  public:
    /**
     * @brief 构造函数
     */
    SHFlowBox()
    {
        m_orientation = Horz; // 设置方向为水平
    }
};

/**
 * @class SVFlowBox
 * @brief 垂直流式布局类 - 元素从上到下排列，超高时换列
 */
class SOUI_EXP SVFlowBox : public SFlowLayout {
    DEF_SOBJECT(SFlowLayout, L"vflow")

  public:
    /**
     * @brief 构造函数
     */
    SVFlowBox()
    {
        m_orientation = Vert; // 设置方向为垂直
    }
};

SNSEND

#endif // __SFLOWLAYOUT__H__
