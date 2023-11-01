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

cmake_minimum_required(VERSION 3.4.3)
project(SOUI4)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/__cmake/")
include(__cmake/internal_utils.cmake)
include(__cmake/global.cmake)



option(USE_UNICODE "Use Unicode" ON)

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

option(OUTPATH_WITHOUT_TYPE "On to output debug/release to different folder(Off to same folder and add suffix 'd' for debug )" OFF)

config_compiler_and_linker()

if(OUTPATH_WITHOUT_TYPE)
message("output config to ${PROJECT_BINARY_DIR}/config/config.h")
configure_file("config/config.h.in" "${PROJECT_BINARY_DIR}/config/config.h" @ONLY)
configure_file("config/build.cfg.in" "${PROJECT_BINARY_DIR}/config/build.cfg" @ONLY)
include_directories(${PROJECT_BINARY_DIR}/config)
install(FILES "${PROJECT_BINARY_DIR}/config/config.h" DESTINATION config)
else()
message("output config to ${PROJECT_SOURCE_DIR}/config/config.h")
configure_file("config/config.h.in" "${PROJECT_SOURCE_DIR}/config/config.h" @ONLY)
configure_file("config/build.cfg.in" "${PROJECT_SOURCE_DIR}/config/build.cfg" @ONLY)
include_directories(${PROJECT_SOURCE_DIR}/config)
endif()

#
#
add_subdirectory(third-part)

#
#
#
add_subdirectory(utilities)
add_subdirectory(SOUI)
add_subdirectory(soui-sys-resource)

#
#
# 
add_subdirectory(components)

#
#
add_subdirectory(demo)
add_subdirectory(demo2)

##
##