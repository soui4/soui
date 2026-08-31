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

# 以下变量从 Gradle 的 externalNativeBuild.cmake.arguments 传递进来
# SOUI_ROOT_DIR            - soui4 仓库根目录
# SOUI_ANDROID_LIB_CPP_DIR - soui-android-lib/src/main/cpp 目录（提供 native 源码与头文件）
message(STATUS "SOUI_ROOT_DIR: ${SOUI_ROOT_DIR}")
message(STATUS "SOUI_ANDROID_LIB_CPP_DIR:${SOUI_ANDROID_LIB_CPP_DIR}")
if(NOT EXISTS ${SOUI_ROOT_DIR})
    message(FATAL_ERROR "SOUI_ROOT_DIR: ${SOUI_ROOT_DIR} is not existed")
endif()
if(NOT EXISTS ${SOUI_ANDROID_LIB_CPP_DIR})
    message(FATAL_ERROR "SOUI_ANDROID_LIB_CPP_DIR: ${SOUI_ANDROID_LIB_CPP_DIR} is not existed")
endif()

#=============================================================================
# 引入可复用的 SOUI Android native 构建片段
# 该片段会 add_subdirectory 编译 SOUI 核心，并 add_library(soui4android SHARED)
# 编译 Android JNI 桥。业务侧只需链接 soui4android 即可。
#=============================================================================
include(${SOUI_ANDROID_LIB_CPP_DIR}/soui4_android.cmake)

#=============================================================================
# Source Files
#=============================================================================

file(GLOB SOUI_ANDROID_DEMO_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

#=============================================================================
# soui-android-demo Library
#=============================================================================

add_library(
        soui-android-demo
        SHARED
        ${SOUI_ANDROID_DEMO_SRC}
)

target_include_directories(
        soui-android-demo
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SOUI_ANDROID_LIB_CPP_DIR}/include
        ${SOUI_ROOT_DIR}/SOUI/include
        ${SOUI_ROOT_DIR}/swinx/include
        ${SOUI_ROOT_DIR}/utilities/include
        ${SOUI_ROOT_DIR}/components
        ${PROJECT_BINARY_DIR}/config
)

# 链接所有需要的库（soui4android 由 soui4_android.cmake 提供，
# 其内部已 PUBLIC 链接 soui4/utilities4/swinx 等，这里再次列出无害以便显式化）
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
