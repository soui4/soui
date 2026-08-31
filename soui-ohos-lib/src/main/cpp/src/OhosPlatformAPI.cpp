//
// Created by setou on 2026/8/25.
// OhosPlatformAPI 实现。
//
// 调用链路：
//   swinx g_platformAPI.window.createWindow
//     ↓
//   OhosPlatformAPIReg.cpp 包装函数 → OhosPlatformAPI::createWindow
//     ↓
//   N-API: 调用 SouiPlatformBridge.createWindow
//     ↓
//   ArkTS: SouiPlatformBridge.createWindow → 根据 className 创建 INativeWindow 并 addView
//
// 完全与 Android 端 AndroidPlatformAPI 同构，仅 JNI 部分替换为 N-API。
//
#include "OhosPlatformAPI.h"
#include "SouiSurfaceProxy.h"
#include <helper/slog.h>
#include <souistd.h>
using namespace SNS;
// N-API 基础类型定义（使用前通过弱符号链接到 OH NDK）
extern "C" {
    typedef enum {
        napi_ok, napi_invalid_arg, napi_object_expected, napi_string_expected,
        napi_name_expected, napi_function_expected, napi_number_expected,
        napi_boolean_expected, napi_array_expected, napi_generic_failure,
    } napi_status;

    napi_status __attribute__((weak))
        napi_create_reference(napi_env, napi_value, uint32_t, napi_ref*);
    napi_status __attribute__((weak))
        napi_delete_reference(napi_env, napi_ref);
    napi_status __attribute__((weak))
        napi_get_reference_value(napi_env, napi_ref, napi_value*);
    napi_status __attribute__((weak))
        napi_get_named_property(napi_env, napi_value, const char*, napi_value*);
    napi_status __attribute__((weak))
        napi_call_function(napi_env, napi_value recv, napi_value func,
                           size_t argc, const napi_value* argv, napi_value* result);
    napi_status __attribute__((weak))
        napi_get_value_string_utf8(napi_env, napi_value, char*, size_t, size_t*);
    napi_status __attribute__((weak))
        napi_create_string_utf8(napi_env, const char*, size_t, napi_value*);
    napi_status __attribute__((weak))
        napi_get_value_int32(napi_env, napi_value, int32_t*);
    napi_status __attribute__((weak))
        napi_get_value_uint32(napi_env, napi_value, uint32_t*);
    napi_status __attribute__((weak))
        napi_get_value_int64(napi_env, napi_value, int64_t*);
    napi_status __attribute__((weak))
        napi_create_int32(napi_env, int32_t, napi_value*);
    napi_status __attribute__((weak))
        napi_create_int64(napi_env, int64_t, napi_value*);
    napi_status __attribute__((weak))
        napi_create_object(napi_env, napi_value*);
    napi_status __attribute__((weak))
        napi_coerce_to_bool(napi_env, napi_value, napi_value*);
    napi_status __attribute__((weak))
        napi_get_value_bool(napi_env, napi_value, bool*);

    typedef enum {
        napi_undefined, napi_null, napi_boolean, napi_number,
        napi_string, napi_symbol, napi_object, napi_function,
        napi_external, napi_bigint,
    } napi_valuetype;
    napi_status __attribute__((weak))
        napi_typeof(napi_env, napi_value, napi_valuetype*);

    bool __attribute__((weak)) napi_is_exception_pending(napi_env);
    void __attribute__((weak)) napi_clear_last_exception(napi_env);
    napi_status __attribute__((weak))
        napi_get_boolean(napi_env, bool, napi_value*);

    // 线程安全函数：用于把 Bridge 调用从工作线程投递回 JS 线程 
    napi_status __attribute__((weak))
        napi_create_threadsafe_function(napi_env env,
                                        napi_value func,
                                        napi_value async_resource,
                                        napi_value async_resource_name,
                                        size_t max_queue_size,
                                        size_t initial_thread_count,
                                        void* thread_finalize_data,
                                        napi_finalize thread_finalize_cb,
                                        void* context,
                                        napi_threadsafe_function_call_js call_js_cb,
                                        napi_threadsafe_function* result);
    napi_status __attribute__((weak))
        napi_call_threadsafe_function(napi_threadsafe_function func,
                                      void* data,
                                      napi_threadsafe_function_call_mode is_blocking);
    napi_status __attribute__((weak))
        napi_acquire_threadsafe_function(napi_threadsafe_function func);
    napi_status __attribute__((weak))
        napi_release_threadsafe_function(napi_threadsafe_function func,
                                         napi_threadsafe_function_release_mode mode);

    // ---- OHOS rawfile API（librawfile.z.so）弱引用 ----
    // 与 N-API 一样手动声明：本文件未 include native_api.h（napi_env 为手动 typedef），
    // 故不能 include <rawfile/raw_file_manager.h>，改以弱符号方式链接，运行期由系统解析。
    typedef struct NativeResourceManager NativeResourceManager;
    NativeResourceManager *__attribute__((weak))
        OH_ResourceManager_InitNativeResourceManager(napi_env, napi_value);
    void __attribute__((weak))
        OH_ResourceManager_ReleaseNativeResourceManager(NativeResourceManager *);
}

#define kLogTag "OhosPlatformAPI"

// 手动弱引用 N-API（不 include node_api.h），NAPI_AUTO_LENGTH 需自行定义
#ifndef NAPI_AUTO_LENGTH
#define NAPI_AUTO_LENGTH ((size_t)-1)  // 与官方 node_api.h 一致：字符串长度自动计算
#endif

static inline bool napi_ok_or(napi_status s, bool def) { return s == napi_ok; }
static inline void ClearEx(napi_env env) {
    if (napi_is_exception_pending && env && napi_is_exception_pending(env)) {
        napi_clear_last_exception(env);
    }
}

static Soui4OhosEntry *s_entry = nullptr;

void InitSoui4OhosEntry(Soui4OhosEntry *entry) {
    s_entry = entry;
    SLOGI() << "InitSoui4OhosEntry entry=" << (void*)entry;
}

OhosPlatformAPI &OhosPlatformAPI::instance() {
    static OhosPlatformAPI inst;
    return inst;
}

OhosPlatformAPI::OhosPlatformAPI() = default;
OhosPlatformAPI::~OhosPlatformAPI() { deinit(); }

// ============================================================
// ======== 引用辅助：桥接属性缓存 ==========================
// ============================================================ 

static napi_ref cacheMethod(napi_env env, napi_value bridge, const char *name) {
    if (!env || !bridge) return nullptr;
    napi_value func = nullptr;
    if (!napi_ok_or(napi_get_named_property(env, bridge, name, &func), false)) return nullptr;
    napi_valuetype vt = napi_undefined;
    napi_typeof(env, func, &vt);
    if (vt != napi_function) {
        SLOGE() << "cacheMethod: " << name << " is not a function";
        return nullptr;
    }
    napi_ref ref = nullptr;
    napi_create_reference(env, func, 1, &ref);
    return ref;
}

