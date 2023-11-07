add_definitions(-D_CRT_SECURE_NO_WARNINGS)

set(samplecode_header
SampleApp.h
tools/flags/SkCommandLineFlags.h
tools/flags/SkCommonFlags.h
utils/SamplePipeControllers.h
)
set(samplecode_src
SampleApp.cpp
TransitionView.cpp
OverView.cpp
SamplePictFile.cpp
tools/flags/SkCommandLineFlags.cpp
tools/flags/SkCommonFlags.cpp
tools/sk_tool_utils.cpp
tools/sk_tool_utils_font.cpp
tools/Resources.cpp
utils/SamplePipeControllers.cpp
views/SkWindow.cpp
views/SkWidgets.cpp
views/SkEvent.cpp
views/SkEventSink.cpp
views/SkView.cpp
views/SkViewPriv.cpp
views/SkViewInflate.cpp
views/SkTextBox.cpp
views/SkTagList.cpp
views/SkStackViewLayout.cpp
views/SkProgressView.cpp
views/SkOSMenu.cpp
views/SkTouchGesture.cpp
#views/SkParsePaint.cpp
views/win/skia_win.cpp
views/win/SkOSWindow_win.cpp
pipe/SkGPipeRead.cpp
pipe/SkGPipeWrite.cpp
xml/SkDOM.cpp
xml/SkXMLParser.cpp
xml/SkXMLParser_empty.cpp
gm/gm.cpp
ports/SkImageDecoder_empty.cpp
)

source_group("Header Files" FILES ${samplecode_header})
source_group("Source Files" FILES ${samplecode_src})



if(OUTPATH_WITHOUT_TYPE)
    link_directories(${PROJECT_BINARY_DIR}/bin)
else()
if (CMAKE_CL_64)
    if("${CMAKE_GENERATOR_PLATFORM}" MATCHES "ARM64")
    link_directories(${PROJECT_SOURCE_DIR}/bin64a)
    else()
    link_directories(${PROJECT_SOURCE_DIR}/bin64)
    endif()
else ()
    link_directories(${PROJECT_SOURCE_DIR}/bin)
endif ()
endif()
add_executable(samplecode WIN32 ${samplecode_src} ${samplecode_header})

target_include_directories(samplecode
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tools
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tools/flags
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/utils
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/views
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/gm
  PRIVATE ${PROJECT_SOURCE_DIR}/third-part/skia

  PRIVATE ${PROJECT_SOURCE_DIR}/third-part/skia/include
  PRIVATE ${PROJECT_SOURCE_DIR}/third-part/skia/include/config
  PRIVATE ${PROJECT_SOURCE_DIR}/third-part/skia/include/core
  PRIVATE ${PROJECT_SOURCE_DIR}/third-part/skia/include/pipe
  #PRIVATE ${PROJECT_SOURCE_DIR}/third-part/skia/include/pdf
  PRIVATE ${PROJECT_SOURCE_DIR}/third-part/skia/include/xml
  PRIVATE ${PROJECT_SOURCE_DIR}/third-part/skia/include/views
  PRIVATE ${PROJECT_SOURCE_DIR}/third-part/skia/include/effects
  PRIVATE ${PROJECT_SOURCE_DIR}/third-part/skia/include/images


  PRIVATE ${PROJECT_SOURCE_DIR}/third-part/skia/src/core

  PRIVATE ${PROJECT_SOURCE_DIR}/third-part/skia/include/gpu
  PRIVATE ${PROJECT_SOURCE_DIR}/third-part/skia/include/utils
  PRIVATE ${PROJECT_SOURCE_DIR}/third-part/skia/src/gpu
  PRIVATE ${PROJECT_SOURCE_DIR}/third-part/skia/src/utils
  PRIVATE ${PROJECT_SOURCE_DIR}/third-part/skia/src/fonts
)

if(CMAKE_DEBUG_POSTFIX)
    set_target_properties(samplecode PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
endif()

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
else()
	target_link_libraries(samplecode -municode)
endif()

add_dependencies(samplecode skia)
target_link_libraries(samplecode skia Usp10 opengl32)


set_target_properties (samplecode PROPERTIES
    FOLDER demos
)

