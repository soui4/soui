#include "MainDlg.h"

#include <jni.h>
#include <sstream>
#include <string>

#include "AndroidPlatformAPI.h"
#include <helper/slog.h>

#define kLogTag "maindlg"

SNSBEGIN

CMainDlg::CMainDlg()
    : SHostWnd("layout:dlg_main")
    , m_clickCount(0)
    , m_elapsedSec(0)
    , m_themeColorIdx(0)
{
}

void CMainDlg::OnInit(IEvtArgs *e){
    SLOGI()<<"CMainDlg::OnInit, hWnd="<<m_hWnd;
    SetTimer(kTimerIdTick, 1000, nullptr);
    UpdateInfoText();
}

void CMainDlg::OnClose()
{
    SLOGI()<<"CMainDlg::OnClose";
    ::KillTimer(m_hWnd, kTimerIdTick);
    DestroyWindow();
}

void CMainDlg::OnTimer(UINT_PTR idEvent)
{
    if (idEvent == kTimerIdTick) {
        ++m_elapsedSec;
        UpdateInfoText();
    }
    SetMsgHandled(FALSE); // 让 SHostWnd 默认逻辑也响应其它 timer id
}

void CMainDlg::OnBtnModalInput() {
    BOOL hasKeyboard = IsPhysicalKeyboardPresent();
    SLOGI()<<"hasKeyboard="<<hasKeyboard;
    SModalRoot *pModal = (SModalRoot*)SApplication::getSingleton().CreateWindowByName(SModalRoot::GetClassName());
    pModal->InitFromResId("layout:model_view");
    ModalViewSessionID session_id = BeginModalViewSession(pModal);
    pModal->FindChildByName("btn_ok")->SubscribeEvent(EventCmd::EventID, [=](IEvtArgs *e){
        EndModalViewSession(session_id,IDOK);
        return TRUE;
    });
    pModal->FindChildByName("btn_cancel")->SubscribeEvent(EventCmd::EventID, [=](IEvtArgs *e){
        EndModalViewSession(session_id,IDCANCEL);
        return TRUE;
    });
    pModal->SubscribeEvent(EventExitModalView::EventID,[=](IEvtArgs *e){
        EventExitModalView *e2 = sobj_cast<EventExitModalView>(e);
        SLOGI()<<"OnTest:EventExitModalView:"<<e2->exitCode;
        SStringW strToast=SStringW().Format(L"modal input return:%d",e2->exitCode);
        ShowToastAndroid(strToast.c_str());
        return TRUE;
    });
}

void CMainDlg::OnBtnClick()
{
    ++m_clickCount;
    UpdateInfoText();
    std::wostringstream oss;
    oss << L"来自 C++ CMainDlg：按钮已点击 " << m_clickCount << L" 次";
    ShowToastAndroid(oss.str());
}

void CMainDlg::OnBtnTheme()
{
    static const COLORREF kColors[] = {
        RGB(0x55, 0x55, 0x55),
        RGB(0xd8, 0x1b, 0x60),
        RGB(0x2e, 0x7d, 0x32),
        RGB(0x15, 0x65, 0xc0),
        RGB(0xf5, 0x7c, 0x00),
    };
    static constexpr int kColorCnt = sizeof(kColors) / sizeof(kColors[0]);
    m_themeColorIdx = (m_themeColorIdx + 1) % kColorCnt;

    SWindow* pInfo = FindChildByName(kInfoName);
    if (pInfo) {
        pInfo->SetAttribute(L"colorText",
                            SStringW().Format(L"#%06X", kColors[m_themeColorIdx]));
    }
}

void CMainDlg::OnBtnClose()
{
    OnClose();
}

void CMainDlg::UpdateInfoText()
{
    SWindow* pInfo = FindChildByName(kInfoName);
    if (!pInfo) return;

    SStringT txt;
    txt.Format(_T("点击次数: %d | 运行时长: %ds"), m_clickCount, m_elapsedSec);
    pInfo->SetWindowText(txt);
}

void CMainDlg::ShowToastAndroid(const std::wstring& text)
{
    JNIEnv* env = AndroidPlatformAPI::instance().getJNIEnv();
    jobject ctx = AndroidPlatformAPI::instance().getApplicationContext();
    if (!env || !ctx) {
        SLOGE()<<"ShowToastAndroid: env/ctx is null";
        return;
    }

    // std::wstring(UTF-16 on Win / wchar_t=4 on Android) → jstring(UTF-16)
    jstring jText;
    SStringA utf8 = S_CW2A(text.c_str(),CP_UTF8);
    jText = env->NewStringUTF(utf8.c_str());
    // Toast.makeText(ctx, text, Toast.LENGTH_SHORT).show()
    jclass clsToast = env->FindClass("android/widget/Toast");
    if (!clsToast) { SLOGE()<<"Toast class missing"; return; }
    jmethodID midMakeText = env->GetStaticMethodID(
        clsToast, "makeText",
        "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;");
    jmethodID midShow = env->GetMethodID(clsToast, "show", "()V");
    if (!midMakeText || !midShow) { SLOGE()<<"Toast method missing"; return; }

    const jint LENGTH_SHORT = 0;
    jobject toast = env->CallStaticObjectMethod(
        clsToast, midMakeText, ctx, jText, LENGTH_SHORT);
    if (toast) {
        env->CallVoidMethod(toast, midShow);
        env->DeleteLocalRef(toast);
    }

    env->DeleteLocalRef(clsToast);
    env->DeleteLocalRef(jText);
}

SNSEND
