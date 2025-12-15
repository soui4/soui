# Upgrade game algorithm module

# 设置包含目录
include_directories(${PROJECT_SOURCE_DIR}/third-part/gtest/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)


# 收集源文件
file(GLOB UPGRADE_ALGORITHM_SRC 
    "*.cpp"
)

# 创建静态库
add_executable(upgrade_test ${UPGRADE_ALGORITHM_SRC})

target_link_libraries(upgrade_test
    upgrade_algorithm gtest
)
# 设置库属性
set_target_properties(upgrade_test PROPERTIES
    FOLDER "games/upgrade"
)
