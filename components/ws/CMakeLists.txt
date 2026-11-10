cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 11)
include_directories(${SOUI_ROOT_DIR}/utilities/include)
include_directories(${SOUI_ROOT_DIR}/SOUI/include)

# wasm 下不用 libwebsockets（它会拉起自己的事件循环和工作线程），改用浏览器原生
# WebSocket。实现只保留客户端；链接期需要 -lwebsocket.js，否则
# emscripten_websocket_new 是未定义符号。
if(EMSCRIPTEN)
    # commgr2.h 在 components/ 下，建投递队列要用它的 SComMgr2::CreateTaskLoop；
    # 桌面侧由各应用的 include 路径带到，wasm 分支自己点名这一层。
    include_directories(${SOUI_ROOT_DIR}/components)
    add_library(ws STATIC
        src/wasm/WsClientWasm.cpp
        src/wasm/WsClientWasm.h
        src/wasm/WsWasm.cpp)
    target_link_libraries(ws PUBLIC utilities4 swinx websocket.js)
    set(COM_LIBS ${COM_LIBS} ws CACHE INTERNAL "com_lib")
    set_target_properties(ws PROPERTIES FOLDER components)
    return()
endif()

file(GLOB HEADERS CONFIGURE_DEPENDS src/*.h)
file(GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

project(ws)
include_directories(${SOUI_ROOT_DIR}/include)

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
if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(${PROJECT_NAME} swinx)
endif()

project(ClientTest)
add_executable(${PROJECT_NAME} test/ClientTest.cpp)
if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(${PROJECT_NAME} swinx)
endif()
endif()


