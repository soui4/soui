# windres.cmake - CMake module for compiling Windows resource files on Linux/Unix
#
# This module provides functions to compile .rc resource files using MinGW's windres tool
# and link them into the target binary.
#
# Usage:
#   include(windres)
#   target_compile_resources(target_name resource1.rc resource2.rc ...)
#
# The compiled resources will be linked into the target as object files.
set(SOUI_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "Current directory for windres.cmake")

# 工具查找
if(CMAKE_SYSTEM_NAME MATCHES Windows)
    set(ENABLE_RESOURCES_BUILD ON)
else()
    # 1. 查找 MinGW windres
    find_program(WINDRES_EXE NAMES x86_64-w64-mingw32-windres)
    if(NOT WINDRES_EXE)
        message(STATUS "windres not found. Resource files (.rc) will not be compiled.")
        set(ENABLE_RESOURCES_BUILD OFF)
        return()
    else()
        message(STATUS "windres = ${WINDRES_EXE} found.")
    endif()

    #ffind_program(WINDRES_EXE NAMES x86_64-w64-mingw32-windres) will set WINDRES_EXE to full path, which result in build failed and report winres.h not found.
    set(WINDRES_EXE x86_64-w64-mingw32-windres)
    # 2. 根据平台确定后续工具
    if(APPLE)
        find_program(AS_EXECUTABLE as)
        if(AS_EXECUTABLE)
            message(STATUS "as = ${AS_EXECUTABLE} found.")
            set(ENABLE_RESOURCES_BUILD ON)
        else()
            message(STATUS "as not found. Resource files (.rc) will not be compiled.")
            set(ENABLE_RESOURCES_BUILD OFF)
        endif()
    elseif(CMAKE_SYSTEM_NAME MATCHES Linux)
        find_program(LD_EXECUTABLE NAMES ld)
        find_program(OBJCOPY_EXECUTABLE NAMES objcopy)
        find_program(NM_EXECUTABLE NAMES nm)
        if(LD_EXECUTABLE AND OBJCOPY_EXECUTABLE AND NM_EXECUTABLE)
            message(STATUS "ld = ${LD_EXECUTABLE}, objcopy = ${OBJCOPY_EXECUTABLE}, nm = ${NM_EXECUTABLE}")
            set(ENABLE_RESOURCES_BUILD ON)
        else()
            message(STATUS "ld, objcopy, nm not found. Resource files (.rc) will not be compiled.")
            set(ENABLE_RESOURCES_BUILD OFF)
        endif()
    else()
        message(STATUS "Unsupported platform for resource compilation")
        set(ENABLE_RESOURCES_BUILD OFF)
    endif()
endif()

