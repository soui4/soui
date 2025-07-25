# 
# SOUI cmake配置文件
#

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)

set(TaskLoop_header
	TaskLoop.h
	thread.h
)

set(TaskLoop_src 
	TaskLoop.cpp
	thread.cpp
)

source_group("Header Files" FILES ${TaskLoop_header})
source_group("Source Files" FILES ${TaskLoop_src})

if (NOT ENABLE_SOUI_COM_LIB)
    set (TaskLoop_src  ${TaskLoop_src} TaskLoop.rc)
    add_library(taskloop SHARED ${TaskLoop_src} ${TaskLoop_header})
    target_link_libraries(taskloop utilities4)
    if(WIN32)
        target_link_libraries(taskloop Winmm)
    endif()
else()
    add_library(taskloop STATIC ${TaskLoop_src} ${TaskLoop_header})
endif()

set(COM_LIBS ${COM_LIBS} taskloop CACHE INTERNAL "com_lib")
set_target_properties (taskloop PROPERTIES
    FOLDER components
)

install(TARGETS taskloop
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS taskloop
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
