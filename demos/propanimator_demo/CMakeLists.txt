# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/components)

# Source files
file(GLOB_RECURSE CURRENT_HEADERS *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS *.cpp)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)
file(GLOB_RECURSE CURRENT_SOUIS *.xml *.idx *.ico *.cur *.bmp *.jpg *.png *.gif *.rtf *.html)
 
source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("Resource Files" FILES ${CURRENT_RC})
source_group("SoUI Resource" FILES ${CURRENT_SOUIS})
 
set(current_project "propanimator_demo")
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
