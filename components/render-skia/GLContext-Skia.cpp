#include "GLContext-Skia.h"
#include <stddef.h>

//-----------------------------------------------------------------------------
// 平台 GL 系统头必须在进入 SOUI 命名空间(SNSBEGIN)之前包含：
//   * Xlib 的 Window/Display/Visual 等类型名会与 namespace SOUI 内的同名符号
//     （如 sobject-i.h 里的枚举成员 Window）直接冲突，报
//     “typedef XID Window redeclared as different kind of entity”；
//   * <utils/SkWGL.h> 若被包含进 namespace SOUI，SkCreateWGLContext 的声明就变成
//     SOUI::SkCreateWGLContext，和 skia 里的全局定义对不上，链接期报未定义。
// 所以这里统一提到文件开头，下面各分支只保留实现。
//-----------------------------------------------------------------------------
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <utils/SkWGL.h>
#elif defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#elif defined(__OHOS__) || defined(__ANDROID__)
#include <EGL/egl.h>
#else
#include <X11/Xlib.h>
#include <GL/glx.h>
#endif

SNSBEGIN

#if defined(_WIN32)
//===================================================================================
// Windows: WGL
//===================================================================================
namespace
{
    class SWGLContext : public SGLContext
    {
    public:
        SWGLContext() : m_hWnd(NULL), m_hDC(NULL), m_hRC(NULL), m_gl(NULL) {}
        virtual ~SWGLContext() { destroy(); }

        virtual bool init() OVERRIDE
        {
            WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
            wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
            wc.lpfnWndProc = DefWindowProc;
            wc.hInstance = GetModuleHandle(NULL);
            wc.lpszClassName = TEXT("SOUI_Skia_GL_Helper");
            if (!RegisterClassEx(&wc))
                return false;

            // 仅作为 GL 上下文宿主的 1x1 隐藏窗口，不做任何显示
            m_hWnd = CreateWindowEx(WS_EX_NOACTIVATE, wc.lpszClassName, TEXT(""), WS_OVERLAPPED,
                                    0, 0, 1, 1, NULL, NULL, wc.hInstance, NULL);
            if (!m_hWnd)
                return false;

            m_hDC = GetDC(m_hWnd);
            if (!m_hDC)
                return false;

            m_hRC = SkCreateWGLContext(m_hDC, 0, kGLPreferCompatibilityProfile_SkWGLContextRequest);
            if (!m_hRC)
                return false;

            wglMakeCurrent(m_hDC, m_hRC);

            m_gl = GrGLCreateNativeInterface();
            return m_gl != NULL;
        }

        virtual const GrGLInterface *glInterface() const OVERRIDE { return m_gl; }

        virtual void destroy() OVERRIDE
        {
            if (m_gl)
            {
                m_gl->unref();
                m_gl = NULL;
            }
            if (m_hRC)
            {
                wglMakeCurrent(NULL, NULL);
                wglDeleteContext(m_hRC);
                m_hRC = NULL;
            }
            if (m_hDC && m_hWnd)
            {
                ReleaseDC(m_hWnd, m_hDC);
                m_hDC = NULL;
            }
            if (m_hWnd)
            {
                DestroyWindow(m_hWnd);
                m_hWnd = NULL;
            }
        }

    private:
        HWND m_hWnd;
        HDC m_hDC;
        HGLRC m_hRC;
        const GrGLInterface *m_gl;
    };
} // namespace

SGLContext *SGLContext::Create()
{
    return new SWGLContext();
}

#elif defined(__APPLE__)
//===================================================================================
// macOS/iOS: CGL（纯 C 接口，离屏 FBO 渲染不需要窗口）
//===================================================================================
namespace
{
    class SCGLContext : public SGLContext
    {
    public:
        SCGLContext() : m_ctx(NULL), m_gl(NULL) {}
        virtual ~SCGLContext() { destroy(); }

