TEMPLATE = subdirs
TARGET = soui4
CONFIG(x64){
TARGET = $$TARGET"64"
}
DEPENDPATH += .
INCLUDEPATH += .

SUBDIRS += third-part
SUBDIRS += utilities
SUBDIRS += soui-sys-resource
SUBDIRS += soui-sys-resource2
SUBDIRS += soui
SUBDIRS += components
SUBDIRS += demo
SUBDIRS += demo2

soui.depends += utilities4 soui-sys-resource
demo.depends += soui
demo2.depends += soui
