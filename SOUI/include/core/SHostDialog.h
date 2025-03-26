/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SHostDialog.h
 * @brief
 *
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * Describe
 * SOUI的Dialog模块
 */

#ifndef __SHOSTDIALOG__H__
#define __SHOSTDIALOG__H__

#include <core/SHostWnd.h>
#include <core/SMsgLoop.h>
#include <proxy/SHostWndProxy.h>

SNSBEGIN

class SOUI_EXP SHostDialog : public THostWndProxy<IHostDialog> {
  public:
    SHostDialog(LPCWSTR pszXmlName = NULL);
    SHostDialog(LPCSTR pszXmlName);
    ~SHostDialog(void);

  public:
    STDMETHOD_(IMessageLoop *, GetMsgLoop)(THIS) OVERRIDE;
    STDMETHOD_(INT_PTR, DoModal)(THIS_ HWND hParent DEF_VAL(0), DWORD dwStyle DEF_VAL(WS_POPUP | WS_CLIPCHILDREN), DWORD dwExStyle DEF_VAL(0)) OVERRIDE;
    STDMETHOD_(void, EndDialog)(THIS_ INT_PTR nResult) OVERRIDE;

    SHostWnd *toSHostWnd()
    {
        return this;
    }

  protected:
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    void OnOK();
    void OnCancel();

    EVENT_MAP_BEGIN()
        EVENT_ID_COMMAND(IDOK, OnOK)
        EVENT_ID_COMMAND(IDCANCEL, OnCancel)
    EVENT_MAP_END2(SHostWnd)

    BEGIN_MSG_MAP_EX(SHostDialog)
        MSG_WM_CLOSE(OnCancel)
        MSG_WM_KEYDOWN(OnKeyDown)
        CHAIN_MSG_MAP(SHostWnd)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

    INT_PTR m_nRetCode;

    SAutoRefPtr<IMessageLoop> m_MsgLoop;
};

SNSEND
#endif // __SHOSTDIALOG__H__