# =============================================================================
# components/CMakeLists.txt — SOUI 可选组件总开关
#
# 设计原则：
#   * 各组件相互独立，彼此之间没有编译期依赖；只有少数组件依赖第三方库或特定
#     平台（见下方各 add_subdirectory 旁的注释 / 平台判断）。
#   * 全部 COM 模块默认开启编译（每个一个 SOUI_BUILD_<NAME> 开关）。只有移动端
#     （Android / OHOS）和 WebAssembly 才按自身需要，用 -Dxxx=OFF 挑选用哪些组件；
#     桌面 / iOS 等默认全量，无需改这里。
#   * 依赖第三方库的组件不靠硬编码 OFF，而靠第三方目标是否存在自然跳过：例如 ws
#     看 TARGET websockets、resprovider-7zip 看 TARGET 7z（7z 不在 Android 上构建，
#     故 Android 上 resprovider-7zip 自动跳过，无需显式关）。
#   * WebAssembly（EMSCRIPTEN）走最小构建：第三方依赖（skia / lua / 7z / curl /
#     openssl …）不进构建图，因此这里只补由能力开关打开的少量可选组件。
# =============================================================================

set(SOUI_COM_DIR ${CMAKE_CURRENT_LIST_DIR})
set(SOUI_ROOT_DIR ${SOUI_COM_DIR}/..)

# ---------------------------------------------------------------------------
# 组件开关：全部默认 ON，定义在 __cmake/component-options.cmake（用 option() 写进
# CMake cache）。根 CMakeLists 在 add_subdirectory(third-part) 之前 include 它，
# 因此 third-part / 本文件 / 各 wasm app 顶层读到的是同一个值；命令行 -Dxxx=OFF
# 或构建脚本传入的值已在 cache 里，option() 不会覆盖。移动端 / wasm 由 app 自己
# 用 -Dxxx=OFF 挑选组件，这里不为任何特定 app 预设组合。依赖第三方库的组件
# （ws / resprovider-7zip）另靠 TARGET 可用性门控，平台不编第三方库时自动跳过。
# ---------------------------------------------------------------------------
include("${SOUI_ROOT_DIR}/__cmake/component-options.cmake")

if(SOUI_WASM_MINIMAL)
    # wasm 最小构建：第三方依赖不在构建图内；各组件仍按统一的 SOUI_BUILD_<NAME>
    # 开关挑选（默认全 ON），由各 wasm app 自己的构建脚本用
    # -Dxxx=OFF 决定最小集。这里不为任何特定 app 预设组件组合。
    if(SOUI_BUILD_IMGDECODER_STB)
        add_subdirectory(imgdecoder-stb)
    endif()
    if(SOUI_BUILD_RENDER_GDI)
        add_subdirectory(render-gdi)
    endif()
    if(SOUI_BUILD_TASKLOOP)
        add_subdirectory(TaskLoop)
    endif()
    # httpclient 与桌面共用 SOUI_BUILD_HTTPCLIENT 开关（wasm 下默认 OFF）：打开后走
    # components/httpclient/wasm 的同步 XHR 后端，不需要 libcurl / openssl。需要 HTTP
    # 能力的 wasm app 由自己的构建脚本 -DSOUI_BUILD_HTTPCLIENT=ON 打开。
    if(SOUI_BUILD_HTTPCLIENT)
        add_subdirectory(httpclient)
    endif()
    # Lua 脚本能力不再单独设 wasm 专属开关：与桌面 / 移动端一样由统一的
    # SOUI_BUILD_SCRIPT_LUA（默认 ON）控制。translator 由独立的 SOUI_BUILD_TRANSLATOR
    # 控制；二者已无编译期耦合——ScriptModule-LUA 里的 CreateTranslatorMgr 用
    # SOUI_BUILD_TRANSLATOR 编译宏包住，关掉 translator 时不会引用其符号。
    if(SOUI_BUILD_SCRIPT_LUA)
        add_subdirectory(ScriptModule-LUA)
    endif()
    if(SOUI_BUILD_TRANSLATOR)
        add_subdirectory(translator)
    endif()
    # ws / resprovider-zip 在 wasm 下各自走浏览器原生后端 / swinx 的 zlib_static，
    # 不依赖 third-part 的 libwebsockets / zlib，和其他平台一样由统一开关控制。
    if(SOUI_BUILD_WS)
        add_subdirectory(ws)
    endif()
    if(SOUI_BUILD_RES_ZIP)
        add_subdirectory(resprovider-zip)
    endif()
else()
    # 桌面 / Android / OHOS：默认全量，逐模块开关控制。

    # 图像解码 / 渲染 / 任务循环：默认全 ON。可按 app 需要分别关闭：
    #   IMGDECODER_STB —— 移动端必用；桌面可改用 wic / gdip 解码后端时关。
    #   RENDER_GDI     —— 非 Windows 由 swinx 提供；若 app 选用 render-skia 则多余，可关。
    #   TASKLOOP       —— 并非每个 app 都需要后台任务循环，可关。
    if(SOUI_BUILD_IMGDECODER_STB)
        add_subdirectory(imgdecoder-stb)
    endif()
    if(SOUI_BUILD_RENDER_GDI)
        add_subdirectory(render-gdi)
    endif()
    if(SOUI_BUILD_TASKLOOP)
        add_subdirectory(TaskLoop)
    endif()

    # Windows 专属图像 / 渲染后端（依赖系统 GDI / Direct2D）。
    if(CMAKE_SYSTEM_NAME MATCHES Windows)
        if(SOUI_BUILD_IMGDECODER_WIC)
            add_subdirectory(imgdecoder-wic)
        endif()
        if(SOUI_BUILD_IMGDECODER_GDIP)
            add_subdirectory(imgdecoder-gdip)
        endif()
        if(SOUI_BUILD_RENDER_D2D)
            add_subdirectory(render-d2d)
        endif()
    endif()

    if(SOUI_BUILD_LOG4Z)
        add_subdirectory(log4z)
    endif()
    if(SOUI_BUILD_HTTPCLIENT)
        add_subdirectory(httpclient)
    endif()
    if(SOUI_BUILD_RES_IPC)
        add_subdirectory(SIpcObject)
    endif()
    # resprovider-7zip 依赖 third-part 的 7z 目标；7z 不在 Android 上构建，故
    # Android 上 TARGET 7z 不存在，本组件自然跳过（与 ws 的 TARGET websockets 同套路）。
    if(SOUI_BUILD_RES_7ZIP AND TARGET 7z)
        add_subdirectory(resprovider-7zip)
    endif()
    if(SOUI_BUILD_RES_ZIP)
        add_subdirectory(resprovider-zip)
    endif()
    if(SOUI_BUILD_RENDER_SKIA)
        add_subdirectory(render-skia)
    endif()
    if(SOUI_BUILD_SCRIPT_LUA)
        add_subdirectory(ScriptModule-LUA)
    endif()
    if(SOUI_BUILD_TRANSLATOR)
        add_subdirectory(translator)
    endif()
    if(SOUI_BUILD_NETWORK)
        add_subdirectory(network)
    endif()
    # ws 依赖 third-part 的 websockets 目标；该第三方库未编入（SOUI_BUILD_WS=OFF
    # 或平台不支持）时自然跳过。
    if(SOUI_BUILD_WS AND TARGET websockets)
        add_subdirectory(ws)
    endif()
endif()

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/com-cfg.h" DESTINATION include/components)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/commask.h" DESTINATION include/components)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/commgr2.h" DESTINATION include/components)
