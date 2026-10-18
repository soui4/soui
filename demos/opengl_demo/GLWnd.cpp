#include "stdafx.h"
#include "GLWnd.h"

#define GL_ANIM_TIMER 1
#define GL_ANIM_ELAPSE 16

// realwnd 的 wndclass 标识（逻辑名，与 CGLRealWndHandler 配对用，不是真实的 Win32 窗口类）
static const TCHAR *kGLWndClass = _T("SOUI_GLWnd");

//////////////////////////////////////////////////////////////////////////
// 立方体几何：6 个面，每面 4 个顶点（从外侧看逆时针）
//////////////////////////////////////////////////////////////////////////
static const GLfloat kCubeFaces[6][4][3] = {
    // +X
    {{0.6f, -0.6f, 0.6f}, {0.6f, -0.6f, -0.6f}, {0.6f, 0.6f, -0.6f}, {0.6f, 0.6f, 0.6f}},
    // -X
    {{-0.6f, -0.6f, -0.6f}, {-0.6f, -0.6f, 0.6f}, {-0.6f, 0.6f, 0.6f}, {-0.6f, 0.6f, -0.6f}},
    // +Y
    {{-0.6f, 0.6f, 0.6f}, {0.6f, 0.6f, 0.6f}, {0.6f, 0.6f, -0.6f}, {-0.6f, 0.6f, -0.6f}},
    // -Y
    {{-0.6f, -0.6f, -0.6f}, {0.6f, -0.6f, -0.6f}, {0.6f, -0.6f, 0.6f}, {-0.6f, -0.6f, 0.6f}},
    // +Z
    {{-0.6f, -0.6f, 0.6f}, {0.6f, -0.6f, 0.6f}, {0.6f, 0.6f, 0.6f}, {-0.6f, 0.6f, 0.6f}},
    // -Z
    {{0.6f, -0.6f, -0.6f}, {-0.6f, -0.6f, -0.6f}, {-0.6f, 0.6f, -0.6f}, {0.6f, 0.6f, -0.6f}},
};

static const GLfloat kFaceColor[6][3] = {
    {0.96f, 0.36f, 0.32f}, // +X 红
    {0.24f, 0.62f, 0.94f}, // -X 蓝
    {0.98f, 0.78f, 0.24f}, // +Y 黄
    {0.36f, 0.82f, 0.44f}, // -Y 绿
    {0.66f, 0.45f, 0.92f}, // +Z 紫
    {0.24f, 0.84f, 0.82f}, // -Z 青
};

/** 顶点着色器：在 GPU 侧做两次旋转 + 手工透视除法，避免在 C++ 里拼矩阵。 */
static const char *kVS = "#version 120\n"
                         "attribute vec3 aPos;\n"
                         "attribute vec3 aColor;\n"
                         "uniform float uAngle;\n"
                         "uniform float uPitch;\n"
                         "uniform float uScale;\n"
                         "uniform float uAspect;\n"
                         "varying vec3 vColor;\n"
                         "void main()\n"
                         "{\n"
                         "    float cy = cos(uAngle), sy = sin(uAngle);\n"
                         "    vec3 p = vec3(cy * aPos.x - sy * aPos.z, aPos.y, sy * aPos.x + cy * aPos.z);\n"
                         "    float cp = cos(uPitch), sp = sin(uPitch);\n"
                         "    p = vec3(p.x, cp * p.y - sp * p.z, sp * p.y + cp * p.z);\n"
                         "    vColor = aColor;\n"
                         "    float w = 4.0 - p.z;\n" // 相机在 z=+4，朝 -z 看
                         "    gl_Position = vec4(p.x * uScale * 3.2 / (w * uAspect),\n"
                         "                       p.y * uScale * 3.2 / w,\n"
                         "                       -p.z * 0.4, 1.0);\n"
                         "}\n";

