#include "utilities.h"
#include "string/strcpcvt.h"
#include "xml/SXml.h"
#include <tchar.h>
#ifdef _WIN32
#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib, "version.lib")

#else
#include <strapi.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

int RectWidth(LPCRECT rc) {return rc->right-rc->left;}
int RectHeight(LPCRECT rc) {return rc->bottom-rc->top;}

#define HIMETRIC_PER_INCH   2540
#define MAP_PIX_TO_LOGHIM(x,ppli)   MulDiv(HIMETRIC_PER_INCH, (x), (ppli))
#define MAP_LOGHIM_TO_PIX(x,ppli)   MulDiv((ppli), (x), HIMETRIC_PER_INCH)
#ifdef _WIN32
HANDLE WINAPI LoadImageBuf(const void* buf, UINT length, UINT type,
    INT desiredx, INT desiredy, UINT loadflags);

void SHiMetricToPixel(const SIZEL * lpSizeInHiMetric, LPSIZEL lpSizeInPix)
{
	int nPixelsPerInchX;    // Pixels per logical inch along width
	int nPixelsPerInchY;    // Pixels per logical inch along height

	HDC hDCScreen = GetDC(NULL);
	nPixelsPerInchX = GetDeviceCaps(hDCScreen, LOGPIXELSX);
	nPixelsPerInchY = GetDeviceCaps(hDCScreen, LOGPIXELSY);
	ReleaseDC(NULL, hDCScreen);

	lpSizeInPix->cx = MAP_LOGHIM_TO_PIX(lpSizeInHiMetric->cx, nPixelsPerInchX);
	lpSizeInPix->cy = MAP_LOGHIM_TO_PIX(lpSizeInHiMetric->cy, nPixelsPerInchY);
}

void SPixelToHiMetric(const SIZEL * lpSizeInPix, LPSIZEL lpSizeInHiMetric)
{
	int nPixelsPerInchX;    // Pixels per logical inch along width
	int nPixelsPerInchY;    // Pixels per logical inch along height

	HDC hDCScreen = GetDC(NULL);
	nPixelsPerInchX = GetDeviceCaps(hDCScreen, LOGPIXELSX);
	nPixelsPerInchY = GetDeviceCaps(hDCScreen, LOGPIXELSY);
	ReleaseDC(NULL, hDCScreen);

	lpSizeInHiMetric->cx = MAP_PIX_TO_LOGHIM(lpSizeInPix->cx, nPixelsPerInchX);
	lpSizeInHiMetric->cy = MAP_PIX_TO_LOGHIM(lpSizeInPix->cy, nPixelsPerInchY);
}

BOOL UpdateDIBPixmap(HBITMAP bmp, int wid, int hei, int bitsPixel, int stride, CONST VOID* pjBits) {
    BITMAP bm = { 0 };
    GetObject(bmp, sizeof(bm), &bm);
    if (!bm.bmBits)
        return FALSE;
    if (bm.bmWidth != wid || bm.bmHeight != hei || bm.bmBitsPixel != bitsPixel)
        return FALSE;
    if (pjBits)
        memcpy(bm.bmBits, pjBits, hei * stride);
    else
        memset(bm.bmBits, 0, hei * stride);
    return TRUE;
}
#endif

int Str2Int(LPCWSTR src,BOOL supportHex)
{
	int nRet = 0;
	StrToIntExW(src, supportHex?STIF_SUPPORT_HEX:STIF_DEFAULT, &nRet);
	return nRet;
}

SNS::IStringA * CreateIStringA(LPCSTR src)
{
	return new SNS::SStringA(src);
}

SNS::IStringW * CreateIStringW(LPCWSTR src)
{
	return new SNS::SStringW(src);
}

SNS::IXmlDoc * CreateIXmlDoc()
{
	return new SNS::SXmlDoc();
}

#ifdef _WIN32

