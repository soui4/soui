/**
 * @file       SStackView.h
 * @brief      Definition of the SStackView and related classes.
 * @version    v1.0
 * @author     soui
 * @date       2014-07-08
 *
 * @copyright  Copyright (C) 2014-2050 SOUI团队
 *             All rights reserved.
 */

#ifndef __SSTACKVIEW__H__
#define __SSTACKVIEW__H__

#include <valueAnimator/SValueAnimator.h>
#include <interface/SCtrl-i.h>
#include <core/SWnd.h>
#include <proxy/SWindowProxy.h>

SNSBEGIN

class SStackView;
/**
 * @class SViewSwitchAnimator
 * @brief Animator for switching views in SStackView.
 *
 * This class handles the animation when switching between views in an SStackView.
 */
class SOUI_EXP SViewSwitchAnimator : public TValueAnimator<float> {
    DEF_SOBJECT(TValueAnimator<float>, L"ViewSwitchAnimator")

  public:
    /**
     * @brief Constructor for SViewSwitchAnimator.
     * @param pOwner Pointer to the owner SStackView.
     */
    SViewSwitchAnimator(SStackView *pOwner)
        : TValueAnimator<float>(0.f, 1.f)
        , m_pOwner(pOwner)
        , m_iFrom(-1)
        , m_iTo(-1)
        , m_isOwnerResize(FALSE)
    {
    }

    /**
     * @brief Clones the animator.
     * @return Pointer to the cloned animator.
     */
    STDMETHOD_(IValueAnimator *, clone)(THIS) SCONST OVERRIDE
    {
        return NULL;
    }

    /**
     * @brief Evaluates the value at a given fraction.
     * @param fraction The fraction of the animation.
     */
    STDMETHOD_(void, onEvaluateValue)(THIS_ float fraction) OVERRIDE;

    /**
     * @brief Starts the animation.
     * @param pTimerlineMgr Pointer to the timeline handlers manager.
     */
    STDMETHOD_(void, start)(THIS_ ITimelineHandlersMgr *pTimerlineMgr) OVERRIDE;

    /**
     * @brief Sets the starting child index.
     * @param iChild The starting child index.
     */
    void SetFrom(int iChild)
    {
        m_iFrom = iChild;
    }

    /**
     * @brief Gets the starting child index.
     * @return The starting child index.
     */
    int GetFrom() const
    {
        return m_iFrom;
    }

    /**
     * @brief Sets the ending child index.
     * @param iChild The ending child index.
     */
    void SetTo(int iChild)
    {
        m_iTo = iChild;
    }

    /**
     * @brief Gets the ending child index.
     * @return The ending child index.
     */
    int GetTo() const
    {
        return m_iTo;
    }

    /**
     * @brief Handles owner resize.
     */
    void OnOwnerResize();

    /**
     * @brief Sets the owner size range.
     * @param szFrom The starting size.
     * @param szTo The ending size.
     */
    void SetOwnerSizeRange(SIZE szFrom, SIZE szTo);

    /**
     * @brief Checks if owner resize is enabled.
     * @return TRUE if owner resize is enabled, otherwise FALSE.
     */
    BOOL IsOwnerResize() const
    {
        return m_isOwnerResize;
    }

    /**
     * @brief Disables owner resize.
     */
    void DisableOwnerResize()
    {
        m_isOwnerResize = FALSE;
    }

    /**
     * @brief Gets the owner size.
     * @return The owner size.
     */
    SIZE GetOwnerSize() const;

    /**
     * @brief Gets the animation style.
     * @return The animation style.
     */
    StackViewAniStyle GetAniStyle() const
    {
        return m_aniStyle;
    }

  protected:
    /**
     * @brief Initializes the evaluators.
     */
    void initEvaluator();

    SStackView *m_pOwner;         /**< Pointer to the owner SStackView. */
    int m_iFrom, m_iTo;           /**< Indices of the starting and ending children. */
    BOOL m_bVertAni;              /**< Flag indicating vertical animation. */
    StackViewAniStyle m_aniStyle; /**< Animation style. */

    SAutoRefPtr<IWindow> m_pFrom; /**< Pointer to the starting child window. */
    SAutoRefPtr<IWindow> m_pTo;   /**< Pointer to the ending child window. */

