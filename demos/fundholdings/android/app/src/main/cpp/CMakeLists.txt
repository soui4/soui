cmake_minimum_required(VERSION 3.18.1)

project("fundholdings-android")

# ============================================================================
# Android 15+ 16KB 页设备：ELF PT_LOAD 段必须按 16KB (0x4000) 对齐
# ----------------------------------------------------------------------------
# 本工程使用 AGP 8.2.0 默认的 NDK 25.1.8937393，其 ld.lld 默认仍按 4KB
# (max-page-size=4096) 布局 LOAD 段，GNU_RELRO 的 (VirtAddr + MemSiz) 也不是
# 16KB 的整数倍；这样的 .so 在 16KB 页设备上会被 linker 拒绝加载。
# NDK r28+ 才默认 16KB 对齐，r27 及以下必须显式给出下面两个链接参数。
#
# add_link_options() 是 directory-scoped 属性，放在 include(soui4_android.cmake)
# 之前，会由所有 add_subdirectory() 继承，覆盖 SOUI 核心 / swinx / components /
# third-part / controls.extend 以及本文件里定义的全部本地 .so，无需逐个 target 设置。
# 32 位 ABI 加上这两个参数同样合法（只是段间留白略多），保持全部 ABI 行为一致。
# ============================================================================
add_link_options(
        "-Wl,-z,max-page-size=16384"
        "-Wl,-z,common-page-size=16384"
)

# Android 工具链不支持 TRY_RUN，这里预先给出结果，避免配置阶段报错
set( TOOLCHAIN_SUPPORTS_ATTRIBUTE_CONSTRUCTOR_EXITCODE
        "0" CACHE STRING "Result from TRY_RUN" FORCE)
set( TOOLCHAIN_SUPPORTS_ATTRIBUTE_CONSTRUCTOR_EXITCODE__TRYRUN_OUTPUT
        "" CACHE STRING "Output from TRY_RUN" FORCE)
set( TOOLCHAIN_SUPPORTS_ATTRIBUTE_DESTRUCTOR_EXITCODE
        "0" CACHE STRING "Result from TRY_RUN" FORCE)
set( TOOLCHAIN_SUPPORTS_ATTRIBUTE_DESTRUCTOR_EXITCODE__TRYRUN_OUTPUT
        "" CACHE STRING "Output from TRY_RUN" FORCE)

# 以下变量由 Gradle 的 externalNativeBuild.cmake.arguments 传入（见 app/build.gradle）：
#   SOUI_ROOT_DIR            - soui4 仓库根目录
#   SOUI_ANDROID_LIB_CPP_DIR - soui-android-lib/src/main/cpp（native 源码与头文件）
#   SOUI_CONTROLS_EXTEND_DIR - controls.extend 目录
#   FUNDHOLDINGS_DIR         - 本 demo 的共享业务源码目录
message(STATUS "SOUI_ROOT_DIR: ${SOUI_ROOT_DIR}")
message(STATUS "SOUI_ANDROID_LIB_CPP_DIR: ${SOUI_ANDROID_LIB_CPP_DIR}")
message(STATUS "SOUI_CONTROLS_EXTEND_DIR: ${SOUI_CONTROLS_EXTEND_DIR}")
message(STATUS "FUNDHOLDINGS_DIR: ${FUNDHOLDINGS_DIR}")
if(NOT EXISTS ${SOUI_ROOT_DIR})
    message(FATAL_ERROR "SOUI_ROOT_DIR: ${SOUI_ROOT_DIR} is not existed")
endif()
if(NOT EXISTS ${SOUI_ANDROID_LIB_CPP_DIR})
    message(FATAL_ERROR "SOUI_ANDROID_LIB_CPP_DIR: ${SOUI_ANDROID_LIB_CPP_DIR} is not existed")
endif()
if(NOT EXISTS ${SOUI_CONTROLS_EXTEND_DIR})
    message(FATAL_ERROR "SOUI_CONTROLS_EXTEND_DIR: ${SOUI_CONTROLS_EXTEND_DIR} is not existed")
endif()
if(NOT EXISTS ${FUNDHOLDINGS_DIR})
    message(FATAL_ERROR "FUNDHOLDINGS_DIR: ${FUNDHOLDINGS_DIR} is not existed")
endif()
# android_entry.cc 里定义了业务入口与 R 实体；路径指错时 glob 会静默返回空列表，
# 生成的 .so 不含业务代码，运行时 InitSoui4AndroidEntry 从未被调用（启动即崩）。
if(NOT EXISTS ${FUNDHOLDINGS_DIR}/android_entry.cc)
    message(FATAL_ERROR "FUNDHOLDINGS_DIR: ${FUNDHOLDINGS_DIR} does not contain android_entry.cc")
endif()

# 复用 SOUI Android native 构建片段：编译 SOUI 核心并生成 soui4android JNI 桥
include(${SOUI_ANDROID_LIB_CPP_DIR}/soui4_android.cmake)

# 手机/平板与桌面版只用同一份业务源码，区别仅在资源目录与窗口尺寸（来自 XML 布局）
add_definitions(-D__MOBILE__)

# ============================================================================
# 源文件
# ============================================================================

file(GLOB FUNDHOLDINGS_SRC
        ${FUNDHOLDINGS_DIR}/*.cpp
        ${FUNDHOLDINGS_DIR}/android_entry.cc
)

# main.cpp 是桌面入口（定义 R 实体与 _tWinMain），移动端由 android_entry.cc 提供入口；
# 两者同时编入会重复定义 R。
list(FILTER FUNDHOLDINGS_SRC EXCLUDE REGEX "/main\\.cpp$")

# SGridCtrl 属于扩展控件，桌面版由 exe 直接编译，移动版同样编入本库
file(GLOB FUNDHOLDINGS_GRIDCTRL_SRC
        ${SOUI_CONTROLS_EXTEND_DIR}/gridctrl/*.cpp
)

# ============================================================================
# fundholdings-android
# ============================================================================

add_library(
        fundholdings-android
        SHARED
        ${FUNDHOLDINGS_SRC}
        ${FUNDHOLDINGS_GRIDCTRL_SRC}
)

target_include_directories(
        fundholdings-android
        PRIVATE
        ${FUNDHOLDINGS_DIR}
        ${SOUI_ANDROID_LIB_CPP_DIR}/include
        ${SOUI_ROOT_DIR}/SOUI/include
        ${SOUI_ROOT_DIR}/SOUI/include/config
        ${SOUI_ROOT_DIR}/swinx/include
        ${SOUI_ROOT_DIR}/utilities/include
        ${SOUI_ROOT_DIR}/components
        ${SOUI_CONTROLS_EXTEND_DIR}
        ${SOUI_CONTROLS_EXTEND_DIR}/gridctrl
        ${PROJECT_BINARY_DIR}/config
)

# soui4android 内部已 PUBLIC 链接 soui4/utilities4/swinx；
# httpclient 提供 IHttpClient（libcurl 后端），由 SOUI_BUILD_HTTPCLIENT=ON 打开。
target_link_libraries(
        fundholdings-android
        PUBLIC
        soui4android
        soui4
        utilities4
        swinx
        httpclient
        android
        log
        jnigraphics
)
