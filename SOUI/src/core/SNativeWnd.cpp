#include <souistd.h>
#include <helper/SCriticalSection.h>
#include <core/SNativeWnd.h>

SNSBEGIN

#ifndef ENABLE_THUNK
#ifdef _WIN32
#define ENABLE_THUNK 1
#else
#define ENABLE_THUNK 0
#endif //_WIN32
#endif // ENABLE_THUNK

//////////////////////////////////////////////////////////////////////////
// thunk 技术实现参考http://www.cppblog.com/proguru/archive/2008/08/24/59831.html
//////////////////////////////////////////////////////////////////////////

#if defined(__i386__) || defined(__X86__) || defined(_M_IX86)
#pragma pack(push, 1)
struct tagThunk
{
    DWORD m_mov;
    DWORD m_this;
    BYTE m_jmp;
    DWORD m_relproc;
    void Init(DWORD_PTR proc, void *pThis)
    {
        m_mov = 0x042444C7;
        m_this = (DWORD)(ULONG_PTR)pThis;
        m_jmp = 0xe9;
        m_relproc = (DWORD)((INT_PTR)proc - ((INT_PTR)this + sizeof(tagThunk)));
        FlushInstructionCache(INVALID_HANDLE_VALUE, this, sizeof(tagThunk));
    }
    void *GetCodeAddress()
    {
        return this;
    }
};
#pragma pack(pop)
#elif defined(_M_AMD64) || defined(__amd64__) || defined(__x86_64__)
#if defined(__linux__)
#pragma pack(push, 2)
struct tagThunk
{
    USHORT RdiMov;  // mov rdi, pThis
    ULONG64 RdiImm; //
    USHORT RaxMov;  // mov rax, target
    ULONG64 RaxImm; //
    USHORT RaxJmp;  // jmp target
    void Init(DWORD_PTR proc, void *pThis)
    {
        RdiMov = 0xbf48;         // mov rdi, pThis
        RdiImm = (ULONG64)pThis; //
        RaxMov = 0xb848;         // mov rax, target
        RaxImm = (ULONG64)proc;  //
        RaxJmp = 0xe0ff;         // jmp rax
        BOOL b = FlushInstructionCache(INVALID_HANDLE_VALUE, this, sizeof(tagThunk));
    }
    // some thunks will dynamically allocate the memory for the code
    void *GetCodeAddress()
    {
        return this;
    }
};
#else //__linux__
#pragma pack(push, 2)
struct tagThunk
{
    USHORT RcxMov;  // mov rcx, pThis
    ULONG64 RcxImm; //
    USHORT RaxMov;  // mov rax, target
    ULONG64 RaxImm; //
    USHORT RaxJmp;  // jmp target
    void Init(DWORD_PTR proc, void *pThis)
    {
        RcxMov = 0xb948;         // mov rcx, pThis
        RcxImm = (ULONG64)pThis; //
        RaxMov = 0xb848;         // mov rax, target
        RaxImm = (ULONG64)proc;  //
        RaxJmp = 0xe0ff;         // jmp rax
        FlushInstructionCache(INVALID_HANDLE_VALUE, this, sizeof(tagThunk));
    }
    // some thunks will dynamically allocate the memory for the code
    void *GetCodeAddress()
    {
        return this;
    }
};
#pragma pack(pop)
#endif //__linux__

#elif defined(__arm__) || defined(_M_ARM)
#pragma pack(push, 4)
struct tagThunk // this should come out to 16 bytes
{
    DWORD m_mov_r0; // mov    r0, pThis
    DWORD m_mov_pc; // mov    pc, pFunc
    DWORD m_pThis;
    DWORD m_pFunc;
    void Init(DWORD_PTR proc, void *pThis)
    {
        m_mov_r0 = 0xE59F0000;
        m_mov_pc = 0xE59FF000;
        m_pThis = (DWORD)pThis;
        m_pFunc = (DWORD)proc;
        // write block from data cache and
        //  flush from instruction cache
        FlushInstructionCache(INVALID_HANDLE_VALUE, this, sizeof(tagThunk));
    }
    void *GetCodeAddress()
    {
        return this;
    }
};
#pragma pack(pop)
#elif defined(__arm64__) || defined(_M_ARM64) || defined(__aarch64__)
#pragma pack(push, 1)
struct tagThunk // this should come out to 16 bytes
{
    DWORD m_ldr_r16; // ldr  x16, [pc, #24]
    DWORD m_ldr_r0;  // ldr  x0, [pc, #12]
    DWORD m_br;      // br   x16
    DWORD m_pad;
    ULONG64 m_pThis;
    ULONG64 m_pFunc;

