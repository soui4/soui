cmake_minimum_required(VERSION 3.16)
project(SOUI4)

# Register the project's repeatable unit, integration and core E2E checks.
# Test targets are added by their own CMakeLists.txt files below.
enable_testing()

file(STRINGS  SOUI/include/soui-version.h ver_h_contents REGEX "^#define SOUI_VER[0-9]+ [0-9]+$")

foreach(line IN LISTS ver_h_contents) 
    string(REGEX MATCH "SOUI_VER([0-9]+) ([0-9]+)" match "${line}")
    if(match)
        set(SOUI_VER${CMAKE_MATCH_1} ${CMAKE_MATCH_2})
    endif()
endforeach()

message(STATUS "SOUI_VER: ${SOUI_VER1}.${SOUI_VER2}.${SOUI_VER3}.${SOUI_VER4}")
message(STATUS "install prefix=${CMAKE_INSTALL_PREFIX}")
set(CMAKE_CONFIGURATION_TYPES Debug Release)
if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "build type is ${CMAKE_BUILD_TYPE}")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/__cmake/")
include(__cmake/internal_utils.cmake)

set(BUILD_FOR_ARM OFF)
detectArmTarget(BUILD_FOR_ARM)
message(STATUS "BUILD_FOR_ARM: ${BUILD_FOR_ARM}")

if (CMAKE_SYSTEM_NAME STREQUAL "MSYS")
    message(STATUS "Overriding incorrect Linux detection for MSYS")
    set(CMAKE_SYSTEM_NAME Windows CACHE STRING "Target system" FORCE)
endif()
if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND MINGW)
    message(STATUS "Overriding incorrect Linux detection for MINGW")
    set(CMAKE_SYSTEM_NAME Windows CACHE STRING "Target system" FORCE)
endif()
if (CMAKE_SYSTEM_NAME MATCHES "OHOS|OpenHarmony|HarmonyOS")
    set(IS_OHOS ON)
else()
    set(IS_OHOS OFF)
endif()
message(STATUS "IS_OHOS: ${IS_OHOS}")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
if (CMAKE_SYSTEM_NAME MATCHES Windows)
IF (CMAKE_CL_64)
  set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/output_64 CACHE PATH "default install prefix" FORCE)
ELSE (CMAKE_CL_64)
  set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/output_32 CACHE PATH "default install prefix" FORCE)
ENDIF (CMAKE_CL_64)
else()
set (CMAKE_INSTALL_PREFIX "/usr/local/soui5" CACHE PATH "default install prefix" FORCE)
endif()
endif()

if (CMAKE_SYSTEM_NAME MATCHES Windows)
  option(SOUI_USE_UNICODE "Use Unicode" ON)
else()
  option(SOUI_USE_UNICODE "Use Unicode" OFF)
endif()
if (NOT MSVC AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-D_DEBUG)
endif()
# When other libraries are using a shared version of runtime libraries,
# Google Test also has to use one.
option(SOUI_SHARED_CRT "Use shared crt runtime library." ON)

# When other libraries are using a shared version of runtime libraries,
# Google Test also has to use one.
option(SOUI_WCHAR_AS_DEFAULT "Use wchar_t as internal type" ON)
#
#
option(SOUI_ENABLE_CORE_LIB "Enable compile 'core' as static lib" OFF)
#
#
option(SOUI_ENABLE_COM_LIB "Enable compile 'components' as static lib" OFF)

#
#
option(SOUI_XP_TOOLSET "" ON)
#
#
option(SOUI_ENABLE_SPECTRE "Enable Spectre Mitigation" OFF)
#
#
option(SOUI_BUILD_DEMOS "Build builtin demos" ON)

option(SOUI_BUILD_GAMES "Build soui games" ON)

if(CMAKE_SYSTEM_NAME MATCHES "iOS")
set(SOUI_BUILD_TOOLS OFF)
add_definitions(-D__IOS__) #//add __IOS__ for ios build.
else()
option(SOUI_BUILD_TOOLS "Build soui tools" ON)
endif()
#
#
option(SOUI_BUILD_HTTPCLIENT "Build HTTP client component" ON)

#
#
option(SOUI_ENABLE_SVG "Enable SVG support" ON)

message(STATUS "MSVC_VERSION: ${MSVC_VERSION}")
if(MSVC AND MSVC_VERSION EQUAL 1700)
    # VS2012(v110)std::tuple: 模板参数太多问题
    add_definitions(-D_VARIADIC_MAX=10)
