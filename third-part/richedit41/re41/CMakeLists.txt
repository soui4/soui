
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
-DNOFEPROCESSING
-DNOPRIVATEMESSAGE
-DNOANSIWINDOWS
-DNOMETAFILES
-DNOVERSIONINFO
)

if (NOT CMAKE_SYSTEM_NAME MATCHES Windows)
	add_compile_options(-Wno-invalid-offsetof)
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
#cmsgflt.cpp 
)

source_group("Header Files" FILES ${re_header})
source_group("Source Files" FILES ${re_src})

add_library(msftedit SHARED ${re_src} ${re_header})
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

set_target_properties (msftedit PROPERTIES
    FOLDER third-part
)
