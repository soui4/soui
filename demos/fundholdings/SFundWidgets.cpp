// SFundWidgets.cpp : 暗色自绘控件实现
#include "stdafx.h"
#include "SFundWidgets.h"

#define kLogTag "fundui"

SNSBEGIN

// ------------------------------------------------------------------ 调色板
// 与 ui_mockup.html 中的 CSS 变量一一对应（涨=红、跌=绿，A 股惯例）
static const COLORREF kCard = RGBA(0x12, 0x1a, 0x26, 0xFF);
static const COLORREF kLine = RGBA(0x21, 0x2b, 0x3a, 0xFF);
static const COLORREF kText = RGBA(0xe8, 0xed, 0xf5, 0xFF);
static const COLORREF kText2 = RGBA(0xaa, 0xb6, 0xc8, 0xFF);
static const COLORREF kText3 = RGBA(0x77, 0x84, 0x9a, 0xFF);
static const COLORREF kUp = RGBA(0xff, 0x5c, 0x5c, 0xFF);
static const COLORREF kDown = RGBA(0x22, 0xc5, 0x5e, 0xFF);
static const COLORREF kAccent = RGBA(0xf0, 0xb4, 0x29, 0xFF);

// 取字体（按窗口 DPI 缩放）；字体池内部按描述串缓存，重复调用开销很小。
// 字体族和字号统一在 uires/values/font.xml 里以命名字体（@font/xxx）定义，
// 这里只按「设计字号 + 是否加粗」挑名字，不再硬编码 face。
// 新增字号时：font.xml 加一条，并在下面的 switch 里登记。
static IFontPtr UiFont(int px, bool bold, int scale) {
    const wchar_t *pszFont;
    switch (px) {
        case 10: pszFont = L"@font/f10"; break;
        case 11: pszFont = bold ? L"@font/f11b" : L"@font/f11"; break;
        case 12: pszFont = bold ? L"@font/f12b" : L"@font/f12"; break;
        case 13: pszFont = bold ? L"@font/f13b" : L"@font/f13"; break;
        case 14: pszFont = bold ? L"@font/f14b" : L"@font/f14"; break;
        case 15: pszFont = L"@font/f15b"; break;
        case 21: pszFont = L"@font/f21b"; break;
        case 26: pszFont = L"@font/f26b"; break;
        default: pszFont = bold ? L"@font/defaultb" : L"@font/default"; break;
    }
    return SFontPool::GetFont(pszFont, scale);
}

// 本文件里所有尺寸常量都是按 96DPI（scale = 100）设计的。
// 字体已经随 scale 放大（见上面的 UiFont），所以容器矩形、圆角、笔宽、间距
// 也必须一起放大，否则高 DPI 下文字会撑破卡片（例如覆盖率圆环压住左侧说明文字）。
static inline int SP(int v, int scale) {
    return v * scale / 100;
}

// 圆角卡片背景 + 1px 描边
static void DrawCard(IRenderTarget *pRT, const CRect &rc, int scale) {
    const CPoint round(SP(8, scale), SP(8, scale));
    pRT->FillSolidRoundRect(&rc, round, kCard);
    SAutoRefPtr<IPenS> pen;
    SAutoRefPtr<IRenderObj> oldPen;
    pRT->CreatePen(PS_SOLID, kLine, 1, &pen);
    pRT->SelectObject(pen, &oldPen);
    CRect r = rc;
    r.right -= 1;
    r.bottom -= 1;
    pRT->DrawRoundRect(&r, round);
    pRT->SelectObject(oldPen, NULL);
}

// 用指定字体/颜色在矩形内绘制单行文本
static void DrawStr(IRenderTarget *pRT, const SStringT &s, CRect rc, UINT fmt, IFontPtr font,
                    COLORREF clr) {
    SAutoRefPtr<IRenderObj> oldFont;
    if (font) pRT->SelectObject(font, &oldFont);
    COLORREF oldClr = pRT->SetTextColor(clr);
    pRT->DrawText(s, s.GetLength(), &rc, fmt);
    pRT->SetTextColor(oldClr);
    if (font) pRT->SelectObject(oldFont, NULL);
}

// =============================================================== KPI 卡片
SFundKpiCard::SFundKpiCard()
    : m_valueClr(kText), m_bHero(false), m_bRing(false), m_ringRate(0.f) {
}

void SFundKpiCard::SetHero(bool b) {
    m_bHero = b;
    Invalidate();
}

void SFundKpiCard::SetContent(const SStringT &label, const SStringT &value, const SStringT &unit,
                              const SStringT &sub, const SStringT &pill, COLORREF valueClr) {
    m_label = label;
    m_value = value;
    m_unit = unit;
    m_sub = sub;
    m_pill = pill;
    m_valueClr = valueClr;
    Invalidate();
}

void SFundKpiCard::SetRing(float rate, const SStringT &center, const SStringT &note) {
    m_bRing = true;
    m_ringRate = rate < 0 ? 0 : (rate > 1 ? 1 : rate);
    m_ringCenter = center;
    m_ringNote = note;
    Invalidate();
}

