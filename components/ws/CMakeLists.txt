cmake_minimum_required(VERSION 3.10)

include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)

file(GLOB HEADERS src/*.h)
file(GLOB SOURCES src/*.cpp)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

project(ws)
include_directories(${PROJECT_SOURCE_DIR}/include)

if (NOT SOUI_ENABLE_COM_LIB)
    add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS} ws.rc)
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCES})
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC  websockets utilities4)
if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(${PROJECT_NAME} PUBLIC swinx)
endif()

get_target_property(websockets_includes websockets INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories(${PROJECT_NAME} PRIVATE ${websockets_includes})

set(COM_LIBS ${COM_LIBS} ${PROJECT_NAME} CACHE INTERNAL "com_lib")
set_target_properties (${PROJECT_NAME} PROPERTIES
    FOLDER components
)


if(0)
project(ServerTest)
add_executable(${PROJECT_NAME} test/ServerTest.cpp)
if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(${PROJECT_NAME} swinx)
endif()

project(ClientTest)
add_executable(${PROJECT_NAME} test/ClientTest.cpp)
if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(${PROJECT_NAME} swinx)
endif()
endif()


