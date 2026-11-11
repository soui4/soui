# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
# 仓库根不能靠 PROJECT_SOURCE_DIR 拼：本目录有两种进入方式 ——
#   1) 根工程的 games/cnchess 链 add_subdirectory(algorithm)：PROJECT_SOURCE_DIR=仓库根；
#   2) cnchess/client/wasm 独立工程直接 add_subdirectory(<repo>/games/cnchess/algorithm)：
#      PROJECT_SOURCE_DIR=wasm 目录，${PROJECT_SOURCE_DIR}/SOUI/include 会拼成不存在的路径。
# 一律按本文件所在位置算仓库根，两种拓扑都成立。
get_filename_component(_cnchess_repo_root "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
include_directories(${_cnchess_repo_root}/SOUI/include)

# Source files 
file(GLOB_RECURSE CURRENT_HEADERS *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS *.cpp)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})

set(current_project "cnchess_algorithm")
add_library(${current_project} STATIC ${CURRENT_HEADERS} ${CURRENT_SRCS})

# 非 Windows 平台（Linux / Android / OHOS / Emscripten）用 swinx 的 Win32 仿真头：
# stdafx.h 里的 <windows.h> / <tchar.h>、代码里的 BOOL / POINT 全靠它。根工程在
# 自己的目录作用域里 include_directories(swinx/include)，但独立 wasm 工程把根
# CMakeLists 当作子目录加进来时，那句只覆盖根的子树，传不到本目标 —— 必须自己补，
# 否则 em++ 报 'windows.h' file not found。Windows/MSVC 下必须保持排除，让系统真
# windows.h 命中，不能让仿真头抢先。
if(NOT CMAKE_SYSTEM_NAME MATCHES Windows)
    target_include_directories(${current_project} PRIVATE ${_cnchess_repo_root}/swinx/include)
endif()

set_target_properties (${current_project} PROPERTIES
    FOLDER "games/cnchess"
)

target_link_libraries(${current_project} soui4)