if(SOUI_WASM_MINIMAL)
    # wasm 最小构建：只拉起 components/imgdecoder-stb 需要的 aupng。
    #
    # 这里刻意不加 zlib：soui4/third-part/zlib 与 swinx/thirdparty/zlib（目标名
    # zlib_static）是同一批 inflate/deflate 符号的两份拷贝。桌面构建里两者分处不同
    # .so，靠符号插桩互不干扰；wasm 把所有东西链进同一个模块，wasm-ld 会直接报
    # duplicate symbol。所以依赖 zlib 的组件（resprovider-zip）在 wasm 下不进构建图，
    # 资源改用 SOUI 核心的 SResProviderFiles（见 doc/soui-wasm-port-plan.md）。
    # stb 头文件是 header-only，由 imgdecoder-stb 直接用路径引用，无需子目录。
    add_subdirectory(pbl_aupng)
    # richedit41：SRichEdit 在 wasm 上没有 dlopen，只能静态链接 msftedit，
    # 由 SOUI/CMakeLists.txt 的 INIT_RICHEDIT 分支把它挂到 soui4 上。
    add_subdirectory(richedit41)
    # Scintilla 进 wasm：controls.extend 的 SScintillaView 走它的 headless 接口
    # （Scintilla_CreateHeadless + Scintilla_PaintHeadless），绘制落在 SOUI 的
    # render target 上，不需要真窗口。异常处理按 Scintilla/CMakeLists.txt 的
    # EMSCRIPTEN 分支关掉。
    # SOUI_BUILD_WASM_SCINTILLA=OFF 时整条依赖链（Scintilla 库、SScintillaView.cpp、
    # 注册入口、<scintilla> 标签）一起退出构建图。
    if(SOUI_BUILD_WASM_SCINTILLA)
    add_subdirectory(Scintilla)
    endif()
    # lua 5.4 进 wasm：ScriptModule-LUA 是纯 C + C++ 的静态库，不依赖 dlopen。
    # 只编 C 源，setjmp/longjmp 在 wasm 上由 libc 提供，与桌面同一套语义。
    # 跟 components/ScriptModule-LUA、soui-wasm 的 uires/lua 预载由同一个开关
    # SOUI_BUILD_WASM_LUA 一起进退。
    if(SOUI_BUILD_WASM_LUA)
    add_subdirectory(lua-54)
    endif()
    return()
endif()

add_subdirectory(zlib)
add_subdirectory(lua-54)
if (NOT CMAKE_SYSTEM_NAME MATCHES Android)
add_subdirectory(sqlite3)
add_subdirectory(7z)
endif()
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
add_subdirectory(stdint)
if(SOUI_BUILD_WS OR (SOUI_BUILD_HTTPCLIENT AND NOT CMAKE_SYSTEM_NAME MATCHES Windows))
        add_subdirectory(openssl-cmake)
        message(STATUS "OpenSSL include directories: ${OPENSSL_INCLUDE_DIRS}")
        set(OPENSSL_LIBRARIES crypto ssl)
