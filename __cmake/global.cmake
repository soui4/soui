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