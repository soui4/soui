#include <souistd.h>
#include <control/SSearchDropdownList.h>

SNSBEGIN

//---------------------------------------------------------------------

class SDropdownList : public SDropDownWnd {
  public:
    SDropdownList(ISDropDownOwner *pOwner);

    BOOL Create(IXmlNode *initxml = NULL);

  protected:
    BOOL WINAPI PreTranslateMessage(MSG *pMsg) OVERRIDE
    {
        if (SDropDownWnd::PreTranslateMessage(pMsg))
            return TRUE;

        if (pMsg->message == WM_MOUSEWHEEL || ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP) && (pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN || pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)))
        { // 截获滚轮及上下键消息
            SNativeWnd::SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
            return TRUE;
        }
        return FALSE;
    }
};

SDropdownList::SDropdownList(ISDropDownOwner *pOwner)
    : SDropDownWnd(pOwner)
{
}

BOOL SDropdownList::Create(IXmlNode *initXml)
{
    CRect rc;
    return SDropDownWnd::Create(&rc, initXml, WS_POPUP, WS_EX_TOPMOST | WS_EX_TOOLWINDOW);
}

//---------------------------------------------------------------------
SSearchDropdownList::SSearchDropdownList(void)
    : m_nDropAlign(DROPALIGN_RIGHT)
    , m_nMaxDropHeight(150)
    , m_pDropDownWnd(NULL)
    , m_pListBox(NULL)
{
    GetEventSet()->addEvent(EVENTID(EventDropdownListFill));
    GetEventSet()->addEvent(EVENTID(EventDropdownListSelected));
    GetEventSet()->addEvent(EVENTID(EventDropdownListGetBuddyRect));
}

SSearchDropdownList::~SSearchDropdownList(void)
{
}
void SSearchDropdownList::AdjustDropdownList()
{
    SASSERT(m_pDropDownWnd);
    EventDropdownListGetBuddyRect evt(this);
    FireEvent(&evt);
    CRect rcBuddy = evt.rcBuddy;
    GetContainer()->FrameToHost(rcBuddy);

    ClientToScreen(GetContainer()->GetHostHwnd(), (LPPOINT)&rcBuddy);
    ClientToScreen(GetContainer()->GetHostHwnd(), ((LPPOINT)&rcBuddy) + 1);

    HMONITOR hMonitor = ::MonitorFromWindow(GetContainer()->GetHostHwnd(), MONITOR_DEFAULTTONULL);
    CRect rcMonitor;
    if (hMonitor)
    {
        MONITORINFO mi = { sizeof(MONITORINFO) };
        ::GetMonitorInfo(hMonitor, &mi);
        rcMonitor = mi.rcMonitor;
    }
    else
    {
        rcMonitor.right = GetSystemMetrics(SM_CXSCREEN);
        rcMonitor.bottom = GetSystemMetrics(SM_CYSCREEN);
    }

    CSize szDropdown;
    szDropdown.cx = rcBuddy.Width();
    SLayoutSize szBorder(1, dp);
    CRect rcPadding = m_pDropDownWnd->GetRoot()->GetStyle().GetPadding();
    CRect rcMargin = m_pDropDownWnd->GetRoot()->GetStyle().GetMargin();
    int nItemHeight = rcPadding.top + rcPadding.bottom + rcMargin.top + rcMargin.bottom;
    // 如果没有设置padding和margin，则使用添加默认高度
    if (nItemHeight == 0)
    {
        nItemHeight = szBorder.toPixelSize(GetScale()) * 2;
    }
    szDropdown.cy = m_pListBox->GetItemLocator()->GetTotalHeight() + nItemHeight;

    int nMaxHeight = m_nMaxDropHeight.toPixelSize(GetScale());

    if (szDropdown.cy > nMaxHeight)
        szDropdown.cy = nMaxHeight;

    CPoint pt;
    pt.x = m_nDropAlign == DROPALIGN_LEFT ? rcBuddy.left : (rcBuddy.right - szDropdown.cx);

    if (rcBuddy.bottom + szDropdown.cy <= rcMonitor.bottom)
    {
        pt.y = rcBuddy.bottom;
    }
    else
    {
        pt.y = rcBuddy.top - szDropdown.cy;
    }
    m_pDropDownWnd->SetWindowPos(HWND_TOPMOST, pt.x, pt.y, szDropdown.cx, szDropdown.cy, SWP_SHOWWINDOW | SWP_NOACTIVATE);
    m_pDropDownWnd->SNativeWnd::SetCapture();
}

