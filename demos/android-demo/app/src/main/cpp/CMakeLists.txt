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

# ============================================================================
# 说明：本 demo CMake 只编译 app 自己的 C++ 代码（CMainDlg）。
# SOUI 核心（soui4/utilities4/swinx/third-part/components）已经在 :soui-android-lib
# 里编译并静态链接进了 libsoui-android.so，这里不再重复 add_subdirectory()。
#
# 为什么不用 Prefab find_package(soui-android)?
#   Prefab 依赖 AAR 里的 CMake 配置元数据（*Config.cmake）。但 AGP 7.2 对
#   `implementation project(':soui-android-lib')` 这种同工程 module 依赖，
#   不会先把 library 打包成 AAR，导致 find_package 找不到配置文件。
#   所以这里直接采用更稳定的做法：
#     1) 手动 include soui-android-lib/src/main/cpp 拿 headers
#     2) 直接按库名 link soui-android —— Gradle 会自动把 dependency module 的
#        externalNativeBuild 输出目录加入 app CMake 的 linker 搜索路径，
#        并且任务时序上保证 :soui-android-lib:externalNativeBuild* 先于
#        :app:configureCMake* 执行，因此 libsoui-android.so 必然存在。
# ============================================================================

# SOUI_ROOT_DIR: 通过 Gradle 的 cmake.arguments 传入，兜底用相对路径回退
set(SOUI_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../../../../")
get_filename_component(SOUI_ROOT_DIR "${SOUI_ROOT_DIR}" ABSOLUTE)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${SOUI_ROOT_DIR}/__cmake/")
include(${CMAKE_MODULE_PATH}/internal_utils.cmake)
set(BUILD_FOR_ARM OFF)
detectArmTarget(BUILD_FOR_ARM)
message(STATUS "BUILD_FOR_ARM: ${BUILD_FOR_ARM}")

add_definitions(-DWCHAR_SIZE=4)

set(SOUI_ENABLE_SVG ON)
set(SOUI_BUILD_RICHEDIT  ON)
set(SOUI_ENABLE_ACC  OFF)
set(SOUI_ENABLE_SPY  OFF)

configure_file("${SOUI_ROOT_DIR}/__cmake/soui-config.cmake.in" "${PROJECT_BINARY_DIR}/soui-config.cmake" @ONLY)
configure_file("${SOUI_ROOT_DIR}/config/config.h.in" "${PROJECT_BINARY_DIR}/config/config.h" @ONLY)
configure_file("${SOUI_ROOT_DIR}/config/build.cfg.in" "${PROJECT_BINARY_DIR}/config/build.cfg" @ONLY)

set(SOUI_SOURCE_DIR ${SOUI_ROOT_DIR}/SOUI)
set(SWINX_DIR ${SOUI_ROOT_DIR}/swinx)
set(UTILITIES_DIR ${SOUI_ROOT_DIR}/utilities)
set(COMPONENTS_DIR ${SOUI_ROOT_DIR}/components)
set(THIRD_PART_DIR ${SOUI_ROOT_DIR}/third-part)


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

add_definitions(-DENABLE_VIRTUAL_HWND)

# swinx's Android platform layer includes the demo bridge header directly, so
# expose the app's native source directory to all subdirectories here.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(${SWINX_DIR} ${CMAKE_CURRENT_BINARY_DIR}/swinx)

include_directories(
        ${SOUI_ROOT_DIR}
        ${SWINX_DIR}/include
        ${PROJECT_BINARY_DIR}/config
)

add_subdirectory(${UTILITIES_DIR} ${CMAKE_CURRENT_BINARY_DIR}/utilities)
add_subdirectory(${SOUI_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/SOUI)
add_subdirectory(${THIRD_PART_DIR} ${CMAKE_CURRENT_BINARY_DIR}/third_part)
add_subdirectory(${COMPONENTS_DIR} ${CMAKE_CURRENT_BINARY_DIR}/components)
#=============================================================================
# Include Directories
#=============================================================================

include_directories(
        ${SOUI_ROOT_DIR}
        ${SOUI_SOURCE_DIR}/include
        ${UTILITIES_DIR}/include
        ${SWINX_DIR}/include
        ${COMPONENTS_DIR}
)

#=============================================================================
# Configuration Summary
#=============================================================================

message(STATUS "===========================================")
message(STATUS "SOUI Android Configuration")
message(STATUS "===========================================")
message(STATUS "SOUI Root: ${SOUI_ROOT_DIR}")
message(STATUS "SOUI Source: ${SOUI_SOURCE_DIR}")
message(STATUS "swinx: ${SWINX_DIR}")
message(STATUS "Utilities: ${UTILITIES_DIR}")
message(STATUS "Components: ${COMPONENTS_DIR}")
message(STATUS "Third Party: ${THIRD_PART_DIR}")
message(STATUS "===========================================")

#=============================================================================
# soui-android Library
#=============================================================================

file(GLOB_RECURSE SOUI_ANDROID_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        )

add_library(
        soui-android-demo
        SHARED
        ${SOUI_ANDROID_SRC}
)

target_include_directories(
        soui-android-demo
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SOUI_ROOT_DIR}
        ${SOUI_SOURCE_DIR}/include
        ${UTILITIES_DIR}/include
        ${SWINX_DIR}/include
        ${COMPONENTS_DIR}
        ${PROJECT_BINARY_DIR}/config
)

target_link_libraries(
        soui-android-demo
        PUBLIC
        soui4
        utilities4
        swinx
        android
        log
        jnigraphics
)