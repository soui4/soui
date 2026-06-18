#include <helper/SOhosApp.h>

#if defined(__OHOS__)

#include <SouiFactory.h>
#include <control/SComboBase.h>
#include <control/SEdit.h>
#include <control/SRichEdit.h>
#include <core/SWindowMgr.h>

#include <ace/xcomponent/native_interface_xcomponent.h>
#define LogCallback OHOS_HILOG_LogCallback
#include <hilog/log.h>
#undef LogCallback
#include <ohos_ime_bridge.h>
#include <ohos_napi_bridge.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

SNSBEGIN

namespace {

static const unsigned int LOG_DOMAIN_SOUI_OHOS = 0x5350;
static const char *LOG_TAG_SOUI_OHOS = "SOUI_OHOS";

#define SOHOS_LOGI(fmt, ...) OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN_SOUI_OHOS, LOG_TAG_SOUI_OHOS, fmt, ##__VA_ARGS__)
#define SOHOS_LOGW(fmt, ...) OH_LOG_Print(LOG_APP, LOG_WARN, LOG_DOMAIN_SOUI_OHOS, LOG_TAG_SOUI_OHOS, fmt, ##__VA_ARGS__)
#define SOHOS_LOGE(fmt, ...) OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN_SOUI_OHOS, LOG_TAG_SOUI_OHOS, fmt, ##__VA_ARGS__)

struct QueuedInput {
    UINT msg;
    WPARAM wp;
    LPARAM lp;
    uint64_t tickMs;
    std::string text;
};

struct ImeRequest {
    bool show;
};

std::mutex g_mutex;
std::mutex g_inputMutex;
std::mutex g_imeRequestCallbackMutex;
std::thread g_appThread;
const SOhosAppCallbacks *g_callbacks = nullptr;
SHostWnd *g_mainHost = nullptr;
std::deque<QueuedInput> g_inputQueue;
napi_threadsafe_function g_imeRequestCallback = nullptr;
std::atomic<bool> g_started(false);
std::atomic<bool> g_resizePending(false);
std::atomic<int> g_pendingWidth(0);
std::atomic<int> g_pendingHeight(0);
std::atomic<uint64_t> g_imeCheckDueMs(0);
std::atomic<bool> g_lastImeFocusNeedsIme(false);
std::atomic<unsigned int> g_lastImeFocusSwnd(0);
std::atomic<bool> g_restoreImeFocusPending(false);
std::atomic<bool> g_leftButtonQueued(false);
std::atomic<unsigned int> g_mouseButtonsDown(0);
std::atomic<bool> g_touchDown(false);
std::atomic<bool> g_shiftDown(false);
std::atomic<bool> g_ctrlDown(false);
std::atomic<bool> g_altDown(false);
std::atomic<bool> g_capsLock(false);
std::atomic<unsigned int> g_synthModifierMask(0);
std::atomic<unsigned int> g_synthModifierKeyVk(0);
std::atomic<int> g_lastInputX(0);
std::atomic<int> g_lastInputY(0);
std::atomic<int> g_renderMode(Render_Gdi);

const UINT_PTR kInputDrainTimer = 0x0f010001;
const UINT kQueuedImeTextInput = WM_APP + 0x531;
const size_t kMaxInputQueue = 256;

uint64_t nowMs()
{
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
}

SHostWnd *getMainHost()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_mainHost;
}

const SOhosAppCallbacks *getCallbacks()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_callbacks;
}

void requestResizeMainWindow()
{
    swinx::ohos::XComponentState state = swinx::ohos::GetXComponentState();
    if (state.width <= 0 || state.height <= 0)
        return;
    g_pendingWidth = state.width;
    g_pendingHeight = state.height;
    g_resizePending = true;
}

void applyPendingResize()
{
    if (!g_resizePending.exchange(false))
        return;

    SHostWnd *host = getMainHost();
    if (!host || !host->m_hWnd)
        return;

    int width = g_pendingWidth.load();
    int height = g_pendingHeight.load();
    if (width <= 0 || height <= 0)
        return;

    RECT rc = {};
    GetClientRect(host->m_hWnd, &rc);
    if (rc.right - rc.left == width && rc.bottom - rc.top == height)
        return;

    MoveWindow(host->m_hWnd, 0, 0, width, height, TRUE);
    InvalidateRect(host->m_hWnd, nullptr, TRUE);
}

void callImeRequestCallback(napi_env env, napi_value jsCallback, void *, void *data)
{
    ImeRequest *request = static_cast<ImeRequest *>(data);
    bool show = request && request->show;
    if (!env || !jsCallback)
    {
        delete request;
        return;
    }
    napi_value global = nullptr;
    napi_value argv[1] = { nullptr };
    napi_get_global(env, &global);
    napi_get_boolean(env, show, &argv[0]);
    napi_call_function(env, global, jsCallback, 1, argv, nullptr);
    delete request;
}

