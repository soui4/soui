# 
# SOUI cmake配置文件
#


include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)

set(SIpcObject_header
	stdafx.h
	SIpcObject.h
	ShareMemBuffer.h
)

set(SIpcObject_src 
	SIpcObject.cpp
	ShareMemBuffer.cpp
)

source_group("Header Files" FILES ${SIpcObject_header})
source_group("Source Files" FILES ${SIpcObject_src})

if (NOT ENABLE_SOUI_COM_LIB)
    add_definitions(-DBUILD_SIPC)
    set (SIpcObject_src  ${SIpcObject_src} SIpcObject.rc)
    add_library(sipcobject SHARED ${SIpcObject_src} ${SIpcObject_header})
    target_link_libraries(sipcobject utilities4)
else()
    add_definitions(-D_LIB)
    add_library(sipcobject STATIC ${SIpcObject_src} ${SIpcObject_header})
endif()

set(COM_LIBS ${COM_LIBS} sipcobject CACHE INTERNAL "com_lib")
set_target_properties (sipcobject PROPERTIES
    FOLDER components
)

install(TARGETS sipcobject
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS sipcobject
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
