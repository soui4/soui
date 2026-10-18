#
# opengl_demo : SRealWnd + SNativeWnd + OpenGL 子窗口演示（Windows / macOS / Linux）
#
# 布局里的 <realwnd/> 由自定义 IRealWndHandler 创建 SNativeWnd 派生的原生子窗口，
# 该子窗口自己建立 OpenGL 上下文并渲染，父窗口仍是 SOUI 的宿主窗口。
#
# 上下文管理按 WGL 语义书写（ChoosePixelFormat / wglCreateContext / wglMakeCurrent /
# SwapBuffers），源码三平台统一写 #include <windows.h> + <GL/gl.h>：
# Windows 直接用系统头；macOS/Linux 命中 swinx 的同名兼容头（转发平台 GL 并拉入
# swinx/include/wgl.h 的 WGL 声明），落地到 NSOpenGLContext / GLX，不需要 GLEW/GLFW。
#

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)

file(GLOB_RECURSE CURRENT_HEADERS *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS *.cpp)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)
file(GLOB_RECURSE CURRENT_SOUIS uires/*.*)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("Resource Files" FILES ${CURRENT_RC})
source_group("SoUI Resource" FILES ${CURRENT_SOUIS})

set(current_project "opengl_demo")
add_executable(${current_project} WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC})

# 与其它 demo 不同，这里只依赖 soui4 和"运行期一定会用到"的渲染/解码组件：
# OpenGL 渲染全部发生在原生子窗口里，DUI 部分只有标题栏、按钮和文本，
# 因此不需要拖上 scriptmodule/network/skia 等整条 COM 组件链。
add_dependencies(${current_project} soui4)
target_link_directories(${current_project} PRIVATE ${SOUI_ROOT}/lib)
target_link_libraries(${current_project} soui4)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_dependencies(${current_project} soui-sys-resource render-gdi imgdecoder-stb)
    # GL 1.1 入口全部由 opengl32 提供；1.5+ 的入口在运行期用 wglGetProcAddress 取。
    target_link_libraries(${current_project} opengl32)
elseif(APPLE)
    add_dependencies(${current_project} soui-sys-resource render-gdi imgdecoder-stb)
    # swinx 内部用 NSOpenGLContext 落地 WGL 接口；demo 自身直连调用 GL 1.1 入口
    # （glBegin 等），Mach-O 两级命名空间要求可执行文件自己链接 OpenGL framework。
    find_library(OPENGL_LIBRARY OpenGL REQUIRED)
    target_link_libraries(${current_project} ${OPENGL_LIBRARY} dl)
else()
    add_dependencies(${current_project} soui-sys-resource render-gdi imgdecoder-stb)
    # swinx 运行期 dlopen libGL 走 GLX；demo 直连调用的 GL 1.1 入口（glBegin 等）
    # 需要链接 libGL 才能解析。缺 GL 开发库时在此报错，提示安装 libgl1-mesa-dev。
    find_package(OpenGL QUIET)
    if(OpenGL_FOUND)
        set(OpenGL_GL_PREFERENCE GLVND)
        target_link_libraries(${current_project} OpenGL::GL dl)
    else()
        message(STATUS "opengl_demo: FindOpenGL failed, fallback to link -lGL")
        target_link_libraries(${current_project} GL dl)
    endif()
endif()

if(SOUI_ENABLE_COM_LIB)
    target_link_libraries(${current_project} ${COM_LIBS} ${COM_DEP_LIBS})
endif()

if (CMAKE_SYSTEM_NAME MATCHES Windows)
    if(MSVC AND MSVC_VERSION GREATER_EQUAL 1900)
        set(DPI_AWARE_MANIFEST "${PROJECT_SOURCE_DIR}/__cmake/dpi_aware.manifest")
        set_target_properties(${current_project} PROPERTIES
            LINK_FLAGS "/MANIFEST:EMBED /MANIFESTINPUT:${DPI_AWARE_MANIFEST}"
        )
    endif()
endif()

set_target_properties (${current_project} PROPERTIES
    FOLDER demos
)
