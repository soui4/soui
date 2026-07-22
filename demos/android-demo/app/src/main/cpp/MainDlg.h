#pragma once

#include <souistd.h>
#include <string>

SNSBEGIN

class CMainDlg : public SHostWnd {
public:
    CMainDlg();
    ~CMainDlg() override = default;

protected:
    void OnClose();
    void OnTimer(UINT_PTR idEvent);

    void OnBtnClick();
    void OnBtnTheme();
    void OnBtnClose();
    void OnBtnModalInput();
    void OnInit(IEvtArgs *e);
    EVENT_MAP_BEGIN()
        EVENT_ID_COMMAND(1001, OnBtnClick)
        EVENT_ID_COMMAND(1002, OnBtnTheme)
        EVENT_ID_COMMAND(1003, OnBtnClose)
        EVENT_ID_COMMAND(1004, OnBtnModalInput)
        EVENT_HANDLER(EventInit::EventID,OnInit)
    EVENT_MAP_END2(SHostWnd)

    BEGIN_MSG_MAP_EX(CMainDlg)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_TIMER(OnTimer)
        CHAIN_MSG_MAP(SHostWnd)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

private:
    void UpdateInfoText();
    void ShowToastAndroid(const std::wstring& text);

    int  m_clickCount;
    int  m_elapsedSec;
    int  m_themeColorIdx;

    static constexpr UINT_PTR kTimerIdTick = 1000;
    static constexpr const wchar_t* kInfoName = L"info";
};

SNSEND
