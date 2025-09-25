#
# SOUI cmake配置文件
# 生成主要的SOUI库和demo
# 使用帮助
#
# 使用cmake 3.16 或以上版本
# 
#
# 目前支持编译的库/可执行程序:

# mkdir build
# cd build 
# cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
# nmake
#
# or 
# cmake .. -G "Visual Studio 14 2015"
# 
# 生成SOUI.sln工程文件
#   
# options:
#   SHARED_CRT             ON 
#   USE_UNICODE            ON
#   WCHAR_AS_DEFAULT       ON 
#   XP_TOOLSET             ON   # visual studio 2012
#   ENABLE_SOUI_CORE_LIB   OFF 
#   ENABLE_SOUI_COM_LIB    OFF
# 
# 1394020320@qq.com
#

cmake_minimum_required(VERSION 3.16)
project(SOUI4)
set(CMAKE_CXX_STANDARD 11)


file(STRINGS  SOUI/include/soui-version.h ver_h_contents REGEX "^#define SOUI_VER[0-9]+ [0-9]+$")

foreach(line IN LISTS ver_h_contents) 
    string(REGEX MATCH "SOUI_VER([0-9]+) ([0-9]+)" match "${line}")
    if(match)
        set(SOUI_VER${CMAKE_MATCH_1} ${CMAKE_MATCH_2})
    endif()
endforeach()

message(STATUS "SOUI_VER: ${SOUI_VER1}.${SOUI_VER2}.${SOUI_VER3}.${SOUI_VER4}")

set(CMAKE_CONFIGURATION_TYPES Debug Release)
if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "build type is ${CMAKE_BUILD_TYPE}")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/__cmake/")
include(__cmake/internal_utils.cmake)
include(__cmake/global.cmake)


if (CMAKE_SYSTEM_NAME STREQUAL "MSYS")
    message(STATUS "Overriding incorrect Linux detection for MSYS")
    set(CMAKE_SYSTEM_NAME Windows CACHE STRING "Target system" FORCE)
endif()
if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND MINGW)
    message(STATUS "Overriding incorrect Linux detection for MINGW")
    set(CMAKE_SYSTEM_NAME Windows CACHE STRING "Target system" FORCE)
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
if (CMAKE_SYSTEM_NAME MATCHES Windows)
IF (CMAKE_CL_64)
  set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/output_64 CACHE PATH "default install prefix" FORCE)
ELSE (CMAKE_CL_64)
  set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/output_32 CACHE PATH "default install prefix" FORCE)
ENDIF (CMAKE_CL_64)
else()
set (CMAKE_INSTALL_PREFIX "/usr/local/soui5" CACHE PATH "default install prefix" FORCE)
endif()
endif()

if (CMAKE_SYSTEM_NAME MATCHES Windows)
  option(USE_UNICODE "Use Unicode" ON)
else()
  option(USE_UNICODE "Use Unicode" OFF)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-D_DEBUG)
  endif()
endif()

# When other libraries are using a shared version of runtime libraries,
# Google Test also has to use one.
option(SHARED_CRT "Use shared crt runtime library." ON)

# When other libraries are using a shared version of runtime libraries,
# Google Test also has to use one.
option(WCHAR_AS_DEFAULT "Use wchar_t as internal type" ON)
#
#
option(ENABLE_SOUI_CORE_LIB "Enable compile 'core' as static lib" OFF)
#
#
option(ENABLE_SOUI_COM_LIB "Enable compile 'components' as static lib" OFF)
#
#
option(XP_TOOLSET "" ON)
#
#
option(ENABLE_SPECTRE "Enable Spectre Mitigation" OFF)
#
#
option(BUILD_DEMOS "Build builtin demos" ON)


if (CMAKE_SYSTEM_NAME MATCHES Windows)
if(NOT MINGW)
option(BUILD_RICHEDIT "using richedit from source code" ON)
else()
set(BUILD_RICHEDIT OFF)
endif()
option(ENABLE_SOUI_ACC "Enable Acc" OFF)
option(ENABLE_SOUI_SPY "Enable Swnd Spy" OFF)
if (MSVC)
    add_compile_options(/MP)
