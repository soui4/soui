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
#define _wtoi(x) wcstol(x,NULL,10)
#define memmove_s(dst,ndst,src,len) memmove(dst,src,len)
#define swscanf_s swscanf
#define snwprintf swprintf

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

#define CP_ACP 0
#define CP_UTF8 1
#define CP_UTF7 2
#define CP_SYMBOL 3

#define ERROR_INSUFFICIENT_BUFFER 100
inline void SetLastError(int e){
    errno = e;
}

inline int GetLastError() {
    return errno;
}

inline int MultiByteToWideChar(int cp,int flags, const char *src, int len,wchar_t *dst,int dstLen){
    if(cp != CP_ACP &&cp != CP_UTF8)
        return 0;
    if(len<0) len = strlen(src);
    const char * stop = src+len;
    const char* ptr = src;
    size_t i = 0;

    while (ptr <stop) {
        int result=0;
        if(dst){
            if(i>=dstLen){
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                break;
            }
            result = mbtowc(&dst[i], ptr, MB_CUR_MAX);
        }else{
            wchar_t tmp;
            result = mbtowc(&tmp, ptr, MB_CUR_MAX);
        }
        if (result > 0) {
            ptr += result;
            i++;
        } else {
            break;  // 遇到空字符，结束转换
        }
    }

    return i;
}

inline int WideCharToMultiByte(int cp,int flags,const wchar_t* src,int len,char * dst,int dstLen,BOOL *p1,BOOL *p2){
    const wchar_t* ptr = src;
    if(len<0) len = wcslen(src);
    const wchar_t * stop = src+len;
    size_t i = 0;
    if(cp!=CP_ACP && cp != CP_UTF8)
        return 0;
    while (ptr <stop) {
        char tmp[4];
        int result = wctomb(tmp, *ptr);
        if(result<=0)
            break;
        if(dst){
            if(i+result>dstLen){
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                break;
            }
            memcpy(dst+i,tmp,result);
        }
        ptr++;
        i += result;
    }

    return i;
}

#define STIF_DEFAULT        0x00000000L
#define STIF_SUPPORT_HEX    0x00000001L

inline BOOL StrToInt64ExW(const wchar_t *str, DWORD flags, LONGLONG *ret)
{
    BOOL negative = FALSE;
    LONGLONG value = 0;

    if (!str || !ret)
        return FALSE;

    /* Skip leading space, '+', '-' */
    while (*str == ' ' || *str == '\t' || *str == '\n') str++;

    if (*str == '-')
    {
        negative = TRUE;
        str++;
    }
    else if (*str == '+')
        str++;

    if (flags & STIF_SUPPORT_HEX && *str == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        /* Read hex number */
        str += 2;

        if (!isxdigit(*str))
            return FALSE;

        while (isxdigit(*str))
        {
            value *= 16;
            if (*str >= '0' && *str <= '9')
                value += (*str - '0');
            else if (*str >= 'A' && *str <= 'Z')
                value += 10 + (*str - 'A');
            else
                value += 10 + (*str - 'a');
            str++;
        }

        *ret = value;
        return TRUE;
    }

    /* Read decimal number */
    if (*str < '0' || *str > '9')
        return FALSE;

    while (*str >= '0' && *str <= '9')
    {
        value *= 10;
        value += (*str - '0');
        str++;
    }

    *ret = negative ? -value : value;
    return TRUE;
}

inline BOOL StrToIntExW(const wchar_t *str, DWORD flags, INT *ret)
{
    LONGLONG value;
    BOOL res;
    res = StrToInt64ExW(str, flags, &value);
    if (res) *ret = value;
    return res;
}

inline int MulDiv(int a,int b,int c){
    int64_t t = a*b;
    return (int)(t/c);
}

inline uint32_t GetCurrentThreadId(){
   return (uint32_t)pthread_self();
}
#endif
