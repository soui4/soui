
###################################################################
# aupng
###################################################################

add_library(aupng STATIC
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

if (NOT CMAKE_SYSTEM_NAME MATCHES Windows)
target_compile_options(aupng PRIVATE -fPIC)
endif()

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
