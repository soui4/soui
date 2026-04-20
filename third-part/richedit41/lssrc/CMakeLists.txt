
include_directories(../lsinc)
include_directories(../inci)

add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_NO_WINDOWS)

file(GLOB_RECURSE msls_header ../lsinc/*.h)
file(GLOB_RECURSE msls_src *.c)

source_group("Header Files" FILES ${msls_header})
source_group("Source Files" FILES ${msls_src} msls.rc)

add_library(msls31 STATIC ${msls_src} ${msls_header})
if (NOT CMAKE_SYSTEM_NAME MATCHES Windows)
	target_compile_options(msls31 PRIVATE -fPIC)
endif()

if(SOUI_ENABLE_CORE_LIB)
    set(CORE_LIBS ${CORE_LIBS} msls31 CACHE INTERNAL "core_lib") 
endif()
set_target_properties (msls31 PROPERTIES
    FOLDER third-part
)

install(TARGETS msls31
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS msls31
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)