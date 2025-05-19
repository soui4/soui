﻿#include "souistd.h"

#include "SApp.h"
#include "core/SHostWnd.h"
#include "helper/SAutoBuf.h"
#include "helper/SColor.h"
#include "helper/SplitString.h"
#include "helper/STime.h"
#include <helper/SHostMgr.h>
#include <core/SHostPresenter.h>
#ifdef _WIN32
#include <Imm.h>
#pragma comment(lib, "imm32.lib")
#endif
SNSBEGIN

//////////////////////////////////////////////////////////////////////////
//    SDummyWnd
//////////////////////////////////////////////////////////////////////////
class SDummyWnd : public SNativeWnd {
  public:
    SDummyWnd(SHostWnd *pOwner)
        : m_pOwner(pOwner)
    {
    }

    void OnPaint(HDC dc)
    {
        PAINTSTRUCT ps;
        ::BeginPaint(m_hWnd, &ps);
        ::EndPaint(m_hWnd, &ps);
        m_pOwner->OnPrint(NULL);
    }

    void OnDestroy()
    {
        m_pOwner->m_dummyWnd = NULL;
    }

    virtual void OnFinalMessage(HWND hWnd)
    {
        delete this;
    }

    BEGIN_MSG_MAP_EX(SDummyWnd)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_DESTROY(OnDestroy)
    END_MSG_MAP()
  private:
    SHostWnd *m_pOwner;
};

//////////////////////////////////////////////////////////////////////////
//    SHostWndAttr
//////////////////////////////////////////////////////////////////////////
SHostWndAttr::SHostWndAttr(void)
    : m_hAppIconSmall(NULL)
    , m_hAppIconBig(NULL)
{
    Init();
}

SHostWndAttr::~SHostWndAttr()
{
    if (m_hAppIconSmall)
        DestroyIcon(m_hAppIconSmall);
    if (m_hAppIconBig)
        DestroyIcon(m_hAppIconBig);
}

void SHostWndAttr::Init()
{
    m_bResizable = FALSE;
    m_bTranslucent = FALSE;
    m_bAutoShape = FALSE;
    m_bAppWnd = FALSE;
    m_bToolWnd = FALSE;
    m_wndType = WT_UNDEFINE;
    m_bAllowSpy = TRUE;
    m_bSendWheel2Hover = FALSE;
    m_bHasMsgLoop = TRUE;
    m_dwStyle = (0);
    m_dwExStyle = (0);
    if (m_hAppIconSmall)
        DestroyIcon(m_hAppIconSmall);
    if (m_hAppIconBig)
        DestroyIcon(m_hAppIconBig);
    m_hAppIconSmall = NULL;
    m_hAppIconBig = NULL;
}

LPCWSTR SHostWndAttr::GetTrCtx() const
{
    return m_strTrCtx;
}

CRect SHostWndAttr::GetMaxInset(int nScale) const
{
    CRect rcRet;
    rcRet.left = m_rcMaxInset[0].toPixelSize(nScale);
    rcRet.top = m_rcMaxInset[1].toPixelSize(nScale);
    rcRet.right = m_rcMaxInset[2].toPixelSize(nScale);
    rcRet.bottom = m_rcMaxInset[3].toPixelSize(nScale);
    return rcRet;
}

CRect SHostWndAttr::GetMargin(int nScale) const
{
    CRect rcRet;
    rcRet.left = m_rcMargin[0].toPixelSize(nScale);
    rcRet.top = m_rcMargin[1].toPixelSize(nScale);
    rcRet.right = m_rcMargin[2].toPixelSize(nScale);
    rcRet.bottom = m_rcMargin[3].toPixelSize(nScale);
    return rcRet;
}

CSize SHostWndAttr::GetMinSize(int nScale) const
{
    CSize szRet;
    szRet.cx = m_szMin[0].toPixelSize(nScale);
    szRet.cy = m_szMin[1].toPixelSize(nScale);
    return szRet;
}

void SHostWndAttr::SetTranslucent(bool bTranslucent)
{
    m_bTranslucent = bTranslucent;
}

void SHostWndAttr::SetTrCtx(const SStringW &strTrCtx)
{
    m_strTrCtx = strTrCtx;
}

void SHostWndAttr::SetSendWheel2Hover(bool value)
{
    m_bSendWheel2Hover = value;
}

//////////////////////////////////////////////////////////////////////////
// SRootWindow
//////////////////////////////////////////////////////////////////////////

enum
{
    kAni4Destroy = 1,
    kAni4Hide = 2,
};

enum AniState
{
    Ani_none = 0,
    Ani_win_enter = 1,
    Ani_win_exit = 2,
    Ani_win = (Ani_win_enter | Ani_win_exit),
    Ani_host = 4,
    Ani_All = (Ani_win | Ani_host),
};

SRootWindow::SRootWindow(SHostWnd *pHostWnd)
    : m_pHostWnd(pHostWnd)
{
    GetEventSet()->addEvent(EVENTID(EventInit));
    GetEventSet()->addEvent(EVENTID(EventExit));
    GetEventSet()->addEvent(EVENTID(EventMenuCmd));
}

void SRootWindow::OnAnimationInvalidate(IAnimation *pAni, bool bErase)
{
    if (bErase)
    {
        CRect rcWnd = GetClientRect();
        m_pHostWnd->m_memRT->BeginDraw();
        m_pHostWnd->m_memRT->ClearRect(rcWnd, 0);
        m_pHostWnd->m_memRT->EndDraw();
    }
    SWindow::OnAnimationInvalidate(pAni, bErase);
    if (!bErase)
        Update();
}

void SRootWindow::OnAnimationStop(IAnimation *pAni)
{
    SWindow::OnAnimationStop(pAni);
    if (pAni == m_aniEnter || pAni == m_aniExit)
    {
        m_pHostWnd->m_AniState &= ~Ani_win;
        if (pAni == m_aniExit)
        {
            ULONG_PTR data = pAni->getUserData();
            if (data == kAni4Destroy)
                m_pHostWnd->SNativeWnd::DestroyWindow();
            else if (data == kAni4Hide)
            {
                m_pHostWnd->SNativeWnd::SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
            }
            pAni->setUserData(0);
        }
    }
}

void SRootWindow::BeforePaint(IRenderTarget *pRT, SPainter &painter)
{
    int iState = SState2Index::GetDefIndex(GetState(), true);
    SwndStyle &style = SWindow::GetStyle();
    IFontPtr pFont = style.GetTextFont(iState);
    if (pFont)
        pRT->SelectObject(pFont, (IRenderObj **)&painter.oldFont);
    else
        pRT->SelectObject(GETUIDEF->GetFont(FF_DEFAULTFONT, GetScale()), NULL);

    COLORREF crTxt = style.GetTextColor(iState);
    if (crTxt != CR_INVALID)
        painter.oldTextColor = pRT->SetTextColor(crTxt);
    else
        pRT->SetTextColor(RGBA(0, 0, 0, 255));
}

void SRootWindow::AfterPaint(IRenderTarget *pRT, SPainter &painter)
{
    pRT->SelectDefaultObject(OT_FONT, NULL);
}

void SRootWindow::UpdateLayout()
{
    if (!IsLayoutDirty())
        return;
    SIZE szAppSet = m_pHostWnd->m_szAppSetted;
    if ((szAppSet.cx <= 0 || szAppSet.cy <= 0) && GetLayoutParam()->IsWrapContent(Any))
    {
        int nWid = szAppSet.cx;
        if (nWid <= 0)
        {
            nWid = GetLayoutParam()->IsSpecifiedSize(Horz) ? GetLayoutParam()->GetSpecifiedSize(Horz).toPixelSize(GetScale()) : SIZE_WRAP_CONTENT;
        }
        int nHei = szAppSet.cy;
        if (nHei <= 0)
        {
            nHei = GetLayoutParam()->IsSpecifiedSize(Vert) ? GetLayoutParam()->GetSpecifiedSize(Vert).toPixelSize(GetScale()) : SIZE_WRAP_CONTENT;
        }
        CSize szRoot;
        GetDesiredSize(&szRoot, nWid, nHei);
        OnRelayout(CRect(CPoint(), szRoot));
    }
    else
    {
        SWindow::UpdateLayout();
    }
}

HRESULT SRootWindow::OnLanguageChanged()
{
    SStringT str = S_CW2T(tr(m_pHostWnd->m_hostAttr.m_strTitle));
    m_pHostWnd->SetWindowText(str);
    return 3;
}

void SRootWindow::OnScaleChanged(int scale)
{
    m_layoutDirty = dirty_self;
    SWindow::OnScaleChanged(scale);
    SWindow::InvalidateRect(NULL);
}

void SRootWindow::RequestRelayout(SWND hSource, BOOL bSourceResizable)
{
    SWindow *pSource = SWindowMgr::GetWindow(hSource);
    SASSERT(pSource);

    if (m_layoutDirty != dirty_self)
        m_layoutDirty = (hSource == m_swnd || bSourceResizable) ? dirty_self : dirty_child;

    CRect rcSour = pSource->GetWindowRect();
    while (rcSour.IsRectEmpty())
    {
        pSource = pSource->GetParent();
        if (!pSource)
            break;
        rcSour = pSource->GetWindowRect();
    }
    m_pHostWnd->OnRedraw(rcSour, FALSE);
}

SHostWnd *SRootWindow::GetHostWnd() const
{
    return m_pHostWnd;
}

void SRootWindow::FireMenuCmd(int menuID)
{
    EventMenuCmd evt(this);
    evt.menuId = menuID;
    FireEvent(evt);
}

