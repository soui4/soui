#include <souistd.h>
#include "common.h"
#include "ScintillaWnd.h"
#include <math.h>

using namespace SOUI;
#define kLogTag "test_native"

static const TCHAR* kPath_TestPng = _T("/uires/image/soui.png");
static const TCHAR* kPath_TestXml = _T("/uires/xml/dlg_main.xml");
static const TCHAR* kPath_TestYyLogo = _T("/uires/image/yy.png");

class SChildWnd : public SNativeWnd {
public:
    SChildWnd() {}
    ~SChildWnd() {}

protected:
    int OnCreate(LPCREATESTRUCT lpCs) {
        SCROLLINFO si;
        si.fMask = SIF_ALL;
        si.nMin = 0;
        si.nMax = 100;
        si.nPos = 20;
        si.nPage = 10;
        si.nTrackPos = -1;
        SetScrollInfo(m_hWnd, SB_VERT, &si, TRUE);
        ShowScrollBar(m_hWnd, SB_VERT, TRUE);

        SetScrollInfo(m_hWnd, SB_HORZ, &si, TRUE);
        ShowScrollBar(m_hWnd, SB_HORZ, TRUE);
        return 0;
    }

    void OnPaint(HDC hdc) {
        PAINTSTRUCT ps;
        hdc = BeginPaint(m_hWnd, &ps);

        CRect rc;
        GetClientRect(&rc);
        HBRUSH br = CreateSolidBrush(RGB(128, 128, 128));
        FillRect(hdc, &rc, br);
        DeleteObject(br);
        CPoint pt;
        {
            SCROLLINFO si = { 0 };
            si.fMask = SIF_POS | SIF_TRACKPOS;
            GetScrollInfo(m_hWnd, SB_VERT, &si);
            if (si.nTrackPos != -1)
                pt.y = si.nTrackPos;
            else
                pt.y = si.nPos;
        }
        {
            SCROLLINFO si = { 0 };
            si.fMask = SIF_POS | SIF_TRACKPOS;
            GetScrollInfo(m_hWnd, SB_HORZ, &si);
            if (si.nTrackPos != -1)
                pt.x = si.nTrackPos;
            else
                pt.x = si.nPos;
        }
        SStringT str = SStringT().Format(_T("pos:(%d,%d)"), pt.x, pt.y);
        DrawText(hdc, str, str.GetLength(), &rc, DT_SINGLELINE | DT_VCENTER);

        EndPaint(m_hWnd, &ps);
    }

    void OnHScroll(int code, int pos, HWND hWnd) {
        OnScroll(FALSE, code, pos);
    }
    void OnVScroll(int code, int pos, HWND hWnd) {
        OnScroll(TRUE, code, pos);
    }
    void OnScroll(BOOL bVert, int code, int pos) {
        SCROLLINFO si;
        si.fMask = SIF_POS | SIF_PAGE;
        GetScrollInfo(m_hWnd, bVert ? SB_VERT : SB_HORZ, &si);
        switch (code) {
        case SB_LINEUP:pos = si.nPos - 1; break;
        case SB_PAGELEFT: pos = si.nPos - si.nPage; break;
        case SB_PAGERIGHT: pos = si.nPos + si.nPage; break;
        case SB_LINEDOWN:pos = si.nPos + 1; break;
        default:pos = si.nPos; break;
        }
        printf("onScroll,code=%d,pos=%d\n", code, pos);
        if (code != SB_ENDSCROLL)
            SetScrollPos(m_hWnd, bVert ? SB_VERT : SB_HORZ, pos, TRUE);
        Invalidate();
    }
    BEGIN_MSG_MAP_EX(SChildWnd)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_HSCROLL(OnHScroll)
        MSG_WM_VSCROLL(OnVScroll)
        CHAIN_MSG_MAP(SNativeWnd)
        END_MSG_MAP()
};

class SNativeWnd2 : public SNativeWnd {
    HBITMAP m_bmp;
    HBITMAP m_yylogo;
    CScintillaWnd m_pChild;
public:
    SNativeWnd2() :m_bmp(0), m_yylogo(0) {};

protected:
    void OnClose();
    void OnPaint(HDC hdc);
    void OnSize(UINT state, SIZE sz);
    void OnDestroy();
    int  OnCreate(LPCREATESTRUCT lpCs);

