# Upgrade game algorithm module

# 设置包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

# 收集源文件
file(GLOB UPGRADE_ALGORITHM_SRC 
    "*.cpp"
)

# 创建静态库
add_library(upgrade_algorithm STATIC ${UPGRADE_ALGORITHM_SRC})

# 设置库属性
set_target_properties(upgrade_algorithm PROPERTIES
    FOLDER "games/upgrade"
)
