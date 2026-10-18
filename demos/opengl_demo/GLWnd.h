/**
 * @file       GLWnd.h
 * @brief      用 SNativeWnd 承载的 OpenGL 子窗口。
 *
 * @details    这个窗口由 <realwnd/> 的 IRealWndHandler 创建（见 GLRealWndHandler.h），
 *             父窗口是 SOUI 的宿主 HWND，自身是普通的 Win32 子窗口，
 *             窗口过程由 SNativeWnd 的消息映射接管，内部持有一个 OpenGL 渲染上下文。
 *
 *             创建走 SNativeWnd::CreateNative，也就是复用 SOUI 注册的“简单窗口类”
 *             （见 SNativeWndHelper），窗口对象与 HWND 的绑定由框架的 thunk 完成，
 *             消息/对象的绑定方式与框架内其它真窗口完全一致。
 *
 *             注意：该窗口类没有 CS_OWNDC，所以 SetPixelFormat 只做一次，
 *             并把 GetDC 取得的 DC 一直持有到窗口销毁（见 m_hDC 的说明）。
 */

#ifndef __SOUI_GL_WND_H__
#define __SOUI_GL_WND_H__

#include <windows.h>
#include <GL/gl.h> // macOS/Linux 上命中 swinx 的兼容头：转发平台 GL 并带出 WGL 声明

#include <core/SNativeWnd.h>

#include "GLProc.h"

class CGLWnd : public SNativeWnd
{
  public:
    /** @brief 渲染路径 */
    enum RenderMode
    {
        Mode_FixedFunction = 0, /**< 固定管线：glBegin/glEnd 立即模式 */
        Mode_Shader = 1,        /**< 可编程管线：GLSL + VBO */
    };

    CGLWnd();
    virtual ~CGLWnd();

    /**
     * @brief realwnd 的 wndclass 标识（逻辑名，不是真实的 Win32 窗口类）
     * @details XML 里 <realwnd wndclass="SOUI_GLWnd"/> 与 CGLRealWndHandler 用它配对，
     *          真实的窗口类由 SNativeWndHelper 统一注册。
     */
    static LPCTSTR GetWndClassName();

    /**
     * @brief 创建子窗口并建立 OpenGL 上下文
     * @param rc 客户区坐标（相对宿主窗口）
     */
    BOOL Create(LPCTSTR pszName, DWORD dwStyle, const RECT &rc, HWND hParent, HINSTANCE hInst);

    /** @brief 开始/停止动画（内部 16ms 定时器） */
    void SetAnimating(BOOL bAnim);
    BOOL IsAnimating() const
    {
        return m_bAnimating;
    }
    void ToggleAnimating();

    RenderMode GetRenderMode() const
    {
        return m_nMode;
    }

    /** @brief 在固定管线/着色器之间切换；着色器不可用时保持固定管线 */
    RenderMode ToggleRenderMode();

    /** @brief 销毁并重建 GL 上下文（顺带重建 VBO/Program），用于验证上下文生命周期 */
    BOOL RecreateContext();

    /** @brief 把当前帧读回内存并保存为 24 位 BMP（glReadPixels 验证） */
    BOOL SaveSnapshot(LPCTSTR pszPath);

    /** @brief 一行状态摘要，给 DUI 文本用 */
    SStringT GetSummary() const;

    /** @brief 多行 API 探针报告 */
    SStringT GetProbeReport() const;

    /**
     * @brief 跑一遍 WGL 入口的契约测试，返回多行报告（实现见 GLWglTest.cpp）。
     * @param [out] pnPass/pnFail/pnSkip 可选的结果统计，NULL 表示不关心
     *
     * @details 覆盖 swinx WGL 兼容层新增的入口：extensions string、swap control、
     *          create context attribs、make current read、share lists、swap multiple
     *          buffers、default proc address，以及明确降级的那一组（图层平面 / GDI 字形 /
     *          上下文拷贝）。断言写成"三平台都成立的契约"：Windows 上真实现的返回值交给
     *          驱动，swinx 上必须返回文档写明的失败值；整个过程不能改变线程级的当前上下文。
     */
    SStringT RunApiTests(int *pnPass = NULL, int *pnFail = NULL, int *pnSkip = NULL);

  protected:
    BOOL EnsurePixelFormat();
    BOOL CreateContext();
    void ReleaseGL();
    void SetupGLState();
    void Render(BOOL bSwap);
    void DrawFixedFunction();
    BOOL InitShaderPath();
    void DrawShaderPath();
    void QueryGLInfo();
    void UpdateFPS();

    void OnPaint(HDC hdc);
    BOOL OnEraseBkgnd(HDC hdc);
    void OnSize(UINT nType, CSize size);
    void OnTimer(UINT_PTR nIDEvent);
    void OnLButtonDown(UINT nFlags, CPoint pt);
    void OnLButtonUp(UINT nFlags, CPoint pt);
    void OnMouseMove(UINT nFlags, CPoint pt);
    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    void OnLButtonDblClk(UINT nFlags, CPoint pt);
    void OnDestroy();

    BEGIN_MSG_MAP_EX(CGLWnd)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_ERASEBKGND(OnEraseBkgnd)
        MSG_WM_SIZE(OnSize)
        MSG_WM_TIMER(OnTimer)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
        MSG_WM_DESTROY(OnDestroy)
        CHAIN_MSG_MAP(SNativeWnd)
    END_MSG_MAP()

  protected:
    HDC m_hDC;      /**< 窗口 DC：建窗时取一次并一直持有，销毁时才 ReleaseDC */
    HGLRC m_hRC;    /**< GL 渲染上下文 */
    BOOL m_bPixelFormatSet; /**< SetPixelFormat 只能成功一次 */
    int m_nPixelFormat;     /**< ChoosePixelFormat 返回的像素格式索引 */
    PIXELFORMATDESCRIPTOR m_pfd; /**< DescribePixelFormat 回来的实际格式 */

    SGLProc m_gl;    /**< 1.5/2.0 入口表 */
    GLuint m_nProgram; /**< 着色器程序，0 表示未建 */
    GLuint m_nVbo;     /**< 立方体顶点缓冲 */
    GLint m_nAttrPos;   /**< 顶点属性位置 */
    GLint m_nAttrColor;
    GLint m_nLocAngle;  /**< uniform 位置 */
    GLint m_nLocPitch;
    GLint m_nLocScale;
    GLint m_nLocAspect;

    RenderMode m_nMode;
    BOOL m_bAnimating;
    BOOL m_bDragging;

    int m_nWidth;
    int m_nHeight;

    float m_fAngle;  /**< 自转角度 */
    float m_fYaw;    /**< 鼠标拖动产生的偏航 */
    float m_fPitch;  /**< 鼠标拖动产生的俯仰 */
    float m_fZoom;   /**< 滚轮缩放 */
    CPoint m_ptDragLast;

    DWORD m_dwFpsTick;
    int m_nFrames;
    float m_fFPS;

    /* ---- 探针缓存（上下文变化时查询一次） ---- */
    SStringT m_strVendor;
    SStringT m_strRenderer;
    SStringT m_strVersion;
    SStringT m_strGLSL;
    SStringT m_strReport; /**< 多行报告 */
    int m_nDepthBits;
    int m_nMaxTexture;
    BOOL m_bDoubleBuffer;
    BOOL m_bShaderOK;   /**< 着色器路径是否可用 */
    SStringT m_strShaderLog;
};

#endif // __SOUI_GL_WND_H__
