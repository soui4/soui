#ifndef __SOUILAYOUT__H__
#define __SOUILAYOUT__H__

#include <interface/slayout-i.h>
#include <layout/SouiLayoutParamStruct.h>
#include <sobject/Sobject.hpp>

SNSBEGIN

/**
 * @class SouiLayoutParam
 * @brief Soui布局参数类
 */
class SouiLayoutParam
    : public TObjRefImpl<SObjectImpl<ILayoutParam> >
    , protected SouiLayoutParamStruct {
    DEF_SOBJECT(SObjectImpl<ILayoutParam>, L"SouiLayoutParam")

    friend class SouiLayout;
  public:
    /**
     * @brief 构造函数
     */
    SouiLayoutParam();

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
    STDMETHOD_(void, SetSpecifiedSize)
    (THIS_ ORIENTATION orientation, const LAYOUTSIZE *layoutSize) OVERRIDE;

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
    /**
     * @brief 检查是否需要偏移
     * @param orientation 方向（水平或垂直）
     * @return 如果需要偏移返回TRUE，否则返回FALSE
     */
    bool IsOffsetRequired(ORIENTATION orientation) const;

    /**
     * @brief 获取额外大小
     * @param orientation 方向（水平或垂直）
     * @param nScale 缩放比例
     * @return 额外大小
     */
    int GetExtraSize(ORIENTATION orientation, int nScale) const;
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
        ATTR_CUSTOM(L"width", OnAttrWidth)   // 宽度
        ATTR_CUSTOM(L"height", OnAttrHeight) // 高度
        ATTR_CUSTOM(L"pos", OnAttrPos)       // 位置
        ATTR_CUSTOM(L"size", OnAttrSize)     // 大小
        ATTR_CUSTOM(L"offset", OnAttrOffset) // 偏移
        ATTR_FLOAT(L"offsetX", fOffsetX, TRUE)
        ATTR_FLOAT(L"offsetY", fOffsetY, TRUE)
    SOUI_ATTRS_BREAK()

  protected:
    /**
     * @brief 将字符串描述的坐标转换成POSITION_ITEM
     * @param strPos 坐标字符串
     * @param posItem 位置信息结构体引用
     * @return 如果成功转换返回TRUE，否则返回FALSE
     */
    BOOL StrPos2ItemPos(const SStringW &strPos, POS_INFO &posItem);

    /**
     * @brief 解析在pos中定义的前两个位置
     * @param pos1 第一个位置字符串
     * @param pos2 第二个位置字符串
     * @return 如果成功解析返回TRUE，否则返回FALSE
     */
    BOOL ParsePosition12(const SStringW &pos1, const SStringW &pos2);

    /**
     * @brief 解析在pos中定义的后两个位置
     * @param pos3 第三个位置字符串
     * @param pos4 第四个位置字符串
     * @return 如果成功解析返回TRUE，否则返回FALSE
     */
    BOOL ParsePosition34(const SStringW &pos3, const SStringW &pos4);
};

/**
 * @class SouiLayout
 * @brief Soui布局类
 */
class SOUI_EXP SouiLayout : public TObjRefImpl<SObjectImpl<ILayout> > {
    DEF_SOBJECT(SObjectImpl<ILayout>, L"SouiLayout")

  public:
    /**
     * @brief 构造函数
     */
    SouiLayout(void);

    /**
     * @brief 析构函数
     */
    ~SouiLayout(void);

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

  protected:
    /**
     * @struct WndPos
     * @brief 窗口位置结构体
     */
    struct WndPos
    {
        SWindow *pWnd;     // 窗口指针
        CRect rc;          // 窗口矩形区域
        bool bWaitOffsetX; // 是否等待X方向偏移
        bool bWaitOffsetY; // 是否等待Y方向偏移
    };

    /**
     * @brief 计算扩展位置
     * @param pListChildren 子窗口列表指针
     * @param nWidth 宽度
     * @param nHeight 高度
     */
    void CalcPositionEx(SList<WndPos> *pListChildren, int nWidth, int nHeight) const;

    /**
     * @brief 计算位置
     * @param pListChildren 子窗口列表指针
     * @param nWidth 宽度
     * @param nHeight 高度
     * @return 计算结果
     */
    int CalcPostion(SList<WndPos> *pListChildren, int nWidth, int nHeight) const;

    /**
     * @brief 将位置项转换为值
     * @param pLstChilds 子窗口列表指针
     * @param position 位置指针
     * @param pos 位置信息结构体
     * @param nMax 最大值
     * @param bX 是否为X方向
     * @param nScale 缩放比例
     * @return 计算的值
     */
    int PositionItem2Value(SList<WndPos> *pLstChilds, SPOSITION position, const POS_INFO &pos, int nMax, BOOL bX, int nScale) const;

    /**
     * @brief 计算子窗口左侧位置
     * @param pWindow 窗口指针
     * @param pParam 布局参数对象指针
     * @return 计算的左侧位置
     */
    int CalcChildLeft(SWindow *pWindow, SouiLayoutParam *pParam);

    /**
     * @brief 计算子窗口右侧位置
     * @param pWindow 窗口指针
     * @param pParam 布局参数对象指针
     * @return 计算的右侧位置
     */
    int CalcChildRight(SWindow *pWindow, SouiLayoutParam *pParam);

    /**
     * @brief 计算子窗口顶部位置
     * @param pWindow 窗口指针
     * @param pParam 布局参数对象指针
     * @return 计算的顶部位置
     */
    int CalcChildTop(SWindow *pWindow, SouiLayoutParam *pParam);

    /**
     * @brief 计算子窗口底部位置
     * @param pWindow 窗口指针
     * @param pParam 布局参数对象指针
     * @return 计算的底部位置
     */
    int CalcChildBottom(SWindow *pWindow, SouiLayoutParam *pParam);

    /**
     * @brief 检查位置是否等待
     * @param nPos 位置值
     * @return 如果等待返回TRUE，否则返回FALSE
     */
    BOOL IsWaitingPos(int nPos) const;

    /**
     * @brief 获取参考兄弟窗口
     * @param pCurWnd 当前窗口指针
     * @param uCode 代码
     * @return 参考兄弟窗口指针
     */
    SWindow *GetRefSibling(SWindow *pCurWnd, int uCode);

    /**
     * @brief 获取窗口布局矩形区域
     * @param pWindow 窗口指针
     * @return 窗口布局矩形区域
     */
    CRect GetWindowLayoutRect(SWindow *pWindow);
};

SNSEND

#endif // __SOUILAYOUT__H__