void OhosPlatformAPI::init(napi_env env, napi_value bridge, napi_value ctx) {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (m_env) { deinit(); }
    m_env = env;
    napi_create_reference(env, bridge, 1, &m_bridgeRef);
    napi_create_reference(env, ctx, 1, &m_ctxRef);

    // 缓存 SouiPlatformBridge 方法（等价 Android jmethodID）
    m_mCreateWindow               = cacheMethod(env, bridge, "createWindow");
    m_mSetTimer                   = cacheMethod(env, bridge, "setTimer");
    m_mKillTimer                  = cacheMethod(env, bridge, "killTimer");
    m_mKillWindowTimers           = cacheMethod(env, bridge, "killWindowTimers");
    m_mSetFocus                   = cacheMethod(env, bridge, "setFocus");
    m_mGetFocus                   = cacheMethod(env, bridge, "getFocus");
    m_mShowSoftKeyboard           = cacheMethod(env, bridge, "showSoftKeyboard");
    m_mPlaySound                  = cacheMethod(env, bridge, "playSound");
    m_mScheduleMessageProcessing  = cacheMethod(env, bridge, "scheduleMessageProcessing");
    m_mGetTempPath                = cacheMethod(env, bridge, "getTempPath");
    m_mGetSpecialFolderPath       = cacheMethod(env, bridge, "getSpecialFolderPath");
    m_mHwndAsNativeWindow         = cacheMethod(env, bridge, "hwndAsNativeWindow");

    // Window 管理扩展
    m_mDestroyView                = cacheMethod(env, bridge, "destroyView");
    m_mShowView                   = cacheMethod(env, bridge, "showView");
    m_mMoveWindow                 = cacheMethod(env, bridge, "moveWindow");
    m_mSetWindowSize              = cacheMethod(env, bridge, "setWindowSize");
    m_mSetWindowPosEx             = cacheMethod(env, bridge, "setWindowPosEx");
    m_mSetWindowPos               = cacheMethod(env, bridge, "setWindowPos");
    m_mInvalidateRect             = cacheMethod(env, bridge, "invalidateRect");
    m_mIsWindowVisible            = cacheMethod(env, bridge, "isWindowVisible");
    m_mEnableWindow               = cacheMethod(env, bridge, "enableWindow");
    m_mIsWindowEnabled            = cacheMethod(env, bridge, "isWindowEnabled");
    m_mGetWindow                  = cacheMethod(env, bridge, "getWindow");
    m_mSetCapture                 = cacheMethod(env, bridge, "setCapture");
    m_mReleaseCapture             = cacheMethod(env, bridge, "releaseCapture");
    m_mGetScreenSize              = cacheMethod(env, bridge, "getScreenSize");
    m_mGetDpi                     = cacheMethod(env, bridge, "getDpi");

    // Clipboard
    m_mGetClipboardSysText        = cacheMethod(env, bridge, "getClipboardSysText");
    m_mSetClipboardSysText        = cacheMethod(env, bridge, "setClipboardSysText");

    // 记录 env 所属线程：后续所有 N-API 调用都必须发生在该线程上
    m_jsThreadId = std::this_thread::get_id();
    createThreadSafeFn();

    SLOGI() << "OhosPlatformAPI init done";
}

void OhosPlatformAPI::deinit() {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (!m_env) return;
    destroyThreadSafeFn();
    napi_env env = m_env;
    napi_delete_reference(env, m_bridgeRef);  m_bridgeRef = nullptr;
    napi_delete_reference(env, m_ctxRef);     m_ctxRef = nullptr;
    auto freeRef = [&](napi_ref &r) { if (r) napi_delete_reference(env, r); r = nullptr; };
    freeRef(m_mCreateWindow);
    freeRef(m_mSetTimer);
    freeRef(m_mKillTimer);
    freeRef(m_mKillWindowTimers);
    freeRef(m_mSetFocus);
    freeRef(m_mGetFocus);
    freeRef(m_mShowSoftKeyboard);
    freeRef(m_mPlaySound);
    freeRef(m_mScheduleMessageProcessing);
    freeRef(m_mGetTempPath);
    freeRef(m_mGetSpecialFolderPath);
    freeRef(m_mHwndAsNativeWindow);
    // Window 管理扩展
    freeRef(m_mDestroyView);
    freeRef(m_mShowView);
    freeRef(m_mMoveWindow);
    freeRef(m_mSetWindowSize);
    freeRef(m_mSetWindowPosEx);
    freeRef(m_mSetWindowPos);
    freeRef(m_mInvalidateRect);
    freeRef(m_mIsWindowVisible);
    freeRef(m_mEnableWindow);
    freeRef(m_mIsWindowEnabled);
    freeRef(m_mGetWindow);
    freeRef(m_mSetCapture);
    freeRef(m_mReleaseCapture);
    freeRef(m_mGetScreenSize);
    freeRef(m_mGetDpi);
    // Clipboard
    freeRef(m_mGetClipboardSysText);
    freeRef(m_mSetClipboardSysText);
    m_activeScreenStack.clear();
    m_clipOwner = 0;
    m_timerEntries.clear(); // 对齐 Android deinit：清理定时器条目
    m_env = nullptr;
    SLOGI() << "OhosPlatformAPI deinit done";
}

// ============================================================
// ======== 调用桥接通用辅助 =================================
// ============================================================ 
napi_value OhosPlatformAPI::callBridge(napi_ref methodRef, size_t argc, napi_value *argv) {
    if (!m_env || !m_bridgeRef || !methodRef) return nullptr;
    // napi_env 只能在创建它的 JS 线程上使用。非 JS 线程调用会直接崩溃，
    // 这里退化为"调用失败 + 明确日志"，把崩溃降级为功能降级。
    // 需要跨线程能力的调用请改用 invokeBridge（它会自动投递到 JS 线程）。
    if (!isJsThread()) {
        SLOGE() << "callBridge on non-JS thread, dropped. Use invokeBridge instead.";
        return nullptr;
    }
    napi_value bridge = nullptr, func = nullptr, result = nullptr;
    napi_get_reference_value(m_env, m_bridgeRef, &bridge);
    napi_get_reference_value(m_env, methodRef, &func);
    if (!bridge || !func) return nullptr;
    napi_status s = napi_call_function(m_env, bridge, func, argc, argv, &result);
    if (s != napi_ok) {
        SLOGE() << "callBridge failed, status=" << (int)s;
        ClearEx(m_env);
        return nullptr;
    }
    return result;
}

// ============================================================
// ======== 跨线程安全的 Bridge 调用 =========================
//
// 背景：napi_env 只能在其所属（创建它的）JS 线程上使用，从工作线程
// 直接 napi_call_function 会崩溃；且 N-API 没有 JNI 那样的
// AttachCurrentThread 机制可以补救（Android 端正因有它才不受影响）。
//
// 方案：用 napi_threadsafe_function 把调用投递回 JS 线程执行。
// napi_value 同样不能跨线程传递，因此参数与结果一律用 C++ 标量承载，
// 在 JS 线程现场构造 napi_value、现场解析返回值。
// ============================================================ 

static napi_value makeNapiValue(napi_env env, const BridgeArg &a) {
    napi_value v = nullptr;
    switch (a.type) {
    case BridgeArgType::Int64:  napi_create_int64(env, a.i, &v); break;
    case BridgeArgType::Int32:  napi_create_int32(env, (int32_t)a.i, &v); break;
    case BridgeArgType::Bool:   napi_get_boolean(env, a.b, &v); break;
    case BridgeArgType::String: napi_create_string_utf8(env, a.s.c_str(), NAPI_AUTO_LENGTH, &v); break;
    }
    return v;
}