void notifyImeRequest(bool show)
{
    std::lock_guard<std::mutex> lock(g_imeRequestCallbackMutex);
    if (!g_imeRequestCallback)
        return;
    ImeRequest *request = new ImeRequest{ show };
    napi_status status = napi_call_threadsafe_function(g_imeRequestCallback, request, napi_tsfn_nonblocking);
    if (status != napi_ok)
    {
        SOHOS_LOGW("notify IME request failed status=%{public}d", static_cast<int>(status));
        delete request;
    }
}

bool focusedWindowNeedsIme()
{
    SHostWnd *host = getMainHost();
    if (!host || !host->GetRoot())
        return false;

    const ISwndContainer *container = host->GetRoot()->GetContainer();
    if (!container)
        return false;

    SWindow *focus = SWindowMgr::GetWindow(container->GetFocus());
    bool needsIme = focus && (focus->IsClass(SEdit::GetClassName()) || focus->IsClass(SRichEdit::GetClassName()) ||
                              focus->IsClass(SComboEdit::GetClassName()));
    if (!focus && swinx::ohos::IsImeProxyActive() && g_lastImeFocusNeedsIme.load())
    {
        SOHOS_LOGI("focused window is nil while IME proxy is active; keep previous edit IME state");
        return true;
    }
    if (focus)
    {
        g_lastImeFocusNeedsIme = needsIme;
        if (needsIme)
            g_lastImeFocusSwnd = focus->GetSwnd();
        else if (!swinx::ohos::IsImeProxyActive())
            g_lastImeFocusSwnd = 0;
    }
    SOHOS_LOGI("focused window=%{public}p class=%{public}ls needsIme=%{public}d",
               reinterpret_cast<void *>(focus), focus ? focus->GetObjectClass() : L"", needsIme ? 1 : 0);
    return needsIme;
}

void scheduleImeFocusCheck()
{
    g_imeCheckDueMs = nowMs() + 30;
}

void applyPendingImeFocusCheck()
{
    uint64_t due = g_imeCheckDueMs.load();
    if (!due || nowMs() < due)
        return;

    {
        std::lock_guard<std::mutex> lock(g_inputMutex);
        if (!g_inputQueue.empty())
        {
            g_imeCheckDueMs = nowMs() + 16;
            return;
        }
    }

    if (!g_imeCheckDueMs.compare_exchange_strong(due, 0))
        return;
    notifyImeRequest(focusedWindowNeedsIme());
}

void queueInput(UINT msg, WPARAM wp, int x, int y)
{
    if (msg != WM_MOUSEMOVE)
        SOHOS_LOGI("queue input msg=%{public}u wp=%{public}llu x=%{public}d y=%{public}d",
                   static_cast<unsigned int>(msg), static_cast<unsigned long long>(wp), x, y);
    std::lock_guard<std::mutex> lock(g_inputMutex);
    if (msg == WM_LBUTTONDOWN)
        g_leftButtonQueued = true;
    else if (msg == WM_LBUTTONUP || msg == WM_CANCELMODE)
        g_leftButtonQueued = false;
    if (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST && msg != WM_MOUSELEAVE)
    {
        g_lastInputX = x;
        g_lastInputY = y;
    }

    LPARAM lp = MAKELPARAM(x, y);
    uint64_t tick = nowMs();
    if (!g_inputQueue.empty())
    {
        const QueuedInput &last = g_inputQueue.back();
        if (last.msg == msg && last.wp == wp && last.lp == lp && tick - last.tickMs < 30)
            return;
    }
    if (!g_inputQueue.empty() && msg == WM_MOUSEMOVE && g_inputQueue.back().msg == WM_MOUSEMOVE)
    {
        g_inputQueue.back() = { msg, wp, lp, tick };
        return;
    }
    if (g_inputQueue.size() >= kMaxInputQueue)
        g_inputQueue.pop_front();
    g_inputQueue.push_back({ msg, wp, lp, tick });
}

void queueWindowMessage(UINT msg, WPARAM wp, LPARAM lp)
{
    SOHOS_LOGI("queue window msg=%{public}u wp=%{public}llu lp=%{public}lld",
               static_cast<unsigned int>(msg), static_cast<unsigned long long>(wp), static_cast<long long>(lp));
    std::lock_guard<std::mutex> lock(g_inputMutex);
    uint64_t tick = nowMs();
    if (!g_inputQueue.empty())
    {
        const QueuedInput &last = g_inputQueue.back();
        if (last.msg == msg && last.wp == wp && last.lp == lp && tick - last.tickMs < 10)
            return;
    }
    if (g_inputQueue.size() >= kMaxInputQueue)
        g_inputQueue.pop_front();
    g_inputQueue.push_back({ msg, wp, lp, tick });
}

void queueUtf8Text(const char *text, size_t textLen)
{
    if (!text || textLen == 0)
        return;
    if (swinx::ohos::IsImeProxyActive() && g_lastImeFocusSwnd.load())
        g_restoreImeFocusPending = true;
    std::lock_guard<std::mutex> lock(g_inputMutex);
    if (g_inputQueue.size() >= kMaxInputQueue)
        g_inputQueue.pop_front();
    g_inputQueue.push_back({ kQueuedImeTextInput, 0, 0, nowMs(), std::string(text, textLen) });
}

