// MainDlg.cpp : 基金持仓查看器主对话框
#include <souistd.h>
#include <SGridCtrl.h>
#include <cmath>
#include "MainDlg.h"
#include "fund_eastmoney.h"

using namespace SOUI;

#define kLogTag "fundholdings"

// 基金代码/年份均为 ASCII 数字，安全转 std::string
static std::string toAscii(const SStringT &s) {
    std::string out;
    out.reserve(s.GetLength());
    for (int i = 0; i < s.GetLength(); ++i) out += (char)s[i];
    return out;
}

// ---------- 变化表渲染辅助（与 HTML 变化表对齐）----------
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

// 数值格式化（带千分位），dec 为小数位；负数自动加负号
SStringT formatNumber(double v, int dec) {
    if (v < 0) return _T("-") + formatNumber(-v, dec);
    double scale = std::pow(10.0, dec);
    double r = std::floor(v * scale + 0.5) / scale;
    long long ip = (long long)r;
    SStringT ipStr = addCommas(SStringT(std::to_wstring(ip).c_str()));
    if (dec > 0) {
        double frac = r - (double)ip;
        long long f = (long long)(frac * scale + 0.5);
        if (f >= (long long)scale) f = (long long)scale - 1;  // 防止舍入进位溢出
        SStringT fStr = std::to_wstring(f).c_str();
        while (fStr.GetLength() < dec) fStr = _T("0") + fStr;
        return ipStr + _T(".") + fStr;
    }
    return ipStr;
}

// 变化量格式化：正数前加 “+”（A 股红涨绿跌，正值用红色）
SStringT formatDelta(double v, int dec) {
    if (v > 0) return _T("+") + formatNumber(v, dec);
    return formatNumber(v, dec);
}

// 单元格数值：未持有（has==false）显示 “—”
SStringT cellNum(double v, bool has, int dec, const SStringT &dash) {
    if (!has) return dash;
    return formatNumber(v, dec);
}
}  // namespace

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:dlg_main")) {
}

CMainDlg::~CMainDlg() {
    if (m_taskLoop) {
        m_taskLoop->cancelTasksForObject(this);
        m_taskLoop->stop();
        m_taskLoop = NULL;
    }
}

int CMainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    SetMsgHandled(FALSE);
    return 0;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam) {
    // 创建并启动后台任务循环（工作线程），用于异步抓取数据，避免阻塞 UI
    m_comMgr.CreateTaskLoop((IObjRef **)&m_taskLoop);
    if (m_taskLoop) m_taskLoop->start("fund_fetcher", Normal);

    // 注册结果事件，使其可被 SNotifyCenter 跨线程投递
    SNotifyCenter::getSingletonPtr()->addEvent(EVENTID(EventQueryResult));

    SetStatus(_T("就绪：输入基金代码与年份后点击「查询」"));
    return TRUE;
}

void CMainDlg::OnClose() {
    DestroyWindow();
}

void CMainDlg::OnMaximize() { SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE); }
void CMainDlg::OnRestore() { SendMessage(WM_SYSCOMMAND, SC_RESTORE); }
void CMainDlg::OnMinimize() { SendMessage(WM_SYSCOMMAND, SC_MINIMIZE); }

void CMainDlg::OnSize(UINT nType, CSize size) {
    if (nType == SIZE_MAXIMIZED) {
        if (auto *p = FindChildByName2<SWindow>(L"btn_max")) p->SetVisible(FALSE);
        if (auto *p = FindChildByName2<SWindow>(L"btn_restore")) p->SetVisible(TRUE);
    } else {
        if (auto *p = FindChildByName2<SWindow>(L"btn_max")) p->SetVisible(TRUE);
        if (auto *p = FindChildByName2<SWindow>(L"btn_restore")) p->SetVisible(FALSE);
    }
    SetMsgHandled(FALSE);
}

void CMainDlg::SetStatus(const SStringT &s) {
    if (auto *p = FindChildByName(L"txt_status")) p->SetWindowText(s);
}

void CMainDlg::SetBusy(BOOL bBusy) {
    m_bBusy = bBusy;
    if (auto *p = FindChildByName2<SWindow>(L"btn_query")) p->EnableWindow(!bBusy);
}

