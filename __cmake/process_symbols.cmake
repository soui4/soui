# CMake script to process symbols and generate rename script

# Extract global symbols using nm -g
execute_process(
    COMMAND ${NM_EXECUTABLE} -g ${elf_obj}
    OUTPUT_VARIABLE NM_OUTPUT
    RESULT_VARIABLE NM_RESULT
    ERROR_VARIABLE NM_ERROR
)

if(NOT NM_RESULT EQUAL 0)
    message(FATAL_ERROR "nm failed: ${NM_ERROR}")
endif()

# Parse symbols and create rename mapping
set(RENAME_ENTRIES "# Symbol rename script\n")
string(REPLACE "\n" ";" NM_LINES "${NM_OUTPUT}")
foreach(LINE ${NM_LINES})
    # Skip empty lines
    string(STRIP "${LINE}" LINE)
    if(NOT LINE STREQUAL "")
        # Split the line by spaces
        string(REGEX REPLACE "^ +" "" LINE "${LINE}")  # Remove leading spaces
        string(REGEX REPLACE " +" ";" LINE_FIELDS "${LINE}")
        # Get the third field (symbol name)
        list(GET LINE_FIELDS 2 SYM_NAME)
        if(SYM_NAME MATCHES "_coff_o_(start|end|size)")
            # Generate new symbol name
            string(REGEX REPLACE "_binary_(.*)_(coff_o_start|coff_o_end|coff_o_size)" 
                   "_binary_soui_\\2" NEW_SYM_NAME "${SYM_NAME}")
            
            # Add to rename entries
            set(RENAME_ENTRIES "${RENAME_ENTRIES}${SYM_NAME} ${NEW_SYM_NAME}\n")
        endif()
    endif()
endforeach()

# Write rename script
file(WRITE ${rename_script} "${RENAME_ENTRIES}")

# Also save raw symbol list for debugging
file(WRITE ${symbol_list} "${NM_OUTPUT}")
