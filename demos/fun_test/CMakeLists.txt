
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

add_executable(fun_test ${CURRENT_HEADERS} ${FUN_TEST_SRC})

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
if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows|iOS" AND NOT IS_OHOS)
    add_custom_command(TARGET fun_test POST_BUILD
        COMMAND $<TARGET_FILE:fun_test> --gtest_filter=-window.* --gtest_color=yes
        COMMENT "fun_test: running unit tests (excluding interactive window.*)..."
        VERBATIM
    )
endif()