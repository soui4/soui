add_definitions(-D_CRT_SECURE_NO_WARNINGS)

include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/soui-sys-resource)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB CURRENT_HEADERS *.h *.hpp)
file(GLOB DEMO_SRCS  *.cpp)
file(GLOB TRAYICON trayicon/*.*)
file(GLOB SNAPSHOT snapshot/*.*)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

if (CMAKE_SYSTEM_NAME MATCHES Windows)
set(CURRENT_SRCS ${DEMO_SRCS} ${TRAYICON} ${SNAPSHOT})
else()
set(CURRENT_SRCS ${DEMO_SRCS} ${TRAYICON})
endif()

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resouece" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})

add_executable(wxDemo WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC} ${EXTENDCOTROLS})

add_dependencies(wxDemo utilities4 soui4 resprovider-zip log4z render-skia render-gdi)

if (CMAKE_SYSTEM_NAME MATCHES Windows)
add_dependencies(demo soui-sys-resource ${COM_LIBS})
target_link_libraries(wxDemo soui4 utilities4)
else()
target_link_libraries(wxDemo soui4 utilities4 dl)
endif()

set_target_properties (wxDemo PROPERTIES
    FOLDER demos
)

install(TARGETS wxDemo
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS wxDemo
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
