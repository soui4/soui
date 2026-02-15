#include "souistd.h"
#include "control/SRichEdit.h"
#include "SApp.h"
#include "helper/SMenu.h"
#include "helper/SplitString.h"
#include "helper/SAutoBuf.h"
#include "string/tstring.h"
#include <gdialpha.h>
#include <helper/STimer.h>
#include <helper/SUnkImpl.h>
#ifndef LY_PER_INCH
#define LY_PER_INCH 1440
#endif

#ifndef HIMETRIC_PER_INCH
#define HIMETRIC_PER_INCH 2540
#endif

#define USE_MSFTEDIT

#ifdef INIT_RICHEDIT
extern "C"
{
    STDAPI InitRichedit(HINSTANCE hInst);
    STDAPI UninitRichedit(HINSTANCE hInst);
}
#endif // INIT_RICHEDIT

SNSBEGIN
//////////////////////////////////////////////////////////////////////////
//  STextServiceHelper
/**
 * @class      STextServiceHelper
 * @brief
 *
 * Describe
 */
class STextServiceHelper *s_textServiceHelper = NULL;

class STextServiceHelper {
    friend class SRichEdit;

  public:
    static STextServiceHelper *instance()
    {
        SASSERT(s_textServiceHelper);
        return s_textServiceHelper;
    }

  public:
    /**
     * STextServiceHelper::CreateTextServices
     * @brief
     * @param  IUnknown *punkOuter
     * @param  ITextHost *pITextHost
     * @param  IUnknown **ppUnk
     * @return 返回HRESULT
     *
     * Describe
     */
    HRESULT CreateTextServices(IUnknown *punkOuter, ITextHost *pITextHost, IUnknown **ppUnk);

  protected:
    /**
     * STextServiceHelper::STextServiceHelper
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    STextServiceHelper();
    /**
     * STextServiceHelper::~STextServiceHelper
     * @brief    析构函数
     *
     * Describe  析构函数
     */
    ~STextServiceHelper();

    HINSTANCE m_rich20;                          /**< richedit module */
    PCreateTextServices m_funCreateTextServices; /**< 回调函数 */
};

STextServiceHelper::STextServiceHelper()
    : m_funCreateTextServices(NULL)
{
#ifdef INIT_RICHEDIT
    m_rich20 = NULL;
    ::InitRichedit(GetModuleHandle(NULL));
    m_funCreateTextServices = ::CreateTextServices;
    return;
#else // INIT_RICHEDIT
#ifdef _WIN32
    m_rich20 = LoadLibrary(_T("Msftedit.dll"));
#else
    m_rich20 = LoadLibrary(_T("libmsftedit"));
#endif

    if (m_rich20)
    {
        m_funCreateTextServices = (PCreateTextServices)GetProcAddress(m_rich20, "CreateTextServices");
    }
    else
    {
#ifndef _WIN32
        const char *err = dlerror();
        printf("load so failed, err=%s\n", err);
#endif
    }
#endif // INIT_RICHEDIT
}

STextServiceHelper::~STextServiceHelper()
{
#ifdef INIT_RICHEDIT
    ::UninitRichedit(GetModuleHandle(NULL));
#else
    if (m_rich20)
        FreeLibrary(m_rich20);
#endif // INIT_RICHEDIT
    m_funCreateTextServices = NULL;
}

HRESULT STextServiceHelper::CreateTextServices(IUnknown *punkOuter, ITextHost *pITextHost, IUnknown **ppUnk)
{
    if (!m_funCreateTextServices)
        return E_NOTIMPL;
    return m_funCreateTextServices(punkOuter, pITextHost, ppUnk);
}

void SRichEdit::InitTextService()
{
    SASSERT(s_textServiceHelper == NULL);
    s_textServiceHelper = new STextServiceHelper;
}

void SRichEdit::UninitTextService()
{
    SASSERT(s_textServiceHelper);
    delete s_textServiceHelper;
    s_textServiceHelper = NULL;
}

//////////////////////////////////////////////////////////////////////////
//
class SRicheditDropTarget : public SUnkImpl<IDropTarget> {
  public:
    SRicheditDropTarget(ITextServices *pTxtSvr)
        : pserv(pTxtSvr)
    {
        SASSERT(pserv);
        pserv->AddRef();
    }

    ~SRicheditDropTarget()
    {
        SASSERT(pserv);
        pserv->Release();
    }

    IUNKNOWN_BEGIN(IDropTarget)
    IUNKNOWN_END()

    // IDropTarget
    STDMETHOD(DragEnter)
    (
        /* [unique][in] */ IDataObject *pDataObj,
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ DWORD *pdwEffect)
    {
        HRESULT hr = S_FALSE;
        IDropTarget *pDropTarget = NULL;
        hr = pserv->TxGetDropTarget(&pDropTarget);
        if (SUCCEEDED(hr))
        {
            hr = pDropTarget->DragEnter(pDataObj, grfKeyState, pt, pdwEffect);
            *pdwEffect = DROPEFFECT_COPY;
            pDropTarget->Release();
        }
        return hr;
    }

    STDMETHOD(DragOver)
    (
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ DWORD *pdwEffect)
    {
        HRESULT hr = S_FALSE;
        IDropTarget *pDropTarget = NULL;
        hr = pserv->TxGetDropTarget(&pDropTarget);
        if (SUCCEEDED(hr))
        {
            hr = pDropTarget->DragOver(grfKeyState, pt, pdwEffect);
            *pdwEffect = DROPEFFECT_COPY;
            pDropTarget->Release();
        }
        return hr;
    }

    STDMETHOD(DragLeave)(void)
    {
        HRESULT hr = S_FALSE;
        IDropTarget *pDropTarget = NULL;
        hr = pserv->TxGetDropTarget(&pDropTarget);
        if (SUCCEEDED(hr))
        {
            hr = pDropTarget->DragLeave();
            pDropTarget->Release();
        }
        return hr;
    }

    STDMETHOD(Drop)
    (
        /* [unique][in] */ IDataObject *pDataObj,
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ DWORD *pdwEffect)
    {
        if (*pdwEffect == DROPEFFECT_NONE)
            return S_FALSE;
        HRESULT hr = S_FALSE;
        IDropTarget *pDropTarget = NULL;
        hr = pserv->TxGetDropTarget(&pDropTarget);
        if (SUCCEEDED(hr))
        {
            hr = pDropTarget->Drop(pDataObj, grfKeyState, pt, pdwEffect);
            pDropTarget->Release();
        }
        return hr;
    }

  protected:
    ITextServices *pserv; // pointer to Text Services object
};

const LONG cInitTextMax = (32 * 1024) - 1;
#define FValidCF(_pcf)   ((_pcf)->cbSize == sizeof(CHARFORMAT2W))
#define FValidPF(_ppf)   ((_ppf)->cbSize == sizeof(PARAFORMAT2))
#define TIMER_INVALIDATE 6

EXTERN_C SELECT_ANY const IID IID_ITextServices = // 8d33f740-cf58-11ce-a89d-00aa006cadc5
    { 0x8d33f740, 0xcf58, 0x11ce, { 0xa8, 0x9d, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5 } };

EXTERN_C SELECT_ANY const IID IID_ITextHost = /* c5bdd8d0-d26e-11ce-a89e-00aa006cadc5 */
    { 0xc5bdd8d0, 0xd26e, 0x11ce, { 0xa8, 0x9e, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5 } };

// Convert Device Pixels to Himetric
LONG DtoHimetric(LONG d, LONG dPerInch)
{
    return (LONG)MulDiv(d, HIMETRIC_PER_INCH, dPerInch);
}

// Convert Himetric Device pixels
LONG HimetrictoD(LONG lHimetric, LONG dPerInch)
{
    return (LONG)MulDiv(lHimetric, dPerInch, HIMETRIC_PER_INCH);
}

/**
 * @class      STextHost
 * @brief
 *
 * Describe
 */
class STextHost : public SUnkImpl<ITextHost> {
    friend class SRichEdit;

  public:
    /**
     * STextHost::STextHost
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    STextHost(void);
    /**
     * STextHost::~STextHost
     * @brief    析构函数
     *
     * Describe  析构函数
     */
    ~STextHost(void);
    /**
     * STextHost::Init
     * @brief    初始化函数
     * @param    SRichEdit* pRichEdit -- SRichEdit对象
     *
     * Describe  初始化函数
     */
    BOOL Init(SRichEdit *pRichEdit);
    /**
     * STextHost::GetTextService
     * @brief
     *
     * Describe
     */
    ITextServices *GetTextService()
    {
        return pserv;
    }

  protected:
    IUNKNOWN_BEGIN(ITextHost)
    IUNKNOWN_END()

    /**
     * STextHost::TxGetDC
     * @brief     Get the DC for the host
     * @return    返回HDC
     *
     * Describe   Get the DC for the host
     */
    virtual HDC TxGetDC();

    /**
     * STextHost::TxReleaseDC
     * @brief     Release the DC gotten from the host
     * @return    返回INT
     *
     * Describe   Release the DC gotten from the host
     */
    virtual INT TxReleaseDC(HDC hdc);

    /**
     * STextHost::TxShowScrollBar
     * @brief     Show the scroll bar
     * @param     INT fnBar --
     * @param     BOOL fShow --
     * @return    返回BOOL
     *
     * Describe   Show the scroll bar
     */
    virtual BOOL TxShowScrollBar(INT fnBar, BOOL fShow);