        virtual bool init() OVERRIDE
        {
            CGLPixelFormatAttribute attrs[] = {
                kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_Legacy,
                kCGLPFAColorSize, (CGLPixelFormatAttribute)24,
                kCGLPFAAlphaSize, (CGLPixelFormatAttribute)8,
                kCGLPFAAccelerated,
                (CGLPixelFormatAttribute)0
            };
            CGLPixelFormatObj pixFmt = NULL;
            GLint nPix = 0;
            if (CGLChoosePixelFormat(attrs, &pixFmt, &nPix) != kCGLNoError || !pixFmt)
                return false;

            bool bOK = CGLCreateContext(pixFmt, NULL, &m_ctx) == kCGLNoError;
            CGLReleasePixelFormat(pixFmt);
            if (!bOK || !m_ctx)
                return false;

            if (CGLSetCurrentContext(m_ctx) != kCGLNoError)
                return false;

            m_gl = GrGLCreateNativeInterface();
            return m_gl != NULL;
        }

        virtual const GrGLInterface *glInterface() const OVERRIDE { return m_gl; }

        virtual void destroy() OVERRIDE
        {
            if (m_gl)
            {
                m_gl->unref();
                m_gl = NULL;
            }
            if (m_ctx)
            {
                CGLSetCurrentContext(NULL);
                CGLReleaseContext(m_ctx);
                m_ctx = NULL;
            }
        }

    private:
        CGLContextObj m_ctx;
        const GrGLInterface *m_gl;
    };
} // namespace

SGLContext *SGLContext::Create()
{
    return new SCGLContext();
}

#elif defined(__OHOS__) || defined(__ANDROID__)
//===================================================================================
// Android/OHOS: EGL pbuffer
//===================================================================================
namespace
{
    class SEGLContext : public SGLContext
    {
    public:
        SEGLContext() : m_dpy(EGL_NO_DISPLAY), m_ctx(EGL_NO_CONTEXT), m_surf(EGL_NO_SURFACE), m_gl(NULL) {}
        virtual ~SEGLContext() { destroy(); }

        virtual bool init() OVERRIDE
        {
            m_dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            if (m_dpy == EGL_NO_DISPLAY || !eglInitialize(m_dpy, NULL, NULL))
                return false;

            const EGLint cfgAttrs[] = {
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
                EGL_STENCIL_SIZE, 8,
                EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL_NONE
            };
            EGLConfig cfg = NULL;
            EGLint nCfg = 0;
            if (!eglChooseConfig(m_dpy, cfgAttrs, &cfg, 1, &nCfg) || nCfg < 1)
                return false;

            const EGLint ctxAttrs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
            m_ctx = eglCreateContext(m_dpy, cfg, EGL_NO_CONTEXT, ctxAttrs);
            if (m_ctx == EGL_NO_CONTEXT)
                return false;

            // 1x1 pbuffer 仅用于让上下文可被 makeCurrent，真正的绘制目标在 FBO 上
            const EGLint pbufAttrs[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
            m_surf = eglCreatePbufferSurface(m_dpy, cfg, pbufAttrs);
            if (m_surf == EGL_NO_SURFACE || !eglMakeCurrent(m_dpy, m_surf, m_surf, m_ctx))
                return false;

            m_gl = GrGLCreateNativeInterface();
            return m_gl != NULL;
        }

        virtual const GrGLInterface *glInterface() const OVERRIDE { return m_gl; }

        virtual void destroy() OVERRIDE
        {
            if (m_gl)
            {
                m_gl->unref();
                m_gl = NULL;
            }
            if (m_dpy != EGL_NO_DISPLAY)
            {
                eglMakeCurrent(m_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
                if (m_surf != EGL_NO_SURFACE)
                    eglDestroySurface(m_dpy, m_surf);
                if (m_ctx != EGL_NO_CONTEXT)
                    eglDestroyContext(m_dpy, m_ctx);
                eglTerminate(m_dpy);
            }
            m_surf = EGL_NO_SURFACE;
            m_ctx = EGL_NO_CONTEXT;
            m_dpy = EGL_NO_DISPLAY;
        }

    private:
        EGLDisplay m_dpy;
        EGLContext m_ctx;
        EGLSurface m_surf;
        const GrGLInterface *m_gl;
    };
} // namespace

SGLContext *SGLContext::Create()
{
    return new SEGLContext();
}

#else
//===================================================================================
// Linux: GLX pbuffer
//===================================================================================
namespace
{
    class SGLXContext : public SGLContext
    {
    public:
        SGLXContext() : m_dpy(NULL), m_ctx(NULL), m_pbuf(0), m_cfg(NULL), m_gl(NULL) {}
        virtual ~SGLXContext() { destroy(); }

