
add_definitions(-DDEFAULT_XKB_RULES=\"evdev\")
add_definitions(-DDEFAULT_XKB_MODEL=\"pc105\")
add_definitions(-DDEFAULT_XKB_LAYOUT=\"us\")
add_definitions(-DDFLT_XKB_CONFIG_ROOT=\"/usr/share/X11/xkb\")

# OUR SOURCES
set(xkbcommon_headers
	xkbcommon/xkbcommon.h
)

set(xkbcommon_sources
src/atom.c
src/context.c
src/xkb-keymap.c # renamed: keymap.c -> xkb-keymap.c
src/x11/keymap.c
src/keysym.c
src/keysym-utf.c
src/state.c
src/text.c
src/context-priv.c
src/keymap-priv.c
src/utils.c
src/utf8.c
src/xkbcomp/action.c
src/xkbcomp/ast-build.c
src/xkbcomp/compat.c
src/xkbcomp/expr.c
src/xkbcomp/include.c
src/xkbcomp/keycodes.c
src/xkbcomp/keymap-dump.c
src/xkbcomp/keymap.c
src/xkbcomp/keywords.c
src/xkbcomp/rules.c
src/xkbcomp/scanner.c
src/xkbcomp/symbols.c
src/xkbcomp/types.c
src/xkbcomp/vmod.c
src/xkbcomp/xkbcomp.c
src/xkbcomp/parser.c
src/x11/util.c
src/x11/state.c
src/x11/keymap.c
)

source_group("Header Files" FILES ${xkbcommon_headers})
source_group("Source Files" FILES ${xkbcommon_sources})

add_library(xkbcommon STATIC ${xkbcommon_sources} ${xkbcommon_headers})
set(COM_LIBS ${COM_LIBS} xkbcommon CACHE INTERNAL "com_lib")
target_include_directories(xkbcommon PRIVATE .)
target_include_directories(xkbcommon PRIVATE ..)
target_include_directories(xkbcommon PRIVATE ../xcb)
target_include_directories(xkbcommon PRIVATE ./src)
set_target_properties(xkbcommon PROPERTIES POSITION_INDEPENDENT_CODE ON)

set_target_properties (xkbcommon PROPERTIES
    FOLDER third-part
)

install(TARGETS xkbcommon
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS xkbcommon
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