    /**
     * STextHost::TxEnableScrollBar
     * @brief     Enable the scroll bar
     * @param     INT fuSBFlags --
     * @param     INT fuArrowflags --
     * @return    返回BOOL
     *
     * Describe   Enable the scroll bar
     */
    virtual BOOL TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags);

    /**
     * STextHost::TxEnableScrollBar
     * @brief     Set the scroll range
     * @param     INT fnBar --
     * @param     LONG nMinPos --
     * @param     INT nMaxPos --
     * @param     BOOL fRedraw --
     * @return    返回BOOL
     *
     * Describe   Set the scroll range
     */
    virtual BOOL TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw);

    /**
     * STextHost::TxSetScrollPos
     * @brief     Set the scroll position
     * @param     INT fnBar --
     * @param     INT nPos --
     * @param     BOOL fRedraw --
     * @return    返回BOOL
     *
     * Describe   Set the scroll position
     */
    virtual BOOL TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw);

    /**
     * STextHost::TxInvalidateRect
     * @brief     InvalidateRect
     * @param     LPCRECT prc --
     * @param     BOOL fMode --
     *
     * Describe   Set the scroll position
     */
    virtual void TxInvalidateRect(LPCRECT prc, BOOL fMode);

    /**
     * STextHost::TxViewChange
     * @brief     Send a WM_PAINT to the window
     * @param     BOOL fUpdate --
     *
     * Describe   Send a WM_PAINT to the window
     */
    virtual void TxViewChange(BOOL fUpdate);

    /**
     * STextHost::TxCreateCaret
     * @brief     Create the caret
     * @param     HBITMAP hbmp -- caret bitmap
     * @param     INT xWidth -- caret width
     * @param     INT yHeight -- caret height
     * @return    返回BOOL
     *
     * Describe   Create the caret
     */
    virtual BOOL TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight);

    /**
     * STextHost::TxShowCaret
     * @brief     Show the caret
     * @param     BOOL fShow -- true to show the caret
     * @return    返回BOOL
     *
     * Describe   Show the caret
     */
    virtual BOOL TxShowCaret(BOOL fShow);

    /**
     * STextHost::TxSetCaretPos
     * @brief     Set the caret position
     * @param     INT x -- caret position:x
     * @param     INT y -- caret position:y
     * @return    返回BOOL
     *
     * Describe   Set the caret position
     */
    virtual BOOL TxSetCaretPos(INT x, INT y);

    /**
     * STextHost::TxSetTimer
     * @brief     Create a timer with the specified timeout
     * @param     UINT idTimer -- timer ID
     * @param     UINT uTimeout -- time interval
     * @return    返回BOOL
     *
     * Describe   Create a timer with the specified timeout
     */
    virtual BOOL TxSetTimer(UINT idTimer, UINT uTimeout);

    /**
     * STextHost::TxSetTimer
     * @brief     Destroy a timer
     * @param     UINT idTimer -- timer id
     * @return    返回BOOL
     *
     * Describe   Destroy a timer
     */
    virtual void TxKillTimer(UINT idTimer);

    /**
     * STextHost::TxScrollWindowEx
     * @brief     Scroll the content of the specified window's client area
     * @param     INT dx --
     * @param     INT dy --
     * @param     LPCRECT lprcScroll --
     * @param     LPCRECT lprcClip --
     * @param     HRGN hrgnUpdate --
     * @param     LPRECT lprcUpdate --
     * @param     UINT fuScroll --
     *
     * Describe   Scroll the content of the specified window's client area
     */
    virtual void TxScrollWindowEx(INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll);

    /**
     * STextHost::TxSetCapture
     * @brief     Get mouse capture
     * @param     BOOL fCapture --
     *
     * Describe   Get mouse capture
     */
    virtual void TxSetCapture(BOOL fCapture);

    /**
     * STextHost::TxSetFocus
     * @brief     Set the focus to the text window
     *
     * Describe   Set the focus to the text window
     */
    virtual void TxSetFocus();

    /**
     * STextHost::TxSetCursor
     * @brief     Establish a new cursor shape
     * @param     HCURSOR hcur --
     * @param     BOOL fText --
     *
     * Describe   Establish a new cursor shape
     */
    virtual void TxSetCursor(HCURSOR hcur, BOOL fText);

    /**
     * STextHost::TxScreenToClient
     * @brief     Converts screen coordinates of a specified point to the client coordinates
     * @param     LPPOINT lppt --
     * @return    返回BOOL
     *
     * Describe   Converts screen coordinates of a specified point to the client coordinates
     */
    virtual BOOL TxScreenToClient(LPPOINT lppt);

    /**
     * STextHost::TxClientToScreen
     * @brief     Converts the client coordinates of a specified point to screen coordinates
     * @param     LPPOINT lppt --
     * @return    返回BOOL
     *
     * Describe   Converts the client coordinates of a specified point to screen coordinates
     */
    virtual BOOL TxClientToScreen(LPPOINT lppt);

    /**
     * STextHost::TxActivate
     * @brief     Request host to activate text services
     * @param     LONG * plOldState --
     * @return    返回HRESULT
     *
     * Describe   Request host to activate text services
     */
    virtual HRESULT TxActivate(LONG *plOldState);

    /**
     * STextHost::TxDeactivate
     * @brief     Request host to deactivate text services
     * @param     LONG lNewState --
     * @return    返回HRESULT
     *
     * Describe   Request host to deactivate text services
     */
    virtual HRESULT TxDeactivate(LONG lNewState);

    /**
     * STextHost::TxGetClientRect
     * @brief     Retrieves the coordinates of a window's client area
     * @param     LPRECT prc --
     * @return    返回HRESULT
     *
     * Describe   Retrieves the coordinates of a window's client area
     */
    virtual HRESULT TxGetClientRect(LPRECT prc);

    /**
     * STextHost::TxGetViewInset
     * @brief     Get the view rectangle relative to the inset
     * @param     LPRECT prc --
     * @return    返回HRESULT
     *
     * Describe   Get the view rectangle relative to the inset
     */
    virtual HRESULT TxGetViewInset(LPRECT prc);

    /**
     * STextHost::TxGetCharFormat
     * @brief     Get the default character format for the text
     * @param     const CHARFORMATW **ppCF --
     * @return    返回HRESULT
     *
     * Describe   Get the default character format for the text
     */
    virtual HRESULT TxGetCharFormat(const CHARFORMATW **ppCF);

    /**
     * STextHost::TxGetParaFormat
     * @brief     Get the default paragraph format for the text
     * @param     const PARAFORMAT **ppPF --
     * @return    返回HRESULT
     *
     * Describe   Get the default character format for the text
     */
    virtual HRESULT TxGetParaFormat(const PARAFORMAT **ppPF);

    /**
     * STextHost::TxGetSysColor
     * @brief     Get the background color for the window
     * @param     int nIndex --
     * @return    返回COLORREF
     *
     * Describe   Get the background color for the window
     */
    virtual COLORREF TxGetSysColor(int nIndex);

    /**
     * STextHost::TxGetBackStyle
     * @brief     Get the background (either opaque or transparent)
     * @param     TXTBACKSTYLE *pstyle --
     * @return    返回HRESULT
     *
     * Describe   Get the background (either opaque or transparent)
     */
    virtual HRESULT TxGetBackStyle(TXTBACKSTYLE *pstyle);

    /**
     * STextHost::TxGetMaxLength
     * @brief     Get the maximum length for the text
     * @param     DWORD *plength --
     * @return    返回HRESULT
     *
     * Describe   Get the maximum length for the text
     */
    virtual HRESULT TxGetMaxLength(DWORD *plength);

    /**
     * STextHost::TxGetScrollBars
     * @brief     Get the bits representing requested scroll bars for the window
     * @param     DWORD *pdwScrollBar --
     * @return    返回HRESULT
     *
     * Describe   Get the bits representing requested scroll bars for the window
     */
    virtual HRESULT TxGetScrollBars(DWORD *pdwScrollBar);

    /**
     * STextHost::TxGetPasswordChar
     * @brief     Get the character to display for password input
     * @param     TCHAR *pch --
     * @return    返回HRESULT
     *
     * Describe   Get the character to display for password input
     */
    virtual HRESULT TxGetPasswordChar(TCHAR *pch);

    /**
     * STextHost::TxGetAcceleratorPos
     * @brief     Get the accelerator character
     * @param     LONG *pcp --
     * @return    返回HRESULT
     *
     * Describe   Get the accelerator character
     */
    virtual HRESULT TxGetAcceleratorPos(LONG *pcp);

    /**
     * STextHost::TxGetExtent
     * @brief     Get the native size
     * @param     LPSIZEL lpExtent --
     * @return    返回HRESULT
     *
     * Describe   Get the native size
     */
    virtual HRESULT TxGetExtent(LPSIZEL lpExtent);

    /**
     * STextHost::OnTxCharFormatChange
     * @brief     Notify host that default character format has changed
     * @param     const CHARFORMATW * pcf --
     * @return    返回HRESULT
     *
     * Describe   Notify host that default character format has changed
     */
    virtual HRESULT OnTxCharFormatChange(const CHARFORMATW *pcf);

    /**
     * STextHost::OnTxParaFormatChange
     * @brief     Notify host that default paragraph format has changed
     * @param     const PARAFORMAT * ppf --
     * @return    返回HRESULT
     *
     * Describe   Notify host that default paragraph format has changed
     */
    virtual HRESULT OnTxParaFormatChange(const PARAFORMAT *ppf);

    /**
     * STextHost::TxGetPropertyBits
     * @brief     Bulk access to bit properties
     * @param     DWORD dwMask --
     * @param     DWORD *pdwBits --
     * @return    返回HRESULT
     *
     * Describe   Bulk access to bit properties
     */
    virtual HRESULT TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits);

    /**
     * STextHost::TxNotify
     * @brief     Notify host of events
     * @param     DWORD iNotify  --
     * @param     void *pv --
     * @return    返回HRESULT
     *
     * Describe   Bulk access to bit properties
     */
    virtual HRESULT TxNotify(DWORD iNotify, void *pv);

    // Far East Methods for getting the Input Context
    //#ifdef WIN95_IME
    /**
     * STextHost::TxImmGetContext
     * @brief
     * @return     返回HIMC
     *
     * Describe
     */
    virtual HIMC TxImmGetContext();
    /**
     * STextHost::TxImmReleaseContext
     * @brief
     * @param     HIMC himc  --
     *
     * Describe
     */
    virtual void TxImmReleaseContext(HIMC himc);
    //#endif

    /**
     * STextHost::TxGetSelectionBarWidth
     * @brief     Returns HIMETRIC size of the control bar
     * @param     LONG *plSelBarWidth  --
     *
     * Describe   Returns HIMETRIC size of the control bar
     */
    virtual HRESULT TxGetSelectionBarWidth(LONG *plSelBarWidth);

  protected:
    BOOL m_fUiActive;       /**< Whether control is inplace active */
    ITextServices *pserv;   /**< pointer to Text Services object */
    SRichEdit *m_pRichEdit; /**< swindow for text host */
    POINT m_ptCaret;
};

