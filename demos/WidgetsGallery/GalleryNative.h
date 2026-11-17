#pragma once

#include <control/SRealWnd.h>
#include <control/SCmnCtrl.h>
#include <res.mgr/SUiDef.h>
#include <cstring>

namespace Gallery
{
// SIconWnd owns the HICON passed to SetIcon. Render the same vector skin used
// elsewhere; do not introduce a second bitmap artwork or leak GDI objects.
inline HICON CreateVectorIcon(int scale)
{
    const int size = MulDiv(32, scale, 100);
    SOUI::SAutoRefPtr<SOUI::IRenderTarget> target;
    SOUI::ISkinObj *skin = GETSKIN(L"gallery_icon_layout", scale);
    if (!skin || !GETRENDERFACTORY->CreateRenderTarget(&target, size, size))
        return NULL;
    const SOUI::CRect bounds(0, 0, size, size);
    target->BeginDraw();
    target->ClearRect(&bounds, RGBA(0, 0, 0, 0));
    skin->DrawByIndex(target, &bounds, 0);
    target->EndDraw();
    SOUI::IBitmapS *bitmap = static_cast<SOUI::IBitmapS *>(target->GetCurrentObject(SOUI::OT_BITMAP));
    const void *source = bitmap->LockPixelBits();
    if (!source)
        return NULL;
    BITMAPINFO info = {0};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = size;
    info.bmiHeader.biHeight = -size;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    void *pixels = NULL;
    HBITMAP color = CreateDIBSection(NULL, &info, DIB_RGB_COLORS, &pixels, NULL, 0);
    if (pixels)
        std::memcpy(pixels, source, static_cast<size_t>(size) * size * 4);
    bitmap->UnlockPixelBits(const_cast<void *>(source));
    HBITMAP mask = CreateBitmap(size, size, 1, 1, NULL);
    ICONINFO icon = {0};
    icon.fIcon = TRUE;
    icon.hbmColor = color;
    icon.hbmMask = mask;
    HICON result = color && mask ? CreateIconIndirect(&icon) : NULL;
    if (color)
        DeleteObject(color);
    if (mask)
        DeleteObject(mask);
    return result;
}

inline void PaintNativePreview(HWND window, HDC dc)
{
    RECT bounds;
    GetClientRect(window, &bounds);
    // SOUI colors carry alpha; native GDI COLORREF requires the high byte clear.
    HBRUSH brush = CreateSolidBrush(GETCOLOR(L"@color/gallery_subtle") & 0x00ffffff);
    FillRect(dc, &bounds, brush);
    DeleteObject(brush);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, GETCOLOR(L"@color/fluent_text") & 0x00ffffff);
    // Use the same configured family and logical size as the SOUI controls,
    // rather than an unrelated stock font. The SRealWnd supplies its scale.
    const int scale = static_cast<int>(GetWindowLongPtr(window, GWLP_USERDATA));
    SOUI::IFontPtr uiFont = GETUIDEF->GetFont(L"", scale > 0 ? scale : 100);
    HFONT font = uiFont ? CreateFontIndirect(uiFont->LogFont()) : NULL;
    HFONT previous = font ? static_cast<HFONT>(SelectObject(dc, font)) : NULL;
    const TCHAR *content = _T("Native HWND / local owner-drawn integration\nCreated and destroyed by SRealWnd events");
    RECT text = bounds;
    const int height = DrawText(dc, content, -1, &text, DT_CENTER | DT_NOPREFIX | DT_CALCRECT);
    text = bounds;
    text.top += (bounds.bottom - bounds.top - height) / 2;
    text.bottom = text.top + height;
    DrawText(dc, content, -1, &text, DT_CENTER | DT_NOPREFIX);
    if (previous)
        SelectObject(dc, previous);
    if (font)
        DeleteObject(font);
}

inline LRESULT CALLBACK PreviewWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_CREATE)
    {
        const CREATESTRUCT *creation = reinterpret_cast<const CREATESTRUCT *>(lParam);
        SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(creation->lpCreateParams));
        return 0;
    }
    if (message == WM_PAINT)
    {
        PAINTSTRUCT paint;
        HDC dc = BeginPaint(window, &paint);
        PaintNativePreview(window, dc);
        EndPaint(window, &paint);
        return 0;
    }
    if (message == WM_PRINTCLIENT)
    {
        PaintNativePreview(window, reinterpret_cast<HDC>(wParam));
        return 0;
    }
    if (message == WM_ERASEBKGND)
        return 1;
    return DefWindowProc(window, message, wParam, lParam);
}

inline HWND CreateNativePreview(HWND parent, int scale)
{
    const HINSTANCE instance = GetModuleHandle(NULL);
    const TCHAR *name = _T("SOUIWidgetsGalleryNativePreview");
    WNDCLASS windowClass = {0};
    if (!GetClassInfo(instance, name, &windowClass))
    {
        windowClass.lpfnWndProc = PreviewWindowProc;
        windowClass.hInstance = instance;
        windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        windowClass.lpszClassName = name;
        if (!RegisterClass(&windowClass))
            return NULL;
    }
    return CreateWindowEx(0, name, _T("Gallery native preview"), WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, parent, NULL, instance, reinterpret_cast<LPVOID>(static_cast<INT_PTR>(scale)));
}
} // namespace Gallery
