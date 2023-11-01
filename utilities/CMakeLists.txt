# 
# SOUI cmake配置文件
#

add_definitions(-DUTILITIES_EXPORTS -D_CRT_SECURE_NO_WARNINGS)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(include)

file(GLOB_RECURSE CURRENT_HEADERS *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)


source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})

if (NOT ENABLE_SOUI_CORE_LIB)
    set (source  ${CURRENT_SRCS} utilities.rc)
    add_library(utilities4 SHARED ${CURRENT_SRCS} ${CURRENT_HEADERS})
    target_link_libraries(utilities4 shlwapi)
else()
    add_library(utilities4 STATIC ${CURRENT_SRCS} ${CURRENT_HEADERS})
endif()

set(CORE_LIBS ${CORE_LIBS} utilities4)
set(CORE_LIBS ${CORE_LIBS} utilities4 CACHE INTERNAL "core_lib")

install(TARGETS utilities4
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS utilities4
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include" DESTINATION include/utilities)