STextHost::STextHost(void)
    : m_pRichEdit(NULL)
    , m_fUiActive(FALSE)
    , pserv(NULL)
{
    m_ptCaret.x = m_ptCaret.y = -1;
}

STextHost::~STextHost(void)
{
    if (pserv)
        pserv->Release();
}

//////////////////////////////////////////////////////////////////////////
// ITextHost
HRESULT STextHost::TxGetViewInset(LPRECT prc)
{
    *prc = m_pRichEdit->m_rcInset;
    return S_OK;
}

HRESULT STextHost::TxGetCharFormat(const CHARFORMATW **ppCF)
{
    *ppCF = &m_pRichEdit->m_cfDef;
    return S_OK;
}

HRESULT STextHost::TxGetParaFormat(const PARAFORMAT **ppPF)
{
    *ppPF = &m_pRichEdit->m_pfDef;
    return S_OK;
}

HRESULT STextHost::TxGetClientRect(LPRECT prc)
{
    m_pRichEdit->GetClientRect(prc);
    return S_OK;
}

HRESULT STextHost::TxDeactivate(LONG lNewState)
{
    m_fUiActive = FALSE;
    return S_OK;
}

HRESULT STextHost::TxActivate(LONG *plOldState)
{
    *plOldState = m_fUiActive;
    m_fUiActive = TRUE;
    return S_OK;
}

BOOL STextHost::TxClientToScreen(LPPOINT lppt)
{
    RECT rc = { 0 };
    m_pRichEdit->GetContainer()->FrameToHost(&rc);
    lppt->x += rc.left;
    lppt->y += rc.top;
    return ::ClientToScreen(m_pRichEdit->GetContainer()->GetHostHwnd(), lppt);
}

BOOL STextHost::TxScreenToClient(LPPOINT lppt)
{
    RECT rc = { 0 };
    m_pRichEdit->GetContainer()->FrameToHost(&rc);
    lppt->x -= rc.left;
    lppt->y -= rc.top;
    return ::ScreenToClient(m_pRichEdit->GetContainer()->GetHostHwnd(), lppt);
}

void STextHost::TxSetCursor(HCURSOR hcur, BOOL fText)
{
    ::SetCursor(hcur);
}

void STextHost::TxSetFocus()
{
    m_pRichEdit->SetFocus();
}

void STextHost::TxSetCapture(BOOL fCapture)
{
    if (fCapture)
        m_pRichEdit->SetCapture();
    else
        m_pRichEdit->ReleaseCapture();
}

void STextHost::TxScrollWindowEx(INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll)
{
    m_pRichEdit->Invalidate();
}

void STextHost::TxKillTimer(UINT idTimer)
{
    m_pRichEdit->OnTxKillTimer(idTimer);
}

BOOL STextHost::TxSetTimer(UINT idTimer, UINT uTimeout)
{
    return m_pRichEdit->OnTxSetTimer(idTimer, uTimeout);
}

BOOL STextHost::TxSetCaretPos(INT x, INT y)
{
    m_ptCaret.x = x, m_ptCaret.y = y;
    m_pRichEdit->SetCaretPos(x, y);
    return TRUE;
}

BOOL STextHost::TxShowCaret(BOOL fShow)
{
    if (fShow && !m_fUiActive)
        return FALSE;
    m_pRichEdit->ShowCaret(fShow);
    return TRUE;
}

BOOL STextHost::TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight)
{
    return m_pRichEdit->CreateCaret(hbmp, xWidth, yHeight);
}

HDC STextHost::TxGetDC()
{
    return ::GetDC(NULL);
}

INT STextHost::TxReleaseDC(HDC hdc)
{
    return ::ReleaseDC(NULL, hdc);
}

BOOL STextHost::TxShowScrollBar(INT fnBar, BOOL fShow)
{
    int wBar = 0;
    switch (fnBar)
    {
    case SB_BOTH:
        wBar = SSB_BOTH;
        break;
    case SB_VERT:
        wBar = SSB_VERT;
        break;
    case SB_HORZ:
        wBar = SSB_HORZ;
        break;
    }
    return m_pRichEdit->ShowScrollBar(wBar, fShow);
}

BOOL STextHost::TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags)
{
    int wBar = 0;
    switch (fuSBFlags)
    {
    case SB_BOTH:
        wBar = SSB_BOTH;
        break;
    case SB_VERT:
        wBar = SSB_VERT;
        break;
    case SB_HORZ:
        wBar = SSB_HORZ;
        break;
    }
    return m_pRichEdit->EnableScrollBar(wBar, fuArrowflags == ESB_ENABLE_BOTH);
}

BOOL STextHost::TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw)
{
    return m_pRichEdit->SetScrollRange(fnBar != SB_HORZ, nMinPos, nMaxPos, fRedraw);
}

BOOL SRichEdit::OnTxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw)
{
    if (m_fScrollPending)
        return TRUE;
    BOOL bVertical = fnBar != SB_HORZ;
    SCROLLINFO *psi = bVertical ? (&m_siVer) : (&m_siHoz);

    if (psi->nPos != nPos)
    {
        psi->nPos = nPos;
        CRect rcSb = GetScrollBarRect(!!bVertical);
        InvalidateRect(rcSb);
    }
    if (fRedraw)
    {
        Invalidate();
    }
    return TRUE;
}

BOOL STextHost::TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw)
{
    return m_pRichEdit->OnTxSetScrollPos(fnBar, nPos, fRedraw);
}

void STextHost::TxInvalidateRect(LPCRECT prc, BOOL fMode)
{
    if (prc)
    {
        m_pRichEdit->InvalidateRect(prc);
    }
    else
    {
        m_pRichEdit->Invalidate();
    }
}

void STextHost::TxViewChange(BOOL fUpdate)
{
    if (fUpdate)
    {
        m_pRichEdit->InvalidateRect(NULL);
    }
}

COLORREF STextHost::TxGetSysColor(int nIndex)
{
    return ::GetSysColor(nIndex);
}

HRESULT STextHost::TxGetBackStyle(TXTBACKSTYLE *pstyle)
{
    *pstyle = TXTBACK_TRANSPARENT;
    return S_OK;
}

HRESULT STextHost::TxGetMaxLength(DWORD *plength)
{
    *plength = m_pRichEdit->m_cchTextMost;
    return S_OK;
}

HRESULT STextHost::TxGetScrollBars(DWORD *pdwScrollBar)
{
    *pdwScrollBar = m_pRichEdit->m_dwStyle & (WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_DISABLENOSCROLL);

    return S_OK;
}

HRESULT STextHost::TxGetPasswordChar(TCHAR *pch)
{
    *pch = m_pRichEdit->m_chPasswordChar;
    return S_OK;
}

HRESULT STextHost::TxGetAcceleratorPos(LONG *pcp)
{
    *pcp = m_pRichEdit->m_lAccelPos;
    return S_OK;
}

HRESULT STextHost::TxGetExtent(LPSIZEL lpExtent)
{
    *lpExtent = m_pRichEdit->m_sizelExtent;
    return S_OK;
}

HRESULT STextHost::OnTxCharFormatChange(const CHARFORMATW *pcf)
{
    return S_OK;
}

HRESULT STextHost::OnTxParaFormatChange(const PARAFORMAT *ppf)
{
    return S_OK;
}

HRESULT STextHost::TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits)
{
    DWORD dwProperties = 0;

    if (m_pRichEdit->m_fRich)
    {
        dwProperties = TXTBIT_RICHTEXT;
    }

    if (m_pRichEdit->m_dwStyle & ES_MULTILINE)
    {
        dwProperties |= TXTBIT_MULTILINE;
    }

    if (m_pRichEdit->m_dwStyle & ES_READONLY)
    {
        dwProperties |= TXTBIT_READONLY;
    }

    if (m_pRichEdit->m_dwStyle & ES_PASSWORD)
    {
        dwProperties |= TXTBIT_USEPASSWORD;
    }

    if (!(m_pRichEdit->m_dwStyle & ES_NOHIDESEL))
    {
        dwProperties |= TXTBIT_HIDESELECTION;
    }

    if (m_pRichEdit->m_fEnableAutoWordSel)
    {
        dwProperties |= TXTBIT_AUTOWORDSEL;
    }

    if (m_pRichEdit->m_fVertical)
    {
        dwProperties |= TXTBIT_VERTICAL;
    }

    if (m_pRichEdit->m_fWordWrap)
    {
        dwProperties |= TXTBIT_WORDWRAP;
    }

    if (m_pRichEdit->m_fAllowBeep)
    {
        dwProperties |= TXTBIT_ALLOWBEEP;
    }

    if (m_pRichEdit->m_fSaveSelection)
    {
        dwProperties |= TXTBIT_SAVESELECTION;
    }

    *pdwBits = dwProperties & dwMask;
    return NOERROR;
}

