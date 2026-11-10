// MainDlg.cpp : 基金持仓 · 盘中估值助手（暗色主题）
#include "stdafx.h"
#include <SGridCtrl.h>  // FillGrid / FillEstGrid 用 SGridCtrl 完整类型
#include "MainDlg.h"
#include "applog.h"

#define kLogTag "fundholdings"

// 后台任务成员函数指针类型。原实现用 decltype(&CMainDlg::doFetchWatch) 自动推导，
// VS2008 不支持 decltype，这里显式写出类型。
typedef void (CMainDlg::*PFN_FetchWatch)(const std::string &, const std::string &);
typedef void (CMainDlg::*PFN_FetchMarket)(const std::string &, const std::string &, bool);
typedef void (CMainDlg::*PFN_FetchIndexes)();

// ---------------------------------------------------------------- 调色板
static const COLORREF kText = RGBA(0xe8, 0xed, 0xf5, 0xFF);
static const COLORREF kText2 = RGBA(0xaa, 0xb6, 0xc8, 0xFF);
static const COLORREF kText3 = RGBA(0x77, 0x84, 0x9a, 0xFF);
static const COLORREF kDim = RGBA(0x5c, 0x67, 0x79, 0xFF);
static const COLORREF kPos = RGBA(0xff, 0x5c, 0x5c, 0xFF);  // 涨=红
static const COLORREF kNeg = RGBA(0x22, 0xc5, 0x5e, 0xFF);  // 跌=绿
static const COLORREF kAccent = RGBA(0xf0, 0xb4, 0x29, 0xFF);
static const COLORREF kRowBg = RGBA(0x12, 0x1a, 0x26, 0xFF);
static const COLORREF kRowBg2 = RGBA(0x0e, 0x15, 0x1f, 0xFF);  // 隔行
static const COLORREF kHeadBg = RGBA(0x0f, 0x16, 0x22, 0xFF);

// 基金代码为 ASCII 数字，安全转 std::string
static std::string toAscii(const SStringT &s) {
    std::string out;
    out.reserve(s.GetLength());
    for (int i = 0; i < s.GetLength(); ++i) out += (char)s[i];
    return out;
}

// 天天基金的持仓接口按年度返回报告期数据。工具栏不再暴露年度输入，
// 统一取本机当前年份，跨年后自动跟随，无需改代码。
static std::string CurrentYear() {
    std::time_t t = std::time(NULL);
    std::tm tmv;
#if defined(_WIN32)
    localtime_s(&tmv, &t);
#else
    localtime_r(&t, &tmv);
#endif
    char buf[16] = {0};
    fund_snprintf(buf, sizeof(buf), "%04d", tmv.tm_year + 1900);
    return std::string(buf);
}

// ---------- 变化表渲染辅助 ----------
namespace {
// UTF-8 std::string -> SOUI 宽字符串
SStringT U8(const std::string &s) { return S_CA2T(s.c_str(), CP_UTF8); }

// 整数部分的千分位插入
SStringT addCommas(const SStringT &s) {
    SStringT out;
    int n = s.GetLength();
    for (int i = 0; i < n; ++i) {
        if (i > 0 && (n - i) % 3 == 0) out += _T(",");
        out += s[i];
    }
    return out;
}

// 64 位整数 -> 字符串（VS2008 没有 std::to_wstring）
SStringT i64ToStr(fund_int64 v) {
    return SStringT().Format(FUND_I64_FMT,v);
}

// 数值格式化（带千分位），dec 为小数位；负数自动加负号
SStringT formatNumber(double v, int dec) {
    if (v < 0) return _T("-") + formatNumber(-v, dec);
    double scale = std::pow(10.0, dec);
    double r = std::floor(v * scale + 0.5) / scale;
    fund_int64 ip = (fund_int64)r;
    SStringT ipStr = addCommas(i64ToStr(ip));
    if (dec > 0) {
        double frac = r - (double)ip;
        fund_int64 f = (fund_int64)(frac * scale + 0.5);
        if (f >= (fund_int64)scale) f = (fund_int64)scale - 1;
        SStringT fStr = i64ToStr(f);
        while (fStr.GetLength() < dec) fStr = _T("0") + fStr;
        return ipStr + _T(".") + fStr;
    }
    return ipStr;
}

// 变化量格式化：正数前加 "+"
SStringT formatDelta(double v, int dec) {
    if (v > 0) return _T("+") + formatNumber(v, dec);
    return formatNumber(v, dec);
}

// 单元格数值：未持有（has==false）显示 "—"
SStringT cellNum(double v, bool has, int dec, const SStringT &dash) {
    if (!has) return dash;
    return formatNumber(v, dec);
}

COLORREF deltaColor(double v) {
    if (v > 1e-9) return kPos;
    if (v < -1e-9) return kNeg;
    return 0;
}

std::vector<std::string> SplitCsv(const std::string &s) {
    std::vector<std::string> out;
    size_t p = 0;
    while (p <= s.size() && !s.empty()) {
        size_t q = s.find(',', p);
        if (q == std::string::npos) q = s.size();
        if (q > p) out.push_back(s.substr(p, q - p));
        p = q + 1;
    }
    return out;
}

// 模拟行情：随机游走，使走势图与贡献度在非交易时段也能演示
void SimulateQuotes(const std::vector<std::string> &codes, std::vector<StockQuote> &out) {
    static std::map<std::string, double> chg;
    static bool seeded = false;
    if (!seeded) {
        std::srand((unsigned)std::time(NULL));
        seeded = true;
    }
    out.clear();
    out.reserve(codes.size());
    for (size_t i = 0; i < codes.size(); ++i) {
        const std::string &c = codes[i];
        double &v = chg[c];
        if (v == 0.0) v = ((double)std::rand() / (double)RAND_MAX - 0.5) * 2.0;
        v += ((double)std::rand() / (double)RAND_MAX - 0.5) * 0.35;
        if (v > 4.0) v = 4.0;
        if (v < -4.0) v = -4.0;

        StockQuote q;
        q.code = c;
        q.price = 100.0;
        q.chgPct = v;
        q.preClose = 100.0;
        q.ok = true;
        out.push_back(q);
    }
}

// 本地时间 HH:MM:SS：纯 ASCII，转 SStringT 时不会引入编码歧义
SStringT ClockHms() {
    std::time_t t = std::time(NULL);
    std::tm tmv;
#if defined(_WIN32)
    localtime_s(&tmv, &t);
#else
    localtime_r(&t, &tmv);
#endif
    char buf[16];
    fund_snprintf(buf, sizeof(buf), "%02d:%02d:%02d", tmv.tm_hour, tmv.tm_min, tmv.tm_sec);
    return S_CA2T(buf, CP_UTF8);
}
}  // namespace

// ------------------------------------------------------------- 自选持久化
// 只落盘 6 位数字代码（纯 ASCII），每行一个；名称/净值每次启动重新抓取，
// 避免把中文写进配置文件后遇到 GBK/UTF-8 混淆。
static const TCHAR *kWatchFile = _T("fundholdings_watch.txt");
static const UINT kRefreshIntervalMs = 60 * 1000;  // 默认每分钟自动刷新一次估值

static std::vector<std::string> LoadWatchCodes() {
    std::vector<std::string> out;
	SStringT strDir = SApplication::getSingleton().GetAppDir();
	SStringT strFile = strDir +_T("/") + kWatchFile;
    FILE *f = _tfopen(strFile, _T("rb"));
    if (!f) return out;

    char line[128];
    while (fgets(line, sizeof(line), f)) {
        std::string s(line);
        // VS2008 的 std::string 没有 back()/pop_back()，用下标操作等价实现
        while (!s.empty()) {
            const char c = s[s.size() - 1];
            if (c != '\r' && c != '\n' && c != ' ' && c != '\t') break;
            s.erase(s.size() - 1);
        }
        size_t b = 0;
        while (b < s.size() && (s[b] == ' ' || s[b] == '\t')) ++b;
        s = s.substr(b);

        if (s.size() != 6) continue;  // 跳过注释与空行
        if (s.find_first_not_of("0123456789") != std::string::npos) continue;
        if (std::find(out.begin(), out.end(), s) == out.end()) out.push_back(s);
    }
    fclose(f);
    return out;
}

static void SaveWatchCodes(const std::vector<std::string> &codes) {
	SStringT strDir = SApplication::getSingleton().GetAppDir();
	SStringT strFile = strDir +_T("/") + kWatchFile;
    FILE *f = _tfopen(strFile, _T("wb"));
    if (!f) return;
    fputs("# fundholdings_demo watch list (one 6-digit code per line)\n", f);
    for (size_t i = 0; i < codes.size(); ++i) {
        fputs(codes[i].c_str(), f);
        fputc('\n', f);
    }
    fclose(f);
}

// 显式初始化所有 POD 成员：后台任务是否在跑、当前页签下标等都由这些标志决定，
// 依赖「对象内存恰好为 0」会让首次刷新/关闭页签的行为变成随机。
CMainDlg::CMainDlg()
    : SHostWnd(_T("LAYOUT:dlg_main")),
      m_cur(-1), m_bBusy(FALSE), m_bQuoteBusy(FALSE), m_bSimulated(FALSE), m_timer(0),
      m_pFundSwipe(NULL), m_pRenderRoot(NULL), m_bSyncingFundPages(FALSE),
      m_logSeen(0), m_logTimer(0),
      m_bAbout(FALSE), m_bIndexBusy(FALSE) {
}

CMainDlg::~CMainDlg() {
    // 只撤销本对象的待执行任务；不要在这里 stop()/等待线程，
    // 否则关窗口时要等正在跑的 HTTP 请求收尾（最长十几秒），表现就是"关不掉/卡死"。
    // 真正的收尾由 Shutdown() 在窗口销毁之后完成。
    if (m_taskLoop) {
        m_taskLoop->cancelTasksForObject(this);
        m_taskLoop = NULL;
    }
}

