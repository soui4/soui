add_definitions(-D_CRT_SECURE_NO_WARNINGS)

include_directories(${PROJECT_SOURCE_DIR}/third-part/7z)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)

set(resprovider-7zip_header
	stdafx.h
	SResProvider7Zip.h
	Zip7Archive.h
)

set(resprovider-7zip_src 
	SResProvider7Zip.cpp
	Zip7Archive.cpp
)

add_definitions(-DRESPROVIDER7ZIP_EXPORTS)

source_group("Header Files" FILES ${resprovider-7zip_header})
source_group("Source Files" FILES ${resprovider-7zip_src})

if (NOT ENABLE_SOUI_COM_LIB)
    set (resprovider-7zip_src  ${resprovider-7zip_src} resprovider-7zip.rc)
    add_library(resprovider-7zip SHARED ${resprovider-7zip_src} ${resprovider-7zip_header})
    target_link_libraries(resprovider-7zip utilities4 7z)
else()
    add_library(resprovider-7zip STATIC ${resprovider-7zip_src} ${resprovider-7zip_header})
endif()

add_dependencies(resprovider-7zip 7z utilities4)
if(MSVC)
set_target_properties(resprovider-7zip PROPERTIES LINK_FLAGS "/SAFESEH:NO") 
endif()

set(COM_LIBS ${COM_LIBS} resprovider-7zip CACHE INTERNAL "com_lib")
set_target_properties (resprovider-7zip PROPERTIES
    FOLDER components
)

install(TARGETS resprovider-7zip
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS resprovider-7zip
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
