

include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/soui-sys-resource)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})


file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico *.png *.jpg *.bmp)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS}) 
source_group("SoUI Resource" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})

set(current_project "mclistview_demo")
add_executable(${current_project} WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC})

add_dependencies(${current_project} soui4 ${COM_LIBS})
target_link_directories(${current_project} PRIVATE ${SOUI_ROOT}/lib)
target_link_libraries(${current_project} soui4)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_dependencies(${current_project} soui-sys-resource)
endif()

if(SOUI_ENABLE_COM_LIB)
    target_link_libraries(${current_project} ${COM_LIBS})
endif()
set_target_properties (${current_project} PROPERTIES
    FOLDER demos
)