std::string formatUtf8Prefix(const std::string &text)
{
    static const char *hexDigits = "0123456789ABCDEF";
    std::string out;
    const size_t count = std::min<size_t>(text.size(), 12);
    out.reserve(count * 3);
    for (size_t i = 0; i < count; ++i)
    {
        unsigned char ch = static_cast<unsigned char>(text[i]);
        if (!out.empty())
            out.push_back(' ');
        out.push_back(hexDigits[ch >> 4]);
        out.push_back(hexDigits[ch & 0x0F]);
    }
    return out;
}

void restoreImeEditFocusIfNeeded()
{
    if (!g_restoreImeFocusPending.exchange(false))
        return;

    unsigned int swnd = g_lastImeFocusSwnd.load();
    if (!swnd)
        return;

    SWindow *focus = SWindowMgr::GetWindow(swnd);
    if (!focus || focus->IsDisabled(TRUE) || !focus->IsVisible(TRUE))
    {
        SOHOS_LOGW("restore IME edit focus skipped swnd=%{public}u focus=%{public}p",
                   swnd, reinterpret_cast<void *>(focus));
        return;
    }

    const ISwndContainer *container = focus->GetContainer();
    unsigned int curFocus = container ? container->GetFocus() : 0;
    if (curFocus != swnd)
    {
        focus->SetFocus();
        SOHOS_LOGI("restore IME edit focus swnd=%{public}u class=%{public}ls previous=%{public}u",
                   swnd, focus->GetObjectClass(), curFocus);
    }
}

void insertQueuedImeText(const std::string &text)
{
    if (text.empty())
        return;

    restoreImeEditFocusIfNeeded();

    unsigned int swnd = g_lastImeFocusSwnd.load();
    SWindow *focus = SWindowMgr::GetWindow(swnd);
    if (!focus)
    {
        SHostWnd *host = getMainHost();
        const ISwndContainer *container = host && host->GetRoot() ? host->GetRoot()->GetContainer() : nullptr;
        focus = container ? SWindowMgr::GetWindow(container->GetFocus()) : nullptr;
    }

    SRichEdit *edit = focus && focus->IsClass(SRichEdit::GetClassName()) ? sobj_cast<SRichEdit>(focus) : nullptr;
    if (!edit)
    {
        SOHOS_LOGW("insert IME text skipped focus=%{public}p swnd=%{public}u len=%{public}u",
                   reinterpret_cast<void *>(focus), swnd, static_cast<unsigned int>(text.size()));
        return;
    }

    SStringA utf8(text.c_str(), static_cast<int>(text.size()));
    SStringW str = S_CA2W(utf8, CP_UTF8);
    edit->SSendMessage(EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)str.c_str());
    std::string prefix = formatUtf8Prefix(text);
    SOHOS_LOGI("insert IME text len=%{public}u wlen=%{public}u bytes=%{public}s swnd=%{public}u class=%{public}ls",
               static_cast<unsigned int>(text.size()), static_cast<unsigned int>(str.GetLength()), prefix.c_str(),
               edit->GetSwnd(), edit->GetObjectClass());
}

bool isPrintableUnicode(UINT unicode)
{
    return unicode >= 0x20 && unicode != 0x7f;
}

bool isTextInputVk(UINT vk)
{
    return (vk >= '0' && vk <= '9') || (vk >= 'A' && vk <= 'Z') || vk == VK_SPACE ||
           (vk >= VK_OEM_1 && vk <= VK_OEM_8);
}

void updateModifierState(UINT vk, bool down)
{
    if (vk == VK_SHIFT)
        g_shiftDown = down;
    else if (vk == VK_CONTROL)
        g_ctrlDown = down;
    else if (vk == VK_MENU)
        g_altDown = down;
    else if (vk == VK_CAPITAL && down)
        g_capsLock = !g_capsLock.load();
}

bool isModifierVk(UINT vk)
{
    return vk == VK_SHIFT || vk == VK_CONTROL || vk == VK_MENU || vk == VK_CAPITAL;
}

void queueSynthModifierDown(unsigned int mask)
{
    if (mask & 1)
        queueWindowMessage(WM_KEYDOWN, VK_CONTROL, 0);
    if (mask & 2)
        queueWindowMessage(WM_SYSKEYDOWN, VK_MENU, 0);
    if (mask & 4)
        queueWindowMessage(WM_KEYDOWN, VK_SHIFT, 0);
}

void queueSynthModifierUp(unsigned int mask)
{
    if (mask & 4)
        queueWindowMessage(WM_KEYUP, VK_SHIFT, 0);
    if (mask & 2)
        queueWindowMessage(WM_SYSKEYUP, VK_MENU, 0);
    if (mask & 1)
        queueWindowMessage(WM_KEYUP, VK_CONTROL, 0);
}

