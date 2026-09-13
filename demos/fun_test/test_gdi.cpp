#include <souistd.h>
#include "common.h"
#include <math.h>

using namespace SOUI;
#define kLogTag "test_native"

static const TCHAR* kPath_TestPng = _T("/uires/image/soui.png");
static const TCHAR* kPath_YyPng   = _T("/uires/image/yy.png");
static const TCHAR* kPath_Icon    = _T("/uires/image/soui.ico");
#define GDI_GROUP_COUNT 15

class SGdiTestWnd : public SNativeWnd {
    int m_groupIndex;
    HBITMAP m_testBmp;
    HBITMAP m_yyBmp;

public:
    SGdiTestWnd() : m_groupIndex(10), m_testBmp(0), m_yyBmp(0) {}
    ~SGdiTestWnd() {}

protected:
    int OnCreate(LPCREATESTRUCT lpCs) {
        SOUI::SStringT srcDir = getSourceDir();
        m_testBmp = LoadPng((srcDir + kPath_TestPng).c_str());
        m_yyBmp   = LoadPng((srcDir + kPath_YyPng).c_str());
        SetMsgHandled(FALSE);
        return 0;
    }

    void OnDestroy() {
        if (m_testBmp) {
            DeleteObject(m_testBmp);
            m_testBmp = 0;
        }
        if (m_yyBmp) {
            DeleteObject(m_yyBmp);
            m_yyBmp = 0;
        }
        SetMsgHandled(FALSE);
    }

