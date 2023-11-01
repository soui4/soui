add_definitions(-D_CRT_SECURE_NO_WARNINGS)

set(header
	stdafx.h
	render-d2d.h
)

set(src
    render-d2d.cpp
)

source_group("Header Files" FILES ${header})
source_group("Source Files" FILES ${src})

if (NOT ENABLE_SOUI_COM_LIB)
    set (src  ${src} render-d2d.rc)
    add_library(render-d2d SHARED ${src} ${header})
    target_link_libraries(render-d2d utilities4 d2d1.lib dwrite.lib)
else()
    add_library(render-d2d STATIC ${src} ${header})
endif()

target_compile_definitions(render-d2d
  PUBLIC SOUI_RENDER_USE_D2D
)
target_include_directories(render-d2d
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${PROJECT_SOURCE_DIR}/utilities/include
  PRIVATE ${PROJECT_SOURCE_DIR}/SOUI/include
)

if(MSVC_VERSION LESS_EQUAL 1500)
message("vs is less than vs2008")
target_include_directories(render-d2d
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/sdk7.1_d2d/include
)
if (CMAKE_CL_64)
	target_link_directories(render-d2d PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/sdk7.1_d2d/lib/x64")
else ()
	target_link_directories(render-d2d PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/sdk7.1_d2d/lib/x86")
endif ()
endif()


    
set(COM_LIBS ${COM_LIBS} render-d2d CACHE INTERNAL "com_lib")
set_target_properties (render-d2d PROPERTIES
    FOLDER components/render
)

install(TARGETS render-d2d
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS render-d2d
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
