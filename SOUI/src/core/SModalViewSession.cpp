/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SModalViewSession.cpp
 * @brief      Modal View Session v3 Implementation
 * @version    v3.0
 * @author     SOUI group
 * @date       2024/01/01
 *
 * Description: Implementation of SModalRoot and SModalView. Uses SOUI's event
 *              system (EventExitModalView) for notification and
 *              IModalViewExitCallback for internal cleanup.
 */

#include <souistd.h>
#include <core/SModalViewSession.h>
#include <core/SRootWindow.h>
#include <core/SHostWnd.h>
#include <layout/SAnchorLayout.h>

SNSBEGIN

//////////////////////////////////////////////////////////////////////////
// SModalRoot
//////////////////////////////////////////////////////////////////////////

ModalViewSessionID SModalRoot::s_sessionIDCounter = 0;

SModalRoot::SModalRoot()
    : m_sessionID(0)
    , m_bQuitOnClick(TRUE)
    , m_exitCode(-1)
{
    GetEventSet()->addEvent(EVENTID(EventExitModalView));
    MemberFunctionSlot<SModalRoot, IEvtArgs> slot = Subscriber(&SModalRoot::OnEventCmd,this);
    SubscribeEvent(EventCmd::EventID,&slot);
}

SModalRoot::~SModalRoot()
{
}

BOOL SModalRoot::OnEventCmd(IEvtArgs *e){
    if(m_bQuitOnClick){
        // If the modal view is clicked, quit the modal session.
        SRootWindow *pRoot = sobj_cast<SRootWindow>(GetRoot());
        pRoot->GetHostWnd()->EndModalViewSession(m_sessionID,IDCANCEL);
    }
    return TRUE;
}
void SModalRoot::OnFinish()
{
    if(m_exitCallback)
    {
        AddRef();
        FireExitCallback(m_exitCode);
        m_exitCallback->OnModalViewExit(this);
        Release();
    }
}

SModalView*SModalRoot::GetModalView() const
{
    return sobj_cast<SModalView>(GetWindow(GSW_FIRSTCHILD));
}

void SModalRoot::EndModalViewSession(IModalViewExitCallback* pCb, int exitCode)
{
    m_exitCallback = pCb;
    m_exitCode = exitCode;
    GetModalView()->PlayExitAnimation();
}

void SModalRoot::AssignSessionID()
{
    SASSERT(m_sessionID == 0);
    m_sessionID = ++s_sessionIDCounter;
}

void SModalRoot::FireExitCallback(int exitCode)
{
    EventExitModalView evt(this);
    evt.exitCode = exitCode;
    FireEvent(&evt);
}

void SModalRoot::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags){
    if(nChar == VK_ESCAPE){
        // If the modal view is clicked, quit the modal session.
        SRootWindow *pRoot = sobj_cast<SRootWindow>(GetRoot());
        pRoot->GetHostWnd()->EndModalViewSession(m_sessionID,IDCANCEL);
    }else{
        SetMsgHandled(FALSE);
    }
}
//////////////////////////////////////////////////////////////////////////
// SModalView
//////////////////////////////////////////////////////////////////////////

SModalView::SModalView()
{
}

SModalView::~SModalView()
{
}

void SModalView::PlayEnterAnimation()
{
    if (!m_aniEnter)
        return;
    m_aniEnter->SetID(Ani_win_enter);
    StartAnimation(m_aniEnter,TRUE);
}

void SModalView::PlayExitAnimation()
{
    ClearAnimation(); // stop any running animation first
    if (!m_aniExit)
    {
        GetModalRoot()->OnFinish();
        return;
    }
    m_aniExit->SetID(Ani_win_exit);
    StartAnimation(m_aniExit);
}

SModalRoot* SModalView::GetModalRoot() const
{
    return sobj_cast<SModalRoot>(GetParent());
}

void SModalView::OnAnimationStop(IAnimation *pAni)
{
    __baseCls::OnAnimationStop(pAni);
    if (pAni->GetID()==Ani_win_exit)
    {
        GetModalRoot()->OnFinish();
    }
}

SWND SModalView::SwndFromPoint(CPoint& pt, BOOL bIncludeMsgTransparent) const
{
    if (m_isAnimating)
        return m_swnd;
    return __baseCls::SwndFromPoint(pt, bIncludeMsgTransparent);
}

SNSEND
