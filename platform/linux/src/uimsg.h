#ifndef _UIMSG_H_
#define _UIMSG_H_
#include <platform.h>
#include <sdef.h>
#include "gdi.h"

SNSBEGIN
struct Msg : MSG{
    virtual ~Msg(){}
    virtual Msg* clone() const{
        Msg *ret = new Msg;
        memcpy(ret,this,sizeof(MSG));
        return ret;
    }
};

struct MsgPaint: Msg{
    HRGN rgn;
    MsgPaint(HRGN src=nullptr):rgn(src){}
    ~MsgPaint(){
        if(rgn){
            DeleteObject(rgn);
        }
    }
    virtual Msg* clone() const{
        MsgPaint *ret = new MsgPaint;
        memcpy(ret,this,sizeof(MSG));
        ret->rgn = CreateRectRgn(0,0,0,0);
        CombineRgn(ret->rgn,rgn,nullptr,RGN_COPY);
        ret->lParam = (LPARAM)ret->rgn;
        return ret;
    }
};

struct MsgWndPosChanged: Msg{
    WINDOWPOS pos;

    virtual Msg* clone() const{
        MsgWndPosChanged *ret = new MsgWndPosChanged;
        memcpy(ret,this,sizeof(MsgWndPosChanged));
        ret->lParam= (LPARAM) &ret->pos;
        return ret;
    }
};

SNSEND

#endif//_UIMSG_H_