    void Init(DWORD_PTR proc, void *pThis)
    {
        m_ldr_r16 = 0x580000D0;
        m_ldr_r0 = 0x58000060;
        m_br = 0xd61f0200;
        m_pThis = (ULONG64)pThis;
        m_pFunc = (ULONG64)proc;
        // write block from data cache and
        //  flush from instruction cache
        FlushInstructionCache(INVALID_HANDLE_VALUE, this, sizeof(tagThunk));
    }
    void *GetCodeAddress()
    {
        return this;
    }
};
#pragma pack(pop)
#else
#error Only AMD64, ARM, ARM64 and X86 supported
#endif

SNativeWndHelper::SNativeWndHelper()
    : m_hInst(0)
    , m_sharePtr(NULL)
    , m_atom(0)
{
}

BOOL SNativeWndHelper::Init(HINSTANCE hInst, LPCTSTR pszClassName, BOOL bImeApp)
{
    SAutoLock lock(m_cs);
    if (m_hHeap)
        return FALSE;
    m_hInst = hInst;
    m_hHeap = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE, 0, 0);
    m_atom = SNativeWnd::RegisterSimpleWnd(m_hInst, pszClassName, bImeApp);
    return TRUE;
}

SNativeWndHelper::~SNativeWndHelper()
{
    if (m_hHeap)
        HeapDestroy(m_hHeap);
    if (m_atom)
        UnregisterClass((LPCTSTR)(UINT_PTR)m_atom, m_hInst);
}

void SNativeWndHelper::LockSharePtr(void *p)
{
    m_cs.Enter();
    m_sharePtr = p;
}

void SNativeWndHelper::UnlockSharePtr()
{
    m_cs.Leave();
}

//////////////////////////////////////////////////////////////////////////
SNativeWnd::SNativeWnd()
    : m_bDestoryed(FALSE)
    , m_pCurrentMsg(NULL)
    , m_hWnd(0)
    , m_pfnSuperWindowProc(::DefWindowProc)
    , m_pThunk(NULL)
{
    m_msgHandlerInfo.fun = NULL;
    m_msgHandlerInfo.ctx = NULL;
}

SNativeWnd::~SNativeWnd(void)
{
}

ATOM SNativeWnd::RegisterSimpleWnd(HINSTANCE hInst, LPCTSTR pszSimpleWndName, BOOL bImeWnd)
{
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX), 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | (bImeWnd ? CS_IME : 0);
    wcex.lpfnWndProc = StartWindowProc; 
    wcex.hInstance = hInst;
    wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wcex.lpszClassName = pszSimpleWndName;
    return ::RegisterClassEx(&wcex);
}

void SNativeWnd::InitWndClass(HINSTANCE hInst, LPCTSTR pszSimpleWndName, BOOL bImeWnd)
{
    SNativeWndHelper::instance()->Init(hInst, pszSimpleWndName, bImeWnd);
}

HWND SNativeWnd::CreateNative(LPCTSTR lpWindowName, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, int nID, LPVOID lpParam)
{
    SNativeWndHelper::instance()->LockSharePtr(this);
#if ENABLE_THUNK
    m_pThunk = (tagThunk *)HeapAlloc(SNativeWndHelper::instance()->GetHeap(), HEAP_ZERO_MEMORY | HEAP_CREATE_ENABLE_EXECUTE, sizeof(tagThunk));
#endif
    HWND hWnd = ::CreateWindowEx(dwExStyle, (LPCTSTR)(UINT_PTR)SNativeWndHelper::instance()->GetSimpleWndAtom(), lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, (HMENU)(UINT_PTR)nID, SNativeWndHelper::instance()->GetAppInstance(), lpParam);
    SNativeWndHelper::instance()->UnlockSharePtr();
#if ENABLE_THUNK
    if (!hWnd)
    {
        HeapFree(SNativeWndHelper::instance()->GetHeap(), 0, m_pThunk);
        m_pThunk = NULL;
    }
#endif
    return hWnd;
}

