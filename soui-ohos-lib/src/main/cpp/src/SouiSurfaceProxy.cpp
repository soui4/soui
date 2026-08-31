//
// SouiSurfaceProxy.cpp
//
// SOUI 自绘 Surface C++ 代理实现，完全对应 Android SouiSurfaceProxy.cpp。
// 差异：N-API 替代 JNI，napi_ref 替代 jobject GlobalRef，
//       napi_get_typedarray_info 替代 AndroidBitmap_lockPixels。
//

#include "SouiSurfaceProxy.h"
#include "OhosPlatformAPI.h"
#include <cassert>
#include <cstring>
#include <algorithm>
#include <gdi.h>
// souistd.h 提供 SOUI 公共基础类型（SXmlNode/SArray/SApplication/IMessageLoop 等），
// 并已包含 SApp.h / SMsgLoop.h / slog.h，须在依赖 SOUI 类型的代码之前引入
#include <souistd.h>

#define kLogTag "SouiSurfaceProxy"

// 从 m_buttonsDown 位图同步到 OhosPlatformAPI 的 MK_* 掩码
static inline void syncGlobalMouseButtons(int buttonsDown) {
    unsigned int mk = 0;
    if (buttonsDown & (1 << 0)) mk |= MK_LBUTTON;
    if (buttonsDown & (1 << 1)) mk |= MK_RBUTTON;
    if (buttonsDown & (1 << 2)) mk |= MK_MBUTTON;
    OhosPlatformAPI::instance().setMouseButtons(mk);
}

// SIMD 头文件（按架构条件引入）
#if defined(__aarch64__) || defined(__ARM_NEON) || defined(__ARM_NEON__)
    #define SOUI_SIMD_NEON 1
    #include <arm_neon.h>
#elif defined(__SSSE3__)
    #define SOUI_SIMD_SSSE3 1
    #include <tmmintrin.h>
    #if defined(__SSE4_1__) || defined(_M_IX86_FP) || defined(__x86_64__)
        #include <emmintrin.h>
    #endif
#endif

// ArkUI TouchType 常量
#define ARKUI_TOUCH_DOWN    0
#define ARKUI_TOUCH_UP      1
#define ARKUI_TOUCH_MOVE    2
#define ARKUI_TOUCH_CANCEL  3

// Android MotionEvent action 常量（与 android.view.MotionEvent 保持一致，对齐 Android SouiSurfaceProxy）
#define AMOTION_EVENT_ACTION_BUTTON_PRESS   11
#define AMOTION_EVENT_ACTION_BUTTON_RELEASE 12
#define AMOTION_EVENT_ACTION_SCROLL         13
#define AMOTION_EVENT_ACTION_HOVER_MOVE      7
#define AMOTION_EVENT_ACTION_HOVER_ENTER     9
#define AMOTION_EVENT_ACTION_HOVER_EXIT     10

// 通用 metaState 常量（与 Android AMETA_* 对齐）
#define AMETA_SHIFT_ON          0x0001
#define AMETA_SHIFT_LEFT_ON     0x0040
#define AMETA_SHIFT_RIGHT_ON    0x0080
#define AMETA_CTRL_ON           0x1000
#define AMETA_CTRL_LEFT_ON      0x2000
#define AMETA_CTRL_RIGHT_ON     0x4000
#define AMETA_ALT_ON            0x0002
#define AMETA_ALT_LEFT_ON       0x0010
#define AMETA_ALT_RIGHT_ON      0x0020

#define ABUTTON_PRIMARY   0x0001
#define ABUTTON_SECONDARY 0x0002
#define ABUTTON_TERTIARY  0x0004
#define ABUTTON_BACK      0x0008
#define ABUTTON_FORWARD   0x0010
#define ABUTTON_STYLUS_PRIMARY   0x0020
#define ABUTTON_STYLUS_SECONDARY 0x0040

#define KEYF_EXTENDED ((1UL) << 24)

using namespace SNS;

// N-API 弱引用符号声明
extern "C" {
    typedef struct napi_env__ * napi_env;
    typedef struct napi_value__ * napi_value;
    typedef struct napi_ref__ * napi_ref;
    typedef enum { napi_ok = 0 } napi_status;
    // 枚举值需与官方 node_api.h 一致（运行时按值解释）
    typedef enum {
        napi_int8_array = 0,
        napi_uint8_array = 1,
        napi_uint8_clamped_array = 2,
        napi_int16_array = 3,
        napi_uint16_array = 4,
        napi_int32_array = 5,
        napi_uint32_array = 6,
        napi_float32_array = 7,
        napi_float64_array = 8,
        napi_bigint64_array = 9,
        napi_biguint64_array = 10,
    } napi_typedarray_type;

    napi_status __attribute__((weak)) napi_create_reference(napi_env, napi_value, uint32_t, napi_ref*);
    napi_status __attribute__((weak)) napi_delete_reference(napi_env, napi_ref);
    napi_status __attribute__((weak)) napi_get_reference_value(napi_env, napi_ref, napi_value*);
    napi_status __attribute__((weak)) napi_get_typedarray_info(napi_env, napi_value,
        napi_typedarray_type*, size_t*, void**, napi_value*, size_t*);
    napi_status __attribute__((weak)) napi_get_arraybuffer_info(napi_env, napi_value,
        void**, size_t*);
    napi_status __attribute__((weak)) napi_create_int64(napi_env, int64_t, napi_value*);
}