void OhosPlatformAPI::executeBridgeTask(BridgeTask *task) {
    // 只在 JS 线程执行
    task->local = BridgeResult();
    if (!m_env || !m_bridgeRef || !task->methodRef) return;

    napi_value bridge = nullptr, func = nullptr, result = nullptr;
    napi_get_reference_value(m_env, m_bridgeRef, &bridge);
    napi_get_reference_value(m_env, task->methodRef, &func);
    if (!bridge || !func) return;

    const size_t argc = task->args.size();
    std::vector<napi_value> argv(argc, nullptr);
    for (size_t i = 0; i < argc; i++) argv[i] = makeNapiValue(m_env, task->args[i]);

    napi_status s = napi_call_function(m_env, bridge, func, argc, argv.data(), &result);
    if (s != napi_ok) {
        SLOGE() << "callBridge(tsfn) failed, status=" << (int)s;
        ClearEx(m_env);
        return;
    }
    if (!result) return;

    task->local.ok = true;
    switch (task->retType) {
    case BridgeRetType::Int64:
        napi_get_value_int64(m_env, result, &task->local.i64);
        break;
    case BridgeRetType::Bool:
        napi_get_value_bool(m_env, result, &task->local.b);
        break;
    case BridgeRetType::String: {
        size_t len = 0;
        if (napi_ok_or(napi_get_value_string_utf8(m_env, result, nullptr, 0, &len), false) && len > 0) {
            task->local.s.resize(len);
            napi_get_value_string_utf8(m_env, result, &task->local.s[0], len + 1, &len);
        }
        break;
    }
    case BridgeRetType::None:
    default:
        break;
    }
}

void OhosPlatformAPI::bridgeCallJsCb(napi_env env, napi_value jsCallback, void *context, void *data) {
    (void)env; (void)jsCallback; (void)context;
    // data 是堆上的 shared_ptr 副本：取出后立刻归还堆内存，
    // 任务对象由本线程的局部 shared_ptr 保证存活。
    auto holderPtr = static_cast<std::shared_ptr<BridgeTask> *>(data);
    std::shared_ptr<BridgeTask> task;
    if (holderPtr) {
        task = *holderPtr;
        delete holderPtr;
    }
    if (!task) return;

    OhosPlatformAPI &self = instance();
    self.executeBridgeTask(task.get());

    {
        std::lock_guard<std::mutex> lk(task->mtx);
        task->done = true;
    }
    task->cv.notify_all();
    // task 在此析构：若等待方已超时返回，则由本处完成最终释放
}

void OhosPlatformAPI::bridgeThreadFinalizeCb(napi_env env, void *finalizeData, void *finalizeHint) {
    (void)env; (void)finalizeData; (void)finalizeHint;
    // 工作线程退出：配对释放该线程在 invokeBridge 中 acquire 的引用
    napi_threadsafe_function tsfn = instance().m_tsfn;
    if (tsfn) napi_release_threadsafe_function(tsfn, napi_tsfn_release);
}

void OhosPlatformAPI::createThreadSafeFn() {
    if (!m_env || !napi_create_threadsafe_function) {
        SLOGE() << "createThreadSafeFn: napi_create_threadsafe_function unavailable";
        return;
    }
    napi_value name = nullptr;
    napi_create_string_utf8(m_env, "OhosPlatformBridge", NAPI_AUTO_LENGTH, &name);
    // func 传 NULL：实际调用在 bridgeCallJsCb 内完成，不依赖 JS 侧回调
    napi_status s = napi_create_threadsafe_function(
            m_env,
            nullptr,                 // func 
            nullptr,                 // async_resource 
            name,                    // async_resource_name 
            0,                       // max_queue_size: 0 = 不限制 
            1,                       // initial_thread_count: JS 线程自身 
            nullptr,                 // thread_finalize_data 
            bridgeThreadFinalizeCb,  // 线程退出时 release 
            nullptr,                 // context 
            bridgeCallJsCb,
            &m_tsfn);
    if (s != napi_ok) {
        SLOGE() << "createThreadSafeFn failed, status=" << (int)s;
        m_tsfn = nullptr;
        return;
    }
    SLOGI() << "createThreadSafeFn ok";
}

void OhosPlatformAPI::destroyThreadSafeFn() {
    if (!m_tsfn) return;
    // abort：丢弃队列中尚未执行的任务
    napi_release_threadsafe_function(m_tsfn, napi_tsfn_abort);
    m_tsfn = nullptr;
}

bool OhosPlatformAPI::invokeBridge(napi_ref methodRef, const std::vector<BridgeArg> &args,
                                   BridgeRetType retType, BridgeResult *out, int waitMs) {
    if (!methodRef) return false;

    // 已在 JS 线程：直接执行，无需投递
    if (isJsThread()) {
        auto task = std::make_shared<BridgeTask>();
        task->methodRef = methodRef;
        task->args = args;
        task->retType = retType;
        executeBridgeTask(task.get());
        if (out) *out = task->local;
        return task->local.ok;
    }

    // 工作线程：必须投递回 JS 线程
    napi_threadsafe_function tsfn = m_tsfn;
    if (!tsfn || !m_env) {
        SLOGE() << "invokeBridge: tsfn unavailable, drop call from worker thread";
        return false;
    }

    // 每个工作线程首次调用前 acquire（与 bridgeThreadFinalizeCb 中的 release 配对）
    static thread_local bool t_acquired = false;
    if (!t_acquired) {
        napi_acquire_threadsafe_function(tsfn);
        t_acquired = true;
    }

    auto task = std::make_shared<BridgeTask>();
    task->methodRef = methodRef;
    task->args = args;
    task->retType = retType;
    auto holder = new std::shared_ptr<BridgeTask>(task);
    napi_status s = napi_call_threadsafe_function(tsfn, holder, napi_tsfn_nonblocking);
    if (s != napi_ok) {
        delete holder;
        SLOGE() << "invokeBridge: napi_call_threadsafe_function failed, status=" << (int)s;
        return false;
    }
    if (waitMs <= 0) return true; // 异步模式：不等待结果

    std::unique_lock<std::mutex> lk(task->mtx);
    bool completed = task->cv.wait_for(lk, std::chrono::milliseconds(waitMs),
                                      [&task] { return task->done; });
    lk.unlock();
    if (!completed) {
        SLOGE() << "invokeBridge: timeout waiting JS thread (" << waitMs << "ms)";
        return false;
    }
    if (out) *out = task->local;
    return true;
}

// ============================================================
// ======== 业务入口：initSouiApp / screenStartup ===========
// ============================================================ 

extern "C" void RegisterOhosPlatformAPI();
extern "C" void UnregisterOhosPlatformAPI();

BOOL OhosPlatformAPI::initSouiApp(napi_env env, napi_value rawfileMgr,
                                   std::string const& appFilesDir)
{
    if (!s_entry) {
        SLOGE() << "initSouiApp: no Soui4OhosEntry registered (业务层需调用 InitSoui4OhosEntry)";
        return FALSE;
    }
    RegisterOhosPlatformAPI();
    // 释放上一次会话遗留的资源管理器（防止重复启动泄漏）
    if (m_pAssetMgr) {
        if (OH_ResourceManager_ReleaseNativeResourceManager) {
            OH_ResourceManager_ReleaseNativeResourceManager(
                reinterpret_cast<NativeResourceManager*>(m_pAssetMgr));
        }
        m_pAssetMgr = nullptr;
    }
    // 由 ArkTS resourceManager 创建真实 NativeResourceManager*，存入 opaque 句柄
    if (env && rawfileMgr && OH_ResourceManager_InitNativeResourceManager) {
        m_pAssetMgr = reinterpret_cast<SouiOhosAssetManager>(
            OH_ResourceManager_InitNativeResourceManager(env, rawfileMgr));
    }
    if (!m_pAssetMgr) {
        SLOGE() << "initSouiApp: OH_ResourceManager_InitNativeResourceManager failed";
        return FALSE;
    }
    auto dir = appFilesDir;
    BOOL ret = s_entry->InitApp(m_pAssetMgr, dir.c_str());
    if(ret){
        SApplication::getSingletonPtr()->GetMsgLoop()->OnStart();
    }
    return ret;
}