static const char *kFS = "#version 120\n"
                         "varying vec3 vColor;\n"
                         "void main()\n"
                         "{\n"
                         "    gl_FragColor = vec4(vColor, 1.0);\n"
                         "}\n";

/** 把 6 个面展开成 36 个顶点（位置 3 float + 颜色 3 float）。 */
static void BuildCubeVertices(GLfloat *pOut)
{
    static const int kTri[6] = {3, 0, 1, 3, 1, 2}; // 四边形拆成两个三角形
    int n = 0;
    for (int f = 0; f < 6; ++f)
    {
        for (int k = 0; k < 6; ++k)
        {
            const GLfloat *p = kCubeFaces[f][kTri[k]];
            pOut[n++] = p[0];
            pOut[n++] = p[1];
            pOut[n++] = p[2];
            pOut[n++] = kFaceColor[f][0];
            pOut[n++] = kFaceColor[f][1];
            pOut[n++] = kFaceColor[f][2];
        }
    }
}

/**
 * @brief 写 24 位 BMP。
 *
 * @param pBits  glReadPixels(GL_RGB) 得到的像素，行序自底向上（与 BMP 一致）
 * @details BMP 规定像素按 BGR 存放，而 GL 给的是 RGB，所以逐行交换 R/B。
 */
static BOOL WriteBMP24(LPCTSTR pszPath, int nWidth, int nHeight, const BYTE *pBits, int nRowSize)
{
    BITMAPFILEHEADER bfh = {};
    BITMAPINFOHEADER bih = {};
    bih.biSize = sizeof(bih);
    bih.biWidth = nWidth;
    bih.biHeight = nHeight; // 正数 = 自底向上，与 GL 行序一致
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = nRowSize * nHeight;

    bfh.bfType = 0x4D42; // 'BM'
    bfh.bfOffBits = sizeof(bfh) + sizeof(bih);
    bfh.bfSize = bfh.bfOffBits + bih.biSizeImage;

    HANDLE hFile = ::CreateFile(pszPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    BOOL bOK = TRUE;
    DWORD dwWritten = 0;
    bOK = ::WriteFile(hFile, &bfh, sizeof(bfh), &dwWritten, NULL);
    bOK = bOK && ::WriteFile(hFile, &bih, sizeof(bih), &dwWritten, NULL);

    BYTE *pRow = (BYTE *)malloc(nRowSize);
    if (!pRow)
    {
        ::CloseHandle(hFile);
        return FALSE;
    }
    for (int y = 0; y < nHeight && bOK; ++y)
    {
        const BYTE *pSrc = pBits + (size_t)y * nRowSize;
        memset(pRow, 0, nRowSize);
        for (int x = 0; x < nWidth; ++x)
        {
            pRow[x * 3 + 0] = pSrc[x * 3 + 2]; // B
            pRow[x * 3 + 1] = pSrc[x * 3 + 1]; // G
            pRow[x * 3 + 2] = pSrc[x * 3 + 0]; // R
        }
        bOK = ::WriteFile(hFile, pRow, nRowSize, &dwWritten, NULL);
    }
    free(pRow);
    ::CloseHandle(hFile);
    return bOK;
}

//////////////////////////////////////////////////////////////////////////
CGLWnd::CGLWnd()
    : m_hDC(NULL)
    , m_hRC(NULL)
    , m_bPixelFormatSet(FALSE)
    , m_nPixelFormat(0)
    , m_nProgram(0)
    , m_nVbo(0)
    , m_nAttrPos(-1)
    , m_nAttrColor(-1)
    , m_nLocAngle(-1)
    , m_nLocPitch(-1)
    , m_nLocScale(-1)
    , m_nLocAspect(-1)
    , m_nMode(Mode_FixedFunction)
    , m_bAnimating(FALSE)
    , m_bDragging(FALSE)
    , m_nWidth(0)
    , m_nHeight(0)
    , m_fAngle(0.0f)
    , m_fYaw(0.0f)
    , m_fPitch(0.0f)
    , m_fZoom(1.0f)
    , m_dwFpsTick(0)
    , m_nFrames(0)
    , m_fFPS(0.0f)
    , m_nDepthBits(0)
    , m_nMaxTexture(0)
    , m_bDoubleBuffer(FALSE)
    , m_bShaderOK(FALSE)
{
    ::ZeroMemory(&m_pfd, sizeof(m_pfd));
    m_ptDragLast.x = 0;
    m_ptDragLast.y = 0;
}

CGLWnd::~CGLWnd()
{
    ReleaseGL();
}

LPCTSTR CGLWnd::GetWndClassName()
{
    return kGLWndClass;
}

BOOL CGLWnd::Create(LPCTSTR pszName, DWORD dwStyle, const RECT &rc, HWND hParent, HINSTANCE hInst)
{
    if (::IsWindow(m_hWnd))
        return TRUE;

    // 走框架的 SNativeWnd::CreateNative：复用 SOUI 注册的“简单窗口类”，
    // 创建期消息由 thunk 直接派发到本对象，之后 WM_* 都进本类的消息映射。
    HWND hWnd = CreateNative(pszName, dwStyle, 0, rc.left, rc.top, rc.right - rc.left,
                             rc.bottom - rc.top, hParent);
    if (!hWnd)
        return FALSE;

    // 上下文建不起来也保留窗口：界面上的探针报告会把失败点列出来
    CreateContext();

    SetAnimating(TRUE);
    return TRUE;
}

BOOL CGLWnd::EnsurePixelFormat()
{
    // 窗口类不是 CS_OWNDC，所以这里把 DC 取出来一直持有到 ReleaseGL，
    // 保证 SetPixelFormat/wglMakeCurrent/SwapBuffers 始终作用在同一个 DC 上。
    if (!m_hDC)
        m_hDC = GetDC();
    if (!m_hDC)
        return FALSE;
    if (m_bPixelFormatSet)
        return TRUE;

    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int nFormat = ::ChoosePixelFormat(m_hDC, &pfd);
    if (nFormat == 0)
        return FALSE;
    if (!::SetPixelFormat(m_hDC, nFormat, &pfd))
        return FALSE;

    m_bPixelFormatSet = TRUE;
    m_nPixelFormat = nFormat;
    ::DescribePixelFormat(m_hDC, nFormat, sizeof(m_pfd), &m_pfd);
    return TRUE;
}

BOOL CGLWnd::CreateContext()
{
    if (!EnsurePixelFormat())
        return FALSE;

    m_hRC = ::wglCreateContext(m_hDC);
    if (!m_hRC)
        return FALSE;
    if (!::wglMakeCurrent(m_hDC, m_hRC))
    {
        ::wglDeleteContext(m_hRC);
        m_hRC = NULL;
        return FALSE;
    }

    m_gl.Load();
    QueryGLInfo();
    SetupGLState();
    // 着色器路径作为"探针"的一部分在此尝试一次；失败时保持固定管线
    if (!InitShaderPath() && m_nMode == Mode_Shader)
        m_nMode = Mode_FixedFunction;
    return TRUE;
}

void CGLWnd::ReleaseGL()
{
    if (m_hRC)
    {
        // 删 GL 对象必须有当前上下文
        if (::wglGetCurrentContext() != m_hRC)
            ::wglMakeCurrent(m_hDC, m_hRC);
        if (::wglGetCurrentContext() == m_hRC)
        {
            if (m_nVbo && m_gl.glDeleteBuffers)
                m_gl.glDeleteBuffers(1, &m_nVbo);
            if (m_nProgram && m_gl.glDeleteProgram)
                m_gl.glDeleteProgram(m_nProgram);
            ::wglMakeCurrent(NULL, NULL);
        }
        m_nVbo = 0;
        m_nProgram = 0;

        ::wglDeleteContext(m_hRC);
        m_hRC = NULL;
    }
    m_gl.Unload();
    if (m_hDC)
    {
        ReleaseDC(m_hDC);
        m_hDC = NULL;
    }
}

void CGLWnd::SetupGLState()
{
    glClearColor(0.09f, 0.10f, 0.13f, 1.0f);
    glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    m_fAngle = 0.0f;
    m_fYaw = 25.0f;   // 初始视角稍微偏一点，一上来就能看出是立方体
    m_fPitch = 18.0f; // 略微俯视，能同时看到顶面
    m_fZoom = 1.0f;
    m_dwFpsTick = 0;
    m_nFrames = 0;
}

void CGLWnd::QueryGLInfo()
{
    m_strReport.Empty();

    const char *pszVendor = (const char *)glGetString(GL_VENDOR);
    const char *pszRenderer = (const char *)glGetString(GL_RENDERER);
    const char *pszVersion = (const char *)glGetString(GL_VERSION);
    const char *pszGLSL = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
    m_strVendor = pszVendor ? S_CA2T(pszVendor) : _T("(null)");
    m_strRenderer = pszRenderer ? S_CA2T(pszRenderer) : _T("(null)");
    m_strVersion = pszVersion ? S_CA2T(pszVersion) : _T("(null)");
    m_strGLSL = pszGLSL ? S_CA2T(pszGLSL) : _T("(null)");

    glGetIntegerv(GL_DEPTH_BITS, (GLint *)&m_nDepthBits);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint *)&m_nMaxTexture);
    glGetIntegerv(GL_DOUBLEBUFFER, (GLint *)&m_bDoubleBuffer);

    m_strReport.AppendFormat(_T("[上下文]\n"));
    m_strReport.AppendFormat(_T("  像素格式: format=%d, color=%d, depth=%d, doubleBuffer=%d\n"),
                             m_nPixelFormat, (int)m_pfd.cColorBits, (int)m_pfd.cDepthBits,
                             (m_pfd.dwFlags & PFD_DOUBLEBUFFER) ? 1 : 0);
    m_strReport.AppendFormat(_T("  wglCreateContext / wglMakeCurrent: OK\n"));

    // 立即模式探针：画一个退化四边形，确认 GL 1.1 入口可用
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glBegin(GL_QUADS);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glEnd();
    GLenum errImm = glGetError();

    m_strReport.AppendFormat(_T("[驱动]\n"));
    m_strReport.AppendFormat(_T("  GL_VENDOR   : %s\n"), m_strVendor.c_str());
    m_strReport.AppendFormat(_T("  GL_RENDERER : %s\n"), m_strRenderer.c_str());
    m_strReport.AppendFormat(_T("  GL_VERSION  : %s\n"), m_strVersion.c_str());
    m_strReport.AppendFormat(_T("  GLSL        : %s\n"), m_strGLSL.c_str());

    m_strReport.AppendFormat(_T("[能力]\n"));
    m_strReport.AppendFormat(_T("  GL_MAX_TEXTURE_SIZE = %d\n"), m_nMaxTexture);
    m_strReport.AppendFormat(_T("  GL_DEPTH_BITS = %d, GL_DOUBLEBUFFER = %d\n"), m_nDepthBits,
                             m_bDoubleBuffer);
    m_strReport.AppendFormat(_T("  wglSwapIntervalEXT : %s\n"),
                             ::wglGetProcAddress("wglSwapIntervalEXT") ? _T("可用") : _T("不可用"));

    m_strReport.AppendFormat(_T("[API 路径]\n"));
    m_strReport.AppendFormat(_T("  GL 1.1 立即模式(glBegin/glEnd) : %s\n"),
                             errImm == GL_NO_ERROR ? _T("OK") : _T("FAIL"));
    m_strReport.AppendFormat(_T("  GL 1.5 VBO 入口              : %s\n"),
                             m_gl.HasVBO() ? _T("OK") : _T("缺失"));
    m_strReport.AppendFormat(_T("  GL 2.0 着色器入口            : %s\n"),
                             m_gl.HasShader() ? _T("OK") : _T("缺失"));

    m_strRenderer = m_strRenderer.IsEmpty() ? _T("(unknown)") : m_strRenderer;
}