void CMainDlg::Shutdown() {
    // 此时主窗口已经销毁，等待后台线程退出不会再影响界面响应。
    if (m_logTimer) {
        ::KillTimer(m_hWnd, 2);
        m_logTimer = 0;
    }
    if (m_timer) {
        ::KillTimer(m_hWnd, 1);
        m_timer = 0;
    }
    if (m_taskLoop) {
        m_taskLoop->cancelTasksForObject(this);
        m_taskLoop->stop();
        m_taskLoop = NULL;
    }
    // 工作线程停了才能安全释放 HTTP 客户端（后台可能还在用它取 Cookie）
    m_http = NULL;
}

int CMainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    SetMsgHandled(FALSE);
    return 0;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam) {
    // 后台任务循环（工作线程），用于异步抓取，避免阻塞 UI
    m_comMgr.CreateTaskLoop((IObjRef **)&m_taskLoop);
    if (m_taskLoop) m_taskLoop->start("fund_fetcher", Normal);

    // 平台无关的 HTTP 客户端（Windows=WinHTTP，其它平台=libcurl）。
    // 工具里所有网络请求（会话 Cookie、持仓、行情、净值）都走它，业务代码不再直接调平台 API。
    // 只有一条后台工作线程，所以这一个实例被串行复用是安全的。
    if (m_comMgr.CreateHttpClient((IObjRef **)&m_http) && m_http) {
        // 超时给得保守一点：请求都在后台线程跑，但配太短会让慢网络下"刷新"总是失败，
        // 配太长又会拖住退出/切基金的收尾。接收超时 8s 足够拉完几百 KB 的档案脚本。
        m_http->SetTimeOut(4000, 4000, 8000);
        // UA / X-Requested-With 等公共头统一在这里配一次（缺 X-Requested-With 会 404）
        configureHttpClient(m_http);
    }

    // 注册结果事件，使其可被 SNotifyCenter 跨线程投递
    SNotifyCenter::getSingletonPtr()->addEvent(EVENTID(EventWatchResult));
    SNotifyCenter::getSingletonPtr()->addEvent(EVENTID(EventMarketResult));
    SNotifyCenter::getSingletonPtr()->addEvent(EVENTID(EventIndexResult));

    // 大盘指数条先给出等待态，第一次后台请求回来后再填真实数值
    m_indexNote = _T("正在获取大盘指数…");
    UpdateIndexBar();

    // 「请求日志」只读 rich edit 每页一份，统一在 InitDetailPage 里按页初始化。

    // 桌面 / mobile / ohos 三套 uires 共用同一个布局模型：fund_swipe 是隐藏
    // 页头的真实 tabctrl，一支基金一页，页数由自选列表决定（SyncFundPages
    // 动态建页）。只订阅 Changed：滑动时让 tabctrl 自己完成换页 + CommitDrag
    // 连续动画，不再在 Changing 里 bCancel 掉换页、也不再复位到「中间页」。
    m_pFundSwipe = FindChildByName2<STabCtrl>("fund_swipe");
    if (m_pFundSwipe)
        m_pFundSwipe->GetEventSet()->subscribeEvent(
            EventTabSelChanged::EventID, Subscriber(&CMainDlg::OnFundSwipeChanged, this));

    // 基金页签（自选列表）：点击切换、点 × 关闭
    if (SFundWatchTabs *p = FindCtrl<SFundWatchTabs>(R.ID.watch_tabs)) {
        p->SetListener(this);
    }

    UpdateBadge();

    // 载入自选列表：允许 0 支收藏。首次启动或用户清空后保持空界面，
    // 不再偷偷塞一只默认基金（收藏数由用户决定，0 也是合法状态）。
    std::vector<std::string> codes = LoadWatchCodes();
    for (size_t i = 0; i < codes.size(); ++i) {
        WatchFund w;
        w.code = codes[i];
        m_watch.push_back(w);
    }
    m_year = CurrentYear();  // 持仓查询年度（UI 不再提供输入，按当前年份自动确定）

    // 每一支基金建一个真实 page（include fund_detail.xml）；三端都有 fund_swipe，
    // 建页完成后 ShowPane 才能按页找到 pane_swipe。保留 NULL 兜底兼容旧布局。
    SyncFundPages();
    if (!m_pFundSwipe) InitDetailPage(NULL);
    ShowPane(0);

    UpdateViewMode();
    if (m_watch.empty()) {
        ShowEmptyState();
    } else {
        ActivateFund(0);  // 会触发后台串行加载持仓
    }
    RestartTimer();
    RefreshIndexes();  // 指数与基金列表无关，启动就拉一次

    // 日志增量刷新：400ms 轮询一次内存队列（无新日志时直接返回）。
    // 以前是每次刷新都去读 log4z 的日志文件，文件一大就会卡住 UI 线程。
    m_logTimer = ::SetTimer(m_hWnd, 2, 400, NULL);

    if (m_watch.empty())
        SetStatus(_T("就绪：暂无收藏基金，大盘三大指数正在刷新；填入代码后点「＋ 收藏」开始"));
    else
        SetStatus(_T("就绪：自选基金正在后台逐个加载持仓，估值与大盘指数每分钟自动刷新一次"));
    return TRUE;
}

void CMainDlg::OnClose() {
    if (m_timer) {
        ::KillTimer(m_hWnd, 1);
        m_timer = 0;
    }
    if (m_logTimer) {
        ::KillTimer(m_hWnd, 2);
        m_logTimer = 0;
    }
    // 日志控件按当前页动态取，不再缓存指针；窗口销毁后由控件树统一析构。
    DestroyWindow();
}

void CMainDlg::OnMaximize() { SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE); }
void CMainDlg::OnRestore() { SendMessage(WM_SYSCOMMAND, SC_RESTORE); }
void CMainDlg::OnMinimize() { SendMessage(WM_SYSCOMMAND, SC_MINIMIZE); }

void CMainDlg::OnSize(UINT nType, CSize size) {
    if (nType == SIZE_MAXIMIZED) {
        if (SWindow *p = FindWnd(R.ID.btn_max)) p->SetVisible(FALSE);
        if (SWindow *p = FindWnd(R.ID.btn_restore)) p->SetVisible(TRUE);
    } else {
        if (SWindow *p = FindWnd(R.ID.btn_max)) p->SetVisible(TRUE);
        if (SWindow *p = FindWnd(R.ID.btn_restore)) p->SetVisible(FALSE);
    }
    SetMsgHandled(FALSE);
}

void CMainDlg::SetStatus(const SStringT &s) {
    if (SWindow *p = FindWnd(R.ID.txt_status)) p->SetWindowText(s);
}

void CMainDlg::SetBusy(BOOL bBusy) {
    m_bBusy = bBusy;
    if (SWindow *p = FindWnd(R.ID.btn_query)) p->EnableWindow(!bBusy);
    if (bBusy) {
        if (SWindow *p = FindWnd(R.ID.txt_status))
            p->SetWindowText(_T("正在请求天天基金并解析季度持仓…"));
    }
}

// ------------------------------------------------------------- 页签/面板
void CMainDlg::ShowPane(int idx) {
    if (idx < 0 || idx > 2) return;

    BOOL bSwitched = FALSE;
    STabCtrl *pSwipe = CurPaneSwipe();
    if (pSwipe) {
        // 移动版：三个面板是 tabctrl 的 page，页面可见性由 tabctrl 维护；
        // SetCurSel 成功会回调 OnPaneSwipeChanged → UpdatePaneChrome，只有
        // 真正切页时才省略这里的收尾，避免日志页整篇重填两次。
        if (pSwipe->GetCurSel() != idx) bSwitched = pSwipe->SetCurSel(idx);
    } else {
        const int paneIds[3] = {R.ID.pane_change, R.ID.pane_est, R.ID.pane_log};
        for (int i = 0; i < 3; ++i) {
            if (SWindow *p = FindWnd(paneIds[i])) p->SetVisible(i == idx);
        }
    }
    if (!bSwitched) UpdatePaneChrome(idx);
}

// 用户左右滑动切页：同步上方自绘页签的选中态、筛选行与日志内容
BOOL CMainDlg::OnPaneSwipeChanged(EventTabSelChanged *evt) {
    STabCtrl *pSwipe = CurPaneSwipe();
    const int idx = evt ? (int)evt->uNewSel : (pSwipe ? pSwipe->GetCurSel() : 0);
    UpdatePaneChrome(idx);
    return FALSE;
}

void CMainDlg::UpdatePaneChrome(int idx) {
    if (idx < 0 || idx > 2) return;
    const int paneIds[3] = {R.ID.pane_change, R.ID.pane_est, R.ID.pane_log};
    SWindow *pTarget = FindWnd(paneIds[idx]);

    // 「请求日志」页签下，筛选相关的控件没有意义，换成「清空日志」
    // 注意第二个参数必须传 TRUE：只改可见性而不失效区域时，
    // 隐藏后留下的旧像素不会被擦掉，看起来像"控件没藏住"。
    const BOOL bLog = (idx == 2);
    if (SWindow *p = FindWnd(R.ID.edit_filter)) p->SetVisible(!bLog, TRUE);
    if (SWindow *p = FindWnd(R.ID.btn_filter)) p->SetVisible(!bLog, TRUE);
    if (SWindow *p = FindWnd(R.ID.btn_reset)) p->SetVisible(!bLog, TRUE);
    if (SWindow *p = FindWnd(R.ID.btn_log_clear)) p->SetVisible(bLog, TRUE);

    // SetVisible(FALSE) 只改可见性，被隐藏面板的旧像素不会自动擦掉，
    // 这里显式重绘目标面板与父容器，避免切页签后仍看到上一页的内容。
    if (pTarget) pTarget->Invalidate();
    Invalidate();

    // 手势切页时把上方自绘页签的选中态同步过来；SetCurSel 不会再回调监听器，不会递归。
    if (SFundTabs *p = FindCtrl<SFundTabs>(R.ID.tabs)) p->SetCurSel(idx);

    if (bLog) PumpLog(true);  // 切到日志页时整篇重填一次（取最近 200 行）
}

