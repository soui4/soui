#
# 1394020320@qq.com
#

if(NOT CMAKE_SYSTEM_NAME MATCHES Windows)
add_compile_options(-Wno-deprecated-declarations)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lexlib)

file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS *.cxx)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("Resource Files" FILES ${CURRENT_RC})

add_definitions(-DSCI_LEXER -DSTATIC_BUILD -D_CRT_SECURE_NO_WARNINGS)

add_library(Scintilla STATIC ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_RC})

if (NOT CMAKE_SYSTEM_NAME MATCHES Windows)
   add_dependencies(Scintilla swinx)
endif()

set(COM_LIBS ${COM_LIBS} Scintilla CACHE INTERNAL "com_lib")
set_target_properties (Scintilla PROPERTIES
    FOLDER third-part
)

install(TARGETS Scintilla
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS Scintilla
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
