option(SOUI_ENABLE_GAME_TEST "Enable game test" OFF)
# 音效播放 SPlaySound 由 utilities4 提供(utilities/include/SPlaySound.h),
# 各 client 均已无条件 include/链接 utilities4,无需在此额外配置
if(CMAKE_SYSTEM_NAME MATCHES Darwin)
    find_package(GLEW REQUIRED)
    find_package(glfw3 REQUIRED)
endif()
add_subdirectory(upgrade)
add_subdirectory(cnchess)
add_subdirectory(junqi)