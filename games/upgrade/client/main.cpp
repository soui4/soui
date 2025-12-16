#include "stdafx.h"
#include <SAppCfg.h>
#include "MainDlg.h"
#include "SGameTheme.h"
#include "SSpriteCard.h"

static const TCHAR * kPath_SysRes = _T("/../../../soui-sys-resource");
static const TCHAR *kPath_UiRes = _T("/uires");

using namespace SOUI;

static SStringT getSourceDir()
{
    SStringA file(__FILE__);
    file = file.Left(file.ReverseFind(PATH_SLASH));
    return S_CA2T(file);
}


int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int /*nCmdShow*/)
{
    HRESULT hRes = OleInitialize(NULL);
    SASSERT(SUCCEEDED(hRes));

    int nRet = 0;
    SApplication app(hInstance);
    // Register external window class
    app.RegisterSkinClass<SCard>();
    app.RegisterSkinClass<SSkinAni>();

    app.RegisterWindowClass<SGifPlayer>();
    SStringT appDir = app.GetAppDir();

    SAppCfg cfg;
    SStringT srcDir = getSourceDir();


    cfg.SetRender(Render_Skia)
        .SetImgDecoder(ImgDecoder_Stb)
        .SetLog(TRUE);
#ifdef _WIN32
    cfg.SetSysResPeFile(appDir + _T("/soui-sys-resource.dll"));
    cfg.SetAppResPeHandle(hInstance);
#else
    SStringT strAppRes = appDir+_T("/upgrade_uires.zip");
    if(GetFileAttributes(strAppRes) != INVALID_FILE_ATTRIBUTES)
        cfg.SetAppResZip(strAppRes, "");
    else
        cfg.SetAppResFile(srcDir + kPath_UiRes);

    SStringT strSysRes = appDir + _T("/soui-sys-resource.zip");
    if(GetFileAttributes(strSysRes) != INVALID_FILE_ATTRIBUTES)
        cfg.SetSysResZip(strSysRes, "");
    else
        cfg.SetSysResFile(srcDir + kPath_SysRes);
#endif
    if (!cfg.DoConfig(&app))
    {
        return -1;
    }
#ifndef _WIN32
    AddFontResource((srcDir + _T("/../../../simsun.ttc")).c_str());
#endif
    // Show main dialog
    {
        MyProfile *pMyProfile = new MyProfile();

        SAutoRefPtr<SGameTheme> theme(new SGameTheme(),FALSE);
        SStringT themeDir = app.GetAppDir() + _T("/upgrade/def_theme");
        if(GetFileAttributes(themeDir) == INVALID_FILE_ATTRIBUTES)
        {
            themeDir = srcDir + _T("/def_theme");
        }
        theme->Load(themeDir);

        // 加载头像
        static LPCTSTR kMyAvatar = _T("/avatar/myshow.gif");
        SStringT strAvatarPath = appDir + kMyAvatar;
        if(GetFileAttributes(strAvatarPath) == INVALID_FILE_ATTRIBUTES){
            strAvatarPath = srcDir + kMyAvatar;
        }
        FILE *f = _tfopen(strAvatarPath, _T("rb"));
        if(f){
            fseek(f, 0, SEEK_END);
            int nSize = ftell(f);
            fseek(f, 0, SEEK_SET);
            BYTE *pData = new BYTE[nSize];
            fread(pData, 1, nSize, f);
            pMyProfile->SetAvatarData(pData, nSize);
            fclose(f);
            delete[] pData;
        }

        CMainDlg dlgMain(theme);
        dlgMain.Create(GetActiveWindow());
        if(dlgMain.SendMessage(WM_INITDIALOG))
        {
            dlgMain.CenterWindow(dlgMain.m_hWnd);
            dlgMain.ShowWindow(SW_SHOWNORMAL);
            nRet = app.Run(dlgMain.m_hWnd);
        }
        delete pMyProfile;
    }

    OleUninitialize();
    return nRet;
}


#if !defined(_WIN32) || defined(__MINGW32__) 
int main(int argc, char **argv)
{
    HINSTANCE hInst = GetModuleHandle(NULL);
    return _tWinMain(hInst, 0, NULL, SW_SHOWNORMAL);
}
#endif //_WIN32