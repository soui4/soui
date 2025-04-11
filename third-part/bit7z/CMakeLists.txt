# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

cmake_minimum_required( VERSION 3.10 )

project(bit7z)
set( CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON" )

cmake_policy( SET CMP0022 NEW )
if( POLICY CMP0076 )
    cmake_policy( SET CMP0076 NEW )
endif()

# public headers
set( PUBLIC_HEADERS
     ${PROJECT_SOURCE_DIR}/include/bit7z/bit7z.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bit7zlibrary.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitabstractarchivecreator.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitabstractarchivehandler.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitabstractarchiveopener.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitarchiveeditor.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitarchiveitem.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitarchiveiteminfo.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitarchiveitemoffset.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitarchivereader.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitarchivewriter.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitcompressionlevel.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitcompressionmethod.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitcompressor.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitdefines.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/biterror.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitexception.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitextractor.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitfilecompressor.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitfileextractor.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitformat.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitfs.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitgenericitem.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitinputarchive.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bititemsvector.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitmemcompressor.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitmemextractor.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitoutputarchive.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitpropvariant.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitstreamcompressor.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitstreamextractor.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bittypes.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7z/bitwindows.hpp )

# header files
set( HEADERS
     ${PROJECT_SOURCE_DIR}/src/internal/archiveproperties.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/bufferextractcallback.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/bufferitem.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/bufferutil.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/callback.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/cbufferinstream.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/cbufferoutstream.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/cfileinstream.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/cfileoutstream.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/cfixedbufferoutstream.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/cmultivolumeinstream.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/cmultivolumeoutstream.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/com.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/cstdinstream.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/cstdoutstream.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/csymlinkinstream.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/cvolumeinstream.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/cvolumeoutstream.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/dateutil.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/extractcallback.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/failuresourcecategory.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/fileextractcallback.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/fixedbufferextractcallback.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/formatdetect.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/fsindexer.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/fsitem.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/fsutil.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/fs.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/genericinputitem.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/guiddef.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/guids.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/hresultcategory.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/internalcategory.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/macros.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/opencallback.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/operationcategory.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/operationresult.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/processeditem.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/renameditem.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/stdinputitem.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/streamextractcallback.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/streamutil.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/stringutil.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/updatecallback.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/util.hpp
     ${PROJECT_SOURCE_DIR}/src/internal/windows.hpp )

# source files
set( SOURCES
     ${PROJECT_SOURCE_DIR}/src/bit7zlibrary.cpp
     ${PROJECT_SOURCE_DIR}/src/bitabstractarchivecreator.cpp
     ${PROJECT_SOURCE_DIR}/src/bitabstractarchivehandler.cpp
     ${PROJECT_SOURCE_DIR}/src/bitabstractarchiveopener.cpp
     ${PROJECT_SOURCE_DIR}/src/bitarchiveeditor.cpp
     ${PROJECT_SOURCE_DIR}/src/bitarchiveitem.cpp
     ${PROJECT_SOURCE_DIR}/src/bitarchiveiteminfo.cpp
     ${PROJECT_SOURCE_DIR}/src/bitarchiveitemoffset.cpp
     ${PROJECT_SOURCE_DIR}/src/bitarchivereader.cpp
     ${PROJECT_SOURCE_DIR}/src/bitarchivewriter.cpp
     ${PROJECT_SOURCE_DIR}/src/biterror.cpp
     ${PROJECT_SOURCE_DIR}/src/bitexception.cpp
     ${PROJECT_SOURCE_DIR}/src/bitfilecompressor.cpp
     ${PROJECT_SOURCE_DIR}/src/bitformat.cpp
     ${PROJECT_SOURCE_DIR}/src/bitinputarchive.cpp
     ${PROJECT_SOURCE_DIR}/src/bititemsvector.cpp
     ${PROJECT_SOURCE_DIR}/src/bitoutputarchive.cpp
     ${PROJECT_SOURCE_DIR}/src/bitpropvariant.cpp
     ${PROJECT_SOURCE_DIR}/src/bittypes.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/bufferextractcallback.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/bufferitem.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/bufferutil.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/callback.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/cbufferinstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/cbufferoutstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/cfileinstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/cfileoutstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/cfixedbufferoutstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/cmultivolumeinstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/cmultivolumeoutstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/cstdinstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/cstdoutstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/csymlinkinstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/cvolumeinstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/cvolumeoutstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/dateutil.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/extractcallback.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/failuresourcecategory.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/fileextractcallback.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/fixedbufferextractcallback.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/formatdetect.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/fsindexer.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/fsitem.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/fsutil.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/genericinputitem.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/guids.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/hresultcategory.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/internalcategory.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/opencallback.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/operationcategory.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/operationresult.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/processeditem.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/renameditem.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/stdinputitem.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/streamextractcallback.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/stringutil.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/updatecallback.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/windows.cpp )

# library output file name options
include( cmake/OutputOptions.cmake )

# detecting if the std::filesystem is available and usable
include( cmake/FilesystemDetect.cmake )

# printing library and compiler information (useful for debugging)
message( STATUS "Target Version: ${CMAKE_PROJECT_VERSION}" )
message( STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}" )
message( STATUS "Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}" )
message( STATUS "Architecture: ${BIT7Z_TARGET_ARCH_NAME}" )
message( STATUS "Build Type: ${CMAKE_BUILD_TYPE}" )
message( STATUS "Language Standard for bit7z: C++${CMAKE_CXX_STANDARD}" )

# library target options
set( LIB_TARGET bit7z${ARCH_POSTFIX} )
add_library( ${LIB_TARGET} STATIC )
target_sources( ${LIB_TARGET}
                PUBLIC ${PUBLIC_HEADERS}
                PRIVATE ${HEADERS} ${SOURCES} )

# additional target without the architecture suffix in the name
if( NOT "${ARCH_POSTFIX}" STREQUAL "" )
    add_library( bit7z ALIAS ${LIB_TARGET} )
endif()

# bit7z build options
include( cmake/BuildOptions.cmake )

# compiler-specific options
include( cmake/CompilerOptions.cmake )

# dependencies
#include( cmake/Dependencies.cmake )


set(7-zip_SOURCE_DIR ${PROJECT_SOURCE_DIR}/third_party/7-zip)
message( STATUS "7-zip source code available at ${7-zip_SOURCE_DIR}" )
add_library( 7-zip INTERFACE IMPORTED )
target_include_directories( 7-zip INTERFACE "${7-zip_SOURCE_DIR}/CPP/" )
		
# 7-zip source code
target_link_libraries( ${LIB_TARGET} PRIVATE 7-zip )

# filesystem library (needed if std::filesystem is not available)
if( ghc_filesystem_ADDED )
    target_link_libraries( ${LIB_TARGET} PRIVATE ghc_filesystem )
endif()

# public includes
target_include_directories( ${LIB_TARGET} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
                                                 "$<INSTALL_INTERFACE:include>" )

# private includes
target_include_directories( ${LIB_TARGET} PRIVATE "${PROJECT_SOURCE_DIR}/include/bit7z"
                                                  "${PROJECT_SOURCE_DIR}/src"
						  "${PROJECT_SOURCE_DIR}/third_party/"
						  "${PROJECT_SOURCE_DIR}/third_party/7-zip/CPP/" )

# 7-zip compilation definitions
target_compile_definitions( ${LIB_TARGET} PRIVATE UNICODE _UNICODE )
if( WIN32 )
    target_compile_definitions( ${LIB_TARGET} PRIVATE _WINDOWS )
    target_link_libraries( ${LIB_TARGET} PUBLIC oleaut32 )
endif()

if( MINGW )
    target_link_libraries( ${LIB_TARGET} PUBLIC uuid )
endif()

if( UNIX )
    target_compile_definitions( ${LIB_TARGET} PRIVATE _LARGEFILE64_SOURCE _LARGEFILE_SOURCE _REENTRANT
                                                      EXTERNAL_CODECS ENV_UNIX BREAK_HANDLER USE_WIN_FILE )
    target_compile_definitions( ${LIB_TARGET} PUBLIC _TIME_BITS=64 )
    if ( ( NOT ANDROID ) OR ( ANDROID_PLATFORM GREATER_EQUAL 24 ) )
        target_compile_definitions( ${LIB_TARGET} PUBLIC _FILE_OFFSET_BITS=64 )
    endif()
    target_link_libraries( ${LIB_TARGET} PUBLIC ${CMAKE_DL_LIBS} )
endif()

# sanitizers
include( cmake/Sanitizers.cmake )

# tests
if( BIT7Z_BUILD_TESTS )
    enable_testing()
    add_subdirectory( tests )
endif()

# docs
if( BIT7Z_BUILD_DOCS )
    add_subdirectory( docs )
endif()

target_compile_definitions( ${LIB_TARGET} PRIVATE BIT7Z_AUTO_FORMAT BIT7Z_USE_NATIVE_STRING)
set_target_properties(${LIB_TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)
