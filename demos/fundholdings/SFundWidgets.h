// SFundWidgets.h : 基金持仓工具的暗色自绘控件
//
// 三个控件对应设计稿的三个自绘区域：
//   fundkpi     —— KPI 卡片（标签/主数值/副标题/胶囊/覆盖度圆环）
//   fundspark   —— 估算分时走势（折线 + 面积 + 0 轴 + 时间刻度）
//   fundcontrib —— 贡献度分解（以 0 轴为中心的双向条）
//
// 三者都自带圆角卡片背景与描边，不依赖皮肤资源，便于像素级贴合设计稿。
#pragma once
#include "fund_estimate.h"

SNSBEGIN

// ---------------------------------------------------------------- 回调接口
// VS2008 没有 std::function / std::bind，控件对宿主的回调改回最朴素的
// 多态接口：宿主（CMainDlg）实现下面的接口，再把 this 交给 SetListener 即可。
class IFundTabsListener {
  public:
    virtual void OnFundTabsSel(int idx) = 0;
    virtual ~IFundTabsListener() {}
};

class IFundWatchTabsListener {
  public:
    virtual void OnFundWatchSel(int idx) = 0;
    virtual void OnFundWatchClose(int idx) = 0;
    virtual ~IFundWatchTabsListener() {}
};

// ---------------------------------------------------------------- KPI 卡片
class SFundKpiCard : public SWindow {
    DEF_SOBJECT(SWindow, L"fundkpi")
  public:
    SFundKpiCard();

    // hero=true 时使用更大的主数值字号（对应设计稿的"盘中估算涨跌幅"主卡）
    void SetHero(bool b);

    // label：左上角小标题；value/unit：主数值与单位；sub：底部说明；pill：右上角状态胶囊
    void SetContent(const SStringT &label, const SStringT &value, const SStringT &unit,
                    const SStringT &sub, const SStringT &pill, COLORREF valueClr);

    // 覆盖度圆环（rate: 0~1；center: 环内主文本；note: 环右侧说明）
    void SetRing(float rate, const SStringT &center, const SStringT &note);

  protected:
    void OnPaint(IRenderTarget *pRT);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
    SOUI_MSG_MAP_END()

  private:
    SStringT m_label, m_value, m_unit, m_sub, m_pill, m_ringCenter, m_ringNote;
    COLORREF m_valueClr;
    bool m_bHero;
    bool m_bRing;
    float m_ringRate;
};

// ------------------------------------------------------------ 估算分时走势
class SFundSpark : public SWindow {
    DEF_SOBJECT(SWindow, L"fundspark")
  public:
    SFundSpark();

    void SetSeries(const std::vector<double> &v);  // 估算涨跌幅序列（%），x 轴按交易分钟均匀分布
    void SetNote(const SStringT &note);            // 右上角说明（如 "2026-11-06 · 全天"）

  protected:
    void OnPaint(IRenderTarget *pRT);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
    SOUI_MSG_MAP_END()

  private:
    std::vector<double> m_v;
    SStringT m_note;
};

// ------------------------------------------------------------ 贡献度分解
class SFundContrib : public SWindow {
    DEF_SOBJECT(SWindow, L"fundcontrib")
  public:
    SFundContrib();

    void SetItems(const std::vector<ContributionItem> &items);  // 已按 |贡献| 降序
    void Clear();

  protected:
    void OnPaint(IRenderTarget *pRT);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
    SOUI_MSG_MAP_END()

  private:
    std::vector<ContributionItem> m_items;
};

// ------------------------------------------------------ 基金页签（自选列表）
// 每个收藏的基金对应一张卡片式页签，同时承担「收藏列表」与「多基金工作区」两个职责：
//   · 卡片上直接显示估算涨跌幅与估算净值，不切换也能看到多只基金的状态
//   · 点卡片切换到该基金的页面，点右上角 × 从收藏中移除，鼠标滚轮横向滚动
struct WatchTabItem {
    SStringT code;  // 基金代码
    SStringT name;  // 基金名称（可能为空，此时显示代码）
    bool loading;   // 持仓是否正在加载
    bool loaded;    // 持仓是否已加载
    bool hasEst;    // 是否有估算结果（无行情时置 false）
    double estPct;  // 估算涨跌幅(%)
    double estNav;  // 估算净值
    double nav;     // 最新净值

    WatchTabItem() : loading(false), loaded(false), hasEst(false), estPct(0), estNav(0), nav(0) {}
};

// 自选基金页签：横向滚动条 + 拖拽滚动 + fling 惯性全部复用 SPanel
// （与 SListView 同一套机制），这里只负责卡片自绘和滚动范围的维护。
// LBUTTONDOWN / MOUSEWHEEL 不在此映射，由 SOUI_MSG_MAP_END() 回退到 SPanel；
// MOUSEMOVE / LBUTTONUP 用常规映射拦截后先链式调用 SPanel，再跑自己的逻辑。
class SFundWatchTabs : public SPanel {
    DEF_SOBJECT(SPanel, L"fundwatchtabs")
  public:
    SFundWatchTabs();

    void SetItems(const std::vector<WatchTabItem> &items);
    void SetCurSel(int i);   // 用户主动选中：同步选中态并确保卡片可见
    void SyncCurSel(int i);  // 后台刷新只同步选中态，绝不改动滚动位置
    int GetCurSel() const { return m_sel; }
    void SetListener(IFundWatchTabsListener *p) { m_pListener = p; }