void SFundKpiCard::OnPaint(IRenderTarget *pRT) {
    CRect rc = GetClientRect();
    if (rc.IsRectEmpty()) return;

    const int scale = GetScale();
    DrawCard(pRT, rc, scale);

    const int padX = SP(12, scale);
    CRect rcBody = rc;
    rcBody.DeflateRect(padX, SP(10, scale), padX, SP(9, scale));

    // 顶部：小标题（左）+ 状态胶囊（右）
    CRect rcLabel = rcBody;
    rcLabel.bottom = rcLabel.top + SP(16, scale);
    DrawStr(pRT, m_label, rcLabel, DT_LEFT | DT_SINGLELINE | DT_VCENTER,
            UiFont(12, false, scale), kText3);

    if (!m_pill.IsEmpty()) {
        IFontPtr fPill = UiFont(11, false, scale);
        SIZE sz = {0, 0};
        SAutoRefPtr<IRenderObj> oldFont;
        pRT->SelectObject(fPill, &oldFont);
        pRT->MeasureText(m_pill, m_pill.GetLength(), &sz);
        pRT->SelectObject(oldFont, NULL);

        int w = sz.cx + SP(14, scale), h = SP(18, scale);
        CRect rcPill(rcLabel.right - w, rcLabel.top + (rcLabel.Height() - h) / 2, rcLabel.right,
                     rcLabel.top + (rcLabel.Height() - h) / 2 + h);
        pRT->FillSolidRoundRect(&rcPill, CPoint(h / 2, h / 2), RGBA(0x1f, 0x2a, 0x3c, 0xFF));
        DrawStr(pRT, m_pill, rcPill, DT_CENTER | DT_SINGLELINE | DT_VCENTER, fPill, kText2);
        rcLabel.right = rcPill.left - SP(6, scale);
    }

    if (m_bRing) {
        // 圆环画在右侧，主数值区让出右侧一块
        int cx = rcBody.right - SP(26, scale), cy = rcBody.top + SP(42, scale), rad = SP(21, scale);
        CRect rcRing(cx - rad, cy - rad, cx + rad, cy + rad);
        SAutoRefPtr<IPenS> penBg, penFg;
        SAutoRefPtr<IRenderObj> oldPen;
        pRT->CreatePen(PS_SOLID, RGBA(0x1d, 0x26, 0x35, 0xFF), SP(6, scale), &penBg);
        pRT->SelectObject(penBg, &oldPen);
        pRT->DrawArc(&rcRing, 0.f, 360.f, FALSE);
        pRT->CreatePen(PS_SOLID, kAccent, SP(6, scale), &penFg);
        pRT->SelectObject(penFg, NULL);
        pRT->DrawArc(&rcRing, -90.f, 360.f * m_ringRate, FALSE);
        pRT->SelectObject(oldPen, NULL);

        DrawStr(pRT, m_ringCenter, rcRing, DT_CENTER | DT_SINGLELINE | DT_VCENTER,
                UiFont(15, true, scale), kText);

        CRect rcNote = rcBody;
        rcNote.left = rcRing.left - SP(92, scale);
        rcNote.right = rcRing.left - SP(10, scale);
        rcNote.top = cy - SP(22, scale);
        rcNote.bottom = cy + SP(22, scale);
        DrawStr(pRT, m_ringNote, rcNote, DT_LEFT | DT_VCENTER | DT_WORDBREAK,
                UiFont(11, false, scale), kText3);

        rcBody.right = rcRing.left - SP(8, scale);
    }

    // 主数值：hero 更大字号；单位用小字紧随其后
    IFontPtr fVal = UiFont(m_bHero ? 26 : 21, true, scale);
    IFontPtr fUnit = UiFont(12, false, scale);

    int valTop = rcBody.top + SP(20, scale);
    CRect rcVal = rcBody;
    rcVal.top = valTop;
    rcVal.bottom = valTop + SP(m_bHero ? 40 : 32, scale);

    SIZE szVal = {0, 0};
    {
        SAutoRefPtr<IRenderObj> oldFont;
        pRT->SelectObject(fVal, &oldFont);
        pRT->MeasureText(m_value, m_value.GetLength(), &szVal);
        pRT->SelectObject(oldFont, NULL);
    }
    DrawStr(pRT, m_value, rcVal, DT_LEFT | DT_SINGLELINE | DT_VCENTER, fVal, m_valueClr);

    if (!m_unit.IsEmpty()) {
        CRect rcUnit = rcVal;
        rcUnit.left += szVal.cx + SP(2, scale);
        rcUnit.top += SP(m_bHero ? 12 : 8, scale);
        DrawStr(pRT, m_unit, rcUnit, DT_LEFT | DT_SINGLELINE | DT_VCENTER, fUnit, kText3);
    }

    // 底部说明
    if (!m_sub.IsEmpty()) {
        CRect rcSub = rcBody;
        rcSub.top = rcBody.bottom - SP(16, scale);
        DrawStr(pRT, m_sub, rcSub, DT_LEFT | DT_SINGLELINE | DT_VCENTER,
                UiFont(11, false, scale), kText3);
    }
}

// ========================================================== 估算分时走势
SFundSpark::SFundSpark() {
}

void SFundSpark::SetSeries(const std::vector<double> &v) {
    m_v = v;
    Invalidate();
}

void SFundSpark::SetNote(const SStringT &note) {
    m_note = note;
    Invalidate();
}

// 走势图的坐标换算：x 轴按序列下标均匀分布，y 轴按 ±maxAbs 映射到中点上下。
// 原先写成两个 lambda（X/Y），VS2008 不支持 lambda，改为文件级静态函数。
static int SparkX(const CRect &rcPlot, int n, int i) {
    const int w = rcPlot.Width();
    return rcPlot.left + (int)((double)i / (double)(n - 1) * (w - 1));
}

static int SparkY(int midY, int halfH, double maxAbs, double v) {
    double t = v / maxAbs;
    if (t > 1) t = 1;
    if (t < -1) t = -1;
    return midY - (int)(t * halfH);
}

