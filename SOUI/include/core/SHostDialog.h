/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SHostDialog.h
 * @brief      SOUI Dialog Module
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * @details    This file defines the SHostDialog class, which manages dialog windows in the SOUI framework.
 */

#ifndef __SHOSTDIALOG__H__
#define __SHOSTDIALOG__H__

#include <core/SHostWnd.h>
#include <core/SMsgLoop.h>
#include <proxy/SHostWndProxy.h>

SNSBEGIN

/**
 * @class      SHostDialog
 * @brief      Dialog host window class
 *
 * @details    This class manages dialog windows in the SOUI framework. It inherits from `THostWndProxy<IHostDialog>`
 *             and provides methods for handling dialog messages and events.
 */
class SOUI_EXP SHostDialog : public THostWndProxy<IHostDialog> {
  public:
    /**
     * @brief    Constructor
     * @param    pszXmlName  Name of the XML file for the dialog layout (wide character version)
     *
     * @details  Initializes the dialog with the specified XML layout file.
     */
    SHostDialog(LPCWSTR pszXmlName = NULL);

    /**
     * @brief    Constructor
     * @param    pszXmlName  Name of the XML file for the dialog layout (narrow character version)
     *
     * @details  Initializes the dialog with the specified XML layout file.
     */
    SHostDialog(LPCSTR pszXmlName);

    /**
     * @brief    Destructor
     *
     * @details  Cleans up the dialog object.
     */
    ~SHostDialog(void);

  public:
    /**
     * @brief    Gets the message loop for the dialog
     * @return   Pointer to the message loop
     *
     * @details  Returns the message loop associated with the dialog.
     */
    STDMETHOD_(IMessageLoop *, GetMsgLoop)(THIS) OVERRIDE;

    /**
     * @brief    Displays the dialog as a modal window
     * @param    hParent    Handle to the parent window
     * @param    dwStyle    Window style
     * @param    dwExStyle  Extended window style
     * @return   Result code of the dialog
     *
     * @details  Displays the dialog as a modal window and returns the result code.
     */
    STDMETHOD_(INT_PTR, DoModal)(THIS_ HWND hParent DEF_VAL(0), DWORD dwStyle DEF_VAL(WS_POPUP | WS_CLIPCHILDREN), DWORD dwExStyle DEF_VAL(0)) OVERRIDE;

    /**
     * @brief    Ends the dialog
     * @param    nResult  Result code to return from the dialog
     *
     * @details  Ends the dialog and sets the result code.
     */
    STDMETHOD_(void, EndDialog)(THIS_ INT_PTR nResult) OVERRIDE;

    /**
     * @brief    Casts the object to SHostWnd
     * @return   Pointer to the SHostWnd object
     *
     * @details  Casts the SHostDialog object to an SHostWnd object.
     */
    SHostWnd *toSHostWnd()
    {
        return this;
    }

  protected:
    /**
     * @brief    Handles the key down event
     * @param    nChar    Virtual key code
     * @param    nRepCnt  Repeat count
     * @param    nFlags   Flags associated with the key event
     *
     * @details  Handles the key down event for the dialog.
     */
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief    Handles the OK button click event
     *
     * @details  Handles the click event for the OK button, typically ending the dialog with a result code.
     */
    void OnOK();

    /**
     * @brief    Handles the Cancel button click event
     *
     * @details  Handles the click event for the Cancel button, typically ending the dialog with a result code.
     */
    void OnCancel();

    EVENT_MAP_BEGIN()
        EVENT_ID_COMMAND(IDOK, OnOK)         /**< Maps the IDOK command to the OnOK method. */
        EVENT_ID_COMMAND(IDCANCEL, OnCancel) /**< Maps the IDCANCEL command to the OnCancel method. */
    EVENT_MAP_END2(SHostWnd)

    BEGIN_MSG_MAP_EX(SHostDialog)
        MSG_WM_CLOSE(OnCancel)     /**< Maps the WM_CLOSE message to the OnCancel method. */
        MSG_WM_KEYDOWN(OnKeyDown)  /**< Maps the WM_KEYDOWN message to the OnKeyDown method. */
        CHAIN_MSG_MAP(SHostWnd)    /**< Chains message map to the base class SHostWnd. */
        REFLECT_NOTIFICATIONS_EX() /**< Reflects notifications to the dialog. */
    END_MSG_MAP()

  protected:
    INT_PTR m_nRetCode;                  /**< Result code of the dialog. */
    SAutoRefPtr<IMessageLoop> m_MsgLoop; /**< Message loop for the dialog. */
};

SNSEND

#endif // __SHOSTDIALOG__H__