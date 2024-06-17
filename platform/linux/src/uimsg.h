#ifndef _UIMSG_H_
#define _UIMSG_H_
#include <platform.h>
#include <sdef.h>

SNSBEGIN
struct Msg : MSG{
    virtual ~Msg(){}
};

struct MsgCreate: Msg,CREATESTRUCT
{
    MsgCreate(){
        wParam = 0;
        lParam = (LPARAM)(CREATESTRUCT*)this;
    }
};

SNSEND

#endif//_UIMSG_H_