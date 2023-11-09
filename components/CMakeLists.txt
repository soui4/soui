# 
# SOUI cmake配置文件
#
add_subdirectory(imgdecoder-gdip)
add_subdirectory(imgdecoder-png)
add_subdirectory(imgdecoder-stb)
add_subdirectory(imgdecoder-wic)


add_subdirectory(render-gdi)
add_subdirectory(render-skia)
add_subdirectory(render-d2d)
add_subdirectory(resprovider-zip)

add_subdirectory(translator)
add_subdirectory(ScriptModule-Lua)

add_subdirectory(log4z)
add_subdirectory(resprovider-7zip)
add_subdirectory(TaskLoop)
add_subdirectory(SIpcObject)
add_subdirectory(httpclient)

#include(CheckCXXCompilerFlag)
#CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
#if(COMPILER_SUPPORTS_CXX11)
#if(MSVC_VERSION LESS 1700)
#	message(STATUS "build resprovider-7zip OFF")
#else()
#	add_subdirectory(resprovider-7zip)
#	message(STATUS "build resprovider-7zip ON")
#endif()

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/com-cfg.h" DESTINATION components)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/commask.h" DESTINATION components)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/commgr2.h" DESTINATION components)