BOOL CGLWnd::InitShaderPath()
{
    m_bShaderOK = FALSE;
    if (m_nProgram)
    {
        m_bShaderOK = TRUE;
        return TRUE;
    }
    if (!m_gl.HasShader() || !m_gl.HasVBO())
    {
        m_strReport.AppendFormat(_T("  GLSL 程序                    : 跳过（入口缺失）\n"));
        return FALSE;
    }

    GLint bCompiled = 0;
    GLuint nVS = m_gl.glCreateShader(GL_VERTEX_SHADER);
    m_gl.glShaderSource(nVS, 1, &kVS, NULL);
    m_gl.glCompileShader(nVS);
    m_gl.glGetShaderiv(nVS, GL_COMPILE_STATUS, &bCompiled);
    if (!bCompiled)
    {
        char szLog[512] = {0};
        m_gl.glGetShaderInfoLog(nVS, sizeof(szLog) - 1, NULL, szLog);
        m_strShaderLog = S_CA2T(szLog);
        m_gl.glDeleteShader(nVS);
        m_strReport.AppendFormat(_T("  GLSL 程序                    : 顶点着色器编译失败(%s)\n"),
                                 m_strShaderLog.c_str());
        return FALSE;
    }

    GLuint nFS = m_gl.glCreateShader(GL_FRAGMENT_SHADER);
    m_gl.glShaderSource(nFS, 1, &kFS, NULL);
    m_gl.glCompileShader(nFS);
    m_gl.glGetShaderiv(nFS, GL_COMPILE_STATUS, &bCompiled);
    if (!bCompiled)
    {
        char szLog[512] = {0};
        m_gl.glGetShaderInfoLog(nFS, sizeof(szLog) - 1, NULL, szLog);
        m_strShaderLog = S_CA2T(szLog);
        m_gl.glDeleteShader(nFS);
        m_gl.glDeleteShader(nVS);
        m_strReport.AppendFormat(_T("  GLSL 程序                    : 片元着色器编译失败(%s)\n"),
                                 m_strShaderLog.c_str());
        return FALSE;
    }

    GLuint nProg = m_gl.glCreateProgram();
    m_gl.glAttachShader(nProg, nVS);
    m_gl.glAttachShader(nProg, nFS);
    m_gl.glLinkProgram(nProg);
    GLint bLinked = 0;
    m_gl.glGetProgramiv(nProg, GL_LINK_STATUS, &bLinked);
    m_gl.glDeleteShader(nVS);
    m_gl.glDeleteShader(nFS);
    if (!bLinked)
    {
        char szLog[512] = {0};
        m_gl.glGetProgramInfoLog(nProg, sizeof(szLog) - 1, NULL, szLog);
        m_strShaderLog = S_CA2T(szLog);
        m_gl.glDeleteProgram(nProg);
        m_strReport.AppendFormat(_T("  GLSL 程序                    : 链接失败(%s)\n"),
                                 m_strShaderLog.c_str());
        return FALSE;
    }

    m_nProgram = nProg;
    m_nAttrPos = m_gl.glGetAttribLocation(nProg, "aPos");
    m_nAttrColor = m_gl.glGetAttribLocation(nProg, "aColor");
    m_nLocAngle = m_gl.glGetUniformLocation(nProg, "uAngle");
    m_nLocPitch = m_gl.glGetUniformLocation(nProg, "uPitch");
    m_nLocScale = m_gl.glGetUniformLocation(nProg, "uScale");
    m_nLocAspect = m_gl.glGetUniformLocation(nProg, "uAspect");

    GLfloat verts[36 * 6];
    BuildCubeVertices(verts);
    m_gl.glGenBuffers(1, &m_nVbo);
    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_nVbo);
    m_gl.glBufferData(GL_ARRAY_BUFFER, (GLsizeiptrSOUI)sizeof(verts), verts, GL_STATIC_DRAW);
    m_gl.glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_bShaderOK = (m_nAttrPos >= 0 && m_nAttrColor >= 0);
    m_strReport.AppendFormat(_T("  GLSL 程序                    : %s\n"),
                             m_bShaderOK ? _T("编译链接 OK") : _T("链接 OK 但属性被优化掉"));
    return m_bShaderOK;
}