UINT textVkToChar(UINT vk)
{
    bool shift = g_shiftDown.load();
    bool caps = g_capsLock.load();
    if (vk >= 'A' && vk <= 'Z')
    {
        bool upper = shift != caps;
        return upper ? vk : (vk - 'A' + 'a');
    }
    if (vk >= '0' && vk <= '9')
    {
        static const char shiftedDigits[] = ")!@#$%^&*(";
        return shift ? shiftedDigits[vk - '0'] : vk;
    }
    switch (vk)
    {
    case VK_SPACE:
        return ' ';
    case VK_OEM_1:
        return shift ? ':' : ';';
    case VK_OEM_PLUS:
        return shift ? '+' : '=';
    case VK_OEM_COMMA:
        return shift ? '<' : ',';
    case VK_OEM_MINUS:
        return shift ? '_' : '-';
    case VK_OEM_PERIOD:
        return shift ? '>' : '.';
    case VK_OEM_2:
        return shift ? '?' : '/';
    case VK_OEM_3:
        return shift ? '~' : '`';
    case VK_OEM_4:
        return shift ? '{' : '[';
    case VK_OEM_5:
        return shift ? '|' : '\\';
    case VK_OEM_6:
        return shift ? '}' : ']';
    case VK_OEM_7:
        return shift ? '"' : '\'';
    default:
        return 0;
    }
}

UINT ohosKeyCodeToVk(UINT keyCode)
{
    UINT vk = MapVirtualKey(keyCode, MAPVK_VSC_TO_VK);
    return vk ? vk : keyCode;
}

void CALLBACK drainInputTimer(HWND, UINT, UINT_PTR, DWORD)
{
    SHostWnd *host = getMainHost();
    if (!host || !host->m_hWnd)
        return;

    applyPendingResize();
    applyPendingImeFocusCheck();
    restoreImeEditFocusIfNeeded();

    for (int i = 0; i < 64; ++i)
    {
        QueuedInput input = {};
        {
            std::lock_guard<std::mutex> lock(g_inputMutex);
            if (g_inputQueue.empty())
                break;
            input = g_inputQueue.front();
            g_inputQueue.pop_front();
        }
        if (input.msg != WM_MOUSEMOVE)
            SOHOS_LOGI("drain input msg=%{public}u wp=%{public}llu lp=%{public}lld",
                       static_cast<unsigned int>(input.msg), static_cast<unsigned long long>(input.wp),
                       static_cast<long long>(input.lp));
        if (input.msg == kQueuedImeTextInput)
        {
            insertQueuedImeText(input.text);
            continue;
        }
        if (input.msg == WM_CHAR || input.msg == WM_IME_CHAR)
            restoreImeEditFocusIfNeeded();
        PostMessage(host->m_hWnd, input.msg, input.wp, input.lp);
    }
}

void fillXComponentState(OH_NativeXComponent *component, void *window, swinx::ohos::XComponentState &state)
{
    state = swinx::ohos::GetXComponentState();
    state.component = component;
    state.nativeWindow = window;
    state.density = state.density > 0.0f ? state.density : 1.0f;

    uint64_t width = 0;
    uint64_t height = 0;
    if (component && window && OH_NativeXComponent_GetXComponentSize(component, window, &width, &height) == 0)
    {
        state.width = static_cast<int>(width);
        state.height = static_cast<int>(height);
    }
    if (state.width <= 0)
        state.width = 1;
    if (state.height <= 0)
        state.height = 1;
}

