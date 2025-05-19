﻿#include "souistd.h"
#include "core/SWnd.h"
#include "core/SNcPainter.h"
#include "helper/SColor.h"
#include "helper/SplitString.h"
#include "layout/SouiLayout.h"
#include "interface/sacchelper-i.h"
#include "helper/SwndFinder.h"
#include "helper/STime.h"
#include "animation/STransformation.h"
#include "core/SCaret.h"
#include <atl.mini/SComCli.h>

SNSBEGIN

//////////////////////////////////////////////////////////////////////////
// STextTr
//////////////////////////////////////////////////////////////////////////
STrText::STrText(SWindow *pOwner_ /*= NULL*/)
    : pOwner(pOwner_)
    , bAutoEscape(true)
{
}

void STrText::SetOwner(SWindow *pOwner_)
{
    pOwner = pOwner_;
}

SStringT STrText::GetText(BOOL bRawText) const
{
    return bRawText ? strRaw : strTr;
}

void STrText::SetText(const SStringT &strText, bool bEscape /*=true*/)
{
    strRaw = strText;
    bAutoEscape = bEscape;
    TranslateText();
}

void STrText::TranslateText()
{
    if (pOwner == NULL)
        return;
    SStringW str = pOwner->tr(S_CT2W(strRaw));
    if (bAutoEscape)
        str = EscapeString(str);
    strTr = S_CW2T(str);
}

SStringW STrText::EscapeString(const SStringW &strValue)
{
    if (strValue.IsEmpty())
        return strValue;
    SStringW strText = strValue;
    SStringW strCvt;
    LPCWSTR pszBuf = strText;
    int i = 0;
    int iBegin = i;
    while (i < strText.GetLength())
    {
        if (pszBuf[i] == L'\\' && i + 1 < strText.GetLength())
        {
            if (pszBuf[i + 1] == L'n')
            {
                strCvt += strText.Mid(iBegin, i - iBegin);
                strCvt += L"\n";
                i += 2;
                iBegin = i;
            }
            else if (pszBuf[i + 1] == L't')
            {
                strCvt += strText.Mid(iBegin, i - iBegin);
                strCvt += L"\t";
                i += 2;
                iBegin = i;
            }
            else if (pszBuf[i + 1] == L'\\')
            {
                strCvt += strText.Mid(iBegin, i - iBegin);
                strCvt += L"\\";
                i += 2;
                iBegin = i;
            }
            else
            {
                i += 1;
            }
        }
        else
        {
            i += 1;
        }
    }
    strCvt += strText.Mid(iBegin);
    return strCvt;
}

//////////////////////////////////////////////////////////////////////////
// SWindow Implement
//////////////////////////////////////////////////////////////////////////

SWindow::SWindow()
    : m_swnd(SWindowMgr::NewWindow(this))
    , m_pContainer(NULL)
    , m_pParent(NULL)
    , m_pFirstChild(NULL)
    , m_pLastChild(NULL)
    , m_pNextSibling(NULL)
    , m_pPrevSibling(NULL)
    , m_nChildrenCount(0)
    , m_uZorder(0)
    , m_dwState(WndState_Normal)
    , m_bMsgTransparent(FALSE)
    , m_bVisible(TRUE)
    , m_bDisplay(TRUE)
    , m_bDisable(FALSE)
    , m_nUpdateLockCnt(0)
    , m_bClipClient(FALSE)
    , m_bFocusable(FALSE)
    , m_bDrawFocusRect(TRUE)
    , m_bCacheDraw(FALSE)
    , m_bCacheDirty(TRUE)
    , m_layoutDirty(dirty_self)
    , m_bHoverAware(TRUE)
    , m_bLayeredWindow(FALSE)
    , m_bMsgHandled(FALSE)
    , m_uData(0)
    , m_pOwner(NULL)
    , m_pCurMsg(NULL)
    , m_pBgSkin(NULL)
    , m_pNcSkin(NULL)
    , m_pGetRTData(NULL)
    , m_bFloat(FALSE)
    , m_crColorize(0)
    , m_strText(this)
    , m_strToolTipText(this)
    , m_animationHandler(this)
    , m_isAnimating(false)
    , m_isDestroying(false)
    , m_isLoading(false)
    , m_funSwndProc(NULL)
#ifdef _DEBUG
    , m_nMainThreadId(::GetCurrentThreadId()) // 初始化对象的线程不一定是主线程
#endif
{
    m_nMaxWidth.setWrapContent();

    m_pLayout.Attach(new SouiLayout());
    m_pLayoutParam.Attach(new SouiLayoutParam());
    m_pLayoutParam->SetMatchParent(Both);

    m_evtSet.addEvent(EVENTID(EventGetCaret));
    m_evtSet.addEvent(EVENTID(EventSwndCreate));
    m_evtSet.addEvent(EVENTID(EventSwndInitFinish));
    m_evtSet.addEvent(EVENTID(EventSwndDestroy));
    m_evtSet.addEvent(EVENTID(EventSwndSize));
    m_evtSet.addEvent(EVENTID(EventSwndPos));
    m_evtSet.addEvent(EVENTID(EventSwndMouseHover));
    m_evtSet.addEvent(EVENTID(EventSwndMouseLeave));
    m_evtSet.addEvent(EVENTID(EventSwndStateChanged));
    m_evtSet.addEvent(EVENTID(EventSwndVisibleChanged));
    m_evtSet.addEvent(EVENTID(EventSwndCaptureChanged));
    m_evtSet.addEvent(EVENTID(EventSwndUpdateTooltip));
    m_evtSet.addEvent(EVENTID(EventMouseClick));
    m_evtSet.addEvent(EVENTID(EventCmd));
    m_evtSet.addEvent(EVENTID(EventCtxMenu));
    m_evtSet.addEvent(EVENTID(EventSetFocus));
    m_evtSet.addEvent(EVENTID(EventKillFocus));

    m_evtSet.addEvent(EVENTID(EventSwndAnimationStart));
    m_evtSet.addEvent(EVENTID(EventSwndAnimationStop));
    m_evtSet.addEvent(EVENTID(EventSwndAnimationRepeat));

    IAttrStorageFactory *pAttrFac = SApplication::getSingleton().GetAttrStorageFactory();
    if (pAttrFac)
    {
        pAttrFac->CreateAttrStorage(this, &m_attrStorage);
    }
}

SWindow::~SWindow()
{
#ifdef SOUI_ENABLE_ACC
    if (m_pAcc)
    {
        SComPtr<IAccHelper> accHelper;
        if (m_pAcc->QueryInterface(__uuidof(IAccHelper), (void **)&accHelper) == S_OK)
        {
            SASSERT(accHelper->GetOwner() == NULL);
        }
    }
#endif
    SWindowMgr::DestroyWindow(m_swnd);
}

void SWindow::OnFinalRelease(THIS)
{
    if (m_funSwndProc)
    {
        m_funSwndProc(this, WM_NCDESTROY, 0, 0, NULL);
    }
    delete this;
}

BOOL SWindow::IsMsgHandled() const
{
    return m_bMsgHandled;
}

void SWindow::SetMsgHandled(BOOL bHandled)
{
    m_bMsgHandled = bHandled ? 1 : 0;
}

// Get align
UINT SWindow::GetTextAlign() const
{
    return GetStyle().GetTextAlign();
}

void SWindow::GetWindowRect(LPRECT prect) const
{
    SASSERT(prect);
    CRect rcWnd = GetWindowRect();
    memcpy(prect, &rcWnd, sizeof(RECT));
}

CRect SWindow::GetWindowRect() const
{
    return m_rcWindow;
}

void SWindow::GetClientRect(LPRECT pRect) const
{
    SASSERT(pRect);
    CRect rc = GetWindowRect();
    rc.DeflateRect(GetStyle().GetMargin());
    *pRect = rc;
}

CRect SWindow::GetClientRect() const
{
    CRect rc;
    SWindow::GetClientRect(&rc);
    return rc;
}

SStringT SWindow::GetWindowText(BOOL bRawText /*=FALSE*/)
{
    return m_strText.GetText(bRawText);
}

int SWindow::GetWindowTextU8(THIS_ IStringA *pStr, BOOL bRawText)
{
    SStringT strText = GetWindowText(bRawText);
    SStringA strA = S_CT2A(strText, CP_UTF8);
    pStr->Copy(&strA);
    return pStr->GetLength();
}

int SWindow::GetWindowText(TCHAR *pBuf, int nBufLen, BOOL bRawText)
{
    SStringT str = GetWindowText(bRawText);
    if (!pBuf)
        return str.GetLength();
    int nRet = smin(nBufLen, str.GetLength());
    _tcsncpy(pBuf, str.c_str(), nRet);
    if (nBufLen > nRet)
    {
        pBuf[nRet] = 0;
    }
    return nRet;
}

BOOL SWindow::UpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo)
{
    tipInfo.swnd = m_swnd;
    tipInfo.dwCookie = 0;
    tipInfo.rcTarget = GetWindowRect();

    SMatrix mtx = _GetMatrixEx();
    if (!mtx.isIdentity())
    {
        SRect src = SRect::IMake(tipInfo.rcTarget);
        mtx.mapRect(&src);
        tipInfo.rcTarget = src.toRect();
    }

    SStringT strTip;
    EventSwndUpdateTooltip evt(this);
    evt.bUpdated = FALSE;
    evt.strToolTip = &strTip;
    FireEvent(&evt);

    if (evt.bUpdated)
    {
        tipInfo.strTip = strTip;
        return !tipInfo.strTip.IsEmpty();
    }
    else
    {
        if (m_strToolTipText.GetText(FALSE).IsEmpty())
            return FALSE;
        tipInfo.strTip = m_strToolTipText.GetText(FALSE);
    }
    return TRUE;
}

void SWindow::SetWindowText(LPCTSTR lpszText)
{
    m_strText.SetText(lpszText, false);
    accNotifyEvent(EVENT_OBJECT_NAMECHANGE);
    OnContentChanged();
}

void SWindow::SetWindowTextU8(THIS_ LPCSTR lpszText)
{
    SStringT str = S_CA2T(lpszText, CP_UTF8);
    return SetWindowText(str);
}

void SWindow::SetEventMute(BOOL bMute)
{
    GetEventSet()->setMutedState(!!bMute);
}

void SWindow::OnContentChanged()
{
    if (IsVisible(TRUE))
        Invalidate();
    if (GetLayoutParam()->IsWrapContent(Any))
    {
        RequestRelayout();
        if (IsVisible(TRUE))
            Invalidate();
    }
    else if (GetLayoutParam()->IsMatchParent(Any) && GetParent())
    {
        GetParent()->OnContentChanged();
    }
}

void SWindow::TestMainThread()
{
#ifdef _DEBUG
    if (IsBadWritePtr(this, sizeof(SWindow)))
    {
        SASSERT_MSGA(FALSE, "this is null!!!");
    }
    else
    {
        // 当你看到这个东西的时候，我不幸的告诉你，你的其他线程在刷界面
        // 这是一件很危险的事情
        tid_t dwCurThreadID = GetCurrentThreadId();
        DWORD dwProcID = GetCurrentProcessId();
        SASSERT_FMTW(m_nMainThreadId == dwCurThreadID, L"ProcessID:%d,请准备好红包再到群里提问", dwProcID);
    }
#endif
}

// Send a message to SWindow
LRESULT SWindow::SSendMessage(UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/, BOOL *pbMsgHandled /*=NULL*/)
{
    LRESULT lResult = 0;

    ASSERT_UI_THREAD();
    SWindow *pOwner = GetOwner();
    if (pOwner && uMsg != WM_DESTROY)
        pOwner->AddRef();
    AddRef();
    SWNDMSG msgCur = { uMsg, wParam, lParam };
    SWNDMSG *pOldMsg = m_pCurMsg;
    m_pCurMsg = &msgCur;

    BOOL bMsgHandled = SwndProc(uMsg, wParam, lParam, &lResult);
    if (pbMsgHandled)
        *pbMsgHandled = bMsgHandled;

    m_pCurMsg = pOldMsg;
    Release();
    if (pOwner && uMsg != WM_DESTROY)
        pOwner->Release();
    return lResult;
}

void SWindow::SDispatchMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SSendMessage(uMsg, wParam, lParam);
    SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
    while (pChild)
    {
        pChild->SDispatchMessage(uMsg, wParam, lParam);
        pChild = pChild->GetWindow(GSW_NEXTSIBLING);
    }
}

void SWindow::Move(LPCRECT prect)
{
    ASSERT_UI_THREAD();

    if (prect)
    {
        m_bFloat = TRUE; //使用Move后，程序不再自动计算窗口坐标
        OnRelayout(*prect);
    }
    else if (GetParent())
    {
        //恢复自动计算位置
        m_bFloat = FALSE;
        //重新计算自己及兄弟窗口的坐标
        RequestRelayout();
    }
}

void SWindow::Move2(int x, int y, int cx /*=-1*/, int cy /*=-1*/)
{
    CRect rcWnd = GetWindowRect();
    if (cx == -1)
        cx = rcWnd.Width();
    if (cy == -1)
        cy = rcWnd.Height();
    CRect rcNew(x, y, x + cx, y + cy);
    Move(&rcNew);
}

// Set current cursor, when hover
BOOL SWindow::OnSetCursor(const CPoint &pt)
{
    HCURSOR hCursor = GETRESPROVIDER->LoadCursor(GetStyle().m_strCursor);
    ::SetCursor(hCursor);
    return TRUE;
}

// Get SWindow state
DWORD SWindow::GetState(void) const
{
    return m_dwState;
}

// Modify SWindow state
DWORD SWindow::ModifyState(DWORD dwStateAdd, DWORD dwStateRemove, BOOL bUpdate /*=FALSE*/)
{
    ASSERT_UI_THREAD();

    DWORD dwOldState = m_dwState;

    DWORD dwNewState = m_dwState;
    dwNewState &= ~dwStateRemove;
    dwNewState |= dwStateAdd;

    OnStateChanging(dwOldState, dwNewState);
    m_dwState = dwNewState;
    OnStateChanged(dwOldState, dwNewState);

    if (bUpdate && NeedRedrawWhenStateChange())
    {
        MarkCacheDirty(true);
        InvalidateRect(GetWindowRect());
    }
    return dwOldState;
}

ULONG_PTR SWindow::GetUserData() const
{
    return m_uData;
}

ULONG_PTR SWindow::SetUserData(ULONG_PTR uData)
{
    ULONG_PTR uOld = m_uData;
    m_uData = uData;
    return uOld;
}

BOOL SWindow::SetTimer(char id, UINT uElapse)
{
    STimerID timerID(m_swnd, id);
    return (BOOL)::SetTimer(GetContainer()->GetHostHwnd(), DWORD(timerID), uElapse, NULL);
}

