add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NON_CONFORMING_SWPRINTFS)

include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/third-part/lua-54/src)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lua_tinker)

set(ScriptModule-lua_header
	src/stdafx.h
	src/ScriptModule-Lua.h
           src/exports/exp_Basic.h
           src/exports/exp_ctrls.h
           src/exports/exp_eventArgs.h
           src/exports/exp_global.h
           src/exports/exp_IApp.h
           src/exports/exp_IContainer.h
           src/exports/exp_ICtrl.h
           src/exports/exp_IEvtArgs.h
           src/exports/exp_IHostWnd.h
           src/exports/exp_INativeWnd.h
           src/exports/exp_IObject.h
           src/exports/exp_IObjRef.h
           src/exports/exp_IResProvider.h
           src/exports/exp_IResProviderMgr.h
           src/exports/exp_IScriptModule.h
           src/exports/exp_ISouifac.h
           src/exports/exp_IString.h
           src/exports/exp_IWindow.h
           src/exports/exp_IXml.h
           src/exports/exp_string.h
           src/exports/exp_Window.h
           src/exports/exp_IMenu.h
           src/exports/exp_IMenuEx.h
           src/exports/exp_IAnimation.h
           src/exports/exp_IInterpolator.h
           src/exports/exp_IValueAnimator.h
           src/exports/luaAnimationListener.h
           src/exports/exp_SysApi.h
           src/exports/luaFunSlot.h
           src/exports/toobj.h
	lua_tinker/lua_tinker.h
)

set(ScriptModule-lua_src
    src/ScriptModule-Lua.cpp
    src/exports/exp_soui.cpp
    lua_tinker/lua_tinker.cpp
)

source_group("Header Files" FILES ${ScriptModule-lua_header})
source_group("Source Files" FILES ${ScriptModule-lua_src})

if (NOT ENABLE_SOUI_COM_LIB)
    set (ScriptModule-Lua_src  ${ScriptModule-lua_src} ScriptModule-lua.rc)
    add_library(ScriptModule-Lua SHARED ${ScriptModule-lua_src} ${ScriptModule-lua_header})
    target_link_libraries(ScriptModule-Lua lua-54 utilities4 soui4)
else()
    add_library(ScriptModule-Lua STATIC ${ScriptModule-lua_src} ${ScriptModule-lua_header})
endif()

if (NOT ENABLE_SOUI_CORE_LIB)
	message(STATUS "[DLL_CORE] ScriptModule-Lua: ON")
    set(COM_LIBS ${COM_LIBS} ScriptModule-Lua CACHE INTERNAL "com_lib")
else()
	message(STATUS "[LIB_CORE] ScriptModule-Lua: OFF")
endif()

set_target_properties (ScriptModule-Lua PROPERTIES
    FOLDER components
)

install(TARGETS ScriptModule-Lua
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS ScriptModule-Lua
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
