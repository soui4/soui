#
# 1394020320@qq.com
#

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/config)
include_directories(${CMAKE_SOURCE_DIR}/components)
include_directories(${CMAKE_SOURCE_DIR}/utilities/include)
include_directories(${CMAKE_SOURCE_DIR}/SOUI/include)
include_directories(${CMAKE_SOURCE_DIR}/third-part/Scintilla/include)
include_directories(${CMAKE_SOURCE_DIR}/components/resprovider-zip)
include_directories(${CMAKE_SOURCE_DIR}/controls.extend)

set(CURRENT_SRCS
    DesignWnd.cpp
    helpapi.cpp
    MainDlg.cpp
    ResManger.cpp
    ScintillaWnd.cpp
    SImageBtnEx.cpp
    SImgCanvas.cpp
    SouiEditor.cpp
    SouiEditorApp.cpp
    SouiRealWndHandler.cpp
    SysdataMgr.cpp
    XmlEditor.cpp
    XmlParser.cpp
    SToolBar.cpp
    spugixml/pugixml.cpp
    Dialog/DlgAbout.cpp
    Dialog/DlgCreatePro.cpp
    Dialog/DlgInput.cpp
    Dialog/DlgNewLayout.cpp
    Dialog/DlgResMgr.cpp
    Dialog/DlgInsertXmlElement.cpp
)

set(CURRENT_HEADERS
    stdafx.h
    CDebug.h
    res/resource.h
    CmdLine.h
    DesignWnd.h
    FileHelper.h
    Global.h
    helpapi.h
    MainDlg.h
    ResManger.h
    resource.h
    DropTarget.h
    souidlgs.h
    ScintillaWnd.h
    SImageBtnEx.h
    SImgCanvas.h
    SouiEditorApp.h
    SouiRealWndHandler.h
    SysdataMgr.h
    xpm_icons.h
    XmlEditor.h
    XmlParser.h
    SToolBar.h
    spugixml/pugiconfig.hpp
    spugixml/pugixml.hpp
    Dialog/DlgAbout.h
    Dialog/DlgCreatePro.h
    Dialog/DlgInput.h
    Dialog/DlgNewLayout.h
    Dialog/DlgResMgr.h
    Dialog/DlgInsertXmlElement.h
    SImgCanvas.h
    ${PROJECT_SOURCE_DIR}/controls.extend/SCtrlsRegister.h
)

file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

source_group("SoUI Resouece" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})


add_executable(uieditor WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC})
add_dependencies(uieditor ExtendCtrls)
target_link_libraries(uieditor soui4 utilities4 Scintilla ExtendCtrls)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
target_link_libraries(uieditor imm32)
else()
target_link_libraries(uieditor swinx)
endif()

target_precompile_headers(uieditor PRIVATE "stdafx.h")
set_target_properties (uieditor PROPERTIES
    FOLDER demos/uieditor
)