# 函数：windres_compile_rc
# 将 .rc 编译为适合当前平台的目标文件（ELF on Linux, Mach-O on macOS）
function(windres_compile_rc output_var rc_file)
    cmake_parse_arguments(ARG "" "" "INCLUDE_DIRS" ${ARGN})
    get_filename_component(rc_name "${rc_file}" NAME_WE)
    get_filename_component(rc_abs_path "${rc_file}" ABSOLUTE)

    set(coff_obj "${CMAKE_CURRENT_BINARY_DIR}/${rc_name}_res.coff.o")

    # 准备包含路径
    set(include_flags)
    list(APPEND include_flags "-I${PROJECT_BINARY_DIR}/config")
    list(APPEND include_flags "-I${PROJECT_SOURCE_DIR}/soui-sys-resource")
    foreach(dir ${ARG_INCLUDE_DIRS})
        if(NOT dir STREQUAL "" AND NOT dir MATCHES "\\$<")
            list(APPEND include_flags "-I${dir}")
        endif()
    endforeach()

    # Step 1: 生成 COFF 目标文件
    add_custom_command(
            OUTPUT ${coff_obj}
            COMMAND ${WINDRES_EXE} --target=pe-x86-64 ${include_flags} -i "${rc_abs_path}" -o "${coff_obj}" -O coff
            DEPENDS ${rc_abs_path}
            COMMENT "Compiling ${rc_file} to COFF"
    )

    if(APPLE)
        # macOS: 编译为 Mach-O 对象文件
        set(macho_obj "${CMAKE_CURRENT_BINARY_DIR}/${rc_name}_res.o")
        set(asm_file "${CMAKE_CURRENT_BINARY_DIR}/${rc_name}.s")

        # Step 2: 生成汇编文件，嵌入整个 COFF 文件
        set(asm_content "
            .section __DATA,__data
            .globl __binary_soui_coff_o_start
            .globl __binary_soui_coff_o_end

            __binary_soui_coff_o_start:
                .incbin \"${coff_obj}\"
            __binary_soui_coff_o_end:
                .byte 0
        ")
        file(GENERATE OUTPUT ${asm_file} CONTENT "${asm_content}")

        # Step 3: 汇编生成 Mach-O 对象
        add_custom_command(
            OUTPUT ${macho_obj}
            COMMAND ${AS_EXECUTABLE} -o ${macho_obj} ${asm_file}
            DEPENDS ${asm_file} ${coff_obj}
            COMMENT "Assembling into Mach-O: ${macho_obj}"
        )
        set(${output_var} ${macho_obj} PARENT_SCOPE)

    else()
        set(elf_obj "${CMAKE_CURRENT_BINARY_DIR}/${rc_name}_res.o")
        set(final_obj "${CMAKE_CURRENT_BINARY_DIR}/${rc_name}_res.final.o")
        set(symbol_list "${CMAKE_CURRENT_BINARY_DIR}/${rc_name}_symbols.txt")
        set(rename_script "${CMAKE_CURRENT_BINARY_DIR}/${rc_name}_rename.txt")
            # Step 2: Link COFF to ELF format using ld
            add_custom_command(
                OUTPUT ${elf_obj}
                COMMAND ${LD_EXECUTABLE}
                        -r
                        -b binary "${coff_obj}"
                        -o "${elf_obj}"
                DEPENDS ${coff_obj}
                COMMENT "Step 2: Link COFF to ELF format using ld: ${rc_name}"
                VERBATIM
            )
            
            # Step 3: Extract symbols and create rename script directly
            add_custom_command(
                OUTPUT ${rename_script} ${symbol_list}
                COMMAND ${CMAKE_COMMAND} -E echo "# Symbol rename script for ${rc_name}" > ${rename_script}
                COMMAND ${NM_EXECUTABLE} -g ${elf_obj} > ${symbol_list}
                COMMAND ${CMAKE_COMMAND} -DNM_EXECUTABLE=${NM_EXECUTABLE} -Delf_obj=${elf_obj} -Drename_script=${rename_script} -Dsymbol_list=${symbol_list} -P "${SOUI_CMAKE_DIR}/process_symbols.cmake"
                DEPENDS ${elf_obj}
                COMMENT "Step 3: Extracting symbols with nm -g and generating rename script for ${rc_name}"
                VERBATIM
            )
            
            # Step 4: Apply symbol renaming using objcopy
            add_custom_command(
                OUTPUT ${final_obj}
                COMMAND ${OBJCOPY_EXECUTABLE} --redefine-syms=${rename_script} ${elf_obj} ${final_obj}
                DEPENDS ${elf_obj} ${rename_script}
                COMMENT "Step 4: Renaming symbols to use fixed 'soui' prefix: ${rc_name}"
                VERBATIM
            )
            
            # Optional: Verify the renamed symbols with nm -g
            set(final_symbols "${CMAKE_CURRENT_BINARY_DIR}/${rc_name}_symbols_final.txt")
            add_custom_command(
                OUTPUT ${final_symbols}
                COMMAND ${NM_EXECUTABLE} -g ${final_obj} > ${final_symbols}
                DEPENDS ${final_obj}
                COMMENT "Listing final global symbols for ${rc_name}"
                VERBATIM
            )

            # Add all outputs as targets to ensure they're built
            add_custom_target(${rc_name}_res_target ALL
                DEPENDS ${final_obj} ${final_symbols}
                COMMENT "Resource compilation target for ${rc_file}"
            )

            # Return the final object file path
            set(${output_var} ${final_obj} PARENT_SCOPE)
    endif()
endfunction()

# 函数：target_compile_resources
function(target_compile_resources target)
    if(NOT ENABLE_RESOURCES_BUILD)
        message(STATUS "Resource compilation disabled for target ${target}")
        return()
    endif()
    if(NOT TARGET ${target})
        message(FATAL_ERROR "target_compile_resources: target '${target}' does not exist")
    endif()
    if(NOT CMAKE_SYSTEM_NAME MATCHES Windows)
        cmake_parse_arguments(ARG "" "" "INCLUDE_DIRS" ${ARGN})
        set(resource_objects)
        foreach(rc_file ${ARG_UNPARSED_ARGUMENTS})
            get_filename_component(rc_ext "${rc_file}" EXT)
            string(TOLOWER "${rc_ext}" rc_ext_lower)
            if(NOT rc_ext_lower STREQUAL ".rc")
                continue()
            endif()
            windres_compile_rc(rc_obj ${rc_file} ${ARG_INCLUDE_DIRS})
            list(APPEND resource_objects ${rc_obj})
        endforeach()
        if(resource_objects)
            target_sources(${target} PRIVATE ${resource_objects})
            if(CMAKE_SYSTEM_NAME MATCHES Linux)
                # Set linker options to export symbols
                target_link_options(${target} PRIVATE 
                    -rdynamic
                    -Wl,--version-script=${PROJECT_SOURCE_DIR}/__cmake/export_res_sym.lst
                )
                # 保持资源段不被 GC 删除
                target_link_options(${target} PRIVATE -Wl,--no-gc-sections)
            endif()
        endif()
    endif()
    # Add ENABLE_BUILD_RES define to target
    target_compile_definitions(${target} PRIVATE ENABLE_BUILD_RESOURCE)
endfunction()