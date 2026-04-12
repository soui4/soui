

include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/soui-sys-resource)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})


file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS}) 


set(current_project "svg2png")

add_executable(${current_project} WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS})

add_dependencies(${current_project} soui4 ${COM_LIBS})
target_link_libraries(${current_project} soui4)

if(SOUI_ENABLE_COM_LIB)
    target_link_libraries(${current_project} ${COM_LIBS})
endif()
set_target_properties (${current_project} PROPERTIES
    FOLDER demos
)
