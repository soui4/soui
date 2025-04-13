﻿#include "include/souistd.h"
#include "helper/SMenuEx.h"
#include "helper/SplitString.h"
#include <layout/SLinearLayout.h>
SNSBEGIN
#define TIMERID_POPSUBMENU 100
#define TIME_PUPSUBMENU    500

#define WIDTH_MENU_INIT 10000
#define WIDTH_MENU_MAX  2000
#define WIDTH_MENU_MIN  100.f

#define Y_MIDFLAG  L"-1000px"
#define Y_IMIDFLAG (-1000)

//////////////////////////////////////////////////////////////////////////
class SMenuExRoot : public SRootWindow {
    DEF_SOBJECT(SWindow, L"menuRoot")
    friend class SMenuEx;
    friend class SMenuExItem;

  protected:
    SAutoRefPtr<ISkinObj> m_pItemSkin;
    SAutoRefPtr<ISkinObj> m_pIconSkin;
    SAutoRefPtr<ISkinObj> m_pCheckSkin;
    SAutoRefPtr<ISkinObj> m_pArrowSkin;
    SAutoRefPtr<ISkinObj> m_pSepSkin;

    SMenuEx *m_pMenuEx;

    SLayoutSize m_nItemHei;
    SLayoutSize m_nIconBarWidth;
    SLayoutSize m_nTextOffset;
    SLayoutSize m_iconX, m_iconY;
    SLayoutSize m_nMinWidth;
    SLayoutSize m_nSubMenuOffset;

    DWORD m_dwContextHelpId;

    void Copy(SMenuExRoot *pNewMenuExRoot)
    {
        pNewMenuExRoot->m_pNcSkin = m_pNcSkin;
        pNewMenuExRoot->m_pItemSkin = m_pItemSkin;
        pNewMenuExRoot->m_pIconSkin = m_pIconSkin;
        pNewMenuExRoot->m_pCheckSkin = m_pCheckSkin;
        pNewMenuExRoot->m_pArrowSkin = m_pArrowSkin;
        pNewMenuExRoot->m_pSepSkin = m_pSepSkin;
        pNewMenuExRoot->m_nItemHei = m_nItemHei;
        pNewMenuExRoot->m_nIconBarWidth = m_nIconBarWidth;
        pNewMenuExRoot->m_nTextOffset = m_nTextOffset;
        pNewMenuExRoot->m_iconX = m_iconX;
        pNewMenuExRoot->m_iconY = m_iconY;
        pNewMenuExRoot->m_nMinWidth = m_nMinWidth;
        pNewMenuExRoot->m_nSubMenuOffset = m_nSubMenuOffset;
        pNewMenuExRoot->m_style = m_style; // 设置了 些 margin 之类的 属性 也要 copy
    }

    HRESULT OnAttrIconPos(const SStringW &strValue, BOOL bLoading);

    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"itemSkin", m_pItemSkin, FALSE)
        ATTR_SKIN(L"checkSkin", m_pCheckSkin, FALSE)
        ATTR_SKIN(L"sepSkin", m_pSepSkin, FALSE)
        ATTR_SKIN(L"arrowSkin", m_pArrowSkin, FALSE)
        ATTR_LAYOUTSIZE(L"itemHeight", m_nItemHei, FALSE)
        ATTR_CUSTOM(L"iconPos", OnAttrIconPos)
        ATTR_SKIN(L"iconSkin", m_pIconSkin, FALSE)
        ATTR_LAYOUTSIZE(L"iconBarWidth", m_nIconBarWidth, FALSE)
        ATTR_LAYOUTSIZE(L"textOffset", m_nTextOffset, FALSE)
        ATTR_LAYOUTSIZE(L"minWidth", m_nMinWidth, FALSE)
        ATTR_LAYOUTSIZE(L"subMenuOffset", m_nSubMenuOffset, FALSE)
        ATTR_DWORD(L"contextHelpId", m_dwContextHelpId, FALSE)
    SOUI_ATTRS_END()

  public:
    SMenuExRoot(SMenuEx *pMenuEx)
        : m_pItemSkin(GETBUILTINSKIN(SKIN_SYS_MENU_SKIN))
        , m_pSepSkin(GETBUILTINSKIN(SKIN_SYS_MENU_SEP))
        , m_pCheckSkin(GETBUILTINSKIN(SKIN_SYS_MENU_CHECK))
        , m_pArrowSkin(GETBUILTINSKIN(SKIN_SYS_MENU_ARROW))
        , m_pIconSkin(NULL)
        , m_pMenuEx(pMenuEx)
        , m_dwContextHelpId(0)
        , SRootWindow(pMenuEx)
    {
        m_nItemHei.setSize(26.f, SLayoutSize::dp);
        m_nIconBarWidth.setSize(24.f, SLayoutSize::dp);
        m_nMinWidth.setSize(WIDTH_MENU_MIN, SLayoutSize::dp);
        OnAttrLayout(SVBox::GetClassName(), TRUE); // set layout to vbox
        GetLayoutParam()->SetWrapContent(Both);
    }

    SMenuExItem *GetNextMenuItem(SMenuExItem *pItem, BOOL bForword, int nCount = 0);

    CSize CalcMenuSize()
    {
        HMONITOR hMonitor = MonitorFromWindow(m_pMenuEx->m_hWnd, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO monitorInfo = { sizeof(MONITORINFO), 0 };
        GetMonitorInfo(hMonitor, &monitorInfo);
        int maxHei = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
        CSize szRet;
        GetDesiredSize(&szRet, -1, maxHei);
        return szRet;
    }

    STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE *psz, int wid, int hei) OVERRIDE
    {
        __baseCls::GetDesiredSize(psz, wid, hei);
        if (psz->cx > m_nMaxWidth.toPixelSize(GetScale()) && !m_nMaxWidth.isWrapContent())
            psz->cx = m_nMaxWidth.toPixelSize(GetScale());
        if (psz->cx < m_nMinWidth.toPixelSize(GetScale()))
            psz->cx = m_nMinWidth.toPixelSize(GetScale());
    }

    STDMETHOD_(BOOL, InitFromXml)(THIS_ IXmlNode *pNode) OVERRIDE
    {
        SXmlNode xmlNode(pNode);
        // 找到根节点，获取在根节点上配置的全局菜单对象属性
        SXmlNode xmlRoot = xmlNode.root().first_child();
        if (xmlNode != xmlRoot)
        {
            __baseCls::__baseCls::InitFromXml(&xmlRoot); // IObject::InitFromXml
        }
        BOOL bRet = __baseCls::InitFromXml(&xmlNode);
        SetWindowText(_T("")); // 防止子菜单显示父级菜单项的文本。
        return TRUE;
    }

    BOOL CreateChildren(SXmlNode xmlNode) OVERRIDE
    {
        SXmlNode xmlItem = xmlNode.first_child();
        while (xmlItem)
        {
            SWindow *pMenuItem = CreateMenuItem(xmlItem.name());
            if (pMenuItem)
            {
                InsertChild(pMenuItem);
                pMenuItem->InitFromXml(&xmlItem);
                pMenuItem->GetLayoutParam()->SetMatchParent(Horz);
            }
            xmlItem = xmlItem.next_sibling();
        }
        return TRUE;
    }

    STDMETHOD_(void, UpdateChildrenPosition)(THIS) OVERRIDE
    {
        GetLayout()->LayoutChildren(this);
    }

    virtual void OnScaleChanged(int nScale)
    {
        __baseCls::OnScaleChanged(nScale);
        GetScaleSkin(m_pItemSkin, nScale);
        GetScaleSkin(m_pIconSkin, nScale);
        GetScaleSkin(m_pCheckSkin, nScale);
        GetScaleSkin(m_pArrowSkin, nScale);
        GetScaleSkin(m_pSepSkin, nScale);
    }

    SWindow *CreateMenuItem(const SStringW &strItemName);
};