    void OnLButtonUp(int flag, CPoint pt) {
        /*if (pt.x < 50 && pt.y < 50) {
            SetTimer(100, 10);
        }*/
        SetFocus();
    }
    void OnTimer(UINT_PTR id) {
        if (id == 100) {
            static int nCount = 0;
            if (++nCount > 150) {
                KillTimer(id);
            }
            CRect rc;
            GetClientRect(&rc);
            SetWindowPos(0, 0, 0, rc.Width() + 1, rc.Height(), SWP_NOZORDER | SWP_NOMOVE);
        }

        SetMsgHandled(FALSE);
    }

    void OnSetFocus(HWND hWnd);
    void OnKillFocus(HWND hWnd);

    BEGIN_MSG_MAP_EX(SNativeWnd2)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_SIZE(OnSize)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_SETFOCUS(OnSetFocus)
        MSG_WM_KILLFOCUS(OnKillFocus)
        MSG_WM_TIMER(OnTimer)
        CHAIN_MSG_MAP(SNativeWnd)
        END_MSG_MAP()
};

void SNativeWnd2::OnDestroy() {
    if (m_bmp) {
        DeleteObject(m_bmp);
        m_bmp = 0;
    }
    if (m_yylogo) {
        DeleteObject(m_yylogo);
        m_yylogo = 0;
    }
    SetMsgHandled(FALSE);
}

int  SNativeWnd2::OnCreate(LPCREATESTRUCT lpCs) {
    SOUI::SStringT srcDir = getSourceDir();
    m_bmp = LoadPng((srcDir + kPath_TestPng).c_str());
    if (!m_bmp)
        return 1;
    m_yylogo = LoadPng((srcDir + kPath_TestYyLogo).c_str());
    if (!m_yylogo)
        return 2;
    SetMsgHandled(FALSE);

    m_pChild.Create(_T("sci window"), WS_CHILD|WS_VISIBLE|WS_BORDER, CRect(0, 0, 100, 100), m_hWnd, 100,0);
    HWND hList = ::CreateWindow(_T("ListBox"), NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER, 300, 10, 130, 200, m_hWnd, 0, 0, 0);
    if (hList) {
        for (int i = 0; i < 30; i++) {
            SStringA str = SStringA().Format("list item %d", i);
            ::SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)str.c_str());
        }
        {
            int nLen = ::SendMessageW(hList, LB_GETTEXTLEN, 3, 0);
            wchar_t* buf = new wchar_t[nLen + 1];
            ::SendMessageW(hList, LB_GETTEXT, 3, (LPARAM)buf);
            delete[]buf;
        }
        {
            int nLen = ::SendMessageA(hList, LB_GETTEXTLEN, 3, 0);
            char* buf = new char[nLen + 1];
            ::SendMessageA(hList, LB_GETTEXT, 3, (LPARAM)buf);
            delete[]buf;
        }
    }
    return 0;
}

void SNativeWnd2::OnSetFocus(HWND hWnd)
{
    CRect rc;
    GetClientRect(&rc);
    int x = rand() % (rc.Width()-5);
    int y = rand() % (rc.Height() - 20);

    CreateCaret(NULL, 2, 20);
    SetCaretPos(10, 10);
    ShowCaret();
}

void SNativeWnd2::OnKillFocus(HWND hWnd)
{
    HideCaret();
    DestroyCaret();
}

void SNativeWnd2::OnSize(UINT state, SIZE sz) {
    SetMsgHandled(FALSE);
    CRect rc;
    GetClientRect(&rc);
    rc.left = rc.right - 200;
    rc.top = rc.bottom - 100;
    m_pChild.MoveWindow(rc.left, rc.top, rc.Width(), rc.Height());
}

