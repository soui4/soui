cmake_minimum_required(VERSION 3.18.1)

project("soui-android-lib")

# ============================================================================
# soui-android-lib 现已退化为「Java 壳 + native 源码/头文件提供方」。
# native 的实际编译由各业务 app 通过 soui4_android.cmake 完成，
# 见 games/cnchess/client/android/app/src/main/cpp/CMakeLists.txt。
#
# 本文件仅供需要单独编译本模块时复用同一套构建逻辑（与业务 app 保持一致）。
# 注意：soui-android-lib 的 build.gradle 已不再 externalNativeBuild，
# 因此日常构建不会触发本文件；native 由业务 app 统一编译。
# ============================================================================

set(SOUI_ANDROID_LIB_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SOUI_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../)

include(${CMAKE_CURRENT_SOURCE_DIR}/soui4_android.cmake)
