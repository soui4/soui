add_definitions(-D_CRT_SECURE_NO_WARNINGS -DZIP_DISABLE_WILDCARD)

include_directories(${PROJECT_SOURCE_DIR}/third-part/zlib)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)

set(resprovider-zip_header
	stdafx.h
	SResProviderZip.h
	ZipArchive.h
)

set(resprovider-zip_src 
    SResProviderZip.cpp
    zipArchive.cpp
)

source_group("Header Files" FILES ${resprovider-zip_header})
source_group("Source Files" FILES ${resprovider-zip_src})

if (NOT ENABLE_SOUI_COM_LIB)
    set (resprovider-zip_src  ${resprovider-zip_src} resprovider-zip.rc)
    add_library(resprovider-zip SHARED ${resprovider-zip_src} ${resprovider-zip_header})
    target_link_libraries(resprovider-zip utilities4 zlib)
else()
    add_library(resprovider-zip STATIC ${resprovider-zip_src} ${resprovider-zip_header})
endif()

add_dependencies(resprovider-zip zlib utilities4)

set(COM_LIBS ${COM_LIBS} resprovider-zip CACHE INTERNAL "com_lib")
set_target_properties (resprovider-zip PROPERTIES
    FOLDER components
)

install(TARGETS resprovider-zip
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS resprovider-zip
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
