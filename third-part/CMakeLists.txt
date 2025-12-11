add_subdirectory(zlib)
add_subdirectory(lua-54)
add_subdirectory(sqlite3)
add_subdirectory(pbl_aupng)
add_subdirectory(jsoncpp)
add_subdirectory(gtest)
if (CMAKE_SYSTEM_NAME MATCHES Windows)
        #message(STATUS "cmake compiler id: ${CMAKE_C_COMPILER_ID}")
        if(CMAKE_C_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")
                message(STATUS "Detected MinGW")
        else()
                add_subdirectory(Detours)
                add_subdirectory(smiley)
        endif()
endif()
if(SOUI_BUILD_RICHEDIT)
        message("---Building with richedit41")
        add_subdirectory(richedit41)
endif()

add_subdirectory(Scintilla)
add_subdirectory(skia)
add_subdirectory(7z)
add_subdirectory(stdint)
if(SOUI_BUILD_WS)
        set(ENABLE_WS ON)
        if(CMAKE_SYSTEM_NAME MATCHES "Windows")
                #disable build for windows.arm
                if(CMAKE_GENERATOR_PLATFORM MATCHES "ARM" OR CMAKE_GENERATOR_PLATFORM MATCHES "ARM64")
                        message(STATUS "disable build for windows.arm")
                        set(ENABLE_WS OFF)
                endif()
                if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                        set(platform "x64")
                else()
                        set(platform "x86")
                endif()
                set(LWS_OPENSSL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/openssl_1.1.1d/${platform}/include
                        CACHE PATH "Path to the OpenSSL include directory" FORCE
                        )
                if(MSVC)
                        set(LWS_OPENSSL_LIBRARIES 
                                "${CMAKE_CURRENT_SOURCE_DIR}/openssl_1.1.1d/${platform}/lib/$(Configuration)/libssl.lib"
                                "${CMAKE_CURRENT_SOURCE_DIR}/openssl_1.1.1d/${platform}/lib/$(Configuration)/libcrypto.lib"
                                CACHE PATH "Path to the OpenSSL library" FORCE
                        )
                else()
                        set(LWS_OPENSSL_LIBRARIES 
                                "${CMAKE_CURRENT_SOURCE_DIR}/openssl_1.1.1d/${platform}/lib/mingw/libssl.a"
                                "${CMAKE_CURRENT_SOURCE_DIR}/openssl_1.1.1d/${platform}/lib/mingw/libcrypto.a"
                                CACHE PATH "Path to the OpenSSL library" FORCE
                        )
                endif()
        endif()
        if(ENABLE_WS)
                set(LWS_WITH_MINIMAL_EXAMPLES OFF)
                set(LWS_WITHOUT_TESTAPPS ON)
                set(LWS_WITH_STATIC ON)
                set(LWS_WITH_SHARED OFF)
                set(LWS_STATIC_PIC ON)
                message("---Building with libwebsockets")
                add_subdirectory(libwebsockets)
        endif(ENABLE_WS)
endif()