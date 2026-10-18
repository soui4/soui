#include "stdafx.h"
#include "GLProc.h"

/**
 * @brief 取一个 GL 入口地址。
 *
 * @details 先试 wglGetProcAddress；Windows 上部分驱动对不存在的入口返回 1/2/3/-1
 *          而不是 NULL，所以要把这些"伪句柄"也当成失败，再去 opengl32.dll 的导出表
 *          里找（GL 1.1 的入口是 opengl32 正常导出的）。macOS/Linux 上 wgl.h 兼容层
 *          的实现已内置 dlsym/glxGetProcAddress 回退，直接返回即可。
 */
#ifdef _WIN32
static PROC GetGLProcAddress(const char *pszName)
{
    PROC p = ::wglGetProcAddress(pszName);
    if (p == NULL || p == (PROC)1 || p == (PROC)2 || p == (PROC)3 || p == (PROC)-1)
    {
        HMODULE hGL = ::GetModuleHandleA("opengl32.dll");
        p = hGL ? ::GetProcAddress(hGL, pszName) : NULL;
        if (p == (PROC)1 || p == (PROC)2 || p == (PROC)3 || p == (PROC)-1)
            p = NULL;
    }
    return p;
}
#else
static PROC GetGLProcAddress(const char *pszName)
{
    return (PROC)::wglGetProcAddress(pszName);
}
#endif

SGLProc::SGLProc()
{
    Unload();
}

void SGLProc::Unload()
{
    glCreateShader = NULL;
    glShaderSource = NULL;
    glCompileShader = NULL;
    glGetShaderiv = NULL;
    glGetShaderInfoLog = NULL;
    glDeleteShader = NULL;

    glCreateProgram = NULL;
    glAttachShader = NULL;
    glLinkProgram = NULL;
    glGetProgramiv = NULL;
    glGetProgramInfoLog = NULL;
    glDeleteProgram = NULL;
    glUseProgram = NULL;
    glGetAttribLocation = NULL;
    glGetUniformLocation = NULL;
    glUniform1f = NULL;

    glGenBuffers = NULL;
    glDeleteBuffers = NULL;
    glBindBuffer = NULL;
    glBufferData = NULL;
    glEnableVertexAttribArray = NULL;
    glVertexAttribPointer = NULL;
}

void SGLProc::Load()
{
    glCreateShader = (PFN_GL_CreateShader)GetGLProcAddress("glCreateShader");
    glShaderSource = (PFN_GL_ShaderSource)GetGLProcAddress("glShaderSource");
    glCompileShader = (PFN_GL_CompileShader)GetGLProcAddress("glCompileShader");
    glGetShaderiv = (PFN_GL_GetShaderiv)GetGLProcAddress("glGetShaderiv");
    glGetShaderInfoLog = (PFN_GL_GetShaderInfoLog)GetGLProcAddress("glGetShaderInfoLog");
    glDeleteShader = (PFN_GL_DeleteShader)GetGLProcAddress("glDeleteShader");

    glCreateProgram = (PFN_GL_CreateProgram)GetGLProcAddress("glCreateProgram");
    glAttachShader = (PFN_GL_AttachShader)GetGLProcAddress("glAttachShader");
    glLinkProgram = (PFN_GL_LinkProgram)GetGLProcAddress("glLinkProgram");
    glGetProgramiv = (PFN_GL_GetProgramiv)GetGLProcAddress("glGetProgramiv");
    glGetProgramInfoLog = (PFN_GL_GetProgramInfoLog)GetGLProcAddress("glGetProgramInfoLog");
    glDeleteProgram = (PFN_GL_DeleteProgram)GetGLProcAddress("glDeleteProgram");
    glUseProgram = (PFN_GL_UseProgram)GetGLProcAddress("glUseProgram");
    glGetAttribLocation = (PFN_GL_GetAttribLocation)GetGLProcAddress("glGetAttribLocation");
    glGetUniformLocation = (PFN_GL_GetUniformLocation)GetGLProcAddress("glGetUniformLocation");
    glUniform1f = (PFN_GL_Uniform1f)GetGLProcAddress("glUniform1f");

    glGenBuffers = (PFN_GL_GenBuffers)GetGLProcAddress("glGenBuffers");
    glDeleteBuffers = (PFN_GL_DeleteBuffers)GetGLProcAddress("glDeleteBuffers");
    glBindBuffer = (PFN_GL_BindBuffer)GetGLProcAddress("glBindBuffer");
    glBufferData = (PFN_GL_BufferData)GetGLProcAddress("glBufferData");
    glEnableVertexAttribArray = (PFN_GL_EnableVertexAttribArray)GetGLProcAddress("glEnableVertexAttribArray");
    glVertexAttribPointer = (PFN_GL_VertexAttribPointer)GetGLProcAddress("glVertexAttribPointer");
}

BOOL SGLProc::HasShader() const
{
    return glCreateShader && glShaderSource && glCompileShader && glGetShaderiv && glGetShaderInfoLog &&
           glDeleteShader && glCreateProgram && glAttachShader && glLinkProgram && glGetProgramiv &&
           glGetProgramInfoLog && glDeleteProgram && glUseProgram && glGetAttribLocation &&
           glGetUniformLocation && glUniform1f;
}

BOOL SGLProc::HasVBO() const
{
    return glGenBuffers && glDeleteBuffers && glBindBuffer && glBufferData && glEnableVertexAttribArray &&
           glVertexAttribPointer;
}
