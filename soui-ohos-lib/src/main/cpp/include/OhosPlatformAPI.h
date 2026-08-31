//
// SOUI for OHOS 平台 API 单例。
// 完全对应 Android 端 AndroidPlatformAPI，差异：
//   - JNIEnv* 替换为 napi_env（OHOS N-API 环境）
//   - jobject(GlobalRef) 替换为 napi_ref（全局 N-API 对象引用）
//   - AAssetManager 替换为 OHOS RawfileManager
//   - jmethodID 替换为 napi_value（N-API 属性句柄）
// 目标：保持 swinx 平台层 → PlatformAPI → 单例 → N-API → ArkTS 的
// 调用链路与 Android 同构，便于业务层在两端共享。
//
#ifndef SOUI4OHOS_OHOSPLATFORMAPI_H
#define SOUI4OHOS_OHOSPLATFORMAPI_H

#include <windows.h>
#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <unordered_map>
#include <vector>

#include "soui4ohos.h"

#ifdef __cplusplus
extern "C" {
#endif

// N-API 环境 & 值封装（OHOS 原生 N-API 类型前置声明）
typedef struct napi_env__ * napi_env;
typedef struct napi_value__ * napi_value;
typedef struct napi_ref__ * napi_ref;
typedef struct napi_threadsafe_function__ * napi_threadsafe_function;

typedef enum {
    napi_tsfn_release,
    napi_tsfn_abort
} napi_threadsafe_function_release_mode;

typedef enum {
    napi_tsfn_nonblocking,
    napi_tsfn_blocking
} napi_threadsafe_function_call_mode;

// 线程安全函数回调：在 JS（env 所属）线程上执行
typedef void (*napi_threadsafe_function_call_js)(napi_env env, napi_value js_callback,
                                                 void *context, void *data);
// 线程退出时回调，用于配对 napi_acquire_threadsafe_function
typedef void (*napi_finalize)(napi_env env, void *finalize_data, void *finalize_hint);

#ifdef __cplusplus
} // extern "C"
#endif

// ============================================================
// 跨线程 Bridge 调用：参数/结果封装
//   napi_value 不能跨线程传递（只属于创建它的 env 与线程），
//   因此跨线程时只传 C++ 标量，在 JS 线程现场构造 napi_value。
// ============================================================
enum class BridgeArgType { Int64, Int32, Bool, String };

struct BridgeArg {
    BridgeArgType type = BridgeArgType::Int64;
    int64_t i = 0;
    bool b = false;
    std::string s;

    static BridgeArg fromInt64(int64_t v) { BridgeArg a; a.type = BridgeArgType::Int64; a.i = v; return a; }
    static BridgeArg fromInt32(int32_t v) { BridgeArg a; a.type = BridgeArgType::Int32; a.i = v; return a; }
    static BridgeArg fromBool(bool v)     { BridgeArg a; a.type = BridgeArgType::Bool;   a.b = v; return a; }
    static BridgeArg fromStr(const std::string &v) { BridgeArg a; a.type = BridgeArgType::String; a.s = v; return a; }
};

enum class BridgeRetType { None, Int64, Bool, String };

struct BridgeResult {
    bool ok = false;   // JS 调用是否成功取得结果
    int64_t i64 = 0;
    bool b = false;
    std::string s;
};

class SouiSurfaceProxy; // 前向声明

class OhosPlatformAPI {
public:
    // 单例
    static OhosPlatformAPI &instance();

    OhosPlatformAPI();
    ~OhosPlatformAPI();

    // ---- 生命周期：init / deinit ----

    // ArkTS 启动时调用（对应 AndroidPlatformAPI::init）
    void init(napi_env env, napi_value bridge, napi_value ctx);
    void deinit();

    // ---- 业务入口：initSouiApp / screenStartup 等 ----

