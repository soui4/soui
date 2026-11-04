
###############################################################################
# fun_test —— SOUI 的 Google Test 测试工程
#
# 本文件负责三件事：
#   1) 收集本目录下的源文件，编译出唯一的可执行文件 fun_test；
#   2) 把可执行文件里的每个 TEST / TEST_F 注册成独立 CTest 条目，并按层级打标签；
#   3) 桌面平台上构建完成后自动跑一遍非交互用例（可用选项关闭）。
#
# 测试分层完全由【文件名约定】决定，新增用例只要按约定命名文件即可，不必改本文件：
#
#   test_integration_*.cpp        -> soui-integration  组件 / ZIP 资源提供器等集成测试
#   test_e2e_*.cpp                -> soui-e2e          无窗口端到端
#   其余 test_*.cpp               -> soui-unit         平台兼容层(swinx) / utilities 等单测
#   test_gui_*.cpp                -> soui-gui          需要真实桌面的窗口烟测（默认不注册）
#   test_gdi.cpp / test_soui.cpp  -> soui-interactive  需要人工操作的演示窗口（默认不注册）
#
# 标签（LABELS）分两层，一个用例可以同时带门禁标签和层级标签：
#   门禁标签  soui-headless                              = unit + integration + e2e
#   层级标签  soui-unit / soui-integration / soui-e2e    可按层单独跑
#   可选标签  soui-gui / soui-interactive                不带 soui-headless，
#                                                        所以打开 GUI 开关也不会污染门禁
#
# 常用命令：
#   跑门禁（全部无头用例）  ctest --test-dir build -L '^soui-headless$' --output-on-failure
#   只跑一层                ctest --test-dir build -L '^soui-unit$'
#   只跑一组用例            ctest --test-dir build -R '^swinx_region\.' --output-on-failure
#   查看注册结果（不执行）  ctest --test-dir build -N -L '^soui-headless$'
###############################################################################

