//
// Created by setou on 2026/8/25.
// 注册 OhosPlatformAPI 到全局 g_platformAPI。
// 完全对应 Android 端 AndroidPlatformAPIReg.cpp 结构：
//   每个包装函数 -> OhosPlatformAPI::instance().xxx()
//
#include "OhosPlatformAPI.h"
#include "platform_api.h"
#include <helper/slog.h>

#define kLogTag "OhosPlatformAPIReg"

extern "C" PlatformAPI g_platformAPI;

// ========= Window API wrappers ========= 

static UINT_PTR ohosWin_createWindow(UINT_PTR hParent, LPCSTR className, LPCSTR title,
                                     DWORD style, DWORD exStyle, int x, int y, int w, int h,
                                     UINT_PTR hMenu, UINT_PTR hInstance, LPVOID lpParam)
{
    (void)hInstance; (void)lpParam;
    UINT_PTR screenId = OhosPlatformAPI::instance().getActiveScreenId(); // 激活屏幕路由（HWND_DESKTOP 顶层窗口）
    return (UINT_PTR)OhosPlatformAPI::instance().createWindow((HWND)hParent, screenId, className, title,
                                                               style, exStyle, x, y, w, h, (HMENU)hMenu);
}
static BOOL ohosWin_destroyWindow(UINT_PTR hWnd) {
    return OhosPlatformAPI::instance().destroyWindow((HWND)hWnd);
}
static HWND ohosWin_getWindow(HWND hWnd, int code) {
    return OhosPlatformAPI::instance().getWindow(hWnd, code);
}
static BOOL ohosWin_moveWindow(HWND hWnd, int x, int y, int nWidth, int nHeight, BOOL bRepaint) {
    return OhosPlatformAPI::instance().moveWindow(hWnd, x, y, nWidth, nHeight, bRepaint);
}
static BOOL ohosWin_setWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y,
                                 int cx, int cy, UINT uFlags) {
    return OhosPlatformAPI::instance().setWindowPos(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
}
static BOOL ohosWin_setWindowSize(HWND hWnd, int cx, int cy) {
    return OhosPlatformAPI::instance().setWindowSize(hWnd, cx, cy);
}
static BOOL ohosWin_setWindowPosEx(HWND hWnd, int x, int y) {
    return OhosPlatformAPI::instance().setWindowPosEx(hWnd, x, y);
}
static void ohosWin_invalidRect(HWND hWnd, LPCRECT prc) {
    if (!prc) return;
    OhosPlatformAPI::instance().invalidateRect(hWnd, prc->left, prc->top, prc->right, prc->bottom);
}
static BOOL ohosWin_isWindowVisible(HWND hWnd) {
    return OhosPlatformAPI::instance().isWindowVisible(hWnd);
}
static BOOL ohosWin_showWindow(HWND hWnd, int nCmdShow) {
    return OhosPlatformAPI::instance().showWindow(hWnd, nCmdShow);
}
static BOOL ohosWin_enableWindow(HWND hWnd, BOOL bEnable) {
    return OhosPlatformAPI::instance().enableWindow(hWnd, bEnable);
}
static BOOL ohosWin_isWindowEnabled(HWND hWnd) {
    return OhosPlatformAPI::instance().isWindowEnabled(hWnd);
}
static HWND ohosWin_getForegroundWindow() {
    return 0; // OHOS 无前台窗口概念（对应 Android stub）
}
static BOOL ohosWin_setForegroundWindow(HWND hWnd) {
    (void)hWnd;
    return TRUE; // 对应 Android stub
}
static int ohosWin_getScreenWidth(int screenIndex) {
    (void)screenIndex;
    return OhosPlatformAPI::instance().getScreenWidth();
}
static int ohosWin_getScreenHeight(int screenIndex) {
    (void)screenIndex;
    return OhosPlatformAPI::instance().getScreenHeight();
}
static void ohosWin_getWorkArea(HMONITOR hMonitor, RECT *prc) {
    (void)hMonitor;
    OhosPlatformAPI::instance().getWorkArea(prc);
}
static HWND ohosWin_setCapture(HWND hWnd) {
    return OhosPlatformAPI::instance().setCapture(hWnd);
}
static BOOL ohosWin_releaseCapture() {
    return OhosPlatformAPI::instance().releaseCapture();
}
static void ohosWin_postMessage() {
    OhosPlatformAPI::instance().postMessage();
}
static int ohosWin_getDpi() {
    return OhosPlatformAPI::instance().getDpi();
}
static BOOL ohosWin_getCursorPos(LPPOINT ppt) {
    return OhosPlatformAPI::instance().getCursorPos(ppt);
}

static DWORD ohosWin_getMouseButtons(void) {
    return OhosPlatformAPI::instance().getMouseButtons();
}
static UINT ohosWin_getRawInputDeviceList(PRAWINPUTDEVICELIST pRawInputDeviceList,
                                          PUINT puiNumDevices, UINT cbSize) {
    return OhosPlatformAPI::instance().getRawInputDeviceList(
        pRawInputDeviceList, puiNumDevices, cbSize);
}
static UINT ohosWin_getRawInputDeviceInfoA(HRAWINPUT hDevice, UINT uiCommand,
                                           LPVOID pData, PUINT pcbSize) {
    return OhosPlatformAPI::instance().getRawInputDeviceInfoA(hDevice, uiCommand, pData, pcbSize);
}
static UINT ohosWin_getRawInputDeviceInfoW(HRAWINPUT hDevice, UINT uiCommand,
                                           LPVOID pData, PUINT pcbSize) {
    return OhosPlatformAPI::instance().getRawInputDeviceInfoW(hDevice, uiCommand, pData, pcbSize);
}
static HWND ohosWin_setFocus(HWND hWnd) {
    return OhosPlatformAPI::instance().setFocus(hWnd);
}
static HWND ohosWin_getFocus() {
    return OhosPlatformAPI::instance().getFocus();
}
static UINT_PTR ohosWin_setTimer(UINT_PTR hWnd, UINT_PTR id, UINT uElapse, TIMERPROC proc) {
    // 完整透传 proc：proc 型定时器（STimerGenerator/输入排水等）由
    // onTimerExpired 直接回调，不能丢弃（丢弃后 hWnd==NULL 定时器消息无处投递）
    return (UINT_PTR)OhosPlatformAPI::instance().setTimer((HWND)hWnd, id, uElapse, proc);
}
static BOOL ohosWin_killTimer(UINT_PTR hWnd, UINT_PTR id) {
    return OhosPlatformAPI::instance().killTimer((HWND)hWnd, id);
}
static BOOL ohosWin_killWindowTimers(UINT_PTR hWnd) {
    OhosPlatformAPI::instance().killWindowTimers((HWND)hWnd);
    return TRUE;
}
static BOOL ohosWin_showSoftKeyboard(HWND hWnd, BOOL bShow) {
    return OhosPlatformAPI::instance().showSoftKeyboard(hWnd, bShow);
}

// ========= Clipboard API wrappers ========= 

static BOOL ohosClip_openClipboard(HWND hWndNewOwner) {
    return OhosPlatformAPI::instance().clipboardOpen(hWndNewOwner);
}
static BOOL ohosClip_closeClipboard() {
    return OhosPlatformAPI::instance().clipboardClose();
}
static BOOL ohosClip_emptyClipboard() {
    return OhosPlatformAPI::instance().clipboardEmpty();
}
static HANDLE ohosClip_getClipboardData(UINT uFormat) {
    return OhosPlatformAPI::instance().clipboardGetData(uFormat);
}
static HANDLE ohosClip_setClipboardData(UINT uFormat, HANDLE hMem) {
    return OhosPlatformAPI::instance().clipboardSetData(uFormat, hMem);
}
static BOOL ohosClip_isClipboardFormatAvailable(UINT format) {
    return OhosPlatformAPI::instance().clipboardIsFormatAvailable(format);
}
static UINT ohosClip_registerClipboardFormat(LPCSTR pszName) {
    return OhosPlatformAPI::instance().clipboardRegisterFormat(pszName);
}
static HWND ohosClip_getClipboardOwner() {
    return OhosPlatformAPI::instance().clipboardGetOwner();
}
static BOOL ohosClip_hasFormat(UINT format) {
    return OhosPlatformAPI::instance().clipboardHasFormat(format);
}

// ========= Path API wrappers ========= 

static DWORD ohosPath_getTempPathA(DWORD nBufferLength, LPSTR lpBuffer) {
    return OhosPlatformAPI::instance().getTempPathA(nBufferLength, lpBuffer);
}
static BOOL ohosPath_getSpecialFolderPathA(HWND hwndOwner, LPSTR pszPath, int nFolder, BOOL fCreate) {
    (void)hwndOwner; // 未用
    return OhosPlatformAPI::instance().getSpecialFolderPathA(
        nFolder, fCreate, pszPath, MAX_PATH);
}

// ========= Audio ========= 

static BOOL ohosAudio_playSound(LPCSTR pszSoundFile, HMODULE hMod, DWORD fdwSound) {
    return OhosPlatformAPI::instance().playSound(pszSoundFile, hMod, fdwSound);
}

extern "C" void RegisterOhosPlatformAPI() {
    SLOGI() << "RegisterOhosPlatformAPI start";
    PlatformAPI &api = g_platformAPI;

    // Window API
    api.window.createWindow              = ohosWin_createWindow;
    api.window.destroyWindow             = ohosWin_destroyWindow;
    api.window.getWindow                 = ohosWin_getWindow;
    api.window.setFocus                  = ohosWin_setFocus;
    api.window.getFocus                  = ohosWin_getFocus;
    api.window.setTimer                  = ohosWin_setTimer;
    api.window.killTimer                 = ohosWin_killTimer;
    api.window.killWindowTimers          = ohosWin_killWindowTimers;
    api.window.moveWindow                = ohosWin_moveWindow;
    api.window.setWindowPos              = ohosWin_setWindowPos;
    api.window.setWindowSize             = ohosWin_setWindowSize;
    api.window.setWindowPosEx            = ohosWin_setWindowPosEx;
    api.window.invalidRect               = ohosWin_invalidRect;
    api.window.isWindowVisible           = ohosWin_isWindowVisible;
    api.window.showWindow                = ohosWin_showWindow;
    api.window.enableWindow              = ohosWin_enableWindow;
    api.window.isWindowEnabled           = ohosWin_isWindowEnabled;
    api.window.getForegroundWindow       = ohosWin_getForegroundWindow;
    api.window.setForegroundWindow       = ohosWin_setForegroundWindow;
    api.window.getScreenWidth            = ohosWin_getScreenWidth;
    api.window.getScreenHeight           = ohosWin_getScreenHeight;
    api.window.getWorkArea               = ohosWin_getWorkArea;
    api.window.setCapture                = ohosWin_setCapture;
    api.window.releaseCapture            = ohosWin_releaseCapture;
    api.window.postMessage               = ohosWin_postMessage;
    api.window.getDpi                    = ohosWin_getDpi;
    api.window.getCursorPos              = ohosWin_getCursorPos;
    api.window.getMouseButtons           = ohosWin_getMouseButtons;
    api.window.getRawInputDeviceList     = ohosWin_getRawInputDeviceList;
    api.window.getRawInputDeviceInfoA    = ohosWin_getRawInputDeviceInfoA;
    api.window.getRawInputDeviceInfoW    = ohosWin_getRawInputDeviceInfoW;
    api.window.showSoftKeyboard          = ohosWin_showSoftKeyboard;

    // Clipboard API
    api.clipboard.openClipboard           = ohosClip_openClipboard;
    api.clipboard.closeClipboard          = ohosClip_closeClipboard;
    api.clipboard.emptyClipboard          = ohosClip_emptyClipboard;
    api.clipboard.getClipboardData        = ohosClip_getClipboardData;
    api.clipboard.setClipboardData        = ohosClip_setClipboardData;
    api.clipboard.isClipboardFormatAvailable = ohosClip_isClipboardFormatAvailable;
    api.clipboard.registerClipboardFormat = ohosClip_registerClipboardFormat;
    api.clipboard.getClipboardOwner       = ohosClip_getClipboardOwner;
    api.clipboard.hasFormat               = ohosClip_hasFormat;

    // Path API
    api.path.getTempPathA                = ohosPath_getTempPathA;
    api.path.getSpecialFolderPathA       = ohosPath_getSpecialFolderPathA;

    // Audio
    api.audio.playSound                  = ohosAudio_playSound;

    SLOGI() << "RegisterOhosPlatformAPI done, version=" << api.version;
}

// C++11 不支持泛型 lambda（auto 参数），用模板函数完成"置空全部函数指针"
namespace {
template<typename T>
inline void NullifyOhosEntry(T &p) { p = nullptr; }
}

extern "C" void UnregisterOhosPlatformAPI() {
    PlatformAPI &api = g_platformAPI;
    NullifyOhosEntry(api.window.createWindow);
    NullifyOhosEntry(api.window.destroyWindow);
    NullifyOhosEntry(api.window.getWindow);
    NullifyOhosEntry(api.window.setFocus);
    NullifyOhosEntry(api.window.getFocus);
    NullifyOhosEntry(api.window.setTimer);
    NullifyOhosEntry(api.window.killTimer);
    NullifyOhosEntry(api.window.killWindowTimers);
    NullifyOhosEntry(api.window.moveWindow);
    NullifyOhosEntry(api.window.setWindowPos);
    NullifyOhosEntry(api.window.setWindowSize);
    NullifyOhosEntry(api.window.setWindowPosEx);
    NullifyOhosEntry(api.window.invalidRect);
    NullifyOhosEntry(api.window.isWindowVisible);
    NullifyOhosEntry(api.window.showWindow);
    NullifyOhosEntry(api.window.enableWindow);
    NullifyOhosEntry(api.window.isWindowEnabled);
    NullifyOhosEntry(api.window.getForegroundWindow);
    NullifyOhosEntry(api.window.setForegroundWindow);
    NullifyOhosEntry(api.window.getScreenWidth);
    NullifyOhosEntry(api.window.getScreenHeight);
    NullifyOhosEntry(api.window.getWorkArea);
    NullifyOhosEntry(api.window.setCapture);
    NullifyOhosEntry(api.window.releaseCapture);
    NullifyOhosEntry(api.window.postMessage);
    NullifyOhosEntry(api.window.getDpi);
    NullifyOhosEntry(api.window.getCursorPos);
    NullifyOhosEntry(api.window.getMouseButtons);
    NullifyOhosEntry(api.window.getRawInputDeviceList);
    NullifyOhosEntry(api.window.getRawInputDeviceInfoA);
    NullifyOhosEntry(api.window.getRawInputDeviceInfoW);
    NullifyOhosEntry(api.window.showSoftKeyboard);
    NullifyOhosEntry(api.clipboard.openClipboard);
    NullifyOhosEntry(api.clipboard.closeClipboard);
    NullifyOhosEntry(api.clipboard.emptyClipboard);
    NullifyOhosEntry(api.clipboard.getClipboardData);
    NullifyOhosEntry(api.clipboard.setClipboardData);
    NullifyOhosEntry(api.clipboard.isClipboardFormatAvailable);
    NullifyOhosEntry(api.clipboard.registerClipboardFormat);
    NullifyOhosEntry(api.clipboard.getClipboardOwner);
    NullifyOhosEntry(api.clipboard.hasFormat);
    NullifyOhosEntry(api.path.getTempPathA);
    NullifyOhosEntry(api.path.getSpecialFolderPathA);
    NullifyOhosEntry(api.audio.playSound);
    SLOGI() << "UnregisterOhosPlatformAPI done";
}
