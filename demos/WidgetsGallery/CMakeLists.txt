set(current_project WidgetsGallery)

# 与 demos/uieditor 一致的全平台配置：
#   Windows(MSVC)  - WIN32 窗口程序，链接器嵌入 PerMonitorV2 清单；
#   Windows(MinGW) - windres 把 uires 与系统资源都编进 exe，由 .rc 以
#                    RT_MANIFEST 嵌入高 DPI 清单；
#   macOS          - .app bundle，拷入 uires 与系统资源 zip；
#   Linux          - 普通可执行文件；装了 MinGW 时同样把资源编进 ELF，
#                    否则回退到 exe 同目录的资源目录 / zip。
file(GLOB_RECURSE CURRENT_HEADERS CONFIGURE_DEPENDS *.h)
file(GLOB_RECURSE CURRENT_SRCS CONFIGURE_DEPENDS *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS CONFIGURE_DEPENDS uires/*.*)
file(GLOB_RECURSE CURRENT_RC CONFIGURE_DEPENDS *.rc res/*.rc2)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SOUI Resources" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})

add_soui_exe(${current_project}
    ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC})

target_include_directories(${current_project} PRIVATE
    ${PROJECT_SOURCE_DIR}/components
    ${PROJECT_SOURCE_DIR}/utilities/include
    ${PROJECT_SOURCE_DIR}/SOUI/include
    ${PROJECT_SOURCE_DIR}/soui-sys-resource
    ${PROJECT_SOURCE_DIR}/__cmake
    ${CMAKE_CURRENT_SOURCE_DIR})

# MSVC 下只补充 .rc 的 include 目录并定义 ENABLE_BUILD_RESOURCE；
# MinGW 与 Linux/macOS 的 windres 才会真正把 .rc 编成目标文件。
# 找不到 windres 时不定义 ENABLE_BUILD_RESOURCE，main.cpp 回退到磁盘资源。
target_compile_resources(${current_project} ${CURRENT_RC}
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/soui-sys-resource
        ${PROJECT_SOURCE_DIR}/__cmake)

target_compile_definitions(${current_project} PRIVATE DLL_SOUI
    SOUI_GALLERY_RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/uires")

target_link_libraries(${current_project} soui4)
add_dependencies(${current_project} soui4 ${COM_LIBS})

if(CMAKE_SYSTEM_NAME MATCHES Windows)
    add_dependencies(${current_project} soui-sys-resource)
    if(MSVC AND MSVC_VERSION GREATER_EQUAL 1900)
        # 高 DPI 必须在窗口创建前生效，因此把清单嵌进 PE，而不是运行期再调用
        # SetProcessDpiAwareness（SOUI 内部不会主动设置）。
        set(DPI_AWARE_MANIFEST "${PROJECT_SOURCE_DIR}/__cmake/dpi_aware.manifest")
        set_target_properties(${current_project} PROPERTIES
            LINK_FLAGS "/MANIFEST:EMBED /MANIFESTINPUT:${DPI_AWARE_MANIFEST}"
        )
    else()
        # MinGW 与 VS2008 没有 /MANIFESTINPUT，由 .rc 以 RT_MANIFEST 嵌入同一份清单。
        set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/WidgetsGallery.rc"
            PROPERTIES COMPILE_DEFINITIONS "SOUI_EMBED_MANIFEST")
    endif()
endif()

if(CMAKE_SYSTEM_NAME MATCHES Darwin)
    set(TARGET_LIST
        swinx
        soui4
        msftedit
        utilities4
        imgdecoder-stb
        resprovider-zip
        resprovider-7zip
        log4z
        translator
        render-skia
    )
    copy_targets_to_bundle(${current_project} "${TARGET_LIST}")
    # main.cpp 在 __APPLE__ 下用 GetAppleBundlePath 定位资源，目录布局与下面一致。
    add_macos_res_folder(${current_project} ${CMAKE_CURRENT_SOURCE_DIR}/uires WidgetsGallery-res)
    add_macos_res_file(${current_project} ${PROJECT_SOURCE_DIR}/soui-sys-resource.zip ".")
    set_macos_icon(${current_project} ${CMAKE_CURRENT_SOURCE_DIR}/logo.icns ${PROJECT_SOURCE_DIR}/__cmake/plist.in)
elseif(NOT CMAKE_SYSTEM_NAME MATCHES Windows)
    # Linux：exe 同目录可直接拷走运行；这两个目录只在未启用资源内嵌时被读取。
    add_app_res_folder(${current_project} ${CMAKE_CURRENT_SOURCE_DIR}/uires WidgetsGallery-res)
    add_app_res_file(${current_project} ${PROJECT_SOURCE_DIR}/soui-sys-resource.zip ".")
endif()

if(CMAKE_SYSTEM_NAME MATCHES Windows)
    target_link_libraries(${current_project} Msimg32)
else()
    target_link_libraries(${current_project} dl)
endif()

if(SOUI_ENABLE_COM_LIB)
    target_link_libraries(${current_project} ${COM_LIBS} ${COM_DEP_LIBS})
endif()

set_target_properties(${current_project} PROPERTIES FOLDER demos)