    void OnClose() {
        PostThreadMessage(GetCurrentThreadId(), WM_QUIT, 1, 0);
    }

    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
        if (nChar == VK_SPACE) {
            m_groupIndex = (m_groupIndex + 1) % GDI_GROUP_COUNT;
            Invalidate();
            SetMsgHandled(TRUE);
        } else if (nChar == VK_ESCAPE) {
            OnClose();
            SetMsgHandled(TRUE);
        } else {
            SetMsgHandled(FALSE);
        }
    }

    void DrawLabel(HDC hdc, int x, int y, const SStringA& label) {
        COLORREF oldTxt = SetTextColor(hdc, RGB(0, 0, 255));
        int oldBk = SetBkMode(hdc, TRANSPARENT);
        LOGFONTA lf = { 0 };
        lf.lfHeight = 14;
        strcpy_s(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), "Arial");
        HFONT hFont = CreateFontIndirectA(&lf);
        HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
        // 强制 TA_NOUPDATECP：页面可能残留 TA_UPDATECP，那会让这里的 (x,y) 失效
        UINT oldAlign = SetTextAlign(hdc, TA_NOUPDATECP | TA_LEFT | TA_TOP);
        TextOutA(hdc, x, y, label.c_str(), label.GetLength());
        SetTextAlign(hdc, oldAlign);
        SelectObject(hdc, oldFont);
        DeleteObject(hFont);
        SetBkMode(hdc, oldBk);
        SetTextColor(hdc, oldTxt);
    }

    void DrawHeader(HDC hdc, const RECT& rc) {
        SStringA header;
        header.Format("=== GDI API Test Group %d/%d (Press SPACE to switch, ESC to exit) ===",
            m_groupIndex + 1, GDI_GROUP_COUNT);
        SetTextColor(hdc, RGB(255, 0, 0));
        SetBkColor(hdc, RGB(255, 255, 0));
        SetBkMode(hdc, OPAQUE);
        LOGFONTA lf = { 0 };
        lf.lfHeight = 18;
        lf.lfWeight = FW_BOLD;
        strcpy_s(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), "Arial");
        HFONT hFont = CreateFontIndirectA(&lf);
        HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
        RECT rcHeader = { 10, 5, rc.right - 10, 30 };
        DrawTextA(hdc, header.c_str(), -1, &rcHeader, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
        SelectObject(hdc, oldFont);
        DeleteObject(hFont);
    }
    HRESULT DrawBitmapEx(HDC hdc, LPCRECT pRcDest, HBITMAP bmp, LPCRECT pRcSrc, BOOL bTile, BYTE byAlpha /*=0xFF*/)
    {
        HDC hmemdc = CreateCompatibleDC(hdc);
        ::SelectObject(hmemdc, bmp);

        BLENDFUNCTION bf = { AC_SRC_OVER, 0, byAlpha, AC_SRC_ALPHA };
        if (!bTile)
        {
            ::AlphaBlend(hdc, pRcDest->left, pRcDest->top, pRcDest->right - pRcDest->left, pRcDest->bottom - pRcDest->top, hmemdc, pRcSrc->left, pRcSrc->top, pRcSrc->right - pRcSrc->left, pRcSrc->bottom - pRcSrc->top, bf);
        }
        else
        {
            ::SaveDC(hdc);
            ::IntersectClipRect(hdc, pRcDest->left, pRcDest->top, pRcDest->right, pRcDest->bottom);
            int nWid = pRcSrc->right - pRcSrc->left;
            int nHei = pRcSrc->bottom - pRcSrc->top;
            for (int y = pRcDest->top; y < pRcDest->bottom; y += nHei)
            {
                for (int x = pRcDest->left; x < pRcDest->right; x += nWid)
                {
                    ::AlphaBlend(hdc, x, y, nWid, nHei, hmemdc, pRcSrc->left, pRcSrc->top, nWid, nHei, bf);
                }
            }
            ::RestoreDC(hdc, -1);
        }
        DeleteDC(hmemdc);
        return S_OK;
    }

    HRESULT DrawBitmap9Patch(HDC hdc, LPCRECT pRcDest, HBITMAP bmp, LPCRECT pRcSrc, LPCRECT pRcSourMargin, BOOL bTile, BYTE byAlpha /*=0xFF*/)
    {
        int xDest[4] = { pRcDest->left, pRcDest->left + pRcSourMargin->left, pRcDest->right - pRcSourMargin->right, pRcDest->right };
        int xSrc[4] = { pRcSrc->left, pRcSrc->left + pRcSourMargin->left, pRcSrc->right - pRcSourMargin->right, pRcSrc->right };
        int yDest[4] = { pRcDest->top, pRcDest->top + pRcSourMargin->top, pRcDest->bottom - pRcSourMargin->bottom, pRcDest->bottom };
        int ySrc[4] = { pRcSrc->top, pRcSrc->top + pRcSourMargin->top, pRcSrc->bottom - pRcSourMargin->bottom, pRcSrc->bottom };

        // 首先保证九宫分割正常
        if (!(xSrc[0] <= xSrc[1] && xSrc[1] <= xSrc[2] && xSrc[2] <= xSrc[3]))
            return S_FALSE;
        if (!(ySrc[0] <= ySrc[1] && ySrc[1] <= ySrc[2] && ySrc[2] <= ySrc[3]))
            return S_FALSE;

        // 调整目标位置
        int nDestWid = pRcDest->right - pRcDest->left;
        int nDestHei = pRcDest->bottom - pRcDest->top;

        if ((pRcSourMargin->left + pRcSourMargin->right) > nDestWid)
        { // 边缘宽度大于目标宽度的处理
            if (pRcSourMargin->left >= nDestWid)
            { // 只绘制左边部分
                xSrc[1] = xSrc[2] = xSrc[3] = xSrc[0] + nDestWid;
                xDest[1] = xDest[2] = xDest[3] = xDest[0] + nDestWid;
            }
            else if (pRcSourMargin->right >= nDestWid)
            { // 只绘制右边部分
                xSrc[0] = xSrc[1] = xSrc[2] = xSrc[3] - nDestWid;
                xDest[0] = xDest[1] = xDest[2] = xDest[3] - nDestWid;
            }
            else
            { // 先绘制左边部分，剩余的用右边填充
                int nRemain = xDest[3] - xDest[1];
                xSrc[2] = xSrc[3] - nRemain;
                xDest[2] = xDest[3] - nRemain;
            }
        }

        if (pRcSourMargin->top + pRcSourMargin->bottom > nDestHei)
        {
            if (pRcSourMargin->top >= nDestHei)
            { // 只绘制上边部分
                ySrc[1] = ySrc[2] = ySrc[3] = ySrc[0] + nDestHei;
                yDest[1] = yDest[2] = yDest[3] = yDest[0] + nDestHei;
            }
            else if (pRcSourMargin->bottom >= nDestHei)
            { // 只绘制下边部分
                ySrc[0] = ySrc[1] = ySrc[2] = ySrc[3] - nDestHei;
                yDest[0] = yDest[1] = yDest[2] = yDest[3] - nDestHei;
            }
            else
            { // 先绘制左边部分，剩余的用右边填充
                int nRemain = yDest[3] - yDest[1];
                ySrc[2] = ySrc[3] - nRemain;
                yDest[2] = yDest[3] - nRemain;
            }
        }

        for (int y = 0; y < 3; y++)
        {
            if (ySrc[y] == ySrc[y + 1])
                continue;
            for (int x = 0; x < 3; x++)
            {
                if (xSrc[x] == xSrc[x + 1])
                    continue;
                RECT rcSrc = { xSrc[x], ySrc[y], xSrc[x + 1], ySrc[y + 1] };
                RECT rcDest = { xDest[x], yDest[y], xDest[x + 1], yDest[y + 1] };
                DrawBitmapEx(hdc,&rcDest, bmp, &rcSrc, FALSE, byAlpha);
            }
        }

        return S_OK;
    }

    // ========== Group 1: Basic Line & Shape Drawing ==========
    void DrawGroup1(HDC hdc, const RECT& rc) {
        int y0 = 40;
        int x0 = 30;

        // 1. MoveToEx + LineTo
        HPEN hPen1 = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
        HPEN oldPen = (HPEN)SelectObject(hdc, hPen1);
        MoveToEx(hdc, x0, y0, NULL);
        LineTo(hdc, x0 + 200, y0);
        SelectObject(hdc, oldPen);
        DeleteObject(hPen1);
        DrawLabel(hdc, x0 + 210, y0 - 8, "#1 MoveToEx+LineTo");

        // 2. Polyline
        POINT pts[] = { {x0, y0 + 30}, {x0 + 50, y0 + 60}, {x0 + 100, y0 + 30}, {x0 + 150, y0 + 60}, {x0 + 200, y0 + 30} };
        hPen1 = CreatePen(PS_SOLID, 2, RGB(0, 128, 0));
        oldPen = (HPEN)SelectObject(hdc, hPen1);
        Polyline(hdc, pts, 5);
        SelectObject(hdc, oldPen);
        DeleteObject(hPen1);
        DrawLabel(hdc, x0 + 210, y0 + 35, "#2 Polyline");

        // 3. Rectangle (filled)
        HBRUSH hBr1 = CreateSolidBrush(RGB(255, 200, 100));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, hBr1);
        hPen1 = CreatePen(PS_SOLID, 2, RGB(128, 0, 0));
        oldPen = (HPEN)SelectObject(hdc, hPen1);
        Rectangle(hdc, x0, y0 + 80, x0 + 150, y0 + 140);
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBr);
        DeleteObject(hPen1);
        DeleteObject(hBr1);
        DrawLabel(hdc, x0 + 160, y0 + 100, "#3 Rectangle");

        // 4. RoundRect
        hBr1 = CreateSolidBrush(RGB(100, 200, 255));
        oldBr = (HBRUSH)SelectObject(hdc, hBr1);
        hPen1 = CreatePen(PS_SOLID, 2, RGB(0, 0, 128));
        oldPen = (HPEN)SelectObject(hdc, hPen1);
        RoundRect(hdc, x0 + 220, y0 + 80, x0 + 400, y0 + 140, 20, 30);
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBr);
        DeleteObject(hPen1);
        DeleteObject(hBr1);
        DrawLabel(hdc, x0 + 410, y0 + 100, "#4 RoundRect");

        // 5. Rectangle (NULL_BRUSH + NULL_PEN combos)
        int x = x0, y = y0 + 160;
        RECT rcShape = { x, y, x + 80, y + 50 };
        // 5a: solid brush + null pen
        oldBr = (HBRUSH)SelectObject(hdc, GetStockObject(GRAY_BRUSH));
        oldPen = (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));
        Rectangle(hdc, rcShape.left, rcShape.top, rcShape.right, rcShape.bottom);
        OffsetRect(&rcShape, 100, 0);
        // 5b: null brush + solid pen
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        SelectObject(hdc, GetStockObject(BLACK_PEN));
        Rectangle(hdc, rcShape.left, rcShape.top, rcShape.right, rcShape.bottom);
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBr);
        DrawLabel(hdc, x0 + 210, y0 + 175, "#5 Rectangle(NULL pen/brush)");

        // 6. Polygon (triangle)
        POINT polyPts[] = { {x0, y0 + 240}, {x0 + 60, y0 + 240}, {x0 + 30, y0 + 190} };
        hBr1 = CreateSolidBrush(RGB(200, 100, 200));
        oldBr = (HBRUSH)SelectObject(hdc, hBr1);
        hPen1 = CreatePen(PS_SOLID, 2, RGB(128, 0, 128));
        oldPen = (HPEN)SelectObject(hdc, hPen1);
        Polygon(hdc, polyPts, 3);
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBr);
        DeleteObject(hPen1);
        DeleteObject(hBr1);
        DrawLabel(hdc, x0 + 80, y0 + 210, "#6 Polygon");

        // 7. DrawIcon + DrawIconEx
        SOUI::SStringT strIcon = getSourceDir() + kPath_Icon;
        HICON hIcon = (HICON)LoadImage(0,strIcon,IMAGE_ICON,48,48, LR_LOADFROMFILE);
        if(hIcon){
            DrawIconEx(hdc, x0, y0 + 230, hIcon, 32, 32, 0, NULL, DI_NORMAL);
            DrawIconEx(hdc, x0 + 50, y0 + 230, hIcon, 48, 48, 0, NULL, DI_NORMAL);
            DrawIconEx(hdc, x0 + 120, y0 + 230, hIcon, 64, 64, 0, NULL, DI_NORMAL);
            DestroyIcon(hIcon);
        }
        DrawLabel(hdc, x0 + 200, y0 + 250, "#7 DrawIcon/DrawIconEx");

        // 8. 位图绘制 (DrawBitmapEx: stretch + tile)
        if(1){
            HBITMAP hBmp = LoadPng(getSourceDir() + kPath_YyPng);
            BITMAP bm;
            GetObject(hBmp, sizeof(bm), &bm);
            // 8a: 原始尺寸
            RECT rcSrc = {0, 0, bm.bmWidth, bm.bmHeight};
            RECT rcDst1 = {x0, y0 + 300, x0 + bm.bmWidth, y0 + 300 + bm.bmHeight};
            DrawBitmapEx(hdc, &rcDst1, hBmp, &rcSrc, FALSE, 255);
            // 8b: 拉伸到 80x80
            RECT rcDst2 = {x0 + 70, y0 + 300, x0 + 150, y0 + 380};
            DrawBitmapEx(hdc, &rcDst2, hBmp, &rcSrc, FALSE, 255);
            // 8c: 半透明拉伸
            RECT rcDst3 = {x0 + 160, y0 + 300, x0 + 260, y0 + 380};
            DrawBitmapEx(hdc, &rcDst3, hBmp, &rcSrc, FALSE, 128);
            DeleteObject(hBmp);
        }
        DrawLabel(hdc, x0 + 280, y0 + 330, "#8 DrawBitmapEx (stretch/alpha)");

        // 9. AlphaBlend 九宫格绘制 (DrawBitmap9Patch)
        {
            HBITMAP hBmp = m_testBmp;
            BITMAP bm;
            GetObject(hBmp, sizeof(bm), &bm);
            // 定义九宫格边距：四角 20x20 不拉伸
            RECT rcSrc = {0, 0, bm.bmWidth, bm.bmHeight};
            RECT rcMargin = {20, 20, 20, 20};
            // 9a: 拉伸模式九宫格
            RECT rcDst1 = {x0 + 380, y0 + 230, x0 + 380+ bm.bmWidth+50, y0 + 230 + bm.bmHeight+50};
            DrawBitmap9Patch(hdc, &rcDst1, hBmp, &rcSrc, &rcMargin, FALSE, 255);
            DrawLabel(hdc, x0 + 590, y0 + 280, "#9a 9-Patch Stretch");
        }

        // 10. AlphaBlend 直接绘制
        if(1){
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hBmp = m_testBmp;
            SelectObject(hdcMem,hBmp);
            BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
            AlphaBlend(hdc,x0,y0+400,64,64,hdcMem,0,0,64,64,bf);
            // 半透明
            BLENDFUNCTION bf2 = { AC_SRC_OVER, 0, 128, AC_SRC_ALPHA };
            AlphaBlend(hdc,x0+80,y0+400,64,64,hdcMem,64,64,64,64,bf2);
            DeleteDC(hdcMem);
        }
        DrawLabel(hdc, x0 + 160, y0 + 430, "#10 AlphaBlend");
    }

    // ========== Group 2: Curves & Circle Shapes ==========
    void DrawGroup2(HDC hdc, const RECT& rc) {
        int y0 = 40;
        int x0 = 30;

        // 7. Ellipse (solid fill)
        HBRUSH hBr1 = CreateSolidBrush(RGB(255, 180, 180));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, hBr1);
        HPEN hPen1 = CreatePen(PS_SOLID, 2, RGB(200, 0, 0));
        HPEN oldPen = (HPEN)SelectObject(hdc, hPen1);
        Ellipse(hdc, x0, y0, x0 + 160, y0 + 100);
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBr);
        DeleteObject(hPen1);
        DeleteObject(hBr1);
        DrawLabel(hdc, x0 + 170, y0 + 40, "#7 Ellipse(filled)");

        // 8. Arc
        RECT rcArc = { x0 + 240, y0, x0 + 380, y0 + 120 };
        hPen1 = CreatePen(PS_SOLID, 3, RGB(0, 0, 200));
        oldPen = (HPEN)SelectObject(hdc, hPen1);
        Arc(hdc, rcArc.left, rcArc.top, rcArc.right, rcArc.bottom,
            rcArc.left, rcArc.top, rcArc.right, rcArc.bottom);
        SelectObject(hdc, oldPen);
        DeleteObject(hPen1);
        DrawLabel(hdc, x0 + 390, y0 + 40, "#8 Arc");

        int y1 = y0 + 140;

        // 9. Pie (filled)
        RECT rcPie = { x0, y1, x0 + 160, y1 + 120 };
        hBr1 = CreateSolidBrush(RGB(180, 255, 180));
        oldBr = (HBRUSH)SelectObject(hdc, hBr1);
        hPen1 = CreatePen(PS_SOLID, 2, RGB(0, 150, 0));
        oldPen = (HPEN)SelectObject(hdc, hPen1);
        Pie(hdc, rcPie.left, rcPie.top, rcPie.right, rcPie.bottom,
            rcPie.left, (rcPie.top + rcPie.bottom) / 2,
            (rcPie.left + rcPie.right) / 2, rcPie.top);
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBr);
        DeleteObject(hPen1);
        DeleteObject(hBr1);
        DrawLabel(hdc, x0 + 170, y1 + 40, "#9 Pie(filled)");

        // 10. Chord (filled)
        RECT rcChord = { x0 + 240, y1, x0 + 400, y1 + 120 };
        hBr1 = CreateSolidBrush(RGB(180, 220, 255));
        oldBr = (HBRUSH)SelectObject(hdc, hBr1);
        hPen1 = CreatePen(PS_SOLID, 2, RGB(0, 80, 200));
        oldPen = (HPEN)SelectObject(hdc, hPen1);
        Chord(hdc, rcChord.left, rcChord.top, rcChord.right, rcChord.bottom,
            rcChord.left, rcChord.top, rcChord.right, rcChord.bottom);
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBr);
        DeleteObject(hPen1);
        DeleteObject(hBr1);
        DrawLabel(hdc, x0 + 410, y1 + 40, "#10 Chord(filled)");

        // 11. PolyBezier
        int y2 = y1 + 150;
        POINT bezierPts[] = {
            {x0, y2 + 50},
            {x0 + 40, y2},
            {x0 + 80, y2 + 100},
            {x0 + 120, y2 + 50},
            {x0 + 160, y2},
            {x0 + 200, y2 + 100},
            {x0 + 240, y2 + 50}
        };
        hPen1 = CreatePen(PS_SOLID, 3, RGB(200, 0, 200));
        oldPen = (HPEN)SelectObject(hdc, hPen1);
        PolyBezier(hdc, bezierPts, 7);
        SelectObject(hdc, oldPen);
        DeleteObject(hPen1);
        DrawLabel(hdc, x0 + 260, y2 + 20, "#11 PolyBezier");

        // 12. PolyBezierTo
        int y3 = y2 + 120;
        MoveToEx(hdc, x0, y3 + 40, NULL);
        POINT bezierToPts[] = {
            {x0 + 40, y3},
            {x0 + 80, y3 + 80},
            {x0 + 120, y3 + 40},
            {x0 + 160, y3},
            {x0 + 200, y3 + 80},
            {x0 + 240, y3 + 40}
        };
        hPen1 = CreatePen(PS_SOLID, 3, RGB(255, 128, 0));
        oldPen = (HPEN)SelectObject(hdc, hPen1);
        PolyBezierTo(hdc, bezierToPts, 6);
        SelectObject(hdc, oldPen);
        DeleteObject(hPen1);
        DrawLabel(hdc, x0 + 260, y3 + 20, "#12 PolyBezierTo");
    }

    // ========== Group 3: Pen/Brush Styles ==========
    void DrawGroup3(HDC hdc, const RECT& rc) {
        int y0 = 40;
        int x0 = 30;

        // 13. Pen styles: PS_SOLID, PS_DASH, PS_DOT, PS_DASHDOT, PS_DASHDOTDOT
        int penStyles[] = { PS_SOLID, PS_DASH, PS_DOT, PS_DASHDOT, PS_DASHDOTDOT };
        const char* styleNames[] = { "SOLID", "DASH", "DOT", "DASHDOT", "DASHDOTDOT" };
        COLORREF penColors[] = { RGB(200,0,0), RGB(0,150,0), RGB(0,0,200), RGB(180,0,180), RGB(200,120,0) };
        for (int i = 0; i < 5; i++) {
            HPEN hPen = CreatePen(penStyles[i], 2, penColors[i]);
            HPEN oldPen = (HPEN)SelectObject(hdc, hPen);
            MoveToEx(hdc, x0, y0 + i * 25, NULL);
            LineTo(hdc, x0 + 200, y0 + i * 25);
            SelectObject(hdc, oldPen);
            DeleteObject(hPen);
            SStringA label;
            label.Format("#13-%d PS_%s (width=2)", i + 1, styleNames[i]);
            DrawLabel(hdc, x0 + 220, y0 + i * 25 - 8, label);
        }

        // 14. Solid brushes with different colors
        int y1 = y0 + 150;
        COLORREF brushColors[] = { RGB(255,0,0), RGB(0,255,0), RGB(0,0,255), RGB(255,255,0), RGB(255,0,255) };
        for (int i = 0; i < 5; i++) {
            RECT rcBr = { x0 + i * 70, y1, x0 + i * 70 + 50, y1 + 50 };
            HBRUSH hBr = CreateSolidBrush(brushColors[i]);
            HBRUSH oldBr = (HBRUSH)SelectObject(hdc, hBr);
            FillRect(hdc, &rcBr, hBr);
            SelectObject(hdc, oldBr);
            DeleteObject(hBr);
            FrameRect(hdc, &rcBr, (HBRUSH)(HBRUSH)GetStockObject(BLACK_BRUSH));
        }
        DrawLabel(hdc, x0 + 360, y1 + 15, "#14 CreateSolidBrush x5");

        // 15. Pattern brush (if bitmap available)
        if (m_testBmp) {
            int y2 = y1 + 80;
            RECT rcPattern = { x0, y2, x0 + 250, y2 + 60 };
            HBRUSH hPatternBr = CreatePatternBrush(m_testBmp);
            if (hPatternBr) {
                FillRect(hdc, &rcPattern, hPatternBr);
                DeleteObject(hPatternBr);
            }
            DrawLabel(hdc, x0 + 260, y2 + 20, "#15 CreatePatternBrush (soui.png)");
        }

        // 15b. Bitmap pattern brush using yy.png (small tile) - FillRect / Rectangle / Ellipse / RoundRect
        if (m_yyBmp) {
            int y2b = (m_testBmp ? y1 + 150 : y1 + 80);
            BITMAP bm;
            GetObject(m_yyBmp,sizeof(bm),&bm);
            // FillRect
            x0=32+10;
            y2b=18+123;
            RECT rcFill = { x0, y2b, x0 + bm.bmWidth*3, (int)(y2b + bm.bmHeight*2.5)};
            HBRUSH hYyBr = CreatePatternBrush(m_yyBmp);
            if (hYyBr) {
                SetBrushOrgEx(hdc, rcFill.left, rcFill.top, NULL);
                FillRect(hdc, &rcFill, hYyBr);
                SetBrushOrgEx(hdc, 0, 0, NULL);
            }
            FrameRect(hdc, &rcFill, (HBRUSH)GetStockObject(BLACK_BRUSH));
            DrawLabel(hdc, x0 + 160, y2b + 20, "#15b.1 yy.png PatternBrush + FillRect");

            // Rectangle (fill + stroke)
            RECT rcRect = { x0 + 380, y2b, x0 + 380 + 120, y2b + 60 };
            if (hYyBr) {
                HBRUSH oldBr = (HBRUSH)SelectObject(hdc, hYyBr);
                HPEN hPen = CreatePen(PS_SOLID, 2, RGB(200, 0, 0));
                HPEN oldPen = (HPEN)SelectObject(hdc, hPen);
                Rectangle(hdc, rcRect.left, rcRect.top, rcRect.right, rcRect.bottom);
                SelectObject(hdc, oldPen);
                SelectObject(hdc, oldBr);
                DeleteObject(hPen);
            }
            DrawLabel(hdc, x0 + 510, y2b + 20, "#15b.2 Rectangle fill");

            // Ellipse (fill + stroke)
            int y2c = y2b + 80;
            RECT rcEll = { x0, y2c, x0 + 150, y2c + 90 };
            if (hYyBr) {
                HBRUSH oldBr = (HBRUSH)SelectObject(hdc, hYyBr);
                HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 100, 200));
                HPEN oldPen = (HPEN)SelectObject(hdc, hPen);
                Ellipse(hdc, rcEll.left, rcEll.top, rcEll.right, rcEll.bottom);
                SelectObject(hdc, oldPen);
                SelectObject(hdc, oldBr);
                DeleteObject(hPen);
            }
            DrawLabel(hdc, x0 + 160, y2c + 30, "#15b.3 Ellipse fill");

            // RoundRect (fill + stroke)
            RECT rcRR = { x0 + 380, y2c, x0 + 380 + 150, y2c + 90 };
            if (hYyBr) {
                HBRUSH oldBr = (HBRUSH)SelectObject(hdc, hYyBr);
                HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 160, 0));
                HPEN oldPen = (HPEN)SelectObject(hdc, hPen);
                RoundRect(hdc, rcRR.left, rcRR.top, rcRR.right, rcRR.bottom, 25, 25);
                SelectObject(hdc, oldPen);
                SelectObject(hdc, oldBr);
                DeleteObject(hPen);
            }
            DrawLabel(hdc, x0 + 540, y2c + 30, "#15b.4 RoundRect fill");

            if (hYyBr) DeleteObject(hYyBr);
        }

        // 16. Gradient brush (swinx-only: CreateGradientBrush; use GradientFill as fallback on Windows)
        {
            int y3 = (m_yyBmp ? 440 : (m_testBmp ? 290 : 230));
            RECT rcGrad = { x0, y3, x0 + 300, y3 + 60 };
#ifdef _WIN32
            TRIVERTEX vert[] = {
                { rcGrad.left,  rcGrad.top,    0xFF00, 0x0000, 0x0000, 0xFFFF },
                { rcGrad.right, rcGrad.bottom, 0x0000, 0x0000, 0xFF00, 0xFFFF }
            };
            GRADIENT_RECT gRect = { 0, 1 };
            GradientFill(hdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
            DrawLabel(hdc, x0 + 310, y3 + 20, "#16 GradientFill(R->B, Windows fallback)");
#else
            if (m_testBmp) {
                GRADIENTITEM items[] = {
                    { RGB(255, 0, 0) , 0.0f},
                    { RGB(0, 255, 0),0.5f },
                    { RGB(0, 0, 255) ,1.0f}
                };

                GRADIENTINFO info;
                info.type = grad_linear;
                info.angle=0.0f;
                HBRUSH hGradBr = CreateGradientBrush(items, 3, &info, 255, kTileMode_Clamp);
                if (hGradBr) {
                    FillRect(hdc, &rcGrad, hGradBr);
                    DeleteObject(hGradBr);
                }
                DrawLabel(hdc, x0 + 310, y3 + 20, "#16 CreateGradientBrush");
            }
#endif
        }

        // 17. ROP2 modes
        int y4 = (m_yyBmp ? 520 : 370);
        RECT rcRop = { x0, y4, x0 + 120, y4 + 40 };
        HBRUSH hBgBr = CreateSolidBrush(RGB(200, 200, 200));
        FillRect(hdc, &rcRop, hBgBr);
        DeleteObject(hBgBr);
        HBRUSH hBlueBr = CreateSolidBrush(RGB(0, 0, 255));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, hBlueBr);
        int oldRop = SetROP2(hdc, R2_NOT);
        Rectangle(hdc, x0 + 10, y4 + 5, x0 + 60, y4 + 35);
        SetROP2(hdc, R2_XORPEN);
        Rectangle(hdc, x0 + 70, y4 + 5, x0 + 110, y4 + 35);
        SetROP2(hdc, oldRop);
        SelectObject(hdc, oldBr);
        DeleteObject(hBlueBr);
        DrawLabel(hdc, x0 + 130, y4 + 10, "#17 SetROP2 (R2_NOT, R2_XORPEN)");
    }

    // ========== Group 4: Bitmap Operations ==========
    void DrawGroup4(HDC hdc, const RECT& rc) {
        if (!m_testBmp) {
            DrawLabel(hdc, 30, 50, "Test bitmap not loaded!");
            return;
        }

        int y0 = 40;
        int x0 = 30;

        HDC memdc = CreateCompatibleDC(hdc);
        HBITMAP oldBmp = (HBITMAP)SelectObject(memdc, m_testBmp);

        // Get bitmap size
        BITMAP bm;
        GetObjectA(m_testBmp, sizeof(bm), &bm);

        // 18. BitBlt
        RECT rcDst = { x0, y0, x0 + bm.bmWidth, y0 + bm.bmHeight };
        BitBlt(hdc, rcDst.left, rcDst.top, rcDst.right - rcDst.left, rcDst.bottom - rcDst.top,
            memdc, 0, 0, SRCCOPY);
        DrawLabel(hdc, x0 + bm.bmWidth + 10, y0 + bm.bmHeight / 2 - 10, "#18 BitBlt (SRCCOPY)");

        // 19. StretchBlt (scale up)
        int x1 = x0 + bm.bmWidth + 180;
        RECT rcStretch = { x1, y0, x1 + bm.bmWidth * 2, y0 + bm.bmHeight * 2 };
        StretchBlt(hdc, rcStretch.left, rcStretch.top, rcStretch.right - rcStretch.left, rcStretch.bottom - rcStretch.top,
            memdc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
        DrawLabel(hdc, x1, y0 - 20, "#19 StretchBlt (2x)");

        // 20. StretchBlt (scale down + mirror)
        int y1 = y0 + bm.bmHeight * 2 + 30;
        RECT rcStretch2 = { x0, y1, x0 + bm.bmWidth / 2, y1 + bm.bmHeight / 2 };
        StretchBlt(hdc, rcStretch2.left, rcStretch2.top, rcStretch2.right - rcStretch2.left, rcStretch2.bottom - rcStretch2.top,
            memdc, bm.bmWidth, bm.bmHeight, -bm.bmWidth, -bm.bmHeight, SRCCOPY);
        DrawLabel(hdc, x0 + bm.bmWidth / 2 + 10, y1 + 10, "#20 StretchBlt (mirror 0.5x)");

        // 21. AlphaBlend (50% transparent)
        int x2 = x0 + 200;
        BLENDFUNCTION bf = { 0 };
        bf.BlendOp = AC_SRC_OVER;
        bf.SourceConstantAlpha = 128;
        RECT rcAlpha = { x2, y1, x2 + bm.bmWidth, y1 + bm.bmHeight };
        AlphaBlend(hdc, rcAlpha.left, rcAlpha.top, rcAlpha.right - rcAlpha.left, rcAlpha.bottom - rcAlpha.top,
            memdc, 0, 0, bm.bmWidth, bm.bmHeight, bf);
        DrawLabel(hdc, x2, y1 - 20, "#21 AlphaBlend (alpha=128)");

        // 22. TransparentBlt (simple test)
        int x3 = x2 + bm.bmWidth + 30;
        RECT rcTrans = { x3, y1, x3 + bm.bmWidth, y1 + bm.bmHeight };
        // First draw a color background
        HBRUSH hBg = CreateSolidBrush(RGB(255, 200, 100));
        RECT rcBg = rcTrans;
        FillRect(hdc, &rcBg, hBg);
        DeleteObject(hBg);
        TransparentBlt(hdc, rcTrans.left, rcTrans.top, rcTrans.right - rcTrans.left, rcTrans.bottom - rcTrans.top,
            memdc, 0, 0, bm.bmWidth, bm.bmHeight, RGB(255, 255, 255));
        DrawLabel(hdc, x3, y1 - 20, "#22 TransparentBlt");

        // 23. PatBlt
        int y2 = y1 + bm.bmHeight + 30;
        RECT rcPat = { x0, y2, x0 + 200, y2 + 50 };
        HBRUSH hPatBr = CreateSolidBrush(RGB(0, 128, 255));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, hPatBr);
        PatBlt(hdc, rcPat.left, rcPat.top, rcPat.right - rcPat.left, rcPat.bottom - rcPat.top, PATCOPY);
        SelectObject(hdc, oldBr);
        DeleteObject(hPatBr);
        DrawLabel(hdc, x0 + 210, y2 + 15, "#23 PatBlt (PATCOPY)");

        // 24. GradientFill (horizontal)
        int y3 = y2 + 80;
        TRIVERTEX vert[] = {
            { x0, y3, 0xFF00, 0x0000, 0x0000, 0xFFFF },
            { x0 + 250, y3 + 50, 0x0000, 0xFF00, 0x0000, 0xFFFF }
        };
        GRADIENT_RECT gRect = { 0, 1 };
        GradientFill(hdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
        DrawLabel(hdc, x0 + 260, y3 + 15, "#24 GradientFill (H, R->G)");

        SelectObject(memdc, oldBmp);
        DeleteDC(memdc);
    }

    // ========== Group 5: Text Output APIs ==========
    void DrawGroup5(HDC hdc, const RECT& rc) {
        int y0 = 40;
        int x0 = 30;

        COLORREF oldTxt = SetTextColor(hdc, RGB(0, 0, 0));
        COLORREF oldBk = SetBkColor(hdc, RGB(255, 255, 255));
        int oldBkMode = SetBkMode(hdc, TRANSPARENT);

        // Default font
        LOGFONTA lfNormal = { 0 };
        lfNormal.lfHeight = 18;
        strcpy_s(lfNormal.lfFaceName, ARRAYSIZE(lfNormal.lfFaceName), "Arial");
        HFONT hNormal = CreateFontIndirectA(&lfNormal);
        HFONT oldFont = (HFONT)SelectObject(hdc, hNormal);

        // 25. TextOutA
        SetBkMode(hdc, OPAQUE);
        SetBkColor(hdc, RGB(200, 255, 200));
		const char* str = "#25 TextOutA: Hello SOUI";
        TextOutA(hdc, x0, y0, str, strlen(str));
        SetBkMode(hdc, TRANSPARENT);
        DrawLabel(hdc, x0 + 280, y0, "(OPAQUE bg)");

        // 26. TextOutW
        SetBkMode(hdc, OPAQUE);
        SetBkColor(hdc, RGB(255, 200, 200));
        const wchar_t* wstr = L"#26 TextOutW: Unicode \u4E2D\u6587\u6D4B\u8BD5";
        TextOutW(hdc, x0, y0 + 30, wstr, (int)wcslen(wstr));
        SetBkMode(hdc, TRANSPARENT);
        DrawLabel(hdc, x0 + 350, y0 + 30, "(OPAQUE bg)");

        // 27. DrawTextA (center + single line)
        RECT rcDT = { x0, y0 + 70, x0 + 300, y0 + 120 };
        HBRUSH hFrameBr = CreateSolidBrush(RGB(240, 240, 240));
        FillRect(hdc, &rcDT, hFrameBr);
        DeleteObject(hFrameBr);
        FrameRect(hdc, &rcDT, (HBRUSH)(HBRUSH)GetStockObject(BLACK_BRUSH));
        SetTextColor(hdc, RGB(0, 0, 200));
        DrawTextA(hdc, "#27 DrawTextA DT_CENTER+DT_VCENTER", -1, &rcDT, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SetTextColor(hdc, RGB(0, 0, 0));
        DrawLabel(hdc, x0 + 310, y0 + 90, "(CENTER/VCENTER)");

        // 28. DrawTextA (multiline + wordbreak)
        RECT rcDT2 = { x0, y0 + 140, x0 + 280, y0 + 220 };
        FillRect(hdc, &rcDT2, hFrameBr = CreateSolidBrush(RGB(250, 250, 220)));
        DeleteObject(hFrameBr);
        FrameRect(hdc, &rcDT2, (HBRUSH)GetStockObject(BLACK_BRUSH));
        SetTextColor(hdc, RGB(128, 0, 128));
        DrawTextA(hdc, "#28 DrawTextA multiline wordbreak test. Second line here.\nThird line manually.", -1, &rcDT2, DT_WORDBREAK | DT_LEFT);
        SetTextColor(hdc, RGB(0, 0, 0));
        DrawLabel(hdc, x0 + 290, y0 + 160, "(WORDBREAK)");

        // 29. ExtTextOutA with ETO_OPAQUE + ETO_CLIPPED
        RECT rcETO = { x0 + 320, y0 + 70, x0 + 550, y0 + 110 };
        SetBkColor(hdc, RGB(180, 220, 255));
        SetTextColor(hdc, RGB(0, 100, 0));
		const char* etoStr = "#29 ExtTextOutA ETO_OPAQUE+CLIPPED";
        ExtTextOutA(hdc, x0 + 330, y0 + 85, ETO_OPAQUE | ETO_CLIPPED, &rcETO,
            etoStr, (int)strlen(etoStr), NULL);
        SetTextColor(hdc, RGB(0, 0, 0));
        FrameRect(hdc, &rcETO, (HBRUSH)GetStockObject(BLACK_BRUSH));

        // 30. TabbedTextOutA
        const char* tabbedStr = "#30 TabbedTextOut:\tcol1\tcol2\tcol3";
        int tabPositions[] = { 150, 250, 350 };
        SetTextColor(hdc, RGB(200, 80, 0));
        TabbedTextOutA(hdc, x0, y0 + 240, tabbedStr, (int)strlen(tabbedStr),
            3, tabPositions, 0);
        SetTextColor(hdc, RGB(0, 0, 0));

        // 31. Font styles: Bold, Italic, Underline, StrikeOut
        int y1 = y0 + 280;
        const char* fontLabels[] = { "Bold", "Italic", "Underline", "StrikeOut" };
        for (int i = 0; i < 4; i++) {
            LOGFONTA lf = lfNormal;
            switch (i) {
            case 0: lf.lfWeight = FW_BOLD; break;
            case 1: lf.lfItalic = TRUE; break;
            case 2: lf.lfUnderline = TRUE; break;
            case 3: lf.lfStrikeOut = TRUE; break;
            }
            HFONT hf = CreateFontIndirectA(&lf);
            HFONT oldf = (HFONT)SelectObject(hdc, hf);
            SStringA txt;
            txt.Format("#31-%d CreateFont %s: Sample", i + 1, fontLabels[i]);
            TextOutA(hdc, x0, y1 + i * 28, txt.c_str(), txt.GetLength());
            SelectObject(hdc, oldf);
            DeleteObject(hf);
        }

        // 32. SetTextAlign + GetTextExtentPoint32
        int y2 = y1 + 120;
        RECT rcBase = { x0, y2, x0 + 400, y2 + 60 };
        FillRect(hdc, &rcBase, hFrameBr = CreateSolidBrush(RGB(240, 240, 240)));
        DeleteObject(hFrameBr);
        // baseline
        HPEN hPen = CreatePen(PS_DASH, 1, RGB(255, 0, 0));
        HPEN oldPen = (HPEN)SelectObject(hdc, hPen);
        MoveToEx(hdc, x0, y2 + 30, NULL);
        LineTo(hdc, x0 + 400, y2 + 30);
        SelectObject(hdc, oldPen);
        DeleteObject(hPen);

        UINT oldAlign = SetTextAlign(hdc, TA_LEFT | TA_TOP);
        TextOutA(hdc, x0 + 50, y2 + 30, "TA_LEFT|TOP", (int)strlen("TA_LEFT|TOP"));

        SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
        TextOutA(hdc, x0 + 200, y2 + 30, "TA_CENTER|BASELINE", (int)strlen("TA_CENTER|BASELINE"));

        SetTextAlign(hdc, TA_RIGHT | TA_BOTTOM);
        TextOutA(hdc, x0 + 350, y2 + 30, "TA_RIGHT|BOTTOM", (int)strlen("TA_RIGHT|BOTTOM"));
        SetTextAlign(hdc, oldAlign);
        DrawLabel(hdc, x0 + 410, y2 + 20, "#32 SetTextAlign");

        // 33. GetTextExtentPoint32 + GetTextMetrics
        SIZE sz;
        const char* extentStr = "TextExtent Test";
        GetTextExtentPoint32A(hdc, extentStr, (int)strlen(extentStr), &sz);
        SStringA extentLbl;
        extentLbl.Format("#33 GetTextExtentPoint32: sz=(%d,%d)", sz.cx, sz.cy);
        TextOutA(hdc, x0, y2 + 70, extentLbl.c_str(), -1);

        TEXTMETRICA tm;
        GetTextMetricsA(hdc, &tm);
        SStringA tmLbl;
        tmLbl.Format("#34 GetTextMetrics: h=%d asc=%d desc=%d avgW=%d",
            tm.tmHeight, tm.tmAscent, tm.tmDescent, tm.tmAveCharWidth);
        TextOutA(hdc, x0, y2 + 95, tmLbl.c_str(), (int)tmLbl.GetLength());

        // Restore
        SelectObject(hdc, oldFont);
        DeleteObject(hNormal);
        SetBkMode(hdc, oldBkMode);
        SetBkColor(hdc, oldBk);
        SetTextColor(hdc, oldTxt);
    }

    // ========== Group 6: Path APIs ==========
    void DrawGroup6(HDC hdc, const RECT& rc) {
        int y0 = 40;
        int x0 = 30;

        // 35. BeginPath / EndPath / StrokePath
        BeginPath(hdc);
        MoveToEx(hdc, x0, y0 + 50, NULL);
        LineTo(hdc, x0 + 50, y0);
        LineTo(hdc, x0 + 100, y0 + 50);
        LineTo(hdc, x0 + 150, y0);
        LineTo(hdc, x0 + 200, y0 + 50);
        EndPath(hdc);
        HPEN hPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
        HPEN oldPen = (HPEN)SelectObject(hdc, hPen);
        StrokePath(hdc);
        SelectObject(hdc, oldPen);
        DeleteObject(hPen);
        DrawLabel(hdc, x0 + 210, y0 + 20, "#35 BeginPath+EndPath+StrokePath");

        // 36. FillPath
        BeginPath(hdc);
        MoveToEx(hdc, x0, y0 + 100, NULL);
        LineTo(hdc, x0 + 60, y0 + 70);
        LineTo(hdc, x0 + 120, y0 + 100);
        LineTo(hdc, x0 + 90, y0 + 160);
        LineTo(hdc, x0 + 30, y0 + 160);
        CloseFigure(hdc);
        EndPath(hdc);
        HBRUSH hBr = CreateSolidBrush(RGB(100, 255, 100));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, hBr);
        FillPath(hdc);
        SelectObject(hdc, oldBr);
        DeleteObject(hBr);
        DrawLabel(hdc, x0 + 130, y0 + 115, "#36 FillPath");

        // 37. StrokeAndFillPath
        BeginPath(hdc);
        Ellipse(hdc, x0 + 280, y0 + 70, x0 + 440, y0 + 170);
        EndPath(hdc);
        hBr = CreateSolidBrush(RGB(180, 180, 255));
        oldBr = (HBRUSH)SelectObject(hdc, hBr);
        hPen = CreatePen(PS_SOLID, 3, RGB(0, 0, 200));
        oldPen = (HPEN)SelectObject(hdc, hPen);
        StrokeAndFillPath(hdc);
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBr);
        DeleteObject(hPen);
        DeleteObject(hBr);
        DrawLabel(hdc, x0 + 450, y0 + 115, "#37 StrokeAndFillPath");

        // 38. PathToRegion + FillRgn
        int y1 = y0 + 200;
        BeginPath(hdc);
        Rectangle(hdc, x0, y1, x0 + 160, y1 + 70);
        Ellipse(hdc, x0 + 80, y1 - 20, x0 + 160, y1 + 50);
        EndPath(hdc);
        HRGN hRgn = PathToRegion(hdc);
        if (hRgn) {
            hBr = CreateSolidBrush(RGB(255, 200, 100));
            oldBr = (HBRUSH)SelectObject(hdc, hBr);
            FillRgn(hdc, hRgn, hBr);
            hPen = CreatePen(PS_DASH, 1, RGB(128, 64, 0));
            oldPen = (HPEN)SelectObject(hdc, hPen);
            FrameRgn(hdc, hRgn, (HBRUSH)GetStockObject(BLACK_BRUSH), 1, 1);
            SelectObject(hdc, oldPen);
            SelectObject(hdc, oldBr);
            DeleteObject(hPen);
            DeleteObject(hBr);
            DeleteObject(hRgn);
        }
        DrawLabel(hdc, x0 + 170, y1 + 20, "#38 PathToRegion+FillRgn");

        // 39. SelectClipPath (RGN_COPY) + fill clipped
        int x1 = x0 + 280;
        RECT rcClipArea = { x1, y1, x1 + 180, y1 + 100 };
        HBRUSH hBgBr = CreateSolidBrush(RGB(240, 240, 240));
        FillRect(hdc, &rcClipArea, hBgBr);
        DeleteObject(hBgBr);
        FrameRect(hdc, &rcClipArea, (HBRUSH)GetStockObject(BLACK_BRUSH));

        BeginPath(hdc);
        Ellipse(hdc, x1 + 20, y1 + 10, x1 + 160, y1 + 90);
        EndPath(hdc);
        int saveDc = SaveDC(hdc);
        SelectClipPath(hdc, RGN_COPY);
        hBr = CreateSolidBrush(RGB(0, 200, 200));
        FillRect(hdc, &rcClipArea, hBr);
        DeleteObject(hBr);
        RestoreDC(hdc, saveDc);
        DrawLabel(hdc, x1 + 190, y1 + 30, "#39 SelectClipPath(RGN_COPY)");

        // 40. GetPath (point count query)
        int y2 = y1 + 130;
        BeginPath(hdc);
        MoveToEx(hdc, x0, y2 + 30, NULL);
        LineTo(hdc, x0 + 50, y2);
        LineTo(hdc, x0 + 100, y2 + 30);
        EndPath(hdc);
        int nPts = GetPath(hdc, NULL, NULL, 0);
        SStringA lbl;
        lbl.Format("#40 GetPath point count: %d (should be 3)", nPts);
        // just stroke for visual
        hPen = CreatePen(PS_SOLID, 2, RGB(128, 0, 128));
        oldPen = (HPEN)SelectObject(hdc, hPen);
        // re-create since GetPath consumed path above
        BeginPath(hdc);
        MoveToEx(hdc, x0, y2 + 30, NULL);
        LineTo(hdc, x0 + 50, y2);
        LineTo(hdc, x0 + 100, y2 + 30);
        EndPath(hdc);
        StrokePath(hdc);
        SelectObject(hdc, oldPen);
        DeleteObject(hPen);
        DrawLabel(hdc, x0 + 120, y2 + 10, lbl);

        // 41. SetMiterLimit / GetMiterLimit
        float oldMiter, newMiter;
        GetMiterLimit(hdc, &oldMiter);
        SetMiterLimit(hdc, 5.0f, NULL);
        GetMiterLimit(hdc, &newMiter);
        SStringA mlbl;
        mlbl.Format("#41 Set/GetMiterLimit: old=%.1f new=%.1f", oldMiter, newMiter);
        DrawLabel(hdc, x0 + 280, y2 + 20, mlbl);

        // restore to default
        SetMiterLimit(hdc, oldMiter, NULL);
    }

    // ========== Group 7: Region & Clip APIs ==========
    void DrawGroup7(HDC hdc, const RECT& rc) {
        int y0 = 40;
        int x0 = 30;

        // 42. CreateRectRgn + FillRgn
        HRGN hRgn1 = CreateRectRgn(x0, y0, x0 + 150, y0 + 80);
        HBRUSH hBr = CreateSolidBrush(RGB(255, 150, 150));
        FillRgn(hdc, hRgn1, hBr);
        DeleteObject(hBr);
        DeleteObject(hRgn1);
        DrawLabel(hdc, x0 + 160, y0 + 25, "#42 CreateRectRgn+FillRgn");

        // 43. CombineRgn RGN_OR
        int y1 = y0 + 100;
        HRGN hR2a = CreateRectRgn(x0, y1, x0 + 100, y1 + 80);
        HRGN hR2b = CreateRectRgn(x0 + 60, y1 + 30, x0 + 160, y1 + 100);
        HRGN hR2 = CreateRectRgn(0, 0, 0, 0);
        CombineRgn(hR2, hR2a, hR2b, RGN_OR);
        hBr = CreateSolidBrush(RGB(150, 255, 150));
        FillRgn(hdc, hR2, hBr);
        HPEN hPen = CreatePen(PS_DASH, 1, RGB(0, 128, 0));
        HPEN oldPen = (HPEN)SelectObject(hdc, hPen);
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, x0, y1, x0 + 100, y1 + 80);
        Rectangle(hdc, x0 + 60, y1 + 30, x0 + 160, y1 + 100);
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBr);
        DeleteObject(hPen);
        DeleteObject(hBr);
        DeleteObject(hR2a); DeleteObject(hR2b); DeleteObject(hR2);
        DrawLabel(hdc, x0 + 170, y1 + 40, "#43 CombineRgn RGN_OR");

        // 44. CombineRgn RGN_DIFF
        int y2 = y1 + 120;
        HRGN hR3a = CreateRectRgn(x0, y2, x0 + 140, y2 + 80);
        HRGN hR3b = CreateRectRgn(x0 + 50, y2 + 20, x0 + 120, y2 + 60);
        HRGN hR3 = CreateRectRgn(0, 0, 0, 0);
        CombineRgn(hR3, hR3a, hR3b, RGN_DIFF);
        hBr = CreateSolidBrush(RGB(150, 180, 255));
        FillRgn(hdc, hR3, hBr);
        DeleteObject(hBr);
        DeleteObject(hR3a); DeleteObject(hR3b); DeleteObject(hR3);
        DrawLabel(hdc, x0 + 150, y2 + 30, "#44 CombineRgn RGN_DIFF");

        // 45. FrameRgn
        int y3 = y0 + 100;
        int x1 = x0 + 300;
        HRGN hR4 = CreateRectRgn(x1, y3, x1 + 150, y3 + 80);
        FrameRgn(hdc, hR4, (HBRUSH)GetStockObject(BLACK_BRUSH), 5, 5);
        DeleteObject(hR4);
        DrawLabel(hdc, x1 + 160, y3 + 30, "#45 FrameRgn(thick=5)");

        // 46. SelectClipRgn (intersect clip) + draw outside
        int y4 = y2 + 110;
        RECT rcBg = { x0, y4, x0 + 260, y4 + 90 };
        HBRUSH hBgBr = CreateSolidBrush(RGB(230, 230, 230));
        FillRect(hdc, &rcBg, hBgBr);
        DeleteObject(hBgBr);
        FrameRect(hdc, &rcBg, (HBRUSH)GetStockObject(BLACK_BRUSH));

        HRGN hClipRgn = CreateRectRgn(x0 + 30, y4 + 15, x0 + 150, y4 + 75);
        int saveDc = SaveDC(hdc);
        SelectClipRgn(hdc, hClipRgn);
        hBr = CreateSolidBrush(RGB(255, 100, 0));
        FillRect(hdc, &rcBg, hBr);
        DeleteObject(hBr);
        RestoreDC(hdc, saveDc);
        DeleteObject(hClipRgn);
        DrawLabel(hdc, x0 + 270, y4 + 35, "#46 SelectClipRgn (clipped fill)");

        // 47. ExcludeClipRect
        int y5 = y4;
        int x2 = x0 + 350;
        RECT rcBg2 = { x2, y5, x2 + 200, y5 + 90 };
        hBgBr = CreateSolidBrush(RGB(230, 230, 230));
        FillRect(hdc, &rcBg2, hBgBr);
        DeleteObject(hBgBr);
        FrameRect(hdc, &rcBg2, (HBRUSH)GetStockObject(BLACK_BRUSH));

        saveDc = SaveDC(hdc);
        ExcludeClipRect(hdc, x2 + 40, y5 + 20, x2 + 120, y5 + 70);
        hBr = CreateSolidBrush(RGB(0, 160, 160));
        FillRect(hdc, &rcBg2, hBr);
        DeleteObject(hBr);
        RestoreDC(hdc, saveDc);
        DrawLabel(hdc, x2 - 5, y5 - 20, "#47 ExcludeClipRect");

        // 48. IntersectClipRect
        int y6 = y5 + 110;
        RECT rcBg3 = { x0, y6, x0 + 200, y6 + 70 };
        hBgBr = CreateSolidBrush(RGB(230, 230, 230));
        FillRect(hdc, &rcBg3, hBgBr);
        DeleteObject(hBgBr);
        FrameRect(hdc, &rcBg3, (HBRUSH)GetStockObject(BLACK_BRUSH));

        saveDc = SaveDC(hdc);
        IntersectClipRect(hdc, x0 + 40, y6 - 50, x0 + 150, y6 + 30);
        hBr = CreateSolidBrush(RGB(180, 0, 200));
        FillRect(hdc, &rcBg3, hBr);
        DeleteObject(hBr);
        RestoreDC(hdc, saveDc);
        DrawLabel(hdc, x0 + 210, y6 + 20, "#48 IntersectClipRect");

        // 49. GetClipBox
        RECT rcClipBox;
        int clipType = GetClipBox(hdc, &rcClipBox);
        SStringA cblbl;
        cblbl.Format("#49 GetClipBox type=%d rc=(%d,%d,%d,%d)", clipType,
            rcClipBox.left, rcClipBox.top, rcClipBox.right, rcClipBox.bottom);
        DrawLabel(hdc, x0, y6 + 90, cblbl);
    }

    // ========== Group 8: Rect Operations & DC State ==========
    void DrawGroup8(HDC hdc, const RECT& rc) {
        int y0 = 40;
        int x0 = 30;

        // 50. FillRect
        RECT rcTest = { x0, y0, x0 + 180, y0 + 50 };
        HBRUSH hBr = CreateSolidBrush(RGB(255, 220, 100));
        FillRect(hdc, &rcTest, hBr);
        DeleteObject(hBr);
        DrawLabel(hdc, x0 + 190, y0 + 15, "#50 FillRect");

        // 51. FrameRect
        RECT rcTest2 = { x0 + 300, y0, x0 + 480, y0 + 50 };
        FrameRect(hdc, &rcTest2, (HBRUSH)GetStockObject(GRAY_BRUSH));
        DrawLabel(hdc, x0 + 490, y0 + 15, "#51 FrameRect");

        // 52. InvertRect (twice to show effect)
        int y1 = y0 + 70;
        HBRUSH hBgBr = CreateSolidBrush(RGB(100, 200, 255));
        RECT rcInv = { x0, y1, x0 + 180, y1 + 50 };
        FillRect(hdc, &rcInv, hBgBr);
        DeleteObject(hBgBr);
        InvertRect(hdc, &rcInv);
        // Draw side by side for comparison
        RECT rcInv2 = { x0, y1 + 60, x0 + 180, y1 + 110 };
        hBgBr = CreateSolidBrush(RGB(100, 200, 255));
        FillRect(hdc, &rcInv2, hBgBr);
        DeleteObject(hBgBr);
        DrawLabel(hdc, x0 + 190, y1 + 15, "#52 InvertRect");
        DrawLabel(hdc, x0 + 190, y1 + 75, "(original for compare)");

        // 53. DrawFocusRect
        int y2 = y1 + 130;
        RECT rcFocus = { x0, y2, x0 + 150, y2 + 40 };
        hBgBr = CreateSolidBrush(RGB(240, 240, 240));
        FillRect(hdc, &rcFocus, hBgBr);
        DeleteObject(hBgBr);
        DrawFocusRect(hdc, &rcFocus);
        DrawLabel(hdc, x0 + 160, y2 + 10, "#53 DrawFocusRect");

        // 54. ClearRect (use FillRect for Windows compat; swinx has native ClearRect)
        RECT rcClear = { x0 + 300, y2, x0 + 480, y2 + 50 };
        hBgBr = CreateSolidBrush(RGB(180, 100, 200));
        FillRect(hdc, &rcClear, hBgBr);
        DeleteObject(hBgBr);
        COLORREF clearColor = RGB(255, 255, 0);
#ifdef _WIN32
        HBRUSH hClearBr = CreateSolidBrush(clearColor);
        FillRect(hdc, &rcClear, hClearBr);
        DeleteObject(hClearBr);
#else
        ClearRect(hdc, &rcClear, clearColor);
#endif
        FrameRect(hdc, &rcClear, (HBRUSH)GetStockObject(BLACK_BRUSH));
        DrawLabel(hdc, x0 + 490, y2 + 10, "#54 ClearRect(yellow)");

        // 55. SaveDC / RestoreDC state restoration
        int y3 = y2 + 80;
        COLORREF origTxt = GetTextColor(hdc);
        COLORREF origBk = GetBkColor(hdc);
        int origBkMode = GetBkMode(hdc);
        HPEN origPen = (HPEN)GetCurrentObject(hdc, OBJ_PEN);
        HBRUSH origBrush = (HBRUSH)GetCurrentObject(hdc, OBJ_BRUSH);

        int saveId = SaveDC(hdc);
        SetTextColor(hdc, RGB(255, 0, 0));
        SetBkMode(hdc, OPAQUE);
        SetBkColor(hdc, RGB(0, 255, 0));
        HPEN tempPen = CreatePen(PS_SOLID, 5, RGB(0, 0, 255));
        SelectObject(hdc, tempPen);
        HBRUSH tempBr = CreateSolidBrush(RGB(255, 0, 255));
        SelectObject(hdc, tempBr);
        Rectangle(hdc, x0, y3, x0 + 120, y3 + 50);
        TextOutA(hdc, x0 + 10, y3 + 15, "MODIFIED", -1);

        RestoreDC(hdc, saveId);
        // After restore, draw with original state
        Rectangle(hdc, x0 + 150, y3, x0 + 270, y3 + 50);
        TextOutA(hdc, x0 + 160, y3 + 15, "RESTORED", -1);

        // RestoreDC deselected the temp objects; free them
        DeleteObject(tempPen);
        DeleteObject(tempBr);

        SStringA lbl;
        lbl.Format("#55 SaveDC(id=%d)/RestoreDC - state restored correctly", saveId);
        DrawLabel(hdc, x0 + 290, y3 + 15, lbl);

        // 56. SetPixel / GetPixel
        int y4 = y3 + 80;
        SetPixel(hdc, x0 + 10, y4 + 10, RGB(255, 0, 0));
        SetPixel(hdc, x0 + 12, y4 + 10, RGB(0, 255, 0));
        SetPixel(hdc, x0 + 14, y4 + 10, RGB(0, 0, 255));
        COLORREF p1 = GetPixel(hdc, x0 + 10, y4 + 10);
        COLORREF p2 = GetPixel(hdc, x0 + 12, y4 + 10);
        COLORREF p3 = GetPixel(hdc, x0 + 14, y4 + 10);
        SStringA plbl;
        plbl.Format("#56 Set/GetPixel: R=0x%06X G=0x%06X B=0x%06X (zoomed 3px)", p1, p2, p3);
        // Enlarge pixels for visibility
        HBRUSH hR = CreateSolidBrush(RGB(255, 0, 0));
        RECT rcp =  CRect( x0 + 8, y4 + 30, x0 + 28, y4 + 50 ); FillRect(hdc, &rcp, hR); DeleteObject(hR);
        HBRUSH hG = CreateSolidBrush(RGB(0, 255, 0));
        rcp = CRect( x0 + 28, y4 + 30, x0 + 48, y4 + 50 ); FillRect(hdc, &rcp, hG); DeleteObject(hG);
        HBRUSH hB = CreateSolidBrush(RGB(0, 0, 255));
        rcp = CRect(x0 + 48, y4 + 30, x0 + 68, y4 + 50 ); FillRect(hdc, &rcp, hB); DeleteObject(hB);
        DrawLabel(hdc, x0 + 80, y4 + 25, plbl);

        // 57. GetDeviceCaps
        int caps[] = { HORZRES, VERTRES, BITSPIXEL, PLANES, LOGPIXELSX, LOGPIXELSY };
        const char* capNames[] = { "HORZRES", "VERTRES", "BITSPIXEL", "PLANES", "LOGPIXELSX", "LOGPIXELSY" };
        SStringA dcCaps = "#57 GetDeviceCaps: ";
        for (int i = 0; i < 6; i++) {
            SStringA one;
            one.Format("%s=%d ", capNames[i], GetDeviceCaps(hdc, caps[i]));
            dcCaps += one;
        }
        DrawLabel(hdc, x0, y4 + 70, dcCaps);
    }

    // ========== Group 9: Coordinate Transform & Misc ==========
    void DrawGroup9(HDC hdc, const RECT& rc) {
        int y0 = 40;
        int x0 = 30;

        // 58. SetViewportOrgEx / GetViewportOrgEx
        POINT oldVpOrg, newVpOrg;
        GetViewportOrgEx(hdc, &oldVpOrg);
        RECT rcVP = { x0, y0, x0 + 200, y0 + 80 };
        HBRUSH hBgBr = CreateSolidBrush(RGB(240, 240, 200));
        FillRect(hdc, &rcVP, hBgBr);
        DeleteObject(hBgBr);
        FrameRect(hdc, &rcVP, (HBRUSH)GetStockObject(BLACK_BRUSH));

        int saveDc = SaveDC(hdc);
        SetViewportOrgEx(hdc, x0 + 30, y0 + 20, NULL);
        HBRUSH hBr = CreateSolidBrush(RGB(255, 0, 0));
        Rectangle(hdc, 0, 0, 80, 40);  // drawn at (x0+30, y0+20) absolute
        DeleteObject(hBr);
        GetViewportOrgEx(hdc, &newVpOrg);
        RestoreDC(hdc, saveDc);
        SStringA vplbl;
        vplbl.Format("#58 SetViewportOrgEx: old=(%d,%d) new=(%d,%d) [red rect translated]",
            oldVpOrg.x, oldVpOrg.y, newVpOrg.x, newVpOrg.y);
        DrawLabel(hdc, x0 + 210, y0 + 30, vplbl);

        // 59. OffsetViewportOrgEx
        int y1 = y0 + 100;
        RECT rcOff = { x0, y1, x0 + 200, y1 + 80 };
        hBgBr = CreateSolidBrush(RGB(240, 240, 200));
        FillRect(hdc, &rcOff, hBgBr);
        DeleteObject(hBgBr);
        FrameRect(hdc, &rcOff, (HBRUSH)GetStockObject(BLACK_BRUSH));

        saveDc = SaveDC(hdc);
        SetViewportOrgEx(hdc, x0 + 10, y1 + 10, NULL);
        OffsetViewportOrgEx(hdc, 30, 20, NULL);  // offset by (30,20)
        HBRUSH hGreen = CreateSolidBrush(RGB(0, 180, 0));
        Rectangle(hdc, 0, 0, 80, 40);
        DeleteObject(hGreen);
        RestoreDC(hdc, saveDc);
        DrawLabel(hdc, x0 + 210, y1 + 30, "#59 OffsetViewportOrgEx (+30,+20 from VP org)");

        // 60. SetWorldTransform (scale + translate)
        int y2 = y1 + 100;
        RECT rcWT = { x0, y2, x0 + 260, y2 + 120 };
        hBgBr = CreateSolidBrush(RGB(240, 240, 200));
        FillRect(hdc, &rcWT, hBgBr);
        DeleteObject(hBgBr);
        FrameRect(hdc, &rcWT, (HBRUSH)GetStockObject(BLACK_BRUSH));

        // Draw reference outline
        saveDc = SaveDC(hdc);
        // reference (no transform)
        HPEN hDashPen = CreatePen(PS_DASH, 1, RGB(128, 128, 128));
        HPEN oldPen = (HPEN)SelectObject(hdc, hDashPen);
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, x0 + 10, y2 + 10, x0 + 50, y2 + 40);  // ref rect 40x30
        // Transformed
        XFORM xform;
        xform.eM11 = 2.0f;   // scale x 2x
        xform.eM12 = 0.0f;
        xform.eM21 = 0.0f;
        xform.eM22 = 1.5f;   // scale y 1.5x
        xform.eDx = (FLOAT)(x0 + 70);
        xform.eDy = (FLOAT)(y2 + 10);
        SetWorldTransform(hdc, &xform);
        SelectObject(hdc, GetStockObject(BLACK_PEN));
        HBRUSH hBlueBr = CreateSolidBrush(RGB(100, 100, 255));
        oldBr = (HBRUSH)SelectObject(hdc, hBlueBr);
        Rectangle(hdc, 0, 0, 40, 30);  // scaled to 80x45, translated
        DeleteObject(hBlueBr);
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBr);
        DeleteObject(hDashPen);
        RestoreDC(hdc, saveDc);
        DrawLabel(hdc, x0 + 270, y2 + 20, "#60 SetWorldTransform (scale 2x1.5 + translate)");
        DrawLabel(hdc, x0 + 270, y2 + 45, "[dashed = ref; blue = transformed]");

        // 61. SetROP2 with all visible effects
        int y3 = y2 + 140;
        const char* ropNames[] = { "R2_BLACK", "R2_WHITE", "R2_NOT", "R2_COPYPEN", "R2_XORPEN", "R2_MERGEPEN" };
        int ropModes[] = { R2_BLACK, R2_WHITE, R2_NOT, R2_COPYPEN, R2_XORPEN, R2_MERGEPEN };
        HPEN hBluePen = CreatePen(PS_SOLID, 5, RGB(0, 0, 255));
        oldPen = (HPEN)SelectObject(hdc, hBluePen);
        for (int i = 0; i < 6; i++) {
            RECT r = { x0 + i * 80, y3, x0 + i * 80 + 60, y3 + 50 };
            HBRUSH hBg = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(hdc, &r, hBg);
            DeleteObject(hBg);
            FrameRect(hdc, &r, (HBRUSH)GetStockObject(LTGRAY_BRUSH));

            // red background shape
            int oldRop = SetROP2(hdc, R2_COPYPEN);
            HBRUSH hRedBr = CreateSolidBrush(RGB(255, 0, 0));
            HBRUSH oldB = (HBRUSH)SelectObject(hdc, hRedBr);
            Ellipse(hdc, r.left + 5, r.top + 5, r.right - 5, r.bottom - 5);
            SelectObject(hdc, oldB);
            DeleteObject(hRedBr);
            SetROP2(hdc, ropModes[i]);
            MoveToEx(hdc, r.left, (r.top + r.bottom) / 2, NULL);
            LineTo(hdc, r.right, (r.top + r.bottom) / 2);
            SetROP2(hdc, oldRop);

            SStringA lbl2;
            lbl2.Format("#61-%d %s", i + 1, ropNames[i]);
            DrawLabel(hdc, r.left, r.bottom + 2, lbl2);
        }
        SelectObject(hdc, oldPen);
        DeleteObject(hBluePen);

        // 62. SetTextColor / SetBkColor / SetBkMode
        int y4 = y3 + 90;
        COLORREF oldTxt = SetTextColor(hdc, RGB(255, 255, 255));
        COLORREF oldBk = SetBkColor(hdc, RGB(0, 0, 128));
        int oldBkMode = SetBkMode(hdc, OPAQUE);
        TextOutA(hdc, x0, y4, "#62 SetTextColor(white) + SetBkColor(darkblue) + SetBkMode(OPAQUE)", -1);
        SetBkMode(hdc, TRANSPARENT);
        TextOutA(hdc, x0, y4 + 25, "#62 SetBkMode(TRANSPARENT) - no bg fill", -1);
        SetBkMode(hdc, oldBkMode);
        SetBkColor(hdc, oldBk);
        SetTextColor(hdc, oldTxt);
    }

    // ========== Group 10: Icon / Font Resource + Misc Edge Cases ==========
    void DrawGroup10(HDC hdc, const RECT& rc) {
        int y0 = 40;
        int x0 = 30;

        // 63. ExtCreatePen (PS_USERSTYLE custom dash)
        DWORD customDash[] = { 10, 4, 2, 4 };
        LOGBRUSH lb = { BS_SOLID, RGB(180, 0, 100), 0 };
        HPEN hCustomPen = ExtCreatePen(PS_USERSTYLE | PS_GEOMETRIC, 3, &lb, 4, customDash);
        HPEN oldPen = (HPEN)SelectObject(hdc, hCustomPen);
        MoveToEx(hdc, x0, y0 + 20, NULL);
        LineTo(hdc, x0 + 350, y0 + 20);
        SelectObject(hdc, oldPen);
        DeleteObject(hCustomPen);
        DrawLabel(hdc, x0 + 360, y0 + 10, "#63 ExtCreatePen PS_USERSTYLE custom dash 10,4,2,4");

        // 64. ExtCreatePen PS_GEOMETRIC with pattern brush (if bitmap)
        if (m_testBmp) {
            LOGBRUSH lb2;
            lb2.lbStyle = BS_PATTERN;
            lb2.lbColor = 0;
            lb2.lbHatch = (ULONG_PTR)m_testBmp;
            HPEN hPatPen = ExtCreatePen(PS_GEOMETRIC | PS_SOLID, 6, &lb2, 0, NULL);
            if (hPatPen) {
                oldPen = (HPEN)SelectObject(hdc, hPatPen);
                MoveToEx(hdc, x0, y0 + 50, NULL);
                LineTo(hdc, x0 + 350, y0 + 50);
                Ellipse(hdc, x0, y0 + 60, x0 + 120, y0 + 130);
                SelectObject(hdc, oldPen);
                DeleteObject(hPatPen);
                DrawLabel(hdc, x0 + 360, y0 + 80, "#64 ExtCreatePen PS_GEOMETRIC + BS_PATTERN brush");
            }
        }

        // 65. GetStockObject objects
        int stockObjs[] = { WHITE_BRUSH, LTGRAY_BRUSH, GRAY_BRUSH, DKGRAY_BRUSH, BLACK_BRUSH,
                           WHITE_PEN, BLACK_PEN, NULL_PEN, NULL_BRUSH, DEFAULT_GUI_FONT };
        const char* stockNames[] = { "WHITE_BRUSH", "LTGRAY_BRUSH", "GRAY_BRUSH", "DKGRAY_BRUSH", "BLACK_BRUSH",
                                     "WHITE_PEN", "BLACK_PEN", "NULL_PEN", "NULL_BRUSH", "DEFAULT_GUI_FONT" };
        int y1 = y0 + 150;
        for (int i = 0; i < 5; i++) {
            RECT r = { x0 + i * 70, y1, x0 + i * 70 + 50, y1 + 40 };
            HGDIOBJ obj = GetStockObject(stockObjs[i]);
            if (GetObjectType(obj) == OBJ_BRUSH) {
                FillRect(hdc, &r, (HBRUSH)obj);
            }
            FrameRect(hdc, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
            SStringA lbl;
            lbl.Format("#65-%d %s", i + 1, stockNames[i]);
            DrawLabel(hdc, r.left, r.bottom + 2, lbl);
        }
        // Draw pens as lines
        for (int i = 5; i < 8; i++) {
            HPEN hp = (HPEN)GetStockObject(stockObjs[i]);
            oldPen = (HPEN)SelectObject(hdc, hp);
            MoveToEx(hdc, x0 + (i - 5) * 100, y1 + 80, NULL);
            LineTo(hdc, x0 + (i - 5) * 100 + 80, y1 + 80);
            SStringA lbl;
            lbl.Format("#65-%d %s", i + 1, stockNames[i]);
            DrawLabel(hdc, x0 + (i - 5) * 100, y1 + 85, lbl);
        }
        SelectObject(hdc, GetStockObject(BLACK_PEN));

        // 66. GetObjectA on each stock object type
        SStringA goLbl = "#66 GetObjectA: ";
        {
            BITMAP bm = { 0 };
            if (m_testBmp) {
                GetObjectA(m_testBmp, sizeof(bm), &bm);
                SStringA tmp;
                tmp.Format("BMP(%dx%d, bpp=%d) ", bm.bmWidth, bm.bmHeight, bm.bmBitsPixel);
                goLbl += tmp;
            }
        }
        {
            LOGPEN lp = { 0 };
            HPEN hp = CreatePen(PS_SOLID, 3, RGB(12, 34, 56));
            GetObjectA(hp, sizeof(lp), &lp);
            SStringA tmp;
            tmp.Format("PEN(sty=%u,w=%d,cr=0x%08X) ", lp.lopnStyle, lp.lopnWidth.x, lp.lopnColor);
            goLbl += tmp;
            DeleteObject(hp);
        }
        {
            LOGBRUSH lb = { 0 };
            HBRUSH hb = CreateSolidBrush(RGB(0xAB, 0xCD, 0xEF));
            GetObjectA(hb, sizeof(lb), &lb);
            SStringA tmp;
            tmp.Format("BRUSH(sty=%d,cr=0x%08X)", lb.lbStyle, lb.lbColor);
            goLbl += tmp;
            DeleteObject(hb);
        }
        DrawLabel(hdc, x0, y1 + 120, goLbl);

        // 67. CreateFont with various heights + GetTextFace
        int y2 = y1 + 150;
        int heights[] = { 10, 14, 20, 28, 40 };
        for (int i = 0; i < 5; i++) {
            LOGFONTA lf = { 0 };
            lf.lfHeight = heights[i];
            strcpy_s(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), "Arial");
            HFONT hf = CreateFontIndirectA(&lf);
            HFONT oldf = (HFONT)SelectObject(hdc, hf);
            SStringA txt;
            txt.Format("#67-%d Height=%d", i + 1, heights[i]);
            TextOutA(hdc, x0 + i * 120, y2, txt.c_str(), -1);
            SelectObject(hdc, oldf);
            DeleteObject(hf);
        }
        // GetTextFaceA
        char faceName[LF_FACESIZE];
        HFONT hTestFont = (HFONT)GetCurrentObject(hdc, OBJ_FONT);
        if (hTestFont) {
            // select a known font first
            LOGFONTA lf = { 0 };
            lf.lfHeight = 16;
            strcpy_s(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), "Arial");
            HFONT hf = CreateFontIndirectA(&lf);
            HFONT oldf = (HFONT)SelectObject(hdc, hf);
            int fnLen = GetTextFaceA(hdc, LF_FACESIZE, faceName);
            SelectObject(hdc, oldf);
            DeleteObject(hf);
            SStringA fnamelbl;
            fnamelbl.Format("#68 GetTextFaceA: len=%d name='%s'", fnLen, faceName);
            DrawLabel(hdc, x0, y2 + 60, fnamelbl);
        }

        // 69. GetCurrentObject checks
        HGDIOBJ curPen = GetCurrentObject(hdc, OBJ_PEN);
        HGDIOBJ curBrush = GetCurrentObject(hdc, OBJ_BRUSH);
        HGDIOBJ curFont = GetCurrentObject(hdc, OBJ_FONT);
        HGDIOBJ curBmp = GetCurrentObject(hdc, OBJ_BITMAP);
        SStringA coLbl;
        coLbl.Format("#69 GetCurrentObject: PEN=%p BRUSH=%p FONT=%p BMP=%p",
            curPen, curBrush, curFont, curBmp);
        DrawLabel(hdc, x0, y2 + 90, coLbl);

        // 70. InvertRgn (visual)
        int y3 = y2 + 120;
        RECT rcRgnBg = { x0, y3, x0 + 250, y3 + 70 };
        HBRUSH hBg = CreateSolidBrush(RGB(100, 200, 255));
        FillRect(hdc, &rcRgnBg, hBg);
        DeleteObject(hBg);
        FrameRect(hdc, &rcRgnBg, (HBRUSH)GetStockObject(BLACK_BRUSH));
        HRGN hInvRgn = CreateRectRgn(x0 + 30, y3 + 10, x0 + 150, y3 + 60);
        InvertRgn(hdc, hInvRgn);
        DeleteObject(hInvRgn);
        DrawLabel(hdc, x0 + 260, y3 + 25, "#70 InvertRgn (inner rectangle inverted)");

        // 71. PaintRgn
        HRGN hPaintRgn = CreateRectRgn(x0 + 320, y3, x0 + 480, y3 + 70);
        HBRUSH hPaintBr = CreateSolidBrush(RGB(255, 150, 50));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, hPaintBr);
        PaintRgn(hdc, hPaintRgn);  // uses currently selected brush
        SelectObject(hdc, oldBr);
        DeleteObject(hPaintBr);
        DeleteObject(hPaintRgn);
        CRect rc2(CPoint(x0 + 320, y3), CSize(160, 70));
        FrameRect(hdc, &rc2, (HBRUSH)GetStockObject(BLACK_BRUSH));
        DrawLabel(hdc, x0 + 490, y3 + 25, "#71 PaintRgn (uses current brush)");
    }

    // ========== Group 11: CreateCompatibleDC + Window Origin + BitBlt ==========
    void DrawGroup11(HDC hdc, const RECT& rc) {
        int x0 = 30;
        int y0 = 40;

        DrawLabel(hdc, x0, y0, "#72 CreateCompatibleDC 内存DC绘制测试");
        y0 += 25;

        // 1. 在主DC上画一个参考区域边框，指示BitBlt目标位置
        const int MEM_W = 600;
        const int MEM_H = 420;
        RECT rcDst = { x0, y0, x0 + MEM_W, y0 + MEM_H };

        // 画一个浅蓝色参考背景 (用于对比原点偏移后的可见效果)
        HBRUSH hRefBg = CreateSolidBrush(RGB(240, 248, 255));
        FillRect(hdc, &rcDst, hRefBg);
        DeleteObject(hRefBg);
        FrameRect(hdc, &rcDst, (HBRUSH)GetStockObject(GRAY_BRUSH));

        // 画参考十字线(主DC上200,0对应内存DC逻辑坐标(300,40)，用户判断偏移效果)
        {
            HPEN hRefPen = CreatePen(PS_DOT, 1, RGB(120, 120, 120));
            HPEN oldPen = (HPEN)SelectObject(hdc, hRefPen);
            // 主DC目标的 (200 - x0, 0) 相对于 rcDst 起点
            MoveToEx(hdc, rcDst.left + 200, rcDst.top, NULL);
            LineTo(hdc, rcDst.left + 200, rcDst.bottom);
            MoveToEx(hdc, rcDst.left, rcDst.top + 0, NULL);
            LineTo(hdc, rcDst.right, rcDst.top + 0);
            SelectObject(hdc, oldPen);
            DeleteObject(hRefPen);
        }
        DrawLabel(hdc, x0 + MEM_W + 10, y0 + 10, "灰色=BitBlt目标区域(600x420)");
        DrawLabel(hdc, x0 + MEM_W + 10, y0 + 30, "虚线=目标(200,0)基准线");

        // 2. CreateCompatibleDC + CreateCompatibleBitmap
        HDC memDc = CreateCompatibleDC(hdc);
        if (!memDc) {
            DrawLabel(hdc, x0 + 10, y0 + 10, "CreateCompatibleDC 失败!");
            return;
        }
        HBITMAP memBmp = CreateCompatibleBitmap(hdc, MEM_W, MEM_H);
        if (!memBmp) {
            DeleteDC(memDc);
            DrawLabel(hdc, x0 + 10, y0 + 10, "CreateCompatibleBitmap 失败!");
            return;
        }
        HBITMAP oldMemBmp = (HBITMAP)SelectObject(memDc, memBmp);

        // 3. 用白色填满内存DC，防止初始内容为随机
        RECT rcMemFill = { 0, 0, MEM_W, MEM_H };
        FillRect(memDc, &rcMemFill, (HBRUSH)GetStockObject(WHITE_BRUSH));

        // 4. 画内存DC本地边界，便于看到逻辑 (-100,-100) -> (0,0) 偏移后
        //    逻辑坐标 (-100,-100) ~~ 实际就是 设备坐标 (0,0)
        //    逻辑坐标 (100,100) ~~ 实际就是 设备坐标 (200,200)
        // 先在未偏移时画一个参考框（黄色，会被覆盖的区域）
        {
            HPEN hB = CreatePen(PS_SOLID, 1, RGB(200, 200, 0));
            HPEN oldPen = (HPEN)SelectObject(memDc, hB);
            HBRUSH oldBr = (HBRUSH)SelectObject(memDc, GetStockObject(NULL_BRUSH));
            Rectangle(memDc, 0, 0, MEM_W, MEM_H);
            SelectObject(memDc, oldPen);
            SelectObject(memDc, oldBr);
            DeleteObject(hB);
        }

        // 5. SetViewportOrgEx (设置窗口/视口原点偏移到 (-100,-100))
        //    这样调用: 逻辑坐标 (200, 0) -> 设备坐标 = (300, 100)
        //    使用 SetViewportOrgEx 设置负数偏移 (即窗口在视口之外)
        POINT oldVpOrg;
        SetViewportOrgEx(memDc, -100, -100, &oldVpOrg);
        {
            SStringA lbl;
            lbl.Format("SetViewportOrgEx(memDc, -100, -100)  old=(%d,%d)",
                oldVpOrg.x, oldVpOrg.y);
            DrawLabel(hdc, x0 + MEM_W + 10, y0 + 60, lbl);
        }

        // 6. 从逻辑坐标 (200, 0) 开始绘制一组图形
        //    (200,0) 经 (-100,-100) 偏移后，在设备坐标中位置为 = (300, 100)
        int gx = 200;
        int gy = 0;

        // 6a. 画一个蓝色填充矩形
        HBRUSH hBlue = CreateSolidBrush(RGB(100, 160, 255));
        HPEN hPenBlue = CreatePen(PS_SOLID, 2, RGB(0, 40, 120));
        HBRUSH oldBr = (HBRUSH)SelectObject(memDc, hBlue);
        HPEN oldPen = (HPEN)SelectObject(memDc, hPenBlue);
        Rectangle(memDc, gx, gy, gx + 150, gy + 80);
        SelectObject(memDc, oldPen);
        SelectObject(memDc, oldBr);
        DeleteObject(hBlue);
        DeleteObject(hPenBlue);
        {
            SStringA lbl;
            lbl.Format("Rect 逻辑(200,0,350,80)  =>  设备(300,100,450,180)");
            DrawLabel(hdc, x0 + MEM_W + 10, y0 + 90, lbl);
        }

        // 6b. 紧挨着画一个yy.png位图画刷填充椭圆
        if (m_yyBmp) {
            HBRUSH hYyBr = CreatePatternBrush(m_yyBmp);
            HPEN hPenR = CreatePen(PS_SOLID, 2, RGB(180, 0, 60));
            oldBr = (HBRUSH)SelectObject(memDc, hYyBr);
            oldPen = (HPEN)SelectObject(memDc, hPenR);
            Ellipse(memDc, gx + 160, gy, gx + 160 + 140, gy + 90);
            SelectObject(memDc, oldPen);
            SelectObject(memDc, oldBr);
            DeleteObject(hYyBr);
            DeleteObject(hPenR);
            DrawLabel(hdc, x0 + MEM_W + 10, y0 + 110, "Ellipse yy.png图案填充 逻辑(360,0,500,90)");
        }

        // 6c. 画一条红色粗线起点在 (gx, gy + 100)
        HPEN hRedWide = CreatePen(PS_SOLID, 6, RGB(220, 20, 20));
        oldPen = (HPEN)SelectObject(memDc, hRedWide);
        MoveToEx(memDc, gx, gy + 100, NULL);
        LineTo(memDc, gx + 300, gy + 160);
        SelectObject(memDc, oldPen);
        DeleteObject(hRedWide);

        // 6d. 画一段文字
        SetBkMode(memDc, OPAQUE);
        SetBkColor(memDc, RGB(255, 255, 200));
        SetTextColor(memDc, RGB(0, 0, 128));
        {
            const char * text = "Text from memDC: viewport(-100,-100) draw@(200,0)";
            TextOutA(memDc, gx, gy + 180,
                text, strlen(text));

        }

        // 6e. 如果有 yy.png，再把它作为位图 StretchBlt 画到内存DC里
        if (m_yyBmp) {
            HDC tmpDc = CreateCompatibleDC(memDc);
            HBITMAP oldTmp = (HBITMAP)SelectObject(tmpDc, m_yyBmp);
            BITMAP bm;
            GetObjectA(m_yyBmp, sizeof(bm), &bm);
            StretchBlt(memDc, gx, gy + 210, bm.bmWidth * 4, bm.bmHeight * 4,
                tmpDc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
            SelectObject(tmpDc, oldTmp);
            DeleteDC(tmpDc);
        }

        // 7. 恢复视口原点 (仅用于在内存DC左上角落个说明标记)
        SetViewportOrgEx(memDc, oldVpOrg.x, oldVpOrg.y, NULL);
        SetTextColor(memDc, RGB(128, 128, 128));
        SetBkMode(memDc, TRANSPARENT);
        {
            const char * text = "memDC native origin (0,0) corner";
            TextOutA(memDc, 5, 5, text, strlen(text));
        }

        // 8. BitBlt 到主DC (SRCCOPY)
        BitBlt(hdc,
            rcDst.left, rcDst.top, MEM_W, MEM_H,
            memDc, 0, 0, SRCCOPY);
        {
            SStringA lbl;
            lbl.Format("BitBlt(hdc, dst=(%d,%d) size=%dx%d, memDc, src=(0,0), SRCCOPY)",
                rcDst.left, rcDst.top, MEM_W, MEM_H);
            DrawLabel(hdc, x0 + MEM_W + 10, y0 + 140, lbl);
        }

        // 9. 再用 SRCAND / SRCPAINT 做一次小的对比块（必须在 memDC 销毁前）
        {
            int bx = rcDst.left;
            int by = rcDst.bottom + 10;
            int sz = 80;
            HBRUSH hMaskBg = CreateSolidBrush(RGB(0xFF, 0xFF, 0x00));
            RECT rcMask = { bx, by, bx + sz * 3 + 20, by + sz };
            FillRect(hdc, &rcMask, hMaskBg);
            DeleteObject(hMaskBg);

            BitBlt(hdc, bx,      by, sz, sz, memDc, 0, 0, SRCCOPY);
            BitBlt(hdc, bx+sz+10,by, sz, sz, memDc, 0, 0, SRCAND);
            BitBlt(hdc, bx+sz*2+20, by, sz, sz, memDc, 0, 0, SRCPAINT);

            DrawLabel(hdc, bx,       by + sz + 30, "SRCCOPY (yellow bg)");
            DrawLabel(hdc, bx+sz+10, by + sz + 3, "SRCAND");
            DrawLabel(hdc, bx+sz*2+20, by + sz + 30, "SRCPAINT");
        }

        // 清理
        SelectObject(memDc, oldMemBmp);
        DeleteObject(memBmp);
        DeleteDC(memDc);
    }

    // ========== Group 12: render-gdi 回归测试（装饰线 / 文本+矩阵 / SetMatrix 旋转） ==========
    // 本组针对 render-gdi 切换后暴露的三个 BUG：
    //   BUG2: DrawTextA/TextOutA 装饰线（下划线/删除线）位置错误或重复绘制
    //   BUG3: 世界变换（SetWorldTransform）激活时文本绘制异常/不显示
    //   BUG1: SetMatrix 旋转后控件位置错误（底层为 SetWorldTransform 旋转+文本+裁剪交互）
    void DrawGroup12(HDC hdc, const RECT& rc) {
        int x0 = 30;
        int y0 = 40;

        COLORREF oldTxt = SetTextColor(hdc, RGB(0, 0, 0));
        int oldBkMode = SetBkMode(hdc, TRANSPARENT);
        int oldGm = SetGraphicsMode(hdc, GM_ADVANCED);

        LOGFONTA lfNormal = { 0 };
        lfNormal.lfHeight = 20;
        strcpy_s(lfNormal.lfFaceName, ARRAYSIZE(lfNormal.lfFaceName), "Arial");
        HFONT hNormal = CreateFontIndirectA(&lfNormal);
        HFONT oldFont = (HFONT)SelectObject(hdc, hNormal);

        // ---- BUG2-A: DrawTextA 装饰线（DT_SINGLELINE + DT_VCENTER） ----
        // 期望：下划线在文字下方、删除线在文字中部，且只画一次、宽度=文字宽
        {
            int y = y0;
            const char* labels[] = { "Underline", "StrikeOut", "Underline+StrikeOut" };
            for (int i = 0; i < 3; i++) {
                LOGFONTA lf = lfNormal;
                if (i == 0) lf.lfUnderline = TRUE;
                else if (i == 1) lf.lfStrikeOut = TRUE;
                else { lf.lfUnderline = TRUE; lf.lfStrikeOut = TRUE; }
                HFONT hf = CreateFontIndirectA(&lf);
                HFONT of = (HFONT)SelectObject(hdc, hf);
                RECT r = { x0, y + i * 32, x0 + 360, y + i * 32 + 28 };
                HBRUSH hb = CreateSolidBrush(RGB(245, 245, 250));
                FillRect(hdc, &r, hb);
                DeleteObject(hb);
                FrameRect(hdc, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
                char buf[128];
                sprintf(buf, "#B2A-%d DrawTextA %s: Sample 文本", i + 1, labels[i]);
                DrawTextA(hdc, buf, -1, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                SelectObject(hdc, of);
                DeleteObject(hf);
            }
            DrawLabel(hdc, x0 + 380, y0 + 30, "BUG2-A: DrawTextA 装饰线");
            DrawLabel(hdc, x0 + 380, y0 + 48, "(下划线/删除线位置+是否重复)");
        }

        // ---- BUG2-B: TextOutA 装饰线 ----
        // 期望：TextOutA 的下划线/删除线位置与 DrawTextA 一致
        {
            int y = y0 + 110;
            LOGFONTA lf = lfNormal;
            lf.lfUnderline = TRUE;
            HFONT hf = CreateFontIndirectA(&lf);
            HFONT of = (HFONT)SelectObject(hdc, hf);
            TextOutA(hdc, x0, y, "#B2B-1 TextOutA Underline: Sample", -1);
            SelectObject(hdc, of);
            DeleteObject(hf);

            lf = lfNormal;
            lf.lfStrikeOut = TRUE;
            hf = CreateFontIndirectA(&lf);
            of = (HFONT)SelectObject(hdc, hf);
            TextOutA(hdc, x0, y + 28, "#B2B-2 TextOutA StrikeOut: Sample", -1);
            SelectObject(hdc, of);
            DeleteObject(hf);
            DrawLabel(hdc, x0 + 380, y + 14, "BUG2-B: TextOutA 装饰线");
        }

        // ---- BUG3-A: DrawTextA 在旋转变换下 ----
        // 期望：文字随矩阵旋转，且可正常显示（不被裁剪/不消失）
        {
            int y = y0 + 180;
            RECT r = { x0, y, x0 + 300, y + 100 };
            HBRUSH hb = CreateSolidBrush(RGB(255, 255, 240));
            FillRect(hdc, &r, hb);
            DeleteObject(hb);
            FrameRect(hdc, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));

            int saveDc = SaveDC(hdc);
            double ang = 15.0 * 3.14159265358979 / 180.0;
            double cs = cos(ang), sn = sin(ang);
            double cx = x0 + 150, cy = y + 50;
            XFORM xform;
            xform.eM11 = (FLOAT)cs;  xform.eM12 = (FLOAT)sn;
            xform.eM21 = (FLOAT)-sn; xform.eM22 = (FLOAT)cs;
            xform.eDx = (FLOAT)(cx - cs * cx + sn * cy);
            xform.eDy = (FLOAT)(cy - sn * cx - cs * cy);
            SetWorldTransform(hdc, &xform);
            SetTextColor(hdc, RGB(200, 0, 0));
            RECT rcText = { x0, y, x0 + 300, y + 100 };
            DrawTextA(hdc, "#B3A DrawTextA under rotate 15deg", -1, &rcText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SetTextColor(hdc, RGB(0, 0, 0));
            RestoreDC(hdc, saveDc);
            DrawLabel(hdc, x0 + 320, y + 40, "BUG3-A: DrawTextA + 旋转");
            DrawLabel(hdc, x0 + 320, y + 58, "(文字应旋转且可见)");
        }

        // ---- BUG3-B: TextOutA 在缩放变换下 ----
        // 期望：文字按 2x 缩放后仍可正常显示
        {
            int y = y0 + 300;
            RECT r = { x0, y, x0 + 300, y + 60 };
            HBRUSH hb = CreateSolidBrush(RGB(240, 255, 240));
            FillRect(hdc, &r, hb);
            DeleteObject(hb);
            FrameRect(hdc, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));

            int saveDc = SaveDC(hdc);
            XFORM xform;
            xform.eM11 = 1.5f; xform.eM12 = 0.0f;
            xform.eM21 = 0.0f; xform.eM22 = 1.5f;
            xform.eDx = (FLOAT)x0; xform.eDy = (FLOAT)y;
            SetWorldTransform(hdc, &xform);
            SetTextColor(hdc, RGB(0, 0, 180));
            TextOutA(hdc, 0, 0, "#B3B TextOutA under scale 1.5x", -1);
            SetTextColor(hdc, RGB(0, 0, 0));
            RestoreDC(hdc, saveDc);
            DrawLabel(hdc, x0 + 320, y + 20, "BUG3-B: TextOutA + 缩放 1.5x");
        }

        // ---- BUG1: SetMatrix 旋转场景（旋转中心+裁剪+文本综合） ----
        // 模拟 CTestMcAdapterFix::getView 中按钮绕中心旋转 10 度的场景：
        //   mtx.setRotate(10); preTranslate(-w/2,-h/2); postTranslate(w/2,h/2)
        // 期望：矩形+文字一起绕中心旋转，位置正确，文字可见
        {
            int y = y0 + 380;
            int bx = x0, by = y, bw = 160, bh = 50;

            // 参考框（无旋转）
            HBRUSH hb0 = CreateSolidBrush(RGB(235, 235, 235));
            RECT rcRef = { bx, by, bx + bw, by + bh };
            FillRect(hdc, &rcRef, hb0);
            DeleteObject(hb0);
            FrameRect(hdc, &rcRef, (HBRUSH)GetStockObject(BLACK_BRUSH));
            DrawTextA(hdc, "ref (no rotate)", -1, &rcRef, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            DrawLabel(hdc, bx + bw + 10, by + 16, "BUG1: ref vs rotate-around-center");

            // 旋转 10 度
            int saveDc = SaveDC(hdc);
            double ang = 10.0 * 3.14159265358979 / 180.0;
            double cs = cos(ang), sn = sin(ang);
            double cx = bx + bw / 2.0, cy = by + bh / 2.0;
            XFORM xform;
            xform.eM11 = (FLOAT)cs;  xform.eM12 = (FLOAT)sn;
            xform.eM21 = (FLOAT)-sn; xform.eM22 = (FLOAT)cs;
            xform.eDx = (FLOAT)(cx - cs * cx + sn * cy);
            xform.eDy = (FLOAT)(cy - sn * cx - cs * cy);
            SetWorldTransform(hdc, &xform);

            HBRUSH hb1 = CreateSolidBrush(RGB(255, 230, 230));
            RECT rcBtn = { bx, by + 70, bx + bw, by + 70 + bh };
            FillRect(hdc, &rcBtn, hb1);
            DeleteObject(hb1);
            FrameRect(hdc, &rcBtn, (HBRUSH)GetStockObject(BLACK_BRUSH));
            SetTextColor(hdc, RGB(180, 0, 0));
            DrawTextA(hdc, "rotate 10deg", -1, &rcBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SetTextColor(hdc, RGB(0, 0, 0));
            RestoreDC(hdc, saveDc);
            DrawLabel(hdc, bx + bw + 10, by + 86, "(红框+文字应绕中心旋转 10°)");
        }

        // ---- BUG3-C: SaveDC/RestoreDC 与世界变换的状态保持 ----
        // 期望：SaveDC→SetWorldTransform→RestoreDC 后矩阵恢复；
        //      嵌套 SaveDC/RestoreDC 不会破坏文本绘制
        {
            int y = y0 + 520;
            // 1) 先设一个平移变换
            int s1 = SaveDC(hdc);
            XFORM xf1;
            xf1.eM11 = 1.0f; xf1.eM12 = 0.0f; xf1.eM21 = 0.0f; xf1.eM22 = 1.0f;
            xf1.eDx = (FLOAT)(x0 + 20); xf1.eDy = (FLOAT)y;
            SetWorldTransform(hdc, &xf1);
            TextOutA(hdc, 0, 0, "#B3C-1 after SetWorldTransform(translate)", -1);

            // 2) 嵌套 SaveDC + 不同变换 + 文本
            int s2 = SaveDC(hdc);
            XFORM xf2;
            xf2.eM11 = 1.0f; xf2.eM12 = 0.0f; xf2.eM21 = 0.0f; xf2.eM22 = 1.0f;
            xf2.eDx = (FLOAT)(x0 + 20); xf2.eDy = (FLOAT)(y + 30);
            SetWorldTransform(hdc, &xf2);
            TextOutA(hdc, 0, 0, "#B3C-2 nested SaveDC + translate", -1);
            RestoreDC(hdc, s2);

            // 3) RestoreDC 后应回到 s1 的变换，此行应在 (x0+20, y+60) 位置
            //    若状态被破坏则位置错误或不可见
            XFORM xf3;
            xf3.eM11 = 1.0f; xf3.eM12 = 0.0f; xf3.eM21 = 0.0f; xf3.eM22 = 1.0f;
            xf3.eDx = (FLOAT)(x0 + 20); xf3.eDy = (FLOAT)(y + 60);
            SetWorldTransform(hdc, &xf3);
            TextOutA(hdc, 0, 0, "#B3C-3 after nested restore (should be at left+20)", -1);

            RestoreDC(hdc, s1);
            DrawLabel(hdc, x0 + 380, y + 20, "BUG3-C: SaveDC/RestoreDC + WorldTransform");
            DrawLabel(hdc, x0 + 380, y + 38, "(三行应左对齐于 x0+20)");
        }

        // Restore
        SelectObject(hdc, oldFont);
        DeleteObject(hNormal);
        SetGraphicsMode(hdc, oldGm);
        SetBkMode(hdc, oldBkMode);
        SetTextColor(hdc, oldTxt);
    }

    // =========================================================================
    // DrawGroup13: Clip + ViewportOrg + DrawText 交互（GetRenderTarget 序列回归）
    //   SOUI 真实序列(Swnd.cpp GetRenderTarget):
    //     1. CreateRenderTarget(rcRT.Width(), rcRT.Height())  -> RT 是一张小位图
    //     2. OffsetViewportOrg(-rcRT.left, -rcRT.top)         -> 负偏移把"客户区坐标"
    //        映射到 RT 面（控件绘制一律用客户区坐标）
    //     3. PushClipRegion(rcRT)                             -> RT 面上 (0,0,W,H)
    //     4. 控件绘制 DrawText(hdc, rcText[客户区坐标], ...)
    //     5. BitBlt 合成回主面
    //   版式: 每个子测试独占一个横带；方框靠左，说明标签靠右，互不重叠
    //   D1-D4 用主 DC + 正偏移演示 offset/clip 交互；
    //   P2A/P2C 用 memdc 忠实模拟 SOUI 负偏移序列（在真实 GDI 与 swinx 上都应可见）
    // =========================================================================
    void DrawGroup13(HDC hdc, const RECT& rc) {
        SUNUSED(rc);

        COLORREF oldTxt = SetTextColor(hdc, RGB(0, 0, 0));
        int oldBkMode = SetBkMode(hdc, TRANSPARENT);
        int oldGm = SetGraphicsMode(hdc, GM_ADVANCED);

        LOGFONTA lfNormal = { 0 };
        lfNormal.lfHeight = 20;
        strcpy_s(lfNormal.lfFaceName, ARRAYSIZE(lfNormal.lfFaceName), "Arial");
        HFONT hNormal = CreateFontIndirectA(&lfNormal);
        HFONT oldFont = (HFONT)SelectObject(hdc, hNormal);

        HPEN hCrossPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));

        // ---- D1-D4: 诊断小组（隔离性测试，四个并排小方框，y=40..90）----
        //   主 DC + 正偏移(+x,+y) + RT 相对坐标，考察 offset 与 clip 的交互：
        //   D1 只 OffsetViewport / D2 Offset->Clip / D3 Clip->Offset / D4 只 Clip
        //   注：swinx 的 clip API 工作在当前逻辑坐标空间（CTM 含视口原点），
        //       与 SOUI render-gdi(linux) 对后端的约定一致：偏移后再 PushClip
        //       必须传逻辑矩形（D2），偏移前 PushClip 用绝对矩形（D3）。
        //       真实 Win32 GDI 的 clip 区域则是原始设备坐标（SOUI 的 Windows
        //       渲染层用 m_ptOrg 手动补偿），同一矩形在两边的解释不同。
        {
            const int y = 40;
            const int H = 50;
            const int W = 170;
            const int gap = 20;
            int x = 30;

            // --- D1: 纯 OffsetViewportOrg + DrawText（无任何 clip）---
            {
                RECT rcBox = { x, y, x + W, y + H };
                HBRUSH hb = CreateSolidBrush(RGB(255, 240, 240));
                FillRect(hdc, &rcBox, hb);
                DeleteObject(hb);
                FrameRect(hdc, &rcBox, (HBRUSH)GetStockObject(BLACK_BRUSH));

                ::SaveDC(hdc);
                ::OffsetViewportOrgEx(hdc, x, y, NULL);

                RECT rt = { 0, 0, W, H };
                SetTextColor(hdc, RGB(180, 0, 0));
                DrawTextA(hdc, "D1 ONLY-OFFSET", -1, &rt, DT_LEFT | DT_TOP | DT_SINGLELINE);
                SetTextColor(hdc, RGB(0, 0, 0));

                HPEN op = (HPEN)SelectObject(hdc, hCrossPen);
                MoveToEx(hdc, 0, 0, NULL); LineTo(hdc, 30, 0);
                MoveToEx(hdc, 0, 0, NULL); LineTo(hdc, 0, 30);
                SelectObject(hdc, op);

                ::RestoreDC(hdc, -1);
            }
            DrawLabel(hdc, x, y + H + 4,  "D1: 只 OffsetViewport(+x,+y)");
            DrawLabel(hdc, x, y + H + 20, "预期: 红字+红十字在框内左上角");
            x += W + gap;

            // --- D2: 先 OffsetViewport 后 PushClip（SOUI SOsrPanel::Draw 模式）---
            {
                RECT rcBox = { x, y, x + W, y + H };
                HBRUSH hb = CreateSolidBrush(RGB(240, 255, 240));
                FillRect(hdc, &rcBox, hb);
                DeleteObject(hb);
                FrameRect(hdc, &rcBox, (HBRUSH)GetStockObject(BLACK_BRUSH));

                ::SaveDC(hdc);
                ::OffsetViewportOrgEx(hdc, x, y, NULL);
                // 偏移后再 PushClip 必须传当前逻辑坐标（相对视口原点）——
                // SOUI render-gdi(linux) 的 PushClipRect 即此约定
                RECT rcClipLogical = { 0, 0, W, H };
                HRGN hrgn = ::CreateRectRgnIndirect(&rcClipLogical);
                ::ExtSelectClipRgn(hdc, hrgn, RGN_COPY);
                ::DeleteObject(hrgn);

                RECT rt = { 0, 0, W, H };
                SetTextColor(hdc, RGB(0, 128, 0));
                DrawTextA(hdc, "D2 OFFSET-THEN-CLIP", -1, &rt, DT_LEFT | DT_TOP | DT_SINGLELINE);
                SetTextColor(hdc, RGB(0, 0, 0));

                HPEN op = (HPEN)SelectObject(hdc, hCrossPen);
                MoveToEx(hdc, 0, 0, NULL); LineTo(hdc, 30, 0);
                MoveToEx(hdc, 0, 0, NULL); LineTo(hdc, 0, 30);
                SelectObject(hdc, op);

                ::RestoreDC(hdc, -1);
            }
            DrawLabel(hdc, x, y + H + 4,  "D2: Offset THEN Clip(逻辑矩形)");
            DrawLabel(hdc, x, y + H + 20, "预期: 绿字+红十字在框内左上角");
            x += W + gap;

            // --- D3: 先 PushClip 后 OffsetViewport ---
            {
                RECT rcBox = { x, y, x + W, y + H };
                HBRUSH hb = CreateSolidBrush(RGB(240, 240, 255));
                FillRect(hdc, &rcBox, hb);
                DeleteObject(hb);
                FrameRect(hdc, &rcBox, (HBRUSH)GetStockObject(BLACK_BRUSH));

                ::SaveDC(hdc);
                HRGN hrgn = ::CreateRectRgnIndirect(&rcBox);
                ::ExtSelectClipRgn(hdc, hrgn, RGN_COPY);
                ::DeleteObject(hrgn);
                ::OffsetViewportOrgEx(hdc, x, y, NULL);

                RECT rt = { 0, 0, W, H };
                SetTextColor(hdc, RGB(0, 0, 180));
                DrawTextA(hdc, "D3 CLIP-THEN-OFFSET", -1, &rt, DT_LEFT | DT_TOP | DT_SINGLELINE);
                SetTextColor(hdc, RGB(0, 0, 0));

                HPEN op = (HPEN)SelectObject(hdc, hCrossPen);
                MoveToEx(hdc, 0, 0, NULL); LineTo(hdc, 30, 0);
                MoveToEx(hdc, 0, 0, NULL); LineTo(hdc, 0, 30);
                SelectObject(hdc, op);

                ::RestoreDC(hdc, -1);
            }
            DrawLabel(hdc, x, y + H + 4,  "D3: Clip THEN Offset");
            DrawLabel(hdc, x, y + H + 20, "预期: 与 D2 相同(clip 在偏移前用绝对坐标)");
            x += W + gap;

            // --- D4: 纯 PushClip + DrawText（client 坐标，无 OffsetViewport）---
            {
                RECT rcBox = { x, y, x + W, y + H };
                HBRUSH hb = CreateSolidBrush(RGB(255, 250, 240));
                FillRect(hdc, &rcBox, hb);
                DeleteObject(hb);
                FrameRect(hdc, &rcBox, (HBRUSH)GetStockObject(BLACK_BRUSH));

                ::SaveDC(hdc);
                HRGN hrgn = ::CreateRectRgnIndirect(&rcBox);
                ::ExtSelectClipRgn(hdc, hrgn, RGN_COPY);
                ::DeleteObject(hrgn);

                RECT rt = rcBox;
                SetTextColor(hdc, RGB(160, 0, 160));
                DrawTextA(hdc, "D4 ONLY-CLIP", -1, &rt, DT_LEFT | DT_TOP | DT_SINGLELINE);
                SetTextColor(hdc, RGB(0, 0, 0));

                HPEN op = (HPEN)SelectObject(hdc, hCrossPen);
                MoveToEx(hdc, x, y, NULL); LineTo(hdc, x + 30, y);
                MoveToEx(hdc, x, y, NULL); LineTo(hdc, x, y + 30);
                SelectObject(hdc, op);

                ::RestoreDC(hdc, -1);
            }
            DrawLabel(hdc, x, y + H + 4,  "D4: 只 Clip，无 OffsetViewport");
            DrawLabel(hdc, x, y + H + 20, "预期: 紫字+红十字在框内左上角");
        }

        // ---- P2A: SOUI GetRenderTarget 序列（memdc 忠实模拟）----
        //   CreateRT(W,H) -> PushClipRegion -> OffsetViewportOrg(-L,-T)
        //   -> 控件以客户区坐标绘制 -> BitBlt 合成回主面
        {
            const int L = 30, T = 140, W = 420, H = 60;
            RECT rcRT = { L, T, L + W, T + H };
            FrameRect(hdc, &rcRT, (HBRUSH)GetStockObject(BLACK_BRUSH));

            HDC memdc = CreateCompatibleDC(hdc);
            HBITMAP bmp = CreateCompatibleBitmap(hdc, W, H);
            HGDIOBJ oldBmp = SelectObject(memdc, bmp);

            RECT rcFull = { 0, 0, W, H };
            HBRUSH hb = CreateSolidBrush(RGB(240, 250, 240));
            FillRect(memdc, &rcFull, hb); // ClearRect
            DeleteObject(hb);

            ::SaveDC(memdc);
            HRGN hrgn = ::CreateRectRgn(0, 0, W, H); // PushClipRegion: RT 面上区域
            ::ExtSelectClipRgn(memdc, hrgn, RGN_COPY);
            ::DeleteObject(hrgn);
            ::OffsetViewportOrgEx(memdc, -rcRT.left, -rcRT.top, NULL); // SOUI 负偏移

            // 控件按客户区坐标绘制（rcText 即 rcRT 在客户区的矩形）
            SetTextColor(memdc, RGB(0, 128, 0));
            DrawTextA(memdc, "hello soui in page2", -1, &rcRT,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            SetTextColor(memdc, RGB(0, 0, 0));

            HPEN op = (HPEN)SelectObject(memdc, hCrossPen);
            ::MoveToEx(memdc, rcRT.left, rcRT.top, NULL); ::LineTo(memdc, rcRT.left + 40, rcRT.top);
            ::MoveToEx(memdc, rcRT.left, rcRT.top, NULL); ::LineTo(memdc, rcRT.left, rcRT.top + 40);
            ::SelectObject(memdc, op);

            ::RestoreDC(memdc, -1);

            BitBlt(hdc, rcRT.left, rcRT.top, W, H, memdc, 0, 0, SRCCOPY);
            SelectObject(memdc, oldBmp);
            DeleteObject(bmp);
            DeleteDC(memdc);

            DrawLabel(hdc, 470, T,      "#P2A: CreateRT(W,H) + OffsetViewportOrg(-L,-T)");
            DrawLabel(hdc, 470, T + 18, "控件按客户区坐标绘制(SOUI 真实序列)");
            DrawLabel(hdc, 470, T + 36, "绿字应在方框内垂直居中, 红十字在左上角");
        }

        // ---- P2B: 仅 PushClipRect，无 OffsetViewportOrg（纯 ClipRect 检查）----
        {
            const int L = 30, T = 220, W = 420, H = 60;
            RECT rcRT = { L, T, L + W, T + H };

            HBRUSH hb = CreateSolidBrush(RGB(250, 245, 230));
            FillRect(hdc, &rcRT, hb);
            DeleteObject(hb);
            FrameRect(hdc, &rcRT, (HBRUSH)GetStockObject(BLACK_BRUSH));

            ::SaveDC(hdc);
            HRGN hrgn = ::CreateRectRgnIndirect(&rcRT);
            ::ExtSelectClipRgn(hdc, hrgn, RGN_COPY);
            ::DeleteObject(hrgn);
            // 注意：没有 OffsetViewportOrg

            SetTextColor(hdc, RGB(180, 80, 0));
            DrawTextA(hdc, "#P2B: PushClipRect 仅裁剪 不偏移Viewport", -1, &rcRT,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            SetTextColor(hdc, RGB(0, 0, 0));

            ::RestoreDC(hdc, -1);

            DrawLabel(hdc, 470, T,      "#P2B: PushClipRect 无 OffsetViewportOrg");
            DrawLabel(hdc, 470, T + 18, "橙字应在方框内垂直居中");
        }

        // ---- P2C: 嵌套 RT（子控件在父控件 RT 内绘制，SOUI 负偏移 + 客户区坐标）----
        //   外层 RT: PushClip + OffsetViewportOrg(-(oL,oT))；内层: 子矩形 PushClip
        //   （clip 在当前逻辑空间，绝对矩形经 CTM 负偏移映射回 RT 面）+ 再次
        //   Offset；内层 PopClip 后外层 viewport/clip 应保持正确
        {
            const int oL = 30, oT = 300, oW = 500, oH = 180;
            RECT rcOuterRT = { oL, oT, oL + oW, oT + oH };
            FrameRect(hdc, &rcOuterRT, (HBRUSH)GetStockObject(BLACK_BRUSH));

            HDC memdc = CreateCompatibleDC(hdc);
            HBITMAP bmp = CreateCompatibleBitmap(hdc, oW, oH);
            HGDIOBJ oldBmp = SelectObject(memdc, bmp);

            RECT rcFull = { 0, 0, oW, oH };
            HBRUSH hbOut = CreateSolidBrush(RGB(235, 245, 255));
            FillRect(memdc, &rcFull, hbOut);
            DeleteObject(hbOut);

            // --- 外层 RT：offset(-(oL,oT))，之后一律客户区坐标 ---
            int sOuter = ::SaveDC(memdc);
            HRGN hOuterRgn = ::CreateRectRgn(0, 0, oW, oH);
            ::ExtSelectClipRgn(memdc, hOuterRgn, RGN_COPY);
            ::DeleteObject(hOuterRgn);
            ::OffsetViewportOrgEx(memdc, -oL, -oT, NULL);

            SetTextColor(memdc, RGB(0, 80, 160));
            RECT rcOuterLabel = { oL + 10, oT + 8, oL + oW - 10, oT + 30 };
            DrawTextA(memdc, "OUTER RT (OffsetViewport applied)", -1, &rcOuterLabel,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE);

            // --- 内层 RT：客户区坐标的子矩形 (oL+40,oT+60)~(oL+440,oT+120) ---
            const int subX = 40, subY = 60, subW = 400, subH = 60;
            RECT rcSubClipClient = { oL + subX, oT + subY, oL + subX + subW, oT + subY + subH };

            ::Rectangle(memdc, rcSubClipClient.left, rcSubClipClient.top,
                rcSubClipClient.right, rcSubClipClient.bottom);

            int sInner = ::SaveDC(memdc);
            // clip 矩形用客户区（绝对）坐标——SOUI DispatchPaint 的 PushClipRect
            // 传 GetClientRect()（绝对坐标），后端在当前逻辑空间（CTM 含负偏移）
            // 下应用，映射到 RT 面上恰为 (subX,subY)
            HRGN hInnerRgn = ::CreateRectRgnIndirect(&rcSubClipClient);
            ::ExtSelectClipRgn(memdc, hInnerRgn, RGN_AND);
            ::DeleteObject(hInnerRgn);
            // SOUI 中子控件有自己的 RT（新位图，原点从 0 开始），故这里必须用
            // 绝对 SetViewportOrg 把子控件客户区 (0,0) 对到 RT 面上的 (subX,subY)，
            // 而不是在外层偏移基础上再做相对偏移
            ::SetViewportOrgEx(memdc, subX, subY, NULL);

            SetTextColor(memdc, RGB(180, 0, 0));
            RECT rcSubText = { 0, 0, subW, subH };
            DrawTextA(memdc, "INNER RT: hello soui in page2 (嵌套 RT 文本)", -1, &rcSubText,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            SetTextColor(memdc, RGB(0, 0, 0));

            // 内层 PopClip：恢复到内层 SaveDC 之前
            ::RestoreDC(memdc, -1);

            RECT rcOuterCheck = { oL + 10, oT + 140, oL + oW - 10, oT + 170 };
            SetTextColor(memdc, RGB(0, 128, 0));
            DrawTextA(memdc, "OUTER RT after inner PopClip: 此处文字仍应可见（绿色）", -1, &rcOuterCheck,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            SetTextColor(memdc, RGB(0, 0, 0));

            ::RestoreDC(memdc, sOuter);

            BitBlt(hdc, oL, oT, oW, oH, memdc, 0, 0, SRCCOPY);
            SelectObject(memdc, oldBmp);
            DeleteObject(bmp);
            DeleteDC(memdc);

            DrawLabel(hdc, 550, oT,      "#P2C: 嵌套 PushClip + 两次 OffsetViewport");
            DrawLabel(hdc, 550, oT + 18, "(memdc 模拟子控件在父控件 RT 内绘制)");
            DrawLabel(hdc, 550, oT + 36, "蓝字: outer RT 顶部");
            DrawLabel(hdc, 550, oT + 54, "红字: inner RT (hello soui)");
            DrawLabel(hdc, 550, oT + 72, "绿字: inner PopClip 后 outer RT 底部");
        }

        // ---- P2D: GetClipBox 行为验证（返回 rect 应与输入裁剪一致）----
        {
            const int T = 500;
            RECT rects[3] = {
                { 30, T, 230, T + 40 },
                { 250, T, 450, T + 40 },
                { 470, T, 670, T + 40 },
            };
            for (int i = 0; i < 3; i++) {
                FrameRect(hdc, &rects[i], (HBRUSH)GetStockObject(GRAY_BRUSH));
                int s = ::SaveDC(hdc);
                HRGN hr = ::CreateRectRgnIndirect(&rects[i]);
                ::ExtSelectClipRgn(hdc, hr, RGN_COPY);
                ::DeleteObject(hr);
                RECT rcBox;
                int typ = ::GetClipBox(hdc, &rcBox);
                char buf[128];
                sprintf(buf, "#P2D-%d GetClipBox=%d rc=(%d,%d)-(%d,%d)", i + 1, typ,
                    rcBox.left, rcBox.top, rcBox.right, rcBox.bottom);
                // 文字直接画进方框：坐标一致则文字可见且不出框
                SetTextColor(hdc, RGB(80, 0, 128));
                DrawTextA(hdc, buf, -1, &rects[i], DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                SetTextColor(hdc, RGB(0, 0, 0));
                ::RestoreDC(hdc, s);
            }
            // 第 4 块：视口偏移下 GetClipBox 必须返回逻辑坐标（相对原点）——
            // 该用例可直接区分"逻辑空间"与"设备空间"两种 clip 实现，
            // mc 控件空白回归（GetClipBox 返回设备坐标）即由此发现
            {
                const int oy = T + 64;
                RECT rcHost = { 30, oy, 450, oy + 40 };
                FrameRect(hdc, &rcHost, (HBRUSH)GetStockObject(GRAY_BRUSH));
                int s = ::SaveDC(hdc);
                ::SetViewportOrgEx(hdc, 30, oy, NULL);
                RECT rcClipLogical = { 0, 0, 420, 40 };
                HRGN hr = ::CreateRectRgnIndirect(&rcClipLogical);
                ::ExtSelectClipRgn(hdc, hr, RGN_COPY);
                ::DeleteObject(hr);
                RECT rcBox;
                ::GetClipBox(hdc, &rcBox);
                char buf[128];
                sprintf(buf, "#P2D-4 GetClipBox=(%d,%d)-(%d,%d)",
                    rcBox.left, rcBox.top, rcBox.right, rcBox.bottom);
                SetTextColor(hdc, RGB(80, 0, 128));
                DrawTextA(hdc, buf, -1, &rcClipLogical, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                SetTextColor(hdc, RGB(0, 0, 0));
                ::RestoreDC(hdc, s);
                DrawLabel(hdc, 470, oy,      "#P2D-4: 偏移下 GetClipBox 应返回逻辑坐标");
                DrawLabel(hdc, 470, oy + 18, "期望 (0,0)-(420,40)，且紫字可见");
            }
            DrawLabel(hdc, 30, T + 44, "#P2D: GetClipBox 打印在方框内，坐标应与方框一致且文字可见");
        }

        // ---- P2E: IntersectClipRect / ExcludeClipRect 之后的 DrawText ----
        {
            const int L = 30, T = 620, W = 420, H = 60;
            RECT rBig = { L, T, L + W, T + H };

            HBRUSH hb = CreateSolidBrush(RGB(245, 245, 245));
            FillRect(hdc, &rBig, hb);
            DeleteObject(hb);
            FrameRect(hdc, &rBig, (HBRUSH)GetStockObject(BLACK_BRUSH));

            int s0 = ::SaveDC(hdc);
            HRGN hBig = ::CreateRectRgnIndirect(&rBig);
            ::ExtSelectClipRgn(hdc, hBig, RGN_COPY);
            ::DeleteObject(hBig);

            // 排除中间一块
            RECT rExcl = { L + 120, T + 10, L + 280, T + 50 };
            HPEN hRedPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
            HPEN oldRP = (HPEN)SelectObject(hdc, hRedPen);
            HBRUSH oldRB = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Rectangle(hdc, rExcl.left, rExcl.top, rExcl.right, rExcl.bottom);
            SelectObject(hdc, oldRB);
            SelectObject(hdc, oldRP);
            DeleteObject(hRedPen);
            ::ExcludeClipRect(hdc, rExcl.left, rExcl.top, rExcl.right, rExcl.bottom);

            SetTextColor(hdc, RGB(0, 0, 180));
            DrawTextA(hdc, "#P2E: 中间红框后文字应在两侧可见", -1, &rBig,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            SetTextColor(hdc, RGB(0, 0, 0));

            ::RestoreDC(hdc, s0);

            DrawLabel(hdc, 470, T,      "#P2E: ExcludeClipRect 后 DrawText");
            DrawLabel(hdc, 470, T + 18, "(红框内部不应有文字)");
        }

        DeleteObject(hCrossPen);

        // Restore
        SelectObject(hdc, oldFont);
        DeleteObject(hNormal);
        SetGraphicsMode(hdc, oldGm);
        SetBkMode(hdc, oldBkMode);
        SetTextColor(hdc, oldTxt);
    }

    // 辅助: 画一行当前点(CP)读数。强制用 TA_NOUPDATECP 输出，避免污染被测的 CP。
    void DrawCpReadout(HDC hdc, int x, int y, const char* prefix) {
        POINT pt = { -1, -1 };
        GetCurrentPositionEx(hdc, &pt);
        char buf[96];
        sprintf(buf, "%s CP=(%d,%d)", prefix, pt.x, pt.y);
        UINT a = SetTextAlign(hdc, TA_NOUPDATECP | TA_LEFT | TA_TOP);
        COLORREF c = SetTextColor(hdc, RGB(0, 0, 180));
        TextOutA(hdc, x, y, buf, (int)strlen(buf));
        SetTextColor(hdc, c);
        SetTextAlign(hdc, a);
    }

    // =========================================================================
    // DrawGroup14: SetTextAlign —— TA_UPDATECP / TA_NOUPDATECP 当前点语义
    //   Win32 语义（swinx 期望对齐）:
    //     - 默认 TA_NOUPDATECP(值 0): TextOut/ExtTextOut/TabbedTextOut 在参数 (x,y)
    //       处绘制，既不读取也不修改当前点(CP)
    //     - TA_UPDATECP: 忽略 x/y，从 CP 起画，完成后 CP 推进到文本末端；
    //       MoveToEx 可随时重设下一笔的起点；TA_RIGHT/TA_CENTER 决定文本相对 CP 的对齐
    //   每个横带一个子测试: 方框内为实际输出（含 CP 数值读数），右侧为预期说明
    // =========================================================================
    void DrawGroup14(HDC hdc, const RECT& rc) {
        SUNUSED(rc);

        COLORREF oldTxt = SetTextColor(hdc, RGB(0, 0, 0));
        int oldBkMode = SetBkMode(hdc, TRANSPARENT);
        UINT oldAlign = SetTextAlign(hdc, TA_NOUPDATECP | TA_LEFT | TA_TOP);

        LOGFONTA lfNormal = { 0 };
        lfNormal.lfHeight = 20;
        strcpy_s(lfNormal.lfFaceName, ARRAYSIZE(lfNormal.lfFaceName), "Arial");
        HFONT hNormal = CreateFontIndirectA(&lfNormal);
        HFONT oldFont = (HFONT)SelectObject(hdc, hNormal);

        // ---- A: TA_NOUPDATECP（默认）: 不读不改 CP ----
        {
            const int L = 30, T = 40, W = 420, H = 65;
            RECT rcBox = { L, T, L + W, T + H };
            HBRUSH hb = CreateSolidBrush(RGB(245, 245, 245));
            FillRect(hdc, &rcBox, hb);
            DeleteObject(hb);
            FrameRect(hdc, &rcBox, (HBRUSH)GetStockObject(BLACK_BRUSH));

            SetTextAlign(hdc, TA_NOUPDATECP | TA_LEFT | TA_BASELINE);
            MoveToEx(hdc, 100, 100, NULL);            // 设置一个已知 CP
            SetTextColor(hdc, RGB(200, 0, 0));
            TextOutA(hdc, 60, T + 50, "AAAA", 4);      // 画在 (60, T+50)
            SetTextColor(hdc, RGB(0, 150, 0));
            TextOutA(hdc, 60, T + 50, "BBBB", 4);      // 同一 (x,y)：应覆盖 AAAA
            SetTextColor(hdc, RGB(0, 0, 0));
            // CP 读数: 应保持 MoveToEx 设置的 (100,100)，两次 TextOut 均未改动
            DrawCpReadout(hdc, 300, T + 15, "A:");

            DrawLabel(hdc, 470, T,      "A: TA_NOUPDATECP(默认)");
            DrawLabel(hdc, 470, T + 18, "两次 TextOut 同一(60,y): 只见绿色BBBB");
            DrawLabel(hdc, 470, T + 36, "CP 读数应保持 (100,100) 不变");
        }

        // ---- B: TA_UPDATECP: x/y 被忽略，文本自动接续 ----
        {
            const int L = 30, T = 125, W = 420, H = 65;
            RECT rcBox = { L, T, L + W, T + H };
            HBRUSH hb = CreateSolidBrush(RGB(245, 245, 245));
            FillRect(hdc, &rcBox, hb);
            DeleteObject(hb);
            FrameRect(hdc, &rcBox, (HBRUSH)GetStockObject(BLACK_BRUSH));

            SetTextAlign(hdc, TA_UPDATECP | TA_LEFT | TA_BASELINE);
            MoveToEx(hdc, 60, T + 50, NULL);
            SetTextColor(hdc, RGB(0, 0, 200));
            TextOutA(hdc, 999, 999, "ABC", 3);   // x/y 被忽略，从 CP(60,·) 起画
            TextOutA(hdc, 999, 999, "DEF", 3);   // 自动接续
            TextOutA(hdc, 999, 999, "GHI", 3);
            SetTextColor(hdc, RGB(0, 0, 0));
            DrawCpReadout(hdc, 300, T + 15, "B:");

            DrawLabel(hdc, 470, T,      "B: TA_UPDATECP 顺序输出");
            DrawLabel(hdc, 470, T + 18, "x/y(999,999)被忽略, 文本接续 ABCDEFGHI");
            DrawLabel(hdc, 470, T + 36, "CP 读数应为文本末端 x=60+9字宽");
        }

        // ---- C: TA_UPDATECP + MoveToEx 中途重定位 ----
        {
            const int L = 30, T = 210, W = 420, H = 65;
            RECT rcBox = { L, T, L + W, T + H };
            HBRUSH hb = CreateSolidBrush(RGB(245, 245, 245));
            FillRect(hdc, &rcBox, hb);
            DeleteObject(hb);
            FrameRect(hdc, &rcBox, (HBRUSH)GetStockObject(BLACK_BRUSH));

            SetTextAlign(hdc, TA_UPDATECP | TA_LEFT | TA_BASELINE);
            MoveToEx(hdc, 60, T + 55, NULL);
            SetTextColor(hdc, RGB(0, 0, 200));
            TextOutA(hdc, 999, 999, "first-", 6);
            MoveToEx(hdc, 230, T + 25, NULL);        // 中途重设起点（换行/换列）
            TextOutA(hdc, 999, 999, "second", 6);    // 从新 CP 起画
            SetTextColor(hdc, RGB(0, 0, 0));
            DrawCpReadout(hdc, 300, T + 15, "C:");

            DrawLabel(hdc, 470, T,      "C: UPDATECP + MoveToEx 重定位");
            DrawLabel(hdc, 470, T + 18, "\"first-\" 从 (60,T+55), \"second\" 从 (230,T+25)");
            DrawLabel(hdc, 470, T + 36, "MoveToEx 设置的起点即下一笔文本起点");
        }

        // ---- D: TA_UPDATECP 与 TA_RIGHT / TA_CENTER 对齐 ----
        {
            const int L = 30, T = 295, W = 420, H = 65;
            RECT rcBox = { L, T, L + W, T + H };
            HBRUSH hb = CreateSolidBrush(RGB(245, 245, 245));
            FillRect(hdc, &rcBox, hb);
            DeleteObject(hb);
            FrameRect(hdc, &rcBox, (HBRUSH)GetStockObject(BLACK_BRUSH));

            // 竖线标记 CP 锚点
            HPEN hpn = CreatePen(PS_DOT, 1, RGB(255, 0, 255));
            HPEN op = (HPEN)SelectObject(hdc, hpn);
            MoveToEx(hdc, 420, T + 10, NULL); LineTo(hdc, 420, T + 40);
            MoveToEx(hdc, 240, T + 40, NULL); LineTo(hdc, 240, T + 60);
            SelectObject(hdc, op);
            DeleteObject(hpn);

            SetTextColor(hdc, RGB(0, 0, 200));
            SetTextAlign(hdc, TA_UPDATECP | TA_RIGHT | TA_BASELINE);
            MoveToEx(hdc, 420, T + 35, NULL);
            TextOutA(hdc, 999, 999, "RIGHT-END", 9);   // 文本向 CP 左侧延伸
            SetTextAlign(hdc, TA_UPDATECP | TA_CENTER | TA_BASELINE);
            MoveToEx(hdc, 240, T + 58, NULL);
            TextOutA(hdc, 999, 999, "CENTER", 6);       // 以 CP 为中心
            SetTextColor(hdc, RGB(0, 0, 0));

            DrawLabel(hdc, 470, T,      "D: UPDATECP + TA_RIGHT/TA_CENTER");
            DrawLabel(hdc, 470, T + 18, "RIGHT: 文本从 CP 左侧延伸到紫线");
            DrawLabel(hdc, 470, T + 36, "CENTER: 文本以紫线(CP)为中心");
        }

        // ---- E: TextOut / ExtTextOut / TabbedTextOut 在 UPDATECP 下混链 ----
        {
            const int L = 30, T = 380, W = 420, H = 65;
            RECT rcBox = { L, T, L + W, T + H };
            HBRUSH hb = CreateSolidBrush(RGB(245, 245, 245));
            FillRect(hdc, &rcBox, hb);
            DeleteObject(hb);
            FrameRect(hdc, &rcBox, (HBRUSH)GetStockObject(BLACK_BRUSH));

            SetTextAlign(hdc, TA_UPDATECP | TA_LEFT | TA_BASELINE);
            MoveToEx(hdc, 60, T + 50, NULL);
            SetTextColor(hdc, RGB(0, 0, 200));
            TextOutA(hdc, 999, 999, "Tx-", 3);                            // 接续 1
            ExtTextOutA(hdc, 999, 999, 0, NULL, "Ex-", 3, NULL);          // 接续 2
            TabbedTextOutA(hdc, 999, 999, "Tb-End", 6, 0, NULL, 0);       // 接续 3
            SetTextColor(hdc, RGB(0, 0, 0));
            DrawCpReadout(hdc, 300, T + 15, "E:");

            DrawLabel(hdc, 470, T,      "E: 三种文本 API 的 UPDATECP 混链");
            DrawLabel(hdc, 470, T + 18, "应输出 \"Tx-Ex-Tb-End\" 连续无重叠");
            DrawLabel(hdc, 470, T + 36, "TextOut/ExtTextOut/TabbedTextOut 均遵循");
        }

        // ---- F: CP 读数对照: NOUPDATECP 不动 CP vs UPDATECP 推进 CP ----
        {
            const int L = 30, T = 465, W = 420, H = 65;
            RECT rcBox = { L, T, L + W, T + H };
            HBRUSH hb = CreateSolidBrush(RGB(245, 245, 245));
            FillRect(hdc, &rcBox, hb);
            DeleteObject(hb);
            FrameRect(hdc, &rcBox, (HBRUSH)GetStockObject(BLACK_BRUSH));

            // 左半: NOUPDATECP —— TextOut 后 CP 不动
            SetTextAlign(hdc, TA_NOUPDATECP | TA_LEFT | TA_BASELINE);
            MoveToEx(hdc, 60, T + 45, NULL);
            DrawCpReadout(hdc, 40, T + 10, "N-before:");
            SetTextColor(hdc, RGB(0, 0, 200));
            TextOutA(hdc, 150, T + 45, "AA", 2);      // 显式 (150,·) 输出
            SetTextColor(hdc, RGB(0, 0, 0));
            DrawCpReadout(hdc, 40, T + 25, "N-after: ");

            // 右半: UPDATECP —— TextOut 后 CP 前进
            SetTextAlign(hdc, TA_UPDATECP | TA_LEFT | TA_BASELINE);
            MoveToEx(hdc, 260, T + 45, NULL);
            DrawCpReadout(hdc, 250, T + 10, "U-before:");
            SetTextColor(hdc, RGB(0, 0, 200));
            TextOutA(hdc, 999, 999, "BB", 2);          // 从 CP 起画并推进
            SetTextColor(hdc, RGB(0, 0, 0));
            DrawCpReadout(hdc, 250, T + 25, "U-after: ");

            DrawLabel(hdc, 470, T,      "F: CP 读数对照");
            DrawLabel(hdc, 470, T + 18, "N: before/after 应相同 (60,·)");
            DrawLabel(hdc, 470, T + 36, "U: after 应比 before 前进 2 字宽");
        }

        // Restore
        SetTextAlign(hdc, oldAlign);
        SelectObject(hdc, oldFont);
        DeleteObject(hNormal);
        SetBkMode(hdc, oldBkMode);
        SetTextColor(hdc, oldTxt);
    }

    // TransparentBlt 测试辅助：构建 32bpp top-down 源位图（alpha 全 255，
    // 键色匹配只看 RGB，与真实 GDI 的 TransparentBlt 一致）
    // pattern 0: 蓝框 + 内部品红键色芯   1: 品红/绿竖条纹   2: 品红底 + 中心红块
    HBITMAP CreateTBSource(HDC hdc, int w, int h, int pattern) {
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = w;
        bmi.bmiHeader.biHeight = -h; // top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        void *bits = NULL;
        HBITMAP hbmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
        if (!hbmp || !bits)
            return hbmp;
        int stride = ((w * 32 + 31) / 32) * 4;
        unsigned char *p = (unsigned char *)bits;
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                unsigned char *px = p + (size_t)y * stride + (size_t)x * 4;
                COLORREF c;
                switch (pattern) {
                case 0:
                    c = (x < 8 || y < 8 || x >= w - 8 || y >= h - 8) ? RGB(0, 0, 255) : RGB(255, 0, 255);
                    break;
                case 1:
                    c = ((x / 8) % 2) ? RGB(0, 200, 0) : RGB(255, 0, 255);
                    break;
                default:
                    c = (x >= w / 4 && x < w * 3 / 4 && y >= h / 4 && y < h * 3 / 4) ? RGB(255, 0, 0) : RGB(255, 0, 255);
                    break;
                }
                px[0] = GetBValue(c); px[1] = GetGValue(c); px[2] = GetRValue(c); px[3] = 0xFF;
            }
        }
        return hbmp;
    }

    // 条纹背景（4 色横条），键色透明处应透出这些颜色
    void FillStripes(HDC hdc, const RECT& rcBox) {
        COLORREF cols[4] = { RGB(170, 170, 170), RGB(255, 255, 0), RGB(0, 255, 255), RGB(255, 128, 0) };
        int hh = (rcBox.bottom - rcBox.top) / 4;
        for (int i = 0; i < 4; i++) {
            RECT rc = { rcBox.left, rcBox.top + i * hh, rcBox.right,
                        (i == 3) ? rcBox.bottom : rcBox.top + (i + 1) * hh };
            HBRUSH hb = CreateSolidBrush(cols[i]);
            FillRect(hdc, &rc, hb);
            DeleteObject(hb);
        }
    }

    // =========================================================================
    // DrawGroup15: TransparentBlt —— 颜色键透明（经典三步掩码语义）
    //   Win32 语义（swinx 期望对齐）:
    //   源区中 RGB == crTransparent 的像素不绘制（目标原有内容逐位保留），
    //   其余像素拉伸拷贝；键控在拉伸前按源位图原始像素比较（alpha 不参与）。
    // =========================================================================
    void DrawGroup15(HDC hdc, const RECT& rc) {
        SUNUSED(rc);
        const COLORREF kKey = RGB(255, 0, 255); // 品红键色

        COLORREF oldTxt = SetTextColor(hdc, RGB(0, 0, 0));
        int oldBkMode = SetBkMode(hdc, TRANSPARENT);
        UINT oldAlign = SetTextAlign(hdc, TA_NOUPDATECP | TA_LEFT | TA_TOP);

        // ---- A: 1:1 键色透明（BitBlt 不透明参考 vs TransparentBlt）----
        {
            const int L = 30, T = 40, W = 420, H = 100;
            RECT rcBox = { L, T, L + W, T + H };
            FillStripes(hdc, rcBox);
            FrameRect(hdc, &rcBox, (HBRUSH)GetStockObject(BLACK_BRUSH));

            HBITMAP hSrc = CreateTBSource(hdc, 48, 48, 0);
            HDC hmem = CreateCompatibleDC(hdc);
            HGDIOBJ oldBmp = SelectObject(hmem, hSrc);
            BitBlt(hdc, L + 40, T + 26, 48, 48, hmem, 0, 0, SRCCOPY);                 // 参考:品红芯可见
            TransparentBlt(hdc, L + 240, T + 26, 48, 48, hmem, 0, 0, 48, 48, kKey);   // 品红处透出条纹
            SelectObject(hmem, oldBmp);
            DeleteDC(hmem);
            DeleteObject(hSrc);

            DrawLabel(hdc, 470, T,      "A: TransparentBlt 1:1 键色透明");
            DrawLabel(hdc, 470, T + 18, "左(BitBlt 参考): 品红芯可见");
            DrawLabel(hdc, 470, T + 36, "右(TransparentBlt): 品红处透出条纹背景");
        }

        // ---- B: 拉伸 + 键色透明（键控发生在拉伸前）----
        {
            const int L = 30, T = 150, W = 420, H = 100;
            RECT rcBox = { L, T, L + W, T + H };
            FillStripes(hdc, rcBox);
            FrameRect(hdc, &rcBox, (HBRUSH)GetStockObject(BLACK_BRUSH));

            HBITMAP hSrc = CreateTBSource(hdc, 48, 48, 0);
            HDC hmem = CreateCompatibleDC(hdc);
            HGDIOBJ oldBmp = SelectObject(hmem, hSrc);
            TransparentBlt(hdc, L + 40, T + 26, 96, 48, hmem, 0, 0, 48, 48, kKey);    // 2x 拉伸
            SelectObject(hmem, oldBmp);
            DeleteDC(hmem);
            DeleteObject(hSrc);

            DrawLabel(hdc, 470, T,      "B: TransparentBlt 拉伸 (48x48 -> 96x48)");
            DrawLabel(hdc, 470, T + 18, "键控按源像素在拉伸前比较:");
            DrawLabel(hdc, 470, T + 36, "拉伸后品红芯仍透出条纹, 蓝框变宽");
        }

        // ---- C: 目标内容从键色条纹透出 ----
        {
            const int L = 30, T = 260, W = 420, H = 100;
            RECT rcBox = { L, T, L + W, T + H };
            HBRUSH hb = CreateSolidBrush(RGB(240, 240, 240));
            FillRect(hdc, &rcBox, hb);
            DeleteObject(hb);
            LOGFONTA lf = { 0 };
            lf.lfHeight = 40;
            lf.lfWeight = FW_BOLD;
            strcpy_s(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), "Arial");
            HFONT hFont = CreateFontIndirectA(&lf);
            HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
            SetTextColor(hdc, RGB(0, 0, 200));
            SetBkMode(hdc, TRANSPARENT);
            TextOutA(hdc, L + 30, T + 30, "BACKGROUND", 10);
            SelectObject(hdc, oldFont);
            DeleteObject(hFont);
            FrameRect(hdc, &rcBox, (HBRUSH)GetStockObject(BLACK_BRUSH));

            HBITMAP hSrc = CreateTBSource(hdc, 96, 48, 1);   // 品红/绿竖条纹
            HDC hmem = CreateCompatibleDC(hdc);
            HGDIOBJ oldBmp = SelectObject(hmem, hSrc);
            TransparentBlt(hdc, L + 30, T + 26, 96, 48, hmem, 0, 0, 96, 48, kKey);
            SelectObject(hmem, oldBmp);
            DeleteDC(hmem);
            DeleteObject(hSrc);

            DrawLabel(hdc, 470, T,      "C: 目标内容从键色条纹透出");
            DrawLabel(hdc, 470, T + 18, "品红条纹处应看到 BACKGROUND 文字");
            DrawLabel(hdc, 470, T + 36, "绿条纹处不透明");
        }

        // ---- D: GetPixel 读数验证（keyed 保留 dst / 非键色精确覆盖）----
        {
            const int L = 30, T = 370, W = 420, H = 100;
            RECT rcBox = { L, T, L + W, T + H };
            HBRUSH hb = CreateSolidBrush(RGB(200, 200, 200));
            FillRect(hdc, &rcBox, hb);
            DeleteObject(hb);
            FrameRect(hdc, &rcBox, (HBRUSH)GetStockObject(BLACK_BRUSH));

            const int BX = L + 60, BY = T + 30;
            HBITMAP hSrc = CreateTBSource(hdc, 32, 32, 2);   // 品红底 + 中心红块
            HDC hmem = CreateCompatibleDC(hdc);
            HGDIOBJ oldBmp = SelectObject(hmem, hSrc);
            TransparentBlt(hdc, BX, BY, 32, 32, hmem, 0, 0, 32, 32, kKey);
            SelectObject(hmem, oldBmp);
            DeleteDC(hmem);
            DeleteObject(hSrc);

            // 键色角点 (4,4) → 应逐位保留灰底 200,200,200；中心 (16,16) → 红块 255,0,0
            COLORREF cKey = GetPixel(hdc, BX + 4, BY + 4);
            COLORREF cRed = GetPixel(hdc, BX + 16, BY + 16);
            char buf[128];
            UINT a = SetTextAlign(hdc, TA_NOUPDATECP | TA_LEFT | TA_TOP);
            COLORREF cOld = SetTextColor(hdc, RGB(0, 0, 180));
            sprintf(buf, "keyed(%d,%d)=R%d G%d B%d [exp 200/200/200]",
                    BX + 4, BY + 4, (int)GetRValue(cKey), (int)GetGValue(cKey), (int)GetBValue(cKey));
            TextOutA(hdc, L + 10, T + 8, buf, (int)strlen(buf));
            sprintf(buf, "red  (%d,%d)=R%d G%d B%d [exp 255/0/0]",
                    BX + 16, BY + 16, (int)GetRValue(cRed), (int)GetGValue(cRed), (int)GetBValue(cRed));
            TextOutA(hdc, L + 10, T + 86, buf, (int)strlen(buf));
            SetTextColor(hdc, cOld);
            SetTextAlign(hdc, a);

            DrawLabel(hdc, 470, T,      "D: GetPixel 读数验证");
            DrawLabel(hdc, 470, T + 18, "keyed 角点应保留灰底 (200,200,200)");
            DrawLabel(hdc, 470, T + 36, "中心应精确为红块 (255,0,0)");
        }

        // Restore
        SetTextAlign(hdc, oldAlign);
        SetBkMode(hdc, oldBkMode);
        SetTextColor(hdc, oldTxt);
    }

    void OnPaint(HDC hdc) {
        PAINTSTRUCT ps;
        hdc = BeginPaint(m_hWnd, &ps);

        CRect rc;
        GetClientRect(&rc);

        // Fill white background
        FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

        // Draw header
        DrawHeader(hdc, rc);

        // Draw current group
        switch (m_groupIndex) {
        case 0: DrawGroup1(hdc, rc); break;  // Basic shapes
        case 1: DrawGroup2(hdc, rc); break;  // Curves
        case 2: DrawGroup3(hdc, rc); break;  // Pen/Brush
        case 3: DrawGroup4(hdc, rc); break;  // Bitmap ops
        case 4: DrawGroup5(hdc, rc); break;  // Text
        case 5: DrawGroup6(hdc, rc); break;  // Path
        case 6: DrawGroup7(hdc, rc); break;  // Region/Clip
        case 7: DrawGroup8(hdc, rc); break;  // Rect/DCState
        case 8: DrawGroup9(hdc, rc); break;  // Transform/ROP2
        case 9: DrawGroup10(hdc, rc); break; // ExtCreatePen/Misc
        case 10: DrawGroup11(hdc, rc); break; // CreateCompatibleDC + SetViewportOrgEx
        case 11: DrawGroup12(hdc, rc); break; // render-gdi 回归: 装饰线/文本+矩阵/SetMatrix旋转
        case 12: DrawGroup13(hdc, rc); break; // render-gdi 回归2: Clip+Viewport+DrawText 交互 (重新排版)
        case 13: DrawGroup14(hdc, rc); break; // SetTextAlign: TA_UPDATECP/TA_NOUPDATECP 当前点语义
        case 14: DrawGroup15(hdc, rc); break; // TransparentBlt: 颜色键透明（掩码合成语义）
        default:
            DrawLabel(hdc, 30, 50, "Invalid group index!");
            break;
        }

        EndPaint(m_hWnd, &ps);
    }

    BEGIN_MSG_MAP_EX(SGdiTestWnd)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_KEYDOWN(OnKeyDown)
        CHAIN_MSG_MAP(SNativeWnd)
    END_MSG_MAP()
};

int run_window() {
    int ret = 0;
    SNativeWndHelper::instance()->Init(0, _T("soui_host"), FALSE);
    SGdiTestWnd wnd;
    SOUI::SStringT iconPath = getSourceDir() + _T("/fun_test/uires/image/soui.ico");
    HICON hIcon = (HICON)LoadImage(0, iconPath.c_str(), IMAGE_ICON, 128, 128, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    HWND hWnd = wnd.CreateNative(_T("GDI API Test - swinx/cairo"),
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME, WS_EX_APPWINDOW, 0, 0, 0, 0, 0);
    if (hWnd) {
        wnd.SendMessage(WM_SETICON, 1, (LPARAM)hIcon);
        wnd.SetWindowPos(0, 0, 0, 800, 690, SWP_NOMOVE | SWP_NOZORDER);
        wnd.ShowWindow(SW_SHOW);
        SMessageLoop loop(NULL);
        ret = loop.Run();
        wnd.DestroyWindow();
    }
    DestroyIcon(hIcon);
    SNativeWndHelper::instance()->Uninit();
    return ret;
}


TEST(window,gdi){
    EXPECT_EQ(run_window(), 1);
}