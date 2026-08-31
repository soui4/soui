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
# SOUI_INCLUDE_DIR        - SOUI 源码根目录
# SOUI_LIB_DIR            - SOUI 预编译库目录
# SOUI4ANDROID_INCLUDE_DIR - soui4android 头文件目录
# SOUI_CONTROLS_EXTEND_DIR - controls.extend 目录
# CNCHESS_CLIENT_DIR      - cnchess 客户端共享源码目录 (games/cnchess/client)
# CNCHESS_ROOT_DIR        - cnchess 项目根目录 (games/cnchess)
message(STATUS "SOUI_INCLUDE_DIR: ${SOUI_INCLUDE_DIR}")
message(STATUS "SOUI_LIB_DIR: ${SOUI_LIB_DIR}")
message(STATUS "SOUI4ANDROID_INCLUDE_DIR:${SOUI4ANDROID_INCLUDE_DIR}")
message(STATUS "SOUI_CONTROLS_EXTEND_DIR: ${SOUI_CONTROLS_EXTEND_DIR}")
message(STATUS "CNCHESS_CLIENT_DIR: ${CNCHESS_CLIENT_DIR}")
message(STATUS "CNCHESS_ROOT_DIR: ${CNCHESS_ROOT_DIR}")
if(NOT EXISTS ${SOUI_INCLUDE_DIR})
    message(FATAL_ERROR "SOUI_INCLUDE_DIR: ${SOUI_INCLUDE_DIR} is not existed")
endif()
if(NOT EXISTS ${SOUI4ANDROID_INCLUDE_DIR})
    message(FATAL_ERROR "SOUI4ANDROID_INCLUDE_DIR: ${SOUI4ANDROID_INCLUDE_DIR} is not existed")
endif()
if(NOT EXISTS ${SOUI_CONTROLS_EXTEND_DIR})
    message(FATAL_ERROR "SOUI_CONTROLS_EXTEND_DIR: ${SOUI_CONTROLS_EXTEND_DIR} is not existed")
endif()
if(NOT EXISTS ${CNCHESS_CLIENT_DIR})
    message(FATAL_ERROR "CNCHESS_CLIENT_DIR: ${CNCHESS_CLIENT_DIR} is not existed")
endif()

#=============================================================================
# Compile Definitions for Android
#=============================================================================

add_definitions(
        -DSOUI_ANDROID
        -DANDROID
        -D__ANDROID__
        -D__MOBILE__
        -DWCHAR_SIZE=4
        -DSOUI_EXPORTS
        -D_CRT_SECURE_NO_WARNINGS
        -DSOUI_RENDER_USE_SKIA
        -DSOUI_IMAGE_DECODER_USE_STB
        -DENABLE_VIRTUAL_HWND
)

#=============================================================================
# Compiler Options
#=============================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-fexceptions)
add_compile_options(-frtti)
add_compile_options(-fPIC)
add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-Wno-unused-parameter)
add_compile_options(-Wno-macro-redefined)

#=============================================================================
# Include Directories
#=============================================================================

link_directories(
        ${SOUI_LIB_DIR}/${ANDROID_ABI}
)

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
        ${CNCHESS_NATIVE_SRC}
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
        ${SOUI_INCLUDE_DIR}
        ${SOUI_INCLUDE_DIR}/config
        ${SOUI_INCLUDE_DIR}/SOUI/include
        ${SOUI_INCLUDE_DIR}/swinx/include
        ${SOUI_INCLUDE_DIR}/utilities/include
        ${SOUI_INCLUDE_DIR}/components
        ${SOUI4ANDROID_INCLUDE_DIR}
        ${SOUI_CONTROLS_EXTEND_DIR}
)

# 链接所有需要的库
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
