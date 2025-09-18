#
# 1394020320@qq.com
#

set(SOUI_INCLUDE ${CMAKE_SOURCE_DIR})

include_directories(${SOUI_INCLUDE}/components/resprovider-zip)

add_subdirectory(copy_files)
add_subdirectory(uieditor)
add_subdirectory(uiviewer)
