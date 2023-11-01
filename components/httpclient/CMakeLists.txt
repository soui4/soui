# 
# SOUI cmake配置文件
#

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)

set(headers
	define.h
	Encoder.h
	HttpClient.h
	httpHeader.h
	winhttp.h
)

set(srcs 
	Encoder.cpp
	HttpClient.cpp
	httpHeader.cpp
)

source_group("Header Files" FILES ${headers})
source_group("Source Files" FILES ${srcs})

if (NOT ENABLE_SOUI_COM_LIB)
    set (srcs  ${srcs} httpclient.rc)
    add_library(httpclient SHARED ${srcs} ${headers})
    target_link_libraries(httpclient utilities4 Winhttp.lib)
else()
    add_library(httpclient STATIC ${srcs} ${headers})
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
