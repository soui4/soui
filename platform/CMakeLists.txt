# 
# SOUI cmake�����ļ�
#
add_definitions(-DPLATFORM_EXPORTS)


if (CMAKE_SYSTEM_NAME MATCHES Windows)
set(SRC_DIR "windows")
elseif(CMAKE_SYSTEM_NAME MATCHES Darwin)
set(SRC_DIR "mac")
else()
set(SRC_DIR "linux")
endif()

file(GLOB_RECURSE HEADERS_COM include/*.h include/*.hpp interface/*.h)
file(GLOB_RECURSE HEADERS_PLATFORM  ${SRC_DIR}/include/*.hpp ${SRC_DIR}/include/*.h)
file(GLOB_RECURSE CURRENT_SRCS  ${SRC_DIR}/src/*.cpp)

source_group("Header Files(Com)" FILES ${HEADERS_COM})
source_group("Header Files(Platform)" FILES ${HEADERS_PLATFORM})
source_group("Source Files" FILES ${CURRENT_SRCS})

if (NOT ENABLE_SOUI_CORE_LIB)
    add_library(platform SHARED ${CURRENT_SRCS} ${HEADERS_COM} ${HEADERS_PLATFORM})
    if(CMAKE_SYSTEM_NAME MATCHES Linux)
    target_link_libraries(
        platform
        cairo xcb-util
    )
    endif()

else()
    add_library(platform STATIC ${CURRENT_SRCS} ${HEADERS_COM} ${HEADERS_PLATFORM})
endif()

target_include_directories(platform
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/include
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/include
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
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/include" DESTINATION platform/include)