//////////////////////////////////////////////////////////////////////////
// SHostWnd
//////////////////////////////////////////////////////////////////////////

BOOL SHostWnd::s_HideLocalUiDef = TRUE;
int SHostWnd::s_TaskQueueBufSize = 5;

void SHostWnd::SetHideLocalUiDef(BOOL bHide)
{
    s_HideLocalUiDef = bHide;
}

void SHostWnd::SetTaskQueueBufSize(int nBufSize)
{
    s_TaskQueueBufSize = nBufSize;
}

SHostWnd::SHostWnd(LPCWSTR pszResName /*= NULL*/)
{
    if (pszResName)
        m_strXmlLayout = S_CW2T(pszResName);
    _Init();
}

SHostWnd::SHostWnd(LPCSTR pszResName)
{
    if (pszResName)
        m_strXmlLayout = S_CA2T(pszResName, CP_UTF8);
    _Init();
}

void SHostWnd::_Init()
{
    SApplication::getSingletonPtr()->AddRef();
    m_bTrackFlag = FALSE;
    m_bNeedRepaint = FALSE;
    m_bNeedAllRepaint = TRUE;
    m_pTipCtrl = NULL;
    m_dummyWnd = NULL;
    m_szAppSetted = CSize(0, 0);
    m_nAutoSizing = 0;
    m_bResizing = false;
    m_dwThreadID = 0;
    m_AniState = 0;
    m_pRoot = NULL;
    m_pNcPainter.Attach(new SNcPainter(this));
    m_msgMouse.message = 0;

    m_hostAnimationHandler.m_pHostWnd = this;
    m_evtHandler.fun = NULL;
    m_evtHandler.ctx = NULL;
    m_cEnableUiDefCount = 0;
    m_xmlInit = NULL;
}

SHostWnd::~SHostWnd()
{
    if (m_pRoot)
    {
        m_pRoot->Release();
        m_pRoot = NULL;
    }
    SApplication::getSingletonPtr()->Release();
}

HWND SHostWnd::CreateEx(HWND hWndParent, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight, IXmlNode *xmlInit)
{
    if (NULL != m_hWnd)
        return m_hWnd;
    UpdateAutoSizeCount(true);
    SXmlDoc xmlDoc;
    SXmlNode xmlRoot;
    if (!xmlInit)
    {
        xmlRoot = OnGetInitXmlNode(xmlDoc);
        if (!xmlRoot)
        {
            SSLOGW() << "OnGetInitXmlNode return empty xml";
        }
        else
        {
            xmlInit = &xmlRoot;
        }
    }
    // read translucent property
    m_hostAttr.Init();
    m_hostAttr.InitFromXml(xmlInit);
    if (m_hostAttr.m_bTranslucent)
    {
        dwExStyle |= WS_EX_LAYERED;
#ifndef _WIN32
        dwExStyle |= WS_EX_COMPOSITED;
#endif //_WIN32
    }
    m_xmlInit = xmlInit;
    HWND hWnd = SNativeWnd::CreateNative(_T("HOSTWND"), dwStyle, dwExStyle, x, y, nWidth, nHeight, hWndParent, 0);
    m_xmlInit = NULL;
    UpdateAutoSizeCount(false);
    if (!hWnd)
        return NULL;

    if ((nWidth == 0 || nHeight == 0) && (x == 0 && y == 0))
        CenterWindow(hWndParent);
    return hWnd;
}

HWND SHostWnd::Create(HWND hWndParent, int x, int y, int nWidth, int nHeight)
{
    DWORD dwStyle = WS_CLIPCHILDREN;
    if (hWndParent)
        dwStyle |= WS_TABSTOP;
    return CreateEx(hWndParent, dwStyle, 0, x, y, nWidth, nHeight);
}

