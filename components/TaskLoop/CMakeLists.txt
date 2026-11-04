# 
# SOUI cmake配置文件
#
# ITaskLoop 的两个后端按目录分开，一次构建只选中其中一个目录：
#   thread/ —— 真线程版（Win32 / pthread），桌面、Android、OHOS 用；
#   wasm/   —— 浏览器版，同一份实现里再按 -pthread 分成"宿主主循环泵"和
#              "wasm worker 线程"两条路。两边对上都是同一套 ITaskLoop 语义。
# 目录里只有这一个后端的文件，所以源列表直接按目录取，不再逐文件点名。

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
include_directories(${SOUI_ROOT_DIR}/utilities/include)
include_directories(${SOUI_ROOT_DIR}/SOUI/include)

if(EMSCRIPTEN)
    set(TaskLoop_backend wasm)
else()
    set(TaskLoop_backend thread)
endif()

file(GLOB TaskLoop_header CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${TaskLoop_backend}/*.h)
file(GLOB TaskLoop_src CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${TaskLoop_backend}/*.cpp)
source_group("Header Files" FILES ${TaskLoop_header})
source_group("Source Files" FILES ${TaskLoop_src})

# wasm 侧只有静态库形态：浏览器没有 COM 注册表，宿主直接链 SCreateInstance 符号。
# 桌面侧按 SOUI_ENABLE_COM_LIB 决定编成动态组件（带 TaskLoop.rc）还是静态库。
if(EMSCRIPTEN OR SOUI_ENABLE_COM_LIB)
    add_library(taskloop STATIC ${TaskLoop_src} ${TaskLoop_header})
else()
    add_library(taskloop SHARED ${TaskLoop_src} ${TaskLoop_header} ${TaskLoop_backend}/TaskLoop.rc)
    if(WIN32)
        target_link_libraries(taskloop Winmm)
    endif()
endif()
target_link_libraries(taskloop utilities4)

set(COM_LIBS ${COM_LIBS} taskloop CACHE INTERNAL "com_lib")
set_target_properties(taskloop PROPERTIES
    FOLDER components
)

if(NOT EMSCRIPTEN)
    install(TARGETS taskloop
        CONFIGURATIONS Release
        RUNTIME DESTINATION bin/Release
        LIBRARY DESTINATION lib/Release
        ARCHIVE DESTINATION lib/Release
    )
    install(TARGETS taskloop
        CONFIGURATIONS Debug
        RUNTIME DESTINATION bin/Debug
        LIBRARY DESTINATION lib/Debug
        ARCHIVE DESTINATION lib/Debug
    )
endif()