static LRESULT Ohos_SendMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    SApplication *pApp = SApplication::getSingletonPtr();
    if (!IsWindow(hWnd)) {
        SLOGE() << "hWnd is invalid, hWnd=" << hWnd;
        return 0;
    }
    if (pApp) {
        IMessageLoop *pMsgLoop = pApp->GetMsgLoop();
        if (pMsgLoop) {
            MSG msgStruct = {0};
            msgStruct.hwnd = hWnd;
            msgStruct.message = msg;
            msgStruct.wParam = wParam;
            msgStruct.lParam = lParam;
            pMsgLoop->PreTranslateMessage(&msgStruct);
        }
        return ::SendMessage(hWnd, msg, wParam, lParam);
    }
    return 0;
}

// ============================================================
// 构造 / 析构
// ============================================================

SouiSurfaceProxy::SouiSurfaceProxy(napi_env env, napi_value nativeWnd)
    : m_ref(nullptr)
    , m_hoverTracked(false)
    , m_buttonsDown(0)
    , m_pointerIdPrimary(0)
{
    memset(m_lastDownTime, 0, sizeof(m_lastDownTime));
    memset(m_lastDownX, 0, sizeof(m_lastDownX));
    memset(m_lastDownY, 0, sizeof(m_lastDownY));
    if (env && nativeWnd && napi_create_reference) {
        napi_create_reference(env, nativeWnd, 1, &m_ref);
    }
    SLOGI() << "hjx, SouiSurfaceProxy +++: id=" << m_ref;
}

SouiSurfaceProxy::~SouiSurfaceProxy() {
    SASSERT(m_ref == nullptr);
}

void SouiSurfaceProxy::clean(napi_env env){
    SLOGI() << "hjx, SouiSurfaceProxy ---: id=" << m_ref;
    napi_delete_reference(env,m_ref);
    m_ref = nullptr;
}

napi_value SouiSurfaceProxy::getLocalRef(napi_env env) {
    if (!env || !m_ref || !napi_get_reference_value) return nullptr;
    napi_value val = nullptr;
    napi_get_reference_value(env, m_ref, &val);
    return val;
}

// ============================================================
// 尺寸变化
//   注意：width/height 为物理像素（ArkTS 侧已 ×density 换算，
//   与 SOUI 窗口对象及像素缓冲区坐标系一致）。
// ============================================================

void SouiSurfaceProxy::onSizeChanged(int width, int height) {
    HWND hWnd = getNativeId();
    SLOGI()<<"hjx, onSizeChanged,hWnd="<<(void*)hWnd<<" width="<<width<<" height="<<height;
    SetWindowPos(hWnd, 0, 0, 0, width, height,
                 SWP_NOZORDER | SWP_NOMOVE);
}

// ============================================================
// 鼠标事件辅助
// ============================================================

WPARAM SouiSurfaceProxy::buildMouseWParam(int metaState, int extraMkBits) {
    WPARAM mk = (WPARAM)extraMkBits;
    if (metaState & (AMETA_SHIFT_ON | AMETA_SHIFT_LEFT_ON | AMETA_SHIFT_RIGHT_ON))
        mk |= MK_SHIFT;
    if (metaState & (AMETA_CTRL_ON | AMETA_CTRL_LEFT_ON | AMETA_CTRL_RIGHT_ON))
        mk |= MK_CONTROL;
    return mk;
}

WPARAM SouiSurfaceProxy::buttonStateToMkBits(int buttonState) {
    WPARAM mk = 0;
    if (buttonState & (ABUTTON_PRIMARY   | ABUTTON_STYLUS_PRIMARY))    mk |= MK_LBUTTON;
    if (buttonState & (ABUTTON_SECONDARY | ABUTTON_STYLUS_SECONDARY))  mk |= MK_RBUTTON;
    if (buttonState & ABUTTON_TERTIARY)                                 mk |= MK_MBUTTON;
    if (buttonState & ABUTTON_BACK)                                     mk |= MK_XBUTTON1;
    if (buttonState & ABUTTON_FORWARD)                                  mk |= MK_XBUTTON2;
    return mk;
}