endif()
if(SOUI_BUILD_HTTPCLIENT AND NOT CMAKE_SYSTEM_NAME MATCHES Windows)
        message("---Building with libcurl for HTTP client")
        
        # Disable debug postfix to avoid "-d" suffix on debug builds
        set(CMAKE_DEBUG_POSTFIX "" CACHE STRING "Disable Debug postfix for library names" FORCE)
        
        set(BUILD_CURL_EXE OFF CACHE BOOL "Build curl executable" FORCE)
        set(BUILD_TESTING OFF CACHE BOOL "Build curl tests" FORCE)
        set(BUILD_EXAMPLES OFF CACHE BOOL "Build libcurl examples" FORCE)
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
        set(BUILD_STATIC_LIBS ON CACHE BOOL "Build static libraries" FORCE)
        set(CURL_STATICLIB ON CACHE BOOL "Build static libcurl" FORCE)
        set(HTTP_ONLY ON CACHE BOOL "Only build HTTP protocol support" FORCE)
        set(ENABLE_THREADED_RESOLVER ON CACHE BOOL "Enable threaded resolver" FORCE)
        
        # Enable OpenSSL support using locally compiled openssl-cmake
        get_target_property(OPENSSL_INCLUDE_DIRS crypto INTERFACE_INCLUDE_DIRECTORIES)
        set(CMAKE_USE_OPENSSL ON CACHE BOOL "Use OpenSSL" FORCE)
        
        # Ensure libcurl uses our local OpenSSL
        set(OPENSSL_FOUND TRUE CACHE BOOL "OpenSSL found" FORCE)
        set(OPENSSL_INCLUDE_DIR ${OPENSSL_INCLUDE_DIRS} CACHE PATH "OpenSSL include directory" FORCE)
        set(OPENSSL_CRYPTO_LIBRARY crypto CACHE STRING "OpenSSL crypto library" FORCE)
        set(OPENSSL_SSL_LIBRARY ssl CACHE STRING "OpenSSL SSL library" FORCE)
        set(OPENSSL_LIBRARIES crypto;ssl CACHE STRING "OpenSSL libraries" FORCE)
        
        # Set prefix path so check_symbol_exists can find OpenSSL targets
        list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/third-part/openssl-cmake")
        
        # Disable all optional dependencies except OpenSSL
        set(CURL_USE_LIBPSL OFF CACHE BOOL "Use libpsl" FORCE)
        set(CURL_ENABLE_EXPORT_TARGET OFF CACHE BOOL "Enable CMake export target" FORCE)
        set(CURL_USE_LIBSSH2 OFF CACHE BOOL "Use libssh2" FORCE)
        set(CURL_USE_LIBSSH OFF CACHE BOOL "Use libssh" FORCE)
        set(USE_LIBIDN2 OFF CACHE BOOL "Use libidn2" FORCE)
        set(CURL_USE_GSSAPI OFF CACHE BOOL "Use GSSAPI implementation" FORCE)
        set(CURL_USE_GSASL OFF CACHE BOOL "Use libgsasl" FORCE)
        set(USE_NGHTTP2 OFF CACHE BOOL "Use nghttp2 library" FORCE)
        set(USE_NGTCP2 OFF CACHE BOOL "Use ngtcp2 for HTTP/3" FORCE)
        set(USE_QUICHE OFF CACHE BOOL "Use quiche for HTTP/3" FORCE)
        set(CURL_ZLIB OFF CACHE BOOL "Use zlib" FORCE)
        set(CURL_BROTLI OFF CACHE BOOL "Use brotli" FORCE)
        set(CURL_ZSTD OFF CACHE BOOL "Use zstd" FORCE)
        set(ENABLE_ARES OFF CACHE BOOL "Enable c-ares support" FORCE)
        set(CURL_USE_LIBBACKTRACE OFF CACHE BOOL "Use libbacktrace" FORCE)
        set(CURL_USE_LIBUV OFF CACHE BOOL "Use libuv" FORCE)
        set(USE_WIN32_IDN OFF CACHE BOOL "Use WinIDN for IDN support" FORCE)
        set(USE_APPLE_IDN OFF CACHE BOOL "Use Apple built-in IDN support" FORCE)
        set(CURL_WINDOWS_SSPI OFF CACHE BOOL "Enable SSPI on Windows" FORCE)
        set(CURL_USE_MBEDTLS OFF CACHE BOOL "Enable mbedTLS for SSL/TLS" FORCE)
        set(CURL_USE_WOLFSSL OFF CACHE BOOL "Enable wolfSSL for SSL/TLS" FORCE)
        set(CURL_USE_GNUTLS OFF CACHE BOOL "Enable GnuTLS for SSL/TLS" FORCE)
        set(CURL_USE_RUSTLS OFF CACHE BOOL "Enable Rustls for SSL/TLS" FORCE)
        set(CURL_USE_SCHANNEL OFF CACHE BOOL "Enable Windows native SSL/TLS (Schannel)" FORCE)
        
        add_subdirectory(libcurl)
endif()
if(SOUI_BUILD_WS)
        get_target_property(OPENSSL_INCLUDE_DIRS crypto INTERFACE_INCLUDE_DIRECTORIES)

        set(LWS_OPENSSL_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIRS} CACHE PATH "Path to the OpenSSL include directory" FORCE)
        set(LWS_OPENSSL_LIBRARIES ${OPENSSL_LIBRARIES} CACHE PATH "Path to the OpenSSL library" FORCE)

        set(LWS_WITH_MINIMAL_EXAMPLES OFF)
        set(LWS_WITHOUT_TESTAPPS ON)
        set(LWS_WITH_STATIC ON)
        set(LWS_WITH_SHARED OFF)
        set(LWS_STATIC_PIC ON)
        if(MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            # Unlike cl.exe (which merely ignores -Werror), clang-cl enforces it, and this
            # vendored libwebsockets snapshot trips -Wsign-compare in
            # lib/roles/http/header.c.  Keep third-party warnings from becoming build
            # failures; this only affects the clang-cl toolset.
            set(DISABLE_WERROR ON CACHE BOOL "Avoid treating compiler warnings as fatal errors" FORCE)
        endif()
        message("---Building with libwebsockets")
        add_subdirectory(libwebsockets)
endif()
