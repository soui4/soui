# 
# SOUI cmake配置文件
#
add_subdirectory(imgdecoder-png)
add_subdirectory(imgdecoder-stb)

add_subdirectory(translator)

add_subdirectory(log4z)
add_subdirectory(TaskLoop)
if (CMAKE_SYSTEM_NAME MATCHES Windows)



add_subdirectory(imgdecoder-wic)
add_subdirectory(imgdecoder-gdip)
add_subdirectory(resprovider-7zip)
add_subdirectory(render-d2d)
add_subdirectory(ScriptModule-LUA)
add_subdirectory(httpclient)
endif()
add_subdirectory(SIpcObject)
add_subdirectory(resprovider-zip)
add_subdirectory(render-gdi)
add_subdirectory(render-skia)

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/com-cfg.h" DESTINATION include/components)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/commask.h" DESTINATION include/components)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/commgr2.h" DESTINATION include/components)
