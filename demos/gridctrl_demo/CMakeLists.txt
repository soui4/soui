
include_directories(${PROJECT_SOURCE_DIR}/third-part/jsoncpp/include)
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/soui-sys-resource)
include_directories(${PROJECT_SOURCE_DIR}/controls.extend)
include_directories(${PROJECT_SOURCE_DIR}/controls.extend/gridctrl)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})


file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico *.png *.jpg *.bmp)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)
file(GLOB_RECURSE GRIDCTRL_SRC ${PROJECT_SOURCE_DIR}/controls.extend/gridctrl/*.*)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS}) 
source_group("SoUI Resouece" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})

set(current_project "gridctrl_demo")
add_executable(${current_project} WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC} ${GRIDCTRL_SRC})

add_dependencies(${current_project} soui4 )
link_directories(${SOUI_ROOT}/lib)
target_link_libraries(${current_project} soui4 utilities4)
if(ENABLE_SOUI_COM_LIB)
    if(MSVC_VERSION LESS 1600 AND ENABLE_SOUI_COM_LIB)
    message("vs is less than vs2008")
    if (CMAKE_CL_64)
	    link_directories("${PROJECT_SOURCE_DIR}/components/render-d2d/sdk7.1_d2d/lib/x64")
    else ()
	    link_directories("${PROJECT_SOURCE_DIR}/components/render-d2d/sdk7.1_d2d/lib/x86")
    endif ()
    endif()
	target_link_libraries(${current_project} d2d1.lib dwrite.lib)
endif()
set_target_properties (${current_project} PROPERTIES
    FOLDER demos
)
