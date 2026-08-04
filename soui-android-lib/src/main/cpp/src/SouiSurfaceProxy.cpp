#include "SouiSurfaceProxy.h"
#include "AndroidPlatformAPI.h"
#include <cassert>
#include <mutex>
#include <cstring>
#include <windows.h>
#include <gdi.h>
#include <helper/slog.h>
#define kLogTag "surfaceproxy"

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

// Android MotionEvent 常量（与 android.view.MotionEvent 保持一致）
#ifndef AMOTION_EVENT_ACTION_BUTTON_PRESS
#define AMOTION_EVENT_ACTION_BUTTON_PRESS  11
#endif
#ifndef AMOTION_EVENT_ACTION_BUTTON_RELEASE
#define AMOTION_EVENT_ACTION_BUTTON_RELEASE 12
#endif
#ifndef AMOTION_EVENT_ACTION_SCROLL
#define AMOTION_EVENT_ACTION_SCROLL        13
#endif
#ifndef AMOTION_EVENT_ACTION_HOVER_MOVE
#define AMOTION_EVENT_ACTION_HOVER_MOVE     7
#endif
#ifndef AMOTION_EVENT_ACTION_HOVER_ENTER
#define AMOTION_EVENT_ACTION_HOVER_ENTER    9
#endif
#ifndef AMOTION_EVENT_ACTION_HOVER_EXIT
#define AMOTION_EVENT_ACTION_HOVER_EXIT    10
#endif

// Android KeyEvent / MotionEvent 常用 metaState 常量
#define AMETA_SHIFT_ON          0x0001
#define AMETA_SHIFT_LEFT_ON     0x0040
#define AMETA_SHIFT_RIGHT_ON    0x0080
#define AMETA_CTRL_ON           0x1000
#define AMETA_CTRL_LEFT_ON      0x2000
#define AMETA_CTRL_RIGHT_ON     0x4000
#define AMETA_ALT_ON            0x0002
#define AMETA_ALT_LEFT_ON       0x0010
#define AMETA_ALT_RIGHT_ON      0x0020
#define AMETA_SYM_ON            0x0004
#define AMETA_CAPS_LOCK_ON      0x0100
#define AMETA_NUM_LOCK_ON       0x0200
#define AMETA_SCROLL_LOCK_ON    0x0400
#define AMETA_META_ON           0x0008

// Android MotionEvent BUTTON_* 位
#define ABUTTON_PRIMARY   0x0001
#define ABUTTON_SECONDARY 0x0002
#define ABUTTON_TERTIARY  0x0004
#define ABUTTON_BACK      0x0008
#define ABUTTON_FORWARD   0x0010
#define ABUTTON_STYLUS_PRIMARY   0x0020
#define ABUTTON_STYLUS_SECONDARY 0x0040

// Windows 扩展键 VK_* 的 Extended bit（lParam bit 24）标志
#define KEYF_EXTENDED ((1UL) << 24)



using namespace SNS;

static LRESULT Android_SendMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    SApplication *pApp = SApplication::getSingletonPtr();
    if(!IsWindow(hWnd)) {
        SLOGE()<<"hWnd is invalid, hWnd="<<hWnd;
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
        return ::SendMessage(hWnd,msg,wParam,lParam);
    }else{
		return 0;
	}
}


SouiSurfaceProxy::SouiSurfaceProxy(JNIEnv* env, jobject nativeWnd)
    : m_javaRef(nullptr)
    , m_hoverTracked(false)
    , m_buttonsDown(0)
    , m_pointerIdPrimary(0)
{
    memset(m_lastDownTime, 0, sizeof(m_lastDownTime));
    memset(m_lastDownX, 0, sizeof(m_lastDownX));
    memset(m_lastDownY, 0, sizeof(m_lastDownY));
    m_javaRef = env->NewGlobalRef(nativeWnd);
    SLOGI()<<"SouiSurfaceProxy, +++: id="<<m_javaRef;
}

SouiSurfaceProxy::~SouiSurfaceProxy() {
    SLOGI()<<"SouiSurfaceProxy ---, id="<<m_javaRef;
    JNIEnv* env = AndroidPlatformAPI::instance().getJNIEnv();
    if (env) {
        env->DeleteGlobalRef(m_javaRef);
    }
    m_javaRef = nullptr;
}

