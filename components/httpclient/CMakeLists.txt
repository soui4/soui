# 
# SOUI cmake配置文件
#

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

include_directories(${PROJECT_SOURCE_DIR}/config)
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
    target_link_libraries(httpclient utilities4)
else()
    add_library(httpclient STATIC ${srcs} ${headers})
endif()

set_target_properties (httpclient PROPERTIES
    FOLDER components
)
