

include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/soui-sys-resource)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/controls.extend)

file(GLOB_RECURSE CURRENT_SRCS  *.cpp)

set(current_project "default")
if(SOUI_ENABLE_CORE_LIB)
add_library(${current_project} STATIC ${CURRENT_SRCS})
else()
add_library(${current_project} SHARED ${CURRENT_SRCS})
set_target_properties(${current_project} PROPERTIES 
    SUFFIX ".exctrl"
)
endif()
target_link_libraries(${current_project} PUBLIC soui4 utilities4 ExtendCtrls)

set_target_properties (${current_project} PROPERTIES
    FOLDER demos/uieditor
)
