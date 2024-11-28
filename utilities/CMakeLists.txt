# 
# SOUI cmake配置文件
#

add_definitions(-DUTILITIES_EXPORTS -D_CRT_SECURE_NO_WARNINGS)

file(GLOB_RECURSE CURRENT_HEADERS *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  src/*.cpp)

if (CMAKE_SYSTEM_NAME MATCHES Windows)
    list(APPEND CURRENT_SRCS cursoricon/cursoricon.cpp)
endif()

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})


if (NOT ENABLE_SOUI_CORE_LIB)
    add_library(utilities4 SHARED ${CURRENT_SRCS} ${CURRENT_HEADERS})
else()
    add_library(utilities4 STATIC ${CURRENT_SRCS} ${CURRENT_HEADERS})
endif()

target_include_directories(utilities4
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}

)
if (NOT CMAKE_SYSTEM_NAME MATCHES Windows)
add_dependencies(utilities4  swinx)
target_link_libraries(utilities4  swinx)
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
