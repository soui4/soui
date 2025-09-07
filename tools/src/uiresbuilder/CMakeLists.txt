
cmake_minimum_required(VERSION 3.16)
project(uiresbuilder VERSION 1.0.0)

# 查找所有cpp源文件
file(GLOB_RECURSE SOURCES
    "*.cpp"
)

# 查找所有头文件
file(GLOB_RECURSE HEADERS
    "*.h"
)


# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 设置包含目录
target_include_directories(${PROJECT_NAME} PRIVATE .)

remove_definitions(-DUNICODE -D_UNICODE)

# 根据操作系统设置特定选项
if(NOT WIN32)
    add_dependencies(${PROJECT_NAME} swinx)
    target_link_libraries(${PROJECT_NAME} PRIVATE swinx)
endif()

target_precompile_headers(${PROJECT_NAME} PRIVATE "stdafx.h")
set_target_properties (${PROJECT_NAME} PROPERTIES
    FOLDER tools
)
install(TARGETS ${PROJECT_NAME}
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
)

install(TARGETS ${PROJECT_NAME}
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
)