void SNativeWnd2::OnPaint(HDC hdc) {
    RECT rcWnd;
    GetClientRect(&rcWnd);
    PAINTSTRUCT ps;
    hdc = BeginPaint(m_hWnd, &ps);
    FillRect(hdc, &rcWnd, (HBRUSH)GetStockObject(WHITE_BRUSH));
    if (1) {
        const char* test = "te函数st";
        const wchar_t* test2 = L"test";
        int fit;
        int dx[20];
        SIZE size;
        GetTextExtentExPointA(hdc, test, strlen(test), 10000, &fit, dx, &size);
        assert(fit == strlen(test));
        GetTextExtentExPointW(hdc, test2, wcslen(test2), 10000, &fit, dx, &size);
        assert(fit == wcslen(test2));
        fit = 0;
    }
    if (1) {
        HDC memdc = CreateCompatibleDC(hdc);
        HBITMAP bmp = CreateCompatibleBitmap(hdc, rcWnd.right, rcWnd.bottom);
        HGDIOBJ oldBmp = SelectObject(memdc, bmp);
        FillRect(memdc, &rcWnd, (HBRUSH)GetStockObject(WHITE_BRUSH));
        HBRUSH hbr = CreateSolidBrush(RGB(255, 0, 0));
        int save = SaveDC(memdc);
        RestoreDC(memdc, save);

        save = SaveDC(memdc);
        HRGN hrgn = CreateRectRgn(0, 0, 100, 100);
        HRGN hrgn2 = CreateRectRgn(100, 100, 500, 500);
        CombineRgn(hrgn, hrgn, hrgn2, RGN_OR);
        ExtSelectClipRgn(memdc, hrgn, RGN_COPY);
        RECT rcClip;
        GetClipBox(memdc, &rcClip);

        DeleteObject(hrgn);
        DeleteObject(hrgn2);

        FillRect(memdc, &rcWnd, hbr);
        RestoreDC(memdc, -1);
        const TCHAR* txt = _T("hello soui");
        SIZE szTxt;
        GetTextExtentPoint32(memdc, txt, -1, &szTxt);
        CRect rc(CPoint(10, 70), szTxt);
        SetTextColor(memdc, RGB(0, 255, 0));
        DrawText(memdc, txt, -1, &rc, DT_SINGLELINE);//todo: disable this line to make test crash
        StretchBlt(hdc, 0, 0, rcWnd.right, rcWnd.bottom, memdc, 0, 0, rcWnd.right, -rcWnd.bottom, SRCCOPY);

        SelectObject(memdc, oldBmp);
        DeleteObject(bmp);
        DeleteDC(memdc);
    }

    if (1) {
        SetTextColor(hdc, RGB(255, 0, 0));
        SetBkColor(hdc, RGB(200, 200, 200));
        SetTextAlign(hdc, TA_TOP);
        //SetBkMode(hdc,TRANSPARENT);
        LOGFONT lf = { 0 };
        lf.lfHeight = 50;
        _tcscpy(lf.lfFaceName, _T("Arial"));
        HFONT hFont = CreateFontIndirect(&lf);
        HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
        std::tstring str = _T("hello soui");
        SIZE sz = { 0 };
        GetTextExtentPoint32(hdc, str.c_str(), str.length(), &sz);
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);

        HBRUSH br = CreateSolidBrush(RGB(128, 128, 128));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);

        TextOut(hdc, 10, 60, str.c_str(), -1);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
        SelectObject(hdc, oldBr);
        DeleteObject(br);
        SelectObject(hdc, oldFont);
        DeleteObject(hFont);
    }
    if (1) {//test arc api
        HBRUSH brPattern = CreatePatternBrush(m_yylogo);
        HBRUSH oldBr2 = (HBRUSH)SelectObject(hdc, brPattern);

        RECT rcArc = { 10,50,60,200 };
        int wid = rcArc.right - rcArc.left;
        int hei = rcArc.bottom - rcArc.top;
        OffsetRect(&rcArc, 10, 10);
        HBRUSH oldbr = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Ellipse(hdc, rcArc.left, rcArc.top, rcArc.right, rcArc.bottom);
        OffsetRect(&rcArc, wid, 0);
        Arc(hdc, rcArc.left, rcArc.top, rcArc.right, rcArc.bottom, rcArc.left, (rcArc.top + rcArc.bottom) / 2, (rcArc.left + rcArc.right) / 2, rcArc.top);
        OffsetRect(&rcArc, wid, 0);
        SelectObject(hdc, oldbr);
        HPEN pen = CreatePen(PS_DASH, 2, RGB(0, 128, 0));
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        Pie(hdc, rcArc.left, rcArc.top, rcArc.right, rcArc.bottom, rcArc.left, (rcArc.top + rcArc.bottom) / 2, (rcArc.left + rcArc.right) / 2, rcArc.top);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);

        SelectObject(hdc, oldBr2);
        DeleteObject(brPattern);
    }
    if (1) {//test round rect api
        RECT rc = { 300,400,400,450 };
        POINT pt = { 10,20 };
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, pt.x, pt.y);
        OffsetRect(&rc, 0, 50);
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, pt.x, pt.x);
    }

    if (1) {//test dash line
        HPEN dashPen[4];
        COLORREF cr[4] = { RGB(0,255,0),RGB(0,0, 255),RGB(255,255,0) ,RGB(0,255,255) };
        for (int i = 0; i < 4; i++) {
            dashPen[i] = CreatePen(PS_DASH + i, 2, cr[i]);
        }
        HPEN oldPen = (HPEN)SelectObject(hdc, dashPen[0]);
        MoveToEx(hdc, 100, 410, NULL);
        LineTo(hdc, 300, 410);

        SelectObject(hdc, dashPen[1]);
        MoveToEx(hdc, 100, 420, NULL);
        LineTo(hdc, 300, 420);

        SelectObject(hdc, dashPen[2]);
        MoveToEx(hdc, 100, 430, NULL);
        LineTo(hdc, 300, 430);

        SelectObject(hdc, dashPen[3]);
        MoveToEx(hdc, 100, 440, NULL);
        LineTo(hdc, 300, 440);

        SelectObject(hdc, oldPen);
        for (int i = 0; i < 4; i++) {
            DeleteObject(dashPen[i]);
        }
    }

    if (1) {
        //draw hbitmap
        HDC memdc = CreateCompatibleDC(hdc);
        HBITMAP oldBmp = (HBITMAP)SelectObject(memdc, m_bmp);

        RECT rc2 = { 0,100,64,100 + 64 };
        //BitBlt(hdc,rc2.left,rc2.top,rc2.right-rc2.left,rc2.bottom-rc2.top,memdc,64,64,SRCCOPY);
        BLENDFUNCTION bf;
        bf.SourceConstantAlpha = 128;//set alpha to 0.5
        AlphaBlend(hdc, 100, 100, 256, 256, memdc, 0, 0, 128, 128, bf);
        SelectObject(memdc, oldBmp);
        DeleteDC(memdc);
    }
    if (1) {//test gradient
        TRIVERTEX        vert[3];
        GRADIENT_RECT    gRect[2];
        vert[0].x = 10;
        vert[0].y = 300;
        vert[0].Red = 0x0000;
        vert[0].Green = 0x0000;
        vert[0].Blue = 0x0000;
        vert[0].Alpha = 0xffff;

        vert[1].x = 200;
        vert[1].y = 350;
        vert[1].Red = 0x0000;
        vert[1].Green = 0x0000;
        vert[1].Blue = 0xff00;
        vert[1].Alpha = 0xffff;

        vert[2].x = 300;
        vert[2].y = 300;
        vert[2].Red = 0x0000;
        vert[2].Green = 0xff00;
        vert[2].Blue = 0x0000;
        vert[2].Alpha = 0xffff;

        gRect[0].UpperLeft = 0;
        gRect[0].LowerRight = 1;
        gRect[1].UpperLeft = 1;
        gRect[1].LowerRight = 2;
        GradientFill(hdc, vert, 3, &gRect, 2, GRADIENT_FILL_RECT_H);
    }
    if (1) {//test bitmap brush
        HBRUSH br = CreatePatternBrush(m_yylogo);
        FillRect(hdc, CRect(300, 100, 400, 200), br);
        DeleteObject(br);
    }
    if (1) {//test Path API
        // Test BeginPath/EndPath with complex path
        BeginPath(hdc);

        // Create a star shape using MoveTo and LineTo
        MoveToEx(hdc, 450, 150, NULL);
        LineTo(hdc, 470, 120);
        LineTo(hdc, 500, 120);
        LineTo(hdc, 480, 140);
        LineTo(hdc, 490, 170);
        LineTo(hdc, 450, 155);
        LineTo(hdc, 410, 170);
        LineTo(hdc, 420, 140);
        LineTo(hdc, 400, 120);
        LineTo(hdc, 430, 120);
        CloseFigure(hdc);

        // Add a circle to the path
        MoveToEx(hdc, 520, 150, NULL);
        for (int i = 0; i <= 360; i += 10) {
            double angle = i * 3.14159 / 180.0;
            int x = 520 + (int)(20 * cos(angle));
            int y = 150 + (int)(20 * sin(angle));
            if (i == 0) {
                MoveToEx(hdc, x, y, NULL);
            } else {
                LineTo(hdc, x, y);
            }
        }
        CloseFigure(hdc);

        EndPath(hdc);

        // Test StrokePath
        HPEN pathPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 255));
        HPEN oldPathPen = (HPEN)SelectObject(hdc, pathPen);
        StrokePath(hdc);
        SelectObject(hdc, oldPathPen);
        DeleteObject(pathPen);

        // Test FillPath with a different path
        BeginPath(hdc);
        MoveToEx(hdc, 450, 200, NULL);
        LineTo(hdc, 500, 220);
        LineTo(hdc, 480, 250);
        LineTo(hdc, 420, 250);
        LineTo(hdc, 400, 220);
        CloseFigure(hdc);
        EndPath(hdc);

        HBRUSH pathBrush = CreateSolidBrush(RGB(0, 255, 128));
        HBRUSH oldPathBrush = (HBRUSH)SelectObject(hdc, pathBrush);
        FillPath(hdc);
        SelectObject(hdc, oldPathBrush);
        DeleteObject(pathBrush);

        // Test PathToRegion
        BeginPath(hdc);
        MoveToEx(hdc, 550, 200, NULL);
        LineTo(hdc, 580, 200);
        LineTo(hdc, 580, 230);
        LineTo(hdc, 550, 230);
        CloseFigure(hdc);
        EndPath(hdc);

        HRGN pathRegion = PathToRegion(hdc);
        if (pathRegion) {
            HBRUSH regionBrush = CreateSolidBrush(RGB(255, 128, 0));
            FillRgn(hdc, pathRegion, regionBrush);
            DeleteObject(regionBrush);
            DeleteObject(pathRegion);
        }

        // Test SelectClipPath
        BeginPath(hdc);
        MoveToEx(hdc, 450, 280, NULL);
        LineTo(hdc, 500, 280);
        LineTo(hdc, 500, 320);
        LineTo(hdc, 450, 320);
        CloseFigure(hdc);
        EndPath(hdc);

        int saveState = SaveDC(hdc);
        SelectClipPath(hdc, RGN_COPY);

        // Draw something that will be clipped
        HBRUSH clipBrush = CreateSolidBrush(RGB(128, 0, 255));
        FillRect(hdc, CRect(440, 270, 510, 330), clipBrush);
        DeleteObject(clipBrush);

        RestoreDC(hdc, saveState);
    }
    if (1) {//invert rect
        RECT rc = { 10,60,40,80 };
        InvertRect(hdc, &rc);
        //InvertRect(hdc, &rc);
    }
    if (1) {//TabbedTextout
        const char* text = "tabtabtab\ttext\ttest";
        int tabLen[] = { 100,250 };
        TabbedTextOutA(hdc, 0, 0, text, strlen(text), 2, tabLen, 50);
    }
    if (1) {
        RECT rc = { 0,0,50,50 };
        DrawFocusRect(hdc, &rc);
        OffsetRect(&rc, 0, 100);

        DrawFocusRect(hdc, &rc);
        DrawFocusRect(hdc, &rc);//clear focus rect
    }
    
    if (1) {//test Ellipse api
        CRect rcArc(CPoint(0,200),CSize(50,80));
        HBRUSH oldbr = (HBRUSH)SelectObject(hdc, GetStockObject(GRAY_BRUSH));
        HPEN oldPen = (HPEN)SelectObject(hdc, GetStockObject(BLACK_PEN));
        Ellipse(hdc, rcArc.left, rcArc.top, rcArc.right, rcArc.bottom);
        rcArc.OffsetRect(0, rcArc.Height()+5);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Ellipse(hdc, rcArc.left, rcArc.top, rcArc.right, rcArc.bottom);
        rcArc.OffsetRect(0, rcArc.Height()+5);
        SelectObject(hdc, GetStockObject(NULL_PEN));
        SelectObject(hdc, GetStockObject(GRAY_BRUSH));
        Ellipse(hdc, rcArc.left, rcArc.top, rcArc.right, rcArc.bottom);
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldbr);
    }
    if(1){//test bezier curve.
        printf("Testing PolyBezier and PolyBezierTo functions...\n");

        // Test 1: Basic PolyBezier (single curve)
        printf("1. Testing PolyBezier with single curve:\n");
        POINT bezierPoints1[4] = {
            {50, 100},   // Start point
            {100, 50},   // Control point 1
            {150, 150},  // Control point 2
            {200, 100}   // End point
        };

        BOOL result1 = PolyBezier(hdc, bezierPoints1, 4);
        printf("   PolyBezier single curve: %s\n", result1 ? "SUCCESS" : "FAILED");

        // Verify current position unchanged
        POINT currentPos;
        if (GetCurrentPositionEx(hdc, &currentPos)) {
            printf("   Current position after PolyBezier: (%d,%d) - should be unchanged\n",
                   currentPos.x, currentPos.y);
        }

        // Test 2: PolyBezier with multiple curves
        printf("2. Testing PolyBezier with multiple curves:\n");
        POINT bezierPoints2[7] = {
            {250, 100},  // Start point
            {300, 50},   // Curve 1: Control point 1
            {350, 150},  // Curve 1: Control point 2
            {400, 100},  // Curve 1: End point
            {450, 50},   // Curve 2: Control point 1
            {500, 150},  // Curve 2: Control point 2
            {550, 100}   // Curve 2: End point
        };

        BOOL result2 = PolyBezier(hdc, bezierPoints2, 7);
        printf("   PolyBezier multiple curves: %s\n", result2 ? "SUCCESS" : "FAILED");

        // Test 3: PolyBezierTo (requires current position)
        printf("3. Testing PolyBezierTo:\n");
        BOOL moveResult = MoveToEx(hdc, 50, 200, NULL);
        printf("   Set current position to (50,200): %s\n", moveResult ? "SUCCESS" : "FAILED");

        POINT bezierToPoints1[3] = {
            {100, 150},  // Control point 1
            {150, 250},  // Control point 2
            {200, 200}   // End point
        };

        BOOL result3 = PolyBezierTo(hdc, bezierToPoints1, 3);
        printf("   PolyBezierTo single curve: %s\n", result3 ? "SUCCESS" : "FAILED");

        // Verify current position updated
        if (GetCurrentPositionEx(hdc, &currentPos)) {
            printf("   Current position after PolyBezierTo: (%d,%d)\n", currentPos.x, currentPos.y);
            printf("   Expected: (200,200) - %s\n",
                   (currentPos.x == 200 && currentPos.y == 200) ? "MATCH" : "MISMATCH");
        }

        // Test 4: Chain multiple PolyBezierTo calls
        printf("4. Testing chained PolyBezierTo calls:\n");
        POINT bezierToPoints2[6] = {
            {250, 150},  // Curve 1: Control point 1
            {300, 250},  // Curve 1: Control point 2
            {350, 200},  // Curve 1: End point
            {400, 150},  // Curve 2: Control point 1
            {450, 250},  // Curve 2: Control point 2
            {500, 200}   // Curve 2: End point
        };

        BOOL result4 = PolyBezierTo(hdc, bezierToPoints2, 6);
        printf("   Chained PolyBezierTo: %s\n", result4 ? "SUCCESS" : "FAILED");

        if (GetCurrentPositionEx(hdc, &currentPos)) {
            printf("   Final current position: (%d,%d)\n", currentPos.x, currentPos.y);
            printf("   Expected: (500,200) - %s\n",
                   (currentPos.x == 500 && currentPos.y == 200) ? "MATCH" : "MISMATCH");
        }

        // Test 5: Bezier curves with Path API
        printf("5. Testing Bezier curves with Path recording:\n");
        BOOL beginResult = BeginPath(hdc);
        printf("   BeginPath: %s\n", beginResult ? "SUCCESS" : "FAILED");

        // Add PolyBezier to path
        MoveToEx(hdc, 50, 300, NULL);
        POINT pathBezierPoints[4] = {
            {50, 300},   // Start point
            {100, 250},  // Control point 1
            {150, 350},  // Control point 2
            {200, 300}   // End point
        };

        BOOL pathResult1 = PolyBezier(hdc, pathBezierPoints, 4);
        printf("   Add PolyBezier to path: %s\n", pathResult1 ? "SUCCESS" : "FAILED");

        // Add PolyBezierTo to path
        POINT pathBezierToPoints[3] = {
            {250, 250},  // Control point 1
            {300, 350},  // Control point 2
            {350, 300}   // End point
        };

        BOOL pathResult2 = PolyBezierTo(hdc, pathBezierToPoints, 3);
        printf("   Add PolyBezierTo to path: %s\n", pathResult2 ? "SUCCESS" : "FAILED");

        BOOL endResult = EndPath(hdc);
        printf("   EndPath: %s\n", endResult ? "SUCCESS" : "FAILED");

        BOOL strokeResult = StrokePath(hdc);
        printf("   StrokePath: %s\n", strokeResult ? "SUCCESS" : "FAILED");

        // Test 6: Error handling
        printf("6. Testing error conditions:\n");
        POINT invalidPoints[5] = {{0,0}, {10,10}, {20,20}, {30,30}, {40,40}};

        // Invalid point count for PolyBezier
        BOOL errorResult1 = PolyBezier(hdc, invalidPoints, 5);
        printf("   PolyBezier invalid point count (5): %s\n",
               errorResult1 ? "UNEXPECTED SUCCESS" : "CORRECTLY FAILED");

        // Invalid point count for PolyBezierTo
        BOOL errorResult2 = PolyBezierTo(hdc, invalidPoints, 4);
        printf("   PolyBezierTo invalid point count (4): %s\n",
               errorResult2 ? "UNEXPECTED SUCCESS" : "CORRECTLY FAILED");

        // NULL points
        BOOL errorResult3 = PolyBezier(hdc, NULL, 4);
        printf("   PolyBezier with NULL points: %s\n",
               errorResult3 ? "UNEXPECTED SUCCESS" : "CORRECTLY FAILED");

        BOOL errorResult4 = PolyBezierTo(hdc, NULL, 3);
        printf("   PolyBezierTo with NULL points: %s\n",
               errorResult4 ? "UNEXPECTED SUCCESS" : "CORRECTLY FAILED");

        // Test PolyBezierTo without current position
        HDC newHdc = CreateCompatibleDC(NULL);
        BOOL errorResult5 = PolyBezierTo(newHdc, bezierToPoints1, 3);
        printf("   PolyBezierTo without current position: %s\n",
               errorResult5 ? "UNEXPECTED SUCCESS" : "CORRECTLY FAILED");
        DeleteDC(newHdc);

        printf("Bezier curve testing completed!\n\n");
    }
    EndPaint(m_hWnd, &ps);
}
void SNativeWnd2::OnClose() {
    //PostMessage(WM_QUIT,1);
    PostThreadMessage(GetCurrentThreadId(), WM_QUIT, 1, 0);
}

