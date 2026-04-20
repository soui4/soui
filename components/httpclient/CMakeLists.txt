# 
# SOUI cmake配置文件
#

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)

set(headers
	define.h
	httpHeader.h
)

set(srcs 
	httpHeader.cpp
)

if (CMAKE_SYSTEM_NAME MATCHES Windows)
    set(enable_http ON)
    set(headers
        ${headers}
        win/winhttp.h
        win/HttpClient.h
    )

    set(srcs 
        ${srcs}
        win/HttpClient.cpp
    )
else()
    message(STATUS "building with httpclient using libcurl")
    if(NOT TARGET libcurl_static)
    message(FATAL_ERROR "curl not found. Please build curl at first")
    endif()
    get_target_property(CURL_INCLUDE_DIRS libcurl_static INTERFACE_INCLUDE_DIRECTORIES)
    message(STATUS "building with httpclient using libcurl ${CURL_INCLUDE_DIRS}")
    set(CURL_LIBRARIES libcurl_static)
    
    set(headers
            ${headers}
            linux/HttpClient.h
        )

        set(srcs 
            ${srcs}
            linux/HttpClient.cpp
        )
endif()


source_group("Header Files" FILES ${headers})
source_group("Source Files" FILES ${srcs})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if (NOT SOUI_ENABLE_COM_LIB)
    if (CMAKE_SYSTEM_NAME MATCHES Windows)
        set (srcs  ${srcs} httpclient.rc)
        add_library(httpclient SHARED ${srcs} ${headers})
        target_link_libraries(httpclient utilities4 Winhttp.lib)
    else()
        add_library(httpclient SHARED ${srcs} ${headers})
        target_link_libraries(httpclient utilities4 ${CURL_LIBRARIES})
    endif()
else()
    add_library(httpclient STATIC ${srcs} ${headers})
    target_link_libraries(httpclient PUBLIC utilities4 ${CURL_LIBRARIES})
endif()

set(COM_LIBS ${COM_LIBS} httpclient CACHE INTERNAL "com_lib")
if(SOUI_ENABLE_COM_LIB)
    if (CMAKE_SYSTEM_NAME MATCHES Windows)
        set(COM_DEP_LIBS ${COM_DEP_LIBS} Winhttp.lib CACHE INTERNAL "com_dep_libs")
    else()
        set(COM_DEP_LIBS ${COM_DEP_LIBS} curl ssl_1_1 crypto_1_1 CACHE INTERNAL "com_dep_libs")
    endif()
endif()

set_target_properties (httpclient PROPERTIES
    FOLDER components
)

install(TARGETS httpclient
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS httpclient
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