BOOL SWindow::KillTimer(char id)
{
    STimerID timerID(m_swnd, id);
    return ::KillTimer(GetContainer()->GetHostHwnd(), DWORD(timerID));
}

SWND SWindow::GetSwnd() const
{
    return m_swnd;
}

IWindow *SWindow::GetIParent() const
{
    return GetParent();
}

IWindow *SWindow::GetIRoot() const
{
    return GetRoot();
}

BOOL SWindow::Destroy()
{
    ASSERT_UI_THREAD();
    if (!GetParent())
    {
        SSendMessage(WM_DESTROY);
        Release();
    }
    else
    {
        GetParent()->DestroyChild(this);
    }
    return TRUE;
}

BOOL SWindow::DestroyChild(SWindow *pChild)
{
    ASSERT_UI_THREAD();
    if (this != pChild->GetParent())
        return FALSE;
    if (pChild->m_isDestroying)
        return FALSE;
    pChild->InvalidateRect(NULL);
    pChild->SSendMessage(WM_DESTROY);
    RemoveChild(pChild);
    pChild->Release();
    return TRUE;
}

UINT SWindow::GetChildrenCount() const
{
    return m_nChildrenCount;
}

void SWindow::InsertChild(SWindow *pNewChild, SWindow *pInsertAfter /*=ICWND_LAST*/)
{
    ASSERT_UI_THREAD();
    if (pNewChild->GetParent() == this)
        return;
    OnBeforeInsertChild(pNewChild);
    pNewChild->SetContainer(GetContainer());
    pNewChild->m_pParent = this;
    pNewChild->m_pPrevSibling = pNewChild->m_pNextSibling = NULL;

    if (pInsertAfter == m_pLastChild)
        pInsertAfter = ICWND_LAST;

    if (pInsertAfter == ICWND_LAST)
    {
        // insert window at head
        pNewChild->m_pPrevSibling = m_pLastChild;
        if (m_pLastChild)
            m_pLastChild->m_pNextSibling = pNewChild;
        else
            m_pFirstChild = pNewChild;
        m_pLastChild = pNewChild;
    }
    else if (pInsertAfter == ICWND_FIRST)
    {
        // insert window at tail
        pNewChild->m_pNextSibling = m_pFirstChild;
        if (m_pFirstChild)
            m_pFirstChild->m_pPrevSibling = pNewChild;
        else
            m_pLastChild = pNewChild;
        m_pFirstChild = pNewChild;
    }
    else
    {
        // insert window at middle
        SASSERT(pInsertAfter->m_pParent == this);
        SASSERT(m_pFirstChild && m_pLastChild);
        SWindow *pNext = pInsertAfter->m_pNextSibling;
        SASSERT(pNext);
        pInsertAfter->m_pNextSibling = pNewChild;
        pNewChild->m_pPrevSibling = pInsertAfter;
        pNewChild->m_pNextSibling = pNext;
        pNext->m_pPrevSibling = pNewChild;
    }
    m_nChildrenCount++;

    m_layoutDirty = dirty_self;

    if (!GetLayout()->IsParamAcceptable(pNewChild->GetLayoutParam()))
    { //检查子窗口原有的布局属性是不是和当前窗口的布局类型是否匹配
        ILayoutParam *pLayoutParam = GetLayout()->CreateLayoutParam();
        pNewChild->SetLayoutParam(pLayoutParam);
        pLayoutParam->Release();
    }

    //继承父窗口的disable状态
    pNewChild->OnEnable(!IsDisabled(TRUE), ParentEnable);

    //只在插入新控件时需要标记zorder失效,删除控件不需要标记
    GetContainer()->MarkWndTreeZorderDirty();
    OnAfterInsertChild(pNewChild);
}

BOOL SWindow::RemoveChild(SWindow *pChild)
{
    ASSERT_UI_THREAD();
    if (this != pChild->GetParent())
        return FALSE;

    OnBeforeRemoveChild(pChild);
    pChild->SetContainer(NULL);

    SWindow *pPrevSib = pChild->m_pPrevSibling;
    SWindow *pNextSib = pChild->m_pNextSibling;

    if (pPrevSib)
        pPrevSib->m_pNextSibling = pNextSib;
    else
        m_pFirstChild = pNextSib;

    if (pNextSib)
        pNextSib->m_pPrevSibling = pPrevSib;
    else
        m_pLastChild = pPrevSib;

    pChild->m_pParent = NULL;
    pChild->m_pNextSibling = NULL;
    pChild->m_pPrevSibling = NULL;
    m_nChildrenCount--;

    OnAfterRemoveChild(pChild);
    return TRUE;
}

BOOL SWindow::IsChecked() const
{
    return WndState_Check == (m_dwState & WndState_Check);
}

BOOL SWindow::IsDisabled(BOOL bCheckParent /*= FALSE*/) const
{
    if (bCheckParent)
        return m_dwState & WndState_Disable;
    else
        return m_bDisable;
}

BOOL SWindow::IsVisible(BOOL bCheckParent /*= FALSE*/) const
{
    if (bCheckParent)
        return (0 == (m_dwState & WndState_Invisible));
    else
        return m_bVisible;
}

//因为NotifyInvalidateRect只有窗口可见时再通知刷新，这里在窗口可见状态改变前后都执行一次通知。
void SWindow::SetVisible(BOOL bVisible, BOOL bUpdate /*=FALSE*/)
{
    if (bUpdate)
        InvalidateRect(GetWindowRect());
    SSendMessage(WM_SHOWWINDOW, bVisible);
    if (bUpdate)
        InvalidateRect(GetWindowRect());
}

void SWindow::EnableWindow(BOOL bEnable, BOOL bUpdate)
{
    SSendMessage(WM_ENABLE, bEnable);
    if (bUpdate)
        InvalidateRect(GetWindowRect());
}

void SWindow::SetCheck(BOOL bCheck)
{
    if (bCheck)
        ModifyState(WndState_Check, 0, TRUE);
    else
        ModifyState(0, WndState_Check, TRUE);
}

ISwndContainer *SWindow::GetContainer()
{
    return m_pContainer;
}

const ISwndContainer *SWindow::GetContainer() const
{
    return m_pContainer;
}

void SWindow::SetContainer(ISwndContainer *pContainer)
{
    ASSERT_UI_THREAD();
    OnContainerChanged(m_pContainer, pContainer);
    SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
    while (pChild)
    {
        pChild->SetContainer(m_pContainer);
        pChild = pChild->GetWindow(GSW_NEXTSIBLING);
    }
}

void SWindow::SetOwner(SWindow *pOwner)
{
    m_pOwner = pOwner;
}

SWindow *SWindow::GetOwner() const
{
    return m_pOwner;
}

BOOL SWindow::IsMsgTransparent() const
{
    return m_bMsgTransparent;
}

const SwndStyle &SWindow::GetStyle() const
{
    return m_style;
}

SwndStyle &SWindow::GetStyle()
{
    return m_style;
}

SWindow *SWindow::_FindChildByID(int id, int nDeep)
{
    SWindow *pChild = (SWindow *)GetWindow(GSW_FIRSTCHILD);
    while (pChild)
    {
        if (pChild->GetID() == id)
            return pChild;
        pChild = (SWindow *)pChild->GetWindow(GSW_NEXTSIBLING);
    }

    if (nDeep > 0)
        nDeep--;
    if (nDeep == 0)
        return NULL;

    pChild = (SWindow *)GetWindow(GSW_FIRSTCHILD);
    while (pChild)
    {
        SWindow *pChildFind = pChild->_FindChildByID(id, nDeep);
        if (pChildFind)
            return pChildFind;
        pChild = (SWindow *)pChild->GetWindow(GSW_NEXTSIBLING);
    }

    return NULL;
}

SWindow *SWindow::_FindChildByName(const SStringW &strName, int nDeep)
{
    SWindow *pChild = (SWindow *)GetWindow(GSW_FIRSTCHILD);
    while (pChild)
    {
        if (pChild->m_strName == strName)
            return pChild;
        pChild = (SWindow *)pChild->GetWindow(GSW_NEXTSIBLING);
    }

    if (nDeep > 0)
        nDeep--;
    if (nDeep == 0)
        return NULL;

    pChild = (SWindow *)GetWindow(GSW_FIRSTCHILD);
    while (pChild)
    {
        SWindow *pChildFind = pChild->_FindChildByName(strName, nDeep);
        if (pChildFind)
            return pChildFind;
        pChild = (SWindow *)pChild->GetWindow(GSW_NEXTSIBLING);
    }

    return NULL;
}

//改用广度优先算法搜索控件,便于逐级查找 2014年12月8日
SWindow *SWindow::FindChildByID(int id, int nDeep /* =-1*/)
{
    if (id == SWindowMgr::SWND_INVALID || nDeep == 0)
        return NULL;
    SWindow *pRet = SWindowFinder::getSingletonPtr()->FindChildByID(this, id, nDeep);
    if (pRet)
        return pRet;

    pRet = _FindChildByID(id, nDeep);
    if (pRet)
        SWindowFinder::getSingletonPtr()->CacheResultForID(this, id, nDeep, pRet);
    return pRet;
}

SWindow *SWindow::FindChildByName(LPCWSTR pszName, int nDeep)
{
    if (pszName == NULL || nDeep == 0)
        return NULL;
    SStringW strName(pszName);
    if (strName.IsEmpty())
        return NULL;

    SWindow *pRet = SWindowFinder::getSingletonPtr()->FindChildByName(this, strName, nDeep);
    if (pRet)
        return pRet;

    pRet = _FindChildByName(strName, nDeep);
    if (pRet)
        SWindowFinder::getSingletonPtr()->CacheResultForName(this, strName, nDeep, pRet);
    return pRet;
}

SWindow *SWindow::FindChildByName(LPCSTR strName, int nDeep /*= -1*/)
{
    return FindChildByName(S_CA2W(strName, CP_UTF8), nDeep);
}

const static wchar_t KLabelInclude[] = L"include"; //文件包含的标签
const static wchar_t KTempNamespace[] = L"t:";     //模板识别ＮＳ
const static wchar_t KTempData[] = L"data";        //模板参数
const static wchar_t KTempParamFmt[] = L"{{%s}}";  //模板数据替换格式

BOOL SWindow::CreateChildren(SXmlNode xmlNode)
{
    ASSERT_UI_THREAD();
    SAutoEnableHostPrivUiDef enableUiDef(this);
    for (SXmlNode xmlChild = xmlNode.first_child(); xmlChild; xmlChild = xmlChild.next_sibling())
    {
        if (xmlChild.type() != node_element)
            continue;

        if (_wcsicmp(xmlChild.name(), KLabelInclude) == 0)
        { //在窗口布局中支持include标签
            SStringT strSrc = S_CW2T(xmlChild.attribute(L"src").value());
            SXmlDoc xmlDoc;
            if (LOADXML(xmlDoc, strSrc))
            {
                SXmlNode xmlInclude = xmlDoc.root().first_child();
                if (_wcsicmp(xmlInclude.name(), KLabelInclude) == 0)
                { // compatible with 2.9.0.1
                    CreateChildren(xmlInclude);
                }
                else
                {
                    // merger include attribute to xml node.
                    for (SXmlAttr attr = xmlChild.first_attribute(); attr; attr = attr.next_attribute())
                    {
                        if (_wcsicmp(attr.name(), L"src") == 0)
                            continue;
                        if (xmlInclude.attribute(attr.name()))
                        {
                            xmlInclude.attribute(attr.name()).set_value(attr.value());
                        }
                        else
                        {
                            xmlInclude.append_attribute(attr.name()).set_value(attr.value());
                        }
                    }
                    CreateChild(xmlInclude);
                }
            }
            else
            {
                SASSERT(FALSE);
            }
        }
        else if (!xmlChild.get_userdata()) //通过userdata来标记一个节点是否可以忽略
        {
            SStringW strName = xmlChild.name();
            if (strName.StartsWith(KTempNamespace))
            {
                strName = strName.Right(strName.GetLength() - 2);
                SStringW strXmlTemp = GETUIDEF->GetTemplateString(strName);
                SASSERT(!strXmlTemp.IsEmpty());
                if (!strXmlTemp.IsEmpty())
                { // create children by template.
                    SXmlNode xmlData = xmlChild.child(KTempData);
                    while (xmlData)
                    {
                        SStringW strXml = strXmlTemp;
                        for (SXmlAttr param = xmlData.first_attribute(); param; param = param.next_attribute())
                        {
                            SStringW strParam = SStringW().Format(KTempParamFmt, param.name());
                            SStringW strValue = param.value();
                            strValue.Replace(L"\"", L"&#34;");  //防止数据中包含“双引号”，导致破坏XML结构
                            strXml.Replace(strParam, strValue); // replace params to value.
                        }
                        SXmlDoc xmlDoc;
                        if (xmlDoc.load_buffer_inplace(strXml.GetBuffer(strXml.GetLength()), strXml.GetLength() * sizeof(WCHAR), 116, sizeof(wchar_t) == 2 ? enc_utf16 : enc_utf32))
                        {
                            CreateChilds(xmlDoc.root());
                        }
                        strXml.ReleaseBuffer();
                        xmlData = xmlData.next_sibling(KTempData);
                    }
                }
            }
            else
            {
                CreateChild(xmlChild);
            }
        }
    }
    if (!m_isLoading)
    {
        //动态创建子窗口，同步窗口的的属性
        if (GetScale() != 100)
            SDispatchMessage(UM_SETSCALE, GetScale(), 0);
        if (m_crColorize != 0)
            SDispatchMessage(UM_SETCOLORIZE, GetColorizeColor());
    }
    return TRUE;
}

BOOL SWindow::CreateChild(SXmlNode xmlChild)
{
    SWindow *pChild = CreateChildByName(xmlChild.name());
    if (!pChild)
    {
        return FALSE;
    }
    InsertChild(pChild);
    pChild->InitFromXml(&xmlChild);
    return TRUE;
}

void SWindow::CreateChilds(SXmlNode xmlNode)
{
    for (SXmlNode xmlChild = xmlNode.first_child(); xmlChild; xmlChild = xmlChild.next_sibling())
    {
        CreateChild(xmlChild);
    }
}

SWindow *SWindow::CreateChildByName(LPCWSTR pszName)
{
    return (SWindow *)SApplication::getSingleton().CreateWindowByName(pszName);
}

SStringW SWindow::tr(const SStringW &strSrc) const
{
    return TR(strSrc, GetTrCtx());
}

