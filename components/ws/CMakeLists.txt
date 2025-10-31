cmake_minimum_required(VERSION 3.10)

include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)

file(GLOB HEADERS src/*.h)
file(GLOB SOURCES src/*.cpp)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

project(ws)
include_directories(${PROJECT_SOURCE_DIR}/include)

if (NOT ENABLE_SOUI_COM_LIB)
    add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS} ws.rc)
    target_link_libraries(${PROJECT_NAME}  websockets)
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
    target_link_libraries(${PROJECT_NAME} PUBLIC  websockets)
endif()
get_target_property(websockets_includes websockets INTERFACE_INCLUDE_DIRECTORIES)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(platform "x64")
else()
        set(platform "x86")
endif()
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/third-part/openssl_1.1.1d/${platform}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${websockets_includes})

set(COM_LIBS ${COM_LIBS} ${PROJECT_NAME} CACHE INTERNAL "com_lib")
set_target_properties (${PROJECT_NAME} PROPERTIES
    FOLDER components
)

install(TARGETS ${PROJECT_NAME}
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS ${PROJECT_NAME}
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)


if(0)
project(ServerTest)
add_executable(${PROJECT_NAME} test/ServerTest.cpp)

project(ClientTest)
add_executable(${PROJECT_NAME} test/ClientTest.cpp)
endif()


