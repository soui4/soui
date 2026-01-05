# Junqi algorithm test module

# 设置包含目录
include_directories(${PROJECT_SOURCE_DIR}/third-part/gtest/include)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../algorithm)
# 收集源文件
file(GLOB JUNQI_TEST_SRC 
    "*.cpp"
)

# 创建可执行文件
add_executable(junqi_test ${JUNQI_TEST_SRC})

target_link_libraries(junqi_test
    junqi_algorithm gtest
)

# 设置库属性
set_target_properties(junqi_test PROPERTIES
    FOLDER "games/junqi"
)

