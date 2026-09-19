
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
)

# 把源码目录以字符串字面量编进代码，common.h 的 getSourceDir() 靠它定位 uires /
# soui-sys-resource，从而不依赖进程工作目录（CTest 每个用例的工作目录是
# build/demos/fun_test）。两边的 \" 转义是必需的，否则路径会被当成标识符撕开。
target_compile_definitions(fun_test PRIVATE
    SOUI_FUN_TEST_SOURCE_DIR=\"${SOUI_FUN_TEST_DIR}\")

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
    gtest_add_tests(TARGET fun_test
        SOURCES  ${SOUI_TEST_GUI_SRC}
        TEST_LIST SOUI_GUI_SMOKE_TESTS)
    soui_assert_group_not_empty(SOUI_GUI_SMOKE_TESTS)
    set_tests_properties(${SOUI_GUI_SMOKE_TESTS}
        PROPERTIES LABELS soui-gui TIMEOUT 30)
endif()

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
    add_dependencies(fun_test soui-sys-resource)
    target_link_libraries(fun_test Msimg32 imm32 Oleacc)
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