HWND SNativeWnd::GetHwnd()
{
    return m_hWnd;
}

void SNativeWnd::OnFinalMessage(HWND hWnd)
{
#if ENABLE_THUNK
    if (m_pThunk)
    {
        HeapFree(SNativeWndHelper::instance()->GetHeap(), 0, m_pThunk);
        m_pThunk = NULL;
    }
#endif
    m_hWnd = 0;
}

LRESULT CALLBACK SNativeWnd::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#if ENABLE_THUNK
    SNativeWnd *pThis = (SNativeWnd *)hWnd;
#else
    SNativeWnd *pThis = (SNativeWnd *)::GetWindowLongPtr(hWnd, GWLP_OPAQUE);
#endif
    MSG msg = { pThis->m_hWnd, uMsg, wParam, lParam };
    const MSG *pOldMsg = pThis->m_pCurrentMsg;
    pThis->m_pCurrentMsg = &msg;
    LRESULT lRes = 0;
    if (pThis->m_msgHandlerInfo.fun)
    {
        BOOL bHandled = pThis->m_msgHandlerInfo.fun(&msg, &lRes, pThis->m_msgHandlerInfo.ctx);
        if (bHandled)
            return lRes;
    }

    // pass to the message map to process
    BOOL bRet = pThis->ProcessWindowMessage(pThis->m_hWnd, uMsg, wParam, lParam, lRes, 0);
    // restore saved value for the current message
    SASSERT(pThis->m_pCurrentMsg == &msg);

    // do the default processing if message was not handled
    if (!bRet)
    {
        if (uMsg != WM_NCDESTROY)
            lRes = pThis->DefWindowProc(uMsg, wParam, lParam);
        else
        {
            // unsubclass, if needed
            LONG_PTR pfnWndProc = ::GetWindowLongPtr(pThis->m_hWnd, GWLP_WNDPROC);
            lRes = pThis->DefWindowProc(uMsg, wParam, lParam);
            if (pThis->m_pfnSuperWindowProc != ::DefWindowProc && ::GetWindowLongPtr(pThis->m_hWnd, GWLP_WNDPROC) == pfnWndProc)
                ::SetWindowLongPtr(pThis->m_hWnd, GWLP_WNDPROC, (LONG_PTR)pThis->m_pfnSuperWindowProc);
            // mark window as destryed
            pThis->m_bDestoryed = TRUE;
        }
    }
    if ((pThis->m_bDestoryed) && pOldMsg == NULL)
    {
        // clear out window handle
        HWND hWndThis = pThis->m_hWnd;
        pThis->m_hWnd = 0;
        pThis->m_bDestoryed = FALSE;
        // clean up after window is destroyed
        pThis->m_pCurrentMsg = pOldMsg;
        pThis->OnFinalMessage(hWndThis);
    }
    else
    {
        pThis->m_pCurrentMsg = pOldMsg;
    }
    return lRes;
}

LRESULT CALLBACK SNativeWnd::StartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SNativeWnd *pThis = (SNativeWnd *)SNativeWndHelper::instance()->GetSharePtr();
    SNativeWndHelper::instance()->UnlockSharePtr();
    pThis->m_hWnd = hWnd;
#if ENABLE_THUNK
    // 初始化Thunk，做了两件事:1、mov指令替换hWnd为对象指针，2、jump指令跳转到WindowProc
    pThis->m_pThunk->Init((DWORD_PTR)WindowProc, pThis);
    // 得到Thunk指针
    WNDPROC pProc = (WNDPROC)pThis->m_pThunk->GetCodeAddress();
    // 调用下面的语句后，以后消息来了，都由pProc处理
    ::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)pProc);
    return pProc(hWnd, uMsg, wParam, lParam);
