#ifndef _UIMSG_H_
#define _UIMSG_H_
#include <platform.h>
#include <sdef.h>
#include <cairo/cairo.h>

SNSBEGIN
struct Msg : MSG{
    virtual ~Msg(){}
};

struct MsgPaint: Msg{
    cairo_region_t * rgn;

    MsgPaint(cairo_region_t * src=nullptr):rgn(src){}
    ~MsgPaint(){
        if(rgn){
            cairo_region_destroy(rgn);
        }
    }
};

SNSEND

#endif//_UIMSG_H_