// Create SWindow from xml element
BOOL SWindow::InitFromXml(IXmlNode *pNode)
{
    ASSERT_UI_THREAD();
    SXmlNode xmlNode(pNode);
    SASSERT(m_pContainer);
    m_isLoading = true;
    if (xmlNode)
    {
        if (m_pLayoutParam)
            m_pLayoutParam->Clear();

        //优先处理"layout"属性
        SXmlAttr attrLayout = xmlNode.attribute(L"layout");
        if (attrLayout)
        {
            MarkAttributeHandled(attrLayout, true);
            SetAttribute(attrLayout.name(), attrLayout.value(), TRUE);
        }

        //优先处理"class"属性
        SXmlAttr attrClass = xmlNode.attribute(L"class");
        if (attrClass)
        {
            MarkAttributeHandled(attrClass, true);
            SetAttribute(attrClass.name(), attrClass.value(), TRUE);
        }

        __baseCls::InitFromXml(pNode);

        MarkAttributeHandled(attrClass, false);
        MarkAttributeHandled(attrLayout, false);

        if (m_strText.GetText(TRUE).IsEmpty())
        {
            SStringW strText = GetXmlText(xmlNode);
            if (!strText.IsEmpty())
            {
                OnAttrText(strText, TRUE);
            }
        }
    }

    //发送WM_CREATE消息
    if (0 != SSendMessage(WM_CREATE))
    {
        if (m_pParent)
            m_pParent->DestroyChild(this);
        return FALSE;
    }

    //给this发一个WM_SHOWWINDOW消息，一些控件需要在WM_SHOWWINDOW中处理状态
    //初始化的WM_SHOWWINDOW只影响this,子窗口的SHOW由子窗口发出。
    //不改变窗口的m_bVisible状态，需要使用ParentShow标志
    if (m_pParent)
    { //从父窗口更新状态
        if (!m_pParent->IsVisible(TRUE))
            m_dwState |= WndState_Invisible;
        if (m_pParent->IsDisabled(TRUE))
            m_dwState |= WndState_Disable;
    }
    SSendMessage(WM_SHOWWINDOW, IsVisible(TRUE), ParentShow);

    //创建子窗口
    if (!CreateChildren(xmlNode))
    {
        if (m_pParent)
            m_pParent->DestroyChild(this);
        return FALSE;
    }
    //请求根窗口重新布局。由于布局涉及到父子窗口同步进行，同步执行布局操作可能导致布局过程重复执行。
    RequestRelayout();

    EventSwndInitFinish evt(this);
    FireEvent(&evt);
    m_isLoading = false;
    return TRUE;
}

BOOL SWindow::CreateChildrenFromXml(LPCWSTR pszXml)
{
    SXmlDoc xmlDoc;
    if (!xmlDoc.load_buffer(pszXml, wcslen(pszXml) * sizeof(wchar_t), xml_parse_default, sizeof(wchar_t) == 2 ? enc_utf16 : enc_utf32))
        return FALSE;
    return CreateChildren(xmlDoc.root());
}

BOOL SWindow::CreateChildrenFromResId(LPCTSTR pszResId)
{
    SXmlDoc xmlDoc;
    if (!LOADXML(xmlDoc, pszResId))
        return FALSE;
    return CreateChildren(xmlDoc.root());
}

SWND SWindow::SwndFromPoint(POINT *pt, BOOL bIncludeMsgTransparent) const
{
    if (!pt)
        return 0;
    CPoint pt2(*pt);
    SWND ret = SwndFromPoint(pt2, bIncludeMsgTransparent);
    *pt = pt2;
    return ret;
}

// Hittest children
SWND SWindow::SwndFromPoint(CPoint &pt, BOOL bIncludeMsgTransparent) const
{
    CPoint pt2(pt);
    TransformPoint(pt2);

    if (!IsContainPoint(pt2, FALSE))
        return 0;

    if (!IsContainPoint(pt2, TRUE))
    {
        pt = pt2;      // update pt;
        return m_swnd; //只在鼠标位于客户区时，才继续搜索子窗口
    }
    SWND swndChild = 0;

    SWindow *pChild = GetWindow(GSW_LASTCHILD);
    while (pChild)
    {
        if (pChild->IsVisible(TRUE) && (bIncludeMsgTransparent || !pChild->IsMsgTransparent()))
        {
            swndChild = pChild->SwndFromPoint(pt2, bIncludeMsgTransparent);

            if (swndChild)
            {
                pt = pt2;
                return swndChild;
            }
        }

        pChild = pChild->GetWindow(GSW_PREVSIBLING);
    }
    pt = pt2; // update pt;
    return m_swnd;
}

BOOL SWindow::NeedRedrawWhenStateChange()
{
    if (m_pBgSkin && m_pBgSkin->GetStates() > 1)
    {
        return TRUE;
    }
    return GetStyle().GetStates() > 1;
}

//如果当前窗口有绘制缓存，它可能是由cache属性定义的，也可能是由于定义了alpha
void SWindow::_PaintClient(IRenderTarget *pRT)
{
    if (m_pGetRTData)
    {
        CRect &rcRT = m_pGetRTData->rcRT;
        pRT->AlphaBlend(rcRT, m_pGetRTData->rt, rcRT, 255);
    }
    else if (IsDrawToCache())
    {
        IRenderTarget *pRTCache = m_cachedRT;
        if (pRTCache)
        { //在窗口正在创建的时候进来pRTCache可能为NULL
            CRect rcWnd = GetWindowRect();
            pRTCache->SetViewportOrg(-rcWnd.TopLeft());
            if (IsCacheDirty())
            {
                pRTCache->BeginDraw();
                pRTCache->ClearRect(&rcWnd, 0);

                SAutoRefPtr<IFontS> oldFont;
                COLORREF crOld = pRT->GetTextColor();
                pRTCache->SelectObject(pRT->GetCurrentObject(OT_FONT), (IRenderObj **)&oldFont);
                pRTCache->SetTextColor(crOld);

                SSendMessage(WM_ERASEBKGND, (WPARAM)pRTCache);
                SSendMessage(WM_PAINT, (WPARAM)pRTCache);

                pRTCache->SelectObject(oldFont, NULL);
                pRTCache->SetTextColor(crOld);

                MarkCacheDirty(false);
                pRTCache->EndDraw();
            }
            pRT->AlphaBlend(rcWnd, pRTCache, rcWnd, 255);
        }
    }
    else
    {
        SSendMessage(WM_ERASEBKGND, (WPARAM)pRT);
        SSendMessage(WM_PAINT, (WPARAM)pRT);
    }
}

void SWindow::_PaintNonClient(IRenderTarget *pRT)
{
    CRect rcWnd = GetWindowRect();
    CRect rcClient = GetClientRect();
    if (rcWnd == rcClient)
        return;

    if (m_pGetRTData)
    {
        CRect &rcRT = m_pGetRTData->rcRT;
        pRT->AlphaBlend(rcRT, m_pGetRTData->rt, rcRT, 255);
    }
    else if (IsDrawToCache())
    {
        IRenderTarget *pRTCache = m_cachedRT;
        if (pRTCache)
        {
            SSendMessage(WM_NCPAINT, (WPARAM)pRTCache);
            pRT->PushClipRect(&rcClient, RGN_DIFF);
            pRT->AlphaBlend(rcWnd, pRTCache, rcWnd, 255);
            pRT->PopClip();
        }
    }
    else
    {
        SSendMessage(WM_NCPAINT, (WPARAM)pRT);
    }
}

void SWindow::_RedrawNonClient()
{
    CRect rcWnd = GetWindowRect();
    CRect rcClient = SWindow::GetClientRect();
    if (rcWnd == rcClient)
        return;
    Update();
    InvalidateRect(rcWnd, TRUE, FALSE);   // invalid window rect
    InvalidateRect(rcClient, TRUE, TRUE); // but clip client rect
    Update();
}

static SAutoRefPtr<IRegionS> ConvertRect2RenderRegion(const CRect &rc, const SMatrix &mtx)
{
    SAutoRefPtr<IRegionS> pRet;
    GETRENDERFACTORY->CreateRegion(&pRet);
    if (!mtx.isIdentity())
    {
        SRect sRc = SRect::IMake(rc);
        if (mtx.rectStaysRect())
        {
            mtx.mapRect(&sRc);
            CRect rc2 = sRc.toRect();
            pRet->CombineRect(&rc2, RGN_COPY);
        }
        else
        {
            SPoint quad[4];
            mtx.mapRectToQuad(quad, sRc);
            POINT pts[4];
            for (int i = 0; i < 4; i++)
            {
                pts[i] = quad[i].toPoint();
            }
            pRet->CombinePolygon(pts, 4, WINDING, RGN_COPY);
        }
    }
    else
    {
        pRet->CombineRect(&rc, RGN_COPY);
    }
    return pRet;
}

static bool RgnInRgn(const IRegionS *r1, IRegionS *r2)
{
    SAutoRefPtr<IRegionS> pRet;
    GETRENDERFACTORY->CreateRegion(&pRet);
    pRet->CombineRgn(r1, RGN_COPY);
    pRet->CombineRgn(r2, RGN_AND);
    return !pRet->IsEmpty();
}

bool SWindow::_WndRectInRgn(const CRect &rc, const IRegionS *rgn) const
{
    CRect rc2;
    rgn->GetRgnBox(&rc2);

    SMatrix mtx = _GetMatrixEx();
    if (mtx.isIdentity() && !rc2.IntersectRect(rc2, rc))
        return false;
    SAutoRefPtr<IRegionS> rgn2 = ConvertRect2RenderRegion(rc, mtx);
    return RgnInRgn(rgn, rgn2);
}

void SWindow::_PaintChildren(IRenderTarget *pRT, IRegionS *pRgn, UINT iBeginZorder, UINT iEndZorder)
{
    SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
    while (pChild)
    {
        if (pChild->m_uZorder >= iEndZorder)
            break;
        if (pChild->m_uZorder < iBeginZorder)
        { //看整个分枝的zorder是不是在绘制范围内
            SWindow *pNextChild = pChild->GetWindow(GSW_NEXTSIBLING);
            if (pNextChild)
            {
                if (pNextChild->m_uZorder <= iBeginZorder)
                {
                    pChild = pNextChild;
                    continue;
                }
            }
            else
            { //最后一个节点时查看最后子窗口的zorder
                SWindow *pLastChild = pChild;
                while (pLastChild->GetChildrenCount())
                {
                    pLastChild = pLastChild->GetWindow(GSW_LASTCHILD);
                }
                if (pLastChild->m_uZorder < iBeginZorder)
                {
                    break;
                }
            }
        }

        pChild->DispatchPaint(pRT, pRgn, iBeginZorder, iEndZorder);
        pChild = pChild->GetWindow(GSW_NEXTSIBLING);
    }
}

// paint zorder in [iZorderBegin,iZorderEnd) widnows
void SWindow::DispatchPaint(IRenderTarget *pRT, IRegionS *pRgn, UINT iZorderBegin, UINT iZorderEnd)
{
    if (!IsVisible(FALSE) || !GetContainer())
        return;

    SMatrix oriMtx;
    bool bMtx = _ApplyMatrix(pRT, oriMtx);

    CRect rcWnd = GetWindowRect();
    CRect rcClient = GetClientRect();
    float fMat[9];
    pRT->GetTransform(fMat);
    SMatrix curMtx(fMat);
    BOOL bRgnInClient = FALSE;
    if (curMtx.isIdentity())
    { // detect client area only if matrix is identity.
        CRect rcRgn = rcWnd;
        if (pRgn && !pRgn->IsEmpty())
        {
            pRgn->GetRgnBox(&rcRgn);
        }
        CRect rcRgnUnionClient;
        rcRgnUnionClient.UnionRect(rcClient, rcRgn);
        bRgnInClient = rcRgnUnionClient == rcClient;
    }

    IRenderTarget *pRTBackup = NULL; // backup current RT

    if (IsLayeredWindow())
    { //获得当前LayeredWindow RT来绘制内容
        pRTBackup = pRT;
        pRT = NULL;
        GETRENDERFACTORY->CreateRenderTarget(&pRT, rcWnd.Width(), rcWnd.Height());
        pRT->BeginDraw();
        pRT->OffsetViewportOrg(-rcWnd.left, -rcWnd.top, NULL);
        //绘制到窗口的缓存上,需要继承原RT的绘图属性
        pRT->SelectObject(pRTBackup->GetCurrentObject(OT_FONT), NULL);
        pRT->SelectObject(pRTBackup->GetCurrentObject(OT_PEN), NULL);
        pRT->SelectObject(pRTBackup->GetCurrentObject(OT_BRUSH), NULL);
        pRT->SetTextColor(pRTBackup->GetTextColor());
        pRT->ClearRect(&rcWnd, 0);
    }
    // save clip state
    int nSave1 = -1;
    pRT->SaveClip(&nSave1);
    if (m_clipRgn)
    {
        m_clipRgn->Offset(rcWnd.TopLeft());
        pRT->PushClipRegion(m_clipRgn, RGN_AND);
        m_clipRgn->Offset(-rcWnd.TopLeft());
    }
    if (m_clipPath)
    {
        m_clipPath->offset((float)rcWnd.left, (float)rcWnd.top);
        pRT->PushClipPath(m_clipPath, RGN_AND, true);
        m_clipPath->offset(-(float)rcWnd.left, -(float)rcWnd.top);
    }
    int nSave2 = -1;
    pRT->SaveClip(&nSave2);

    if (IsClipClient())
    {
        pRT->PushClipRect(rcClient, RGN_AND);
    }
    if (m_uZorder >= iZorderBegin && m_uZorder < iZorderEnd && (!pRgn || pRgn->IsEmpty() || _WndRectInRgn(rcClient, pRgn)))
    { // paint client
        _PaintClient(pRT);
        if (IsFocused() && m_caret)
        { // draw caret
            m_caret->Draw(pRT);
        }
    }

    SPainter painter;
    BeforePaint(pRT, painter);

    CRect rcText;
    GetTextRect(rcText);
    if (rcText != rcClient && IsClipClient())
    {
        pRT->PushClipRect(rcText, RGN_AND);
    }

    _PaintChildren(pRT, pRgn, iZorderBegin, iZorderEnd);
    AfterPaint(pRT, painter);
    pRT->RestoreClip(nSave2);

    if (m_uZorder >= iZorderBegin && m_uZorder < iZorderEnd && !bRgnInClient && (!pRgn || pRgn->IsEmpty() || _WndRectInRgn(rcWnd, pRgn)))
    { // paint nonclient
        _PaintNonClient(pRT);
    }
    // restore clip state.
    pRT->RestoreClip(nSave1);

    if (IsLayeredWindow())
    { //将绘制到窗口的缓存上的图像返回到上一级RT
        SASSERT(pRTBackup);
        pRT->EndDraw();
        OnCommitSurface(pRTBackup, &rcWnd, pRT, &rcWnd, GetAlpha());
        pRT->Release();
        pRT = pRTBackup;
    }
    if (bMtx)
        pRT->SetTransform(oriMtx.fMat, NULL);
}

