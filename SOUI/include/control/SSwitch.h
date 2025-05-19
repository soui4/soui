/**
 * @file       SSwitch.h
 * @brief      Definition of the SSwitch class.
 * @version    v1.0
 * @author     soui
 * @date       2014-07-08
 *
 * @copyright  Copyright (C) 2014-2050 SOUI团队
 *             All rights reserved.
 */

#ifndef _SWITCH_H_
#define _SWITCH_H_

#include <core/SWnd.h>

SNSBEGIN

/**
 * @class SSwitch
 * @brief A switch control class derived from SWindow.
 *
 * This class represents a switch control that can be toggled between two states.
 */
class SSwitch
    : public SWindow
    , protected IAnimatorListener
    , public IAnimatorUpdateListener {
    DEF_SOBJECT(SWindow, L"switch")

  public:
    /**
     * @brief Constructor for SSwitch.
     */
    SSwitch();

    /**
     * @brief Destructor for SSwitch.
     */
    ~SSwitch();

  protected:
    /**
     * @brief Handles the start of an animation.
     * @param pAnimator Pointer to the animator.
     */
    STDMETHOD_(void, onAnimationStart)(THIS_ IValueAnimator *pAnimator) override
    {
    }

    /**
     * @brief Handles the end of an animation.
     * @param pAnimator Pointer to the animator.
     */
    STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator *pAnimator) override;

    /**
     * @brief Handles the repetition of an animation.
     * @param pAnimator Pointer to the animator.
     */
    STDMETHOD_(void, onAnimationRepeat)(THIS_ IValueAnimator *pAnimator) override
    {
    }

    /**
     * @brief Handles the update of an animation.
     * @param pAnimator Pointer to the animator.
     */
    STDMETHOD_(void, onAnimationUpdate)(THIS_ IValueAnimator *pAnimator) override;

  protected: // SWindow's virtual functions
    /**
     * @brief Calculates the desired size of the control.
     * @param psz Pointer to the SIZE structure to receive the desired size.
     * @param nParentWid Width of the parent window.
     * @param nParentHei Height of the parent window.
     */
    STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE *psz, int nParentWid, int nParentHei) OVERRIDE;

    /**
     * @brief Determines whether the control needs to be redrawn when its state changes.
     * @return TRUE if the control needs to be redrawn, otherwise FALSE.
     */
    BOOL NeedRedrawWhenStateChange() override
    {
        return TRUE;
    }

  protected: // Message handling, similar to WTL and MFC
    /**
     * @brief Handles the paint event.
     * @param pRT Pointer to the render target.
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief Handles the left mouse button up event.
     * @param nFlags Flags associated with the mouse event.
     * @param point The position of the mouse cursor.
     */
    void OnLButtonUp(UINT nFlags, CPoint point);

    /**
     * @brief Handles the key down event.
     * @param nChar The virtual-key code of the key.
     * @param nRepCnt The repeat count for the key.
     * @param nFlags Flags indicating the key state.
     */
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    // SOUI control message map
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)      /**< Window paint message. */
        MSG_WM_LBUTTONUP(OnLButtonUp) /**< Left mouse button up message. */
        MSG_WM_KEYDOWN(OnKeyDown)     /**< Key down message. */
    SOUI_MSG_MAP_END()

    SAutoRefPtr<ISkinObj> m_pSkin;      /**< Skin object for the switch. */
    SAutoRefPtr<ISkinObj> m_pSkinForce; /**< Skin object for the forced state. */

    /**
     * @brief Handles the 'animator' attribute.
     * @param value The attribute value.
     * @param bLoading Flag indicating if the attribute is being loaded.
     * @return Result of the attribute handling.
     */
    HRESULT OnAttrAnimator(const SStringW &value, BOOL bLoading);

    /**
     * @brief Handles the 'checked' attribute.
     * @param strValue The attribute value.
     * @param bLoading Flag indicating if the attribute is being loaded.
     * @return Result of the attribute handling.
     */
    HRESULT OnAttrCheck(const SStringW &strValue, BOOL bLoading);

    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"skin", m_pSkin, TRUE)           /**< Skin object for the switch. */
        ATTR_SKIN(L"skinforce", m_pSkinForce, TRUE) /**< Skin object for the forced state. */
        ATTR_CUSTOM(L"animator", OnAttrAnimator)    /**< Custom animator attribute. */
        ATTR_CUSTOM(L"checked", OnAttrCheck)        /**< Custom checked attribute. */
        ATTR_CHAIN_PTR(m_animator, 0)               /**< Chain attributes to animator. */
    SOUI_ATTRS_END()

  private:
    SAutoRefPtr<IValueAnimator> m_animator; /**< Animator for the switch. */
};

SNSEND

#endif // _SWITCH_H_