#else
    ::SetWindowLongPtr(hWnd, GWLP_OPAQUE, (ULONG_PTR)pThis);
    ::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (ULONG_PTR)WindowProc);
    return WindowProc(hWnd, uMsg, wParam, lParam);
#endif // ENABLE_THUNK
}

BOOL SNativeWnd::SubclassWindow(HWND hWnd)
{
    SASSERT(::IsWindow(hWnd));
// Allocate the thunk structure here, where we can fail gracefully.
#if ENABLE_THUNK
    m_pThunk = (tagThunk *)HeapAlloc(SNativeWndHelper::instance()->GetHeap(), HEAP_ZERO_MEMORY | HEAP_CREATE_ENABLE_EXECUTE, sizeof(tagThunk));
    m_pThunk->Init((DWORD_PTR)WindowProc, this);
    WNDPROC pProc = (WNDPROC)m_pThunk->GetCodeAddress();
#else
    ::SetWindowLongPtr(hWnd, GWLP_OPAQUE, (LONG_PTR)this);
    WNDPROC pProc = WindowProc;
#endif
    WNDPROC pfnWndProc = (WNDPROC)::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)pProc);
    if (pfnWndProc == NULL)
    {
#if ENABLE_THUNK
        HeapFree(SNativeWndHelper::instance()->GetHeap(), 0, m_pThunk);
        m_pThunk = NULL;
#endif
        return FALSE;
    }
    m_pfnSuperWindowProc = pfnWndProc;
    m_hWnd = hWnd;
    return TRUE;
}

HWND SNativeWnd::UnsubclassWindow(BOOL bForce /*= FALSE*/)
{
    SASSERT(m_hWnd != 0);

#if ENABLE_THUNK
    WNDPROC pOurProc = (WNDPROC)m_pThunk->GetCodeAddress();
#else
    WNDPROC pOurProc = WindowProc;
#endif
    WNDPROC pActiveProc = (WNDPROC)::GetWindowLongPtr(m_hWnd, GWLP_WNDPROC);

    HWND hWnd = 0;
    if (bForce || pOurProc == pActiveProc)
    {
        if (!::SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)m_pfnSuperWindowProc))
            return 0;
#if ENABLE_THUNK == 0
        ::SetWindowLongPtr(m_hWnd, GWLP_OPAQUE, (LONG_PTR)0);
#endif // ENABLE_THUNK
        m_pfnSuperWindowProc = ::DefWindowProc;
        hWnd = m_hWnd;
        m_hWnd = 0;
    }
    return hWnd;
}

LRESULT SNativeWnd::ForwardNotifications(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    LRESULT lResult = 0;
    switch (uMsg)
    {
    case WM_COMMAND:
    case WM_NOTIFY:
    case WM_PARENTNOTIFY:
    case WM_DRAWITEM:
    case WM_MEASUREITEM:
    case WM_COMPAREITEM:
    case WM_DELETEITEM:
    case WM_VKEYTOITEM:
    case WM_CHARTOITEM:
    case WM_HSCROLL:
    case WM_VSCROLL:
        bHandled = TRUE;
        lResult = ::SendMessage(GetParent(), uMsg, wParam, lParam);
        break;
    default:
        bHandled = FALSE;
        break;
    }
    return lResult;
}

