# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/controls.extend)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${PROJECT_SOURCE_DIR}/soui-sys-resource)

# Source files 
file(GLOB_RECURSE CURRENT_HEADERS *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS *.cpp)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)
file(GLOB_RECURSE CURRENT_SOUIS *.xml *.idx *.ico *.cur *.bmp *.jpg *.png *.gif *.rtf *.html)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("Resource Files" FILES ${CURRENT_RC})
source_group("SoUI Resource" FILES ${CURRENT_SOUIS})

set(current_project "upgrade_client")
add_executable(${current_project} WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC})
 
add_dependencies(${current_project} soui4 ExtendCtrls)
link_directories(${SOUI_ROOT}/lib)
target_link_libraries(${current_project} soui4 utilities4 ExtendCtrls upgrade_algorithm)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(${current_project} winmm)
    if(MSVC)
    set(DPI_AWARE_MANIFEST "${PROJECT_SOURCE_DIR}/__cmake/dpi_aware.manifest")
    set_target_properties(${current_project} PROPERTIES
            LINK_FLAGS "/MANIFEST:EMBED /MANIFESTINPUT:${DPI_AWARE_MANIFEST}"
        )
    endif(MSVC)
else()
    target_link_libraries(${current_project} swinx)
endif()

set_target_properties (${current_project} PROPERTIES
    FOLDER "games/upgrade"
)