HRESULT SMenuExRoot::OnAttrIconPos(const SStringW &strValue, BOOL bLoading)
{
    SStringWList values;
    SplitString(strValue, L',', values);
    if (1 == values.GetCount())
    {
        //只设置X时，让Y方向自动居中
        m_iconX = GETLAYOUTSIZE(values[0]);
        m_iconY.parseString(Y_MIDFLAG);
        return S_OK;
    }
    else if (2 != values.GetCount())
        return E_INVALIDARG;
    m_iconX = GETLAYOUTSIZE(values[0]);
    m_iconY = GETLAYOUTSIZE(values[1]);
    return S_OK;
}

SMenuExItem *SMenuExRoot::GetNextMenuItem(SMenuExItem *pItem, BOOL bForword, int nCount)
{
    if ((UINT)nCount == GetChildrenCount())
        return NULL;

    SMenuExItem *pRet = NULL;
    if (pItem)
    {
        SASSERT(pItem->GetParent() == this);
        pRet = (SMenuExItem *)pItem->GetWindow(bForword ? GSW_NEXTSIBLING : GSW_PREVSIBLING);
    }
    if (!pRet)
    {
        pRet = (SMenuExItem *)GetWindow(bForword ? GSW_FIRSTCHILD : GSW_LASTCHILD);
    }

    if (!pRet->IsDisabled(TRUE))
        return pRet;
    else
        return GetNextMenuItem(pRet, bForword, nCount + 1);
}

//////////////////////////////////////////////////////////////////////////
// SMenuExItem

SMenuExItem::~SMenuExItem()
{
    if (m_pSubMenu)
    {
        delete m_pSubMenu;
    }
}

SMenuExItem::SMenuExItem(SMenuEx *pOwnerMenu, ISkinObj *pItemSkin)
    : m_pSubMenu(NULL)
    , m_pOwnerMenu(pOwnerMenu)
    , m_iIcon(-1)
    , m_bCheck(FALSE)
    , m_bRadio(FALSE)
    , m_cHotKey(0)
{
    m_bDisplay = FALSE;
    m_pBgSkin = pItemSkin;
    m_style.m_bTrackMouseEvent = TRUE;
    m_style.SetAlign(DT_LEFT);
}

void SMenuExItem::OnPaint(IRenderTarget *pRT)
{
    __baseCls::OnPaint(pRT);

    CRect rc = GetClientRect();
    SMenuExRoot *pMenuRoot = sobj_cast<SMenuExRoot>(GetRoot());
    SASSERT(pMenuRoot);
    rc.right = rc.left + pMenuRoot->m_nIconBarWidth.toPixelSize(GetScale());
    rc.left += pMenuRoot->m_iconX.toPixelSize(GetScale());
    int icoY = pMenuRoot->m_iconY.toPixelSize(GetScale());

    if (m_bCheck || m_bRadio)
    {
        if (pMenuRoot->m_pCheckSkin)
        {
            int nState = 0;
            if (m_bRadio)
            {
                nState = m_bCheck ? 1 : 2;
            }
            if (icoY == Y_IMIDFLAG)
            {
                icoY = (rc.Height() - pMenuRoot->m_pCheckSkin->GetSkinSize().cy) / 2;
            }
            rc.top += icoY;
            CRect rcIcon(rc.TopLeft(), pMenuRoot->m_pCheckSkin->GetSkinSize());
            pMenuRoot->m_pCheckSkin->DrawByIndex(pRT, rcIcon, nState);
        }
    }
    else if (pMenuRoot->m_pIconSkin)
    {
        if (icoY == Y_IMIDFLAG)
        {
            icoY = (rc.Height() - pMenuRoot->m_pIconSkin->GetSkinSize().cy) / 2;
        }
        rc.top += icoY;
        CRect rcIcon(rc.TopLeft(), pMenuRoot->m_pIconSkin->GetSkinSize());
        pMenuRoot->m_pIconSkin->DrawByIndex(pRT, rcIcon, m_iIcon);
    }

    if (m_pSubMenu && pMenuRoot->m_pArrowSkin)
    {
        CRect rcArrow = GetClientRect();
        CSize szArrow = pMenuRoot->m_pArrowSkin->GetSkinSize();
        rcArrow.left = rcArrow.right - szArrow.cx;
        rcArrow.DeflateRect(0, (rcArrow.Height() - szArrow.cy) / 2);

        pMenuRoot->m_pArrowSkin->DrawByIndex(pRT, rcArrow, (GetState() & WndState_Hover) ? 1 : 0);
    }
}