int run_window() {
    int ret = 0;
    CScintillaWnd::InitScintilla(0);
    SNativeWndHelper::instance()->Init(0, _T("soui_host"), FALSE);
    SNativeWnd2 wnd;
    SOUI::SStringT iconPath = getSourceDir() + _T("/fun_test/uires/image/soui.ico");
    HICON hIcon = (HICON)LoadImage(0, iconPath.c_str(), IMAGE_ICON, 128, 128, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    HWND hWnd = wnd.CreateNative(_T("test window"), WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME, WS_EX_APPWINDOW, 0, 0, 0, 0, 0);
    if (hWnd) {
        wnd.SendMessage(WM_SETICON, 1, (LPARAM)hIcon);
        wnd.SetWindowPos(0, 0, 0, 600, 600, SWP_NOMOVE | SWP_NOZORDER);
        if (0) {
            SCROLLINFO si;
            si.fMask = SIF_ALL;
            si.nMin = 0;
            si.nMax = 100;
            si.nPos = 20;
            si.nPage = 10;
            si.nTrackPos = -1;
            SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
            ShowScrollBar(hWnd, SB_VERT, TRUE);

            SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
            ShowScrollBar(hWnd, SB_HORZ, TRUE);
        }
        wnd.ShowWindow(SW_SHOW);
        SMessageLoop loop(NULL);
        ret = loop.Run();
        wnd.DestroyWindow();
    }
    DestroyIcon(hIcon);
    CScintillaWnd::UninitScintilla();
    SNativeWndHelper::instance()->Uninit();
    return ret;
}
