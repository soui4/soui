# 
# SOUI cmake配置文件
#
add_subdirectory(imgdecoder-stb)
add_subdirectory(translator)
add_subdirectory(log4z)
add_subdirectory(TaskLoop)
add_subdirectory(httpclient)

if (CMAKE_SYSTEM_NAME MATCHES Windows)
add_subdirectory(imgdecoder-wic)
add_subdirectory(imgdecoder-gdip)
if(NOT MINGW AND NOT CYGWIN)
add_subdirectory(render-d2d)
endif()
endif()

add_subdirectory(SIpcObject)
add_subdirectory(resprovider-zip)
add_subdirectory(resprovider-7zip)
add_subdirectory(render-gdi)
add_subdirectory(render-skia)
add_subdirectory(ScriptModule-LUA)

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/com-cfg.h" DESTINATION include/components)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/commask.h" DESTINATION include/components)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/commgr2.h" DESTINATION include/components)
