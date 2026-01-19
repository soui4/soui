cmake_minimum_required(VERSION 3.10)

project(network)

# 添加 utilities 依赖
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../utilities/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../SOUI/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# 只保留网络相关的核心头文件
set(HEADERS 
    src/socket.h 
    src/tcp_client_socket.h 
    src/udp_socket.h 
    src/raw_udp_socket.h 
    src/server_socket.h
    src/network.h 
)

# 只保留网络相关的核心源文件
set(SOURCES 
    src/socket.cpp 
    src/tcp_client_socket.cpp 
    src/udp_socket.cpp 
    src/raw_udp_socket.cpp 
    src/server_socket.cpp
    src/network.cpp
)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(SOURCES ${SOURCES} 
        src/private/windows/socket_win32.cpp 
        src/private/windows/system_util_win32.cpp
        )
else()
    set(SOURCES ${SOURCES} 
        src/private/posix/socket_posix.cpp 
        src/private/posix/system_util_posix.cpp
        )
    if(NOT CMAKE_SYSTEM_NAME MATCHES "Darwin")
        set(SOURCES ${SOURCES} 
            src/private/linux/ifaddrs.c
            )
    endif()
endif()

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

# 链接库
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(${PROJECT_NAME} ws2_32 iphlpapi utilities4)
else()
    target_link_libraries(${PROJECT_NAME} utilities4)
endif()

set(COM_LIBS ${COM_LIBS} ${PROJECT_NAME} CACHE INTERNAL "com_lib")
set_target_properties (${PROJECT_NAME} PROPERTIES
    FOLDER components
)

# Add test project
option(SOUI_BUILD_NETWORK_TEST "Build network test" OFF)
if(SOUI_BUILD_NETWORK_TEST)
    add_subdirectory(test)
endif()
