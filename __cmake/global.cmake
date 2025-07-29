#
# 1394020320@qq.com
#

#option(OUTPATH_WITHOUT_TYPE "Put All generation in same Path" ON)
if ("${OUTPATH_WITHOUT_TYPE}" STREQUAL "")
	set(OUTPATH_WITHOUT_TYPE 1)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "possible configurations" FORCE)

if (ENABLE_SOUI_CORE_LIB)
    message("---Building [soui] with LIB_CORE")
else()
    message("---Building [soui] with DLL_CORE")
endif()
if (ENABLE_SOUI_COM_LIB)
    message("---Building [soui components] with LIB_SOUI_COM")
else()
    message("---Building [soui components] with DLL_SOUI_COM")
endif()

macro( readSettingFile CFG_FILE KEY DEFAULT_RESULT STRING_RESULT_OUT)

    unset(STRING_RESULT)

    file (TO_CMAKE_PATH "$ENV{SOUI4PATH}" SOUIROOTPATH)
    file(STRINGS "${CFG_FILE}" CONFIGSTRING )

    foreach(LINE ${CONFIGSTRING})
        if("${LINE}" MATCHES "${KEY}=.*")
            string(REPLACE "${KEY}=" "" STRING_RESULT ${LINE})
        endif()
    endforeach()

    if("${STRING_RESULT}" STREQUAL "")
        set(STRING_RESULT ${DEFAULT_RESULT} )
    endif()

    set(${STRING_RESULT_OUT} ${STRING_RESULT})
    if ("${KEY}" STREQUAL "MT")
        if (${STRING_RESULT_OUT} STREQUAL "1")
            set(${STRING_RESULT_OUT} "0")
        else()
            set(${STRING_RESULT_OUT} "1")
        endif()
    endif()
endmacro()

set(CORE_LIBS CACHE INTERNAL "core_lib")
set(COM_LIBS CACHE INTERNAL "com_lib")


macro(add_soui_exe exe_name)
if (WIN32)
    add_executable(${exe_name} WIN32 ${ARGN})
elseif (UNIX AND NOT APPLE)
    add_executable(${exe_name} ${ARGN})
elseif (APPLE)
    add_executable(${exe_name} MACOSX_BUNDLE ${ARGN})
endif()
endmacro()

