include_directories(${PROJECT_SOURCE_DIR}/third-part/jsoncpp/include)
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/soui-sys-resource)
include_directories(${PROJECT_SOURCE_DIR}/controls.extend)
include_directories(${PROJECT_SOURCE_DIR}/controls.extend/gridctrl)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})


file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico *.png *.jpg *.bmp)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)
file(GLOB_RECURSE GRIDCTRL_SRC ${PROJECT_SOURCE_DIR}/controls.extend/gridctrl/*.*)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resource" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})

set(current_project "fundholdings_demo")
add_executable(${current_project} WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC} ${GRIDCTRL_SRC})

add_dependencies(${current_project} soui4 httpclient)
target_link_libraries(${current_project} soui4)

if (CMAKE_SYSTEM_NAME MATCHES Windows)
    add_dependencies(${current_project} soui-sys-resource)
endif()

# httpclient 作为 COM 组件提供 IHttpClient，运行时由 SComMgr2 自动加载
# libhttpclient.dll（位于 build/bin）。显式链接以登记 DLL 依赖。
target_link_libraries(${current_project} httpclient)
# 原生 WinHTTP：用于访问基金详情页取回会话 Cookie（东财部分网络需要）
target_link_libraries(${current_project} winhttp)

set_target_properties (${current_project} PROPERTIES
    FOLDER demos
)
