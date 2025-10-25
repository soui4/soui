#
# 1394020320@qq.com
#

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/config)
include_directories(${CMAKE_SOURCE_DIR}/components)
include_directories(${CMAKE_SOURCE_DIR}/utilities/include)
include_directories(${CMAKE_SOURCE_DIR}/SOUI/include)
include_directories(${CMAKE_SOURCE_DIR}/third-part/Scintilla/include)
include_directories(${CMAKE_SOURCE_DIR}/third-part/wke/include)
include_directories(${CMAKE_SOURCE_DIR}/components/resprovider-zip)
include_directories(${CMAKE_SOURCE_DIR}/controls.extend)

set(CURRENT_SRCS
    helpapi.cpp
    MainDlg.cpp
    ResManger.cpp
    ScintillaWnd.cpp
    SImageBtnEx.cpp
    SImgCanvas.cpp
    SouiEditor.cpp
    SouiRealWndHandler.cpp
    SysdataMgr.cpp
    XmlEditor.cpp
    XmlParser.cpp
    AttrStorage.cpp
    spugixml/pugixml.cpp
    Dialog/DlgAbout.cpp
    Dialog/DlgCreatePro.cpp
    Dialog/DlgInput.cpp
    Dialog/DlgNewLayout.cpp
    Dialog/DlgResMgr.cpp
    Dialog/DlgInsertXmlElement.cpp
    designer/Adapter.cpp
    designer/DesignWnd.cpp
    designer/PreviewHost.cpp
    designer/SizingFrame.cpp
    monitor/ThreadObject.cpp
    monitor/PathMonitor.cpp
)

set(CURRENT_HEADERS
    stdafx.h
    res/resource.h
    CmdLine.h
    FileHelper.h
    helpapi.h
    MainDlg.h
    ResManger.h
    resource.h
    DropTarget.h
    souidlgs.h
    ScintillaWnd.h
    SImageBtnEx.h
    SouiRealWndHandler.h
    SysdataMgr.h
    xpm_icons.h
    XmlEditor.h
    XmlParser.h
    SImgCanvas.h
    AttrStorage.h
    spugixml/pugiconfig.hpp
    spugixml/pugixml.hpp
    Dialog/DlgAbout.h
    Dialog/DlgCreatePro.h
    Dialog/DlgInput.h
    Dialog/DlgNewLayout.h
    Dialog/DlgResMgr.h
    Dialog/DlgInsertXmlElement.h
    designer/Adapter.h
    designer/DesignWnd.h
    designer/PreviewHost.h
    designer/SizingFrame.h
    monitor/ThreadObject.h
    monitor/PathMonitor.h
)

file(GLOB_RECURSE CURRENT_SOUIS  uires/*.*)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

source_group("SoUI Resouece" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})

add_executable(uieditor WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS}  ${CURRENT_RC})
set_source_files_properties(${CURRENT_RC} PROPERTIES OBJECT_DEPENDS "${CURRENT_SOUIS}")

if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
add_app_res_folder(uieditor ${CMAKE_CURRENT_SOURCE_DIR}/uires uieditor_uires)
endif()
add_app_res_folder(uieditor ${CMAKE_CURRENT_SOURCE_DIR}/Config Config)
add_app_res_files(uieditor "${PROJECT_SOURCE_DIR}/soui-sys-resource.zip" ".")
target_link_libraries(uieditor soui4 utilities4 Scintilla ExtendCtrls)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
target_link_libraries(uieditor imm32 msimg32 gdiplus)
else()
target_link_libraries(uieditor swinx)
endif()

target_precompile_headers(uieditor PRIVATE "stdafx.h")
set_target_properties (uieditor PROPERTIES
    FOLDER demos
)
