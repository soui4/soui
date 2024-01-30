#ifndef __STRFUN_H_
#define __STRFUN_H_
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#define _vscprintf vprintf
#define vsprintf_s vsnprintf

inline const uint8_t *_mbsinc(const uint8_t * srcU8){
    wchar_t wc;
    int bytes = mbtowc(&wc,(const char*)srcU8,5);
    if(bytes>0)
        return srcU8+bytes;
    else
        return NULL;
}

inline uint8_t * _mbscvt(uint8_t * srcU8,BOOL bLower){
    uint8_t *p=srcU8;
    while(*p){
        uint8_t *next = (uint8_t *)_mbsinc(p);
        if(!next) break;
        if(next-p == 1){
            if(bLower)
                *p = tolower(*p);
            else
                *p = toupper(*p);
        }
        p=next;
    }
    return srcU8;
}

#define _mbslwr(x) _mbscvt(x,TRUE)
#define _mbsupr(x) _mbscvt(x,FALSE)


#define _mbsstr(s1,s2) strstr((const char*)(s1),(const char *)(s2))

#define _mbschr(s1,c) strchr((const char*)(s1),(c))
#define _mbsrchr(s1,c) strrchr((const char*)(s1),(c))
#define _mbsicmp(s1,s2) strcasecmp((const char*)(s1),(const char *)(s2))
#define _mbscmp(s1,s2) strcmp((const char*)(s1),(const char *)(s2))
#define _ismbcspace isspace
#define _vscwprintf(fmt,args) vswprintf(NULL,0,fmt,args)
#define vswprintf_s vswprintf

inline wchar_t * _wcslwr(wchar_t* s) {
    wchar_t *str = s;
    while (*str) {
        *str = towlower(*str);
        str++;
    }
    return s;
}

inline wchar_t * _wcsupr(wchar_t* s) {
    wchar_t *str = s;
    while (*str) {
        *str = towupper(*str);
        str++;
    }
    return s;
}

#define _wcsicmp(s1,s2) wcscasecmp(s1,s2)

#endif
