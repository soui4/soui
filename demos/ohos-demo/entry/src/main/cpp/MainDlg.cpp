/**
 * MainDlg.cpp
 *
 * SOUI 主对话框（OHOS 版本），对应 Android 端 CMainDlg。
 * 演示按钮点击计数、定时器更新运行时长等基本 SOUI 功能。
 */
#include "MainDlg.h"

#include <sstream>
#include <string>

#include <core/SModalViewSession.h>
#include <helper/slog.h>

#define kLogTag "soui-ohos-demo"

SNSBEGIN

CMainDlg::CMainDlg()
    : SHostWnd(_T("layout:dlg_main"))
    , m_clickCount(0)
    , m_elapsedSec(0)
    , m_themeColorIdx(0)
{
}

void CMainDlg::OnInit(IEvtArgs *e) {
    SLOGI() << "CMainDlg::OnInit, hWnd=" << m_hWnd;
    SetTimer(kTimerIdTick, 1000, nullptr);
    UpdateInfoText();
}

void CMainDlg::OnClose() {
    SLOGI() << "CMainDlg::OnClose";
    ::KillTimer(m_hWnd, kTimerIdTick);
    DestroyWindow();
}

void CMainDlg::OnTimer(UINT_PTR idEvent) {
    if (idEvent == kTimerIdTick) {
        ++m_elapsedSec;
        UpdateInfoText();
    }
    SetMsgHandled(FALSE);
}

void CMainDlg::OnBtnClick() {
    ++m_clickCount;
    SLOGI() << "CMainDlg::OnBtnClick count=" << m_clickCount;
    UpdateInfoText();

    std::wstringstream ss;
    ss << L"来自 C++ CMainDlg：按钮已点击 " << m_clickCount << L" 次";
    ShowStatus(ss.str().c_str());
}

void CMainDlg::OnBtnTheme() {
    // 演示 SWindow::SetAttribute 动态修改控件属性（对应 Android demo OnBtnTheme）
    static const COLORREF kColors[] = {
        RGB(0x55, 0x55, 0x55),
        RGB(0xd8, 0x1b, 0x60),
        RGB(0x2e, 0x7d, 0x32),
        RGB(0x15, 0x65, 0xc0),
        RGB(0xf5, 0x7c, 0x00),
    };
    static constexpr int kColorCnt = sizeof(kColors) / sizeof(kColors[0]);
    m_themeColorIdx = (m_themeColorIdx + 1) % kColorCnt;

    SWindow *pInfo = FindChildByName(kInfoName);
    if (pInfo) {
        pInfo->SetAttribute(L"colorText",
                            SStringW().Format(L"#%06X", kColors[m_themeColorIdx]));
    }

    SStringW strStatus = SStringW().Format(L"主题色已切换为 #%06X", kColors[m_themeColorIdx]);
    ShowStatus(strStatus.c_str());
}

void CMainDlg::OnBtnModalInput() {
    // 演示 Modal View Session（对应 Android demo OnBtnModalInput）
    SModalRoot *pModal = (SModalRoot *)SApplication::getSingleton().CreateWindowByName(SModalRoot::GetClassName());
    pModal->InitFromResId("layout:model_view");
    ModalViewSessionID session_id = BeginModalViewSession(pModal);
    pModal->FindChildByName("btn_ok")->SubscribeEvent(EventCmd::EventID, [=](IEvtArgs *e) {
        (void)e;
        EndModalViewSession(session_id, IDOK);
        return TRUE;
    });
    pModal->FindChildByName("btn_cancel")->SubscribeEvent(EventCmd::EventID, [=](IEvtArgs *e) {
        (void)e;
        EndModalViewSession(session_id, IDCANCEL);
        return TRUE;
    });
    pModal->SubscribeEvent(EventExitModalView::EventID, [=](IEvtArgs *e) {
        EventExitModalView *e2 = sobj_cast<EventExitModalView>(e);
        SLOGI() << "CMainDlg::OnBtnModalInput: EventExitModalView exitCode=" << e2->exitCode;
        SStringW strStatus = SStringW().Format(L"modal input return: %d", e2->exitCode);
        ShowStatus(strStatus.c_str());
        return TRUE;
    });
}

void CMainDlg::OnBtnClose() {
    SLOGI() << "CMainDlg::OnBtnClose";
    OnClose();
}

void CMainDlg::UpdateInfoText() {
    std::wstringstream ss;
    ss << L"点击次数: " << m_clickCount << L" | 运行时长: " << m_elapsedSec << L"s";
    SWindow *pInfo = FindChildByName(kInfoName);
    if (pInfo) {
        pInfo->SetWindowTextU8(S_CW2A(ss.str().c_str(), CP_UTF8).c_str());
    }
}

void CMainDlg::ShowStatus(const wchar_t *pszText) {
    if (!pszText) return;
    SLOGI() << "CMainDlg::ShowStatus: " << S_CW2A(pszText, CP_UTF8).c_str();
    SWindow *pStatus = FindChildByName(kStatusName);
    if (pStatus) {
        pStatus->SetWindowTextU8(S_CW2A(pszText, CP_UTF8).c_str());
    }
}

SNSEND