void CGLWnd::DrawFixedFunction()
{
    if (m_gl.glUseProgram)
        m_gl.glUseProgram(0);

    GLfloat aspect = m_nHeight > 0 ? (GLfloat)m_nWidth / (GLfloat)m_nHeight : 1.0f;
    const GLdouble nearZ = 1.0;
    const GLdouble top = nearZ * 0.41421356; // fov = 45°
    const GLdouble right = top * aspect;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-right, right, -top, top, nearZ, 10.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -3.0f / m_fZoom);
    glRotatef(m_fPitch, 1.0f, 0.0f, 0.0f);
    glRotatef(m_fYaw + m_fAngle, 0.0f, 1.0f, 0.0f);

    glBegin(GL_QUADS);
    for (int f = 0; f < 6; ++f)
    {
        glColor3fv(kFaceColor[f]);
        for (int v = 0; v < 4; ++v)
            glVertex3fv(kCubeFaces[f][v]);
    }
    glEnd();

    // 2D 叠加层：正交投影 + alpha 混合（验证 glOrtho / GL_BLEND）
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    glColor4f(1.0f, 0.84f, 0.25f, 0.35f);
    glVertex2f(0.02f, 0.03f);
    glVertex2f(0.28f, 0.03f);
    glVertex2f(0.28f, 0.10f);
    glVertex2f(0.02f, 0.10f);
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void CGLWnd::DrawShaderPath()
{
    if (!m_nProgram)
        return;

    GLfloat aspect = m_nHeight > 0 ? (GLfloat)m_nWidth / (GLfloat)m_nHeight : 1.0f;
    const float kDeg2Rad = 3.14159265f / 180.0f;
    m_gl.glUseProgram(m_nProgram);
    m_gl.glUniform1f(m_nLocAngle, (m_fAngle + m_fYaw) * kDeg2Rad);
    m_gl.glUniform1f(m_nLocPitch, m_fPitch * kDeg2Rad);
    m_gl.glUniform1f(m_nLocScale, m_fZoom);
    m_gl.glUniform1f(m_nLocAspect, aspect);

    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_nVbo);
    m_gl.glEnableVertexAttribArray(m_nAttrPos);
    m_gl.glEnableVertexAttribArray(m_nAttrColor);
    m_gl.glVertexAttribPointer(m_nAttrPos, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const void *)0);
    m_gl.glVertexAttribPointer(m_nAttrColor, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
                               (const void *)(3 * sizeof(GLfloat)));
    glDrawArrays(GL_TRIANGLES, 0, 36);
    m_gl.glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CGLWnd::Render(BOOL bSwap)
{
    if (!m_hRC || !m_hDC)
        return;
    RECT rc = {};
    ::GetClientRect(m_hWnd, &rc);
    int nW = rc.right - rc.left;
    int nH = rc.bottom - rc.top;
    if (nW <= 0 || nH <= 0)
        return;
    if (!::wglMakeCurrent(m_hDC, m_hRC))
        return;

    m_nWidth = nW;
    m_nHeight = nH;

    glViewport(0, 0, nW, nH);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_nMode == Mode_Shader && !m_bShaderOK)
        m_nMode = Mode_FixedFunction;

    if (m_nMode == Mode_Shader)
        DrawShaderPath();
    else
        DrawFixedFunction();

    if (bSwap)
        ::SwapBuffers(m_hDC);
    UpdateFPS();
}

