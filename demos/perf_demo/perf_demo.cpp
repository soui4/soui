/*
 * perf_demo.cpp -- X11 / Wayland 后端绘制性能对比
 *
 * 同一份源码按后端分别编译。**每个后端只跑一次**，一次输出全部场景、全部
 * 重复次数（repeat）以及由它们推导出来的归因指标，结果同时落盘成文本文件，
 * 两个文件再拿 --compare 并排比出 delta / ratio。
 *
 *   cmake -S . -B build-x11 -DSOUI_LINUX_BACKEND=x11
 *   cmake -S . -B build-wl -DSOUI_LINUX_BACKEND=wayland
 *   cmake --build build-x11 -j --target perf_demo
 *   cmake --build build-wl  -j --target perf_demo
 *
 *   cd build-x11 && ./bin/perf_demo            # 写出 perf_x11.txt
 *   cd build-wl  && ./bin/perf_demo            # 写出 perf_wayland.txt
 *   ./bin/perf_demo --compare=../build-x11/perf_x11.txt,./perf_wayland.txt
 *
 * 用法：
 *   perf_demo [--frames=N] [--repeat=1,4] [--case=a,b,...] [--save=FILE]
 *             [--warmup=N] [--hold] [--compare=A,B] [--help]
 *
 *   --frames=N    每个用例的采样帧数，默认 100
 *   --repeat=LIST 每次 WM_PAINT 里重复绘制的次数列表，默认 1,4。
 *                 total(N) = fixed + N*perPass ⇒ perPass = (paint(4)-paint(1))/3，
 *                 这条把"每帧固定成本"和"每次图元成本"解成两个数。
 *   --case=LIST   只跑指定场景（all/fill/text/blit/lines/empty/textmeasure）
 *   --save=FILE   结果落盘路径，默认 perf_<backend>.txt
 *   --warmup=N    每个用例开始前丢弃的暖机帧数，默认 3
 *   --hold        跑完保持窗口（ESC 或关窗退出）
 *   --compare=A,B 不绘制、不开窗，直接把两份结果文件并排比较
 *
 * 一次 sweep 覆盖的用例（默认）：
 *   empty        什么都不画，量每帧固定成本（BeginPaint/端到端提交/pump）
 *   textmeasure  200 次 DT_CALCRECT：只布局、不栅格化
 *   fill/text/blit/lines  单类图元，各跑 repeat=1 与 repeat=4
 *   all          全部图元混合，repeat=1 与 4，附分类 breakdown
 *
 * 由这些用例推导的指标（打在同一次输出里，不必再分参数跑）：
 *   fixed        = paint(empty r1)                     每帧固定成本
 *   layout       = paint(textmeasure r1) - fixed        纯文本布局
 *   glyph_raster = paint(text r1) - paint(textmeasure r1)  字形栅格化+合成
 *   per_pass(S)  = (paint(S r4) - paint(S r1)) / 3      单趟图元净成本
 *   per_call(S)  = per_pass / calls                     每次 API 调用的成本
 * 因为两个后端共享同一份 gdi/cairo，"建路径 / shaping / clip 求交"这类客户端
 * 成本在 A/B 里会抵消；真正不会抵消的是每次调用走一遍后端 compositor 的派发
 * 成本（xcb 只编协议请求，image surface 要跑 pixman）。per_call 就是量这个的。
 *
 * 只使用 swinx 提供的 Win32 子集（CreateWindowExA / BeginPaint / FillRect /
 * DrawTextA / BitBlt / CreateCompatibleDC 等），因此两个后端测的是同一段代码、
 * 同一批图元，差异只来自后端实现（surface 类型、提交、SHM 拷贝、flush）。
 */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <vector>
#include <string>
#include <map>
#include <algorithm>

#ifndef PERF_BACKEND_NAME
#define PERF_BACKEND_NAME "unknown"
#endif

static const int kDefaultWidth = 1000;
static const int kDefaultHeight = 700;

/* 场景规模：固定写死，保证两个后端跑的是同一负载。 */
static const int kFillRects = 1200;
static const int kTextRuns = 200;
static const int kBlits = 24;
static const int kLines = 128;
static const int kBlitW = 200;
static const int kBlitH = 150;

enum Scene
{
    SCENE_ALL = 0,
    SCENE_FILL,
    SCENE_TEXT,
    SCENE_BLIT,
    SCENE_LINES,
    SCENE_EMPTY,
    SCENE_TEXTMEASURE,
    SCENE_COUNT,
};

static const char *kSceneNames[] = { "all", "fill", "text", "blit", "lines", "empty", "textmeasure" };

/* 图元类别下标（breakdown 用）。 */
enum
{
    PART_FILL = 0,
    PART_TEXT,
    PART_BLIT,
    PART_LINES,
    PART_COUNT,
};

static const char *kPartNames[] = { "fill", "text", "blit", "lines" };
static const char *kPartLoads[] = { "rects", "texts", "blits", "lines" };

/* 每种场景"单趟"里各类图元的调用次数。0 = 该场景不含这类图元。 */
static const int kPartBase[SCENE_COUNT][PART_COUNT] = {
    /* ALL        */ { kFillRects, kTextRuns, kBlits, kLines },
    /* FILL       */ { kFillRects, 0, 0, 0 },
    /* TEXT       */ { 0, kTextRuns, 0, 0 },
    /* BLIT       */ { 0, 0, kBlits, 0 },
    /* LINES      */ { 0, 0, 0, kLines },
    /* EMPTY      */ { 0, 0, 0, 0 },
    /* TEXTMEASURE*/ { 0, 0, 0, 0 },
};

