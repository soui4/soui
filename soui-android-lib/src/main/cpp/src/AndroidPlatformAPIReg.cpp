#include "platform_api.h"
#include "AndroidPlatformAPI.h"

#include <wnd.h>
#include <class.h>
#include <commctrl.h>
#include <winuser.h>

#include <algorithm>
#include <helper/slog.h>

#define kLogTag "AndroidPlatformAPIReg"

extern ATOM RegisterAndroidEdit();

static UINT_PTR androidWindow_createWindow(UINT_PTR hParent, const char* pszClsName, const char*pszTitle, DWORD dwStyle,DWORD dwExStyle, int x, int y, int nWidth, int nHeight,
                                           UINT_PTR hMenu, UINT_PTR hInstance, LPVOID lpParam) {
    return AndroidPlatformAPI::instance().createWindow(hParent,pszClsName,pszTitle,dwStyle,dwExStyle, x, y, nWidth, nHeight, hMenu, hInstance, lpParam);
}

static BOOL androidWindow_destroyWindow(UINT_PTR hWnd) {
    return AndroidPlatformAPI::instance().destroyWindow(hWnd);
}

static UINT_PTR androidWindow_getWindow(UINT_PTR hWnd,int code){
    return AndroidPlatformAPI::instance().getWindow(hWnd,code);
}

static BOOL androidWindow_moveWindow(HWND hWnd, int x, int y, int nWidth, int nHeight, BOOL bRepaint) {
    return AndroidPlatformAPI::instance().moveWindow(hWnd, x, y, nWidth, nHeight, bRepaint);
}

