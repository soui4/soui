#ifndef __STRFUN_H_
#define __STRFUN_H_
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>
#include <ctypes.h>
#include <errno.h>
#include <pthread.h>

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

#define _mbslwr(x) _mbscvt((uint8_t*)(x),TRUE)
#define _mbsupr(x) _mbscvt((uint8_t*)(x),FALSE)


#define _mbsstr(s1,s2) strstr((const char*)(s1),(const char *)(s2))

#define _mbschr(s1,c) strchr((const char*)(s1),(c))
#define _mbsrchr(s1,c) strrchr((const char*)(s1),(c))
#define _mbsicmp(s1,s2) strcasecmp((const char*)(s1),(const char *)(s2))
#define _mbscmp(s1,s2) strcmp((const char*)(s1),(const char *)(s2))
#define _ismbcspace isspace
#define _vscwprintf(fmt,args) vswprintf(NULL,0,fmt,args)
#define vswprintf_s vswprintf
#define _wtoi(x) wcstol(x,NULL,10)
#define memmove_s(dst,ndst,src,len) memmove(dst,src,len)
#define swscanf_s swscanf
#define snwprintf swprintf
#define CharLowerBuff(str,len) _mbscvt((uint8_t*)(str),len)

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

inline void wcscpy_s(wchar_t* destination,  size_t num, const wchar_t* source)
{
    wcsncpy(destination, source, num);
    destination[num - 1] = L'\0'; 
}

inline float _wtof(const wchar_t *src){
    wchar_t* endptr;
    double value = wcstod(src, &endptr);
    if (src == endptr) {
        return 0.0;
    } else {
        return value;
    }
}

inline const char *CharNext(const char* src){
    return (const char*)_mbsinc((const uint8_t*)src);
}

#define _wcsicmp(s1,s2) wcscasecmp(s1,s2)
#define _wcsnicmp(s1,s2,num) wcsncasecmp(s1,s2,num)
#define stricmp(s1,s2) strcasecmp(s1,s2)
#define strnicmp(s1,s2,num) strncasecmp(s1,s2,num)

#ifdef _UNICODE
#define _tcsncmp wcsncmp
#define _tcslen wcslen
#define _ttoi  wcstol
#define _tcsdup wcsdup
#define _tcsicmp _wcsicmp
#define _tcsnicmp _wcsnicmp
#define _tcscpy wcscpy
#define _tcstok wcstok
#define _tcsncpy wcsncpy
#define _tcsrchr wcschr
#else
#define _tcsncmp strncmp
#define _tcslen strlen
#define _ttoi atoi
#define _tcsdup strdup
#define _tcsicmp stricmp
#define _tcsnicmp strnicmp
#define _tcscpy strcpy
#define _tcstok strtok
#define _tcsncpy strncpy
#define _tcsrchr strchr
#endif

#define CP_ACP 0
#define CP_UTF8 1
#define CP_UTF7 2
#define CP_SYMBOL 3

#define ERROR_INSUFFICIENT_BUFFER 100

#endif
