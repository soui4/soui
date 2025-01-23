cmake_minimum_required (VERSION 3.10 FATAL_ERROR)
project(aupng)

###################################################################
# aupng
###################################################################

add_library(aupng
    include/upng.h
    include/upng_config.h
    src/upng_internal.h
    src/upng.c
    src/upng_inflate.c
    src/upng_decode.c
)

target_include_directories(aupng
    PUBLIC include
)
target_compile_options(aupng PRIVATE $<$<CXX_COMPILER_ID:GNU>:-fPIC> $<$<CXX_COMPILER_ID:Clang>:-fPIC>)

set(COM_LIBS ${COM_LIBS} aupng CACHE INTERNAL "com_lib")
set_target_properties (aupng PROPERTIES
    FOLDER third-part
)

install(TARGETS aupng
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS aupng
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
