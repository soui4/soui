add_definitions(-D_CRT_SECURE_NO_WARNINGS)

set(render-cairo_header
	stdafx.h
	render-cairo.h
)

set(render-cairo_src
    render-cairo.cpp
)

source_group("Header Files" FILES ${render-cairo_header})
source_group("Source Files" FILES ${render-cairo_src})

if (NOT ENABLE_SOUI_COM_LIB)
    set (render-cairo_src  ${render-cairo_src} render-cairo.rc)
    add_library(render-cairo SHARED ${render-cairo_src} ${render-cairo_header})
    target_link_libraries(render-cairo utilities4 platform)
else()
    add_library(render-cairo STATIC ${render-cairo_src} ${render-cairo_header})
endif()

target_compile_definitions(render-cairo
  PUBLIC SOUI_RENDER_USE_cairo
)
target_include_directories(render-cairo
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${PROJECT_SOURCE_DIR}/platform/include
  PRIVATE ${PROJECT_SOURCE_DIR}/utilities/include
  PRIVATE ${PROJECT_SOURCE_DIR}/SOUI/include
)

set(COM_LIBS ${COM_LIBS} render-cairo CACHE INTERNAL "com_lib")
set_target_properties (render-cairo PROPERTIES
    FOLDER components/render
)

install(TARGETS render-cairo
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS render-cairo
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