void SFundSpark::OnPaint(IRenderTarget *pRT) {
    CRect rc = GetClientRect();
    if (rc.IsRectEmpty()) return;

    const int scale = GetScale();
    const int left = SP(4, scale), right = SP(4, scale);
    const int top = SP(6, scale), bottom = SP(20, scale);
    CRect rcPlot = rc;
    rcPlot.DeflateRect(left, top, right, bottom);
    if (rcPlot.Width() < SP(20, scale) || rcPlot.Height() < SP(20, scale)) return;

    if (m_v.empty()) {
        DrawStr(pRT, _T("等待行情数据…"), rcPlot, DT_CENTER | DT_SINGLELINE | DT_VCENTER,
                UiFont(12, false, scale), kText3);
        return;
    }

    // 非交易时段只会拿到一个快照，此时按水平线绘制，避免空图
    const bool single = (m_v.size() == 1);
    std::vector<double> data = m_v;
    if (single) data.push_back(data[0]);

    // 纵向量程：取绝对值最大者，留 15% 余量（下限 ±0.5%，避免小幅波动顶满全图）
    double maxAbs = 0.5;
    for (size_t i = 0; i < data.size(); ++i) maxAbs = (std::max)(maxAbs, std::fabs(data[i]));
    maxAbs *= 1.15;

    const int midY = rcPlot.top + rcPlot.Height() / 2;
    const int halfH = rcPlot.Height() / 2 - SP(3, scale);
    const int n = (int)data.size();
    const int w = rcPlot.Width();

    double last = data.back();
    COLORREF crLine = last >= 0 ? kUp : kDown;
    COLORREF crArea = last >= 0 ? RGBA(0xff, 0x5c, 0x5c, 0x22) : RGBA(0x22, 0xc5, 0x5e, 0x22);

    // 面积：逐像素竖线填充（SOUI 无渐变填充接口，用半透明实心线近似）
    // 单点快照只画水平线，不铺面积，否则会是一整块色带
    for (int x = rcPlot.left; !single && x <= rcPlot.right; ++x) {
        double pos = (double)(x - rcPlot.left) / (double)(w - 1) * (n - 1);
        int i0 = (int)pos;
        int i1 = (std::min)(i0 + 1, n - 1);
        double frac = pos - i0;
        double v = data[i0] * (1 - frac) + data[i1] * frac;
        int y = SparkY(midY, halfH, maxAbs, v);
        CRect seg(x, (std::min)(y, midY), x + 1, (std::max)(y, midY) + 1);
        if (seg.Height() > 0) pRT->FillSolidRect(&seg, crArea);
    }

    // 0 轴（虚线）
    SAutoRefPtr<IPenS> penGrid;
    SAutoRefPtr<IRenderObj> oldPenGrid;
    pRT->CreatePen(PS_SOLID, RGBA(0x3a, 0x47, 0x5c, 0xFF), 1, &penGrid);
    pRT->SelectObject(penGrid, &oldPenGrid);
    for (int x = rcPlot.left; x < rcPlot.right; x += SP(6, scale))
        pRT->DrawLine(CPoint(x, midY), CPoint((std::min)(x + SP(3, scale), (int)rcPlot.right), midY));

    // 纵向刻度（0 / 1/3 / 2/3 / 1 分别对应 09:30 / 11:30 / 13:00 / 15:00）
    static const TCHAR *kTime[] = {_T("09:30"), _T("11:30"), _T("13:00"), _T("15:00")};
    const int halfLab = SP(20, scale), labOff = SP(2, scale);
    for (int k = 0; k < 4; ++k) {
        int x = rcPlot.left + (int)((double)k / 3.0 * (w - 1));
        if (k > 0 && k < 3) {
            for (int y = rcPlot.top; y < rcPlot.bottom; y += SP(5, scale))
                pRT->DrawLine(CPoint(x, y), CPoint(x, (std::min)(y + SP(2, scale), (int)rcPlot.bottom)));
        }
        // 首尾刻度贴边对齐，避免文字被绘图区裁剪成 "30" / "15"
        CRect rcT = (k == 0)   ? CRect(x, rcPlot.bottom + labOff, x + halfLab * 2, rc.bottom)
                    : (k == 3) ? CRect(x - halfLab * 2, rcPlot.bottom + labOff, x, rc.bottom)
                               : CRect(x - halfLab, rcPlot.bottom + labOff, x + halfLab, rc.bottom);
        UINT fmt = (k == 0) ? (DT_LEFT | DT_SINGLELINE | DT_VCENTER)
                            : ((k == 3) ? (DT_RIGHT | DT_SINGLELINE | DT_VCENTER)
                                        : (DT_CENTER | DT_SINGLELINE | DT_VCENTER));
        DrawStr(pRT, kTime[k], rcT, fmt, UiFont(10, false, scale), kText3);
    }
    pRT->SelectObject(oldPenGrid, NULL);

    // 折线
    std::vector<POINT> pts(n);
    for (int i = 0; i < n; ++i) {
        pts[i].x = SparkX(rcPlot, n, i);
        pts[i].y = SparkY(midY, halfH, maxAbs, data[i]);
    }
    SAutoRefPtr<IPenS> penLine;
    SAutoRefPtr<IRenderObj> oldPenLine;
    pRT->CreatePen(PS_SOLID, crLine, SP(2, scale) < 1 ? 1 : SP(2, scale), &penLine);
    pRT->SelectObject(penLine, &oldPenLine);
    pRT->DrawLines(&pts[0], (size_t)n);
    pRT->SelectObject(oldPenLine, NULL);

    // 末点圆点
    int lx = pts[n - 1].x, ly = pts[n - 1].y;
    const int dotR = SP(4, scale), dotIn = SP(2, scale);
    CRect rcDot(lx - dotR, ly - dotR, lx + dotR, ly + dotR);
    pRT->FillSolidEllipse(&rcDot, crLine);
    CRect rcDotIn(lx - dotIn, ly - dotIn, lx + dotIn, ly + dotIn);
    pRT->FillSolidEllipse(&rcDotIn, kCard);
}

// ============================================================ 贡献度分解
SFundContrib::SFundContrib() {
}

void SFundContrib::Clear() {
    m_items.clear();
    Invalidate();
}

void SFundContrib::SetItems(const std::vector<ContributionItem> &items) {
    m_items = items;
    Invalidate();
}

void SFundContrib::OnPaint(IRenderTarget *pRT) {
    CRect rc = GetClientRect();
    if (rc.IsRectEmpty()) return;

    const int scale = GetScale();
    if (m_items.empty()) {
        DrawStr(pRT, _T("暂无估值数据"), rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER,
                UiFont(12, false, scale), kText3);
        return;
    }

    const int nameW = SP(84, scale), valW = SP(62, scale), gap = SP(8, scale);
    const int rowMinH = SP(24, scale);
    const int maxRows = (std::max)(1, rc.Height() / (rowMinH > 0 ? rowMinH : 1));
    int rows = (std::min)((int)m_items.size(), maxRows);
    int rowH = rc.Height() / rows;

    double maxAbs = 0.001;
    for (int i = 0; i < rows; ++i) maxAbs = (std::max)(maxAbs, std::fabs(m_items[i].contrib));

    int barL = rc.left + nameW + gap;
    int barR = rc.right - valW - gap;
    if (barR - barL < SP(30, scale)) return;
    int barCx = (barL + barR) / 2;
    int halfMax = (barR - barL) / 2 - 1;

    IFontPtr fName = UiFont(11, false, scale);
    IFontPtr fVal = UiFont(11, false, scale);

    for (int i = 0; i < rows; ++i) {
        const ContributionItem &it = m_items[i];
        int yTop = rc.top + i * rowH;
        CRect rcRow(rc.left, yTop, rc.right, yTop + rowH);
        int cy = yTop + rowH / 2;

        // 名称（超宽截断加省略号）
        CRect rcName(rc.left, yTop, rc.left + nameW, yTop + rowH);
        SStringT sName = S_CA2T(it.name.c_str(), CP_UTF8);
        {
            SAutoRefPtr<IRenderObj> oldFont;
            pRT->SelectObject(fName, &oldFont);
            while (sName.GetLength() > 1) {
                SIZE sz = {0, 0};
                pRT->MeasureText(sName, sName.GetLength(), &sz);
                if (sz.cx <= nameW) break;
                sName = sName.Left(sName.GetLength() - 1);
            }
            pRT->SelectObject(oldFont, NULL);
        }
        DrawStr(pRT, sName, rcName, DT_LEFT | DT_SINGLELINE | DT_VCENTER, fName, kText2);

        // 中心轴与双向条
        CRect rcAxis(barCx, yTop + SP(3, scale), barCx + 1, yTop + rowH - SP(3, scale));
        pRT->FillSolidRect(&rcAxis, RGBA(0x24, 0x30, 0x44, 0xFF));

        int len = (int)(std::fabs(it.contrib) / maxAbs * halfMax);
        if (len < 1) len = 1;
        COLORREF cr = it.contrib >= 0 ? kUp : kDown;
        const int halfBar = SP(4, scale) < 2 ? 2 : SP(4, scale);
        CRect rcBar;
        if (it.contrib >= 0) rcBar.SetRect(barCx + 1, cy - halfBar, barCx + 1 + len, cy + halfBar);
        else rcBar.SetRect(barCx - len, cy - halfBar, barCx, cy + halfBar);
        pRT->FillSolidRoundRect(&rcBar, CPoint(SP(3, scale), SP(3, scale)), cr);

        // 贡献数值
        SStringT sVal;
        sVal.Format(_T("%+.3f"), it.contrib);
        CRect rcVal(rc.right - valW, yTop, rc.right, yTop + rowH);
        DrawStr(pRT, sVal, rcVal, DT_RIGHT | DT_SINGLELINE | DT_VCENTER, fVal, cr);
    }
}

