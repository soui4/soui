#include <souistd.h>
#include <SouiFactory.h>
#include <com-cfg.h>
#include <SGridCtrl.h>
#include <SAppCfg.h>
#include <helper/slog.h>
#include <mutex>
#include <cstdio>
#include <cstring>
#include "MainDlg.h"

// 把 SOUI 的 SLOGI() 日志同时落盘到 exe 同目录的 fundholdings_demo.log，
// 便于在本机排查网络/编码问题（SLog 默认只输出到 OutputDebugString）。
static std::mutex g_logMtx;
static void FileLogSink(const char* tag, const char* pLogStr, int level,
                        const char* file, int line, const char* fun, void* retAddr) {
    (void)tag; (void)level; (void)file; (void)line; (void)fun; (void)retAddr;
    std::lock_guard<std::mutex> lk(g_logMtx);
    FILE* f = fopen("fundholdings_demo.log", "a");
    if (f) {
        fwrite(pLogStr, 1, strlen(pLogStr), f);
        fputc('\n', f);
        fclose(f);
    }
}

static const TCHAR* kPath_SysRes = _T("/../../soui-sys-resource");
static const TCHAR* kPath_UiRes = _T("/uires");

using namespace SOUI;

static SStringT getSourceDir() {
    SStringA file(__FILE__);
    file = file.Left(file.ReverseFind(PATH_SLASH));
    return S_CA2T(file);
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int /*nCmdShow*/) {
    // 注册日志回调：SLOGI() 的每条日志同时写入 fundholdings_demo.log（线程安全，须在程序启动早期调用）
    SLog::setLogCallback(&FileLogSink);

    HRESULT hRes = OleInitialize(NULL);
    SASSERT(SUCCEEDED(hRes));

    int nRet = 0;
    SApplication app(hInstance);
    SAppCfg cfg;
    SStringT srcDir = getSourceDir();
    cfg.SetRender(Render_Skia)
        .SetImgDecoder(ImgDecoder_Stb)
        .SetAppDir(srcDir)
        .SetSysResFile(srcDir + kPath_SysRes)
        .SetAppResFile(srcDir + kPath_UiRes)
        .SetLog(TRUE);
    if (!cfg.DoConfig(&app)) {
        return -1;
    }
    app.EnableNotifyCenter(TRUE);
    // 注册外部窗口类
    app.RegisterWindowClass<SGridCtrl>();

    {
        CMainDlg dlgMain;
        dlgMain.Create(GetActiveWindow());
        dlgMain.SendMessage(WM_INITDIALOG);
        dlgMain.CenterWindow(dlgMain.m_hWnd);
        dlgMain.ShowWindow(SW_SHOWNORMAL);
        nRet = app.Run(dlgMain.m_hWnd);
    }

    OleUninitialize();
    return nRet;
}

#if !defined(_WIN32) || defined(__MINGW32__)
int main(int argc, char** argv) {
    HINSTANCE hInst = GetModuleHandle(NULL);
    return _tWinMain(hInst, 0, NULL, SW_SHOWNORMAL);
}
#endif  //_WIN32
