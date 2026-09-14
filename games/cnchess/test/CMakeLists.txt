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

# ============================================================================
# 残局打谱 - 游戏桌端到端(服务器逻辑层)测试
# 复用服务器端 EndgameConfig / PropBag / Game / CnChess 源码, 验证残局桌模型
# ============================================================================
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../server)
include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/third-part/jsoncpp/include)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/components)

add_executable(cnchess_endgame_test
    EndgameTableTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../server/EndgameConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../server/PropBag.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../server/Game.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../server/CnChess.cpp
)

target_link_libraries(cnchess_endgame_test
    cnchess_algorithm
    gtest
    jsoncpp
    log4z
    utilities4
)

# 端到端测试直接读取真实残局配置文件
target_compile_definitions(cnchess_endgame_test PRIVATE
    "ENDGAME_TEST_JSON=\"${CMAKE_CURRENT_SOURCE_DIR}/../server/config/endgames.json\""
)

set_target_properties(cnchess_endgame_test PROPERTIES
    FOLDER "games/cnchess"
)