// ===================================================== 基金页签（自选列表）
static const int kCardW = 214;   // 单张卡片宽
static const int kCardH = 60;    // 单张卡片高
static const int kCardGap = 8;   // 卡片间距

// 滚轮 / 拖拽的一格步进都以「一张卡片」为单位。
static int CardStep(int scale) {
    return SP(kCardW + kCardGap, scale);
}

SFundWatchTabs::SFundWatchTabs()
    : m_sel(0), m_hover(-1), m_closeHover(-1), m_arrowHover(0), m_pListener(NULL) {
}

void SFundWatchTabs::SetItems(const std::vector<WatchTabItem> &items) {
    m_items = items;
    if (m_sel >= (int)m_items.size()) m_sel = (int)m_items.size() - 1;
    if (m_sel < 0) m_sel = 0;
    if (m_hover >= (int)m_items.size()) m_hover = -1;
    if (m_closeHover >= (int)m_items.size()) m_closeHover = -1;
    // 后台数据刷新会频繁走这里，不能顺带把视图拉回选中项：
    // 那会打断用户正在进行的拖拽 / fling / 自由滚动。
    UpdateScrollRange();
    Invalidate();
}

void SFundWatchTabs::SetCurSel(int i) {
    if (i < 0 || i >= (int)m_items.size()) return;
    m_sel = i;
    EnsureSelVisible();
    Invalidate();
}

// 后台数据刷新（持仓/估值/指数）会高频同步页签，只能改选中态，不能碰滚动位置：
// EnsureSelVisible() 会把已滚出可视区的第 0 张卡片重新拉回 pos=0，
// 表现就是「点完右箭头马上自动滚回第一个元素」。
void SFundWatchTabs::SyncCurSel(int i) {
    if (i < 0 || i >= (int)m_items.size()) return;
    if (m_sel == i) return;
    m_sel = i;
    Invalidate();
}

void SFundWatchTabs::UpdateScrollRange() {
    const int scale = GetScale();
    // 用 SWindow 的客户区，避免 SPanel::GetClientRect() 的 m_rcClient 尚未刷新
    CRect rc = SWindow::GetClientRect();
    const int page = (std::max)(0, rc.Width());
    const int total = m_items.empty()
                           ? 0
                           : (int)m_items.size() * CardStep(scale) - SP(kCardGap, scale);

    // SPanel 不会自动算范围，SListView 也是直接维护 m_wBarVisible / m_siHoz。
    // 注意 sbWid=0 时滚动条宽度虽为 0，HasScrollBar(FALSE) 仍须为 TRUE：
    // IsEnableDragMode() / OnDragScroll() / StartFlingAnimation() 都以它为准。
    m_wBarVisible = SSB_NULL;
    m_siHoz.nMin = 0;
    m_siHoz.nPage = page;
    m_siHoz.nMax = (std::max)(0, total - 1);
    if (total > page && page > 0)
        m_wBarVisible |= SSB_HORZ;
    else
        m_siHoz.nMax = page > 0 ? page - 1 : 0;

    int maxPos = (int)m_siHoz.nMax - (int)m_siHoz.nPage + 1;
    if (maxPos < 0) maxPos = 0;
    if (m_siHoz.nPos < 0) m_siHoz.nPos = 0;
    if (m_siHoz.nPos > maxPos) m_siHoz.nPos = maxPos;

    // 竖向保持「无需滚动」，否则 SPanel::OnMouseWheel 会把滚轮派给竖轴。
    m_siVer.nMin = 0;
    m_siVer.nPage = (std::max)(0, rc.Height());
    m_siVer.nMax = m_siVer.nPage > 0 ? (int)m_siVer.nPage - 1 : 0;
    m_siVer.nPos = 0;

    SetScrollPos(FALSE, m_siHoz.nPos, FALSE);
    SSendMessage(WM_NCCALCSIZE);  // 重算 m_rcClient（sbWid=0 时等于原客户区）
    InvalidateRect(NULL);
}

void SFundWatchTabs::EnsureSelVisible() {
    if (m_sel < 0 || !HasScrollBar(FALSE)) return;
    CRect rc = GetClientRect();
    CRect r = CardRect(m_sel);
    int pos = GetScrollPos(FALSE);
    if (r.left < rc.left)
        pos -= rc.left - r.left;
    else if (r.right > rc.right)
        pos += r.right - rc.right;

    int maxPos = (int)m_siHoz.nMax - (int)m_siHoz.nPage + 1;
    if (maxPos < 0) maxPos = 0;
    if (pos < 0) pos = 0;
    if (pos > maxPos) pos = maxPos;
    if (pos != GetScrollPos(FALSE)) ScrollToPos(FALSE, pos);
}

void SFundWatchTabs::OnSize(UINT nType, CSize size) {
    __baseCls::OnSize(nType, size);
    // UpdateScrollRange() 已把 nPos clamp 到新的合法范围；这里不要再
    // EnsureSelVisible()，否则视图 show/hide、窗口缩放都会把自由滚动拉回选中项。
    UpdateScrollRange();
}

int SFundWatchTabs::GetScrollLineSize(BOOL bVertical) {
    if (bVertical) return __baseCls::GetScrollLineSize(bVertical);
    return CardStep(GetScale());
}