void CMainDlg::OnQuery() {
    if (m_bBusy) return;  // 正在抓取，忽略重复点击

    SEdit *pCode = FindChildByName2<SEdit>(L"edit_code");
    SEdit *pYear = FindChildByName2<SEdit>(L"edit_year");
    SStringT code = pCode ? pCode->GetWindowText() : _T("161725");
    SStringT year = pYear ? pYear->GetWindowText() : _T("2026");

    SetStatus(_T("正在从天天基金网请求数据..."));
    SetBusy(TRUE);

    // 把抓取任务投递到后台 TaskLoop 工作线程；完成后 doFetch 通过
    // SNotifyCenter::FireEventAsync 把结果事件回投到 UI 线程处理。
    if (m_taskLoop) {
        SFunctor2<CMainDlg, decltype(&CMainDlg::doFetch), std::string, std::string>
            task(this, &CMainDlg::doFetch, toAscii(code), toAscii(year));
        m_taskLoop->postTask(&task, FALSE, 0);
    } else {
        // 兜底：无后台循环时同步执行（理论上不会发生）
        SetBusy(FALSE);
        SetStatus(_T("错误：后台任务循环未初始化"));
    }
}

void CMainDlg::doFetch(const std::string &code, const std::string &year) {
    // 此函数在后台工作线程中执行，不触碰任何 UI 对象
    std::vector<FundHolding> list;
    std::string err;
    bool ok = fetchFundHoldings(code, year, list, err);

    // 构造结果事件，堆分配，随 FireEventAsync 投递到 UI 线程
    EventQueryResult *evt = new EventQueryResult(NULL);
    evt->bOk = ok ? TRUE : FALSE;
    if (ok) {
        evt->list = std::move(list);
    } else {
        evt->err = std::move(err);
    }
    SNotifyCenter::getSingletonPtr()->FireEventAsync(evt);
    evt->Release();
}

void CMainDlg::OnQueryResult(EventArgs *e) {
    EventQueryResult *e2 = sobj_cast<EventQueryResult>(e);
    if (!e2) return;

    SetBusy(FALSE);

    if (!e2->bOk) {
        FillGrid({});
        SetStatus(S_CA2T(e2->err.c_str(), CP_UTF8));
        return;
    }

    // 把扁平持仓列表转换为按股票透视的持仓变化表（与 gen_change_table.py 对齐）
    std::vector<HoldingChange> rows;
    buildHoldingsChange(e2->list, rows);
    FillGrid(rows);

    // 汇总（对应 HTML 的摘要卡片：Q1/Q2/Q3 持仓数、Q3 新进/退出）
    int cQ[3] = {0, 0, 0}, nNew = 0, nExit = 0;
    for (const auto &d : rows) {
        for (int q = 0; q < 3; ++q) if (d.hasQ[q]) cQ[q]++;
        if (d.tag.find("Q3新进") != std::string::npos) nNew++;
        if (d.tag.find("Q3退出") != std::string::npos) nExit++;
    }
    SStringT msg;
    msg.Format(_T("共 %d 只股票（去重）· Q1=%d Q2=%d Q3=%d · Q3新进 %d / 退出 %d"),
               (int)rows.size(), cQ[0], cQ[1], cQ[2], nNew, nExit);
    SetStatus(msg);
}

