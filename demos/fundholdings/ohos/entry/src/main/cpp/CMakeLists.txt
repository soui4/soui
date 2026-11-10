cmake_minimum_required(VERSION 3.5.0)

project(fundholdings)

set(SOUI_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../)
get_filename_component(SOUI_ROOT_DIR ${SOUI_ROOT_DIR} ABSOLUTE)

set(SOUI_OHOS_LIB_CPP_DIR ${SOUI_ROOT_DIR}/soui-ohos-lib/src/main/cpp)
set(SOUI_CONTROLS_EXTEND_DIR ${SOUI_ROOT_DIR}/controls.extend)
set(FUNDHOLDINGS_DIR ${SOUI_ROOT_DIR}/demos/fundholdings)

message(STATUS "SOUI_ROOT_DIR: ${SOUI_ROOT_DIR}")
message(STATUS "SOUI_OHOS_LIB_CPP_DIR: ${SOUI_OHOS_LIB_CPP_DIR}")
message(STATUS "SOUI_CONTROLS_EXTEND_DIR: ${SOUI_CONTROLS_EXTEND_DIR}")
message(STATUS "FUNDHOLDINGS_DIR: ${FUNDHOLDINGS_DIR}")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译宏定义：与 soui-ohos-lib 保持一致（片段的默认值在同一平台上必须口径一致）
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

add_compile_options(-fexceptions)
add_compile_options(-fPIC)
add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-Wno-unused-parameter)
add_compile_options(-Wno-macro-redefined)

# 复用 SOUI OHOS native 构建片段：编译 SOUI 核心并生成 soui4ohos JNI 桥
include(${SOUI_OHOS_LIB_CPP_DIR}/soui4_ohos.cmake)

# 手机/平板与桌面版只用同一份业务源码，区别仅在资源目录与窗口尺寸（来自 XML 布局）
add_definitions(-D__MOBILE__)

# ============================================================================
# 源文件
# ============================================================================

file(GLOB FUNDHOLDINGS_SRC
        ${FUNDHOLDINGS_DIR}/*.cpp
        ${FUNDHOLDINGS_DIR}/ohos_entry.cc
)

# main.cpp 是桌面入口（定义 R 实体与 _tWinMain），移动端由 ohos_entry.cc 提供入口；
# 两者同时编入会重复定义 R。
list(FILTER FUNDHOLDINGS_SRC EXCLUDE REGEX "/main\\.cpp$")

# SGridCtrl 属于扩展控件，桌面版由 exe 直接编译，移动版同样编入本库
file(GLOB FUNDHOLDINGS_GRIDCTRL_SRC
        ${SOUI_CONTROLS_EXTEND_DIR}/gridctrl/*.cpp
)

# ============================================================================
# fundholdings
# ============================================================================

add_library(fundholdings SHARED ${FUNDHOLDINGS_SRC} ${FUNDHOLDINGS_GRIDCTRL_SRC})

target_include_directories(
        fundholdings
        PRIVATE
        ${FUNDHOLDINGS_DIR}
        ${SOUI_OHOS_LIB_CPP_DIR}/include
        ${SOUI_ROOT_DIR}/SOUI/include
        ${SOUI_ROOT_DIR}/SOUI/include/config
        ${SOUI_ROOT_DIR}/swinx/include
        ${SOUI_ROOT_DIR}/utilities/include
        ${SOUI_ROOT_DIR}/components
        ${SOUI_CONTROLS_EXTEND_DIR}
        ${SOUI_CONTROLS_EXTEND_DIR}/gridctrl
        ${PROJECT_BINARY_DIR}/config
)

# soui4ohos 内部已 PUBLIC 链接 soui4/utilities4/swinx/components；
# httpclient 显式列出以保证该 COM 模块随 HAP 一起打包（基金数据抓取依赖它）。
target_link_libraries(
        fundholdings
        PUBLIC
        soui4ohos
        httpclient
)
