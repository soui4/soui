#include "platform.h"
#include <sysapi.h>
#include <pthread.h>

void SetLastError(int e){
    errno = e;
}

int GetLastError() {
    return errno;
}

int MulDiv(int a,int b,int c){
    int64_t t = a*b;
    return (int)(t/c);
}

uint32_t GetCurrentThreadId(){
   return (uint32_t)pthread_self();
}

int MultiByteToWideChar(int cp,int flags, const char *src, int len,wchar_t *dst,int dstLen){
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

int WideCharToMultiByte(int cp,int flags,const wchar_t* src,int len,char * dst,int dstLen,BOOL *p1,BOOL *p2){
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

BOOL StrToInt64ExW(const wchar_t *str, DWORD flags, LONGLONG *ret)
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

BOOL StrToIntExW(const wchar_t *str, DWORD flags, INT *ret)
{
    LONGLONG value;
    BOOL res;
    res = StrToInt64ExW(str, flags, &value);
    if (res) *ret = value;
    return res;
}

static inline BOOL is_valid_hex(WCHAR c)
{
    if (!(((c >= '0') && (c <= '9'))  ||
          ((c >= 'a') && (c <= 'f'))  ||
          ((c >= 'A') && (c <= 'F'))))
        return FALSE;
    return TRUE;
}

static const BYTE guid_conv_table[256] =
{
    0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x00 */
    0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x10 */
    0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x20 */
    0,   1,   2,   3,   4,   5,   6, 7, 8, 9, 0, 0, 0, 0, 0, 0, /* 0x30 */
    0, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x40 */
    0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x50 */
    0, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf                             /* 0x60 */
};

BOOL IIDFromString(LPCWSTR s, GUID *id)
{
    int i;

    if (!s || s[0] != '{')
    {
        memset(id, 0, sizeof(*id));
        if (!s) return TRUE;
        return FALSE;
    }

    /* In form {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX} */

    id->Data1 = 0;
    for (i = 1; i < 9; ++i)
    {
        if (!is_valid_hex(s[i])) return FALSE;
        id->Data1 = (id->Data1 << 4) | guid_conv_table[s[i]];
    }
    if (s[9] != '-') return FALSE;

    id->Data2 = 0;
    for (i = 10; i < 14; ++i)
    {
        if (!is_valid_hex(s[i])) return FALSE;
        id->Data2 = (id->Data2 << 4) | guid_conv_table[s[i]];
    }
    if (s[14] != '-') return FALSE;

    id->Data3 = 0;
    for (i = 15; i < 19; ++i)
    {
        if (!is_valid_hex(s[i])) return FALSE;
        id->Data3 = (id->Data3 << 4) | guid_conv_table[s[i]];
    }
    if (s[19] != '-') return FALSE;

    for (i = 20; i < 37; i += 2)
    {
        if (i == 24)
        {
            if (s[i] != '-') return FALSE;
            i++;
        }
        if (!is_valid_hex(s[i]) || !is_valid_hex(s[i + 1])) return FALSE;
        id->Data4[(i - 20) / 2] = guid_conv_table[s[i]] << 4 | guid_conv_table[s[i + 1]];
    }

    if (s[37] == '}' && s[38] == '\0')
        return TRUE;

    return FALSE;
}


void DestroyIcon(HICON hIcon){

}

void GetWindowRect(HWND hWnd,RECT *rc){
    
}

int  GetWindowScale(HWND hWnd){
    return 100;
}

void PostThreadMessage(int tid, UINT msg,WPARAM wp,LPARAM lp){

}

BOOL WaitMessage(){
    return FALSE;
}