void SWindow::TransformPoint(CPoint &pt) const
{
    STransformation xform = GetTransformation();
    if (xform.hasMatrix() && !xform.getMatrix().isIdentity())
    {
        CRect rc = GetWindowRect();
        SMatrix mtx = xform.getMatrix();
        mtx.preTranslate((int)-rc.left, (int)-rc.top);
        mtx.postTranslate((int)rc.left, (int)rc.top);
        if (mtx.invert(&mtx))
        {
            SPoint spt = SPoint::IMake(pt);
            mtx.mapPoints(&spt, 1);
            pt = spt.toPoint();
        }
    }
}

void SWindow::TransformPointEx(CPoint &pt) const
{
    SList<const SWindow *> lstParent;
    const SWindow *pParent = this;
    while (pParent)
    {
        lstParent.AddHead(pParent);
        pParent = pParent->GetParent();
    }
    SPOSITION pos = lstParent.GetHeadPosition();
    while (pos)
    {
        pParent = lstParent.GetNext(pos);
        pParent->TransformPoint(pt);
    }
}

//当前函数中的参数包含zorder,为了保证传递进来的zorder是正确的,必须在外面调用zorder重建.
void SWindow::_PaintRegion(IRenderTarget *pRT, IRegionS *pRgn, UINT iZorderBegin, UINT iZorderEnd)
{
    ASSERT_UI_THREAD();
    if (!IsVisible(TRUE))
        return;
    BeforePaintEx(pRT);
    DispatchPaint(pRT, pRgn, iZorderBegin, iZorderEnd);
}

void SWindow::RedrawRegion(IRenderTarget *pRT, IRegionS *pRgn)
{
    ASSERT_UI_THREAD();
    if (!IsVisible(TRUE))
        return;
    DispatchPaint(pRT, pRgn, (UINT)ZORDER_MIN, (UINT)ZORDER_MAX);
}

void SWindow::Update(BOOL bForce)
{
    if (!GetContainer())
        return;
    GetContainer()->UpdateWindow(bForce);
}

void SWindow::Invalidate()
{
    CRect rcClient;
    GetClientRect(&rcClient);
    InvalidateRect(rcClient);
}

void SWindow::InvalidateRect(LPCRECT lprect)
{
    if (lprect)
    {
        CRect rect = *lprect;
        InvalidateRect(rect);
    }
    else
    {
        InvalidateRect(GetWindowRect());
    }
}

void SWindow::InvalidateRect(const CRect &rect, BOOL bFromThis /*=TRUE*/, BOOL bClip /*=FALSE*/)
{
    ASSERT_UI_THREAD();
    if (!IsVisible(TRUE) || IsUpdateLocked() || !GetContainer())
        return;

    //只能更新窗口有效区域
    CRect rcWnd = GetWindowRect();

    CRect rcIntersect = rect & rcWnd;
    if (rcIntersect.IsRectEmpty())
        return;
    if (!bClip)
        MarkCacheDirty(true);

    STransformation xForm = GetTransformation();
    if (xForm.hasMatrix() && !xForm.getMatrix().isIdentity())
    {
        SMatrix mtx = xForm.getMatrix();
        mtx.preTranslate((int)-rcWnd.left, (int)-rcWnd.top);
        mtx.postTranslate((int)rcWnd.left, (int)rcWnd.top);
        SRect fRc = SRect::IMake(rcIntersect);
        mtx.mapRect(&fRc);
        rcIntersect = fRc.toRect();
    }
    if (GetParent())
    {
        GetParent()->InvalidateRect(rcIntersect, FALSE, bClip);
    }
    else
    {
        GetContainer()->OnRedraw(rcIntersect, bClip);
    }
}

void SWindow::LockUpdate()
{
    m_nUpdateLockCnt++;
}

void SWindow::UnlockUpdate()
{
    m_nUpdateLockCnt--;
    SASSERT(m_nUpdateLockCnt >= 0);
}

BOOL SWindow::IsUpdateLocked(BOOL bCheckParent) const
{
    BOOL bLocked = m_nUpdateLockCnt > 0;
    if (bLocked)
        return TRUE;
    if (!bCheckParent || !GetParent())
        return bLocked;
    return GetParent()->IsUpdateLocked(TRUE);
}

void SWindow::BringWindowToTop()
{
    AdjustZOrder(ICWND_LAST);
}

BOOL SWindow::AdjustZOrder(SWindow *pInsertAfter)
{
    ASSERT_UI_THREAD();
    SWindow *pParent = GetParent();
    if (!pParent)
        return TRUE;
    if (m_isDestroying)
        return TRUE;
    if (pInsertAfter != ICWND_FIRST && pInsertAfter != ICWND_LAST && pInsertAfter->GetParent() != pParent)
    {
        return FALSE;
    }
    if (pInsertAfter == this)
    {
        return FALSE;
    }
    pParent->RemoveChild(this);
    pParent->InsertChild(this, pInsertAfter);
    pParent->Invalidate();
    return TRUE;
}

BOOL SWindow::FireEvent(IEvtArgs *evt)
{
    ASSERT_UI_THREAD();
    if (m_evtSet.isMuted() || !GetContainer())
        return FALSE;

    AddRef();
    BOOL bRet = FALSE;
    do
    {
        //调用事件订阅的处理方法
        m_evtSet.FireEvent(evt);
        if (!evt->IsBubbleUp())
        {
            bRet = evt->HandleCount() > 0;
            break;
        }

        //调用脚本事件处理方法
        if (GetScriptModule())
        {
            SStringW strEvtName = evt->GetName();
            if (!strEvtName.IsEmpty())
            {
                SStringA strScriptHandler = m_evtSet.getEventScriptHandler(strEvtName);
                if (!strScriptHandler.IsEmpty())
                {
                    GetScriptModule()->executeScriptedEventHandler(strScriptHandler, evt);
                    if (!evt->IsBubbleUp())
                    {
                        bRet = evt->HandleCount() > 0;
                        break;
                    }
                }
            }
        }
        if (GetOwner())
        {
            bRet = GetOwner()->FireEvent(evt);
            break;
        }
        bRet = GetContainer()->OnFireEvent(evt);
    } while (false);
    Release();
    return bRet;
}

BOOL SWindow::OnRelayout(const CRect &rcWnd)
{
    if (m_rcWindow.EqualRect(&rcWnd) && m_layoutDirty == dirty_clean)
        return FALSE;
    CRect rcLayout;
    GetChildrenLayoutRect(&rcLayout);
    CPoint ptDiff = (rcWnd.left - m_rcWindow.left, rcWnd.top - m_rcWindow.top);
    if (!m_rcWindow.EqualRect(&rcWnd))
    {
        InvalidateRect(m_rcWindow);
        m_rcWindow = rcWnd;

        m_layoutDirty = dirty_self;

        if (m_rcWindow.left > m_rcWindow.right)
            m_rcWindow.right = m_rcWindow.left;
        if (m_rcWindow.top > m_rcWindow.bottom)
            m_rcWindow.bottom = m_rcWindow.top;

        InvalidateRect(m_rcWindow);

        SSendMessage(WM_NCCALCSIZE); //计算非客户区大小
    }
    // keep relative position of float children
    if (ptDiff.x != 0 || ptDiff.y != 0)
    {
        CRect rcLayout2;
        GetChildrenLayoutRect(&rcLayout2);

        SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
        while (pChild)
        {
            if (pChild->IsFloat())
            {
                CRect rcChild = pChild->GetWindowRect();
                CPoint ptRelative(rcChild.left - rcLayout.left, rcChild.top - rcLayout.top); // relative pos
                rcChild.MoveToXY(rcLayout2.left + ptRelative.x, rcLayout2.top + ptRelative.y);
                pChild->Move(rcChild);
            }
            pChild = pChild->GetWindow(GSW_NEXTSIBLING);
        }
    }
    // only if window is visible now, we do relayout.
    if (IsVisible(FALSE))
    {
        // don't call UpdateLayout, otherwise will result in dead cycle.
        if (m_layoutDirty != dirty_clean && GetChildrenCount())
        {
            UpdateChildrenPosition(); //更新子窗口位置
        }
        m_layoutDirty = dirty_clean;
    }
    else
    { // mark layout to self dirty.
        m_layoutDirty = dirty_self;
    }

    EventSwndPos evt(this);
    evt.rcWnd = m_rcWindow;
    FireEvent(evt);

    CRect rcClient;
    GetClientRect(&rcClient);
    SSendMessage(WM_SIZE, 0, MAKELPARAM(rcClient.Width(), rcClient.Height()));
    return TRUE;
}

int SWindow::OnCreate(LPVOID)
{
    SASSERT(GetContainer());
    if (GetStyle().m_bTrackMouseEvent)
        GetContainer()->RegisterTrackMouseEvent(m_swnd);
    if (GetStyle().m_bVideoCanvas)
        GetContainer()->RegisterVideoCanvas(m_swnd);

    GetStyle().SetScale(GetScale());

    EventSwndCreate evt(this);
    FireEvent(evt);
    accNotifyEvent(EVENT_OBJECT_CREATE);
    return 0;
}

IWindow *SWindow::FindIChildByID(THIS_ int nId)
{
    return FindChildByID(nId, -1);
}

IWindow *SWindow::FindIChildByName(THIS_ LPCWSTR pszName)
{
    return FindChildByName(pszName, -1);
}

IWindow *SWindow::FindIChildByNameA(THIS_ LPCSTR pszName)
{
    return FindChildByName(pszName, -1);
}

void SWindow::DestroyAllChildren()
{
    // destroy children windows
    SWindow *pChild = m_pFirstChild;
    while (pChild)
    {
        SWindow *pNextChild = pChild->m_pNextSibling;
        pChild->SSendMessage(WM_DESTROY);
        pChild->Release();

        pChild = pNextChild;
    }
    m_pFirstChild = m_pLastChild = NULL;
    m_nChildrenCount = 0;
}

void SWindow::OnDestroy()
{
    if (m_isDestroying)
        return;

    m_isDestroying = true;
    EventSwndDestroy evt(this);
    FireEvent(evt);
    accNotifyEvent(EVENT_OBJECT_DESTROY);

#ifdef SOUI_ENABLE_ACC
    if (m_pAcc)
    {
        SComPtr<IAccHelper> accHelper;
        if (m_pAcc->QueryInterface(__uuidof(IAccHelper), (void **)&accHelper) == S_OK)
        {
            accHelper->SetOwner(NULL);
        }
    }
#endif
    if (GetStyle().m_bTrackMouseEvent)
        GetContainer()->UnregisterTrackMouseEvent(m_swnd);
    if (GetStyle().m_bVideoCanvas)
        GetContainer()->UnregisterVideoCanvas(m_swnd);

    DestroyAllChildren();
    ClearAnimation();
    m_style = SwndStyle();
    m_isDestroying = false;
}

// Draw background default
BOOL SWindow::OnEraseBkgnd(IRenderTarget *pRT)
{
    CRect rcClient = GetClientRect();
    if (!m_pBgSkin)
    {
        COLORREF crBg = GetBkgndColor();

        if (CR_INVALID != crBg)
        {
            pRT->FillSolidRect(&rcClient, crBg);
        }
    }
    else
    {
        int idx = SState2Index::GetDefIndex(GetState(), true);
        if (idx >= m_pBgSkin->GetStates())
            idx = 0;
        m_pBgSkin->DrawByIndex(pRT, rcClient, idx);
    }
    return TRUE;
}

void SWindow::BeforePaint(IRenderTarget *pRT, SPainter &painter)
{
    int iState = SState2Index::GetDefIndex(GetState(), true);
    IFontPtr pFont = GetStyle().GetTextFont(iState);
    if (pFont)
        pRT->SelectObject(pFont, (IRenderObj **)&painter.oldFont);

    COLORREF crTxt = GetStyle().GetTextColor(iState);
    if (crTxt != CR_INVALID)
        painter.oldTextColor = pRT->SetTextColor(crTxt);
}

void SWindow::BeforePaintEx(IRenderTarget *pRT)
{
    SWindow *pParent = GetParent();
    if (pParent)
        pParent->BeforePaintEx(pRT);
    SPainter painter;
    BeforePaint(pRT, painter);
}

void SWindow::AfterPaint(IRenderTarget *pRT, SPainter &painter)
{
    if (painter.oldFont)
        pRT->SelectObject(painter.oldFont, NULL);
    if (painter.oldTextColor != CR_INVALID)
        pRT->SetTextColor(painter.oldTextColor);
}

// Draw inner text default and focus rect
void SWindow::OnPaint(IRenderTarget *pRT)
{
    SPainter painter;

    BeforePaint(pRT, painter);

    CRect rcText;
    GetTextRect(rcText);
    SStringT strText = GetWindowText(FALSE);
    DrawText(pRT, strText, strText.GetLength(), rcText, GetTextAlign());

    // draw focus rect
    if (IsFocused())
    {
        DrawFocus(pRT);
    }

    AfterPaint(pRT, painter);
}

void SWindow::OnNcPaint(IRenderTarget *pRT)
{
    if (!IsVisible(TRUE))
        return;
    if (!GetStyle().GetMargin().IsRectNull())
    {
        SASSERT(pRT);
        CRect rcClient = SWindow::GetClientRect();
        CRect rcWnd = GetWindowRect();

        pRT->PushClipRect(&rcClient, RGN_DIFF);

        int nState = 0;
        if (WndState_Hover & m_dwState)
            nState = 1;
        if (m_pNcSkin)
        {
            if (nState >= m_pNcSkin->GetStates())
                nState = 0;
            m_pNcSkin->DrawByIndex(pRT, &rcWnd, nState);
        }
        else
        {
            COLORREF crBg = GetStyle().m_crBorder;
            if (CR_INVALID != crBg)
            {
                pRT->FillSolidRect(&rcWnd, crBg);
            }
        }
        pRT->PopClip();
    }
}