    BOOL initSouiApp(napi_env env, napi_value rawfileMgr, std::string const& appFilesDir);
    void uninitSouiApp(napi_env env);
    // SOUI 启动入口（对应 Android nativeScreenStartup）
    // screenHwnd = ArkUI 主 Surface 的 HWND（= napi_ref 指针值），
    // 必须注册进 swinx 供主 Surface render 时 GetDC 反查
    HWND screenStartup(napi_env env, long screenId, UINT_PTR screenHwnd, LPCSTR pszLayout);
    void screenShutdown(napi_env env, long screenId);

    void setScreenHwnd(long screenId, UINT_PTR screenHwnd);
    HWND getScreenHwnd(long screenId) const;
    void eraseScreen(long screenId);

    // ---- SouiSurfaceProxy：自绘 Surface ↔ C++ 映射 ----

    // 新建一个 Surface Proxy 并插入 map（对应 Android createNative）
    UINT_PTR nativeViewCreate(napi_env env, napi_value nativeWnd);
    // 根据 nativeId 查找 Proxy（线程安全）
    std::shared_ptr<SouiSurfaceProxy> nativeViewLookup(UINT_PTR nativeId);
    // 销毁 Proxy（ArkTS 侧组件释放时调用）
    void nativeViewDestroy(UINT_PTR nativeId);
    // HWND → ArkTS 侧 NativeWindow 对象（napi_ref）
    napi_value hwndAsNativeWindow(UINT_PTR hwnd);

    // ---- 定时器：ArkTS 侧 setTimer/killTimer ----

    // Win32 SetTimer，委托 ArkTS setInterval，proc 在 onTimerExpired 直接回调（对齐 Android）
    DWORD_PTR setTimer(HWND hWnd, UINT_PTR id, UINT uElapse, TIMERPROC proc);
    // Win32 KillTimer
    BOOL killTimer(HWND hWnd, UINT_PTR id);
    // 一次性销毁某窗口全部定时器
    void killWindowTimers(HWND hWnd);
    // ArkTS 侧触发定时回调时，由 N-API 入口调用
    void onTimerExpired(HWND hWnd, UINT_PTR timerId);

    // ---- 输入：setFocus / showSoftKeyboard / IME ----

    HWND setFocus(HWND hWnd);
    HWND getFocus();
    BOOL showSoftKeyboard(HWND hWnd, BOOL bShow);
    void sendImeString(HWND hWnd, const char *utf8, size_t len);
    void notifyFocusGained(HWND hWnd);
    // 软键盘高度变化（WM_SOUI_KEYBOARD_HEIGHT 下发）
    void setKeyboardHeight(int keyboardHeight);

    // ---- 音频：playSound（委托 ArkTS AudioPlayer）----

    // 播放 WAV 文件，语义对齐 Win32 PlaySound / Android playSound
    BOOL playSound(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);

    // ---- 路径：getCacheDir / getFilesDir / getTempPath ----

    // 返回应用私有缓存目录（UTF-8），对应 platform_api getTempPathA
    DWORD getTempPathA(DWORD nBufferLength, LPSTR lpBuffer);
    // CSIDL 特殊目录解析，对应 platform_api getSpecialFolderPathA
    BOOL getSpecialFolderPathA(int nFolder, BOOL fCreate, LPSTR pszPath, DWORD cchPath);

    // ---- Window 管理：createWindow / postMessage ----

    HWND createWindow(HWND hParent, UINT_PTR screenId,
                      LPCSTR pszClassName, LPCSTR pszTitle,
                      DWORD dwStyle, DWORD dwExStyle,
                      int x, int y, int w, int h,
                      HMENU hMenuOrCtrlId);

    // ---- Window 管理扩展：销毁/显示/移动/尺寸/状态 ----

