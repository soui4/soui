#pragma once
#include <core/SHostWnd.h>
#include <core/SWndStyle.h>
#include <res.mgr/SUiDef.h>

namespace SNS
{
namespace FluentDemo
{

// These are shadow gutters, not the 26/20/26/32 nine-slice cut lines.
inline CRect WindowShadowGutter(int scale)
{
    return CRect(MulDiv(18, scale, 100), MulDiv(12, scale, 100), MulDiv(18, scale, 100), MulDiv(24, scale, 100));
}

inline CRect ExpandForWindowShadow(CRect content, const CRect &gutter)
{
    content.left -= gutter.left;
    content.top -= gutter.top;
    content.right += gutter.right;
    content.bottom += gutter.bottom;
    return content;
}

// Pointer-to-member access preserves the actual layout units, including mixed
// px/dp padding. There is no downcast of a base object to a fictitious subclass.
class ChromeStyleAccess : public SwndStyle {
  public:
    static void CopyPadding(const SwndStyle &style, SLayoutSize (&padding)[4])
    {
        SLayoutSize (SwndStyle::*member)[4] = &ChromeStyleAccess::m_rcInset;
        for (int edge = 0; edge < 4; ++edge)
            padding[edge] = (style.*member)[edge];
    }
};

class ChromeHostAccess : public SHostWndAttr {
  public:
    static void CopyMetrics(const SHostWndAttr &attributes, SLayoutSize (&margin)[4], SLayoutSize (&minimum)[2])
    {
        SLayoutSize (SHostWndAttr::*margins)[4] = &ChromeHostAccess::m_rcMargin;
        SLayoutSize (SHostWndAttr::*sizes)[2] = &ChromeHostAccess::m_szMin;
        for (int edge = 0; edge < 4; ++edge)
            margin[edge] = (attributes.*margins)[edge];
        for (int axis = 0; axis < 2; ++axis)
            minimum[axis] = (attributes.*sizes)[axis];
    }
};

class ChromeWindowAccess : public SWindow {
  public:
    static ISkinObj *Background(SWindow *window)
    {
        SAutoRefPtr<ISkinObj> SWindow::*member = &ChromeWindowAccess::m_pBgSkin;
        return window->*member;
    }
};

// Demo-only decoration. Existing non-layered/NC/child hosts keep their mode.
// No animation is installed on the main window: authored enter/exit animations
// remain owned by their original demo.
class WindowChrome {
  public:
    WindowChrome()
        : m_host(NULL)
        , m_root(NULL)
        , m_maximized(false)
        , m_busy(false)
        , m_bg(CR_INVALID)
        , m_border(CR_INVALID)
    {
        m_previous.fun = NULL;
        m_previous.ctx = NULL;
    }
    ~WindowChrome()
    {
        Detach();
    }

    BOOL Attach(SHostWnd &host)
    {
        if (m_host == &host)
        {
            Refresh();
            return TRUE;
        }
        // Optional decoration: a non-layered host does not need a fake opaque
        // shadow, and no window is converted to a different presenter/mode.
        if (!host.IsTranslucent() || (host.GetStyle() & WS_CHILD) || !GETSKIN(L"fluent_window_shadow", host.GetRoot()->GetScale()))
            return TRUE;
        if (m_host || host.GetMsgHandler()->fun == HandleMessage)
            return FALSE;
        m_host = &host;
        m_root = host.GetRoot();
        m_previous = *host.GetMsgHandler();
        m_skinName.Empty();
        m_bgReference.Empty();
        m_borderReference.Empty();
        ChromeStyleAccess::CopyPadding(m_root->GetStyle(), m_padding);
        ChromeHostAccess::CopyMetrics(host.GetHostAttr(), m_margin, m_minimum);
        if (ISkinObj *skin = ChromeWindowAccess::Background(m_root))
            m_skinName = skin->GetName();
        m_bg = m_root->GetStyle().m_crBg;
        m_border = m_root->GetStyle().m_crBorder;
        m_root->GetAttribute(L"colorBkgnd", &m_bgReference);
        m_root->GetAttribute(L"colorBorder", &m_borderReference);
        m_maximized = ::IsZoomed(host.m_hWnd) != FALSE;
        host.SetMsgHandler(HandleMessage, this);
        host.GetHostAttr().SetAttribute(L"margin", L"18dp,12dp,18dp,24dp", FALSE);
        Refresh();
        ResizeDecoration(true);
        return TRUE;
    }

    SHostWnd *Host() const
    {
        return m_host;
    }