BOOL CGLWnd::RecreateContext()
{
    if (!::IsWindow(m_hWnd))
        return FALSE;

    // DC 与像素格式保持不动（SetPixelFormat 一个窗口只能设置一次），
    // 只销毁/重建 HGLRC，并顺带重建挂在上下文上的 VBO/Program。
    if (m_hRC)
    {
        ::wglMakeCurrent(m_hDC, m_hRC);
        if (m_nVbo && m_gl.glDeleteBuffers)
            m_gl.glDeleteBuffers(1, &m_nVbo);
        m_nVbo = 0;
        if (m_nProgram && m_gl.glDeleteProgram)
            m_gl.glDeleteProgram(m_nProgram);
        m_nProgram = 0;
        ::wglMakeCurrent(NULL, NULL);
        ::wglDeleteContext(m_hRC);
        m_hRC = NULL;
    }
    m_gl.Unload();
    m_bShaderOK = FALSE;

    // 重建上下文不应该打断用户当前的视角：CreateContext 里的 SetupGLState 会复位，
    // 所以在外面保存/还原一次。
    float fAngle = m_fAngle, fYaw = m_fYaw, fPitch = m_fPitch, fZoom = m_fZoom;

    BOOL bOK = CreateContext();

    m_fAngle = fAngle;
    m_fYaw = fYaw;
    m_fPitch = fPitch;
    m_fZoom = fZoom;
    Invalidate(FALSE);
    return bOK;
}

