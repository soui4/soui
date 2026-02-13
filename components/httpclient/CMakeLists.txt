# 
# SOUI cmake配置文件
#

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)

set(headers
	define.h
	Encoder.h
	httpHeader.h
)

set(srcs 
	Encoder.cpp
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
    find_package(CURL)
    # 检查是否找到 libcurl
    if(CURL_FOUND)
        set(enable_http ON)
        message(STATUS "building with httpclient using libcurl ${CURL_INCLUDE_DIRS}")
        include_directories(${CURL_INCLUDE_DIRS})

        set(headers
            ${headers}
            linux/HttpClient.h
        )

        set(srcs 
            ${srcs}
            linux/HttpClient.cpp
        )
    else()
        message(STATUS "libcurl not found. Disabling httpclient")
        set(enable_http OFF)
    endif()
endif()

if(enable_http)

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

endif(enable_http)