BOOL SHostWnd::onRootResize(IEvtArgs *e)
{
    if (!m_bResizing)
    {
        EventSwndSize *e2 = sobj_cast<EventSwndSize>(e);
        UpdateAutoSizeCount(true);
        CRect rcWnd(CPoint(), e2->szWnd);
        rcWnd.InflateRect(GetRoot()->GetStyle().GetMargin());
        SetWindowPos(NULL, 0, 0, rcWnd.Width(), rcWnd.Height(), SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
        UpdateAutoSizeCount(false);
    }
    return TRUE;
}

void SHostWnd::EnablePrivateUiDef(THIS_ BOOL bEnable)
{
    if (!m_privateUiDefInfo)
        return;
    if (bEnable)
    {
        if (++m_cEnableUiDefCount == 1)
            GETUIDEF->PushUiDefInfo(m_privateUiDefInfo, TRUE);
    }
    else
    {
        if (--m_cEnableUiDefCount == 0)
            GETUIDEF->PopUiDefInfo(m_privateUiDefInfo, TRUE);
    }
}

BOOL SHostWnd::InitFromXml(IXmlNode *pNode)
{
    SASSERT(pNode);
    if (pNode->Empty())
    {
        SASSERT_MSGA(FALSE, "Null XML node");
        return FALSE;
    }
    if (!SNativeWnd::IsWindow())
        return FALSE;

    m_privateUiDefInfo = NULL;
    m_cEnableUiDefCount = 0;
    IUiDefInfo *pUiDefInfo = SUiDef::CreateUiDefInfo();
    if (pUiDefInfo->Init2(pNode, FALSE))
    { // init private uidef info.
        m_privateUiDefInfo = pUiDefInfo;
    }
    pUiDefInfo->Release();
    EnablePrivateUiDef(TRUE);

    if (m_AniState != Ani_none)
    {
        if (m_AniState & Ani_host)
            StopHostAnimation();
        if (m_AniState & Ani_win)
            GetRoot()->GetAnimation()->cancel();
        SASSERT(m_AniState == Ani_none);
    }

    // free old script module
    if (m_pScriptModule)
    {
        GetMsgLoop()->RemoveIdleHandler(m_pScriptModule->getIdleHandler());
        EventExit evt(GetRoot());
        GetRoot()->FireEvent(evt);
        m_pScriptModule = NULL;
    }

    //为了能够重入，先销毁原有的SOUI窗口
    GetRoot()->SSendMessage(WM_DESTROY);
    m_bFirstShow = TRUE;

    //加载脚本数据
    SXmlNode xmlNode(pNode);
    SXmlNode xmlScript = xmlNode.child(L"script");
    if (xmlScript)
    { // create new script module
        xmlScript.set_userdata(1);
        SApplication::getSingleton().CreateScriptModule(&m_pScriptModule);
        if (m_pScriptModule)
        {
            IIdleHandler *pIdleHandler = m_pScriptModule->getIdleHandler();
            if (pIdleHandler)
                GetMsgLoop()->AddIdleHandler(pIdleHandler);
            SXmlAttr attrSrc = xmlScript.attribute(L"src");
            if (attrSrc)
            {
                SStringT strSrc = S_CW2T(attrSrc.value());
                SStringTList lstSrc;
                ParseResID(strSrc, lstSrc);
                LPCTSTR pszType = NULL;
                LPCTSTR pszName = NULL;
                if (lstSrc.GetCount() == 2)
                {
                    pszType = lstSrc[0];
                    pszName = lstSrc[1];
                }
                else
                {
                    pszName = strSrc;
                }
                {
                    size_t dwSize = SApplication::getSingleton().GetRawBufferSize(pszType, pszName);
                    if (dwSize)
                    {
                        SAutoBuf buff(dwSize);
                        SApplication::getSingleton().GetRawBuffer(pszType, pszName, buff, dwSize);
                        m_pScriptModule->executeScriptBuffer(buff, dwSize);
                    }
                }
            }
            else
            {
                //从script节点的cdata中获取脚本
                SStringW strScript = xmlScript.child_value();
                if (!strScript.IsEmpty())
                {
                    SStringA utf8Script = S_CW2A(strScript, CP_UTF8);
                    m_pScriptModule->executeScriptBuffer(utf8Script, utf8Script.GetLength());
                }
            }
        }
    }
    SXmlNode xmlNcPainter = xmlNode.child(SNcPainter::GetClassName());
    xmlNcPainter.set_userdata(1);
    m_pNcPainter->InitFromXml(&xmlNcPainter);

    DWORD dwStyle = SNativeWnd::GetStyle();
    DWORD dwExStyle = SNativeWnd::GetExStyle();

    if (m_hostAttr.m_bResizable)
    {
        dwStyle |= WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;
    }
    else
    {
        dwStyle |= WS_MINIMIZEBOX;
    }
    if (m_hostAttr.m_bAppWnd)
    {
        dwStyle |= WS_SYSMENU;
        dwExStyle |= WS_EX_APPWINDOW;
    }
    else if (m_hostAttr.m_bToolWnd)
    {
        dwExStyle |= WS_EX_TOOLWINDOW;
    }

    if (m_hostAttr.m_dwStyle != 0)
        dwStyle = m_hostAttr.m_dwStyle & (~WS_VISIBLE);
    if (m_hostAttr.m_dwExStyle != 0)
        dwExStyle = m_hostAttr.m_dwExStyle;

    ModifyStyle(0, dwStyle);
    ModifyStyleEx(0, dwExStyle);
    m_pRoot->OnLanguageChanged();

    if (m_hostAttr.m_hAppIconSmall)
    {
        SendMessage(WM_SETICON, FALSE, (LPARAM)m_hostAttr.m_hAppIconSmall);
    }
    if (m_hostAttr.m_hAppIconBig)
    {
        SendMessage(WM_SETICON, TRUE, (LPARAM)m_hostAttr.m_hAppIconBig);
    }

    // compatible with 2.x width and height properties in soui element.
    SXmlAttr attrWid = xmlNode.attribute(L"width");
    SXmlAttr attrHei = xmlNode.attribute(L"height");
    SXmlNode xmlRoot = xmlNode.child(L"root");
    xmlRoot.set_userdata(1);
    if (attrWid && !xmlRoot.attribute(attrWid.name()))
        xmlRoot.append_copy(attrWid);
    if (attrHei && !xmlRoot.attribute(attrHei.name()))
        xmlRoot.append_copy(attrHei);

    GetRoot()->InitFromXml(&xmlRoot);

    if (IsTranslucent())
    {
#ifdef _WIN32
        if (!m_dummyWnd)
        {
            SetWindowLongPtr(GWL_EXSTYLE, GetWindowLongPtr(GWL_EXSTYLE) | WS_EX_LAYERED);
            m_dummyWnd = new SDummyWnd(this);
            HMONITOR hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
            MONITORINFO info = { sizeof(MONITORINFO) };
            GetMonitorInfo(hMonitor, &info);
            SStringT dummyTitle = SStringT().Format(_T("%s_dummy"), m_hostAttr.m_strTitle.c_str());
            m_dummyWnd->CreateNative(dummyTitle, WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, info.rcWork.left, info.rcWork.top, 1, 1, m_hWnd, 0, NULL);
            m_dummyWnd->SetWindowLongPtr(GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_LAYERED);
            ::SetLayeredWindowAttributes(m_dummyWnd->m_hWnd, 0, 0, LWA_ALPHA);
            if (IsWindowVisible())
            {
                m_dummyWnd->ShowWindow(SW_SHOWNOACTIVATE);
            }
        }
#endif //_WIN32
    }
    else if (dwExStyle & WS_EX_LAYERED || GetRoot()->GetAlpha() != 0xFF)
    {
        if (m_dummyWnd)
        {
            m_dummyWnd->DestroyWindow(); // m_dummyWnd will be set to null in SDummyWnd::OnDestroy
        }
        if (!(dwExStyle & WS_EX_LAYERED))
        {
#ifdef _WIN32
            ModifyStyleEx(0, WS_EX_LAYERED);
#else
            ModifyStyleEx(0, WS_EX_LAYERED | WS_EX_COMPOSITED);
#endif //_WIN32
        }
        SetLayeredWindowAttributes(0, GetRoot()->GetAlpha(), LWA_ALPHA);
    }
    m_memRT = NULL;
    if (IsTranslucent())
    {
        CRect rcWnd;
        SNativeWnd::GetWindowRect(&rcWnd);
        GETRENDERFACTORY->CreateRenderTarget(&m_memRT, rcWnd.Width(), rcWnd.Height());
    }
    else
    {
        GETRENDERFACTORY->CreateRenderTarget2(&m_memRT, m_hWnd);
    }

    BuildWndTreeZorder();

    int nWidth = m_szAppSetted.cx;
    int nHeight = m_szAppSetted.cy;
    CSize szNc = m_pNcPainter->GetNcSize();
    CSize szRoot;
    if (nWidth == 0)
        nWidth = SIZE_WRAP_CONTENT;
    if (nHeight == 0)
        nHeight = SIZE_WRAP_CONTENT;
    GetRoot()->GetDesiredSize(&szRoot, nWidth, nHeight);

    ILayoutParam *pLayoutParam = GetRoot()->GetLayoutParam();
    if (nWidth == 0 && pLayoutParam->IsSpecifiedSize(Horz))
    {
        nWidth = pLayoutParam->GetSpecifiedSize(Horz).toPixelSize(GetScale()) + szNc.cx;
    }
    if (nHeight == 0 && pLayoutParam->IsSpecifiedSize(Vert))
    {
        nHeight = pLayoutParam->GetSpecifiedSize(Vert).toPixelSize(GetScale()) + szNc.cy;
    }

    if (nWidth <= 0 || nHeight <= 0)
    { //计算出root大小
        if (nWidth <= 0)
        {
            nWidth = SIZE_WRAP_CONTENT;
        }
        if (nHeight <= 0)
        {
            nHeight = SIZE_WRAP_CONTENT;
        }

        if (nWidth == SIZE_WRAP_CONTENT)
            nWidth = szRoot.cx + szNc.cx;
        if (nHeight == SIZE_WRAP_CONTENT)
            nHeight = szRoot.cy + szNc.cy;
        GetRoot()->GetEventSet()->subscribeEvent(EventSwndSize::EventID, Subscriber(&SHostWnd::onRootResize, this));
    }

    if (nWidth != m_szAppSetted.cx || nHeight != m_szAppSetted.cy)
    {
        UpdateAutoSizeCount(true);
        SetWindowPos(NULL, 0, 0, nWidth, nHeight, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
        UpdateAutoSizeCount(false);
    }

    CRect rcClient;
    SNativeWnd::GetClientRect(&rcClient);
    GetRoot()->OnRelayout(rcClient);

    //设置重绘标记
    m_bNeedAllRepaint = TRUE;
    m_bNeedRepaint = TRUE;
    m_rgnInvalidate->Clear();

    EventInit evt(GetRoot());
    GetRoot()->FireEvent(evt);

    // handle user xml node
    SXmlNode xmlChild = xmlNode.first_child();
    while (xmlChild)
    {
        if (xmlChild.get_userdata() != 1)
        {
            OnUserXmlNode(xmlChild);
        }
        else
        {
            xmlChild.set_userdata(0);
        }
        xmlChild = xmlChild.next_sibling();
    }

    if (s_HideLocalUiDef)
        EnablePrivateUiDef(FALSE);

    return TRUE;
}

void SHostWnd::_Redraw()
{
    m_bNeedAllRepaint = TRUE;
    m_rgnInvalidate->Clear();
    _Invalidate(NULL);
}

void SHostWnd::_RedrawRegion(IRegionS *pRgnUpdate, CRect &rcInvalid)
{
    m_memRT->BeginDraw();
    CRect rcWnd = m_pRoot->GetWindowRect();

    SPainter painter;
    m_pRoot->BeforePaint(m_memRT, painter);

    if (!pRgnUpdate->IsEmpty())
    {
        pRgnUpdate->GetRgnBox(&rcInvalid);
        rcInvalid.IntersectRect(rcInvalid, rcWnd);
        m_memRT->PushClipRegion(pRgnUpdate, RGN_COPY);
    }
    else
    {
        rcInvalid = rcWnd;
        pRgnUpdate->CombineRect(&rcWnd, RGN_COPY);
        m_memRT->PushClipRect(&rcInvalid, RGN_COPY);
    }
    //清除残留的alpha值
    m_memRT->ClearRect(rcInvalid, 0);

    int clipState = 0;
    m_memRT->SaveClip(&clipState);
    _ExcludeVideoCanvasFromPaint(m_memRT); // exclude video canvas region from normal paint routine.
    GetRoot()->RedrawRegion(m_memRT, pRgnUpdate);
    m_memRT->RestoreClip(clipState);
    _PaintVideoCanvasForeground(m_memRT); // paint foreground of video canvas.

    m_memRT->PopClip();

    m_pRoot->AfterPaint(m_memRT, painter);
    m_memRT->EndDraw();
}

void SHostWnd::OnPrint(HDC dc, UINT uFlags)
{
    if (!(GetRoot()->IsLayoutDirty() || IsWindowVisible()))
        return;
    //刷新前重新布局，会自动检查布局脏标志
    GetRoot()->UpdateLayout();

    if (m_bNeedAllRepaint)
    {
        m_rgnInvalidate->Clear();
        m_bNeedAllRepaint = FALSE;
        m_bNeedRepaint = TRUE;
    }

    CRect rcInvalid;
    if (m_bNeedRepaint)
    {
        m_bNeedRepaint = FALSE;
        BuildWndTreeZorder();
        // m_rgnInvalidate有可能在RedrawRegion时被修改，必须生成一个临时的区域对象
        SAutoRefPtr<IRegionS> pRgnUpdate = m_rgnInvalidate;
        m_rgnInvalidate = NULL;
        GETRENDERFACTORY->CreateRegion(&m_rgnInvalidate);
        _RedrawRegion(pRgnUpdate, rcInvalid);
    }
    else
    { //缓存已经更新好了，只需要重新更新到窗口
        m_rgnInvalidate->GetRgnBox(&rcInvalid);
        m_rgnInvalidate->Clear();
    }
    if (dc)
    { //由系统发的WM_PAINT或者WM_PRINT产生的重绘请求
        CRect rcUpdate;
        ::GetClipBox(dc, &rcUpdate);
        rcInvalid = rcInvalid | rcUpdate;
    }
    UpdatePresenter(dc, m_memRT, rcInvalid, 255, uFlags);
}

void SHostWnd::OnPaint(HDC dc)
{
    PAINTSTRUCT ps;
    dc = ::BeginPaint(m_hWnd, &ps);
#ifdef _WIN32
    OnPrint(m_hostAttr.m_bTranslucent ? NULL : dc);
#else
    OnPrint(dc);
#endif //_WIN32
    ::EndPaint(m_hWnd, &ps);
}

BOOL SHostWnd::OnEraseBkgnd(HDC dc)
{
    return TRUE;
}

IToolTip *SHostWnd::CreateTooltip() const
{
    return GETTOOLTIPFACTORY->CreateToolTip(m_hWnd);
}

void SHostWnd::DestroyTooltip(IToolTip *pTooltip) const
{
    GETTOOLTIPFACTORY->DestroyToolTip(pTooltip);
}

SXmlNode SHostWnd::OnGetInitXmlNode(SXmlDoc &xmlDoc)
{
    OnLoadLayoutFromResourceID(xmlDoc);
    return xmlDoc.root().first_child();
}

BOOL SHostWnd::OnLoadLayoutFromResourceID(SXmlDoc &xmlDoc)
{
    if (m_strXmlLayout.IsEmpty())
    {
        SSLOGW() << "resId is empty";
        return FALSE;
    }
    return LOADXML(xmlDoc, m_strXmlLayout);
}

SRootWindow *SHostWnd::CreateRoot()
{
    return new SRootWindow(this);
}

int SHostWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (!m_presenter)
    {
        m_presenter.Attach(new SHostPresenter(this));
    }
    m_presenter->OnHostCreate();
    m_dwThreadID = GetCurrentThreadId();
    SHostMgr::getSingletonPtr()->AddHostMsgHandler(m_hWnd);
    UpdateAutoSizeCount(true);
    m_memRT = NULL;
    CRect rcWnd = GetClientRect();
    GETRENDERFACTORY->CreateRenderTarget(&m_memRT, rcWnd.Width(), rcWnd.Height());
    m_rgnInvalidate = NULL;
    GETRENDERFACTORY->CreateRegion(&m_rgnInvalidate);
    m_szAppSetted.cx = lpCreateStruct->cx;
    m_szAppSetted.cy = lpCreateStruct->cy;

    if (m_pRoot)
    {
        m_pRoot->Release();
        m_pRoot = NULL;
    }
    m_pRoot = CreateRoot();
    m_pRoot->SetContainer(this);
    SwndContainerImpl::SetRoot(m_pRoot);

    // load xml resource
    SXmlDoc xmlDoc;
    SXmlNode xmlRoot;
    IXmlNode *pXmlRoot = m_xmlInit;
    if (!pXmlRoot)
    {
        xmlRoot = OnGetInitXmlNode(xmlDoc);
        if (!xmlRoot)
        {
            SSLOGW() << "OnGetInitXmlNode return empty xml";
        }
        else
        {
            pXmlRoot = &xmlRoot;
        }
    }
    if (pXmlRoot && !InitFromXml(pXmlRoot))
        return -1;
    GetRoot()->RequestRelayout();
    m_pTipCtrl = CreateTooltip();
    if (m_pTipCtrl && m_hostAttr.m_bHasMsgLoop)
        GetMsgLoop()->AddMessageFilter(m_pTipCtrl);
    UpdateAutoSizeCount(false);
    return 0;
}

void SHostWnd::OnDestroy()
{
    m_presenter->OnHostDestroy();
    m_presenter = NULL;
    EventExit evt(GetRoot());
    GetRoot()->FireEvent(evt);

    GetRoot()->SSendMessage(WM_DESTROY);
    GetRoot()->SSendMessage(UM_SETSCALE, 100);
    if (m_pTipCtrl)
    {
        if (m_hostAttr.m_bHasMsgLoop)
            GetMsgLoop()->RemoveMessageFilter(m_pTipCtrl);
        DestroyTooltip(m_pTipCtrl);
        m_pTipCtrl = NULL;
    }
    if (m_pScriptModule)
    {
        IIdleHandler *pIdleHandler = m_pScriptModule->getIdleHandler();
        if (pIdleHandler)
            GetMsgLoop()->RemoveIdleHandler(pIdleHandler);
        m_pScriptModule = NULL;
    }

    if (!s_HideLocalUiDef)
        EnablePrivateUiDef(FALSE);
    SASSERT(m_cEnableUiDefCount == 0);
    m_privateUiDefInfo = NULL;

    m_memRT = NULL;
    m_rgnInvalidate = NULL;

    // clear pending tasks.
    SPOSITION pos = m_runnables.GetHeadPosition();
    while (pos)
    {
        IRunnable *pRunnable = m_runnables.GetNext(pos);
        pRunnable->Release();
    }
    m_runnables.RemoveAll();

    SHostMgr::getSingletonPtr()->RemoveHostMsgHandler(m_hWnd);
    // exit app. (copy from wtl)
    if (m_hostAttr.m_wndType == SHostWndAttr::WT_APPMAIN || (m_hostAttr.m_wndType == SHostWndAttr::WT_UNDEFINE && (SNativeWnd::GetStyle() & (WS_CHILD | WS_POPUP)) == 0 && (SNativeWnd::GetExStyle() & WS_EX_TOOLWINDOW) == 0))
        GetMsgLoop()->Quit(1);
}

void SHostWnd::OnSize(UINT nType, CSize size)
{
    SetMsgHandled(FALSE); // chain wm_size to ncpainter.
    SNcPainter::updateSystemButton(GetRoot(), nType);
    if (IsIconic())
        return;
    if (size.cx == 0 || size.cy == 0)
        return;

    BOOL bSizeChange = size != m_szPrev;
    // SSLOGI() << "on host size,szPrev=" << m_szPrev.cx << "," << m_szPrev.cy << " size=" << size.cx << "," << size.cy << " sizeChange=" << bSizeChange;
    if (bSizeChange)
    {
        if (m_nAutoSizing == 0)
        {
            m_szAppSetted = size;
        }
        m_bResizing = TRUE;
        m_memRT->Resize(size);
        BOOL bDirty = GetRoot()->IsLayoutDirty();
        GetRoot()->OnRelayout(CRect(0, 0, size.cx, size.cy));
        if (m_nAutoSizing && bDirty)
            GetRoot()->m_layoutDirty = SWindow::dirty_self;
        m_presenter->OnHostResize(size);
        m_bResizing = FALSE;
        m_szPrev = size;
    }
    _Redraw();
}

void SHostWnd::OnMouseMove(UINT nFlags, CPoint point)
{
    if (!m_bTrackFlag)
    {
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(tme);
        tme.hwndTrack = m_hWnd;
        tme.dwFlags = TME_LEAVE;
        tme.dwHoverTime = 0;
        m_bTrackFlag = TrackMouseEvent(&tme);
    }
    OnMouseEvent(WM_MOUSEMOVE, nFlags, MAKELPARAM(point.x, point.y));
}

void SHostWnd::OnMouseLeave()
{
    m_bTrackFlag = FALSE;
    OnMouseEvent(WM_MOUSELEAVE, 0, 0);
}

BOOL SHostWnd::OnSetCursor(HWND hwnd, UINT nHitTest, UINT message)
{
    if (hwnd != m_hWnd)
        return FALSE;
    if (nHitTest == HTCLIENT)
    {
        CPoint pt;
        GetCursorPos(&pt);
        ScreenToClient(&pt);
        return DoFrameEvent(WM_SETCURSOR, 0, MAKELPARAM(pt.x, pt.y)) != 0;
    }
    return DefWindowProc() != 0;
}

void SHostWnd::OnTimer(UINT_PTR idEvent)
{
    if (m_memRT == NULL) // host was destroyed
        return;
    if (idEvent == kPulseTimer)
    {
        if (!IsIconic())
        {
            SwndContainerImpl::OnNextFrame();
        }
        return;
    }
    else if (idEvent == kTaskTimer)
    {
        OnRunTasks(0, 0, 0);
        return;
    }

    STimerID sTimerID((DWORD)idEvent);
    if (sTimerID.bSwndTimer)
    {
        SWindow *pSwnd = SWindowMgr::GetWindow((SWND)sTimerID.swnd);
        if (pSwnd)
        {
            pSwnd->SSendMessage(WM_TIMER, sTimerID.uTimerID, 0);
        }
        else
        {
            //窗口已经删除，自动清除该窗口的定时器
            ::KillTimer(m_hWnd, idEvent);
        }
    }
    else
    {
        SetMsgHandled(FALSE);
    }
}

LRESULT SHostWnd::OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK:
        m_msgMouse.message = uMsg;
        m_msgMouse.wParam = wParam;
        m_msgMouse.lParam = lParam;
        if (SNativeWnd::GetStyle() & WS_CHILD)
            SNativeWnd::SetFocus(); //子窗口情况下才自动获取焦点
        break;
    case WM_LBUTTONUP:
        if (!(SNativeWnd::GetStyle() & WS_CHILD))
        {
            m_pNcPainter->OnLButtonUp(wParam, lParam);
        }
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        m_msgMouse.message = 0;
        break;
    case WM_MOUSEMOVE:
        m_msgMouse.message = 0;
        m_msgMouse.wParam = wParam;
        m_msgMouse.lParam = lParam;
        m_pNcPainter->OnMouseMove(wParam, lParam);
        break;
    case WM_MOUSELEAVE:
        m_msgMouse.message = uMsg;
        m_msgMouse.wParam = 0;
        m_msgMouse.lParam = 0;
        break;
    default:
        break;
    }

    DoFrameEvent(uMsg, wParam, lParam); //将鼠标消息转发到SWindow处理

    UpdateTooltip();

    return 0;
}