void CMainDlg::OnTabSel(int idx) {
    ShowPane(idx);
}

// ------------------------------------------------------------- 三个主视图
// view_main（有基金的分析页）/ view_empty（0 支基金）/ view_about（SOUI5 介绍）
// 三者用同一组 pos 叠放，切换只改可见性；第二参数传 TRUE 强制擦掉旧像素，
// 否则隐藏后残留的画面会一直挂在屏幕上。
void CMainDlg::UpdateViewMode() {
    const BOOL bAbout = (m_bAbout != FALSE);
    const BOOL bEmpty = m_watch.empty();
    const int viewIds[3] = {R.ID.view_main, R.ID.view_empty, R.ID.view_about};
    const BOOL vis[3] = {(!bAbout && !bEmpty) ? TRUE : FALSE,
                         (!bAbout && bEmpty) ? TRUE : FALSE,
                         bAbout ? TRUE : FALSE};
    for (int i = 0; i < 3; ++i) {
        if (SWindow *p = FindWnd(viewIds[i])) p->SetVisible(vis[i], TRUE);
    }
    // 状态栏的 powered by soui5 是常驻署名，不随视图切换忽隐忽现。
    Invalidate();
}

// 收藏被清空后，把「当前基金」的所有派生状态复位，避免下次添加时看到上一只的残留
void CMainDlg::ShowEmptyState() {
    m_cur = -1;
    m_rows.clear();
    m_view.clear();
    m_meta = FundMeta();
    m_est = FundEstimate();

    FillGrid(m_rows);
    FillEstGrid(m_est);
    UpdateEstimateUI(m_est, false);
    SyncTabCounts();
    RefreshWatchTabs();

    std::vector<double> empty;
    if (SFundSpark *p = FindCtrl<SFundSpark>(R.ID.chart_est)) p->SetSeries(empty);
    if (SWindow *p = FindWnd(R.ID.txt_fund_chip)) p->SetWindowText(_T("未选择基金"));
    if (SWindow *p = FindWnd(R.ID.txt_upd)) p->SetWindowText(_T("更新于 --:--:--"));
}

void CMainDlg::UpdateIndexBar() {
    if (SFundIndexBar *p = FindCtrl<SFundIndexBar>(R.ID.index_bar)) {
        p->SetItems(m_indexes);
        p->SetNote(m_indexNote);
    }
}

void CMainDlg::OnAbout() {
    // 状态栏署名常驻可点：在介绍页里再点一次就退回行情/空界面，
    // 避免「点了没反应」；返回也可走页内的「← 返回行情」。
    if (m_bAbout) {
        OnAboutBack();
        return;
    }
    m_bAbout = TRUE;
    UpdateViewMode();
    SetStatus(_T("SOUI5 介绍 · 仓库 soui4 / 版本 5.x / 对外称 SOUI5 · 点「← 返回行情」返回"));
}

void CMainDlg::OnAboutBack() {
    m_bAbout = FALSE;
    UpdateViewMode();
    if (m_watch.empty()) SetStatus(_T("暂无收藏基金 · 大盘三大指数每分钟自动刷新"));
    else UpdateStatusSummary();
}

void CMainDlg::OnAddSample() {
    if (SEdit *p = FindCtrl<SEdit>(R.ID.edit_code)) p->SetWindowText(_T("161725"));
    OnWatchAdd();
}

