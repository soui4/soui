#
# 1394020320@qq.com
#

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/soui-sys-resource)
include_directories(${PROJECT_SOURCE_DIR}/controls.extend)
include_directories(${PROJECT_SOURCE_DIR}/third-part/Scintilla/include)
include_directories(${PROJECT_SOURCE_DIR}/third-part/wke/include)
file(GLOB_RECURSE CTRLS_SRC ${PROJECT_SOURCE_DIR}/controls.extend/*.*)


add_library(ExtendCtrls STATIC ${CTRLS_SRC})

target_link_libraries(ExtendCtrls soui4 utilities4)

if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
target_compile_options(ExtendCtrls PRIVATE -fPIC)
target_link_libraries(ExtendCtrls swinx)
endif()

# 仅安装当前源码目录下的 .h/.hpp/.hxx/.hh 头文件到 include/ 目录
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/" DESTINATION include/controls.extend FILES_MATCHING PATTERN "*.h")

install(TARGETS ExtendCtrls
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS ExtendCtrls
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