LRESULT SHostWnd::OnKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP)
    {
        SWindow *pFocus = SWindowMgr::GetWindow(m_focusMgr.GetFocusedHwnd());
        if (!pFocus || !(pFocus->OnGetDlgCode() & SC_WANTSYSKEY))
        {
            SetMsgHandled(FALSE);
            return 0;
        }
    }
    LRESULT lRet = DoFrameEvent(uMsg, wParam, lParam);
    SetMsgHandled(GetRoot()->IsMsgHandled());
    return lRet;
}

LRESULT SHostWnd::OnActivateApp(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!m_presenter) //防止host.OnDestroy中destroy子窗口可能给host发这个消息
        return 0;
    return DoFrameEvent(uMsg, wParam, lParam);
}

BOOL SHostWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    ScreenToClient(&pt);
    return DoFrameEvent(WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt.x, pt.y)) != 0;
}

void SHostWnd::OnActivate(UINT nState, BOOL bMinimized, HWND wndOther)
{
    if (nState != WA_INACTIVE)
    {
        ::SetFocus(m_hWnd);
    }
    else
    {
        ::SetFocus(NULL);
        _RestoreClickState();
    }
}

BOOL SHostWnd::OnFireEvent(IEvtArgs *evt)
{
    if (m_evtHandler.fun)
    {
        if (m_evtHandler.fun(evt, m_evtHandler.ctx))
            return TRUE;
    }
    EventCmd *e2 = sobj_cast<EventCmd>(evt);
    if (e2)
    {
        bool bSysBtn = true;
        switch (e2->idFrom)
        {
        case SNcPainter::IDC_SYS_CLOSE:
            PostMessage(WM_SYSCOMMAND, SC_CLOSE);
            break;
        case SNcPainter::IDC_SYS_MIN:
            PostMessage(WM_SYSCOMMAND, SC_MINIMIZE);
            break;
        case SNcPainter::IDC_SYS_MAX:
            PostMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
            break;
        case SNcPainter::IDC_SYS_RESTORE:
            PostMessage(WM_SYSCOMMAND, SC_RESTORE);
            break;
        default:
            bSysBtn = false;
            break;
        }
        if (bSysBtn)
            return TRUE;
    }
    return _HandleEvent(evt);
}

