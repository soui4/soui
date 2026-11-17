#pragma once
#include <core/SHostWnd.h>

namespace SNS
{
namespace FluentDemo
{
inline int WindowScaleFromDpi(UINT dpi)
{
    return dpi ? MulDiv(dpi, 100, 96) : 100;
}

inline void ApplyWindowDpi(SHostWnd &host, UINT dpi = 0, LPCRECT suggested = NULL)
{
#ifdef _WIN32
    if (!dpi)
    {
        typedef UINT(WINAPI * GetWindowDpi)(HWND);
        GetWindowDpi query = reinterpret_cast<GetWindowDpi>(::GetProcAddress(::GetModuleHandleW(L"user32.dll"), "GetDpiForWindow"));
        dpi = query ? query(host.m_hWnd) : 96;
    }
    const int scale = WindowScaleFromDpi(dpi);
    const int previousScale = host.GetScale();
    if (!suggested && scale == previousScale)
        return;
    CRect bounds = host.GetWindowRect();
    if (suggested)
        bounds = *suggested;
    else
    {
        bounds.right = bounds.left + MulDiv(bounds.Width(), scale, previousScale);
        bounds.bottom = bounds.top + MulDiv(bounds.Height(), scale, previousScale);
    }
    host.SetScale(scale, &bounds);
#else
    (void)host;
    (void)dpi;
    (void)suggested;
#endif
}
}
}
