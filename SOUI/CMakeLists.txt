#
# SOUI4 cmake配置文件
#

#
# 1394020320@qq.com
#

add_definitions(-DSOUI_EXPORTS -D_CRT_SECURE_NO_WARNINGS -D_M_AMD64)

file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_UIRES  *.xml *.idx *.ico)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

file(GLOB SRC_ROOT src/*.cpp)
file(GLOB_RECURSE SRC_VALUEANI src/valueAnimator/*.cpp)
set(SRC_TEST 
    src/test.cpp
)
source_group("src_root" FILES ${SRC_ROOT})
source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resource" FILES ${CURRENT_UIRES})
source_group("Resource Files" FILES ${CURRENT_RC})

set(source ${CURRENT_HEADERS} ${SRC_TEST} )

if (NOT ENABLE_SOUI_CORE_LIB)
    #set (source  ${source} soui.rc)
    add_library(soui4 SHARED ${source})

else()
    add_library(soui4 STATIC ${source})
endif()

target_include_directories(soui4
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/interface
  PUBLIC ${PROJECT_SOURCE_DIR}/utilities/include
)
add_dependencies(soui4 utilities4 platform)
target_link_libraries(soui4 utilities4 platform)

#target_precompile_headers(soui4 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/souistd.h")
set(CORE_LIBS ${CORE_LIBS} soui4 CACHE INTERNAL "core_lib")

install(TARGETS soui4
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS soui4
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include" DESTINATION include/SOUI)
