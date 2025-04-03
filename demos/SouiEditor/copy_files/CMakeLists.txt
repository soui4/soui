# copy_files/CMakeLists.txt

# 初始化TARGET_DIR变量
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
set(TARGET_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)")
else()
set(TARGET_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()

# 添加自定义目标
add_custom_target(copy_files ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TARGET_DIR}
)

set(UIEDITOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
add_custom_command(TARGET copy_files PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${UIEDITOR_DIR}/copy_files/soui-sys-resource.zip
            ${TARGET_DIR}
    COMMENT "Copying ${FILE} to ${TARGET_DIR}/soui-sys-resource.zip"
)

add_custom_command(TARGET copy_files PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${UIEDITOR_DIR}/uieditor/uires
            ${TARGET_DIR}/uieditor_uires
    COMMENT "Copying ${FILE} to ${TARGET_DIR}"
)
add_custom_command(TARGET copy_files PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${UIEDITOR_DIR}/uieditor/Config
            ${TARGET_DIR}/Config
    COMMENT "Copying ${FILE} to ${TARGET_DIR}"
)


set_target_properties (copy_files PROPERTIES
    FOLDER demos/uieditor
)
