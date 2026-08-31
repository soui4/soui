cmake_minimum_required(VERSION 3.18.1)

project("soui-ohos-lib")

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
# 说明：本 CMake 编译 soui-ohos-lib 自己的 C++ 代码。
# SOUI 核心（soui4/utilities4/swinx/third-part/components）已经在 :soui-ohos-lib
# 里编译并静态链接进了 libsoui-ohos.so。
# ============================================================================

# SOUI_ROOT_DIR: 通过 hvigor 的 cmake.arguments 传入，兜底用相对路径回退
message(STATUS "SOUI_ROOT_DIR: ${SOUI_ROOT_DIR}")

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${SOUI_ROOT_DIR}/__cmake/")
include(${CMAKE_MODULE_PATH}/internal_utils.cmake)
set(BUILD_FOR_ARM OFF)
detectArmTarget(BUILD_FOR_ARM)
message(STATUS "BUILD_FOR_ARM: ${BUILD_FOR_ARM}")

add_definitions(-DWCHAR_SIZE=4)

# 与仓库根 CMakeLists.txt 一致：子目录构建时根文件未加载，需在此判定 OHOS 平台，
# 供 SOUI/windres 等子目录正确链接 OHOS 系统库（ace_napi / hilog / ace_ndk）。
if (CMAKE_SYSTEM_NAME MATCHES "OHOS|OpenHarmony|HarmonyOS")
    set(IS_OHOS ON)
else()
    set(IS_OHOS OFF)
endif()
message(STATUS "IS_OHOS: ${IS_OHOS}")

# 使用动态库模式，所有模块编译为 .so 文件
set(SOUI_ENABLE_CORE_LIB OFF)

set(SOUI_ENABLE_SVG ON)
set(SOUI_BUILD_RICHEDIT  ON)
set(SOUI_ENABLE_ACC  OFF)
set(SOUI_ENABLE_SPY  OFF)
set(SOUI_BUILD_WS ON)

configure_file("${SOUI_ROOT_DIR}/__cmake/soui-config.cmake.in" "${PROJECT_BINARY_DIR}/soui-config.cmake" @ONLY)
configure_file("${SOUI_ROOT_DIR}/config/config.h.in" "${PROJECT_BINARY_DIR}/config/config.h" @ONLY)
configure_file("${SOUI_ROOT_DIR}/config/build.cfg.in" "${PROJECT_BINARY_DIR}/config/build.cfg" @ONLY)

set(SOUI_SOURCE_DIR ${SOUI_ROOT_DIR}/SOUI)
set(SWINX_DIR ${SOUI_ROOT_DIR}/swinx)
set(UTILITIES_DIR ${SOUI_ROOT_DIR}/utilities)
set(COMPONENTS_DIR ${SOUI_ROOT_DIR}/components)
set(THIRD_PART_DIR ${SOUI_ROOT_DIR}/third-part)


#=============================================================================
# Compile Definitions for OHOS
#=============================================================================

add_definitions(
        -DSOUI_OHOS
        -D__OHOS__
        -DOHOS
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

# swinx's OHOS platform layer includes the demo bridge header directly, so
# expose the app's native source directory to all subdirectories here.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})


#=============================================================================
# 添加子目录（按照依赖顺序）
#=============================================================================

# 1. swinx (Windows API 封装层) - 使用 OHOS 平台配置
set(PLATFORM_CMAKE_FILE "${SWINX_DIR}/ohos.cmake")
add_subdirectory(${SWINX_DIR} ${CMAKE_CURRENT_BINARY_DIR}/swinx)


#=============================================================================
# Include Directories (添加所有需要的头文件目录)
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
# Configuration Summary
#=============================================================================

message(STATUS "===========================================")
message(STATUS "SOUI OHOS Library Configuration")
message(STATUS "===========================================")
message(STATUS "SOUI Root: ${SOUI_ROOT_DIR}")
message(STATUS "SOUI Source: ${SOUI_SOURCE_DIR}")
message(STATUS "swinx: ${SWINX_DIR}")
message(STATUS "Utilities: ${UTILITIES_DIR}")
message(STATUS "Components: ${COMPONENTS_DIR}")
message(STATUS "Third Party: ${THIRD_PART_DIR}")
message(STATUS "===========================================")

#=============================================================================
# OHOS N-API / Native Libraries
#=============================================================================

find_library(OHOS_ACE_NAPI ace_napi.z)
find_library(OHOS_ACE_NDK ace_ndk.z)
find_library(OHOS_NATIVE_WINDOW native_window)
find_library(OHOS_HILOG hilog_ndk.z)
find_library(OHOS_NATIVE_BUFFER native_buffer)
find_library(OHOS_IMAGE native_image)
find_library(OHOS_DRAWING native_drawing)
find_library(OHOS_RAWFILE rawfile.z rawfile)

set(OHOS_LINK_LIBS "")
foreach(_ohos_lib IN ITEMS
        OHOS_ACE_NAPI OHOS_ACE_NDK OHOS_NATIVE_WINDOW
        OHOS_HILOG OHOS_NATIVE_BUFFER OHOS_IMAGE OHOS_DRAWING
        OHOS_RAWFILE)
    if(${_ohos_lib})
        list(APPEND OHOS_LINK_LIBS ${${_ohos_lib}})
        message(STATUS "Found OHOS lib: ${_ohos_lib} = ${${_ohos_lib}}")
    endif()
endforeach()

#=============================================================================
# soui4ohos Library
#=============================================================================

file(GLOB_RECURSE SOUI_OHOS_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        )

# 桥接实现（swinx::ohos::*）编译进 swinx 而非 soui4ohos：
# libsoui4.so 链接带 --no-undefined，必须在链接期解析这些符号，
# 而 swinx 是 soui4 与 soui4ohos 共同链接的库。
list(REMOVE_ITEM SOUI_OHOS_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/OhosBridge.cpp
        )
target_sources(swinx PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/OhosBridge.cpp)

add_library(
        soui4ohos
        SHARED
        ${SOUI_OHOS_SRC}
)


target_include_directories(
        soui4ohos
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${SOUI_ROOT_DIR}
        ${SOUI_SOURCE_DIR}/include
        ${UTILITIES_DIR}/include
        ${SWINX_DIR}/include
        ${COMPONENTS_DIR}
        ${PROJECT_BINARY_DIR}/config
)

target_link_libraries(
        soui4ohos
        PUBLIC
        soui4
        utilities4
        swinx
        ${OHOS_LINK_LIBS}
        dl
        m
)