jobject SouiSurfaceProxy::getLocallRef(){
    JNIEnv* env = AndroidPlatformAPI::instance().getJNIEnv();
    if (env) {
        return env->NewLocalRef(m_javaRef);
    }
    return nullptr;
}

void SouiSurfaceProxy::onSizeChanged(int width, int height) {
    SetWindowPos(getNativeId(),0,0,0,width,height,SWP_NOZORDER|SWP_NOMOVE);
}

WPARAM SouiSurfaceProxy::buildMouseWParam(int metaState, int extraMkBits) {
    WPARAM mk = (WPARAM)extraMkBits;
    if (metaState & (AMETA_SHIFT_ON|AMETA_SHIFT_LEFT_ON|AMETA_SHIFT_RIGHT_ON))   mk |= MK_SHIFT;
    if (metaState & (AMETA_CTRL_ON |AMETA_CTRL_LEFT_ON |AMETA_CTRL_RIGHT_ON ))   mk |= MK_CONTROL;
    // Windows 没有 MK_ALT，ALT 状态通过 WM_SYSKEYDOWN 前置消息同步；wParam 只累计实际按钮。
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

UINT SouiSurfaceProxy::pickKeyMessage(UINT baseDown, int metaState, int vk) {
    const bool alt = (metaState & (AMETA_ALT_ON|AMETA_ALT_LEFT_ON|AMETA_ALT_RIGHT_ON)) != 0;
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
    if (metaState & (AMETA_ALT_ON|AMETA_ALT_LEFT_ON|AMETA_ALT_RIGHT_ON))
        lParam |= (1UL << 29); // 上下文码 = ALT 按下
    if (wasDown)           lParam |= (1UL << 30); // 前一按键状态：已按下
    // action 0/2=DOWN/MULTIPLE → transition=0；1=UP → transition=1
    if (action == 1)       lParam |= (1UL << 31);
    return (LPARAM)lParam;
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
        case 0:
            outDown = WM_LBUTTONDOWN; outUp = WM_LBUTTONUP; outDbl = WM_LBUTTONDBLCLK;
            outMk = MK_LBUTTON; return true;
        case 1:
            outDown = WM_RBUTTONDOWN; outUp = WM_RBUTTONUP; outDbl = WM_RBUTTONDBLCLK;
            outMk = MK_RBUTTON; return true;
        case 2:
            outDown = WM_MBUTTONDOWN; outUp = WM_MBUTTONUP; outDbl = WM_MBUTTONDBLCLK;
            outMk = MK_MBUTTON; return true;
        case 3:
            outDown = WM_XBUTTONDOWN; outUp = WM_XBUTTONUP; outDbl = WM_XBUTTONDBLCLK;
            outMk = MK_XBUTTON1; return true;
        case 4:
            outDown = WM_XBUTTONDOWN; outUp = WM_XBUTTONUP; outDbl = WM_XBUTTONDBLCLK;
            outMk = MK_XBUTTON2; return true;
        default:
            return false;
    }
}

