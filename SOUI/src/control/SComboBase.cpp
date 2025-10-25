﻿#include "souistd.h"
#include "control/SComboBase.h"

SNSBEGIN

static const wchar_t *KStyle_Dropdown = L"dropdownStyle"; //下拉列表风格，只包含root节点
static const wchar_t *KStyle_Edit = L"editStyle";         //编辑框风格

//////////////////////////////////////////////////////////////////////////
// CComboEdit
SComboEdit::SComboEdit()
{
}

SComboEdit::~SComboEdit()
{
}

void SComboEdit::OnMouseHover(WPARAM wParam, CPoint ptPos)
{
    SEdit::OnMouseHover((UINT)wParam, ptPos);
    GetOwner()->SSendMessage(WM_MOUSEHOVER, wParam, MAKELPARAM(ptPos.x, ptPos.y));
}

void SComboEdit::OnMouseLeave()
{
    __baseCls::OnMouseLeave();
    GetOwner()->SSendMessage(WM_MOUSELEAVE);
}

void SComboEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    SWindow *pOwner = GetOwner();
    if (pOwner && (nChar == VK_DOWN || nChar == VK_UP || nChar == VK_ESCAPE))
    {
        pOwner->SSendMessage(WM_KEYDOWN, nChar, MAKELONG(nFlags, nRepCnt));
        return;
    }

    SetMsgHandled(FALSE);
}

BOOL SComboEdit::FireEvent(IEvtArgs *evt)
{
    if (evt->GetID() == EVT_RE_NOTIFY)
    { //转发richedit的txNotify消息
        evt->SetIdFrom(GetOwner()->GetID());
        evt->SetNameFrom(GetOwner()->GetName());
    }
    return SEdit::FireEvent(evt);
}

void SComboEdit::OnKillFocus(SWND wndFocus)
{
    __baseCls::OnKillFocus(wndFocus);
    GetOwner()->SSendMessage(WM_KILLFOCUS, wndFocus);
}

//////////////////////////////////////////////////////////////////////////
// SDropDownWnd_ComboBox
BOOL SDropDownWnd_ComboBox::PreTranslateMessage(MSG *pMsg)
{
    if (SDropDownWnd::PreTranslateMessage(pMsg))
        return TRUE;
    if (pMsg->message == WM_MOUSEWHEEL || ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP) && (pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN || pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)))
    { //截获滚轮及上下键消息
        SNativeWnd::SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
        return TRUE;
    }
    return FALSE;
}

//////////////////////////////////////////////////////////////////////////
// SComboBoxBase
SComboBase::SComboBase(void)
    : m_pSkinBtn(GETBUILTINSKIN(SKIN_SYS_DROPBTN))
    , m_pEdit(NULL)
    , m_bDropdown(TRUE)
    , m_nDropHeight(200, SLayoutSize::dp)
    , m_dwBtnState(WndState_Normal)
    , m_nAnimTime(200)
    , m_pDropDownWnd(NULL)
    , m_iInitSel(-1)
    , m_bAutoFitDropBtn(TRUE)
    , m_crCue(RGBA(0xcc, 0xcc, 0xcc, 0xff))
    , m_strCue(this)
    , m_LastPressTime(0)
    , m_bAutoMatch(FALSE)
    , m_bAutoDropdown(FALSE)
    , m_nTextLength(0)
{
    m_bFocusable = TRUE;
    m_style.SetAlign(SwndStyle::Align_Left);
    m_style.SetVAlign(SwndStyle::VAlign_Middle);

    m_evtSet.addEvent(EVENTID(EventCBSelChange));
    m_evtSet.addEvent(EVENTID(EventRENotify));
    m_evtSet.addEvent(EVENTID(EventCBDropdown));
    m_evtSet.addEvent(EVENTID(EventCBBeforeCloseUp));
}

SComboBase::~SComboBase(void)
{
}

