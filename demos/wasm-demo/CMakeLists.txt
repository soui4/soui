# =============================================================================
# demos/wasm-demo/CMakeLists.txt -- wasm 演示的构建入口
#
# 只在 Emscripten 下、且根 CMakeLists 打开 SOUI_BUILD_WASM_DEMO 时进入。
# 目录原先叫 soui-wasm/（在仓库根下），现在挪到 demos/wasm-demo/，深度多了一层；
# 下面所有指向仓库根的路径都用 CMAKE_SOURCE_DIR，不要用 PROJECT_SOURCE_DIR ——
# project() 会把 PROJECT_SOURCE_DIR 重置成本目录，指向根的那些路径就全错了。
#
# 这个文件存在的理由：演示的链接命令原先写在 build-soui.sh 里，用了 bash 数组、
# sed、find 与进程替换，Windows 主机上根本没有这些工具——同一套移植在 Windows
# 上就出不了包。搬进 CMake 之后三平台都是同两条命令：
#
#   emcmake cmake -S <soui4> -B <build> -G Ninja -DSOUI_BUILD_WASM_DEMO=ON
#   cmake --build <build>
#
# 链接期的五件事都在这里：
#   1. 静态库组：浏览器没有 dlopen，soui4 / 组件 / 扩展控件 / Scintilla / richedit
#      全都要链进同一个模块；swinx 与 soui4 互相引用，靠 CMake 的静态循环展开。
#   2. 资源预载：uires/ 与 soui-sys-resource/ 用 --preload-file 挂进 MEMFS，
#      程序里按 /opt/soui/* 读（SResProviderFiles，不依赖 resprovider-zip）。
#   3. 字体：GDI 后端由 SWINX_WASM_GDI_BACKEND 选择（canvas2d 默认 / cairo）。
#      canvas2d 的文本走浏览器 Canvas 2D 的 fillText + 系统 CJK 字体，不打包字体；
#      cairo 走 freetype/fontconfig，必须预载字体到 /opt/soui/fonts（SOUI_DEMO_FONT
#      指定的文件，目的名固定 demo.ttf，与 soui_main.cpp 的 kFontPath 一致）。
#   4. 内存布局：显式 --stack-first（见文件末尾的说明，-O1 以上默认不是这个布局）。
#   5. 运行时形状：MODULARIZE、内存增长、32 MB 栈（SOUI_DEMO_STACK 覆盖）、
#      以及页面外壳要用的 SwinxDemo* / SwinxFb* 导出符号。
# =============================================================================
project(wasm-demo)
# CMAKE_SOURCE_DIR 才是仓库根（本文件由根 CMakeLists 的 add_subdirectory 进入）。

set(SOUI_WASM_DEMO_OUT "${CMAKE_CURRENT_SOURCE_DIR}/out" CACHE PATH
    "wasm demo: output directory for soui-wasm-demo.{html,js,wasm,data}")

# Emscripten 默认给 5 MB 栈；SOUI 的绘制链是 render-gdi -> swinx GDI（route C：
# 浏览器 Canvas 2D），帧缓冲/控件树的递归深度按桌面线程栈写的，5 MB 下直接踩爆（症状是
# SHostWnd::Create() 返回 NULL 或 "Stack overflow! Stack cookie has been overwritten"）。
set(SOUI_DEMO_STACK "32MB" CACHE STRING "wasm demo: -sSTACK_SIZE")

set(SOUI_DEMO_FONT "" CACHE FILEPATH
    "wasm demo: TTF/TTC preloaded as /opt/soui/fonts/demo.ttf (empty = auto-detect)")

# 字体候选顺序按"单个文件要同时覆盖拉丁与汉字"排：
#   微软雅黑 / 苹方 / Noto Sans CJK 两套都有；
#   DroidSansFallbackFull 名字里带 "Sans" 但 cmap 里一个 ASCII 都没有（纯汉字兜底），
#   DejaVuSans / Arial 反过来只有拉丁（中文变空白）——都排到最后。
# 注册成功不等于字形够用：面板上的 font 位只反映 FcConfigAppFontAddFile 的返回值。
if(SOUI_DEMO_FONT)
    set(_soui_demo_font "${SOUI_DEMO_FONT}")
elseif(DEFINED ENV{SOUI_DEMO_FONT} AND EXISTS "$ENV{SOUI_DEMO_FONT}")
    set(_soui_demo_font "$ENV{SOUI_DEMO_FONT}")
