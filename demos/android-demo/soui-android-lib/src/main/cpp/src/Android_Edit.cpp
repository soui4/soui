#include <windows.h>
#include <string.h>
#include <algorithm>
#include <string>
#include "AndroidPlatformAPI.h"

static LRESULT CALLBACK AndroidEdit_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    AndroidPlatformAPI& api = AndroidPlatformAPI::instance();

    switch (message) {
        case WM_SETTEXT:
        case EM_REPLACESEL: {
            const char* str = reinterpret_cast<const char*>(lParam);
            AutoStringSlot slot(str);
            return api.sendMessage(
                    (UINT_PTR)hWnd, message, wParam, (LPARAM)slot.id());
        }

        case WM_GETTEXT: {
            char* buf = reinterpret_cast<char*>(lParam);
            const int maxCch = (int)wParam;
            if (buf == nullptr || maxCch <= 0) return 0;

            AutoStringSlot slot(NULL);
            (void)api.sendMessage((UINT_PTR)hWnd, WM_GETTEXT,
                                  (WPARAM)maxCch, (LPARAM)slot.id());
            const std::string result = slot.read();

            const size_t copyCap = (size_t)std::max(0, maxCch - 1);
            const size_t actual = std::min(result.size(), copyCap);
            if (actual > 0) memcpy(buf, result.data(), actual);
            buf[actual] = '\0';
            return (LRESULT)actual;
        }

        case EM_GETLINE: {
            char* buf = reinterpret_cast<char*>(lParam);
            const int lineNo = (int)wParam;
            const WORD maxCchW = (buf != nullptr)
                    ? *reinterpret_cast<WORD*>(buf)
                    : 0;
            const int maxCch = (int)maxCchW;
            if (buf == nullptr || maxCch <= 0) return 0;

            AutoStringSlot slot(NULL);
            const LPARAM lpSlot = ((LPARAM)(UINT)maxCch << 16) | (LPARAM)(WORD)slot.id();
            (void)api.sendMessage((UINT_PTR)hWnd, EM_GETLINE, (WPARAM)lineNo, lpSlot);
            const std::string result = slot.read();

            const size_t actual = std::min((size_t)maxCch, result.size());
            if (actual > 0) memcpy(buf, result.data(), actual);
            return (LRESULT)actual;
        }

        case WM_GETTEXTLENGTH:
            return api.sendMessage((UINT_PTR)hWnd, message, 0, 0);

        case EM_GETSEL:
        case EM_SETSEL:
        case EM_GETLINECOUNT:
        case EM_LINEINDEX:
        case EM_LINELENGTH:
        case EM_LINEFROMCHAR:
        case EM_GETMODIFY:
        case EM_SETMODIFY:
        case EM_UNDO:
        case EM_CANUNDO:
        case EM_EMPTYUNDOBUFFER:
        case EM_LIMITTEXT:
        case EM_SETREADONLY:
        case EM_SETMARGINS:
        case EM_GETMARGINS:
        case EM_SETPASSWORDCHAR:
        case EM_GETPASSWORDCHAR:
        case EM_SCROLLCARET:
        case EM_GETLIMITTEXT:
            return api.sendMessage((UINT_PTR)hWnd, message, wParam, lParam);

        default:
            break;
    }
    return ::DefWindowProc(hWnd, message, wParam, lParam);
}


ATOM RegisterAndroidEdit(){
    WNDCLASSEX cls={sizeof(cls),0};
    cls.lpfnWndProc = AndroidEdit_WndProc;
    cls.lpszClassName = WC_EDIT;
    return ::RegisterClassEx(&cls);
}