BOOL SComboBase::CreateChildren(SXmlNode xmlNode)
{
    m_xmlDropdownStyle.root().append_copy(xmlNode.child(KStyle_Dropdown));
    //创建edit对象
    SXmlNode xmlEditStyle = xmlNode.child(KStyle_Edit);
    SStringW strEditClass = xmlEditStyle.attribute(L"wndclass").as_string(SComboEdit::GetClassName());
    m_pEdit = sobj_cast<SComboEdit>(CreateChildByName(strEditClass));
    if (!m_pEdit)
        return FALSE;
    m_pEdit->SetOwner(this);
    InsertChild(m_pEdit);
    m_pEdit->GetEventSet()->setMutedState(TRUE);
    if (xmlEditStyle)
        m_pEdit->InitFromXml(&xmlEditStyle);
    else
        m_pEdit->SSendMessage(WM_CREATE);
    m_pEdit->GetEventSet()->setMutedState(FALSE);

    m_pEdit->SetID(IDC_CB_EDIT);
    m_pEdit->SSendMessage(EM_SETEVENTMASK, 0, ENM_CHANGE);
    m_pEdit->SetVisible(!m_bDropdown);
    GetEventSet()->setMutedState(TRUE);
    BOOL ret = CreateListBox(xmlNode);
    GetEventSet()->setMutedState(FALSE);
    return ret;
}

void SComboBase::GetDropBtnRect(LPRECT prc)
{
    SIZE szBtn = m_pSkinBtn->GetSkinSize();
    CRect rcClient = GetClientRect();
    CRect rcPadding = GetStyle().GetPadding();
    rcClient.DeflateRect(rcPadding);
    *prc = rcClient;

    int nHei = prc->bottom - prc->top;
    prc->left = prc->right - nHei * szBtn.cx / szBtn.cy;
    if (m_bAutoFitDropBtn && szBtn.cy < nHei)
    {
        prc->top += (prc->bottom - prc->top - szBtn.cy) / 2;
        prc->left = prc->right - szBtn.cx;
        prc->right = prc->left + szBtn.cx;
        prc->bottom = prc->top + szBtn.cy;
    }
}

void SComboBase::GetTextRect(LPRECT pRect)
{
    CRect rc = GetClientRect();
    rc.DeflateRect(GetStyle().GetPadding());
    CRect rcBtn;
    GetDropBtnRect(&rcBtn);
    rc.right -= rcBtn.Width();
    *pRect = rc;
}

void SComboBase::OnPaint(IRenderTarget *pRT)
{
    SPainter painter;

    BeforePaint(pRT, painter);
    if (m_bDropdown)
    {
        CRect rcText;
        GetTextRect(rcText);
        if (GetCurSel() != -1)
        {
            SStringT strText = GetWindowText();
            DrawText(pRT, strText, strText.GetLength(), rcText, GetTextAlign());
        }
        else
        {
            SStringT strCue = GetCueText();
            if (!strCue.IsEmpty())
            {
                COLORREF crOld = pRT->SetTextColor(m_crCue);
                DrawText(pRT, strCue, strCue.GetLength(), rcText, GetTextAlign());
                pRT->SetTextColor(crOld);
            }
        }
    }
    // draw focus rect
    if (IsFocused())
    {
        DrawFocus(pRT);
    }
    AfterPaint(pRT, painter);
    CRect rcBtn;
    GetDropBtnRect(&rcBtn);
    m_pSkinBtn->DrawByState(pRT, rcBtn, m_dwBtnState);
}

void SComboBase::OnLButtonDown(UINT nFlags, CPoint pt)
{
    if (m_bFocusable)
        SetFocus();
    DropDown();
}

void SComboBase::OnMouseMove(UINT nFlags, CPoint pt)
{
    if (m_dwBtnState == WndState_PushDown)
        return;

    __baseCls::OnMouseHover(nFlags, pt);
    CRect rcBtn;
    GetDropBtnRect(&rcBtn);
    if (rcBtn.PtInRect(pt))
    {
        m_dwBtnState = WndState_Hover;
        InvalidateRect(rcBtn);
    }
    else if (m_dwBtnState == WndState_Hover)
    {
        m_dwBtnState = WndState_Normal;
        InvalidateRect(rcBtn);
    }
}

void SComboBase::OnMouseLeave()
{
    if (m_dwBtnState == WndState_PushDown)
        return;

    if (GetState() & WndState_Hover)
        __baseCls::OnMouseLeave();
    if (m_dwBtnState == WndState_Hover)
    {
        m_dwBtnState = WndState_Normal;
        CRect rcBtn;
        GetDropBtnRect(&rcBtn);
        InvalidateRect(rcBtn);
    }
}

