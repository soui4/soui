include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)

set(render-skia_header
	stdafx.h
	drawtext-skia.h
	render-skia.h
	skia2rop2.h
	PathEffect-Skia.h
)
set(render-skia_src
	drawtext-skia.cpp
	render-skia.cpp
	skia2rop2.cpp
	PathEffect-Skia.cpp
)

source_group("Header Files" FILES ${render-skia_header})
source_group("Source Files" FILES ${render-skia_src})

if (NOT ENABLE_SOUI_COM_LIB)
    set (render-skia_src  ${render-skia_src} render-skia.rc)
    add_library(render-skia SHARED ${render-skia_src} ${render-skia_header})
    add_dependencies(render-skia skia utilities4)
    if (CMAKE_SYSTEM_NAME MATCHES Windows)
        target_link_libraries(render-skia skia utilities4 Usp10 opengl32 soui4)
    else()
        find_package(Freetype REQUIRED)
        add_dependencies(render-skia swinx)
        target_link_libraries(render-skia swinx skia ${FREETYPE_LIBRARIES} utilities4 soui4 GL)
    endif()
else()
    add_library(render-skia STATIC ${render-skia_src} ${render-skia_header})
endif()

target_compile_definitions(render-skia
  PUBLIC SOUI_RENDER_USE_SKIA
)
target_include_directories(render-skia
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${PROJECT_SOURCE_DIR}/config
  PRIVATE ${PROJECT_SOURCE_DIR}/utilities/include
  PRIVATE ${PROJECT_SOURCE_DIR}/SOUI/include
  PRIVATE ${PROJECT_SOURCE_DIR}/third-part/skia
  PRIVATE ${PROJECT_SOURCE_DIR}/third-part/skia/include
  PRIVATE ${PROJECT_SOURCE_DIR}/third-part/skia/include/config
  PRIVATE ${PROJECT_SOURCE_DIR}/third-part/skia/include/core
  PRIVATE ${PROJECT_SOURCE_DIR}/third-part/skia/src/core
)

set(COM_LIBS ${COM_LIBS} render-skia CACHE INTERNAL "com_lib")
set_target_properties (render-skia PROPERTIES
    FOLDER components/render
)

install(TARGETS render-skia
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS render-skia
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