BOOL CGLWnd::SaveSnapshot(LPCTSTR pszPath)
{
    if (!m_hRC || !m_hDC)
        return FALSE;
    if (!::wglMakeCurrent(m_hDC, m_hRC))
        return FALSE;

    Render(FALSE); // 只渲染不交换，直接读后缓冲

    RECT rc = {};
    ::GetClientRect(m_hWnd, &rc);
    int nW = rc.right - rc.left;
    int nH = rc.bottom - rc.top;
    if (nW <= 0 || nH <= 0)
        return FALSE;

    int nRowSize = (nW * 3 + 3) & ~3;
    BYTE *pBits = (BYTE *)malloc((size_t)nRowSize * nH);
    if (!pBits)
        return FALSE;

    glPixelStorei(GL_PACK_ALIGNMENT, 4); // 4 字节对齐，与 BMP 的行填充一致
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, nW, nH, GL_RGB, GL_UNSIGNED_BYTE, pBits);
    GLenum err = glGetError();
    ::SwapBuffers(m_hDC); // 这一帧也正常显示

    BOOL bOK = FALSE;
    if (err == GL_NO_ERROR)
        bOK = WriteBMP24(pszPath, nW, nH, pBits, nRowSize);
    free(pBits);
    return bOK;
}

void CGLWnd::UpdateFPS()
{
    m_nFrames++;
    DWORD dwNow = ::GetTickCount();
    if (m_dwFpsTick == 0)
        m_dwFpsTick = dwNow;
    if (dwNow - m_dwFpsTick >= 1000)
    {
        m_fFPS = (float)m_nFrames * 1000.0f / (float)(dwNow - m_dwFpsTick);
        m_nFrames = 0;
        m_dwFpsTick = dwNow;
    }
}