BOOL SMenuExItem::OnEraseBkgnd(IRenderTarget *pRT)
{
    if (!m_pBgSkin)
        return FALSE;
    int nState = 0;

    if (GetState() & WndState_Disable)
    {
        nState = 2;
    }
    else if (GetState() & WndState_Check || GetState() & WndState_PushDown || GetState() & WndState_Hover)
    {
        nState = 1;
    }
    if (nState >= m_pBgSkin->GetStates())
        nState = 0;
    m_pBgSkin->DrawByIndex(pRT, GetClientRect(), nState);

    return TRUE;
}

void SMenuExItem::GetTextRect(LPRECT pRect)
{
    GetClientRect(pRect);
    SMenuExRoot *pMenuRoot = sobj_cast<SMenuExRoot>(GetRoot());
    SASSERT(pMenuRoot);
    int nScale = GetScale();
    pRect->left += pMenuRoot->m_nIconBarWidth.toPixelSize(nScale) + pMenuRoot->m_nTextOffset.toPixelSize(nScale);
    if (m_pSubMenu)
        pRect->right -= pMenuRoot->m_pArrowSkin->GetSkinSize().cx;
}

void SMenuExItem::GetDesiredSize(SIZE *psz, int wid, int hei)
{
    CSize szRet;
    __baseCls::GetDesiredSize(&szRet, wid, hei);
    SMenuExRoot *pMenuRoot = sobj_cast<SMenuExRoot>(GetRoot());
    SASSERT(pMenuRoot);
    if (GetChildrenCount() == 0)
    {
        if (!GetLayoutParam()->IsSpecifiedSize(Horz))
        {
            szRet.cx += pMenuRoot->m_nIconBarWidth.toPixelSize(GetScale()) + pMenuRoot->m_nTextOffset.toPixelSize(GetScale());
            if (m_pSubMenu)
                szRet.cx += pMenuRoot->m_pArrowSkin->GetSkinSize().cx; //加上子菜单箭头宽度
        }
    }
    if (!GetLayoutParam()->IsSpecifiedSize(Vert))
    {
        szRet.cy = smax(szRet.cy, pMenuRoot->m_nItemHei.toPixelSize(GetScale()));
    }
    *psz = szRet;
}

BOOL SMenuExItem::CreateChildren(SXmlNode xmlNode)
{
    __baseCls::CreateChildren(xmlNode);
    SXmlNode xmlChild = xmlNode.child(SMenuExItem::GetClassName());
    if (xmlChild)
    { //有子菜单
        m_pSubMenu = new SMenuEx(this);
        m_pSubMenu->LoadMenu2(&xmlNode);
    }
    return TRUE;
}

SWindow *SMenuExItem::CreateChildByName(LPCWSTR pszName)
{
    if (pszName == NULL)
        return NULL;
    if (_wcsicmp(pszName, SMenuExItem::GetClassName()) == 0)
        return NULL;
    return __baseCls::CreateChildByName(pszName);
}

void SMenuExItem::OnSubMenuHided(BOOL bUncheckItem)
{
    m_pOwnerMenu->OnSubMenuHided(bUncheckItem);
}

WCHAR SMenuExItem::GetHotKey() const
{
    return m_cHotKey;
}

void SMenuExItem::ShowSubMenu(BOOL bCheckFirstItem)
{
    if (!m_pSubMenu)
        return;
    m_pOwnerMenu->PopupSubMenu(this, bCheckFirstItem);
}

void SMenuExItem::HideSubMenu()
{
    if (!m_pSubMenu)
        return;
    m_pSubMenu->HideMenu(FALSE);
}

SMenuEx *SMenuExItem::GetOwnerMenu()
{
    return m_pOwnerMenu;
}

SMenuEx *SMenuExItem::GetSubMenu()
{
    return m_pSubMenu;
}

//////////////////////////////////////////////////////////////////////////

class SMenuExSep : public SMenuExItem {
    DEF_SOBJECT(SMenuExItem, L"sep")
  public:
    SMenuExSep(SMenuEx *pOwnerMenu, ISkinObj *pItemSkin)
        : SMenuExItem(pOwnerMenu, pItemSkin)
    {
        m_dwState |= WndState_Disable;
        m_bDisable = TRUE;
    }

    STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE *psz, int wid, int hei) OVERRIDE
    {
        SMenuExRoot *pMenuRoot = sobj_cast<SMenuExRoot>(GetRoot());
        SASSERT(pMenuRoot);
        CSize szRet;
        szRet.cx = 0;
        if (!GetLayoutParam()->IsSpecifiedSize(Vert))
        {
            if (m_pBgSkin)
            {
                szRet.cy = m_pBgSkin->GetSkinSize().cy;
                if (szRet.cy == 0) //纯色皮肤没有size
                    szRet.cy = 1;
            }
            else
                szRet.cy = 0;
        }
        else
        {
            szRet.cy = GetLayoutParam()->GetSpecifiedSize(Vert).toPixelSize(GetScale());
        }
        szRet.cy += m_style.GetMargin().top + m_style.GetMargin().bottom;
        *psz = szRet;
    }

  protected:
    BOOL OnEraseBkgnd(IRenderTarget *pRT)
    {
        if (!m_pBgSkin)
            return FALSE;
        m_pBgSkin->DrawByIndex(pRT, GetClientRect(), 0);
        return TRUE;
    }
    void OnPaint(IRenderTarget *pRT)
    {
        (void)pRT;
    }
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_ERASEBKGND_EX(OnEraseBkgnd)
    SOUI_MSG_MAP_END()
};

//////////////////////////////////////////////////////////////////////////
SWindow *SMenuExRoot::CreateMenuItem(const SStringW &strItemName)
{
    SWindow *pMenuItem = NULL;
    if (strItemName == SMenuExItem::GetClassName())
    {
        pMenuItem = new SMenuExItem(m_pMenuEx, m_pItemSkin);
    }
    else if (strItemName == SMenuExSep::GetClassName())
    {
        pMenuItem = new SMenuExSep(m_pMenuEx, m_pSepSkin);
    }
    //从style里初始化MenuItem
    if (pMenuItem)
        SApplication::getSingleton().SetSwndDefAttr(pMenuItem);

    return pMenuItem;
}
//////////////////////////////////////////////////////////////////////////
class SMenuExRunData : public TObjRefImpl<IObjRef> {
    friend class SMenuEx;

