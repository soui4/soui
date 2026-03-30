#ifndef __SACCPROXYCMNCTRL__H__
#define __SACCPROXYCMNCTRL__H__

#include "SAccProxyWindow.h"

SNSBEGIN

#ifdef SOUI_ENABLE_ACC

/**
 * @class SAccProxyButton
 * @brief Accessibility proxy class for button controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyButton : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyButton(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyCheckButton
 * @brief Accessibility proxy class for check button controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyCheckButton : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyCheckButton(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyRadioButton
 * @brief Accessibility proxy class for radio button controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyRadioButton : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyRadioButton(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyCombobox
 * @brief Accessibility proxy class for combo box controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyCombobox : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyCombobox(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyEdit
 * @brief Accessibility proxy class for edit controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyEdit : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyEdit(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyProgress
 * @brief Accessibility proxy class for progress controls.
 *        Inherits from SAccProxyWindow and overrides methods for name, value, and role.
 */
class SOUI_EXP SAccProxyProgress : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyProgress(IWindow *pWnd);

    /**
     * @brief Destructor.
     */
    virtual ~SAccProxyProgress(void);

  public:
    /**
     * @brief Retrieves the name of the accessible object.
     * @param pszValue Pointer to receive the name of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accName(BSTR *pszValue);

    /**
     * @brief Retrieves the value of the accessible object.
     * @param pszValue Pointer to receive the value of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accValue(BSTR *pszValue);

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxySlideBar
 * @brief Accessibility proxy class for slider bar controls.
 *        Inherits from SAccProxyProgress and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxySlideBar : public SAccProxyProgress {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxySlideBar(IWindow *pWnd)
        : SAccProxyProgress(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyStatic
 * @brief Accessibility proxy class for static text controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyStatic : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyStatic(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyLink
 * @brief Accessibility proxy class for link controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyLink : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyLink(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyImage
 * @brief Accessibility proxy class for image controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyImage : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyImage(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyIcon
 * @brief Accessibility proxy class for icon controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyIcon : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyIcon(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyHrLine
 * @brief Accessibility proxy class for horizontal line controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyHrLine : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyHrLine(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyGroup
 * @brief Accessibility proxy class for group controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyGroup : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyGroup(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyActiveX
 * @brief Accessibility proxy class for ActiveX controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyActiveX : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyActiveX(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyCalendar
 * @brief Accessibility proxy class for calendar controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyCalendar : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyCalendar(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyCaption
 * @brief Accessibility proxy class for caption controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyCaption : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyCaption(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyListCtrl
 * @brief Accessibility proxy class for list control controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyListCtrl : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyListCtrl(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyDateTimePicker
 * @brief Accessibility proxy class for date time picker controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyDateTimePicker : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyDateTimePicker(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyDockBar
 * @brief Accessibility proxy class for dock bar controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyDockBar : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyDockBar(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyDropDown
 * @brief Accessibility proxy class for drop down controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyDropDown : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyDropDown(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyFrame
 * @brief Accessibility proxy class for frame controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyFrame : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyFrame(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyHeaderCtrl
 * @brief Accessibility proxy class for header control controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyHeaderCtrl : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyHeaderCtrl(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyHotKeyCtrl
 * @brief Accessibility proxy class for hot key control controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyHotKeyCtrl : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyHotKeyCtrl(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyListView
 * @brief Accessibility proxy class for list view controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyListView : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyListView(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyListBox
 * @brief Accessibility proxy class for list box controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyListBox : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyListBox(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyMCListView
 * @brief Accessibility proxy class for multi-column list view controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyMCListView : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyMCListView(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyMenuBar
 * @brief Accessibility proxy class for menu bar controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyMenuBar : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyMenuBar(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyRealWnd
 * @brief Accessibility proxy class for real window controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyRealWnd : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyRealWnd(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyScrollBar
 * @brief Accessibility proxy class for scroll bar controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyScrollBar : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyScrollBar(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxySearchDropdownList
 * @brief Accessibility proxy class for search dropdown list controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxySearchDropdownList : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxySearchDropdownList(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxySpinButtonCtrl
 * @brief Accessibility proxy class for spin button control controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxySpinButtonCtrl : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxySpinButtonCtrl(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxySplitWnd
 * @brief Accessibility proxy class for split window controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxySplitWnd : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxySplitWnd(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyStackView
 * @brief Accessibility proxy class for stack view controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyStackView : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyStackView(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxySwitch
 * @brief Accessibility proxy class for switch controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxySwitch : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxySwitch(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyTabCtrl
 * @brief Accessibility proxy class for tab control controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyTabCtrl : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyTabCtrl(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyTileView
 * @brief Accessibility proxy class for tile view controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyTileView : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyTileView(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyToolBar
 * @brief Accessibility proxy class for tool bar controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyToolBar : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyToolBar(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyTreeCtrl
 * @brief Accessibility proxy class for tree control controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyTreeCtrl : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyTreeCtrl(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyTreeView
 * @brief Accessibility proxy class for tree view controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyTreeView : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyTreeView(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

#endif // SOUI_ENABLE_ACC

SNSEND

#endif // __SACCPROXYCMNCTRL__H__