else()
    if(CMAKE_HOST_WIN32)
        # $ENV{WINDIR} 在个别精简环境下为空，所以 SystemRoot 与盘符路径都列一遍。
        set(_soui_demo_font_candidates
            "$ENV{WINDIR}/Fonts/msyh.ttc"
            "$ENV{SystemRoot}/Fonts/msyh.ttc"
            "C:/Windows/Fonts/msyh.ttc"
            "$ENV{WINDIR}/Fonts/simsun.ttc"
            "$ENV{SystemRoot}/Fonts/simsun.ttc"
            "C:/Windows/Fonts/simsun.ttc"
            "$ENV{WINDIR}/Fonts/Deng.ttf"
            "$ENV{SystemRoot}/Fonts/Deng.ttf"
            "$ENV{WINDIR}/Fonts/arial.ttf"
            "$ENV{SystemRoot}/Fonts/arial.ttf")
    # 必须是 CMAKE_HOST_APPLE：Emscripten 交叉编译时 CMAKE_SYSTEM_NAME=Emscripten，
    # 目标变量 APPLE 为假，用它会让 macOS 主机跳过下面的候选、落到 Linux 字体路径。
    elseif(CMAKE_HOST_APPLE)
        set(_soui_demo_font_candidates
            "/System/Library/Fonts/PingFang.ttc"
            "/System/Library/Fonts/STHeiti Medium.ttc"
            "/System/Library/Fonts/Supplemental/Arial Unicode.ttf"
            "/Library/Fonts/Arial Unicode.ttf")
    else()
        set(_soui_demo_font_candidates
            /usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc
            /usr/share/fonts/opentype/noto/NotoSansCJKsc-Regular.otf
            /usr/share/fonts/opentype/adobe/SourceHanSansCN-Regular.otf
            /usr/share/fonts/truetype/arphic/uming.ttc
            /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf
            /usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf)
    endif()
    foreach(_cand IN LISTS _soui_demo_font_candidates)
        if(EXISTS "${_cand}")
            set(_soui_demo_font "${_cand}")
            break()
        endif()
    endforeach()
endif()

foreach(_req
        "${CMAKE_CURRENT_SOURCE_DIR}/uires/uires.idx"
        "${CMAKE_SOURCE_DIR}/soui-sys-resource/uires.idx")
    if(NOT EXISTS "${_req}")
        message(FATAL_ERROR
            "wasm demo: 缺少 ${_req}（应用资源或系统资源目录没检出？资源目录里必须有 uires.idx）")
    endif()
endforeach()

# --preload-file 的目标路径要和 soui_main.cpp 里的 kAppResDir/kSysResDir/kFontPath 一致。
# 宿主机路径一律用 CMake 给出的正斜杠形式：emcc 的 file_packager 按 '@' 切分，
# 反斜杠会在 Windows 上被当成转义。
#
# uires 逐个条目预载（而不是整目录一次）：SOUI_BUILD_WASM_LUA=OFF 时要把
# uires/lua 从 .data 里摘掉。uires.idx 仍然列着 <lua> 组，SResProviderFiles 找不到
# 文件也只是取不到脚本——而这时脚本工厂本身就没编进来，SHostWnd 解析 <script> 时
# CreateScriptModule() 直接返回 E_FAIL，那条索引永远不会被读到。
# GDI 后端选择：与 swinx/wasm.cmake 的 SWINX_WASM_GDI_BACKEND 同源（cache 变量，
# 命令行 -DSWINX_WASM_GDI_BACKEND=cairo 时整条链——swinx、本演示、cnchess——统一切换）。
# canvas2d（默认）：文本走浏览器 Canvas 2D + 系统 CJK 字体，不打包字体；
# cairo：软件光栅 + freetype/fontconfig，必须预载字体到 MEMFS 的 /opt/soui/fonts。
if(NOT DEFINED SWINX_WASM_GDI_BACKEND OR SWINX_WASM_GDI_BACKEND STREQUAL "")
    set(SWINX_WASM_GDI_BACKEND "canvas2d" CACHE STRING
        "wasm GDI backend: canvas2d (browser font) | cairo (bundled font)" FORCE)
endif()
set(_soui_demo_gdi_backend "${SWINX_WASM_GDI_BACKEND}")

set(_soui_demo_preload "")
file(GLOB _soui_uires_items "${CMAKE_CURRENT_SOURCE_DIR}/uires/*")
foreach(_item IN LISTS _soui_uires_items)
    get_filename_component(_item_name "${_item}" NAME)
    if(_item_name STREQUAL "lua" AND NOT SOUI_BUILD_WASM_LUA)
        continue()
    endif()
    list(APPEND _soui_demo_preload
        "--preload-file=${_item}@/opt/soui/uires/${_item_name}")