void CMainDlg::FillGrid(const std::vector<HoldingChange> &list) {
    SGridCtrl *pGrid = FindChildByName2<SGridCtrl>(L"grid_holdings");
    if (!pGrid) return;

    // 列定义与 gen_change_table.py 生成的 HTML 变化表严格对齐（共 17 列）
    const int cols = 17;
    static const SStringT hdr[cols] = {
        _T("最新占净值%"), _T("代码"), _T("名称"),
        _T("Q1占净值%"), _T("Q2占净值%"), _T("Q3占净值%"),
        _T("Q1持股(万)"), _T("Q2持股(万)"), _T("Q3持股(万)"),
        _T("Q1市值(万)"), _T("Q2市值(万)"), _T("Q3市值(万)"),
        _T("占净值Δ(Q3-Q1)"), _T("持股Δ(Q3-Q1)"), _T("市值Δ(Q3-Q1)"),
        _T("持有季度"), _T("变动标记")
    };
    static const int cw[cols] = {
        90, 80, 100,
        90, 90, 90,
        95, 95, 95,
        100, 100, 100,
        110, 110, 110,
        80, 120
    };

    int rows = (int)list.size() + 1;  // + 表头
    pGrid->SetRowCount(rows);
    pGrid->SetColumnCount(cols);
    pGrid->SetFixedRowCount(1);
    pGrid->SetFixedColumnCount(0);  // 无冻结列，整体横向滚动，与 HTML 一致

    for (int c = 0; c < cols; ++c) {
        pGrid->SetItemText(0, c, hdr[c]);
        pGrid->SetColumnWidth(c, cw[c]);
    }

    // A 股惯例：涨=红、跌=绿；“—”用灰色
    const COLORREF clrPos = RGBA(0xC0, 0x39, 0x2B, 0xFF);  // 红（升）
    const COLORREF clrNeg = RGBA(0x1E, 0x8E, 0x3E, 0xFF);  // 绿（降）
    const COLORREF clrDim = RGBA(0x6B, 0x77, 0x85, 0xFF);  // 灰（缺失/无变化）
    const DWORD fmtR = DT_RIGHT | DT_VCENTER;
    const DWORD fmtL = DT_LEFT | DT_VCENTER;
    const SStringT dash = _T("—");

    auto set = [&](int row, int c, const SStringT &s, DWORD fmt, COLORREF clr) {
        pGrid->SetItemText(row, c, s);
        IGridCell *cell = pGrid->GetCell(row, c);
        if (cell) {
            cell->SetFormat(fmt);
            if (clr) cell->SetTextClr(clr);
        }
    };

    for (int r = 0; r < (int)list.size(); ++r) {
        int row = r + 1;
        const HoldingChange &d = list[r];

        // 最新占净值%（取最新存在的季度）
        SStringT sLp = (d.lpQuarter > 0)
            ? cellNum(d.pct[d.lpQuarter - 1], d.hasQ[d.lpQuarter - 1], 2, dash)
            : dash;
        set(row, 0, sLp, fmtR, clrDim);
        set(row, 1, U8(d.code), fmtL, 0);
        set(row, 2, U8(d.name), fmtL, 0);

        // Q1/Q2/Q3 占净值% / 持股(万) / 市值(万)
        for (int q = 0; q < 3; ++q) set(row, 3 + q, cellNum(d.pct[q], d.hasQ[q], 2, dash), fmtR, 0);
        for (int q = 0; q < 3; ++q) set(row, 6 + q, cellNum(d.shares[q], d.hasQ[q], 2, dash), fmtR, 0);
        for (int q = 0; q < 3; ++q) set(row, 9 + q, cellNum(d.mv[q], d.hasQ[q], 0, dash), fmtR, 0);

        // Δ(Q3-Q1)：仅当 Q3 与 Q1 同时存在时有效
        if (d.chgValid) {
            set(row, 12, formatDelta(d.pctChg, 2), fmtR, d.pctChg > 0 ? clrPos : (d.pctChg < 0 ? clrNeg : 0));
            set(row, 13, formatDelta(d.shChg, 2), fmtR, d.shChg > 0 ? clrPos : (d.shChg < 0 ? clrNeg : 0));
            set(row, 14, formatDelta(d.mvChg, 0), fmtR, d.mvChg > 0 ? clrPos : (d.mvChg < 0 ? clrNeg : 0));
        } else {
            for (int c = 12; c <= 14; ++c) set(row, c, dash, fmtR, clrDim);
        }

        // 持有季度（✓/—）与变动标记
        set(row, 15, S_CA2T(d.status.c_str(), CP_UTF8), fmtL, 0);
        set(row, 16, d.tag.empty() ? dash : S_CA2T(d.tag.c_str(),CP_ACP), fmtL, d.tag.empty() ? clrDim : 0);
    }

    // 表头对齐：代码/名称/持有季度/变动标记 左对齐，其余右对齐
    for (int c = 0; c < cols; ++c) {
        IGridCell *hc = pGrid->GetCell(0, c);
        if (hc) hc->SetFormat((c == 1 || c == 2 || c == 15 || c == 16) ? fmtL : fmtR);
    }
}
