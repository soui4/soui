TEMPLATE = subdirs
TARGET = components
CONFIG(x64){
TARGET = $$TARGET"64"
}
DEPENDPATH += .
INCLUDEPATH += .

SUBDIRS += imgdecoder-wic
SUBDIRS += render-gdi
SUBDIRS += render-skia
SUBDIRS += render-d2d
SUBDIRS += translator
SUBDIRS += resprovider-zip
SUBDIRS += imgdecoder-stb
SUBDIRS += imgdecoder-png
SUBDIRS += imgdecoder-gdip
SUBDIRS += ScriptModule-LUA
SUBDIRS += log4z
SUBDIRS += resprovider-7zip
SUBDIRS += TaskLoop
SUBDIRS += SIpcObject
SUBDIRS += httpclient

imgdecoder-png.depends += zlib png
render-skia.depends += skia
render-d2d.depends += utilities4
resprovider-zip.depends += zlib utilities4
translator.depends += utilities4
resprovider-7zip.depends += 7z utilities4
ScriptModule-LUA.depends += soui4 lua-54
TaskLoop.depends += utilities4
SIpcObject.depends += utilities4
httpclient.depends += utilities4