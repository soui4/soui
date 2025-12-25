# Chinese Chess algorithm test module

# 设置包含目录
include_directories(${PROJECT_SOURCE_DIR}/third-part/gtest/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../algorithm)

# 收集源文件
file(GLOB CNCHESS_TEST_SRC 
    "*.cpp"
)

# 创建可执行文件
add_executable(cnchess_test ${CNCHESS_TEST_SRC})

target_link_libraries(cnchess_test
    cnchess_algorithm gtest
)

# 设置库属性
set_target_properties(cnchess_test PROPERTIES
    FOLDER "games/cnchess"
)