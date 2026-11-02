#!/bin/sh
# build + run the swinx FreeRTOS host-emulation timer probe (scratch harness).
# Prereq: cmake --build /tmp/build-frt --target swinx
set -e
SW=/home/flyhigh/work/soui4/swinx
cd /tmp/frt-probe
INC="-I$SW/src/platform/freertos/stl -I$SW/src/platform/freertos/uuid -I$SW -I$SW/include -I$SW/src -I$SW/src/platform/freertos -I$SW/thirdparty/cairo/src"
DEF="-DSOUI_FREERTOS_HOST_EMU -DSOUI_PLATFORM_FREERTOS -DWCHAR_SIZE=__SIZEOF_WCHAR_T__"
FLAGS="-g -std=gnu++11 -Wno-deprecated-declarations -Wno-attributes -include$SW/src/swinx_stl.h"
if [ ! -f gdiobj/gdi.o ]; then
    mkdir -p gdiobj
    for f in $SW/src/gdi/cairo/*.cpp; do
        b=$(basename "$f" .cpp)
        [ "$b" = fontapi ] && continue   # needs std::once_flag, absent from the freertos stl/mutex shim
        c++ $FLAGS $DEF $(pkg-config --cflags cairo fontconfig) $INC -c "$f" -o "gdiobj/$b.o" 2>>gdi-err.txt || echo "skip $b"
    done
    c++ -g -std=gnu++11 -c gdi_stubs.cpp -o gdiobj/stubs.o
fi
c++ $FLAGS $DEF $(pkg-config --cflags cairo fontconfig) $INC \
    probe_timer.cpp -o probe gdiobj/*.o /tmp/build-frt/libswinx.a \
    $(pkg-config --libs cairo fontconfig) -lfreetype -lpthread
exec timeout 60 ./probe