static int getScaleOld(HWND hWnd = NULL)
{
    HDC screen = ::GetDC(hWnd);
    int nScale = 100;
    nScale = GetDeviceCaps(screen, LOGPIXELSX) * 100 / 96;
    ReleaseDC(hWnd, screen);
    return nScale;
}

// 获取一个PE文件的version
static BOOL PEVersion(LPCTSTR pszFileName, WORD& wMajor, WORD& wMinor, WORD& wVer3, WORD& wVer4)
{
    DWORD dwResHandle;
    void* pBuf;
    DWORD dwVerInfoSize = GetFileVersionInfoSize(pszFileName, &dwResHandle);
    if (!dwVerInfoSize)
        return FALSE;
    pBuf = malloc(dwVerInfoSize);
    if (!pBuf)
        return FALSE;
    GetFileVersionInfo(pszFileName, dwResHandle, dwVerInfoSize, pBuf);
    UINT nVersionLen;
    VS_FIXEDFILEINFO* pstFileVersion;
    if (VerQueryValue(pBuf, _T("\\"), (void**)&pstFileVersion, &nVersionLen) && nVersionLen >= sizeof(VS_FIXEDFILEINFO))
    {
        wVer4 = LOWORD(pstFileVersion->dwFileVersionLS);
        wVer3 = HIWORD(pstFileVersion->dwFileVersionLS);
        wMinor = LOWORD(pstFileVersion->dwFileVersionMS);
        wMajor = HIWORD(pstFileVersion->dwFileVersionMS);
    }
    free(pBuf);
    return TRUE;
}

static BOOL IsVerOrGreater(WORD wVers[4], WORD wMajor, WORD wMinor, WORD wSpBuild = 0)
{
    if (wVers[0] < wMajor)
        return FALSE;
    if (wVers[0] > wMajor)
        return TRUE;
    if (wVers[1] < wMinor)
        return FALSE;
    if (wVers[1] > wMinor)
        return TRUE;

    return wVers[2] >= wSpBuild;
}

int GetWindowScale(HWND hWnd)
{
    WORD wVers[4];
    PEVersion(_T("ntdll.dll"), wVers[0], wVers[1], wVers[2], wVers[3]);
    // win7
    if (!IsVerOrGreater(wVers, 6, 1, 7600))
        return 100;
    int nScale = 100;
    // For Win10 1607
    if (IsVerOrGreater(wVers, 10, 0, 14955))
    {
        HMODULE hModule = LoadLibrary(_T("User32.dll"));
        if (hModule)
        {
            typedef UINT(WINAPI* FunGetDpiForWindow)(HWND);

            FunGetDpiForWindow GetDpiForWindow = (FunGetDpiForWindow)GetProcAddress(hModule, "GetDpiForWindow");
            if (GetDpiForWindow)
            {
                UINT dpi = GetDpiForWindow(hWnd);
                nScale = dpi * 100 / 96;
            }
            FreeLibrary(hModule);
        }
    }
    // for win 8.1 or later
    else if (IsVerOrGreater(wVers, 6, 3))
    {
        HMODULE hModule = LoadLibrary(_T("Shcore.dll"));
        if (hModule)
        {
            RECT winrc;
            GetWindowRect(hWnd, &winrc);
            HMONITOR hMonitor = MonitorFromRect(&winrc, MONITOR_DEFAULTTONEAREST);
            UINT dpiX, dpiY;
            typedef enum _MONITOR_DPI_TYPE
            {
                MDT_EFFECTIVE_DPI = 0,
                MDT_ANGULAR_DPI = 1,
                MDT_RAW_DPI = 2,
                MDT_DEFAULT = MDT_EFFECTIVE_DPI
            } MONITOR_DPI_TYPE;
            typedef HRESULT(WINAPI* FunGetDpiForMonitor)(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*);

            FunGetDpiForMonitor GetDpiForMonitor = (FunGetDpiForMonitor)GetProcAddress(hModule, "GetDpiForMonitor");
            if (GetDpiForMonitor)
            {
                HRESULT hr = GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
                if (SUCCEEDED(hr))
                {
                    nScale = dpiX * 100 / 96;
                }
            }
            FreeLibrary(hModule);
        }
    }
    else
    {
        nScale = getScaleOld(hWnd);
    }
    return nScale;
}