    // 销毁窗口（委托 ArkTS destroyView）
    BOOL destroyWindow(HWND hWnd);
    // Win32 GetWindow（GW_HWND*/GW_PARENT/GW_CHILD 等），简化实现
    HWND getWindow(HWND hWnd, int code);
    // 移动并调整大小（bRepaint 时补一次 invalidateRect）
    BOOL moveWindow(HWND hWnd, int x, int y, int nWidth, int nHeight, BOOL bRepaint);
    // Win32 SetWindowPos，按 uFlags 拆分 move / size
    BOOL setWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
    // 仅调整大小
    BOOL setWindowSize(HWND hWnd, int cx, int cy);
    // 仅移动位置
    BOOL setWindowPosEx(HWND hWnd, int x, int y);
    // 请求重绘（委托 ArkTS invalidateRect）
    BOOL invalidateRect(HWND hWnd, int left, int top, int right, int bottom);
    BOOL isWindowVisible(HWND hWnd);
    // 显示/隐藏（委托 ArkTS showView）
    BOOL showWindow(HWND hWnd, int nCmdShow);
    BOOL enableWindow(HWND hWnd, BOOL bEnable);
    BOOL isWindowEnabled(HWND hWnd);

    // ---- Screen：激活栈 + 屏幕尺寸 ----

    // 压入激活屏幕（screenStartup 时调用，对应 Android pushActiveScreen）
    long pushActiveScreen(long screenId);
    long popActiveScreen(long screenId);
    // 当前激活屏幕 ID（HWND_DESKTOP 顶层窗口路由 / getScreenWidth 用）
    long getActiveScreenId() const;
    // 屏幕宽度（px），无激活屏幕时返回 1280 兜底
    int getScreenWidth();
    int getScreenHeight();
    void getWorkArea(RECT *prc);

    // ---- 捕获 / 焦点 / 光标 / DPI / 输入设备 ----

    HWND setCapture(HWND hWnd);
    BOOL releaseCapture();
    // 委托 ArkTS scheduleMessageProcessing（postMessage 语义）
    void postMessage();
    // 屏幕 DPI（ArkTS 读取，缓存兜底 160）
    int getDpi();
    // 光标位置（OHOS 触摸为主，返回 FALSE）
    BOOL getCursorPos(LPPOINT ppt);
    // 查询当前鼠标按键状态（MK_LBUTTON | MK_RBUTTON | MK_MBUTTON）
    DWORD getMouseButtons() const { return m_mouseButtons.load(); }
    void setMouseButtons(unsigned int mk) { m_mouseButtons.store(mk); }
    // 输入设备列表（OHOS 暂不枚举物理设备，返回 0）
    UINT getRawInputDeviceList(PRAWINPUTDEVICELIST pRawInputDeviceList, PUINT puiNumDevices, UINT cbSize);
    UINT getRawInputDeviceInfoA(HRAWINPUT hDevice, UINT uiCommand, LPVOID pData, PUINT pcbSize);
    UINT getRawInputDeviceInfoW(HRAWINPUT hDevice, UINT uiCommand, LPVOID pData, PUINT pcbSize);

    // ---- 剪贴板（委托 ArkTS pasteboard）----

    // 打开剪贴板并记录 owner（与 Win32 语义对齐）
    BOOL clipboardOpen(HWND hWndNewOwner);
    BOOL clipboardClose();
    BOOL clipboardEmpty();
    // 读取文本：CF_UNICODETEXT → 宽字符 HGLOBAL，其它 → UTF-8 HGLOBAL
    HANDLE clipboardGetData(UINT uFormat);
    // 写入文本：委托 ArkTS setClipboardSysText
    HANDLE clipboardSetData(UINT uFormat, HANDLE hMem);
    BOOL clipboardIsFormatAvailable(UINT format);
    // 自定义格式名 → 0xC000+ 静态分配（与 Android 一致）
    UINT clipboardRegisterFormat(LPCSTR pszName);
    HWND clipboardGetOwner();
    BOOL clipboardHasFormat(UINT format);
    // 投递 scheduleMessageProcessing：ArkTS 侧用 setTimeout(0) 触发一次消息处理
    void scheduleMessageProcessing();

