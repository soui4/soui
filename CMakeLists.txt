#
# SOUI cmake配置文件
# 生成主要的SOUI库和demo
# 使用帮助
#
# 使用cmake 3.4.3 或以上版本
# 
#
# 目前支持编译的库/可执行程序:

# mkdir build
# cd build 
# cmake .. -G"NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
# nmake
#
# or 
# cmake .. -G"Visual Studio 14 2015"
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

cmake_minimum_required(VERSION 3.1)
project(SOUI4)

set(CMAKE_CONFIGURATION_TYPES Debug Release)
if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE "Debug")
endif()

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/__cmake/")
include(__cmake/internal_utils.cmake)
include(__cmake/global.cmake)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
IF (CMAKE_CL_64)
  set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/output_64 CACHE PATH "default install prefix" FORCE)
ELSE (CMAKE_CL_64)
  set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/output_32 CACHE PATH "default install prefix" FORCE)
ENDIF (CMAKE_CL_64)
endif()

if (CMAKE_SYSTEM_NAME MATCHES Windows)
  option(USE_UNICODE "Use Unicode" ON)
else()
  set(USE_UNICODE OFF)
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

config_compiler_and_linker()

configure_file("config/config.h.in" "${PROJECT_BINARY_DIR}/config/config.h" @ONLY)
configure_file("config/build.cfg.in" "${PROJECT_BINARY_DIR}/config/build.cfg" @ONLY)
include_directories(${PROJECT_BINARY_DIR}/config)
install(FILES "${PROJECT_BINARY_DIR}/config/config.h" DESTINATION include/config)
install(FILES "${PROJECT_BINARY_DIR}/config/build.cfg" DESTINATION include/config)



# 设置库文件的输出路径
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_definitions(-D_CRT_NONSTDC_NO_WARNINGS -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)

if (CMAKE_SYSTEM_NAME MATCHES Windows)
  if(MSVC_VERSION LESS 1600)
      include_directories(${PROJECT_SOURCE_DIR}/utilities/include/stdint)
  endif()
  add_definitions(-DWCHAR_SIZE=2)
elseif(CMAKE_SYSTEM_NAME MATCHES Darwin)
else()
  # 添加调试标志
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
  add_definitions(-DWCHAR_SIZE=4)
  include_directories(${PROJECT_SOURCE_DIR}/swinx/include)
  add_subdirectory(swinx)
endif()

add_subdirectory(third-part)
add_subdirectory(utilities)

add_subdirectory(SOUI)
if (CMAKE_SYSTEM_NAME MATCHES Windows)
add_subdirectory(soui-sys-resource)
endif()
#
#
# 
add_subdirectory(components)

#
#
if (CMAKE_SYSTEM_NAME MATCHES Windows)
add_subdirectory(demo)
add_subdirectory(demo2)
endif()

add_subdirectory(fun_test)