  public:
    SMenuExRunData(HWND hOwner, int nScale)
        : m_hOwner(hOwner)
        , m_bExit(FALSE)
        , m_nCmdID(-1)
        , m_nScale(nScale)
    {
    }

    BOOL IsMenuWnd(HWND hWnd)
    {
        SPOSITION pos = m_lstMenuEx.GetTailPosition();
        while (pos)
        {
            if (m_lstMenuEx.GetPrev(pos)->m_hWnd == hWnd)
                return TRUE;
        }
        return FALSE;
    }

    void PushMenuEx(SMenuEx *pMenu)
    {
        m_lstMenuEx.AddTail(pMenu);
    }

    SMenuEx *GetMenuEx()
    {
        if (m_lstMenuEx.IsEmpty())
            return 0;
        return m_lstMenuEx.GetTail();
    }

    SMenuEx *PopMenuEx()
    {
        SASSERT(!m_lstMenuEx.IsEmpty());
        SMenuEx *pMenuEx = m_lstMenuEx.RemoveTail();
        return pMenuEx;
    }

    SMenuEx *SMenuExFromHwnd(HWND hWnd)
    {
        SPOSITION pos = m_lstMenuEx.GetTailPosition();
        while (pos)
        {
            SMenuEx *pMenuEx = m_lstMenuEx.GetPrev(pos);
            if (pMenuEx->m_hWnd == hWnd)
                return pMenuEx;
        }
        return NULL;
    }

    BOOL IsMenuExited()
    {
        return m_bExit;
    }

    void ExitMenu(int nCmdID)
    {
        // hide all menu window
        SPOSITION pos = m_lstMenuEx.GetTailPosition();
        while (pos)
        {
            SMenuEx *pMenuEx = m_lstMenuEx.GetPrev(pos);
            pMenuEx->ShowWindow(SW_HIDE);
        }

        m_bExit = TRUE;
        m_nCmdID = nCmdID;
    }

    int GetCmdID()
    {
        return m_nCmdID;
    }

    HWND GetOwner()
    {
        return m_hOwner;
    }

    int GetScale() const
    {
        return m_nScale;
    }

  protected:
    SList<SMenuEx *> m_lstMenuEx;

    BOOL m_bExit;
    int m_nCmdID;
    HWND m_hOwner;
    int m_nScale;
};

static SMenuExRunData *s_MenuData = NULL;

//////////////////////////////////////////////////////////////////////////
SMenuEx::SMenuEx(void)
    : m_pParent(NULL)
    , m_pHoverItem(NULL)
    , m_pCheckItem(NULL)
    , m_bMenuInitialized(FALSE)
{
    m_hostAttr.SetTranslucent(true);
}

SMenuEx::SMenuEx(SMenuExItem *pParent)
    : m_pParent(pParent)
    , m_pHoverItem(NULL)
    , m_pCheckItem(NULL)
    , m_bMenuInitialized(FALSE)
{
}

SMenuEx::~SMenuEx(void)
{
    DestroyMenu();
}

SRootWindow *SMenuEx::CreateRoot()
{
    return new SMenuExRoot(this);
}

BOOL SMenuEx::LoadMenu(LPCTSTR strMenu)
{
    SXmlDoc xmlMenu;
    BOOL bLoad = LOADXML(xmlMenu, strMenu);
    if (!bLoad)
        return FALSE;
    SXmlNode xmlNode = xmlMenu.root().first_child();
    return LoadMenu2(&xmlNode);
}

BOOL SMenuEx::LoadMenuU8(THIS_ LPCSTR resId)
{
    SStringT strResId = S_CA2T(resId, CP_UTF8);
    return LoadMenu(strResId);
}

BOOL SMenuEx::OnLoadLayoutFromResourceID(SXmlDoc &souiXml)
{
    return TRUE;
}

BOOL SMenuEx::LoadMenu2(IXmlNode *xmlMenu)
{
    if (IsWindow())
        return FALSE;
    SXmlNode xmlNode(xmlMenu);
    if (xmlNode.name() != SStringW(SMenuExRoot::GetClassName()) && xmlNode.name() != SStringW(SMenuExItem::GetClassName()))
        return FALSE;
    SXmlDoc souiXml;
    SXmlNode root = souiXml.root().append_child(L"SOUI");
    root.append_attribute(L"translucent").set_value(1);
    if (m_pParent == NULL)
    {
        root.append_attribute(L"trCtx").set_value(xmlNode.attribute(L"trCtx").value());
    }

    HWND hWnd = CreateEx(NULL, WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE, 0, 0, 0, 0, &root);
    if (!hWnd)
        return FALSE;

    GetRoot()->InitFromXml(&xmlNode);
    m_hostAttr.SetSendWheel2Hover(true);
    return TRUE;
}

SMenuExItem *SMenuEx::GetMenuItem(int nID, BOOL byCmdId)
{
    if (byCmdId)
    {
        return FindChildByID2<SMenuExItem>(nID);
    }
    else
    {
        SMenuExRoot *pMenuRoot = sobj_cast<SMenuExRoot>(GetRoot());
        SWindow *pItem = pMenuRoot->GetWindow(GSW_FIRSTCHILD);
        for (int i = 0; i < nID && pItem; i++)
        {
            pItem = pItem->GetWindow(GSW_NEXTSIBLING);
        }
        return sobj_cast<SMenuExItem>(pItem);
    }
}

SMenuEx *SMenuEx::GetSubMenu(int nID, BOOL byCmdId)
{
    SMenuExItem *pItem = GetMenuItem(nID, byCmdId);
    if (!pItem)
        return NULL;
    return pItem->GetSubMenu();
}

IMenuEx *SMenuEx::GetSubMenu(THIS_ int nPos)
{
    return GetSubMenu(nPos, FALSE);
}

