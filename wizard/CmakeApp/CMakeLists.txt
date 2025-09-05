#
# 1394020320@qq.com
#

cmake_minimum_required(VERSION 3.16)
set(project_name "soui5_app")
project(${project_name})
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	if(SIZEOF_VOID_P EQUAL 4)
	file (TO_CMAKE_PATH "$ENV{SOUI4_INSTALL_32}" SOUI_INSTALL_DIR)
	else()
	file (TO_CMAKE_PATH "$ENV{SOUI4_INSTALL_64}" SOUI_INSTALL_DIR)
	endif()
else()
	file (TO_CMAKE_PATH "$ENV{SOUI4_INSTALL}" SOUI_INSTALL_DIR)
endif()
if(DEFINED SOUI_INSTALL_DIR)
	message(STATUS "SOUI_INSTALL_DIR: ${SOUI_INSTALL_DIR}")
else()
	set(SOUI_INSTALL_DIR "/Users/jianxionghuang/work/soui4_install")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
add_compile_options(-Wno-c++11-extensions)
add_compile_options(-Wno-inconsistent-missing-override)
endif()
# 查找 pkg-config
find_package(PkgConfig REQUIRED)
set(ENV{PKG_CONFIG_PATH} "${SOUI_INSTALL_DIR}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
pkg_check_modules(SOUI REQUIRED libsoui)
message(STATUS "SOUI_INCLUDE_DIRS: ${SOUI_INCLUDE_DIRS}")
include_directories(${SOUI_INCLUDE_DIRS})
link_directories(${SOUI_LIBRARY_DIRS})
set(SOUI_ROOT ${SOUI_PREFIX})

message(STATUS "SOUI_ROOT: ${SOUI_ROOT}")
if ("${SOUI_ROOT}" STREQUAL "")
	message(FATAL_ERROR "FATAL:can't find env variable SOUI4_INSTALL_64, clone soui core and install wizard first, please.")
endif()

include(${SOUI_PREFIX}/__cmake/internal_utils.cmake)
include(${SOUI_PREFIX}/__cmake/global.cmake)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	readSettingFile(${SOUI_ROOT}/include/config/build.cfg UNICODE "1" USE_UNICODE)
	readSettingFile(${SOUI_ROOT}/include/config/build.cfg WCHAR "1" WCHAR_AS_DEFAULT)
	readSettingFile(${SOUI_ROOT}/include/config/build.cfg MT "0" SHARED_CRT)
	readSettingFile(${SOUI_ROOT}/include/config/build.cfg SUPPORT_XP "1" XP_TOOLSET)
	config_compiler_and_linker()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB CURRENT_HEADERS  *.h *.hpp)
file(GLOB CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resouece" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})


add_soui_exe(${project_name} WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC})
if(CMAKE_SYSTEM_NAME MATCHES Darwin)
set(MACOSX_BUNDLE_HIGH_RESOLUTION_CAPABLE "true")
set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.soui.app")
set(MACOSX_BUNDLE_BUNDLE_NAME "soui5_app")
# 配置代码签名
set(MACOSX_CODESIGN_IDENTITY "Apple Development")
set(MACOSX_TEAM_ID "XXXXXXXXXXXX")
 
# 设置 Xcode 特定的签名属性
set_target_properties(${project_name} PROPERTIES
    XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${MACOSX_TEAM_ID}"
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${MACOSX_CODESIGN_IDENTITY}"
    XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
    XCODE_ATTRIBUTE_PROVISIONING_PROFILE_SPECIFIER ""
)
file(GLOB_RECURSE DYLIB_SOUI  ${SOUI_LIBRARY_DIRS}/*.dylib)
copy_frameworks_to_bundle(${project_name} "${DYLIB_SOUI}")

add_macos_res_file(${project_name} ${SOUI_ROOT}/resources/soui-sys-resource.zip data) 
add_macos_res_file(${project_name} ${SOUI_ROOT}/resources/simsun.ttc fonts) 
add_macos_res_folder(${project_name} ${CMAKE_CURRENT_SOURCE_DIR}/uires data/uires)
set_macos_icon(${project_name} ${CMAKE_CURRENT_SOURCE_DIR}/res/soui-app.icns ${SOUI_ROOT}/__cmake/plist.in)
endif()

target_link_libraries(${project_name} ${SOUI_LIBRARIES})

target_precompile_headers(${project_name} PRIVATE "stdafx.h")
 