HRESULT STextHost::TxNotify(DWORD iNotify, void *pv)
{
    return m_pRichEdit->OnTxNotify(iNotify, pv);
}

HIMC STextHost::TxImmGetContext()
{
    return ImmGetContext(m_pRichEdit->GetContainer()->GetHostHwnd());
}

void STextHost::TxImmReleaseContext(HIMC himc)
{
    ImmReleaseContext(m_pRichEdit->GetContainer()->GetHostHwnd(), himc);
}

HRESULT STextHost::TxGetSelectionBarWidth(LONG *plSelBarWidth)
{
    *plSelBarWidth = 0;
    return S_OK;
}

BOOL STextHost::Init(SRichEdit *pRichEdit)
{
    IUnknown *pUnk;
    HRESULT hr;

    m_pRichEdit = pRichEdit;

    // Create Text Services component
    if (FAILED(STextServiceHelper::instance()->CreateTextServices(NULL, this, &pUnk)))
        return FALSE;

    hr = pUnk->QueryInterface(IID_ITextServices, (void **)&pserv);

    pUnk->Release();

    return SUCCEEDED(hr);
}

//////////////////////////////////////////////////////////////////////////
// SRichEdit

SRichEdit::SRichEdit()
    : m_fTransparent(0)
    , m_fRich(1)
    , m_fSaveSelection(TRUE)
    , m_fVertical(FALSE)
    , m_fWordWrap(FALSE)
    , m_fAllowBeep(FALSE)
    , m_fEnableAutoWordSel(TRUE)
    , m_fWantTab(FALSE)
    , m_fSingleLineVCenter(TRUE)
    , m_fScrollPending(FALSE)
    , m_fEnableDragDrop(FALSE)
    , m_fAutoSel(FALSE)
    , m_fNotifyChange(FALSE)
    , m_fDisableCaret(FALSE)
    , m_cchTextMost(cInitTextMax)
    , m_chPasswordChar(_T('*'))
    , m_lAccelPos(-1)
    , m_dwStyle(ES_LEFT | ES_AUTOHSCROLL)
    , m_byDbcsLeadByte(0)
    , m_nFontHeight(10)
{
    m_pNcSkin = GETBUILTINSKIN(SKIN_SYS_BORDER);

    m_bFocusable = TRUE;
    m_bClipClient = TRUE;
    m_sizelExtent.cx = m_sizelExtent.cy = 0;
    m_evtSet.addEvent(EVENTID(EventRENotify));
    m_evtSet.addEvent(EVENTID(EventREMenu));

    m_pTxtHost = new STextHost;
    if (!m_pTxtHost->Init(this))
    {
        m_pTxtHost->Release();
        m_pTxtHost = NULL;
    }
}

SRichEdit::~SRichEdit()
{
    if (m_pTxtHost)
    {
        m_pTxtHost->Release();
        m_pTxtHost = NULL;
    }
}
int SRichEdit::OnCreate(LPVOID)
{
    int ret = __baseCls::OnCreate(NULL);
    if (ret != 0)
        return ret;
    if (!m_pTxtHost)
    {
        SSLOGE() << "create text host failed!";
        return 2;
    }
    InitDefaultCharFormat(&m_cfDef);
    InitDefaultParaFormat(&m_pfDef);

    if (!m_fTransparent && m_style.m_crBg == CR_INVALID && !m_pBgSkin)
        m_style.m_crBg = RGB(0xff, 0xff, 0xff);
    // inplace activate
    m_pTxtHost->GetTextService()->OnTxInPlaceActivate(NULL);
    //默认没有焦点
    m_pTxtHost->m_fUiActive = FALSE;
    m_pTxtHost->GetTextService()->OnTxUIDeactivate();
    m_pTxtHost->GetTextService()->TxSendMessage(WM_KILLFOCUS, 0, 0, 0);

    // set IME
    DWORD dw = (DWORD)SSendMessage(EM_GETLANGOPTIONS);
    dw |= IMF_AUTOKEYBOARD | IMF_DUALFONT | IMF_UIFONTS;
    dw &= ~IMF_AUTOFONT;
    SSendMessage(EM_SETLANGOPTIONS, 0, dw);

    if (m_fNotifyChange)
    { // receive enm_change event
        SSendMessage(EM_SETEVENTMASK, 0, ENM_CHANGE);
    }

    if (m_strRtfSrc.IsEmpty())
        SetWindowText(SWindow::GetWindowText());
    else
        SetAttribute(L"rtf", m_strRtfSrc, FALSE);
    // register droptarget
    OnEnableDragDrop(!(m_dwStyle & ES_READONLY) & m_fEnableDragDrop);
    return 0;
}

void SRichEdit::OnDestroy()
{
    KillFocus();
    OnEnableDragDrop(FALSE);

    if (m_pTxtHost)
    {
        m_pTxtHost->GetTextService()->OnTxInPlaceDeactivate();
    }

    m_mapTimer.RemoveAll();
    __baseCls::OnDestroy();
}

void SRichEdit::OnPaint(IRenderTarget *pRT)
{
    CRect rcClient;
    GetClientRect(&rcClient);
    pRT->PushClipRect(&rcClient, RGN_AND);
    // save font and text color
    SAutoRefPtr<IRenderObj> pFont = pRT->GetCurrentObject(OT_FONT);
    COLORREF crText = pRT->GetTextColor();
    float fMtx[9];
    pRT->GetTransform(fMtx);
    SMatrix mtx(fMtx);

    SAutoRefPtr<IRenderTarget> rt;
    if (!mtx.isIdentity() || !pRT->IsOffscreen())
    {
        GETRENDERFACTORY->CreateRenderTarget(&rt, rcClient.Width(), rcClient.Height());
        rt->BeginDraw();
        rt->SetViewportOrg(-rcClient.TopLeft());
        if (!SUCCEEDED(rt->BitBlt(&rcClient, pRT, rcClient.left, rcClient.top)))
        {
            //从ID2D1HwndRenderTarget复制背景会失败，重新生成背景
            rt->ClearRect(&rcClient, RGBA(255, 255, 255, 255));
            SSendMessage(WM_ERASEBKGND, (WPARAM)(IRenderTarget *)rt);
        }
    }
    else
    {
        rt = pRT;
    }
    HDC hdc = rt->GetDC(0);
    if (hdc)
    {
        int nOldMode = ::SetGraphicsMode(hdc, GM_COMPATIBLE); // richedit需要将GraphicMode强制设置为GM_COMPATIBLE

#ifdef _WIN32
        ALPHAINFO ai;
        CGdiAlpha::AlphaBackup(hdc, &rcClient, ai);
#endif //_WIN32
        LONG lPos = 0;
        m_pTxtHost->GetTextService()->TxGetVScroll(NULL, NULL, &lPos, NULL, NULL);
        RECTL rcL = { rcClient.left, rcClient.top, rcClient.right, rcClient.bottom };
        m_pTxtHost->GetTextService()->TxDraw(DVASPECT_CONTENT, // Draw Aspect
                                             0,                // Lindex
                                             NULL,             // Info for drawing optimazation
                                             NULL,             // target device information
                                             hdc,              // Draw device HDC
                                             NULL,             // Target device HDC
                                             &rcL,             // Bounding client rectangle
                                             NULL,             // Clipping rectangle for metafiles
                                             &rcClient,        // Update rectangle
                                             NULL,             // Call back function
                                             0,                // Call back parameter
                                             TXTVIEW_ACTIVE);
#ifdef _WIN32
        CGdiAlpha::AlphaRestore(ai);
#endif //_WIN32
        ::SetGraphicsMode(hdc, nOldMode);
    }
    rt->ReleaseDC(hdc, &rcClient);
    if (rt != pRT)
    {
        rt->EndDraw();
        pRT->AlphaBlend(&rcClient, rt, &rcClient, 255);
        rt = NULL;
    }
    // restore font and text color
    pRT->SelectObject(pFont);
    pRT->SetTextColor(crText);
    pRT->PopClip();
}

void SRichEdit::OnSetFocus(SWND wndOld)
{
    __baseCls::OnSetFocus(wndOld);

    if (m_pTxtHost && IsVisible(TRUE) && !IsDisabled(TRUE))
    {
        m_pTxtHost->m_fUiActive = TRUE;
        m_pTxtHost->GetTextService()->OnTxUIActivate();
        m_pTxtHost->GetTextService()->TxSendMessage(WM_SETFOCUS, 0, 0, 0);
        if (m_fAutoSel)
            SetSel((DWORD)MAKELONG(0, -1), TRUE);
    }

    if (ES_PASSWORD & m_dwStyle || ES_NUMBER & m_dwStyle)
    {
        GetContainer()->EnableIME(FALSE);
    }
}