    // ---- 消息驱动：executePendingTask / processIdle ----

    void executePendingTask();
    BOOL processIdle(int idleCount);

    // ---- 字符串槽：ID 化 C++/ArkTS 字符串传递 ----
    // 完全对应 Android 端 string slot 机制（0-65535 循环复用）
    int  stringSlotAlloc(std::string const& s);
    void stringSlotFree(int slotId);
    std::string ReadString(int slotId);
    void WriteString(int slotId, std::string const& s);

    // ---- Rawfile 资源访问（内部用）----
    SouiOhosAssetManager getAssetMgr() const { return m_pAssetMgr; }

private:
    mutable std::mutex m_mutex;

    napi_env m_env{nullptr};
    napi_ref m_bridgeRef{nullptr};    // SouiPlatformBridge 单例（强全局引用）
    napi_ref m_ctxRef{nullptr};       // Context 强全局引用

    // SouiPlatformBridge 属性缓存（等价于 Android 的 jmethodID）
    napi_ref m_mCreateWindow{nullptr};
    napi_ref m_mSetTimer{nullptr};
    napi_ref m_mKillTimer{nullptr};
    napi_ref m_mKillWindowTimers{nullptr};
    napi_ref m_mSetFocus{nullptr};
    napi_ref m_mGetFocus{nullptr};
    napi_ref m_mShowSoftKeyboard{nullptr};
    napi_ref m_mPlaySound{nullptr};               // playSound(path, fdwSound): boolean
    napi_ref m_mScheduleMessageProcessing{nullptr};
    napi_ref m_mGetTempPath{nullptr};
    napi_ref m_mGetSpecialFolderPath{nullptr};
    napi_ref m_mHwndAsNativeWindow{nullptr};

    // Window 管理扩展（ArkTS SouiPlatformBridge 对应方法）
    napi_ref m_mDestroyView{nullptr};       // destroyView(hwnd): boolean
    napi_ref m_mShowView{nullptr};          // showView(hwnd, cmdShow): boolean
    napi_ref m_mMoveWindow{nullptr};        // moveWindow(hwnd, x, y, w, h): boolean
    napi_ref m_mSetWindowSize{nullptr};     // setWindowSize(hwnd, cx, cy): boolean
    napi_ref m_mSetWindowPosEx{nullptr};    // setWindowPosEx(hwnd, x, y): boolean
    napi_ref m_mSetWindowPos{nullptr};      // setWindowPos(hwnd, insertAfter, x, y, cx, cy, flags): boolean
    napi_ref m_mInvalidateRect{nullptr};    // invalidateRect(hwnd, l, t, r, b): boolean
    napi_ref m_mIsWindowVisible{nullptr};   // isWindowVisible(hwnd): boolean
    napi_ref m_mEnableWindow{nullptr};      // enableWindow(hwnd, enabled): boolean
    napi_ref m_mIsWindowEnabled{nullptr};   // isWindowEnabled(hwnd): boolean
    napi_ref m_mGetWindow{nullptr};         // getWindow(hwnd, code): number
    napi_ref m_mSetCapture{nullptr};        // setCapture(hwnd): number
    napi_ref m_mReleaseCapture{nullptr};    // releaseCapture(): boolean
    napi_ref m_mGetScreenSize{nullptr};     // getScreenSize(screenId): number (w<<32|h)
    napi_ref m_mGetDpi{nullptr};            // getDpi(): number

    // Clipboard（ArkTS pasteboard）
    napi_ref m_mGetClipboardSysText{nullptr}; // getClipboardSysText(): string
    napi_ref m_mSetClipboardSysText{nullptr}; // setClipboardSysText(text: string): void

    std::map<UINT_PTR, std::shared_ptr<SouiSurfaceProxy>> m_nativeViews;
    UINT_PTR m_nextNativeId{1};