int SouiSurfaceProxy::buttonIndexFromButtonState(int buttonState) {
    if (buttonState & ABUTTON_PRIMARY)           return 0;
    if (buttonState & ABUTTON_SECONDARY)         return 1;
    if (buttonState & ABUTTON_TERTIARY)          return 2;
    if (buttonState & ABUTTON_BACK)              return 3;
    if (buttonState & ABUTTON_FORWARD)           return 4;
    if (buttonState & ABUTTON_STYLUS_PRIMARY)    return 0;
    if (buttonState & ABUTTON_STYLUS_SECONDARY)  return 1;
    return -1;
}

bool SouiSurfaceProxy::buttonMessages(int btnIdx, UINT& outDown, UINT& outUp,
                                      UINT& outDbl, WPARAM& outMk) {
    switch (btnIdx) {
        case 0: outDown = WM_LBUTTONDOWN; outUp = WM_LBUTTONUP;
                outDbl = WM_LBUTTONDBLCLK; outMk = MK_LBUTTON; return true;
        case 1: outDown = WM_RBUTTONDOWN; outUp = WM_RBUTTONUP;
                outDbl = WM_RBUTTONDBLCLK; outMk = MK_RBUTTON; return true;
        case 2: outDown = WM_MBUTTONDOWN; outUp = WM_MBUTTONUP;
                outDbl = WM_MBUTTONDBLCLK; outMk = MK_MBUTTON; return true;
        case 3: outDown = WM_XBUTTONDOWN; outUp = WM_XBUTTONUP;
                outDbl = WM_XBUTTONDBLCLK; outMk = MK_XBUTTON1; return true;
        case 4: outDown = WM_XBUTTONDOWN; outUp = WM_XBUTTONUP;
                outDbl = WM_XBUTTONDBLCLK; outMk = MK_XBUTTON2; return true;
        default: return false;
    }
}

void SouiSurfaceProxy::dispatchMouseButton(int action, int btnIdx, int ix, int iy,
                                           WPARAM mkExtra, long timestamp) {
    UINT msgDown = 0, msgUp = 0, msgDbl = 0;
    WPARAM mk = 0;
    if (!buttonMessages(btnIdx, msgDown, msgUp, msgDbl, mk)) return;

    HWND hHost = (HWND)(intptr_t)getNativeId();
    // 对齐 Android：BUTTON_PRESS(11)/BUTTON_RELEASE(12) 与 DOWN(0)/UP(1) 同为按下/抬起语义
    // （case 5 POINTER_DOWN / case 6 POINTER_UP 即用 BUTTON_PRESS/RELEASE 驱动次指右键）
    const bool isPress = (action == AMOTION_EVENT_ACTION_BUTTON_PRESS) || (action == 0);
    const bool isRelease = (action == AMOTION_EVENT_ACTION_BUTTON_RELEASE) || (action == 1);
    LPARAM lParamXY = MAKELPARAM((WORD)ix, (WORD)iy);

    if (isPress) {
        WPARAM w = (WPARAM)(mk | mkExtra);
        // XBUTTON：wParam 高 16 位必须是 XBUTTON1/XBUTTON2
        if (msgDown == WM_XBUTTONDOWN) {
            WORD xbtn = (btnIdx == 3) ? XBUTTON1 : XBUTTON2;
            w &= 0xFFFFu;
            w |= ((WPARAM)xbtn) << 16;
        }
        bool isDbl = false;
        if (timestamp > 0 && m_lastDownTime[btnIdx] > 0) {
            long dt = timestamp - m_lastDownTime[btnIdx];
            int dx = ix - m_lastDownX[btnIdx];
            int dy = iy - m_lastDownY[btnIdx];
            UINT dct = ::GetDoubleClickTime();
            if (dct == 0) dct = 500;
            if (dt >= 0 && dt <= (long)dct && dx * dx + dy * dy <= 16)
                isDbl = true;
        }
        m_lastDownTime[btnIdx] = timestamp;
        m_lastDownX[btnIdx] = ix;
        m_lastDownY[btnIdx] = iy;
        ::Ohos_SendMessage(hHost, isDbl ? msgDbl : msgDown, w, lParamXY);
        if (btnIdx >= 0 && btnIdx < 31) m_buttonsDown |= (1 << btnIdx);
        syncGlobalMouseButtons(m_buttonsDown);
        if (!m_hoverTracked) {
            TRACKMOUSEEVENT tme = {0};
            tme.cbSize = sizeof(tme);
            tme.dwFlags = TME_HOVER | TME_LEAVE;
            tme.hwndTrack = hHost;
            tme.dwHoverTime = HOVER_DEFAULT;
            ::TrackMouseEvent(&tme);
            m_hoverTracked = true;
        }
    } else if (isRelease) {
        if (btnIdx >= 0 && btnIdx < 31) m_buttonsDown &= ~(1 << btnIdx);
        syncGlobalMouseButtons(m_buttonsDown);

        WPARAM w = (WPARAM)mkExtra; // UP wParam 不含刚抬起的按钮
        if (msgUp == WM_XBUTTONUP) {
            WORD xbtn = (btnIdx == 3) ? XBUTTON1 : XBUTTON2;
            w &= 0xFFFFu;
            w |= ((WPARAM)xbtn) << 16;
        }
        ::Ohos_SendMessage(hHost, msgUp, w, lParamXY);

        // 右键抬起 → WM_CONTEXTMENU（SOUI 菜单/控件依赖）
        if (btnIdx == 1) {
            ::Ohos_SendMessage(hHost, WM_CONTEXTMENU,
                              (WPARAM)hHost, lParamXY);
        }
        // 所有按钮抬起时：左键是最后按钮，清 hover（ReleaseCapture 会触发 WM_MOUSELEAVE）
        if (btnIdx == 0 && m_buttonsDown == 0)
            m_hoverTracked = false;
    }
}

