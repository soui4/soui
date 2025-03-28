#
#
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}src)

set(headers
	src/lapi.h
	src/lauxlib.h
	src/lcode.h
	src/lctype.h
	src/ldebug.h
	src/ldo.h
	src/lfunc.h
	src/lgc.h
	src/llex.h
	src/llimits.h
	src/lmem.h
	src/lobject.h
	src/lopcodes.h
	src/lparser.h
	src/lstate.h
	src/lstring.h
	src/ltable.h
	src/ltm.h
	src/lua.h
	src/lua.hpp
	src/luaconf.h
	src/lualib.h
	src/lundump.h
	src/lvm.h
	src/lzio.h
)

set(sources
	src/lapi.c
	src/lauxlib.c
	src/lbaselib.c
	src/lcode.c
	src/lcorolib.c
	src/lctype.c
	src/ldblib.c
	src/ldebug.c
	src/ldo.c
	src/ldump.c
	src/lfunc.c
	src/lgc.c
	src/linit.c
	src/liolib.c
	src/llex.c
	src/lmathlib.c
	src/lmem.c
	src/loadlib.c
	src/lobject.c
	src/lopcodes.c
	src/loslib.c
	src/lparser.c
	src/lstate.c
	src/lstring.c
	src/lstrlib.c
	src/ltable.c
	src/ltablib.c
	src/ltm.c
	src/lundump.c
	src/lvm.c
	src/lzio.c
	src/lutf8lib.c
)

source_group("Header Files" FILES ${headers})
source_group("Source Files" FILES ${sources})

add_library(lua-54 STATIC ${sources} ${headers})
if (NOT CMAKE_SYSTEM_NAME MATCHES Windows)
target_compile_options(lua-54 PRIVATE -fPIC)
endif()
set(COM_LIBS ${COM_LIBS} lua-54 CACHE INTERNAL "com_lib")
set_target_properties (lua-54 PROPERTIES
    FOLDER third-part
)

install(TARGETS lua-54
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS lua-54
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