    TypeEvaluator<RECT> m_evalRcFrom, m_evalRcTo;       /**< Evaluators for rectangle animations. */
    TypeEvaluator<BYTE> m_evalAlphaFrom, m_evalAlphaTo; /**< Evaluators for alpha animations. */
    TypeEvaluator<SIZE> m_evalOwnerSize;                /**< Evaluator for owner size animations. */
    BOOL m_isOwnerResize;                               /**< Flag indicating if owner resize is enabled. */
};

/**
 * @class SStackPage
 * @brief A page within a stack view.
 *
 * This class represents a page within an SStackView, which can have animation styles.
 */
class SOUI_EXP SStackPage : public SWindow {
    DEF_SOBJECT(SWindow, L"stackPage")

  public:
    /**
     * @brief Constructor for SStackPage.
     */
    SStackPage()
        : m_bVertAni(FALSE)
        , m_aniStyle(kAniNone)
    {
    }

  public:
    /**
     * @brief Gets the animation style.
     * @return The animation style.
     */
    StackViewAniStyle GetAnimateStyle() const
    {
        return m_aniStyle;
    }

    /**
     * @brief Checks if vertical animation is enabled.
     * @return TRUE if vertical animation is enabled, otherwise FALSE.
     */
    BOOL IsVertAnimate() const
    {
        return m_bVertAni;
    }

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_BOOL(L"vertical", m_bVertAni, FALSE) /**< Flag indicating vertical animation. */
        ATTR_ENUM_BEGIN(L"aniType", StackViewAniStyle, FALSE)
            ATTR_ENUM_VALUE(L"none", kAniNone)   /**< No animation. */
            ATTR_ENUM_VALUE(L"fade", kFadeInOut) /**< Fade in/out animation. */
            ATTR_ENUM_VALUE(L"move", kMoveInOut) /**< Move in/out animation. */
            ATTR_ENUM_VALUE(L"push", kPushInOut) /**< Push in/out animation. */
        ATTR_ENUM_END(m_aniStyle)
    SOUI_ATTRS_END()

  protected:
    BOOL m_bVertAni;              /**< Flag indicating vertical animation. */
    StackViewAniStyle m_aniStyle; /**< Animation style. */
};

/**
 * @class SStackView
 * @brief A stack view that can contain multiple pages.
 *
 * This class represents a stack view that can switch between multiple pages with animations.
 */