CRect SFundWatchTabs::CardRect(int i) const {
    CRect rc = GetClientRect();
    const int scale = GetScale();
    const int w = SP(kCardW, scale), h = SP(kCardH, scale), gap = SP(kCardGap, scale);
    const int top = rc.top + ((std::max)(0, rc.Height() - h)) / 2;
    const int x = rc.left + i * (w + gap) - GetScrollPos(FALSE);
    return CRect(x, top, x + w, top + h);
}

int SFundWatchTabs::HitTest(CPoint pt, bool *pClose) const {
    if (pClose) *pClose = false;
    const int scale = GetScale();
    for (int i = 0; i < (int)m_items.size(); ++i) {
        CRect r = CardRect(i);
        if (!r.PtInRect(pt)) continue;
        CRect rcClose(r.right - SP(24, scale), r.top + SP(4, scale), r.right - SP(4, scale),
                      r.top + SP(24, scale));
        if (pClose && rcClose.PtInRect(pt)) *pClose = true;
        return i;
    }
    return -1;
}

// ------------------------------------------------------ 左右滚动按钮（‹ / ›）
// 这两个圆形提示本来只在 OnPaint 里画出来，没有点击逻辑；点上去会落到卡片上，
// 走 HitTest→SetCurSel→EnsureSelVisible，滚动量只是「把这张卡补到完整可见」的任意
// 像素，还会顺带切换选中基金。这里把它们变成真按钮：命中区域与绘制几何同源。
CRect SFundWatchTabs::ArrowRect(bool bRight) const {
    const int d = SP(22, GetScale());
    CRect rc = GetClientRect();
    const int cy = (rc.top + rc.bottom) / 2;
    if (bRight) return CRect(rc.right - d, cy - d / 2, rc.right, cy + d / 2);
    return CRect(rc.left, cy - d / 2, rc.left + d, cy + d / 2);
}

int SFundWatchTabs::HitArrow(CPoint pt) const {
    const int maxS = (int)m_siHoz.nMax - (int)m_siHoz.nPage + 1;
    if (maxS <= 0) return 0;  // 内容没超出可视宽度时两个按钮都不显示
    const int pos = GetScrollPos(FALSE);
    if (pos > 0 && ArrowRect(false).PtInRect(pt)) return -1;
    if (pos < maxS && ArrowRect(true).PtInRect(pt)) return 1;
    return 0;
}

void SFundWatchTabs::StepScroll(int dir) {
    // 一次点击 = 一个元素：SB_LINEUP/LINEDOWN 的步长取自
    // GetScrollLineSize(FALSE)=CardStep()，与滚轮一格完全一致。
    // 必须先掐掉惯性，否则正在跑的 fling 会把这次位移继续放大。
    StopFlingAnimation();
    OnScroll(FALSE, dir < 0 ? SB_LINEUP : SB_LINEDOWN, 0);
    Invalidate();
}

// SPanel 的「非拖拽」扩展点：拖拽/fling 结束时 HandleMouseDrag 会直接吞掉
// WM_LBUTTONUP，不会走到这里，因此甩动列表不会误触发选中/关闭。
void SFundWatchTabs::OnLButtonUpEx(UINT nFlags, CPoint pt) {
    __baseCls::OnLButtonUpEx(nFlags, pt);
    // 箭头优先于卡片：只滚一个元素，不改选中项、不触发 EnsureSelVisible。
    // 用「抬起位置」判定而不是按下位置，这样 fling 运行中那次按下被
    // HandleMouseDrag 吞掉（顺便 StopFlingAnimation）时，本次点击依然生效。
    const int arrow = HitArrow(pt);
    if (arrow != 0) {
        StepScroll(arrow);
        return;
    }
    bool bClose = false;
    const int i = HitTest(pt, &bClose);
    if (i < 0) return;
    if (bClose) {
        if (m_pListener) m_pListener->OnFundWatchClose(i);
        return;
    }
    SetCurSel(i);
    if (m_pListener) m_pListener->OnFundWatchSel(i);
}

void SFundWatchTabs::OnMouseMoveEx(UINT nFlags, CPoint pt) {
    __baseCls::OnMouseMoveEx(nFlags, pt);
    // 箭头浮在卡片之上，命中箭头时不要同时把底下那张卡标成 hover。
    const int arrow = HitArrow(pt);
    bool bClose = false;
    const int i = (arrow == 0) ? HitTest(pt, &bClose) : -1;
    const int hover = (i >= 0 && !bClose) ? i : -1;
    const int closeHover = (i >= 0 && bClose) ? i : -1;
    if (hover != m_hover || closeHover != m_closeHover || arrow != m_arrowHover) {
        m_hover = hover;
        m_closeHover = closeHover;
        m_arrowHover = arrow;
        Invalidate();
    }
}

void SFundWatchTabs::OnMouseLeave() {
    if (m_hover != -1 || m_closeHover != -1 || m_arrowHover != 0) {
        m_hover = -1;
        m_closeHover = -1;
        m_arrowHover = 0;
        Invalidate();
    }
}

// WM_MOUSEWHEEL 不在此处理：SPanel::OnMouseWheel 会按 HasScrollBar(TRUE)=FALSE
// 把滚轮派到横轴，配合 GetScrollLineSize() 每格滚一张卡片。

