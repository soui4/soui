#include "MainDlg.h"
#include "GalleryButton.h"
#include "fluent/FluentTheme.h"
#include <SAppCfg.h>

#define INIT_R_DATA
#include "res/resource.h"

namespace
{
std::wstring OptionValue(const std::wstring &options, const wchar_t *key)
{
    const std::wstring prefix = std::wstring(L"--") + key + L"=";
    size_t start = options.find(prefix);
    if (start == std::wstring::npos)
        return std::wstring();
    start += prefix.size();
    return options.substr(start, options.find_first_of(L" \t", start) - start);
}

void OnGalleryThemeChanged(void *context)
{
    static_cast<GalleryMainDialog *>(context)->OnThemeChanged();
}

class OleScope {
  public:
    OleScope()
        : result(OleInitialize(NULL))
    {
    }
    ~OleScope()
    {
        if (SUCCEEDED(result))
            OleUninitialize();
    }
    HRESULT result;
};
} // namespace

static SStringT GalleryResourceRoot(const SStringT &appDirectory)
{
#ifdef __APPLE__
    // .app bundle 内可执行文件在 Contents/MacOS，资源由 CMake 拷到 bundle 根目录。
    char bundlePath[1024] = {0};
    if (GetAppleBundlePath(bundlePath, sizeof(bundlePath)))
        return S_CA2T(bundlePath);
#endif
    return appDirectory;
}

int WINAPI _tWinMain(HINSTANCE instance, HINSTANCE, LPTSTR arguments, int)
{
    const std::wstring options = arguments ? S_CT2W(arguments).c_str() : L"";
    OleScope ole;
    if (FAILED(ole.result))
        return EXIT_FAILURE;
    SApplication app(instance);
    SAppCfg config;
    const SStringT resRoot = GalleryResourceRoot(app.GetAppDir());
#ifdef _WIN32
    config.SetRender(Render_Gdi);
#else
    config.SetRender(Render_Skia);
#endif
#ifdef ENABLE_BUILD_RESOURCE
    // CMake 已把 uires 与系统资源都编进当前 exe：两者都从 PE 文件读取，
    // 单文件拷走即可运行。
    config.SetAppResPeHandle(instance);
    config.SetSysResPeHandle(instance);
#else
#ifdef _WIN32
    config.SetSysResPeFile(_T("soui-sys-resource.dll"));
    config.SetAppResFile(S_CA2T(SOUI_GALLERY_RESOURCE_DIR, CP_UTF8));
#else
    config.SetSysResZip(resRoot + _T("/soui-sys-resource.zip"));
    config.SetAppResFile(resRoot + _T("/WidgetsGallery-res"));
#endif
#endif
    config.SetImgDecoder(ImgDecoder_Stb).SetLog(TRUE);
    if (!config.DoConfig(&app))
        return EXIT_FAILURE;
    app.RegisterWindowClass<GalleryButton>();
    app.RegisterWindowClass<GalleryTabCtrl>();
    app.RegisterWindowClass<GalleryRebar>();
    app.InitXmlNamedID((const LPCWSTR *)&R.name, (const int *)&R.id, sizeof(R.id) / sizeof(int));

    FluentDemo::Theme theme;
    if (!theme.Load())
        return EXIT_FAILURE;
    GalleryMainDialog gallery;
    if (!gallery.Create(NULL) || !theme.Bind(gallery))
    {
        gallery.DestroyWindow();
        return EXIT_FAILURE;
    }
    theme.SetChangedCallback(OnGalleryThemeChanged, &gallery);
    gallery.SendMessage(WM_INITDIALOG);
    const std::wstring page = OptionValue(options, L"page");
    for (int i = 0; i < Gallery::CategoryCount; ++i)
        if (page == Gallery::Categories()[i].key)
            gallery.OnNavigate(Gallery::NavigationId + i);
    if (options.find(L"--dark") != std::wstring::npos)
        theme.Switch(TRUE);
    if (options.find(L"--source") != std::wstring::npos)
        gallery.OnSource();
    const std::wstring section = OptionValue(options, L"section");
    if (!section.empty())
        gallery.ShowSection(section.c_str());
    if (options.find(L"--modal") != std::wstring::npos)
        gallery.OnModal();
    gallery.CenterWindow();
    gallery.ShowWindow(options.find(L"--snapshot") != std::wstring::npos ? SW_SHOWNOACTIVATE : SW_SHOWNORMAL);
    return app.Run(gallery.m_hWnd);
}

#if !defined(_WIN32) || defined(__MINGW32__)
int main(int argc, char **argv)
{
    SStringA arguments;
    for (int i = 1; i < argc; ++i)
        arguments += SStringA(argv[i]) + " ";
    SStringT command = S_CA2T(arguments, CP_UTF8);
    return _tWinMain(GetModuleHandle(NULL), NULL, const_cast<LPTSTR>(command.c_str()), SW_SHOWNORMAL);
}
#endif