void SComboBase::OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags)
{
    //方向键改变当前选项
    switch (nChar)
    {
    case VK_DOWN:
    case VK_RIGHT:
    {
        int iSel = GetCurSel();
        iSel += 1;
        if (iSel < GetCount())
            SetCurSel(iSel);
    }
    break;
    case VK_UP:
    case VK_LEFT:
    {
        int iSel = GetCurSel();
        iSel -= 1;
        if (iSel < GetCount() && iSel >= 0)
            SetCurSel(iSel);
    }
    break;
    default:
    {
        if (isprint(nChar))
        {
            if ((GetTickCount() - m_LastPressTime) > 300)
            {
                m_strMatch.Empty();
            }
            m_LastPressTime = GetTickCount();
            m_strMatch += nChar;
            int iCurSel = GetCurSel();
            int iStart = iCurSel + 1;
            bool matched = false;
            while (iStart < GetCount())
            {
                SStringT itemText = GetLBText(iStart);
                if (itemText.StartsWith(m_strMatch, true))
                {
                    SetCurSel(iStart);
                    matched = true;
                    break;
                }
                iStart++;
            }
            if (!matched)
            {
                iStart = 0;
                while (iStart <= iCurSel)
                {
                    SStringT itemText = GetLBText(iStart);
                    if (itemText.StartsWith(m_strMatch, true))
                    {
                        SetCurSel(iStart);
                        matched = true;
                        break;
                    }
                    iStart++;
                }
            }
        }
    }
    }
}

BOOL SComboBase::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    //鼠标滚轮改变当前选项
    if (zDelta > 0) // 上滚
    {
        int iSel = GetCurSel();
        iSel -= 1;
        if (iSel < GetCount() && iSel >= 0)
            SetCurSel(iSel);
    }
    else // 下滚
    {
        int iSel = GetCurSel();
        iSel += 1;
        if (iSel < GetCount())
            SetCurSel(iSel);
    }
    return TRUE;
}

void SComboBase::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (!m_bDropdown)
    {
        SComboEdit *pEdit = static_cast<SComboEdit *>(FindChildByID(IDC_CB_EDIT));
        if (pEdit)
            pEdit->SSendMessage(WM_CHAR, nChar, MAKELONG(nFlags, nRepCnt));
        return;
    }
}

UINT SComboBase::OnGetDlgCode() const
{
    return SC_WANTARROWS;
}

BOOL SComboBase::IsFocusable() const
{
    if (m_bDropdown && m_bFocusable)
        return TRUE;
    return FALSE;
}

SWindow *SComboBase::GetDropDownOwner()
{
    return this;
}

static const wchar_t *KAttrTrCtx = L"trCtx";
void SComboBase::OnCreateDropDown(SDropDownWnd *pDropDown)
{
    GetContainer()->EnableHostPrivateUiDef(TRUE);
    SXmlNode xmlDropdownStyleNode = m_xmlDropdownStyle.root().child(KStyle_Dropdown);
    if (xmlDropdownStyleNode)
    {
        if (!xmlDropdownStyleNode.attribute(KAttrTrCtx))
        {
            xmlDropdownStyleNode.append_attribute(KAttrTrCtx).set_value(GetTrCtx());
        }
        pDropDown->InitFromXml(&xmlDropdownStyleNode);
    }
    else
    {
        pDropDown->GetHostAttr().SetTrCtx(GetTrCtx());
    }
    GetContainer()->EnableHostPrivateUiDef(FALSE);
    m_dwBtnState = WndState_PushDown;
    CRect rcBtn;
    GetDropBtnRect(&rcBtn);
    InvalidateRect(rcBtn);
}

void SComboBase::OnDestroyDropDown(SDropDownWnd *pDropDown)
{
    if (!m_bDropdown)
    {
        m_pEdit->SetFocus();
    }

    m_dwBtnState = WndState_Normal;
    m_pDropDownWnd = NULL;
    CRect rcBtn;
    GetDropBtnRect(&rcBtn);
    InvalidateRect(rcBtn);
    ModifyState(0, WndState_Hover, TRUE);
    CPoint pt;
    GetCursorPos(&pt);
    ScreenToClient(GetContainer()->GetHostHwnd(), &pt);
    ::PostMessage(GetContainer()->GetHostHwnd(), WM_MOUSEMOVE, 0, MAKELPARAM(pt.x, pt.y));

    if (pDropDown->GetExitCode() == IDOK)
    {
        OnSelChanged();
    }
}

