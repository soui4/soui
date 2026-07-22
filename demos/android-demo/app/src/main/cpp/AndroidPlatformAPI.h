#ifndef ANDROID_PLATFORM_API_H
#define ANDROID_PLATFORM_API_H

#include <jni.h>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <atomic>
#include <memory>
#include <vector>
#include <string>
#include <windows.h>
#include <souistd.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

/** 定时器条目。真正调度由 Java Handler 完成，C++ 层仅记录回调与元数据。 */
struct AndroidTimerEntry {
    UINT_PTR  timerId;    ///< 定时器 ID
    TIMERPROC proc;       ///< 回调（nullptr 表示走 WM_TIMER）
    bool      repeating;  ///< 是否循环
    UINT      interval;   ///< 周期（ms）
};

/**
 * SOUI for Android 入口抽象类。
 * 业务层通过继承此接口，实现平台层与业务层的解耦。
 * AndroidPlatformAPI 通过注册的 entry 指针调用这些虚函数。
 */
struct Soui4AndroidEntry{
    /**
     * 初始化 SOUI 应用。
     * @param assetMgr Android 资源管理器，用于加载 assets 目录下的资源
     * @param pszAssetDir 应用资源目录路径（UTF-8）
     * @return 创建的 SApplication 对象，失败返回 nullptr
     */
    virtual SNS::SApplication * InitApp(AAssetManager* assetMgr, LPCSTR pszAssetDir) = 0;

    /**
     * 反初始化 SOUI 应用。
     * @param pApp 需要释放的 SApplication 对象
     */
    virtual void UninitApp(SNS::SApplication *pApp) = 0;

    /**
     * 启动指定 screenId 的 SOUI 窗口系统。
     * @param screenId 屏幕唯一标识，用于多 Activity 场景
     * @param pszLayout 布局文件名（如 "layout:dlg_main"）
     * @return 创建的主窗口 HWND，失败返回 0
     */
    virtual HWND Startup(long screenId,const char* pszLayout) = 0;

    /**
     * 关闭指定 screenId 的 SOUI 窗口系统。
     * @param screenId 需要关闭的屏幕标识
     */
    virtual void Shutdown(long screenId) = 0;
};

/**
 * 注册 SOUI Android 入口实现。
 * 业务层在全局静态变量构造函数中调用此函数，将自己的实现注册到系统。
 * @param entry Soui4AndroidEntry 实现类的指针
 */
void InitSoui4AndroidEntry(Soui4AndroidEntry * entry);

class SouiSurfaceProxy; // 前向声明

/**
 * Android 平台 API 类。实现 Win32 窗口系统的跨平台抽象，提供窗口创建、
 * 定时器、消息转发等功能。采用单例模式。
 */
class AndroidPlatformAPI {
public:
    static AndroidPlatformAPI &instance();

    /** 初始化。 */
    void init(JNIEnv *env, jobject bridge, jobject ctx);

    /** 反初始化。 */
    void deinit();

    /** 初始化 SOUI 应用。 */
    bool initSouiApp(JNIEnv* env,jobject assetManagerJ,  jstring appFilesDirJ);
    /** 反初始化 SOUI 应用。 */
    void uninitSouiApp(JNIEnv* env);

    jobject getJavaBridge() const { return m_javaBridge; }
    jobject getApplicationContext() const{return m_javaCtx;}

    /** 创建窗口（与 Win32 CreateWindowEx 语义一致）。@return 新窗口 HWND，失败返回 0。 */
    UINT_PTR createWindow(UINT_PTR hParent,
                          const char* pszClsName, const char* pszTitle,
                          DWORD dwStyle, DWORD dwExStyle,
                          int x, int y, int nWidth, int nHeight,
                          UINT_PTR hMenu, UINT_PTR hInstance, LPVOID lpParam);

    BOOL destroyWindow(UINT_PTR hWnd);

    HWND getWindow(HWND hWnd,int code);

    BOOL moveWindow(HWND hWnd, int x, int y, int nWidth, int nHeight, BOOL bRepaint);

