cmake_minimum_required(VERSION 3.18.1)

project("cnchess-ohos")

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
# 路径计算：本工程位于 <SOUI_ROOT>/games/cnchess/client/ohos
# ============================================================================
set(SOUI_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../)
get_filename_component(SOUI_ROOT_DIR "${SOUI_ROOT_DIR}" ABSOLUTE)
set(SOUI_OHOS_LIB_DIR ${SOUI_ROOT_DIR}/soui-ohos-lib/src/main/cpp)
set(CNCHESS_CLIENT_DIR ${SOUI_ROOT_DIR}/games/cnchess/client)
set(CNCHESS_ROOT_DIR ${SOUI_ROOT_DIR}/games/cnchess)
set(CNCHESS_ALGORITHM_DIR ${CNCHESS_ROOT_DIR}/algorithm)
set(CONTROLS_EXTEND_GIF_DIR ${SOUI_ROOT_DIR}/controls.extend/gif)

message(STATUS "===========================================")
message(STATUS "SOUI OHOS cnchess Configuration")
message(STATUS "===========================================")
message(STATUS "SOUI_ROOT_DIR: ${SOUI_ROOT_DIR}")
message(STATUS "SOUI_OHOS_LIB_DIR: ${SOUI_OHOS_LIB_DIR}")
message(STATUS "CNCHESS_CLIENT_DIR: ${CNCHESS_CLIENT_DIR}")
message(STATUS "CNCHESS_ALGORITHM_DIR: ${CNCHESS_ALGORITHM_DIR}")
message(STATUS "CONTROLS_EXTEND_GIF_DIR: ${CONTROLS_EXTEND_GIF_DIR}")
message(STATUS "===========================================")

if(NOT EXISTS ${SOUI_OHOS_LIB_DIR})
    message(FATAL_ERROR "SOUI_OHOS_LIB_DIR: ${SOUI_OHOS_LIB_DIR} is not existed")
endif()
if(NOT EXISTS ${CNCHESS_CLIENT_DIR})
    message(FATAL_ERROR "CNCHESS_CLIENT_DIR: ${CNCHESS_CLIENT_DIR} is not existed")
endif()

# ============================================================================
# 编译宏定义（与 soui-ohos-lib 保持一致）
# ============================================================================
add_definitions(
        -DSOUI_OHOS
        -D__OHOS__
        -D__MOBILE__
        -DOHOS
        -DWCHAR_SIZE=4
        -DSOUI_EXPORTS
        -D_CRT_SECURE_NO_WARNINGS
        -DSOUI_RENDER_USE_SKIA
        -DSOUI_IMAGE_DECODER_USE_STB
        -DENABLE_VIRTUAL_HWND
)

# ============================================================================
# 编译选项
# ============================================================================
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-fexceptions)
add_compile_options(-fPIC)
add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-Wno-unused-parameter)
add_compile_options(-Wno-macro-redefined)

# ============================================================================
# 添加 soui-ohos-lib 子目录（编译出 libsoui4ohos.so 及 SOUI 核心库）
# ============================================================================
add_subdirectory(${SOUI_OHOS_LIB_DIR} ${CMAKE_CURRENT_BINARY_DIR}/soui-ohos-lib)

# ============================================================================
# cnchess 业务层 native 代码（复用 android 共享源码，排除 android_entry.cc）
# ============================================================================
file(GLOB CNCHESS_CLIENT_SRC
        ${CNCHESS_CLIENT_DIR}/*.cpp
        ${CNCHESS_CLIENT_DIR}/ohos_entry.cc
)

file(GLOB_RECURSE CNCHESS_ALGORITHM_SRC
        ${CNCHESS_ALGORITHM_DIR}/*.cpp
)

# SOUI controls.extend/gif 源文件（cnchess 依赖 SSkinAni 和 SGifPlayer）
set(SOUI_GIF_SRC
        ${CONTROLS_EXTEND_GIF_DIR}/SSkinAni.cpp
        ${CONTROLS_EXTEND_GIF_DIR}/SGifPlayer.cpp
)

message(STATUS "CNCHESS_CLIENT_SRC count: ${CNCHESS_CLIENT_SRC}")
message(STATUS "CNCHESS_ALGORITHM_SRC count: ${CNCHESS_ALGORITHM_SRC}")

add_library(
        cnchess
        SHARED
        ${CNCHESS_CLIENT_SRC}
        ${CNCHESS_ALGORITHM_SRC}
        ${SOUI_GIF_SRC}
)

target_include_directories(
        cnchess
        PRIVATE
        ${CNCHESS_CLIENT_DIR}
        ${CNCHESS_CLIENT_DIR}/res
        ${CNCHESS_ROOT_DIR}/algorithm
        ${CNCHESS_ROOT_DIR}/include
        ${SOUI_ROOT_DIR}/SOUI/include
        ${SOUI_ROOT_DIR}/SOUI/include/config
        ${SOUI_ROOT_DIR}/swinx/include
        ${SOUI_ROOT_DIR}/utilities/include
        ${SOUI_ROOT_DIR}/components
        ${CONTROLS_EXTEND_GIF_DIR}
        ${SOUI_OHOS_LIB_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/soui-ohos-lib/config
)

# 链接 soui-ohos-lib 导出的 soui4ohos 目标（其内部已链接 soui4/utilities4/swinx/components/ws）
target_link_libraries(
        cnchess
        PUBLIC
        soui4ohos
)
