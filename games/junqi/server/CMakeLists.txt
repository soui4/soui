# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../algorithm)
 
# Source files 
file(GLOB_RECURSE SERVER_HEADERS *.h *.hpp)
file(GLOB_RECURSE SERVER_SOURCES *.cpp)
file(GLOB_RECURSE SERVER_RESOURCES *.rc *.rc2)

source_group("Header Files" FILES ${SERVER_HEADERS})
source_group("Source Files" FILES ${SERVER_SOURCES})
source_group("Resource Files" FILES ${SERVER_RESOURCES})

set(current_project "junqi_server")

# 创建可执行文件
add_executable(${current_project} 
    ${SERVER_SOURCES}
    ${SERVER_HEADERS}
    ${SERVER_RESOURCES}
)

# 链接库
target_link_libraries(${current_project}
    utilities4
    junqi_algorithm
    ws
    log4z
)


if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(${current_project} swinx)
endif()


set_target_properties (${current_project} PROPERTIES
    FOLDER "games/junqi"
)