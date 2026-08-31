if(CMAKE_SYSTEM_NAME MATCHES "iOS")
add_definitions(-D__MOBILE__)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/controls.extend)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../algorithm)
include_directories(${PROJECT_SOURCE_DIR}/soui-sys-resource)
if(NOT CMAKE_SYSTEM_NAME MATCHES Windows)
    include_directories(${PROJECT_SOURCE_DIR}/swinx/include)
endif()

# Source files
file(GLOB CURRENT_HEADERS *.h *.hpp)
file(GLOB CURRENT_SRCS *.cpp main.cc)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)
file(GLOB_RECURSE CURRENT_UIRES uires/*.*)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("Resource Files" FILES ${CURRENT_RC})
source_group("SoUI Resource" FILES ${CURRENT_UIRES})

# 设置.rc文件依赖于UI资源文件，确保资源文件更新时重新编译.rc文件
set_source_files_properties(${CURRENT_RC} PROPERTIES OBJECT_DEPENDS "${CURRENT_UIRES}")

# 将RESBUILD_EXE修改为指向uiresbuilder项目生成的目标文件
set(RESBUILD_EXE $<TARGET_FILE:uiresbuilder>)
# 将生成的resource.h文件添加到项目中
set(UIRES_RES_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/res/resource.h)

# 确保在构建时会处理uires.idx文件
set_source_files_properties(${UIRES_RES_HEADER} PROPERTIES GENERATED TRUE)

# Apple bundle properties
set(MACOSX_BUNDLE_HIGH_RESOLUTION_CAPABLE "true")
set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.soui.cnchess")
set(MACOSX_BUNDLE_BUNDLE_NAME "cnchess")

set(current_project "cnchess_client")
add_soui_exe(${current_project} ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${UIRES_RES_HEADER} ${CURRENT_RC})

# Compile resource files with windres on non-Windows
target_compile_resources(${current_project} ${CURRENT_RC} INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/soui-sys-resource)

add_dependencies(${current_project} soui4 ExtendCtrls resprovider-zip)
target_link_libraries(${current_project} soui4 utilities4 ExtendCtrls cnchess_algorithm resprovider-zip)

if(SOUI_ENABLE_COM_LIB)
    target_link_libraries(${current_project} ${COM_LIBS} ${COM_DEP_LIBS})
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(${current_project} win32_audio)
    if(MSVC)
    set(DPI_AWARE_MANIFEST "${PROJECT_SOURCE_DIR}/__cmake/dpi_aware.manifest")
    set_target_properties(${current_project} PROPERTIES
            LINK_FLAGS "/MANIFEST:EMBED /MANIFESTINPUT:${DPI_AWARE_MANIFEST}"
        )
    endif(MSVC)
else()
    target_link_libraries(${current_project} swinx dl)
endif()

if(APPLE)
    set(MACOSX_CODESIGN_IDENTITY "Apple Development")
    set(MACOSX_TEAM_ID 2YL33QR3KY)

    set_target_properties(${current_project} PROPERTIES
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${MACOSX_TEAM_ID}
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ${MACOSX_CODESIGN_IDENTITY}
        XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
        XCODE_ATTRIBUTE_PROVISIONING_PROFILE_SPECIFIER ""
    )

    # 准备要复制的目标库列表
    set(CNCHESS_TARGET_LIST
        swinx
        soui4
        utilities4
        msftedit
        imgdecoder-stb
        resprovider-zip
        resprovider-7zip
        render-skia
        log4z
        taskloop
        translator
        ws
    )
    copy_targets_to_bundle(${current_project} "${CNCHESS_TARGET_LIST}")
    set_macos_icon(${current_project} ${CMAKE_CURRENT_SOURCE_DIR}/icon.icns ${PROJECT_SOURCE_DIR}/__cmake/plist.in)
    # iOS 和 Android 共享相同的资源包
    # 系统资源（来自 SOUI 源码树，与 demo 一致）
    add_macos_res_folder(${current_project} ${PROJECT_SOURCE_DIR}/soui-sys-resource soui-sys-resource)
    # 移动版 UI 布局（来自 Android assets/uires，iOS 和 Android 共享）
    if(CMAKE_SYSTEM_NAME MATCHES "iOS")
    add_macos_res_folder(${current_project} ${CMAKE_CURRENT_SOURCE_DIR}/android/app/src/main/assets/uires uires)
    else()
    add_macos_res_folder(${current_project} ${CMAKE_CURRENT_SOURCE_DIR}/uires uires)
    endif()
endif()

set_target_properties (${current_project} PROPERTIES
    FOLDER "games/cnchess"
)