LRESULT SNativeWnd::ReflectNotifications(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    HWND hWndChild = 0;

    switch (uMsg)
    {
    case WM_COMMAND:
        if (lParam != 0) // not from a menu
            hWndChild = (HWND)lParam;
        break;
    case WM_NOTIFY:
        hWndChild = ((LPNMHDR)lParam)->hwndFrom;
        break;
    case WM_PARENTNOTIFY:
        switch (LOWORD(wParam))
        {
        case WM_CREATE:
        case WM_DESTROY:
            hWndChild = (HWND)lParam;
            break;
        default:
            hWndChild = GetDlgItem(m_hWnd, HIWORD(wParam));
            break;
        }
        break;
    case WM_DRAWITEM:
        if (wParam) // not from a menu
            hWndChild = ((LPDRAWITEMSTRUCT)lParam)->hwndItem;
        break;
    case WM_MEASUREITEM:
        if (wParam) // not from a menu
            hWndChild = GetDlgItem(m_hWnd, ((LPMEASUREITEMSTRUCT)lParam)->CtlID);
        break;
    case WM_COMPAREITEM:
        if (wParam) // not from a menu
            hWndChild = ((LPCOMPAREITEMSTRUCT)lParam)->hwndItem;
        break;
    case WM_DELETEITEM:
        if (wParam) // not from a menu
            hWndChild = ((LPDELETEITEMSTRUCT)lParam)->hwndItem;

        break;
    case WM_VKEYTOITEM:
    case WM_CHARTOITEM:
    case WM_HSCROLL:
    case WM_VSCROLL:
        hWndChild = (HWND)lParam;
        break;
#ifdef _WIN32
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
        hWndChild = (HWND)lParam;
        break;
#endif //_WIN32
    default:
        break;
    }

    if (hWndChild == 0)
    {
        bHandled = FALSE;
        return 1;
    }
    bHandled = TRUE;
    SASSERT(::IsWindow(hWndChild));
    return ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
}

BOOL SNativeWnd::DefaultReflectionHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &lResult)
{
    switch (uMsg)
    {
    case OCM_COMMAND:
    case OCM_NOTIFY:
    case OCM_PARENTNOTIFY:
    case OCM_DRAWITEM:
    case OCM_MEASUREITEM:
    case OCM_COMPAREITEM:
    case OCM_DELETEITEM:
    case OCM_VKEYTOITEM:
    case OCM_CHARTOITEM:
    case OCM_HSCROLL:
    case OCM_VSCROLL:
#ifdef _WIN32
    case OCM_CTLCOLORBTN:
    case OCM_CTLCOLORDLG:
    case OCM_CTLCOLOREDIT:
    case OCM_CTLCOLORLISTBOX:
    case OCM_CTLCOLORMSGBOX:
    case OCM_CTLCOLORSCROLLBAR:
    case OCM_CTLCOLORSTATIC:
#endif //_WIN32
        lResult = ::DefWindowProc(hWnd, uMsg - OCM__BASE, wParam, lParam);
        return TRUE;
    default:
        break;
    }
    return FALSE;
}

LRESULT SNativeWnd::DefWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return ::CallWindowProc(m_pfnSuperWindowProc, m_hWnd, uMsg, wParam, lParam);
}

LRESULT SNativeWnd::DefWindowProc()
{
    const MSG *pMsg = m_pCurrentMsg;
    LRESULT lRes = 0;
    if (pMsg != NULL)
        lRes = DefWindowProc(pMsg->message, pMsg->wParam, pMsg->lParam);
    return lRes;
}

