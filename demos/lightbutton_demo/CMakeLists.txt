#
# LightButton Demo CMakeLists.txt
#

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/controls.extend)
include_directories(${PROJECT_SOURCE_DIR}/components)

file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico *.png *.bmp *.jpg *.gif *.jpeg)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

# 直接包含SLightButton源文件
set(LIGHTBUTTON_SRCS
    ${PROJECT_SOURCE_DIR}/controls.extend/SLightButton.cpp
)
set(LIGHTBUTTON_HEADERS
    ${PROJECT_SOURCE_DIR}/controls.extend/SLightButton.h
)

source_group("Header Files" FILES ${CURRENT_HEADERS} ${LIGHTBUTTON_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS} ${LIGHTBUTTON_SRCS})
source_group("SoUI Resouece" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})

add_executable(lightbutton_demo WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC} ${LIGHTBUTTON_HEADERS} ${LIGHTBUTTON_SRCS})

add_dependencies(lightbutton_demo soui4 utilities4)
target_link_libraries(lightbutton_demo utilities4 soui4)


target_precompile_headers(lightbutton_demo PRIVATE "stdafx.h")


set_target_properties (lightbutton_demo PROPERTIES
    FOLDER demos
)
