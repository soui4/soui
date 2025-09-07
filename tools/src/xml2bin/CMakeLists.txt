
cmake_minimum_required(VERSION 3.16)
project(xml2bin VERSION 1.0.0)

# 查找所有cpp源文件
file(GLOB_RECURSE SOURCES
    "*.cpp"
)

# 查找所有头文件
file(GLOB_RECURSE HEADERS
    "*.h"
)

if(NOT CMAKE_SYSTEM_NAME MATCHES Windows)
    remove_definitions(-DUNICODE -D_UNICODE)
endif()
# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
add_dependencies(${PROJECT_NAME} soui4 utilities4)
target_link_libraries(${PROJECT_NAME} PRIVATE soui4 utilities4)
# 设置包含目录
get_target_property(SOUI4_INCLUDE_DIRS soui4 INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(${PROJECT_NAME} PRIVATE ${SOUI4_INCLUDE_DIRS})

# 根据操作系统设置特定选项
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)
    if(MINGW)
        target_link_options(${PROJECT_NAME} PRIVATE -municode)
    endif()
else(WIN32)
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