SStringT CGLWnd::GetSummary() const
{
    RECT rcW = {}, rcC = {};
    if (::IsWindow(m_hWnd))
    {
        ::GetWindowRect(m_hWnd, &rcW);
        ::GetClientRect(m_hWnd, &rcC);
    }
    SStringT str;
    str.Format(_T("GL_VERSION: %s    GLSL: %s"), m_strVersion.c_str(), m_strGLSL.c_str());
    str.AppendFormat(_T("\nGL_RENDERER: %s    GL_VENDOR: %s"), m_strRenderer.c_str(),
                     m_strVendor.c_str());
    str.AppendFormat(_T("\n模式: %s    动画: %s    FPS: %.1f    窗口: %dx%d  客户区: %dx%d"),
                     m_nMode == Mode_Shader ? _T("GLSL 着色器") : _T("固定管线"),
                     m_bAnimating ? _T("运行中") : _T("已暂停"), m_fFPS, rcW.right - rcW.left,
                     rcW.bottom - rcW.top, rcC.right - rcC.left, rcC.bottom - rcC.top);
    return str;
}

SStringT CGLWnd::GetProbeReport() const
{
    return m_strReport;
}

void CGLWnd::SetAnimating(BOOL bAnim)
{
    if (bAnim == m_bAnimating)
        return;
    m_bAnimating = bAnim;
    if (m_bAnimating)
        SetTimer(GL_ANIM_TIMER, GL_ANIM_ELAPSE, NULL);
    else
        KillTimer(GL_ANIM_TIMER);
}

