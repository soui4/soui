#include "stdafx.h"
#include "SWkeWebkit.h"
#include <imm.h>
#include <helper/slog.h>
#define kLogTag "wke"
#ifdef _WIN32
#pragma comment(lib, "imm32.lib")
#pragma comment(lib, "msimg32.lib")
#endif

SNSBEGIN

//////////////////////////////////////////////////////////////////////////


//wkeString -> wchar_t*，通过 SWkeLoader 里动态解析的函数指针完成
static const wchar_t *WkeStringToW(const wkeString str)
{
    SWkeLoader *pLoader = SWkeLoader::GetInstance();
    if (!pLoader || !pLoader->m_funWkeToStringW || !str)
        return NULL;
    return pLoader->m_funWkeToStringW(str);
}

//标题变化回调
static void WkeOnTitleChanged(const wkeClientHandler *pHandler, const wkeString title)
{
    const wchar_t *psz = WkeStringToW(title);
    SLOGI()<<"wke "<<pHandler<< "title changed : "<< psz;
}

//地址变化回调
static void WkeOnURLChanged(const wkeClientHandler *pHandler, const wkeString url)
{
    const wchar_t *psz = WkeStringToW(url);
	SLOGI()<<"wke "<<pHandler<< "url changed : "<< psz;
}

//////////////////////////////////////////////////////////////////////////
// SWkeLoader
SWkeLoader *SWkeLoader::s_pInst = 0;

SWkeLoader *SWkeLoader::GetInstance()
{
    return s_pInst;
}

SWkeLoader::SWkeLoader()
    : m_hModWke(0)
{
    m_funWkeToStringW = NULL;
    m_funWkeToString = NULL;
    SASSERT(!s_pInst);
    s_pInst = this;
}

SWkeLoader::~SWkeLoader()
{
    if (m_hModWke)
        FreeLibrary(m_hModWke);
}

BOOL SWkeLoader::Init(LPCTSTR pszDll)
{
    if (m_hModWke)
        return TRUE;
    HMODULE hModWke = LoadLibrary(pszDll);
    if (!hModWke)
        return FALSE;
    m_funWkeInit = (FunWkeInit)GetProcAddress(hModWke, "wkeInit");
    m_funWkeShutdown = (FunWkeShutdown)GetProcAddress(hModWke, "wkeShutdown");
    m_funWkeCreateWebView = (FunWkeCreateWebView)GetProcAddress(hModWke, "wkeCreateWebView");
    m_funWkeDestroyWebView = (FunWkeDestroyWebView)GetProcAddress(hModWke, "wkeDestroyWebView");
    //可选接口：老版本 wke.dll 不一定导出，取不到只影响日志里字符串的显示
    m_funWkeToStringW = (FunWkeToStringW)GetProcAddress(hModWke, "wkeToStringW");
    m_funWkeToString = (FunWkeToString)GetProcAddress(hModWke, "wkeToString");
    if (!m_funWkeInit || !m_funWkeShutdown || !m_funWkeCreateWebView || !m_funWkeDestroyWebView)
    {
        FreeLibrary(hModWke);
        return FALSE;
    }
    m_funWkeInit();
    m_hModWke = hModWke;
    return TRUE;
}

BOOL SWkeLoader::IsLoaded() const
{
    return m_hModWke != NULL;
}
//////////////////////////////////////////////////////////////////////////
// SWkeWebkit

SWkeWebkit::SWkeWebkit(void)
    : m_pWebView(NULL)
    , m_bLoggedLoadFailed(FALSE)
    , m_bLoggedLoadComplete(FALSE)
    , m_nLoggedContentsW(0)
    , m_nLoggedContentsH(0)
{
    m_clientHandler.onTitleChanged = &WkeOnTitleChanged;
    m_clientHandler.onURLChanged = &WkeOnURLChanged;
    m_bFocusable = true;
}

SWkeWebkit::~SWkeWebkit(void)
{
}

void SWkeWebkit::OnPaint(IRenderTarget *pRT)
{
    CRect rcClip;
    pRT->GetClipBox(&rcClip);
    CRect rcClient;
    GetClientRect(&rcClient);
    CRect rcInvalid;
    rcInvalid.IntersectRect(&rcClip, &rcClient);
    HDC hdc = pRT->GetDC(0);
    if (GetAlpha() != 0xff)
    {
        BLENDFUNCTION bf = { AC_SRC_OVER, GetAlpha(), AC_SRC_ALPHA };
        AlphaBlend(hdc, rcInvalid.left, rcInvalid.top, rcInvalid.Width(), rcInvalid.Height(),
                   m_pWebView->getViewDC(), rcInvalid.left - rcClient.left,
                   rcInvalid.top - rcClient.top, rcInvalid.Width(), rcInvalid.Height(), bf);
    }
    else
    {
        BitBlt(hdc, rcInvalid.left, rcInvalid.top, rcInvalid.Width(), rcInvalid.Height(),
               m_pWebView->getViewDC(), rcInvalid.left - rcClient.left,
               rcInvalid.top - rcClient.top, SRCCOPY);
    }
    pRT->ReleaseDC(hdc,&rcInvalid);
}

