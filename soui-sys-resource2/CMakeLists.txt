# 
# SOUI cmake配置文件
#

include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(include)

set(RCS_MAIN soui-sys-resource.rc)
set(HDRS_MAIN resource.h)

source_group("Header Files" FILES ${HDRS_MAIN})
source_group("Resource Files" FILES ${RCS_MAIN})

if (NOT ENABLE_SOUI_CORE_LIB)
    add_library(soui-sys-resource2 SHARED  ${HDRS_MAIN} ${RCS_MAIN})
else()
    add_library(soui-sys-resource2 STATIC  ${HDRS_MAIN} ${RCS_MAIN})
endif()

set_target_properties(soui-sys-resource2 PROPERTIES LINK_FLAGS "/NOENTRY")
