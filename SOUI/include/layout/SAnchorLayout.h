#ifndef __SANILAYOUT__H__
#define __SANILAYOUT__H__

#include <interface/slayout-i.h>
#include <layout/SAnchorLayoutParamStruct.h>
#include <sobject/Sobject.hpp>

SNSBEGIN

/**
 * @class SAnchorLayoutParam
 * @brief 9锚点布局参数类
 */
class SOUI_EXP SAnchorLayoutParam
    : public TObjRefImpl<SObjectImpl<ILayoutParam> >
    , protected SAnchorLayoutParamStruct {
    DEF_SOBJECT(SObjectImpl<ILayoutParam>, L"AnchorLayoutParam")

    friend class SAnchorLayout;

    SAutoRefPtr<IPropertyValuesHolder> m_aniPosHolder;
    SAutoRefPtr<IPropertyValuesHolder> m_aniWidthHolder;
    SAutoRefPtr<IPropertyValuesHolder> m_aniHeightHolder;
    float m_fAniFraction;
    ANI_STATE m_aniState;
  public:
    /**
     * @brief 构造函数
     */
    SAnchorLayoutParam();
    SAnchorLayoutParam(const SAnchorLayoutParam &other);
    ~SAnchorLayoutParam();

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
  protected:
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
     * @brief 处理大小属性
     * @param strValue 属性值字符串
     * @param bLoading 是否正在加载
     * @return 处理结果
     */
    HRESULT OnAttrSize(const SStringW &strValue, BOOL bLoading);

    /**
     * @brief 处理位置属性
     * @param strValue 属性值字符串
     * @param bLoading 是否正在加载
     * @return 处理结果
     */
    HRESULT OnAttrPos(const SStringW &strValue, BOOL bLoading);

    /**
     * @brief 处理偏移属性
     * @param strValue 属性值字符串
     * @param bLoading 是否正在加载
     * @return 处理结果
     */
    HRESULT OnAttrOffset(const SStringW &strValue, BOOL bLoading);

    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"size", OnAttrSize)     // 大小
        ATTR_CUSTOM(L"width", OnAttrWidth)   // 宽度
        ATTR_CUSTOM(L"height", OnAttrHeight) // 高度
        ATTR_CUSTOM(L"pos", OnAttrPos)       // 位置
        ATTR_CUSTOM(L"offset", OnAttrOffset) // 偏移
        ATTR_FLOAT(L"offsetX", pos.fOffsetX, TRUE)
        ATTR_FLOAT(L"offsetY", pos.fOffsetY, TRUE)
    SOUI_ATTRS_BREAK()

  public:

    static BOOL ParsePosition(const SStringW &pos, AnchorPos &AniPos);
};

/**
 * @class SouiLayout
 * @brief Soui布局类
 */
class SOUI_EXP SAnchorLayout : public TObjRefImpl<SObjectImpl<ILayout> > {
    DEF_SOBJECT(SObjectImpl<ILayout>, L"Anchor")

  public:
    typedef CPoint (CALLBACK *PFN_ANCHOR_TO_POS)(const CRect &rcParent, int type);
    /**
     * @brief 构造函数
     */
    SAnchorLayout(void);

    /**
     * @brief 析构函数
     */
    ~SAnchorLayout(void);

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

    void SetAnchor2PosCallback(PFN_ANCHOR_TO_POS pfnAnchor2Pos);
    PFN_ANCHOR_TO_POS GetAnchor2PosCallback() const{
        return m_pfnAnchor2Pos;
    }

    static CPoint CALLBACK DefaultAnchor2Pos(const CRect &rcParent, int type);
  protected:
    POINT Position2Point(const AnchorPos &pos, const CRect &rcParent, const CSize & szChild, int nScale) const;
    POINT CalcPoint4Animator(const AnchorPos &start, const AnchorPos &end, float fraction, const CRect &rcParent, const CSize & szChild, int nScale) const;

    PFN_ANCHOR_TO_POS m_pfnAnchor2Pos;
  };

SNSEND

#endif // __SANILAYOUT__H__