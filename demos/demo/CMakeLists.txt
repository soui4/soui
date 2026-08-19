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
include_directories(${PROJECT_SOURCE_DIR}/controls.extend)

include_directories(${PROJECT_SOURCE_DIR}/soui-sys-resource)
if (NOT CMAKE_SYSTEM_NAME MATCHES Windows)
include_directories(${PROJECT_SOURCE_DIR}/swinx/include)
include_directories(${PROJECT_SOURCE_DIR}/third-part/richedit41/inc41)
endif()
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE CURRENT_HEADERS *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS  uires/*.*)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

set(OHOS_DEMO_ENTRY ${CMAKE_CURRENT_SOURCE_DIR}/ohos_demo_entry.cpp)
if(IS_OHOS)
    list(REMOVE_ITEM CURRENT_SRCS ${OHOS_DEMO_ENTRY})
endif()

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
# Compile resource files with windres on Linux
target_compile_resources(demo ${CURRENT_RC} INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/soui-sys-resource)

if(APPLE)
set(MACOSX_CODESIGN_IDENTITY "Apple Development")
set(MACOSX_TEAM_ID 2YL33QR3KY)#replace with your team id

# 设置 Xcode 特定的签名属性
set_target_properties(demo PROPERTIES
    XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${MACOSX_TEAM_ID}
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ${MACOSX_CODESIGN_IDENTITY}
    XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
    XCODE_ATTRIBUTE_PROVISIONING_PROFILE_SPECIFIER ""
)

# 准备要复制的目标列表
set(DEMO_TARGET_LIST
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
#add_macos_res_file(demo ${PROJECT_SOURCE_DIR}/simsun.ttc fonts)
set_macos_icon(demo ${CMAKE_CURRENT_SOURCE_DIR}/demo.icns ${PROJECT_SOURCE_DIR}/__cmake/plist.in)
else()
add_app_res_folder(demo ${CMAKE_CURRENT_SOURCE_DIR}/uires demo_res/uires)
endif(APPLE)
add_dependencies(demo resprovider-zip)
if(CMAKE_DEBUG_POSTFIX)
    set_target_properties(demo PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
endif()

add_dependencies(demo ${COM_LIBS})
target_link_libraries(demo soui4 utilities4 ExtendCtrls)
if (CMAKE_SYSTEM_NAME MATCHES Windows)
	target_link_libraries(demo render-gdi render-skia render-d2d)
	add_dependencies(demo soui-sys-resource)
	if(MSVC)
	    if(MSVC_VERSION GREATER_EQUAL 1900)
			set(DPI_AWARE_MANIFEST "${PROJECT_SOURCE_DIR}/__cmake/dpi_aware.manifest")
			set_target_properties(demo PROPERTIES
					LINK_FLAGS "/MANIFEST:EMBED /MANIFESTINPUT:${DPI_AWARE_MANIFEST}"
				)
    	endif()
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
endif()
if(SOUI_ENABLE_COM_LIB)
    target_link_libraries(demo ${COM_LIBS} ${COM_DEP_LIBS})
endif()
target_precompile_headers(demo PRIVATE "stdafx.h")

set_target_properties (demo PROPERTIES
    FOLDER demos
)

if(IS_OHOS)
    set(OHOS_DEMO_SRCS ${CURRENT_SRCS})
    list(REMOVE_ITEM OHOS_DEMO_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/demo.cpp)

    add_library(souidemo SHARED ${CURRENT_HEADERS} ${OHOS_DEMO_SRCS} ${OHOS_DEMO_ENTRY})
    target_include_directories(souidemo PRIVATE
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/components
        ${PROJECT_SOURCE_DIR}/utilities/include
        ${PROJECT_SOURCE_DIR}/SOUI/include
        ${PROJECT_SOURCE_DIR}/swinx/include
        ${PROJECT_SOURCE_DIR}/soui-sys-resource
        ${PROJECT_SOURCE_DIR}/controls.extend
        ${PROJECT_SOURCE_DIR}/third-part/wke/include
        ${PROJECT_SOURCE_DIR}/third-part/Detours/include
        ${PROJECT_SOURCE_DIR}/third-part/nanosvg/src
        ${PROJECT_SOURCE_DIR}/third-part/stb
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_link_libraries(souidemo PRIVATE soui4 utilities4 ExtendCtrls swinx dl)
    target_compile_resources(souidemo ${CURRENT_RC} INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/soui-sys-resource)
    add_dependencies(souidemo imgdecoder-stb render-skia)
    target_link_libraries(souidemo PRIVATE render-skia)
    if(SOUI_BUILD_RICHEDIT)
        add_dependencies(souidemo msftedit)
        target_link_libraries(souidemo PRIVATE msftedit)
    endif()
    find_library(OHOS_ACE_NAPI ace_napi.z)
    find_library(OHOS_ACE_NDK ace_ndk.z)
    find_library(OHOS_HILOG hilog_ndk.z)
    foreach(_ohos_lib IN ITEMS OHOS_ACE_NAPI OHOS_ACE_NDK OHOS_HILOG)
        if(${_ohos_lib})
            target_link_libraries(souidemo PRIVATE ${${_ohos_lib}})
        endif()
    endforeach()
    target_precompile_headers(souidemo PRIVATE "stdafx.h")
    set_target_properties(souidemo PROPERTIES FOLDER demos)
endif()