# ---------------------------------------------------------------------------
# 0. wasm 腿：同一批用例编到 wasm 上，换 GDI 后端跑 A/B
# ---------------------------------------------------------------------------
# 下面那套桌面流程在 wasm 上有三条不成立，因此这里走一条独立分支并提前 return：
#   * 源集只能是子集。其余 TU 依赖 wasm 构建图里根本没有的能力 —— log4z / ws / network
#     等组件（见 components/CMakeLists.txt 的 SOUI_WASM_MINIMAL 白名单）、dlopen
#     （test_hook / test_module 的 LoadLibrary 路径）、pthread（test_thread / test_sync /
#     test_cs）、zlib 资源包（test_integration_*，wasm 下没有 resprovider-zip）、真窗口
#     与显示（test_gui_* / test_soui.cpp）。列进来是链接错误，不是运行期降级，所以按
#     能力剔除，并把"为什么没进来"写在下面。
#   * 不注册 CTest。宿主没有 wasm 测试启动器，ctest 起不了 .js；执行由
#     wasm/run-node.js（node + 假 DOM/canvas 桩）与产物自带的 .html（浏览器真 canvas）
#     两条腿承担，见 demos/fun_test/readme.md 的"wasm 腿"一节。
#   * 不跑 §7 的 POST_BUILD —— 那条命令会去 exec 一个 .html。
if(EMSCRIPTEN)
    # 只取 swinx 的 GDI / 文本 / 区域 / 句柄这一层：四个 TU 全是纯 Win32 兼容面调用
    # （CreateCompatibleDC、ExtTextOut、CombineRgn、GetGuiResources），不碰 SOUI 控件树，
    # 也不碰系统资源包，因此不需要 --preload-file。
    set(FUN_TEST_WASM_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/test_dc.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test_region.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test_text.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test_swinx_handle_type_unit.cpp)

    # 产物目录按 GDI 后端分开：两条腿的 .wasm 名字相同，混在一个目录里 A/B 会互相覆盖。
    # 后端变量由 swinx/wasm.cmake 拥有（swinx 先于本目录进入构建图，这里只读）。
    if(NOT DEFINED SWINX_WASM_GDI_BACKEND OR SWINX_WASM_GDI_BACKEND STREQUAL "")
        set(SWINX_WASM_GDI_BACKEND "canvas2d")
    endif()
    set(_fun_test_out "${CMAKE_CURRENT_SOURCE_DIR}/out.${SWINX_WASM_GDI_BACKEND}")

    set(SOUI_WASM_FUN_TEST_STACK "32MB" CACHE STRING "wasm fun_test: -sSTACK_SIZE")
    # 找缺陷的产物默认带可读断言：abort 时给出具体原因，而不是 "Aborted()"。
    option(SOUI_WASM_FUN_TEST_ASSERTIONS "wasm fun_test: link with -sASSERTIONS=2" ON)

    add_executable(fun_test ${FUN_TEST_WASM_SRC})
    target_include_directories(fun_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/utilities/include
        ${CMAKE_SOURCE_DIR}/third-part/gtest/include
        ${CMAKE_SOURCE_DIR}/swinx/include
        ${CMAKE_BINARY_DIR}/config)

    # swinx 是这层唯一的实现方：GDI 后端的差异全在 swinx 内部按 SWINX_WASM_GDI_BACKEND
    # 分派，本目标不带 soui4 —— 这几个用例只调 Win32 兼容面。
    target_link_libraries(fun_test PRIVATE gtest swinx utilities4 m)
    # cairo 后端：freetype 只 PRIVATE 链 brotlidec，而 brotli 的 CMakeLists 在 Emscripten
    # 下删掉了 dec -> common 这条链接边（见 swinx/wasm.cmake 同处的说明），所以最终链接
    # 必须显式带上 brotlicommon，否则 _kBrotliPrefixCodeRanges 等符号全 undefined。
    # canvas2d 分支不构建 brotli，TARGET 不存在时自然跳过。
    if(TARGET brotlicommon)
        target_link_libraries(fun_test PRIVATE brotlicommon)
    endif()
    target_compile_options(fun_test PRIVATE -Wall)

    set_target_properties(fun_test PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON
        OUTPUT_NAME "fun_test-wasm"
        # emcc 只有在输出名是 .html 时才生成浏览器外壳；.js 仍然单独可用，node 腿直接
        # require 它（先在 wasm/run-node.js 里装好假 DOM/canvas 桩）。
        SUFFIX ".html"
        RUNTIME_OUTPUT_DIRECTORY "${_fun_test_out}")
    foreach(_cfg IN ITEMS DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
        set_target_properties(fun_test PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${_cfg} "${_fun_test_out}")
    endforeach()

    target_link_options(fun_test PRIVATE
        # canvas2d 后端用 emscripten::val 持有离屏 canvas / 2D context，emval 运行时由
        # embind 提供；cairo 后端不引用这些符号，多带一份不改变产物。
        "-lembind"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sSTACK_SIZE=${SOUI_WASM_FUN_TEST_STACK}"
        "-sEXPORTED_FUNCTIONS=_main,_malloc,_free"
        # gtest 跑完 main 返回即退出：node 腿要拿到退出码，浏览器腿只是停止泵帧。
        "-sEXIT_RUNTIME=1"
        # 数据段必须落在 0x10000 以上，否则 swinx 的 IS_INTRESOURCE 会把串字面量当成
        # 原子号（同一件事的完整说明见 demos/wasm-demo/CMakeLists.txt 末尾）。
        "-Wl,--stack-first")
    if(SOUI_WASM_FUN_TEST_ASSERTIONS)
        target_link_options(fun_test PRIVATE "-sASSERTIONS=2")
    endif()

    message(STATUS "SOUI_WASM_FUN_TEST: gdi-backend=${SWINX_WASM_GDI_BACKEND}, out=${_fun_test_out}, "
                   "stack=${SOUI_WASM_FUN_TEST_STACK}, assertions=${SOUI_WASM_FUN_TEST_ASSERTIONS}")
    return()
endif()

# ---------------------------------------------------------------------------
# 1. 编译设置与头文件搜索路径
# ---------------------------------------------------------------------------
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_HAS_TR1_NAMESPACE -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)