// ============================================================
// 触摸 / 鼠标事件
// ============================================================

void SouiSurfaceProxy::onMotionEvent(int action, float x, float y, int pointerId,
                                     int buttonState, float vscroll, float hscroll,
                                     int metaState, long timestamp) {
    HWND hHost = (HWND)(intptr_t)getNativeId();
    const int ix = static_cast<int>(x);
    const int iy = static_cast<int>(y);

    if (action == ARKUI_TOUCH_DOWN) m_pointerIdPrimary = pointerId;

    WPARAM mkModsOnly = buildMouseWParam(metaState, 0);
    WPARAM mkWithButtons = mkModsOnly | buttonStateToMkBits(buttonState);
    switch (action) {
        case ARKUI_TOUCH_DOWN: // ACTION_DOWN → 主鼠标左键按下
            dispatchMouseButton(action, // LEFT
0, ix, iy, mkModsOnly, timestamp);
            // 同步一次 WM_MOUSEMOVE（与 Windows DOWN 行为一致）
            ::Ohos_SendMessage(hHost, WM_MOUSEMOVE,
                              (WPARAM)(MK_LBUTTON | mkModsOnly),
                              MAKELPARAM((WORD)ix, (WORD)iy));
            return;
        case ARKUI_TOUCH_UP: // ACTION_UP → 主鼠标左键抬起
            dispatchMouseButton(action, 0, ix, iy, mkModsOnly, timestamp);
            return;
        case ARKUI_TOUCH_MOVE: { // ACTION_MOVE
            WPARAM mk = mkModsOnly;
            if (buttonState != 0)
                mk = mkWithButtons;
            else if (m_buttonsDown != 0) {
                if (m_buttonsDown & (1 << 0)) mk |= MK_LBUTTON;
                if (m_buttonsDown & (1 << 1)) mk |= MK_RBUTTON;
                if (m_buttonsDown & (1 << 2)) mk |= MK_MBUTTON;
                if (m_buttonsDown & (1 << 3)) mk |= MK_XBUTTON1;
                if (m_buttonsDown & (1 << 4)) mk |= MK_XBUTTON2;
            }
            ::Ohos_SendMessage(hHost, WM_MOUSEMOVE, mk,
                              MAKELPARAM((WORD)ix, (WORD)iy));
            return;
        }
        case ARKUI_TOUCH_CANCEL: // ACTION_CANCEL → WM_MOUSELEAVE
            m_buttonsDown = 0;
            syncGlobalMouseButtons(m_buttonsDown);
            ::Ohos_SendMessage(hHost, WM_MOUSELEAVE, mkModsOnly, 0);
            m_hoverTracked = false;
            return;
        case 5: // ACTION_POINTER_DOWN → 次指 = 右键按下（对齐 Android）
            dispatchMouseButton(AMOTION_EVENT_ACTION_BUTTON_PRESS, // RIGHT
1,
                                ix, iy, mkModsOnly, timestamp);
            return;
        case 6: // ACTION_POINTER_UP → 次指 = 右键抬起（对齐 Android）
            dispatchMouseButton(AMOTION_EVENT_ACTION_BUTTON_RELEASE, // RIGHT
1,
                                ix, iy, mkModsOnly, timestamp);
            return;

        // 物理鼠标按钮（BUTTON_PRESS / BUTTON_RELEASE）
        case AMOTION_EVENT_ACTION_BUTTON_PRESS:
        case AMOTION_EVENT_ACTION_BUTTON_RELEASE: {
            int idx = buttonIndexFromButtonState(buttonState);
            if (idx < 0) {
                SLOGW() << "onMotionEvent: unknown buttonState=0x" << buttonState << ", skip";
                return;
            }
            dispatchMouseButton(action, idx, ix, iy, mkModsOnly, timestamp);
            return;
        }

        // 滚轮（垂直 + 水平）
        case AMOTION_EVENT_ACTION_SCROLL: {
            WPARAM mk = mkWithButtons;
            LPARAM lpXY = MAKELPARAM((WORD)ix, (WORD)iy);
            // vscroll：Android "一档 ≈ 1.0"，与 WHEEL_DELTA(120) 对齐
            float fV = vscroll;
            if (fV < -16.0f) fV = -16.0f;
            if (fV >  16.0f) fV =  16.0f;
            int iV = (int)(fV * (float)WHEEL_DELTA);
            if (iV != 0) {
                WPARAM wWheel = (WPARAM)mk;
                wWheel &= 0xFFFFu;
                wWheel |= ((WPARAM)(short)iV) << 16;
                ::Ohos_SendMessage(hHost, WM_MOUSEWHEEL, wWheel, lpXY);
            }
            float fH = hscroll;
            if (fH < -16.0f) fH = -16.0f;
            if (fH >  16.0f) fH =  16.0f;
            int iH = (int)(fH * (float)WHEEL_DELTA);
            if (iH != 0) {
                WPARAM wHWheel = (WPARAM)mk;
                wHWheel &= 0xFFFFu;
                wHWheel |= ((WPARAM)(short)iH) << 16;
                ::Ohos_SendMessage(hHost, WM_MOUSEHWHEEL, wHWheel, lpXY);
            }
            return;
        }

        // 悬停（HOVER_MOVE / HOVER_ENTER / HOVER_EXIT）
        case AMOTION_EVENT_ACTION_HOVER_ENTER:
        case AMOTION_EVENT_ACTION_HOVER_MOVE: {
            WPARAM mk = mkWithButtons;
            LPARAM lp = MAKELPARAM((WORD)ix, (WORD)iy);
            if (action == AMOTION_EVENT_ACTION_HOVER_ENTER) {
                if (!m_hoverTracked) {
                    TRACKMOUSEEVENT tme = {0};
                    tme.cbSize = sizeof(tme);
                    tme.dwFlags = TME_HOVER | TME_LEAVE;
                    tme.hwndTrack = hHost;
                    tme.dwHoverTime = HOVER_DEFAULT;
                    ::TrackMouseEvent(&tme);
                    m_hoverTracked = true;
                }
            }
            ::Ohos_SendMessage(hHost, WM_MOUSEMOVE, mk, lp);
            return;
        }
        case AMOTION_EVENT_ACTION_HOVER_EXIT: {
            ::Ohos_SendMessage(hHost, WM_MOUSELEAVE, mkModsOnly, 0);
            m_hoverTracked = false;
            return;
        }
        default:
            SLOGW() << "onMotionEvent: unknown action=" << action;
            return;
    }
}

