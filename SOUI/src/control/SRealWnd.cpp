#include <souistd.h>
#include <control/SRealWnd.h>
#include <interface/SRealWndHandler-i.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

class DefRealWndHandler : public TObjRefImpl2<IRealWndHandler, DefRealWndHandler> {
  public:
    DefRealWndHandler(SRealWnd *pRealWnd)
        : m_owner(pRealWnd)
    {
    }

    HWND WINAPI OnRealWndCreate(IWindow *pRealWnd) OVERRIDE
    {
        EventRealWndCreate evt(m_owner);
        evt.hRet = 0;
        m_owner->FireEvent(&evt);
        return evt.hRet;
    }
    void WINAPI OnRealWndDestroy(IWindow *pRealWnd) OVERRIDE
    {
        EventRealWndDestroy evt(m_owner);
        m_owner->FireEvent(&evt);
    }
    BOOL WINAPI OnRealWndInit(IWindow *pRealWnd) OVERRIDE
    {
        EventRealWndInit evt(m_owner);
        evt.bRet = FALSE;
        m_owner->FireEvent(&evt);
        return evt.bRet;
    }

    BOOL WINAPI OnRealWndPosition(IWindow *pRealWnd, const RECT *rcWnd) OVERRIDE
    {
        EventRealWndPosition evt(m_owner);
        evt.rc = *rcWnd;
        evt.bRet = FALSE;
        m_owner->FireEvent(&evt);
        return evt.bRet;
    }

  protected:
    SRealWnd *m_owner;
};

SRealWnd::SRealWnd()
    : m_bInit(FALSE)
    , m_lpData(0)
    , m_hRealWnd(0)
{
    GetEventSet()->addEvent(EVENTID(EventRealWndCreate));
    GetEventSet()->addEvent(EVENTID(EventRealWndDestroy));
    GetEventSet()->addEvent(EVENTID(EventRealWndInit));
    GetEventSet()->addEvent(EVENTID(EventRealWndPosition));

    m_pRealWndHandler = GETREALWNDHANDLER;
    if (!m_pRealWndHandler)
    {
        m_pRealWndHandler.Attach(new DefRealWndHandler(this));
    }
}

SRealWnd::~SRealWnd()
{
}

BOOL SRealWnd::NeedRedrawWhenStateChange()
{
    return FALSE;
}

void SRealWnd::ShowRealWindow()
{
    if (IsVisible(TRUE) && !IsWindow(m_hRealWnd))
    {
        InitRealWnd();
    }
    if (IsWindow(m_hRealWnd))
    {
        ShowWindow(m_hRealWnd, IsVisible(TRUE) ? SW_SHOW : SW_HIDE);
    }
}

void SRealWnd::OnShowWindow(BOOL bShow, UINT nStatus)
{
    __baseCls::OnShowWindow(bShow, nStatus);
    ShowRealWindow();
}

void SRealWnd::OnDestroy()
{
    if (IsWindow(m_hRealWnd))
    {
        m_pRealWndHandler->OnRealWndDestroy(this);
    }
    __baseCls::OnDestroy();
}

BOOL SRealWnd::InitFromXml(IXmlNode *pNode)
{
    SXmlNode xmlNode(pNode);
    m_xmlParams.root().append_copy(xmlNode.child(L"params"));
    BOOL bRet = __baseCls::InitFromXml(pNode);
    if (bRet)
    {
        if (m_bInit)
            InitRealWnd();
    }
    return bRet;
}

HWND SRealWnd::GetRealHwnd(BOOL bAutoCreate /*=TRUE*/)
{
    if (!bAutoCreate)
        return m_hRealWnd;

    if (!m_bInit && !IsWindow(m_hRealWnd))
    {
        InitRealWnd();
    }

    return m_hRealWnd;
}

BOOL SRealWnd::InitRealWnd()
{
    m_dwStyle |= WS_CHILD;
    m_hRealWnd = m_pRealWndHandler->OnRealWndCreate(this);

    if (::IsWindow(m_hRealWnd))
    {
        CRect rcClient;
        GetClientRect(&rcClient);
        SetRealWndPos(m_hRealWnd, &rcClient);
        m_pRealWndHandler->OnRealWndInit(this);
        return TRUE;
    }
    return FALSE;
}

BOOL SRealWnd::OnRelayout(const CRect &rcWnd)
{
    if (!__baseCls::OnRelayout(rcWnd))
        return FALSE;

    HWND hRealWnd = GetRealHwnd(FALSE);
    if (::IsWindow(hRealWnd))
    {
        SetRealWndPos(hRealWnd, &rcWnd);
    }
    return TRUE;
}

void SRealWnd::SetRealWndPos(HWND hRealWnd, const CRect *prc)
{
    if (!m_pRealWndHandler->OnRealWndPosition(this, prc))
    {
        CPoint pt = prc->TopLeft();
        if (GetWindowLong(hRealWnd, GWL_STYLE) & WS_POPUP)
        {
            ::ClientToScreen(GetContainer()->GetHostHwnd(), &pt);
        }
        ::SetWindowPos(hRealWnd, 0, pt.x, pt.y, prc->Width(), prc->Height(), SWP_NOZORDER);
    }
}

SNSEND