void SFundWatchTabs::OnPaint(IRenderTarget *pRT) {
    CRect rc = GetClientRect();
    if (rc.IsRectEmpty()) return;

    const int scale = GetScale();
    const CPoint round(SP(8, scale), SP(8, scale));

    pRT->PushClipRect(&rc);

    if (m_items.empty()) {
        DrawStr(pRT, _T("暂无自选基金：填写代码后点「＋ 收藏」即可加入"), rc,
                DT_CENTER | DT_SINGLELINE | DT_VCENTER, UiFont(11, false, scale), kText3);
        pRT->PopClip();
        return;
    }

    for (int i = 0; i < (int)m_items.size(); ++i) {
        CRect r = CardRect(i);
        if (r.right <= rc.left || r.left >= rc.right) continue;

        const WatchTabItem &it = m_items[i];
        const bool on = (i == m_sel);
        const bool hov = (i == m_hover);

        // 卡片底色：选中略微提亮，悬停取中间态
        COLORREF bg = on ? RGBA(0x1a, 0x23, 0x33, 0xFF)
                         : (hov ? RGBA(0x14, 0x1c, 0x29, 0xFF) : RGBA(0x0e, 0x15, 0x1f, 0xFF));
        pRT->FillSolidRoundRect(&r, round, bg);

        // 描边：选中为暗金，未选中为常规分隔线色
        {
            SAutoRefPtr<IPenS> pen;
            SAutoRefPtr<IRenderObj> oldPen;
            pRT->CreatePen(PS_SOLID, on ? RGBA(0x4a, 0x3b, 0x18, 0xFF) : kLine, 1, &pen);
            pRT->SelectObject(pen, &oldPen);
            CRect rb = r;
            rb.right -= 1;
            rb.bottom -= 1;
            pRT->DrawRoundRect(&rb, round);
            pRT->SelectObject(oldPen, NULL);
        }

        // 选中态底部金条（浏览器标签页的观感）
        if (on) {
            CRect rcBar(r.left + SP(10, scale), r.bottom - SP(3, scale), r.right - SP(10, scale),
                        r.bottom - SP(1, scale));
            pRT->FillSolidRoundRect(&rcBar, CPoint(SP(1, scale), SP(1, scale)), kAccent);
        }

        const int padX = SP(12, scale);

        // 第一行：基金代码
        CRect rcCode(r.left + padX, r.top + SP(7, scale), r.right - SP(24, scale),
                     r.top + SP(25, scale));
        DrawStr(pRT, it.code, rcCode, DT_LEFT | DT_SINGLELINE | DT_VCENTER,
                UiFont(12, true, scale), on ? kText : kText2);

        // 关闭按钮
        CRect rcClose(r.right - SP(24, scale), r.top + SP(4, scale), r.right - SP(4, scale),
                      r.top + SP(24, scale));
        DrawStr(pRT, _T("×"), rcClose, DT_CENTER | DT_SINGLELINE | DT_VCENTER,
                UiFont(12, false, scale), (i == m_closeHover) ? kUp : kText3);

        // 第二行：基金名称（过长由绘制区裁切）
        CRect rcName(r.left + padX, r.top + SP(26, scale), r.right - SP(10, scale),
                     r.top + SP(42, scale));
        DrawStr(pRT, it.name.IsEmpty() ? SStringT(_T("—")) : it.name, rcName,
                DT_LEFT | DT_SINGLELINE | DT_VCENTER, UiFont(10, false, scale), kText3);

        // 第三行：估算涨跌幅（左）+ 估算净值（右）
        CRect rcPct(r.left + padX, r.bottom - SP(24, scale), r.left + padX + SP(94, scale),
                    r.bottom - SP(5, scale));
        SStringT sPct;
        COLORREF clr = kText3;
        if (it.hasEst) {
            sPct.Format(_T("%+.2f%%"), it.estPct);
            clr = it.estPct > 0 ? kUp : (it.estPct < 0 ? kDown : kText2);
        } else if (it.loading) {
            sPct = _T("加载中…");
        } else if (it.loaded) {
            sPct = _T("—");
        } else {
            sPct = _T("待加载");
        }
        DrawStr(pRT, sPct, rcPct, DT_LEFT | DT_SINGLELINE | DT_VCENTER, UiFont(14, true, scale),
                clr);

        CRect rcNav(r.right - SP(104, scale), rcPct.top, r.right - SP(10, scale), rcPct.bottom);
        SStringT sNav;
        if (it.hasEst && it.estNav > 0) sNav.Format(_T("估 %.4f"), it.estNav);
        else if (it.nav > 0) sNav.Format(_T("净 %.4f"), it.nav);
        if (!sNav.IsEmpty())
            DrawStr(pRT, sNav, rcNav, DT_RIGHT | DT_SINGLELINE | DT_VCENTER,
                    UiFont(10, false, scale), kText3);
    }

    // 卡片超出可视宽度时，在边缘给两个可点击的滚动按钮（一次滚一个元素）。
    // 几何与 HitArrow() 共用 ArrowRect()，避免「看得见却点不到 / 点到的不是它」。
    const int pos = GetScrollPos(FALSE);
    const int maxS = (int)m_siHoz.nMax - (int)m_siHoz.nPage + 1;
    if (maxS > 0) {
        const int d = SP(22, scale);
        const CPoint roundBtn(d / 2, d / 2);
        if (pos < maxS) {
            CRect rcR = ArrowRect(true);
            const bool hot = (m_arrowHover > 0);
            pRT->FillSolidRoundRect(&rcR, roundBtn, hot ? RGBA(0x2e, 0x3d, 0x56, 0xFF)
                                                         : RGBA(0x1f, 0x2a, 0x3c, 0xFF));
            DrawStr(pRT, _T("›"), rcR, DT_CENTER | DT_SINGLELINE | DT_VCENTER,
                    UiFont(14, true, scale), hot ? kText : kText2);
        }
        if (pos > 0) {
            CRect rcL = ArrowRect(false);
            const bool hot = (m_arrowHover < 0);
            pRT->FillSolidRoundRect(&rcL, roundBtn, hot ? RGBA(0x2e, 0x3d, 0x56, 0xFF)
                                                         : RGBA(0x1f, 0x2a, 0x3c, 0xFF));
            DrawStr(pRT, _T("‹"), rcL, DT_CENTER | DT_SINGLELINE | DT_VCENTER,
                    UiFont(14, true, scale), hot ? kText : kText2);
        }
    }

    pRT->PopClip();
}

// ============================================================== 页签栏
static const int kTabW = 108;  // 单个页签宽度
static const int kTabH = 30;   // 页签高度

SFundTabs::SFundTabs() : m_sel(0), m_pListener(NULL) {
}

void SFundTabs::SetItems(const std::vector<std::pair<SStringT, int> > &items) {
    m_items = items;
    Invalidate();
}

void SFundTabs::SetCurSel(int i) {
    if (i < 0 || i >= (int)m_items.size()) return;
    m_sel = i;
    Invalidate();
}

void SFundTabs::LayoutTabs(std::vector<CRect> &rcs) const {
    rcs.clear();
    CRect rc = GetClientRect();
    // 页签宽度/高度也要跟着 DPI 走，否则放大字体后会挤成两行或被裁掉
    const int scale = GetScale();
    const int tabW = SP(kTabW, scale), tabH = SP(kTabH, scale), gap = SP(6, scale);
    int cy = (rc.top + rc.bottom) / 2;
    int x = rc.left;
    for (size_t i = 0; i < m_items.size(); ++i) {
        rcs.push_back(CRect(x, cy - tabH / 2, x + tabW, cy + tabH / 2));
        x += tabW + gap;
    }
}

int SFundTabs::HitTest(CPoint pt) const {
    std::vector<CRect> rcs;
    LayoutTabs(rcs);
    for (size_t i = 0; i < rcs.size(); ++i)
        if (rcs[i].PtInRect(pt)) return (int)i;
    return -1;
}

