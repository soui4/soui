# 服务器项目配置

# 设置项目名称
project(upgrade_server)

# 定义源文件
set(SERVER_SOURCES
    WebSocketGame.cpp
    Game.cpp
    StdAfx.cpp
    Upgrade.cpp
    server.cpp
    PropBag.cpp
)

# 定义头文件
set(SERVER_HEADERS
    WebSocketGame.h
    Game.h
    stdafx.h
    Upgrade.h
    resource.h
    server.h
    PropBag.h
)

# 定义资源文件
set(SERVER_RESOURCES
    server.rc
)

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/utilities/include
    ${CMAKE_SOURCE_DIR}/SOUI/include
    ${CMAKE_SOURCE_DIR}/components
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set(current_project upgrade_server)
# 创建可执行文件
add_executable(${current_project} 
    ${SERVER_SOURCES}
    ${SERVER_HEADERS}
    ${SERVER_RESOURCES}
)

# 链接库
target_link_libraries(${current_project}
    utilities4
    upgrade_algorithm
)


if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(${current_project} swinx)
endif()

# 设置预处理器定义
target_compile_definitions(${current_project} PRIVATE
    WIN32
    _WINDOWS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

set_target_properties (${current_project} PROPERTIES
    FOLDER "games/upgrade"
)