endif()
if(MSVC AND MSVC_VERSION LESS 1800)
set(SOUI_BUILD_WS OFF)
message(STATUS "Build Websocket module disabled for VS2013 or lower")
else()
if(EXISTS "${PROJECT_SOURCE_DIR}/third-part/libwebsockets/CMakeLists.txt")
option(SOUI_BUILD_WS "Build Websocket module" ON)
else()
set(SOUI_BUILD_WS OFF)
endif()
endif()

if(IS_OHOS)
set(SOUI_BUILD_RICHEDIT ON CACHE BOOL "using richedit from source code" FORCE)
set(SOUI_ENABLE_ACC OFF)
else(IS_OHOS)
if (CMAKE_SYSTEM_NAME MATCHES Windows)
option(SOUI_BUILD_RICHEDIT "using richedit from source code" OFF)
option(SOUI_ENABLE_SPY "Enable Swnd Spy" OFF)
if (MSVC)
    add_compile_options(/MP)
endif()
else()
set(SOUI_BUILD_RICHEDIT ON)
set(SOUI_ENABLE_SPY OFF)
endif()
option(SOUI_ENABLE_ACC "Enable MSAA/IAccessible (provided by swinx)" OFF)
endif(IS_OHOS)

# ---------------------------------------------------------------------------
# WebAssembly (Emscripten) 裁剪
#
# 浏览器形态下构建 swinx + utilities4 + SOUI 核心 + 最小静态组件（render-gdi、
# imgdecoder-stb）：third-part 里的 skia/lua/sqlite/7z/curl/openssl…、controls.extend
# 和资源包要么依赖桌面/联网能力，要么依赖宿主文件系统，第一阶段全部不进入构建图。
# SOUI 核心与 utilities 均不 include third-part 头文件，只链接 utilities4 + swinx。
#
# 静态形态是必须的：浏览器没有 dlopen，组件库无法按 SOUI_ENABLE_CORE_LIB=OFF 的
# DLL 方式在运行时加载，只能编进最终二进制。
#
# 组件白名单不写在根 CMakeLists 里，而是由 third-part/CMakeLists.txt 和
# components/CMakeLists.txt 各自的 SOUI_WASM_MINIMAL 分支决定——那两个文件才是
# 自己内容的所有者，根这里只负责把开关打开、把目录挂进构建图。
# ---------------------------------------------------------------------------
if(EMSCRIPTEN)
    set(SOUI_WASM_MINIMAL ON CACHE INTERNAL "wasm build: swinx + utilities4 + SOUI core + minimal components")
    set(SOUI_ENABLE_CORE_LIB ON CACHE BOOL "Enable compile 'core' as static lib" FORCE)
    set(SOUI_ENABLE_COM_LIB ON CACHE BOOL "Enable compile 'components' as static lib" FORCE)
    set(SOUI_BUILD_DEMOS OFF CACHE BOOL "Build builtin demos" FORCE)
    set(SOUI_BUILD_GAMES OFF CACHE BOOL "Build soui games" FORCE)
    set(SOUI_BUILD_TOOLS OFF CACHE BOOL "Build soui tools" FORCE)
    set(SOUI_BUILD_HTTPCLIENT OFF CACHE BOOL "Build HTTP client component" FORCE)
    set(SOUI_BUILD_WS OFF CACHE BOOL "Build Websocket module" FORCE)
    # 页面上真正能打字的 <edit> 需要 richedit：SOUI 的 SRichEdit 有两条路，桌面
    # 构建默认走 LoadLibrary 加载系统 riched20/msftedit，wasm 没有 dlopen，只能走
    # INIT_RICHEDIT 的静态链接版本（SOUI/CMakeLists.txt 里由本开关决定）。
    set(SOUI_BUILD_RICHEDIT ON CACHE BOOL "using richedit from source code" FORCE)
    # SVG 在 wasm 下也开：Lua 游戏页（消消乐/俄罗斯方块/坦克大战）的皮肤全是
    # src="svg:..."（多帧雪碧图 + 九宫药丸底），关掉之后这些 skin 拿不到 ISvgObj，
    # 整页画成空白。实现只多一份 header-only 的 nanosvg（SOUI/src/core/Svg.cpp）
    # 加 render-gdi 的 SVG 绘制分支，体积代价见 demos/wasm-demo/readme.md 的体积表。
    set(SOUI_ENABLE_SVG ON CACHE BOOL "Enable SVG support" FORCE)
    set(SOUI_ENABLE_SPY OFF CACHE BOOL "Enable Swnd Spy" FORCE)
    set(SOUI_ENABLE_ACC OFF CACHE BOOL "Enable MSAA/IAccessible (provided by swinx)" FORCE)
    # 两个"可选能力"开关：默认开，关掉能直接从 .wasm/.data 里省掉对应体积
    # （lua-54 + ScriptModule-LUA + translator ≈ 数 MB；Scintilla 的 .cxx 更多）。
    # 它们只影响 wasm 最小构建图（桌面构建不受这两个变量约束），消费的四个位置：
    #   third-part/CMakeLists.txt   lua-54 / Scintilla 子目录
    #   components/CMakeLists.txt   ScriptModule-LUA / translator 子目录
    #   controls.extend/…           SScintillaView 源文件过滤 + 注册守卫
    #   demos/wasm-demo/CMakeLists.txt  链接列表 + uires/lua 预载 + 传给宿主的宏
    # 宿主的 SOUI_WASM_HAS_LUA/HAS_SCINTILLA 宏会汇总进 SwinxDemoFeatureMask()，
    # 无头烟测据此跳过被关掉的页内断言（关功能不该让烟测变红）。
    option(SOUI_BUILD_WASM_LUA "wasm: build ScriptModule-LUA + lua-54 + translator and preload uires/lua" ON)
    option(SOUI_BUILD_WASM_SCINTILLA "wasm: build Scintilla and the <scintilla> control from controls.extend" ON)
    # 中国象棋客户端（games/cnchess/client）的 wasm 版本。它比演示多要三个组件：
    # TaskLoop 的单线程后端、ws 的浏览器 WebSocket 后端、resprovider-zip（解压
    # 服务器下发的主题包）。开关只挂 games/cnchess/algorithm + client/wasm 两个
    # 子目录，桌面构建不受影响。
    option(SOUI_BUILD_WASM_CNCHESS "wasm: build the cnchess client (games/cnchess/client/wasm)" OFF)
    # Emscripten 用 clang 编译，而下面 Darwin 分支的告警抑制只按目标平台判断，交叉编译
    # 到 wasm 时不生效：同一批头文件会在 macOS/Linux 主机上各刷上万条
    # -Winconsistent-missing-override。这里按编译器口径补一条，与 §Darwin 分支一致。
    add_compile_options(-Wno-inconsistent-missing-override)
    message(STATUS "SOUI_WASM_MINIMAL: swinx + utilities4 + SOUI core + render-gdi/imgdecoder-stb only")