void SouiSurfaceProxy::dispatchMouseButton(int action, int btnIdx, int ix, int iy,
                                           WPARAM mkExtra, long timestamp) {
    UINT msgDown = 0, msgUp = 0, msgDbl = 0;
    WPARAM mk = 0;
    if (!buttonMessages(btnIdx, msgDown, msgUp, msgDbl, mk)) return;

    HWND hHost = (HWND)getNativeId();
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

        // 双击检测（阈值：GetDoubleClickTime 默认 500ms，位移 ≤ 4px）
        bool isDbl = false;
        if (timestamp > 0 && m_lastDownTime[btnIdx] > 0) {
            long dt = timestamp - m_lastDownTime[btnIdx];
            int dx = ix - m_lastDownX[btnIdx];
            int dy = iy - m_lastDownY[btnIdx];
            UINT dct = ::GetDoubleClickTime();
            if (dct == 0) dct = 500;
            if (dt >= 0 && dt <= (long)dct && dx*dx + dy*dy <= 16) {
                isDbl = true;
            }
        }
        m_lastDownTime[btnIdx] = timestamp;
        m_lastDownX[btnIdx] = ix;
        m_lastDownY[btnIdx] = iy;

        if (isDbl) {
            ::Android_SendMessage(hHost, msgDbl, w, lParamXY);
        } else {
            ::Android_SendMessage(hHost, msgDown, w, lParamXY);
        }

        if (btnIdx >= 0 && btnIdx < 31) m_buttonsDown |= (1 << btnIdx);

        // 首次进入开启 TrackMouseEvent(HOVER + LEAVE)
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

        WPARAM w = (WPARAM)mkExtra; // UP wParam 不含刚抬起的按钮
        if (msgUp == WM_XBUTTONUP) {
            WORD xbtn = (btnIdx == 3) ? XBUTTON1 : XBUTTON2;
            w &= 0xFFFFu;
            w |= ((WPARAM)xbtn) << 16;
        }
        ::Android_SendMessage(hHost, msgUp, w, lParamXY);

        // 右键抬起 → WM_CONTEXTMENU（SOUI 菜单/控件依赖）
        if (btnIdx == 1) {
            WPARAM wpCtx = (WPARAM)hHost; // Win32：wParam = 点击窗口句柄
            LPARAM lpCtx = MAKELPARAM((WORD)ix, (WORD)iy);
            ::Android_SendMessage(hHost, WM_CONTEXTMENU, wpCtx, lpCtx);
        }

        // 所有按钮抬起时：左键是最后按钮，清 hover（ReleaseCapture 会触发 WM_MOUSELEAVE）
        if (btnIdx == 0 && m_buttonsDown == 0) {
            m_hoverTracked = false;
        }
    }
}

void SouiSurfaceProxy::onMotionEvent(int action, float x, float y, int pointerId,
                                     int buttonState, float vscroll, float hscroll,
                                     int metaState, long timestamp) {
    HWND hHost = (HWND)getNativeId();
    const int ix = static_cast<int>(x);
    const int iy = static_cast<int>(y);

    if (action == 0) m_pointerIdPrimary = pointerId;

    WPARAM mkModsOnly = buildMouseWParam(metaState, 0);
    WPARAM mkWithButtons = mkModsOnly | buttonStateToMkBits(buttonState);
    AndroidPlatformAPI &api = AndroidPlatformAPI::instance();
    jlong screenId = api.getActiveScreenId();
    HWND hWndDesktop = api.getScreenHwnd(screenId);
    POINT pt = {ix,iy};
    ::MapWindowPoints(hHost,hWndDesktop,&pt,1);
    api.setCursorPos(pt);
    switch (action) {
        // 触摸 / 鼠标
        case 0: // ACTION_DOWN → 主鼠标左键
            dispatchMouseButton(action, /*LEFT*/0, ix, iy, mkModsOnly, timestamp);
            // 同步一次 WM_MOUSEMOVE（与 Windows DOWN 行为一致）
            {
                LPARAM lp = MAKELPARAM((WORD)ix, (WORD)iy);
                ::Android_SendMessage(hHost, WM_MOUSEMOVE, (WPARAM)(MK_LBUTTON | mkModsOnly), lp);
            }
            return;
        case 1: // ACTION_UP → 主鼠标左键抬起
            dispatchMouseButton(action, 0, ix, iy, mkModsOnly, timestamp);
            return;
        case 2: // ACTION_MOVE
            // 按钮状态：物理鼠标 → buttonState；触摸/捕获 → m_buttonsDown 合成
            WPARAM mk;
            mk = mkModsOnly;
            if (buttonState != 0) {
                mk = mkWithButtons;
            } else if (m_buttonsDown != 0) {
                if (m_buttonsDown & (1 << 0)) mk |= MK_LBUTTON;
                if (m_buttonsDown & (1 << 1)) mk |= MK_RBUTTON;
                if (m_buttonsDown & (1 << 2)) mk |= MK_MBUTTON;
                if (m_buttonsDown & (1 << 3)) mk |= MK_XBUTTON1;
                if (m_buttonsDown & (1 << 4)) mk |= MK_XBUTTON2;
            }
            LPARAM lp;
            lp = MAKELPARAM((WORD)ix, (WORD)iy);
            ::Android_SendMessage(hHost, WM_MOUSEMOVE, mk, lp);
            return;
        case 3: // ACTION_CANCEL → WM_MOUSELEAVE
            m_buttonsDown = 0;
            ::Android_SendMessage(hHost, WM_MOUSELEAVE, mkModsOnly, 0);
            m_hoverTracked = false;
            return;
        case 5: // ACTION_POINTER_DOWN → 次指 = 右键
            dispatchMouseButton(AMOTION_EVENT_ACTION_BUTTON_PRESS, 1, ix, iy, mkModsOnly, timestamp);
            return;
        case 6: // ACTION_POINTER_UP → 次指 = 右键抬起
            dispatchMouseButton(AMOTION_EVENT_ACTION_BUTTON_RELEASE, 1, ix, iy, mkModsOnly, timestamp);
            return;

        // 物理鼠标按钮（BUTTON_PRESS / BUTTON_RELEASE）
        case AMOTION_EVENT_ACTION_BUTTON_PRESS:
        case AMOTION_EVENT_ACTION_BUTTON_RELEASE: {
            int idx = buttonIndexFromButtonState(buttonState);
            if (idx < 0) {
                SLOGW()<<"onMotionEvent: unknown buttonState=0x"<<buttonState<<", skip";
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
                ::Android_SendMessage(hHost, WM_MOUSEWHEEL, wWheel, lpXY);
            }

            float fH = hscroll;
            if (fH < -16.0f) fH = -16.0f;
            if (fH >  16.0f) fH =  16.0f;
            int iH = (int)(fH * (float)WHEEL_DELTA);
            if (iH != 0) {
                WPARAM wHWheel = (WPARAM)mk;
                wHWheel &= 0xFFFFu;
                wHWheel |= ((WPARAM)(short)iH) << 16;
                ::Android_SendMessage(hHost, WM_MOUSEHWHEEL, wHWheel, lpXY);
            }
            return;
        }

        // 悬停（HOVER_MOVE / ENTER / EXIT）
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
            ::Android_SendMessage(hHost, WM_MOUSEMOVE, mk, lp);
            return;
        }
        case AMOTION_EVENT_ACTION_HOVER_EXIT: {
            ::Android_SendMessage(hHost, WM_MOUSELEAVE, mkModsOnly, 0);
            m_hoverTracked = false;
            return;
        }
        default: {
            SLOGW()<<"onMotionEvent: unknown action="<<action<<" x="<<ix<<" y="<<iy;
            return;
        }
    }
}

