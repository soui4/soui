add_definitions(-DSOUI_EXPORTS -D_CRT_SECURE_NO_WARNINGS)

if (CMAKE_SYSTEM_NAME MATCHES Windows)
set(ENABLE_THUNK ON)
else()
option(ENABLE_THUNK "Enable Thunk" OFF)
endif()
#set(ENABLE_THUNK 0) #open this line for centos or fedora
if(${ENABLE_THUNK}) 
message("---enable thunk in soui")
add_definitions(-DENABLE_THUNK=1)
else()
message("---disable thunk in soui")
add_definitions(-DENABLE_THUNK=0)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "iOS|Android")
    add_definitions(-DDISABLE_TOOLTIP)
endif()

set(SOUI_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_UIRES  *.xml *.idx *.ico)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)
file(GLOB_RECURSE NANOSVG_HEADERS  ${SOUI_ROOT_DIR}/third-part/nanosvg/nanosvg.h)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resource" FILES ${CURRENT_UIRES})
source_group("Resource Files" FILES ${CURRENT_RC})
source_group("nanosvg" FILES ${NANOSVG_HEADERS})


if (NOT CMAKE_SYSTEM_NAME MATCHES Windows)
# 添加 -fpermissive 选项到 C++ 编译器标志
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive" )
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpermissive")
add_compile_options(-Wno-conversion-null -Wno-pointer-arith -Wno-format)
endif()
 
set(source ${CURRENT_HEADERS} ${NANOSVG_HEADERS} ${CURRENT_SRCS} ${CURRENT_UIRES})

if (NOT SOUI_ENABLE_CORE_LIB)
    if (CMAKE_SYSTEM_NAME MATCHES Windows)
        set (source  ${source} soui.rc)
    endif()
    add_library(soui4 SHARED ${source})
else()
    add_library(soui4 STATIC ${source})
endif()
target_precompile_headers(soui4 PRIVATE "include/souistd.h")

if(IS_OHOS)
    target_include_directories(soui4 PRIVATE ${PROJECT_SOURCE_DIR}/swinx/include)
    find_library(OHOS_ACE_NAPI ace_napi.z)
    find_library(OHOS_ACE_NDK ace_ndk.z)
    find_library(OHOS_HILOG hilog_ndk.z)
    find_library(OHOS_RAWFILE rawfile.z rawfile)
    foreach(_ohos_lib IN ITEMS OHOS_ACE_NAPI OHOS_ACE_NDK)
        if(${_ohos_lib})
            target_link_libraries(soui4 PRIVATE ${${_ohos_lib}})
        endif()
    endforeach()
    if(OHOS_HILOG)
        target_link_libraries(soui4 PRIVATE ${OHOS_HILOG})
    endif()
    if(OHOS_RAWFILE)
        target_link_libraries(soui4 PRIVATE ${OHOS_RAWFILE})
    endif()
endif()

target_include_directories(soui4
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PUBLIC ${SOUI_ROOT_DIR}/utilities/include
  PUBLIC ${SOUI_ROOT_DIR}/components
  PUBLIC ${SOUI_ROOT_DIR}/controls.extend
  PRIVATE ${SOUI_ROOT_DIR}/third-part/nanosvg
)

add_dependencies(soui4 utilities4)
target_link_libraries(soui4 PUBLIC utilities4)

if (CMAKE_SYSTEM_NAME MATCHES Windows)
    target_link_libraries(soui4 PUBLIC imm32)
else()
    add_dependencies(soui4 swinx)
    target_link_libraries(soui4 PUBLIC swinx)
endif()
if (SOUI_ENABLE_CORE_LIB AND SOUI_BUILD_RICHEDIT)
    target_compile_definitions(soui4 PRIVATE INIT_RICHEDIT)
    target_link_libraries(soui4 PUBLIC msftedit)
endif()

target_include_directories(soui4
  INTERFACE
    "${PROJECT_BINARY_DIR}/config"
)

set(CORE_LIBS soui4 ${CORE_LIBS} CACHE INTERNAL "core_lib")

install(TARGETS soui4
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS soui4
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include" DESTINATION include/SOUI)