// ============================================================
// 键盘事件
// ============================================================

UINT SouiSurfaceProxy::pickKeyMessage(UINT baseDown, int metaState, int vk) {
    const bool alt = (metaState & (AMETA_ALT_ON | AMETA_ALT_LEFT_ON | AMETA_ALT_RIGHT_ON)) != 0;
    if (!alt) return baseDown;
    switch (baseDown) {
        case WM_KEYDOWN:  return WM_SYSKEYDOWN;
        case WM_KEYUP:    return WM_SYSKEYUP;
        case WM_CHAR:     return WM_SYSCHAR;
        default:          return baseDown;
    }
}

LPARAM SouiSurfaceProxy::buildKeyLParam(int action, int metaState,
                                        int repeatCount, int scanCode,
                                        bool extended, bool wasDown) {
    if (repeatCount < 1) repeatCount = 1;
    if (repeatCount > 0xFFFF) repeatCount = 0xFFFF;
    if (scanCode < 0) scanCode = 0;
    if (scanCode > 0xFF) scanCode = scanCode & 0xFF;

    ULONG lParam = (ULONG)(repeatCount & 0xFFFF);
    lParam |= ((ULONG)(scanCode & 0xFF)) << 16;
    if (extended)          lParam |= KEYF_EXTENDED;
    if (metaState & (AMETA_ALT_ON | AMETA_ALT_LEFT_ON | AMETA_ALT_RIGHT_ON))
        lParam |= (1UL << 29);
    if (wasDown)           lParam |= (1UL << 30);
    if (action == 1)       lParam |= (1UL << 31);
    return (LPARAM)lParam;
}

