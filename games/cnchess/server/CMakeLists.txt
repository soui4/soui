# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../algorithm)

# Source files
file(GLOB_RECURSE SERVER_HEADERS *.h *.hpp)
file(GLOB_RECURSE SERVER_SOURCES *.cpp)
file(GLOB_RECURSE SERVER_RESOURCES *.rc *.rc2)

source_group("Header Files" FILES ${SERVER_HEADERS})
source_group("Source Files" FILES ${SERVER_SOURCES})
source_group("Resource Files" FILES ${SERVER_RESOURCES})

set(current_project "cnchess_server")

# 创建可执行文件
add_executable(${current_project}
    ${SERVER_SOURCES}
    ${SERVER_HEADERS}
    ${SERVER_RESOURCES}
)

# 链接库
target_link_libraries(${current_project}
    utilities4
    cnchess_algorithm
    ws
    log4z
)


if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(${current_project} swinx)
endif()

# 打包主题资源为 pc_theme.zip 并复制到输出目录
set(PC_ZIP_NAME pc_theme.zip)
set(MOBILE_ZIP_NAME mobile_theme.zip)
set(PC_THEME_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../client/pc_theme)
set(MOBILE_THEME_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../client/mobile_theme)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(PC_THEME_ZIP_FILE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/${PC_ZIP_NAME})
    set(MOBILE_THEME_ZIP_FILE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/${MOBILE_ZIP_NAME})
    # Windows: 使用 PowerShell Compress-Archive
    add_custom_command(TARGET ${current_project} POST_BUILD
        COMMAND powershell -Command
            "Compress-Archive -Path '${PC_THEME_SOURCE_DIR}/*' -DestinationPath '${PC_THEME_ZIP_FILE}' -Force"
        COMMENT "Packaging ${PC_ZIP_NAME}"
    )
    add_custom_command(TARGET ${current_project} POST_BUILD
        COMMAND powershell -Command
            "Compress-Archive -Path '${MOBILE_THEME_SOURCE_DIR}/*' -DestinationPath '${MOBILE_THEME_ZIP_FILE}' -Force"
        COMMENT "Packaging ${MOBILE_ZIP_NAME}"
    )
else()
    # Unix: 使用 zip 命令
    add_custom_command(TARGET ${current_project} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>
        COMMAND zip -r ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/${PC_ZIP_NAME} .
        WORKING_DIRECTORY ${PC_THEME_SOURCE_DIR}
        COMMENT "Packaging ${PC_ZIP_NAME}"
    )
    add_custom_command(TARGET ${current_project} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>
        COMMAND zip -r ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/${MOBILE_ZIP_NAME} .
        WORKING_DIRECTORY ${MOBILE_THEME_SOURCE_DIR}
        COMMENT "Packaging ${MOBILE_ZIP_NAME}"
    )
endif()

set_target_properties (${current_project} PROPERTIES
    FOLDER "games/cnchess"
)
