#
# 1394020320@qq.com
#
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/controls.extend)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/soui-sys-resource)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB CURRENT_HEADERS  *.h *.hpp)
file(GLOB CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS  uires/*.*)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resource" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})

add_executable(SoTool WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC})

add_dependencies(SoTool soui4 ${COM_LIBS})
target_link_libraries(SoTool soui4)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_dependencies(${current_project} soui-sys-resource)
    target_link_libraries(SoTool comctl32.lib Psapi.lib)
endif()
if(SOUI_ENABLE_COM_LIB)
    target_link_libraries(SoTool ${COM_LIBS})
endif()

target_precompile_headers(SoTool PRIVATE "stdafx.h")
set_target_properties (SoTool PROPERTIES
    FOLDER demos
)