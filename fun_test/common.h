#include <souistd.h>
#include <SouiFactory.h>
#include <commgr2.h>
#include <gtest/gtest.h>
#include <string>

#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif

static const TCHAR * kPath_SysRes = _T("/soui-sys-resource");
static const TCHAR * kPath_TestRes =_T( "/fun_test/uires");
static const TCHAR * kPath_ZipRes =  _T("/fun_test/uires.zip");
static const char *  kZip_Psw = "souizip";

#define UM_IPC  (WM_USER+100)
#define UM_IPC2  (WM_USER+101)

inline std::tstring getSourceDir() {
    TCHAR szPath[MAX_PATH];
    GetModuleFileName(NULL, szPath, MAX_PATH);
    //if run on remote linux machine, path should like /home/user/.vs/pathname/xxx/out/build/Linux-GCC-Debug/bin
    //convert this path to /home/user/.vs/pathname/xxx/src
    TCHAR* pdotvs = _tcsstr(szPath, _T(".vs"));
    if (pdotvs) {
        //run on remote linux machine
        TCHAR* out = _tcsstr(szPath, _T("out"));
        SASSERT(out);
        _tcscpy(out, _T("src"));
        return szPath;
    }
    else {
#ifdef _WIN32
		TCHAR* build = _tcsstr(szPath, _T("\\out"));
		if(!build)
		{
			GetCurrentDirectory(MAX_PATH,szPath);
			return szPath;
		}
#else
        TCHAR* build = _tcsstr(szPath, _T("/build/bin"));
        SASSERT(build);
#endif
        build[0] = 0;
        return szPath;
    }
}


inline HBITMAP LoadPng(const TCHAR* path) {
#ifdef _WIN32
    SOUI::SComMgr2 comMgr(_T("imgdecoder-stb"));
#else
    SOUI::SComMgr2 comMgr(_T("libimgdecoder-stb"));
#endif
    SOUI::SAutoRefPtr<SOUI::IImgDecoderFactory> decoder;
    EXPECT_EQ(comMgr.CreateImgDecoder((IObjRef**)&decoder), TRUE);
    if (!decoder) {
        printf("load image decoder failed!\n");
        return 0;
    }
    SOUI::SAutoRefPtr<SOUI::IImgX> img;
    decoder->CreateImgX(&img);
#ifdef _UNICODE
    EXPECT_TRUE(img->LoadFromFileW(path));
#else
    EXPECT_TRUE(img->LoadFromFileA(path));
#endif

    SOUI::IImgFrame* pFrame = img->GetFrame(0);
    if (!pFrame) {
        printf("get frame failed!\n");
        return 0;
    }
    UINT wid, hei;
    pFrame->GetSize(&wid, &hei);
    BITMAPINFO bmi;
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = wid;
#ifdef _WIN32
    bmi.bmiHeader.biHeight = -(int)hei; // top-down image 
#else
    bmi.bmiHeader.biHeight = hei; // top-down image 
#endif//_WIN32
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = 0;

    HDC hdc = GetDC(0);
    HBITMAP bmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, NULL, 0, 0);
    ReleaseDC(0, hdc);

    if (bmp) {
        UpdateDIBPixmap(bmp, wid, hei, 32, wid * 4, pFrame->GetPixels());
    }
    return bmp;
}