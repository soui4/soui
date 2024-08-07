
# OUR SOURCES
set(xcb_sources
xkb.c
)

source_group("Source Files" FILES ${xcb_sources})

add_library(libxcb STATIC ${xcb_sources} )
set(COM_LIBS ${COM_LIBS} libxcb CACHE INTERNAL "com_lib")
target_include_directories(libxcb PRIVATE .)
target_include_directories(libxcb PRIVATE ..)
target_include_directories(libxcb PRIVATE ../xcb)
target_include_directories(libxcb PRIVATE ../sysinclude)
set_target_properties(libxcb PROPERTIES POSITION_INDEPENDENT_CODE ON)

set_target_properties (libxcb PROPERTIES
    FOLDER platform
)

install(TARGETS libxcb
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS libxcb
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
