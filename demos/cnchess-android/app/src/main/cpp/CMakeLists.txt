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

# SOUI_INCLUDE_DIR 和 SOUI_LIB_DIR 从 Gradle 的 externalNativeBuild.cmake.arguments 传递进来
message(STATUS "SOUI_INCLUDE_DIR: ${SOUI_INCLUDE_DIR}")
message(STATUS "SOUI_LIB_DIR: ${SOUI_LIB_DIR}")
message(STATUS "SOUI4ANDROID_INCLUDE_DIR:${SOUI4ANDROID_INCLUDE_DIR}")
message(STATUS "SOUI_CONTROLS_EXTEND_DIR: ${SOUI_CONTROLS_EXTEND_DIR}")
if(NOT EXISTS ${SOUI_INCLUDE_DIR})
    message(FATAL_ERROR "SOUI_INCLUDE_DIR: ${SOUI_INCLUDE_DIR} is not existed")
endif()
if(NOT EXISTS ${SOUI4ANDROID_INCLUDE_DIR})
    message(FATAL_ERROR "SOUI4ANDROID_INCLUDE_DIR: ${SOUI4ANDROID_INCLUDE_DIR} is not existed")
endif()
if(NOT EXISTS ${SOUI_CONTROLS_EXTEND_DIR})
    message(FATAL_ERROR "SOUI_CONTROLS_EXTEND_DIR: ${SOUI_CONTROLS_EXTEND_DIR} is not existed")
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

# 客户端源文件（排除 main.cpp, stdafx.cpp, .rc 文件）
file(GLOB_RECURSE CNCHESS_CLIENT_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/client/*.cpp
)
list(REMOVE_ITEM CNCHESS_CLIENT_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/client/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/client/stdafx.cpp
)

# 算法源文件（排除 stdafx.cpp）
file(GLOB_RECURSE CNCHESS_ALGORITHM_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/*.cpp
)
list(REMOVE_ITEM CNCHESS_ALGORITHM_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/stdafx.cpp
)

# 入口和桥接源文件
set(CNCHESS_NATIVE_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/cnchess_native.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SouiRealWndHandler.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/client
        ${CMAKE_CURRENT_SOURCE_DIR}/algorithm
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/res
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