void runApp()
{
    const SOhosAppCallbacks *callbacks = getCallbacks();
    if (!callbacks)
    {
        SOHOS_LOGE("OHOS app callbacks are not set");
        g_started = false;
        return;
    }

    OleInitialize(nullptr);

    HINSTANCE hInstance = GetModuleHandle(nullptr);
    SApplication app(hInstance);

    int renderMode = g_renderMode.load();
    Render render = renderMode == Render_Skia ? Render_Skia : Render_Gdi;
    SOHOS_LOGI("SOUI OHOS render mode=%{public}d", renderMode);

    SAppCfg cfg;
    cfg.SetRender(render)
        .SetImgDecoder(ImgDecoder_Stb)
        .SetLog(TRUE, 2, callbacks->logName ? callbacks->logName : "ohos_soui")
        .EnableMultiLang(_T("translator:lang_cn"), TRUE)
        .EnableScript(TRUE);

    cfg.SetSysResPeFile(S_CA2T(callbacks->sysResourceDir ? callbacks->sysResourceDir : "soui-sys-resource"));
    cfg.SetAppResPeFile(S_CA2T(callbacks->appName ? callbacks->appName : "libsouidemo"));

    if (callbacks->registerClasses)
        callbacks->registerClasses(app);

    void *appContext = nullptr;
    if (!cfg.DoConfig(&app))
    {
        SOHOS_LOGE("SAppCfg DoConfig failed");
        g_started = false;
        OleUninitialize();
        return;
    }

    if (callbacks->onAppConfigured)
        appContext = callbacks->onAppConfigured(app, nullptr, nullptr);

    swinx::ohos::XComponentState state = swinx::ohos::GetXComponentState();
    SHostWnd *host = callbacks->createMainWindow ? callbacks->createMainWindow(state.width, state.height) : nullptr;
    if (!host || !host->m_hWnd)
    {
        SOHOS_LOGE("createMainWindow failed host=%{public}p", reinterpret_cast<void *>(host));
        if (callbacks->destroyMainWindow && host)
            callbacks->destroyMainWindow(host);
        if (callbacks->onAppCleanup)
            callbacks->onAppCleanup(app, appContext);
        g_started = false;
        OleUninitialize();
        return;
    }

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_mainHost = host;
    }

    HWND hWnd = host->m_hWnd;
    host->GetNative()->SendMessage(WM_INITDIALOG);
    host->ShowWindow(SW_SHOWNORMAL);
    SetTimer(hWnd, kInputDrainTimer, 8, drainInputTimer);
    requestResizeMainWindow();
    InvalidateRect(hWnd, nullptr, TRUE);

    SOHOS_LOGI("SOUI OHOS window created hwnd=%{public}p size=%{public}dx%{public}d",
               reinterpret_cast<void *>(hWnd), state.width, state.height);
    app.Run(hWnd);
    KillTimer(hWnd, kInputDrainTimer);

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_mainHost = nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(g_inputMutex);
        g_inputQueue.clear();
    }
    if (callbacks->destroyMainWindow)
        callbacks->destroyMainWindow(host);
    if (callbacks->onAppCleanup)
        callbacks->onAppCleanup(app, appContext);
    OleUninitialize();
    g_started = false;
}

void startAppIfNeeded()
{
    bool expected = false;
    if (!g_started.compare_exchange_strong(expected, true))
    {
        requestResizeMainWindow();
        return;
    }

    g_appThread = std::thread(runApp);
    g_appThread.detach();
}

void onSurfaceCreated(OH_NativeXComponent *component, void *window)
{
    swinx::ohos::XComponentState state = {};
    fillXComponentState(component, window, state);
    swinx::ohos::SetXComponentState(state);
    SOHOS_LOGI("surface created window=%{public}p size=%{public}dx%{public}d", window, state.width, state.height);
    startAppIfNeeded();
}

void onSurfaceChanged(OH_NativeXComponent *component, void *window)
{
    swinx::ohos::XComponentState state = {};
    fillXComponentState(component, window, state);
    swinx::ohos::SetXComponentState(state);
    SOHOS_LOGI("surface changed window=%{public}p size=%{public}dx%{public}d", window, state.width, state.height);
    requestResizeMainWindow();
}

void onSurfaceDestroyed(OH_NativeXComponent *component, void *window)
{
    swinx::ohos::XComponentState state = swinx::ohos::GetXComponentState();
    if (state.component == component && state.nativeWindow == window)
    {
        state.nativeWindow = nullptr;
        state.width = 0;
        state.height = 0;
        swinx::ohos::SetXComponentState(state);
    }
    SOHOS_LOGI("surface destroyed window=%{public}p", window);
}

void onTouchEvent(OH_NativeXComponent *component, void *window)
{
    OH_NativeXComponent_TouchEvent event;
    memset(&event, 0, sizeof(event));
    if (OH_NativeXComponent_GetTouchEvent(component, window, &event) != 0)
        return;

    OH_NativeXComponent_EventSourceType sourceType = OH_NATIVEXCOMPONENT_SOURCE_TYPE_UNKNOWN;
    if (OH_NativeXComponent_GetTouchEventSourceType(component, event.id, &sourceType) == 0 &&
        (sourceType == OH_NATIVEXCOMPONENT_SOURCE_TYPE_MOUSE || sourceType == OH_NATIVEXCOMPONENT_SOURCE_TYPE_TOUCHPAD))
    {
        return;
    }

    swinx::ohos::XComponentState state = {};
    fillXComponentState(component, window, state);
    swinx::ohos::SetXComponentState(state);

    UINT msg = WM_MOUSEMOVE;
    WPARAM wp = 0;
    switch (event.type)
    {
    case OH_NATIVEXCOMPONENT_DOWN:
        msg = WM_LBUTTONDOWN;
        wp = MK_LBUTTON;
        g_touchDown = true;
        break;
    case OH_NATIVEXCOMPONENT_UP:
        msg = WM_LBUTTONUP;
        g_touchDown = false;
        scheduleImeFocusCheck();
        break;
    case OH_NATIVEXCOMPONENT_MOVE:
        if (!g_touchDown.load())
            return;
        msg = WM_MOUSEMOVE;
        wp = MK_LBUTTON;
        break;
    case OH_NATIVEXCOMPONENT_CANCEL:
        msg = WM_LBUTTONUP;
        g_touchDown = false;
        break;
    default:
        return;
    }
    queueInput(msg, wp, static_cast<int>(event.x), static_cast<int>(event.y));
}

