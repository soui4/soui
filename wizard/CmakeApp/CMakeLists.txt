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
	#set(SOUI_INSTALL_DIR "/home/flyhigh/work/soui4_install")
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

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-D_DEBUG)
endif()

include(${SOUI_INSTALL_DIR}/__cmake/soui-config.cmake)


message(STATUS "SOUI_INCLUDE_DIRS: ${SOUI_INCLUDE_DIRS}")
include_directories(${SOUI_INCLUDE_DIRS})
link_directories(${SOUI_LIBRARY_DIRS})
set(SOUI_ROOT ${SOUI_PREFIX})

message(STATUS "SOUI_ROOT: ${SOUI_ROOT}")
if ("${SOUI_ROOT}" STREQUAL "")
	message(FATAL_ERROR "FATAL:can't find env variable SOUI4_INSTALL_64, clone soui core and install wizard first, please.")
endif()

set(RESBUILD_EXE ${SOUI_ROOT}/bin/Release/uiresbuilder)

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
file(GLOB_RECURSE CURRENT_SOUIS  ${CMAKE_CURRENT_SOURCE_DIR}/uires/*.*)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resouece" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})

# 将生成的resource.h文件添加到项目中
set(UIRES_RES_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/res/resource.h)
# 添加自定义命令来处理uires.idx文件
# 这将使用uiresbuilder工具将资源索引文件转换为resource.h 
add_custom_command(
    OUTPUT ${UIRES_RES_HEADER}
    COMMAND ${RESBUILD_EXE} -i ${CMAKE_CURRENT_SOURCE_DIR}/uires/uires.idx -p ${CMAKE_CURRENT_SOURCE_DIR}/uires -r ${CMAKE_CURRENT_SOURCE_DIR}/res/soui_res.rc2 -h ${CMAKE_CURRENT_SOURCE_DIR}/res/resource.h idtable
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/uires/uires.idx
    COMMENT "Processing uires.idx to generate soui_res.rc2"
    VERBATIM
)

# 确保在构建时会处理uires.idx文件
set_source_files_properties(${UIRES_RES_HEADER} PROPERTIES GENERATED TRUE)

add_soui_exe(${project_name} WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${UIRES_RES_HEADER} ${CURRENT_RC})
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
else()
	# add UI resources folder (shared for all configs)
	add_app_res_folder(${project_name} ${CMAKE_CURRENT_SOURCE_DIR}/uires data/uires)
	file(GLOB_RECURSE SOUI_RES ${SOUI_ROOT}/resources/*.*)
	add_app_res_files(${project_name} "${SOUI_RES}" data)


	if(MSVC OR CMAKE_CONFIGURATION_TYPES)
		add_custom_command(TARGET ${project_name} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E echo "Copying SOUI DLLs from ${SOUI_ROOT}/bin/$<CONFIG> to target output"
			COMMAND ${CMAKE_COMMAND} -E copy_directory
				"${SOUI_ROOT}/bin/$<CONFIG>"
				$<TARGET_FILE_DIR:${project_name}>
			COMMENT "Copying SOUI DLLs for configuration $<CONFIG>")
	else()
		# Single-config generators (Makefile, Ninja): use CMAKE_BUILD_TYPE
		if(NOT CMAKE_BUILD_TYPE)
			set(CMAKE_BUILD_TYPE Release)
		endif()
		set(SOUI_BINARY_DIR ${SOUI_ROOT}/bin/${CMAKE_BUILD_TYPE}) 
		if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
			file(GLOB_RECURSE SOUI_DLL_FILES ${SOUI_BINARY_DIR}/*.dll)
		else()
			file(GLOB_RECURSE SOUI_DLL_FILES ${SOUI_BINARY_DIR}/*.so)
		endif()
		add_app_res_files(${project_name} "${SOUI_DLL_FILES}" "") 
	endif()
endif()

target_link_libraries(${project_name} ${SOUI_LIBRARIES})

target_precompile_headers(${project_name} PRIVATE "stdafx.h")
 