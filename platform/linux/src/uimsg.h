#ifndef _UIMSG_H_
#define _UIMSG_H_
#include <platform.h>
#include <sdef.h>
#include "gdi.h"

SNSBEGIN
struct Msg : MSG{
    virtual ~Msg(){}
};

struct MsgPaint: Msg{
    HRGN rgn;
    MsgPaint(HRGN src=nullptr):rgn(src){}
    ~MsgPaint(){
        if(rgn){
            DeleteObject(rgn);
        }
    }
};

SNSEND

#endif//_UIMSG_H_