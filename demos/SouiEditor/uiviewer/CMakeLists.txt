#
# 1394020320@qq.com
#

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${SOUI_INCLUDE}/config)
include_directories(${SOUI_INCLUDE}/components)
include_directories(${SOUI_INCLUDE}/utilities/include)
include_directories(${SOUI_INCLUDE}/SOUI/include)
include_directories(${SOUI_INCLUDE}/third-part/Scintilla/include)
include_directories(${SOUI_INCLUDE}/controls.extend)

file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resouece" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})

add_executable(uiviewer WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC})
target_link_libraries(uiviewer soui4 utilities4 ExtendCtrls)

target_precompile_headers(uiviewer PRIVATE "stdafx.h")
set_target_properties (uiviewer PROPERTIES
    FOLDER demos/uieditor
)

install(TARGETS uiviewer
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS uiviewer
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