BOOL SNativeWnd::CenterWindow(HWND hWndCenter /*= NULL*/)
{
    SASSERT(::IsWindow(m_hWnd));

    // determine owner window to center against
    DWORD dwStyle = GetStyle();
    if (hWndCenter == 0)
    {
        if (dwStyle & WS_CHILD)
            hWndCenter = ::GetParent(m_hWnd);
        else
            hWndCenter = ::GetWindow(m_hWnd, GW_OWNER);

        if (hWndCenter == 0)
        {
            hWndCenter = ::GetActiveWindow();
            if (hWndCenter == m_hWnd)
                hWndCenter = 0;
        }
    }

    // get coordinates of the window relative to its parent
    RECT rcDlg;
    ::GetWindowRect(m_hWnd, &rcDlg);
    RECT rcArea;
    RECT rcCenter;
    HWND hWndParent;
    if (!(dwStyle & WS_CHILD))
    {
        // don't center against invisible or minimized windows
        if (hWndCenter != 0)
        {
            DWORD dwStyleCenter = (DWORD)::GetWindowLongPtr(hWndCenter, GWL_STYLE);
            if (!(dwStyleCenter & WS_VISIBLE) || (dwStyleCenter & WS_MINIMIZE))
                hWndCenter = 0;
        }

        // center within screen coordinates
#if WINVER < 0x0500
        ::SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcArea, NULL);
#else
        HMONITOR hMonitor = 0;
        if (hWndCenter != 0)
        {
            hMonitor = ::MonitorFromWindow(hWndCenter, MONITOR_DEFAULTTONEAREST);
        }
        else
        {
            hMonitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
        }

        MONITORINFO minfo;
        minfo.cbSize = sizeof(MONITORINFO);
        ::GetMonitorInfo(hMonitor, &minfo);

        rcArea = minfo.rcWork;
#endif
        if (hWndCenter == 0)
            rcCenter = rcArea;
        else
            ::GetWindowRect(hWndCenter, &rcCenter);
    }
    else
    {
        // center within parent client coordinates
        hWndParent = ::GetParent(m_hWnd);
        SASSERT(::IsWindow(hWndParent));

        ::GetClientRect(hWndParent, &rcArea);
        SASSERT(::IsWindow(hWndCenter));
        ::GetClientRect(hWndCenter, &rcCenter);
        ::MapWindowPoints(hWndCenter, hWndParent, (POINT *)&rcCenter, 2);
    }

    int DlgWidth = rcDlg.right - rcDlg.left;
    int DlgHeight = rcDlg.bottom - rcDlg.top;

    // find dialog's upper left based on rcCenter
    int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
    int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

    // if the dialog is outside the screen, move it inside
    if (xLeft + DlgWidth > rcArea.right)
        xLeft = rcArea.right - DlgWidth;
    if (xLeft < rcArea.left)
        xLeft = rcArea.left;

    if (yTop + DlgHeight > rcArea.bottom)
        yTop = rcArea.bottom - DlgHeight;
    if (yTop < rcArea.top)
        yTop = rcArea.top;

    // map screen coordinates to child coordinates
    return ::SetWindowPos(m_hWnd, 0, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

BOOL SNativeWnd::ModifyStyle(DWORD dwRemove, DWORD dwAdd, UINT nFlags /*= 0*/)
{
    SASSERT(::IsWindow(m_hWnd));

    DWORD dwStyle = (DWORD)::GetWindowLongPtr(m_hWnd, GWL_STYLE);
    DWORD dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
    if (dwStyle == dwNewStyle)
        return FALSE;

    ::SetWindowLongPtr(m_hWnd, GWL_STYLE, dwNewStyle);
    if (nFlags != 0)
    {
        ::SetWindowPos(m_hWnd, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | nFlags);
    }

    return TRUE;
}

BOOL SNativeWnd::ModifyStyleEx(DWORD dwRemove, DWORD dwAdd, UINT nFlags /*= 0*/)
{
    // todo:hjx to support ws_ex_composite change during running.
    SASSERT(::IsWindow(m_hWnd));

    DWORD dwStyle = (DWORD)::GetWindowLongPtr(m_hWnd, GWL_EXSTYLE);
    DWORD dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
    if (dwStyle == dwNewStyle)
        return FALSE;

    ::SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, dwNewStyle);
    if (nFlags != 0)
    {
        ::SetWindowPos(m_hWnd, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | nFlags);
    }

    return TRUE;
}

BOOL SNativeWnd::ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &lResult, DWORD dwMsgMapID /*= 0*/)
{
    return FALSE;
}

BOOL SNativeWnd::UpdateLayeredWindow(HDC hdcDst, POINT *pptDst, SIZE *psize, HDC hdcSrc, POINT *pptSrc, COLORREF crKey, BLENDFUNCTION *pblend, DWORD dwFlags)
{
    SASSERT(::IsWindow(m_hWnd));
#ifdef _WIN32
    return ::UpdateLayeredWindow(m_hWnd, hdcDst, pptDst, psize, hdcSrc, pptSrc, crKey, pblend, dwFlags);
#else
    return FALSE;
#endif //_WIN32
}

BOOL SNativeWnd::SetLayeredWindowAttributes(COLORREF crKey, BYTE bAlpha, DWORD dwFlags)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::SetLayeredWindowAttributes(m_hWnd, crKey, bAlpha, dwFlags);
}

