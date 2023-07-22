include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)

set(render-skia_header
	stdafx.h
	drawtext-skia.h
	render-skia.h
	skia2rop2.h
	patheffect-skia.h
	pathmeasure-skia.h
)
set(render-skia_src
	drawtext-skia.cpp
	render-skia.cpp
	skia2rop2.cpp
	patheffect-skia.cpp
	pathmeasure-skia.cpp
)

source_group("Header Files" FILES ${render-skia_header})
source_group("Source Files" FILES ${render-skia_src})

if (NOT ENABLE_SOUI_COM_LIB)
    set (render-skia_src  ${render-skia_src} render-skia.rc)
    add_library(render-skia SHARED ${render-skia_src} ${render-skia_header})
    target_link_libraries(render-skia skia utilities4 Usp10 opengl32)
else()
    add_library(render-skia STATIC ${render-skia_src} ${render-skia_header})
endif()

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

set(COM_LIBS ${COM_LIBS} render-skia)
set_target_properties (render-skia PROPERTIES
    FOLDER components/render
)
