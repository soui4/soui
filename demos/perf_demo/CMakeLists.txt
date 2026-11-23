#
# perf_demo -- X11 / Wayland 后端绘制性能对比
#
# 只在 Linux 上构建：同一份源码按 SOUI_LINUX_BACKEND 编出对应后端的可执行
# 文件，两个 build 目录各跑一次即可对比绘制/上屏开销。
#
#   cmake -S . -B build-x11 -DSOUI_LINUX_BACKEND=x11
#   cmake -S . -B build-wl  -DSOUI_LINUX_BACKEND=wayland
#   cmake --build build-x11 -j --target perf_demo
#   cmake --build build-wl  -j --target perf_demo
#
if(CMAKE_SYSTEM_NAME MATCHES "Linux")

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${PROJECT_SOURCE_DIR}/swinx/include)

    file(GLOB CURRENT_SRCS *.cpp)
    file(GLOB CURRENT_HEADERS *.h)

    add_executable(perf_demo ${CURRENT_HEADERS} ${CURRENT_SRCS})
    add_dependencies(perf_demo swinx)
    target_link_libraries(perf_demo swinx pthread dl)

    # 后端名只用于报告表头与窗口标题，真正的后端由 swinx 的编译开关决定。
    if(SOUI_LINUX_BACKEND STREQUAL "wayland" OR SOUI_USE_WAYLAND)
        target_compile_definitions(perf_demo PRIVATE PERF_BACKEND_NAME="wayland")
    else()
        target_compile_definitions(perf_demo PRIVATE PERF_BACKEND_NAME="x11")
    endif()

    set_target_properties(perf_demo PROPERTIES
        FOLDER demos
    )

else()
    message(STATUS "perf_demo: 仅在 Linux 上构建（当前 ${CMAKE_SYSTEM_NAME}），已跳过")
endif()