endif()

include(__cmake/global.cmake)
include(__cmake/windres.cmake)
message(STATUS "ENABLE_RESOURCES_BUILD: ${ENABLE_RESOURCES_BUILD}")
config_compiler_and_linker()


if (CMAKE_SYSTEM_NAME MATCHES Windows)
	if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 4)
	    add_link_options(/SAFESEH:NO)
	endif()
  if(MSVC_VERSION LESS 1600)
      include_directories(${PROJECT_SOURCE_DIR}/third-part/stdint)
  endif()
  add_definitions(-DWCHAR_SIZE=2)
else()
        if(NOT EXISTS "${PROJECT_SOURCE_DIR}/swinx/CMakeLists.txt")
                message(WARNING "swinx submodule not found. Attempting to initialize it...")
                find_package(Git QUIET)
                if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
                        message(STATUS "Initializing and updating sinx submodule...")
                        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init -- swinx
                                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                                RESULT_VARIABLE GIT_SUBMOD_RESULT)
                        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
                                message(FATAL_ERROR "git submodule update failed with ${GIT_SUBMOD_RESULT}, please checkout submodules manually")
                        endif()
                else()
                        message(WARNING "Git not found or not a git repository. Please manually initialize the libwebsockets submodule")
                endif()
        endif()
        
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/swinx/CMakeLists.txt")
                add_subdirectory(swinx)
        else()
                message(FATAL_ERROR "swinx submodule not available, please checkout submodules manually")
        endif()

  if(CMAKE_SYSTEM_NAME MATCHES Darwin)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0")
        add_compile_options(-Wno-deprecated-declarations)
        add_compile_options(-Wno-expansion-to-defined)
        add_compile_options(-Wno-inconsistent-missing-override)
        add_compile_options(-Wno-parentheses)
        add_compile_options(-Wno-unused-value)
        add_compile_options(-Wno-switch)
        add_compile_options(-Wno-unsequenced)
        add_compile_options(-Wno-undefined-var-template)
        add_compile_options(-Wno-c++11-extensions)
        set(CMAKE_CXX_STANDARD 11)
  else()
        set(CMAKE_SKIP_BUILD_RPATH FALSE)
        set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
        set(CMAKE_INSTALL_RPATH $ORIGIN)
  endif()

  add_definitions(-DWCHAR_SIZE=4)
  include_directories(${PROJECT_SOURCE_DIR}/swinx/include)
