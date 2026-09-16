cmake_minimum_required(VERSION 3.16)
project(SOUI4)

# Register the project's repeatable unit, integration and core E2E checks.
# Test targets are added by their own CMakeLists.txt files below.
enable_testing()

file(STRINGS  SOUI/include/soui-version.h ver_h_contents REGEX "^#define SOUI_VER[0-9]+ [0-9]+$")

foreach(line IN LISTS ver_h_contents) 
    string(REGEX MATCH "SOUI_VER([0-9]+) ([0-9]+)" match "${line}")
    if(match)
        set(SOUI_VER${CMAKE_MATCH_1} ${CMAKE_MATCH_2})
    endif()
endforeach()

message(STATUS "SOUI_VER: ${SOUI_VER1}.${SOUI_VER2}.${SOUI_VER3}.${SOUI_VER4}")
message(STATUS "install prefix=${CMAKE_INSTALL_PREFIX}")
set(CMAKE_CONFIGURATION_TYPES Debug Release)
if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "build type is ${CMAKE_BUILD_TYPE}")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/__cmake/")
include(__cmake/internal_utils.cmake)

set(BUILD_FOR_ARM OFF)
detectArmTarget(BUILD_FOR_ARM)
message(STATUS "BUILD_FOR_ARM: ${BUILD_FOR_ARM}")

if (CMAKE_SYSTEM_NAME STREQUAL "MSYS")
    message(STATUS "Overriding incorrect Linux detection for MSYS")
    set(CMAKE_SYSTEM_NAME Windows CACHE STRING "Target system" FORCE)
endif()
if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND MINGW)
    message(STATUS "Overriding incorrect Linux detection for MINGW")
    set(CMAKE_SYSTEM_NAME Windows CACHE STRING "Target system" FORCE)
endif()
if (CMAKE_SYSTEM_NAME MATCHES "OHOS|OpenHarmony|HarmonyOS")
    set(IS_OHOS ON)
else()
    set(IS_OHOS OFF)
endif()
message(STATUS "IS_OHOS: ${IS_OHOS}")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
if (CMAKE_SYSTEM_NAME MATCHES Windows)
IF (CMAKE_CL_64)
  set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/output_64 CACHE PATH "default install prefix" FORCE)
ELSE (CMAKE_CL_64)
  set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/output_32 CACHE PATH "default install prefix" FORCE)
ENDIF (CMAKE_CL_64)
else()
set (CMAKE_INSTALL_PREFIX "/usr/local/soui5" CACHE PATH "default install prefix" FORCE)
endif()
endif()

if (CMAKE_SYSTEM_NAME MATCHES Windows)
  option(SOUI_USE_UNICODE "Use Unicode" ON)
else()
  option(SOUI_USE_UNICODE "Use Unicode" OFF)
endif()
if (NOT MSVC AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-D_DEBUG)
endif()
# When other libraries are using a shared version of runtime libraries,
# Google Test also has to use one.
option(SOUI_SHARED_CRT "Use shared crt runtime library." ON)

# When other libraries are using a shared version of runtime libraries,
# Google Test also has to use one.
option(SOUI_WCHAR_AS_DEFAULT "Use wchar_t as internal type" ON)
#
#
option(SOUI_ENABLE_CORE_LIB "Enable compile 'core' as static lib" OFF)
#
#
option(SOUI_ENABLE_COM_LIB "Enable compile 'components' as static lib" OFF)

#
#
option(SOUI_XP_TOOLSET "" ON)
#
#
option(SOUI_ENABLE_SPECTRE "Enable Spectre Mitigation" OFF)
#
#
option(SOUI_BUILD_DEMOS "Build builtin demos" ON)

option(SOUI_BUILD_GAMES "Build soui games" ON)

if(CMAKE_SYSTEM_NAME MATCHES "iOS")
set(SOUI_BUILD_TOOLS OFF)
add_definitions(-D__IOS__) #//add __IOS__ for ios build.
else()
option(SOUI_BUILD_TOOLS "Build soui tools" ON)
endif()
#
#
option(SOUI_BUILD_HTTPCLIENT "Build HTTP client component" ON)

#
#
option(SOUI_ENABLE_SVG "Enable SVG support" ON)

message(STATUS "MSVC_VERSION: ${MSVC_VERSION}")
if(MSVC AND MSVC_VERSION LESS 1800)
set(SOUI_BUILD_WS OFF)
message(STATUS "Build Websocket module disabled for VS2013 or lower")
else()
if(EXISTS "${PROJECT_SOURCE_DIR}/third-part/libwebsockets/CMakeLists.txt")
option(SOUI_BUILD_WS "Build Websocket module" ON)
else()
set(SOUI_BUILD_WS OFF)
endif()
endif()