    // Screen 管理
    std::map<long, UINT_PTR> m_screenHwnds;
    // 激活屏幕栈（对应 Android m_activeScreenStack）
    std::vector<long> m_activeScreenStack;

    SouiOhosAssetManager m_pAssetMgr{nullptr};

    // 屏幕密度（ArkTS getDpi 兜底），剪贴板 owner
    int m_density{160};
    HWND m_clipOwner{0};
    std::atomic<DWORD> m_mouseButtons{0};  // MK_LBUTTON | MK_RBUTTON | MK_MBUTTON

    // 定时器条目：hWnd → (timerId → TIMERPROC)，与 Android m_timerEntries 镜像，
    // C++ 层仅记录 proc 回调；真正调度由 ArkTS setInterval 完成。
    std::unordered_map<UINT_PTR, std::unordered_map<UINT_PTR, TIMERPROC>> m_timerEntries;

    // String slots（与 Android 端一致的 slot 机制）
    struct SlotEntry { std::string data; };
    std::vector<SlotEntry> m_slots;
    std::vector<int> m_slotFree;

    // 直接调用 ArkTS Bridge（仅允许在 JS 线程调用）；非 JS 线程请使用 invokeBridge
    napi_value callBridge(napi_ref methodRef, size_t argc = 0, napi_value *argv = nullptr);

    // ============================================================
    // 跨线程安全的 Bridge 调用
    //   napi_env 只能在创建它的 JS 线程上使用，从工作线程直接
    //   napi_call_function 会崩溃。这里通过 napi_threadsafe_function
    //   把调用投递回 JS 线程执行。
    // ============================================================
public:
    // 同步等待上限（毫秒）：JS 线程被阻塞时不至于永久挂起工作线程，超时按失败处理。
    // 用 enum 而不用 constexpr static：本库以 C++11 编译，避免 ODR-use 问题。
    enum { kBridgeWaitMs = 2000 };

    // 当前线程是否为 m_env 所属 JS 线程
    bool isJsThread() const { return std::this_thread::get_id() == m_jsThreadId; }

    // 线程安全地调用 ArkTS Bridge 方法
    // waitMs > 0 同步等待（带超时，避免死锁）；<= 0 异步投递不等待（恒返回 true）
    bool invokeBridge(napi_ref methodRef, const std::vector<BridgeArg> &args,
                      BridgeRetType retType, BridgeResult *out, int waitMs = kBridgeWaitMs);

private:
    // 跨线程调用任务（shared_ptr 持有：等待方与 JS 线程各持一份，
    // 超时返回后对象仍存活，避免等待方栈对象已销毁而 JS 线程仍在写结果）
    struct BridgeTask {
        napi_ref methodRef = nullptr;
        std::vector<BridgeArg> args;
        BridgeRetType retType = BridgeRetType::None;
        BridgeResult local;            // 结果：JS 线程写，调用方读
        bool done = false;
        std::mutex mtx;
        std::condition_variable cv;
    };

    // 在 JS 线程上真正执行一次 bridge 调用（含参数构造与结果转换）
    void executeBridgeTask(BridgeTask *task);
    // napi_threadsafe_function 的 JS 线程回调（静态）
    static void bridgeCallJsCb(napi_env env, napi_value jsCallback, void *context, void *data);
    // 工作线程退出时的清理回调（静态）
    static void bridgeThreadFinalizeCb(napi_env env, void *finalizeData, void *finalizeHint);

    // 创建/销毁线程安全函数（必须在 JS 线程调用）
    void createThreadSafeFn();
    void destroyThreadSafeFn();

    std::thread::id m_jsThreadId{};          // m_env 所属线程（init 时记录）
    napi_threadsafe_function m_tsfn{nullptr};
};

#endif // SOUI4OHOS_OHOSPLATFORMAPI_H
