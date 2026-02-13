add_definitions(-D_CRT_SECURE_NO_WARNINGS)

include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/soui-sys-resource)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB CURRENT_HEADERS *.h *.hpp)
file(GLOB DEMO_SRCS  *.cpp)
file(GLOB TRAYICON trayicon/*.*)
file(GLOB SNAPSHOT snapshot/*.*)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

if (CMAKE_SYSTEM_NAME MATCHES Windows)
set(CURRENT_SRCS ${DEMO_SRCS} ${TRAYICON} ${SNAPSHOT})
else()
set(CURRENT_SRCS ${DEMO_SRCS} ${TRAYICON})
endif()

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resource" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})

add_executable(wxDemo WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC} ${EXTENDCOTROLS})

add_dependencies(wxDemo soui4 ${COM_LIBS})
target_link_libraries(wxDemo soui4)
if (CMAKE_SYSTEM_NAME MATCHES Windows)
    add_dependencies(wxDemo soui-sys-resource)
else()
    target_link_libraries(wxDemo dl)
endif()
if(SOUI_ENABLE_COM_LIB)
    target_link_libraries(wxDemo ${COM_LIBS})
endif()
target_precompile_headers(wxDemo PRIVATE "stdafx.h")
set_target_properties (wxDemo PROPERTIES
    FOLDER demos
)