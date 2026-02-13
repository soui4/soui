add_definitions(-D_CRT_SECURE_NO_WARNINGS)
#include(${PROJECT_SOURCE_DIR}/__cmake/global.cmake)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/third-part/wke/include)
include_directories(${PROJECT_SOURCE_DIR}/third-part/Detours/include)
include_directories(${PROJECT_SOURCE_DIR}/third-part/nanosvg/src)
include_directories(${PROJECT_SOURCE_DIR}/third-part/stb)

include_directories(${PROJECT_SOURCE_DIR}/soui-sys-resource)
if (NOT CMAKE_SYSTEM_NAME MATCHES Windows)
include_directories(${PROJECT_SOURCE_DIR}/swinx/include)
include_directories(${PROJECT_SOURCE_DIR}/third-part/richedit41/inc41)
endif()
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE CURRENT_HEADERS *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

if (NOT CMAKE_SYSTEM_NAME MATCHES Windows)
# 添加 -fpermissive 选项到 C++ 编译器标志
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive" )
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpermissive")
endif()


source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resource" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})


set(MACOSX_BUNDLE_HIGH_RESOLUTION_CAPABLE "true")
set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.soui.demo")
set(MACOSX_BUNDLE_BUNDLE_NAME "demo")

add_soui_exe(demo ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC} )
if(CMAKE_SYSTEM_NAME MATCHES Darwin)
# 配置代码签名
set(MACOSX_CODESIGN_IDENTITY "Apple Development")
set(MACOSX_TEAM_ID "XXXXXXXXXXXX")

# 设置 Xcode 特定的签名属性
set_target_properties(demo PROPERTIES
    XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${MACOSX_TEAM_ID}"
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${MACOSX_CODESIGN_IDENTITY}"
    XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
    XCODE_ATTRIBUTE_PROVISIONING_PROFILE_SPECIFIER ""
)
endif()

# 准备要复制的目标列表
set(DEMO_TARGET_LIST
	cairo
    swinx
    soui4
	msftedit
    utilities4
	imgdecoder-stb
	resprovider-zip
	resprovider-7zip
	log4z
	taskloop
	translator
	scriptmodule-lua
	render-gdi
	render-skia
)
copy_targets_to_bundle(demo "${DEMO_TARGET_LIST}")

add_macos_res_folder(demo ${PROJECT_SOURCE_DIR}/soui-sys-resource soui-sys-resource)
add_macos_res_folder(demo ${CMAKE_CURRENT_SOURCE_DIR}/uires uires)
add_macos_res_file(demo ${PROJECT_SOURCE_DIR}/simsun.ttc fonts)
set_macos_icon(demo ${CMAKE_CURRENT_SOURCE_DIR}/demo.icns ${PROJECT_SOURCE_DIR}/__cmake/plist.in)
add_dependencies(demo resprovider-zip)
if(CMAKE_DEBUG_POSTFIX)
    set_target_properties(demo PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
endif()

add_dependencies(demo ${COM_LIBS})
target_link_libraries(demo soui4 utilities4 ExtendCtrls)
if (CMAKE_SYSTEM_NAME MATCHES Windows)
	add_dependencies(demo soui-sys-resource)
if(MSVC)
    set(DPI_AWARE_MANIFEST "${PROJECT_SOURCE_DIR}/__cmake/dpi_aware.manifest")
    set_target_properties(${current_project} PROPERTIES
            LINK_FLAGS "/MANIFEST:EMBED /MANIFESTINPUT:${DPI_AWARE_MANIFEST}"
	)

	if("${CMAKE_GENERATOR_PLATFORM}" MATCHES "ARM64" OR "${CMAKE_GENERATOR_PLATFORM}" MATCHES "ARM")
	target_link_libraries(demo detours)
	else()
	target_link_libraries(demo detours smiley)
	endif()
else(MSVC)
	target_link_libraries(demo Winmm Msimg32 gdiplus)
endif(MSVC)

else()
	target_link_libraries(demo dl)
if(SOUI_ENABLE_COM_LIB)
    target_link_libraries(demo ${COM_LIBS})
endif()
endif()
target_precompile_headers(demo PRIVATE "stdafx.h")

set_target_properties (demo PROPERTIES
    FOLDER demos
)
