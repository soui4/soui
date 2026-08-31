cmake_minimum_required(VERSION 3.18.1)

project("cnchess-android")

set( TOOLCHAIN_SUPPORTS_ATTRIBUTE_CONSTRUCTOR_EXITCODE
        "0"
        CACHE STRING "Result from TRY_RUN" FORCE)

set( TOOLCHAIN_SUPPORTS_ATTRIBUTE_CONSTRUCTOR_EXITCODE__TRYRUN_OUTPUT
        ""
        CACHE STRING "Output from TRY_RUN" FORCE)

set( TOOLCHAIN_SUPPORTS_ATTRIBUTE_DESTRUCTOR_EXITCODE
        "0"
        CACHE STRING "Result from TRY_RUN" FORCE)

set( TOOLCHAIN_SUPPORTS_ATTRIBUTE_DESTRUCTOR_EXITCODE__TRYRUN_OUTPUT
        ""
        CACHE STRING "Output from TRY_RUN" FORCE)

# 以下变量从 Gradle 的 externalNativeBuild.cmake.arguments 传递进来
# SOUI_ROOT_DIR            - soui4 仓库根目录
# SOUI_ANDROID_LIB_CPP_DIR - soui-android-lib/src/main/cpp 目录（提供 native 源码与头文件）
# SOUI_CONTROLS_EXTEND_DIR - controls.extend 目录
# CNCHESS_CLIENT_DIR       - cnchess 客户端共享源码目录 (games/cnchess/client)
# CNCHESS_ROOT_DIR         - cnchess 项目根目录 (games/cnchess)
message(STATUS "SOUI_ROOT_DIR: ${SOUI_ROOT_DIR}")
message(STATUS "SOUI_ANDROID_LIB_CPP_DIR:${SOUI_ANDROID_LIB_CPP_DIR}")
message(STATUS "SOUI_CONTROLS_EXTEND_DIR: ${SOUI_CONTROLS_EXTEND_DIR}")
message(STATUS "CNCHESS_CLIENT_DIR: ${CNCHESS_CLIENT_DIR}")
message(STATUS "CNCHESS_ROOT_DIR: ${CNCHESS_ROOT_DIR}")
if(NOT EXISTS ${SOUI_ROOT_DIR})
    message(FATAL_ERROR "SOUI_ROOT_DIR: ${SOUI_ROOT_DIR} is not existed")
endif()
if(NOT EXISTS ${SOUI_ANDROID_LIB_CPP_DIR})
    message(FATAL_ERROR "SOUI_ANDROID_LIB_CPP_DIR: ${SOUI_ANDROID_LIB_CPP_DIR} is not existed")
endif()
if(NOT EXISTS ${SOUI_CONTROLS_EXTEND_DIR})
    message(FATAL_ERROR "SOUI_CONTROLS_EXTEND_DIR: ${SOUI_CONTROLS_EXTEND_DIR} is not existed")
endif()
if(NOT EXISTS ${CNCHESS_CLIENT_DIR})
    message(FATAL_ERROR "CNCHESS_CLIENT_DIR: ${CNCHESS_CLIENT_DIR} is not existed")
endif()

#=============================================================================
# 引入可复用的 SOUI Android native 构建片段
# 该片段会 add_subdirectory 编译 SOUI 核心，并 add_library(soui4android SHARED)
# 编译 Android JNI 桥。业务侧只需链接 soui4android 即可。
#=============================================================================
include(${SOUI_ANDROID_LIB_CPP_DIR}/soui4_android.cmake)

# 业务侧专属编译定义
add_definitions(-D__MOBILE__)

#=============================================================================
# Source Files
#=============================================================================

# 客户端源文件（引用共享源码目录，排除桌面版独有文件）
file(GLOB CNCHESS_CLIENT_SRC
        ${CNCHESS_CLIENT_DIR}/*.cpp
        ${CNCHESS_CLIENT_DIR}/android_entry.cc
)

# 算法源文件（引用共享 algorithm 目录，排除 stdafx.cpp）
file(GLOB_RECURSE CNCHESS_ALGORITHM_SRC
        ${CNCHESS_ROOT_DIR}/algorithm/*.cpp
)

# SOUI controls.extend/gif 源文件（cnchess 依赖 SSkinAni 和 SGifPlayer）
set(SOUI_GIF_SRC
        ${SOUI_CONTROLS_EXTEND_DIR}/gif/SSkinAni.cpp
        ${SOUI_CONTROLS_EXTEND_DIR}/gif/SGifPlayer.cpp
)

#=============================================================================
# cnchess-android Library
#=============================================================================

add_library(
        cnchess-android
        SHARED
        ${CNCHESS_CLIENT_SRC}
        ${CNCHESS_ALGORITHM_SRC}
        ${SOUI_GIF_SRC}
)

target_include_directories(
        cnchess-android
        PRIVATE
        ${CNCHESS_CLIENT_DIR}
        ${CNCHESS_CLIENT_DIR}/res
        ${CNCHESS_ROOT_DIR}/algorithm
        ${CNCHESS_ROOT_DIR}/include
        ${SOUI_ANDROID_LIB_CPP_DIR}/include
        ${SOUI_ROOT_DIR}/SOUI/include
        ${SOUI_ROOT_DIR}/swinx/include
        ${SOUI_ROOT_DIR}/utilities/include
        ${SOUI_ROOT_DIR}/components
        ${PROJECT_BINARY_DIR}/config
        ${SOUI_CONTROLS_EXTEND_DIR}
)

# 链接所有需要的库（soui4android 由 soui4_android.cmake 提供，
# 其内部已 PUBLIC 链接 soui4/utilities4/swinx 等，这里再次列出无害以便显式化）
target_link_libraries(
        cnchess-android
        PUBLIC
        soui4android
        soui4
        utilities4
        swinx
        android
        log
        jnigraphics
)
