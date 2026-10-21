#pragma once

#include <interface/SRender-i.h>

#include <core/SkSurface.h>
#include <gpu/GrContext.h>
#include <gpu/gl/GrGLInterface.h>

SNSBEGIN

/**
 * @brief 平台相关的 OpenGL 上下文封装（用于 Skia GPU 后端）。
 *
 * 上下文不绑定任何可见窗口，只作为 Skia GrContext 的宿主：
 * 所有绘制都发生在 GPU 端 FBO（纹理）上，与具体窗口系统解耦，
 * 因此 Windows/macOS/Linux/Android/OHOS 统一走同一条路径。
 */
class SGLContext
{
public:
    virtual ~SGLContext() {}

    /// 按当前平台创建默认实现，失败返回 NULL
    static SGLContext *Create();

    /// 创建上下文并绑定到当前线程，成功后可通过 glInterface() 获取函数表
    virtual bool init() = 0;

    virtual const GrGLInterface *glInterface() const = 0;

    virtual void destroy() = 0;
};

/**
 * @brief 进程级共享的 Skia GPU 渲染环境。
 *
 * 懒创建一个 GL 上下文和一个 GrContext，供所有 GPU 加速的 RenderTarget
 * 共享。SOUI 的 UI 渲染固定在 UI 线程上完成，因此这里不做多线程保护。
 */
class SGPURenderEnv
{
public:
    /// 引用计数 +1（由 SRenderFactory_Skia 构造时调用）
    static void Acquire();

    /// 引用计数 -1，归零时释放共享环境（由 SRenderFactory_Skia 析构时调用）
    static void Release();

    /// 获取共享 GrContext，首次调用时初始化；任何一步失败返回 NULL
    static GrContext *GetGrContext();

    /// 共享环境是否已初始化
    static bool IsAlive();

    /// 释放共享环境（幂等），仅内部与归零时调用
    static void Shutdown();
};

SNSEND
