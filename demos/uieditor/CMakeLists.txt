

include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/soui-sys-resource)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/third-part/Scintilla/include)

include_directories(${CMAKE_SOURCE_DIR}/controls.extend)


file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico *.png *.jpg *.bmp)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS}) 
source_group("SoUI Resouece" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})

set(current_project "uieditor")
add_executable(${current_project} WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC})

add_dependencies(${current_project} soui4 )
link_directories(${SOUI_ROOT}/lib)
target_link_libraries(${current_project} soui4 utilities4 Scintilla ExtendCtrls)
if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
add_app_res_folder(${current_project} ${CMAKE_CURRENT_SOURCE_DIR}/uires uieditor_uires)
endif()
add_app_res_folder(${current_project} ${CMAKE_CURRENT_SOURCE_DIR}/Config Config)
add_app_res_files(${current_project} "${PROJECT_SOURCE_DIR}/soui-sys-resource.zip" ".")
target_link_libraries(${current_project} soui4 utilities4 Scintilla ExtendCtrls)
if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
target_link_libraries(${current_project} swinx)
endif()

target_precompile_headers(${current_project} PRIVATE "stdafx.h")

set_target_properties (${current_project} PROPERTIES
    FOLDER demos
)