void SRichEdit::OnKillFocus(SWND wndFocus)
{
    if (ES_PASSWORD & m_dwStyle || ES_NUMBER & m_dwStyle)
    {
        GetContainer()->EnableIME(TRUE);
    }

    __baseCls::OnKillFocus(wndFocus);
    if (m_pTxtHost)
    {
        m_pTxtHost->m_fUiActive = FALSE;
        m_pTxtHost->GetTextService()->OnTxUIDeactivate();
        m_pTxtHost->GetTextService()->TxSendMessage(WM_KILLFOCUS, 0, 0, 0);
        m_pTxtHost->TxShowCaret(FALSE);
    }
    //防止正在编辑时隐藏了cursor
    GetContainer()->OnUpdateCursor();
}

void SRichEdit::OnTimer(char idEvent)
{
    if (idEvent == TIMER_INVALIDATE)
    {
        Invalidate();
        KillTimer(idEvent);
    }
    else
    {
        __baseCls::OnTimer(idEvent);
    }
}

UINT SRichEdit::OnGetDlgCode() const
{
    UINT uRet = SC_WANTCHARS | SC_WANTARROWS;
    if (m_fWantTab)
        uRet |= SC_WANTTAB;
    if (m_dwStyle & ES_WANTRETURN)
        uRet |= SC_WANTRETURN;
    return uRet;
}

BOOL SRichEdit::OnScroll(BOOL bVertical, UINT uCode, int nPos)
{
    if (m_fScrollPending)
        return FALSE;
    LRESULT lresult = -1;
    m_fScrollPending = TRUE;
    SPanel::OnScroll(bVertical, uCode, nPos);

#ifdef USE_MSFTEDIT
    LONG lPos = 0;
    if (uCode == SB_THUMBPOSITION)
    {
        POINT scrollPos;

        m_pTxtHost->GetTextService()->TxSendMessage(EM_GETSCROLLPOS, 0, reinterpret_cast<LPARAM>(&scrollPos), NULL);
        if (bVertical)
        {
            scrollPos.y = nPos;
        }
        else
        {
            scrollPos.x = nPos;
        }

        m_pTxtHost->GetTextService()->TxSendMessage(EM_SETSCROLLPOS, 0, reinterpret_cast<LPARAM>(&scrollPos), NULL);
        m_pTxtHost->GetTextService()->TxSendMessage(EM_GETSCROLLPOS, 0, reinterpret_cast<LPARAM>(&scrollPos), NULL);

        if (bVertical)
        {
            lPos = scrollPos.y;
        }
        else
        {
            lPos = scrollPos.x;
        }

        if (lPos != GetScrollPos(bVertical))
            SetScrollPos(bVertical, lPos, TRUE);
    }
    else
    {
        m_pTxtHost->GetTextService()->TxSendMessage(bVertical ? WM_VSCROLL : WM_HSCROLL, MAKEWPARAM(uCode, nPos), 0, &lresult);
        if (bVertical)
        {
            m_pTxtHost->GetTextService()->TxGetVScroll(NULL, NULL, &lPos, NULL, NULL);
        }
        else
        {
            m_pTxtHost->GetTextService()->TxGetHScroll(NULL, NULL, &lPos, NULL, NULL);
        }

        if (lPos != GetScrollPos(bVertical))
            SetScrollPos(bVertical, lPos, TRUE);
    }
#else
    m_pTxtHost->GetTextService()->TxSendMessage(bVertical ? WM_VSCROLL : WM_HSCROLL, MAKEWPARAM(uCode, nPos), 0, &lresult);
    LONG lPos = 0;
    if (bVertical)
    {
        m_pTxtHost->GetTextService()->TxGetVScroll(NULL, NULL, &lPos, NULL, NULL);
    }
    else
    {
        m_pTxtHost->GetTextService()->TxGetHScroll(NULL, NULL, &lPos, NULL, NULL);
    }
    if (lPos != GetScrollPos(bVertical))
        SetScrollPos(bVertical, lPos, TRUE);
#endif

    m_fScrollPending = FALSE;
    if (uCode == SB_THUMBTRACK)
        ScrollUpdate();
    return lresult == 0;
}

BOOL SRichEdit::OnSetCursor(const CPoint &pt)
{
    CRect rcClient;
    GetClientRect(&rcClient);
    if (!rcClient.PtInRect(pt))
        return FALSE;

    HDC hdc = GetDC(GetContainer()->GetHostHwnd());
    m_pTxtHost->GetTextService()->OnTxSetCursor(DVASPECT_CONTENT, -1, NULL, NULL, hdc, NULL, &rcClient, pt.x, pt.y);
    ReleaseDC(GetContainer()->GetHostHwnd(), hdc);
    return TRUE;
}

BOOL SRichEdit::SwndProc(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lResult)
{
    BOOL bRet = __baseCls::SwndProc(uMsg, wParam, lParam, lResult);
    if (bRet)
        return TRUE;
    if (m_pTxtHost && m_pTxtHost->GetTextService())
    {
        if (uMsg == EM_GETRECT)
        {
            SetMsgHandled(TRUE);
            GetClientRect((LPRECT)lParam);
            return TRUE;
        }
        if (m_pTxtHost->GetTextService()->TxSendMessage(uMsg, wParam, lParam, lResult) == S_OK)
        {
            SetMsgHandled(TRUE);
            return TRUE;
        }
    }
    return bRet;
}

HRESULT SRichEdit::InitDefaultCharFormat(CHARFORMAT2W *pcf, IFontS *pFont)
{
    SAutoRefPtr<IRenderTarget> pRT;
    GETRENDERFACTORY->CreateRenderTarget(&pRT, 0, 0);
    SASSERT(pRT);
    BeforePaintEx(pRT);

    SAutoRefPtr<IFontS> oldFont;
    if (pFont == NULL)
        pFont = (IFontS *)pRT->GetCurrentObject(OT_FONT);
    SIZE szTxt;
    pRT->MeasureText(_T("A"), 1, &szTxt);
    m_nFontHeight = szTxt.cy;

    memset(pcf, 0, sizeof(CHARFORMAT2W));
    pcf->cbSize = sizeof(CHARFORMAT2W);
    pcf->dwMask = CFM_SIZE | CFM_OFFSET | CFM_FACE | CFM_CHARSET | CFM_COLOR | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE;

#ifdef _WIN32
    pcf->crTextColor = pRT->GetTextColor() & 0x00ffffff;
#else
    pcf->crTextColor = pRT->GetTextColor();
#endif //_WIN32
    HDC hdc = GetDC(NULL);
    LONG yPixPerInch = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(NULL, hdc);
    const LOGFONT *plf = pFont->LogFont();
    pcf->yHeight = abs(MulDiv(pFont->TextSize(), LY_PER_INCH, yPixPerInch));
    if (SLayoutSize::defUnit != px && IsRichScale())
    {
        // rich scale 的情况下，edit内部已经对文字进行了放大，不再放大默认字体。
        pcf->yHeight /= (GetScale() / 100);
    }
    pcf->yOffset = 0;
    pcf->dwEffects = 0;
    if (pFont->IsBold())
        pcf->dwEffects |= CFE_BOLD;
    if (pFont->IsItalic())
        pcf->dwEffects |= CFE_ITALIC;
    if (pFont->IsUnderline())
        pcf->dwEffects |= CFE_UNDERLINE;
    pcf->bCharSet = plf->lfCharSet;
    pcf->bPitchAndFamily = plf->lfPitchAndFamily;
#ifdef _UNICODE
    wcscpy(pcf->szFaceName, plf->lfFaceName);
#else
    // need to thunk pcf->szFaceName to a standard char string.in this case it's easy because our
    // thunk is also our copy
    MultiByteToWideChar(CP_ACP, 0, plf->lfFaceName, LF_FACESIZE, pcf->szFaceName, LF_FACESIZE);
#endif

    return S_OK;
}

HRESULT SRichEdit::InitDefaultParaFormat(PARAFORMAT2 *ppf)
{
    memset(ppf, 0, sizeof(PARAFORMAT2));
    ppf->cbSize = sizeof(PARAFORMAT2);
    ppf->dwMask = PFM_ALL;
    ppf->cTabCount = 1;
    ppf->rgxTabs[0] = lDefaultTab;

    if (m_dwStyle & ES_CENTER)
        ppf->wAlignment = PFA_CENTER;
    else if (m_dwStyle & ES_RIGHT)
        ppf->wAlignment = PFA_RIGHT;
    else
        ppf->wAlignment = PFA_LEFT;

    return S_OK;
}

HRESULT SRichEdit::OnTxNotify(DWORD iNotify, LPVOID pv)
{
    EventRENotify evt(this);
    evt.iNotify = iNotify;
    evt.pv = pv;
    evt.hr = S_OK;
    FireEvent(evt);
    return evt.hr;
}
//////////////////////////////////////////////////////////////////////////
//    richedit interfaces
BOOL SRichEdit::GetWordWrap(void) const
{
    return m_fWordWrap;
}

void SRichEdit::SetWordWrap(BOOL fWordWrap)
{
    m_fWordWrap = fWordWrap;
    m_pTxtHost->GetTextService()->OnTxPropertyBitsChange(TXTBIT_WORDWRAP, fWordWrap ? TXTBIT_WORDWRAP : 0);
}

BOOL SRichEdit::GetReadOnly() const
{
    return (m_dwStyle & ES_READONLY) != 0;
}

BOOL SRichEdit::SetReadOnly(BOOL bReadOnly)
{
    return 0 != SSendMessage(EM_SETREADONLY, bReadOnly);
}

LONG SRichEdit::GetLimitText() const
{
    return m_cchTextMost;
}

