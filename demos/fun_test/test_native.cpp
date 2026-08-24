#include <souistd.h>
#include "common.h"
#include <math.h>

using namespace SOUI;
#define kLogTag "test_native"

static const TCHAR* kPath_TestPng = _T("/uires/image/soui.png");
static const TCHAR* kPath_YyPng   = _T("/uires/image/yy.png");
static const TCHAR* kPath_Icon    = _T("/uires/image/soui.ico");
#define GDI_GROUP_COUNT 13

class SGdiTestWnd : public SNativeWnd {
    int m_groupIndex;
    HBITMAP m_testBmp;
    HBITMAP m_yyBmp;

public:
    SGdiTestWnd() : m_groupIndex(12), m_testBmp(0), m_yyBmp(0) {}
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
        TextOutA(hdc, x, y, label.c_str(), label.GetLength());
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
        TextOutA(memDc, gx, gy + 180,
            "Text from memDC: viewport(-100,-100) draw@(200,0)", -1);

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
        TextOutA(memDc, 5, 5, "memDC native origin (0,0) corner", -1);

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

            DrawLabel(hdc, bx,       by + sz + 3, "SRCCOPY (yellow bg)");
            DrawLabel(hdc, bx+sz+10, by + sz + 3, "SRCAND");
            DrawLabel(hdc, bx+sz*2+20, by + sz + 3, "SRCPAINT");
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
    // DrawGroup13: Clip + ViewportOrg + DrawText 交互
    // 直接模拟 SWindow::GetRenderTarget 的调用序列（page2 文本不显示就是由此组合触发）
    //   调用序列:
    //     1. CreateRenderTarget(w, h)  -> 对应已有 memdc
    //     2. PushClipRegion(rcRT)      -> SaveDC + ExtSelectClipRgn(RGN_COPY)
    //     3. OffsetViewportOrg(-rcRT.left, -rcRT.top)
    //     4. ClearRect(&rcRT)
    //     5. 控件绘制: DrawText(hdc, &rcInnerText, ...) 其中 rcInnerText 在 viewport 平移后的坐标系
    //     6. PopClip                   -> RestoreDC(-1)
    // =========================================================================
    void DrawGroup13(HDC hdc, const RECT& rc) {
        SUNUSED(rc);

        const int x0 = 30;
        const int y0 = 40;

        COLORREF oldTxt = SetTextColor(hdc, RGB(0, 0, 0));
        int oldBkMode = SetBkMode(hdc, TRANSPARENT);
        int oldGm = SetGraphicsMode(hdc, GM_ADVANCED);

        LOGFONTA lfNormal = { 0 };
        lfNormal.lfHeight = 20;
        strcpy_s(lfNormal.lfFaceName, ARRAYSIZE(lfNormal.lfFaceName), "Arial");
        HFONT hNormal = CreateFontIndirectA(&lfNormal);
        HFONT oldFont = (HFONT)SelectObject(hdc, hNormal);

        // 先在白色背景上画：每个子测试先 FillRect 一块浅底色，再在上面模拟 GetRenderTarget 序列，
        // 然后 FrameRect 画外框，能一眼看出文字应该在框内的什么位置。

        // ---- D1-D4: 诊断小组 ----
        // 先在 page2 区域上面一行放 4 个并排 150x60 小方框做隔离性测试，
        // 每个下面打印一行文字直接告诉你结论，便于肉眼判断
        {
            const int y = 10;
            const int H = 60;
            const int W = 170;
            const int gap = 20;
            int x = 30;

            // --- D1: 纯 OffsetViewportOrg + DrawText（无任何 clip）---
            // 验证: OffsetViewportOrg(-40,-10) 后，画 DrawText(rc=(0,0,W,H)) 应出现在 client (40,10)~(210,70)
            {
                RECT rcBox = { x, y, x + W, y + H };
                HBRUSH hb = CreateSolidBrush(RGB(255, 240, 240));
                FillRect(hdc, &rcBox, hb);
                DeleteObject(hb);
                FrameRect(hdc, &rcBox, (HBRUSH)GetStockObject(BLACK_BRUSH));

                int s = ::SaveDC(hdc);
                ::SetGraphicsMode(hdc, GM_ADVANCED);
                ::OffsetViewportOrgEx(hdc, -x, -y, NULL);

                RECT rt = { 0, 0, W, H };
                SetTextColor(hdc, RGB(180, 0, 0));
                DrawTextA(hdc, "D1 ONLY-OFFSET", -1, &rt, DT_LEFT | DT_TOP | DT_SINGLELINE);
                SetTextColor(hdc, RGB(0, 0, 0));

                // 十字标记在偏移后 (0,0)
                HPEN hpn = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
                HPEN op = (HPEN)SelectObject(hdc, hpn);
                MoveToEx(hdc, 0, 0, NULL); LineTo(hdc, 30, 0);
                MoveToEx(hdc, 0, 0, NULL); LineTo(hdc, 0, 30);
                SelectObject(hdc, op); DeleteObject(hpn);

                ::RestoreDC(hdc, -1);
                SUNUSED(s);
            }
            DrawLabel(hdc, x, y + H + 2, "D1: 只做 OffsetViewport，无 clip");
            DrawLabel(hdc, x, y + H + 20,  "预期: 红字+蓝十字在框内左上角");
            x += W + gap;

            // --- D2: 先 OffsetViewport 后 PushClip（真实 SOUI 顺序）+ DrawText ---
            {
                RECT rcBox = { x, y, x + W, y + H };
                HBRUSH hb = CreateSolidBrush(RGB(240, 255, 240));
                FillRect(hdc, &rcBox, hb);
                DeleteObject(hb);
                FrameRect(hdc, &rcBox, (HBRUSH)GetStockObject(BLACK_BRUSH));

                int s = ::SaveDC(hdc);
                ::SetGraphicsMode(hdc, GM_ADVANCED);
                // 顺序 1: OffsetViewport first
                ::OffsetViewportOrgEx(hdc, -x, -y, NULL);
                // 顺序 2: Clip RGN_COPY (rcBox in client coords)
                HRGN hrgn = ::CreateRectRgnIndirect(&rcBox);
                ::ExtSelectClipRgn(hdc, hrgn, RGN_COPY);
                ::DeleteObject(hrgn);

                RECT rt = { 0, 0, W, H };
                SetTextColor(hdc, RGB(0, 128, 0));
                DrawTextA(hdc, "D2 OFFSET-THEN-CLIP", -1, &rt, DT_LEFT | DT_TOP | DT_SINGLELINE);
                SetTextColor(hdc, RGB(0, 0, 0));

                HPEN hpn = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
                HPEN op = (HPEN)SelectObject(hdc, hpn);
                MoveToEx(hdc, 0, 0, NULL); LineTo(hdc, 30, 0);
                MoveToEx(hdc, 0, 0, NULL); LineTo(hdc, 0, 30);
                SelectObject(hdc, op); DeleteObject(hpn);

                ::RestoreDC(hdc, -1);
            }
            DrawLabel(hdc, x, y + H + 2,  "D2: Offset THEN Clip (真·SOUI 顺序)");
            DrawLabel(hdc, x, y + H + 20, "预期: 绿字+红十在框内左上角");
            x += W + gap;

            // --- D3: 先 PushClip 后 OffsetViewport（P2A 旧顺序）+ DrawText ---
            {
                RECT rcBox = { x, y, x + W, y + H };
                HBRUSH hb = CreateSolidBrush(RGB(240, 240, 255));
                FillRect(hdc, &rcBox, hb);
                DeleteObject(hb);
                FrameRect(hdc, &rcBox, (HBRUSH)GetStockObject(BLACK_BRUSH));

                int s = ::SaveDC(hdc);
                ::SetGraphicsMode(hdc, GM_ADVANCED);
                // 顺序 1: Clip first
                HRGN hrgn = ::CreateRectRgnIndirect(&rcBox);
                ::ExtSelectClipRgn(hdc, hrgn, RGN_COPY);
                ::DeleteObject(hrgn);
                // 顺序 2: OffsetViewport
                ::OffsetViewportOrgEx(hdc, -x, -y, NULL);

                RECT rt = { 0, 0, W, H };
                SetTextColor(hdc, RGB(0, 0, 180));
                DrawTextA(hdc, "D3 CLIP-THEN-OFFSET", -1, &rt, DT_LEFT | DT_TOP | DT_SINGLELINE);
                SetTextColor(hdc, RGB(0, 0, 0));

                HPEN hpn = CreatePen(PS_SOLID, 1, RGB(255, 128, 0));
                HPEN op = (HPEN)SelectObject(hdc, hpn);
                MoveToEx(hdc, 0, 0, NULL); LineTo(hdc, 30, 0);
                MoveToEx(hdc, 0, 0, NULL); LineTo(hdc, 0, 30);
                SelectObject(hdc, op); DeleteObject(hpn);

                ::RestoreDC(hdc, -1);
            }
            DrawLabel(hdc, x, y + H + 2,  "D3: Clip THEN Offset (旧 P2A 顺序)");
            DrawLabel(hdc, x, y + H + 20, "预期: 蓝字+橙十字在框内左上角");
            x += W + gap;

            // --- D4: 纯 PushClip + DrawText（都用 client 坐标，无 OffsetViewport）---
            // 验证 clip rect 方向是否正确（无任何 viewport 干扰）
            {
                RECT rcBox = { x, y, x + W, y + H };
                HBRUSH hb = CreateSolidBrush(RGB(255, 250, 240));
                FillRect(hdc, &rcBox, hb);
                DeleteObject(hb);
                FrameRect(hdc, &rcBox, (HBRUSH)GetStockObject(BLACK_BRUSH));

                int s = ::SaveDC(hdc);
                ::SetGraphicsMode(hdc, GM_ADVANCED);
                HRGN hrgn = ::CreateRectRgnIndirect(&rcBox);
                ::ExtSelectClipRgn(hdc, hrgn, RGN_COPY);
                ::DeleteObject(hrgn);

                RECT rt = rcBox;
                SetTextColor(hdc, RGB(160, 0, 160));
                DrawTextA(hdc, "D4 ONLY-CLIP no-offset", -1, &rt, DT_LEFT | DT_TOP | DT_SINGLELINE);
                SetTextColor(hdc, RGB(0, 0, 0));

                HPEN hpn = CreatePen(PS_SOLID, 1, RGB(0, 128, 128));
                HPEN op = (HPEN)SelectObject(hdc, hpn);
                MoveToEx(hdc, x, y, NULL); LineTo(hdc, x + 30, y);
                MoveToEx(hdc, x, y, NULL); LineTo(hdc, x, y + 30);
                SelectObject(hdc, op); DeleteObject(hpn);

                ::RestoreDC(hdc, -1);
            }
            DrawLabel(hdc, x, y + H + 2,  "D4: 只做 Clip，无 OffsetViewport");
            DrawLabel(hdc, x, y + H + 20, "预期: 紫字+青十字在框内左上角");
        }

        // ---- P2A: 最小场景：PushClipRect(rcRT) + OffsetViewportOrg(-left,-top) + DrawText 单行 ----
        // 等效于一个简单文本控件的 GetRenderTarget + DrawText
        {
            const char* label = "#P2A: PushClipRect + OffsetViewportOrg(-100,-80) + DrawText";
            const int left = x0, top = y0;
            const int W = 400, H = 60;
            RECT rcRT = { left + 100, top + 80, left + 100 + W, top + 80 + H }; // 裁剪在 (130,120)~(530,180)

            HBRUSH hb = CreateSolidBrush(RGB(240, 250, 240));
            FillRect(hdc, &rcRT, hb);
            DeleteObject(hb);
            FrameRect(hdc, &rcRT, (HBRUSH)GetStockObject(BLACK_BRUSH));

            // --- 开始模拟 SWindow::GetRenderTarget(GRT_PAINTBKGND, rcRT) ---
            ::SaveDC(hdc);                                                 // PushClipRect 内部 SaveDC
            HRGN hrgn = ::CreateRectRgnIndirect(&rcRT);
            ::ExtSelectClipRgn(hdc, hrgn, RGN_COPY);                      // PushClipRect: RGN_COPY 裁剪
            ::DeleteObject(hrgn);
            ::OffsetViewportOrgEx(hdc, -rcRT.left, -rcRT.top, NULL);      // OffsetViewportOrg(-left,-top)

            // --- 此时 viewport 原点 = (-rcRT.left, -rcRT.top)，坐标 (0,0) 对应 client (rcRT.left, rcRT.top)
            RECT rcText = { 0, 0, W, H };  // 在偏移后的坐标系，文本从 (0,0) 画到 (W,H)
            SetTextColor(hdc, RGB(0, 128, 0));
            // 用 DT_LEFT|DT_VCENTER|DT_SINGLELINE 绘制单行（与 SStatic::OnDrawLine 一致）
            DrawTextA(hdc, "hello soui in page2", -1, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            SetTextColor(hdc, RGB(0, 0, 0));

            // 画一条线表示 (0,0) 在偏移后的坐标系应该就在 rcRT 的左上角
            HPEN hpen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
            HPEN oldpen = (HPEN)::SelectObject(hdc, hpen);
            ::MoveToEx(hdc, 0, 0, NULL); ::LineTo(hdc, 40, 0);   // 水平红线
            ::MoveToEx(hdc, 0, 0, NULL); ::LineTo(hdc, 0, 40);   // 垂直红线
            ::SelectObject(hdc, oldpen);
            DeleteObject(hpen);

            // PopClip
            ::RestoreDC(hdc, -1);
            // --- 模拟结束 ---

            DrawLabel(hdc, left + W + 20, top + 80, label);
            DrawLabel(hdc, left + W + 20, top + 98, "绿字应在绿色方框内(靠近左上角)");
            DrawLabel(hdc, left + W + 20, top + 116, "红线十字应出现在方框左上角");
        }

        // ---- P2B: 与 P2A 相同，但 rcRT 位置不做偏移，检查纯 ClipRect（无 OffsetViewport） ----
        {
            const int left = x0, top = y0 + 90;
            const int W = 400, H = 60;
            RECT rcRT = { left + 100, top + 80, left + 100 + W, top + 80 + H };

            HBRUSH hb = CreateSolidBrush(RGB(250, 245, 230));
            FillRect(hdc, &rcRT, hb);
            DeleteObject(hb);
            FrameRect(hdc, &rcRT, (HBRUSH)GetStockObject(BLACK_BRUSH));

            ::SaveDC(hdc);
            HRGN hrgn = ::CreateRectRgnIndirect(&rcRT);
            ::ExtSelectClipRgn(hdc, hrgn, RGN_COPY);
            ::DeleteObject(hrgn);
            // 注意：没有 OffsetViewportOrg！

            RECT rcText = rcRT;  // 直接用 client 坐标
            SetTextColor(hdc, RGB(180, 80, 0));
            DrawTextA(hdc, "#P2B: PushClipRect 仅裁剪 不偏移Viewport", -1, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            SetTextColor(hdc, RGB(0, 0, 0));

            ::RestoreDC(hdc, -1);

            DrawLabel(hdc, left + W + 20, top + 80, "#P2B: PushClipRect 无 OffsetViewportOrg");
            DrawLabel(hdc, left + W + 20, top + 98, "橙字应在橙色方框内");
        }

        // ---- P2C: 嵌套 PushClipRect / PopClip (模拟子控件在父控件 RT 内绘制) ----
        //   外层: OffsetViewportOrg(-50,-300) + 大裁剪
        //   内层: 子控件再次 PushClipRect(一个子矩形) + DrawText + PopClip
        //   验证内层 PopClip 用 RestoreDC(-1) 后不会把外层 viewport 也回滚
        {
            const int outerL = x0 + 100, outerT = y0 + 220;
            const int outerW = 500, outerH = 180;
            RECT rcOuterRT = { outerL, outerT, outerL + outerW, outerT + outerH };

            HBRUSH hbOut = CreateSolidBrush(RGB(235, 245, 255));
            FillRect(hdc, &rcOuterRT, hbOut);
            DeleteObject(hbOut);
            FrameRect(hdc, &rcOuterRT, (HBRUSH)GetStockObject(BLACK_BRUSH));

            // --- 外层：模拟父控件 GetRenderTarget ---
            int sOuter = ::SaveDC(hdc);
            HRGN hOuterRgn = ::CreateRectRgnIndirect(&rcOuterRT);
            ::ExtSelectClipRgn(hdc, hOuterRgn, RGN_COPY);
            ::DeleteObject(hOuterRgn);
            ::OffsetViewportOrgEx(hdc, -rcOuterRT.left, -rcOuterRT.top, NULL);

            // 外层 RT 坐标系原点：(0,0)=outerL,outerT。先画一个外层标记文字
            SetTextColor(hdc, RGB(0, 80, 160));
            RECT rcOuterLabel = { 10, 8, outerW - 10, 30 };
            DrawTextA(hdc, "OUTER RT (OffsetViewport applied)", -1, &rcOuterLabel, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

            // --- 内层：模拟子控件 GetRenderTarget 在父 RT 里画一个子区域 ---
            // 子控件在父 RT 坐标系下占 (40,60)~(40+W,60+H) 位置
            const int subX = 40, subY = 60, subW = 400, subH = 60;
            RECT rcSubClipInOuter = { subX, subY, subX + subW, subY + subH };
            // 子控件的裁剪应该是相对 client（不是外层偏移后），所以在真实 client 坐标为:
            RECT rcSubClipClient = { outerL + subX, outerT + subY, outerL + subX + subW, outerT + subY + subH };

            HBRUSH hbIn = CreateSolidBrush(RGB(255, 230, 230));
            // 注意 FillRect 必须用 client 坐标（此时已应用 OffsetViewport，会再叠加一次！要小心）
            // 正确做法：先暂时 ModifyViewportOrg 回去再画底，或者就直接画方框参考（我们已经 Fill 外框了）
            // 这里跳过内层 fill，直接 FrameRect 用子 RT 坐标系
            ::Rectangle(hdc, subX, subY, subX + subW, subY + subH); // 在 offset 后坐标系画
            DeleteObject(hbIn);

            int sInner = ::SaveDC(hdc);
            HRGN hInnerRgn = ::CreateRectRgnIndirect(&rcSubClipClient);  // client 坐标下的子裁剪
            ::ExtSelectClipRgn(hdc, hInnerRgn, RGN_AND);
            ::DeleteObject(hInnerRgn);
            ::OffsetViewportOrgEx(hdc, -subX, -subY, NULL);  // 再次对子 RT 做偏移

            // 现在：子 RT 坐标 (0,0) = 父 RT 坐标 (subX, subY) = client (outerL+subX, outerT+subY)
            SetTextColor(hdc, RGB(180, 0, 0));
            RECT rcSubText = { 0, 0, subW, subH };
            DrawTextA(hdc, "INNER RT: hello soui in page2 (嵌套 RT 文本)", -1, &rcSubText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            SetTextColor(hdc, RGB(0, 0, 0));

            // 内层 PopClip = RestoreDC(-1)：恢复到内层 saveDC 之前的状态
            ::RestoreDC(hdc, -1);

            // 内层恢复后，在外层 RT 中继续画文字，验证外层 viewport 和 clip 是否还保持正确
            RECT rcOuterCheck = { 10, 140, outerW - 10, 170 };
            SetTextColor(hdc, RGB(0, 128, 0));
            DrawTextA(hdc, "OUTER RT after inner PopClip: 此处文字仍应可见（绿色）", -1, &rcOuterCheck, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            SetTextColor(hdc, RGB(0, 0, 0));

            ::RestoreDC(hdc, sOuter);

            DrawLabel(hdc, outerL + outerW + 20, outerT + 0, "#P2C: 嵌套 PushClip + 两次 OffsetViewport");
            DrawLabel(hdc, outerL + outerW + 20, outerT + 18, "(模拟子控件在父控件 RT 中绘制)");
            DrawLabel(hdc, outerL + outerW + 20, outerT + 36, "蓝字: outer RT顶部");
            DrawLabel(hdc, outerL + outerW + 20, outerT + 54, "红字: inner RT(P2C 的 hello soui)");
            DrawLabel(hdc, outerL + outerW + 20, outerT + 72, "绿字: inner PopClip 后 outer RT 底部");
        }

        // ---- P2D: GetClipBox 行为验证 (检查 clip 盒坐标是否对得上) ----
        // 依次 ExtSelectClipRgn(rect) 后调用 GetClipBox，看返回 rect 是否和输入一致
        {
            const int top = y0 + 430;
            const int left = x0;
            RECT rects[3] = {
                { left + 20, top + 10, left + 220, top + 50 },
                { left + 260, top + 10, left + 460, top + 50 },
                { left + 500, top + 10, left + 700, top + 50 },
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
                // 直接 DrawText 到 rcBox 内部，如果 GetClipBox 返回值包含区域则文字可见
                SetTextColor(hdc, RGB(80, 0, 128));
                DrawTextA(hdc, buf, -1, &rcBox, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                SetTextColor(hdc, RGB(0, 0, 0));
                ::RestoreDC(hdc, -1);
            }
            DrawLabel(hdc, left + 20, top + 60, "#P2D: GetClipBox 返回值，应与灰色方框坐标一致且文字可见");
        }

        // ---- P2E: IntersectClipRect / ExcludeClipRect 之后的 DrawText ----
        {
            const int top = y0 + 520;
            const int left = x0 + 20;
            RECT rBig = { left, top, left + 480, top + 70 };

            HBRUSH hb = CreateSolidBrush(RGB(245, 245, 245));
            FillRect(hdc, &rBig, hb);
            DeleteObject(hb);
            FrameRect(hdc, &rBig, (HBRUSH)GetStockObject(BLACK_BRUSH));

            // 设置整个大矩形为 clip
            int s0 = ::SaveDC(hdc);
            HRGN hBig = ::CreateRectRgnIndirect(&rBig);
            ::ExtSelectClipRgn(hdc, hBig, RGN_COPY);
            ::DeleteObject(hBig);

            // 排除中间一块
            RECT rExcl = { left + 160, top + 10, left + 320, top + 60 };
            HPEN hRedPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
            HPEN oldRP = (HPEN)SelectObject(hdc, hRedPen);
            HBRUSH oldRB = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Rectangle(hdc, rExcl.left, rExcl.top, rExcl.right, rExcl.bottom);
            SelectObject(hdc, oldRB);
            SelectObject(hdc, oldRP);
            DeleteObject(hRedPen);
            ::ExcludeClipRect(hdc, rExcl.left, rExcl.top, rExcl.right, rExcl.bottom);

            SetTextColor(hdc, RGB(0, 0, 180));
            DrawTextA(hdc, "#P2E: ExcludeClipRect 中间红框后 文字应在两侧可见 中间被红框挡住", -1, &rBig,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            SetTextColor(hdc, RGB(0, 0, 0));

            ::RestoreDC(hdc, s0);

            DrawLabel(hdc, left + 500, top + 10, "#P2E: ExcludeClipRect 后 DrawText");
            DrawLabel(hdc, left + 500, top + 28, "(红框内部不应有文字)");
        }

        // Restore
        SelectObject(hdc, oldFont);
        DeleteObject(hNormal);
        SetGraphicsMode(hdc, oldGm);
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
        case 12: DrawGroup13(hdc, rc); break; // render-gdi 回归2: Clip+Viewport+DrawText 交互 (page2 文本不显示)
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
        wnd.SetWindowPos(0, 0, 0, 800, 640, SWP_NOMOVE | SWP_NOZORDER);
        wnd.ShowWindow(SW_SHOW);
        SMessageLoop loop(NULL);
        ret = loop.Run();
        wnd.DestroyWindow();
    }
    DestroyIcon(hIcon);
    SNativeWndHelper::instance()->Uninit();
    return ret;
}
