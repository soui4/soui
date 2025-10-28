#pragma once
#include <SLightButton.h>

namespace SOUI
{

class CMainDlg : public SHostDialog
{
public:
    CMainDlg();
    ~CMainDlg();

    void OnClose();
    void OnMaximize();
    void OnRestore();
    void OnMinimize();
    void OnSize(UINT nType, CSize size);

protected:
    BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

    // LightButton 事件处理
    void OnLightButton1Click();
    void OnLightButton2Click();
    void OnLightButton3Click();
    void OnLightButton4Click();
    void OnLightButton5Click();
    void OnLightButton6Click();

    // 控制按钮事件处理
    void OnBtnSetGreen();
    void OnBtnSetRed();
    void OnBtnSetYellow();
    void OnBtnSetBlue();
    void OnBtnSetLightBlue();
    void OnBtnSetLightRed();
    void OnBtnSetLightGreen();
    void OnBtnSetBlack();
    void OnBtnSetGray();
    
    void OnBtnStartAlarm();
    void OnBtnStopAlarm();
    void OnBtnToggleRect();
    void OnBtnToggleOverlay();

private:
    // LightButton 控件指针
    SLightButton* m_pLightButton1;
    SLightButton* m_pLightButton2;
    SLightButton* m_pLightButton3;
    SLightButton* m_pLightButton4;
    SLightButton* m_pLightButton5;
    SLightButton* m_pLightButton6;

    // 当前选中的按钮
    SLightButton* m_pCurrentButton;

    EVENT_MAP_BEGIN()
        EVENT_NAME_COMMAND(L"btn_close", OnClose)
        EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
        EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
        EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
        
        // LightButton 点击事件
        EVENT_NAME_COMMAND(L"lightbutton1", OnLightButton1Click)
        EVENT_NAME_COMMAND(L"lightbutton2", OnLightButton2Click)
        EVENT_NAME_COMMAND(L"lightbutton3", OnLightButton3Click)
        EVENT_NAME_COMMAND(L"lightbutton4", OnLightButton4Click)
        EVENT_NAME_COMMAND(L"lightbutton5", OnLightButton5Click)
        EVENT_NAME_COMMAND(L"lightbutton6", OnLightButton6Click)
        
        // 控制按钮事件
        EVENT_NAME_COMMAND(L"btn_green", OnBtnSetGreen)
        EVENT_NAME_COMMAND(L"btn_red", OnBtnSetRed)
        EVENT_NAME_COMMAND(L"btn_yellow", OnBtnSetYellow)
        EVENT_NAME_COMMAND(L"btn_blue", OnBtnSetBlue)
        EVENT_NAME_COMMAND(L"btn_light_blue", OnBtnSetLightBlue)
        EVENT_NAME_COMMAND(L"btn_light_red", OnBtnSetLightRed)
        EVENT_NAME_COMMAND(L"btn_light_green", OnBtnSetLightGreen)
        EVENT_NAME_COMMAND(L"btn_black", OnBtnSetBlack)
        EVENT_NAME_COMMAND(L"btn_gray", OnBtnSetGray)
        
        EVENT_NAME_COMMAND(L"btn_start_alarm", OnBtnStartAlarm)
        EVENT_NAME_COMMAND(L"btn_stop_alarm", OnBtnStopAlarm)
        EVENT_NAME_COMMAND(L"btn_toggle_rect", OnBtnToggleRect)
        EVENT_NAME_COMMAND(L"btn_toggle_overlay", OnBtnToggleOverlay)
    EVENT_MAP_END2(SHostDialog)

    BEGIN_MSG_MAP_EX(CMainDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_SIZE(OnSize)
        CHAIN_MSG_MAP(SHostDialog)
    END_MSG_MAP()
};

} // namespace SOUI