void SHostWnd::GetContainerRect(RECT *ret) const
{
    *ret = GetRoot()->GetWindowRect();
}

HWND SHostWnd::GetHostHwnd()
{
    return m_hWnd;
}

void SHostWnd::UpdatePresenter(HDC dc, IRenderTarget *pRT, LPCRECT rcInvalid, BYTE byAlpha, UINT uFlag)
{
    byAlpha = (BYTE)((int)byAlpha * GetRoot()->GetAlpha() / 255);
    if (pRT->IsOffscreen() || uFlag != 0)
        m_presenter->OnHostPresent(dc, pRT, rcInvalid, byAlpha);
}

void SHostWnd::OnRedraw(LPCRECT rc, BOOL bClip)
{
    if (!IsWindow())
        return;

    m_rgnInvalidate->CombineRect(rc, bClip ? RGN_DIFF : RGN_OR);

    m_bNeedRepaint = TRUE;

    _Invalidate(rc);
}

void SHostWnd::UpdateRegion(IRegionS *rgn)
{
    CRect rcInvalid;
    _RedrawRegion(rgn, rcInvalid);
    UpdatePresenter(0, m_memRT, rcInvalid);
}

BOOL SHostWnd::OnReleaseSwndCapture()
{
    if (!SwndContainerImpl::OnReleaseSwndCapture())
        return FALSE;
    if (GetCapture() == m_hWnd)
        ReleaseCapture();
    CPoint pt;
    GetCursorPos(&pt);
    ScreenToClient(&pt);
    PostMessage(WM_MOUSEMOVE, 0, MAKELPARAM(pt.x, pt.y));
    return TRUE;
}

SWND SHostWnd::OnSetSwndCapture(SWND swnd)
{
    SNativeWnd::SetCapture();
    return SwndContainerImpl::OnSetSwndCapture(swnd);
}

BOOL SHostWnd::IsTranslucent() const
{
    return m_hostAttr.m_bTranslucent;
}

BOOL SHostWnd::IsSendWheel2Hover() const
{
    return m_hostAttr.m_bSendWheel2Hover;
}

BOOL SHostWnd::UpdateWindow(BOOL bForce)
{
    if (m_bResizing)
        return FALSE;
    if (!m_bNeedRepaint && !bForce)
        return FALSE;
    if (m_dummyWnd)
        ::UpdateWindow(m_dummyWnd->m_hWnd);
    else
        ::UpdateWindow(m_hWnd);
    return TRUE;
}

void SHostWnd::UpdateTooltip()
{
    if (!m_pTipCtrl)
        return;
    if (!m_hostAttr.m_bHasMsgLoop && GetCurrentMessage())
    {
        m_pTipCtrl->RelayEvent(GetCurrentMessage());
    }
    SWindow *pHover = SWindowMgr::GetWindow(m_hHover);
    if (m_msgMouse.message != 0 || !pHover)
    {
        m_pTipCtrl->ClearTip();
    }
    else
    {
        CPoint pt(GET_X_LPARAM(m_msgMouse.lParam), GET_Y_LPARAM(m_msgMouse.lParam));
        SwndToolTipInfo tipInfo;
        BOOL bOK = pHover->UpdateToolTip(pt, tipInfo);
        if (bOK)
        {
            _SetToolTipInfo(&tipInfo, FALSE);
        }
        else
        { // hide tooltip
            _SetToolTipInfo(NULL, FALSE);
        }
    }
}

void SHostWnd::SetToolTip(LPCRECT rc, UINT tipAlign, LPCTSTR pszTip)
{
    if (!m_pTipCtrl)
        return;
    CRect rc2;
    if (rc)
    {
        rc2 = *rc;
        ClientToScreen2(&rc2);
    }

    m_pTipCtrl->SetToolTip(rc2, tipAlign, pszTip, GetScale());
}

void SHostWnd::OnGetMinMaxInfo(LPMINMAXINFO lpMMI)
{
    HMONITOR hMonitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONULL);

    if (hMonitor && GetRoot())
    {
        MONITORINFO mi = { sizeof(MONITORINFO) };
        ::GetMonitorInfo(hMonitor, &mi);

        CRect rcWork = mi.rcWork, rcMonitor = mi.rcMonitor;
        CRect rcMaxInset = m_hostAttr.GetMaxInset(GetScale());
        lpMMI->ptMaxPosition.x = abs(rcWork.left - rcMonitor.left) - rcMaxInset.left;
        lpMMI->ptMaxPosition.y = abs(rcWork.top - rcMonitor.top) - rcMaxInset.top;
        lpMMI->ptMaxSize.x = abs(rcWork.Width()) + rcMaxInset.right + rcMaxInset.left;
        lpMMI->ptMaxSize.y = abs(rcWork.Height()) + rcMaxInset.bottom + rcMaxInset.top;
        lpMMI->ptMaxTrackSize.x = abs(rcWork.Width()) + rcMaxInset.right + rcMaxInset.left;
        lpMMI->ptMaxTrackSize.y = abs(rcWork.Height()) + rcMaxInset.bottom + rcMaxInset.top;
        CSize szMin = m_hostAttr.GetMinSize(GetScale());
        lpMMI->ptMinTrackSize = CPoint(szMin.cx, szMin.cy);
    }
    else
    {
        SetMsgHandled(FALSE);
    }
}

void SHostWnd::OnSetFocus(HWND wndOld)
{
    DoFrameEvent(WM_SETFOCUS, 0, 0);
}

void SHostWnd::OnKillFocus(HWND wndFocus)
{
    DoFrameEvent(WM_KILLFOCUS, 0, 0);
}

static BOOL _BitBlt(IRenderTarget *pRTDst, IRenderTarget *pRTSrc, CRect rcDst, CPoint ptSrc)
{
    return S_OK == pRTDst->BitBlt(&rcDst, pRTSrc, ptSrc.x, ptSrc.y, SRCCOPY);
}