  protected:
    void OnPaint(IRenderTarget *pRT);
    void OnSize(UINT nType, CSize size);
    void OnMouseLeave();

    // 常规消息映射拦截：先链式调用 SPanel 的手势处理，被吞掉就直接返回。
    void OnLButtonUp(UINT nFlags, CPoint pt);
    void OnMouseMove(UINT nFlags, CPoint pt);
    // 滚轮一格滚过一张卡片，比 SPanel 默认的 10px 更跟手。
    int GetScrollLineSize(BOOL bVertical);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_SIZE(OnSize)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_LBUTTONUP(OnLButtonUp)
    SOUI_MSG_MAP_END()

  private:
    // SPanel 不会自动算滚动范围，SListView 也是直接维护 m_wBarVisible / m_siHoz。
    void UpdateScrollRange();
    void EnsureSelVisible();
    CRect CardRect(int i) const;
    int HitTest(CPoint pt, bool *pClose) const;  // 返回卡片下标，pClose 指出是否落在关闭按钮上

    // 左右滚动按钮：命中矩形必须与 OnPaint 的绘制几何完全一致。
    CRect ArrowRect(bool bRight) const;
    int HitArrow(CPoint pt) const;  // -1=左、+1=右、0=未命中（或该方向已到头）
    void StepScroll(int dir);       // 滚动恰好一个元素（一张卡片）

    std::vector<WatchTabItem> m_items;
    int m_sel;
    int m_hover;
    int m_closeHover;
    int m_arrowHover;  // 当前悬停的滚动按钮：-1 / 0 / +1
    IFundWatchTabsListener *m_pListener;
};

// ------------------------------------------------------------ 页签栏
// 自绘页签：选中态为胶囊底 + 金色文字，标签后可带数量徽标。
class SFundTabs : public SWindow {
    DEF_SOBJECT(SWindow, L"fundtabs")
  public:
    SFundTabs();

    // 每项：标签文案 + 数量（-1 表示不显示数量）
    void SetItems(const std::vector<std::pair<SStringT, int> > &items);
    void SetCurSel(int i);
    int GetCurSel() const { return m_sel; }
    void SetListener(IFundTabsListener *p) { m_pListener = p; }

  protected:
    void OnPaint(IRenderTarget *pRT);
    void OnLButtonUp(UINT nFlags, CPoint pt);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_LBUTTONUP(OnLButtonUp)
    SOUI_MSG_MAP_END()

  private:
    int HitTest(CPoint pt) const;
    void LayoutTabs(std::vector<CRect> &rcs) const;

    std::vector<std::pair<SStringT, int> > m_items;
    int m_sel;
    IFundTabsListener *m_pListener;
};

// ------------------------------------------------------ 大盘三大指数速览条
// 常驻在工具栏下方，与自选基金数量无关：0 支基金时同样定时刷新并显示。
// 涨=红、跌=绿（A 股惯例），字段口径见 fund_estimate.h 的 IndexQuote。
class SFundIndexBar : public SWindow {
    DEF_SOBJECT(SWindow, L"fundindexbar")
  public:
    SFundIndexBar();

    void SetItems(const std::vector<IndexQuote> &items);
    void SetNote(const SStringT &note);  // 右侧说明：数据源 / 更新时间 / 失败原因

  protected:
    void OnPaint(IRenderTarget *pRT);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
    SOUI_MSG_MAP_END()

  private:
    std::vector<IndexQuote> m_items;
    SStringT m_note;
};

// ------------------------------------------------- 竖向滚动容器
// SScrollView 的拖拽滚动只处理直接落到自己身上的鼠标消息；移动端内容都是子窗口，
// 按下后由子窗口捕获鼠标，滚动容器收不到后续 MOVE。这里复用 SOUI 的触摸拦截链
// （SwndContainerImpl::DispatchTouchEvent），只声明纵向轴：
//   - 纵向拖动由本控件接管并滚动，子窗口的点击行为不受影响；
//   - 横向拖动仍交给外层 fund_swipe（切换基金）和内层 pane_swipe（切换表格页）。
// 移动版 dlg_main.xml 用它包住 KPI/走势/贡献度/表格，屏幕高度不足时可上下滚动。
class SFundScroll : public SScrollView {
    DEF_SOBJECT(SScrollView, L"fundscroll")
  protected:
    // 只声明策略：识别 / 接管 / 惯性与 fling 全部走 SPanel 的共享状态机。
    BOOL CanInterceptTouchGesture() const OVERRIDE;
    SGestureDetector::Axis GetGestureAxis() const OVERRIDE;
    BOOL CanScrollByGesture(const CPoint &ptDelta) const OVERRIDE;
    int GetTouchInterceptAxis() const OVERRIDE;

  private:
    BOOL CanScrollVertical() const;
};

// ------------------------------------------------------------ SOUI5 介绍页
// 纯展示控件（无交互），内容随矩形自适应；所有尺寸按 GetScale() 缩放。
// 文案口径：仓库名 soui4、版本 5.x、对外品牌统一称 SOUI5。
class SFundAbout : public SWindow {
    DEF_SOBJECT(SWindow, L"fundabout")
  public:
    SFundAbout();

  protected:
    void OnPaint(IRenderTarget *pRT);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
    SOUI_MSG_MAP_END()
};

SNSEND