WPARAM mouseButtonToWParam(OH_NativeXComponent_MouseEventButton button)
{
    WPARAM wp = 0;
    if (button & OH_NATIVEXCOMPONENT_LEFT_BUTTON)
        wp |= MK_LBUTTON;
    if (button & OH_NATIVEXCOMPONENT_RIGHT_BUTTON)
        wp |= MK_RBUTTON;
    if (button & OH_NATIVEXCOMPONENT_MIDDLE_BUTTON)
        wp |= MK_MBUTTON;
    return wp;
}

void onMouseEvent(OH_NativeXComponent *component, void *window)
{
    OH_NativeXComponent_MouseEvent event;
    memset(&event, 0, sizeof(event));
    if (OH_NativeXComponent_GetMouseEvent(component, window, &event) != 0)
        return;

    swinx::ohos::XComponentState state = {};
    fillXComponentState(component, window, state);
    swinx::ohos::SetXComponentState(state);

    UINT msg = WM_MOUSEMOVE;
    unsigned int buttonMask = static_cast<unsigned int>(mouseButtonToWParam(event.button));
    unsigned int buttonsDown = g_mouseButtonsDown.load();
    switch (event.action)
    {
    case OH_NATIVEXCOMPONENT_MOUSE_PRESS:
        if (event.button & OH_NATIVEXCOMPONENT_RIGHT_BUTTON)
            msg = WM_RBUTTONDOWN;
        else if (event.button & OH_NATIVEXCOMPONENT_MIDDLE_BUTTON)
            msg = WM_MBUTTONDOWN;
        else
            msg = WM_LBUTTONDOWN;
        buttonsDown |= buttonMask ? buttonMask : MK_LBUTTON;
        break;
    case OH_NATIVEXCOMPONENT_MOUSE_RELEASE:
        if (event.button & OH_NATIVEXCOMPONENT_RIGHT_BUTTON)
            msg = WM_RBUTTONUP;
        else if (event.button & OH_NATIVEXCOMPONENT_MIDDLE_BUTTON)
            msg = WM_MBUTTONUP;
        else
            msg = WM_LBUTTONUP;
        buttonsDown &= ~(buttonMask ? buttonMask : MK_LBUTTON);
        scheduleImeFocusCheck();
        break;
    case OH_NATIVEXCOMPONENT_MOUSE_MOVE:
        msg = WM_MOUSEMOVE;
        break;
    case OH_NATIVEXCOMPONENT_MOUSE_CANCEL:
        msg = WM_LBUTTONUP;
        buttonsDown = 0;
        break;
    default:
        return;
    }
    g_mouseButtonsDown = buttonsDown;
    queueInput(msg, buttonsDown, static_cast<int>(event.x), static_cast<int>(event.y));
}

void onHoverEvent(OH_NativeXComponent *, bool isHover)
{
    if (!isHover && g_leftButtonQueued.load())
        queueInput(WM_LBUTTONUP, 0, g_lastInputX.load(), g_lastInputY.load());
    else if (!isHover)
        queueInput(WM_MOUSELEAVE, 0, 0, 0);
}

void onKeyEvent(OH_NativeXComponent *component, void *)
{
    OH_NativeXComponent_KeyEvent *event = nullptr;
    if (OH_NativeXComponent_GetKeyEvent(component, &event) != 0 || !event)
        return;

    OH_NativeXComponent_KeyAction action = OH_NATIVEXCOMPONENT_KEY_ACTION_UNKNOWN;
    OH_NativeXComponent_KeyCode keyCode = KEY_UNKNOWN;
    if (OH_NativeXComponent_GetKeyEventAction(event, &action) != 0 ||
        OH_NativeXComponent_GetKeyEventCode(event, &keyCode) != 0)
    {
        return;
    }

    UINT vk = ohosKeyCodeToVk(static_cast<UINT>(keyCode));
    if (!vk)
        return;

    bool down = action == OH_NATIVEXCOMPONENT_KEY_ACTION_DOWN;
    bool up = action == OH_NATIVEXCOMPONENT_KEY_ACTION_UP;
    if (!down && !up)
        return;

    if (vk == VK_SHIFT)
    {
        g_shiftDown = down;
        queueWindowMessage(down ? WM_KEYDOWN : WM_KEYUP, vk, 0);
        return;
    }
    if (vk == VK_CONTROL)
    {
        g_ctrlDown = down;
        queueWindowMessage(down ? WM_KEYDOWN : WM_KEYUP, vk, 0);
        return;
    }
    if (vk == VK_MENU)
    {
        g_altDown = down;
        queueWindowMessage(down ? WM_SYSKEYDOWN : WM_SYSKEYUP, vk, 0);
        return;
    }
    if (vk == VK_CAPITAL && down)
        g_capsLock = !g_capsLock.load();

    if (down && isTextInputVk(vk) && !g_ctrlDown.load() && !g_altDown.load())
    {
        UINT ch = textVkToChar(vk);
        if (ch)
            queueWindowMessage(WM_CHAR, ch, 0);
        return;
    }

    queueWindowMessage(down ? WM_KEYDOWN : WM_KEYUP, vk, 0);
}

