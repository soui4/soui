#include <souistd.h>
#include <core/SHostDialog.h>
#include <core/SMsgLoop.h>

SNSBEGIN

#define RC_INIT 0xcccccccc

SHostDialog::SHostDialog(LPCWSTR pszXmlName)
    : THostWndProxy<IHostDialog>(pszXmlName)
    , m_nRetCode(RC_INIT)
{
}

SHostDialog::SHostDialog(LPCSTR pszXmlName)
    : THostWndProxy<IHostDialog>(pszXmlName)
    , m_nRetCode(RC_INIT)
{
}

SHostDialog::~SHostDialog(void)
{
    m_MsgLoop = NULL;
}

INT_PTR SHostDialog::DoModal(HWND hParent /*=NULL*/)
{
    SASSERT(!m_MsgLoop);
    SAutoRefPtr<IMessageLoop> parentMsgLoop = SApplication::getSingletonPtr()->GetMsgLoop();
    SApplication::getSingleton().GetMsgLoopFactory()->CreateMsgLoop(&m_MsgLoop, parentMsgLoop);

    if (!hParent)
    {
        hParent = ::GetActiveWindow();
    }

    BOOL bEnableParent = FALSE;
    if (hParent && hParent != ::GetDesktopWindow() && ::IsWindowEnabled(hParent))
    {
        ::EnableWindow(hParent, FALSE);
        bEnableParent = TRUE;
    }

    if (!SHostWnd::CreateEx(hParent, WS_POPUP | WS_CLIPCHILDREN, 0, 0, 0, 0, 0))
    {
        if (bEnableParent)
        {
            ::EnableWindow(hParent, TRUE);
        }
        return 0;
    }
    SNativeWnd::SendMessage(WM_INITDIALOG, (WPARAM)m_hWnd);

    if (m_nRetCode == RC_INIT)
    {
        HWND hWndLastActive = ::SetActiveWindow(m_hWnd);

        if (GetExStyle() & WS_EX_TOOLWINDOW)
            ::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
        else
            ::ShowWindow(m_hWnd, SW_SHOWNORMAL);

        int nRet = m_MsgLoop->Run();

        // From MFC
        // hide the window before enabling the parent, etc.
        if (IsWindow())
        {
            ShowHostWnd(SW_HIDE, TRUE);
        }

        if (m_nRetCode == RC_INIT)
        { //可能是程序中主动退出app，而不是自己EndDialog关闭窗口，重新把WM_QUIT放回消息队列。
            parentMsgLoop->Quit(nRet);
        }

        if (bEnableParent)
        {
            ::EnableWindow(hParent, TRUE);
        }

        ::SetActiveWindow(hWndLastActive);
    }

    if (IsWindow())
        SNativeWnd::DestroyWindow();
    m_MsgLoop = NULL;
    return m_nRetCode;
}

void SHostDialog::EndDialog(INT_PTR nResult)
{
    if (!m_MsgLoop)
    {
        SSLOGW() << "dialog is not show by DoModal";
        return;
    }
    SASSERT(nResult != RC_INIT);
    if (m_nRetCode == RC_INIT)
    {
        m_nRetCode = nResult;
        m_MsgLoop->Quit((int)m_nRetCode);
    }
}

IMessageLoop *SHostDialog::GetMsgLoop(THIS)
{
    if (m_MsgLoop)
        return m_MsgLoop;
    else
        return SHostWnd::GetMsgLoop();
}

void SHostDialog::OnOK()
{
    EndDialog(IDOK);
}

void SHostDialog::OnCancel()
{
    EndDialog(IDCANCEL);
}

void SHostDialog::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    SHostWnd::OnKeyEvent(WM_KEYDOWN, (WPARAM)nChar, MAKELPARAM(nRepCnt, nFlags));
    if (SHostWnd::IsMsgHandled())
        return;
    if (nChar == VK_ESCAPE || nChar == VK_RETURN)
    {
        SWindow *pBtnExit = FindChildByID(nChar == VK_ESCAPE ? IDCANCEL : IDOK);
        if (pBtnExit)
        {
            pBtnExit->FireCommand();
        }
    }
}

SNSEND