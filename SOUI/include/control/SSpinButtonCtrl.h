/**
 * @file SSpinButtonCtrl.h
 * @brief Definition of the SSpinButtonCtrl class.
 */

#ifndef __SSPINBUTTONCTRL__H__
#define __SSPINBUTTONCTRL__H__

#include <core/SWnd.h>
#include <interface/SCtrls-i.h>
#include <proxy/SWindowProxy.h>

SNSBEGIN

/**
 * @class SSpinButtonCtrl
 * @brief A spin button control class derived from TWindowProxy<ISpinButtonCtrl>.
 *
 * This class represents a spin button control that can be used to increase or decrease a value within a specified range.
 */
class SOUI_EXP SSpinButtonCtrl : public TWindowProxy<ISpinButtonCtrl> {
    /**
     * @brief Define the SObject type and name, base class is SWindow, control name is spinButton.
     */
    DEF_SOBJECT(SWindow, L"spinButton")

  public:
    /**
     * @brief Constructor for SSpinButtonCtrl.
     */
    SSpinButtonCtrl(void);

    /**
     * @brief Destructor for SSpinButtonCtrl.
     */
    ~SSpinButtonCtrl(void);

    /**
     * @brief Retrieves the buddy window associated with the spin button.
     * @return Pointer to the buddy SWindow object.
     */
    SWindow *GetBuddy() const;

  public:
    /**
     * @brief Sets the current value of the spin button.
     * @param nValue The new value to set.
     */
    STDMETHOD_(void, SetValue)(THIS_ int nValue) OVERRIDE;

    /**
     * @brief Sets the range of values for the spin button.
     * @param nMin The minimum value of the range.
     * @param nMax The maximum value of the range.
     */
    STDMETHOD_(void, SetRange)(THIS_ int nMin, int nMax) OVERRIDE;

    /**
     * @brief Sets the step size for the spin button.
     * @param nStep The step size to use when incrementing or decrementing the value.
     */
    STDMETHOD_(void, SetStep)(THIS_ UINT nStep) OVERRIDE;

    /**
     * @brief Retrieves the current value of the spin button.
     * @return The current value of the spin button.
     */
    STDMETHOD_(int, GetValue)(THIS) SCONST OVERRIDE;

    /**
     * @brief Retrieves the buddy window interface associated with the spin button.
     * @return Pointer to the buddy IWindow interface.
     */
    STDMETHOD_(IWindow *, GetIBuddy)(THIS) SCONST OVERRIDE;

  protected:
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
    virtual BOOL NeedRedrawWhenStateChange() OVERRIDE
    {
        return TRUE;
    }

    /**
     * @brief Handles colorization of the control.
     * @param cr Color reference for colorization.
     */
    virtual void OnColorize(COLORREF cr);

  protected:
    /**
     * @brief Handles value change events.
     * @param bInit Flag indicating if the change is during initialization.
     */
    void OnValueChanged(bool bInit = false);

    /**
     * @brief Handles scale change events.
     * @param scale The new scale value.
     */
    void OnScaleChanged(int scale);

    /**
     * @brief Handles left mouse button down events.
     * @param nFlags Flags associated with the mouse event.
     * @param point The position of the mouse cursor.
     */
    void OnLButtonDown(UINT nFlags, CPoint point);

    /**
     * @brief Handles left mouse button up events.
     * @param nFlags Flags associated with the mouse event.
     * @param point The position of the mouse cursor.
     */
    void OnLButtonUp(UINT nFlags, CPoint point);

    /**
     * @brief Handles mouse move events.
     * @param nFlags Flags associated with the mouse event.
     * @param point The position of the mouse cursor.
     */
    void OnMouseMove(UINT nFlags, CPoint point);

    /**
     * @brief Handles paint events.
     * @param pRT Pointer to the render target.
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief Handles timer events.
     * @param cTimerId Identifier for the timer.
     */
    void OnTimer(char cTimerId);

    /**
     * @brief Handles buddy change events.
     * @param pEvt Pointer to the event arguments.
     * @return TRUE if the event was handled, otherwise FALSE.
     */
    BOOL OnBuddyChange(IEvtArgs *pEvt);

    /**
     * @brief Handles creation events.
     * @param lp Create structure.
     * @return Result of the creation process.
     */
    int OnCreate(void *);

    /**
     * @brief Handles click events.
     */
    void OnClick();

    /**
     * @brief Maps messages to handler functions.
     */
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_CREATE(OnCreate)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDown)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_TIMER_EX(OnTimer)
    SOUI_MSG_MAP_END()

  protected:
    /**
     * @brief Handles attribute value changes.
     * @param strValue The new attribute value.
     * @param bLoading Flag indicating if the change is during loading.
     * @return Result of the attribute change process.
     */
    HRESULT OnAttrValue(const SStringW &strValue, BOOL bLoading);

    /**
     * @brief Maps attributes to member variables.
     */
    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"max", m_nMax, FALSE)
        ATTR_INT(L"min", m_nMin, FALSE)
        ATTR_CUSTOM(L"value", OnAttrValue)
        ATTR_UINT(L"step", m_uStep, FALSE)
        ATTR_INT(L"circle", m_bCircle, FALSE)
        ATTR_STRINGW(L"buddy", m_strBuddy, FALSE)
        ATTR_SKIN(L"upskin", m_pUpSkin, TRUE)
        ATTR_SKIN(L"downSkin", m_pDownSkin, TRUE)
    SOUI_ATTRS_END()

  protected:
    int m_nMax;          /**< Maximum value of the spin button. */
    int m_nMin;          /**< Minimum value of the spin button. */
    int m_nValue;        /**< Current value of the spin button. */
    UINT m_uStep;        /**< Step size for incrementing or decrementing the value. */
    BOOL m_bCircle;      /**< Flag indicating if the spin button is circular. */
    SStringW m_strBuddy; /**< Buddy window identifier. */

    SAutoRefPtr<ISkinObj> m_pUpSkin;   /**< Skin object for the up button. */
    SAutoRefPtr<ISkinObj> m_pDownSkin; /**< Skin object for the down button. */

    /**
     * @enum ActionButton
     * @brief Enumeration for action buttons.
     */
    enum ActionButton
    {
        ACTION_NULL = -1, /**< No action. */
        ACTION_UP,        /**< Up action. */
        ACTION_DOWN       /**< Down action. */
    };
    ActionButton m_iActionBtn; /**< Current action button. */
};

SNSEND

#endif // __SSPINBUTTONCTRL__H__