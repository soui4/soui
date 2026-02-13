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

# Source files 
file(GLOB_RECURSE CURRENT_HEADERS *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS *.cpp)
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

set(current_project "cnchess_client")
add_executable(${current_project} WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${UIRES_RES_HEADER} ${CURRENT_RC})
 
add_dependencies(${current_project} soui4 ExtendCtrls)
target_link_libraries(${current_project} soui4 utilities4 ExtendCtrls cnchess_algorithm)

if(SOUI_ENABLE_COM_LIB)
    target_link_libraries(${current_project} ${COM_LIBS})
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
    target_link_libraries(${current_project} swinx)
endif()

set_target_properties (${current_project} PROPERTIES
    FOLDER "games/cnchess"
)

# Copy def_theme directory to output directory
add_custom_command(TARGET ${current_project} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${CMAKE_CURRENT_SOURCE_DIR}/def_theme 
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/cnchess/def_theme
)