BOOL SComboBase::CalcPopupRect(int nHeight, CRect &rcPopup)
{
    CRect rcWnd = GetWindowRect();
    GetContainer()->FrameToHost(&rcWnd);

    ClientToScreen(GetContainer()->GetHostHwnd(), (LPPOINT)&rcWnd);
    ClientToScreen(GetContainer()->GetHostHwnd(), ((LPPOINT)&rcWnd) + 1);

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
    if (rcWnd.bottom + nHeight <= rcMonitor.bottom)
    {
        rcPopup = CRect(rcWnd.left, rcWnd.bottom, rcWnd.right, rcWnd.bottom + nHeight);
        return TRUE;
    }
    else
    {
        rcPopup = CRect(rcWnd.left, rcWnd.top - nHeight, rcWnd.right, rcWnd.top);
        return FALSE;
    }
}

void SComboBase::DropDown()
{
    if (m_dwBtnState == WndState_PushDown)
        return;

    if (!m_pDropDownWnd)
    {
        m_pDropDownWnd = new SDropDownWnd_ComboBox(this);
    }

    EventCBDropdown evt(this);
    evt.pDropDown = m_pDropDownWnd;
    SStringT strInput = GetWindowText(TRUE);
    evt.strInput = NULL;
    FireEvent(&evt);
    m_pDropDownWnd->Create(CRect(0, 0, 100, 100), 0);
    m_pDropDownWnd->GetRoot()->SDispatchMessage(UM_SETSCALE, GetScale(), 0);

    CRect rcPadding = m_pDropDownWnd->GetRoot()->GetStyle().GetPadding();
    CRect rcMargin = m_pDropDownWnd->GetRoot()->GetStyle().GetMargin();
    int nDropHeight = GetListBoxHeight() + rcPadding.top + rcPadding.bottom + rcMargin.top + rcMargin.bottom;

    CRect rcPopup;
    BOOL bDown = CalcPopupRect(nDropHeight, rcPopup);
    m_pDropDownWnd->MoveWindow(rcPopup.left, rcPopup.top, rcPopup.Width(), rcPopup.Height());
    m_pDropDownWnd->GetRoot()->UpdateChildrenPosition();

#ifdef _WIN32
    if (m_nAnimTime > 0)
        m_pDropDownWnd->AnimateHostWindow(m_nAnimTime, AW_SLIDE | (bDown ? AW_VER_POSITIVE : AW_VER_NEGATIVE));
    else
        m_pDropDownWnd->SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
#else
    m_pDropDownWnd->SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
#endif
    m_pDropDownWnd->SNativeWnd::SetCapture();
}

void SComboBase::UpdateDropdown(const SStringT &strInput)
{
    if (!m_pDropDownWnd)
    {
        m_pDropDownWnd = new SDropDownWnd_ComboBox(this);
        m_pDropDownWnd->GetRoot()->SDispatchMessage(UM_SETSCALE, GetScale(), 0);
    }

    EventCBDropdown evt(this);
    evt.pDropDown = m_pDropDownWnd;
    evt.strInput = &strInput;
    FireEvent(&evt);
    BOOL bNewPopup = !m_pDropDownWnd->IsWindow();
    if (bNewPopup)
        m_pDropDownWnd->Create(CRect(0, 0, 100, 100), 0);

    CRect rcPadding = m_pDropDownWnd->GetRoot()->GetStyle().GetPadding();
    CRect rcMargin = m_pDropDownWnd->GetRoot()->GetStyle().GetMargin();
    int nDropHeight = GetListBoxHeight() + rcPadding.top + rcPadding.bottom + rcMargin.top + rcMargin.bottom;

    CRect rcPopup;
    BOOL bDown = CalcPopupRect(nDropHeight, rcPopup);
    m_pDropDownWnd->MoveWindow(rcPopup.left, rcPopup.top, rcPopup.Width(), rcPopup.Height());
    m_pDropDownWnd->GetRoot()->UpdateChildrenPosition();

#ifdef _WIN32
    if (m_nAnimTime > 0 && bNewPopup)
        m_pDropDownWnd->AnimateHostWindow(m_nAnimTime, AW_SLIDE | (bDown ? AW_VER_POSITIVE : AW_VER_NEGATIVE));
    else
        m_pDropDownWnd->SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
#else
    m_pDropDownWnd->SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
#endif
    m_pDropDownWnd->SNativeWnd::SetCapture();
}

