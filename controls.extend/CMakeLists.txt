#
# 1394020320@qq.com
#

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SOUI_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

include_directories(${SOUI_ROOT_DIR}/components)
include_directories(${SOUI_ROOT_DIR}/utilities/include)
include_directories(${SOUI_ROOT_DIR}/SOUI/include)
include_directories(${SOUI_ROOT_DIR}/soui-sys-resource)
include_directories(${SOUI_ROOT_DIR}/controls.extend)
include_directories(${SOUI_ROOT_DIR}/third-part/Scintilla/include)
include_directories(${SOUI_ROOT_DIR}/third-part/wke/include)
file(GLOB_RECURSE CTRLS_SRC CONFIGURE_DEPENDS ${SOUI_ROOT_DIR}/controls.extend/*.*)

# wasm 的 Scintilla 开关（根 CMakeLists 的 EMSCRIPTEN 分支定义，默认 ON）。
# 关掉时三个源文件一起退出编译：SScintillaView（headless 控件）、ScintillaWnd
# （真窗口封装）和 SRealWndHandler_Scintilla（<realwnd> 处理器），它们的头文件
# 都 #include <Scintilla.h>，而 Scintilla 子目录此时根本不在构建图里。
# SOUI_WASM_HAS_SCINTILLA=0 同时传给 SCtrlsRegister.cpp，把 RegisterWindowClass
# 挡掉；不挡的话标签 <scintilla> 会在 SApp 的窗口类表里查不到，控件静默消失。
if(EMSCRIPTEN AND NOT SOUI_BUILD_WASM_SCINTILLA)
    list(FILTER CTRLS_SRC EXCLUDE REGEX "(SScintillaView|ScintillaWnd|SRealWndHandler_Scintilla)\\.cpp$")
    set(_soui_extend_scintilla OFF)
else()
    set(_soui_extend_scintilla ON)
endif()

add_library(ExtendCtrls STATIC ${CTRLS_SRC})
set(OPTION_LIBS ${OPTION_LIBS} ExtendCtrls CACHE INTERNAL "option_lib")

target_link_libraries(ExtendCtrls soui4 utilities4)
if(NOT _soui_extend_scintilla)
    target_compile_definitions(ExtendCtrls PRIVATE SOUI_WASM_HAS_SCINTILLA=0)
else()
    add_dependencies(ExtendCtrls Scintilla)
endif()

if(CMAKE_SYSTEM_NAME MATCHES Windows)
target_link_libraries(ExtendCtrls gdiplus Msimg32)
target_link_libraries(ExtendCtrls Scintilla)
if(NOT MINGW)
    target_link_libraries(ExtendCtrls smiley)
endif()

else()
target_compile_options(ExtendCtrls PRIVATE -fPIC)
target_link_libraries(ExtendCtrls swinx)
if(_soui_extend_scintilla)
target_link_libraries(ExtendCtrls Scintilla)
endif()
endif()

# 仅安装当前源码目录下的 .h/.hpp/.hxx/.hh 头文件到 include/ 目录
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/" DESTINATION include/controls.extend FILES_MATCHING PATTERN "*.h")

install(TARGETS ExtendCtrls
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS ExtendCtrls
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