/* sweep 的默认场景顺序：先给基准线，再给单类，最后混合。 */
static const int kSweepScenes[] = { SCENE_EMPTY, SCENE_TEXTMEASURE, SCENE_FILL,
                                    SCENE_TEXT, SCENE_BLIT, SCENE_LINES, SCENE_ALL };

static int g_frames = 100;      /* 每个用例的采样帧数 */
static int g_warmFrames = 3;    /* 每个用例开始前丢弃的暖机帧 */
static int g_scene = SCENE_ALL;
static bool g_hold = false;
static int g_repeat = 1;        /* 当前用例的重复绘制次数 */
static bool g_recording = false;
static int g_frame = 0;         /* 动画相位：每次 WM_PAINT 递增 */

static HWND g_hwnd = NULL;
static HDC g_memdc = NULL;
static HBITMAP g_membmp = NULL;
static HBRUSH g_brushes[16];
static HPEN g_pens[8];
static HFONT g_font = NULL;

static bool g_running = true;
static std::vector<double> g_paintMs;
/* 分类累计：各类图元在采样帧里的总耗时（毫秒）。report 时除以采样帧数即得
 * 每帧平均。四项之和通常略小于 paint 均值，差值就是 BeginPaint/裁剪 +
 * EndPaint（含 canvas 提交）的固定成本，报告里记为 other。 */
static double g_partMs[PART_COUNT];
static double g_probeMs = 0.0; /* textmeasure：只布局不栅格化 */

using Clock = std::chrono::steady_clock;

static double msSince(Clock::time_point t0)
{
    return std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
}

static void resetAccum()
{
    g_paintMs.clear();
    for (int i = 0; i < PART_COUNT; ++i)
        g_partMs[i] = 0.0;
    g_probeMs = 0.0;
}

/* ------------------------------------------------------------------ */
/* 绘制场景                                                           */
/* ------------------------------------------------------------------ */

/* 填充：网格状彩色矩形带，纵向随帧滚动。模拟大量控件背景/皮肤绘制。 */
static void drawFill(HDC hdc, int w, int h)
{
    if (w < 4 || h < 16)
        return;
    const int cols = 40;
    int cw = w / cols;
    if (cw < 2)
        cw = 2;
    for (int i = 0; i < kFillRects; ++i)
    {
        int x = (i % cols) * cw;
        int y = ((i / cols) * 11 + g_frame * 3) % (h - 12);
        RECT rc;
        rc.left = x;
        rc.top = y;
        rc.right = x + cw - 1;
        rc.bottom = y + 10;
        FillRect(hdc, &rc, g_brushes[(i + g_frame) & 15]);
    }
}

/* 第 i 段文本的内容与目标矩形。drawText 与 probeTextMetrics 共用，保证
 * "只布局"和"布局+栅格化"测的是同一批字符串、同一批 rc。 */
static inline int textItem(int i, int w, int h, char *buf, int cb, RECT *rc)
{
    const int colW = w / 4;
    int x = (i & 3) * colW;
    int y = ((i >> 2) * 15 + g_frame) % (h - 18);
    int n = snprintf(buf, cb, "f%04d item%03d value=%08x SOUI Wayland",
                     g_frame, i, (unsigned)(i * 2654435761u));
    rc->left = x + 2;
    rc->top = y;
    rc->right = x + colW - 2;
    rc->bottom = y + 16;
    return n;
}

