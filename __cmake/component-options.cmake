# =============================================================================
# __cmake/component-options.cmake -- components/ 下各可选组件的统一编译开关
#
# 为什么单独一个文件，且必须在 add_subdirectory(third-part) 之前 include：
#   * third-part/CMakeLists.txt 的 wasm 分支按 SOUI_BUILD_SCRIPT_LUA 决定是否
#     编 lua-54；各 wasm app 的顶层 CMakeLists 也按同一批开关决定链接哪些库、
#     是否 --preload-file 预载 uires/lua 与 snd。
#   * 早先只在 components/CMakeLists.txt 里 set(SOUI_BUILD_X ON)（普通变量），
#     作用域仅限 components/ 子树。根 CMakeLists 先 add_subdirectory(third-part)
#     再 add_subdirectory(components)，于是 third-part 与 app 顶层都读到「未定义」：
#     lua-54 不进构建图、SOUI_WASM_HAS_LUA=0、scriptmodule-lua 虽被编译却无人链接，
#     产物静默丢掉 Lua 脚本功能，编译和链接都不报错。
#   * 因此这里用 option() 把默认值写进 CMake cache，整个构建图（顶层工程、
#     third-part、components 及其子目录）读到的都是同一个值。
#
# 约定：全部组件默认 ON。命令行 -Dxxx=OFF 或构建脚本同名环境变量已写进 cache，
#       option() 不覆盖，定制能力不变。本文件只提供开关，不为任何特定 app 预设组合。
# =============================================================================

set(_soui_component_options
    IMGDECODER_STB RENDER_GDI LOG4Z TASKLOOP HTTPCLIENT
    IMGDECODER_WIC IMGDECODER_GDIP RENDER_D2D
    RES_IPC RES_7ZIP RES_ZIP RENDER_SKIA SCRIPT_LUA TRANSLATOR NETWORK WS)

foreach(_c IN LISTS _soui_component_options)
    if(NOT DEFINED SOUI_BUILD_${_c})
        option(SOUI_BUILD_${_c} "Build SOUI component ${_c}" ON)
    endif()
endforeach()
unset(_soui_component_options)
unset(_c)