int SouiSurfaceProxy::convertKeyCode(int kc) {
    //
    // ArkTS KeyEvent.keyCode 是 HarmonyOS OpenHarmony MMI 的 KeyCode 枚举值
    // （不是 Android 的 KEYCODE_*），分段如下：
    //   - 1-999        系统/功能键（Home=1, Back=2, 音量/电源等）
    //   - 2000-2099    字母/数字/符号/Tab/Space/Enter/Del/方向/修饰/Esc等
    //   - 2100-2122    小键盘、F1~F12、NumLock
    // 参考 native_key_event.h ArkUI_KeyCode。
    //
    switch (kc) {
        // ===== 系统键 1-999 =====
        case 1:  return VK_HOME;           // KEYCODE_HOME
        case 2:  return VK_ESCAPE;         // KEYCODE_BACK(硬返回键) → ESC

        // ===== 数字键 0-9 =====
        case 2000: return '0';             // KEYCODE_0
        case 2001: return '1';             // KEYCODE_1
        case 2002: return '2';             // KEYCODE_2
        case 2003: return '3';             // KEYCODE_3
        case 2004: return '4';             // KEYCODE_4
        case 2005: return '5';             // KEYCODE_5
        case 2006: return '6';             // KEYCODE_6
        case 2007: return '7';             // KEYCODE_7
        case 2008: return '8';             // KEYCODE_8
        case 2009: return '9';             // KEYCODE_9

        // ===== 字母键 A-Z =====
        case 2017: return 'A';
        case 2018: return 'B';
        case 2019: return 'C';
        case 2020: return 'D';
        case 2021: return 'E';
        case 2022: return 'F';
        case 2023: return 'G';
        case 2024: return 'H';
        case 2025: return 'I';
        case 2026: return 'J';
        case 2027: return 'K';
        case 2028: return 'L';
        case 2029: return 'M';
        case 2030: return 'N';
        case 2031: return 'O';
        case 2032: return 'P';
        case 2033: return 'Q';
        case 2034: return 'R';
        case 2035: return 'S';
        case 2036: return 'T';
        case 2037: return 'U';
        case 2038: return 'V';
        case 2039: return 'W';
        case 2040: return 'X';
        case 2041: return 'Y';
        case 2042: return 'Z';

        // ===== 常用符号 =====
        case 2043: return VK_OEM_COMMA;    // KEYCODE_COMMA      ','
        case 2044: return VK_OEM_PERIOD;   // KEYCODE_PERIOD     '.'
        case 2056: return VK_OEM_3;        // KEYCODE_GRAVE      '`'
        case 2057: return VK_OEM_MINUS;    // KEYCODE_MINUS      '-'
        case 2058: return VK_OEM_PLUS;     // KEYCODE_EQUALS     '='
        case 2059: return VK_OEM_4;        // KEYCODE_LEFT_BRACKET '['
        case 2060: return VK_OEM_6;        // KEYCODE_RIGHT_BRACKET ']'
        case 2061: return VK_OEM_5;        // KEYCODE_BACKSLASH  '\\'
        case 2062: return VK_OEM_1;        // KEYCODE_SEMICOLON  ';'
        case 2063: return VK_OEM_7;        // KEYCODE_APOSTROPHE '\''
        case 2064: return VK_OEM_2;        // KEYCODE_SLASH      '/'
        case 2066: return VK_OEM_PLUS;     // KEYCODE_PLUS       '+'

        // ===== 编辑/控制键 =====
        case 2049: return VK_TAB;          // KEYCODE_TAB
        case 2050: return VK_SPACE;        // KEYCODE_SPACE
        case 2054: return VK_RETURN;       // KEYCODE_ENTER (回车)
        case 2055: return VK_BACK;         // KEYCODE_DEL  (退格 / Backspace)
        case 2071: return VK_DELETE;       // KEYCODE_FORWARD_DEL (Delete)
        case 2070: return VK_ESCAPE;       // KEYCODE_ESCAPE
        case 2083: return VK_INSERT;       // KEYCODE_INSERT
        case 2081: return VK_HOME;         // KEYCODE_MOVE_HOME
        case 2082: return VK_END;          // KEYCODE_MOVE_END
        case 2068: return VK_PRIOR;        // KEYCODE_PAGE_UP
        case 2069: return VK_NEXT;         // KEYCODE_PAGE_DOWN
        case 2067: return VK_APPS;         // KEYCODE_MENU

        // ===== 方向键 =====
        case 2012: return VK_UP;           // KEYCODE_DPAD_UP
        case 2013: return VK_DOWN;         // KEYCODE_DPAD_DOWN
        case 2014: return VK_LEFT;         // KEYCODE_DPAD_LEFT
        case 2015: return VK_RIGHT;        // KEYCODE_DPAD_RIGHT
        case 2016: return VK_RETURN;       // KEYCODE_DPAD_CENTER (导航确定→回车)

        // ===== 修饰键 =====
        case 2047: return VK_SHIFT;        // KEYCODE_SHIFT_LEFT
        case 2048: return VK_SHIFT;        // KEYCODE_SHIFT_RIGHT
        case 2072: return VK_CONTROL;      // KEYCODE_CTRL_LEFT
        case 2073: return VK_CONTROL;      // KEYCODE_CTRL_RIGHT
        case 2045: return VK_MENU;         // KEYCODE_ALT_LEFT
        case 2046: return VK_MENU;         // KEYCODE_ALT_RIGHT
        case 2076: return VK_LWIN;         // KEYCODE_META_LEFT
        case 2077: return VK_RWIN;         // KEYCODE_META_RIGHT
        case 2074: return VK_CAPITAL;      // KEYCODE_CAPS_LOCK
        case 2075: return VK_SCROLL;       // KEYCODE_SCROLL_LOCK
        case 2102: return VK_NUMLOCK;      // KEYCODE_NUM_LOCK
        case 2079: return VK_SNAPSHOT;     // KEYCODE_SYSRQ (PrintScreen)
        case 2080: return VK_PAUSE;        // KEYCODE_BREAK / Pause

        // ===== F1-F12 =====
        case 2090: return VK_F1;
        case 2091: return VK_F2;
        case 2092: return VK_F3;
        case 2093: return VK_F4;
        case 2094: return VK_F5;
        case 2095: return VK_F6;
        case 2096: return VK_F7;
        case 2097: return VK_F8;
        case 2098: return VK_F9;
        case 2099: return VK_F10;
        case 2100: return VK_F11;
        case 2101: return VK_F12;

        // ===== 小键盘 Numpad =====
        case 2103: return VK_NUMPAD0;
        case 2104: return VK_NUMPAD1;
        case 2105: return VK_NUMPAD2;
        case 2106: return VK_NUMPAD3;
        case 2107: return VK_NUMPAD4;
        case 2108: return VK_NUMPAD5;
        case 2109: return VK_NUMPAD6;
        case 2110: return VK_NUMPAD7;
        case 2111: return VK_NUMPAD8;
        case 2112: return VK_NUMPAD9;
        case 2113: return VK_DIVIDE;
        case 2114: return VK_MULTIPLY;
        case 2115: return VK_SUBTRACT;
        case 2116: return VK_ADD;
        case 2117: return VK_DECIMAL;
        case 2119: return VK_RETURN;       // NUMPAD_ENTER → 同回车

        default: return 0;
    }
}