UINT SMenuEx::TrackPopupMenu(UINT flag, int x, int y, HWND hOwner, int nScale)
{
    if (!IsWindow())
        return (UINT)-1;
    if (!s_MenuData)
        s_MenuData = new SMenuExRunData(hOwner, nScale);
    else
        s_MenuData->AddRef();

    HWND hActive = hOwner;
    if (!hOwner || !::IsWindowEnabled(hOwner))
        hActive = ::GetActiveWindow();

    HWND hRoot = hActive;
    while ((::GetWindowLong(hRoot, GWL_STYLE) & WS_CHILD) && ::GetParent(hRoot))
    {
        hRoot = ::GetParent(hRoot);
    }
    SetForegroundWindow(hRoot);

    ShowMenu(flag, x, y);
    RunMenu(hRoot);
    HideMenu(FALSE);

    if (hActive)
    {
        CPoint pt;
        GetCursorPos(&pt);
        ::ScreenToClient(hActive, &pt);
        ::PostMessage(hActive, WM_MOUSEMOVE, 0, MAKELPARAM(pt.x, pt.y));
    }

    int nRet = s_MenuData->GetCmdID();
    if (0 == s_MenuData->Release())
    {
        s_MenuData = NULL;
    }
    if (flag & TPM_RETURNCMD)
    {
        return nRet;
    }
    else
    {
        ::SendMessage(hOwner, WM_COMMAND, MAKEWPARAM(nRet, 0), 0);
        return TRUE;
    }
}

void SMenuEx::ShowMenu(UINT uFlag, int x, int y)
{
    SMenuExRoot *pMenuRoot = sobj_cast<SMenuExRoot>(GetRoot());
    SASSERT(pMenuRoot);

    SendInitPopupMenu2Owner(0);
    pMenuRoot->SDispatchMessage(UM_SETSCALE, GetScale(), 0);

    CSize szMenu = pMenuRoot->CalcMenuSize();

    pMenuRoot->Move(CRect(CPoint(), szMenu));
    if (uFlag & TPM_CENTERALIGN)
    {
        x -= szMenu.cx / 2;
    }
    else if (uFlag & TPM_RIGHTALIGN)
    {
        x -= szMenu.cx;
    }

    if (uFlag & TPM_VCENTERALIGN)
    {
        y -= szMenu.cy / 2;
    }
    else if (uFlag & TPM_BOTTOMALIGN)
    {
        y -= szMenu.cy;
    }

    HMONITOR hMor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO mi = { sizeof(MONITORINFO), 0 };
    GetMonitorInfo(hMor, &mi);
    CRect rcMenu(CPoint(x, y), szMenu);
    CRect rcInter;
    rcInter.IntersectRect(&rcMenu, &mi.rcMonitor);
    int subMenuOffset = m_pParent ? pMenuRoot->m_nSubMenuOffset.toPixelSize(GetScale()) : 0;
    if (rcInter != rcMenu)
    {
        if (m_pParent)
        {
            SMenuEx *pParent = m_pParent->GetOwnerMenu();
            CRect rcParent = pParent->GetWindowRect();
            if (rcMenu.right > mi.rcMonitor.right)
            {
                rcMenu.MoveToX(x - szMenu.cx - rcParent.Width() - subMenuOffset);
            }
            else
            {
                rcMenu.MoveToX(x + subMenuOffset);
            }

            int xOffset = 0, yOffset = 0;
            if (rcMenu.left < mi.rcMonitor.left)
                xOffset = mi.rcMonitor.left - rcMenu.left;
            else if (rcMenu.right > mi.rcMonitor.right)
                xOffset = mi.rcMonitor.right - rcMenu.right;
            if (rcMenu.top < mi.rcMonitor.top)
                yOffset = mi.rcMonitor.top - rcMenu.top;
            else if (rcMenu.bottom > mi.rcMonitor.bottom)
                yOffset = mi.rcMonitor.bottom - rcMenu.bottom;

            rcMenu.OffsetRect(xOffset, yOffset);
        }
        else
        {

            if (rcMenu.right > mi.rcMonitor.right)
            {
                rcMenu.MoveToX(x - szMenu.cx - subMenuOffset);
            }
            else
            {
                rcMenu.MoveToX(x + subMenuOffset);
            }

            if (rcMenu.top < mi.rcMonitor.top)
            {
                rcMenu.MoveToY(y + szMenu.cy);
            }
            if (rcMenu.bottom > mi.rcMonitor.bottom)
            {
                rcMenu.MoveToY(y - szMenu.cy);
            }
        }
    }
    else
    {

        rcMenu.MoveToX(x + subMenuOffset);
    }

    SetWindowPos(HWND_TOPMOST, rcMenu.left, rcMenu.top, szMenu.cx, szMenu.cy, SWP_SHOWWINDOW | SWP_NOACTIVATE);
    s_MenuData->PushMenuEx(this);
}

void SMenuEx::HideMenu(BOOL bUncheckParentItem)
{
    if (!SNativeWnd::IsWindowVisible())
        return;
    HideSubMenu();
    ShowWindow(SW_HIDE);
    if (m_pCheckItem)
    {
        m_pCheckItem->SetCheck(FALSE);
        m_pCheckItem = NULL;
    }
    s_MenuData->PopMenuEx();
    if (m_pParent)
    {
        m_pParent->OnSubMenuHided(bUncheckParentItem);
    }
}

void SMenuEx::HideSubMenu()
{
    if (m_pCheckItem)
        m_pCheckItem->HideSubMenu();
}

int SMenuEx::OnMouseActivate(HWND wndTopLevel, UINT nHitTest, UINT message)
{
    return MA_NOACTIVATE;
}

