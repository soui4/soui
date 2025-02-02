#
#
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB_RECURSE headers
	include/*.h
    )

file(GLOB_RECURSE sources
	src/*.cpp
    )

source_group("Header Files" FILES ${headers})
source_group("Source Files" FILES ${sources})

add_library(detours STATIC ${sources} ${headers})
set(COM_LIBS ${COM_LIBS} detours CACHE INTERNAL "com_lib")
set_target_properties (detours PROPERTIES
    FOLDER third-part
)

install(TARGETS detours
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS detours
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
