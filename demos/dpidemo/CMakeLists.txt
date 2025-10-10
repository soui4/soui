add_definitions(-D_CRT_SECURE_NO_WARNINGS)

include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/soui-sys-resource)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})


file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resouece" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})

link_directories(${SOUI_ROOT}/bin)
add_executable(dpidemo WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC})
add_dependencies(dpidemo translator)
target_link_libraries(dpidemo soui4 utilities4 jsoncpp)
target_precompile_headers(dpidemo PRIVATE "stdafx.h")


set_target_properties (dpidemo PROPERTIES
    FOLDER demos
)