void SMenuEx::RunMenu(HWND hRoot)
{
    SASSERT(s_MenuData);

    BOOL bMsgQuit(FALSE);
    HWND hCurMenu(NULL);

    SAutoRefPtr<IMessageLoop> msgLoop;
    SApplication::getSingletonPtr()->GetMsgLoopFactory()->CreateMsgLoop(&msgLoop, GetMsgLoop());
    for (;;)
    {

        if (s_MenuData->IsMenuExited())
        {
            break;
        }

        if (GetForegroundWindow() != hRoot)
        {
            break;
        }
        MSG msg = { 0 };

        for (;;)
        { //获取菜单相关消息，抄自wine代码
            if (msgLoop->PeekMsg(&msg, 0, 0, FALSE))
            {
                if (!CallMsgFilter(&msg, MSGF_MENU))
                    break;
                msgLoop->PeekMsg(&msg, msg.message, msg.message, TRUE);
            }
            else
            {
                msgLoop->WaitMsg();
            }
        }

        if (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP || msg.message == WM_SYSKEYDOWN || msg.message == WM_SYSKEYUP)
        { //拦截alt键
            if (msg.wParam == VK_MENU)
            { // handle alt key down, exit menu loop
                s_MenuData->ExitMenu(0);
                break;
            }
        }
        else if (msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN || msg.message == WM_NCLBUTTONDOWN || msg.message == WM_NCRBUTTONDOWN || msg.message == WM_LBUTTONDBLCLK)
        {
            // click on other window
            if (!s_MenuData->IsMenuWnd(msg.hwnd))
            {
                s_MenuData->ExitMenu(0);
                break;
            }
            else
            {
                SMenuEx *pMenu = s_MenuData->SMenuExFromHwnd(msg.hwnd);
                pMenu->HideSubMenu();
            }
        }
        else if (msg.message == WM_QUIT)
        {
            bMsgQuit = TRUE;
        }

        //移除消息队列中当前的消息。
        msgLoop->PeekMsg(&msg, msg.message, msg.message, TRUE);

        //拦截非菜单窗口的MouseMove消息
        if (msg.message == WM_MOUSEMOVE)
        {
            if (msg.hwnd != hCurMenu)
            {
                if (hCurMenu)
                {
                    ::SendMessage(hCurMenu, WM_MOUSELEAVE, 0, 0);
                }
                hCurMenu = msg.hwnd;
            }

            SMenuEx *pMenu = s_MenuData->SMenuExFromHwnd(msg.hwnd);
            if (!pMenu)
            {
                hCurMenu = NULL;
            }
        }

        msgLoop->OnMsg(&msg);

        if (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP || msg.message == WM_CHAR)
        { //将键盘事件强制发送到最后一级菜单窗口，让菜单处理快速键
            HWND menuWnd = s_MenuData->GetMenuEx()->m_hWnd;
            ::SendMessage(menuWnd, msg.message, msg.wParam, msg.lParam);
        }

        if (bMsgQuit)
        {
            PostQuitMessage((int)msg.wParam);
            break;
        }
    }
}

class SMenuExEventOwner {
  public:
    SMenuExEventOwner(SMenuEx *pMenuEx)
    {
        s_pMenuEx = pMenuEx;
    }
    ~SMenuExEventOwner()
    {
        s_pMenuEx = NULL;
    }

    static SMenuEx *GetEvtOwner()
    {
        return s_pMenuEx;
    }

  private:
    static SMenuEx *s_pMenuEx;
};

SMenuEx *SMenuExEventOwner::s_pMenuEx = NULL;

SMenuEx *SMenuEx::GetEvtOwner()
{
    return SMenuExEventOwner::GetEvtOwner();
}

BOOL SMenuEx::_HandleEvent(IEvtArgs *pEvt)
{
    if (pEvt->Sender()->IsClass(SMenuExItem::GetClassName()))
    {
        SMenuExItem *pMenuItem = sobj_cast<SMenuExItem>(pEvt->Sender());
        if (pEvt->GetID() == EventSwndMouseHover::EventID)
        {
            if (pMenuItem->GetSubMenu() != NULL)
            {
                SNativeWnd::SetTimer(TIMERID_POPSUBMENU, TIME_PUPSUBMENU);
                m_pHoverItem = pMenuItem;
            }
            HideSubMenu();
            return FALSE;
        }
        else if (pEvt->GetID() == EventSwndMouseLeave::EventID)
        {
            if (pMenuItem->GetSubMenu() != NULL)
            {
                SNativeWnd::KillTimer(TIMERID_POPSUBMENU);
                m_pHoverItem = NULL;
            }
            return FALSE;
        }

        if (pEvt->GetID() != EventCmd::EventID)
            return FALSE;
        SASSERT(pMenuItem);
        if (pMenuItem->GetSubMenu())
        {
            PopupSubMenu(pMenuItem, FALSE);
            return FALSE;
        }
        else if (pMenuItem->GetID() == 0)
        {
            return FALSE;
        }
        s_MenuData->ExitMenu(pMenuItem->GetID());
        return TRUE;
    }
    else if (s_MenuData && ::IsWindow(s_MenuData->GetOwner()))
    {
        SMenuExEventOwner evtOwner(this);
        return (BOOL)::SendMessage(s_MenuData->GetOwner(), UM_MENUEVENT, (WPARAM)this, (LPARAM)pEvt);
    }
    else
    {
        return FALSE;
    }
}

void SMenuEx::OnTimer(UINT_PTR timeID)
{
    if (timeID == TIMERID_POPSUBMENU)
    {
        PopupSubMenu(m_pHoverItem, FALSE);
    }
    else
    {
        SetMsgHandled(FALSE);
    }
}

void SMenuEx::OnSubMenuHided(BOOL bUncheckItem)
{
    SASSERT(m_pCheckItem);
    if (!bUncheckItem)
    {
        m_pCheckItem->SetCheck(FALSE);
        m_pCheckItem = NULL;
    }
}

void SMenuEx::PopupSubMenu(SMenuExItem *pItem, BOOL bCheckFirstItem)
{
    SNativeWnd::KillTimer(TIMERID_POPSUBMENU);

    SMenuEx *pSubMenu = pItem->GetSubMenu();
    SASSERT(pSubMenu);
    if (!pSubMenu->m_bMenuInitialized)
    {
        int idx = 0;
        SWindow *pPrev = pItem->GetWindow(GSW_PREVSIBLING);
        while (pPrev)
        {
            idx++;
            pPrev = pPrev->GetWindow(GSW_PREVSIBLING);
        }
        pSubMenu->SendInitPopupMenu2Owner(idx);
    }

    CRect rcWnd = GetRoot()->GetWindowRect();
    CRect rcItem = pItem->GetWindowRect();
    rcItem.left = rcWnd.left, rcItem.right = rcWnd.right;
    ClientToScreen2(&rcItem);

    m_pCheckItem = pItem;
    m_pCheckItem->SetCheck(TRUE);
    pSubMenu->SetWindowLongPtr(GWLP_HWNDPARENT, (ULONG_PTR)m_hWnd);
    pSubMenu->ShowMenu(0, rcItem.right, rcItem.top);
    if (bCheckFirstItem)
    {
        SMenuExRoot *pMenuRoot = sobj_cast<SMenuExRoot>(pSubMenu->GetRoot());
        SASSERT(pMenuRoot);
        SMenuExItem *pFirstItem = pMenuRoot->GetNextMenuItem(NULL, TRUE);
        if (pFirstItem)
        {
            pSubMenu->m_pCheckItem = pFirstItem;
            pFirstItem->SetCheck(TRUE);
        }
    }
}