class SOUI_EXP SStackView
    : public TWindowProxy<IStackView>
    , public IAnimatorListener {
    friend class SViewSwitchAnimator;
    DEF_SOBJECT(SWindow, L"stack")

  public:
    /**
     * @brief Constructor for SStackView.
     */
    SStackView(void);

    /**
     * @brief Destructor for SStackView.
     */
    ~SStackView(void);

  public:
    /**
     * @brief Selects a page.
     * @param iView The index of the page to select.
     * @param enableAnimate Flag indicating if animation should be enabled.
     * @return TRUE if successful, otherwise FALSE.
     */
    STDMETHOD_(BOOL, SelectPage)(THIS_ int iView, BOOL enableAnimate DEF_VAL(TRUE)) OVERRIDE;

    /**
     * @brief Sets the animation style.
     * @param aniStyle The animation style.
     */
    STDMETHOD_(void, SetAniStyle)(THIS_ StackViewAniStyle aniStyle) OVERRIDE;

    /**
     * @brief Sets the animation direction.
     * @param bVert Flag indicating vertical animation.
     */
    STDMETHOD_(void, SetAniDir)(THIS_ BOOL bVert) OVERRIDE;

    /**
     * @brief Gets the selected page.
     * @return Pointer to the selected page.
     */
    STDMETHOD_(IWindow *, GetSelPage)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Gets a page by index.
     * @param iPage The index of the page.
     * @return Pointer to the page.
     */
    STDMETHOD_(IWindow *, GetPage)(CTHIS_ int iPage) SCONST OVERRIDE;

  public:
    /**
     * @brief Gets a child window by index.
     * @param iChild The index of the child window.
     * @return Pointer to the child window.
     */
    STDMETHOD_(IWindow *, GetIChild)(THIS_ int iChild) SCONST OVERRIDE;

    /**
     * @brief Updates the positions of child windows.
     */
    STDMETHOD_(void, UpdateChildrenPosition)(THIS) OVERRIDE;

  protected:
    /**
     * @brief Handles the start of an animation.
     * @param pAnimator Pointer to the animator.
     */
    STDMETHOD_(void, onAnimationStart)(THIS_ IValueAnimator *pAnimator) OVERRIDE
    {
    }

    /**
     * @brief Handles the repetition of an animation.
     * @param pAnimator Pointer to the animator.
     */
    STDMETHOD_(void, onAnimationRepeat)(THIS_ IValueAnimator *pAnimator) OVERRIDE
    {
    }

    /**
     * @brief Handles the end of an animation.
     * @param pAnimator Pointer to the animator.
     */
    STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator *pAnimator) OVERRIDE;

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"curSel", m_iSel, FALSE)        /**< Index of the currently selected page. */
        ATTR_BOOL(L"vertical", m_bVertAni, FALSE) /**< Flag indicating vertical animation. */
        ATTR_ENUM_BEGIN(L"aniType", StackViewAniStyle, FALSE)
            ATTR_ENUM_VALUE(L"none", kAniNone)   /**< No animation. */
            ATTR_ENUM_VALUE(L"fade", kFadeInOut) /**< Fade in/out animation. */
            ATTR_ENUM_VALUE(L"move", kMoveInOut) /**< Move in/out animation. */
            ATTR_ENUM_VALUE(L"push", kPushInOut) /**< Push in/out animation. */
        ATTR_ENUM_END(m_aniStyle)
        ATTR_BOOL(L"samePageSize", m_isSamePageSize, FALSE) /**< Flag indicating if all pages have the same size. */
        ATTR_CHAIN_CLASS(__baseCls)                         /**< Chain attributes to base class. */
        ATTR_CHAIN(m_animator, 0)                           /**< Chain attributes to animator. */
    SOUI_ATTRS_BREAK()

  protected:
    /**
     * @brief Handles the destruction of the window.
     */
    void OnDestroy();

    /**
     * @brief Handles the size change of the window.
     * @param nType The type of size change.
     * @param size The new size.
     */
    void OnSize(UINT nType, CSize size);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SIZE(OnSize)
    SOUI_MSG_MAP_END()

  protected:
    /**
     * @brief Creates child windows from an XML node.
     * @param xmlNode The XML node containing the child window definitions.
     * @return TRUE if successful, otherwise FALSE.
     */
    BOOL CreateChildren(SXmlNode xmlNode) override;

    /**
     * @brief Measures the size of child windows.
     * @param nParentWid The width of the parent window.
     * @param nParentHei The height of the parent window.
     * @return The measured size.
     */
    SIZE MeasureChildren(int nParentWid, int nParentHei) override;

    /**
     * @brief Handles the change of the container.
     * @param pOldContainer Pointer to the old container.
     * @param pNewContainer Pointer to the new container.
     */
    void OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer) override;

    /**
     * @brief Handles the insertion of a child window.
     * @param pChild Pointer to the child window.
     */
    void OnAfterInsertChild(SWindow *pChild) override;

    /**
     * @brief Handles the removal of a child window.
     * @param pChild Pointer to the child window.
     */
    void OnAfterRemoveChild(SWindow *pChild) override;

  protected:
    /**
     * @brief Builds the array of child windows.
     * @param updateSel Flag indicating if the selection should be updated.
     */
    void BuildChildsArray(BOOL updateSel);

    /**
     * @brief Gets the animation style of a child.
     * @param iChild The index of the child.
     * @return The animation style.
     */
    StackViewAniStyle GetChildAnimateStyle(int iChild) const;

    /**
     * @brief Checks if vertical animation is enabled for a child.
     * @param iChild The index of the child.
     * @return TRUE if vertical animation is enabled, otherwise FALSE.
     */
    BOOL IsVertChildAnimate(int iChild) const;

    /**
     * @brief Gets the size of a child window.
     * @param pPage Pointer to the child window.
     * @return The size of the child window.
     */
    CSize GetChildSize(IWindow *pPage) const;

  protected:
    int m_iSel;                     /**< Index of the currently selected page. */
    SViewSwitchAnimator m_animator; /**< Animator for view switching. */
    SArray<SWindow *> m_childs;     /**< Array of child windows. */

    BOOL m_bVertAni;              /**< Flag indicating vertical animation. */
    StackViewAniStyle m_aniStyle; /**< Animation style. */
    BOOL m_isSamePageSize;        /**< Flag indicating if all pages have the same size. */
};

SNSEND

#endif // __SSTACKVIEW__H__