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

///////////////////////////////////////////////////////////////////////
/** SModalRoot */
///////////////////////////////////////////////////////////////////////

ModalViewSessionID SModalRoot::s_sessionIDCounter = 0;

SModalRoot::SModalRoot()
    : m_sessionID(0)
    , m_bQuitOnClick(TRUE)
    , m_bQuitOnEsc(TRUE)
    , m_exitCode(-1)
    , m_bFinished(FALSE)
{
    GetEventSet()->addEvent(EVENTID(EventExitModalView));
    MemberFunctionSlot<SModalRoot, IEvtArgs> slot = Subscriber(&SModalRoot::OnEventCmd, this);
    SubscribeEvent(EventCmd::EventID, &slot);
}

SModalRoot::~SModalRoot()
{
}

BOOL SModalRoot::OnEventCmd(IEvtArgs *e)
{
    if (m_bQuitOnClick)
    {
        // If the modal view is clicked, quit the modal session.
        SRootWindow *pRoot = sobj_cast<SRootWindow>(GetRoot());
        if (pRoot && pRoot->GetHostWnd())
            pRoot->GetHostWnd()->EndModalViewSession(m_sessionID, IDCANCEL);
    }
    return TRUE;
}
void SModalRoot::OnFinish()
{
    // A modal root that is already being destroyed (the host is tearing the whole
    // window tree down) must not try to remove itself from that tree: the parent's
    // DestroyAllChildren() already owns the teardown, and calling RemoveChild()
    // there would release this window a second time.
    if (m_bFinished || m_isDestroying)
        return;
    m_bFinished = TRUE;

    if (!m_exitCallback)
        return; // nothing to report (session started without a host callback)

    // The notification can destroy this modal root: SHostWnd removes it from the
    // window tree and drops the session reference inside OnModalViewExit(), which
    // is what finally releases the window.  Hold a reference of our own so `this`
    // stays valid until the callback has returned; it is released at scope exit,
    // and nothing touches `this` afterwards.
    SAutoRefPtr<SModalRoot> pThis(this);
    FireExitCallback(m_exitCode);
    m_exitCallback->OnModalViewExit(this);
}

SModalView *SModalRoot::GetModalView() const
{
    return sobj_cast<SModalView>(GetWindow(GSW_FIRSTCHILD));
}

void SModalRoot::EndModalViewSession(IModalViewExitCallback *pCb, int exitCode, BOOL bNoAnimation)
{
    if (m_bFinished)
        return;

    // The finish path can run synchronously (forced close / no exit animation) and
    // the host drops the session reference inside the callback, which may free this
    // window; keep an own reference so `this` stays valid for the whole call.
    SAutoRefPtr<SModalRoot> pThis(this);

    SModalView *pView = GetModalView();
    if (pView)
    {
        if (pView->GetAniState() == SModalView::AniState_Enter)
        { // Still opening: finish the enter animation first so the exit animation
          // does not have to start on top of it.
            pView->FinishAnimation();
        }
        else if (pView->GetAniState() == SModalView::AniState_Exit)
        { // Already closing: an ordinary request (a repeated ESC / backdrop click)
          // must not schedule the cleanup twice, while a forced one ends the run of
          // the animation now instead of waiting for its remaining frames.
            if (!bNoAnimation)
                return;
            if (pCb && !m_exitCallback)
                m_exitCallback = pCb;
            m_exitCode = exitCode;
            pView->FinishAnimation(TRUE);
            return;
        }
    }

    m_exitCode = exitCode;
    if (pCb)
        m_exitCallback = pCb;

    if (!bNoAnimation && pView && pView->HasExitAnimation())
        pView->PlayExitAnimation();
    else // Forced close, or nothing to animate
        OnFinish();
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

void SModalRoot::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_ESCAPE && m_bQuitOnEsc)
    {
        // If the modal view is clicked, quit the modal session.
        SRootWindow *pRoot = sobj_cast<SRootWindow>(GetRoot());
        if (pRoot && pRoot->GetHostWnd())
            pRoot->GetHostWnd()->EndModalViewSession(m_sessionID, IDCANCEL);
    }
    else
    {
        SetMsgHandled(FALSE);
    }
}
///////////////////////////////////////////////////////////////////////
/** SModalView */
///////////////////////////////////////////////////////////////////////

SModalView::SModalView()
    : m_aniState(AniState_None)
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
    // Set the state after StartAnimation(): it cancels an animation that is still
    // running, and that cancel reports its end synchronously, which resets the state
    // to None - assigning before the call would be overwritten by it.
    StartAnimation(m_aniEnter, TRUE);
    m_aniState = AniState_Enter;
}

void SModalView::PlayExitAnimation()
{
    SModalRoot *pRoot = GetModalRoot();
    if (!pRoot)
        return;
    if (m_aniState == AniState_Enter)
        FinishAnimation(); // one animation at a time
    if (!m_aniExit)
    {
        pRoot->OnFinish();
        return;
    }
    m_aniExit->SetID(Ani_win_exit);
    StartAnimation(m_aniExit); // same reason as above: assign after it
    m_aniState = AniState_Exit;
}

void SModalView::FinishAnimation(BOOL bFinishSession)
{
    if (m_aniState == AniState_None)
    {
        if (bFinishSession)
        { // Nothing to cancel, but the caller still wants the session to end.
            SModalRoot *pRoot = GetModalRoot();
            if (pRoot)
                pRoot->OnFinish();
        }
        return;
    }
    m_aniState = AniState_None;
    // ClearAnimation() cancels the running animation, and cancel() reports its end
    // synchronously, which finishes (and may destroy) this view through
    // SModalRoot::OnFinish().  Hold a reference of our own so the rest of
    // ClearAnimation() works on a live object.
    SAutoRefPtr<SModalView> pThis(this);
    ClearAnimation();
    if (bFinishSession)
    { // cancel() only notifies while the animation has actually started; cover the
      // case where it never got its first frame.
        SModalRoot *pRoot = GetModalRoot();
        if (pRoot)
            pRoot->OnFinish();
    }
}

SModalRoot *SModalView::GetModalRoot() const
{
    return sobj_cast<SModalRoot>(GetParent());
}

void SModalView::OnAnimationStop(IAnimation *pAni)
{
    __baseCls::OnAnimationStop(pAni);
    // The animation is over, whatever kind it was: the view must not keep claiming
    // that it is still opening/closing (EndModalViewSession and the host rely on it).
    m_aniState = AniState_None;
    if (pAni->GetID() == Ani_win_exit)
    { // The session is complete once the exit animation has played out.
        SModalRoot *pRoot = GetModalRoot();
        if (pRoot)
            pRoot->OnFinish();
    }
}

SWND SModalView::SwndFromPoint(CPoint &pt, BOOL bIncludeMsgTransparent) const
{
    if (m_isAnimating)
        return m_swnd;
    return __baseCls::SwndFromPoint(pt, bIncludeMsgTransparent);
}

SNSEND
