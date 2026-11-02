add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# wasm 下没有 third-part/zlib：那份 zlib 和 swinx/thirdparty 里的 zlib_static 是同一批
# 符号的两份拷贝，全链进一个模块会 duplicate symbol。这里改吃 swinx 的那一份
# （zlib_static 的 include 目录是 PUBLIC 的，链接它就能拿到 zlib.h）。
if(EMSCRIPTEN)
    include_directories(${SOUI_ROOT_DIR}/utilities/include)
    include_directories(${SOUI_ROOT_DIR}/SOUI/include)
else()
    include_directories(${SOUI_ROOT_DIR}/third-part/zlib)
    include_directories(${SOUI_ROOT_DIR}/utilities/include)
    include_directories(${SOUI_ROOT_DIR}/SOUI/include)
endif()

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

if(EMSCRIPTEN)
set(LIBS utilities4 zlib_static)
else()
set(LIBS utilities4 zlib)
endif()
if(NOT CMAKE_SYSTEM_NAME MATCHES Windows)
include_directories(${CMAKE_SOURCE_DIR}swinx/include)
set(LIBS ${LIBS} swinx)
endif()
if (NOT SOUI_ENABLE_COM_LIB)
    set (resprovider-zip_src  ${resprovider-zip_src} resprovider-zip.rc)
    add_library(resprovider-zip SHARED ${resprovider-zip_src} ${resprovider-zip_header})
    target_link_libraries(resprovider-zip ${LIBS})
else()
    add_library(resprovider-zip STATIC ${resprovider-zip_src} ${resprovider-zip_header})
endif()

add_dependencies(resprovider-zip ${LIBS})

if(EMSCRIPTEN)
    # zlib_static 的头文件目录挂在它的 PUBLIC usage requirements 上，而
    # add_dependencies 只保证构建顺序、不把 usage requirements 带进编译：zipArchive.cpp
    # 的 #include <zconf.h> 会直接找不到。显式链接一次，编译期就能拿到 zlib.h/zconf.h。
    target_link_libraries(resprovider-zip PUBLIC zlib_static)
endif()

set(COM_LIBS ${COM_LIBS} resprovider-zip CACHE INTERNAL "com_lib")
if(SOUI_ENABLE_COM_LIB)
    if(EMSCRIPTEN)
        set(COM_DEP_LIBS ${COM_DEP_LIBS} zlib_static CACHE INTERNAL "com_dep_libs")
    else()
        set(COM_DEP_LIBS ${COM_DEP_LIBS} zlib CACHE INTERNAL "com_dep_libs")
    endif()
endif()

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

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/zipresprovider-param.h"
        DESTINATION include/components/resprovider-zip)