# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)

# Source files 
file(GLOB_RECURSE CURRENT_HEADERS *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS *.cpp)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})

set(current_project "junqi_algorithm")
add_library(${current_project} STATIC ${CURRENT_HEADERS} ${CURRENT_SRCS})
target_link_libraries(${current_project} PUBLIC utilities4)
set_target_properties (${current_project} PROPERTIES
    FOLDER "games/junqi"
)