# 添加macOS资源文件夹, app为可执行文件名, res_path为资源路径, res_name为资源名, sys_resources为系统资源包
macro(add_macos_res_folder app res_path res_name sys_resources)
if (APPLE)
# 添加整个文件夹作为资源
file(GLOB_RECURSE DATA_FILES ${res_path}/*)
foreach(file ${DATA_FILES})
    #message(STATUS "add resource ${file}")
    target_sources(${app} PRIVATE ${file})
    file(RELATIVE_PATH relative_file "${res_path}" "${file}")
    get_filename_component(relative_dir ${relative_file} DIRECTORY)
    set_source_files_properties(${file} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/${res_name}/${relative_dir})
endforeach()

# 添加整个文件夹作为资源
file(GLOB_RECURSE SYSRES_FILES ${sys_resources}/*)
foreach(file ${SYSRES_FILES})
    #message(STATUS "add system resource ${file}")
    get_filename_component(ext ${file} EXT)
    if (NOT ${ext} STREQUAL ".rc" AND NOT ${ext} STREQUAL ".txt" AND NOT ${ext} STREQUAL ".h" AND NOT ${ext} STREQUAL ".rc2")
        target_sources(${app} PRIVATE ${file})
        file(RELATIVE_PATH relative_file "${sys_resources}" "${file}")
        get_filename_component(relative_dir ${relative_file} DIRECTORY)
        set_source_files_properties(${file} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/soui-sys-resource/${relative_dir})
    endif()
endforeach()

endif(APPLE)
endmacro()

macro(add_macos_res_file app res_file dest_path)
    if(APPLE)
        message(STATUS "Adding resource file: ${res_file}")
        target_sources(${app} PRIVATE ${res_file})
        set_source_files_properties(${res_file} 
            PROPERTIES 
            MACOSX_PACKAGE_LOCATION "Resources/${dest_path}"
        )

    endif()
endmacro()

macro(set_macos_icon app icon)
if (APPLE)
message(STATUS "add resource ${icon}")
target_sources(${app} PRIVATE ${icon})
set_source_files_properties(${icon} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
get_filename_component(APP_ICON_FILE ${icon} NAME)
set(MACOSX_BUNDLE_ICON_FILE ${APP_ICON_FILE})


if(NOT DEFINED MACOSX_BUNDLE_HIGH_RESOLUTION_CAPABLE)
    set(MACOSX_BUNDLE_HIGH_RESOLUTION_CAPABLE "false")
endif()

set_target_properties(
      ${app}
      PROPERTIES
      MACOSX_BUNDLE_INFO_PLIST ./plist.in
   )

endif(APPLE)
endmacro()

# 宏：复制dylib列表到macOS bundle的Frameworks目录
# 参数：
#   app_name - 应用程序目标名称
#   dylib_list - 要复制的dylib文件路径列表
macro(copy_dylibs_to_bundle app_name dylib_list)
if(APPLE)
    # 创建Frameworks目录
    add_custom_command(TARGET ${app_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_BUNDLE_DIR:${app_name}>/Contents/Frameworks"
        COMMENT "Creating Frameworks directory in ${app_name} bundle"
    )

    # 复制每个dylib文件
    foreach(dylib_path ${dylib_list})
        # 检查文件是否存在（在配置时检查）
        if(EXISTS "${dylib_path}")
            get_filename_component(dylib_name "${dylib_path}" NAME)
            add_custom_command(TARGET ${app_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${dylib_path}"
                    "$<TARGET_BUNDLE_DIR:${app_name}>/Contents/Frameworks/"
                COMMENT "Copying ${dylib_name} to ${app_name} bundle"
            )
        else()
            message(WARNING "Dylib file not found: ${dylib_path}")
        endif()
    endforeach()

    # 设置bundle的运行时库搜索路径
    set_target_properties(${app_name} PROPERTIES
        INSTALL_RPATH "@executable_path/../Frameworks"
        BUILD_WITH_INSTALL_RPATH TRUE
    )

    # 修复库依赖路径（可选，需要fix_bundle_deps.cmake脚本）
    if(EXISTS "${CMAKE_SOURCE_DIR}/__cmake/fix_bundle_deps.cmake")
        add_custom_command(TARGET ${app_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND}
                -DCMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
                -DBUNDLE_NAME=${app_name}
                -DDYLIB_LIST="${dylib_list}"
                -P "${CMAKE_SOURCE_DIR}/__cmake/fix_bundle_deps.cmake"
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Fixing library dependencies in ${app_name} bundle"
        )
    endif()

endif(APPLE)
endmacro()

# 宏：从目标列表复制dylib到bundle（自动检测目标文件）
# 参数：
#   app_name - 应用程序目标名称
#   target_list - CMake目标名称列表
macro(copy_targets_to_bundle app_name target_list)
if(APPLE)
    # 创建Frameworks目录
    add_custom_command(TARGET ${app_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_BUNDLE_DIR:${app_name}>/Contents/Frameworks"
        COMMENT "Creating Frameworks directory in ${app_name} bundle"
    )

    # 复制每个目标的库文件
    foreach(target_name ${target_list})
        if(TARGET ${target_name})
            # 检查目标类型
            get_target_property(target_type ${target_name} TYPE)
            message(STATUS "Target name: ${target_name}, type: ${target_type}")
            if(target_type STREQUAL "SHARED_LIBRARY" OR target_type STREQUAL "MODULE_LIBRARY")
                add_custom_command(TARGET ${app_name} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "$<TARGET_FILE:${target_name}>"
                        "$<TARGET_BUNDLE_DIR:${app_name}>/Contents/Frameworks/"
                    COMMENT "Copying ${target_name} library to ${app_name} bundle"
                )
            else()
                message(STATUS "Skipping ${target_name} (not a shared library)")
            endif()
        else()
            message(WARNING "Target not found: ${target_name}")
        endif()
    endforeach()

    # 设置bundle的运行时库搜索路径
    set_target_properties(${app_name} PROPERTIES
        INSTALL_RPATH "@executable_path/../Frameworks;@loader_path/../Frameworks"
        BUILD_WITH_INSTALL_RPATH TRUE
        MACOSX_RPATH TRUE
    )

    # 创建并使用脚本修复动态库依赖关系
    set(fix_script "${CMAKE_CURRENT_BINARY_DIR}/fix_${app_name}_deps.cmake")

    # 生成修复脚本内容
    set(script_content "# Auto-generated script to fix dylib dependencies\n")
    string(APPEND script_content "set(BUNDLE_DIR \"$<TARGET_BUNDLE_DIR:${app_name}>\")\n")
    string(APPEND script_content "set(EXECUTABLE_PATH \"\${BUNDLE_DIR}/Contents/MacOS/${app_name}\")\n")
    string(APPEND script_content "set(FRAMEWORKS_DIR \"\${BUNDLE_DIR}/Contents/Frameworks\")\n\n")

    # 添加修复主可执行文件依赖的命令
    string(APPEND script_content "# Fix executable dependencies\n")
    foreach(target_name ${target_list})
        if(TARGET ${target_name})
            get_target_property(target_type ${target_name} TYPE)
            if(target_type STREQUAL "SHARED_LIBRARY" OR target_type STREQUAL "MODULE_LIBRARY")
                string(APPEND script_content "execute_process(COMMAND install_name_tool -change \"$<TARGET_FILE:${target_name}>\" \"@executable_path/../Frameworks/$<TARGET_FILE_NAME:${target_name}>\" \"\${EXECUTABLE_PATH}\" ERROR_QUIET)\n")
            endif()
        endif()
    endforeach()

    string(APPEND script_content "\n# Fix dylib install names and inter-dependencies\n")
    foreach(target_name ${target_list})
        if(TARGET ${target_name})
            get_target_property(target_type ${target_name} TYPE)
            if(target_type STREQUAL "SHARED_LIBRARY" OR target_type STREQUAL "MODULE_LIBRARY")
                # 设置install_name
                string(APPEND script_content "execute_process(COMMAND install_name_tool -id \"@rpath/$<TARGET_FILE_NAME:${target_name}>\" \"\${FRAMEWORKS_DIR}/$<TARGET_FILE_NAME:${target_name}>\" ERROR_QUIET)\n")

                # 修复对其他库的依赖
                foreach(dep_target ${target_list})
                    if(TARGET ${dep_target} AND NOT "${target_name}" STREQUAL "${dep_target}")
                        get_target_property(dep_type ${dep_target} TYPE)
                        if(dep_type STREQUAL "SHARED_LIBRARY" OR dep_type STREQUAL "MODULE_LIBRARY")
                            string(APPEND script_content "execute_process(COMMAND install_name_tool -change \"$<TARGET_FILE:${dep_target}>\" \"@loader_path/$<TARGET_FILE_NAME:${dep_target}>\" \"\${FRAMEWORKS_DIR}/$<TARGET_FILE_NAME:${target_name}>\" ERROR_QUIET)\n")
                        endif()
                    endif()
                endforeach()
            endif()
        endif()
    endforeach()

    # 写入脚本文件
    file(GENERATE OUTPUT "${fix_script}" CONTENT "${script_content}")

    # 执行修复脚本
    add_custom_command(TARGET ${app_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -P "${fix_script}"
        COMMENT "Fixing dylib dependencies for ${app_name} bundle"
        VERBATIM
    )

endif(APPLE)
endmacro()