static const int KWnd_MaxSize = 10000;
void SWindow::GetDesiredSize(SIZE *psz, int nParentWid, int nParentHei)
{
    if (m_funSwndProc)
    {
        //使用回调函数计算窗口Size
        BOOL bRet = m_funSwndProc(this, UM_GETDESIREDSIZE, nParentHei, nParentHei, (LRESULT *)psz);
        if (bRet)
        {
            return;
        }
    }
    //检查当前窗口的MatchParent属性及容器窗口的WrapContent属性。
    ILayoutParam *pLayoutParam = GetLayoutParam();
    bool bSaveHorz = nParentWid == SIZE_WRAP_CONTENT && pLayoutParam->IsMatchParent(Horz);
    bool bSaveVert = nParentHei == SIZE_WRAP_CONTENT && pLayoutParam->IsMatchParent(Vert);
    if (bSaveHorz)
        pLayoutParam->SetWrapContent(Horz);
    if (bSaveVert)
        pLayoutParam->SetWrapContent(Vert);

    CSize szRet(KWnd_MaxSize, KWnd_MaxSize);
    if (pLayoutParam->IsSpecifiedSize(Horz))
    { //检查设置大小
        szRet.cx = pLayoutParam->GetSpecifiedSize(Horz).toPixelSize(GetScale());
    }
    else if (pLayoutParam->IsMatchParent(Horz) && nParentWid >= 0)
    {
        szRet.cx = nParentWid;
    }

    if (pLayoutParam->IsSpecifiedSize(Vert))
    { //检查设置大小
        szRet.cy = pLayoutParam->GetSpecifiedSize(Vert).toPixelSize(GetScale());
    }
    else if (pLayoutParam->IsMatchParent(Vert) && nParentHei >= 0)
    {
        szRet.cy = nParentHei;
    }

    if (szRet.cx == KWnd_MaxSize || szRet.cy == KWnd_MaxSize)
    {
        int nTestDrawMode = GetTextAlign() & ~(DT_CENTER | DT_RIGHT | DT_VCENTER | DT_BOTTOM);

        CRect rcPadding = GetStyle().GetPadding();
        CRect rcMargin = GetStyle().GetMargin();

        CSize szContent = MeasureContent(szRet.cx, szRet.cy);
        CSize szChilds;
        if (GetChildrenCount() > 0)
        {
            //计算子窗口大小
            CSize szParent(nParentWid, nParentHei);
            if (nParentWid > 0)
            {
                szParent.cx -= rcMargin.left + rcPadding.left + rcMargin.right + rcPadding.right;
                if (szParent.cx < 0)
                    szParent.cx = 0;
            }
            if (nParentHei > 0)
            {
                szParent.cy -= rcMargin.top + rcPadding.top + rcMargin.bottom + rcPadding.bottom;
                if (szParent.cy < 0)
                    szParent.cy = 0;
            }
            szChilds = MeasureChildren(szParent.cx, szParent.cy);
        }

        CRect rcTest(0, 0, smax(szChilds.cx, szContent.cx), smax(szChilds.cy, szContent.cy));

        rcTest.InflateRect(rcMargin);
        rcTest.InflateRect(rcPadding);

        if (pLayoutParam->IsWrapContent(Horz) || (pLayoutParam->IsMatchParent(Horz) && nParentWid < SIZE_WRAP_CONTENT))
            szRet.cx = rcTest.Width();
        if (pLayoutParam->IsWrapContent(Vert) || (pLayoutParam->IsMatchParent(Vert) && nParentHei < SIZE_WRAP_CONTENT))
            szRet.cy = rcTest.Height();
    }

    if (bSaveHorz)
        pLayoutParam->SetMatchParent(Horz);
    if (bSaveVert)
        pLayoutParam->SetMatchParent(Vert);

    *psz = szRet;
}

SIZE SWindow::MeasureContent(int nParentWid, int nParentHei)
{
    ILayoutParam *pLayoutParam = GetLayoutParam();
    CRect rcPadding = GetStyle().GetPadding();

    //计算文本大小
    SStringT strText = GetWindowText(FALSE);
    CRect rcTest4Text;
    if (!strText.IsEmpty())
    {
        int nTestDrawMode = GetTextAlign() & ~(DT_CENTER | DT_RIGHT | DT_VCENTER | DT_BOTTOM);
        rcTest4Text = CRect(0, 0, nParentWid, nParentHei);
        int nMaxWid = pLayoutParam->IsWrapContent(Horz) ? m_nMaxWidth.toPixelSize(GetScale()) : nParentWid;
        if (nMaxWid == SIZE_WRAP_CONTENT)
        {
            nMaxWid = KWnd_MaxSize;
        }
        else // if(nMaxWid >= SIZE_SPEC)
        {
            nMaxWid -= rcPadding.left + rcPadding.right;
            nTestDrawMode |= DT_WORDBREAK;
        }
        rcTest4Text.right = smax(nMaxWid, 10);
        SAutoRefPtr<IRenderTarget> pRT;
        GETRENDERFACTORY->CreateRenderTarget(&pRT, 0, 0);
        BeforePaintEx(pRT);
        DrawText(pRT, strText, strText.GetLength(), rcTest4Text, nTestDrawMode | DT_CALCRECT);
    }
    return rcTest4Text.Size();
}

SIZE SWindow::MeasureChildren(int nParentWid, int nParentHei)
{
    return GetLayout()->MeasureChildren(this, nParentWid, nParentHei);
}

void SWindow::GetTextRect(LPRECT pRect)
{
    CRect rcClient = GetClientRect();
    rcClient.DeflateRect(GetStyle().GetPadding());
    *pRect = rcClient;
}

void SWindow::DrawText(IRenderTarget *pRT, LPCTSTR pszBuf, int cchText, LPRECT pRect, UINT uFormat)
{
    pRT->DrawText(pszBuf, cchText, pRect, uFormat);
}

void SWindow::DrawFocus(IRenderTarget *pRT)
{
    CRect rcFocus;
    GetTextRect(&rcFocus);
    if (IsFocusable() && m_bDrawFocusRect)
        DrawDefFocusRect(pRT, rcFocus);
}

void SWindow::DrawDefFocusRect(IRenderTarget *pRT, CRect rcFocus)
{
    rcFocus.DeflateRect(2, 2);
    SAutoRefPtr<IPenS> pPen, oldPen;
    pRT->CreatePen(PS_DOT, RGBA(88, 88, 88, 0xFF), 1, &pPen);
    pRT->SelectObject(pPen, (IRenderObj **)&oldPen);
    pRT->DrawRectangle(&rcFocus);
    pRT->SelectObject(oldPen, NULL);
}

UINT SWindow::OnGetDlgCode() const
{
    return 0;
}

BOOL SWindow::IsFocusable() const
{
    return m_bFocusable;
}

void SWindow::OnShowWindow(BOOL bShow, UINT nStatus)
{
    if (nStatus == ParentShow)
    {
        if (bShow && !IsVisible(FALSE))
            bShow = FALSE;
    }
    else
    {
        m_bVisible = bShow;
    }
    if (bShow && m_pParent)
    {
        bShow = m_pParent->IsVisible(TRUE);
    }
    if (bShow)
    { // delay refresh layout of children.
        UpdateLayout();
    }
    if (bShow)
    {
        ModifyState(0, WndState_Invisible);
        accNotifyEvent(EVENT_OBJECT_SHOW);
    }
    else
    {
        ModifyState(WndState_Invisible, 0);
        accNotifyEvent(EVENT_OBJECT_HIDE);
    }

    SWindow *pChild = m_pFirstChild;
    while (pChild)
    {
        pChild->AddRef();
        pChild->SSendMessage(WM_SHOWWINDOW, bShow, ParentShow);
        SWindow *pNextChild = pChild->GetWindow(GSW_NEXTSIBLING);
        ;
        pChild->Release();
        pChild = pNextChild;
    }
    if (!IsVisible(TRUE))
    {
        if (IsFocused() && GetContainer())
            GetContainer()->OnSetSwndFocus(0); //窗口隐藏时自动失去焦点
        if (GetCapture() == m_swnd)
            ReleaseCapture(); //窗口隐藏时自动失去Capture
    }

    if (!m_bDisplay)
    {
        RequestRelayout();
    }

    EventSwndVisibleChanged evtShow(this);
    evtShow.bVisible = bShow;
    FireEvent(evtShow);
}

void SWindow::OnEnable(BOOL bEnable, UINT nStatus)
{
    if (nStatus == ParentEnable)
    {
        if (bEnable && IsDisabled(FALSE))
            bEnable = FALSE;
    }
    else
    {
        m_bDisable = !bEnable;
    }
    if (bEnable && m_pParent)
    {
        bEnable = !m_pParent->IsDisabled(TRUE);
    }

    if (bEnable)
        ModifyState(0, WndState_Disable);
    else
        ModifyState(WndState_Disable, WndState_Hover);

    SWindow *pChild = m_pFirstChild;
    while (pChild)
    {
        pChild->SSendMessage(WM_ENABLE, bEnable, ParentEnable);
        pChild = pChild->GetWindow(GSW_NEXTSIBLING);
    }
    if (IsDisabled(TRUE) && IsFocused() && GetContainer())
    {
        GetContainer()->OnSetSwndFocus(0);
    }
}

void SWindow::OnLButtonDown(UINT nFlags, CPoint pt)
{
    if (m_bFocusable)
        SetFocus();
    SetCapture();
    ModifyState(WndState_PushDown, 0, TRUE);
}

void SWindow::OnLButtonDbClick(UINT nFlags, CPoint point)
{
    OnLButtonDown(nFlags, point);
}

void SWindow::OnLButtonUp(UINT nFlags, CPoint pt)
{
    ReleaseCapture();

    if (!(GetState() & WndState_PushDown))
        return;

    ModifyState(0, WndState_PushDown, TRUE);
    if (!GetWindowRect().PtInRect(pt))
        return;

    if (GetID() || GetName())
    {
        FireCommand();
    }
}

void SWindow::OnRButtonDown(UINT nFlags, CPoint point)
{
}

void SWindow::OnRButtonUp(UINT nFlags, CPoint point)
{
    FireCtxMenu(point);
}

void SWindow::OnMouseMove(UINT nFlags, CPoint pt)
{
}

void SWindow::OnMouseHover(UINT nFlags, CPoint ptPos)
{
    if (!m_bHoverAware)
        return;
    if (GetCapture() == m_swnd)
        ModifyState(WndState_PushDown, 0, FALSE);
    ModifyState(WndState_Hover, 0, TRUE);
    _RedrawNonClient();
    EventSwndMouseHover evtHover(this);
    FireEvent(evtHover);
}

void SWindow::OnMouseLeave()
{
    if (!m_bHoverAware)
        return;
    if (GetCapture() == m_swnd)
        ModifyState(0, WndState_PushDown, FALSE);
    ModifyState(0, WndState_Hover, TRUE);
    _RedrawNonClient();
    EventSwndMouseLeave evtLeave(this);
    FireEvent(evtLeave);
}

BOOL SWindow::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    BOOL bRet = FALSE;
    if (m_pParent)
        bRet = (BOOL)m_pParent->SSendMessage(WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt.x, pt.y));
    return bRet;
}

LRESULT SWindow::OnMouseClick(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SetMsgHandled(FALSE);

    EventMouseClick evt(this);
    evt.clickId = MouseClickId(uMsg - WM_LBUTTONDOWN);
    evt.uFlags = (UINT)wParam;
    evt.pt.x = GET_X_LPARAM(lParam);
    evt.pt.y = GET_Y_LPARAM(lParam);
    evt.bHover = GetClientRect().PtInRect(evt.pt);
    FireEvent(&evt);
    return 0;
}

void SWindow::GetChildrenLayoutRect(RECT *prc) const
{
    CRect rcRet;
    GetClientRect(rcRet);
    rcRet.DeflateRect(GetStyle().GetPadding());
    *prc = rcRet;
}

void SWindow::UpdateChildrenPosition()
{
    if (m_layoutDirty == dirty_self)
    { //当前窗口所有子窗口全部重新布局
        GetLayout()->LayoutChildren(this);

        SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
        while (pChild)
        {
            if (pChild->m_bFloat)
            {
                RECT rcChild;
                GetChildrenLayoutRect(&rcChild);
                pChild->OnUpdateFloatPosition(rcChild);
                if (pChild->m_layoutDirty != dirty_clean)
                {
                    pChild->UpdateChildrenPosition();
                }
            }
            pChild = pChild->GetWindow(GSW_NEXTSIBLING);
        }
    }
    else if (m_layoutDirty == dirty_child)
    { //只有个别子窗口需要重新布局
        SWindow *pChild = GetNextLayoutChild(NULL);
        while (pChild)
        {
            if (pChild->IsLayoutDirty())
            {
                pChild->UpdateChildrenPosition();
            }
            pChild = GetNextLayoutChild(pChild);
        }
    }
}

void SWindow::RequestRelayout()
{
    RequestRelayout(m_swnd, TRUE); //此处bSourceResizable可以为任意值
}

void SWindow::RequestRelayout(SWND hSource, BOOL bSourceResizable)
{
    SASSERT(SWindowMgr::IsWindow(hSource));

    if (bSourceResizable)
    { //源窗口大小发生变化,当前窗口的所有子窗口全部重新布局
        m_layoutDirty = dirty_self;
    }

    if (m_layoutDirty != dirty_self)
    { //需要检测当前窗口是不是内容自适应
        m_layoutDirty = (hSource == m_swnd || GetLayoutParam()->IsWrapContent(Any)) ? dirty_self : dirty_child;
    }

    SWindow *pParent = GetParent();
    if (pParent && pParent->IsVisible())
    {
        pParent->RequestRelayout(hSource, GetLayoutParam()->IsWrapContent(Any) || !IsDisplay());
    }
}

void SWindow::UpdateLayout()
{
    if (m_layoutDirty == dirty_clean)
        return;
    if (GetChildrenCount())
        UpdateChildrenPosition();
    m_layoutDirty = dirty_clean;
}

IWindow *SWindow::GetNextLayoutIChild(THIS_ const IWindow *pCurChild) const
{
    return GetNextLayoutChild((const SWindow *)pCurChild);
}

void SWindow::OnSetFocus(SWND wndOld)
{
    InvalidateRect(GetWindowRect());
    accNotifyEvent(EVENT_OBJECT_FOCUS);
    EventSetFocus evt(this);
    evt.wndOld = wndOld;
    FireEvent(evt);
}

void SWindow::OnKillFocus(SWND wndFocus)
{
    InvalidateRect(GetWindowRect());
    EventKillFocus evt(this);
    evt.wndFocus = wndFocus;
    FireEvent(evt);
}

LRESULT SWindow::OnSetScale(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int nScale = (int)wParam;
    OnScaleChanged(nScale);
    return 0;
}

LRESULT SWindow::OnSetLanguage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = OnLanguageChanged();
    if (hr == S_FALSE)
        Invalidate();
    else if (hr == S_OK)
        RequestRelayout(m_swnd, TRUE);
    return 0;
}

BOOL SWindow::IsLayeredWindow() const
{
    return m_bLayeredWindow || GetAlpha() != 0xFF;
}

