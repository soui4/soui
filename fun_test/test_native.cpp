#include <souistd.h>
#include "common.h"
#include "ScintillaWnd.h"

using namespace SOUI;
#define kLogTag "test_native"

static const TCHAR* kPath_TestPng = _T("/demo/uires/image/soui.png");
static const TCHAR* kPath_TestXml = _T("/demo2/uires/xml/dlg_main.xml");
static const TCHAR* kPath_TestYyLogo = _T("/fun_test/uires/image/yy.png");

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
    std::tstring srcDir = getSourceDir();
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
        const char* test = "teº¯Êýst";
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
    EndPaint(m_hWnd, &ps);
}
void SNativeWnd2::OnClose() {
    //PostMessage(WM_QUIT,1);
    PostThreadMessage(GetCurrentThreadId(), WM_QUIT, 1, 0);
}

int run_window() {
    int ret = 0;
    CScintillaWnd::InitScintilla(0);
    SNativeWnd::InitWndClass(0, _T("soui_host"), FALSE);
    SNativeWnd2 wnd;
    std::tstring iconPath = getSourceDir() + _T("/fun_test/uires/image/soui.ico");
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
    return ret;
}