BOOL SHostWnd::AnimateHostWindow(DWORD dwTime, DWORD dwFlags)
{
    if (!IsTranslucent())
    {
        return ::AnimateWindow(m_hWnd, dwTime, dwFlags);
    }
    else
    {
        CRect rcWnd; //窗口矩形
        SNativeWnd::GetClientRect(&rcWnd);
        CRect rcShow(rcWnd); //动画过程中可见部分

        SAutoRefPtr<IRenderTarget> pRT;
        GETRENDERFACTORY->CreateRenderTarget(&pRT, rcShow.Width(), rcShow.Height());

        _Redraw();
        GetRoot()->RedrawRegion(m_memRT, m_rgnInvalidate);

        int nSteps = dwTime / 10;
        const BYTE byAlpha = GetRoot()->GetAlpha();
        if (dwFlags & AW_HIDE)
        {
            if (dwFlags & AW_SLIDE)
            {
                LONG x1 = rcShow.left;
                LONG x2 = rcShow.left;
                LONG y1 = rcShow.top;
                LONG y2 = rcShow.top;
                LONG *x = &rcShow.left;
                LONG *y = &rcShow.top;

                if (dwFlags & AW_HOR_POSITIVE)
                { // left->right:move left
                    x1 = rcShow.left, x2 = rcShow.right;
                    x = &rcShow.left;
                }
                else if (dwFlags & AW_HOR_NEGATIVE)
                { // right->left:move right
                    x1 = rcShow.right, x2 = rcShow.left;
                    x = &rcShow.right;
                }
                if (dwFlags & AW_VER_POSITIVE)
                { // top->bottom
                    y1 = rcShow.top, y2 = rcShow.bottom;
                    y = &rcShow.top;
                }
                else if (dwFlags & AW_VER_NEGATIVE)
                { // bottom->top
                    y1 = rcShow.bottom, y2 = rcShow.top;
                    y = &rcShow.bottom;
                }
                LONG xStepLen = (x2 - x1) / nSteps;
                LONG yStepLen = (y2 - y1) / nSteps;

                for (int i = 0; i < nSteps; i++)
                {
                    *x += xStepLen;
                    *y += yStepLen;
                    pRT->BeginDraw();
                    pRT->ClearRect(rcWnd, 0);
                    CPoint ptAnchor;
                    if (dwFlags & AW_VER_NEGATIVE)
                        ptAnchor.y = rcWnd.bottom - rcShow.Height();
                    if (dwFlags & AW_HOR_NEGATIVE)
                        ptAnchor.x = rcWnd.right - rcShow.Width();
                    _BitBlt(pRT, m_memRT, rcShow, ptAnchor);
                    pRT->EndDraw();
                    UpdatePresenter(0, pRT, &rcWnd, byAlpha);
                    Sleep(10);
                }
                ShowWindow(SW_HIDE);
                return TRUE;
            }
            else if (dwFlags & AW_CENTER)
            {
                int xStep = rcShow.Width() / (2 * nSteps);
                int yStep = rcShow.Height() / (2 * nSteps);
                for (int i = 0; i < nSteps; i++)
                {
                    rcShow.DeflateRect(xStep, yStep);
                    pRT->BeginDraw();
                    pRT->ClearRect(rcWnd, 0);
                    _BitBlt(pRT, m_memRT, rcShow, rcShow.TopLeft());
                    pRT->EndDraw();
                    UpdatePresenter(0, pRT, rcWnd, byAlpha);
                    Sleep(10);
                }
                ShowWindow(SW_HIDE);
                return TRUE;
            }
            else if (dwFlags & AW_BLEND)
            {
                BYTE byAlpha2 = byAlpha;
                for (int i = 0; i < nSteps; i++)
                {
                    byAlpha2 -= 255 / nSteps;
                    UpdatePresenter(0, m_memRT, &rcWnd, byAlpha2);
                    Sleep(10);
                }
                ShowWindow(SW_HIDE);
                return TRUE;
            }
            return FALSE;
        }
        else
        {
            pRT->ClearRect(rcWnd, 0);
            UpdatePresenter(0, pRT, &rcWnd, byAlpha);
            SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW | ((dwFlags & AW_ACTIVATE) ? 0 : SWP_NOACTIVATE));

            if (dwFlags & AW_SLIDE)
            {
                LONG x1 = rcShow.left;
                LONG x2 = rcShow.left;
                LONG y1 = rcShow.top;
                LONG y2 = rcShow.top;
                LONG *x = &rcShow.left;
                LONG *y = &rcShow.top;

                if (dwFlags & AW_HOR_POSITIVE)
                { // left->right:move right
                    x1 = rcShow.left, x2 = rcShow.right;
                    rcShow.right = rcShow.left, x = &rcShow.right;
                }
                else if (dwFlags & AW_HOR_NEGATIVE)
                { // right->left:move left
                    x1 = rcShow.right, x2 = rcShow.left;
                    rcShow.left = rcShow.right, x = &rcShow.left;
                }
                if (dwFlags & AW_VER_POSITIVE)
                { // top->bottom
                    y1 = rcShow.top, y2 = rcShow.bottom;
                    rcShow.bottom = rcShow.top, y = &rcShow.bottom;
                }
                else if (dwFlags & AW_VER_NEGATIVE)
                { // bottom->top
                    y1 = rcShow.bottom, y2 = rcShow.top;
                    rcShow.top = rcShow.bottom, y = &rcShow.top;
                }
                LONG xStepLen = (x2 - x1) / nSteps;
                LONG yStepLen = (y2 - y1) / nSteps;

                for (int i = 0; i < nSteps; i++)
                {
                    *x += xStepLen;
                    *y += yStepLen;
                    pRT->BeginDraw();
                    pRT->ClearRect(rcWnd, 0);
                    CPoint ptAnchor;
                    if (dwFlags & AW_VER_POSITIVE)
                        ptAnchor.y = rcWnd.bottom - rcShow.Height();
                    if (dwFlags & AW_HOR_POSITIVE)
                        ptAnchor.x = rcWnd.right - rcShow.Width();
                    _BitBlt(pRT, m_memRT, rcShow, ptAnchor);
                    pRT->EndDraw();
                    UpdatePresenter(0, pRT, &rcWnd, byAlpha);
                    Sleep(10);
                }
                UpdatePresenter(0, m_memRT, &rcWnd, byAlpha);

                return TRUE;
            }
            else if (dwFlags & AW_CENTER)
            {
                int xStep = rcShow.Width() / (2 * nSteps);
                int yStep = rcShow.Height() / (2 * nSteps);
                rcShow.left = rcShow.right = (rcShow.left + rcShow.right) / 2;
                rcShow.top = rcShow.bottom = (rcShow.top + rcShow.bottom) / 2;
                for (int i = 0; i < nSteps; i++)
                {
                    rcShow.InflateRect(xStep, yStep);
                    pRT->BeginDraw();
                    pRT->ClearRect(rcWnd, 0);
                    _BitBlt(pRT, m_memRT, rcShow, rcShow.TopLeft());
                    pRT->EndDraw();
                    UpdatePresenter(0, pRT, &rcWnd, byAlpha);
                    Sleep(10);
                }
                UpdatePresenter(0, m_memRT, &rcWnd, byAlpha);
                return TRUE;
            }
            else if (dwFlags & AW_BLEND)
            {
                BYTE byAlpha2 = 0;
                for (int i = 0; i < nSteps; i++)
                {
                    byAlpha2 += byAlpha / nSteps;
                    UpdatePresenter(0, m_memRT, &rcWnd, byAlpha2);
                    Sleep(10);
                }
                UpdatePresenter(0, m_memRT, &rcWnd, byAlpha);
                return TRUE;
            }
        }
        return FALSE;
    }
}

BOOL SHostWnd::RegisterTimelineHandler(ITimelineHandler *pHandler)
{
    bool bEmpty1 = m_timelineHandlerMgr.IsEmpty();
    BOOL bRet = SwndContainerImpl::RegisterTimelineHandler(pHandler);
    bool bEmpty2 = m_timelineHandlerMgr.IsEmpty();
    if (bEmpty1 && !bEmpty2)
    {
        SNativeWnd::SetTimer(kPulseTimer, kPulseInterval, NULL);
    }
    return bRet;
}

BOOL SHostWnd::UnregisterTimelineHandler(ITimelineHandler *pHandler)
{
    bool bEmpty1 = m_timelineHandlerMgr.IsEmpty();
    BOOL bRet = SwndContainerImpl::UnregisterTimelineHandler(pHandler);
    bool bEmpty2 = m_timelineHandlerMgr.IsEmpty();
    if (!bEmpty1 && bEmpty2)
    {
        SNativeWnd::KillTimer(kPulseTimer);
    }
    return bRet;
}

LPCWSTR SHostWnd::GetTranslatorContext() const
{
    return m_hostAttr.m_strTrCtx;
}

IMessageLoop *SHostWnd::GetMsgLoop()
{
    tid_t tid = m_dwThreadID;
    if (tid == 0)
        tid = GetCurrentThreadId();
    return SApplication::getSingletonPtr()->GetMsgLoop(tid);
}

#if (!DISABLE_SWNDSPY)

LRESULT SHostWnd::OnSpyMsgSwndEnum(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!m_hostAttr.m_bAllowSpy)
        return 0;
    SWND swndCur = (SWND)wParam;
    if (swndCur == 0)
        swndCur = GetRoot()->m_swnd;
    SWindow *pSwnd = SWindowMgr::GetWindow(swndCur);
    if (!pSwnd)
        return 0;
    SWindow *pRet = pSwnd->GetWindow((int)lParam);
    if (!pRet)
        return 0;
    return pRet->GetSwnd();
}

LRESULT SHostWnd::OnSpyMsgSwndSpy(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!m_hostAttr.m_bAllowSpy)
        return 0;
    SWND swndCur = (SWND)wParam;
    if (swndCur == 0)
        swndCur = GetRoot()->m_swnd;
    SWindow *pSwnd = SWindowMgr::GetWindow(swndCur);
    if (!pSwnd)
        return 0;

    SWNDINFO *pSwndInfo = new SWNDINFO;
    COPYDATASTRUCT cds;
    cds.dwData = SPYMSG_SWNDINFO;
    cds.cbData = sizeof(SWNDINFO);
    cds.lpData = pSwndInfo;
    memset(pSwndInfo, 0, sizeof(SWNDINFO));

    pSwndInfo->swnd = swndCur;
    pSwnd->GetWindowRect(&pSwndInfo->rcWnd);
    pSwnd->GetClientRect(&pSwndInfo->rcClient);
    pSwndInfo->bVisible = pSwnd->IsVisible(TRUE);
    pSwndInfo->nID = pSwnd->GetID();

    SStringW strTmp = pSwnd->GetName();
    if (strTmp.GetLength() <= SWND_MAX_NAME)
        wcscpy(pSwndInfo->szName, strTmp);
    else
        wcscpy(pSwndInfo->szName, L"##buf overflow!");

    strTmp = pSwnd->GetObjectClass();
    if (strTmp.GetLength() <= SWND_MAX_CLASS)
        wcscpy(pSwndInfo->szClassName, strTmp);
    else
        wcscpy(pSwndInfo->szClassName, L"##buf overflow!");

    wcscpy(pSwndInfo->szXmlStr, L"##unavailable!");
#ifdef _DEBUG
    strTmp = pSwnd->m_strXml;
    if (strTmp.GetLength() <= SWND_MAX_XML)
        wcscpy(pSwndInfo->szXmlStr, strTmp);
    else
        wcscpy(pSwndInfo->szXmlStr, L"##buf overflow!");
