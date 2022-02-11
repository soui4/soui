TEMPLATE = subdirs
TARGET = soui4
CONFIG(x64){
TARGET = $$TARGET"64"
}
DEPENDPATH += .
INCLUDEPATH += .

include(cpy-cfg.pri)

SUBDIRS += third-part
SUBDIRS += utilities
SUBDIRS += soui-sys-resource
SUBDIRS += soui
SUBDIRS += components
SUBDIRS += demo

soui.depends += utilities4 soui-sys-resource
demo.depends += soui