void SMenuEx::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    SMenuExRoot *pMenuRoot = sobj_cast<SMenuExRoot>(GetRoot());
    SASSERT(pMenuRoot);
    switch (nChar)
    {
    case VK_UP:
    case VK_DOWN:
        if (m_pCheckItem)
            m_pCheckItem->SetCheck(FALSE);
        m_pCheckItem = pMenuRoot->GetNextMenuItem(m_pCheckItem, nChar == VK_DOWN);
        if (m_pCheckItem)
        {
            m_pCheckItem->SetCheck(TRUE);
            m_pCheckItem->Invalidate();
        }
        break;
    case VK_ESCAPE:
    case VK_LEFT:
        if (m_pParent)
        {
            HideMenu(TRUE);
        }
        else
        {
            s_MenuData->ExitMenu(0);
        }
        break;
    case VK_RIGHT:
        if (m_pCheckItem)
        {
            m_pCheckItem->ShowSubMenu(TRUE);
        }
        break;
    case VK_RETURN:
        if (m_pCheckItem)
            m_pCheckItem->FireCommand();
        break;
        break;
    default:
        if (isprint(nChar))
        {
            nChar = tolower(nChar);
            SMenuExItem *pMenuItem = (SMenuExItem *)pMenuRoot->GetWindow(GSW_FIRSTCHILD);
            while (pMenuItem)
            {
                if ((UINT)tolower(pMenuItem->GetHotKey()) == nChar)
                {
                    pMenuItem->FireCommand();
                    return;
                }
                pMenuItem = (SMenuExItem *)pMenuItem->GetWindow(GSW_NEXTSIBLING);
            }
        }
        else
        {
            SetMsgHandled(FALSE);
        }
        break;
    }
}

LPCWSTR SMenuEx::GetTranslatorContext() const
{
    if (m_pParent != NULL)
    {
        return m_pParent->GetTrCtx();
    }
    else
    {
        return __baseCls::GetTranslatorContext();
    }
}

int SMenuEx::GetScale() const
{
    if (!s_MenuData)
        return 100;
    return s_MenuData->GetScale();
}

void SMenuEx::EndMenu(int nCmdId /*=0*/)
{
    if (!s_MenuData)
        return;
    s_MenuData->ExitMenu(nCmdId);
    ::PostMessage(s_MenuData->GetOwner(), WM_NULL, 0, 0);
}

EXTERN_C void EndMenuEx(int nCmdId)
{
    SMenuEx::EndMenu(nCmdId);
}

SWindow *SMenuEx::FindItem(UINT uPos, UINT uFlag)
{
    SMenuExRoot *pMenuRoot = sobj_cast<SMenuExRoot>(GetRoot());
    SASSERT(pMenuRoot);
    SWindow *pItemRef = NULL;

    if (uFlag & MF_BYPOSITION)
    {
        if (uPos != -1)
        {
            UINT i = 0;
            SWindow *p = pMenuRoot->GetWindow(GSW_FIRSTCHILD);
            while (i < uPos && p)
            {
                i++;
                p = p->GetWindow(GSW_NEXTSIBLING);
            }
            pItemRef = p;
        }
    }
    else // MF_BYCOMMAND
    {
        pItemRef = pMenuRoot->FindChildByID2<SMenuExItem>(uPos);
    }
    return pItemRef;
}

BOOL SMenuEx::InsertMenu(UINT uPos, UINT uFlag, int nId, LPCTSTR lpNewItem, int iIcon)
{
    SMenuExRoot *pMenuRoot = sobj_cast<SMenuExRoot>(GetRoot());
    SASSERT(pMenuRoot);
    SWindow *pItemRef = FindItem(uPos, uFlag);

    if (uFlag & MF_BYCOMMAND && !pItemRef)
        return FALSE;

    SMenuExItem *pMenuItem = (SMenuExItem *)pMenuRoot->CreateMenuItem((uFlag & MF_SEPARATOR) ? SMenuExSep::GetClassName() : SMenuExItem::GetClassName());

    if (pItemRef)
    {
        SWindow *pRefPrev = pItemRef->GetWindow(GSW_PREVSIBLING);
        if (pRefPrev)
        {
            pRefPrev->GetParent()->InsertChild(pMenuItem, pRefPrev);
        }
        else
        {
            pItemRef->GetParent()->InsertChild(pMenuItem, ICWND_FIRST);
        }
    }
    else
    {
        pMenuRoot->InsertChild(pMenuItem);
    }
    if (!(uFlag & MF_SEPARATOR))
    {
        pMenuItem->SetWindowText(lpNewItem);
        if (uFlag & MF_POPUP)
        {
            pMenuItem->m_pSubMenu = new SMenuEx(pMenuItem);
            pMenuItem->m_pSubMenu->IniNullMenu(pMenuRoot);
        }

        pMenuItem->SetAttribute(L"ID", SStringW().Format(L"%d", nId));
        pMenuItem->SetAttribute(L"icon", SStringW().Format(L"%d", iIcon));
        if (uFlag & MF_CHECKED)
        {
            pMenuItem->SetAttribute(L"check", L"1");
        }
    }

    return TRUE;
}

BOOL SMenuEx::AppendMenu(THIS_ UINT uFlags, int uIDNewItem, LPCTSTR lpNewItem, int iIcon)
{
    return InsertMenu(-1, uFlags, uIDNewItem, lpNewItem, iIcon);
}