//查询当前窗口内容将被渲染到哪一个渲染层上，没有渲染层时返回NULL
SWindow *SWindow::_GetCurrentLayeredWindow()
{
    SWindow *pWnd = this;
    while (pWnd)
    {
        if (pWnd->IsLayeredWindow())
        {
            break;
        }
        pWnd = pWnd->GetParent();
    }

    return pWnd;
}

IRenderTarget *SWindow::GetRenderTarget(LPCRECT pRc, GrtFlag gdcFlags /*=GRT_NODRAW*/, BOOL bClientRT /*=TRUE*/)
{
    CRect rcRT;
    if (bClientRT)
    {
        GetClientRect(&rcRT);
    }
    else
    {
        GetWindowRect(&rcRT);
    }
    if (pRc)
        rcRT.IntersectRect(pRc, &rcRT);

    SAutoRefPtr<IRegionS> rgn;
    GETRENDERFACTORY->CreateRegion(&rgn);
    rgn->CombineRect(rcRT, RGN_COPY);

    return GetRenderTarget(gdcFlags, rgn);
}

IRenderTarget *SWindow::GetRenderTarget(GrtFlag gdcFlags, IRegionS *pRgn)
{
    SASSERT(!m_pGetRTData);

    BOOL bRenderValid = GetContainer() && IsVisible(TRUE) && !IsUpdateLocked(TRUE);
    if (!bRenderValid)
    { // return a empty render target
        IRenderTarget *pRT = NULL;
        GETRENDERFACTORY->CreateRenderTarget(&pRT, 0, 0);
        return pRT;
    }

    GetContainer()->BuildWndTreeZorder();

    CRect rcRT;
    pRgn->GetRgnBox(&rcRT);
    IRenderTarget *pRT = NULL;
    m_pGetRTData = new GETRTDATA;
    GETRENDERFACTORY->CreateRenderTarget(&pRT, rcRT.Width(), rcRT.Height());
    pRT->BeginDraw();
    pRT->OffsetViewportOrg(-rcRT.left, -rcRT.top, NULL);
    BeforePaintEx(pRT);
    pRT->PushClipRegion(pRgn, RGN_COPY);
    pRT->ClearRect(&rcRT, 0);
    m_pGetRTData->gdcFlags = gdcFlags;
    m_pGetRTData->rcRT = rcRT;
    m_pGetRTData->rgn = pRgn;
    m_pGetRTData->rt = pRT;
    return pRT;
}

void SWindow::ReleaseRenderTarget(IRenderTarget *pRT)
{
    pRT->Release();
    if (!m_pGetRTData)
        return;
    if (m_pGetRTData->gdcFlags != GRT_NODRAW)
    {
        m_pGetRTData->rt->EndDraw();
        SMatrix mtx;
        SWindow *p = this;
        while (p)
        {
            STransformation xform = p->GetTransformation();
            if (xform.hasMatrix() && !xform.getMatrix().isIdentity())
            {
                SMatrix mtx2 = xform.getMatrix();
                CRect rc = p->GetWindowRect();
                mtx2.preTranslate((int)-rc.left, (int)-rc.top);
                mtx2.postTranslate((int)rc.left, (int)rc.top);
                mtx.preConcat(mtx2);
            }
            p = p->GetParent();
        }

        CRect rcRT = m_pGetRTData->rcRT;
        if (!mtx.isIdentity())
        {
            SRect sRcRT = SRect::IMake(rcRT);
            mtx.mapRect(&sRcRT);
            rcRT = sRcRT.toRect();
        }

        SASSERT(GetContainer());
        SWindow *pRoot = GetRoot();
        SAutoRefPtr<IRegionS> rgn;
        GETRENDERFACTORY->CreateRegion(&rgn);
        if (!mtx.isIdentity())
        {
            SRect sRcRT = SRect::IMake(m_pGetRTData->rcRT);
            SPoint quad[4];
            mtx.mapRectToQuad(quad, sRcRT);
            POINT pts[4];
            for (int i = 0; i < 4; i++)
            {
                pts[i] = quad[i].toPoint();
            }
            rgn->CombinePolygon(pts, 4, WINDING, RGN_COPY);
        }
        else
        {
            rgn->CombineRect(rcRT, RGN_COPY);
        }
        if (mtx.isIdentity())
        { // todo: if matrix transform existed, combine getrt.rgn to the root rgn will not work.
            rgn->CombineRgn(m_pGetRTData->rgn, RGN_AND);
        }
        if (!rgn->IsEmpty())
            GetContainer()->UpdateRegion(rgn);
    }
    delete m_pGetRTData;
    m_pGetRTData = NULL;
}

bool SWindow::_ApplyMatrix(IRenderTarget *pRT, SMatrix &oriMtx)
{
    STransformation xform = GetTransformation();
    if (!xform.hasMatrix() || xform.getMatrix().isIdentity())
        return false;
    pRT->GetTransform(oriMtx.fMat);
    CRect rcWnd = GetWindowRect();
    SMatrix mtx = xform.getMatrix();
    mtx.preTranslate((int)-rcWnd.left, (int)-rcWnd.top);
    mtx.postTranslate((int)rcWnd.left, (int)rcWnd.top);
    mtx.preConcat(oriMtx);
    pRT->SetTransform(mtx.fMat, NULL);
    return true;
}

SMatrix SWindow::_GetMatrixEx() const
{
    SMatrix mtx;
    const SWindow *p = this;
    while (p)
    {
        STransformation xform = p->GetTransformation();
        if (xform.hasMatrix() && !xform.getMatrix().isIdentity())
        {
            SMatrix &mtx2 = xform.getMatrix();
            CRect rcWnd = p->GetWindowRect();
            mtx2.preTranslate((int)-rcWnd.left, (int)-rcWnd.top);
            mtx2.postTranslate((int)rcWnd.left, (int)rcWnd.top);
            mtx.preConcat(mtx2);
        }
        p = p->GetParent();
    }
    return mtx;
}

SWND SWindow::GetCapture() const
{
    if (!GetContainer())
        return 0;
    return GetContainer()->OnGetSwndCapture();
}

void SWindow::OnCaptureChanged(BOOL bCaptured)
{
    EventSwndCaptureChanged evt(this);
    evt.bCaptured = bCaptured;
    FireEvent(&evt);
}

SWND SWindow::SetCapture()
{
    if (!GetContainer())
        return 0;
    return GetContainer()->OnSetSwndCapture(m_swnd);
}

BOOL SWindow::ReleaseCapture()
{
    if (!GetContainer())
        return FALSE;
    return GetContainer()->OnReleaseSwndCapture();
}

/**
 * Sets the next animation to play for this view.
 * If you want the animation to play immediately, use
 * {@link #startAnimation(android.view.animation.Animation)} instead.
 * This method provides allows fine-grained
 * control over the start time and invalidation, but you
 * must make sure that 1) the animation has a start time set, and
 * 2) the view's parent (which controls animations on its children)
 * will be invalidated when the animation is supposed to
 * start.
 *
 * @param animation The next animation, or null.
 */

void SWindow::SetAnimation(IAnimation *animation)
{
    if (m_isAnimating)
    {
        ClearAnimation();
    }
    m_animation = animation;
    if (m_animation)
    {
        m_animation->setAnimationListener(this);
        if (m_animation->getStartTime() == START_ON_FIRST_FRAME)
        {
            m_animation->startNow();
        }
        if (GetContainer())
            GetContainer()->RegisterTimelineHandler(&m_animationHandler);
    }
}

/**
 * Get the animation currently associated with this view.
 *
 * @return The animation that is currently playing or
 *         scheduled to play for this view.
 */

IAnimation *SWindow::GetAnimation() const
{
    return m_animation;
}

/**
 * Start the specified animation now.
 *
 * @param animation the animation to start now
 */

void SWindow::StartAnimation(IAnimation *animation)
{
    SASSERT(animation);
    animation->setStartTime(START_ON_FIRST_FRAME);
    SetAnimation(animation);
}

/**
 * Cancels any animations for this view.
 */
void SWindow::ClearAnimation()
{
    if (m_animation)
    {
        if (m_isAnimating)
        {
            m_animation->cancel();
            OnAnimationStop(m_animation);
        }
        else if (GetContainer())
        {
            GetContainer()->UnregisterTimelineHandler(&m_animationHandler);
        }
        m_animation->setAnimationListener(NULL);
        m_animation = NULL;
    }
}

STransformation SWindow::GetTransformation() const
{
    if (!m_isAnimating && !m_animationHandler.getFillAfter())
        return m_transform;
    else
    {
        STransformation ret = m_transform;
        ret.postCompose(m_animationHandler.GetTransformation());
        return ret;
    }
}

void SWindow::SetMatrix(const SMatrix &mtx)
{
    InvalidateRect(NULL);
    m_transform.setMatrix(mtx);
    InvalidateRect(NULL);
}

void SWindow::SetAlpha(BYTE byAlpha)
{
    m_transform.SetAlpha(byAlpha);
    InvalidateRect(NULL);
}

BYTE SWindow::GetAlpha() const
{
    return (BYTE)((int)m_transform.GetAlpha() * m_animationHandler.GetTransformation().GetAlpha() / 255);
}

void SWindow::SetMatrix(const IMatrix *mtx)
{
    SMatrix smtx(mtx->Data()->fMat);
    SetMatrix(&smtx);
}

void SWindow::GetMatrix(IMatrix *mtx) SCONST
{
    SMatrix smtx = m_transform.getMatrix();
    smtx.postConcat(m_animationHandler.GetTransformation().getMatrix());
    memcpy(mtx->Data()->fMat, smtx.fMat, sizeof(smtx.fMat));
}

void SWindow::SetFocus()
{
    if (!IsVisible(TRUE) || IsDisabled(TRUE) || !IsFocusable())
        return;
    if (!GetContainer())
        return;
    GetContainer()->OnSetSwndFocus(m_swnd);
}

void SWindow::KillFocus()
{
    if (IsFocused())
    {
        if (!GetContainer())
            return;
        GetContainer()->OnSetSwndFocus(0);
    }
}

BOOL SWindow::IsFocused() const
{
    if (!GetContainer())
        return FALSE;

    return GetContainer()->GetFocus() == m_swnd;
}

BOOL SWindow::OnNcHitTest(CPoint pt)
{
    CRect rcClient = GetClientRect();
    return !rcClient.PtInRect(pt);
}

IWindow *SWindow::GetIWindow(int uCode) const
{
    return GetWindow(uCode);
}

IWindow *SWindow::GetIChild(int iChild) const
{
    if (iChild == CHILDID_SELF)
        return (IWindow *)this;
    IWindow *pChild = GetIWindow(GSW_FIRSTCHILD);
    for (int i = 0; i < iChild - 1 && pChild; i++)
    {
        pChild = pChild->GetIWindow(GSW_NEXTSIBLING);
        if (!pChild)
            return NULL;
    }

    return pChild;
}

void SWindow::PaintBackground(IRenderTarget *pRT, LPRECT pRc)
{
    CRect rcDraw = GetWindowRect();
    if (pRc)
        rcDraw.IntersectRect(rcDraw, pRc);
    pRT->PushClipRect(&rcDraw, RGN_AND);

    SWindow *pTopWnd = GetRoot();
    SAutoRefPtr<IRegionS> pRgn;
    GETRENDERFACTORY->CreateRegion(&pRgn);
    pRgn->CombineRect(&rcDraw, RGN_COPY);

    pRT->ClearRect(&rcDraw, 0); //清除残留的alpha值

    SASSERT(GetContainer());
    GetContainer()->BuildWndTreeZorder();
    pTopWnd->_PaintRegion(pRT, pRgn, ZORDER_MIN, m_uZorder);

    pRT->PopClip();
}

void SWindow::PaintForeground(IRenderTarget *pRT, LPRECT pRc, SWindow *pStartFrom /*=NULL*/)
{
    CRect rcDraw = GetWindowRect();
    if (pRc)
        rcDraw.IntersectRect(rcDraw, pRc);
    SAutoRefPtr<IRegionS> pRgn;
    GETRENDERFACTORY->CreateRegion(&pRgn);
    pRgn->CombineRect(&rcDraw, RGN_COPY);
    pRT->PushClipRect(&rcDraw, RGN_AND);

    SASSERT(GetContainer());
    GetContainer()->BuildWndTreeZorder();
    if (!pStartFrom)
        pStartFrom = GetRoot();
    pStartFrom->_PaintRegion(pRT, pRgn, (UINT)m_uZorder + 1, (UINT)ZORDER_MAX);

    pRT->PopClip();
}

BOOL SWindow::FireCommand()
{
    EventCmd evt(this);
    return FireEvent(evt);
}

BOOL SWindow::FireCtxMenu(POINT pt)
{
    EventCtxMenu evt(this);
    evt.bCancel = FALSE;
    evt.pt = pt;
    FireEvent(evt);
    return evt.bCancel;
}

bool SWindow::IsCacheDirty() const
{
    return m_bCacheDirty && IsDrawToCache();
}

void SWindow::MarkCacheDirty(bool bDirty)
{
    m_bCacheDirty = bDirty;
}
//////////////////////////////////////////////////////////////////////////

HRESULT SWindow::OnAttrVisible(const SStringW &strValue, BOOL bLoading)
{
    BOOL bVisible = STRINGASBOOL(strValue);
    if (!bLoading)
        SetVisible(bVisible, TRUE);
    else
        m_bVisible = bVisible;
    return S_FALSE;
}

HRESULT SWindow::OnAttrEnable(const SStringW &strValue, BOOL bLoading)
{
    BOOL bEnable = STRINGASBOOL(strValue);
    if (bLoading)
    {
        if (bEnable)
            ModifyState(0, WndState_Disable);
        else
            ModifyState(WndState_Disable, WndState_Hover);
    }
    else
    {
        EnableWindow(bEnable, TRUE);
    }
    return S_FALSE;
}

HRESULT SWindow::OnAttrDisplay(const SStringW &strValue, BOOL bLoading)
{
    m_bDisplay = STRINGASBOOL(strValue);
    if (bLoading)
        return S_FALSE;

    RequestRelayout();
    return S_OK;
}

HRESULT SWindow::OnAttrSkin(const SStringW &strValue, BOOL bLoading)
{
    m_pBgSkin = GETSKIN(strValue, GetScale());
    if (bLoading)
        return S_FALSE;

    if ((GetLayoutParam()->IsWrapContent(Vert) || GetLayoutParam()->IsWrapContent(Horz)))
    {
        RequestRelayout();
    }
    return S_OK;
}

