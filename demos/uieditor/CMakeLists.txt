

include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/soui-sys-resource)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/third-part/Scintilla/include)

include_directories(${CMAKE_SOURCE_DIR}/controls.extend)


file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico *.png *.jpg *.bmp)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS}) 
source_group("SoUI Resource" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})

set(current_project "uieditor")
add_soui_exe(${current_project} ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC})

if(CMAKE_SYSTEM_NAME MATCHES Darwin)
    # 配置代码签名
    set(MACOSX_CODESIGN_IDENTITY "Apple Development")
    set(MACOSX_TEAM_ID "XXXXXXXXXXXX")

    # 设置 Xcode 特定的签名属性
    set_target_properties(${current_project} PROPERTIES
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${MACOSX_TEAM_ID}"
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${MACOSX_CODESIGN_IDENTITY}"
        XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
        XCODE_ATTRIBUTE_PROVISIONING_PROFILE_SPECIFIER ""
    )

    # 准备要复制的目标列表
    set(DEMO_TARGET_LIST
        cairo
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
    copy_targets_to_bundle(${current_project} "${DEMO_TARGET_LIST}")

    add_macos_res_folder(${current_project} ${CMAKE_CURRENT_SOURCE_DIR}/uires uires)
    add_macos_res_file(${current_project} ${PROJECT_SOURCE_DIR}/simsun.ttc fonts)
    add_macos_res_file(${current_project} ${PROJECT_SOURCE_DIR}/soui-sys-resource.zip data)
    add_macos_res_folder(${current_project} ${CMAKE_CURRENT_SOURCE_DIR}/Config data/Config)
    set_macos_icon(${current_project} ${CMAKE_CURRENT_SOURCE_DIR}/logo.icns ${PROJECT_SOURCE_DIR}/__cmake/plist.in)
else()
    add_app_res_folder(${current_project} ${CMAKE_CURRENT_SOURCE_DIR}/Config Config)
    add_app_res_file(${current_project} ${PROJECT_SOURCE_DIR}/soui-sys-resource.zip ".")
endif()


if (CMAKE_SYSTEM_NAME MATCHES Windows)
	add_dependencies(${current_project} soui-sys-resource ${COM_LIBS})
    if(MSVC AND MSVC_VERSION GREATER_EQUAL 1900)
    set(DPI_AWARE_MANIFEST "${PROJECT_SOURCE_DIR}/__cmake/dpi_aware.manifest")
    set_target_properties(${current_project} PROPERTIES
            LINK_FLAGS "/MANIFEST:EMBED /MANIFESTINPUT:${DPI_AWARE_MANIFEST}"
        )
    endif()
endif()

if (CMAKE_SYSTEM_NAME MATCHES Windows)
	target_link_libraries(${current_project} soui4 utilities4 ExtendCtrls Scintilla)
else()
    target_link_libraries(${current_project} soui4 utilities4 ExtendCtrls Scintilla dl)
endif()

target_precompile_headers(${current_project} PRIVATE "stdafx.h")

set_target_properties (${current_project} PROPERTIES
    FOLDER demos
)
