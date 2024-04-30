#ifndef _UIMSG_H_
#define _UIMSG_H_
#include <platform.h>
#include <sdef.h>

SNSBEGIN
struct UiMsg : MSG{
    virtual ~UiMsg(){}
};

SNSEND

#endif//_UIMSG_H_