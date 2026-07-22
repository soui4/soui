/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SModalViewSession.h
 * @brief      Modal View Session for SOUI
 * @version    v3.0
 * @author     SOUI group
 * @date       2024/01/01
 *
 * Description: Modal view session support, simplified by absorbing the former
 *              SModalViewSession bookkeeping object directly into SModalRoot:
 *
 *              - SModalRoot is an SWindow subclass that fills the entire
 *                SRootWindow client area and intercepts all mouse/keyboard
 *                messages via normal window-tree hit-testing. Clicking on the
 *                background of SModalRoot (outside any child) dismisses the
 *                active modal session with IDCANCEL.
 *              - SModalView is an SWindow subclass, used as the visible
 *                "dialog" child inside SModalRoot. Supports enter/exit animations.
 *              - Each SModalRoot itself carries a unique session id. When the
 *                modal session ends, it fires an EventExitModalView event that
 *                subscribers can listen to. SHostWnd manages the modal stack
 *                and uses IModalViewExitCallback for internal cleanup.
 */

#ifndef __SMODALVIEWSESSION__H__
#define __SMODALVIEWSESSION__H__

#include <core/SWnd.h>

SNSBEGIN

/**
 * @brief Modal View Session ID type.
 */
typedef uint32_t ModalViewSessionID;

class SModalView;
class SModalRoot;

/**
 * @brief Interface for modal view exit callback.
 *
 * This interface is used by SHostWnd to be notified when a modal view session
 * has completed its exit animation and is ready to be removed from the view tree.
 */
struct IModalViewExitCallback : public IObjRef
{
    /**
     * @brief Called when the modal view session has finished (exit animation completed).
     * @param pModalRoot The modal root window that is exiting.
     */
    virtual void OnModalViewExit(SModalRoot* pModalRoot) = 0;
};


/**
 * @brief Full-screen container for a single modal view session.
 *
 * SModalRoot fills its parent (SRootWindow) entirely, so that mouse/keyboard
 * hit-testing always prefers the top-most SModalRoot over any non-modal children.
 * The user is expected to put a single SModalView inside this window.
 *
 * When the modal session ends, SModalRoot fires an EventExitModalView event
 * (containing the exit code) that subscribers can listen to via SubscribeEvent.
 * Clicking on the area outside any child window (i.e. on the dimmed background)
 * dismisses the active session with exitCode = IDCANCEL.
 *
 * SHostWnd manages the modal stack and uses IModalViewExitCallback internally
 * to handle cleanup after the exit animation completes.
 */
class SOUI_EXP SModalRoot : public SWindow
{
    DEF_SOBJECT(SWindow, L"modalroot")
    friend class SHostWnd;
	friend class SModalView;
public:
    SModalRoot();
    ~SModalRoot();

public:
    /**
     * @brief Returns the session ID bound to this modal root.
     */
    ModalViewSessionID GetSessionID() const { return m_sessionID; }

    /**
     * @brief Returns the first SModalView child, or NULL if none found.
     *
     * Also returns any SModalView-compatible SWindow that has name
     * L"modalview" if a direct SModalView subclass is not used.
     */
    SModalView* GetModalView() const;

    /**
     * @brief Initiates the modal view session exit process.
     *
     * Starts the exit animation (if defined) and schedules cleanup.
     * This method is called by SHostWnd::endModalViewSession.
     *
     * @param pCb Optional callback invoked when exit animation completes.
     * @param exitCode The exit code to pass to subscribers of EventExitModalView.
     */
    void endModalViewSession(IModalViewExitCallback * pCb,int exitCode);
public:
    SOUI_ATTRS_BEGIN()
        ATTR_BOOL(L"quitOnClick", m_bQuitOnClick, FALSE)
    SOUI_ATTRS_END()

    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_KEYDOWN(OnKeyDown)
    SOUI_MSG_MAP_END()
protected:
    /**
     * @brief Assigns a session id (called exclusively by SHostWnd when the
     *        root is pushed onto the modal stack).
     */
    void AssignSessionID();

    /**
     * @brief Fires the EventExitModalView event with the given exit code.
     *
     * This notifies all subscribers that the modal session is ending.
     *
     * @param exitCode The exit code to include in the event.
     */
    void FireExitCallback(int exitCode);

    /**
     * @brief Handles EventCmd events (used when quitOnClick is enabled).
     */
    BOOL OnEventCmd(IEvtArgs *e);

    /**
     * @brief Called when the exit animation has finished.
     *
     * Invokes the exit callback (if set) and completes the session cleanup.
     */
    void OnFinish();
protected:
    ModalViewSessionID           m_sessionID;        ///< Session ID bound to this root; 0 if unattached.
    BOOL                         m_bQuitOnClick;     ///< If TRUE, clicking background dismisses modal view.
    SAutoRefPtr<IModalViewExitCallback> m_exitCallback; ///< Callback invoked when exit animation completes.
    int                          m_exitCode;         ///< Exit code to pass to EventExitModalView subscribers.
    static ModalViewSessionID    s_sessionIDCounter; ///< Global counter for generating unique session IDs.
};


/**
 * @brief Visible dialog area for a single modal view session.
 *
 * Supports `enterAnimation` and `exitAnimation` attributes exactly like
 * SRootWindow: both point to a named SOUI animation (referenced via the
 * `anim:` prefix in XML). The enter animation is started after the modal
 * view has been added to the root tree and layouted; the exit animation is
 * started when the owning modal session ends; the modal view delays actual
 * destruction until `exitAnimation` completes so the user can observe the
 * fade/scale-out.
 */
class SOUI_EXP SModalView : public SWindow
{
    DEF_SOBJECT(SWindow, L"modalview")
    friend class SHostWnd;

public:
    SModalView();
    ~SModalView();

public:
    /**
     * @brief Requests the enter animation to be played.
     *
     * @return TRUE if an enter animation was found and started, FALSE otherwise.
     */
    void PlayEnterAnimation();

    /**
     * @brief Requests the exit animation to be played.
     *
     * When the exit animation completes (or immediately if no exit animation
     * is defined), the modal root's OnFinish method is called to complete
     * the session cleanup.
     */
    void PlayExitAnimation();

protected:
    STDMETHOD_(void, OnAnimationStop)(THIS_ IAnimation *pAni) OVERRIDE;

    /**
     * @brief Returns the parent SModalRoot that contains this view.
     * @return Pointer to the parent SModalRoot, or NULL if not found.
     */
    SModalRoot* GetModalRoot() const;
public:
    SOUI_ATTRS_BEGIN()
        ATTR_ANIMATION(L"enterAnimation", m_aniEnter, FALSE)
        ATTR_ANIMATION(L"exitAnimation",  m_aniExit,  FALSE)
    SOUI_ATTRS_END()

protected:
    SAutoRefPtr<IAnimation> m_aniEnter; ///< Show/enter animation.
    SAutoRefPtr<IAnimation> m_aniExit;  ///< Hide/exit animation.
};

SNSEND

#endif // __SMODALVIEWSESSION__H__
