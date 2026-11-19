# =============================================================================
# __cmake/wasm-host-js.cmake -- 浏览器宿主脚本 soui-host.js 的接入
#
# 仓库里所有 wasm 程序共用同一份宿主实现：tools/wasm/soui-host.js。它负责画布上屏、
# 鼠标/键盘/滚轮注入（含触摸折算）、IME 组合输入代理、剪贴板桥、WebAudio 音效、崩溃
# 上报与加载遮罩；页面壳只写自己的排版和一行 SouiHost.start({...})。
#
# emcc 只产出 .html/.js/.wasm/.data 四件套，宿主 js 不在其中，必须由构建单独拷进产物
# 目录 —— 页面用相对路径 <script src="soui-host.js"> 引用它，缺了就是白屏。
#
# 用法（各 wasm app 的顶层 CMakeLists 里，target 建好之后）：
#
#   include("${SOUI4_DIR}/__cmake/wasm-host-js.cmake")
#   soui_wasm_host_js(<target> <out_dir>)
#
# 它做四件事：
#   1. 定 SOUI_HOST_JS 缓存变量（默认 ${SOUI4_DIR}/tools/wasm/soui-host.js，可用
#      -DSOUI_HOST_JS=<path> 覆盖；搬迁前生成的构建目录里指向旧路径的缓存会自动回落）；
#   2. 文件不存在就 FATAL_ERROR —— 后果是页面白屏，早点在配置期炸掉比上线后查好；
#   3. 加进 target 的 LINK_DEPENDS：只改 js 也会触发一次重链，重链顺带跑下面的
#      POST_BUILD；不声明的话改完 js 刷新页面看到的还是产物目录里那一份旧的；
#   4. POST_BUILD copy_if_different 到 <out_dir>/soui-host.js。
# =============================================================================

function(soui_wasm_host_js _target _out_dir)
    set(_soui_host_js_default "${SOUI4_DIR}/tools/wasm/soui-host.js")
    set(_soui_host_js_desc "wasm: shared browser host script (tools/wasm/soui-host.js)")

    if(NOT SOUI_HOST_JS)
        set(SOUI_HOST_JS "${_soui_host_js_default}" CACHE FILEPATH "${_soui_host_js_desc}" FORCE)
    endif()
    # CACHE 变量只写一次：搬迁（demos/wasm-demo/ -> tools/wasm/）之前配置过的构建目录
    # 里还留着旧路径，那个文件已经不在了。这里回落到默认值，免得产物目录里没宿主 js。
    if(NOT EXISTS "${SOUI_HOST_JS}" AND EXISTS "${_soui_host_js_default}")
        message(STATUS "soui-host.js: ${SOUI_HOST_JS} 已不存在，回落默认值")
        set(SOUI_HOST_JS "${_soui_host_js_default}" CACHE FILEPATH "${_soui_host_js_desc}" FORCE)
    endif()
    if(NOT EXISTS "${SOUI_HOST_JS}")
        message(FATAL_ERROR "soui-host.js 不存在: ${SOUI_HOST_JS} —— 产物目录里不会有它，页面打不开")
    endif()

    set_property(TARGET ${_target} APPEND PROPERTY LINK_DEPENDS "${SOUI_HOST_JS}")
    add_custom_command(TARGET ${_target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SOUI_HOST_JS}"
                "${_out_dir}/soui-host.js"
        COMMENT "Copying soui-host.js -> ${_out_dir}"
        VERBATIM)

    message(STATUS "${_target}: host js = ${SOUI_HOST_JS}")
endfunction()
