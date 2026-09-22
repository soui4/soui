cmake_minimum_required(VERSION 3.18.1)

project("soui-ohos-lib")

# ============================================================================
# soui-ohos-lib 现已退化为「HAR 壳 + native 源码/头文件提供方」。
# native 的实际编译由各业务 app 通过 soui4_ohos.cmake 完成，
# 见 games/cnchess/client/ohos/entry/src/main/cpp/CMakeLists.txt 与
# demos/ohos-demo/entry/src/main/cpp/CMakeLists.txt。
#
# 本文件仅供需要单独编译本模块时复用同一套构建逻辑（与业务 app 保持一致）。
# 注意：日常构建（app 内）不会触发本文件；native 由业务 app 统一编译。
#
# 单独编译时 SOUI_ROOT_DIR 可通过 hvigor 的 cmake.arguments 传入；
# 未传入时按本文件位置回退到 soui4 仓库根目录。
# ============================================================================

set(SOUI_OHOS_LIB_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR})
if(NOT DEFINED SOUI_ROOT_DIR)
    set(SOUI_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/soui4_ohos.cmake)