int SouiSurfaceProxy::convertKeyCode(int kc) {
    switch (kc) {
        // 数字 0-9
        case 7:  return '0';
        case 8:  return '1';
        case 9:  return '2';
        case 10: return '3';
        case 11: return '4';
        case 12: return '5';
        case 13: return '6';
        case 14: return '7';
        case 15: return '8';
        case 16: return '9';
        // 字母 A-Z
        case 29: return 'A';
        case 30: return 'B';
        case 31: return 'C';
        case 32: return 'D';
        case 33: return 'E';
        case 34: return 'F';
        case 35: return 'G';
        case 36: return 'H';
        case 37: return 'I';
        case 38: return 'J';
        case 39: return 'K';
        case 40: return 'L';
        case 41: return 'M';
        case 42: return 'N';
        case 43: return 'O';
        case 44: return 'P';
        case 45: return 'Q';
        case 46: return 'R';
        case 47: return 'S';
        case 48: return 'T';
        case 49: return 'U';
        case 50: return 'V';
        case 51: return 'W';
        case 52: return 'X';
        case 53: return 'Y';
        case 54: return 'Z';

        // 常用符号
        case 57: return VK_OEM_2;      // KEYCODE_SLASH
        case 58: return VK_OEM_1;      // KEYCODE_SEMICOLON
        case 59: return VK_OEM_PLUS;   // KEYCODE_EQUALS
        case 60: return VK_OEM_MINUS;  // KEYCODE_MINUS
        case 61: return VK_TAB;
        case 62: return VK_SPACE;
        case 63: return VK_OEM_4;      // KEYCODE_LEFT_BRACKET
        case 64: return VK_OEM_6;      // KEYCODE_RIGHT_BRACKET
        case 65: return VK_OEM_5;      // KEYCODE_BACKSLASH
        case 66: return VK_RETURN;
        case 67: return VK_BACK;
        case 68: return VK_OEM_3;      // KEYCODE_GRAVE
        case 69: return VK_OEM_7;      // KEYCODE_APOSTROPHE
        case 70: return VK_OEM_102;    // KEYCODE_BACKSLASH2

        // 修饰键
        case 108: case 109: return VK_SHIFT;
        case 113: case 114: return VK_CONTROL;
        case 115: case 116: return VK_MENU;
        case 117: return VK_LWIN;
        case 118: return VK_RWIN;

        // 功能与导航
        case 19: return VK_UP;
        case 20: return VK_DOWN;
        case 21: return VK_LEFT;
        case 22: return VK_RIGHT;
        case 23: return VK_RETURN;
        case 4:  return VK_ESCAPE;     // KEYCODE_BACK → ESC
        case 82: return VK_APPS;
        case 3:  return VK_HOME;
        case 122: return VK_HOME;
        case 123: return VK_END;
        case 92:  return VK_PRIOR;
        case 93:  return VK_NEXT;
        case 96:  return VK_SNAPSHOT;
        case 120: return VK_SNAPSHOT;
        case 121: return VK_PAUSE;
        case 112: return VK_DELETE;
        case 124: return VK_INSERT;
        case 125: return VK_NUMLOCK;

        // F1-F12
        case 131: return VK_F1;
        case 132: return VK_F2;
        case 133: return VK_F3;
        case 134: return VK_F4;
        case 135: return VK_F5;
        case 136: return VK_F6;
        case 137: return VK_F7;
        case 138: return VK_F8;
        case 139: return VK_F9;
        case 140: return VK_F10;
        case 141: return VK_F11;
        case 142: return VK_F12;

        // 小键盘
        case 144: return VK_NUMPAD0;
        case 145: return VK_NUMPAD1;
        case 146: return VK_NUMPAD2;
        case 147: return VK_NUMPAD3;
        case 148: return VK_NUMPAD4;
        case 149: return VK_NUMPAD5;
        case 150: return VK_NUMPAD6;
        case 151: return VK_NUMPAD7;
        case 152: return VK_NUMPAD8;
        case 153: return VK_NUMPAD9;
        case 154: return VK_DIVIDE;
        case 155: return VK_MULTIPLY;
        case 156: return VK_SUBTRACT;
        case 157: return VK_ADD;
        case 158: return VK_DECIMAL;
        case 159: return VK_RETURN;
        case 161: return VK_OEM_COMMA;
        case 162: return VK_OEM_102;

        // 冗余兼容映射
        case 75: return VK_OEM_4;
        case 76: return VK_OEM_6;
        case 77: return VK_OEM_1;
        case 78: return VK_OEM_1;
        case 79: return VK_OEM_7;
        case 74: return VK_OEM_3;
        case 73: return VK_OEM_MINUS;
        case 71: return VK_OEM_COMMA;
        default: return 0; // 未识别：不发
    }
}