void OhosPlatformAPI::uninitSouiApp(napi_env env) {
    (void)env;
    SASSERT(s_entry);
    SApplication *theApp = SApplication::getSingletonPtr();
    if(theApp){
        theApp->GetMsgLoop()->OnStop();
        s_entry->UninitApp();
    }
    UnregisterOhosPlatformAPI();
    if (m_pAssetMgr) {
        if (OH_ResourceManager_ReleaseNativeResourceManager) {
            OH_ResourceManager_ReleaseNativeResourceManager(
                reinterpret_cast<NativeResourceManager*>(m_pAssetMgr));
        }
        m_pAssetMgr = nullptr;
    }
}

extern "C" BOOL SConnection_RegisterVirtualHWND(UINT_PTR externalId, HWND hParent,
    DWORD dwStyle, DWORD dwExStyle, const RECT *prc, int ctrlId);
extern "C" BOOL SConnection_UnregisterVirtualHWND(UINT_PTR externalId);

HWND OhosPlatformAPI::screenStartup(napi_env env, long screenId, UINT_PTR screenHwnd, LPCSTR pszLayout) {
    (void)env;
    if (!s_entry) return 0;
    if (screenHwnd == 0) {
        // 防御：screenHwnd 缺失时回退到 screenId（历史行为），但主 Surface 将无法被 GetDC 命中
        SLOGW() << "screenStartup: screenHwnd==0, fallback to screenId=" << (long long)screenId;
        screenHwnd = (UINT_PTR)screenId;
    }
    RECT rc = {0,0,100,100};
    if (!RegisterVirtualHWND(screenHwnd, 0, WS_POPUP, 0, &rc, screenId)) return 0;
    setScreenHwnd(screenId, screenHwnd);
    pushActiveScreen(screenId);
    HWND hWnd = s_entry->ScreenStartup(screenId, pszLayout);
    return hWnd;
}

void OhosPlatformAPI::screenShutdown(napi_env env, long screenId) {
    (void)env;
    if (s_entry) s_entry->ScreenShutdown(screenId);
    HWND hwndScreen = getScreenHwnd(screenId);
    eraseScreen(screenId);
    popActiveScreen(screenId);
    if (hwndScreen) UnregisterVirtualHWND((UINT_PTR)hwndScreen);
}

void OhosPlatformAPI::setScreenHwnd(long screenId, UINT_PTR screenHwnd) {
    m_screenHwnds[screenId] = screenHwnd;
}
HWND OhosPlatformAPI::getScreenHwnd(long screenId) const {
    auto it = m_screenHwnds.find(screenId);
    return it == m_screenHwnds.end() ? 0 : reinterpret_cast<HWND>(it->second);
}
void OhosPlatformAPI::eraseScreen(long screenId) {
    m_screenHwnds.erase(screenId);
}

// ============================================================
// ======== Screen：激活栈 ===================================
//   对应 Android pushActiveScreen/popActiveScreen/getActiveScreenId
// ============================================================ 
long OhosPlatformAPI::pushActiveScreen(long screenId) {
    if (screenId == 0) return 0;
    std::lock_guard<std::mutex> lk(m_mutex);
    m_activeScreenStack.push_back(screenId);
    SLOGD() << "pushActiveScreen: id=" << (long long)screenId
            << " (stackSize=" << m_activeScreenStack.size() << ")";
    return screenId;
}

long OhosPlatformAPI::popActiveScreen(long screenId) {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (screenId != 0) {
        if (!m_activeScreenStack.empty() && m_activeScreenStack.back() == screenId) {
            m_activeScreenStack.pop_back();
        } else {
            auto it = std::find(m_activeScreenStack.begin(), m_activeScreenStack.end(), screenId);
            if (it != m_activeScreenStack.end()) m_activeScreenStack.erase(it);
        }
        SLOGD() << "popActiveScreen: id=" << (long long)screenId
                << " removed (stackSize=" << m_activeScreenStack.size() << ")";
    }
    return m_activeScreenStack.empty() ? 0 : m_activeScreenStack.back();
}

long OhosPlatformAPI::getActiveScreenId() const {
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_activeScreenStack.empty() ? 0 : m_activeScreenStack.back();
}

// ============================================================
// ======== SouiSurfaceProxy 管理 ==========================
// ============================================================ 

UINT_PTR OhosPlatformAPI::nativeViewCreate(napi_env env, napi_value nativeWnd) {
    try {
        auto *raw = new SouiSurfaceProxy(env, nativeWnd);
        UINT_PTR id = (UINT_PTR)raw->getNativeId();
        std::shared_ptr<SouiSurfaceProxy> sp(raw);
        std::lock_guard<std::mutex> lk(m_mutex);
        if (!m_nativeViews.emplace(id, sp).second) {
            SLOGE() << "nativeViewCreate duplicate id=" << (void*)id;
            return 0;
        }
        return id;
    } catch (const std::exception &e) {
        SLOGE() << "nativeViewCreate exception: " << e.what();
        return 0;
    }
}

std::shared_ptr<SouiSurfaceProxy> OhosPlatformAPI::nativeViewLookup(UINT_PTR nativeId) {
    std::lock_guard<std::mutex> lk(m_mutex);
    auto it = m_nativeViews.find(nativeId);
    return it == m_nativeViews.end() ? nullptr : it->second;
}

void OhosPlatformAPI::nativeViewDestroy(UINT_PTR nativeId) {
    std::shared_ptr<SouiSurfaceProxy> sp;
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        auto it = m_nativeViews.find(nativeId);
        if (it != m_nativeViews.end()) {
            sp = it->second;
            sp->clean(m_env);
            m_nativeViews.erase(it);
        }
    }
    // sp 出作用域析构
}

napi_value OhosPlatformAPI::hwndAsNativeWindow(UINT_PTR hwnd) {
    if (!hwnd || !m_env) return nullptr;
    napi_value argv[1];
    napi_create_int64(m_env, (int64_t)hwnd, &argv[0]);
    return callBridge(m_mHwndAsNativeWindow, 1, argv);
}

// ============================================================
// ======== 定时器 ===========================================
// ============================================================ 

