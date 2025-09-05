#ifndef __SLINEARLAYOUT__H__
#define __SLINEARLAYOUT__H__

#include <interface/slayout-i.h>
#include <layout/SLinearLayoutParamStruct.h>
#include <sobject/Sobject.hpp>

SNSBEGIN

/**
 * @class SLinearLayoutParam
 * @brief 线性布局参数类
 */
class SLinearLayoutParam
    : public TObjRefImpl<SObjectImpl<ILayoutParam> >
    , protected SLinearLayoutParamStruct {
    DEF_SOBJECT(SObjectImpl<ILayoutParam>, L"LinearLayoutParam")

    friend class SLinearLayout;

  public:
    /**
     * @brief 构造函数
     */
    SLinearLayoutParam();

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
    STDMETHOD_(SLayoutSize, GetSpecifiedSize)(THIS_ ORIENTATION orientation) SCONST OVERRIDE;

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
    STDMETHOD_(void, SetSpecifiedSize)
    (THIS_ ORIENTATION orientation, const SLayoutSize &layoutSize) OVERRIDE;

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

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"width", OnAttrWidth)                 // 宽度
        ATTR_CUSTOM(L"height", OnAttrHeight)               // 高度
        ATTR_CUSTOM(L"size", OnAttrSize)                   // 大小
        ATTR_FLOAT(L"weight", weight, FALSE)               // 权重
        ATTR_ENUM_BEGIN(L"layout_gravity", Gravity, FALSE) // 对齐方式
            ATTR_ENUM_VALUE(L"left", G_Left)               // 左对齐
            ATTR_ENUM_VALUE(L"top", G_Top)                 // 顶部对齐
            ATTR_ENUM_VALUE(L"center", G_Center)           // 居中对齐
            ATTR_ENUM_VALUE(L"right", G_Right)             // 右对齐
            ATTR_ENUM_VALUE(L"bottom", G_Bottom)           // 底部对齐
        ATTR_ENUM_END(gravity)
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
 * @class SLinearLayout
 * @brief 线性布局类
 */
class SOUI_EXP SLinearLayout : public TObjRefImpl<SObjectImpl<ILayout> > {
    DEF_SOBJECT(SObjectImpl<ILayout>, L"linearLayout")

  public:
    /**
     * @brief 构造函数
     */
    SLinearLayout(void);

    /**
     * @brief 析构函数
     */
    ~SLinearLayout(void);

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
     * @return 布局参数对象指针
     */
    STDMETHOD_(ILayoutParam *, CreateLayoutParam)(THIS) SCONST OVERRIDE;

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
            ATTR_ENUM_VALUE(L"horizontal", Horz)            // 水平方向
            ATTR_ENUM_VALUE(L"vertical", Vert)              // 垂直方向
        ATTR_ENUM_END(m_orientation)
        ATTR_ENUM_BEGIN(L"gravity", Gravity, FALSE) // 对齐方式
            ATTR_ENUM_VALUE(L"left", G_Left)        // 左对齐
            ATTR_ENUM_VALUE(L"top", G_Top)          // 顶部对齐
            ATTR_ENUM_VALUE(L"center", G_Center)    // 居中对齐
            ATTR_ENUM_VALUE(L"right", G_Right)      // 右对齐
            ATTR_ENUM_VALUE(L"bottom", G_Bottom)    // 底部对齐
        ATTR_ENUM_END(m_gravity)
        ATTR_LAYOUTSIZE(L"interval", m_interval, FALSE) // 间距
    SOUI_ATTRS_BREAK()

  protected:
    ORIENTATION m_orientation; // 方向
    Gravity m_gravity;         // 对齐方式
    SLayoutSize m_interval;    // 间距
};

/**
 * @class SVBox
 * @brief 垂直线性布局类
 */
class SVBox : public SLinearLayout {
    DEF_SOBJECT(SLinearLayout, L"vbox")

  public:
    /**
     * @brief 构造函数
     */
    SVBox()
    {
        m_orientation = Vert; // 设置方向为垂直
    }
};

/**
 * @class SHBox
 * @brief 水平线性布局类
 */
class SHBox : public SLinearLayout {
    DEF_SOBJECT(SLinearLayout, L"hbox")

  public:
    /**
     * @brief 构造函数
     */
    SHBox()
    {
        m_orientation = Horz; // 设置方向为水平
    }
};

SNSEND

#endif // __SLINEARLAYOUT__H__