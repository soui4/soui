# ============================================================================
# soui4_android.cmake
# ----------------------------------------------------------------------------
# 可复用的 SOUI Android native 构建片段，供各业务 app 的 CMakeLists.txt 通过
# include() 引入。本片段只定义构建逻辑，不创建 project()。
#
# 调用前需由业务 CMake 设置以下变量：
#   SOUI_ROOT_DIR            - soui4 仓库根目录
#   SOUI_ANDROID_LIB_CPP_DIR - soui-android-lib/src/main/cpp 目录
#
# 本片段会：
#   1. add_subdirectory 编译 SOUI 核心（soui4/utilities4/swinx/third-part/components）
#   2. add_library(soui4android SHARED ...) 编译 Android JNI 桥
#
# 业务 CMake 随后 add_library(自己的 so) 并
#   target_link_libraries(... soui4android ...)
#
# 重要约束：JNI 采用静态注册（Java_com_soui_android_*，见
# soui-android-lib/src/main/cpp/src/Soui4Android.cpp），导出库名必须保持
# "soui4android"，否则 AAR 内 SouiPlatformBridge.java 的
# System.loadLibrary("soui4android") 会找不到符号。请勿改名。
# ============================================================================

message(STATUS "soui4_android.cmake: SOUI_ROOT_DIR=${SOUI_ROOT_DIR}")
message(STATUS "soui4_android.cmake: SOUI_ANDROID_LIB_CPP_DIR=${SOUI_ANDROID_LIB_CPP_DIR}")

# Android 工具链对 __attribute__((constructor)) 的 TRY_RUN 探测需要固化结果
set(TOOLCHAIN_SUPPORTS_ATTRIBUTE_CONSTRUCTOR_EXITCODE
        "0" CACHE STRING "Result from TRY_RUN" FORCE)
set(TOOLCHAIN_SUPPORTS_ATTRIBUTE_CONSTRUCTOR_EXITCODE__TRYRUN_OUTPUT
        "" CACHE STRING "Output from TRY_RUN" FORCE)
set(TOOLCHAIN_SUPPORTS_ATTRIBUTE_DESTRUCTOR_EXITCODE
        "0" CACHE STRING "Result from TRY_RUN" FORCE)
set(TOOLCHAIN_SUPPORTS_ATTRIBUTE_DESTRUCTOR_EXITCODE__TRYRUN_OUTPUT
        "" CACHE STRING "Output from TRY_RUN" FORCE)

# SOUI 根目录的 cmake 工具函数
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${SOUI_ROOT_DIR}/__cmake/")
include(${CMAKE_MODULE_PATH}/internal_utils.cmake)
set(BUILD_FOR_ARM OFF)
detectArmTarget(BUILD_FOR_ARM)
message(STATUS "BUILD_FOR_ARM: ${BUILD_FOR_ARM}")

add_definitions(-DWCHAR_SIZE=4)

# 动态库模式：所有模块编译为 .so
set(SOUI_ENABLE_CORE_LIB OFF)
set(SOUI_ENABLE_SVG ON)
# 以下两个重量级开关允许业务侧通过 -D 覆盖（默认开启，保证 cnchess 等行为不变）：
# SOUI_BUILD_RICHEDIT  - 富文本编辑控件（richedit41）
# SOUI_BUILD_WS        - WebSocket / OpenSSL / libcurl（联网能力）
# 纯 UI demo 通常不需要联网，可传入 -DSOUI_BUILD_WS=OFF 显著缩短配置与编译时间。
if(NOT DEFINED SOUI_BUILD_RICHEDIT)
    set(SOUI_BUILD_RICHEDIT ON)
endif()
set(SOUI_ENABLE_ACC  OFF)
set(SOUI_ENABLE_SPY  OFF)
if(NOT DEFINED SOUI_BUILD_WS)
    set(SOUI_BUILD_WS ON)
endif()

configure_file("${SOUI_ROOT_DIR}/__cmake/soui-config.cmake.in"
        "${PROJECT_BINARY_DIR}/soui-config.cmake" @ONLY)
configure_file("${SOUI_ROOT_DIR}/config/config.h.in"
        "${PROJECT_BINARY_DIR}/config/config.h" @ONLY)
configure_file("${SOUI_ROOT_DIR}/config/build.cfg.in"
        "${PROJECT_BINARY_DIR}/config/build.cfg" @ONLY)

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

# swinx 的 Android platform 层直接 include 本桥的头，暴露给所有子目录
include_directories(${SOUI_ANDROID_LIB_CPP_DIR})

#=============================================================================
# 添加子目录（按照依赖顺序）
#=============================================================================
# 1. swinx (Windows API 封装层)
add_subdirectory(${SWINX_DIR} ${CMAKE_CURRENT_BINARY_DIR}/swinx)

#=============================================================================
# Include Directories
#=============================================================================
include_directories(
        ${SOUI_ROOT_DIR}
        ${SWINX_DIR}/include
        ${PROJECT_BINARY_DIR}/config
)

# 2. utilities (工具库，依赖 swinx)
add_subdirectory(${UTILITIES_DIR} ${CMAKE_CURRENT_BINARY_DIR}/utilities)
# 3. SOUI 核心（依赖 utilities 和 swinx）
add_subdirectory(${SOUI_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/SOUI)
# 4. third-part (第三方库，包括 skia)
add_subdirectory(${THIRD_PART_DIR} ${CMAKE_CURRENT_BINARY_DIR}/third_part)
# 5. components (组件，依赖 soui4 和 third-part)
add_subdirectory(${COMPONENTS_DIR} ${CMAKE_CURRENT_BINARY_DIR}/components)

#=============================================================================
# soui4android Library（Android JNI 桥，库名保持 soui4android）
#=============================================================================
file(GLOB_RECURSE SOUI_ANDROID_SRC
        ${SOUI_ANDROID_LIB_CPP_DIR}/src/*.cpp
)

add_library(
        soui4android
        SHARED
        ${SOUI_ANDROID_SRC}
)

target_include_directories(
        soui4android
        PRIVATE
        ${SOUI_ANDROID_LIB_CPP_DIR}
        ${SOUI_ANDROID_LIB_CPP_DIR}/include
        ${SOUI_ROOT_DIR}
        ${SOUI_SOURCE_DIR}/include
        ${UTILITIES_DIR}/include
        ${SWINX_DIR}/include
        ${COMPONENTS_DIR}
        ${PROJECT_BINARY_DIR}/config
)

target_link_libraries(
        soui4android
        PUBLIC
        soui4
        utilities4
        swinx
        android
        log
        jnigraphics
)