BOOL SRichEdit::SetLimitText(int nLength)
{
    return 0 != SSendMessage(EM_EXLIMITTEXT, nLength);
}

WORD SRichEdit::GetDefaultAlign() const
{
    return m_pfDef.wAlignment;
}

void SRichEdit::SetDefaultAlign(WORD wNewAlign)
{
    m_pfDef.wAlignment = wNewAlign;

    // Notify control of property change
    m_pTxtHost->GetTextService()->OnTxPropertyBitsChange(TXTBIT_PARAFORMATCHANGE, 0);
}

BOOL SRichEdit::GetRichTextFlag() const
{
    return m_fRich;
}

void SRichEdit::SetRichTextFlag(BOOL fRich)
{
    m_fRich = fRich;

    m_pTxtHost->GetTextService()->OnTxPropertyBitsChange(TXTBIT_RICHTEXT, fRich ? TXTBIT_RICHTEXT : 0);
}

LONG SRichEdit::GetDefaultLeftIndent() const
{
    return m_pfDef.dxOffset;
}

void SRichEdit::SetDefaultLeftIndent(LONG lNewIndent)
{
    m_pfDef.dxOffset = lNewIndent;

    m_pTxtHost->GetTextService()->OnTxPropertyBitsChange(TXTBIT_PARAFORMATCHANGE, 0);
}

BOOL SRichEdit::SetSaveSelection(BOOL fSaveSelection)
{
    BOOL fResult = fSaveSelection;

    m_fSaveSelection = fSaveSelection;

    // notify text services of property change
    m_pTxtHost->GetTextService()->OnTxPropertyBitsChange(TXTBIT_SAVESELECTION, m_fSaveSelection ? TXTBIT_SAVESELECTION : 0);

    return fResult;
}

void SRichEdit::OnLButtonDown(UINT nFlags, CPoint point)
{
    SetCapture();
    if (!IsFocused())
    {
        SetFocus();
        if (!m_fAutoSel)
            m_pTxtHost->GetTextService()->TxSendMessage(GetCurMsg()->uMsg, GetCurMsg()->wParam, GetCurMsg()->lParam, NULL);
    }
    else
    {
        m_pTxtHost->GetTextService()->TxSendMessage(GetCurMsg()->uMsg, GetCurMsg()->wParam, GetCurMsg()->lParam, NULL);
    }
}

void SRichEdit::OnLButtonUp(UINT nFlags, CPoint point)
{
    m_pTxtHost->GetTextService()->TxSendMessage(GetCurMsg()->uMsg, GetCurMsg()->wParam, GetCurMsg()->lParam, NULL);
    ReleaseCapture();
}

LRESULT SRichEdit::OnButtonClick(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    m_pTxtHost->GetTextService()->TxSendMessage(uMsg, wParam, lParam, NULL);
    return 0;
}

void SRichEdit::OnRButtonDown(UINT nFlags, CPoint point)
{
    if (FireCtxMenu(point))
        return; //用户自己响应右键
    SetFocus();
    //弹出默认编辑窗菜单
    SXmlNode xmlMenu = SApplication::getSingletonPtr()->GetEditCtxMenuTemplate();
    if (xmlMenu)
    {
        SMenu menu;
        if (menu.LoadMenu2(&xmlMenu))
        {
            CRect rcCantainer;
            GetContainer()->GetContainerRect(&rcCantainer);
            point.Offset(rcCantainer.TopLeft());
            HWND hHost = GetContainer()->GetHostHwnd();
            ::ClientToScreen(hHost, &point);
            BOOL canPaste = (BOOL)SSendMessage(EM_CANPASTE, 0);
            DWORD dwStart = 0, dwEnd = 0;
            SSendMessage(EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
            BOOL hasSel = dwStart < dwEnd;
            UINT uLen = (UINT)SSendMessage(WM_GETTEXTLENGTH, 0, 0);
            BOOL bReadOnly = m_dwStyle & ES_READONLY;
            EnableMenuItem(menu.m_hMenu, MENU_CUT, MF_BYCOMMAND | ((hasSel && (!bReadOnly)) ? 0 : MF_GRAYED));
            EnableMenuItem(menu.m_hMenu, MENU_COPY, MF_BYCOMMAND | (hasSel ? 0 : MF_GRAYED));
            EnableMenuItem(menu.m_hMenu, MENU_PASTE, MF_BYCOMMAND | ((canPaste && (!bReadOnly)) ? 0 : MF_GRAYED));
            EnableMenuItem(menu.m_hMenu, MENU_DEL, MF_BYCOMMAND | ((hasSel && (!bReadOnly)) ? 0 : MF_GRAYED));
            EnableMenuItem(menu.m_hMenu, MENU_SELALL, MF_BYCOMMAND | ((uLen > 0) ? 0 : MF_GRAYED));

            UINT uCmd = menu.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN, point.x, point.y, hHost, NULL, GetScale());

            EventREMenu evt(this);
            evt.uCmd = uCmd;
            FireEvent(evt);
            if (evt.handled)
                return;

            switch (uCmd)
            {
            case MENU_CUT:
                SSendMessage(WM_CUT);
                break;
            case MENU_COPY:
                SSendMessage(WM_COPY);
                break;
            case MENU_PASTE:
                SSendMessage(WM_PASTE);
                break;
            case MENU_DEL:
                SSendMessage(EM_REPLACESEL, 0, (LPARAM) _T(""));
                break;
            case MENU_SELALL:
                SSendMessage(EM_SETSEL, 0, -1);
                break;
            default:
                break;
            }
        }
    }
}

void SRichEdit::OnMouseMove(UINT nFlags, CPoint point)
{
    m_pTxtHost->GetTextService()->TxSendMessage(GetCurMsg()->uMsg, GetCurMsg()->wParam, GetCurMsg()->lParam, NULL);
}

void SRichEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    EventKeyDown evt(this);
    evt.nChar = nChar;
    evt.nFlags = nFlags;
    FireEvent(evt);
    if (evt.handled && !evt.bubbleUp)
    {
        return;
    }

    if (nChar == VK_RETURN && !(m_dwStyle & ES_WANTRETURN) && !(GetKeyState(VK_CONTROL) & 0x8000))
    {
        SetMsgHandled(FALSE);
        return;
    }
    m_pTxtHost->GetTextService()->TxSendMessage(GetCurMsg()->uMsg, GetCurMsg()->wParam, GetCurMsg()->lParam, NULL);
}

#define CTRL(_ch) (_ch - 'A' + 1)

void SRichEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    SWNDMSG msg = *GetCurMsg();
    switch (nChar)
    {
        // Ctrl-Return generates Ctrl-J (LF), treat it as an ordinary return
    case CTRL('J'):
    case VK_RETURN:
        if (!(GetKeyState(VK_CONTROL) & 0x8000) && !(m_dwStyle & ES_WANTRETURN))
            return;
        break;

    case VK_TAB:
        if (!m_fWantTab && !(GetKeyState(VK_CONTROL) & 0x8000))
            return;
        break;
    default:
        if (m_dwStyle & ES_NUMBER && !isdigit(nChar) && nChar != '-' && nChar != '.' && nChar != ',')
            return;
        if ((m_dwStyle & ES_UPPERCASE) && nChar >= 'a' && nChar <= 'z')
        {
            nChar -= 0x20;
            msg.wParam = nChar;
            break;
        }
        if ((m_dwStyle & ES_LOWERCASE) && nChar >= 'A' && nChar <= 'Z')
        {
            nChar += 0x20;
            msg.wParam = nChar;
            break;
        }
#ifndef _UNICODE
        // todo:hjx
        if (m_byDbcsLeadByte == 0)
        {
            if (IsDBCSLeadByte(nChar))
            {
                m_byDbcsLeadByte = nChar;
                return;
            }
        }
        else
        {
            nChar = MAKEWORD(nChar, m_byDbcsLeadByte);
            m_pTxtHost->GetTextService()->TxSendMessage(WM_IME_CHAR, nChar, 0, NULL);
            m_byDbcsLeadByte = 0;
            return;
        }
#endif //_UNICODE
        break;
    }
    m_pTxtHost->GetTextService()->TxSendMessage(msg.uMsg, msg.wParam, msg.lParam, NULL);
}

LRESULT SRichEdit::OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam)
{
    __baseCls::OnNcCalcSize(bCalcValidRects, lParam);

    CRect rcInsetPixel = GetStyle().GetPadding();

    if (!IsRichScale())
    {
        rcInsetPixel.top = rcInsetPixel.bottom = (m_rcClient.Height() - m_nFontHeight) / 2;
    }

    m_siHoz.nPage = m_rcClient.Width() - rcInsetPixel.left - rcInsetPixel.right;
    m_siVer.nPage = m_rcClient.Height() - rcInsetPixel.top - rcInsetPixel.bottom;

    if (m_pTxtHost)
    {
        HDC hdc = GetDC(NULL);
        LONG xPerInch = ::GetDeviceCaps(hdc, LOGPIXELSX);
        LONG yPerInch = ::GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(NULL, hdc);

        m_sizelExtent.cx = DtoHimetric(m_rcClient.Width(), xPerInch);
        m_sizelExtent.cy = DtoHimetric(m_rcClient.Height(), yPerInch);

        m_rcInset.left = DtoHimetric(rcInsetPixel.left, xPerInch);
        m_rcInset.right = DtoHimetric(rcInsetPixel.right, xPerInch);
        m_rcInset.top = DtoHimetric(rcInsetPixel.top, yPerInch);
        m_rcInset.bottom = DtoHimetric(rcInsetPixel.bottom, yPerInch);

        //窗口有焦点时，需要更新光标位置：先使edit失活用来关闭光标，再激活edit来显示光标。
        //此处不应该直接用setfocus和killfocus，因为这两个消息可能会被外面响应。导致逻辑错误
        BOOL bFocus = IsFocused();
        if (bFocus)
        {
            m_pTxtHost->m_fUiActive = FALSE;
            m_pTxtHost->GetTextService()->OnTxUIDeactivate();
            m_pTxtHost->GetTextService()->TxSendMessage(WM_KILLFOCUS, 0, 0, NULL);
            m_pTxtHost->TxShowCaret(FALSE);
        }
        m_pTxtHost->GetTextService()->OnTxPropertyBitsChange(TXTBIT_EXTENTCHANGE | TXTBIT_CLIENTRECTCHANGE, TXTBIT_EXTENTCHANGE | TXTBIT_CLIENTRECTCHANGE);
        if (bFocus)
        {
            m_pTxtHost->m_fUiActive = TRUE;
            m_pTxtHost->GetTextService()->OnTxUIActivate();
            m_pTxtHost->GetTextService()->TxSendMessage(WM_SETFOCUS, 0, 0, NULL);
        }
    }
    return 0;
}

