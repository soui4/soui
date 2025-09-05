#ifndef __SGRIDLAYOUT__H__
#define __SGRIDLAYOUT__H__

#include <interface/slayout-i.h>
#include <layout/SGridLayoutParamStruct.h>
#include <sobject/Sobject.hpp>

SNSBEGIN

// Int = %d StringA
#define ATTR_GRIDGRAVITY(attribname, varname, allredraw)        \
    if (0 == strAttribName.CompareNoCase(attribname))           \
    {                                                           \
        varname = SGridLayoutParam::parseGridGravity(strValue); \
        hRet = allredraw ? S_OK : S_FALSE;                      \
    }                                                           \
    else

/**
 * @class SGridLayoutParam
 * @brief 网格布局参数类
 */
class SGridLayoutParam
    : public TObjRefImpl<SObjectImpl<ILayoutParam> >
    , protected SGridLayoutParamStruct {
    DEF_SOBJECT(TObjRefImpl<SObjectImpl<ILayoutParam> >, L"GridLayoutParam")

    friend class SGridLayout;

  public:
    /**
     * @brief 解析网格对齐方式
     * @param strValue 对齐方式字符串
     * @return 解析后的对齐方式枚举值
     */
    static GridGravity parseGridGravity(const SStringW &strValue);

    /**
     * @brief 构造函数
     */
    SGridLayoutParam();

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
        ATTR_INT(L"rowSpan", nRowSpan, TRUE)                                       // 行跨越数
        ATTR_INT(L"columnSpan", nColSpan, TRUE)                                    // 列跨越数
        ATTR_CUSTOM(L"width", OnAttrWidth)                                         // 宽度
        ATTR_CUSTOM(L"height", OnAttrHeight)                                       // 高度
        ATTR_CUSTOM(L"size", OnAttrSize)                                           // 大小
        ATTR_GRIDGRAVITY(L"layout_xGravity", layoutGravityX, TRUE)                 // 水平对齐方式
        ATTR_GRIDGRAVITY(L"layout_yGravity", layoutGravityY, TRUE)                 // 垂直对齐方式
        ATTR_GRIDGRAVITY(L"layout_gravity", layoutGravityX = layoutGravityY, TRUE) // 对齐方式（水平和垂直相同）
        ATTR_FLOAT(L"columnWeight", fColWeight, TRUE)                              // 列权重
        ATTR_FLOAT(L"rowWeight", fRowWeight, TRUE)                                 // 行权重
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
};

/**
 * @class SGridLayout
 * @brief 网格布局类
 */
class SOUI_EXP SGridLayout : public TObjRefImpl<SObjectImpl<ILayout> > {
    DEF_SOBJECT_EX(SObjectImpl<ILayout>, L"gridLayout", L"grid")

  public:
    /**
     * @brief 构造函数
     */
    SGridLayout(void);

    /**
     * @brief 析构函数
     */
    ~SGridLayout(void);

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
        ATTR_INT(L"columnCount", m_nCols, TRUE)                       // 列数
        ATTR_INT(L"rowCount", m_nRows, TRUE)                          // 行数
        ATTR_LAYOUTSIZE(L"xInterval", m_xInterval, TRUE)              // 水平间距
        ATTR_LAYOUTSIZE(L"yInterval", m_yInterval, TRUE)              // 垂直间距
        ATTR_LAYOUTSIZE(L"interval", m_xInterval = m_yInterval, TRUE) // 间距（水平和垂直相同）
        ATTR_GRIDGRAVITY(L"xGravity", m_GravityX, TRUE)               // 水平对齐方式
        ATTR_GRIDGRAVITY(L"yGravity", m_GravityY, TRUE)               // 垂直对齐方式
        ATTR_GRIDGRAVITY(L"gravity", m_GravityX = m_GravityY, TRUE)   // 对齐方式（水平和垂直相同）
    SOUI_ATTRS_BREAK()

  protected:
    /**
     * @brief 计算网格单元格数量
     * @param pParent 父窗口指针
     * @return 网格单元格数量
     */
    int CalcCells(const IWindow *pParent) const;

    int m_nCols;             /**< 列数 */
    int m_nRows;             /**< 行数 */
    SLayoutSize m_xInterval; /**< 水平间距 */
    SLayoutSize m_yInterval; /**< 垂直间距 */

    GridGravity m_GravityX; /**< 水平对齐方式 */
    GridGravity m_GravityY; /**< 垂直对齐方式 */
};

SNSEND

#endif // __SGRIDLAYOUT__H__