endif()
else()
set(BUILD_RICHEDIT ON)
set(ENABLE_SOUI_ACC OFF)
set(ENABLE_SOUI_SPY OFF)
endif()


config_compiler_and_linker()


if (CMAKE_SYSTEM_NAME MATCHES Windows)
  if(MSVC_VERSION LESS 1600)
      include_directories(${PROJECT_SOURCE_DIR}/third-part/stdint)
  endif()
  add_definitions(-DWCHAR_SIZE=2)
else()
  # 添加调试标志
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")
  if(CMAKE_SYSTEM_NAME MATCHES Linux)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
  endif()
  if(CMAKE_SYSTEM_NAME MATCHES Darwin)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0")
  add_compile_options(-Wno-deprecated-declarations)
  add_compile_options(-Wno-expansion-to-defined)
  add_compile_options(-Wno-inconsistent-missing-override)
  add_compile_options(-Wno-parentheses)
  add_compile_options(-Wno-unused-value)
  add_compile_options(-Wno-switch)
  add_compile_options(-Wno-unsequenced)
  add_compile_options(-Wno-undefined-var-template)
  else()
  set(CMAKE_SKIP_BUILD_RPATH FALSE)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
  set(CMAKE_INSTALL_RPATH $ORIGIN)
  endif()

  add_definitions(-DWCHAR_SIZE=4)
  include_directories(${PROJECT_SOURCE_DIR}/swinx/include)
  add_subdirectory(swinx)
endif()

configure_file("config/config.h.in" "${PROJECT_BINARY_DIR}/config/config.h" @ONLY)
configure_file("config/build.cfg.in" "${PROJECT_BINARY_DIR}/config/build.cfg" @ONLY)
include_directories(${PROJECT_BINARY_DIR}/config)
install(FILES "${PROJECT_BINARY_DIR}/config/config.h" DESTINATION include/config)
install(FILES "${PROJECT_BINARY_DIR}/config/build.cfg" DESTINATION include/config)

# 设置库文件的输出路径
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_definitions(-D_CRT_NONSTDC_NO_WARNINGS -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)

link_directories(${CMAKE_BINARY_DIR}/bin)

add_subdirectory(third-part)
add_subdirectory(utilities)
add_subdirectory(components)
add_subdirectory(SOUI)
if (CMAKE_SYSTEM_NAME MATCHES Windows)
add_subdirectory(soui-sys-resource)
add_subdirectory(soui-sys-resource2)
endif()
add_subdirectory(tools)
add_subdirectory(controls.extend)
if(BUILD_DEMOS)
add_subdirectory(demos)
endif(BUILD_DEMOS)


if (UNIX OR MINGW)
set(SWINX_LIB_FLAGS "swinx")
set(SWINX_CFLAGS "\$\{includedir\}/swinx/include")

# Generate and install pkgconfig.
configure_file("${PROJECT_SOURCE_DIR}/__cmake/libsoui.pc.in" "${PROJECT_BINARY_DIR}/libsoui.pc" @ONLY)

install(FILES "${PROJECT_BINARY_DIR}/libsoui.pc"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
        COMPONENT pkgconfig)
else()
configure_file("${PROJECT_SOURCE_DIR}/__cmake/soui-config.cmake.in" "${PROJECT_BINARY_DIR}/soui-config.cmake" @ONLY)

install(FILES "${PROJECT_BINARY_DIR}/soui-config.cmake"
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
endif(UNIX OR MINGW)

# Install cmake files
install(DIRECTORY ${PROJECT_SOURCE_DIR}/__cmake
        DESTINATION ${CMAKE_INSTALL_PREFIX}
        FILES_MATCHING PATTERN "*.cmake"
)
install(FILES ${PROJECT_SOURCE_DIR}/__cmake/plist.in
        DESTINATION ${CMAKE_INSTALL_PREFIX}/__cmake
)

install(FILES 
        ${PROJECT_SOURCE_DIR}/simsun.ttc
        ${PROJECT_SOURCE_DIR}/soui-sys-resource.zip
        DESTINATION ${CMAKE_INSTALL_PREFIX}/resources
)

install(FILES ${PROJECT_SOURCE_DIR}/LICENSE
        DESTINATION ${CMAKE_INSTALL_PREFIX}
)