LRESULT SRichEdit::OnSetReadOnly(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return SUCCEEDED(SetAttribute(L"readonly", wParam ? L"1" : L"0"));
}

LRESULT SRichEdit::OnSetLimitText(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (wParam == 0)
        m_cchTextMost = cInitTextMax;
    else
        m_cchTextMost = (DWORD)wParam;
    m_pTxtHost->GetTextService()->OnTxPropertyBitsChange(TXTBIT_MAXLENGTHCHANGE, TXTBIT_MAXLENGTHCHANGE);
    return 1;
}

LRESULT SRichEdit::OnSetCharFormat(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (wParam == SCF_DEFAULT && !FValidCF((CHARFORMAT2W *)lParam))
    { //设置默认字体只支持CHARFORMAT2W
        SSLOGI() << "set default char format failed! only CHARFORMAT2W can be set for default char format";
        return 0;
    }

    m_pTxtHost->GetTextService()->TxSendMessage(uMsg, wParam, lParam, NULL);
    if (wParam == SCF_DEFAULT)
    {
        m_cfDef = *(CHARFORMAT2W *)lParam;
        m_pTxtHost->GetTextService()->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, TXTBIT_CHARFORMATCHANGE);
    }
    return 1;
}

LRESULT SRichEdit::OnSetParaFormat(UINT uMsg, WPARAM wparam, LPARAM lparam)
{
    if (!FValidPF((PARAFORMAT *)lparam))
    {
        return 0;
    }

    // check to see if we're setting the default.
    // either SCF_DEFAULT will be specified *or* there is no
    // no text in the document (richedit1.0 behaviour).
    if (!(wparam & SCF_DEFAULT))
    {
        HRESULT hr = m_pTxtHost->GetTextService()->TxSendMessage(WM_GETTEXTLENGTH, 0, 0, 0);

        if (hr == 0)
        {
            wparam |= SCF_DEFAULT;
        }
    }

    if (wparam & SCF_DEFAULT)
    {
        m_pfDef = *(PARAFORMAT2 *)lparam;
        m_pTxtHost->GetTextService()->OnTxPropertyBitsChange(TXTBIT_PARAFORMATCHANGE, TXTBIT_PARAFORMATCHANGE);
    }
    else
    {
        m_pTxtHost->GetTextService()->TxSendMessage(uMsg, wparam, lparam,
                                                    NULL); // Change selection format
    }
    return 1;
}

LRESULT SRichEdit::OnSetText(UINT uMsg, WPARAM wparam, LPARAM lparam)
{
    // For RichEdit 1.0, the max text length would be reset by a settext so
    // we follow pattern here as well.

    HRESULT hr = m_pTxtHost->GetTextService()->TxSendMessage(uMsg, wparam, lparam, 0);

    if (FAILED(hr))
        return 0;
    // Update succeeded.
    ULONG cNewText = (ULONG)(lparam ? _tcslen((LPCTSTR)lparam) : 0);

    // If the new text is greater than the max set the max to the new
    // text length.
    if (cNewText > m_cchTextMost)
    {
        m_cchTextMost = cNewText;
    }
    return 1;
}

void SRichEdit::OnSetFont(IFontS *pFont, BOOL bRedraw)
{
    if (SUCCEEDED(InitDefaultCharFormat(&m_cfDef, pFont)))
    {
        m_pTxtHost->GetTextService()->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, TXTBIT_CHARFORMATCHANGE);
    }
}

void SRichEdit::SetWindowText(LPCTSTR lpszText)
{
#ifdef _UNICODE
    SSendMessage(WM_SETTEXT, 0, (LPARAM)lpszText);
#else
    SStringW str = S_CT2W(lpszText);
    SSendMessage(WM_SETTEXT, 0, (LPARAM)(LPCWSTR)str);
#endif
    if (IsRichScale())
    {
        // setwindowtext会导致zoom丢失，需要重新设置。
        SSendMessage(EM_SETZOOM, GetScale(), 100);
    }
    Invalidate();
}

int SRichEdit::GetWindowText(THIS_ TCHAR *pBuf, int nBufLen, BOOL bRawText) const
{
    if (!pBuf)
    {
        return GetWindowTextLength();
    }
    SStringT str = GetWindowText(bRawText);
    if (nBufLen < str.GetLength())
        return 0;
    else if (nBufLen == str.GetLength())
        _tcsncpy(pBuf, str.c_str(), str.GetLength());
    else                            // if(nBufLen > str.GetLength())
        _tcscpy(pBuf, str.c_str()); // auto append null char
    return str.GetLength();
}

SStringT SRichEdit::GetWindowText(BOOL bRawText) const
{
    SStringW strRet;
    if (m_pTxtHost)
    {
        LRESULT nLen = 0;
        m_pTxtHost->GetTextService()->TxSendMessage(WM_GETTEXTLENGTH, 0, 0, &nLen);
        wchar_t *pBuf = strRet.GetBufferSetLength(nLen);
        m_pTxtHost->GetTextService()->TxSendMessage(WM_GETTEXT, (WPARAM)nLen + 1, (LPARAM)pBuf, NULL);
        strRet.ReleaseBuffer();
    }
    return S_CW2T(strRet);
}

int SRichEdit::GetWindowTextLength() const
{
#ifdef _UNICODE
    LRESULT lResult = 0;
    if (m_pTxtHost)
    {
        m_pTxtHost->GetTextService()->TxSendMessage(WM_GETTEXTLENGTH, 0, 0, &lResult);
    }
    return (int)lResult;
#else
    SStringT strRet = GetWindowText(TRUE);
    return strRet.GetLength();
#endif //_UNICODE
}

ITextServices *SRichEdit::GetTextServices()
{
    return m_pTxtHost->GetTextService();
}

void SRichEdit::ReplaceSel(LPCTSTR pszText, BOOL bCanUndo)
{
#ifdef _UNICODE
    SSendMessage(EM_REPLACESEL, (WPARAM)bCanUndo, (LPARAM)pszText);
#else
    SStringW str = S_CT2W(pszText);
    SSendMessage(EM_REPLACESEL, (WPARAM)bCanUndo, (LPARAM)str.c_str());
#endif
}

void SRichEdit::SetSel(DWORD dwSelection, BOOL bNoScroll)
{
    SSendMessage(EM_SETSEL, LOWORD(dwSelection), HIWORD(dwSelection));
    if (!bNoScroll)
        SSendMessage(EM_SCROLLCARET, 0, 0L);
}

void SRichEdit::SetSel(long nStartChar, long nEndChar, BOOL bNoScroll)
{
    SSendMessage(EM_SETSEL, nStartChar, nEndChar);
    if (!bNoScroll)
        SSendMessage(EM_SCROLLCARET, 0, 0L);
}

HRESULT SRichEdit::OnAttrTextColor(const SStringW &strValue, BOOL bLoading)
{
    m_style.SetTextColor(0, GETCOLOR(strValue));
    if (!bLoading)
    {
        SetDefaultTextColor(m_style.GetTextColor(0));
    }
    return S_OK;
}

DWORD CALLBACK EditStreamInCallback_FILE(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
    FILE *f = (FILE *)dwCookie;
    LONG nReaded = (LONG)fread(pbBuff, 1, cb, f);
    if (pcb)
        *pcb = nReaded;
    return 0;
}

DWORD CALLBACK EditStreamOutCallback_FILE(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
    FILE *f = (FILE *)dwCookie;
    LONG nWrited = (LONG)fwrite(pbBuff, 1, cb, f);
    if (pcb)
        *pcb = nWrited;
    return 0;
}

struct MemBlock
{
    LPBYTE pBuf;
    LONG nRemains;
};

DWORD CALLBACK EditStreamInCallback_MemBlock(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
    MemBlock *pmb = (MemBlock *)dwCookie;
    if (pmb->nRemains >= cb)
    {
        memcpy(pbBuff, pmb->pBuf, cb);
        pmb->pBuf += cb;
        pmb->nRemains -= cb;
        if (pcb)
            *pcb = cb;
        return 0;
    }
    else
    {
        memcpy(pbBuff, pmb->pBuf, pmb->nRemains);
        pmb->pBuf += pmb->nRemains;
        if (pcb)
            *pcb = pmb->nRemains;
        pmb->nRemains = 0;
        return 0;
    }
}