int SNativeWnd::SetWindowRgn(HRGN hRgn, BOOL bRedraw /*=TRUE*/)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::SetWindowRgn(m_hWnd, hRgn, bRedraw);
}

BOOL SNativeWnd::ShowWindow(int nCmdShow)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::ShowWindow(m_hWnd, nCmdShow);
}

BOOL SNativeWnd::MoveWindow2(LPCRECT lpRect, BOOL bRepaint /*= TRUE*/)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::MoveWindow(m_hWnd, lpRect->left, lpRect->top, lpRect->right - lpRect->left, lpRect->bottom - lpRect->top, bRepaint);
}

BOOL SNativeWnd::MoveWindow(int x, int y, int nWidth, int nHeight, BOOL bRepaint /*= TRUE*/)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::MoveWindow(m_hWnd, x, y, nWidth, nHeight, bRepaint);
}

BOOL SNativeWnd::IsWindowVisible() const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::IsWindowVisible(m_hWnd);
}

BOOL SNativeWnd::IsZoomed() const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::IsZoomed(m_hWnd);
}

BOOL SNativeWnd::IsIconic() const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::IsIconic(m_hWnd);
}

int SNativeWnd::GetWindowText(LPTSTR lpszStringBuf, int nMaxCount) const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::GetWindowText(m_hWnd, lpszStringBuf, nMaxCount);
}

BOOL SNativeWnd::SetWindowText(LPCTSTR lpszString)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::SetWindowText(m_hWnd, lpszString);
}

BOOL SNativeWnd::SendNotifyMessage(UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::SendNotifyMessage(m_hWnd, message, wParam, lParam);
}

BOOL SNativeWnd::PostMessage(UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::PostMessage(m_hWnd, message, wParam, lParam);
}

LRESULT SNativeWnd::SendMessage(UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::SendMessage(m_hWnd, message, wParam, lParam);
}

HWND SNativeWnd::SetFocus()
{
    SASSERT(::IsWindow(m_hWnd));
    return ::SetFocus(m_hWnd);
}

HWND SNativeWnd::SetCapture()
{
    SASSERT(::IsWindow(m_hWnd));
    return ::SetCapture(m_hWnd);
}

HWND SNativeWnd::GetCapture()
{
    return ::GetCapture();
}

BOOL SNativeWnd::ReleaseCapture()
{
    return ::ReleaseCapture();
}

BOOL SNativeWnd::ShowCaret()
{
    SASSERT(::IsWindow(m_hWnd));
    return ::ShowCaret(m_hWnd);
}

BOOL SNativeWnd::HideCaret()
{
    SASSERT(::IsWindow(m_hWnd));
    return ::HideCaret(m_hWnd);
}

BOOL SNativeWnd::CreateCaret(HBITMAP hBitmap, int nWidth, int nHeight)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::CreateCaret(m_hWnd, hBitmap, nWidth, nHeight);
}

int SNativeWnd::ReleaseDC(HDC hDC)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::ReleaseDC(m_hWnd, hDC);
}

HDC SNativeWnd::GetWindowDC()
{
    SASSERT(::IsWindow(m_hWnd));
    return ::GetWindowDC(m_hWnd);
}

HDC SNativeWnd::GetDC()
{
    SASSERT(::IsWindow(m_hWnd));
    return ::GetDC(m_hWnd);
}

BOOL SNativeWnd::KillTimer(UINT_PTR nIDEvent)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::KillTimer(m_hWnd, nIDEvent);
}

UINT_PTR SNativeWnd::SetTimer(UINT_PTR nIDEvent, UINT nElapse, void(CALLBACK *lpfnTimer)(HWND, UINT, UINT_PTR, DWORD) /*= NULL*/)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::SetTimer(m_hWnd, nIDEvent, nElapse, (TIMERPROC)lpfnTimer);
}

int SNativeWnd::MapWindowRect(HWND hWndTo, LPRECT lpRect) const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::MapWindowPoints(m_hWnd, hWndTo, (LPPOINT)lpRect, 2);
}