bool SouiSurfaceProxy::onKeyEvent(int keyCode, int action, int metaState,
                                  int repeatCount, int scanCode, int unicodeChar,
                                  long flags, long timestamp) {
    (void)flags; (void)timestamp;
    if (action < 0 || action > 2) return false;

    HWND hHost = (HWND)(intptr_t)getNativeId();
    int vk = convertKeyCode(keyCode);
    if (vk == 0 && action != 1) return false;

    LRESULT handled = 0;
    bool extended = false;
    switch (vk) {
        case VK_RCONTROL: case VK_RMENU:
        case VK_INSERT: case VK_DELETE: case VK_HOME: case VK_END:
        case VK_PRIOR: case VK_NEXT:
        case VK_LEFT: case VK_RIGHT: case VK_UP: case VK_DOWN:
        case VK_NUMLOCK: case VK_RETURN:
            extended = true; break;
        default: break;
    }

    if (action == 2) { // MULTIPLE
        if (repeatCount < 2) repeatCount = 2;
        const UINT msg = pickKeyMessage(WM_KEYDOWN, metaState, vk);
        const LPARAM lp = buildKeyLParam(0, metaState, repeatCount, scanCode, extended, true);
        if (vk != 0) handled = (0 == ::Ohos_SendMessage(hHost, msg, (WPARAM)vk, lp));
        if (unicodeChar > 0) {
            const UINT charMsg = pickKeyMessage(WM_CHAR, metaState, 0);
            for (int i = 0; i < repeatCount; ++i) {
                const LPARAM charLP = buildKeyLParam(0, metaState, 1, scanCode, extended, true);
                handled = (0 == ::Ohos_SendMessage(hHost, charMsg, (WPARAM)unicodeChar, charLP));
            }
        }
        return handled != 0;
    }

    if (action == 0) { // DOWN
        if (repeatCount < 1) repeatCount = 1;
        const bool wasDown = (repeatCount > 1);
        const UINT msg = pickKeyMessage(WM_KEYDOWN, metaState, vk);
        const LPARAM lp = buildKeyLParam(0, metaState, repeatCount, scanCode, extended, wasDown);
        if (vk != 0) handled = (0 == ::Ohos_SendMessage(hHost, msg, (WPARAM)vk, lp));
        if (unicodeChar > 0) {
            const UINT charMsg = pickKeyMessage(WM_CHAR, metaState, 0);
            for (int i = 0; i < repeatCount; ++i) {
                const bool wd = (i > 0);
                const LPARAM charLP = buildKeyLParam(0, metaState, 1, scanCode, extended, wd);
                handled = (0 == ::Ohos_SendMessage(hHost, charMsg, (WPARAM)unicodeChar, charLP));
            }
        }
        return handled != 0;
    }

    if (action == 1) { // UP
        const UINT msg = pickKeyMessage(WM_KEYUP, metaState, vk);
        const LPARAM lp = buildKeyLParam(1, metaState, 1, scanCode, extended, true);
        if (vk != 0) handled = (0 == ::Ohos_SendMessage(hHost, msg, (WPARAM)vk, lp));
        return handled != 0;
    }
    return false;
}