HRESULT SWindow::OnAttrClass(const SStringW &strValue, BOOL bLoading)
{
    SXmlNode xmlStyle = GETSTYLE(strValue);
    if (xmlStyle)
    {
        //优先处理layout属性
        SXmlAttr attrLayout = xmlStyle.attribute(L"layout");
        if (attrLayout)
        {
            SetAttribute(attrLayout.name(), attrLayout.value(), bLoading);
            MarkAttributeHandled(attrLayout, true);
        }
        for (SXmlAttr attr = xmlStyle.first_attribute(); attr; attr = attr.next_attribute())
        { //解析style中的属性
            if (_wcsicmp(attr.name(), L"class") == 0 || IsAttributeHandled(attr))
                continue; //防止class中包含有其它class属性,避免发生死循环
            SetAttribute(attr.name(), attr.value(), bLoading);
        }
        MarkAttributeHandled(attrLayout, false);
    }
    return S_FALSE;
}

HRESULT SWindow::OnAttrTrackMouseEvent(const SStringW &strValue, BOOL bLoading)
{
    GetStyle().m_bTrackMouseEvent = STRINGASBOOL(strValue);
    if (!bLoading)
    {
        if (GetContainer())
        {
            if (GetStyle().m_bTrackMouseEvent)
                GetContainer()->RegisterTrackMouseEvent(m_swnd);
            else
                GetContainer()->UnregisterTrackMouseEvent(m_swnd);
        }
    }
    return S_FALSE;
}

HRESULT SWindow::OnAttrVideoCanvas(const SStringW &strValue, BOOL bLoading)
{
    GetStyle().m_bVideoCanvas = STRINGASBOOL(strValue);
    if (!bLoading)
    {
        if (GetContainer())
        {
            if (GetStyle().m_bVideoCanvas)
                GetContainer()->RegisterVideoCanvas(m_swnd);
            else
                GetContainer()->UnregisterVideoCanvas(m_swnd);
        }
    }
    return S_FALSE;
}

void SWindow::OnSize(UINT nType, CSize size)
{
    if (IsDrawToCache())
    {
        if (!m_cachedRT)
        {
            GETRENDERFACTORY->CreateRenderTarget(&m_cachedRT, GetWindowRect().Width(), GetWindowRect().Height());
        }
        else
        {
            m_cachedRT->Resize(GetWindowRect().Size());
        }
        m_cachedRT->SetViewportOrg(-GetWindowRect().TopLeft());

        MarkCacheDirty(true);
    }

    EventSwndSize evt(this);
    evt.szWnd = size;
    FireEvent(evt);
}

void SWindow::UpdateCacheMode()
{
    if (IsDrawToCache() && !m_cachedRT)
    {
        GETRENDERFACTORY->CreateRenderTarget(&m_cachedRT, GetWindowRect().Width(), GetWindowRect().Height());
        MarkCacheDirty(true);
    }
    if (!IsDrawToCache() && m_cachedRT)
    {
        m_cachedRT = NULL;
    }
}

HRESULT SWindow::OnAttrCache(const SStringW &strValue, BOOL bLoading)
{
    m_bCacheDraw = STRINGASBOOL(strValue);

    if (!bLoading)
    {
        UpdateCacheMode();
        InvalidateRect(NULL);
    }
    return S_FALSE;
}

HRESULT SWindow::OnAttrAlpha(const SStringW &strValue, BOOL bLoading)
{
    BYTE byAlpha = _wtoi(strValue);
    SetAlpha(byAlpha);
    return bLoading ? S_FALSE : S_OK;
}

HRESULT SWindow::OnAttrID(const SStringW &strValue, BOOL bLoading)
{
    static struct SystemID
    {
        int id;
        LPCWSTR pszName;
    } systemID[] = { IDOK, L"IDOK", IDCANCEL, L"IDCANCEL", IDCLOSE, L"IDCLOSE", IDHELP, L"IDHELP", IDABORT, L"IDABORT", IDYES, L"IDYES", IDNO, L"IDNO", IDRETRY, L"IDRETRY", IDIGNORE, L"IDIGNORE" };
    if (!strValue.IsEmpty())
    {
        if (strValue.Left(2).CompareNoCase(L"ID") == 0)
        {
            for (int i = 0; i < ARRAYSIZE(systemID); i++)
            {
                if (strValue.CompareNoCase(systemID[i].pszName) == 0)
                {
                    m_nID = systemID[i].id;
                    break;
                }
            }
        }
        else
        {
            m_nID = SNcPainter::toNcBuiltinID(strValue);
            if (m_nID == 0)
                m_nID = _wtoi(strValue);
        }
    }
    IAttrStorageFactory *pAttrFac = SApplication::getSingleton().GetAttrStorageFactory();
    if (pAttrFac && !m_attrStorage)
    {
        pAttrFac->CreateAttrStorage(this, &m_attrStorage);
    }

    return S_FALSE;
}

HRESULT SWindow::OnAttrName(const SStringW &strValue, BOOL bLoading)
{
    m_strName = strValue;
    if (m_nID == 0)
    {
        m_nID = STR2ID(strValue);
    }
    IAttrStorageFactory *pAttrFac = SApplication::getSingleton().GetAttrStorageFactory();
    if (pAttrFac && !m_attrStorage)
    {
        pAttrFac->CreateAttrStorage(this, &m_attrStorage);
    }
    return S_FALSE;
}

HRESULT SWindow::OnAttrTip(const SStringW &strValue, BOOL bLoading)
{
    SetToolTipText(S_CW2T(GETSTRING(strValue)));
    return S_FALSE;
}

SStringW SWindow::GetXmlText(const SXmlNode &xmlNode)
{
    SStringW strText = xmlNode.Text();
    strText.TrimBlank();
    if (strText.IsEmpty())
    {
        strText = xmlNode.child_value();
        strText.TrimBlank();
    }
    return strText;
}

HRESULT SWindow::OnAttrText(const SStringW &strValue, BOOL bLoading)
{
    SStringW strText = GETSTRING(strValue);
    SStringT strCvt2 = S_CW2T(strText);
    if (bLoading)
        m_strText.SetText(strCvt2);
    else
        SetWindowText(strCvt2);
    return S_OK;
}

SWindow *SWindow::GetSelectedChildInGroup()
{
    SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
    while (pChild)
    {
        if (pChild->IsSiblingsAutoGroupped())
            break;
        pChild = pChild->GetWindow(GSW_NEXTSIBLING);
    }
    if (!pChild)
        return NULL;
    return pChild->GetSelectedSiblingInGroup();
}

bool SWindow::IsDrawToCache() const
{
    return m_bCacheDraw || m_isAnimating;
}

void SWindow::OnStateChanging(DWORD dwOldState, DWORD dwNewState)
{
}

void SWindow::OnStateChanged(DWORD dwOldState, DWORD dwNewState)
{
    EventSwndStateChanged evt(this);
    evt.dwOldState = dwOldState;
    evt.dwNewState = dwNewState;
    FireEvent(evt);
    accNotifyEvent(EVENT_OBJECT_STATECHANGE);
}

IScriptModule *SWindow::GetScriptModule()
{
    if (!GetContainer())
        return NULL;
    return GetContainer()->GetScriptModule();
}

