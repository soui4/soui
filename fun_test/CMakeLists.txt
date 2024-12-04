
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_HAS_TR1_NAMESPACE -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)

file(GLOB_RECURSE CURRENT_HEADERS *.h *.hpp)

include_directories(  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PUBLIC ${PROJECT_SOURCE_DIR}/utilities/include
  PUBLIC ${PROJECT_SOURCE_DIR}/third-part/gtest/include
  PUBLIC ${PROJECT_SOURCE_DIR}/components
  )


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

set(FUN_TEST_SRC
test.cpp
ScintillaWnd.cpp
test_hostwnd.cpp
test_native.cpp
)

add_executable(fun_test ${CURRENT_HEADERS} ${FUN_TEST_SRC})

add_dependencies(fun_test utilities4 gtest soui4 resprovider-zip log4z  render-gdi  render-skia taskloop Scintilla)
target_include_directories(fun_test 
    PUBLIC ${PROJECT_SOURCE_DIR}/third-part/Scintilla/include
)
target_link_libraries(fun_test utilities4 gtest soui4 Scintilla)
if (NOT CMAKE_SYSTEM_NAME MATCHES Windows)
    add_dependencies(fun_test swinx)
    target_link_libraries(fun_test swinx pthread xcb xcb-util dl)
else()
    target_link_libraries(fun_test Msimg32 imm32)
endif()

set_target_properties (fun_test PROPERTIES
    FOLDER demos
)


set(IPC_SVR_SRC
ipc_svr.cpp
)
add_executable(ipc_svr ${CURRENT_HEADERS} ${IPC_SVR_SRC})

add_dependencies(ipc_svr utilities4 soui4 resprovider-zip log4z  render-gdi  render-skia taskloop)
target_link_libraries(ipc_svr utilities4 soui4)
if (NOT CMAKE_SYSTEM_NAME MATCHES Windows)
add_dependencies(ipc_svr swinx)
target_link_libraries(ipc_svr swinx)
else()
target_link_libraries(ipc_svr Msimg32)
endif()

set_target_properties (ipc_svr PROPERTIES
    FOLDER demos
)



set(IPC_SVR_CLIENT
ipc_client.cpp
)
add_executable(ipc_client ${CURRENT_HEADERS} ${IPC_SVR_CLIENT})

add_dependencies(ipc_client utilities4 soui4 resprovider-zip log4z  render-gdi  render-skia taskloop)
target_link_libraries(ipc_client utilities4 soui4)
if (NOT CMAKE_SYSTEM_NAME MATCHES Windows)
add_dependencies(ipc_client swinx)
target_link_libraries(ipc_client swinx)
else()
target_link_libraries(ipc_client Msimg32)
endif()

set_target_properties (ipc_client PROPERTIES
    FOLDER demos
)