static int CALLBACK DefFontsEnumProc(CONST LOGFONT* lplf,    // logical-font data
    CONST TEXTMETRIC* lptm, // physical-font data
    DWORD dwType,           // font type
    LPARAM lpData           // application-defined data
)
{
    BOOL* pbValidFont = (BOOL*)lpData;
    *pbValidFont = TRUE;
    return 0; // stop enum.
}

BOOL HasFont(LPCTSTR fontName) {
    HDC hdc = GetDC(NULL);
    BOOL bValidFont = FALSE;
    EnumFonts(hdc, fontName, DefFontsEnumProc, (LPARAM)&bValidFont);
    ReleaseDC(NULL, hdc);
    return bValidFont;
}

BOOL IsSupportMaxMove()
{
    OSVERSIONINFOEX OSVerInfo;
    BOOL bOsVersionInfoEx;

    ::ZeroMemory(&OSVerInfo, sizeof(OSVERSIONINFOEX));

    // Get the OS Version Information
    OSVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    bOsVersionInfoEx = ::GetVersionEx((OSVERSIONINFO*)&OSVerInfo);
    if (!(bOsVersionInfoEx))
    {
        OSVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if (!::GetVersionEx((OSVERSIONINFO*)&OSVerInfo))
        {
            return FALSE;
        }
    }

    /**
     * Does it support Windows 8.1,Windows 8,Windows 7,Windows Vista ?
     * I just ignore the Windows Server System
     * if you wanna get more information, you can click the following link:
     *			http://msdn.microsoft.com/en-us/library/ms724833.aspx
     */
    if (OSVerInfo.wProductType == VER_NT_WORKSTATION && OSVerInfo.dwMajorVersion == 6)
    {
        return TRUE;
    }
    return FALSE;
}

long file_length_A(const char* path) {
    WIN32_FIND_DATAA wfd;
    HANDLE hf = FindFirstFileA(path, &wfd);
    if (INVALID_HANDLE_VALUE == hf)
        return 0;
    FindClose(hf);
    return wfd.nFileSizeLow;
}

long file_length_W(const wchar_t* path) {
    WIN32_FIND_DATAW wfd;
    HANDLE hf = FindFirstFileW(path, &wfd);
    if (INVALID_HANDLE_VALUE == hf)
        return 0;
    FindClose(hf);
    return wfd.nFileSizeLow;
}

#else

int GetWindowScale(HWND hWnd)
{
    int dpi = GetDpiForWindow(hWnd);
    return dpi*100/96;
}

BOOL HasFont(LPCTSTR fontName) {
    return TRUE;
}

BOOL IsSupportMaxMove() {
    return FALSE;
}

long file_length(const char* path)
{
    struct stat st;
    int fd = stat(path, &st);
    if (S_ISDIR(st.st_mode))
        return 0;
    return st.st_size;
}

#endif//_WIN32


HANDLE LoadIconFromMemory(const void* buf, UINT cbSize, BOOL fIcon, int width, int height, UINT cFlag) {
    UINT type = fIcon ? IMAGE_ICON : IMAGE_CURSOR;
    return LoadImageBuf(buf, cbSize, type, width, height, cFlag);
}


BOOL IsFilePathValid(LPCTSTR strPath) {
    DWORD dwAttr = ::GetFileAttributes(strPath);
    if(dwAttr == INVALID_FILE_ATTRIBUTES)
        return FALSE;
    return (dwAttr & FILE_ATTRIBUTE_DIRECTORY)==0;
}
