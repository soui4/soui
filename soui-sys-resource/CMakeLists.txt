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
    add_library(soui-sys-resource SHARED  ${HDRS_MAIN} ${RCS_MAIN})
else()
    add_library(soui-sys-resource STATIC  ${HDRS_MAIN} ${RCS_MAIN})
endif()
if(MSVC)
    set_target_properties(soui-sys-resource PROPERTIES LINK_FLAGS "/NOENTRY")
endif()

install(TARGETS soui-sys-resource
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS soui-sys-resource
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