endforeach()
list(APPEND _soui_demo_preload
    "--preload-file=${CMAKE_SOURCE_DIR}/soui-sys-resource@/opt/soui/soui-sys-resource")
# 音效：uires/lua/games.lua 按 GetAppDir() .. "/snd/" 拼路径，而 soui_main.cpp 把
# app dir 设成 /opt/soui，所以这里必须挂成 /opt/soui/snd。只在编译了 Lua 时预载——
# 没有脚本工厂就没有游戏，预载进来只是白占 .data（6 个 wav 共约 140 KB）。
if(SOUI_BUILD_WASM_LUA)
    list(APPEND _soui_demo_preload
        "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/snd@/opt/soui/snd")
endif()
if(_soui_demo_font)
    if(_soui_demo_gdi_backend STREQUAL "cairo")
        # cairo 后端：字体走 freetype/fontconfig，必须预载进 MEMFS 并由 soui_main.cpp
        # 的 AddFontResource(kFontPath=/opt/soui/fonts/demo.ttf) 注册。目的文件名固定
        # 为 demo.ttf（与 kFontPath 一致），源文件名无所谓。
        # canvas2d 后端则忽略（文本走浏览器系统字体），不应打包字体白占 .data。
        # 源路径统一成正斜杠：emcc 的 file_packager 按 '@' 切分，反斜杠在 Windows 上
        # 会被当成转义（见本文件顶部 --preload-file 说明）。
        file(TO_CMAKE_PATH "${_soui_demo_font}" _soui_demo_font_fwd)
        list(APPEND _soui_demo_preload
            "--preload-file=${_soui_demo_font_fwd}@/opt/soui/fonts/demo.ttf")
        message(STATUS "SOUI_WASM_DEMO: GDI=${_soui_demo_gdi_backend}, 打包字体 -> /opt/soui/fonts/demo.ttf")
    else()
        message(STATUS "SOUI_WASM_DEMO: GDI=${_soui_demo_gdi_backend}, 不打包字体（浏览器系统字体）。")
    endif()
elseif(_soui_demo_gdi_backend STREQUAL "cairo")
    # cairo 后端要求字体，但本机没有检测到任何候选 TTF/TTC；cairo 没有字体就画不出
    # 任何字形（AddFontResource 直接失败）。这里直接报错，避免打出一个"能跑但全无文字"
    # 的包。
    message(FATAL_ERROR
        "SOUI_WASM_DEMO: GDI 后端为 cairo 但没有任何字体可打包。请通过 "
        "-DSOUI_DEMO_FONT=<ttf/ttc 路径> 指定一个同时覆盖拉丁与 CJK 的字体文件。")
endif()

# 导出表：页面外壳（soui.html）需要的全部符号。
#   _SwinxDemoPump/_SwinxDemoMouse/_SwinxDemoKey/_SwinxDemoText
#     -- 消息泵与输入注入（键盘按物理键位、鼠标按客户区坐标、IME 按码点）。
#   _SwinxDemoCaretPos -- 页面把 IME 代理框钉在 SOUI 的插入符上，候选词窗才会
#     跟着光标走。
#   _malloc/_free -- 页面要把字符串和 POINT 写进线性内存再传指针。
#   _SwinxFb* -- 帧缓冲地址/尺寸/stride 与上屏计数（canvas 侧读像素）。
# 这份列表是 -sEXPORTED_FUNCTIONS 的唯一真相：漏一项，页面调用就是
# "m._SwinxDemoXxx is not a function" 直接抛异常。
#   _SwinxClipboardSetBrowserText -- 浏览器 paste 事件把剪贴板文本写进 swinx 的
#     SClipboard（CF_UNICODETEXT/CF_TEXT 双格式），随后的 Ctrl+V 注入才能读到。
#   _SwinxDemoFilePickSelfTest -- 异步文件选择的分阶段探针（0 发起 / 1 取结果 /
#     2 负路径）；页面上那颗「选择文件」按钮只调阶段 0，结果由回调打进页面日志。
#   _SwinxDemoFilePickPath -- 探针上一次拿到的路径，烟测要逐字符钉住它（pOut 只有整数）。
#   _SwinxFilePickDone -- 页面把选择结果送回 wasm 的唯一重入口（符号在 swinx 的
#     src/platform/wasm/dlghelper.cpp 里，不是演示专有）。漏了这一项，宿主钩子
#     选完文件也只能报 "Module._SwinxFilePickDone is not a function"。
#   _SwinxFilterToAcceptA -- swinx 的筛选串换算，烟测直接拿它对 accept 串做断言。
set(_soui_demo_exports
    "_main,_SwinxDemoPump,_SwinxDemoMouse,_SwinxDemoWheel,_SwinxDemoKey,_SwinxDemoText,_SwinxDemoCaretPos,_SwinxClipboardSetBrowserText,_SwinxDemoTaskLoopSelfTest,_SwinxDemoFilePickSelfTest,_SwinxDemoFilePickPath,_SwinxFilePickDone,_SwinxFilterToAcceptA,_malloc,_free,_SwinxFbBits,_SwinxFbWidth,_SwinxFbHeight,_SwinxFbStride,_SwinxFbLogicalWidth,_SwinxFbLogicalHeight,_SwinxFbRequests,_SwinxFbPresents")

