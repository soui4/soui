#
# SOUI cmake配置文件
# 生成主要的SOUI库和demo
# 使用帮助
#
# 使用cmake 3.16.3 或以上版本
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

cmake_minimum_required(VERSION 3.16.3)
project(SOUI4)

#
# 设置输出目录
#
if (CMAKE_CL_64)
    if("${CMAKE_GENERATOR_PLATFORM}" MATCHES "ARM64")
    message("---Put All generation in same Path(bin64a)")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin64a)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin64a)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin64a)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin64a)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin64a)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin64a)
    else()
    message("---Put All generation in same Path(bin64)")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin64)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin64)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin64)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin64)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin64)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin64)
    endif()
else ()
    message("---Put All generation in same Path(bin)")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
endif ()

#
# 设置输入文件的后缀
#
set(CMAKE_DEBUG_POSTFIX "d")
set(CMAKE_RELEASE_POSTFIX "")

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

#
# 修正编译参数
#
if (NOT SHARED_CRT)
    message("-- Building with MT")
    FIX_DEFAULT_COMPILER_SETTINGS_MT()
    set(BUILD_CONF_MT "1")
else()
    message("-- Building with MD")
    set(BUILD_CONF_MT "0")
endif()

if (NOT WCHAR_AS_DEFAULT)
    FIX_DEFAULT_COMPILER_SETTINGS_WCHAR()
    set(BUILD_CONF_WCHAR "0")
else()
    set(BUILD_CONF_WCHAR "1")
endif()

if(USE_UNICODE)
    add_definitions(-DUNICODE -D_UNICODE)
    set(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} -DUNICODE -D_UNICODE")
    message("-- Building with Unicode")
    set(BUILD_CONF_UNICODE "1")
else(USE_UNICODE)
    add_definitions(-D_MBCS)
    message("-- Building with MBCS")
    set(BUILD_CONF_UNICODE "0")
endif(USE_UNICODE)

#
# 支持xp环境
#
if (XP_TOOLSET) 
    if (MSVC_VERSION GREATER 1700) # 1700 is Visual Studio 2012.
        add_definitions(-D_USING_V110_SDK71_)
    endif()
    set(BUILD_CONF_XP_TOOLSET "1")
else(XP_TOOLSET)
    set(BUILD_CONF_XP_TOOLSET "0")
endif()

configure_file("config/config.h.in" "${PROJECT_SOURCE_DIR}/config/config.h" @ONLY)
configure_file("config/build.cfg.in" "${PROJECT_SOURCE_DIR}/config/build.cfg" @ONLY)

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