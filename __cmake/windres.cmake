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

if(CMAKE_SYSTEM_NAME MATCHES Windows)
    set(ENABLE_RESOURCES_BUILD ON)
else()
    # Find windres (MinGW resource compiler)
if(EXISTS /usr/local/bin/x86_64-w64-mingw32-windres OR EXISTS /usr/bin/x86_64-w64-mingw32-windres)
            set(WINDRES_EXE x86_64-w64-mingw32-windres)
        endif()

    # Find required tools
    find_program(LD_EXECUTABLE NAMES ld)
    find_program(OBJCOPY_EXECUTABLE NAMES objcopy)
    find_program(NM_EXECUTABLE NAMES nm)

    if(NOT WINDRES_EXE)
        message(STATUS "windres not found. Resource files (.rc) will not be compiled. "
                        "Install mingw-w64-tools or similar package to enable resource compilation.")
    endif()
    if(NOT LD_EXECUTABLE)
        message(STATUS "ld not found. Resource files (.rc) will not be compiled. "
                        "Install mingw-w64-tools or similar package to enable resource compilation.")
    endif()

    if(NOT OBJCOPY_EXECUTABLE)
        message(STATUS "objcopy not found. Resource files (.rc) will not be compiled. "
                        "Install mingw-w64-tools or similar package to enable resource compilation.")
    endif()

    if(NOT NM_EXECUTABLE)
        message(STATUS "nm not found. Resource files (.rc) will not be compiled. "
                        "Install mingw-w64-tools or similar package to enable resource compilation.")
    endif() 
    if(WINDRES_EXE AND LD_EXECUTABLE AND OBJCOPY_EXECUTABLE AND NM_EXECUTABLE)
        message(STATUS "Found windres: ${WINDRES_EXE}, ld: ${LD_EXECUTABLE}, objcopy: ${OBJCOPY_EXECUTABLE}, nm: ${NM_EXECUTABLE}")
        set(ENABLE_RESOURCES_BUILD ON)
    else()
        set(ENABLE_RESOURCES_BUILD OFF)
    endif()
endif()


# Function to compile a single .rc file to .o object file (ELF format for Linux)
# windres_compile_rc(<output_var> <rc_file> [INCLUDE_DIRS dir1 dir2 ...])
function(windres_compile_rc output_var rc_file) 
    cmake_parse_arguments(ARG "" "" "INCLUDE_DIRS" ${ARGN})
    
    message(STATUS "Compiling resource file ${rc_file} to ELF (via COFF conversion), include dirs: ${ARG_INCLUDE_DIRS}")
    get_filename_component(rc_name ${rc_file} NAME_WE)
    get_filename_component(rc_abs_path ${rc_file} ABSOLUTE)
    
    # Output files
    set(coff_obj "${CMAKE_CURRENT_BINARY_DIR}/${rc_name}_res.coff.o")
    set(elf_obj "${CMAKE_CURRENT_BINARY_DIR}/${rc_name}_res.o")
    set(final_obj "${CMAKE_CURRENT_BINARY_DIR}/${rc_name}_res.final.o")
    set(symbol_list "${CMAKE_CURRENT_BINARY_DIR}/${rc_name}_symbols.txt")
    set(rename_script "${CMAKE_CURRENT_BINARY_DIR}/${rc_name}_rename.txt")

    set(include_flags "-I${PROJECT_BINARY_DIR}/config") # Always include the config directory for generated headers
    list(APPEND include_flags "-I${PROJECT_SOURCE_DIR}/soui-sys-resource") # Include soui-sys-resource for resource headers
    foreach(dir ${ARG_INCLUDE_DIRS})
        # do not propagate empty or generator-expression values
        if(NOT dir STREQUAL "" AND NOT dir MATCHES "\$<")
            list(APPEND include_flags "-I${dir}")
        endif()
    endforeach()

    # Step 1: Compile .rc to COFF format (Windows format)
    add_custom_command(
        OUTPUT ${coff_obj}
        COMMAND ${WINDRES_EXE}
                --target=pe-x86-64  # Compile for 64-bit Windows
                ${include_flags}
                -i "${rc_abs_path}"
                -o "${coff_obj}"
                -O coff  # Explicitly output COFF format
        DEPENDS ${rc_abs_path}
        COMMENT "Step 1: Compiling resource to COFF: ${rc_file}"
        VERBATIM
    )
    
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
        COMMAND ${CMAKE_COMMAND} -DNM_EXECUTABLE=${NM_EXECUTABLE} -Delf_obj=${elf_obj} -Drename_script=${rename_script} -Dsymbol_list=${symbol_list} -P "${PROJECT_SOURCE_DIR}/__cmake/process_symbols.cmake"
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
endfunction()

# Function to add resource files to a target
# target_compile_resources(<target> <rc_file1> [<rc_file2> ...])
function(target_compile_resources target) 
    if(NOT ENABLE_RESOURCES_BUILD)
        message(STATUS "windres not found, skipping resource compilation for target: ${target}")
        return()
    endif()
    
    if(NOT TARGET ${target})
        message(FATAL_ERROR "target_compile_resources: target '${target}' does not exist")
    endif()
    if(NOT CMAKE_SYSTEM_NAME MATCHES Windows)
        # Parse arguments
        cmake_parse_arguments(ARG "" "" "INCLUDE_DIRS" ${ARGN})
        message(STATUS "target_compile_resources: ${target} ${ARG_UNPARSED_ARGUMENTS}, include dirs: ${ARG_INCLUDE_DIRS}")
        set(resource_objects)
        # Process all resource files
        foreach(rc_file ${ARG_UNPARSED_ARGUMENTS})
            # Check if file has .rc extension
            get_filename_component(rc_ext ${rc_file} EXT)
            string(TOLOWER ${rc_ext} rc_ext_lower)
            if(NOT rc_ext_lower STREQUAL ".rc")
                message(STATUS "Skipping non-rc file: ${rc_file}")
                continue()
            endif()
            windres_compile_rc(rc_obj ${rc_file} ${ARG_INCLUDE_DIRS})
            list(APPEND resource_objects ${rc_obj})
        endforeach()                

        # Add object files to target
        if(resource_objects)
            target_sources(${target} PRIVATE ${resource_objects})
            # Set linker options to export symbols
            target_link_options(${target} PRIVATE 
                -rdynamic
                -Wl,--version-script=${PROJECT_SOURCE_DIR}/__cmake/export_res_sym.lst
            )
            # Set linker options to keep the resource section
            if(CMAKE_SYSTEM_NAME MATCHES Linux)
                target_link_options(${target} PRIVATE "LINKER:--no-gc-sections")
            endif()
        endif()
    endif(NOT CMAKE_SYSTEM_NAME MATCHES Windows)
    # Add ENABLE_BUILD_RES define to target
    target_compile_definitions(${target} PRIVATE ENABLE_BUILD_RESOURCE)
endfunction()
