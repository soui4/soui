#ifndef __SREBAR__H__
#define __SREBAR__H__

#include <core/SWnd.h>
#include <wtl.mini/souimisc.h>

SNSBEGIN

/**
 * @class SRebar
 * @brief Rebar 控件
 * 
 * SRebar 是一个类似 MFC CRebar 的 SOUI 控件，用于实现可拖动、可调整大小的工具栏容器。
 * 它将多个子窗口（带区/bands）组织在一起，支持以下特性：
 * 
 * 主要功能特性：
 * - 多行布局支持：通过 singleRow 属性控制单行或多行模式
 * - 拖拽重排：通过 gripper（抓取点）拖动带区改变顺序
 * - 宽度调整：通过 splitter（分割条）拖动调整带区宽度
 * - 折叠功能：支持带区的折叠/展开操作
 * - 自动换行：多行模式下带区可自动换行
 * - 视觉反馈：鼠标悬停高亮和拖动状态显示
 * 
 * 使用方式：
 * 1. 在 XML 布局中声明 rebar 控件
 * 2. 添加子控件作为带区内容
 * 3. 通过属性配置带区行为（折叠、大小限制等）
 * 
 * 属性支持：
 * - singleRow: 是否单行模式（默认 true）
 * - colorGripper: gripper 背景颜色
 * - colorGripperDot: gripper 点颜色
 */
class SOUI_EXP SRebar : public SWindow {
    DEF_SOBJECT(SWindow, L"rebar")
  public:
    /**
     * @struct BandInfo
     * @brief Rebar 带区信息结构
     * 
     * 存储 Rebar 控件中每个带区（band）的配置和状态信息
     */
    struct BandInfo
    {
        SStringT title;          // 带区标题文本
        int height;              // 带区高度（像素）
        bool bCollapsible;       // 是否可折叠
        bool bCollapsed;         // 当前是否处于折叠状态
        SWindow *pChild;         // 指向带区内容子窗口的指针
        int minHeight;           // 最小高度
        int maxHeight;           // 最大高度
        int width;               // 当前宽度（水平 Rebar 时使用）
        int minWidth;            // 最小宽度
        int maxWidth;            // 最大宽度
        bool bNewRow;            // 多行模式下为 true 表示从本 band 起新起一行（由拖动产生）
        BandInfo()
            : height(24)
            , bCollapsible(false)
            , bCollapsed(false)
            , pChild(NULL)
            , minHeight(20)
            , maxHeight(200)
            , width(0)
            , minWidth(40)
            , maxWidth(2000)
            , bNewRow(false)
        {
        }
    };

    SRebar();
    virtual ~SRebar();

    // 带区管理方法

    /**
     * @brief 添加一个新的带区（band）到 Rebar 控件
     * @param pChild 指向子窗口的指针，将作为带区内容
     * @param title 带区标题文本
     * @param height 带区高度，默认为 24 像素
     * @param bCollapsible 是否可折叠，默认为 false
     * @return 返回新添加带区的索引，如果失败返回 -1
     */
    int AddBand(SWindow *pChild, const SStringT &title, int height = 24, bool bCollapsible = false);

    /**
     * @brief 移除指定索引的带区
     * @param nIndex 要移除的带区索引（从 0 开始）
     * @return 成功返回 true，失败返回 false（索引无效时）
     */
    bool RemoveBand(int nIndex);

    /**
     * @brief 设置指定带区的高度
     * @param nIndex 带区索引（从 0 开始）
     * @param height 新的高度值（像素）
     * @return 成功返回 true，失败返回 false（索引无效或高度超出范围）
     */
    bool SetBandHeight(int nIndex, int height);

    /**
     * @brief 设置指定带区的标题
     * @param nIndex 带区索引（从 0 开始）
     * @param title 新的标题文本
     * @return 成功返回 true，失败返回 false（索引无效时）
     */
    bool SetBandTitle(int nIndex, const SStringT &title);