void SComboBase::CloseUp()
{
    EventCBBeforeCloseUp evt(this);
    evt.bCloseBlock = FALSE;
    FireEvent(&evt);

    if (!evt.bCloseBlock && m_pDropDownWnd)
    {
        m_pDropDownWnd->EndDropDown(IDCANCEL);
    }
}

void SComboBase::OnDestroy()
{
    CloseUp();
    __baseCls::OnDestroy();
}

int SComboBase::OnCreate(LPVOID)
{
    int ret = __baseCls::OnCreate(NULL);
    if (ret != 0)
        return ret;
    if (!m_pSkinBtn)
    {
        SSLOGW()<<"SComboBase::OnCreate failed! skin of btn is null!";
        return -1;
    }
    return 0;
}

void SComboBase::OnSelChanged()
{
    EventCBSelChange evt(this);
    evt.nCurSel = GetCurSel();
    FireEvent(&evt);
}

BOOL SComboBase::FireEvent(IEvtArgs *evt)
{
    if (evt->GetID() == EventRENotify::EventID)
    {
        EventRENotify *evtRe = (EventRENotify *)evt;
        if (evtRe->iNotify == EN_CHANGE && !m_pEdit->GetEventSet()->isMuted())
        {
            m_pEdit->GetEventSet()->setMutedState(true);
            SStringT strTxt = m_pEdit->GetWindowText();
            if (m_bAutoMatch)
            {
                if (strTxt.GetLength() > m_nTextLength)
                {
                    int iItem = FindString(strTxt, -1, TRUE);
                    if (iItem != -1)
                    {
                        SStringT strItem = GetLBText(iItem);
                        m_pEdit->SetWindowText(strItem);
                        m_pEdit->SetSel(strTxt.GetLength(), strItem.GetLength(), TRUE);
                    }
                }
                m_nTextLength = strTxt.GetLength();
            }
            if (m_bAutoDropdown)
            {
                UpdateDropdown(strTxt);
            }

            GetEventSet()->setMutedState(true);
            SetCurSel(-1);
            GetEventSet()->setMutedState(false);
            m_pEdit->GetEventSet()->setMutedState(false);
        }
    }
    return SWindow::FireEvent(evt);
}

int SComboBase::FindString(LPCTSTR pszFind, int iFindAfter /*=-1*/, BOOL bPartMatch /*=TRUE*/)
{
    if (iFindAfter < 0)
        iFindAfter = -1;
    int iStart = iFindAfter + 1;
    for (int i = 0; i < GetCount(); i++)
    {
        int iTarget = (i + iStart) % GetCount();
        SStringT strItem = GetLBText(iTarget, TRUE);
        if (bPartMatch)
        {
            if (strItem.StartsWith(pszFind))
                return iTarget;
        }
        else
        {
            if (strItem.Compare(pszFind) == 0)
                return iTarget;
        }
    }
    return -1;
}

void SComboBase::GetDesiredSize(SIZE *psz, int nParentWid, int nParentHei)
{
    CSize szRet(-1, -1);
    if (GetLayoutParam()->IsSpecifiedSize(Horz))
    { //检查设置大小
        szRet.cx = GetLayoutParam()->GetSpecifiedSize(Horz).toPixelSize(GetScale());
    }
    else if (GetLayoutParam()->IsMatchParent(Horz))
    {
        szRet.cx = nParentWid;
    }

    if (GetLayoutParam()->IsSpecifiedSize(Vert))
    { //检查设置大小
        szRet.cy = GetLayoutParam()->GetSpecifiedSize(Vert).toPixelSize(GetScale());
    }
    else if (GetLayoutParam()->IsMatchParent(Vert))
    {
        szRet.cy = nParentHei;
    }

    if (szRet.cx != -1 && szRet.cy != -1)
    {
        *psz = szRet;
        return;
    }
    int nTestDrawMode = GetTextAlign() & ~(DT_CENTER | DT_RIGHT | DT_VCENTER | DT_BOTTOM);

    CRect rcPadding = GetStyle().GetPadding();
    //计算文本大小
    CRect rcTest(0, 0, 100000, 100000);

    SAutoRefPtr<IRenderTarget> pRT;
    GETRENDERFACTORY->CreateRenderTarget(&pRT, 0, 0);
    BeforePaintEx(pRT);

    SStringT strText = GetWindowText(FALSE);
    SStringT strForText = strText.IsEmpty() ? SStringT(_T("A")) : strText;
    DrawText(pRT, strForText, strForText.GetLength(), rcTest, nTestDrawMode | DT_CALCRECT);
    if (strText.IsEmpty())
        rcTest.right = rcTest.left;

    SIZE szBtn = m_pSkinBtn->GetSkinSize();
    rcTest.right += szBtn.cx;
    rcTest.bottom = smax(rcTest.Height(), szBtn.cy);

    rcTest.InflateRect(m_style.GetMargin());
    rcTest.InflateRect(rcPadding);

    if (GetLayoutParam()->IsWrapContent(Horz))
        szRet.cx = rcTest.Width();
    if (GetLayoutParam()->IsWrapContent(Vert))
        szRet.cy = rcTest.Height();

    *psz = szRet;
}