    void Refresh()
    {
        if (!m_host || !m_root || m_busy)
            return;
        m_busy = true;
        const int scale = m_root->GetScale();
        const CRect gutter = m_maximized ? CRect(0, 0, 0, 0) : WindowShadowGutter(scale);
        const int extra[4] = { gutter.left, gutter.top, gutter.right, gutter.bottom };
        CRect desired;
        LONG *edges[4] = { &desired.left, &desired.top, &desired.right, &desired.bottom };
        for (int edge = 0; edge < 4; ++edge)
            *edges[edge] = m_padding[edge].toPixelSize(scale) + extra[edge];
        if (m_root->GetStyle().GetPadding() != desired)
        {
            SStringW text;
            text.Format(L"%dpx,%dpx,%dpx,%dpx", desired.left, desired.top, desired.right, desired.bottom);
            m_root->SetAttribute(L"padding", text, FALSE);
            m_root->RequestRelayout();
        }
        const LPCWSTR name = m_maximized ? m_skinName.c_str() : L"fluent_window_shadow";
        ISkinObj *skin = ChromeWindowAccess::Background(m_root);
        if ((skin && SStringW(skin->GetName()) != name) || (!skin && *name))
            m_root->SetAttribute(L"skin", name, FALSE);
        if (m_maximized)
        {
            RestoreColors();
        }
        else
        {
            // Opaque root fill/border would hide the alpha gutter (MC list).
            // The nine-slice supplies the same semantic panel and its border.
            m_root->SetAttribute(L"colorBkgnd", L"#00000000", FALSE);
            m_root->SetAttribute(L"colorBorder", L"#00000000", FALSE);
        }
        const CRect normalGutter = WindowShadowGutter(scale);
        SStringW minimum;
        minimum.Format(L"%dpx,%dpx", m_minimum[0].toPixelSize(scale) + normalGutter.left + normalGutter.right, m_minimum[1].toPixelSize(scale) + normalGutter.top + normalGutter.bottom);
        m_host->GetHostAttr().SetAttribute(L"minsize", minimum, FALSE);
        m_root->Invalidate();
        m_busy = false;
    }

    // A root destruction notification calls this with restoreAppearance=false:
    // it removes the handler before the borrowed root/host are deleted.
    void Detach(bool restoreAppearance = true)
    {
        if (!m_host)
            return;
        SHostWnd *host = m_host;
        const MsgHandlerInfo *current = host->GetMsgHandler();
        if (current->fun == HandleMessage && current->ctx == this)
            host->SetMsgHandler(m_previous.fun, m_previous.ctx);
        if (restoreAppearance && m_root && ::IsWindow(host->m_hWnd))
        {
            RestoreColors();
            m_root->SetAttribute(L"skin", m_skinName, FALSE);
            SStringW padding;
            padding.Format(L"%s,%s,%s,%s", m_padding[0].toString().c_str(), m_padding[1].toString().c_str(), m_padding[2].toString().c_str(), m_padding[3].toString().c_str());
            m_root->SetAttribute(L"padding", padding, FALSE);
            m_root->RequestRelayout();
            SStringW margin;
            margin.Format(L"%s,%s,%s,%s", m_margin[0].toString().c_str(), m_margin[1].toString().c_str(), m_margin[2].toString().c_str(), m_margin[3].toString().c_str());
            host->GetHostAttr().SetAttribute(L"margin", margin, FALSE);
            SStringW minimum;
            minimum.Format(L"%s,%s", m_minimum[0].toString().c_str(), m_minimum[1].toString().c_str());
            host->GetHostAttr().SetAttribute(L"minsize", minimum, FALSE);
            ResizeDecoration(false);
        }
        m_host = NULL;
        m_root = NULL;
    }

  private:
    WindowChrome(const WindowChrome &);
    WindowChrome &operator=(const WindowChrome &);

    void RestoreColors()
    {
        if (!m_bgReference.IsEmpty())
            m_root->SetAttribute(L"colorBkgnd", m_bgReference, FALSE);
        else
            m_root->GetStyle().m_crBg = m_bg;
        if (!m_borderReference.IsEmpty())
            m_root->SetAttribute(L"colorBorder", m_borderReference, FALSE);
        else
            m_root->GetStyle().m_crBorder = m_border;
    }

    void ResizeDecoration(bool expand)
    {
        const CRect gutter = WindowShadowGutter(m_root->GetScale());
        WINDOWPLACEMENT placement = {0};
        placement.length = sizeof(placement);
        if (::IsZoomed(m_host->m_hWnd) || ::IsIconic(m_host->m_hWnd))
        {
            if (::GetWindowPlacement(m_host->m_hWnd, &placement))
            {
                CRect rect(placement.rcNormalPosition);
                const CRect signedGutter = expand ? gutter : CRect(-gutter.left, -gutter.top, -gutter.right, -gutter.bottom);
                placement.rcNormalPosition = ExpandForWindowShadow(rect, signedGutter);
                ::SetWindowPlacement(m_host->m_hWnd, &placement);
            }
            return;
        }
        CRect rect = m_host->GetWindowRect();
        const CRect signedGutter = expand ? gutter : CRect(-gutter.left, -gutter.top, -gutter.right, -gutter.bottom);
        rect = ExpandForWindowShadow(rect, signedGutter);
        m_host->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
    }

    static BOOL HandleMessage(const LPMSG message, LRESULT *result, void *context)
    {
        WindowChrome *chrome = static_cast<WindowChrome *>(context);
        const MsgHandlerInfo previous = chrome->m_previous;
        if (message->message == WM_SIZE && message->wParam != SIZE_MINIMIZED)
        {
            chrome->m_maximized = message->wParam == SIZE_MAXIMIZED;
            chrome->Refresh();
        }
        else if (message->message == WM_WINDOWPOSCHANGING || message->message == WM_GETMINMAXINFO)
        {
            chrome->Refresh();
        }
        else if (message->message == WM_NCDESTROY)
        {
            chrome->Detach(false);
        }
        return previous.fun ? previous.fun(message, result, previous.ctx) : FALSE;
    }

    SHostWnd *m_host;
    SWindow *m_root;
    MsgHandlerInfo m_previous;
    SLayoutSize m_padding[4], m_margin[4], m_minimum[2];
    SStringW m_skinName, m_bgReference, m_borderReference;
    bool m_maximized, m_busy;
    COLORREF m_bg, m_border;
};

} // namespace FluentDemo
} // namespace SNS
