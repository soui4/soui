set(CMAKE_CXX_STANDARD 14)

add_definitions(-D_CRT_SECURE_NO_WARNINGS -DZIP_DISABLE_WILDCARD)

include_directories(${PROJECT_SOURCE_DIR}/third-part/7z)
include_directories(${PROJECT_SOURCE_DIR}/third-part/bit7z/include)
include_directories(${PROJECT_SOURCE_DIR}/third-part/bit7z/third_party)
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
	${PROJECT_SOURCE_DIR}/third-part/7z/SevenZip/FileStream.h
	${PROJECT_SOURCE_DIR}/third-part/7z/SevenZip/FileStream.cpp
	${PROJECT_SOURCE_DIR}/third-part/7z/SevenZip/ErpExcept.h
	${PROJECT_SOURCE_DIR}/third-part/7z/SevenZip/ErpExcept.cpp
	${PROJECT_SOURCE_DIR}/third-part/7z/SevenZip/BlobBuffer.h
	${PROJECT_SOURCE_DIR}/third-part/7z/SevenZip/BlobBuffer.cpp
)

add_definitions(-DRESPROVIDER7ZIP_EXPORTS)

source_group("Header Files" FILES ${resprovider-7zip_header})
source_group("Source Files" FILES ${resprovider-7zip_src})

if (NOT ENABLE_SOUI_COM_LIB)
    set (resprovider-7zip_src  ${resprovider-7zip_src} resprovider-7zip.rc)
    add_library(resprovider-7zip SHARED ${resprovider-7zip_src} ${resprovider-7zip_header})
    target_link_libraries(resprovider-7zip utilities4 bit7z)
else()
    add_library(resprovider-7zip STATIC ${resprovider-7zip_src} ${resprovider-7zip_header})
endif()

set(BIT7Z_AUTO_FORMAT ON CACHE BOOL "enable auto format support" FORCE)
set(BIT7Z_USE_NATIVE_STRING ON CACHE BOOL "enable using native OS strings" FORCE)

add_dependencies(resprovider-7zip bit7z utilities4)
if(MSVC)
set_target_properties(resprovider-7zip PROPERTIES LINK_FLAGS "/SAFESEH:NO") 
endif()

set(COM_LIBS ${COM_LIBS} resprovider-7zip CACHE INTERNAL "com_lib")
set_target_properties (resprovider-7zip PROPERTIES
    FOLDER components
)

set_target_properties(resprovider-7zip PROPERTIES POSITION_INDEPENDENT_CODE ON)

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
