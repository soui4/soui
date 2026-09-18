# Chinese Chess algorithm test module

# 设置包含目录
include_directories(${PROJECT_SOURCE_DIR}/third-part/gtest/include)
include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/third-part/jsoncpp/include)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../algorithm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../server)

# 收集源文件(排除 EndgameTableTest.cpp: 它依赖服务器端源码, 归属 cnchess_endgame_test 目标)
file(GLOB CNCHESS_TEST_SRC 
    "*.cpp"
)

# 创建可执行文件
add_executable(cnchess_test 
    ${CNCHESS_TEST_SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/../server/EndgameConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../server/PropBag.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../server/Game.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../server/CnChess.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../server/RobotAIPool.cpp
)

target_link_libraries(cnchess_test
    cnchess_algorithm 
    gtest 
    jsoncpp
    log4z
    utilities4
    taskloop
    ws
)

# 端到端测试直接读取真实残局配置文件
target_compile_definitions(cnchess_test PRIVATE
    "ENDGAME_TEST_JSON=_T(\"${CMAKE_CURRENT_SOURCE_DIR}/../server/config/endgames.json\")"
)

# 设置库属性
set_target_properties(cnchess_test PROPERTIES
    FOLDER "games/cnchess"
)