void SWkeWebkit::OnSize(UINT nType, CSize size)
{
    __baseCls::OnSize(nType, size);
    m_pWebView->resize(size.cx, size.cy);
    m_pWebView->tick();
}

int SWkeWebkit::OnCreate(void *)
{
    SWkeLoader *pWkeLoader = SWkeLoader::GetInstance();
    if (!pWkeLoader || !pWkeLoader->IsLoaded())
        return 1;
    m_pWebView = pWkeLoader->m_funWkeCreateWebView();
    if (!m_pWebView)
        return 1;
    //注册客户端回调：标题/地址变化会打到调试日志，用于定位 wke 加载问题
    m_pWebView->setClientHandler(&m_clientHandler);
    m_pWebView->setBufHandler(this);
    SLOGI()<<"wke "<<(const void *)&m_clientHandler<<  "created, loadURL : "<< m_strUrl.c_str();
    m_pWebView->loadURL(m_strUrl);
    SetTimer(
        TM_TICKER,
        50); //由于timer不够及时，idle又限制了只在当前的消息循环中有效，使用timer和onidle一起更新浏览器
    return 0;
}

void SWkeWebkit::OnDestroy()
{
    if (m_pWebView)
    {
        m_pWebView->setClientHandler(NULL); //先注销回调，避免销毁过程中再次回调
        SWkeLoader::GetInstance()->m_funWkeDestroyWebView(m_pWebView);
    }
    __baseCls::OnDestroy();
}

LRESULT SWkeWebkit::OnMouseEvent(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_LBUTTONDOWN || message == WM_MBUTTONDOWN || message == WM_RBUTTONDOWN)
    {
        SetFocus();
        SetCapture();
    }
    else if (message == WM_LBUTTONUP || message == WM_MBUTTONUP || message == WM_RBUTTONUP)
    {
        ReleaseCapture();
    }

    CRect rcClient;
    GetClientRect(&rcClient);

    int x = GET_X_LPARAM(lParam) - rcClient.left;
    int y = GET_Y_LPARAM(lParam) - rcClient.top;

    unsigned int flags = 0;

    if (wParam & MK_CONTROL)
        flags |= WKE_CONTROL;
    if (wParam & MK_SHIFT)
        flags |= WKE_SHIFT;

    if (wParam & MK_LBUTTON)
        flags |= WKE_LBUTTON;
    if (wParam & MK_MBUTTON)
        flags |= WKE_MBUTTON;
    if (wParam & MK_RBUTTON)
        flags |= WKE_RBUTTON;

    bool bHandled = m_pWebView->mouseEvent(message, x, y, flags);
    SetMsgHandled(bHandled);
    return 0;
}

LRESULT SWkeWebkit::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    unsigned int flags = 0;
    if (HIWORD(lParam) & KF_REPEAT)
        flags |= WKE_REPEAT;
    if (HIWORD(lParam) & KF_EXTENDED)
        flags |= WKE_EXTENDED;

    SetMsgHandled(m_pWebView->keyDown(wParam, flags, false));
    return 0;
}

LRESULT SWkeWebkit::OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    unsigned int flags = 0;
    if (HIWORD(lParam) & KF_REPEAT)
        flags |= WKE_REPEAT;
    if (HIWORD(lParam) & KF_EXTENDED)
        flags |= WKE_EXTENDED;

    SetMsgHandled(m_pWebView->keyUp(wParam, flags, false));
    return 0;
}

LRESULT SWkeWebkit::OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    POINT pt;
    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);

    CRect rc;
    GetWindowRect(&rc);
    pt.x -= rc.left;
    pt.y -= rc.top;

    int delta = GET_WHEEL_DELTA_WPARAM(wParam);

    unsigned int flags = 0;

    if (wParam & MK_CONTROL)
        flags |= WKE_CONTROL;
    if (wParam & MK_SHIFT)
        flags |= WKE_SHIFT;

    if (wParam & MK_LBUTTON)
        flags |= WKE_LBUTTON;
    if (wParam & MK_MBUTTON)
        flags |= WKE_MBUTTON;
    if (wParam & MK_RBUTTON)
        flags |= WKE_RBUTTON;

    // flags = wParam;

    BOOL handled = m_pWebView->mouseWheel(pt.x, pt.y, delta, flags);
    SetMsgHandled(handled);

    return handled;
}