void CGLWnd::ToggleAnimating()
{
    SetAnimating(!m_bAnimating);
}

CGLWnd::RenderMode CGLWnd::ToggleRenderMode()
{
    if (m_nMode == Mode_FixedFunction)
        m_nMode = (m_bShaderOK || InitShaderPath()) ? Mode_Shader : Mode_FixedFunction;
    else
        m_nMode = Mode_FixedFunction;
    Invalidate(FALSE);
    return m_nMode;
}

//////////////////////////////////////////////////////////////////////////
void CGLWnd::OnPaint(HDC hdc)
{
    PAINTSTRUCT ps = {};
    ::BeginPaint(m_hWnd, &ps);
    Render(TRUE);
    ::EndPaint(m_hWnd, &ps);
}

BOOL CGLWnd::OnEraseBkgnd(HDC hdc)
{
    return TRUE; // 背景由 glClear 负责，避免 GDI 先刷一遍造成闪烁
}

void CGLWnd::OnSize(UINT nType, CSize size)
{
    m_nWidth = size.cx;
    m_nHeight = size.cy;
    Invalidate(FALSE);
}

void CGLWnd::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent != GL_ANIM_TIMER)
    {
        SetMsgHandled(FALSE);
        return;
    }
    m_fAngle += 1.2f;
    if (m_fAngle > 360.0f)
        m_fAngle -= 360.0f;
    Invalidate(FALSE);
}

void CGLWnd::OnLButtonDown(UINT nFlags, CPoint pt)
{
    m_bDragging = TRUE;
    m_ptDragLast = pt;
    SetCapture();
    SetFocus();
}

void CGLWnd::OnLButtonUp(UINT nFlags, CPoint pt)
{
    if (m_bDragging)
    {
        m_bDragging = FALSE;
        ReleaseCapture();
    }
}

void CGLWnd::OnMouseMove(UINT nFlags, CPoint pt)
{
    if (!m_bDragging)
        return;
    m_fYaw += (float)(pt.x - m_ptDragLast.x) * 0.6f;
    m_fPitch += (float)(pt.y - m_ptDragLast.y) * 0.6f;
    if (m_fYaw > 360.0f)
        m_fYaw -= 360.0f;
    if (m_fYaw < -360.0f)
        m_fYaw += 360.0f;
    if (m_fPitch > 89.0f)
        m_fPitch = 89.0f;
    if (m_fPitch < -89.0f)
        m_fPitch = -89.0f;
    m_ptDragLast = pt;
    Invalidate(FALSE);
}

BOOL CGLWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    m_fZoom += (float)zDelta / 1200.0f;
    if (m_fZoom < 0.4f)
        m_fZoom = 0.4f;
    if (m_fZoom > 4.0f)
        m_fZoom = 4.0f;
    Invalidate(FALSE);
    return TRUE;
}

void CGLWnd::OnLButtonDblClk(UINT nFlags, CPoint pt)
{
    ToggleAnimating();
}

void CGLWnd::OnDestroy()
{
    SetAnimating(FALSE);
    ReleaseGL();
    SetMsgHandled(FALSE);
}
