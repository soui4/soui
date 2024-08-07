#ifndef _UIMSG_H_
#define _UIMSG_H_
#include <platform.h>
#include <sdef.h>
#include "gdi.h"

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

struct MsgWndPosChanged: Msg{
    WINDOWPOS pos;
};

#define UM_STATE (WM_INTERNAL+1)

#endif//_UIMSG_H_