int SNativeWnd::MapWindowPoints(HWND hWndTo, LPPOINT lpPoint, UINT nCount) const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::MapWindowPoints(m_hWnd, hWndTo, lpPoint, nCount);
}

BOOL SNativeWnd::ScreenToClient2(LPRECT lpRect) const
{
    SASSERT(::IsWindow(m_hWnd));
    if (!::ScreenToClient(m_hWnd, (LPPOINT)lpRect))
        return FALSE;
    return ::ScreenToClient(m_hWnd, ((LPPOINT)lpRect) + 1);
}

BOOL SNativeWnd::ScreenToClient(LPPOINT lpPoint) const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::ScreenToClient(m_hWnd, lpPoint);
}

BOOL SNativeWnd::ClientToScreen2(LPRECT lpRect) const
{
    SASSERT(::IsWindow(m_hWnd));
    if (!::ClientToScreen(m_hWnd, (LPPOINT)lpRect))
        return FALSE;
    return ::ClientToScreen(m_hWnd, ((LPPOINT)lpRect) + 1);
}

BOOL SNativeWnd::ClientToScreen(LPPOINT lpPoint) const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::ClientToScreen(m_hWnd, lpPoint);
}

BOOL SNativeWnd::GetClientRect(LPRECT lpRect) const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::GetClientRect(m_hWnd, lpRect);
}

BOOL SNativeWnd::GetWindowRect(LPRECT lpRect) const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::GetWindowRect(m_hWnd, lpRect);
}

BOOL SNativeWnd::InvalidateRect(LPCRECT lpRect, BOOL bErase /*= TRUE*/)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::InvalidateRect(m_hWnd, lpRect, bErase);
}

BOOL SNativeWnd::Invalidate(BOOL bErase /*= TRUE*/)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::InvalidateRect(m_hWnd, NULL, bErase);
}

BOOL SNativeWnd::IsWindow() SCONST
{
    return ::IsWindow(m_hWnd);
}

BOOL SNativeWnd::DestroyWindow()
{
    SASSERT(::IsWindow(m_hWnd));
    return ::DestroyWindow(m_hWnd);
}

BOOL SNativeWnd::SetWindowPos(HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::SetWindowPos(m_hWnd, hWndInsertAfter, x, y, cx, cy, nFlags);
}

BOOL SNativeWnd::IsWindowEnabled() const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::IsWindowEnabled(m_hWnd);
}

HWND SNativeWnd::SetParent(HWND hWndNewParent)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::SetParent(m_hWnd, hWndNewParent);
}

HWND SNativeWnd::GetParent()
{
    SASSERT(::IsWindow(m_hWnd));
    return ::GetParent(m_hWnd);
}

LONG_PTR SNativeWnd::SetWindowLongPtr(int nIndex, LONG_PTR dwNewLong)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::SetWindowLongPtr(m_hWnd, nIndex, dwNewLong);
}

LONG_PTR SNativeWnd::GetWindowLongPtr(int nIndex) const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::GetWindowLongPtr(m_hWnd, nIndex);
}

DWORD SNativeWnd::GetExStyle() const
{
    SASSERT(::IsWindow(m_hWnd));
    return (DWORD)::GetWindowLongPtr(m_hWnd, GWL_EXSTYLE);
}

DWORD SNativeWnd::GetStyle() const
{
    SASSERT(::IsWindow(m_hWnd));
    return (DWORD)::GetWindowLongPtr(m_hWnd, GWL_STYLE);
}

int SNativeWnd::GetDlgCtrlID() const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::GetDlgCtrlID(m_hWnd);
}

const MSG *SNativeWnd::GetCurrentMessage() const
{
    return m_pCurrentMsg;
}

void SNativeWnd::SetMsgHandler(THIS_ FunMsgHandler fun, void *ctx)
{
    m_msgHandlerInfo.fun = fun;
    m_msgHandlerInfo.ctx = ctx;
}

MsgHandlerInfo *SNativeWnd::GetMsgHandler()
{
    return &m_msgHandlerInfo;
}

SNSEND