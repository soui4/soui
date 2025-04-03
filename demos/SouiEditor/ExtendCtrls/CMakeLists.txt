#
# 1394020320@qq.com
#

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/soui-sys-resource)
include_directories(${PROJECT_SOURCE_DIR}/controls.extend)
include_directories(${PROJECT_SOURCE_DIR}/third-part/Scintilla/include)
include_directories(${PROJECT_SOURCE_DIR}/third-part/wke/include)
file(GLOB_RECURSE CTRLS_SRC ${PROJECT_SOURCE_DIR}/controls.extend/*.*)

set(CURRENT_SRCS
    SCtrlsRegister.cpp
)
 
set(CURRENT_HEADERS
    stdafx.h
    SCtrlsRegister.h
)
 
add_library(ExtendCtrls STATIC ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CTRLS_SRC})

if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
target_compile_options(ExtendCtrls PRIVATE -fPIC)
target_link_libraries(ExtendCtrls swinx)
endif()
target_precompile_headers(ExtendCtrls PRIVATE "stdafx.h")

set_target_properties (ExtendCtrls PROPERTIES
    FOLDER demos/uieditor
)

install(TARGETS ExtendCtrls
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS ExtendCtrls
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
