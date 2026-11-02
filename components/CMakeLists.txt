# 
# SOUI cmake配置文件
#
set(SOUI_COM_DIR ${CMAKE_CURRENT_LIST_DIR})
set(SOUI_ROOT_DIR ${SOUI_COM_DIR}/..)

# wasm 最小构建只保留 imgdecoder-stb + render-gdi：
#   - render-gdi 是 SAppCfg::SetRender(Render_Gdi) 的落点，它的非 Windows 后端
#     （linux/render-gdi.cpp）只调 Win32 GDI 接口，而这套接口已由 swinx 的
#     src/gdi/cairo 在 wasm 上编译通过；
#   - imgdecoder-stb 是必填项：SAppCfg::DoConfig 拿不到 IImgDecoderFactory 会直接
#     返回 FALSE，应用起不来。
# 其余组件不进构建图：log4z/TaskLoop/SIpcObject/ws/network
# 依赖桌面或联网能力，resprovider-7zip 依赖 7z，resprovider-zip 依赖
# third-part/zlib —— 那份 zlib 和 swinx/thirdparty 里的 zlib_static 是同一批符号的
# 两份拷贝，桌面构建分处不同 .so 无所谓，wasm 全链进一个模块会报 duplicate symbol
# （见 third-part/CMakeLists.txt）。wasm 的资源改用 SOUI 核心自带的 SResProviderFiles。
# ScriptModule-LUA 是个例外：它本来就是"静态库 + 直接 new 出工厂"的形态（COM 在
# wasm 上没有注册表），SOUI_ENABLE_COM_LIB=ON 时编成 libscriptmodule-lua.a，由
# 宿主直接调 Script_Lua_SCreateInstance 并把工厂交给 SApplication::SetScriptFactory。
# translator 跟着脚本模块一起进：Lua 导出表里有全局函数 CreateTranslatorMgr，
# 静态 COM 加载器把它展开成 SNS::TRANSLATOR::SCreateInstance 的真实引用，
# 不把这个组件链进来就是未定义符号（不是运行时才失败）。
add_subdirectory(imgdecoder-stb)
if(SOUI_WASM_MINIMAL)
if(SOUI_BUILD_WASM_LUA)
add_subdirectory(ScriptModule-LUA)
add_subdirectory(translator)
endif()
# 中国象棋 wasm 客户端要多四个组件：TaskLoop（单线程队列）、ws（浏览器
# WebSocket）、resprovider-zip（解压服务器下发的主题包）和它依赖的 zlib。
# 三者都按 wasm 形态单独实现/接线，默认演示构建不受影响。
if(SOUI_BUILD_WASM_CNCHESS)
add_subdirectory(TaskLoop)
add_subdirectory(ws)
add_subdirectory(resprovider-zip)
endif()
endif()
if(NOT SOUI_WASM_MINIMAL)
add_subdirectory(translator)
add_subdirectory(log4z)
add_subdirectory(TaskLoop)
if(SOUI_BUILD_HTTPCLIENT)
    add_subdirectory(httpclient)
endif()
if (CMAKE_SYSTEM_NAME MATCHES Windows)
    add_subdirectory(imgdecoder-wic)
    add_subdirectory(imgdecoder-gdip)
    add_subdirectory(render-d2d)
endif()

if(NOT CMAKE_SYSTEM_NAME MATCHES Android)
add_subdirectory(SIpcObject)
add_subdirectory(resprovider-7zip)
endif()
add_subdirectory(resprovider-zip)
add_subdirectory(render-skia)
add_subdirectory(ScriptModule-LUA)

if(TARGET websockets)
add_subdirectory(ws)
endif()
add_subdirectory(network)
endif()
add_subdirectory(render-gdi)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/com-cfg.h" DESTINATION include/components)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/commask.h" DESTINATION include/components)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/commgr2.h" DESTINATION include/components)