DWORD_PTR OhosPlatformAPI::setTimer(HWND hWnd, UINT_PTR id, UINT uElapse, TIMERPROC proc) {
    if (!m_env) return 0;
    SLOGI()<<"setTimer, hWnd="<<hWnd<<" id="<<id<<" uElapse="<<uElapse;

    // 大部分场景由主线程（JS 线程）调用，直接 callBridge 走快路径；
    // 仅当处于工作线程时才走跨线程投递（napi_env 不能跨线程使用）。
    UINT_PTR actualTimerId = 0;
    if (isJsThread()) {
        napi_value argv[3];
        napi_create_int64(m_env, (int64_t)hWnd, &argv[0]);
        napi_create_int64(m_env, (int64_t)id, &argv[1]);
        napi_create_int32(m_env, (int32_t)uElapse, &argv[2]);
        napi_value r = callBridge(m_mSetTimer, 3, argv);
        int64_t v = 0;
        if (r) napi_get_value_int64(m_env, r, &v);
        actualTimerId = (UINT_PTR)v;
    } else {
        BridgeResult res;
        std::vector<BridgeArg> args = {
            BridgeArg::fromInt64((int64_t)hWnd),
            BridgeArg::fromInt64((int64_t)id),
            BridgeArg::fromInt32((int32_t)uElapse),
        };
        if (!invokeBridge(m_mSetTimer, args, BridgeRetType::Int64, &res, kBridgeWaitMs))
            return 0;
        actualTimerId = static_cast<UINT_PTR>(res.i64);
    }

    if (actualTimerId != 0 && proc != nullptr) {
        // 与 Android 一致：记录 TIMERPROC，onTimerExpired 时直接回调
        //（proc 型定时器 hWnd 可为 NULL，如 STimerGenerator 的全局定时器，
        //  此时 PostMessage 无窗口可投递，必须靠 proc 直接调用分发）
        // 注意：加锁必须在调用之后——不可持锁等待 JS 线程，
        // 否则 JS 线程上的 onTimerExpired 也要取同一把锁，会死锁。
        std::lock_guard<std::mutex> lock(m_mutex);
        m_timerEntries[(UINT_PTR)hWnd][actualTimerId] = proc;
    }
    return actualTimerId;
}
BOOL OhosPlatformAPI::killTimer(HWND hWnd, UINT_PTR id) {
    if (id == 0) return FALSE;
    if (!m_env) return FALSE;
    SLOGI()<<"killTimer, hWnd="<<hWnd<<" id="<<id;

    bool ok = false;
    if (isJsThread()) {
        napi_value argv[2];
        napi_create_int64(m_env, (int64_t)hWnd, &argv[0]);
        napi_create_int64(m_env, (int64_t)id, &argv[1]);
        napi_value r = callBridge(m_mKillTimer, 2, argv);
        if (r) napi_get_value_bool(m_env, r, &ok);
    } else {
        BridgeResult res;
        std::vector<BridgeArg> args = {
            BridgeArg::fromInt64((int64_t)hWnd),
            BridgeArg::fromInt64((int64_t)id),
        };
        invokeBridge(m_mKillTimer, args, BridgeRetType::Bool, &res, kBridgeWaitMs);
        ok = res.ok && res.b;
    }
    // 本地 bookkeeping 按 (hWnd, timerId) 二元擦除（对齐 Android killTimer）
    // 同样注意：加锁必须在调用之后
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto outerIt = m_timerEntries.find((UINT_PTR)hWnd);
        if (outerIt != m_timerEntries.end()) {
            outerIt->second.erase(id);
            if (outerIt->second.empty()) m_timerEntries.erase(outerIt);
        }
    }
    return ok ? TRUE : FALSE;
}
void OhosPlatformAPI::killWindowTimers(HWND hWnd) {
    if (!m_env) return;
    if (isJsThread()) {
        napi_value argv[1];
        napi_create_int64(m_env, (int64_t)hWnd, &argv[0]);
        callBridge(m_mKillWindowTimers, 1, argv);
    } else {
        std::vector<BridgeArg> args = { BridgeArg::fromInt64((int64_t)hWnd) };
        invokeBridge(m_mKillWindowTimers, args, BridgeRetType::None, nullptr, kBridgeWaitMs);
    }
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_timerEntries.erase((UINT_PTR)hWnd);
    }
}
void OhosPlatformAPI::onTimerExpired(HWND hWnd, UINT_PTR timerId) {
    if (timerId == 0) return;
    //SLOGI()<<"onTimerExpired,hWnd="<<hWnd<<" timerId="<<timerId;
    TIMERPROC proc = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto outerIt = m_timerEntries.find((UINT_PTR)hWnd);
        if (outerIt != m_timerEntries.end()) {
            auto innerIt = outerIt->second.find(timerId);
            if (innerIt != outerIt->second.end()) proc = innerIt->second;
        }
    }
    if (proc != nullptr) {
        // Win32 TIMERPROC(HWND, UINT, UINT_PTR id, DWORD elapsed) —— 与 Android 一致
        proc(hWnd, WM_TIMER, timerId, 0);
    } else {
        // 无 proc 的窗口定时器：投递 WM_TIMER 到 SOUI 消息循环（g_appThread）。
        // 用 PostMessage 异步投递（非 Android 的 SendMessage 同步）：ArkTS 定时回调
        // 执行在线程，若 SendMessage 阻塞等待 g_appThread，而 g_appThread 处理
        // WM_TIMER 时再经 callBridge 回调 ArkTS，会造成交叉等待死锁。
        if (!IsWindow(hWnd)) return;
        ::SendMessage(hWnd,WM_TIMER,timerId,0);
    }
}

// ============================================================
// ======== 输入：Focus / IME / 键盘 =======================
// ============================================================ 

HWND OhosPlatformAPI::setFocus(HWND hWnd) {
    if (!m_env) return 0;
    napi_value argv[1];
    napi_create_int64(m_env, (int64_t)hWnd, &argv[0]);
    napi_value r = callBridge(m_mSetFocus, 1, argv);
    int64_t old = 0;
    if (r) napi_get_value_int64(m_env, r, &old);
    return (HWND)(UINT_PTR)old;
}
HWND OhosPlatformAPI::getFocus() {
    if (!m_env) return 0;
    napi_value r = callBridge(m_mGetFocus, 0, nullptr);
    int64_t hwnd = 0;
    if (r) napi_get_value_int64(m_env, r, &hwnd);
    return (HWND)(UINT_PTR)hwnd;
}

BOOL OhosPlatformAPI::showSoftKeyboard(HWND hWnd, BOOL bShow) {
    if (!m_env) return FALSE;
    napi_value argv[2];
    napi_create_int64(m_env, (int64_t)hWnd, &argv[0]);
    napi_create_int32(m_env, (int32_t)bShow, &argv[1]);
    napi_value r = callBridge(m_mShowSoftKeyboard, 2, argv);
    bool ok = false;
    if (r && napi_get_value_bool(m_env, r, &ok) == napi_ok) {
        return ok ? TRUE : FALSE;
    }
    // ArkTS showSoftKeyboard 为 async（返回 Promise），同步调用取不到 bool；
    // 此时视为"请求已发出"，乐观返回 TRUE（Android 同步返回由 JNI 直取）。
    return TRUE;
}
void OhosPlatformAPI::notifyFocusGained(HWND hWnd) {
    ::SetFocus(hWnd); // SWinx SetFocus 自身调用 platform.setFocus 同步 ArkTS 端
}
void OhosPlatformAPI::setKeyboardHeight(int keyboardHeight) {
    HWND hFocus = ::GetFocus();
    HWND hRoot = ::GetAncestor(hFocus, GA_ROOT);
    if (hRoot) ::SendMessageA(hRoot, WM_KEYBOARD_HEIGHT, (WPARAM)keyboardHeight, 0);
}
void OhosPlatformAPI::sendImeString(HWND hWnd, const char *utf8, size_t len) {
    // 对齐 Android AndroidPlatformAPI::sendImeString：
    // UTF-8 → UTF-16 宽字符，逐字符 PostMessage WM_IME_CHAR。
    if (!hWnd || !utf8 || len == 0) return;
    std::string str(utf8, len);
    SStringW strW = S_CA2W(SStringA(str.c_str(), str.length()), CP_UTF8);
    for (int i = 0; i < strW.GetLength(); i++) {
        ::SendMessageA(hWnd, WM_IME_CHAR, (WPARAM)strW[i], 0);
    }
}