if(IS_OHOS)
set(SOUI_BUILD_RICHEDIT ON CACHE BOOL "using richedit from source code" FORCE)
set(SOUI_ENABLE_ACC OFF)
else(IS_OHOS)
if (CMAKE_SYSTEM_NAME MATCHES Windows)
option(SOUI_BUILD_RICHEDIT "using richedit from source code" OFF)
option(SOUI_ENABLE_SPY "Enable Swnd Spy" OFF)
if (MSVC)
    add_compile_options(/MP)
endif()
else()
set(SOUI_BUILD_RICHEDIT ON)
set(SOUI_ENABLE_SPY OFF)
endif()
option(SOUI_ENABLE_ACC "Enable MSAA/IAccessible (provided by swinx)" OFF)
endif(IS_OHOS)

include(__cmake/global.cmake)
include(__cmake/windres.cmake)
message(STATUS "ENABLE_RESOURCES_BUILD: ${ENABLE_RESOURCES_BUILD}")
config_compiler_and_linker()


if (CMAKE_SYSTEM_NAME MATCHES Windows)
  if(MSVC_VERSION LESS 1600)
      include_directories(${PROJECT_SOURCE_DIR}/third-part/stdint)
  endif()
  add_definitions(-DWCHAR_SIZE=2)
else()
        if(NOT EXISTS "${PROJECT_SOURCE_DIR}/swinx/CMakeLists.txt")
                message(WARNING "swinx submodule not found. Attempting to initialize it...")
                find_package(Git QUIET)
                if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
                        message(STATUS "Initializing and updating sinx submodule...")
                        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init -- swinx
                                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                                RESULT_VARIABLE GIT_SUBMOD_RESULT)
                        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
                                message(FATAL_ERROR "git submodule update failed with ${GIT_SUBMOD_RESULT}, please checkout submodules manually")
                        endif()
                else()
                        message(WARNING "Git not found or not a git repository. Please manually initialize the libwebsockets submodule")
                endif()
        endif()
        
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/swinx/CMakeLists.txt")
                add_subdirectory(swinx)
        else()
                message(FATAL_ERROR "swinx submodule not available, please checkout submodules manually")
        endif()

  # 添加调试标志
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")
  if(CMAKE_SYSTEM_NAME MATCHES Linux)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
  endif()
  if(CMAKE_SYSTEM_NAME MATCHES Darwin)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0")
  add_compile_options(-Wno-deprecated-declarations)
  add_compile_options(-Wno-expansion-to-defined)
  add_compile_options(-Wno-inconsistent-missing-override)
  add_compile_options(-Wno-parentheses)
  add_compile_options(-Wno-unused-value)
  add_compile_options(-Wno-switch)
  add_compile_options(-Wno-unsequenced)
  add_compile_options(-Wno-undefined-var-template)
  add_compile_options(-Wno-c++11-extensions)
  set(CMAKE_CXX_STANDARD 11)
  else()
  set(CMAKE_SKIP_BUILD_RPATH FALSE)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
  set(CMAKE_INSTALL_RPATH $ORIGIN)
  endif()

  add_definitions(-DWCHAR_SIZE=4)
  include_directories(${PROJECT_SOURCE_DIR}/swinx/include)
endif()

configure_file("config/config.h.in" "${PROJECT_BINARY_DIR}/config/config.h" @ONLY)
configure_file("config/build.cfg.in" "${PROJECT_BINARY_DIR}/config/build.cfg" @ONLY)
include_directories(${PROJECT_BINARY_DIR}/config)
install(FILES "${PROJECT_BINARY_DIR}/config/config.h" DESTINATION include/config)
install(FILES "${PROJECT_BINARY_DIR}/config/build.cfg" DESTINATION include/config)

