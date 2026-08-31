//
// SOUI for OpenHarmony / HarmonyOS NEXT 入口抽象接口。
// 业务层通过继承此接口实现平台层与业务层解耦。
// OhosPlatformAPI 通过注册的 entry 指针调用这些虚函数。
// 架构完全对应 soui4android.h 中的 Soui4AndroidEntry，便于业务层
// 在 Android 与 OHOS 之间共享同一套启动逻辑代码。
//
#ifndef SOUI4OHOS_SOUI4OHOS_H
#define SOUI4OHOS_SOUI4OHOS_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

// OHOS 资源管理器（opaque 句柄，由 OhosPlatformAPI 填充
// 实际指向 RawFileManager / ResourceManager 的 C++ 封装）
typedef struct SouiOhosAssetManager_ * SouiOhosAssetManager;

struct Soui4OhosEntry {
    // 初始化 SOUI 应用（对应 Android Soui4AndroidEntry::InitApp）
    // assetMgr: OHOS 资源管理器，用于加载 rawfile/rawfile_dir 下的资源
    // pszAssetDir: 应用资源目录名（UTF-8），例如 "uires"
    // return: 创建的 SApplication 单例是否成功（TRUE/FALSE）
    virtual BOOL InitApp(SouiOhosAssetManager assetMgr, LPCSTR pszAssetDir) = 0;

    // 反初始化 SOUI 应用
    virtual void UninitApp() = 0;

    // 启动指定 screenId 的 SOUI 窗口系统（对应 Android Soui4AndroidEntry::ScreenStartup）
    // screenId: 屏幕唯一标识，用于多 UIAbility 场景
    // pszLayout: 布局文件名（UTF-8），例如 "layout:dlg_main"
    // return: 创建的主窗口 HWND，失败返回 0
    virtual HWND ScreenStartup(long screenId, LPCSTR pszLayout) = 0;

    // 关闭指定 screenId 的 SOUI 窗口系统
    virtual void ScreenShutdown(long screenId) = 0;

    virtual ~Soui4OhosEntry() = default;
};

// 注册 SOUI OHOS 入口实现（业务层在静态全局构造中调用）
// 用法示例（业务 app 侧）：
//   class CnchessOhosApp : public Soui4OhosEntry { ... };
//   static CnchessOhosApp s_app;
//   struct Register_ { Register_(){ InitSoui4OhosEntry(&s_app); } };
//   static Register_ s_register_;
void InitSoui4OhosEntry(Soui4OhosEntry * entry);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SOUI4OHOS_SOUI4OHOS_H
