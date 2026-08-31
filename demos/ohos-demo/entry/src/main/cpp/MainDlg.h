/**
 * MainDlg.h
 *
 * SOUI 主对话框（OHOS 版本），对应 Android 端 CMainDlg。
 * 使用 layout:dlg_main 布局，演示按钮点击、定时器更新、编辑框等基本功能。
 */
#pragma once

#include <souistd.h>
#include <string>
#include <helper/SDpiHelper.hpp>

SNSBEGIN

class CMainDlg : public SHostWnd, public SDpiHandler<CMainDlg> {
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
        EVENT_HANDLER(EventInit::EventID, OnInit)
    EVENT_MAP_END2(SHostWnd)

    BEGIN_MSG_MAP_EX(CMainDlg)
        CHAIN_MSG_MAP(SDpiHandler<CMainDlg>)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_TIMER(OnTimer)
        CHAIN_MSG_MAP(SHostWnd)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

private:
    void UpdateInfoText();
    /** 在布局的 status 文本控件上展示一条测试结果（OHOS 无原生 Toast，等价 Android Toast 语义） */
    void ShowStatus(const wchar_t *pszText);

    int  m_clickCount;
    int  m_elapsedSec;
    int  m_themeColorIdx;

    static constexpr UINT_PTR kTimerIdTick = 1000;
    static constexpr const wchar_t *kInfoName = L"info";
    static constexpr const wchar_t *kStatusName = L"status";
};

SNSEND
