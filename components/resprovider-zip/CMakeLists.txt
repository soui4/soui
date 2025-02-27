add_definitions(-D_CRT_SECURE_NO_WARNINGS)

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

set(LIBS utilities4 zlib)
if(NOT CMAKE_SYSTEM_NAME MATCHES Windows)
include_directories(${CMAKE_SOURCE_DIR}swinx/include)
set(LIBS ${LIBS} swinx)
endif()
if (NOT ENABLE_SOUI_COM_LIB)
    set (resprovider-zip_src  ${resprovider-zip_src} resprovider-zip.rc)
    add_library(resprovider-zip SHARED ${resprovider-zip_src} ${resprovider-zip_header})
    target_link_libraries(resprovider-zip ${LIBS})
else()
    add_library(resprovider-zip STATIC ${resprovider-zip_src} ${resprovider-zip_header})
endif()

add_dependencies(resprovider-zip ${LIBS})

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
