add_definitions(-D_CRT_SECURE_NO_WARNINGS)

include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/soui-sys-resource)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB CURRENT_HEADERS *.h *.hpp)
file(GLOB CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

file(GLOB SVR_SRCS  server/*.cpp)
file(GLOB CLIENT_SRCS  client/*.cpp)


source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resource" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})

add_executable(ipc_client WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CLIENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC})

add_dependencies(ipc_client soui4 ${COM_LIBS})
target_link_libraries(ipc_client soui4)
if (CMAKE_SYSTEM_NAME MATCHES Windows)
    add_dependencies(ipc_client soui-sys-resource)
else()
    target_link_libraries(ipc_client dl)
endif()

if(SOUI_ENABLE_COM_LIB)
    target_link_libraries(ipc_client ${COM_LIBS})
endif()

set_target_properties (ipc_client PROPERTIES
    FOLDER demos
)


add_executable(ipc_server WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${SVR_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC})
target_link_libraries(ipc_server soui4 ${COM_LIBS})

if (CMAKE_SYSTEM_NAME MATCHES Windows)
    add_dependencies(ipc_server soui-sys-resource)
else()
    target_link_libraries(ipc_server dl)
endif()

if(SOUI_ENABLE_COM_LIB)
    target_link_libraries(ipc_server render-gdi render-skia imgdecoder-stb translator log4z resprovider-7zip sipcobject)
endif()

set_target_properties (ipc_server PROPERTIES
    FOLDER demos
)
