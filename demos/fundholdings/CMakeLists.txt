include_directories(${PROJECT_SOURCE_DIR}/third-part/jsoncpp/include)
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/soui-sys-resource)
include_directories(${PROJECT_SOURCE_DIR}/controls.extend)
include_directories(${PROJECT_SOURCE_DIR}/controls.extend/gridctrl)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
if(NOT CMAKE_SYSTEM_NAME MATCHES Windows)
    include_directories(${PROJECT_SOURCE_DIR}/swinx/include)
endif()

# iOS 与 Android 共用 mobile/uires：窗口尺寸、控件 name 完全一致，
# 只由 XML 布局决定手机竖屏的排布。
if(CMAKE_SYSTEM_NAME MATCHES "iOS")
    add_definitions(-D__MOBILE__)
endif()

file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico *.png *.jpg *.bmp *.svg *.icns)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)
file(GLOB_RECURSE GRIDCTRL_SRC ${PROJECT_SOURCE_DIR}/controls.extend/gridctrl/*.*)

# android/ 与 ohos/ 是独立的移动端工程（各自带 CMakeLists、Gradle/Hvigor 构建缓存、
# 平台资源），wasm/ 是浏览器入口（自己的 CMakeLists 与 main），都不能被桌面目标的
# 递归 glob 扫进来：否则 .cxx 里的 CMakeCXXCompilerId.cpp 或 wasm 的 main 会混进
# exe 源文件，导致 main 重复定义。
list(FILTER CURRENT_HEADERS EXCLUDE REGEX "/(android|ohos|mobile|wasm)/")
list(FILTER CURRENT_SRCS EXCLUDE REGEX "/(android|ohos|mobile|wasm)/")
list(FILTER CURRENT_SOUIS EXCLUDE REGEX "/(android|ohos|wasm)/")

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resource" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})

set(current_project "fundholdings")

# Debian 包（build_deb.sh）只收集这里列出的运行时组件。根 CMake 会统一构建
# 全部组件供其它 demo 使用，但本程序用不到的
#   ws / network / render-gdi / resprovider-7zip / resprovider-zip /
#   scriptmodule-lua / sipcobject
# 不应该进 .deb。build_deb.sh 不再全量扫 bin/lib*.so，而是读这份清单：
# file(GENERATE) 输出到 <build>/bin/fundholdings-runtime.list，再由 POST_BUILD
# 拷到 exe 同目录（兼容多配置生成器的 bin/Release、bin/Debug）。
#
# 系统资源在任何平台都不使用 zip 包：uires 自带全部皮肤/字体/滚动条皮肤；
# Apple 平台把 soui-sys-resource-dark 目录拷进 .app 资源包，运行时由
# main.cpp 用 SetSysResFile() 从资源包加载（见下方 Darwin 分支）。
# 因此 libresprovider-zip.so / libresprovider-7zip.so 都不需要。

set(FUNDHOLDINGS_RUNTIME_LIBS
    libsoui4.so
    libswinx.so
    libcairo.so
    libutilities4.so
    libmsftedit.so
    libhttpclient.so
    libimgdecoder-stb.so
    liblog4z.so
    librender-skia.so
    libtaskloop.so
    libtranslator.so
)
string(REPLACE ";" "\n" FUNDHOLDINGS_RUNTIME_LIBS_TEXT "${FUNDHOLDINGS_RUNTIME_LIBS}")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/bin/fundholdings-runtime.list"
     CONTENT "# runtime modules for ${current_project}; consumed by build_deb.sh\n${FUNDHOLDINGS_RUNTIME_LIBS_TEXT}\n")

# ---------------------------------------------------------------- 可执行目标
# 与 demos/uieditor 相同：统一用 add_soui_exe 建目标（Windows 为 WIN32 窗口程序、
# macOS 为 MACOSX_BUNDLE），再用 target_compile_resources 处理 .rc 资源。
# 三个桌面平台共用同一份 .rc：Windows 用 windres/rc.exe，Linux/macOS 在装有
# MinGW 时由 windres 编成 COFF 再包成 ELF/Mach-O 数据段。找不到 windres 时
# 该函数直接跳过、不定义 ENABLE_BUILD_RESOURCE，程序改用磁盘资源
# （main.cpp 依据这个宏决定加载方式）。
set(APP_RC ${CURRENT_RC})

add_soui_exe(${current_project}
    ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${APP_RC} ${GRIDCTRL_SRC})

# MSVC 在这里只会追加 include 目录；MinGW/跨平台 windres 才会真正编译 .rc。
target_compile_resources(${current_project} ${APP_RC}
    INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/soui-sys-resource-dark)

# 清单文件随每次构建放到 exe 同目录，打包脚本只需看 BIN_DIR 即可找到。
add_custom_command(TARGET ${current_project} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/bin/fundholdings-runtime.list"
        "$<TARGET_FILE_DIR:${current_project}>/fundholdings-runtime.list"
    COMMENT "Copying runtime module manifest for ${current_project}")

add_dependencies(${current_project} soui4 httpclient)
target_link_libraries(${current_project} soui4)

# httpclient 作为 COM 组件提供 IHttpClient，运行时由 SComMgr2 自动加载
# libhttpclient.dll（位于 bin）。显式链接以登记 DLL 依赖。
target_link_libraries(${current_project} httpclient)

# ---------------------------------------------------------------- 图标 / 高分屏 / 跨平台 bundle
# 配置方式参考 demos/uieditor：
#   图标   - Windows：.rc 里 IDI_ICON1 指向 uires/image/logo.ico（exe 主图标），
#            同时 uires.idx 的 ICON_LOGO 供窗口标题栏使用
#            macOS：logo.icns + Info.plist 的 CFBundleIconFile
#   高分屏 - Windows：CMake 通过链接器开关嵌入 __cmake/dpi_aware.manifest（PerMonitorV2），
#            MinGW 没有等价开关，改由 .rc 以 RT_MANIFEST 资源嵌入同一份清单
#            macOS：Info.plist 的 NSHighResolutionCapable
#   资源   - 与 uieditor 相同：uires 由 cmake 拷到 exe 目录；
#            另外 .rc 通过 res/soui_res.rc2 把 uires 也编进 exe，
#            运行时优先用内置资源，单文件拷走也能跑（见 main.cpp）
if (CMAKE_SYSTEM_NAME MATCHES Windows)
    add_dependencies(${current_project} soui-sys-resource)
    if (MSVC AND MSVC_VERSION GREATER_EQUAL 1900)
        # SOUI 内部不会主动调用 SetProcessDpiAwareness，缺少这份清单时进程会被
        # DWM 当成 96DPI 程序做位图拉伸，高 DPI 屏上整个窗口发虚。
        set(DPI_AWARE_MANIFEST "${PROJECT_SOURCE_DIR}/__cmake/dpi_aware.manifest")
        set_target_properties(${current_project} PROPERTIES
            LINK_FLAGS "/MANIFEST:EMBED /MANIFESTINPUT:${DPI_AWARE_MANIFEST}"
        )
    else ()
        # MinGW/windres 没有等价的链接器开关，改由 .rc 把同一份清单作为
        # RT_MANIFEST 资源嵌入（rc 里用 #ifdef SOUI_EMBED_MANIFEST 包住）
        set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/${current_project}.rc"
            PROPERTIES COMPILE_DEFINITIONS "SOUI_EMBED_MANIFEST")
    endif ()
elseif (CMAKE_SYSTEM_NAME MATCHES "iOS")
    # ---- Info.plist 字段（plist.in 会把这些变量填进模板） ----
    set(MACOSX_BUNDLE_BUNDLE_NAME          "FundHoldings")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER       "com.soui.fundholdings")
    set(MACOSX_BUNDLE_INFO_STRING          "Fund Holdings Assistant")
    set(MACOSX_BUNDLE_BUNDLE_VERSION       "1.0")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING  "1.0.0")
    set(MACOSX_BUNDLE_COPYRIGHT            "Copyright (c) 2026 soui4")

    set_macos_icon(${current_project}
        ${CMAKE_CURRENT_SOURCE_DIR}/logo.icns
        ${PROJECT_SOURCE_DIR}/__cmake/plist.in)

    # iOS 和 Android 共享同一份移动版 uires（mobile/uires）
    add_macos_res_folder(${current_project} ${CMAKE_CURRENT_SOURCE_DIR}/mobile/uires uires)

    # iOS 真机不允许读取系统 CA 证书文件，libcurl(OpenSSL) 因此无法校验 https，
    # 请求会以 CURLE_PEER_FAILED_VERIFICATION(60) 失败。把 Mozilla CA bundle 拷到
    # .app 根目录（与可执行文件同级），httpclient 会按可执行文件目录查找 cacert.pem
    # 并作为 CURLOPT_CAINFO 使用。
    add_macos_res_file(${current_project}
        ${CMAKE_CURRENT_SOURCE_DIR}/ca/cacert.pem
        .)

    # 系统资源：与 macOS 桌面版相同，从 .app 资源包加载
    add_macos_res_folder(${current_project}
        ${PROJECT_SOURCE_DIR}/soui-sys-resource-dark/theme_sys_res
        soui-sys-resource-dark/theme_sys_res)
    add_macos_res_file(${current_project}
        ${PROJECT_SOURCE_DIR}/soui-sys-resource-dark/uires.idx
        soui-sys-resource-dark)

    set(TARGET_LIST
        swinx
        soui4
        msftedit
        utilities4
        imgdecoder-stb
        log4z
        translator
        render-skia
        httpclient
        taskloop
    )
    if (NOT SOUI_ENABLE_CORE_LIB)
        list(APPEND TARGET_LIST "default")
    endif ()
    copy_targets_to_bundle(${current_project} "${TARGET_LIST}")
elseif (CMAKE_SYSTEM_NAME MATCHES Darwin)
    # ---- Info.plist 字段（plist.in 会把这些变量填进模板） ----
    set(MACOSX_BUNDLE_BUNDLE_NAME          "FundHoldings")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER       "com.soui.fundholdings")
    set(MACOSX_BUNDLE_INFO_STRING          "Fund Holdings Assistant")
    set(MACOSX_BUNDLE_BUNDLE_VERSION       "1.0")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING  "1.0.0")
    set(MACOSX_BUNDLE_COPYRIGHT            "Copyright (c) 2026 soui4")
    # Retina 支持：对应 NSHighResolutionCapable / NSHighResolutionMainMenuCapable
    set(MACOSX_BUNDLE_HIGH_RESOLUTION_CAPABLE "true")

    # .icns 图标 + Info.plist（模板里已含高分屏与 CFBundleIconFile 两项）
    set_macos_icon(${current_project}
        ${CMAKE_CURRENT_SOURCE_DIR}/logo.icns
        ${PROJECT_SOURCE_DIR}/__cmake/plist.in)

    # 运行时按需加载的组件库，必须和可执行文件一起进 .app/Contents/Frameworks
    # 系统资源改走目录型 RES_FILE（SOUI 核心内置），不再加载任何 zip 包，
    # 因此 resprovider-zip / resprovider-7zip 都不进 bundle。
    set(TARGET_LIST
        swinx
        soui4
        msftedit
        utilities4
        imgdecoder-stb
        log4z
        translator
        render-skia
        httpclient
    )
    if (NOT SOUI_ENABLE_CORE_LIB)
        list(APPEND TARGET_LIST "default")
    endif ()
    copy_targets_to_bundle(${current_project} "${TARGET_LIST}")

    # 应用资源目录按 uieditor 的方式拷进 bundle。
    # 系统资源使用 dark 主题的「目录型」资源包：只拷运行时真正需要的
    # uires.idx 与 theme_sys_res/（不拷 dark 目录里的 .rc/.cpp/CMakeLists 等构建文件），
    # 装到 .app/soui-sys-resource-dark/ 下。main.cpp 的 getResDir() 返回 bundle 根，
    # 正好用 SetSysResFile(resDir + "/soui-sys-resource-dark") 从资源包加载。
    add_macos_res_folder(${current_project} ${CMAKE_CURRENT_SOURCE_DIR}/uires uires)
    add_macos_res_folder(${current_project}
        ${PROJECT_SOURCE_DIR}/soui-sys-resource-dark/theme_sys_res
        soui-sys-resource-dark/theme_sys_res)
    add_macos_res_file(${current_project}
        ${PROJECT_SOURCE_DIR}/soui-sys-resource-dark/uires.idx
        soui-sys-resource-dark)
else ()
    # Windows(MinGW)/Linux 构建产物目录：uires 与 exe 平级，这样 bin 目录
    # 可以整体拷走直接运行，不再依赖源码树路径。
    # 系统资源不加载、也不拷任何 zip 包：皮肤/字体/滚动条皮肤都在 uires 里。
    add_app_res_folder(${current_project} ${CMAKE_CURRENT_SOURCE_DIR}/uires uires)
endif ()

# 预编译头：与 demos/uieditor 相同的做法，减少每次全量编译 SOUI 头的开销。
target_precompile_headers(${current_project} PRIVATE "stdafx.h")

set_target_properties (${current_project} PROPERTIES
    FOLDER demos
)
