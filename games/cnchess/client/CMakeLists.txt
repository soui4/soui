# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${PROJECT_SOURCE_DIR}/soui-sys-resource)

# Source files 
file(GLOB_RECURSE CURRENT_HEADERS *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS *.cpp)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("Resource Files" FILES ${CURRENT_RC})

set(current_project "chess")
add_executable(${current_project} WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_RC})

add_dependencies(${current_project} soui4)
link_directories(${SOUI_ROOT}/lib)
target_link_libraries(${current_project} 
    soui4 
    utilities4
    ws2_32
    winmm
)

set_target_properties (${current_project} PROPERTIES
    FOLDER "games/cnchess"
)

# Copy def_theme directory to output directory
add_custom_command(TARGET ${current_project} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${CMAKE_CURRENT_SOURCE_DIR}/def_theme 
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/def_theme
)