add_executable(soui-wasm-demo soui_main.cpp)

set_target_properties(soui-wasm-demo PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    # emcc 只有在输出名是 .html 时才套 --shell-file；Emscripten 的 CMake 工具链
    # 默认给可执行文件加 .js 后缀，这里换成 .html，.js/.wasm/.data 由 emcc 一并产出。
    SUFFIX ".html"
    RUNTIME_OUTPUT_DIRECTORY "${SOUI_WASM_DEMO_OUT}")

# 多配置生成器（VS / Ninja Multi-Config）会在输出目录下再套一层 $(Configuration)，
# 这里把四种配置都钉到同一层，产物路径与烟测/文档保持一致。
foreach(_cfg IN ITEMS DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set_target_properties(soui-wasm-demo PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${_cfg} "${SOUI_WASM_DEMO_OUT}")
endforeach()

# 演示本身按 -O0 编译：链接是整模块优化，把 O0 留在这一层，出问题时行号可读。
# 帧缓冲尺寸从 swinx/wasm.cmake 的缓存变量来（那个尺寸已经烘进 libswinx.a 的
# .bss，两侧宏值必须一致；运行时仍走 SwinxFbWidth()/SwinxFbHeight()）。
target_compile_options(soui-wasm-demo PRIVATE -O0 -g -Wall)
target_compile_definitions(soui-wasm-demo PRIVATE
    SWINX_FB_WIDTH=${SWINX_FB_WIDTH}
    SWINX_FB_HEIGHT=${SWINX_FB_HEIGHT}
    # 宿主用这两个宏决定：装不装脚本工厂、页面上的提示语写什么。
    # 值必须和下面的链接列表同源。
    SOUI_WASM_HAS_LUA=$<BOOL:${SOUI_BUILD_WASM_LUA}>
    SOUI_WASM_HAS_SCINTILLA=$<BOOL:${SOUI_BUILD_WASM_SCINTILLA}>)

target_include_directories(soui-wasm-demo PRIVATE
    ${CMAKE_SOURCE_DIR}/utilities/include
    ${CMAKE_SOURCE_DIR}/SOUI/include
    ${CMAKE_SOURCE_DIR}/components
    ${CMAKE_SOURCE_DIR}/controls.extend
    ${CMAKE_SOURCE_DIR}/swinx/include
    ${CMAKE_SOURCE_DIR}/swinx/src/platform/wasm
    ${CMAKE_BINARY_DIR}/config)

# 静态库之间的循环（soui4 调 Win32 API、swinx 的回调又进 SOUI）交给 CMake 的
# 静态循环展开处理：它会把库按依赖图重排、必要时重复列几遍。不要在这里手写
# -Wl,--start-group/-Wl,--end-group——CMake 不把这类标记放进库列表的位置，组不成立。
#
# route C：GDI 后端基于浏览器 Canvas 2D，不再链接 freetype/fontconfig/cairo/pixman，
# 因此也不再需要它们间接拉起的 brotli（freetype 才 PRIVATE 链 brotlidec，brotli 在
# Emscripten 下又删掉 dec -> common 这条边）。wasm 端字体由浏览器系统字体提供。
target_link_libraries(soui-wasm-demo PRIVATE
    soui4 utilities4 swinx taskloop
    render-gdi imgdecoder-stb aupng
    ExtendCtrls msftedit msls31
    m)
# cairo 后端（wasm_cairo 分支）：freetype PRIVATE 链 brotlidec，而 brotli 自己的
# CMakeLists 在 Emscripten 下刻意删掉了 dec -> common 的链接边（见
# swinx/thirdparty/brotli/CMakeLists.txt 的 BROTLI_EMSCRIPTEN），所以最终链接必须
# 显式带上 brotlicommon，否则 libbrotlidec 的 _kBrotliPrefixCodeRanges 等符号全部
# undefined。canvas2d 分支不构建 brotli，TARGET 不存在时自然跳过。
if(TARGET brotlicommon)
    target_link_libraries(soui-wasm-demo PRIVATE brotlicommon)
endif()
# Scintilla 是被 ExtendCtrls 里的 SScintillaView 引用的静态库，开关关掉时那个 .cpp
# 不参与编译（见 controls.extend/CMakeLists.txt），这里也不列——否则链接会找不到目标。
if(SOUI_BUILD_WASM_SCINTILLA)
    target_link_libraries(soui-wasm-demo PRIVATE Scintilla)
endif()
# Lua 三个库是一组：lua-54 是解释器，scriptmodule-lua 是 SOUI 的适配层，
# translator 被脚本导出表里的 CreateTranslatorMgr 直接引用（缺了是未定义符号，
# 不是运行时才失败）。
if(SOUI_BUILD_WASM_LUA)
    target_link_libraries(soui-wasm-demo PRIVATE
        scriptmodule-lua lua-54 translator)
endif()

option(SOUI_WASM_LINK_DEBUG
       "链接期带 -g（保留 wasm name 段与 DWARF，把 trap 的 wasm-function[N] 翻成符号；体积会涨）"
       OFF)
if(SOUI_WASM_LINK_DEBUG)
    # wasm-ld 之后 emcc 还会跑 binaryen 重排函数布局，link map 的偏移对不上运行期 pc，
    # 且 binaryen 会丢掉 name 段。链接期给 -O0 就绕过 binaryen，wasm-ld 的 name 段
    # 原样留下 —— V8 的栈帧直接显示函数名，link map 的偏移也与运行期一致。
    # Emscripten.cmake 的 Release 链接 flags 里带了一个裸 `-s`（strip），它排在
    # target_link_options 之前，把 wasm-ld 的 name 段剥掉了 —— 于是 trap 的栈只剩
    # `wasm-function[N]`。这里在它后面补 `-g`（emcc 里 -s/-g 后者胜）把名字要回来。
    # -O0 让 emcc 跳过 binaryen 后置优化，函数布局与运行时 pc 一致。
    target_link_options(soui-wasm-demo PRIVATE "-O0" "-g"
        "-Wl,-Map=${SOUI_WASM_DEMO_OUT}/soui-wasm-demo.map")
endif()

# 排查运行期 abort（页面报 "Aborted(). Build with -sASSERTIONS for more info."）
# 时打开：abort 会携带具体断言/异常文本。定位完记得关回去（产物更大更慢）。
option(SOUI_WASM_ASSERTIONS "链接期带 -sASSERTIONS=2，abort 时打印具体原因" OFF)
if(SOUI_WASM_ASSERTIONS)
    target_link_options(soui-wasm-demo PRIVATE "-sASSERTIONS=2")
endif()

# ASAN 与 -Wl,--stack-first 不兼容；ASAN 检查构建固定 -O0，而 -O0 下 emcc 默认
# 布局就是 stack-first，省略该标志语义不变。
# 注意：必须在下方 target_link_options 引用之前赋值——CMake 顺序执行，写在后面
# 的话 ${_soui_demo_stack_first} 展开时还是空，--stack-first 就不会进链接命令，
# 数据段落回 --global-base=1024，swinx 的 lowmem 自检会在运行期 abort。
string(FIND "${CMAKE_CXX_FLAGS}" "-fsanitize=address" _soui_asan_pos)
if(_soui_asan_pos GREATER -1)
    set(_soui_demo_stack_first "")
else()
    set(_soui_demo_stack_first "-Wl,--stack-first")
endif()

# -pthread 打开时的三条链接期设置（-sWASMFS=1 / -sINITIAL_MEMORY=64MB /
# GrowableArrayBuffers）在根 CMakeLists 的 SOUI_WASM_PTHREADS 分支里用
# add_link_options 统一给，原因和实测数据记在那边：cnchess 也要能开这个开关，
# 放在单个 demo 里就会漏。

target_link_options(soui-wasm-demo PRIVATE
    # route C（canvas2d）：GDI 后端用 emscripten::val 持有离屏 canvas / 2D context，
    # EM_JS/val 的运行时（_emval_incref/decref/invoke/new_cstring…）由 embind 提供，
    # 不链接 -lembind 会在最终 wasm 链接期报一堆 _emval_* undefined。
    "-lembind"
    "-sMODULARIZE=1"
    "-sEXPORT_NAME=createSouiDemo"
    "-sALLOW_MEMORY_GROWTH=1"
    "-sSTACK_SIZE=${SOUI_DEMO_STACK}"
    "-sEXPORTED_RUNTIME_METHODS=HEAPU32,HEAP32,HEAPU8"
    "-sEXPORTED_FUNCTIONS=${_soui_demo_exports}"
    "--shell-file=${CMAKE_CURRENT_SOURCE_DIR}/soui.html"
    # 数据段必须落在 0x10000 以上。swinx 判断"这个 LPCTSTR 是字符串还是
    # MAKEINTRESOURCE 整数"用的是 Win32 的 IS_INTRESOURCE（(ULONG_PTR)p >> 16 == 0），
    # 桌面平台上任何真实指针都远高于 64 KB，这个判据成立；而 wasm-ld 在
    # --no-stack-first 布局下把数据段放在 --global-base=1024，串字面量会落进
    # 低 64 KB，于是 "HOSTWND" 被当成原子号、GetAtomNameA 查不到、CreateWindowExA
    # 直接返回 0。症状是 SHostWnd::Create() 失败且 GetLastError()=0。
    # emcc 在 -O0 下默认就是 --stack-first（数据段排在栈之后），-O1 及以上换成
    # --no-stack-first，所以这里显式钉住，避免链接优化级别决定运行结果。
    # 同一份 .o、同一批 .a 的实测：-O2 不钉 → 26 项烟测全挂；-O2 钉住 → 全过；
    # -O0 再加 -Wl,--no-stack-first → 复现失败。swinx 侧的根治方式见
    # doc/soui-wasm-port-plan.md（需要改子模块，未在本轮改动内）。
    # ASAN 与 --stack-first 不兼容（em++ 直接报错）；ASAN 检查构建固定用 -O0，
    # 而 -O0 下 emcc 默认就是 --stack-first，语义不变，直接省略。
    ${_soui_demo_stack_first}
    ${_soui_demo_preload})

# 资源与 shell 变了要重链：--preload-file 的内容是在链接期打包的，但 CMake 不知道
# 这些文件参与了链接（它们不是 target 的源文件），不声明就会拿到上一次的 .data。
file(GLOB_RECURSE _soui_demo_res
    "${CMAKE_CURRENT_SOURCE_DIR}/uires/*"
    "${CMAKE_CURRENT_SOURCE_DIR}/snd/*"
    "${CMAKE_SOURCE_DIR}/soui-sys-resource/*")
set(_soui_demo_link_deps
    ${_soui_demo_res}
    "${CMAKE_CURRENT_SOURCE_DIR}/soui.html"
    "${CMAKE_CURRENT_SOURCE_DIR}/soui-host.js")
# cairo 后端时字体也参与链接期打包，改字体要重链（否则拿到上一份 .data）。
if(_soui_demo_font AND _soui_demo_gdi_backend STREQUAL "cairo")
    list(APPEND _soui_demo_link_deps "${_soui_demo_font_fwd}")
endif()
set_property(TARGET soui-wasm-demo APPEND PROPERTY LINK_DEPENDS
    ${_soui_demo_link_deps})

# 页面外壳的宿主逻辑在 soui-host.js 里（可被别的 wasm 程序复用，不限于本演示）；
# emcc 只产出 .html/.js/.wasm/.data 四个文件，这一个得单独拷进产物目录。
# 放进 LINK_DEPENDS 是为了"只改了 js 也触发一次重链"——重链顺带跑下面的 POST_BUILD，
# 否则改完 js 刷新页面看到的还是上一份。
add_custom_command(TARGET soui-wasm-demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/soui-host.js"
            "${SOUI_WASM_DEMO_OUT}/soui-host.js"
    VERBATIM)

message(STATUS "SOUI_WASM_DEMO: ${SWINX_FB_WIDTH}x${SWINX_FB_HEIGHT}, stack=${SOUI_DEMO_STACK}, "
               "gdi-backend=${_soui_demo_gdi_backend}, "
               "font=${_soui_demo_gdi_backend} -> ${SOUI_WASM_DEMO_OUT}")
message(STATUS "SOUI_WASM_DEMO features: lua=${SOUI_BUILD_WASM_LUA} scintilla=${SOUI_BUILD_WASM_SCINTILLA}")
