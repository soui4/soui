add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if (CMAKE_SYSTEM_NAME MATCHES Windows)
set(render-gdi_header
	stdafx.h
	GradientFillHelper.h
	win/render-gdi.h
)

set(render-gdi_src
    GradientFillHelper.cpp
    win/render-gdi.cpp
)
else()
set(render-gdi_header
	stdafx.h
	GradientFillHelper.h
	linux/render-gdi.h
)

set(render-gdi_src
    GradientFillHelper.cpp
    linux/render-gdi.cpp
)
endif()

source_group("Header Files" FILES ${render-gdi_header})
source_group("Source Files" FILES ${render-gdi_src})

if (NOT ENABLE_SOUI_COM_LIB)
    set (render-gdi_src  ${render-gdi_src} render-gdi.rc)
    add_library(render-gdi SHARED ${render-gdi_src} ${render-gdi_header})
    if (CMAKE_SYSTEM_NAME MATCHES Windows)
    	target_link_libraries(render-gdi utilities4 msimg32)
    else()
        target_link_libraries(render-gdi utilities4 swinx)
    endif()

else()
    add_library(render-gdi STATIC ${render-gdi_src} ${render-gdi_header})
endif()

target_compile_definitions(render-gdi
  PUBLIC SOUI_RENDER_USE_GDI
)
target_include_directories(render-gdi
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${PROJECT_SOURCE_DIR}/utilities/include
  PRIVATE ${PROJECT_SOURCE_DIR}/SOUI/include
)

set(COM_LIBS ${COM_LIBS} render-gdi CACHE INTERNAL "com_lib")
set_target_properties (render-gdi PROPERTIES
    FOLDER components/render
)

install(TARGETS render-gdi
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS render-gdi
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