HRESULT SWindow::DefAttributeProc(const SStringW &strAttribName, const SStringW &strValue, BOOL bLoading)
{
    HRESULT hr = E_FAIL;
    if (GetScriptModule())
    {
        if (m_evtSet.setEventScriptHandler(strAttribName, S_CW2A(strValue, CP_UTF8)))
        {
            hr = S_FALSE;
        }
    }
    if (m_attrStorage)
    {
        m_attrStorage->OnSetAttribute(&strAttribName, &strValue, false);
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////////
// caret functions

BOOL SWindow::CreateCaret(HBITMAP pBmp, int nWid, int nHeight)
{
    if (!GetContainer())
        return FALSE;
    if (!m_caret)
    {
        m_caret.Attach(new SCaret(GetContainer()));
        SStringW strCaret;
        EventGetCaret evt(this);
        evt.strCaret = &strCaret;
        FireEvent(&evt);
        SXmlNode xmlCaret;
        SXmlDoc xmlDoc;
        if (!strCaret.IsEmpty())
        {
            if (xmlDoc.load_buffer_inplace(strCaret.GetBuffer(), strCaret.GetLength() * sizeof(wchar_t), xml_parse_default, sizeof(wchar_t) == 2 ? enc_utf16 : enc_utf32))
                xmlCaret = xmlDoc.root().child(L"caret");
            strCaret.ReleaseBuffer();
        }
        if (!xmlCaret)
        {
            xmlCaret = GETUIDEF->GetUiDef()->GetCaretInfo();
        }
        m_caret->InitFromXml(&xmlCaret);
    }
    return m_caret->Init(pBmp, nWid, nHeight);
}

void SWindow::ShowCaret(BOOL bShow)
{
    if (!m_caret)
        return;
    if (m_caret->SetVisible(bShow, m_swnd))
    {
        CRect rcCaret = m_caret->GetRect();
        InvalidateRect(rcCaret);
    }
}

void SWindow::SetCaretPos(int x, int y)
{
    if (!m_caret)
        return;

    if (m_caret->IsVisible())
    {
        {
            CRect rcCaret = m_caret->GetRect();
            InvalidateRect(rcCaret);
        }
        m_caret->SetPosition(x, y);
        {
            CRect rcCaret = m_caret->GetRect();
            InvalidateRect(rcCaret);
        }
    }
    else
    {
        m_caret->SetPosition(x, y);
    }
}

BOOL SWindow::IsContainPoint(POINT pt, BOOL bClientOnly) const
{
    BOOL bRet = FALSE;
    CRect rc = bClientOnly ? GetClientRect() : GetWindowRect();
    bRet = rc.PtInRect(pt);
    if (m_clipRgn)
    {
        CPoint ptTmp = pt;
        ptTmp -= GetWindowRect().TopLeft();
        bRet = m_clipRgn->PtInRegion(ptTmp);
    }
    return bRet;
}

void SWindow::SetWindowRgn(IRegionS *pRgn, BOOL bRedraw /*=TRUE*/)
{
    m_clipRgn = NULL;
    if (pRgn)
    {
        GETRENDERFACTORY->CreateRegion(&m_clipRgn);
        m_clipRgn->CombineRgn(pRgn, RGN_COPY);
    }
    if (bRedraw)
        InvalidateRect(NULL);
}

IRegionS *SWindow::GetWindowRgn() const
{
    return m_clipRgn;
}

void SWindow::SetWindowPath(IPathS *pPath, BOOL bRedraw /*=TRUE*/)
{
    m_clipPath = pPath;
    if (bRedraw)
        InvalidateRect(NULL);
}

IPathS *SWindow::GetWindowPath() const
{
    return m_clipPath;
}

void SWindow::DoColorize(COLORREF cr)
{
    SDispatchMessage(UM_SETCOLORIZE, cr, 0);
}

LRESULT SWindow::OnSetColorize(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    OnColorize((COLORREF)wParam);
    return 0;
}

void SWindow::OnColorize(COLORREF cr)
{
    if (m_crColorize == cr)
        return;
    m_crColorize = cr;
    if (m_pNcSkin)
        m_pNcSkin->OnColorize(cr);
    if (m_pBgSkin)
        m_pBgSkin->OnColorize(cr);
    InvalidateRect(NULL);
}

COLORREF SWindow::GetColorizeColor() const
{
    return m_crColorize;
}

LRESULT SWindow::OnUpdateFont(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    OnRebuildFont();
    return 0;
}

HRESULT SWindow::OnAttrLayout(const SStringW &strValue, BOOL bLoading)
{
    ILayout *pLayout = (ILayout *)SApplication::getSingleton().CreateObject(strValue, Layout);
    if (pLayout == NULL)
        return E_INVALIDARG;

    m_pLayout = pLayout;
    pLayout->Release();
    return S_OK;
}

HRESULT SWindow::AfterAttribute(const SStringW &strAttribName, const SStringW &strValue, BOOL bLoading, HRESULT hr)
{
    if (m_attrStorage)
    {
        m_attrStorage->OnSetAttribute(&strAttribName, &strValue, true);
    }
    if ((hr & 0x0000ffff) == S_OK && !bLoading)
    {
        HRESULT hFlag = hr & 0xFFFF0000;
        if ((hFlag & HRET_FLAG_LAYOUT_PARAM) || (hFlag & HRET_FLAG_LAYOUT))
        { //修改了窗口的布局属性,请求父窗口重新布局
            if (GetParent())
            {
                GetParent()->RequestRelayout();
            }
            else
            {
                InvalidateRect(NULL);
            }
        }
        else if ((hr & 0x0000ffff) == S_OK)
        {
            InvalidateRect(NULL);
        }
    }
    return hr;
}

BOOL SWindow::GetAttribute(LPCWSTR pszAttr, IStringW *strValue) const
{
    if (m_attrStorage)
    {
        SStringW strAttr(pszAttr);
        return m_attrStorage->OnGetAttribute(&strAttr, strValue);
    }
    else
    {
        return __baseCls::GetAttribute(pszAttr, strValue);
    }
}

HRESULT SWindow::OnLanguageChanged()
{
    m_strText.TranslateText();
    m_strToolTipText.TranslateText();
    return GetLayoutParam()->IsWrapContent(Any) ? S_OK : S_FALSE;
}

SStringT SWindow::GetToolTipText()
{
    return m_strToolTipText.GetText(FALSE);
}

void SWindow::SetToolTipText(LPCTSTR pszText)
{
    m_strToolTipText.SetText(pszText, false);
    if (!GetContainer())
        return;
    if (GetContainer()->GetHover() == m_swnd)
    { //请求更新显示的tip
        GetContainer()->UpdateTooltip();
    }
}

void SWindow::SetToolTipTextU8(LPCSTR pszText)
{
    SStringT str = S_CA2T(pszText, CP_UTF8);
    return SetToolTipText(str);
}

LPCWSTR SWindow::GetTrCtx() const
{
    if (GetContainer() && m_strTrCtx.IsEmpty())
        return GetContainer()->GetTranslatorContext();
    else
        return m_strTrCtx;
}

int SWindow::GetScale() const
{
    return GetStyle().GetScale();
}

void SWindow::OnScaleChanged(int scale)
{
    GetStyle().SetScale(scale);
    GetScaleSkin(m_pNcSkin, scale);
    GetScaleSkin(m_pBgSkin, scale);

    //标记布局脏
    m_layoutDirty = dirty_self;

    if (m_animation && m_animation->hasEnded())
    {
        //动画结束状态下，重新刷新动画结束位置
        long tmDuration = m_animation->getDuration();
        long tmOffset = m_animation->getStartOffset();
        m_animation->setStartTime(STime::GetCurrentTimeMs() - tmDuration - tmOffset);
        m_animationHandler.OnNextFrame();
    }
}

void SWindow::GetScaleSkin(SAutoRefPtr<ISkinObj> &pSkin, int nScale)
{
    if (!pSkin)
        return;
    SStringW strName = pSkin->GetName();
    if (!strName.IsEmpty())
    {
        ISkinObj *pNewSkin = GETSKIN(strName, nScale);
        if (pNewSkin)
            pSkin = pNewSkin;
    }
}

void SWindow::OnRebuildFont()
{
    m_style.UpdateFont();
}

#ifdef _WIN32
IAccessible *SWindow::GetAccessible()
{
#ifdef SOUI_ENABLE_ACC
    if (!m_pAcc)
        m_pAcc.Attach(SApplication::getSingleton().CreateAccessible(this));
    return m_pAcc;
#else
    return NULL;
#endif
}
#endif

IAccProxy *SWindow::GetAccProxy()
{
#ifdef SOUI_ENABLE_ACC
    if (!m_pAccProxy)
    {
        m_pAccProxy.Attach(SApplication::getSingleton().CreateAccProxy(this));
    }
    return m_pAccProxy;
#else
    return NULL;
#endif
}

void SWindow::accNotifyEvent(DWORD dwEvt)
{
#ifdef SOUI_ENABLE_ACC
    if (GetContainer())
        NotifyWinEvent(dwEvt, GetContainer()->GetHostHwnd(), GetSwnd(), CHILDID_SELF);
#endif
}

BOOL SWindow::SetLayoutParam(ILayoutParam *pLayoutParam)
{
    SWindow *pParent = GetParent();
    if (!pParent->GetLayout()->IsParamAcceptable(pLayoutParam))
        return FALSE;
    m_pLayoutParam = pLayoutParam;
    return TRUE;
}

void SWindow::OnAnimationStart(THIS_ IAnimation *pAni)
{
    EventSwndAnimationStart evt(this);
    evt.pAni = pAni;
    FireEvent(&evt);

    m_isAnimating = true;
    m_animationHandler.OnAnimationStart();
    UpdateCacheMode();
}

void SWindow::OnAnimationStop(THIS_ IAnimation *pAni)
{
    if (GetContainer())
        GetContainer()->UnregisterTimelineHandler(&m_animationHandler);
    m_animationHandler.OnAnimationStop();
    m_isAnimating = false;
    UpdateCacheMode();
    EventSwndAnimationStop evt(this);
    evt.pAni = pAni;
    FireEvent(&evt);
}

void SWindow::OnAnimationRepeat(THIS_ IAnimation *pAni)
{
    EventSwndAnimationRepeat evt(this);
    evt.pAni = pAni;
    FireEvent(&evt);
}

void SWindow::OnAnimationInvalidate(IAnimation *pAni, bool bErase)
{
    InvalidateRect(NULL);
}

COLORREF SWindow::GetBkgndColor() const
{
    return GetStyle().m_crBg;
}

void SWindow::OnBeforeRemoveChild(SWindow *pChild)
{
}

void SWindow::OnAfterInsertChild(SWindow *pChild)
{
}

void SWindow::OnBeforeInsertChild(SWindow *pChild)
{
}

void SWindow::OnAfterRemoveChild(SWindow *pChild)
{
}

void SWindow::OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer)
{
    if (pOldContainer)
    {
        if (IsFocused())
            pOldContainer->OnSetSwndFocus(0);
        if (GetStyle().m_bTrackMouseEvent)
            pOldContainer->UnregisterTrackMouseEvent(m_swnd);
        if (GetCapture() == m_swnd)
            ReleaseCapture();
        pOldContainer->UnregisterTimelineHandler(&m_animationHandler);
        if (GetStyle().m_bVideoCanvas)
            pOldContainer->UnregisterVideoCanvas(m_swnd);
    }
    m_pContainer = pNewContainer;
    if (pNewContainer)
    {
        if (GetStyle().m_bTrackMouseEvent)
            pNewContainer->RegisterTrackMouseEvent(m_swnd);
        if (GetStyle().m_bVideoCanvas)
            pNewContainer->RegisterVideoCanvas(m_swnd);
    }
}

BOOL SWindow::IsFloat() const
{
    return m_bFloat;
}

BOOL SWindow::IsDisplay() const
{
    return m_bDisplay;
}

BOOL SWindow::IsSiblingsAutoGroupped() const
{
    return FALSE;
}

BOOL SWindow::IsClipClient() const
{
    return m_bClipClient;
}

BOOL SWindow::IsLayoutDirty() const
{
    return m_layoutDirty != dirty_clean;
}

SWindow *SWindow::GetWindow(int uCode) const
{
    SWindow *pRet = NULL;
    switch (uCode)
    {
    case GSW_FIRSTCHILD:
        pRet = m_pFirstChild;
        break;
    case GSW_LASTCHILD:
        pRet = m_pLastChild;
        break;
    case GSW_PREVSIBLING:
        pRet = m_pPrevSibling;
        break;
    case GSW_NEXTSIBLING:
        pRet = m_pNextSibling;
        break;
    case GSW_OWNER:
        pRet = m_pOwner;
        break;
    case GSW_PARENT:
        pRet = m_pParent;
        break;
    }
    return pRet;
}

SWindow *SWindow::GetChild(int iChild) const
{
    return (SWindow *)GetIChild(iChild);
}

SWindow *SWindow::GetParent() const
{
    return m_pParent;
}

SWindow *SWindow::GetRoot() const
{
    SWindow *pParent = (SWindow *)this;
    while (pParent->GetParent())
        pParent = pParent->GetParent();
    return pParent;
}

SWindow *SWindow::GetNextLayoutChild(const SWindow *pCurChild) const
{
    SWindow *pRet = NULL;
    if (pCurChild == NULL)
    {
        pRet = GetWindow(GSW_FIRSTCHILD);
    }
    else
    {
        pRet = pCurChild->GetWindow(GSW_NEXTSIBLING);
    }

    if (pRet && (pRet->IsFloat() || (!pRet->IsDisplay() && !pRet->IsVisible(FALSE))))
        return GetNextLayoutChild(pRet);
    return pRet;
}

BOOL SWindow::IsDescendant(THIS_ const IWindow *pWnd) const
{
    if (!pWnd)
        return FALSE;
    const IWindow *pParent = GetIParent();
    while (pParent)
    {
        if (pParent == pWnd)
            return TRUE;
        pParent = pParent->GetIParent();
    }
    return FALSE;
}

BOOL SWindow::AdjustIZOrder(THIS_ IWindow *pInsertAfter)
{
    return AdjustZOrder((SWindow *)pInsertAfter);
}

void SWindow::InsertIChild(THIS_ IWindow *pNewChild, IWindow *pInsertAfter /*=ICWND_LAST*/)
{
    return InsertChild((SWindow *)pNewChild, (SWindow *)pInsertAfter);
}

BOOL SWindow::RemoveIChild(THIS_ IWindow *pChild)
{
    return RemoveChild((SWindow *)pChild);
}

BOOL SWindow::DestroyIChild(THIS_ IWindow *pChild)
{
    return DestroyChild((SWindow *)pChild);
}

void SWindow::SetIOwner(THIS_ IWindow *pOwner)
{
    SetOwner((SWindow *)pOwner);
}

IWindow *SWindow::GetIOwner(THIS) const
{
    return GetOwner();
}

BOOL SWindow::SubscribeEvent(THIS_ DWORD evtId, const IEvtSlot *pSlot)
{
    return GetEventSet()->subscribeEvent(evtId, pSlot);
}

BOOL SWindow::UnsubscribeEvent(THIS_ DWORD evtId, const IEvtSlot *pSlot)
{
    return GetEventSet()->unsubscribeEvent(evtId, pSlot);
}

HRESULT SWindow::QueryInterface(THIS_ REFGUID id, IObjRef **ppRet)
{
    return E_NOINTERFACE;
}

void SWindow::OnCommitSurface(IRenderTarget *pRtDest, LPCRECT pRcDest, IRenderTarget *pRtSrc, LPCRECT pRcSrc, BYTE alpha)
{
    pRtDest->AlphaBlend(pRcDest, pRtSrc, pRcSrc, alpha);
}

IWindow *SWindow::GetISelectedSiblingInGroup(THIS)
{
    return GetSelectedSiblingInGroup();
}

IWindow *SWindow::GetISelectedChildInGroup(THIS)
{
    return GetSelectedChildInGroup();
}

BOOL SWindow::SwndProc(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lResult)
{
    SASSERT(lResult);
    BOOL bOldMsgHandle = IsMsgHandled(); //备分上一个消息的处理状态
    BOOL bRet = FALSE;
    if (m_funSwndProc)
    {
        bRet = m_funSwndProc(this, uMsg, wParam, lParam, lResult);
    }
    if (!bRet)
    {
        SetMsgHandled(FALSE);
        bRet = ProcessSwndMessage(uMsg, wParam, lParam, *lResult);
    }
    SetMsgHandled(bOldMsgHandle); //恢复上一个消息的处理状态

    return bRet;
}

void SWindow::SetSwndProc(FunSwndProc swndProc)
{
    m_funSwndProc = swndProc;
}

HWND SWindow::GetHostHwnd(THIS)
{
    return GetContainer()->GetHostHwnd();
}

ITimelineHandlersMgr *SWindow::GetTimelineHandlersMgr(THIS)
{
    return GetContainer();
}

BOOL SWindow::AddEvent(THIS_ DWORD dwEventID, LPCWSTR pszEventHandlerName)
{
    return m_evtSet.addEvent(dwEventID, pszEventHandlerName);
}

BOOL SWindow::RemoveEvent(THIS_ DWORD dwEventID)
{
    return m_evtSet.removeEvent(dwEventID);
}

void SWindow::GetVisibleRect(LPRECT prc) const
{
    SASSERT(prc);
    CRect rcWnd;
    *prc = rcWnd;
    if (!IsVisible(TRUE))
        return;
    rcWnd = GetWindowRect();
    SWindow *pParent = GetParent();
    while (pParent)
    {
        CRect rcParent = pParent->GetClientRect();
        rcWnd = rcWnd & rcParent;
        if (rcWnd.IsRectEmpty())
            break;
        pParent = pParent->GetParent();
    }
    *prc = rcWnd;
}

BOOL SWindow::IsVideoCanvas(CTHIS) const
{
    return GetStyle().m_bVideoCanvas;
}

BOOL SWindow::RegisterDragDrop(THIS_ IDropTarget *pDragTarget)
{
    return GetContainer()->RegisterDragDrop(m_swnd, pDragTarget);
}

BOOL SWindow::UnregisterDragDrop(THIS)
{
    return GetContainer()->UnregisterDragDrop(m_swnd);
}

void SWindow::OnAnimationPauseChange(THIS_ IAnimation *animation, BOOL bPaused)
{
    if (bPaused)
        GetContainer()->UnregisterTimelineHandler(&m_animationHandler);
    else
        GetContainer()->RegisterTimelineHandler(&m_animationHandler);
}

//////////////////////////////////////////////////////////////////////////
static SWindow *ICWND_NONE = (SWindow *)-2;
SWindow::SAnimationHandler::SAnimationHandler(SWindow *pOwner)
    : m_pOwner(pOwner)
    , m_bFillAfter(false)
    , m_pPrevSiblingBackup(ICWND_NONE)
{
}

void SWindow::SAnimationHandler::OnAnimationStart()
{
    IAnimation *pAni = m_pOwner->GetAnimation();
    SASSERT(pAni);
    if (pAni->getZAdjustment() != ZORDER_NORMAL)
    {
        m_pPrevSiblingBackup = m_pOwner->GetWindow(GSW_PREVSIBLING);
        if (m_pPrevSiblingBackup == NULL)
            m_pPrevSiblingBackup = ICWND_FIRST;

        if (pAni->getZAdjustment() == ZORDER_TOP)
            m_pOwner->AdjustZOrder(ICWND_LAST);
        else
            m_pOwner->AdjustZOrder(ICWND_FIRST);
    }
    else
    {
        m_pPrevSiblingBackup = ICWND_NONE;
    }
    m_pOwner->GetEventSet()->subscribeEvent(EventSwndSize::EventID, Subscriber(&SWindow::SAnimationHandler::OnOwnerResize, this));
    if (m_pOwner->GetParent())
    {
        m_pOwner->GetParent()->GetEventSet()->subscribeEvent(EventSwndSize::EventID, Subscriber(&SWindow::SAnimationHandler::OnOwnerResize, this));
    }
    OnOwnerResize(NULL);
}

void SWindow::SAnimationHandler::OnAnimationStop()
{
    m_pOwner->GetEventSet()->unsubscribeEvent(EventSwndSize::EventID, Subscriber(&SWindow::SAnimationHandler::OnOwnerResize, this));
    if (m_pOwner->GetParent())
    {
        m_pOwner->GetParent()->GetEventSet()->unsubscribeEvent(EventSwndSize::EventID, Subscriber(&SWindow::SAnimationHandler::OnOwnerResize, this));
    }
    if (m_pPrevSiblingBackup != ICWND_NONE)
    {
        m_pOwner->AdjustZOrder(m_pPrevSiblingBackup);
        m_pPrevSiblingBackup = ICWND_NONE;
    }
}

const STransformation &SWindow::SAnimationHandler::GetTransformation() const
{
    return m_transform;
}

void SWindow::SAnimationHandler::OnNextFrame()
{
    IAnimation *pAni = m_pOwner->GetAnimation();
    SASSERT(pAni);
    pAni->AddRef();
    m_pOwner->AddRef();
    uint64_t tm = pAni->getStartTime();
    if (tm > 0)
    {
        m_pOwner->OnAnimationInvalidate(pAni, true);
        BOOL bMore = pAni->getTransformation(STime::GetCurrentTimeMs(), &m_transform);
        m_pOwner->OnAnimationInvalidate(pAni, false);
        if (!bMore)
        { // animation stopped.
            if (pAni->isFillEnabled() && pAni->getFillAfter())
            {
                m_bFillAfter = true;
            }
            else
            {
                m_bFillAfter = false;
            }
        }
    }
    m_pOwner->Release();
    pAni->Release();
}

BOOL SWindow::SAnimationHandler::OnOwnerResize(IEvtArgs *e)
{
    CSize szOwner = m_pOwner->GetWindowRect().Size();
    CSize szParent = szOwner;
    SWindow *p = m_pOwner->GetParent();
    if (p)
    {
        szParent = p->GetWindowRect().Size();
    }
    m_pOwner->GetAnimation()->initialize(szOwner.cx, szOwner.cy, szParent.cx, szParent.cy, m_pOwner->GetScale());
    return true;
}

bool SWindow::SAnimationHandler::getFillAfter() const
{
    return m_bFillAfter;
}

SNSEND