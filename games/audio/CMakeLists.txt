
    add_library(win32_audio STATIC
        src/win32_audio.cpp
        src/win32_audio.h
    )
    
    target_include_directories(win32_audio PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    
    target_link_libraries(win32_audio PRIVATE
        winmm
    )
    
set_target_properties (win32_audio PROPERTIES
    FOLDER "games"
)