        virtual bool init() OVERRIDE
        {
            m_dpy = XOpenDisplay(NULL);
            if (!m_dpy)
                return false;

            int screen = DefaultScreen(m_dpy);
            const int cfgAttrs[] = {
                GLX_RENDER_TYPE, GLX_RGBA_BIT,
                GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT,
                GLX_RED_SIZE, 8,
                GLX_GREEN_SIZE, 8,
                GLX_BLUE_SIZE, 8,
                GLX_ALPHA_SIZE, 8,
                GLX_STENCIL_SIZE, 8,
                GLX_DOUBLEBUFFER, 0,
                None
            };
            int nCfg = 0;
            GLXFBConfig *cfgs = glXChooseFBConfig(m_dpy, screen, cfgAttrs, &nCfg);
            if (!cfgs || nCfg < 1)
            {
                if (cfgs)
                    XFree(cfgs);
                return false;
            }
            m_cfg = cfgs[0];
            XFree(cfgs);

            m_ctx = glXCreateNewContext(m_dpy, m_cfg, GLX_RGBA_TYPE, NULL, True);
            if (!m_ctx)
                return false;

            const int pbufAttrs[] = { GLX_PBUFFER_WIDTH, 1, GLX_PBUFFER_HEIGHT, 1, None };
            m_pbuf = glXCreatePbuffer(m_dpy, m_cfg, pbufAttrs);
            if (!m_pbuf || !glXMakeContextCurrent(m_dpy, m_pbuf, m_pbuf, m_ctx))
                return false;

            m_gl = GrGLCreateNativeInterface();
            return m_gl != NULL;
        }

        virtual const GrGLInterface *glInterface() const OVERRIDE { return m_gl; }

        virtual void destroy() OVERRIDE
        {
            if (m_gl)
            {
                m_gl->unref();
                m_gl = NULL;
            }
            if (m_dpy)
            {
                glXMakeContextCurrent(m_dpy, None, None, NULL);
                if (m_pbuf)
                    glXDestroyPbuffer(m_dpy, m_pbuf);
                if (m_ctx)
                    glXDestroyContext(m_dpy, m_ctx);
                XCloseDisplay(m_dpy);
            }
            m_pbuf = 0;
            m_ctx = NULL;
            m_dpy = NULL;
        }

    private:
        Display *m_dpy;
        GLXContext m_ctx;
        GLXPbuffer m_pbuf;
        GLXFBConfig m_cfg;
        const GrGLInterface *m_gl;
    };
} // namespace

SGLContext *SGLContext::Create()
{
    return new SGLXContext();
}

#endif

//===================================================================================
// SGPURenderEnv
//===================================================================================
namespace
{
    SGLContext *s_glCtx = NULL;
    GrContext *s_grCtx = NULL;
    int s_nEnvRef = 0;
}

void SGPURenderEnv::Acquire()
{
    s_nEnvRef++;
}

void SGPURenderEnv::Release()
{
    if (--s_nEnvRef <= 0)
    {
        s_nEnvRef = 0;
        Shutdown();
    }
}

bool SGPURenderEnv::IsAlive()
{
    return s_grCtx != NULL;
}

GrContext *SGPURenderEnv::GetGrContext()
{
    if (s_grCtx)
        return s_grCtx;

    SGLContext *glCtx = SGLContext::Create();
    if (!glCtx)
        return NULL;
    if (!glCtx->init())
    {
        delete glCtx;
        return NULL;
    }

    GrContext *grCtx = GrContext::Create(kOpenGL_GrBackend, (GrBackendContext)(uintptr_t)glCtx->glInterface());
    if (!grCtx)
    {
        glCtx->destroy();
        delete glCtx;
        return NULL;
    }

    s_glCtx = glCtx;
    s_grCtx = grCtx;
    return s_grCtx;
}

void SGPURenderEnv::Shutdown()
{
    if (s_grCtx)
    {
        s_grCtx->unref();
        s_grCtx = NULL;
    }
    if (s_glCtx)
    {
        s_glCtx->destroy();
        delete s_glCtx;
        s_glCtx = NULL;
    }
}

SNSEND