/* 文本：200 段短字符串，每帧内容不同（避免字形缓存把开销吃掉）。 */
static void drawText(HDC hdc, int w, int h)
{
    if (w < 40 || h < 24)
        return;
    HFONT hOldFont = (HFONT)SelectObject(hdc, g_font);
    int nOldBk = SetBkMode(hdc, TRANSPARENT);
    for (int i = 0; i < kTextRuns; ++i)
    {
        char buf[96];
        RECT rc;
        int n = textItem(i, w, h, buf, sizeof(buf), &rc);
        SetTextColor(hdc, RGB((i * 29) & 0xFF, (i * 47) & 0xFF, (i * 61) & 0xFF));
        DrawTextA(hdc, buf, n, &rc, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
    }
    SetBkMode(hdc, nOldBk);
    SelectObject(hdc, hOldFont);
}

/* 只做文本布局（DT_CALCRECT），不产生任何像素。和 drawText 之差就是字形
 * 栅格化 + 合成的成本 —— 判断"慢在排版还是慢在光栅化"的分界线。 */
static void probeTextMetrics(HDC hdc, int w, int h)
{
    if (w < 40 || h < 24)
        return;
    HFONT hOldFont = (HFONT)SelectObject(hdc, g_font);
    for (int i = 0; i < kTextRuns; ++i)
    {
        char buf[96];
        RECT rc;
        int n = textItem(i, w, h, buf, sizeof(buf), &rc);
        RECT rcCalc = { 0, 0, rc.right - rc.left, rc.bottom - rc.top };
        DrawTextA(hdc, buf, n, &rcCalc,
                  DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
    }
    SelectObject(hdc, hOldFont);
}

/* 贴图：从离屏位图 1:1 贴 24 块。模拟图片控件 / 缓存重绘。 */
static void drawBlit(HDC hdc, int w, int h)
{
    if (!g_memdc || w < kBlitW || h < kBlitH)
        return;
    const int cols = 6;
    int stepX = (w - kBlitW) / cols;
    if (stepX < 1)
        stepX = 1;
    int stepY = (h - kBlitH) / 4;
    if (stepY < 1)
        stepY = 1;
    for (int i = 0; i < kBlits; ++i)
    {
        int x = (i % cols) * stepX;
        int y = ((i / cols) * stepY + g_frame * 2) % (h - kBlitH);
        BitBlt(hdc, x, y, kBlitW, kBlitH, g_memdc, 0, 0, SRCCOPY);
    }
}

/* 线条：128 条斜线。测路径/描边开销（cairo 的 stroke 与 X11 的折线）。 */
static void drawLines(HDC hdc, int w, int h)
{
    if (w < 8 || h < 8)
        return;
    for (int i = 0; i < kLines; ++i)
    {
        HPEN hOldPen = (HPEN)SelectObject(hdc, g_pens[i & 7]);
        int x0 = (i * 37 + g_frame * 3) % w;
        int y0 = (i * 53) % h;
        int x1 = (i * 91 + g_frame * 5) % w;
        int y1 = (i * 67 + 41) % h;
        MoveToEx(hdc, x0, y0, NULL);
        LineTo(hdc, x1, y1);
        SelectObject(hdc, hOldPen);
    }
}

/* 时间一个子场景，只在采样期累加（暖机帧不计入 breakdown）。
 * 一对 start/stop 约 20~30ns，相对这里是 µs 量级的图元成本可忽略。 */
#define TIMED_PART(id, expr)            \
    do                                  \
    {                                   \
        Clock::time_point _t = Clock::now(); \
        expr;                           \
        if (g_recording)                \
            g_partMs[id] += msSince(_t); \
    } while (0)

/* 画一趟场景。SCENE_EMPTY 不绘制；SCENE_TEXTMEASURE 只布局。 */
static void drawScene(HDC hdc, int w, int h)
{
    if (g_scene == SCENE_EMPTY)
        return;

    if (g_scene == SCENE_TEXTMEASURE)
    {
        Clock::time_point t = Clock::now();
        probeTextMetrics(hdc, w, h);
        if (g_recording)
            g_probeMs += msSince(t);
        return;
    }

    if (g_scene == SCENE_ALL || g_scene == SCENE_FILL)
        TIMED_PART(PART_FILL, drawFill(hdc, w, h));
    if (g_scene == SCENE_ALL || g_scene == SCENE_TEXT)
        TIMED_PART(PART_TEXT, drawText(hdc, w, h));
    if (g_scene == SCENE_ALL || g_scene == SCENE_BLIT)
        TIMED_PART(PART_BLIT, drawBlit(hdc, w, h));
    if (g_scene == SCENE_ALL || g_scene == SCENE_LINES)
        TIMED_PART(PART_LINES, drawLines(hdc, w, h));
}

/* ------------------------------------------------------------------ */
/* 资源                                                               */
/* ------------------------------------------------------------------ */

static bool createResources()
{
    for (int i = 0; i < 16; ++i)
        g_brushes[i] = CreateSolidBrush(RGB((i * 17) & 0xFF, (i * 31) & 0xFF, (i * 47) & 0xFF));
    for (int i = 0; i < 8; ++i)
        g_pens[i] = CreatePen(PS_SOLID, 1, RGB((i * 32) & 0xFF, 255 - i * 24, (i * 64) & 0xFF));
    g_font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    /* 离屏位图：只建一次，内容画一次，之后每帧只贴。 */
    HDC hScreen = GetDC(NULL);
    if (!hScreen)
        return false;
    g_memdc = CreateCompatibleDC(hScreen);
    g_membmp = CreateCompatibleBitmap(hScreen, kBlitW, kBlitH);
    if (!g_memdc || !g_membmp)
    {
        ReleaseDC(NULL, hScreen);
        return false;
    }
    HGDIOBJ hOld = SelectObject(g_memdc, g_membmp);
    for (int i = 0; i < 40; ++i)
    {
        RECT rc;
        rc.left = (i % 8) * (kBlitW / 8);
        rc.top = (i / 8) * (kBlitH / 5);
        rc.right = rc.left + kBlitW / 8;
        rc.bottom = rc.top + kBlitH / 5;
        FillRect(g_memdc, &rc, g_brushes[i & 15]);
    }
    if (hOld)
        SelectObject(g_memdc, hOld);
    ReleaseDC(NULL, hScreen);
    return true;
}

static void destroyResources()
{
    for (int i = 0; i < 16; ++i)
    {
        if (g_brushes[i])
        {
            DeleteObject(g_brushes[i]);
            g_brushes[i] = NULL;
        }
    }
    for (int i = 0; i < 8; ++i)
    {
        if (g_pens[i])
        {
            DeleteObject(g_pens[i]);
            g_pens[i] = NULL;
        }
    }
    if (g_memdc)
    {
        DeleteDC(g_memdc);
        g_memdc = NULL;
    }
    if (g_membmp)
    {
        DeleteObject(g_membmp);
        g_membmp = NULL;
    }
}

/* ------------------------------------------------------------------ */
/* 窗口                                                               */
/* ------------------------------------------------------------------ */

static LRESULT CALLBACK perfWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_PAINT:
    {
        Clock::time_point t0 = Clock::now();
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc = { 0, 0, 0, 0 };
        GetClientRect(hwnd, &rc);
        for (int r = 0; r < g_repeat; ++r)
            drawScene(hdc, rc.right, rc.bottom);
        EndPaint(hwnd, &ps);
        if (g_recording)
            g_paintMs.push_back(msSince(t0));
        ++g_frame;
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_KEYDOWN:
        if (wp == VK_ESCAPE)
        {
            g_running = false;
            DestroyWindow(hwnd);
        }
        return 0;
    case WM_CLOSE:
        g_running = false;
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

/* 泵一轮消息（不阻塞）。返回 false 表示窗口已销毁。 */
static bool pumpMessages()
{
    MSG msg;
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            return false;
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return g_running;
}

static bool drawOneFrame()
{
    InvalidateRect(g_hwnd, NULL, FALSE);
    UpdateWindow(g_hwnd);
    return pumpMessages();
}

/* 让合成器把窗口 map 出来并跑几帧（Wayland 需要收到 configure）。 */
static void warmupWindow(int msBudget)
{
    Clock::time_point t0 = Clock::now();
    while (g_running && msSince(t0) < (double)msBudget)
    {
        if (!drawOneFrame())
            return;
        Sleep(5);
    }
}

/* ------------------------------------------------------------------ */
/* 统计                                                               */
/* ------------------------------------------------------------------ */

struct CaseResult
{
    int scene;
    int repeat;
    int frames;
    double paintAvg, p50, p95, pmin, pmax;
    double wallMs, frameMs;
    double partMs[PART_COUNT];
    double probeMs;
    bool valid;
};

static std::vector<CaseResult> g_results;

static double percentile(const std::vector<double> &sorted, double p)
{
    if (sorted.empty())
        return 0.0;
    size_t idx = (size_t)((sorted.size() - 1) * p + 0.5);
    if (idx >= sorted.size())
        idx = sorted.size() - 1;
    return sorted[idx];
}

/* 跑一个用例：暖机 N 帧丢弃，再采 frames 帧。 */
static bool measureCase(int scene, int repeat, int frames, CaseResult *out)
{
    memset(out, 0, sizeof(*out));
    out->scene = scene;
    out->repeat = repeat;

    g_scene = scene;
    g_repeat = repeat;

    /* 暖机：让字形缓存、clip、合成器 buffer 进入稳定态，这几帧不计入。 */
    g_recording = false;
    resetAccum();
    for (int i = 0; i < g_warmFrames && g_running; ++i)
    {
        if (!drawOneFrame())
            return false;
    }

    resetAccum();
    g_paintMs.reserve((size_t)frames + 8);
    g_recording = true;

    Clock::time_point t0 = Clock::now();
    int painted = 0;
    while (g_running && painted < frames)
    {
        InvalidateRect(g_hwnd, NULL, FALSE);
        UpdateWindow(g_hwnd);
        painted = (int)g_paintMs.size();
        if (painted >= frames)
            break;
        if (!pumpMessages())
            break;
        if (msSince(t0) > 60000.0) /* 兜底：别把脚本挂死 */
            break;
    }
    double wallMs = msSince(t0);
    g_recording = false;

    if (g_paintMs.empty())
        return false;

    /* 采样帧数以实际落进 g_paintMs 的为准：X11 的 16ms 合帧计时器可能在两次
     * UpdateWindow 之间额外补一帧，此时 painted 会小于真实样本数。 */
    const int sampled = (int)g_paintMs.size();
    out->frames = sampled;
    out->wallMs = wallMs;
    out->frameMs = sampled > 0 ? wallMs / (double)sampled : 0.0;

    std::vector<double> v = g_paintMs;
    std::sort(v.begin(), v.end());
    double sum = 0.0;
    for (size_t i = 0; i < v.size(); ++i)
        sum += v[i];
    out->paintAvg = sum / (double)v.size();
    out->p50 = percentile(v, 0.50);
    out->p95 = percentile(v, 0.95);
    out->pmin = v.front();
    out->pmax = v.back();

    for (int i = 0; i < PART_COUNT; ++i)
        out->partMs[i] = g_partMs[i] / (double)v.size();
    out->probeMs = g_probeMs / (double)v.size();
    out->valid = true;
    return true;
}

static const CaseResult *findResult(int scene, int repeat)
{
    for (size_t i = 0; i < g_results.size(); ++i)
    {
        if (g_results[i].scene == scene && g_results[i].repeat == repeat && g_results[i].valid)
            return &g_results[i];
    }
    return NULL;
}

/* 缺哪个用例就返回 -1，方便推导段按需降级。 */
static double casePaint(int scene, int repeat)
{
    const CaseResult *r = findResult(scene, repeat);
    return r ? r->paintAvg : -1.0;
}

static int partCalls(const CaseResult &r, int part)
{
    return kPartBase[r.scene][part] * r.repeat;
}

static int passCalls(int scene)
{
    int n = 0;
    for (int i = 0; i < PART_COUNT; ++i)
        n += kPartBase[scene][i];
    return n;
}

/* ------------------------------------------------------------------ */
/* 输出 / 落盘                                                        */
/* ------------------------------------------------------------------ */

/* 落盘的 key/value 缓冲：既用于打印，也用于 --compare 读取。 */
struct Row
{
    std::string key;
    double val;
};

static std::vector<Row> g_rows;

static void addRow(const char *key, double val)
{
    Row r;
    r.key = key;
    r.val = val;
    g_rows.push_back(r);
}

static void printSweep()
{
    printf("==== perf_demo sweep ====\n");
    printf("backend  : %s\n", PERF_BACKEND_NAME);
    printf("size     : %dx%d\n", kDefaultWidth, kDefaultHeight);
    printf("frames   : %d per case (%d warmup frames discarded)\n", g_frames, g_warmFrames);
    printf("load     : %d rects / %d texts / %d blits / %d lines per pass\n",
           kFillRects, kTextRuns, kBlits, kLines);
    printf("\n%-12s %3s %6s %10s %8s %8s %8s %8s %9s %6s\n",
           "case", "rep", "frames", "paint_avg", "p50", "p95", "min", "max", "frame_ms", "paint%");
    for (size_t i = 0; i < g_results.size(); ++i)
    {
        const CaseResult &r = g_results[i];
        if (!r.valid)
            continue;
        printf("%-12s %3d %6d %10.3f %8.3f %8.3f %8.3f %8.3f %9.2f %6.1f\n",
               kSceneNames[r.scene], r.repeat, r.frames, r.paintAvg, r.p50, r.p95,
               r.pmin, r.pmax, r.frameMs,
               r.frameMs > 0.0 ? 100.0 * r.paintAvg / r.frameMs : 0.0);
    }

    /* 混合场景的分类归因 */
    const CaseResult *allr1 = findResult(SCENE_ALL, 1);
    if (allr1)
    {
        double known = 0.0;
        printf("\nbreakdown '%s' repeat=1 (avg per frame, share of paint):\n", kSceneNames[SCENE_ALL]);
        for (int p = 0; p < PART_COUNT; ++p)
        {
            int n = partCalls(*allr1, p);
            if (n <= 0)
                continue;
            known += allr1->partMs[p];
            printf("  %-6s %8.3f ms  %5.1f%%  %8.2f us/call  (%d %s)\n",
                   kPartNames[p], allr1->partMs[p],
                   allr1->paintAvg > 0.0 ? 100.0 * allr1->partMs[p] / allr1->paintAvg : 0.0,
                   1000.0 * allr1->partMs[p] / (double)n, n, kPartLoads[p]);
            std::string key = std::string("all.r1.part_") + kPartNames[p];
            addRow(key.c_str(), allr1->partMs[p]);
        }
        double other = allr1->paintAvg - known;
        if (other < 0.0)
            other = 0.0;
        printf("  %-6s %8.3f ms  %5.1f%%   BeginPaint/clip + EndPaint/commit\n",
               "other", other,
               allr1->paintAvg > 0.0 ? 100.0 * other / allr1->paintAvg : 0.0);
        addRow("all.r1.part_other", other);
    }

    /* 推导段：把"每帧固定成本 / 布局 / 字形栅格 / 每趟净成本 / 每次调用成本"
     * 一次算完，避免再分参数跑几轮。 */
    double fixed = casePaint(SCENE_EMPTY, 1);
    double tm = casePaint(SCENE_TEXTMEASURE, 1);
    double txt = casePaint(SCENE_TEXT, 1);

    printf("\nderived (ms/frame unless noted):\n");
    if (fixed >= 0.0)
    {
        printf("  %-16s %8.3f   BeginPaint/clip + 端到端提交 + pump\n", "fixed", fixed);
        addRow("derived.fixed", fixed);
    }
    if (tm >= 0.0)
    {
        double layout = tm - (fixed >= 0.0 ? fixed : 0.0);
        printf("  %-16s %8.3f   %6.2f us/run   (textmeasure - fixed)\n",
               "layout", layout, 1000.0 * layout / (double)kTextRuns);
        addRow("derived.layout", layout);
    }
    if (tm >= 0.0 && txt >= 0.0)
    {
        double raster = txt - tm;
        printf("  %-16s %8.3f   %6.2f us/run   (text - textmeasure)\n",
               "glyph_raster", raster, 1000.0 * raster / (double)kTextRuns);
        addRow("derived.glyph_raster", raster);
    }

    for (size_t i = 0; i < g_results.size(); ++i)
    {
        const CaseResult &r = g_results[i];
        int calls = passCalls(r.scene);
        if (!r.valid || r.repeat != 1 || calls <= 0)
            continue;
        const CaseResult *r4 = findResult(r.scene, 4);
        char key[128];

        /* net = paint - fixed：只有拿到 empty 基线时才有意义。极便宜的图元
         * （blit）会和 fixed 处于同一量级的噪声里，负值按 0 处理。 */
        if (fixed >= 0.0)
        {
            double net = r.paintAvg - fixed;
            if (net < 0.0)
                net = 0.0;
            printf("  %-16s %8.3f   %6.2f us/call  (paint - fixed)\n",
                   (std::string(kSceneNames[r.scene]) + "_net").c_str(), net,
                   1000.0 * net / (double)calls);
            snprintf(key, sizeof(key), "derived.%s.net", kSceneNames[r.scene]);
            addRow(key, net);
        }

        if (r4)
        {
            double perPass = (r4->paintAvg - r.paintAvg) / 3.0;
            printf("  %-16s %8.3f   %6.2f us/call  ((r4 - r1) / 3)\n",
                   (std::string(kSceneNames[r.scene]) + "_per_pass").c_str(), perPass,
                   1000.0 * perPass / (double)calls);
            snprintf(key, sizeof(key), "derived.%s.per_pass", kSceneNames[r.scene]);
            addRow(key, perPass);
        }
    }
    printf("====================\n");
    fflush(stdout);
}

static void saveSweep(const char *path)
{
    FILE *f = fopen(path, "w");
    if (!f)
    {
        printf("perf_demo: 无法写入 %s\n", path);
        return;
    }
    fprintf(f, "# perf_demo results (key=value, 供 --compare 读取)\n");
    fprintf(f, "backend=%s\n", PERF_BACKEND_NAME);
    fprintf(f, "size=%dx%d\n", kDefaultWidth, kDefaultHeight);
    fprintf(f, "frames=%d\n", g_frames);
    fprintf(f, "warmup=%d\n", g_warmFrames);
    fprintf(f, "load=%d,%d,%d,%d\n", kFillRects, kTextRuns, kBlits, kLines);

    for (size_t i = 0; i < g_results.size(); ++i)
    {
        const CaseResult &r = g_results[i];
        if (!r.valid)
            continue;
        const char *n = kSceneNames[r.scene];
        fprintf(f, "%s.r%d.frames=%d\n", n, r.repeat, r.frames);
        fprintf(f, "%s.r%d.paint=%.4f\n", n, r.repeat, r.paintAvg);
        fprintf(f, "%s.r%d.p50=%.4f\n", n, r.repeat, r.p50);
        fprintf(f, "%s.r%d.p95=%.4f\n", n, r.repeat, r.p95);
        fprintf(f, "%s.r%d.min=%.4f\n", n, r.repeat, r.pmin);
        fprintf(f, "%s.r%d.max=%.4f\n", n, r.repeat, r.pmax);
        fprintf(f, "%s.r%d.frame=%.4f\n", n, r.repeat, r.frameMs);
        fprintf(f, "%s.r%d.paint_pct=%.2f\n", n, r.repeat,
                r.frameMs > 0.0 ? 100.0 * r.paintAvg / r.frameMs : 0.0);
        for (int p = 0; p < PART_COUNT; ++p)
        {
            if (kPartBase[r.scene][p] <= 0)
                continue;
            fprintf(f, "%s.r%d.part_%s=%.4f\n", n, r.repeat, kPartNames[p], r.partMs[p]);
        }
        if (r.scene == SCENE_TEXTMEASURE)
            fprintf(f, "%s.r%d.layout=%.4f\n", n, r.repeat, r.probeMs);
    }
    for (size_t i = 0; i < g_rows.size(); ++i)
        fprintf(f, "%s=%.4f\n", g_rows[i].key.c_str(), g_rows[i].val);
    fclose(f);
    printf("已保存: %s\n", path);
}

/* ------------------------------------------------------------------ */
/* 比较模式                                                           */
/* ------------------------------------------------------------------ */

struct Saved
{
    std::string path;
    std::string backend;
    std::string size;
    std::vector<std::string> order;
    std::map<std::string, double> vals;
};

static std::string trimStr(const std::string &s)
{
    size_t b = 0, e = s.size();
    while (b < e && (s[b] == ' ' || s[b] == '\t' || s[b] == '\r' || s[b] == '\n'))
        ++b;
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\r' || s[e - 1] == '\n'))
        --e;
    return s.substr(b, e - b);
}

static bool loadSaved(const char *path, Saved *out)
{
    FILE *f = fopen(path, "r");
    if (!f)
    {
        printf("perf_demo: 打不开 %s\n", path);
        return false;
    }
    out->path = path;
    out->backend = "?";
    char line[512];
    while (fgets(line, sizeof(line), f))
    {
        std::string s = trimStr(std::string(line));
        if (s.empty() || s[0] == '#')
            continue;
        size_t eq = s.find('=');
        if (eq == std::string::npos)
            continue;
        std::string key = trimStr(s.substr(0, eq));
        std::string val = trimStr(s.substr(eq + 1));
        if (key == "backend")
        {
            out->backend = val;
            continue;
        }
        if (key == "size")
        {
            out->size = val;
            continue;
        }
        if (key == "frames" || key == "warmup" || key == "load")
            continue;
        double d = atof(val.c_str());
        if (out->vals.find(key) == out->vals.end())
            out->order.push_back(key);
        out->vals[key] = d;
    }
    fclose(f);
    return true;
}

static void printCompareRow(const Saved &a, const Saved &b, const std::string &key)
{
    std::map<std::string, double>::const_iterator ia = a.vals.find(key);
    std::map<std::string, double>::const_iterator ib = b.vals.find(key);
    bool ha = ia != a.vals.end();
    bool hb = ib != b.vals.end();
    char ca[32], cb[32], cd[32], cr[32];
    if (ha)
        snprintf(ca, sizeof(ca), "%.3f", ia->second);
    else
        strcpy(ca, "-");
    if (hb)
        snprintf(cb, sizeof(cb), "%.3f", ib->second);
    else
        strcpy(cb, "-");
    if (ha && hb)
    {
        snprintf(cd, sizeof(cd), "%+.3f", ib->second - ia->second);
        if (ia->second > 0.0005)
            snprintf(cr, sizeof(cr), "%.2fx", ib->second / ia->second);
        else
            strcpy(cr, "-");
    }
    else
    {
        strcpy(cd, "-");
        strcpy(cr, "-");
    }
    printf("%-30s %9s %9s %9s %8s\n", key.c_str(), ca, cb, cd, cr);
}

/* 比较表里优先给出的行：先看这几行就能定性，其余作全量备查。 */
static const char *kHighlight[] = {
    "all.r1.paint",
    "all.r1.p95",
    "all.r4.paint",
    "empty.r1.paint",
    "derived.layout",
    "derived.glyph_raster",
    "derived.all.per_pass",
    "derived.fill.per_pass",
    "derived.text.per_pass",
    "derived.blit.per_pass",
    "derived.lines.per_pass",
    "all.r1.part_fill",
    "all.r1.part_text",
    "all.r1.part_blit",
    "all.r1.part_lines",
    "all.r1.part_other",
};

static bool endsWith(const std::string &s, const char *suffix)
{
    size_t n = strlen(suffix);
    return s.size() >= n && s.compare(s.size() - n, n, suffix) == 0;
}

static void printCompare(const Saved &a, const Saved &b)
{
    printf("==== perf_demo compare ====\n");
    printf("A : %-10s %s\n", a.backend.c_str(), a.path.c_str());
    printf("B : %-10s %s\n", b.backend.c_str(), b.path.c_str());
    if (!a.size.empty() && !b.size.empty() && a.size != b.size)
        printf("(!) 两边窗口尺寸不同: %s vs %s，绝对值不可直接比\n", a.size.c_str(), b.size.c_str());

    /* 键的遍历顺序：先 A 的落盘顺序（即用例顺序），再补 B 独有的。 */
    std::vector<std::string> all;
    for (size_t i = 0; i < a.order.size(); ++i)
        all.push_back(a.order[i]);
    for (size_t i = 0; i < b.order.size(); ++i)
    {
        if (a.vals.find(b.order[i]) == a.vals.end())
            all.push_back(b.order[i]);
    }

    std::map<std::string, bool> done;
    printf("\n-- 关键指标 --\n");
    printf("%-30s %9s %9s %9s %8s\n", "metric", "A", "B", "B-A", "B/A");
    printf("-----------------------------------------------------------------------\n");
    for (size_t i = 0; i < sizeof(kHighlight) / sizeof(kHighlight[0]); ++i)
    {
        bool present = false;
        for (size_t k = 0; k < all.size(); ++k)
        {
            if (all[k] == kHighlight[i])
            {
                present = true;
                break;
            }
        }
        if (!present)
            continue;
        printCompareRow(a, b, kHighlight[i]);
        done[kHighlight[i]] = true;
    }

    printf("\n-- 全量指标 --\n");
    printf("%-30s %9s %9s %9s %8s\n", "metric", "A", "B", "B-A", "B/A");
    printf("-----------------------------------------------------------------------\n");
    for (size_t i = 0; i < all.size(); ++i)
    {
        const std::string &k = all[i];
        if (done.find(k) != done.end())
            continue;
        if (endsWith(k, ".frames"))
            continue; /* 采样帧数，属元数据 */
        printCompareRow(a, b, k);
    }
    printf("=======================================================================\n");
    fflush(stdout);
}

/* ------------------------------------------------------------------ */
/* 入口                                                               */
/* ------------------------------------------------------------------ */

static int parseScene(const char *name)
{
    for (int i = 0; i < SCENE_COUNT; ++i)
    {
        if (strcmp(name, kSceneNames[i]) == 0)
            return i;
    }
    /* 允许只写场景名的前缀，省得记全名（textme -> textmeasure）。 */
    for (int i = 0; i < SCENE_COUNT; ++i)
    {
        if (strncmp(name, kSceneNames[i], strlen(name)) == 0)
            return i;
    }
    return -1;
}

static void splitList(const char *s, std::vector<std::string> &out)
{
    std::string cur;
    for (const char *p = s; *p; ++p)
    {
        if (*p == ',')
        {
            cur = trimStr(cur);
            if (!cur.empty())
                out.push_back(cur);
            cur.clear();
        }
        else
        {
            cur += *p;
        }
    }
    cur = trimStr(cur);
    if (!cur.empty())
        out.push_back(cur);
}

static void usage()
{
    printf(
        "perf_demo -- X11/Wayland 后端绘制性能对比（一次跑完全部用例）\n"
        "\n"
        "  perf_demo [--frames=N] [--repeat=1,4] [--case=a,b,...] [--save=FILE]\n"
        "            [--warmup=N] [--hold] [--compare=A,B] [--help]\n"
        "\n"
        "  --frames=N     每个用例采样帧数，默认 100\n"
        "  --repeat=LIST  每次 WM_PAINT 内重复绘制次数，默认 1,4（用来解出\n"
        "                 每帧固定成本与每趟图元成本：perPass=(p4-p1)/3）\n"
        "  --case=LIST    只跑指定场景：all/fill/text/blit/lines/empty/textmeasure\n"
        "  --save=FILE    结果落盘，默认 perf_<backend>.txt\n"
        "  --warmup=N     每个用例前丢弃的暖机帧，默认 3\n"
        "  --hold         跑完保持窗口（ESC 退出）\n"
        "  --compare=A,B  只比较两份结果文件，不开窗\n");
}

int main(int argc, char **argv)
{
    std::vector<int> cases;
    std::vector<int> repeats;
    std::string savePath;
    const char *cmpA = NULL;
    const char *cmpB = NULL;

    for (int i = 1; i < argc; ++i)
    {
        const char *a = argv[i];
        if (strcmp(a, "--hold") == 0)
            g_hold = true;
        else if (strcmp(a, "--help") == 0 || strcmp(a, "-h") == 0)
        {
            usage();
            return 0;
        }
        else if (strncmp(a, "--frames=", 9) == 0)
            g_frames = atoi(a + 9);
        else if (strncmp(a, "--warmup=", 9) == 0)
            g_warmFrames = atoi(a + 9);
        else if (strncmp(a, "--save=", 7) == 0)
            savePath = a + 7;
        else if (strncmp(a, "--repeat=", 9) == 0)
        {
            std::vector<std::string> list;
            splitList(a + 9, list);
            for (size_t k = 0; k < list.size(); ++k)
            {
                int n = atoi(list[k].c_str());
                if (n >= 0)
                    repeats.push_back(n);
            }
        }
        else if (strncmp(a, "--case=", 7) == 0)
        {
            std::vector<std::string> list;
            splitList(a + 7, list);
            for (size_t k = 0; k < list.size(); ++k)
            {
                int sc = parseScene(list[k].c_str());
                if (sc >= 0)
                {
                    cases.push_back(sc);
                }
                else
                {
                    printf("perf_demo: 未知场景 '%s'\n", list[k].c_str());
                    return 1;
                }
            }
        }
        else if (strncmp(a, "--compare=", 10) == 0)
        {
            std::vector<std::string> list;
            splitList(a + 10, list);
            if (list.size() != 2)
            {
                printf("perf_demo: --compare 需要两个文件路径，用逗号分隔\n");
                return 1;
            }
            cmpA = strdup(list[0].c_str());
            cmpB = strdup(list[1].c_str());
        }
        else if (a[0] >= '0' && a[0] <= '9')
            g_frames = atoi(a);
        else
        {
            /* 兼容旧用法：位置参数写场景名 */
            int sc = parseScene(a);
            if (sc < 0)
            {
                printf("perf_demo: 无法识别的参数 '%s'（--help 看用法）\n", a);
                return 1;
            }
            cases.push_back(sc);
        }
    }

    if (cmpA && cmpB)
    {
        Saved sa, sb;
        if (!loadSaved(cmpA, &sa) || !loadSaved(cmpB, &sb))
            return 2;
        printCompare(sa, sb);
        free((void *)cmpA);
        free((void *)cmpB);
        return 0;
    }

    if (g_frames < 1)
        g_frames = 100;
    if (g_warmFrames < 0)
        g_warmFrames = 0;
    if (repeats.empty())
    {
        /* 默认跑 1 和 4，用来解 fixed / perPass；只指定单个场景时也保留
         * 这一对，否则无法把"每帧固定成本"和"每趟图元成本"分开。 */
        repeats.push_back(1);
        repeats.push_back(4);
    }
    if (cases.empty())
    {
        for (size_t i = 0; i < sizeof(kSweepScenes) / sizeof(kSweepScenes[0]); ++i)
            cases.push_back(kSweepScenes[i]);
    }
    if (savePath.empty())
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "perf_%s.txt", PERF_BACKEND_NAME);
        savePath = buf;
    }

    WNDCLASSEXA wc;
    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.style = 0;
    wc.lpfnWndProc = perfWndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.lpszClassName = "SWINX_PERF_DEMO";
    if (!RegisterClassExA(&wc))
    {
        printf("perf_demo: RegisterClassExA failed (%lu)\n", (unsigned long)GetLastError());
        return 2;
    }

    char title[160];
    snprintf(title, sizeof(title), "perf_demo [%s] sweeping %d cases", PERF_BACKEND_NAME,
             (int)cases.size());
    g_hwnd = CreateWindowExA(0, wc.lpszClassName, title,
                             WS_POPUP | WS_VISIBLE,
                             0, 0, kDefaultWidth, kDefaultHeight,
                             NULL, NULL, wc.hInstance, NULL);
    if (!g_hwnd)
    {
        printf("perf_demo: CreateWindowExA failed (%lu)\n", (unsigned long)GetLastError());
        return 3;
    }

    if (!createResources())
    {
        printf("perf_demo: create GDI resources failed\n");
        DestroyWindow(g_hwnd);
        return 4;
    }

    /* Wayland 下窗口要等合成器 configure 才真正上屏；先跑 500ms 暖机。 */
    warmupWindow(500);

    for (size_t ci = 0; ci < cases.size() && g_running; ++ci)
    {
        for (size_t ri = 0; ri < repeats.size() && g_running; ++ri)
        {
            /* empty / textmeasure 没有图元，repeat 放大没有意义，只跑 1 次。 */
            if (repeats[ri] != 1 && passCalls(cases[ci]) <= 0)
                continue;
            CaseResult r;
            bool ok = measureCase(cases[ci], repeats[ri], g_frames, &r);
            if (!ok)
            {
                if (!g_running)
                    break;
                printf("perf_demo: case %s r%d 没有采到帧，跳过\n",
                       kSceneNames[cases[ci]], repeats[ri]);
                continue;
            }
            g_results.push_back(r);
            printf("  ... %-12s repeat=%d  paint %.3f ms  (%.0f fps)\n",
                   kSceneNames[r.scene], r.repeat, r.paintAvg,
                   r.frameMs > 0.0 ? 1000.0 / r.frameMs : 0.0);
            fflush(stdout);
        }
    }

    printf("\n");
    printSweep();
    if (g_running)
        saveSweep(savePath.c_str());

    const CaseResult *allr1 = findResult(SCENE_ALL, 1);
    if (g_hwnd && allr1)
    {
        char t[200];
        snprintf(t, sizeof(t), "perf_demo [%s] all avg %.2f ms  p95 %.2f ms",
                 PERF_BACKEND_NAME, allr1->paintAvg, allr1->p95);
        SetWindowTextA(g_hwnd, t);
    }
    printf("\n下一步: 在另一个后端跑一次，然后\n"
           "  perf_demo --compare=<另一份结果文件>,%s\n", savePath.c_str());
    fflush(stdout);

    if (g_hold && g_running)
    {
        printf("perf_demo: hold mode, press ESC or close the window to exit\n");
        fflush(stdout);
        MSG msg;
        while (g_running && GetMessageA(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }

    g_running = false;
    if (g_hwnd)
    {
        DestroyWindow(g_hwnd);
        g_hwnd = NULL;
    }
    destroyResources();
    return 0;
}
