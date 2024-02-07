
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_HAS_TR1_NAMESPACE)

message("project src dir = ${PROJECT_SOURCE_DIR}")
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/third-part/gtest/include)

file(GLOB_RECURSE CURRENT_HEADERS *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)


if(OUTPATH_WITHOUT_TYPE)
    link_directories(${PROJECT_BINARY_DIR}/bin)
else()
if (CMAKE_CL_64)
    if("${CMAKE_GENERATOR_PLATFORM}" MATCHES "ARM64")
    link_directories(${PROJECT_SOURCE_DIR}/bin64a)
    else()
    link_directories(${PROJECT_SOURCE_DIR}/bin64)
    endif()
else ()
    link_directories(${PROJECT_SOURCE_DIR}/bin)
endif ()
endif()
add_executable(fun_test ${CURRENT_HEADERS} ${CURRENT_SRCS})

add_dependencies(fun_test utilities4 gtest platform)
target_link_libraries(fun_test utilities4 gtest platform)
if (NOT CMAKE_SYSTEM_NAME MATCHES Windows)
target_link_libraries(fun_test pthread)
endif()

set_target_properties (fun_test PROPERTIES
    FOLDER demos
)