    BOOL setWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);

    BOOL setWindowSize(HWND hWnd, int cx, int cy);

    BOOL setWindowPosEx(HWND hWnd, int x, int y);

    BOOL isWindowVisible(HWND hWnd);

    BOOL showWindow(HWND hWnd, int nCmdShow);

    BOOL enableWindow(HWND hWnd, BOOL bEnable);

    BOOL isWindowEnabled(HWND hWnd);

    UINT_PTR setTimer(HWND hWnd, UINT_PTR id, UINT uElapse, TIMERPROC proc);

    BOOL killTimer(HWND hWnd, UINT_PTR id);

    BOOL killWindowTimers(HWND hWnd);

    void onTimerExpired(HWND hWnd, UINT_PTR timerId);

    /** 绑定 screenId 到原生屏幕 HWND。 */
    void setScreenHwnd(jlong screenId, UINT_PTR hwnd);
    void eraseScreen(jlong screenId);
    HWND getScreenHwnd(jlong screenId) const;
    int getScreenWidth(jlong screenId) const;
    int getScreenHeight(jlong screenId) const;

    int getScreenWidth() const;
    int getScreenHeight() const;

    HWND setCapture(UINT_PTR hwnd);
    BOOL releaseCapture();

    /** 设置输入焦点。@return 原焦点 HWND，无返回 0。 */
    HWND setFocus(UINT_PTR hwnd);
    /** 获取当前输入焦点。@return 焦点 HWND，无返回 0。 */
    HWND getFocus() const;

    /**
     * 消息转发接口。将 swinx 层消息派发到 Java 层原生 View。
     * 字符串参数通过字符串槽交换，纯数值消息按 Win32 语义传递。
     * @return LRESULT
     */
    LRESULT sendMessage(UINT_PTR hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    /** 分配字符串槽，返回正整数 id（范围 [1..65535]，0 保留为无效）。 */
    int stringSlotAlloc(const char * pszStr);
    /** 写入字符串到指定槽。 */
    void WriteString(int id, std::string s);
    /** 读取指定槽字符串。槽不存在或 id 无效返回空串。 */
    std::string ReadString(int id);
    /** 释放槽。推荐使用 AutoStringSlot RAII 自动释放。 */
    void stringSlotFree(int id);

    /** SOUI 启动入口（激活栈压栈 → SouiStartup）。 */
    HWND souiStartup(JNIEnv* env, jlong screenId, const char* layout);

    /** SOUI 销毁入口（SouiShutdown → 激活栈弹栈）。 */
    void souiShutdown(JNIEnv* env, jlong screenId);

    /** 压入激活 screen，SHostWnd::Create(NULL) 据此路由到正确 screen。 */
    jlong pushActiveScreen(jlong screenId);

    /** 从激活栈移除 screenId。@return 移除后栈顶，栈空返回 0。 */
    jlong popActiveScreen(jlong screenId);

    /** 获取当前激活栈栈顶。@return 栈顶 id，栈空返回 0。 */
    jlong getActiveScreenId() const;

    /** 请求局部或全部重绘（全为 0 表示全窗口重绘）。 */
    bool invalidateRect(HWND hWnd, int left, int top, int right, int bottom);

    /** 获取当前线程的 JNIEnv。 */
    JNIEnv *getJNIEnv();

    /** 请求平台层执行一次消息处理任务。 */
    void postMessage();

    /** 获取屏幕 DPI。 */
    int getDpi();

    /** 处理空闲时间，调用 SOUI 的 OnIdle 处理器。 */
    bool processIdle(int idleCount);

    // Clipboard APIs
    BOOL clipboardOpen(HWND hWndNewOwner);
    BOOL clipboardClose();
    BOOL clipboardEmpty();
    HANDLE clipboardGetData(UINT uFormat);
    HANDLE clipboardSetData(UINT uFormat, HANDLE hMem);
    BOOL clipboardIsFormatAvailable(UINT format);
    UINT clipboardRegisterFormat(LPCSTR pszName);
    HWND clipboardGetOwner();
    BOOL clipboardHasFormat(UINT format);

    /** 向指定 HWND 发送 WM_IME_CHAR 消息。 */
    void sendImeString(UINT_PTR hwnd, int slotid);

    /** 将 HWND 转换为 Android View。@return View jobject LocalRef。 */
    jobject hwndAsView(UINT_PTR hWnd);

    /** 注册 nativeId 到 SouiSurfaceProxy 的映射。 */
    bool nativeViewInsert(jlong nativeId, std::shared_ptr<SouiSurfaceProxy> nativeView);

    /** 查找 nativeId 对应的 SouiSurfaceProxy。 */
    std::shared_ptr<SouiSurfaceProxy> nativeViewLookup(jlong nativeId);

    /** 移除 nativeId 映射。 */
    bool nativeViewErase(jlong nativeId);

    /** 检查 HWND 是否有效。 */
    bool isValidHwnd(UINT_PTR hWnd) const;

    UINT getRawInputDeviceList(PRAWINPUTDEVICELIST pRawInputDeviceList, PUINT puiNumDevices, UINT cbSize);
    UINT getRawInputDeviceInfoA(HRAWINPUT hDevice, UINT uiCommand, LPVOID pData, PUINT pcbSize);
    UINT getRawInputDeviceInfoW(HRAWINPUT hDevice, UINT uiCommand, LPVOID pData, PUINT pcbSize);

    BOOL showSoftKeyboard(HWND hWnd,BOOL bShow);

    bool getCursorPos(LPPOINT ppt) const{
        if(!ppt)
            return false;
        *ppt = m_lastCursorPos;
        return true;
    }

    void setCursorPos(POINT pt){
        m_lastCursorPos = pt;
    }

    void executePendingTask();
private:
    AndroidPlatformAPI();

    ~AndroidPlatformAPI();

    AndroidPlatformAPI(const AndroidPlatformAPI &) = delete;

    AndroidPlatformAPI &operator=(const AndroidPlatformAPI &) = delete;

    jobject HwndObj(UINT_PTR hWnd);
    jobject HwndLocalObj(UINT_PTR hWnd);

    void unregisterWindowTimersInternal(UINT_PTR hWnd);

    POINT m_lastCursorPos;
    int   m_density = 96;
    SNS::SAutoRefPtr<SNS::SApplication>  m_theApp;
    mutable std::mutex m_mutex;
    // -------- String slot storage (并发读写互斥；与窗口定时器/screen map 分锁避免互扰) --------
    mutable std::mutex m_slotMtx;
    std::unordered_map<int, std::string> m_slotStrings;
    std::atomic<int> m_slotNextId{1};  // 0 保留为 invalid
    // 定时器条目：hWnd → (timerId → entry)；与 Java mBridgeTimerMap 镜像，但 C++ 层仅
    // 记录 TIMERPROC 回调与元数据，真正调度由 Java Handler 完成。
    std::unordered_map<UINT_PTR, std::unordered_map<UINT_PTR, AndroidTimerEntry>> m_timerEntries;

    // SouiSurfaceProxy 实例全局映射（并发 lookup 用 shared_ptr 保证安全）
    mutable std::mutex m_nativeViewMutex;
    std::unordered_map<jlong, std::shared_ptr<SouiSurfaceProxy>> m_nativeViewMap;
    JavaVM *m_javaVM{nullptr};
    jobject m_javaBridge{nullptr};
    jobject m_javaCtx{nullptr};

    std::unordered_map<jlong, HWND> m_screenContexts;
    std::vector<jlong> m_activeScreenStack;

    jclass    m_clsNativeWnd{nullptr};          // com/soui/INativeWindow (GlobalRef)
    jmethodID m_createWindowMethod{nullptr};
    jmethodID m_setTimerMethod{nullptr};
    jmethodID m_killTimerMethod{nullptr};
    jmethodID m_killWindowTimersMethod{nullptr};
    jmethodID m_setCaptureMethod{nullptr};
    jmethodID m_releaseCaptureMethod{nullptr};
    jmethodID m_setFocusMethod{nullptr};
    jmethodID m_getFocusMethod{nullptr};
    jmethodID m_postMessageMethod{nullptr};
    jmethodID m_getInputDevicesMethod{nullptr};
    jmethodID m_showSoftKeyboard{nullptr};
    jmethodID m_getSystemServiceMethod{nullptr};
    // Clipboard methods
    jmethodID m_clipboardOpenMethod{nullptr};
    jmethodID m_clipboardCloseMethod{nullptr};
    jmethodID m_clipboardEmptyMethod{nullptr};
    jmethodID m_clipboardGetDataMethod{nullptr};
    jmethodID m_clipboardSetDataMethod{nullptr};
    jmethodID m_clipboardIsFormatAvailableMethod{nullptr};
    jmethodID m_clipboardGetOwnerMethod{nullptr};
    jmethodID m_clipboardHasFormatMethod{nullptr};
    // INativeWindow.nativeSendMessage → NativeEditView 分派（2026-07 slot 统一，仅保留这 1 个 methodId）
    jmethodID m_nativeSendMessageMethod{nullptr};
    jmethodID m_nwgGetSurfaceViewMethod{nullptr};
    jmethodID m_nwDestroy{nullptr};
    jmethodID m_nwInvalidate{nullptr};
    jmethodID m_nwShow{nullptr};
    jmethodID m_nwMove{nullptr};
    jmethodID m_nwSetSize{nullptr};
    jmethodID m_nwSetPosition{nullptr};
    jmethodID m_nwIsVisible{nullptr};
    jmethodID m_nwEnable{nullptr};
    jmethodID m_nwIsEnabled{nullptr};
    jmethodID m_nwGetWindow{nullptr};
    jmethodID m_nwAsView{nullptr};
};

