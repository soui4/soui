message(STATUS "PROJECT_ROOT_DIR=${PROJECT_ROOT_DIR}")
# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/controls.extend)
include_directories(${PROJECT_SOURCE_DIR}/controls.extend/hellocharts)
# Source files
file(GLOB_RECURSE CURRENT_HEADERS *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS *.xml *.idx *.ico *.cur *.bmp *.jpg *.png *.gif *.rtf *.html)
file(GLOB_RECURSE CHARTCTRL_SRC ${PROJECT_SOURCE_DIR}/controls.extend/hellocharts/*.*)
 
source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resouece" FILES ${CURRENT_SOUIS})

set(current_project "hellocharts_demo")
add_executable(${current_project} WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC} ${CHARTCTRL_SRC})

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

install(TARGETS ${current_project}
    CONFIGURATIONS Release
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/Release
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/Release
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/Release
)

install(TARGETS ${current_project}
    CONFIGURATIONS Debug
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/Debug
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/Debug
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/Debug
)