static BOOL androidWindow_setWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags) {
    return AndroidPlatformAPI::instance().setWindowPos(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
}

static BOOL androidWindow_setWindowSize(HWND hWnd, int cx, int cy) {
    return AndroidPlatformAPI::instance().setWindowSize(hWnd, cx, cy);
}

static BOOL androidWindow_setWindowPosEx(HWND hWnd, int x, int y) {
    return AndroidPlatformAPI::instance().setWindowPosEx(hWnd, x, y);
}

static void androidWindow_invalidRect(HWND hWnd, LPCRECT prc){
    AndroidPlatformAPI::instance().invalidateRect(hWnd, prc->left, prc->top, prc->right,
                                                  prc->bottom);
}

static BOOL androidWindow_isWindowVisible(HWND hWnd) {
    return AndroidPlatformAPI::instance().isWindowVisible(hWnd);
}

static BOOL androidWindow_showWindow(HWND hWnd, int nCmdShow) {
    return AndroidPlatformAPI::instance().showWindow(hWnd, nCmdShow);
}

static BOOL androidWindow_enableWindow(HWND hWnd, BOOL bEnable) {
    return AndroidPlatformAPI::instance().enableWindow(hWnd, bEnable);
}

static BOOL androidWindow_isWindowEnabled(HWND hWnd) {
    return AndroidPlatformAPI::instance().isWindowEnabled(hWnd);
}

static HWND androidWindow_getForegroundWindow() {
    return 0;
}

static BOOL androidWindow_setForegroundWindow(HWND hWnd) {
    return TRUE;
}

static HWND androidWindow_getFocus() {
    return AndroidPlatformAPI::instance().getFocus();
}

static HWND androidWindow_setFocus(HWND hWnd) {
    return AndroidPlatformAPI::instance().setFocus((UINT_PTR)hWnd);
}

static int androidWindow_getScreenWidth(int screenIndex) {
    return AndroidPlatformAPI::instance().getScreenWidth();
}

static int androidWindow_getScreenHeight(int screenIndex) {
    return AndroidPlatformAPI::instance().getScreenHeight();
}

static void androidWindow_getWorkArea(HMONITOR hMonitor, RECT *prc) {
    if (!prc)
        return;
    int width = AndroidPlatformAPI::instance().getScreenWidth();
    int height = AndroidPlatformAPI::instance().getScreenHeight();
    *prc = {0, 0, width, height};
}

static UINT_PTR androidWindow_setTimer(UINT_PTR hWnd, UINT_PTR id, UINT uElapse, TIMERPROC proc) {
    return AndroidPlatformAPI::instance().setTimer(hWnd, id, uElapse, proc);
}

static BOOL androidWindow_killTimer(UINT_PTR hWnd, UINT_PTR id) {
    return AndroidPlatformAPI::instance().killTimer(hWnd, id);
}

static BOOL androidWindow_killWindowTimers(UINT_PTR hWnd) {
    return AndroidPlatformAPI::instance().killWindowTimers(hWnd);
}

static UINT_PTR androidWindow_setCapture(UINT_PTR hWnd){
    return AndroidPlatformAPI::instance().setCapture(hWnd);
}

static BOOL androidWindow_releaseCapture(){
    return AndroidPlatformAPI::instance().releaseCapture();
}

static void androidWindow_postMessage() {
    AndroidPlatformAPI::instance().postMessage();
}

static int androidWindow_getDpi() {
    return AndroidPlatformAPI::instance().getDpi();
}

static BOOL androidWindow_getCursorPos(LPPOINT ppt){
    return AndroidPlatformAPI::instance().getCursorPos(ppt);
}

static DWORD androidWindow_getMouseButtons(void) {
    return AndroidPlatformAPI::instance().getMouseButtons();
}

static UINT androidWindow_getRawInputDeviceList(PRAWINPUTDEVICELIST pRawInputDeviceList,PUINT puiNumDevices,UINT cbSize){
    return AndroidPlatformAPI::instance().getRawInputDeviceList(pRawInputDeviceList, puiNumDevices,
                                                                cbSize);
}

static UINT androidWindow_getRawInputDeviceInfoA(HRAWINPUT hDevice, UINT uiCommand, LPVOID pData, PUINT pcbSize)
{
    return AndroidPlatformAPI::instance().getRawInputDeviceInfoA(hDevice, uiCommand, pData, pcbSize);
}

static UINT androidWindow_getRawInputDeviceInfoW(HRAWINPUT hDevice, UINT uiCommand, LPVOID pData, PUINT pcbSize)
{
    return AndroidPlatformAPI::instance().getRawInputDeviceInfoW(hDevice, uiCommand, pData, pcbSize);
}

static BOOL androidWindow_showSoftKeyboard(HWND hWnd, BOOL bShow){
    return AndroidPlatformAPI::instance().showSoftKeyboard(hWnd,bShow);
}

// Clipboard API wrappers
static BOOL androidClipboard_openClipboard(HWND hWndNewOwner) {
    return AndroidPlatformAPI::instance().clipboardOpen(hWndNewOwner);
}

static BOOL androidClipboard_closeClipboard() {
    return AndroidPlatformAPI::instance().clipboardClose();
}

static BOOL androidClipboard_emptyClipboard() {
    return AndroidPlatformAPI::instance().clipboardEmpty();
}

static HANDLE androidClipboard_getClipboardData(UINT uFormat) {
    return AndroidPlatformAPI::instance().clipboardGetData(uFormat);
}

static HANDLE androidClipboard_setClipboardData(UINT uFormat, HANDLE hMem) {
    return AndroidPlatformAPI::instance().clipboardSetData(uFormat, hMem);
}

static BOOL androidClipboard_isClipboardFormatAvailable(UINT format) {
    return AndroidPlatformAPI::instance().clipboardIsFormatAvailable(format);
}

static UINT androidClipboard_registerClipboardFormat(LPCSTR pszName) {
    return AndroidPlatformAPI::instance().clipboardRegisterFormat(pszName);
}

static HWND androidClipboard_getClipboardOwner() {
    return AndroidPlatformAPI::instance().clipboardGetOwner();
}

static BOOL androidClipboard_hasFormat(UINT format) {
    return AndroidPlatformAPI::instance().clipboardHasFormat(format);
}

// Audio API wrapper
static BOOL androidAudio_playSound(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound) {
    return AndroidPlatformAPI::instance().playSound(pszSound, hmod, fdwSound);
}

// Path API wrapper
static DWORD androidPath_getTempPathA(DWORD nBufferLength, LPSTR lpBuffer) {
    return AndroidPlatformAPI::instance().getTempPathA(nBufferLength, lpBuffer);
}

static BOOL androidPath_getSpecialFolderPathA(HWND hwndOwner, LPSTR lpszPath, int nFolder, BOOL fCreate) {
    return AndroidPlatformAPI::instance().getSpecialFolderPathA(hwndOwner, lpszPath, nFolder, fCreate);
}

extern "C" void RegisterAndroidPlatformAPI() {
    PlatformAPI api = {};
    api.version = PLATFORM_API_VERSION;

    api.window.getWindow = androidWindow_getWindow;
    api.window.createWindow = androidWindow_createWindow;
    api.window.destroyWindow = androidWindow_destroyWindow;
    api.window.setTimer = androidWindow_setTimer;
    api.window.killTimer = androidWindow_killTimer;
    api.window.killWindowTimers = androidWindow_killWindowTimers;
    api.window.moveWindow = androidWindow_moveWindow;
    api.window.setWindowPos = androidWindow_setWindowPos;
    api.window.setWindowSize = androidWindow_setWindowSize;
    api.window.invalidRect = androidWindow_invalidRect;
    api.window.setWindowPosEx = androidWindow_setWindowPosEx;
    api.window.isWindowVisible = androidWindow_isWindowVisible;
    api.window.showWindow = androidWindow_showWindow;
    api.window.enableWindow = androidWindow_enableWindow;
    api.window.isWindowEnabled = androidWindow_isWindowEnabled;
    api.window.getForegroundWindow = androidWindow_getForegroundWindow;
    api.window.setForegroundWindow = androidWindow_setForegroundWindow;
    api.window.getFocus = androidWindow_getFocus;
    api.window.setFocus = androidWindow_setFocus;
    api.window.getScreenWidth = androidWindow_getScreenWidth;
    api.window.getScreenHeight = androidWindow_getScreenHeight;
    api.window.getWorkArea = androidWindow_getWorkArea;
    api.window.setCapture = androidWindow_setCapture;
    api.window.releaseCapture = androidWindow_releaseCapture;
    api.window.postMessage = androidWindow_postMessage;
    api.window.getDpi = androidWindow_getDpi;
    api.window.getCursorPos = androidWindow_getCursorPos;
    api.window.getMouseButtons = androidWindow_getMouseButtons;
    api.window.getRawInputDeviceList = androidWindow_getRawInputDeviceList;
    api.window.getRawInputDeviceInfoA = androidWindow_getRawInputDeviceInfoA;
    api.window.getRawInputDeviceInfoW = androidWindow_getRawInputDeviceInfoW;
    api.window.showSoftKeyboard = androidWindow_showSoftKeyboard;

    // Clipboard APIs
    api.clipboard.openClipboard = androidClipboard_openClipboard;
    api.clipboard.closeClipboard = androidClipboard_closeClipboard;
    api.clipboard.emptyClipboard = androidClipboard_emptyClipboard;
    api.clipboard.getClipboardData = androidClipboard_getClipboardData;
    api.clipboard.setClipboardData = androidClipboard_setClipboardData;
    api.clipboard.isClipboardFormatAvailable = androidClipboard_isClipboardFormatAvailable;
    api.clipboard.registerClipboardFormat = androidClipboard_registerClipboardFormat;
    api.clipboard.getClipboardOwner = androidClipboard_getClipboardOwner;
    api.clipboard.hasFormat = androidClipboard_hasFormat;

    // Audio API
    api.audio.playSound = androidAudio_playSound;

    // Path API
    api.path.getTempPathA = androidPath_getTempPathA;
    api.path.getSpecialFolderPathA = androidPath_getSpecialFolderPathA;

    PlatformAPI_Init(&api);
    SLOGI()<<"Android platform window API registered";

    const ATOM atom = RegisterAndroidEdit();
    SLOGI()<<"Register WC_EDIT class: atom="<<atom<<" (WC_EDIT=\""<<WC_EDIT<<"\")";
}

extern "C" void UnregisterAndroidPlatformAPI() {
    PlatformAPI_Deinit();
    SLOGI()<<"Android platform window API unregistered";
}