// =========================================================================
//  AutoStringSlot —— StringSlot RAII 包装（构造 stringSlotAlloc，析构 stringSlotFree）
//  用法：
//    AutoStringSlot slot("hello");                  // 拿全局 instance
//    AutoStringSlot slot(api, "hello");             // 显式指定引用
//    api.sendMessage(hwnd, msg, wp, (LPARAM)slot); // 利用 operator int 自动转 id
//    auto s = slot.read();                          // 等效 api.ReadString(slot.id())
//    // 作用域结束（或异常抛出）→ 自动调用 api.stringSlotFree(id)
// =========================================================================
class AutoStringSlot {
public:
    /** 显式指定 API 对象 + 初始值。 */
    explicit AutoStringSlot(const char * pszStr){
        m_id = AndroidPlatformAPI::instance().stringSlotAlloc(pszStr);
    }

    /** 析构自动释放槽（id 有效时）；noexcept，吞掉任何异常避免栈展开崩溃。 */
    ~AutoStringSlot() noexcept {
        if (m_id > 0) {
            AndroidPlatformAPI::instance().stringSlotFree(m_id);
            m_id = 0;
        }
    }

    /** 返回槽 id。 */
    int id() const noexcept { return m_id; }

    /** 允许隐式把 AutoStringSlot 当 int 使用（方便写 (LPARAM)slot 等编码）。 */
    explicit operator int() const noexcept { return m_id; }

    /** 便捷读：等价于 AndroidPlatformAPI::ReadString(id())。 */
    std::string read() const { return AndroidPlatformAPI::instance().ReadString(m_id); }

    /** 便捷写：等价于 AndroidPlatformAPI::WriteString(id(), std::move(s))。 */
    void write(std::string s) { AndroidPlatformAPI::instance().WriteString(m_id, std::move(s)); }
private:
    AutoStringSlot(const AutoStringSlot &);
    AutoStringSlot &operator=(const AutoStringSlot &);
    int                 m_id;
};

#endif // ANDROID_PLATFORM_API_H