SStringT SComboBase::GetWindowText(BOOL bRawText /*=TRUE*/) const
{
    if (!m_bDropdown)
    {
        return GetEditText();
    }
    if (GetCurSel() == -1)
        return _T("");
    return GetLBText(GetCurSel(), bRawText);
}

void SComboBase::SetWindowText(LPCTSTR pszText)
{
    SWindow::SetWindowText(pszText);
    m_pEdit->SetWindowText(pszText);
    m_nTextLength = (int)_tcslen(pszText);
}

void SComboBase::OnKillFocus(SWND wndFocus)
{
    __baseCls::OnKillFocus(wndFocus);
    CloseUp();
}

HRESULT SComboBase::OnAttrDropDown(const SStringW &strValue, BOOL bLoading)
{
    m_bDropdown = STRINGASBOOL(strValue);
    if (bLoading)
        return S_OK;
    m_pEdit->SetVisible(!m_bDropdown, TRUE);
    return S_OK;
}

void SComboBase::UpdateChildrenPosition()
{
    __baseCls::UpdateChildrenPosition();
    SIZE szBtn = m_pSkinBtn->GetSkinSize();
    CRect rcPadding = GetStyle().GetPadding();
    CRect rcEdit = GetClientRect();
    CRect rcBtn;
    GetDropBtnRect(&rcBtn);
    rcEdit.right = rcBtn.left;
    m_pEdit->Move(rcEdit);
}

void SComboBase::OnColorize(COLORREF cr)
{
    __baseCls::OnColorize(cr);
    if (m_pSkinBtn)
        m_pSkinBtn->OnColorize(cr);
    if (m_pDropDownWnd)
    {
        m_pDropDownWnd->GetRoot()->SDispatchMessage(UM_SETCOLORIZE, cr, 0);
    }
}

HRESULT SComboBase::OnLanguageChanged()
{
    HRESULT hr = __baseCls::OnLanguageChanged();
    if (m_pDropDownWnd)
    {
        m_pDropDownWnd->GetRoot()->SDispatchMessage(UM_SETLANGUAGE, 0, 0);
    }
    return hr;
}

void SComboBase::OnScaleChanged(int nScale)
{
    __baseCls::OnScaleChanged(nScale);
    if (m_pDropDownWnd)
    {
        m_pDropDownWnd->GetRoot()->SDispatchMessage(UM_SETSCALE, nScale, 0);
    }
    GetScaleSkin(m_pSkinBtn, nScale);
}

BOOL SComboBase::IsDropdown() const
{
    return m_bDropdown;
}

void SComboBase::SetDropdown(BOOL bDropdown)
{
    m_bDropdown = bDropdown;
    m_pEdit->SetVisible(!m_bDropdown, TRUE);
}

SStringT SComboBase::GetCueText(BOOL bRawText) const
{
    return m_strCue.GetText(bRawText);
}

void SComboBase::SetFocus()
{
    if (!m_bDropdown)
        m_pEdit->SetFocus();
    else
        __baseCls::SetFocus();
}

SStringT SComboBase::GetLBText(int iItem, BOOL bRawText /*= FALSE*/) const
{
    SStringT str;
    GetItemText(iItem, bRawText, &str);
    return str;
}

SNSEND