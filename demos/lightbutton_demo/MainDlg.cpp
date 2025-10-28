#include "stdafx.h"
#include "MainDlg.h"
#include <SLightButton.h>

namespace SOUI
{

CMainDlg::CMainDlg()
    : SHostDialog(_T("LAYOUT:XML_MAINWND"))
    , m_pLightButton1(NULL)
    , m_pLightButton2(NULL)
    , m_pLightButton3(NULL)
    , m_pLightButton4(NULL)
    , m_pLightButton5(NULL)
    , m_pLightButton6(NULL)
    , m_pCurrentButton(NULL)
{
}

CMainDlg::~CMainDlg()
{
}

BOOL CMainDlg::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
{
    m_pLightButton1 = FindChildByName2<SLightButton>(L"lightbutton1");
    m_pLightButton2 = FindChildByName2<SLightButton>(L"lightbutton2");
    m_pLightButton3 = FindChildByName2<SLightButton>(L"lightbutton3");
    m_pLightButton4 = FindChildByName2<SLightButton>(L"lightbutton4");
    m_pLightButton5 = FindChildByName2<SLightButton>(L"lightbutton5");
    m_pLightButton6 = FindChildByName2<SLightButton>(L"lightbutton6");

    // 设置初始状态
    if (m_pLightButton1)
    {
        m_pLightButton1->SetLightBlue();
        m_pCurrentButton = m_pLightButton1;
    }

    if (m_pLightButton2)
    {
        m_pLightButton2->SetLightRed();
    }

    if (m_pLightButton3)
    {
        m_pLightButton3->SetLightGreen();
    }

    if (m_pLightButton4)
    {
        m_pLightButton4->SetYellow();
        m_pLightButton4->SetShowRect(TRUE);
    }

    if (m_pLightButton5)
    {
        m_pLightButton5->SetGray();
    }

    if (m_pLightButton6)
    {
        m_pLightButton6->SetRed();
        m_pLightButton6->StartAlarm();
    }

    return TRUE;
}

void CMainDlg::OnClose()
{
    SHostWnd::DestroyWindow();
}

void CMainDlg::OnMaximize()
{
    SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}

void CMainDlg::OnRestore()
{
    SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}

void CMainDlg::OnMinimize()
{
    SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
    if(nType == SIZE_MAXIMIZED){
        FindChildByName2<SWindow>(L"btn_max")->SetVisible(FALSE);
        FindChildByName2<SWindow>(L"btn_restore")->SetVisible(TRUE);
    }else{
        FindChildByName2<SWindow>(L"btn_max")->SetVisible(TRUE);
        FindChildByName2<SWindow>(L"btn_restore")->SetVisible(FALSE);
    }
    SetMsgHandled(FALSE);
}

// LightButton 点击事件处理
void CMainDlg::OnLightButton1Click()
{
    m_pCurrentButton = m_pLightButton1;
    SMessageBox(m_hWnd, _T("点击了按钮1"), _T("提示"), MB_OK);
}

void CMainDlg::OnLightButton2Click()
{
    m_pCurrentButton = m_pLightButton2;
    SMessageBox(m_hWnd, _T("点击了按钮2"), _T("提示"), MB_OK);
}

void CMainDlg::OnLightButton3Click()
{
    m_pCurrentButton = m_pLightButton3;
    SMessageBox(m_hWnd, _T("点击了按钮3"), _T("提示"), MB_OK);
}

void CMainDlg::OnLightButton4Click()
{
    m_pCurrentButton = m_pLightButton4;
    SMessageBox(m_hWnd, _T("点击了矩形按钮"), _T("提示"), MB_OK);
}

void CMainDlg::OnLightButton5Click()
{
    m_pCurrentButton = m_pLightButton5;
    SMessageBox(m_hWnd, _T("点击了可移动按钮"), _T("提示"), MB_OK);
}

void CMainDlg::OnLightButton6Click()
{
    m_pCurrentButton = m_pLightButton6;
    SMessageBox(m_hWnd, _T("点击了报警按钮"), _T("提示"), MB_OK);
}

// 颜色设置按钮事件处理
void CMainDlg::OnBtnSetGreen()
{
    if (m_pCurrentButton)
        m_pCurrentButton->SetGreen();
}

void CMainDlg::OnBtnSetRed()
{
    if (m_pCurrentButton)
        m_pCurrentButton->SetRed();
}

void CMainDlg::OnBtnSetYellow()
{
    if (m_pCurrentButton)
        m_pCurrentButton->SetYellow();
}

void CMainDlg::OnBtnSetBlue()
{
    if (m_pCurrentButton)
        m_pCurrentButton->SetBlue();
}

void CMainDlg::OnBtnSetLightBlue()
{
    if (m_pCurrentButton)
        m_pCurrentButton->SetLightBlue();
}

void CMainDlg::OnBtnSetLightRed()
{
    if (m_pCurrentButton)
        m_pCurrentButton->SetLightRed();
}

void CMainDlg::OnBtnSetLightGreen()
{
    if (m_pCurrentButton)
        m_pCurrentButton->SetLightGreen();
}

void CMainDlg::OnBtnSetBlack()
{
    if (m_pCurrentButton)
        m_pCurrentButton->SetBlack();
}

void CMainDlg::OnBtnSetGray()
{
    if (m_pCurrentButton)
        m_pCurrentButton->SetGray();
}

// 功能控制按钮事件处理
void CMainDlg::OnBtnStartAlarm()
{
    if (m_pCurrentButton)
        m_pCurrentButton->StartAlarm();
}

void CMainDlg::OnBtnStopAlarm()
{
    if (m_pCurrentButton)
        m_pCurrentButton->StopAlarm();
}

void CMainDlg::OnBtnToggleRect()
{
    if (m_pCurrentButton)
    {
        BOOL bShowRect = m_pCurrentButton->GetShowRect();
        m_pCurrentButton->SetShowRect(!bShowRect);
    }
}

void CMainDlg::OnBtnToggleOverlay()
{
    if (m_pCurrentButton)
    {
        BOOL bShowOverlay = m_pCurrentButton->GetShowOverlay();
        m_pCurrentButton->SetShowOverlay(!bShowOverlay);
    }
}

} // namespace SOUI