BOOL SMenuEx::IniNullMenu(SMenuExRoot *ParentRoot)
{
    HWND hWnd = CreateEx(NULL, WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST, 0, 0, 0, 0);
    SXmlDoc souiXml;
    SXmlNode root = souiXml.root().append_child(L"SOUI");
    root.append_attribute(L"translucent").set_value(1);
    InitFromXml(&root);
    if (!hWnd)
        return FALSE;
    SMenuExRoot *pMenuRoot = new SMenuExRoot(this);
    if (ParentRoot)
    {
        //拷贝属性
        ParentRoot->Copy(pMenuRoot);
    }
    GetRoot()->InsertChild(pMenuRoot);
    pMenuRoot->SSendMessage(WM_CREATE);
    pMenuRoot->GetLayoutParam()->SetWrapContent(Both);
    return TRUE;
}

BOOL SMenuEx::DeleteMenu(UINT uPos, UINT uFlag)
{
    SMenuExRoot *pMenuRoot = sobj_cast<SMenuExRoot>(GetRoot());
    SASSERT(pMenuRoot);
    SWindow *pItemRef = FindItem(uPos, uFlag);
    if (!pItemRef)
        return FALSE;
    pMenuRoot->DestroyChild(pItemRef);
    return TRUE;
}

BOOL SMenuEx::CheckMenuItem(UINT uPos, UINT uFlag)
{
    SMenuExRoot *pMenuRoot = sobj_cast<SMenuExRoot>(GetRoot());
    SASSERT(pMenuRoot);
    SWindow *pItemRef = FindItem(uPos, uFlag);
    if (!pItemRef)
        return FALSE;

    if (uFlag & MF_CHECKED)
    {
        pItemRef->SetAttribute(L"check", L"1");
    }
    else
    {
        pItemRef->SetAttribute(L"check", L"0");
    }
    return TRUE;
}

BOOL SMenuEx::CheckMenuRadioItem(UINT idFirst, UINT idLast, UINT idCheck, UINT uFlags)
{
    SMenuExRoot *pMenuRoot = sobj_cast<SMenuExRoot>(GetRoot());
    SASSERT(pMenuRoot);
    SWindow *pItemRefFirst = FindItem(idFirst, uFlags);
    SWindow *pItemRefLast = FindItem(idLast, uFlags);
    SWindow *pItemRefCheck = FindItem(idCheck, uFlags);

    if (!pItemRefFirst || !pItemRefLast || !pItemRefCheck)
        return FALSE;

    int idxFirst = -1;
    int idxLast = -1;
    int idxCheck = -1;

    SWindow *pChild = pMenuRoot->GetWindow(GSW_FIRSTCHILD);
    int i = 0;
    while (pChild)
    {
        if (pChild == pItemRefFirst)
            idxFirst = i;
        else if (pChild == pItemRefCheck)
            idxCheck = i;
        else if (pChild == pItemRefLast)
            idxLast = i;
        pChild = pChild->GetWindow(GSW_NEXTSIBLING);
        i++;
    }
    if (idxFirst == -1 || idxLast == -1 || idxCheck == -1)
        return FALSE;
    if (idxFirst < idxLast)
    {
        SWindow *t = pItemRefFirst;
        pItemRefFirst = pItemRefLast;
        pItemRefLast = t;
        int tIdx = idxFirst;
        idxFirst = idxLast;
        idxLast = tIdx;
    }

    if (idxFirst > idxCheck || idxLast < idxCheck)
        return FALSE;

    pChild = pItemRefFirst;
    for (;;)
    {
        pChild->SetAttribute(L"radio", L"1");
        if (pChild == pItemRefCheck)
        {
            pChild->SetAttribute(L"check", L"1");
        }
        else
        {
            pChild->SetAttribute(L"check", L"0");
        }
        if (pChild == pItemRefLast)
            break;
        else
            pChild = pChild->GetWindow(GSW_NEXTSIBLING);
    }
    return TRUE;
}

void SMenuEx::SendInitPopupMenu2Owner(int idx)
{
    if (m_bMenuInitialized)
        return;

    if (::IsWindow(s_MenuData->GetOwner()))
    {
        ::SendMessage(s_MenuData->GetOwner(), WM_INITMENUPOPUP, (WPARAM)this, (LPARAM)idx);
    }
    m_bMenuInitialized = TRUE;
}

DWORD SMenuEx::GetContextHelpId() const
{
    SMenuExRoot *pMenuRoot = sobj_cast<SMenuExRoot>(GetRoot());
    SASSERT(pMenuRoot);
    return pMenuRoot->m_dwContextHelpId;
}

void SMenuEx::SetContextHelpId(DWORD dwId)
{
    SMenuExRoot *pMenuRoot = sobj_cast<SMenuExRoot>(GetRoot());
    SASSERT(pMenuRoot);
    pMenuRoot->m_dwContextHelpId = dwId;
}

void SMenuEx::DestroyMenu()
{
    if (IsWindow())
        DestroyWindow();
}

BOOL SMenuEx::ModifyMenuString(THIS_ UINT uPosition, UINT uFlags, LPCTSTR lpItemString)
{
    SWindow *pItemRef = FindItem(uPosition, uFlags);
    if (!pItemRef)
        return FALSE;
    pItemRef->SetWindowText(lpItemString);
    return TRUE;
}

BOOL SMenuEx::SetMenuUserData(THIS_ UINT uPosition, UINT uFlags, ULONG_PTR ulUserData)
{
    SWindow *pItemRef = FindItem(uPosition, uFlags);
    if (!pItemRef)
        return FALSE;
    pItemRef->SetUserData(ulUserData);
    return TRUE;
}

ULONG_PTR SMenuEx::GetMenuUserData(THIS_ UINT uPosition, UINT uFlags)
{
    SWindow *pItemRef = FindItem(uPosition, uFlags);
    if (!pItemRef)
        return 0;
    return pItemRef->GetUserData();
}

BOOL SMenuEx::GetMenuString(THIS_ UINT uPosition, UINT uFlags, IStringT *lpItemString)
{
    SWindow *pItemRef = FindItem(uPosition, uFlags);
    if (!pItemRef)
        return FALSE;
    SStringT strText = pItemRef->GetWindowText();
    lpItemString->Copy(&strText);
    return TRUE;
}

SNSEND
