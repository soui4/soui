# ============================================================================
# soui4_ohos.cmake
# ----------------------------------------------------------------------------
# 可复用的 SOUI OpenHarmony native 构建片段，供各业务 app 的 CMakeLists.txt
# 通过 include() 引入。本片段只定义构建逻辑，不创建 project()。
#
# 调用前需由业务 CMake 设置以下变量：
#   SOUI_ROOT_DIR         - soui4 仓库根目录
#   SOUI_OHOS_LIB_CPP_DIR - soui-ohos-lib/src/main/cpp 目录
#
# 本片段会：
#   1. add_subdirectory 编译 SOUI 核心（soui4/utilities4/swinx/third-part/components）
#   2. add_library(soui4ohos SHARED ...) 编译 OHOS N-API 桥
#
# 业务 CMake 随后 add_library(自己的 so) 并
#   target_include_directories(... ${PROJECT_BINARY_DIR}/config ...)
#   target_link_libraries(... soui4ohos ...)
#
# 注意：config.h / soui-config.cmake 生成在 ${PROJECT_BINARY_DIR}/config，
# 业务侧引用该目录取 SOUI 配置头（与本片段 configure_file 的输出一致）。
# ============================================================================

if(NOT DEFINED SOUI_ROOT_DIR)
    message(FATAL_ERROR "soui4_ohos.cmake: SOUI_ROOT_DIR is not set")
endif()
if(NOT DEFINED SOUI_OHOS_LIB_CPP_DIR)
    message(FATAL_ERROR "soui4_ohos.cmake: SOUI_OHOS_LIB_CPP_DIR is not set")
endif()

message(STATUS "soui4_ohos.cmake: SOUI_ROOT_DIR=${SOUI_ROOT_DIR}")
message(STATUS "soui4_ohos.cmake: SOUI_OHOS_LIB_CPP_DIR=${SOUI_OHOS_LIB_CPP_DIR}")

# OHOS 工具链对 __attribute__((constructor/destructor)) 的 TRY_RUN 探测需要固化结果
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

# 与仓库根 CMakeLists.txt 一致：include 方式下根文件未加载，需在此判定 OHOS 平台，
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
# 以下两个重量级开关允许业务侧通过 -D 覆盖（默认开启，保证 cnchess 等行为不变）：
# SOUI_BUILD_RICHEDIT  - 富文本编辑控件（richedit41）
# SOUI_BUILD_WS        - WebSocket / OpenSSL / libcurl（联网能力）
if(NOT DEFINED SOUI_BUILD_RICHEDIT)
    set(SOUI_BUILD_RICHEDIT ON)
endif()
set(SOUI_ENABLE_ACC  OFF)
set(SOUI_ENABLE_SPY  OFF)
if(NOT DEFINED SOUI_BUILD_WS)
    set(SOUI_BUILD_WS ON)
endif()

configure_file("${SOUI_ROOT_DIR}/__cmake/soui4-config.cmake.in" "${PROJECT_BINARY_DIR}/soui-config.cmake" @ONLY)
configure_file("${SOUI_ROOT_DIR}/config/config.h.in" "${PROJECT_BINARY_DIR}/config/config.h" @ONLY)
configure_file("${SOUI_ROOT_DIR}/config/build.cfg.in" "${PROJECT_BINARY_DIR}/config/build.cfg" @ONLY)

set(SOUI_SOURCE_DIR ${SOUI_ROOT_DIR}/SOUI)
set(SWINX_DIR ${SOUI_ROOT_DIR}/swinx)
set(UTILITIES_DIR ${SOUI_ROOT_DIR}/utilities)
set(COMPONENTS_DIR ${SOUI_ROOT_DIR}/components)
set(THIRD_PART_DIR ${SOUI_ROOT_DIR}/third-part)
set(EXTENDCTRL_DIR ${SOUI_ROOT_DIR}/controls.extend)

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

# swinx 的 OHOS platform 层直接 include 本桥的头，暴露给所有子目录
include_directories(${SOUI_OHOS_LIB_CPP_DIR})

#=============================================================================
# 添加子目录（按照依赖顺序）
#=============================================================================

# 1. swinx (Windows API 封装层) - 使用 OHOS 平台配置
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
add_subdirectory(${EXTENDCTRL_DIR} ${CMAKE_CURRENT_BINARY_DIR}/controls.extend)

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
# soui4ohos Library（OHOS N-API 桥，库名保持 soui4ohos：
# HAR 内 ArkTS 的 import soui4ohos from 'libsoui4ohos.so' 依赖此名字，请勿改名）
#=============================================================================

file(GLOB_RECURSE SOUI_OHOS_SRC
        ${SOUI_OHOS_LIB_CPP_DIR}/src/*.cpp
        )

# 桥接实现（swinx::ohos::*）编译进 swinx 而非 soui4ohos：
# libsoui4.so 链接带 --no-undefined，必须在链接期解析这些符号，
# 而 swinx 是 soui4 与 soui4ohos 共同链接的库。
list(REMOVE_ITEM SOUI_OHOS_SRC
        ${SOUI_OHOS_LIB_CPP_DIR}/src/OhosBridge.cpp
        )
target_sources(swinx PRIVATE ${SOUI_OHOS_LIB_CPP_DIR}/src/OhosBridge.cpp)

add_library(
        soui4ohos
        SHARED
        ${SOUI_OHOS_SRC}
)

target_include_directories(
        soui4ohos
        PRIVATE
        ${SOUI_OHOS_LIB_CPP_DIR}
        ${SOUI_OHOS_LIB_CPP_DIR}/include
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
        ExtendCtrls
        soui4
        utilities4
        swinx
        ${OHOS_LINK_LIBS}
        dl
        m
)