// ============================================================
// ======== Audio API ========================================
// ============================================================ 
BOOL OhosPlatformAPI::playSound(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound) {
    (void)hmod;
    if (!m_env || !m_mPlaySound) return FALSE;
    // SND_PURGE (0x40): 停止所有播放——传 null 空串触发 ArkTS 停止
    if (!pszSound || !*pszSound || (fdwSound & 0x00000040)) {
        napi_value argv[2];
        napi_create_string_utf8(m_env, "", NAPI_AUTO_LENGTH, &argv[0]);
        napi_create_int32(m_env, (int32_t)fdwSound, &argv[1]);
        napi_value r = callBridge(m_mPlaySound, 2, argv);
        bool ok = false;
        if (r && napi_get_value_bool(m_env, r, &ok) == napi_ok) return ok ? TRUE : FALSE;
        return TRUE;
    }
    napi_value argv[2];
    napi_create_string_utf8(m_env, pszSound, NAPI_AUTO_LENGTH, &argv[0]);
    napi_create_int32(m_env, (int32_t)fdwSound, &argv[1]);
    napi_value r = callBridge(m_mPlaySound, 2, argv);
    bool ok = false;
    if (r && napi_get_value_bool(m_env, r, &ok) == napi_ok) return ok ? TRUE : FALSE;
    return FALSE;
}

// ============================================================
// ======== Path API =========================================
// ============================================================ 
DWORD OhosPlatformAPI::getTempPathA(DWORD nBufferLength, LPSTR lpBuffer) {
    if (!m_env || !lpBuffer) return 0;
    napi_value r = callBridge(m_mGetTempPath, 0, nullptr);
    if (!r) return 0;
    char buf[512] = {0};
    size_t copied = 0;
    napi_get_value_string_utf8(m_env, r, buf, sizeof(buf), &copied);
    DWORD nLen = (DWORD)copied + 1;
    if (nBufferLength < nLen) return 0;
    memcpy(lpBuffer, buf, nLen);
    return nLen;
}

BOOL OhosPlatformAPI::getSpecialFolderPathA(int nFolder, BOOL fCreate,
                                              LPSTR pszPath, DWORD cchPath)
{
    if (!m_env || !pszPath || cchPath == 0) return FALSE;
    napi_value argv[2];
    napi_create_int32(m_env, (int32_t)nFolder, &argv[0]);
    napi_create_int32(m_env, (int32_t)fCreate, &argv[1]);
    napi_value r = callBridge(m_mGetSpecialFolderPath, 2, argv);
    if (!r) return FALSE;
    char buf[512] = {0};
    size_t copied = 0;
    napi_get_value_string_utf8(m_env, r, buf, sizeof(buf), &copied);
    if (copied == 0) return FALSE;
    if (copied >= cchPath) return FALSE;
    memcpy(pszPath, buf, copied + 1);
    return TRUE;
}

// ============================================================
// ======== Window 管理 & 消息调度 =========================
// ============================================================ 

HWND OhosPlatformAPI::createWindow(HWND hParent, UINT_PTR screenId,
                                   LPCSTR pszClassName, LPCSTR pszTitle,
                                   DWORD dwStyle, DWORD dwExStyle,
                                   int x, int y, int w, int h, HMENU hMenuOrCtrlId)
{
    if (!m_env) return 0;
    // 9 参数：对应 Android createWindow JNI (JLjava/lang/String;Ljava/lang/String;IIIIIII)J
    napi_value argv[11];
    napi_create_int64(m_env, (int64_t)hParent,        &argv[0]);
    napi_create_int64(m_env, (int64_t)screenId,       &argv[1]);
    napi_create_string_utf8(m_env, pszClassName ? pszClassName : "", NAPI_AUTO_LENGTH, &argv[2]);
    napi_create_string_utf8(m_env, pszTitle    ? pszTitle    : "", NAPI_AUTO_LENGTH, &argv[3]);
    napi_create_int32(m_env, (int32_t)dwStyle,  &argv[4]);
    napi_create_int32(m_env, (int32_t)dwExStyle,&argv[5]);
    napi_create_int32(m_env, (int32_t)x,        &argv[6]);
    napi_create_int32(m_env, (int32_t)y,        &argv[7]);
    napi_create_int32(m_env, (int32_t)w,        &argv[8]);
    napi_create_int32(m_env, (int32_t)h,        &argv[9]);
    napi_create_int64(m_env, (int64_t)hMenuOrCtrlId, &argv[10]);
    napi_value r = callBridge(m_mCreateWindow, 11, argv);
    int64_t hwnd = 0;
    if (r) napi_get_value_int64(m_env, r, &hwnd);
    return (HWND)(UINT_PTR)hwnd;
}

// ============================================================
// ======== Window 管理扩展（对应 AndroidPlatformAPI）=========
// ============================================================ 

BOOL OhosPlatformAPI::destroyWindow(HWND hWnd) {
    if (!m_env || !hWnd) return FALSE;
    napi_value argv[1];
    napi_create_int64(m_env, (int64_t)hWnd, &argv[0]);
    napi_value r = callBridge(m_mDestroyView, 1, argv);
    bool ok = false;
    if (r) napi_get_value_bool(m_env, r, &ok);
    return ok ? TRUE : FALSE;
}

HWND OhosPlatformAPI::getWindow(HWND hWnd, int code) {
    // Win32 GetWindow 简化：GW_PARENT/GW_CHILD 等走 ArkTS getWindow，
    // ArkTS 侧当前为模板实现（可通过 SouiSurface 父引用扩展）。
    if (!m_env || !hWnd) return 0;
    napi_value argv[2];
    napi_create_int64(m_env, (int64_t)hWnd, &argv[0]);
    napi_create_int32(m_env, code, &argv[1]);
    napi_value r = callBridge(m_mGetWindow, 2, argv);
    int64_t res = 0;
    if (r) napi_get_value_int64(m_env, r, &res);
    return (HWND)(UINT_PTR)res;
}

BOOL OhosPlatformAPI::moveWindow(HWND hWnd, int x, int y, int nWidth, int nHeight, BOOL bRepaint) {
    if (!m_env || !hWnd) return FALSE;
    napi_value argv[5];
    napi_create_int64(m_env, (int64_t)hWnd, &argv[0]);
    napi_create_int32(m_env, x, &argv[1]);
    napi_create_int32(m_env, y, &argv[2]);
    napi_create_int32(m_env, nWidth, &argv[3]);
    napi_create_int32(m_env, nHeight, &argv[4]);
    napi_value r = callBridge(m_mMoveWindow, 5, argv);
    bool ok = false;
    if (r) napi_get_value_bool(m_env, r, &ok);
    if (ok && bRepaint) invalidateRect(hWnd, 0, 0, 0, 0);
    return ok ? TRUE : FALSE;
}

BOOL OhosPlatformAPI::setWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y,
                                   int cx, int cy, UINT uFlags) {
    (void)hWndInsertAfter;
    const bool hasSize = (uFlags & 0x0001 // SWP_NOSIZE
) == 0;
    const bool hasPos  = (uFlags & 0x0002 // SWP_NOMOVE
) == 0;
    if (!hasSize && !hasPos) return TRUE;
    // ArkTS setWindowPos 一次完成 move+size（对齐 Android：SWP_NOSIZE 组合除外）
    if (hasSize && hasPos) {
        if (!m_env || !hWnd) return FALSE;
        napi_value argv[7];
        napi_create_int64(m_env, (int64_t)hWnd, &argv[0]);
        napi_create_int64(m_env, (int64_t)hWndInsertAfter, &argv[1]);
        napi_create_int32(m_env, X, &argv[2]);
        napi_create_int32(m_env, Y, &argv[3]);
        napi_create_int32(m_env, cx, &argv[4]);
        napi_create_int32(m_env, cy, &argv[5]);
        napi_create_int32(m_env, (int32_t)uFlags, &argv[6]);
        napi_value r = callBridge(m_mSetWindowPos, 7, argv);
        bool ok = false;
        if (r) napi_get_value_bool(m_env, r, &ok);
        return ok ? TRUE : FALSE;
    }
    if (hasSize) return setWindowSize(hWnd, cx, cy);
    if (hasPos)  return setWindowPosEx(hWnd, X, Y);
    return TRUE;
}