void SFundTabs::OnLButtonUp(UINT nFlags, CPoint pt) {
    __baseCls::OnLButtonUp(nFlags, pt);
    int i = HitTest(pt);
    if (i >= 0 && i != m_sel) {
        m_sel = i;
        Invalidate();
        if (m_pListener) m_pListener->OnFundTabsSel(i);
    }
}

void SFundTabs::OnPaint(IRenderTarget *pRT) {
    CRect rc = GetClientRect();
    if (rc.IsRectEmpty()) return;

    const int scale = GetScale();
    std::vector<CRect> rcs;
    LayoutTabs(rcs);

    for (size_t i = 0; i < m_items.size() && i < rcs.size(); ++i) {
        const CRect &r = rcs[i];
        bool on = ((int)i == m_sel);

        if (on) {
            const CPoint round(SP(7, scale), SP(7, scale));
            pRT->FillSolidRoundRect(&r, round, RGBA(0x1a, 0x23, 0x33, 0xFF));
            SAutoRefPtr<IPenS> pen;
            SAutoRefPtr<IRenderObj> oldPen;
            pRT->CreatePen(PS_SOLID, RGBA(0x2b, 0x36, 0x48, 0xFF), 1, &pen);
            pRT->SelectObject(pen, &oldPen);
            CRect rb = r;
            rb.right -= 1;
            rb.bottom -= 1;
            pRT->DrawRoundRect(&rb, round);
            pRT->SelectObject(oldPen, NULL);
        }

        SStringT sText = m_items[i].first;
        if (m_items[i].second >= 0) sText.AppendFormat(_T(" %d"), m_items[i].second);

        DrawStr(pRT, sText, r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, UiFont(12, on, scale),
                on ? kAccent : kText2);
    }
}

// ============================================================== 大盘三大指数条
// 接口未返回名称时的兜底显示名。用 _T 宽字符串，避免窄字符串里的中文
// 在 MSVC 下被按执行字符集(GBK)转码后与 UTF-8 解码不匹配。
static const TCHAR *kIndexLabels[3] = {_T("上证指数"), _T("深证成指"), _T("创业板指")};

// 数值格式化：整数部分插入千分位；sign=true 时正数前补 "+"。
static SStringT FmtIndexNum(double v, int dec, bool sign) {
    double a = v < 0 ? -v : v;
    SStringT s;
    if (dec == 2) s.Format(_T("%.2f"), a);
    else s.Format(_T("%.0f"), a);

    const int dot = s.Find(_T("."));
    const int intEnd = (dot < 0) ? s.GetLength() : dot;
    SStringT out;
    for (int i = 0; i < intEnd; ++i) {
        if (i > 0 && (intEnd - i) % 3 == 0) out += _T(",");
        out += s[i];
    }
    for (int i = intEnd; i < s.GetLength(); ++i) out += s[i];
    if (sign) {
        SStringT pre = (v >= 0) ? SStringT(_T("+")) : SStringT(_T("-"));
        out = pre + out;
    }
    return out;
}

SFundIndexBar::SFundIndexBar() {
}

void SFundIndexBar::SetItems(const std::vector<IndexQuote> &items) {
    m_items = items;
    Invalidate();
}

void SFundIndexBar::SetNote(const SStringT &note) {
    m_note = note;
    Invalidate();
}

void SFundIndexBar::OnPaint(IRenderTarget *pRT) {
    CRect rc = GetClientRect();
    if (rc.IsRectEmpty()) return;

    const int scale = GetScale();
    const int one = SP(1, scale);

    pRT->FillSolidRect(&rc, RGBA(0x0b, 0x11, 0x19, 0xFF));
    CRect rcBottomLine(rc.left, rc.bottom - one, rc.right, rc.bottom);
    pRT->FillSolidRect(&rcBottomLine, kLine);

    CRect body = rc;
    body.DeflateRect(SP(14, scale), SP(3, scale), SP(14, scale), SP(4, scale));
    if (body.IsRectEmpty()) return;

    const int labelW = SP(82, scale);
    const int noteW = SP(196, scale);

    CRect rcLabel(body.left, body.top, body.left + labelW, body.bottom);
    DrawStr(pRT, _T("大盘速览"), rcLabel, DT_LEFT | DT_SINGLELINE | DT_VCENTER,
            UiFont(12, true, scale), kAccent);

    CRect rcNote(body.right - noteW, body.top, body.right, body.bottom);
    if (!m_note.IsEmpty())
        DrawStr(pRT, m_note, rcNote, DT_RIGHT | DT_SINGLELINE | DT_VCENTER,
                UiFont(10, false, scale), kText3);

    CRect rcGrid(body.left + labelW + SP(6, scale), body.top,
                 body.right - noteW - SP(6, scale), body.bottom);
    if (rcGrid.Width() < SP(120, scale)) return;

    const int cells = 3;
    const int cellW = rcGrid.Width() / cells;
    for (int i = 0; i < cells; ++i) {
        CRect c(rcGrid.left + i * cellW + SP(8, scale), rcGrid.top + SP(2, scale),
                rcGrid.left + (i + 1) * cellW - SP(8, scale), rcGrid.bottom - SP(2, scale));
        if (c.IsRectEmpty()) continue;

        if (i > 0) {
            CRect sep(rcGrid.left + i * cellW, body.top + SP(4, scale),
                      rcGrid.left + i * cellW + one, body.bottom - SP(4, scale));
            pRT->FillSolidRect(&sep, kLine);
        }

        const IndexQuote *q = (i < (int)m_items.size()) ? &m_items[i] : NULL;
        const bool ok = (q != NULL && q->ok);
        SStringT name(kIndexLabels[i]);
        if (q && !q->name.empty()) name = S_CA2T(q->name.c_str(), CP_UTF8);

        COLORREF clr = kText2;
        if (ok) clr = q->chgPct > 0 ? kUp : (q->chgPct < 0 ? kDown : kText2);

        CRect r1(c.left, c.top, c.right, c.top + c.Height() / 2);
        CRect r2(c.left, r1.bottom, c.right, c.bottom);

        DrawStr(pRT, name, r1, DT_LEFT | DT_SINGLELINE | DT_VCENTER, UiFont(10, false, scale),
                kText3);
        DrawStr(pRT, ok ? (FmtIndexNum(q->chgPct, 2, true) + SStringT(_T("%")))
                        : SStringT(_T("—")),
                r1, DT_RIGHT | DT_SINGLELINE | DT_VCENTER, UiFont(11, true, scale), clr);
        DrawStr(pRT, ok ? FmtIndexNum(q->price, 2, false) : SStringT(_T("—")), r2,
                DT_LEFT | DT_SINGLELINE | DT_VCENTER, UiFont(15, true, scale),
                ok ? kText : kText3);
        if (ok)
            DrawStr(pRT, FmtIndexNum(q->chgAmt, 2, true), r2,
                    DT_RIGHT | DT_SINGLELINE | DT_VCENTER, UiFont(10, false, scale), clr);
    }
}

