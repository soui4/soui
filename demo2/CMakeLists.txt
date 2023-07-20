add_definitions(-D_CRT_SECURE_NO_WARNINGS)

include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/soui-sys-resource)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE CURRENT_HEADERS *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.c)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resouece" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})

if (CMAKE_CL_64)
    if("${CMAKE_GENERATOR_PLATFORM}" MATCHES "ARM64")
    link_directories(${PROJECT_SOURCE_DIR}/bin64a)
    else()
    link_directories(${PROJECT_SOURCE_DIR}/bin64)
    endif()
else ()
    link_directories(${PROJECT_SOURCE_DIR}/bin)
endif ()
add_executable(demo2 WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC})

#set_target_properties(demo2 PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
add_dependencies(demo2 soui-sys-resource render-gdi imgdecoder-gdip)
target_link_libraries(demo2 soui4 utilities4)
set_target_properties (demo2 PROPERTIES
    FOLDER demos
)