bool SouiSurfaceProxy::onKeyEvent(int keyCode, int action, int metaState,
                                  int repeatCount, int scanCode, int unicodeChar,
                                  long flags, long timestamp) {
    (void)flags; (void)timestamp;
    // action: 0=DOWN, 1=UP, 2=MULTIPLE（长按重复）
    if (action < 0 || action > 2) return false;

    HWND hHost = (HWND)getNativeId();
    int vk = convertKeyCode(keyCode);
    if (vk == 0 && action != 1) // UP 仍转发以释放捕获
        return false;

    // 只要有一个 SendMessage 返回非 0，就认为 SOUI 已处理（Windows 语义：
    // TranslateMessage/DispatchMessage 消费事件时窗口过程通常返回 TRUE）。
    // ANY 语义，用 LRESULT 累积——非 0 即 handled。
    LRESULT handled = 0;

    // VK 扩展键（lParam bit24）
    bool extended = false;
    switch (vk) {
        case VK_RCONTROL: case VK_RMENU: case VK_RWIN:
        case VK_INSERT: case VK_DELETE: case VK_HOME: case VK_END:
        case VK_PRIOR: case VK_NEXT: case VK_SNAPSHOT:
        case VK_LEFT: case VK_RIGHT: case VK_UP: case VK_DOWN:
        case VK_NUMLOCK: case VK_DIVIDE: case VK_RETURN:
        case VK_LWIN: case VK_APPS:
            extended = true; break;
        default: break;
    }

    // MULTIPLE：DOWN 语义 + repeatCount 填 lParam low word
    if (action == 2) {
        if (repeatCount < 2) repeatCount = 2;
        const UINT msg = pickKeyMessage(WM_KEYDOWN, metaState, vk);
        const LPARAM lp = buildKeyLParam(0, metaState, repeatCount, scanCode, extended, /*wasDown=*/true);
        if (vk != 0) handled = 0== ::Android_SendMessage(hHost, msg, (WPARAM)vk, lp);

        // 可打印字符按 repeatCount 次发 WM_CHAR
        if (unicodeChar > 0) {
            const UINT charMsg = pickKeyMessage(WM_CHAR, metaState, 0);
            for (int i = 0; i < repeatCount; ++i) {
                const LPARAM charLP = buildKeyLParam(0, metaState, 1, scanCode, extended, /*wasDown=*/true);
                handled = 0== ::Android_SendMessage(hHost, charMsg, (WPARAM)unicodeChar, charLP);
            }
        }
        return handled != 0;
    }

    // DOWN
    if (action == 0) {
        if (repeatCount < 1) repeatCount = 1;
        const bool wasDown = (repeatCount > 1);
        const UINT msg = pickKeyMessage(WM_KEYDOWN, metaState, vk);
        const LPARAM lp = buildKeyLParam(0, metaState, repeatCount, scanCode, extended, wasDown);
        if (vk != 0) handled = 0 == ::Android_SendMessage(hHost, msg, (WPARAM)vk, lp);

        // WM_CHAR 每次 DOWN 发一次（repeatCount>1 则发多次）
        if (unicodeChar > 0) {
            const UINT charMsg = pickKeyMessage(WM_CHAR, metaState, 0);
            for (int i = 0; i < repeatCount; ++i) {
                const bool wd = (i > 0);
                const LPARAM charLP = buildKeyLParam(0, metaState, 1, scanCode, extended, wd);
                handled = 0 == ::Android_SendMessage(hHost, charMsg, (WPARAM)unicodeChar, charLP);
            }
        }
        return handled != 0;
    }

    // UP
    if (action == 1) {
        const UINT msg = pickKeyMessage(WM_KEYUP, metaState, vk);
        const LPARAM lp = buildKeyLParam(1, metaState, 1, scanCode, extended, /*wasDown=*/true);
        if (vk != 0) handled = 0 == ::Android_SendMessage(hHost, msg, (WPARAM)vk, lp);
        return handled != 0;
    }
    return false;
}