// ============================================================== SOUI5 介绍页
SFundAbout::SFundAbout() {
}

void SFundAbout::OnPaint(IRenderTarget *pRT) {
    CRect rc = GetClientRect();
    if (rc.IsRectEmpty()) return;

    const int scale = GetScale();
    pRT->FillSolidRect(&rc, RGBA(0x07, 0x0a, 0x0f, 0xFF));

    CRect c = rc;
    c.DeflateRect(SP(22, scale), SP(14, scale), SP(22, scale), SP(14, scale));
    if (c.IsRectEmpty()) return;

    // 按可用高度分段，窗口缩小时各段等比收缩，不做固定像素堆叠。
    const int gap = SP(10, scale);
    const int h = c.Height();
    const int hBrand = h * 18 / 100;
    const int hIntro = h * 15 / 100;
    const int hCards = h * 42 / 100;
    const int hFoot = h - hBrand - hIntro - hCards - gap * 3;
    if (hFoot < SP(52, scale)) return;

    // ---- 品牌区 ----
    CRect rBrand(c.left, c.top, c.right, c.top + hBrand);
    CRect rName(rBrand.left, rBrand.top, rBrand.left + SP(220, scale), rBrand.bottom);
    DrawStr(pRT, _T("SOUI 5"), rName, DT_LEFT | DT_SINGLELINE | DT_VCENTER,
            UiFont(26, true, scale), kAccent);
    CRect rTag(rName.right, rBrand.top, rBrand.right, rBrand.top + rBrand.Height() / 2);
    DrawStr(pRT, _T("轻量级 C++ DirectUI 跨平台界面框架"), rTag,
            DT_RIGHT | DT_SINGLELINE | DT_VCENTER, UiFont(13, true, scale), kText);
    CRect rVer(rName.right, rTag.bottom, rBrand.right, rBrand.bottom);
    DrawStr(pRT, _T("仓库名 soui4 · 版本 5.x · 对外品牌统一称 SOUI5"), rVer,
            DT_RIGHT | DT_SINGLELINE | DT_VCENTER, UiFont(11, false, scale), kText3);

    // ---- 简介 ----
    CRect rIntro(c.left, rBrand.bottom + gap, c.right, rBrand.bottom + gap + hIntro);
    DrawCard(pRT, rIntro, scale);
    CRect ri = rIntro;
    ri.DeflateRect(SP(16, scale), SP(8, scale), SP(16, scale), SP(8, scale));
    DrawStr(pRT,
            _T("SOUI 用 XML 描述界面，把布局、字体、颜色、皮肤与字符串统一交给 uires 资源管理，")
            _T("界面与业务代码解耦；核心不依赖 MFC / Qt，业务代码可保持 C++03 兼容。"),
            ri, DT_LEFT | DT_WORDBREAK | DT_VCENTER, UiFont(12, false, scale), kText2);

    // ---- 三张特性卡 ----
    static const TCHAR *titles[3] = {_T("跨平台"), _T("XML + 皮肤"), _T("控件与渲染")};
    static const TCHAR *lines[3][3] = {
        {_T("Windows / Linux / macOS"), _T("Android / iOS / OpenHarmony"),
         _T("WebAssembly (Canvas2D)")},
        {_T("布局与皮肤分离"), _T("字体 / 颜色 / 字符串值表"), _T("换肤不用改业务代码")},
        {_T("SGridCtrl / SListView / STreeView"), _T("Skia / GDI 多种渲染后端"),
         _T("支持自绘控件与 MVC 虚拟列表")},
    };

    CRect rCards(c.left, rIntro.bottom + gap, c.right, rIntro.bottom + gap + hCards);
    const int cardW = (rCards.Width() - 2 * gap) / 3;
    for (int k = 0; k < 3; ++k) {
        CRect card(rCards.left + k * (cardW + gap), rCards.top,
                   rCards.left + k * (cardW + gap) + cardW, rCards.bottom);
        if (card.IsRectEmpty()) continue;
        DrawCard(pRT, card, scale);

        CRect inner = card;
        inner.DeflateRect(SP(16, scale), SP(14, scale), SP(12, scale), SP(10, scale));
        CRect rTitle(inner.left, inner.top, inner.right, inner.top + SP(30, scale));
        DrawStr(pRT, titles[k], rTitle, DT_LEFT | DT_SINGLELINE | DT_VCENTER,
                UiFont(14, true, scale), kAccent);

        CRect rBody(inner.left, rTitle.bottom + SP(8, scale), inner.right, inner.bottom);
        const int lineH = rBody.Height() / 3;
        for (int j = 0; j < 3; ++j) {
            CRect rl(rBody.left, rBody.top + j * lineH, rBody.right,
                     rBody.top + (j + 1) * lineH);
            DrawStr(pRT, lines[k][j], rl, DT_LEFT | DT_WORDBREAK | DT_VCENTER,
                    UiFont(11, false, scale), kText2);
        }
    }

    // ---- 本工具说明 ----
    CRect rFoot(c.left, rCards.bottom + gap, c.right, c.bottom);
    DrawCard(pRT, rFoot, scale);
    CRect rf = rFoot;
    rf.DeflateRect(SP(16, scale), SP(9, scale), SP(16, scale), SP(8, scale));
    CRect rfTitle(rf.left, rf.top, rf.left + SP(150, scale), rf.top + SP(22, scale));
    DrawStr(pRT, _T("关于本工具"), rfTitle, DT_LEFT | DT_SINGLELINE | DT_VCENTER,
            UiFont(13, true, scale), kText);
    CRect rfSite(rf.right - SP(180, scale), rf.top, rf.right, rfTitle.bottom);
    DrawStr(pRT, _T("soui.com.cn"), rfSite, DT_RIGHT | DT_SINGLELINE | DT_VCENTER,
            UiFont(11, false, scale), kAccent);
    CRect rfBody(rf.left, rfTitle.bottom + SP(4, scale), rf.right, rf.bottom);
    DrawStr(pRT,
            _T("fundholdings_demo 由 SOUI5 构建：季度持仓变化表与盘中估值口径参考 akshare，")
            _T("实时行情、基金净值与大盘指数来自东方财富 / 天天基金。估算结果仅供参考。"),
            rfBody, DT_LEFT | DT_WORDBREAK | DT_VCENTER, UiFont(11, false, scale), kText3);
}

SNSEND