void CMainDlg::AppendLogText(const SStringT &text) {
    SRichEdit *pLog = CurLog();
    if (!pLog || text.IsEmpty()) return;

    // 控件里的文本总量要有个上限：日志是滚动窗口，留太长既浪费内存，
    // 也会让 rich edit 每次插入后的内部排版变慢。超过阈值就先丢掉前半段。
    long nLen = (long)pLog->SSendMessage(WM_GETTEXTLENGTH, 0, 0);
    const long kTrimAt = 40000;  // 字符数
    if (nLen > kTrimAt) {
        pLog->SSendMessage(EM_SETSEL, 0, nLen / 2);
        pLog->ReplaceSel(_T(""), FALSE);
    }

    // 追加而不是整篇 SetWindowText：只重排新增的那几百字节，
    // 日志再多也不会出现"每刷新一次就整篇重排"的卡顿。
    // EM_SETSEL(-1,0) 把插入点移到末尾，ReplaceSel 在只读 rich edit 上依然有效
    //（只读只拦用户输入，不拦程序调用），ipcdemo 的日志窗口也是这个用法。
    pLog->SSendMessage(EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
    pLog->ReplaceSel(text, FALSE);
    pLog->SSendMessage(EM_SCROLLCARET, 0, 0);
}

void CMainDlg::PumpLog(bool bRebuild) {
    SRichEdit *pLog = CurLog();
    if (!pLog) return;

    CLogSink &sink = CLogSink::Instance();
    const unsigned long long total = sink.Count();

    if (bRebuild) {
        pLog->SetWindowText(_T(""));
        // 只回填最近 200 行：日志是滚动窗口，翻太久的没有意义
        m_logSeen = (total > 200) ? total - 200 : 0;
    } else if (total == m_logSeen) {
        return;  // 没有新日志，什么都不做（400ms 轮询的成本就是这一次计数比较）
    }

    std::vector<std::wstring> lines = sink.Snapshot(m_logSeen, 400);
    m_logSeen = total;
    if (lines.empty()) return;

    SStringW text;
    for (size_t i = 0; i < lines.size(); ++i) {
        text += SStringW(lines[i].c_str());
        text += L"\r\n";
    }
    AppendLogText(S_CW2T(text));
}

void CMainDlg::OnLogClear() {
    CLogSink::Instance().Clear();
    if (SRichEdit *pLog = CurLog()) pLog->SetWindowText(_T(""));
    m_logSeen = CLogSink::Instance().Count();
    SetStatus(_T("已清空请求日志（log4z 落盘的日志文件不受影响）"));
}

// ------------------------------------------------------------- 自选/页签
int CMainDlg::FindWatch(const std::string &code) const {
    for (size_t i = 0; i < m_watch.size(); ++i)
        if (m_watch[i].code == code) return (int)i;
    return -1;
}

WatchFund *CMainDlg::CurFund() {
    if (m_cur < 0 || m_cur >= (int)m_watch.size()) return NULL;
    return &m_watch[m_cur];
}

void CMainDlg::SaveWatch() {
    std::vector<std::string> codes;
    codes.reserve(m_watch.size());
    for (size_t i = 0; i < m_watch.size(); ++i) codes.push_back(m_watch[i].code);
    SaveWatchCodes(codes);
}

void CMainDlg::OnFilterSetFocus(IEvtArgs *e)
{
    // 只有筛选框获得焦点时才激活键盘占位：输入基金代码等其它位置弹键盘
    // 时不应把详情页顶起来。keyboardSpacer 初始未激活，SKeyboardSpacer 会
    // 在键盘收起时自动回到未激活态，等下一次焦点事件再打开。
    if (SWindow *root = CurDetailRoot())
    {
        SWindow *pKbSpacer = root->FindChildByName("kbspacer");
        if(pKbSpacer){
            SLOGI()<<"show keyboard spacer";
            pKbSpacer->SetVisible(TRUE);
        }
    }
}

// 把 m_watch 同步到页签控件（代码/名称/涨跌幅/净值/加载状态）
void CMainDlg::RefreshWatchTabs() {
    SFundWatchTabs *p = FindCtrl<SFundWatchTabs>(R.ID.watch_tabs);
    if (!p) return;

    std::vector<WatchTabItem> items;
    items.reserve(m_watch.size());
    for (size_t i = 0; i < m_watch.size(); ++i) {
        const WatchFund &w = m_watch[i];
        WatchTabItem it;
        it.code = U8(w.code);
        it.name = U8(w.name);
        it.loaded = w.loaded;
        it.loading = w.loading;
        it.hasEst = w.est.valid;
        it.estPct = w.est.estPct;
        it.estNav = w.est.estNav;
        it.nav = w.meta.nav;
        items.push_back(it);
    }
    p->SetItems(items);
    // 只同步选中态：后台刷新（持仓/估值回来、定时器）不该把用户滚到的位置
    // 拉回第 0 张卡片。要「让选中项可见」只在用户主动切换时由 ActivateFund 做。
    p->SyncCurSel(m_cur);

    if (SWindow *t = FindWnd(R.ID.txt_watch_cnt)) {
        SStringT s;
        s.Format(_T("%d 只"), (int)m_watch.size());
        t->SetWindowText(s);
    }
}

// ------------------------------------------------------------- 一支基金一页（三端统一）
// 桌面 / mobile / ohos 的 fund_swipe 都是页头隐藏的真实 tabctrl：页数与自选
// 列表一一对应，每页通过 <include src="layout:fund_detail"/> 载入完整分析控件树。
// 下面所有 helper 先限定在当前页子树里找，页外控件再回退到宿主根。
SWindow *CMainDlg::DetailPage(int idx) {
    if (!m_pFundSwipe || idx < 0 || idx >= m_pFundSwipe->GetItemCount()) return NULL;
    return m_pFundSwipe->GetItem(idx);
}

SWindow *CMainDlg::CurDetailRoot() {
    if (m_pRenderRoot) return m_pRenderRoot;
    return DetailPage(m_pFundSwipe ? m_pFundSwipe->GetCurSel() : -1);
}

SWindow *CMainDlg::FindWnd(int id) {
    // 每支基金一页、页内控件 ID 同名：必须先限定在当前页的子树里找。
    // SWindow::_FindChildByID 不跳过隐藏页，且 SWindowFinder 按 (parent,id)
    // 缓存，直接从宿主根查会一直命中第 0 页的控件。
    if (SWindow *root = CurDetailRoot())
        if (SWindow *p = root->FindChildByID(id)) return p;
    // 页外控件（工具条 / 状态栏 / 视图容器 / 自选页签 / 指数条 / edit_code …）
    // 挂在宿主根上，不在任何 page 子树内；layout 里没有 fund_swipe 时
    // CurDetailRoot() 恒为 NULL，也一律走这里。
    return SHostWnd::FindChildByID(id);
}

STabCtrl *CMainDlg::CurPaneSwipe() {
    SWindow *root = CurDetailRoot();
    if (!root) return NULL;
    // 按名字查找：三端模板同名，且不必依赖 R.ID 表里的注册顺序。
    SWindow *p = root->FindChildByName(L"pane_swipe");
    return (p && p->IsClass(STabCtrl::GetClassName())) ? (STabCtrl *)p : NULL;
}

SRichEdit *CMainDlg::CurLog() {
    return FindCtrl<SRichEdit>(R.ID.re_log);
}

void CMainDlg::InitDetailPage(SWindow *pPage) {
    // 表格上方自绘页签：每页一份，都要接同一个监听器。
    SFundTabs *pTabs = pPage ? pPage->FindChildByID2<SFundTabs>(R.ID.tabs)
                             : FindCtrl<SFundTabs>(R.ID.tabs);
    if (pTabs) {
        std::vector<std::pair<SStringT, int> > items;
        items.push_back(std::make_pair(SStringT(_T("持仓变化表")), 0));
        items.push_back(std::make_pair(SStringT(_T("估值明细")), 0));
        items.push_back(std::make_pair(SStringT(_T("请求日志")), -1));
        pTabs->SetItems(items);
        pTabs->SetListener(this);
    }
    // 面板 tabctrl 的滑动切页通知：每页一份。
    if (pPage) {
        SWindow *p = pPage->FindChildByName(L"pane_swipe");
        if (p && p->IsClass(STabCtrl::GetClassName())) {
            ((STabCtrl *)p)->GetEventSet()->subscribeEvent(
                EventTabSelChanged::EventID, Subscriber(&CMainDlg::OnPaneSwipeChanged, this));
        }
    }
    // 「请求日志」只读 rich edit：原生支持滚动/选中/复制，文本量大时也不会
    // 像 multiLines 的 text 那样每次重排布局。
    SRichEdit *pLog = pPage ? pPage->FindChildByID2<SRichEdit>(R.ID.re_log) : CurLog();
    if (pLog) {
        pLog->SetReadOnly(TRUE);
        pLog->SetWordWrap(TRUE);
        pLog->SetLimitText(1024 * 1024);  // 清空由 PumpLog/CLogSink 负责，这里给足容量
    }
}

void CMainDlg::SyncFundPages() {
    if (!m_pFundSwipe) return;
    // RemoveItem 删到当前页时会自动 SetCurSel，期间的 Changed 一律忽略，
    // 否则会在收藏列表还没稳定时反过来改写 m_cur。
    m_bSyncingFundPages = TRUE;
    const int nWant = (int)m_watch.size();
    while (m_pFundSwipe->GetItemCount() > nWant)
        m_pFundSwipe->RemoveItem(m_pFundSwipe->GetItemCount() - 1, 0);
    while (m_pFundSwipe->GetItemCount() < nWant) {
        const int nBefore = m_pFundSwipe->GetItemCount();
        const int i = m_pFundSwipe->InsertItem(
            L"<include src='layout:fund_detail'/>");
        InitDetailPage(DetailPage(i));
    }
    m_bSyncingFundPages = FALSE;
}

void CMainDlg::RenderFundPage(int idx) {
    if (!m_pFundSwipe || idx < 0 || idx >= (int)m_watch.size()) return;
    SWindow *pPage = DetailPage(idx);
    if (!pPage) return;

    // 预渲染邻页：临时把「当前基金数据副本 + 渲染目标页」切到该基金，
    // 复用现有的一整套渲染函数，渲染完立刻恢复。这样 STabSlider 在手指刚
    // 开始拖动时拍到的就是真实的新基金页面，而不是空白过渡页。
    SWindow *pOldRoot = m_pRenderRoot;
    std::vector<HoldingChange> oldRows = m_rows, oldView = m_view;
    FundMeta oldMeta = m_meta;
    FundEstimate oldEst = m_est;
    int oldCur = m_cur;

    m_pRenderRoot = pPage;
    m_cur = idx;
    m_rows = m_watch[idx].rows;
    m_view = m_rows;
    m_meta = m_watch[idx].meta;
    m_est = m_watch[idx].est;

    FillGrid(m_rows);
    FillEstGrid(m_est);
    UpdateEstimateUI(m_est, false);  // 预渲染绝不追加走势采样点
    SyncTabCounts();
    if (SFundSpark *p = FindCtrl<SFundSpark>(R.ID.chart_est)) p->SetSeries(m_watch[idx].series);

    m_pRenderRoot = pOldRoot;
    m_cur = oldCur;
    m_rows = oldRows;
    m_view = oldView;
    m_meta = oldMeta;
    m_est = oldEst;
}

void CMainDlg::RenderNeighborPages() {
    if (!m_pFundSwipe) return;
    for (int i = m_cur - 1; i <= m_cur + 1; ++i)
        RenderFundPage(i);
}

void CMainDlg::SyncFundChrome() {
    if (m_cur < 0 || m_cur >= (int)m_watch.size()) return;
    const WatchFund &w = m_watch[m_cur];
    if (SEdit *p = FindCtrl<SEdit>(R.ID.edit_code)) p->SetWindowText(U8(w.code));
    if (SWindow *p = FindWnd(R.ID.txt_fund_chip)) {
        SStringT s;
        s.Format(_T("%s  %s"), U8(w.code).c_str(),
                 w.name.empty() ? _T("加载中…") : U8(w.name).c_str());
        p->SetWindowText(s);
    }
    if (SFundWatchTabs *tabs = FindCtrl<SFundWatchTabs>(R.ID.watch_tabs)) tabs->SetCurSel(m_cur);
}

// 切换当前基金：优先用缓存渲染，不发起请求；未加载时才触发后台抓取。
// 移动版点击自选卡片时也让 fund_swipe 真正换页；滑动换页由 OnFundSwipeChanged
// 负责，那边绝不能反过来调 SetCurSel（会 Stop 掉正在播放的 STabSlider）。
void CMainDlg::ActivateFund(int idx) {
    if (idx < 0 || idx >= (int)m_watch.size()) return;
    m_cur = idx;
    WatchFund &w = m_watch[idx];

    if (SEdit *p = FindCtrl<SEdit>(R.ID.edit_code)) p->SetWindowText(U8(w.code));
    if (SEdit *p = FindCtrl<SEdit>(R.ID.edit_filter)) p->SetWindowText(_T(""));

    m_rows = w.rows;
    m_view = m_rows;
    m_meta = w.meta;
    m_est = w.est;

    // 该基金还没有采样序列但有估值时，先放一个点，避免走势图空白
    if (w.series.empty() && w.est.valid) w.series.push_back(w.est.estPct);

    // 点击页签也走 tabctrl 换页：桌面版没有该控件，直接渲染即可。
    // SetCurSel 会 Stop 掉正在播放的 slider，所以只在「用户主动点击」时用。
    if (m_pFundSwipe && m_pFundSwipe->GetCurSel() != idx)
        m_pFundSwipe->SetCurSel(idx);

    if (SFundSpark *p = FindCtrl<SFundSpark>(R.ID.chart_est)) p->SetSeries(w.series);
    FillGrid(m_rows);
    FillEstGrid(m_est);
    UpdateEstimateUI(w.est, false);  // 切页签只是重绘，不再往走势里追加采样点
    UpdateStatusSummary();
    SyncTabCounts();
    RefreshWatchTabs();
    // 用户主动切换 / 新增收藏时才把选中卡片滚进可视区；
    // 后台刷新走的 RefreshWatchTabs() 已改成 SyncCurSel，不会碰滚动位置。
    if (SFundWatchTabs *tabs = FindCtrl<SFundWatchTabs>(R.ID.watch_tabs))
        tabs->SetCurSel(m_cur);

    if (SWindow *p = FindWnd(R.ID.txt_fund_chip)) {
        SStringT s;
        s.Format(_T("%s  %s"), U8(w.code).c_str(),
                 w.name.empty() ? _T("加载中…") : U8(w.name).c_str());
        p->SetWindowText(s);
    }

    // 预渲染相邻页：下一次手指刚开始拖动，STabSlider 拍到的就是真实的新基金页面。
    RenderNeighborPages();

    if (!w.loaded && !w.loading) {
        LoadWatchFund(idx);
    } else if (w.loaded && !w.est.valid) {
        // 持仓已就绪但还没算过估值（比如刚加载完就切过来），立刻补一次行情
        RefreshAllQuotes(false);
    }
}

void CMainDlg::LoadWatchFund(int idx) {
    if (idx < 0 || idx >= (int)m_watch.size() || !m_taskLoop) return;
    WatchFund &w = m_watch[idx];
    if (w.loaded || w.loading) return;

    w.loading = true;
    w.err.clear();
    RefreshWatchTabs();

    SFunctor2<CMainDlg, PFN_FetchWatch, std::string, std::string> task(
        this, &CMainDlg::doFetchWatch, w.code, m_year);
    m_taskLoop->postTask(&task, FALSE, 0);
}

// 串行加载：一次只抓一只，避免同时开一堆 HTTP 请求
void CMainDlg::LoadNextPending() {
    for (size_t i = 0; i < m_watch.size(); ++i) {
        if (!m_watch[i].loaded && !m_watch[i].loading) {
            LoadWatchFund((int)i);
            return;
        }
    }
}

void CMainDlg::SyncTabCounts() {
    SFundTabs *p = FindCtrl<SFundTabs>(R.ID.tabs);
    if (!p) return;
    std::vector<std::pair<SStringT, int> > items;
    items.push_back(std::make_pair(SStringT(_T("持仓变化表")), (int)m_rows.size()));
    items.push_back(std::make_pair(SStringT(_T("估值明细")), m_est.valid ? m_est.holdCount : 0));
    items.push_back(std::make_pair(SStringT(_T("请求日志")), -1));
    p->SetItems(items);
}

// ------------------------------------------------------------- 查询 / 自选
void CMainDlg::OnQuery() {
    SEdit *pCode = FindCtrl<SEdit>(R.ID.edit_code);
    if (!pCode) return;

    std::string code = toAscii(pCode->GetWindowText());
    code.erase(0, code.find_first_not_of(" \t"));
    if (code.empty()) {
        SetStatus(_T("请先填写 6 位基金代码"));
        return;
    }

    int idx = FindWatch(code);
    if (idx < 0) {
        WatchFund w;
        w.code = code;
        m_watch.push_back(w);
        idx = (int)m_watch.size() - 1;
        SaveWatch();
    } else {
        // 用户主动查询：强制重新加载该基金
        WatchFund &w = m_watch[idx];
        if (!w.loading) {
            w.loaded = false;
            w.rows.clear();
        }
    }

    // 在介绍页里直接点「查询持仓」也要自动回到行情视图
    m_bAbout = FALSE;
    UpdateViewMode();
    SetBusy(TRUE);
    SyncFundPages();
    ActivateFund(idx);
    RestartTimer();
}

void CMainDlg::OnWatchAdd() {
    SEdit *pCode = FindCtrl<SEdit>(R.ID.edit_code);
    if (!pCode) return;

    std::string code = toAscii(pCode->GetWindowText());
    code.erase(0, code.find_first_not_of(" \t"));
    if (code.empty()) {
        SetStatus(_T("请先填写 6 位基金代码，再点「＋ 收藏」"));
        return;
    }
    if (FindWatch(code) >= 0) {
        SetStatus(_T("该基金已在自选列表中"));
        return;
    }

    WatchFund w;
    w.code = code;
    m_watch.push_back(w);
    SaveWatch();
    m_bAbout = FALSE;
    UpdateViewMode();  // 从空界面切回完整分析页
    SyncFundPages();
    ActivateFund((int)m_watch.size() - 1);
    RestartTimer();

    SStringT msg;
    msg.Format(_T("已加入自选：%s（共 %d 只）"), U8(code).c_str(), (int)m_watch.size());
    SetStatus(msg);
}

void CMainDlg::OnWatchSel(int idx) {
    if (idx == m_cur) return;
    ActivateFund(idx);
}

// 移动版：左右滑动切换基金。tabctrl 一支基金一页，STabSlider 在拖动开始时
// 已对前后两页拍好快照并跟手平移，松手后 CommitDrag 连续滑到目标页 —— 与
// 下方「持仓变化 / 估值明细 / 请求日志」用的是完全相同的 tabctrl 换页动画。
// 这里只同步「当前基金」的数据副本和页外 UI；绝不能调 SetCurSel / ActivateFund，
// 因为 SetCurSel 会 Stop 掉刚 start 的 slider，把整段连续过渡动画吞掉。
BOOL CMainDlg::OnFundSwipeChanged(EventTabSelChanged *evt) {
    if (!evt || !m_pFundSwipe || m_bSyncingFundPages) return FALSE;
    const int sel = (int)evt->uNewSel;
    if (sel == m_cur) return FALSE;  // ActivateFund 的程序化切页，已经处理过了
    if (sel < 0 || sel >= (int)m_watch.size()) return FALSE;

    m_cur = sel;
    const WatchFund &w = m_watch[sel];
    m_rows = w.rows;
    m_view = m_rows;
    m_meta = w.meta;
    m_est = w.est;

    SyncFundChrome();
    UpdateStatusSummary();
    RenderNeighborPages();
    return FALSE;
}

void CMainDlg::OnWatchClose(int idx) {
    if (idx < 0 || idx >= (int)m_watch.size()) return;

    m_watch.erase(m_watch.begin() + idx);
    SaveWatch();
    SyncFundPages();  // tabctrl 页数跟着自选列表走

    // 允许删到 0 支：不再要求「至少保留一只」。收藏清空后显示空界面，
    // 大盘指数条与定时器照常工作，用户随时可以重新添加。
    if (m_watch.empty()) {
        ShowEmptyState();
        UpdateViewMode();
        RestartTimer();
        SetStatus(_T("已清空自选（0 只）：填入基金代码后点「＋ 收藏」即可重新开始"));
        return;
    }

    if (m_cur > idx) m_cur--;
    else if (m_cur == idx) m_cur = (std::min)(idx, (int)m_watch.size() - 1);
    if (m_cur < 0) m_cur = 0;

    ActivateFund(m_cur);
    RestartTimer();

    SStringT msg;
    msg.Format(_T("已从自选移除，剩余 %d 只"), (int)m_watch.size());
    SetStatus(msg);
}

// ------------------------------------------------------------- 后台抓取
void CMainDlg::doFetchWatch(const std::string &code, const std::string &year) {
    std::vector<FundHolding> list;
    std::string err;
    const bool ok = fetchFundHoldings(m_http, code, year, list, err);

    std::vector<HoldingChange> rows;
    if (ok) buildHoldingsChange(list, rows);

    FundMeta meta;
    std::string merr;
    fetchFundMeta(m_http, code, meta, merr);

    EventWatchResult *evt = new EventWatchResult(NULL);
    evt->bOk = ok ? TRUE : FALSE;
    evt->code = code;
    evt->rows.swap(rows);
    evt->meta = meta;
    if (!ok) evt->err = err;
    SNotifyCenter::getSingletonPtr()->FireEventAsync(evt);
    evt->Release();
}

void CMainDlg::OnWatchResult(EventArgs *e) {
    EventWatchResult *e2 = sobj_cast<EventWatchResult>(e);
    if (!e2) return;

    const int idx = FindWatch(e2->code);
    if (idx < 0) {
        // 结果回来前该基金已被移出收藏（例如用户清空了列表）。
        // 不能再按旧下标访问 m_watch，但仍要维护「是否还有请求在跑」。
        bool anyLoading = false;
        for (size_t i = 0; i < m_watch.size(); ++i)
            if (m_watch[i].loading) anyLoading = true;
        if (!anyLoading) SetBusy(FALSE);
        return;
    }

    WatchFund &w = m_watch[idx];
    w.loading = false;
    w.loaded = true;
    if (e2->bOk) {
        w.rows.swap(e2->rows);
        w.err.clear();
    } else {
        w.err = e2->err;
        SLOGI() << "[watch] 加载失败 code=" << w.code.c_str() << " err=" << w.err.c_str();
    }
    if (e2->meta.ok) {
        w.meta = e2->meta;
        if (!e2->meta.name.empty()) w.name = e2->meta.name;
    }

    if (idx == m_cur) {
        m_rows = w.rows;
        m_meta = w.meta;
        FillGrid(m_rows);
        UpdateStatusSummary();
        SyncTabCounts();
        if (!e2->bOk) SetStatus(S_CA2T(e2->err.c_str(), CP_UTF8));
    }

    // 只要有结果回来就检查一次"还有没有人在加载"，全部结束才放开查询按钮。
    // 旧逻辑只在结果属于当前页签时才 SetBusy(FALSE)，切了页签会让按钮一直灰着，
    // 看起来就像界面卡死了。
    bool anyLoading = false;
    for (size_t i = 0; i < m_watch.size(); ++i)
        if (m_watch[i].loading) anyLoading = true;
    if (!anyLoading) SetBusy(FALSE);

    RefreshWatchTabs();
    RenderNeighborPages();  // 持仓回来后刷新当前页与相邻页，保证拖动快照是最新数据

    // 有新持仓后立刻拉一次行情，让该基金的估值出来
    if (e2->bOk) RefreshAllQuotes(false);

    // 继续加载下一只未加载的基金（串行，避免并发打爆接口）
    LoadNextPending();
}


// ------------------------------------------------------------- 行情/估值
void CMainDlg::RestartTimer() {
    if (m_timer) {
        ::KillTimer(m_hWnd, 1);
        m_timer = 0;
    }
    // 固定每分钟刷新一次；定时器同时驱动大盘指数与基金估值，
    // 所以即使收藏数为 0 也必须保留，用户随时可以点「刷新估值」手动触发。
    m_timer = ::SetTimer(m_hWnd, 1, kRefreshIntervalMs, NULL);
    UpdateBadge();
}

void CMainDlg::OnTimer(UINT_PTR idEvent) {
    if (idEvent == 2) {
        // 日志增量刷新：只在「请求日志」页签可见时干活，避免无谓的重排
        if (SWindow *p = FindWnd(R.ID.pane_log)) {
            if (p->IsVisible()) PumpLog(false);
        }
        return;
    }
    if (idEvent != 1) {
        SetMsgHandled(FALSE);
        return;
    }
    // 到点刷新大盘指数（与基金列表无关），再刷新行情并重算已打开基金的估值。
    // 净值是日频数据，只在手动刷新时重拉，避免每分钟多打 N 个接口。
    RefreshIndexes();
    RefreshAllQuotes(false);
    SetMsgHandled(FALSE);
}

void CMainDlg::OnRefreshAll() {
    RefreshIndexes();  // 大盘指数不受收藏数影响，手动刷新时一并更新

    if (m_watch.empty()) {
        RestartTimer();
        SetStatus(_T("暂无收藏基金 · 正在刷新大盘三大指数…"));
        return;
    }

    int pending = 0;
    for (size_t i = 0; i < m_watch.size(); ++i)
        if (!m_watch[i].loaded && !m_watch[i].loading) pending++;

    // 手动刷新应该把缺失的持仓也补齐
    if (pending > 0) LoadNextPending();

    RefreshAllQuotes(true);
    RestartTimer();  // 重新计时，避免刚点完按钮定时器立刻又触发

    if (pending > 0)
        SetStatus(_T("正在刷新估值：先补齐未加载的持仓，再拉取净值与行情…"));
    else
        SetStatus(_T("正在刷新估值（净值 + 行情）…"));
    SLOGI() << "[est] 手动刷新估值 基金数=" << (int)m_watch.size();
}

// 所有已打开基金的最新季度持仓代码并集：一次行情请求即可覆盖全部基金
std::string CMainDlg::BuildQuoteCodes() const {
    std::string csv;
    std::vector<std::string> seen;
    for (size_t wi = 0; wi < m_watch.size(); ++wi) {
        const WatchFund &w = m_watch[wi];
        if (!w.loaded || w.rows.empty()) continue;

        int q = 0;
        for (size_t i = 0; i < w.rows.size(); ++i)
            if (w.rows[i].lpQuarter > q) q = w.rows[i].lpQuarter;
        if (q <= 0) continue;

        for (size_t i = 0; i < w.rows.size(); ++i) {
            const HoldingChange &r = w.rows[i];
            if (!r.hasQ[q - 1]) continue;
            if (std::find(seen.begin(), seen.end(), r.code) != seen.end()) continue;
            seen.push_back(r.code);
            if (!csv.empty()) csv += ",";
            csv += r.code;
        }
    }
    return csv;
}

void CMainDlg::RefreshAllQuotes(bool withMeta) {
    if (m_bQuoteBusy || m_watch.empty() || !m_taskLoop) return;

    const std::string csv = BuildQuoteCodes();
    if (csv.empty()) return;

    std::string metaCsv;
    if (withMeta) {
        for (size_t i = 0; i < m_watch.size(); ++i) {
            if (!metaCsv.empty()) metaCsv += ",";
            metaCsv += m_watch[i].code;
        }
    }

    m_bQuoteBusy = TRUE;
    // 后台请求没有回投结果前，界面必须给出"正在干活"的反馈，
    // 否则网络慢的时候用户会以为界面卡住了。
    if (SWindow *p = FindWnd(R.ID.txt_upd)) p->SetWindowText(_T("刷新中…"));
    SFunctor3<CMainDlg, PFN_FetchMarket, std::string, std::string, bool> task(
        this, &CMainDlg::doFetchMarket, csv, metaCsv, m_bSimulated ? true : false);
    m_taskLoop->postTask(&task, FALSE, 0);
}

void CMainDlg::doFetchMarket(const std::string &codesCsv, const std::string &metaCsv,
                             bool simulated) {
    std::vector<std::string> codes = SplitCsv(codesCsv);
    std::vector<StockQuote> quotes;
    std::string err;
    bool quoteOk = false;
    if (simulated) {
        SimulateQuotes(codes, quotes);
        quoteOk = true;
    } else {
        quoteOk = fetchQuotes(m_http, codes, quotes, err);
    }

    std::vector<FundMeta> metas;
    std::vector<std::string> metaCodes = SplitCsv(metaCsv);
    for (size_t i = 0; i < metaCodes.size(); ++i) {
        FundMeta m;
        std::string merr;
        if (fetchFundMeta(m_http, metaCodes[i], m, merr)) metas.push_back(m);
    }

    EventMarketResult *evt = new EventMarketResult(NULL);
    evt->bOk = (quoteOk || !metas.empty()) ? TRUE : FALSE;
    evt->metas.swap(metas);
    evt->quotes.swap(quotes);
    evt->err = err;
    SNotifyCenter::getSingletonPtr()->FireEventAsync(evt);
    evt->Release();
}

void CMainDlg::OnMarketResult(EventArgs *e) {
    EventMarketResult *e2 = sobj_cast<EventMarketResult>(e);
    if (!e2) return;

    m_bQuoteBusy = FALSE;
    // 收藏在请求返回前被清空：结果已无对应页签，直接丢弃。
    if (m_watch.empty()) return;

    if (!e2->bOk) {
        if (e2->err.empty()) SetStatus(_T("估值刷新失败：行情接口无响应"));
        else SetStatus(S_CA2T(e2->err.c_str(), CP_UTF8));
        return;
    }

    // 1) 净值更新（手动刷新时会一并带上）
    for (size_t k = 0; k < e2->metas.size(); ++k) {
        const FundMeta &m = e2->metas[k];
        const int i = FindWatch(m.code);
        if (i < 0) continue;
        m_watch[i].meta = m;
        if (!m.name.empty()) m_watch[i].name = m.name;
    }

    // 2) 同一批行情，按各基金自己的权重分别算估值
    int okCount = 0;
    for (size_t wi = 0; wi < m_watch.size(); ++wi) {
        WatchFund &w = m_watch[wi];
        if (!w.loaded || w.rows.empty()) continue;
        FundEstimate est;
        if (computeEstimate(w.rows, e2->quotes, w.meta, est)) {
            w.est = est;
            okCount++;
        }
    }

    if (m_cur >= 0 && m_cur < (int)m_watch.size()) {
        m_meta = m_watch[m_cur].meta;
        m_est = m_watch[m_cur].est;
        UpdateEstimateUI(m_est, true);
        FillEstGrid(m_est);
        UpdateStatusSummary();
        SyncTabCounts();

        if (SWindow *p = FindWnd(R.ID.txt_fund_chip)) {
            SStringT s;
            s.Format(_T("%s  %s"), U8(m_watch[m_cur].code).c_str(),
                     m_watch[m_cur].name.empty() ? _T("—") : U8(m_watch[m_cur].name).c_str());
            p->SetWindowText(s);
        }
    }

    RefreshWatchTabs();

    // 状态栏留给持仓统计（信息量更大），刷新结果放到工具栏右侧的更新时间里
    if (!m_watch.empty() && !m_est.updatedAt.empty()) {
        SStringT upd;
        upd.Format(_T("更新于 %s · %d/%d 只"), U8(m_est.updatedAt).c_str(), okCount,
                   (int)m_watch.size());
        if (SWindow *p = FindWnd(R.ID.txt_upd)) p->SetWindowText(upd);
    }
    // 各基金估值重算后，把当前页与相邻页一起刷成最新数据
    RenderNeighborPages();
    // 日志面板由定时器按增量刷新，这里不再主动读文件/整篇重填
}

// ------------------------------------------------------------- 大盘三大指数
// 与基金估值完全解耦：单独一条后台任务 + 单独一个回投事件，收藏数为 0 时照常跑。
void CMainDlg::RefreshIndexes() {
    if (m_bIndexBusy || !m_taskLoop) return;
    m_bIndexBusy = TRUE;
    SFunctor0<CMainDlg, PFN_FetchIndexes> task(this, &CMainDlg::doFetchIndexes);
    m_taskLoop->postTask(&task, FALSE, 0);
}

void CMainDlg::doFetchIndexes() {
    std::vector<IndexQuote> idx;
    std::string err;
    const bool ok = fetchMajorIndexes(m_http, idx, err);

    EventIndexResult *evt = new EventIndexResult(NULL);
    evt->bOk = ok ? TRUE : FALSE;
    evt->indexes.swap(idx);
    evt->err = err;
    SNotifyCenter::getSingletonPtr()->FireEventAsync(evt);
    evt->Release();
}

void CMainDlg::OnIndexResult(EventArgs *e) {
    EventIndexResult *e2 = sobj_cast<EventIndexResult>(e);
    if (!e2) return;

    m_bIndexBusy = FALSE;
    if (e2->bOk) {
        m_indexes = e2->indexes;
        const SStringT now = ClockHms();
        m_indexNote.Format(_T("东财行情 · 更新于 %s"), now.c_str());
    } else {
        // 请求失败时保留上一次成功的数值，只把右侧说明换成失败提示，
        // 避免指数条每分钟闪一次占位符。
        m_indexNote = _T("大盘行情暂不可用 · 稍后自动重试");
        if (!e2->err.empty()) SLOGI() << "[index] " << e2->err.c_str();
    }
    UpdateIndexBar();
}


void CMainDlg::UpdateBadge() {
    EMarketSession s = m_bSimulated ? MS_SIMULATED : detectMarketSession(std::time(NULL));
    bool live = (s == MS_TRADING || s == MS_SIMULATED);

    SWindow *pLive = FindWnd(R.ID.txt_market);
    SWindow *pOff = FindWnd(R.ID.txt_market_off);
    if (pLive) {
        pLive->SetWindowText(marketSessionLabel(s));
        pLive->SetVisible(live);
    }
    if (pOff) {
        pOff->SetWindowText(marketSessionLabel(s));
        pOff->SetVisible(!live);
    }
}

void CMainDlg::UpdateStatusSummary() {
    if (m_watch.empty()) {
        SetStatus(_T("暂无收藏基金 · 大盘三大指数每分钟自动刷新 · 填入代码点「＋ 收藏」开始"));
        return;
    }

    if (m_rows.empty()) {
        // 空表（新加的自选还没加载完）也要给状态栏一个可读的提示
        SStringT msg;
        msg.Format(_T("自选 %d 只 · 当前基金 %s 暂无持仓数据（等待加载中…）"),
                   (int)m_watch.size(),
                   (m_cur >= 0 && m_cur < (int)m_watch.size()) ? U8(m_watch[m_cur].code).c_str()
                                                               : _T("—"));
        SetStatus(msg);
        return;
    }

    int cQ[3] = {0, 0, 0}, nNew = 0, nExit = 0;
    for (size_t ri = 0; ri < m_rows.size(); ++ri) {
        const HoldingChange &d = m_rows[ri];
        for (int q = 0; q < 3; ++q)
            if (d.hasQ[q]) cQ[q]++;
        // 直接按季度持仓位判定，避免解析 tag 字符串时把「Q2新进/Q3退出」重复计数
        if (d.hasQ[2] && !d.hasQ[1]) nNew++;
        if (d.hasQ[1] && !d.hasQ[2]) nExit++;
    }

    int qWeight = m_est.quarter;
    if (qWeight <= 0)
        for (int q = 2; q >= 0; --q)
            if (cQ[q] > 0) { qWeight = q + 1; break; }

    // Q2 为半年报口径（披露全部持仓），Q1/Q3 为季报口径（仅前十大），
    // 因此 Q2 的只数天然多于 Q1/Q3，「退出」数按 Q2->Q3 直接相减会偏大，需在界面上标注清楚。
    SStringT msg;
    msg.Format(_T("自选%d · %s：%d只 · Q1=%d Q2=%d(半年报全量) Q3=%d · 新进%d/退出%d · 口径2026Q%d"),
               (int)m_watch.size(),
               (m_cur >= 0 && m_cur < (int)m_watch.size()) ? U8(m_watch[m_cur].code).c_str()
                                                           : _T("—"),
               (int)m_rows.size(), cQ[0], cQ[1], cQ[2], nNew, nExit, qWeight);
    SetStatus(msg);
}

// ------------------------------------------------------------- KPI / 图表
void CMainDlg::UpdateEstimateUI(const FundEstimate &est, bool bAppend) {
    const COLORREF clr = est.estPct > 0 ? kPos : (est.estPct < 0 ? kNeg : kText);
    const SStringT dash = _T("—");

    // 主卡：估算涨跌幅
    if (SFundKpiCard *p = FindCtrl<SFundKpiCard>(R.ID.kpi_hero)) {
        SStringT pill;
        if (m_bSimulated) pill = _T("模拟盘中");
        else pill = marketSessionLabel(detectMarketSession(std::time(NULL)));
        SStringT sub;
        if (est.valid) {
            sub.Format(_T("口径 Σ(占净值×涨跌幅) · 覆盖 %.1f%%"), est.coverPct);
            p->SetContent(_T("盘中估算涨跌幅"), formatDelta(est.estPct, 2), _T("%"), sub, pill, clr);
        } else {
            p->SetContent(_T("盘中估算涨跌幅"), dash, _T(""), _T("等待行情"), pill, kText3);
        }
    }

    // 估算净值
    if (SFundKpiCard *p = FindCtrl<SFundKpiCard>(R.ID.kpi_nav)) {
        SStringT v = dash, sub;
        if (est.estNav > 0) {
            v = formatNumber(est.estNav, 4);
            sub.Format(_T("最新净值 %s（%s）"), formatNumber(est.lastNav, 4).c_str(),
                       U8(est.navDate).c_str());
        } else {
            sub = _T("净值接口未返回");
        }
        p->SetContent(_T("估算净值"), v, _T(""), sub, _T(""), kText);
    }

    // 持仓覆盖度（圆环）：百分比只画在圆环内，卡片左侧不再重复同一数值，
    // 说明文字也只由 SetRing 的 note 提供，避免两个「94%」和重复说明相互压叠。
    if (SFundKpiCard *p = FindCtrl<SFundKpiCard>(R.ID.kpi_cover)) {
        if (est.valid) {
            SStringT center, note;
            center.Format(_T("%.0f%%"), est.coverPct);
            note.Format(_T("权重股 %d 只\n现金及其他 %.1f%%"), est.holdCount,
                        100.0 - est.coverPct);
            p->SetRing((float)(est.coverPct / 100.0), center, note);
            p->SetContent(_T("持仓覆盖度"), _T(""), _T(""), _T(""), _T(""), kText);
        } else {
            p->SetRing(0.f, dash, _T(""));
            p->SetContent(_T("持仓覆盖度"), _T(""), _T(""), _T("等待行情"), _T(""), kText3);
        }
    }

    // 最大拉升 / 最大拖累
    const int kpiIds[2] = {R.ID.kpi_pull, R.ID.kpi_drag};
    for (int k = 0; k < 2; ++k) {
        int idx = (k == 0) ? est.bestIdx : est.worstIdx;
        SFundKpiCard *p = FindCtrl<SFundKpiCard>(kpiIds[k]);
        if (!p) continue;
        if (est.valid && idx >= 0 && idx < (int)est.items.size()) {
            const ContributionItem &it = est.items[idx];
            // 行情源偶尔只给代码不给名称，回退显示代码，避免主数值整块空白。
            SStringT v = it.name.empty() ? U8(it.code) : U8(it.name);
            SStringT sub;
            sub.Format(_T("%s%% · 贡献 %spp"), formatDelta(it.chgPct, 2).c_str(),
                       formatDelta(it.contrib, 3).c_str());
            p->SetContent(k == 0 ? _T("最大拉升 · 贡献") : _T("最大拖累 · 贡献"), v, _T(""), sub,
                          _T(""), it.contrib >= 0 ? kPos : kNeg);
        } else {
            p->SetContent(k == 0 ? _T("最大拉升 · 贡献") : _T("最大拖累 · 贡献"), dash, _T(""),
                          _T("等待行情"), _T(""), kText3);
        }
    }

    // 走势图：每次行情刷新追加一个采样点（切页签重绘时不追加，避免序列被污染）
    if (est.valid && bAppend) {
        if (WatchFund *w = CurFund()) {
            w->series.push_back(est.estPct);
            if (w->series.size() > 241) w->series.erase(w->series.begin());
            if (SFundSpark *p = FindCtrl<SFundSpark>(R.ID.chart_est)) p->SetSeries(w->series);
        }
    }

    // 贡献度分解
    if (SFundContrib *p = FindCtrl<SFundContrib>(R.ID.contrib_list)) p->SetItems(est.items);

    // 走势图右上角说明与更新时间
    {
        char dateBuf[32] = {0};
        std::time_t t = std::time(NULL);
        std::tm tmv;
#if defined(_WIN32)
        localtime_s(&tmv, &t);
#else
        localtime_r(&t, &tmv);
#endif
        fund_snprintf(dateBuf, sizeof(dateBuf), "%04d-%02d-%02d", tmv.tm_year + 1900,
                      tmv.tm_mon + 1, tmv.tm_mday);
        SStringT sub;
        sub.Format(_T("%s · %s"), U8(std::string(dateBuf)).c_str(),
                   m_bSimulated ? _T("模拟盘中") : marketSessionLabel(detectMarketSession(t)));
        if (SWindow *p = FindWnd(R.ID.txt_chart_sub)) p->SetWindowText(sub);

        SStringT upd;
        if (!est.updatedAt.empty()) {
            upd.Format(_T("更新于 %s"), U8(est.updatedAt).c_str());
            if (SWindow *p = FindWnd(R.ID.txt_upd)) p->SetWindowText(upd);
        }
    }
}

// ------------------------------------------------------------- 表格填充
// 单元格内边距（dp）：宽屏保留 6dp 观感；手机竖屏压到 2dp，
// 否则左右共 12dp 会吃掉数值列最后一位（如代码 600519 只显示 60051）。
static int CellMarginDp(int parentWidth, int sc) {
    const int dpi = sc > 0 ? sc : 100;
    return parentWidth < 600 * dpi / 100 ? 2 : 6;
}

// 批量填表时统一设置单元格文本/对齐/边距/底色。
// 原先这里是就地写的 lambda，VS2008 不支持 lambda，改为文件级函数。
static void SetCellText(SGridCtrl *pGrid, int row, int c, const SStringT &s, DWORD fmt,
                        COLORREF clr, int margin) {
    pGrid->SetItemText(row, c, s);
    IGridCell *cell = pGrid->GetCell(row, c);
    if (!cell) return;
    cell->SetFormat(fmt);
    cell->SetMargin(margin);
    cell->SetBackClr((row & 1) ? kRowBg2 : kRowBg);
    cell->SetTextClr(clr ? clr : kText2);
}

// ------------------------------------------------- XML 列布局驱动
// 行高/列宽/表头样式一次性应用到 grid。列定义已由 SGridCtrl 从 <columns>
// 子节点解析好，这里只按 DPI 和窗口宽度计算实际尺寸。
// 固定宽度按 DPI 缩放；权重列按权重分摊父窗口可用宽度。
static void ApplyGridColumns(SGridCtrl *pGrid, int parentWidth, int sc, int rowCount) {
    int rowH = pGrid->GetDefCellHeight() * sc / 100;
    if (rowH <= 0) rowH = 30 * sc / 100;
    for (int r = 0; r < rowCount; ++r) pGrid->SetRowHeight(r, rowH);

    const int nCols = pGrid->GetColumnCount();
    int totalWeight = 0;
    for (int c = 0; c < nCols; ++c) totalWeight += pGrid->GetColumnWeight(c);

    int avail = 0;
    if (totalWeight > 0) {
        avail = parentWidth - 100;  // 扣掉卡片 margin/padding 与垂直滚动条
        if (avail < 400) {
            // 窄屏（手机竖屏）：100dp 的预留太大，按实际客户区宽度分配；
            // 仍是同一套权重算法，桌面宽度下走上面的分支，行为不变。
            // 68dp = 两侧卡片 margin(12+12) + padding(8+8) + 网格内滚动条与边框；
            // 必须随 DPI 缩放，否则 125% 缩放下会多留几像素并出现横向滚动条。
            avail = parentWidth - 68 * sc / 100;
            if (avail < 100) avail = 1340;  // 首次布局宽度尚未就绪时的兜底
        }
    }

    int used = 0;
    for (int c = 0; c < nCols; ++c) {
        int w = 0;
        int weight = pGrid->GetColumnWeight(c);
        if (weight > 0) {
            if (c == nCols - 1)
                w = avail - used;
            else
                w = avail * weight / totalWeight;
            if (w < 40 * sc / 100) w = 40 * sc / 100;  // 手机竖屏列多、单列窄，下限随 DPI 缩放
        } else {
            w = pGrid->GetColumnDefWidth(c) * sc / 100;
        }
        pGrid->SetColumnWidth(c, w);
        used += w;

        // 表头文字/对齐已由控件解析，这里只补应用层的配色与边距。
        IGridCell *hc = pGrid->GetCell(0, c);
        if (!hc) continue;
        hc->SetMargin(CellMarginDp(parentWidth, sc));
        hc->SetTextClr(kText3);
        hc->SetBackClr(kHeadBg);
    }
}

// q1_xxx / q2_xxx / q3_xxx 中的季度下标；不是这类 key 返回 -1。
static int QuarterFromKey(const SStringW &key) {
    if (key.GetLength() < 4 || key[0] != L'q') return -1;
    wchar_t q = key[1];
    if (q < L'1' || q > L'3') return -1;
    return (int)(q - L'1');
}

static BOOL DeltaCell(BOOL bValid, double v, int dec, const SStringT &dash, SStringT *out,
                      COLORREF *clr) {
    if (!bValid) {
        *out = dash;
        *clr = kDim;
    } else {
        *out = formatDelta(v, dec);
        *clr = deltaColor(v);
    }
    return TRUE;
}

// 按 XML key 生成持仓变化表的单元格；返回 FALSE 表示 XML 中出现了未知列。
static BOOL HoldingCellValue(const HoldingChange &d, const SStringW &key, const SStringT &dash,
                             SStringT *out, COLORREF *clr) {
    *clr = 0;
    int q = QuarterFromKey(key);
    if (q >= 0) {
        if (key.EndsWith(L"_pct"))
            *out = cellNum(d.pct[q], d.hasQ[q], 2, dash);
        else if (key.EndsWith(L"_sh"))
            *out = cellNum(d.shares[q], d.hasQ[q], 2, dash);
        else if (key.EndsWith(L"_mv"))
            *out = cellNum(d.mv[q], d.hasQ[q], 0, dash);
        else
            return FALSE;
        return TRUE;
    }

    if (key == L"lp_pct") {
        *out = (d.lpQuarter > 0)
                   ? cellNum(d.pct[d.lpQuarter - 1], d.hasQ[d.lpQuarter - 1], 2, dash)
                   : dash;
        *clr = kAccent;
    } else if (key == L"code") {
        *out = U8(d.code);
        *clr = kText3;
    } else if (key == L"name") {
        *out = U8(d.name);
        *clr = kText;
    } else if (key == L"pct_chg") {
        return DeltaCell(d.chgValid, d.pctChg, 2, dash, out, clr);
    } else if (key == L"sh_chg") {
        return DeltaCell(d.chgValid, d.shChg, 2, dash, out, clr);
    } else if (key == L"mv_chg") {
        return DeltaCell(d.chgValid, d.mvChg, 0, dash, out, clr);
    } else if (key == L"quarters") {
        *out = S_CA2T(d.status.c_str(), CP_UTF8);
        *clr = kText2;
    } else if (key == L"tag") {
        *out = d.tag.empty() ? dash : S_CA2T(d.tag.c_str(), CP_UTF8);
        *clr = d.tag.empty() ? kDim : kAccent;
    } else {
        return FALSE;
    }
    return TRUE;
}

// 按 XML key 生成估值明细的单元格。
static BOOL EstCellValue(const ContributionItem &it, const SStringW &key, const SStringT &dash,
                         SStringT *out, COLORREF *clr) {
    *clr = 0;
    if (key == L"code") {
        *out = U8(it.code);
        *clr = kText3;
    } else if (key == L"name") {
        *out = U8(it.name);
        *clr = kText;
    } else if (key == L"weight") {
        *out = formatNumber(it.weight, 2);
    } else if (key == L"price") {
        *out = it.quoted ? formatNumber(it.price, 2) : dash;
        *clr = kDim;
    } else if (key == L"chg_pct") {
        return DeltaCell(TRUE, it.chgPct, 2, dash, out, clr);
    } else if (key == L"contrib") {
        return DeltaCell(TRUE, it.contrib, 3, dash, out, clr);
    } else {
        return FALSE;
    }
    return TRUE;
}

// 估值明细的合计行：同一批 key，但取值来自整只基金的汇总数据。
static BOOL EstSumCellValue(const FundEstimate &est, const SStringW &key, SStringT *out,
                            COLORREF *clr) {
    *clr = 0;
    if (key == L"code") {
        *out = _T("合计");
        *clr = kText2;
    } else if (key == L"weight") {
        *out = formatNumber(est.coverPct, 2);
        *clr = kAccent;
    } else if (key == L"contrib") {
        *out = formatDelta(est.estPct, 3);
        *clr = deltaColor(est.estPct);
    } else if (key == L"name" || key == L"price" || key == L"chg_pct") {
        *out = _T("");
    } else {
        return FALSE;
    }
    return TRUE;
}

void CMainDlg::FillGrid(const std::vector<HoldingChange> &list) {
    SGridCtrl *pGrid = FindCtrl<SGridCtrl>(R.ID.grid_holdings);
    if (!pGrid) return;

    // 列集合来自 XML 的 <columns> 子节点：移动版可只声明 7 列，这段逻辑不用改。
    const int nCols = pGrid->GetColumnCount();
    if (nCols <= 0) return;

    // 上百行 × 十几列会产生上千次 SetItemText/GetCell，若每次都触发重绘，
    // 在 UI 线程上就是"点一下卡半秒"。这里先锁住刷新，填完统一重绘一次。
    pGrid->LockUpdate();

    const int rows = (int)list.size() + 1;
    pGrid->SetRowCount(rows);
    pGrid->SetColumnCount(nCols);
    pGrid->SetFixedRowCount(1);
    pGrid->SetFixedColumnCount(0);

    const int sc = GetScale();
    const int parentW = GetClientRect().Width();
    ApplyGridColumns(pGrid, parentW, sc, rows);
    const int cellMargin = CellMarginDp(parentW, sc);

    const SStringT dash = _T("—");

    // 注意：SGridCell 默认背景是纯白，必须为每个单元格显式设置背景色，
    // 否则浅色文字会落在白底上（视觉上像半透明残影）。
    for (int r = 0; r < (int)list.size(); ++r) {
        int row = r + 1;
        const HoldingChange &d = list[r];
        for (int c = 0; c < nCols; ++c) {
            SStringT s;
            COLORREF clr = 0;
            if (!HoldingCellValue(d, pGrid->GetColumnKey(c), dash, &s, &clr)) continue;
            SetCellText(pGrid, row, c, s, pGrid->GetColumnAlign(c) | DT_VCENTER, clr, cellMargin);
        }
    }

    pGrid->UnlockUpdate();
    pGrid->Invalidate();
}

void CMainDlg::FillEstGrid(const FundEstimate &est) {
    SGridCtrl *pGrid = FindCtrl<SGridCtrl>(R.ID.grid_est);
    if (!pGrid) return;

    const int nCols = pGrid->GetColumnCount();
    if (nCols <= 0) return;

    pGrid->LockUpdate();  // 同上：批量填完再统一重绘

    int n = est.valid ? (int)est.items.size() : 0;
    pGrid->SetRowCount(n + 2);  // 表头 + 明细 + 合计
    pGrid->SetColumnCount(nCols);
    pGrid->SetFixedRowCount(1);
    pGrid->SetFixedColumnCount(0);

    const int sc = GetScale();
    const int parentW = GetClientRect().Width();
    ApplyGridColumns(pGrid, parentW, sc, n + 2);
    const int cellMargin = CellMarginDp(parentW, sc);

    const SStringT dash = _T("—");

    for (int i = 0; i < n; ++i) {
        int row = i + 1;
        for (int c = 0; c < nCols; ++c) {
            SStringT s;
            COLORREF clr = 0;
            if (!EstCellValue(est.items[i], pGrid->GetColumnKey(c), dash, &s, &clr)) continue;
            SetCellText(pGrid, row, c, s, pGrid->GetColumnAlign(c) | DT_VCENTER, clr, cellMargin);
        }
    }

    // 合计行
    int sumRow = n + 1;
    for (int c = 0; c < nCols; ++c) {
        SStringT s;
        COLORREF clr = 0;
        if (!EstSumCellValue(est, pGrid->GetColumnKey(c), &s, &clr)) continue;
        SetCellText(pGrid, sumRow, c, s, pGrid->GetColumnAlign(c) | DT_VCENTER, clr, cellMargin);
    }

    pGrid->UnlockUpdate();
    pGrid->Invalidate();
}

// ------------------------------------------------------------- 筛选 / 排序
void CMainDlg::OnFilter() {
    if (m_rows.empty()) return;

    SEdit *pEdit = FindCtrl<SEdit>(R.ID.edit_filter);
    SStringT key = pEdit ? pEdit->GetWindowText() : _T("");
    key.TrimBlank();

    m_view.clear();
    if (key.IsEmpty()) {
        m_view = m_rows;
    } else {
        for (size_t ri = 0; ri < m_rows.size(); ++ri) {
            const HoldingChange &d = m_rows[ri];
            SStringT name = U8(d.name);
            SStringT code = U8(d.code);
            if (name.Find(key) >= 0 || code.Find(key) >= 0) m_view.push_back(d);
        }
    }
    FillGrid(m_view);

    SStringT msg;
    msg.Format(_T("筛选「%s」：命中 %d / %d 只"), key.c_str(), (int)m_view.size(),
               (int)m_rows.size());
    SetStatus(msg);
}

void CMainDlg::OnResetSort() {
    if (SEdit *p = FindCtrl<SEdit>(R.ID.edit_filter)) p->SetWindowText(_T(""));
    m_view = m_rows;
    FillGrid(m_view);
    UpdateStatusSummary();
}

// ------------------------------------------------------------- 模拟盘中
void CMainDlg::OnSimOn() {
    m_bSimulated = TRUE;
    if (SWindow *p = FindWnd(R.ID.btn_sim_off)) p->SetVisible(FALSE);
    if (SWindow *p = FindWnd(R.ID.btn_sim_on)) p->SetVisible(TRUE);
    RestartTimer();
    RefreshAllQuotes(false);
    UpdateBadge();
    SetStatus(_T("已开启模拟盘中：使用随机行情演示估值与走势（非真实数据）"));
}

void CMainDlg::OnSimOff() {
    m_bSimulated = FALSE;
    if (SWindow *p = FindWnd(R.ID.btn_sim_off)) p->SetVisible(TRUE);
    if (SWindow *p = FindWnd(R.ID.btn_sim_on)) p->SetVisible(FALSE);
    RestartTimer();
    RefreshAllQuotes(false);
    UpdateBadge();
    UpdateStatusSummary();
}