LRESULT SWkeWebkit::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    unsigned int charCode = wParam;
    unsigned int flags = 0;
    if (HIWORD(lParam) & KF_REPEAT)
        flags |= WKE_REPEAT;
    if (HIWORD(lParam) & KF_EXTENDED)
        flags |= WKE_EXTENDED;

    // flags = HIWORD(lParam);

    SetMsgHandled(m_pWebView->keyPress(charCode, flags, false));
    return 0;
}

LRESULT SWkeWebkit::OnImeStartComposition(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    wkeRect caret = m_pWebView->getCaret();

    CRect rcClient;
    GetClientRect(&rcClient);

    CANDIDATEFORM form;
    form.dwIndex = 0;
    form.dwStyle = CFS_EXCLUDE;
    form.ptCurrentPos.x = caret.x + rcClient.left;
    form.ptCurrentPos.y = caret.y + caret.h + rcClient.top;
    form.rcArea.top = caret.y + rcClient.top;
    form.rcArea.bottom = caret.y + caret.h + rcClient.top;
    form.rcArea.left = caret.x + rcClient.left;
    form.rcArea.right = caret.x + caret.w + rcClient.left;
    COMPOSITIONFORM compForm;
    compForm.ptCurrentPos = form.ptCurrentPos;
    compForm.rcArea = form.rcArea;
    compForm.dwStyle = CFS_POINT;

    HWND hWnd = GetContainer()->GetHostHwnd();
    HIMC hIMC = ImmGetContext(hWnd);
    ImmSetCandidateWindow(hIMC, &form);
    ImmSetCompositionWindow(hIMC, &compForm);
    ImmReleaseContext(hWnd, hIMC);
    return 0;
}

void SWkeWebkit::OnSetFocus(SWND wndOld)
{
    __baseCls::OnSetCursor(wndOld);
    m_pWebView->focus();
}

void SWkeWebkit::OnKillFocus(SWND wndFocus)
{
    m_pWebView->unfocus();
    __baseCls::OnKillFocus(wndFocus);
}

void SWkeWebkit::OnTimer(char cTimerID)
{
    if (cTimerID == TM_TICKER)
    {
        m_pWebView->tick();
        OnWkeTickLog();
    }
}

//加载状态只在变化时打一条日志，用于判断 wke 到底卡在哪一步（请求/加载/布局）
void SWkeWebkit::OnWkeTickLog()
{
    if (!m_pWebView)
        return;
    if (!m_bLoggedLoadFailed && m_pWebView->isLoadFailed())
    {
        m_bLoggedLoadFailed = TRUE;
        SLOGW()<<"wke "<<(const void *)&m_clientHandler<< " load FAILED";
    }
    if (!m_bLoggedLoadComplete && m_pWebView->isLoadComplete())
    {
        m_bLoggedLoadComplete = TRUE;
		SLOGW()<<"wke "<<(const void *)&m_clientHandler<< " load COMPLETE";
    }
    int nW = m_pWebView->contentsWidth();
    int nH = m_pWebView->contentsHeight();
    if (nW != m_nLoggedContentsW || nH != m_nLoggedContentsH)
    {
        m_nLoggedContentsW = nW;
        m_nLoggedContentsH = nH;
		SLOGW()<<"wke "<<(const void *)&m_clientHandler<< " contents="<<nW<<"x"<<nH<<" complete="<<m_pWebView->isLoadComplete()
			<<" failed="<<m_pWebView->isLoadFailed()<<" loaded="<<m_pWebView->isLoaded()<<" ready="<<m_pWebView->isDocumentReady();
    }
}

void SWkeWebkit::onBufUpdated(const HDC hdc, int x, int y, int cx, int cy)
{
    CRect rcClient;
    GetClientRect(&rcClient);
    CRect rcInvalid(CPoint(x, y), CSize(cx, cy));
    rcInvalid.OffsetRect(rcClient.TopLeft());
    InvalidateRect(rcInvalid);
}

BOOL SWkeWebkit::OnIdle(int iRun)
{
    m_pWebView->tick();
    return TRUE;
}

BOOL SWkeWebkit::OnSetCursor(const CPoint &pt)
{
    return TRUE;
}

BOOL SWkeWebkit::OnAttrUrl(SStringW strValue, BOOL bLoading)
{
    m_strUrl = strValue;
    if (!bLoading)
        m_pWebView->loadURL(m_strUrl);
    return !bLoading;
}

SNSEND