HRESULT SRichEdit::OnAttrRTF(const SStringW &strValue, BOOL bLoading)
{
    if (bLoading)
    {
        m_strRtfSrc = strValue; //将数据保存到控件初始化完成再写入控件
        return S_FALSE;
    }
    else
    {
        SStringTList lstSrc;
        int nSegs = ParseResID(S_CW2T(strValue), lstSrc);
        LPCTSTR pszType = NULL;
        LPCTSTR pszName = NULL;
        if (lstSrc.GetCount() == 2)
        {
            pszType = lstSrc[0];
            pszName = lstSrc[1];
        }
        else
        {
            pszName = lstSrc[0];
        }
        size_t dwSize = GETRESPROVIDER->GetRawBufferSize(pszType, pszName);
        if (dwSize)
        {
            EDITSTREAM es;
            MemBlock mb = { NULL, 0 };
            SAutoBuf mybuf;
            mb.pBuf = (LPBYTE)mybuf.Allocate(dwSize);
            mb.nRemains = (DWORD)dwSize;
            GETRESPROVIDER->GetRawBuffer(pszType, pszName, mybuf, dwSize);
            es.dwCookie = (DWORD_PTR)&mb;
            es.pfnCallback = EditStreamInCallback_MemBlock;
            SSendMessage(EM_STREAMIN, SF_RTF, (LPARAM)&es);
        }
        return S_FALSE;
    }
}

COLORREF SRichEdit::SetDefaultTextColor(COLORREF cr)
{
    COLORREF crOld = m_cfDef.crTextColor;
#ifdef _WIN32
    m_cfDef.crTextColor = cr & 0x00ffffff;
#else
    m_cfDef.crTextColor = cr;
#endif //_WIN32
    m_pTxtHost->GetTextService()->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, TXTBIT_CHARFORMATCHANGE);
    return crOld;
}

void SRichEdit::OnEnableDragDrop(BOOL bEnable)
{
    if (!GetContainer())
        return;
    if (bEnable)
    {
        SRicheditDropTarget *pDropTarget = new SRicheditDropTarget(m_pTxtHost->GetTextService());
        GetContainer()->RegisterDragDrop(m_swnd, pDropTarget);
        pDropTarget->Release();
    }
    else
    {
        GetContainer()->UnregisterDragDrop(m_swnd);
    }
}

HRESULT SRichEdit::OnAttrAlign(const SStringW &strValue, BOOL bLoading)
{
    if (!strValue.CompareNoCase(L"center"))
        m_dwStyle |= ES_CENTER;
    else if (!strValue.CompareNoCase(L"right"))
        m_dwStyle |= ES_RIGHT;
    else
        m_dwStyle |= ES_LEFT;
    if (!bLoading)
    {
        if (m_dwStyle & ES_CENTER)
            m_pfDef.wAlignment = PFA_CENTER;
        else if (m_dwStyle & ES_RIGHT)
            m_pfDef.wAlignment = PFA_RIGHT;
        else
            m_pfDef.wAlignment = PFA_LEFT;
        m_pTxtHost->GetTextService()->OnTxPropertyBitsChange(TXTBIT_PARAFORMATCHANGE, 0);
    }
    return bLoading ? S_FALSE : S_OK;
}

HRESULT SRichEdit::OnAttrNotifyChange(const SStringW &strValue, BOOL bLoading)
{
    m_fNotifyChange = STRINGASBOOL(strValue);
    if (!bLoading)
    {
        LPARAM lEvtMask = SSendMessage(EM_GETEVENTMASK);
        if (m_fNotifyChange)
            lEvtMask |= ENM_CHANGE;
        else
            lEvtMask &= ~ENM_CHANGE;
        SSendMessage(EM_SETEVENTMASK, 0, lEvtMask);
    }
    return S_FALSE;
}

DWORD SRichEdit::SaveRtf(LPCTSTR pszFileName)
{
    FILE *f = _tfopen(pszFileName, _T("wb"));
    if (!f)
        return 0;
    EDITSTREAM es;
    es.dwCookie = (DWORD_PTR)f;
    es.pfnCallback = EditStreamOutCallback_FILE;
    DWORD dwRet = (DWORD)SSendMessage(EM_STREAMOUT, SF_RTF, (LPARAM)&es);
    fclose(f);
    return dwRet;
}

DWORD SRichEdit::LoadRtf(LPCTSTR pszFileName)
{
    FILE *f = _tfopen(pszFileName, _T("rb"));
    if (!f)
        return FALSE;
    EDITSTREAM es;
    es.dwCookie = (DWORD_PTR)f;
    es.pfnCallback = EditStreamInCallback_FILE;
    DWORD dwRet = (DWORD)SSendMessage(EM_STREAMIN, SF_RTF, (LPARAM)&es);
    fclose(f);
    return dwRet;
}

void SRichEdit::OnScaleChanged(int nScale)
{
    __baseCls::OnScaleChanged(nScale);
    if (IsRichScale())
    {
        SSendMessage(EM_SETZOOM, nScale, 100);
    }
    else
    { //单行居中的放大，做特殊处理
        OnSetFont(NULL, FALSE);
    }
}

void SRichEdit::OnRebuildFont()
{
    __baseCls::OnRebuildFont();
    OnSetFont(NULL, FALSE); //更新默认字体
}

void SRichEdit::OnEnable(BOOL bEnable, UINT nStatus)
{
    __baseCls::OnEnable(bEnable, nStatus);
    COLORREF cr;
    if (bEnable)
    {
        cr = m_style.GetTextColor(0);
        if (CR_INVALID == cr)
            cr = 0;
    }
    else
    {
        cr = m_style.GetTextColor(3);
    }

    if (CR_INVALID != cr)
        SetDefaultTextColor(cr);
}

BOOL SRichEdit::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    if (!(m_dwStyle & ES_MULTILINE))
    {
        return SWindow::OnMouseWheel(nFlags, zDelta, pt);
    }
    else
    {
        PSWNDMSG p = GetCurMsg();
        LRESULT lResult = 0;
        return m_pTxtHost->GetTextService()->TxSendMessage(p->uMsg, p->wParam, p->lParam, &lResult) == S_OK;
    }
}

BOOL SRichEdit::CreateCaret(HBITMAP pBmp, int nWid, int nHeight)
{
    if (m_fDisableCaret)
        return FALSE;
    return SWindow::CreateCaret(pBmp, nWid, nHeight);
}

HRESULT SRichEdit::OnAttrReStyle(const SStringW &strValue, DWORD dwStyle, DWORD txtBit, BOOL bLoading)
{
    BOOL bValue = STRINGASBOOL(strValue);
    DWORD dwBit = 0;
    if (!bValue)
        m_dwStyle &= ~dwStyle;
    else
        m_dwStyle |= dwStyle, dwBit = txtBit;
    m_pTxtHost->GetTextService()->OnTxPropertyBitsChange(txtBit, dwBit);
    return bLoading ? S_FALSE : S_OK;
}

HRESULT SRichEdit::OnAttrReStyle2(const SStringW &strValue, DWORD dwStyle, DWORD txtBit, BOOL bLoading)
{
    BOOL bValue = STRINGASBOOL(strValue);
    if (!bValue)
        m_dwStyle &= ~dwStyle;
    else
        m_dwStyle |= dwStyle;
    m_pTxtHost->GetTextService()->OnTxPropertyBitsChange(txtBit, txtBit);
    return bLoading ? S_FALSE : S_OK;
}

HRESULT SRichEdit::OnAttrPasswordChar(const SStringW &strValue, BOOL bLoading)
{
    SStringT strValueT = S_CW2T(strValue);
    m_chPasswordChar = strValueT[0];
    return bLoading ? S_FALSE : S_OK;
}

HRESULT SRichEdit::OnAttrEnableDragdrop(const SStringW &strValue, BOOL bLoading)
{
    m_fEnableDragDrop = STRINGASBOOL(strValue);
    if (!bLoading)
    {
        OnEnableDragDrop(!(m_dwStyle & ES_READONLY) & m_fEnableDragDrop);
    }
    return S_FALSE;
}

LRESULT SRichEdit::OnGetRect(UINT uMsg, WPARAM wp, LPARAM lp)
{
    GetClientRect((LPRECT)lp);
    return TRUE;
}

BOOL SRichEdit::OnTxSetTimer(UINT idTimer, UINT uTimeout)
{
    SMap<UINT, SAutoRefPtr<ITimer>>::CPair *p = m_mapTimer.Lookup(idTimer);
    if (p)
    {
        p->m_value->KillTimer();
        p->m_value->StartTimer(uTimeout, TRUE, idTimer);
        return TRUE;
    }
    MemberFunctionSlot<SRichEdit, IEvtArgs> slot = Subscriber(&SRichEdit::OnTimeout, this);
    STimer *timer = new STimer(&slot);
    timer->StartTimer(uTimeout, TRUE, idTimer);
    m_mapTimer[idTimer] = timer;
    timer->Release();

    return TRUE;
}

void SRichEdit::OnTxKillTimer(UINT idTimer)
{
    m_mapTimer.RemoveKey(idTimer);
}

BOOL SRichEdit::OnTimeout(IEvtArgs *e)
{
    EventTimer *e2 = sobj_cast<EventTimer>(e);
    m_pTxtHost->GetTextService()->TxSendMessage(WM_TIMER, e2->uData, 0, NULL);
    return TRUE;
}

BOOL SRichEdit::IsRichScale() const
{
    return m_fRich || !m_fSingleLineVCenter || (m_dwStyle & ES_MULTILINE);
}

SNSEND