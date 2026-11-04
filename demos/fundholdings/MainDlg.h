#pragma once
#include "fund_eastmoney.h"
#include "fund_estimate.h"
#include "SFundWidgets.h"

// 单只基金的完整运行时状态：每个页签一份，持仓/净值/估值/走势互不干扰。
// 切换页签时不需要重新请求，直接用缓存渲染。
struct WatchFund {
    std::string code;                 // 基金代码
    std::string name;                 // 基金名称（来自净值接口）
    std::vector<HoldingChange> rows;  // 持仓变化表（季度内不变，加载一次即可）
    FundMeta meta;                    // 最新净值
    FundEstimate est;                 // 最近一次估算结果
    std::vector<double> series;       // 该基金的估算走势序列
    bool loading;                     // 持仓抓取中
    bool loaded;                      // 持仓已加载（成功或已失败过，避免反复重试）
    std::string err;                  // 加载失败原因

    WatchFund() : loading(false), loaded(false) {}
};

// 持仓 + 净值抓取完成后，由 SNotifyCenter 回投到 UI 线程
DEF_EVT_EXT(EventWatchResult, EVT_WATCH_RESULT, {
    BOOL bOk;
    std::string code;
    std::vector<HoldingChange> rows;
    FundMeta meta;
    std::string err;
})

// 行情（可选同时刷新净值）完成后，回投到 UI 线程。
// 一次请求覆盖所有已打开基金持有的股票，UI 侧再按各基金权重分别算估值。
DEF_EVT_EXT(EventMarketResult, EVT_MARKET_RESULT, {
    BOOL bOk;
    std::vector<FundMeta> metas;
    std::vector<StockQuote> quotes;
    std::string err;
})

// 大盘三大指数抓取完成后回投到 UI 线程。与基金列表无关：
// 即使收藏数为 0，定时器也会继续驱动这条链路。
DEF_EVT_EXT(EventIndexResult, EVT_INDEX_RESULT, {
    BOOL bOk;
    std::vector<IndexQuote> indexes;
    std::string err;
})