BOOL OhosPlatformAPI::setWindowSize(HWND hWnd, int cx, int cy) {
    if (!m_env || !hWnd) return FALSE;
    napi_value argv[3];
    napi_create_int64(m_env, (int64_t)hWnd, &argv[0]);
    napi_create_int32(m_env, cx, &argv[1]);
    napi_create_int32(m_env, cy, &argv[2]);
    napi_value r = callBridge(m_mSetWindowSize, 3, argv);
    bool ok = false;
    if (r) napi_get_value_bool(m_env, r, &ok);
    return ok ? TRUE : FALSE;
}

BOOL OhosPlatformAPI::setWindowPosEx(HWND hWnd, int x, int y) {
    if (!m_env || !hWnd) return FALSE;
    napi_value argv[3];
    napi_create_int64(m_env, (int64_t)hWnd, &argv[0]);
    napi_create_int32(m_env, x, &argv[1]);
    napi_create_int32(m_env, y, &argv[2]);
    napi_value r = callBridge(m_mSetWindowPosEx, 3, argv);
    bool ok = false;
    if (r) napi_get_value_bool(m_env, r, &ok);
    return ok ? TRUE : FALSE;
}

BOOL OhosPlatformAPI::invalidateRect(HWND hWnd, int left, int top, int right, int bottom) {
    if (!m_env || !hWnd) return FALSE;
    napi_value argv[5];
    napi_create_int64(m_env, (int64_t)hWnd, &argv[0]);
    napi_create_int32(m_env, left, &argv[1]);
    napi_create_int32(m_env, top, &argv[2]);
    napi_create_int32(m_env, right, &argv[3]);
    napi_create_int32(m_env, bottom, &argv[4]);
    napi_value r = callBridge(m_mInvalidateRect, 5, argv);
    bool ok = false;
    if (r) napi_get_value_bool(m_env, r, &ok);
    return ok ? TRUE : FALSE;
}

BOOL OhosPlatformAPI::isWindowVisible(HWND hWnd) {
    if (!m_env || !hWnd) return FALSE;
    napi_value argv[1];
    napi_create_int64(m_env, (int64_t)hWnd, &argv[0]);
    napi_value r = callBridge(m_mIsWindowVisible, 1, argv);
    bool ok = false;
    if (r) napi_get_value_bool(m_env, r, &ok);
    return ok ? TRUE : FALSE;
}

BOOL OhosPlatformAPI::showWindow(HWND hWnd, int nCmdShow) {
    if (!m_env || !hWnd) return FALSE;
    napi_value argv[2];
    napi_create_int64(m_env, (int64_t)hWnd, &argv[0]);
    napi_create_int32(m_env, nCmdShow, &argv[1]);
    napi_value r = callBridge(m_mShowView, 2, argv);
    bool ok = false;
    if (r) napi_get_value_bool(m_env, r, &ok);
    return ok ? TRUE : FALSE;
}

BOOL OhosPlatformAPI::enableWindow(HWND hWnd, BOOL bEnable) {
    if (!m_env || !hWnd) return FALSE;
    napi_value argv[2];
    napi_create_int64(m_env, (int64_t)hWnd, &argv[0]);
    napi_create_int32(m_env, (int32_t)bEnable, &argv[1]);
    napi_value r = callBridge(m_mEnableWindow, 2, argv);
    bool ok = false;
    if (r) napi_get_value_bool(m_env, r, &ok);
    return ok ? TRUE : FALSE;
}

BOOL OhosPlatformAPI::isWindowEnabled(HWND hWnd) {
    if (!m_env || !hWnd) return FALSE;
    napi_value argv[1];
    napi_create_int64(m_env, (int64_t)hWnd, &argv[0]);
    napi_value r = callBridge(m_mIsWindowEnabled, 1, argv);
    bool ok = false;
    if (r) napi_get_value_bool(m_env, r, &ok);
    return ok ? TRUE : FALSE;
}

// ============================================================
// ======== Screen：尺寸 / 工作区 ============================
// ============================================================ 

int OhosPlatformAPI::getScreenWidth() {
    long screenId = getActiveScreenId();
    if (screenId == 0) {
        std::lock_guard<std::mutex> lk(m_mutex);
        if (!m_screenHwnds.empty()) screenId = m_screenHwnds.cbegin()->first;
    }
    if (screenId == 0) return 1280;
    if (!m_env) return 1280;
    napi_value argv[1];
    napi_create_int64(m_env, (int64_t)screenId, &argv[0]);
    napi_value r = callBridge(m_mGetScreenSize, 1, argv);
    int64_t packed = 0;
    if (r) napi_get_value_int64(m_env, r, &packed);
    int w = (int)((packed >> 32) & 0xffffffff);
    return w > 0 ? w : 1280;
}

int OhosPlatformAPI::getScreenHeight() {
    long screenId = getActiveScreenId();
    if (screenId == 0) {
        std::lock_guard<std::mutex> lk(m_mutex);
        if (!m_screenHwnds.empty()) screenId = m_screenHwnds.cbegin()->first;
    }
    if (screenId == 0) return 1280;
    if (!m_env) return 1280;
    napi_value argv[1];
    napi_create_int64(m_env, (int64_t)screenId, &argv[0]);
    napi_value r = callBridge(m_mGetScreenSize, 1, argv);
    int64_t packed = 0;
    if (r) napi_get_value_int64(m_env, r, &packed);
    int h = (int)(packed & 0xffffffff);
    return h > 0 ? h : 1280;
}

void OhosPlatformAPI::getWorkArea(RECT *prc) {
    if (!prc) return;
    prc->left = prc->top = 0;
    prc->right  = getScreenWidth();
    prc->bottom = getScreenHeight();
}

// ============================================================
// ======== 捕获 / DPI / 光标 / 输入设备 ======================
// ============================================================ 

HWND OhosPlatformAPI::setCapture(HWND hWnd) {
    if (!m_env) return 0;
    napi_value argv[1];
    napi_create_int64(m_env, (int64_t)hWnd, &argv[0]);
    napi_value r = callBridge(m_mSetCapture, 1, argv);
    int64_t old = 0;
    if (r) napi_get_value_int64(m_env, r, &old);
    return (HWND)(UINT_PTR)old;
}

BOOL OhosPlatformAPI::releaseCapture() {
    if (!m_env) return FALSE;
    napi_value r = callBridge(m_mReleaseCapture, 0, nullptr);
    bool ok = false;
    if (r) napi_get_value_bool(m_env, r, &ok);
    return ok ? TRUE : FALSE;
}

void OhosPlatformAPI::postMessage() {
    scheduleMessageProcessing();
}

int OhosPlatformAPI::getDpi() {
    if (!m_env) return m_density;
    napi_value r = callBridge(m_mGetDpi, 0, nullptr);
    int32_t dpi = m_density;
    if (r) napi_get_value_int32(m_env, r, &dpi);
    return dpi > 0 ? (int)dpi : m_density;
}

BOOL OhosPlatformAPI::getCursorPos(LPPOINT ppt) {
    // OHOS 以触摸输入为主，无系统级光标；保留 Win32 语义 stub
    if (ppt) { ppt->x = ppt->y = 0; }
    return FALSE;
}