#endif //_DEBUG
    ::SendMessage(m_hSpyWnd, WM_COPYDATA, (WPARAM)m_hWnd, (LPARAM)&cds);
    delete pSwndInfo;
    return 1;
}

LRESULT SHostWnd::OnSpyMsgSetSpy(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    m_hSpyWnd = (HWND)lParam;
    if (!::IsWindow(m_hSpyWnd))
    {
        m_hSpyWnd = 0;
        return 0;
    }
    return 1;
}

LRESULT SHostWnd::OnSpyMsgHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    ScreenToClient(&pt);
    return GetRoot()->SwndFromPoint(pt);
}
#endif // DISABLE_SWNDSPY

void SHostWnd::OnCaptureChanged(HWND wnd)
{
    if (wnd == m_hWnd)
        return;
    if (wnd != NULL)
    { //如果当前响应了鼠标按下消息，在lost capture时也应该响应弹起消息
#ifdef _WIN32
        TCHAR szClassName[30];
        ::GetClassName(wnd, szClassName, 30);
        if (_tcscmp(szClassName, _T("CLIPBRDWNDCLASS")) == 0)
            return; //在窗口内拖动时也可能产生capturechange消息。
#endif              //_WIN32
    }
    _RestoreClickState();
}

IScriptModule *SHostWnd::GetScriptModule()
{
    return m_pScriptModule;
}

int SHostWnd::GetScale() const
{
    return GetRoot()->GetScale();
}

//////////////////////////////////////////////////////////////////////////
BOOL SHostWnd::DestroyWindow()
{
    if (m_AniState != Ani_none)
    {
        if (m_AniState | Ani_win_exit)
            return TRUE;

        if (m_AniState & Ani_host)
            StopHostAnimation();
        if (m_AniState & Ani_win)
            GetRoot()->ClearAnimation();
        SASSERT(m_AniState == Ani_none);
    }
    if (m_pRoot->m_aniExit && !IsIconic())
    {
        m_pRoot->m_aniExit->setUserData(kAni4Destroy); // mark for destroy
        GetRoot()->StartAnimation(m_pRoot->m_aniExit);
        m_AniState |= Ani_win_exit;
        return TRUE;
    }
    else
    {
        return SNativeWnd::DestroyWindow();
    }
}

CRect SHostWnd::GetWindowRect() const
{
    CRect rc;
    SNativeWnd::GetWindowRect(&rc);
    return rc;
}

CRect SHostWnd::GetClientRect() const
{
    CRect rc;
    SNativeWnd::GetClientRect(&rc);
    return rc;
}

LRESULT SHostWnd::OnMenuExEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return OnFireEvent((IEvtArgs *)lParam);
}

void SHostWnd::OnWindowPosChanging(LPWINDOWPOS lpWndPos)
{ //默认不处理该消息，同时防止系统处理该消息
    if (lpWndPos->flags & SWP_SHOWWINDOW && m_bFirstShow)
    {
        if (m_pRoot->m_aniEnter)
        {
            m_AniState |= Ani_win_enter;
            m_pRoot->StartAnimation(m_pRoot->m_aniEnter);
            OnNextFrame();
        }
        m_bFirstShow = FALSE;
    }
}

