
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

set(log4z_header
	log4z.h
)

set(log4z_src
    log4z.cpp)

source_group("Header Files" FILES ${log4z_header})
source_group("Source Files" FILES ${log4z_src})

if (NOT ENABLE_SOUI_COM_LIB)
    set (log4z_src  ${log4z_src} log4z.rc)
    add_library(log4z SHARED ${log4z_src} ${log4z_header})
else()
    add_library(log4z STATIC ${log4z_src} ${log4z_header})
endif()

set(COM_LIBS ${COM_LIBS} log4z CACHE INTERNAL "com_lib")
set_target_properties (log4z PROPERTIES
    FOLDER components
)
#target_link_libraries(log4z shlwapi)

install(TARGETS log4z
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS log4z
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