SWindow *SSearchDropdownList::GetDropDownOwner()
{
    return this;
}

void SSearchDropdownList::OnCreateDropDown(SDropDownWnd *pDropDown)
{
    pDropDown->GetRoot()->InsertChild(m_pListBox);
    pDropDown->GetRoot()->UpdateChildrenPosition();
    m_pListBox->SetVisible(TRUE);
    m_pListBox->SetFocus();
}

void SSearchDropdownList::OnDestroyDropDown(SDropDownWnd *pDropDown)
{
    if (pDropDown->GetExitCode() == IDOK)
    {
        // selected item
        EventDropdownListSelected evt(this);
        evt.pListView = m_pListBox;
        evt.nValue = m_pListBox->GetSel();
        FireEvent(&evt);
    }
    pDropDown->GetRoot()->RemoveChild(m_pListBox);
    m_pListBox->SetVisible(FALSE);
    m_pListBox->SetContainer(GetContainer());

    m_pDropDownWnd = NULL;
}

void SSearchDropdownList::CloseUp(int code)
{
    if (m_pDropDownWnd)
    {
        m_pDropDownWnd->EndDropDown(code);
    }
}

BOOL SSearchDropdownList::IsDropdown() const
{
    return m_pDropDownWnd != NULL;
}

void SSearchDropdownList::DropDown(const IStringT *pText)
{
    if (!m_pDropDownWnd)
    {
        m_pDropDownWnd = new SDropdownList(this);
        GetContainer()->EnableHostPrivateUiDef(TRUE);
        SXmlNode xmlDropdownStyleNode = m_xmlDropdownStyle.root().child(SSearchDropdownList_style::kStyle_Dropdown);
        if (xmlDropdownStyleNode)
            m_pDropDownWnd->Create(&xmlDropdownStyleNode);
        else
            m_pDropDownWnd->Create();
        GetContainer()->EnableHostPrivateUiDef(FALSE);
        m_pDropDownWnd->GetRoot()->SDispatchMessage(UM_SETSCALE, GetScale(), 0);
    }

    EventDropdownListFill evt(this);
    evt.strKey = pText;
    evt.pListView = m_pListBox;
    FireEvent(&evt);

    if (evt.bPopup)
    {
        AdjustDropdownList();
    }
    else
    {
        CloseUp();
    }
}

BOOL SSearchDropdownList::CreateChildren(SXmlNode xmlNode)
{
    BOOL bRet = __baseCls::CreateChildren(xmlNode);
    if (!bRet)
        return FALSE;
    m_xmlDropdownStyle.root().append_copy(xmlNode.child(SSearchDropdownList_style::kStyle_Dropdown));
    return CreateListBox(xmlNode);
}

BOOL SSearchDropdownList::FireEvent(IEvtArgs *evt)
{
    if (evt->IdFrom() == IDC_DROPDOWN_LIST)
    {
        if (evt->GetID() == EVT_CMD)
        {
            CloseUp(IDOK);
            return TRUE;
        }
    }
    return __baseCls::FireEvent(evt);
}

BOOL SSearchDropdownList::CreateListBox(SXmlNode xmlNode)
{
    SASSERT(xmlNode);
    //创建列表控件
    SXmlNode listStyle = xmlNode.child(SSearchDropdownList_style::kStyle_DropdownList);
    SStringW strListClass = listStyle.attribute(L"wndclass").as_string(SListView::GetClassName());
    SListView *pListBox = sobj_cast<SListView>(CreateChildByName(strListClass));
    if (!pListBox)
        return FALSE;
    m_pListBox.Attach(pListBox);

    m_pListBox->SetContainer(GetContainer());
    m_pListBox->InitFromXml(&listStyle);
    m_pListBox->SetAttribute(L"pos", L"0,0,-0,-0", TRUE);
    m_pListBox->SetOwner(this); // chain notify message to combobox
    m_pListBox->SetVisible(FALSE);
    m_pListBox->SetID(IDC_DROPDOWN_LIST);

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////

SNSEND