# 设置库文件的输出路径
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_definitions(-D_CRT_NONSTDC_NO_WARNINGS -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
link_directories(${CMAKE_BINARY_DIR}/bin)
if(NOT CMAKE_SYSTEM_NAME MATCHES Windows)
        set(CMAKE_CXX_STANDARD 11)
endif()
add_subdirectory(third-part)
add_subdirectory(utilities)
add_subdirectory(components)
add_subdirectory(SOUI)
add_subdirectory(soui-sys-resource)
add_subdirectory(soui-sys-resource2)
add_subdirectory(soui-sys-resource-dark)
add_subdirectory(soui-sys-resource-light)
add_subdirectory(controls.extend)
if(SOUI_BUILD_TOOLS)
        add_subdirectory(tools)
endif()
if(SOUI_BUILD_DEMOS)
        add_subdirectory(demos)
endif(SOUI_BUILD_DEMOS)
if(SOUI_BUILD_GAMES AND TARGET websockets)
        add_subdirectory(games) 
endif()
if(NOT CMAKE_SYSTEM_NAME MATCHES Windows)
set(SWINX_LIBS swinx)
set(SWINX_CFLAGS swinx/include)
endif()

# ---------------------------------------------------------------------------
# Aggregate the *public* (installed) libraries exposed to consumers.
#
# In the default (shared) build COM_DEP_LIBS / SWINX_DEP_LIBS are empty (those
# variables are only populated when SOUI_ENABLE_CORE_LIB=ON), so they add
# nothing here. When soui4 is built as a set of static libraries
# (SOUI_ENABLE_CORE_LIB=ON), those third-party dependencies are NOT bundled
# into the static archives and MUST be exposed so the consumer can link them.
# Appending them unconditionally is therefore correct for both configurations.
# ---------------------------------------------------------------------------
set(SOUI_PUBLIC_LIBS ${CORE_LIBS} ${COM_LIBS} ${OPTION_LIBS}
    ${SOUI_UTILITIES} ${SOUI_SWINX}
    ${COM_DEP_LIBS} ${SWINX_DEP_LIBS})
list(REMOVE_DUPLICATES SOUI_PUBLIC_LIBS)

set(SOUI_PC_LIBS "")
foreach(_l ${SOUI_PUBLIC_LIBS})
    if(_l)
        list(APPEND SOUI_PC_LIBS "-l${_l}")
    endif()
endforeach()
list(REMOVE_DUPLICATES SOUI_PC_LIBS)
string(REPLACE ";" " " SOUI_PC_LIBS "${SOUI_PC_LIBS}")

# Configure the (single) soui4-config.cmake template — reused for both direct
# include() and find_package(soui4). Installed to __cmake for the legacy
# include(), and to lib/cmake/soui4/ for find_package(soui4).
configure_file("${PROJECT_SOURCE_DIR}/__cmake/soui4-config.cmake.in"
               "${PROJECT_BINARY_DIR}/soui4-config.cmake" @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/soui4-config.cmake"
        DESTINATION ${CMAKE_INSTALL_PREFIX}/__cmake)

# ---------------------------------------------------------------------------
# find_package(soui4) support.
# The generated soui4-config.cmake is installed as the package config for the
# single canonical package name "soui4". A matching <name>-config-version file
# is emitted for version-gated find_package() requests.
# ---------------------------------------------------------------------------
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/soui4-config-version.cmake"
    VERSION "${SOUI_VER1}.${SOUI_VER2}.${SOUI_VER3}.${SOUI_VER4}"
    COMPATIBILITY SameMajorVersion)

install(FILES "${PROJECT_BINARY_DIR}/soui4-config.cmake"
        RENAME soui4-config.cmake
        DESTINATION lib/cmake/soui4)
install(FILES "${PROJECT_BINARY_DIR}/soui4-config-version.cmake"
        DESTINATION lib/cmake/soui4)

# ---------------------------------------------------------------------------
# pkg-config file (soui4.pc) so soui4 can be consumed without CMake.
# ---------------------------------------------------------------------------
configure_file("${PROJECT_SOURCE_DIR}/__cmake/soui4.pc.in"
               "${PROJECT_BINARY_DIR}/soui4.pc" @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/soui4.pc"
        DESTINATION lib/pkgconfig)


# Install cmake files
install(DIRECTORY ${PROJECT_SOURCE_DIR}/__cmake
        DESTINATION ${CMAKE_INSTALL_PREFIX}
        FILES_MATCHING PATTERN "*.cmake"
)
install(FILES ${PROJECT_SOURCE_DIR}/__cmake/plist.in ${PROJECT_SOURCE_DIR}/__cmake/dpi_aware.manifest ${PROJECT_SOURCE_DIR}/__cmake/export_res_sym.lst
        DESTINATION ${CMAKE_INSTALL_PREFIX}/__cmake
)

install(FILES 
        #${PROJECT_SOURCE_DIR}/simsun.ttc
        ${PROJECT_SOURCE_DIR}/soui-sys-resource.zip
        DESTINATION ${CMAKE_INSTALL_PREFIX}/resources
)

install(FILES ${PROJECT_SOURCE_DIR}/LICENSE
        DESTINATION ${CMAKE_INSTALL_PREFIX}
)
