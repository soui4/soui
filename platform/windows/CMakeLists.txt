# 
# SOUI cmake配置文件
#
add_definitions(-DPLATFORM_EXPORTS)

file(GLOB_RECURSE CURRENT_HEADERS include/*.h include/*.hpp)
file(GLOB_RECURSE CURRENT_SRCS  src/*.cpp)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})

if (NOT ENABLE_SOUI_CORE_LIB)
    add_library(platform SHARED ${CURRENT_SRCS} ${CURRENT_HEADERS})
else()
    add_library(platform STATIC ${CURRENT_SRCS} ${CURRENT_HEADERS})
endif()

target_include_directories(platform
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..
  	PRIVATE ${PROJECT_SOURCE_DIR}/utilities/include
)

set(CORE_LIBS ${CORE_LIBS} platform)
set(CORE_LIBS ${CORE_LIBS} platform CACHE INTERNAL "core_lib")

install(TARGETS platform
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS platform
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include" DESTINATION platform/include)