endif()
if(NOT MSVC)
  # MinGW Release 默认 -O3、无 strip、无死代码消除，DLL 体积约为 MSVC 的 4 倍；
  # 对齐 MSVC /O2 + /OPT:REF,ICF 的行为：-O2 + 函数级分段 + gc-sections + 链接期 strip。
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -ffunction-sections -fdata-sections")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -ffunction-sections -fdata-sections")
  if(CMAKE_SYSTEM_NAME MATCHES "Darwin|iOS")
        set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl")
  else()
        set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl,--gc-sections -s")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--gc-sections -s")
  endif()
endif()

configure_file("config/config.h.in" "${PROJECT_BINARY_DIR}/config/config.h" @ONLY)
configure_file("config/build.cfg.in" "${PROJECT_BINARY_DIR}/config/build.cfg" @ONLY)
include_directories(${PROJECT_BINARY_DIR}/config)
install(FILES "${PROJECT_BINARY_DIR}/config/config.h" DESTINATION include/config)
install(FILES "${PROJECT_BINARY_DIR}/config/build.cfg" DESTINATION include/config)

# 设置库文件的输出路径
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_definitions(-D_CRT_NONSTDC_NO_WARNINGS -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
link_directories(${CMAKE_BINARY_DIR}/bin)
if(NOT CMAKE_SYSTEM_NAME MATCHES Windows)
        set(CMAKE_CXX_STANDARD 11)
endif()
# wasm 最小构建：third-part 与 components 只保留渲染器和图片解码器需要的条目，
# 系统资源包（zip）不进入构建图（见各自 CMakeLists 的 SOUI_WASM_MINIMAL 分支）。
add_subdirectory(third-part)
add_subdirectory(utilities)
add_subdirectory(components)
add_subdirectory(SOUI)
if(NOT SOUI_WASM_MINIMAL)
    add_subdirectory(soui-sys-resource)
    add_subdirectory(soui-sys-resource2)
    add_subdirectory(soui-sys-resource-dark)
    add_subdirectory(soui-sys-resource-light)
endif()
# 扩展控件在 wasm 下也进构建图：SScintillaView 走 Scintilla 的 headless 接口，
# 其余控件只用 GDI。注册入口是 SCtrlsRegister::RegisterCtrls，由 demo 显式调用
# （wasm 没有静态自注册）。
add_subdirectory(controls.extend)
# wasm 演示（demos/wasm-demo）不在 SOUI_BUILD_DEMOS 里：那个开关在 EMSCRIPTEN 下恒关，
# 而这套移植唯一的端到端入口就是它。用独立开关、默认关，既有配置一概不受影响。
# 打开后演示随全栈一起构建，链接命令全部在 demos/wasm-demo/CMakeLists.txt 里，宿主
# 不需要 bash/sed/find（Windows 主机因此可以只靠 emcmake + cmake 构建）。
# 注意 demos/CMakeLists.txt 不会遍历子目录（它显式列目标），所以这里直接指路径，
# 与 SOUI_BUILD_DEMOS 不会重复加同一棵树。
option(SOUI_BUILD_WASM_DEMO "Build the wasm demo in demos/wasm-demo (Emscripten hosts only)" OFF)
if(EMSCRIPTEN AND SOUI_BUILD_WASM_DEMO)
    add_subdirectory(demos/wasm-demo)
endif()
# 中国象棋的 wasm 客户端同样独立于 SOUI_BUILD_GAMES：那个开关在 EMSCRIPTEN 下恒关，
# 且 games/ 的入口还要求 websockets 目标（wasm 不用 libwebsockets）。这里只挂客户
# 端真正需要的两棵子树，服务器端的 server/ 留在桌面构建里。
if(EMSCRIPTEN AND SOUI_BUILD_WASM_CNCHESS)
        add_subdirectory(games/cnchess/algorithm)
        add_subdirectory(games/cnchess/client/wasm)
endif()
if(SOUI_BUILD_TOOLS)
        add_subdirectory(tools)
endif()
if(SOUI_BUILD_DEMOS)
        add_subdirectory(demos)
endif(SOUI_BUILD_DEMOS)
if(SOUI_BUILD_GAMES AND TARGET websockets)
        add_subdirectory(games) 
endif()
if(NOT CMAKE_SYSTEM_NAME MATCHES Windows)
set(SWINX_LIBS swinx)
set(SWINX_CFLAGS swinx/include)
endif()

# ---------------------------------------------------------------------------
# Aggregate the *public* (installed) libraries exposed to consumers.
#
# In the default (shared) build COM_DEP_LIBS / SWINX_DEP_LIBS are empty (those
# variables are only populated when SOUI_ENABLE_CORE_LIB=ON), so they add
# nothing here. When soui4 is built as a set of static libraries
# (SOUI_ENABLE_CORE_LIB=ON), those third-party dependencies are NOT bundled
# into the static archives and MUST be exposed so the consumer can link them.
# Appending them unconditionally is therefore correct for both configurations.
# ---------------------------------------------------------------------------
set(SOUI_PUBLIC_LIBS ${CORE_LIBS} ${COM_LIBS} ${OPTION_LIBS}
    ${SOUI_UTILITIES} ${SOUI_SWINX}
    ${COM_DEP_LIBS} ${SWINX_DEP_LIBS})
list(REMOVE_DUPLICATES SOUI_PUBLIC_LIBS)

set(SOUI_PC_LIBS "")
foreach(_l ${SOUI_PUBLIC_LIBS})
    if(_l)
        list(APPEND SOUI_PC_LIBS "-l${_l}")
    endif()
endforeach()
list(REMOVE_DUPLICATES SOUI_PC_LIBS)
string(REPLACE ";" " " SOUI_PC_LIBS "${SOUI_PC_LIBS}")

# Configure the (single) soui4-config.cmake template — reused for both direct
# include() and find_package(soui4). Installed to __cmake for the legacy
# include(), and to lib/cmake/soui4/ for find_package(soui4).
configure_file("${PROJECT_SOURCE_DIR}/__cmake/soui4-config.cmake.in"
               "${PROJECT_BINARY_DIR}/soui4-config.cmake" @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/soui4-config.cmake"
        DESTINATION ${CMAKE_INSTALL_PREFIX}/__cmake)

# ---------------------------------------------------------------------------
# find_package(soui4) support.
# The generated soui4-config.cmake is installed as the package config for the
# single canonical package name "soui4". A matching <name>-config-version file
# is emitted for version-gated find_package() requests.
# ---------------------------------------------------------------------------
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/soui4-config-version.cmake"
    VERSION "${SOUI_VER1}.${SOUI_VER2}.${SOUI_VER3}.${SOUI_VER4}"
    COMPATIBILITY SameMajorVersion)

