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
    set_target_properties(${exe_name} PROPERTIES
        INSTALL_RPATH "@executable_path/../Frameworks;@loader_path/../Frameworks"
        BUILD_WITH_INSTALL_RPATH TRUE
        MACOSX_RPATH TRUE
    )
endif()
endmacro()

# 添加macOS资源文件夹, app为可执行文件名, res_path为资源路径, res_name为资源名
macro(add_macos_res_folder app res_path res_name)
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

endif(APPLE)
endmacro()

macro(add_app_res_files app_name res_files dest_path)
    add_custom_command(TARGET ${app_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${app_name}>/${dest_path}"
        COMMENT "Creating Frameworks directory in ${app_name}"
    )
    foreach(res_file ${res_files})
        # 检查文件是否存在（在配置时检查）
        if(EXISTS "${res_file}")
            #message(STATUS "resource file found: ${res_file}")
            get_filename_component(res_id "${res_file}" NAME)
            add_custom_command(TARGET ${app_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${res_file}"
                    "$<TARGET_FILE_DIR:${app_name}>/${dest_path}/${res_id}"
                COMMENT "Copying ${res_file} to ${app_name} "
            )
        else()
            message(WARNING "resource file not found: ${res_file}")
        endif()
    endforeach()
endmacro()


macro(add_app_res_folder app_name res_path res_name)
    file(GLOB_RECURSE DATA_FILES ${res_path}/*)
    add_custom_command(TARGET ${app_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${app_name}>/${res_name}"
        COMMENT "Creating Frameworks directory in ${app_name}"
    )
    foreach(res_file ${DATA_FILES})
        # 检查文件是否存在（在配置时检查）
        if(EXISTS "${res_file}")
            #message(STATUS "resource file found: ${res_file}")
            get_filename_component(res_id "${res_file}" NAME)
            file(RELATIVE_PATH relative_file "${res_path}" "${res_file}")
            get_filename_component(relative_dir ${relative_file} DIRECTORY)

            add_custom_command(TARGET ${app_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${res_file}"
                    "$<TARGET_FILE_DIR:${app_name}>/${res_name}/${relative_dir}/${res_id}"
                COMMENT "Copying ${res_file} to ${app_name} "
            )
        else()
            message(WARNING "resource file not found: ${res_file}")
        endif()
    endforeach()
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

macro(set_macos_icon app icon plist_in)
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
      MACOSX_BUNDLE_INFO_PLIST ${plist_in}
   )

endif(APPLE)
endmacro()

# 宏：复制dylib列表到macOS bundle的Frameworks目录
# 参数：
#   app_name - 应用程序目标名称
#   dylib_list - 要复制的dylib文件路径列表
macro(copy_frameworks_to_bundle app_name framework_list)
if(APPLE)
    # 创建Frameworks目录
    add_custom_command(TARGET ${app_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_BUNDLE_DIR:${app_name}>/Contents/Frameworks"
        COMMENT "Creating Frameworks directory in ${app_name} bundle"
    )

    # 复制每个dylib文件
    foreach(dylib_path ${framework_list})
        # 检查文件是否存在（在配置时检查）
        if(EXISTS "${dylib_path}")
            #message(STATUS "Dylib file found: ${dylib_path}")
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

endif(APPLE)
endmacro()

# 宏：复制资源目录中的所有文件到目标目录，并能检测文件变化
# 参数：
#   app_name - 应用程序目标名称
#   res_path - 资源目录路径
#   dest_path - 目标目录路径
macro(add_app_res_folder_with_deps app_name res_path dest_path)
    file(GLOB_RECURSE RES_FILES ${res_path}/*)
    
    # 创建目标目录
    add_custom_command(TARGET ${app_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${app_name}>/${dest_path}"
        COMMENT "Creating directory ${dest_path}"
    )
    
    # 为每个资源文件创建复制命令
    foreach(res_file ${RES_FILES})
        file(RELATIVE_PATH relative_file "${res_path}" "${res_file}")
        get_filename_component(relative_dir ${relative_file} DIRECTORY)
        
        add_custom_command(TARGET ${app_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${app_name}>/${dest_path}/${relative_dir}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${res_file}"
                "$<TARGET_FILE_DIR:${app_name}>/${dest_path}/${relative_dir}"
            DEPENDS ${res_file}
            COMMENT "Copying ${relative_file}"
        )
    endforeach()
endmacro()

# 添加macOS资源文件夹，能检测文件变化, app为可执行文件名, res_path为资源路径, res_name为资源名
macro(add_macos_res_folder_with_deps app res_path res_name)
if (APPLE)
    # 添加整个文件夹作为资源
    file(GLOB_RECURSE DATA_FILES ${res_path}/*)
    foreach(file ${DATA_FILES})
        #message(STATUS "add resource ${file}")
        target_sources(${app} PRIVATE ${file})
        file(RELATIVE_PATH relative_file "${res_path}" "${file}")
        get_filename_component(relative_dir ${relative_file} DIRECTORY)
        set_source_files_properties(${file} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/${res_name}/${relative_dir})
        # 添加对文件的依赖，确保文件变化时重新构建
        set_source_files_properties(${file} PROPERTIES 
            OBJECT_DEPENDS ${file})
    endforeach()

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

endif(APPLE)
endmacro()