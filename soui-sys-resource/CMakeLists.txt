# 
# SOUI cmake配置文件
#

include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(include)

set(RCS_MAIN soui-sys-resource.rc)
set(SRC_MAIN resource.h null.cpp)

source_group("Header Files" FILES ${SRC_MAIN})
source_group("Resource Files" FILES ${RCS_MAIN})

add_library(soui-sys-resource SHARED  ${SRC_MAIN} ${RCS_MAIN})
if(MSVC)
    string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    set_target_properties(soui-sys-resource PROPERTIES LINK_FLAGS "/NOENTRY") 
else()
    set_target_properties(soui-sys-resource PROPERTIES PREFIX "")     
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
