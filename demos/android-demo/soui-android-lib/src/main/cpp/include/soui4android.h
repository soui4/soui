//
// Created by setou on 2026/7/23.
//

#ifndef SOUI_ANDROID_DEMO_SOUI4ANDROID_H
#define SOUI_ANDROID_DEMO_SOUI4ANDROID_H

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
    virtual BOOL InitApp(AAssetManager* assetMgr, LPCSTR pszAssetDir) = 0;

    /**
     * 反初始化 SOUI 应用。
     * @param pApp 需要释放的 SApplication 对象
     */
    virtual void UninitApp() = 0;

    /**
     * 启动指定 screenId 的 SOUI 窗口系统。
     * @param screenId 屏幕唯一标识，用于多 Activity 场景
     * @param pszLayout 布局文件名（如 "layout:dlg_main"）
     * @return 创建的主窗口 HWND，失败返回 0
     */
    virtual HWND ScreenStartup(long screenId, LPCSTR pszLayout) = 0;

    /**
     * 关闭指定 screenId 的 SOUI 窗口系统。
     * @param screenId 需要关闭的屏幕标识
     */
    virtual void ScreenShutdown(long screenId) = 0;
};

/**
 * 注册 SOUI Android 入口实现。
 * 业务层在全局静态变量构造函数中调用此函数，将自己的实现注册到系统。
 * @param entry Soui4AndroidEntry 实现类的指针
 */
void InitSoui4AndroidEntry(Soui4AndroidEntry * entry);

#endif //SOUI_ANDROID_DEMO_SOUI4ANDROID_H