class CMainDlg : public SHostWnd,
                 public SDpiHandler<CMainDlg>,
                 public TAutoEventMapReg<CMainDlg>,
                 public IFundTabsListener,
                 public IFundWatchTabsListener {
  public:
    CMainDlg();
    ~CMainDlg();

    int OnCreate(LPCREATESTRUCT lpCreateStruct);
    BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
    void OnClose();
    void OnMaximize();
    void OnRestore();
    void OnMinimize();
    void OnSize(UINT nType, CSize size);

    // 窗口销毁后调用：停掉后台工作线程。
    // 单独抽出来是因为 m_taskLoop->stop() 会等正在跑的请求收尾，
    // 若放在 ~CMainDlg（close 流程里）就会让窗口"关不掉"，
    // 放到 app.Run() 返回之后，界面上已经看不到任何卡顿。
    void Shutdown();

  protected:
    // IFundTabsListener / IFundWatchTabsListener
    virtual void OnFundTabsSel(int idx) { OnTabSel(idx); }
    virtual void OnFundWatchSel(int idx) { OnWatchSel(idx); }
    virtual void OnFundWatchClose(int idx) { OnWatchClose(idx); }

    void OnQuery();        // 打开/查询输入框中的基金
    void OnWatchAdd();     // 把输入框代码加入自选（不切换页面）
    void OnRefreshAll();   // 手动刷新估值：净值 + 行情，并重置自动刷新计时
    void OnSimOn();        // 打开"模拟盘中"
    void OnSimOff();       // 关闭"模拟盘中"
    void OnFilter();
    void OnResetSort();
    void OnLogClear();
    void OnTabSel(int idx);
    void OnTimer(UINT_PTR idEvent);
    void OnAbout();        // 状态栏署名：进/出 SOUI5 介绍视图
    void OnAboutBack();    // 从介绍视图返回行情/空界面
    void OnAddSample();    // 空界面一键加入示例基金 161725

    // 自选页签回调
    void OnWatchSel(int idx);
    void OnWatchClose(int idx);

    // UI 线程处理后台回投结果
    void OnWatchResult(EventArgs *e);
    void OnMarketResult(EventArgs *e);
    void OnIndexResult(EventArgs *e);

    // 后台工作线程任务
    void doFetchWatch(const std::string &code, const std::string &year);
    void doFetchMarket(const std::string &codesCsv, const std::string &metaCsv, bool simulated);
    void doFetchIndexes();

    // 自选管理
    int FindWatch(const std::string &code) const;
    WatchFund *CurFund();
    void LoadWatchFund(int idx);       // 按需（串行）加载某只基金的持仓
    void LoadNextPending();            // 加载下一只尚未加载的基金
    void ActivateFund(int idx);        // 切换当前页签并渲染
    void RefreshWatchTabs();           // 把 m_watch 同步到页签控件
    std::string BuildQuoteCodes() const;  // 所有已打开基金最新季度持仓代码并集

    void FillGrid(const std::vector<HoldingChange> &list);
    void FillEstGrid(const FundEstimate &est);
    void UpdateEstimateUI(const FundEstimate &est, bool bAppend = true);
    void UpdateBadge();
    void UpdateStatusSummary();
    // 日志面板：把 CLogSink 里新增的日志追加到 rich edit（bRebuild=true 时整篇重填）
    void PumpLog(bool bRebuild);
    void AppendLogText(const SStringT &text);
    void ShowPane(int idx);
    void SyncTabCounts();
    void UpdateViewMode();   // 按「是否在介绍页 / 收藏数是否为 0」切换三个视图
    void ShowEmptyState();   // 0 支基金时清空当前数据并显示空界面
    void UpdateIndexBar();

    void SetStatus(const SStringT &s);
    void SetBusy(BOOL bBusy);
    void RestartTimer();
    void RefreshAllQuotes(bool withMeta);  // 触发一次行情抓取（后台线程）
    void RefreshIndexes();                // 触发一次大盘指数抓取（与基金列表无关）
    void SaveWatch();

    EVENT_MAP_BEGIN()
        EVENT_HANDLER(EventWatchResult::EventID, OnWatchResult)
        EVENT_HANDLER(EventMarketResult::EventID, OnMarketResult)
        EVENT_HANDLER(EventIndexResult::EventID, OnIndexResult)
        EVENT_ID_COMMAND(R.ID.btn_close, OnClose)
        EVENT_ID_COMMAND(R.ID.btn_min, OnMinimize)
        EVENT_ID_COMMAND(R.ID.btn_max, OnMaximize)
        EVENT_ID_COMMAND(R.ID.btn_restore, OnRestore)
        EVENT_ID_COMMAND(R.ID.btn_query, OnQuery)
        EVENT_ID_COMMAND(R.ID.btn_watch_add, OnWatchAdd)
        EVENT_ID_COMMAND(R.ID.btn_refresh, OnRefreshAll)
        EVENT_ID_COMMAND(R.ID.btn_sim_on, OnSimOff)
        EVENT_ID_COMMAND(R.ID.btn_sim_off, OnSimOn)
        EVENT_ID_COMMAND(R.ID.btn_filter, OnFilter)
        EVENT_ID_COMMAND(R.ID.btn_reset, OnResetSort)
        EVENT_ID_COMMAND(R.ID.btn_log_clear, OnLogClear)
        EVENT_ID_COMMAND(R.ID.btn_about, OnAbout)
        EVENT_ID_COMMAND(R.ID.btn_about_back, OnAboutBack)
        EVENT_ID_COMMAND(R.ID.btn_add_sample, OnAddSample)
    EVENT_MAP_END2(SHostWnd)

    BEGIN_MSG_MAP_EX(CMainDlg)
        // 必须在 MSG_WM_INITDIALOG 之前：SDpiHandler 要在首屏用
        // GetWindowScale 把逻辑尺寸放大到当前 DPI，随后才执行我们的初始化；
        // MSG_WM_INITDIALOG 一旦置 bHandled 就会短路后续 CHAIN_MSG_MAP。
        CHAIN_MSG_MAP(SDpiHandler<CMainDlg>)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_SIZE(OnSize)
        MSG_WM_TIMER(OnTimer)
        CHAIN_MSG_MAP(SHostWnd)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

  private:
    SComMgr2 m_comMgr;                  // 用于创建后台 TaskLoop 与 IHttpClient
    // 平台无关的 HTTP 客户端：GetCookie 用它取会话 Cookie。
    // 只有一条后台工作线程，所以这个实例在后台被串行复用是安全的。
    // 声明顺序很重要：必须排在 m_taskLoop 之前 —— 成员按声明逆序析构，
    // 这样 m_taskLoop 先析构（等线程退出），m_http 才被释放，
    // 否则后台线程可能正在用 m_http 时对象就已经没了。
    SAutoRefPtr<IHttpClient> m_http;
    SAutoRefPtr<ITaskLoop> m_taskLoop;  // 后台工作线程（抓取数据）

    std::vector<WatchFund> m_watch;  // 自选/已打开的基金（顺序与页签一致）
    int m_cur;                       // 当前页签下标

    std::string m_year;  // 持仓查询年度（按当前年份自动确定，UI 不再暴露）

    // 当前页签的数据副本，供表格/卡片/状态栏直接使用
    std::vector<HoldingChange> m_rows;
    std::vector<HoldingChange> m_view;  // 当前展示（可能经过筛选）
    FundMeta m_meta;
    FundEstimate m_est;

    BOOL m_bBusy;       // 持仓抓取中
    BOOL m_bQuoteBusy;  // 行情抓取中
    BOOL m_bSimulated;  // 模拟盘中
    UINT_PTR m_timer;   // 自动刷新定时器

    SRichEdit *m_pLog;    // 「请求日志」面板（只读 rich edit）
    fund_uint64 m_logSeen;  // 已经显示到 CLogSink 的第几条
    UINT_PTR m_logTimer;    // 日志增量刷新定时器（轮询内存队列，不做文件 IO）

    BOOL m_bAbout;                       // 当前是否显示 SOUI5 介绍视图
    BOOL m_bIndexBusy;                   // 大盘指数抓取中
    std::vector<IndexQuote> m_indexes;    // 最近一次成功抓取的三大指数
    SStringT m_indexNote;                 // 指数条右侧说明（数据源/更新时间/失败原因）
};
