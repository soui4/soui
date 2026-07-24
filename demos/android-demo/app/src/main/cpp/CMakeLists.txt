cmake_minimum_required(VERSION 3.18.1)

project("soui-android-demo")

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

# SOUI_INCLUDE_DIR 和 SOUI_LIB_DIR 从 Gradle 的 externalNativeBuild.cmake.arguments 传递进来
# SOUI_INCLUDE_DIR 指向解压后的 soui4 头文件目录
# SOUI_LIB_DIR 指向解压后的 jni 库目录
message(STATUS "SOUI_INCLUDE_DIR: ${SOUI_INCLUDE_DIR}")
message(STATUS "SOUI_LIB_DIR: ${SOUI_LIB_DIR}")
message(STATUS "SOUI4ANDROID_INCLUDE_DIR:${SOUI4ANDROID_INCLUDE_DIR}")
if(NOT EXISTS  ${SOUI_INCLUDE_DIR})
        message(FATAL_ERROR "SOUI_INCLUDE_DIR: ${SOUI_INCLUDE_DIR} is not existed")
endif()
if(NOT EXISTS  ${SOUI4ANDROID_INCLUDE_DIR})
        message(FATAL_ERROR "SOUI4ANDROID_INCLUDE_DIR: ${SOUI4ANDROID_INCLUDE_DIR} is not existed")
endif()
#=============================================================================
# Compile Definitions for Android
#=============================================================================

add_definitions(
        -DSOUI_ANDROID
        -DANDROID
        -D__ANDROID__
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

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-fexceptions)
add_compile_options(-fPIC)
add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-Wno-unused-parameter)
add_compile_options(-Wno-macro-redefined)

#=============================================================================
# Include Directories
#=============================================================================
# 添加库搜索路径（从 aar 解压出来的 jni 库目录）
link_directories(
        ${SOUI_LIB_DIR}/${ANDROID_ABI}
)

#=============================================================================
# Configuration Summary
#=============================================================================


message(STATUS "===========================================")
message(STATUS "SOUI Android Demo Configuration")
message(STATUS "===========================================")
message(STATUS "SOUI Include Dir: ${SOUI_INCLUDE_DIR}")
message(STATUS "===========================================")

#=============================================================================
# soui-android-demo Library
#=============================================================================
file(GLOB_RECURSE SOUI_ANDROID_DEMO_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        )

add_library(
        soui-android-demo
        SHARED
        ${SOUI_ANDROID_DEMO_SRC}
)


target_include_directories(
        soui-android-demo
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SOUI_INCLUDE_DIR}
        ${SOUI_INCLUDE_DIR}/config
        ${SOUI_INCLUDE_DIR}/SOUI/include
        ${SOUI_INCLUDE_DIR}/swinx/include
        ${SOUI_INCLUDE_DIR}/utilities/include
        ${SOUI_INCLUDE_DIR}/components
        ${SOUI4ANDROID_INCLUDE_DIR}
)

# 链接所有需要的库
# soui4android 依赖 soui4, utilities4, swinx 等动态库，需要显式链接
target_link_libraries(
        soui-android-demo
        PUBLIC
        soui4android
        soui4
        utilities4
        swinx
        android
        log
        jnigraphics
)