// ============================================================
// 渲染：将 SOUI 内容写入像素缓冲区
// 对应 Android SouiSurfaceProxy::render(env, bitmap)
// ============================================================

void SouiSurfaceProxy::render(napi_env env, napi_value pixelBuffer,
                                int width, int height, RECT *rcUpdate) {
    if (!env || !pixelBuffer) {
        SLOGE() << "render: env or pixelBuffer is null";
        return;
    }
    if (width <= 0 || height <= 0) {
        SLOGW() << "render: invalid size " << width << "x" << height;
        return;
    }
    DWORD ts1=GetTickCount();
    // 通过 N-API 获取 typed array 底层指针
    void *pixels = nullptr;
    size_t byteLength = 0;
    if (napi_get_typedarray_info) {
        napi_typedarray_type type;
        size_t length = 0;
        napi_value arraybuffer = nullptr;
        size_t offset = 0;
        napi_get_typedarray_info(env, pixelBuffer, &type, &length,
                                 &pixels, &arraybuffer, &offset);
        byteLength = length;
    }

    if (!pixels || byteLength == 0) {
        // 尝试作为 ArrayBuffer
        if (napi_get_arraybuffer_info) {
            napi_get_arraybuffer_info(env, pixelBuffer, &pixels, &byteLength);
        }
    }

    if (!pixels || byteLength == 0) {
        SLOGE() << "render: failed to get pixel buffer";
        return;
    }

    size_t expectedSize = (size_t)width * height * 4;
    if (byteLength < expectedSize) {
        SLOGE() << "render: buffer too small " << byteLength << " < " << expectedSize;
        return;
    }

    HWND hWnd = (HWND)(intptr_t)getNativeId();
    //SLOGI()<<"hjx, render for hwnd:"<<(void*)hWnd<<" width="<<width<<" height="<<height;
    const int stride = width * 4; // RGBA_8888 行步长

    // 创建 Cairo image surface 包装像素缓冲区（与 Android 一致）
    HBITMAP hBmp = CreateDIBSectionEx(32, width, height, stride, pixels);
    HDC hdc = ::GetDC(hWnd);
    if (!hdc) {
        // 窗口尚未注册进 swinx（screenStartup 之前的一次性时序窗口），
        // 跳过本次渲染；注册完成后 C++ 侧 InvalidateRect 会重新触发渲染。
        SLOGW() << "render: hwnd=" << (void*)hWnd << " not registered, skip frame";
        DeleteObject(hBmp);
        return;
    }
    HRGN hRgn=CreateRectRgn(0, 0, 0, 0);
    GetUpdateRgn(hWnd, hRgn, FALSE);
    CRect rcDirty;
    GetRgnBox(hRgn, &rcDirty);
    DeleteObject(hRgn);
    //SLOGI()<<"update rect="<<rcDirty<<" size="<<rcDirty.Size();
    // 把本次实际重绘区域回传给 ArkTS 侧（物理像素，客户区坐标）
    if (rcUpdate) {
        *rcUpdate  = rcDirty;
    }
    HGDIOBJ oldBmp = SelectObject(hdc, hBmp);
    // 触发 SOUI 窗口过程渲染到 HDC → 像素缓冲区
    ::SendMessage(hWnd, WM_PAINT, 0, 0);
    SelectObject(hdc, oldBmp);
    ::ReleaseDC(hWnd, hdc);
    DeleteObject(hBmp);

    DWORD ts2=GetTickCount();
    //SLOGI()<<"timer, render frame cost="<<(ts2-ts1);
}
