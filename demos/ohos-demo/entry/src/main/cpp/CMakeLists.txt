cmake_minimum_required(VERSION 3.18.1)

project("soui-ohos-demo")

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
# 路径计算：demo 位于 <SOUI_ROOT>/demos/ohos-demo，
# soui-ohos-lib 位于 <SOUI_ROOT>/soui-ohos-lib
# ============================================================================
set(SOUI_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../..)
get_filename_component(SOUI_ROOT_DIR "${SOUI_ROOT_DIR}" ABSOLUTE)
set(SOUI_OHOS_LIB_DIR ${SOUI_ROOT_DIR}/soui-ohos-lib/src/main/cpp)

message(STATUS "===========================================")
message(STATUS "SOUI OHOS Demo Configuration")
message(STATUS "===========================================")
message(STATUS "SOUI_ROOT_DIR: ${SOUI_ROOT_DIR}")
message(STATUS "SOUI_OHOS_LIB_DIR: ${SOUI_OHOS_LIB_DIR}")
message(STATUS "===========================================")

# ============================================================================
# 编译宏定义（与 soui-ohos-lib 保持一致）
# ============================================================================
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
# soui-ohos-lib 的 CMakeLists 依赖 SOUI_ROOT_DIR 变量
add_subdirectory(${SOUI_OHOS_LIB_DIR} ${CMAKE_CURRENT_BINARY_DIR}/soui-ohos-lib)

# ============================================================================
# Demo 业务层 native 代码
# ============================================================================
file(GLOB_RECURSE DEMO_NATIVE_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        )

add_library(
        cnchess
        SHARED
        ${DEMO_NATIVE_SRC}
)

target_include_directories(
        cnchess
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SOUI_ROOT_DIR}
        ${SOUI_ROOT_DIR}/SOUI/include
        ${SOUI_ROOT_DIR}/utilities/include
        ${SOUI_ROOT_DIR}/swinx/include
        ${SOUI_ROOT_DIR}/components
        ${SOUI_OHOS_LIB_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/soui-ohos-lib/config
)

# 链接 soui-ohos-lib 导出的 soui4ohos 目标（其内部已链接 soui4/utilities4/swinx）
target_link_libraries(
        cnchess
        PUBLIC
        soui4ohos
)