bool registerNativeXComponent(napi_env env, napi_value value)
{
    void *native = nullptr;
    if (napi_unwrap(env, value, &native) != napi_ok || !native)
    {
        SOHOS_LOGE("napi_unwrap XComponent failed");
        return false;
    }

    static OH_NativeXComponent_Callback callback = {
        onSurfaceCreated,
        onSurfaceChanged,
        onSurfaceDestroyed,
        onTouchEvent,
    };
    int ret = OH_NativeXComponent_RegisterCallback(static_cast<OH_NativeXComponent *>(native), &callback);
    static OH_NativeXComponent_MouseEvent_Callback mouseCallback = {
        onMouseEvent,
        onHoverEvent,
    };
    int mouseRet = OH_NativeXComponent_RegisterMouseEventCallback(static_cast<OH_NativeXComponent *>(native), &mouseCallback);
    int keyRet = OH_NativeXComponent_RegisterKeyEventCallback(static_cast<OH_NativeXComponent *>(native), onKeyEvent);
    int keyboardRet = OH_NativeXComponent_SetNeedSoftKeyboard(static_cast<OH_NativeXComponent *>(native), true);
    SOHOS_LOGI("RegisterCallback ret=%{public}d", ret);
    if (mouseRet != 0)
        SOHOS_LOGW("RegisterMouseEventCallback ret=%{public}d", mouseRet);
    if (keyRet != 0)
        SOHOS_LOGW("RegisterKeyEventCallback ret=%{public}d", keyRet);
    if (keyboardRet != 0)
        SOHOS_LOGW("SetNeedSoftKeyboard ret=%{public}d", keyboardRet);
    return ret == 0;
}

