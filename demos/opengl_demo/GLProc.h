/**
 * @file       GLProc.h
 * @brief      OpenGL 1.5 / 2.0 入口的运行时加载。
 *
 * @details    Windows 上 opengl32.dll 只导出 OpenGL 1.1 的入口，VBO、着色器这些
 *             1.5 之后的入口必须在上下文建立之后用 wglGetProcAddress 取得；
 *             macOS/Linux 上由 swinx 的兼容头（<GL/gl.h> 拉入的 wgl.h）提供
 *             同名声明，实现内部用 dlsym/glxGetProcAddress 落地，GL 1.1 的入口
 *             同样走这里。
 *             这里只声明本 demo 用到的一小把函数，避免为一个 demo 引入 GLEW/GLAD 依赖。
 */

#ifndef __SOUI_GL_PROC_H__
#define __SOUI_GL_PROC_H__

#include <windows.h>
#include <GL/gl.h> // macOS/Linux 上命中 swinx 的兼容头：转发平台 GL 并带出 WGL 声明

/** 系统 GL/gl.h 只声明到 1.1，缺的常量在这里补齐；用 #ifndef 防止与 glext.h 冲突。 */
#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif
#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW 0x88E4
#endif
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif
#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif
#ifndef GL_INFO_LOG_LENGTH
#define GL_INFO_LOG_LENGTH 0x8B84
#endif
#ifndef GL_SHADING_LANGUAGE_VERSION
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#endif

/** 1.1 的头里没有 GLsizeiptr，用 ptrdiff_t 顶替（x86/x64 宽度都正确）。 */
typedef ptrdiff_t GLsizeiptrSOUI;

typedef GLuint(APIENTRY *PFN_GL_CreateShader)(GLenum type);
typedef void(APIENTRY *PFN_GL_ShaderSource)(GLuint shader, GLsizei count, const char *const *string, const GLint *length);
typedef void(APIENTRY *PFN_GL_CompileShader)(GLuint shader);
typedef void(APIENTRY *PFN_GL_GetShaderiv)(GLuint shader, GLenum pname, GLint *params);
typedef void(APIENTRY *PFN_GL_GetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei *length, char *infoLog);
typedef void(APIENTRY *PFN_GL_DeleteShader)(GLuint shader);

typedef GLuint(APIENTRY *PFN_GL_CreateProgram)(void);
typedef void(APIENTRY *PFN_GL_AttachShader)(GLuint program, GLuint shader);
typedef void(APIENTRY *PFN_GL_LinkProgram)(GLuint program);
typedef void(APIENTRY *PFN_GL_GetProgramiv)(GLuint program, GLenum pname, GLint *params);
typedef void(APIENTRY *PFN_GL_GetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei *length, char *infoLog);
typedef void(APIENTRY *PFN_GL_DeleteProgram)(GLuint program);
typedef void(APIENTRY *PFN_GL_UseProgram)(GLuint program);
typedef GLint(APIENTRY *PFN_GL_GetAttribLocation)(GLuint program, const char *name);
typedef GLint(APIENTRY *PFN_GL_GetUniformLocation)(GLuint program, const char *name);
typedef void(APIENTRY *PFN_GL_Uniform1f)(GLint location, GLfloat v0);

typedef void(APIENTRY *PFN_GL_GenBuffers)(GLsizei n, GLuint *buffers);
typedef void(APIENTRY *PFN_GL_DeleteBuffers)(GLsizei n, const GLuint *buffers);
typedef void(APIENTRY *PFN_GL_BindBuffer)(GLenum target, GLuint buffer);
typedef void(APIENTRY *PFN_GL_BufferData)(GLenum target, GLsizeiptrSOUI size, const void *data, GLenum usage);
typedef void(APIENTRY *PFN_GL_EnableVertexAttribArray)(GLuint index);
typedef void(APIENTRY *PFN_GL_VertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);

/**
 * @struct SGLProc
 * @brief  OpenGL 1.5 / 2.0 入口表。Load() 之后成员为 NULL 表示该入口不可用。
 */
struct SGLProc
{
    PFN_GL_CreateShader glCreateShader;
    PFN_GL_ShaderSource glShaderSource;
    PFN_GL_CompileShader glCompileShader;
    PFN_GL_GetShaderiv glGetShaderiv;
    PFN_GL_GetShaderInfoLog glGetShaderInfoLog;
    PFN_GL_DeleteShader glDeleteShader;

    PFN_GL_CreateProgram glCreateProgram;
    PFN_GL_AttachShader glAttachShader;
    PFN_GL_LinkProgram glLinkProgram;
    PFN_GL_GetProgramiv glGetProgramiv;
    PFN_GL_GetProgramInfoLog glGetProgramInfoLog;
    PFN_GL_DeleteProgram glDeleteProgram;
    PFN_GL_UseProgram glUseProgram;
    PFN_GL_GetAttribLocation glGetAttribLocation;
    PFN_GL_GetUniformLocation glGetUniformLocation;
    PFN_GL_Uniform1f glUniform1f;

    PFN_GL_GenBuffers glGenBuffers;
    PFN_GL_DeleteBuffers glDeleteBuffers;
    PFN_GL_BindBuffer glBindBuffer;
    PFN_GL_BufferData glBufferData;
    PFN_GL_EnableVertexAttribArray glEnableVertexAttribArray;
    PFN_GL_VertexAttribPointer glVertexAttribPointer;

    SGLProc();

    /** @brief 取一遍入口地址，必须在 wglMakeCurrent 之后调用。 */
    void Load();

    /** @brief 清空入口表（销毁上下文前调用）。 */
    void Unload();

    /** @brief 着色器相关的入口是否齐全。 */
    BOOL HasShader() const;

    /** @brief VBO 相关的入口是否齐全。 */
    BOOL HasVBO() const;
};

#endif // __SOUI_GL_PROC_H__