install(FILES "${PROJECT_BINARY_DIR}/soui4-config.cmake"
        RENAME soui4-config.cmake
        DESTINATION lib/cmake/soui4)
install(FILES "${PROJECT_BINARY_DIR}/soui4-config-version.cmake"
        DESTINATION lib/cmake/soui4)

# ---------------------------------------------------------------------------
# pkg-config file (soui4.pc) so soui4 can be consumed without CMake.
# ---------------------------------------------------------------------------
configure_file("${PROJECT_SOURCE_DIR}/__cmake/soui4.pc.in"
               "${PROJECT_BINARY_DIR}/soui4.pc" @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/soui4.pc"
        DESTINATION lib/pkgconfig)


# Install cmake files
install(DIRECTORY ${PROJECT_SOURCE_DIR}/__cmake
        DESTINATION ${CMAKE_INSTALL_PREFIX}
        FILES_MATCHING PATTERN "*.cmake"
)
install(FILES ${PROJECT_SOURCE_DIR}/__cmake/plist.in ${PROJECT_SOURCE_DIR}/__cmake/dpi_aware.manifest ${PROJECT_SOURCE_DIR}/__cmake/export_res_sym.lst
        DESTINATION ${CMAKE_INSTALL_PREFIX}/__cmake
)

install(FILES 
        ${PROJECT_SOURCE_DIR}/soui-sys-resource.zip
        DESTINATION ${CMAKE_INSTALL_PREFIX}/resources
)

install(FILES 
        ${PROJECT_SOURCE_DIR}/LICENSE-en.md
        ${PROJECT_SOURCE_DIR}/LICENSE-zh.md
        DESTINATION ${CMAKE_INSTALL_PREFIX}
)