napi_value setResourceRoot(napi_env env, napi_callback_info info)
{
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

napi_value setRenderMode(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    int32_t mode = Render_Gdi;
    bool ok = argc > 0 && argv[0] && napi_get_value_int32(env, argv[0], &mode) == napi_ok &&
              (mode == Render_Gdi || mode == Render_Skia) && !g_started.load();
    if (ok)
    {
        g_renderMode = mode;
        SOHOS_LOGI("set render mode=%{public}d", mode);
    }

    napi_value result;
    napi_get_boolean(env, ok, &result);
    return result;
}

napi_value dispatchKeyEvent(napi_env env, napi_callback_info info)
{
    size_t argc = 5;
    napi_value argv[5] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    int32_t keyCode = 0;
    int32_t keyType = 0;
    uint32_t unicode = 0;
    if (argc > 0 && argv[0])
        napi_get_value_int32(env, argv[0], &keyCode);
    if (argc > 1 && argv[1])
        napi_get_value_int32(env, argv[1], &keyType);
    if (argc > 2 && argv[2])
        napi_get_value_uint32(env, argv[2], &unicode);

    char keyText[64] = { 0 };
    size_t keyTextLen = 0;
    if (argc > 3 && argv[3])
        napi_get_value_string_utf8(env, argv[3], keyText, sizeof(keyText), &keyTextLen);
    int32_t modifierMask = 0;
    if (argc > 4 && argv[4])
        napi_get_value_int32(env, argv[4], &modifierMask);

    UINT vk = ohosKeyCodeToVk(static_cast<UINT>(keyCode));
    bool handled = false;
    bool down = keyType == 0;
    bool up = keyType == 1;
    bool altActiveBefore = g_altDown.load();
    if (vk && (down || up))
        updateModifierState(vk, down);
    bool effectiveCtrl = g_ctrlDown.load() || ((modifierMask & 1) != 0);
    bool effectiveAlt = g_altDown.load() || ((modifierMask & 2) != 0);
    bool effectiveShift = g_shiftDown.load() || ((modifierMask & 4) != 0);
    bool useSysKey = vk == VK_MENU || effectiveAlt || altActiveBefore;
    bool synthCtrl = down && vk && !isModifierVk(vk) && ((modifierMask & 1) != 0) && !g_ctrlDown.load();
    bool synthAlt = down && vk && !isModifierVk(vk) && ((modifierMask & 2) != 0) && !g_altDown.load();
    bool synthShift = down && vk && !isModifierVk(vk) && ((modifierMask & 4) != 0) && !g_shiftDown.load();
    SOHOS_LOGI("dispatch key keyCode=%{public}d type=%{public}d unicode=%{public}u vk=%{public}u ctrl=%{public}d alt=%{public}d shift=%{public}d mask=%{public}d textLen=%{public}u",
               keyCode, keyType, unicode, vk, effectiveCtrl ? 1 : 0, effectiveAlt ? 1 : 0,
               effectiveShift ? 1 : 0, modifierMask, static_cast<unsigned int>(keyTextLen));
    if (keyType == 0)
    {
        if (isPrintableUnicode(unicode) && !effectiveCtrl && !effectiveAlt)
        {
            if (keyTextLen > 0)
                queueUtf8Text(keyText, keyTextLen);
            else
                queueWindowMessage(WM_CHAR, static_cast<WPARAM>(unicode), 0);
            handled = true;
        }
        else
        {
            if (vk)
            {
                unsigned int synthMask = (synthCtrl ? 1 : 0) | (synthAlt ? 2 : 0) | (synthShift ? 4 : 0);
                if (synthMask)
                {
                    queueSynthModifierUp(g_synthModifierMask.exchange(0));
                    g_synthModifierKeyVk = vk;
                    g_synthModifierMask = synthMask;
                    queueSynthModifierDown(synthMask);
                }
                queueWindowMessage(useSysKey ? WM_SYSKEYDOWN : WM_KEYDOWN, vk, 0);
                handled = true;
            }
        }
    }
    else if (keyType == 1)
    {
        if (vk)
        {
            queueWindowMessage(useSysKey ? WM_SYSKEYUP : WM_KEYUP, vk, 0);
            unsigned int synthMask = g_synthModifierMask.load();
            if (synthMask && (g_synthModifierKeyVk.load() == vk || !isModifierVk(vk)))
            {
                queueSynthModifierUp(g_synthModifierMask.exchange(0));
                g_synthModifierKeyVk = 0;
            }
            handled = true;
        }
    }

    napi_value result;
    napi_get_boolean(env, handled, &result);
    return result;
}

napi_value setImeRequestCallback(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    napi_valuetype type = napi_undefined;
    bool ok = argc > 0 && argv[0] && napi_typeof(env, argv[0], &type) == napi_ok && type == napi_function;
    if (ok)
    {
        std::lock_guard<std::mutex> lock(g_imeRequestCallbackMutex);
        if (g_imeRequestCallback)
        {
            napi_release_threadsafe_function(g_imeRequestCallback, napi_tsfn_abort);
            g_imeRequestCallback = nullptr;
        }

        napi_value name = nullptr;
        napi_create_string_utf8(env, "soui_ime_request", NAPI_AUTO_LENGTH, &name);
        napi_status status = napi_create_threadsafe_function(env, argv[0], nullptr, name, 0, 1, nullptr, nullptr,
                                                             nullptr, callImeRequestCallback, &g_imeRequestCallback);
        ok = status == napi_ok;
        SOHOS_LOGI("setImeRequestCallback status=%{public}d", static_cast<int>(status));
    }

    napi_value result;
    napi_get_boolean(env, ok, &result);
    return result;
}

napi_value setImeProxyActive(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    bool active = false;
    bool ok = argc > 0 && argv[0] && napi_get_value_bool(env, argv[0], &active) == napi_ok;
    if (ok)
    {
        swinx::ohos::SetImeProxyActive(active);
        if (active && g_lastImeFocusSwnd.load())
            g_restoreImeFocusPending = true;
        SOHOS_LOGI("set IME proxy active=%{public}d", active ? 1 : 0);
    }

    napi_value result;
    napi_get_boolean(env, ok, &result);
    return result;
}

napi_value dispatchTextInput(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    size_t textLen = 0;
    bool handled = argc > 0 && argv[0] && napi_get_value_string_utf8(env, argv[0], nullptr, 0, &textLen) == napi_ok &&
                   textLen > 0;
    if (handled)
    {
        std::vector<char> text(textLen + 1, 0);
        handled = napi_get_value_string_utf8(env, argv[0], text.data(), text.size(), &textLen) == napi_ok && textLen > 0;
        if (handled)
            queueUtf8Text(text.data(), textLen);
    }

    napi_value result;
    napi_get_boolean(env, handled, &result);
    return result;
}

} // namespace

napi_value SOhosNapiInit(napi_env env, napi_value exports, const SOhosAppCallbacks *callbacks)
{
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_callbacks = callbacks;
    }
    SOHOS_LOGI("NAPI init app=%{public}s", callbacks && callbacks->appName ? callbacks->appName : "");
    napi_value xcomponent;
    if (napi_get_named_property(env, exports, OH_NATIVE_XCOMPONENT_OBJ, &xcomponent) == napi_ok)
        registerNativeXComponent(env, xcomponent);

    napi_property_descriptor desc[] = {
        { "setResourceRoot", nullptr, setResourceRoot, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "setRenderMode", nullptr, setRenderMode, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "dispatchKeyEvent", nullptr, dispatchKeyEvent, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "setImeRequestCallback", nullptr, setImeRequestCallback, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "setImeProxyActive", nullptr, setImeProxyActive, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "dispatchTextInput", nullptr, dispatchTextInput, nullptr, nullptr, nullptr, napi_default, nullptr },
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}

SNSEND

#endif // __OHOS__