void SHostWnd::OnWindowPosChanged(LPWINDOWPOS lpWndPos)
{
    //下面这一行不能删除，否则显示不正常。
    SetMsgHandled(FALSE);
    if (!m_dummyWnd)
        return;

    if (!(lpWndPos->flags & SWP_NOMOVE))
    {
        HMONITOR hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO info = { sizeof(MONITORINFO) };
        if (GetMonitorInfo(hMonitor, &info))
        {
            m_dummyWnd->SetWindowPos(NULL, info.rcWork.left, info.rcWork.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
        }
    }
    if ((lpWndPos->flags & SWP_SHOWWINDOW))
    {
        m_dummyWnd->SetWindowPos(NULL, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
    }
    else if ((lpWndPos->flags & SWP_HIDEWINDOW))
    {
        m_dummyWnd->SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
    }
}

LRESULT SHostWnd::OnGetObject(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifdef SOUI_ENABLE_ACC
    IAccessible *pAcc = NULL;
    DWORD dwObjectId = lParam;
    if (dwObjectId == OBJID_CLIENT)
    {
        pAcc = GetRoot()->GetAccessible();
    }
    else
    {
        SWindow *pWnd = SWindowMgr::GetWindow(dwObjectId);
        if (pWnd)
            pAcc = pWnd->GetAccessible(); // user swnd to identify a swindow.
    }

    if (pAcc != NULL)
    {
        return LresultFromObject(IID_IAccessible, wParam, pAcc);
    }
    else
#endif
    {
        SetMsgHandled(FALSE);
        return 0;
    }
}

void SHostWnd::_RestoreClickState()
{
    switch (m_msgMouse.message)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
        SendMessage(WM_LBUTTONUP, m_msgMouse.wParam, m_msgMouse.lParam);
        break;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
        SendMessage(WM_RBUTTONUP, m_msgMouse.wParam, m_msgMouse.lParam);
        break;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK:
        SendMessage(WM_MBUTTONUP, m_msgMouse.wParam, m_msgMouse.lParam);
        break;
    default:
        break;
    }
}

void SHostWnd::_Invalidate(LPCRECT prc)
{
#ifdef _WIN32
    if (!m_hostAttr.m_bTranslucent)
    {
        if (prc)
            SNativeWnd::InvalidateRect(prc, FALSE);
        else
            SNativeWnd::Invalidate(FALSE);
    }
    else if (m_dummyWnd)
    {
        m_dummyWnd->Invalidate(FALSE);
    }
#else
    if (prc)
        SNativeWnd::InvalidateRect(prc, FALSE);
    else
        SNativeWnd::Invalidate(FALSE);
#endif //_WIN32
}

bool SHostWnd::StartHostAnimation(IAnimation *pAni)
{
    SASSERT(pAni);
    if (!IsWindow())
        return false;
    StopHostAnimation();

    m_AniState |= Ani_host;
    m_hostAnimation = pAni;
    m_hostAnimation->startNow();

    CRect rcWnd;
    GetNative()->GetWindowRect(&rcWnd);
    CRect rcParent;
    HWND hParent = SNativeWnd::GetParent();
    if (hParent)
    {
        ::GetClientRect(hParent, &rcParent);
    }
    else
    {
        HMONITOR hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO info = { sizeof(MONITORINFO) };
        GetMonitorInfo(hMonitor, &info);
        rcParent = info.rcWork;
    }
    m_hostAnimation->initialize(rcWnd.Width(), rcWnd.Height(), rcParent.Width(), rcParent.Height(), GetScale());
    m_hostAnimationHandler.m_rcInit = rcWnd;
    RegisterTimelineHandler(&m_hostAnimationHandler);
    return true;
}

bool SHostWnd::StopHostAnimation()
{
    if (!m_hostAnimation)
        return false;
    m_hostAnimation->cancel();
    UnregisterTimelineHandler(&m_hostAnimationHandler);
    return true;
}

void SHostWnd::OnUserXmlNode(SXmlNode xmlUser)
{
    SSLOGD() << "unhandled xml node:" << xmlUser.name();
}

void SHostWnd::UpdateAutoSizeCount(bool bInc)
{
    if (bInc)
        m_nAutoSizing++;
    else
        m_nAutoSizing--;
}

void SHostWnd::EnableIME(BOOL bEnable)
{
    if (bEnable)
    {
        HIMC hImc = ImmGetContext(m_hWnd);
        if (!hImc)
        {
            hImc = ImmCreateContext();
            ImmAssociateContext(m_hWnd, hImc);
        }
    }
    else
    {
        HIMC hImc = ImmGetContext(m_hWnd);
        ImmAssociateContext(m_hWnd, NULL);
        if (hImc)
        {
            ImmDestroyContext(hImc);
        }
    }
}

void SHostWnd::OnUpdateCursor()
{
    CPoint pt;
    GetCursorPos(&pt);
    UINT ht = m_pNcPainter->OnNcHitTest(pt);
    PostMessage(WM_SETCURSOR, (WPARAM)m_hWnd, MAKELPARAM(ht, WM_MOUSEMOVE));
}

BOOL SHostWnd::ShowWindow(int nCmdShow)
{
    UpdateAutoSizeCount(true);
    BOOL bRet = SNativeWnd::ShowWindow(nCmdShow);
    UpdateAutoSizeCount(false);
    return bRet;
}

void SHostWnd::OnHostAnimationStarted(IAnimation *pAni)
{
}

void SHostWnd::OnHostAnimationStoped(IAnimation *pAni)
{
    m_AniState &= ~Ani_host;
}

void SHostWnd::OnSysCommand(UINT nID, CPoint lParam)
{
    if (nID == SC_RESTORE)
    {
        UpdateAutoSizeCount(true);
        DefWindowProc();
        UpdateAutoSizeCount(false);
    }
    else
    {
        DefWindowProc();
    }
    SetMsgHandled(TRUE); // todo: hjx, drag window may result in msgHandled flag been set to 0, fix it later.
}

void SHostWnd::_SetToolTipInfo(const SwndToolTipInfo *info, BOOL bNcTip)
{
    if (!m_pTipCtrl)
        return;
    if (info)
    {
        TIPID id = { info->swnd, info->dwCookie, bNcTip };
        m_pTipCtrl->UpdateTip(&id, info->rcTarget, info->strTip, GetScale());
    }
    else
    {
        m_pTipCtrl->ClearTip();
    }
}

void SHostWnd::SetEventHandler(THIS_ FunCallback fun, void *ctx)
{
    m_evtHandler.fun = fun;
    m_evtHandler.ctx = ctx;
}

EventHandlerInfo *SHostWnd::GetEventHandler(THIS)
{
    return &m_evtHandler;
}

BOOL SHostWnd::_HandleEvent(IEvtArgs *pEvt)
{
    return FALSE;
}

void SHostWnd::_ExcludeVideoCanvasFromPaint(IRenderTarget *pRT)
{
    SPOSITION pos = m_lstVideoCanvas.GetHeadPosition();
    while (pos)
    {
        SWND swnd = m_lstVideoCanvas.GetNext(pos);
        SWindow *pWnd = SWindowMgr::GetWindow(swnd);
        if (pWnd && pWnd->IsVisible(TRUE))
        {
            CRect rcCanvas;
            pWnd->GetVisibleRect(&rcCanvas);
            pRT->PushClipRect(&rcCanvas, RGN_DIFF);
        }
    }
}

void SHostWnd::_PaintVideoCanvasForeground(IRenderTarget *pRT)
{
    SPOSITION pos = m_lstVideoCanvas.GetHeadPosition();
    while (pos)
    {
        SWND swnd = m_lstVideoCanvas.GetNext(pos);
        SWindow *pWnd = SWindowMgr::GetWindow(swnd);
        if (pWnd && pWnd->IsVisible(TRUE))
        {
            CRect rcCanvas;
            pWnd->GetVisibleRect(&rcCanvas);
            if (!rcCanvas.IsRectEmpty())
            {
                pWnd->SSendMessage(WM_ERASEBKGND, (WPARAM)pRT);
                pWnd->SSendMessage(WM_PAINT, (WPARAM)pRT);
                pWnd->PaintForeground(pRT, &rcCanvas);
            }
        }
    }
}

IHostPresenter *SHostWnd::GetPresenter(THIS)
{
    return m_presenter;
}

void SHostWnd::SetPresenter(THIS_ IHostPresenter *pPresenter)
{
    m_presenter = pPresenter;
}

void SHostWnd::EnableDragDrop(THIS)
{
    ::RegisterDragDrop(m_hWnd, GetDropTarget());
}

void SHostWnd::ShowHostWnd(THIS_ int nShowCmd, BOOL bWaitAniDone)
{
    if (nShowCmd != SW_HIDE)
    {
        if (IsWindowVisible())
            return;
        ShowWindow(nShowCmd);
        if (m_pRoot->m_aniEnter)
        {
            m_AniState |= Ani_win_enter;
            m_pRoot->StartAnimation(m_pRoot->m_aniEnter);
            OnNextFrame();
        }
        else
        {
            bWaitAniDone = FALSE;
        }
    }
    else
    {
        // hide
        if (!IsWindowVisible())
            return;
        if (m_AniState & Ani_win)
            m_pRoot->ClearAnimation();
        if (m_pRoot->m_aniExit)
        {
            m_AniState |= Ani_win_exit;
            m_pRoot->m_aniExit->setUserData(kAni4Hide); // mark for hide
            m_pRoot->StartAnimation(m_pRoot->m_aniExit);
        }
        else
        {
            SNativeWnd::SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
            bWaitAniDone = FALSE;
        }
    }
    if (bWaitAniDone)
    {
        SAutoRefPtr<IMessageLoop> msgLoop;
        SApplication::getSingletonPtr()->GetMsgLoopFactory()->CreateMsgLoop(&msgLoop, GetMsgLoop());
        for (; m_AniState & Ani_win;)
        {
            if (!msgLoop->WaitMsg())
                break;
            msgLoop->HandleMsg();
        }
    }
}

void SHostWnd::OnCommand(UINT uNotifyCode, int nID, HWND wndCtl)
{
    if (uNotifyCode == 0)
    {
        // for menu, translate to evt_menu_cmd
        m_pRoot->FireMenuCmd(nID);
    }
    else
    {
        SetMsgHandled(FALSE);
    }
}

void SHostWnd::SetScale(THIS_ int nScale, LPCRECT desRect)
{
    EnablePrivateUiDef(TRUE);
    GetRoot()->SDispatchMessage(UM_SETSCALE, nScale, 0);
    GetNcPainter()->GetRoot()->SDispatchMessage(UM_SETSCALE, nScale, 0);
    UpdateAutoSizeCount(true);
    SetWindowPos(NULL, desRect->left, desRect->top, desRect->right - desRect->left, desRect->bottom - desRect->top, SWP_NOZORDER | SWP_NOACTIVATE);
    UpdateAutoSizeCount(false);
    EnablePrivateUiDef(FALSE);
}

LRESULT SHostWnd::OnUpdateFont(UINT uMsg, WPARAM wp, LPARAM lp)
{
    GetNcPainter()->GetRoot()->SDispatchMessage(uMsg);
    GetRoot()->SDispatchMessage(uMsg);
    GetRoot()->RequestRelayout();

    return 0;
}

void SHostWnd::EnableHostPrivateUiDef(THIS_ BOOL bEnable)
{
    EnablePrivateUiDef(bEnable);
}

BOOL SHostWnd::PostTask(THIS_ IRunnable *runable, BOOL bAsync /*DEF_VAL(TRUE)*/)
{
    m_cs.Enter();
    m_runnables.AddTail(runable->clone());
    int tasks = (int)m_runnables.GetCount();
    if (!bAsync)
    {
        m_cs.Leave();
        SendMessage(UM_RUN_TASKS);
        return TRUE;
    }
    else if (tasks > s_TaskQueueBufSize)
    {
        PostMessage(UM_RUN_TASKS);
    }
    else if (tasks == 1)
    {
        SetTimer(kTaskTimer, kTaskInterval);
    }
    m_cs.Leave();
    return TRUE;
}

int SHostWnd::RemoveTasksForObject(THIS_ void *pObj)
{
    int nRet = 0;
    SAutoLock lock(m_cs);
    SPOSITION pos = m_runnables.GetHeadPosition();
    while (pos)
    {
        SPOSITION pos2 = pos;
        IRunnable *p = m_runnables.GetNext(pos);
        if (p->getObject() == pObj)
        {
            p->Release();
            m_runnables.RemoveAt(pos2);
            nRet++;
        }
    }
    SAutoLock lock2(m_csRunningQueue);
    pos = m_runningQueue.GetHeadPosition();
    while (pos)
    {
        SPOSITION pos2 = pos;
        IRunnable *p = m_runningQueue.GetNext(pos);
        if (p->getObject() == pObj)
        {
            p->Release();
            m_runningQueue.RemoveAt(pos2);
            nRet++;
        }
    }
    return nRet;
}

LRESULT SHostWnd::OnRunTasks(UINT uMsg, WPARAM wp, LPARAM lp)
{
    m_cs.Enter();
    KillTimer(kTaskTimer);
    m_runningQueue.Swap(m_runnables);
    m_cs.Leave();
    for (;;)
    {
        SAutoLock lock(m_csRunningQueue);
        if (m_runningQueue.IsEmpty())
            break;
        IRunnable *pRunnable = m_runningQueue.GetHead();
        m_runningQueue.RemoveHead();
        pRunnable->run();
        pRunnable->Release();
    }
    return 0;
}

LRESULT SHostWnd::OnSetLanguage(UINT uMsg, WPARAM wp, LPARAM lp)
{
    GetNcPainter()->GetRoot()->SDispatchMessage(uMsg);
    GetNcPainter()->GetRoot()->RequestRelayout();
    GetNcPainter()->GetRoot()->Invalidate();
    GetRoot()->SDispatchMessage(uMsg);
    GetRoot()->RequestRelayout();
    return 0;
}

//////////////////////////////////////////////////////////////////
//  SHostWnd::SHostAnimationHandler
void SHostWnd::SHostAnimationHandler::OnNextFrame()
{
    if (!m_pHostWnd->m_hostAnimation)
        return;
    IAnimation *pAni = m_pHostWnd->m_hostAnimation;
    uint64_t tm = pAni->getStartTime();
    if (tm == -1)
    {
        m_pHostWnd->OnHostAnimationStarted(pAni);
    }
    STransformation xform;
    BOOL bMore = m_pHostWnd->m_hostAnimation->getTransformation(STime::GetCurrentTimeMs(), &xform);
    SMatrix mtx = xform.getMatrix();
    mtx.preTranslate((int)-m_rcInit.left, (int)-m_rcInit.top);
    mtx.postTranslate((int)m_rcInit.left, (int)m_rcInit.top);
    if (mtx.rectStaysRect())
    {
        SRect rc = SRect::IMake(m_rcInit);
        mtx.mapRect(&rc);
        CRect rc2 = rc.toRect();
        ::SetWindowPos(m_pHostWnd->m_hWnd, NULL, rc2.left, rc2.top, rc2.Width(), rc2.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
    }
    if (xform.hasAlpha())
    { // change alpha.
        if (m_pHostWnd->IsTranslucent())
        {
            CRect rcWnd = m_pHostWnd->GetClientRect();
            IRenderTarget *pRT = m_pHostWnd->m_memRT;
            m_pHostWnd->UpdatePresenter(0, pRT, &rcWnd, xform.GetAlpha());
        }
        else if (m_pHostWnd->GetExStyle() & WS_EX_LAYERED)
        {
            m_pHostWnd->SetLayeredWindowAttributes(0, xform.GetAlpha(), LWA_ALPHA);
        }
    }
    if (!bMore)
    {
        m_pHostWnd->UnregisterTimelineHandler(this);
        m_pHostWnd->OnHostAnimationStoped(pAni);
    }
}

SNSEND