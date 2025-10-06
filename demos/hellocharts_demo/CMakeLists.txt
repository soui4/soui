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

# Add executable
add_executable(hellocharts_demo WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CHARTCTRL_SRC})
 
# Link libraries
target_link_libraries(hellocharts_demo soui4 utilities4)

# Set target properties
if(CMAKE_DEBUG_POSTFIX)
    set_target_properties(hellocharts_demo PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
endif()
if(CMAKE_INSTALL_PREFIX)
    set_target_properties(hellocharts_demo PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_PREFIX})
endif()

# Copy resources
add_custom_command(TARGET hellocharts_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/uires ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
set_target_properties (hellocharts_demo PROPERTIES
    FOLDER demos
)

install(TARGETS hellocharts_demo
    CONFIGURATIONS Release
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/Release
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/Release
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/Release
)

install(TARGETS hellocharts_demo
    CONFIGURATIONS Debug
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/Debug
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/Debug
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/Debug
)