UINT OhosPlatformAPI::getRawInputDeviceList(PRAWINPUTDEVICELIST pRawInputDeviceList,
                                            PUINT puiNumDevices, UINT cbSize) {
    (void)pRawInputDeviceList; (void)cbSize;
    // OHOS 暂不枚举物理输入设备（无 Java getInputDevices 等价接入点）；
    // 返回 0 设备 = 无物理键盘，swinx IsPhysicalKeyboardPresent 为 FALSE
    if (puiNumDevices) *puiNumDevices = 0;
    return 0;
}

UINT OhosPlatformAPI::getRawInputDeviceInfoA(HRAWINPUT hDevice, UINT uiCommand,
                                             LPVOID pData, PUINT pcbSize) {
    (void)hDevice; (void)uiCommand; (void)pData;
    if (pcbSize) *pcbSize = 0;
    SetLastError(ERROR_INVALID_PARAMETER);
    return (UINT)-1;
}

UINT OhosPlatformAPI::getRawInputDeviceInfoW(HRAWINPUT hDevice, UINT uiCommand,
                                             LPVOID pData, PUINT pcbSize) {
    return getRawInputDeviceInfoA(hDevice, uiCommand, pData, pcbSize);
}

// ============================================================
// ======== 剪贴板（委托 ArkTS pasteboard）===================
// ============================================================ 
BOOL OhosPlatformAPI::clipboardOpen(HWND hWndNewOwner) {
    m_clipOwner = hWndNewOwner;
    return TRUE;
}

BOOL OhosPlatformAPI::clipboardClose() {
    m_clipOwner = 0;
    return TRUE;
}

BOOL OhosPlatformAPI::clipboardEmpty() {
    if (!m_env) return FALSE;
    napi_value argv[1];
    napi_create_string_utf8(m_env, "", NAPI_AUTO_LENGTH, &argv[0]);
    callBridge(m_mSetClipboardSysText, 1, argv);
    return TRUE;
}

HANDLE OhosPlatformAPI::clipboardGetData(UINT uFormat) {
    if (!m_env) return NULL;
    napi_value r = callBridge(m_mGetClipboardSysText, 0, nullptr);
    if (!r) return NULL;
    char buf[8192] = {0};
    size_t copied = 0;
    napi_get_value_string_utf8(m_env, r, buf, sizeof(buf), &copied);
    if (copied == 0) return NULL;
    if (uFormat == CF_UNICODETEXT) {
        int wlen = MultiByteToWideChar(CP_UTF8, 0, buf, -1, nullptr, 0);
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, wlen * sizeof(wchar_t));
        if (hMem) {
            wchar_t *pw = (wchar_t*)GlobalLock(hMem);
            if (pw) { MultiByteToWideChar(CP_UTF8, 0, buf, -1, pw, wlen); GlobalUnlock(hMem); }
        }
        return hMem;
    }
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, copied + 1);
    if (hMem) {
        char *p = (char*)GlobalLock(hMem);
        if (p) { memcpy(p, buf, copied + 1); GlobalUnlock(hMem); }
    }
    return hMem;
}

HANDLE OhosPlatformAPI::clipboardSetData(UINT uFormat, HANDLE hMem) {
    if (!hMem || !m_env) return NULL;
    std::string text;
    if (uFormat == CF_UNICODETEXT) {
        wchar_t *pw = (wchar_t*)GlobalLock(hMem);
        if (pw) {
            int len = WideCharToMultiByte(CP_UTF8, 0, pw, -1, nullptr, 0, nullptr, nullptr);
            if (len > 0) {
                text.resize((size_t)len);
                WideCharToMultiByte(CP_UTF8, 0, pw, -1, &text[0], len, nullptr, nullptr);
            }
            GlobalUnlock(hMem);
        }
    } else {
        char *p = (char*)GlobalLock(hMem);
        if (p) { text = p; GlobalUnlock(hMem); }
    }
    if (text.empty()) return NULL;
    napi_value argv[1];
    napi_create_string_utf8(m_env, text.c_str(), NAPI_AUTO_LENGTH, &argv[0]);
    callBridge(m_mSetClipboardSysText, 1, argv);
    return hMem;
}

BOOL OhosPlatformAPI::clipboardIsFormatAvailable(UINT format) {
    return clipboardHasFormat(format);
}

UINT OhosPlatformAPI::clipboardRegisterFormat(LPCSTR pszName) {
    static std::map<std::string, UINT> s_formatMap;
    static UINT s_nextFormat = 0xC000;
    if (!pszName || !*pszName) return 0;
    auto it = s_formatMap.find(pszName);
    if (it != s_formatMap.end()) return it->second;
    const UINT fmt = s_nextFormat++;
    s_formatMap[pszName] = fmt;
    return fmt;
}

HWND OhosPlatformAPI::clipboardGetOwner() {
    return m_clipOwner;
}

BOOL OhosPlatformAPI::clipboardHasFormat(UINT format) {
    (void)format;
    if (!m_env) return FALSE;
    napi_value r = callBridge(m_mGetClipboardSysText, 0, nullptr);
    if (!r) return FALSE;
    char buf[2] = {0};
    size_t copied = 0;
    napi_get_value_string_utf8(m_env, r, buf, sizeof(buf), &copied);
    return copied > 0 ? TRUE : FALSE;
}

void OhosPlatformAPI::scheduleMessageProcessing() {
    callBridge(m_mScheduleMessageProcessing, 0, nullptr);
}

void OhosPlatformAPI::executePendingTask() {
    MSG msg;
    while(GetMessage(&msg,0,0,0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    SApplication *pApp = SApplication::getSingletonPtr();
    if (pApp) {
        IMessageLoop *pMsgLoop = pApp->GetMsgLoop();
        if (pMsgLoop) {
            pMsgLoop->ExecutePendingTask();
        }
    }
}

BOOL OhosPlatformAPI::processIdle(int idleCount) {
    SApplication *pApp = SApplication::getSingletonPtr();
    if (pApp) {
        IMessageLoop *pMsgLoop = pApp->GetMsgLoop();
        if (pMsgLoop) {
            return pMsgLoop->OnIdle(idleCount);
        }
    }
    return FALSE;
}

// ============================================================
// ======== String slots（与 Android 一致的 ID 字符串槽）====
// ============================================================ 
int OhosPlatformAPI::stringSlotAlloc(std::string const& s) {
    std::lock_guard<std::mutex> lk(m_mutex);
    int id;
    if (!m_slotFree.empty()) {
        id = m_slotFree.back(); m_slotFree.pop_back();
    } else {
        m_slots.emplace_back();
        id = (int)m_slots.size() - 1;
        if (id > 65535) { // Slot ID 限制在 0-65535，循环复用
            m_slotFree.insert(m_slotFree.begin(), id);
            m_slots[id].data.clear();
            // 找下一个可复用
            for (size_t i = 0; i < m_slots.size(); ++i) {
                if (m_slots[i].data.empty() && i != m_slots.size() - 1) {
                    id = (int)i;
                    break;
                }
            }
        }
    }
    m_slots[id].data = s;
    return id;
}
void OhosPlatformAPI::stringSlotFree(int slotId) {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (slotId >= 0 && slotId < (int)m_slots.size()) {
        m_slots[slotId].data.clear();
        m_slotFree.push_back(slotId);
    }
}
std::string OhosPlatformAPI::ReadString(int slotId) {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (slotId < 0 || slotId >= (int)m_slots.size()) return {};
    return m_slots[slotId].data;
}
void OhosPlatformAPI::WriteString(int slotId, std::string const& s) {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (slotId >= 0 && slotId < (int)m_slots.size()) {
        m_slots[slotId].data = s;
    }
}
