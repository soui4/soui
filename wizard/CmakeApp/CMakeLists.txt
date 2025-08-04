#
# 1394020320@qq.com
#

cmake_minimum_required(VERSION 3.16)
set(project_name "soui5_app")
project(demo)


file (TO_CMAKE_PATH "$ENV{SOUI4PATH}" SOUI_SRC_ROOT)

message(STATUS "SOUI_SRC_ROOT: ${SOUI_SRC_ROOT}")
if ("${SOUI_SRC_ROOT}" STREQUAL "")
	message(FATAL_ERROR "FATAL:can't find env variable SOUI_SRC_ROOT, clone soui core and install wizard first, please.")
endif()

message(STATUS "CMAKE_GENERATOR_PLATFORM: ${CMAKE_GENERATOR_PLATFORM}")
if(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
file (TO_CMAKE_PATH "$ENV{SOUI4_INSTALL_32}" SOUI_ROOT)
else()
file (TO_CMAKE_PATH "$ENV{SOUI4_INSTALL_64}" SOUI_ROOT)
endif()

message(STATUS "SOUI_ROOT: ${SOUI_ROOT}")
if ("${SOUI_ROOT}" STREQUAL "")
	message(FATAL_ERROR "FATAL:can't find env variable SOUI4_INSTALL_64, clone soui core and install wizard first, please.")
endif()

include(${SOUI_SRC_ROOT}/__cmake/internal_utils.cmake)
include(${SOUI_SRC_ROOT}/__cmake/global.cmake)

readSettingFile(${SOUI_ROOT}/include/config/build.cfg UNICODE "1" USE_UNICODE)
readSettingFile(${SOUI_ROOT}/include/config/build.cfg WCHAR "1" WCHAR_AS_DEFAULT)
readSettingFile(${SOUI_ROOT}/include/config/build.cfg MT "0" SHARED_CRT)
readSettingFile(${SOUI_ROOT}/include/config/build.cfg SUPPORT_XP "1" XP_TOOLSET)

config_compiler_and_linker()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

include_directories(${SOUI_ROOT}/include/config)
include_directories(${SOUI_ROOT}/include/components)
include_directories(${SOUI_ROOT}/include/utilities/include)
include_directories(${SOUI_ROOT}/include/SOUI/include)
include_directories(${SOUI_SRC_ROOT}/controls.extend)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB CURRENT_HEADERS  *.h *.hpp)
file(GLOB CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resouece" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})

link_directories(${SOUI_ROOT}/lib)
link_directories(${SOUI_ROOT}/lib/${CMAKE_BUILD_TYPE})
add_executable(${project_name} WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC})

target_link_libraries(${project_name} soui4 utilities4)

target_precompile_headers(${project_name} PRIVATE "stdafx.h")