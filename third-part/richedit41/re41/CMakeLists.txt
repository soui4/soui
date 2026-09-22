
include_directories(../tom41)
include_directories(../lsinc)
include_directories(../inc41)
include_directories(.)

add_definitions(-D_CRT_SECURE_NO_WARNINGS 
-DNOACCESSIBILITY 
-DNODELAYLOAD 
-DUNICODE 
-DNOLISTCOMBOBOXES 
-DNOCOMPLEXSCRIPTS
#-DNOFEPROCESSING
-DNOPRIVATEMESSAGE
-DNOANSIWINDOWS
-DNOMETAFILES
-DNOVERSIONINFO
)

if (NOT CMAKE_SYSTEM_NAME MATCHES Windows)
	add_compile_options(-Wno-invalid-offsetof)
	# msftedit(richedit41) 是第三方 vendored 代码，存在大量 &&/||、&/^ 混用的
	# 风格性警告（-Wlogical-op-parentheses 等）。这些表达式按 C++ 优先级本就计算
	# 正确，只是缺少显式括号。在源码里逐处加括号既脱离实际又容易改错，这里统一在
	# 构建层关闭的本文件作用域（仅影响本 target 及其子目录，不波及 SOUI 代码）。
	add_compile_options(
		-Wno-logical-op-parentheses  # && 于 || 未加括号
		-Wno-shorten-64-to-32        # 64->32 隐式窄化
		-Wno-unused-value            # 表达式结果未使用（如 for 子句命名变量）
		-Wno-parentheses             # 赋值作条件等
		-Wno-unsequenced             # 同表达式内无序列副作用
		-Wno-undefined-var-template  # 模板静态成员未实例化
		-Wno-constant-conversion     # 常量到较小类型的有损转换（如 TOM 高位枚举）
		-Wno-tautological-constant-out-of-range-compare  # 常量比较恒定越界
	)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -w")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -w")
endif()


file(GLOB_RECURSE re_header *.h *.hpp)

set(re_src
antievt.cpp 
array.cpp 
callmgr.cpp 
cfpf.cpp 
clasifyc.cpp 
coleobj.cpp 
debug.cpp 
devdsc.cpp 
dfreeze.cpp 
disp.cpp 
dispml.cpp 
dispprt.cpp 
dispsl.cpp 
doc.cpp 
dragdrp.cpp 
dxfrobj.cpp 
edit.cpp 
font.cpp 
format.cpp 
frunptr.cpp 
hash.cpp 
host.cpp 
ime.cpp 
ldte.cpp 
layout.cpp 
line.cpp 
m_undo.cpp 
magellan.cpp 
measure.cpp 
notmgr.cpp 
object.cpp 
objmgr.cpp 
osdc.cpp 
propchg.cpp 
range.cpp 
reinit.cpp 
render.cpp 
rtext.cpp 
rtflex.cpp 
rtflog.cpp 
rtfread.cpp 
rtfread2.cpp 
rtfwrit.cpp 
rtfwrit2.cpp 
runptr.cpp 
select.cpp 
text.cpp 
textserv.cpp 
tokens.cpp 
tomdoc.cpp 
tomfmt.cpp 
tomrange.cpp 
tomsel.cpp 
urlsup.cpp 
util.cpp 
uuid.cpp 
ols.cpp 
olsole.cpp 
kern.cpp 
hyph.cpp 
#lbhost.cpp 
#cbhost.cpp 
uspi.cpp 
txtbrk.cpp 
iaccess.cpp 
w32sys.cpp 
#w32win32.cpp 
utilmem.cpp 
textnot.cpp 
#cuim.cpp 
cmsgflt.cpp 
)

source_group("Header Files" FILES ${re_header})
source_group("Source Files" FILES ${re_src})

if(NOT SOUI_ENABLE_CORE_LIB)
    add_library(msftedit SHARED ${re_src} ${re_header})
    add_definitions(-D_BUILD_DLL)
else()
    add_library(msftedit STATIC ${re_src} ${re_header})
endif()
if (CMAKE_SYSTEM_NAME MATCHES Windows)
	target_sources(msftedit PRIVATE msftedit.rc msftedit.rc2)
	target_link_libraries(msftedit Usp10.lib msls31 imm32)
    set_target_properties(msftedit PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    set_target_properties(msftedit PROPERTIES LINK_FLAGS "/DEF:msftedit.def")
else()
	add_compile_options(-Wno-invalid-offsetof)
	add_definitions(-DNOINKOBJECT -DNOPRIVATEMESSAGE)
	target_link_libraries(msftedit swinx msls31)
endif()

if(SOUI_ENABLE_CORE_LIB)
    set(CORE_LIBS msftedit ${CORE_LIBS} CACHE INTERNAL "core_lib") 
endif()

set_target_properties (msftedit PROPERTIES
    FOLDER third-part
)


install(TARGETS msftedit
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
)

install(TARGETS msftedit
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
)