    /**
     * @brief 设置指定带区是否可折叠
     * @param nIndex 带区索引（从 0 开始）
     * @param bCollapsible true 表示可折叠，false 表示不可折叠
     * @return 成功返回 true，失败返回 false（索引无效时）
     */
    bool SetBandCollapsible(int nIndex, bool bCollapsible);

    /**
     * @brief 折叠或展开指定的带区
     * @param nIndex 带区索引（从 0 开始）
     * @param bCollapse true 表示折叠，false 表示展开
     * @return 成功返回 true，失败返回 false（索引无效或带区不可折叠时）
     */
    bool CollapseBand(int nIndex, bool bCollapse);

    /**
     * @brief 检查指定带区是否处于折叠状态
     * @param nIndex 带区索引（从 0 开始）
     * @return 已折叠返回 true，未折叠或索引无效返回 false
     */
    bool IsBandCollapsed(int nIndex) const;

    /**
     * @brief 获取 Rebar 控件中的带区总数
     * @return 带区数量
     */
    int GetBandCount() const;

    /**
     * @brief 获取指定带区的详细信息
     * @param nIndex 带区索引（从 0 开始）
     * @return 指向 BandInfo 结构的指针，索引无效时返回 NULL
     */
    BandInfo *GetBandInfo(int nIndex);

  protected:
    // 消息处理方法
    void OnPaint(IRenderTarget *pRT);
    void OnSize(UINT nType, CSize size);
    BOOL OnMouseHover(UINT nFlags, CPoint point);
    BOOL OnMouseLeave();
    BOOL OnLButtonDown(UINT nFlags, CPoint point);
    BOOL OnLButtonUp(UINT nFlags, CPoint point);
    BOOL OnMouseMove(UINT nFlags, CPoint point);
    int GetTotalHeight() const;
    int GetTotalHeightForWidth(int cx) const;
    virtual BOOL OnSetCursor(const CPoint &pt) OVERRIDE;
    // 重载方法
    virtual BOOL CreateChildren(SXmlNode xmlNode) OVERRIDE;
    STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE *pSize, int nParentWid, int nParentHei) OVERRIDE;
    // 消息映射
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_SIZE(OnSize)
        MSG_WM_MOUSEHOVER(OnMouseHover)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
    SOUI_MSG_MAP_END()

    // 内部方法
    void CalculateBandLayout();
    void UpdateBandLayout();
    int HitTest(CPoint point, int &nHitTest) const;
    void DrawBand(IRenderTarget *pRT, int nIndex, const CRect &rcBand);
    void DrawGripper(IRenderTarget *pRT, const CRect &rcGripper);

  protected:
    // 颜色属性    
    COLORREF m_crGripper;    // gripper颜色
    COLORREF m_crGripperDot; // gripper点颜色

  protected:
    SArray<BandInfo> m_bands;
    // 记录每个 band 当前的矩形位置，供绘制和命中测试共用，
    // 支持“水平排布 + 自动换行”（VS Rebar 风格）
    SArray<CRect> m_bandRects;
    int m_nDragBand;    
    int m_nDragPos;        // 按下时 X（reorder 不用；resize 时用于计算 delta）
    int m_nDragStartWidth; // 按下时 band 宽度（仅 resize 用）
    int m_nHoverBand;
    int m_nHoverPart;
    bool m_bDragging; // 正在拖动（gripper 重排 或 splitter 调宽）
    bool m_bResizing; // true=拖 splitter 调宽，false=拖 gripper 重排
    CRect m_rcClient;
    bool m_bSingleRow; // true=单行（默认），false=多行（由拖动 bNewRow 决定换行）

    // 属性映射
    SOUI_ATTRS_BEGIN()
        ATTR_BOOL(L"singleRow", m_bSingleRow, FALSE)        
        ATTR_COLOR(L"colorGripper", m_crGripper, TRUE)
        ATTR_COLOR(L"colorGripperDot", m_crGripperDot, TRUE)
    SOUI_ATTRS_END()
};

SNSEND

#endif