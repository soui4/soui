
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_HAS_TR1_NAMESPACE -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)

file(GLOB_RECURSE CURRENT_HEADERS *.h *.hpp)

include_directories(  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PUBLIC ${PROJECT_SOURCE_DIR}/utilities/include
  PUBLIC ${PROJECT_SOURCE_DIR}/third-part/gtest/include
  PUBLIC ${PROJECT_SOURCE_DIR}/components
)

# CONFIGURE_DEPENDS: new test_*.cpp files are picked up on rebuild without
# requiring an explicit cmake re-run
file(GLOB FUN_TEST_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB SOUI_CORE_TEST_SRC CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/SOUI/tests/test_*.cpp)
file(GLOB SOUI_INTEGRATION_TEST_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test_integration_*.cpp)
file(GLOB SOUI_E2E_TEST_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test_e2e_*.cpp)
list(APPEND FUN_TEST_SRC ${SOUI_CORE_TEST_SRC})

add_executable(fun_test ${CURRENT_HEADERS} ${FUN_TEST_SRC})
target_compile_definitions(fun_test PRIVATE
    SOUI_FUN_TEST_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")

# CMake 3.16 can scan TEST/TEST_F sources and register each case separately.
# File groups make future tests part of the corresponding CTest label without
# editing a fixed --gtest_filter list. Parameterized tests need separate review.
include(GoogleTest)
gtest_add_tests(TARGET fun_test
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test_region.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/test_string.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/test_utilities.cpp
    TEST_LIST SOUI_BASE_UNIT_TESTS)
gtest_add_tests(TARGET fun_test
    SOURCES ${SOUI_CORE_TEST_SRC}
    TEST_LIST SOUI_CORE_UNIT_TESTS)
gtest_add_tests(TARGET fun_test
    SOURCES ${SOUI_INTEGRATION_TEST_SRC}
    TEST_LIST SOUI_INTEGRATION_TESTS)
gtest_add_tests(TARGET fun_test
    SOURCES ${SOUI_E2E_TEST_SRC}
    TEST_LIST SOUI_E2E_TESTS)

foreach(SOUI_TEST_GROUP SOUI_BASE_UNIT_TESTS SOUI_CORE_UNIT_TESTS
        SOUI_INTEGRATION_TESTS SOUI_E2E_TESTS)
    if(NOT DEFINED ${SOUI_TEST_GROUP} OR "${${SOUI_TEST_GROUP}}" STREQUAL "")
        message(FATAL_ERROR "${SOUI_TEST_GROUP} is empty; basic SOUI validation would silently skip a layer")
    endif()
endforeach()
set_tests_properties(${SOUI_BASE_UNIT_TESTS} ${SOUI_CORE_UNIT_TESTS}
    PROPERTIES LABELS soui-unit TIMEOUT 60)
set_tests_properties(${SOUI_INTEGRATION_TESTS}
    PROPERTIES LABELS soui-integration TIMEOUT 60)
set_tests_properties(${SOUI_E2E_TESTS}
    PROPERTIES LABELS soui-e2e TIMEOUT 60)

# An opt-in native-window smoke check, run with a desktop display (Xvfb on
# Linux). It is separate from the required 36-case headless PR entry.
option(SOUI_ENABLE_GUI_SMOKE "Register the SOUI native-window smoke test" OFF)
if(SOUI_ENABLE_GUI_SMOKE)
    gtest_add_tests(TARGET fun_test
        SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test_gui_smoke.cpp
        TEST_LIST SOUI_GUI_SMOKE_TESTS)
    if("${SOUI_GUI_SMOKE_TESTS}" STREQUAL "")
        message(FATAL_ERROR "SOUI GUI smoke group is empty")
    endif()
    set_tests_properties(${SOUI_GUI_SMOKE_TESTS}
        PROPERTIES LABELS soui-gui TIMEOUT 30)
endif()

target_include_directories(fun_test
    PUBLIC ${PROJECT_SOURCE_DIR}/third-part/Scintilla/include
)

add_dependencies(fun_test gtest soui4 Scintilla ${COM_LIBS})
target_link_libraries(fun_test gtest soui4 Scintilla)
if (CMAKE_SYSTEM_NAME MATCHES Windows)
    add_dependencies(fun_test soui-sys-resource)
    target_link_libraries(fun_test Msimg32 imm32 Oleacc)
else()
    add_dependencies(fun_test swinx)
    target_link_libraries(fun_test swinx pthread dl)
    # SApplication initializes the RichEdit service even in the headless E2E
    # test. Make sure its module is available when only fun_test is built.
    if(TARGET msftedit)
        add_dependencies(fun_test msftedit)
    endif()
endif()

if(SOUI_ENABLE_COM_LIB)
    target_link_libraries(fun_test ${COM_LIBS} ${COM_DEP_LIBS})
endif()

set_target_properties (fun_test PROPERTIES
    FOLDER demos
)

# Linux/macOS 桌面构建：编译完成后自动执行测试。
# -window.*:windows.* 排除需要用户交互的套件（window.soui / window.gdi，
# 会打开 SOUI/GDI 窗口等待人工操作）；其余用例全自动。
# 测试非零退出（有用例失败）将导致构建失败，便于及时发现问题。
# iOS/OHOS 交叉编译目标是移动设备，不在本机执行。
option(SOUI_FUN_TEST_POST_BUILD "Run non-interactive fun_test suite after building it" ON)
if(SOUI_FUN_TEST_POST_BUILD AND NOT CMAKE_SYSTEM_NAME MATCHES "Windows|iOS" AND NOT IS_OHOS)
    add_custom_command(TARGET fun_test POST_BUILD
        COMMAND $<TARGET_FILE:fun_test> --gtest_filter=-window.* --gtest_color=yes
        COMMENT "fun_test: running unit tests (excluding interactive window.*)..."
        VERBATIM
    )
endif()