file(GLOB_RECURSE CURRENT_HEADERS *.h *.hpp)

# ---------------------------------------------------------------------------
# 2. 源文件发现：编译集合 + 分层集合
# ---------------------------------------------------------------------------
# CONFIGURE_DEPENDS：新增/删除 *.cpp 后下一次构建会自动重新 glob，不必手动重跑 cmake。
# 所有分层集合都从 <本目录>/test_*.cpp 这个全集里切出来，保证"编译进二进制的文件"
# 与"注册成 CTest 条目的文件"这两个集合不会各自漂移。
set(SOUI_FUN_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB FUN_TEST_SRC        CONFIGURE_DEPENDS ${SOUI_FUN_TEST_DIR}/*.cpp)
file(GLOB SOUI_TEST_ALL_SRC   CONFIGURE_DEPENDS ${SOUI_FUN_TEST_DIR}/test_*.cpp)
file(GLOB SOUI_TEST_INTEG_SRC CONFIGURE_DEPENDS ${SOUI_FUN_TEST_DIR}/test_integration_*.cpp)
file(GLOB SOUI_TEST_E2E_SRC   CONFIGURE_DEPENDS ${SOUI_FUN_TEST_DIR}/test_e2e_*.cpp)
file(GLOB SOUI_TEST_GUI_SRC   CONFIGURE_DEPENDS ${SOUI_FUN_TEST_DIR}/test_gui_*.cpp)
file(GLOB SOUI_TEST_UITEST_SRC CONFIGURE_DEPENDS ${SOUI_FUN_TEST_DIR}/test_soui_uitest_*.cpp)

# 交互式演示用例：窗口打开后等待人工点击/关闭，程序无法自行判定结束点，因此单独成组
# 且默认不注册 —— 否则一次裸 `ctest` 会永远挂在这里。
set(SOUI_TEST_INTERACTIVE_SRC
    ${SOUI_FUN_TEST_DIR}/test_gdi.cpp
    ${SOUI_FUN_TEST_DIR}/test_soui.cpp
)

# 未被上面任何约定命中的 test_*.cpp 全部视为无头单元测试。这份名单是"全集减掉
# 已分类项"推导出来的，所以将来新增 test_foo.cpp 只要不撞上 integration/e2e/gui
# 命名，就会自动进入 soui-unit 并随门禁一起跑。
set(SOUI_TEST_UNIT_SRC ${SOUI_TEST_ALL_SRC})
list(REMOVE_ITEM SOUI_TEST_UNIT_SRC
    ${SOUI_TEST_INTEG_SRC}
    ${SOUI_TEST_E2E_SRC}
    ${SOUI_TEST_GUI_SRC}
    ${SOUI_TEST_UITEST_SRC}
    ${SOUI_TEST_INTERACTIVE_SRC}
)

# ---------------------------------------------------------------------------
# 3. 可执行文件
# ---------------------------------------------------------------------------
# 全部用例编进同一个 exe；分层只影响 CTest 注册，不影响编译产物。
add_executable(fun_test ${CURRENT_HEADERS} ${FUN_TEST_SRC})

# 头文件搜索路径。原先写成 include_directories(... PRIVATE ... PUBLIC ...)，但
# include_directories() 并不接受 PRIVATE/PUBLIC，这两个词会被当成相对目录名解析，
# 于是白白多出 <本目录>/PRIVATE、<本目录>/PUBLIC 两个不存在的 -I（那条
# <本目录>/include 也一直不存在）。改成标准的 target 形式，既去掉了噪音，
# 也把"这份 include 只作用于 fun_test"写明。
target_include_directories(fun_test PRIVATE
    ${SOUI_FUN_TEST_DIR}
    ${PROJECT_SOURCE_DIR}/utilities/include
    ${PROJECT_SOURCE_DIR}/third-part/gtest/include
    ${PROJECT_SOURCE_DIR}/third-part/Scintilla/include
    ${PROJECT_SOURCE_DIR}/components
    ${PROJECT_SOURCE_DIR}/SOUI/include
)

# 把源码目录以字符串字面量编进代码，common.h 的 getSourceDir() 靠它定位 uires /
# soui-sys-resource，从而不依赖进程工作目录（CTest 每个用例的工作目录是
# build/demos/fun_test）。两边的 \" 转义是必需的，否则路径会被当成标识符撕开。
target_compile_definitions(fun_test PRIVATE
    SOUI_FUN_TEST_SOURCE_DIR=\"${SOUI_FUN_TEST_DIR}\")

# ---------------------------------------------------------------------------
# 3.5 lua 脚本模块测试（test_lua.cpp）
# ---------------------------------------------------------------------------
# test_lua.cpp 用 SComMgr2 加载 scriptmodule-lua：静态 COM 构建（LIB_SOUI_COM）
# 直链 SCRIPT_LUA::SCreateInstance，DLL COM 构建运行时 LoadLibrary —— 两种模式
# 测试代码零差异，因此这里不设任何编译条件。测试 TU 不链接 lua 库、不 include
# lua 头：对象全部由脚本经 IScriptModule::executeScriptBuffer 创建。
#
# 唯一需要的注入是 demo uires 目录（LoadAnimation/LoadValueAnimator 用例要经
# FILE resprovider 加载 anim:/valueAni: 资源）。
target_compile_definitions(fun_test PRIVATE
    SOUI_FUN_TEST_DEMO_UIRES=\"${SOUI_FUN_TEST_DIR}/../demo/uires\")

# ---------------------------------------------------------------------------
# 4. CTest 注册：一个用例一条记录
# ---------------------------------------------------------------------------
# include(GoogleTest) 提供 gtest_add_tests()（CMake >= 3.9）。它在【配置期】用正则
# 扫描 SOURCES 列出的每个文件，取出 TEST / TEST_F / TEST_P 的 `套件.用例` 名，然后为
# 每个用例生成
#     add_test(NAME 套件.用例 COMMAND fun_test --gtest_filter=套件.用例)
# 于是失败粒度是单个用例，也能用 -R 精确复跑、-j 并行。TEST_LIST 把生成的名字列表
# 回写到调用者作用域，下面的 set_tests_properties 就是靠它给整组一起打标签。
#
# 已知代价（不是缺陷，是这种注册方式的固有属性）：
#   * 参数化用例（TEST_P / INSTANTIATE_TEST_SUITE_P）只能扫出模板，拆不出全部实例，
#     新增参数化用例后必须单独核验注册结果；
#   * 一个 exe 拆成 N 条 CTest 记录 => 每条各自起一个进程，比下面 POST_BUILD 的单进程
#     整跑慢，CI 里靠 -j 并行摊掉。
#   * 分组以文件为界：同一个套件若跨文件，可能分属不同层（例如 soui_component 既有
#     test_module.cpp 里的用例，也有 test_integration_resprovider.cpp 里的）。标签是
#     登记层级，不代表语义纯度。
include(GoogleTest)

# 空组守卫：某个约定没命中任何用例，说明该层正在【静默变成 0 个用例】——门禁看着是绿的，
# 实际少验了一层。这种事必须在配置期就炸掉，不能留到 CI 里"跑过了"。
function(soui_assert_group_not_empty group_var)
    if("${${group_var}}" STREQUAL "")
        message(FATAL_ERROR
            "fun_test: CTest group '${group_var}' is empty. "
            "Check the test file-name convention documented at the top of "
            "demos/fun_test/CMakeLists.txt.")
    endif()
endfunction()

# 无头单元测试（swinx 平台兼容层 / utilities / SOUI 内核无窗口单测）
gtest_add_tests(TARGET fun_test
    SOURCES  ${SOUI_TEST_UNIT_SRC}
    TEST_LIST SOUI_UNIT_TESTS)
soui_assert_group_not_empty(SOUI_UNIT_TESTS)
set_tests_properties(${SOUI_UNIT_TESTS}
    PROPERTIES LABELS "soui-headless;soui-unit" TIMEOUT 60)

# 集成测试（资源提供器 / 资源管理器 / 组件接口交界处）
gtest_add_tests(TARGET fun_test
    SOURCES  ${SOUI_TEST_INTEG_SRC}
    TEST_LIST SOUI_INTEGRATION_TESTS)
soui_assert_group_not_empty(SOUI_INTEGRATION_TESTS)
set_tests_properties(${SOUI_INTEGRATION_TESTS}
    PROPERTIES LABELS "soui-headless;soui-integration" TIMEOUT 60)

# 无窗口端到端（资源 -> 控件树 -> 状态）
gtest_add_tests(TARGET fun_test
    SOURCES  ${SOUI_TEST_E2E_SRC}
    TEST_LIST SOUI_E2E_TESTS)
soui_assert_group_not_empty(SOUI_E2E_TESTS)
set_tests_properties(${SOUI_E2E_TESTS}
    PROPERTIES LABELS "soui-headless;soui-e2e" TIMEOUT 60)

# 无头 UI 测试（SouiTest 框架：Tier A 框架级注入 SSendMessage，无需显示，归入门禁）
# 内部用例只允许 Tier A 注入；外部/保真对照用例见 test_gui_uitest_*（soui-gui 层）。
gtest_add_tests(TARGET fun_test
    SOURCES  ${SOUI_TEST_UITEST_SRC}
    TEST_LIST SOUI_UITEST_TESTS)
soui_assert_group_not_empty(SOUI_UITEST_TESTS)
set_tests_properties(${SOUI_UITEST_TESTS}
    PROPERTIES LABELS "soui-headless;soui-uitest" TIMEOUT 60)

# ---------------------------------------------------------------------------
# 5. 可选层：需要真实桌面 / 需要人工操作
# ---------------------------------------------------------------------------
# 这两组故意不带 soui-headless 标签，因此即使打开了开关，门禁命令也选不到它们。
# TIMEOUT 是每个用例的墙钟上限，用来把"挂死"变成"失败"，避免拖垮整条 CI。

# GUI 烟测：创建并显示真实宿主窗口、程序化投递鼠标消息、检查按钮事件；
# 不等待人工操作，但必须有显示（Linux 上靠 xvfb-run）。
option(SOUI_ENABLE_GUI_SMOKE
    "Register the SOUI native-window smoke test (needs a display; Xvfb on Linux)" OFF)
if(SOUI_ENABLE_GUI_SMOKE)
    add_definitions(-DSOUI_ENABLE_GUI_SMOKE)
    gtest_add_tests(TARGET fun_test
        SOURCES  ${SOUI_TEST_GUI_SRC}
        TEST_LIST SOUI_GUI_SMOKE_TESTS)
    soui_assert_group_not_empty(SOUI_GUI_SMOKE_TESTS)
    set_tests_properties(${SOUI_GUI_SMOKE_TESTS}
        PROPERTIES LABELS soui-gui TIMEOUT 30)
endif()

# 容器控件（combobox / tabctrl）无头用例已并入常规 soui-uitest 门禁：
# SouiTest 夹具在 SetUp 中调用 HeadlessApp::LoadSystemResource() 加载系统资源
# soui-sys-resource，内置皮肤 _skin.sys.dropbtn / _skin.sys.tab.page /
# _skin.sys.tab.header 均已就绪，因此 SComboBox / STabCtrl 可直接在无头环境下创建，
# 无需额外开关或专属注册。HeadlessApp 本身仍保持极简（仅 scrollbar），以免改变
# check_sync / rubberband 等既有用例刻意依赖的固定几何度量。

# 交互式演示：window.soui / window.gdi 会开窗并等待人工操作，没有时间上界，
# 所以 TIMEOUT 0（= 不设超时）。仅供人工手动跑。
option(SOUI_ENABLE_INTERACTIVE_TESTS
    "Register the interactive window.* demos (test_gdi.cpp / test_soui.cpp), which wait for a human" OFF)
if(SOUI_ENABLE_INTERACTIVE_TESTS)
    gtest_add_tests(TARGET fun_test
        SOURCES  ${SOUI_TEST_INTERACTIVE_SRC}
        TEST_LIST SOUI_INTERACTIVE_TESTS)
    soui_assert_group_not_empty(SOUI_INTERACTIVE_TESTS)
    set_tests_properties(${SOUI_INTERACTIVE_TESTS}
        PROPERTIES LABELS soui-interactive TIMEOUT 0)
endif()

# ---------------------------------------------------------------------------
# 6. 依赖与链接
# ---------------------------------------------------------------------------
add_dependencies(fun_test gtest soui4 Scintilla ${COM_LIBS})
target_link_libraries(fun_test gtest soui4 Scintilla)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    # Windows 宿主直接跑真实 Win32 API，只需要带上系统资源与系统库。
    # 后面这几个 .lib 是 swinx 在其它平台上自己实现的那批 Win32 入口在 Windows 上的
    # 归属模块：wke 兼容性用例（usp10/shlwapi/version/crypt32/iphlpapi）在 Windows 上
    # 走系统实现，所以必须显式链接，否则 fun_test 链接不过。
    add_dependencies(fun_test soui-sys-resource)
    target_link_libraries(fun_test Msimg32 imm32 Oleacc usp10 shlwapi version crypt32 iphlpapi)
else()
    # 非 Windows 平台由 swinx 提供 Win32 API；dl 供 LoadLibrary 系列用例使用。
    add_dependencies(fun_test swinx)
    target_link_libraries(fun_test swinx pthread dl)
    # SApplication 即使只跑无窗口 E2E 也会初始化 RichEdit 服务。msftedit 目标是否
    # 存在取决于平台与构建选项，所以用 if(TARGET) 兜住，避免 "target not found"。
    if(TARGET msftedit)
        add_dependencies(fun_test msftedit)
    endif()
endif()

if(SOUI_ENABLE_COM_LIB)
    target_link_libraries(fun_test ${COM_LIBS} ${COM_DEP_LIBS})
endif()

set_target_properties(fun_test PROPERTIES
    FOLDER demos
)

# ---------------------------------------------------------------------------
# 7. 构建后自动跑一遍（桌面平台默认开启）
# ---------------------------------------------------------------------------
# 本地默认走这条路：单进程一次跑完全部非交互用例，比 CTest 的逐用例起进程快得多。
# CI 用 -DSOUI_FUN_TEST_POST_BUILD=OFF 关掉它，改跑 ctest -L '^soui-headless$'，
# 以便拿到逐用例的粒度与并行度。
# 两条路覆盖面一致（都是"除 window.* 外的全部用例"）：-window.* 排除交互式与需要
# 桌面的 window 套件（window.soui / window.gdi / window.gui_smoke_*）。
# 测试出现非零退出（有用例失败）会让构建失败，便于第一时间发现回归。
# iOS / OHOS 的交叉编译目标是移动设备，不在本机执行。
option(SOUI_FUN_TEST_POST_BUILD "Run the non-interactive fun_test suite after building it" ON)
if(SOUI_FUN_TEST_POST_BUILD AND NOT CMAKE_SYSTEM_NAME MATCHES "Windows|iOS" AND NOT IS_OHOS)
    add_custom_command(TARGET fun_test POST_BUILD
        COMMAND $<TARGET_FILE:fun_test> --gtest_filter=-window.* --gtest_color=yes
        COMMENT "fun_test: running the non-interactive suite (window.* excluded)..."
        VERBATIM
    )
endif()