void SouiSurfaceProxy::render(JNIEnv* env, jobject bitmap) {
    if (env == nullptr || bitmap == nullptr) {
        SLOGE()<<"render: env or bitmap is null";
        return;
    }
    AndroidBitmapInfo info;
    void* pixels = nullptr;

    int ret = AndroidBitmap_getInfo(env, bitmap, &info);
    if (ret < 0) {
        SLOGE()<<"render: AndroidBitmap_getInfo failed, err="<<ret;
        return;
    }
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        SLOGE()<<"render: unsupported bitmap format="<<(int)info.format
               <<", expected RGBA_8888="<<(int)ANDROID_BITMAP_FORMAT_RGBA_8888;
        return;
    }
    if (info.width <= 0 || info.height <= 0) {
        SLOGW()<<"render: bitmap size invalid ("<<info.width<<" x "<<info.height<<")";
        return;
    }

    ret = AndroidBitmap_lockPixels(env, bitmap, &pixels);
    if (ret < 0 || pixels == nullptr) {
        SLOGE()<<"render: AndroidBitmap_lockPixels failed, err="<<ret<<", pixels="<<pixels;
        return;
    }
    HWND hWnd = (HWND)getNativeId();
    const int w = info.width;
    const int h = info.height;
    const int stride = info.stride;   // bytes per row（可能大于 w*4）
    //LOGD("render: bitmap %dx%d stride=%d pixels=%p (nativeId=%p)", w, h, stride, pixels, getNativeId());
    HBITMAP hBmp = CreateDIBSectionEx(32,w,h,stride,pixels);
    HDC hdc = ::GetDC(hWnd);
    HGDIOBJ oldBmp = SelectObject(hdc,hBmp);
    ::SendMessage(hWnd,WM_PAINT,0,0);
    SelectObject(hdc,oldBmp);
    ::ReleaseDC(hWnd,hdc);
    DeleteObject(hBmp);
    // 颜色通道交换：Cairo ARGB32 小端 = BGRA，Android ARGB_8888 = RGBA
    // 对每个 4 字节像素：swap byte0(B) 与 byte2(R)，保留 byte1(G)、byte3(A)。
    {
        uint8_t* row = (uint8_t*)pixels;
#if defined(SOUI_SIMD_NEON)
  #if defined(__aarch64__)
        // AArch64：单条 Q-reg 128-bit 查表，一次处理 4 像素
        alignas(16) const uint8_t neonPerm16[16] = {
             2, 1, 0, 3,   6, 5, 4, 7,
            10, 9, 8,11,  14,13,12,15
        };
        const uint8x16_t vPerm = vld1q_u8(neonPerm16);
        const int wSimd = w & ~3;
        for (int y = 0; y < h; ++y) {
            uint8_t* p = row;
            int x = 0;
            for (; x < wSimd; x += 4) {
                uint8x16_t v   = vld1q_u8(p);
                uint8x16_t out = vqtbl1q_u8(v, vPerm);
                vst1q_u8(p, out);
                p += 16;
            }
            for (; x < w; ++x) {
                uint8_t tmp = p[0]; p[0] = p[2]; p[2] = tmp;
                p += 4;
            }
            row += stride;
        }
  #else
        // ARMv7-A NEON：D-reg 64-bit 查表 × 双半区，一次处理 4 像素
        alignas(8) const uint8_t neonPerm8[8] = {
            2,1,0,3,   6,5,4,7
        };
        const uint8x8_t vPerm8 = vld1_u8(neonPerm8);
        const int wSimd = w & ~3;
        for (int y = 0; y < h; ++y) {
            uint8_t* p = row;
            int x = 0;
            for (; x < wSimd; x += 4) {
                uint8x16_t v    = vld1q_u8(p);
                uint8x8_t  vLo  = vget_low_u8(v);
                uint8x8_t  vHi  = vget_high_u8(v);
                uint8x8_t  oLo  = vtbl1_u8(vLo, vPerm8);
                uint8x8_t  oHi  = vtbl1_u8(vHi, vPerm8);
                uint8x16_t out  = vcombine_u8(oLo, oHi);
                vst1q_u8(p, out);
                p += 16;
            }
            for (; x < w; ++x) {
                uint8_t tmp = p[0]; p[0] = p[2]; p[2] = tmp;
                p += 4;
            }
            row += stride;
        }
  #endif
#elif defined(SOUI_SIMD_SSSE3)
        // SSSE3：16 字节重排掩码 + _mm_shuffle_epi8
        alignas(16) const char ssePerm[16] = {
             2, 1, 0, 3,   6, 5, 4, 7,
            10, 9, 8,11,  14,13,12,15
        };
        const __m128i vPerm = _mm_load_si128(reinterpret_cast<const __m128i*>(ssePerm));
        const int wSimd = w & ~3;
        for (int y = 0; y < h; ++y) {
            uint8_t* p = row;
            int x = 0;
            for (; x < wSimd; x += 4) {
                __m128i v = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(p));
                __m128i out = _mm_shuffle_epi8(v, vPerm);
                _mm_storeu_si128(reinterpret_cast<__m128i*>(p), out);
                p += 16;
            }
            for (; x < w; ++x) {
                uint8_t tmp = p[0]; p[0] = p[2]; p[2] = tmp;
                p += 4;
            }
            row += stride;
        }
#else
        // 通用标量兜底
        for (int y = 0; y < h; ++y) {
            uint8_t* p = row;
            for (int x = 0; x < w; ++x) {
                uint8_t tmp = p[0]; p[0] = p[2]; p[2] = tmp;
                p += 4;
            }
            row += stride;
        }
#endif
    }
    AndroidBitmap_unlockPixels(env, bitmap);
    //LOGV("SouiSurfaceProxy::render() leave OK");
}
