#include "platform.h"
#include <sysapi.h>
#include <pthread.h>

void SetLastError(int e)
{
    errno = e;
}

int GetLastError()
{
    return errno;
}

int MulDiv(int a, int b, int c)
{
    int64_t t = a * b;
    return (int)(t / c);
}

uint32_t GetCurrentThreadId()
{
    return (uint32_t)pthread_self();
}

int MultiByteToWideChar(int cp, int flags, const char *src, int len, wchar_t *dst, int dstLen)
{
    if (cp != CP_ACP && cp != CP_UTF8)
        return 0;
    if (len < 0)
        len = strlen(src);
    const char *stop = src + len;
    const char *ptr = src;
    size_t i = 0;

    while (ptr < stop)
    {
        int result = 0;
        if (dst)
        {
            if (i >= dstLen)
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                break;
            }
            result = mbtowc(&dst[i], ptr, MB_CUR_MAX);
        }
        else
        {
            wchar_t tmp;
            result = mbtowc(&tmp, ptr, MB_CUR_MAX);
        }
        if (result > 0)
        {
            ptr += result;
            i++;
        }
        else
        {
            break; // 遇到空字符，结束转换
        }
    }

    return i;
}

int WideCharToMultiByte(int cp, int flags, const wchar_t *src, int len, char *dst, int dstLen, BOOL *p1, BOOL *p2)
{
    const wchar_t *ptr = src;
    if (len < 0)
        len = wcslen(src);
    const wchar_t *stop = src + len;
    size_t i = 0;
    if (cp != CP_ACP && cp != CP_UTF8)
        return 0;
    while (ptr < stop)
    {
        char tmp[4];
        int result = wctomb(tmp, *ptr);
        if (result <= 0)
            break;
        if (dst)
        {
            if (i + result > dstLen)
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                break;
            }
            memcpy(dst + i, tmp, result);
        }
        ptr++;
        i += result;
    }

    return i;
}

#define STIF_DEFAULT     0x00000000L
#define STIF_SUPPORT_HEX 0x00000001L

BOOL StrToInt64ExW(const wchar_t *str, DWORD flags, LONGLONG *ret)
{
    BOOL negative = FALSE;
    LONGLONG value = 0;

    if (!str || !ret)
        return FALSE;

    /* Skip leading space, '+', '-' */
    while (*str == ' ' || *str == '\t' || *str == '\n')
        str++;

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
    if (res)
        *ret = value;
    return res;
}

void GetLocalTime(SYSTEMTIME *pSysTime)
{
    struct tm *now = localtime(nullptr);
    pSysTime->wYear = now->tm_year;
    pSysTime->wMonth = now->tm_mon;
    pSysTime->wDayOfWeek = now->tm_wday;
    pSysTime->wDay = now->tm_mday;
    pSysTime->wHour = now->tm_hour;
    pSysTime->wMinute = now->tm_min;
    pSysTime->wSecond = now->tm_sec;
    pSysTime->wMilliseconds = 0; // todo:hjx
}

void GetSystemTime(SYSTEMTIME *pSysTime)
{
    struct tm *now = localtime(nullptr);
    pSysTime->wYear = now->tm_year;
    pSysTime->wMonth = now->tm_mon;
    pSysTime->wDayOfWeek = now->tm_wday;
    pSysTime->wDay = now->tm_mday;
    pSysTime->wHour = now->tm_hour;
    pSysTime->wMinute = now->tm_min;
    pSysTime->wSecond = now->tm_sec;
    pSysTime->wMilliseconds = 0; // todo:hjx
}

time_t _mkgmtime(tm *_Tm)
{
    return 0;
}

int _localtime64_s(tm *ptm, const __time64_t *ptime)
{
    return 0;
}

static inline BOOL is_valid_hex(WCHAR c)
{
    if (!(((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'))))
        return FALSE;
    return TRUE;
}

static const BYTE guid_conv_table[256] = {
    0, 0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x00 */
    0, 0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x10 */
    0, 0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x20 */
    0, 1,   2,   3,   4,   5,   6,   7, 8, 9, 0, 0, 0, 0, 0, 0, /* 0x30 */
    0, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x40 */
    0, 0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x50 */
    0, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf                             /* 0x60 */
};

BOOL IIDFromString(LPCWSTR s, GUID *id)
{
    int i;

    if (!s || s[0] != '{')
    {
        memset(id, 0, sizeof(*id));
        if (!s)
            return TRUE;
        return FALSE;
    }

    /* In form {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX} */

    id->Data1 = 0;
    for (i = 1; i < 9; ++i)
    {
        if (!is_valid_hex(s[i]))
            return FALSE;
        id->Data1 = (id->Data1 << 4) | guid_conv_table[s[i]];
    }
    if (s[9] != '-')
        return FALSE;

    id->Data2 = 0;
    for (i = 10; i < 14; ++i)
    {
        if (!is_valid_hex(s[i]))
            return FALSE;
        id->Data2 = (id->Data2 << 4) | guid_conv_table[s[i]];
    }
    if (s[14] != '-')
        return FALSE;

    id->Data3 = 0;
    for (i = 15; i < 19; ++i)
    {
        if (!is_valid_hex(s[i]))
            return FALSE;
        id->Data3 = (id->Data3 << 4) | guid_conv_table[s[i]];
    }
    if (s[19] != '-')
        return FALSE;

    for (i = 20; i < 37; i += 2)
    {
        if (i == 24)
        {
            if (s[i] != '-')
                return FALSE;
            i++;
        }
        if (!is_valid_hex(s[i]) || !is_valid_hex(s[i + 1]))
            return FALSE;
        id->Data4[(i - 20) / 2] = guid_conv_table[s[i]] << 4 | guid_conv_table[s[i + 1]];
    }

    if (s[37] == '}' && s[38] == '\0')
        return TRUE;

    return FALSE;
}

#define __COMPARE(context, p1, p2)                comp(context, p1, p2)
#define __SHORTSORT(lo, hi, width, comp, context) shortsort_s(lo, hi, width, comp, context);

#ifndef _ASSERT_EXPR
#define _ASSERT_EXPR(expr, expr_str) ((void)0)
#endif
#ifndef _CRT_WIDE
#define __CRT_WIDE(_String) L##_String
#define _CRT_WIDE(_String)  __CRT_WIDE(_String)
#endif

#define _INVALID_PARAMETER(expr) _CALL_INVALID_PARAMETER_FUNC(_invalid_parameter, expr)

#define _VALIDATE_RETURN_VOID(expr, errorcode)       \
    {                                                \
        int _Expr_val = !!(expr);                    \
        _ASSERT_EXPR((_Expr_val), _CRT_WIDE(#expr)); \
        if (!(_Expr_val))                            \
        {                                            \
            errno = errorcode;                       \
            return;                                  \
        }                                            \
    }

#define STKSIZ (8 * sizeof(void *) - 2)
#define CUTOFF 8 /* testing shows that this is good value */

static void shortsort_s(char *lo, char *hi, size_t width, int(__cdecl *comp)(void *, const void *, const void *), void *);
#define swap swap_c

static void swap(char *p, char *q, size_t width);

void OffsetRect(RECT *prc, int xOffset, int yOffset)
{
    prc->left += xOffset;
    prc->right+= xOffset;
    prc->top += yOffset;
    prc->bottom += yOffset;
}

BOOL IsRectEmpty(const RECT *prc)
{
    return prc->left == prc->right && prc->top == prc->bottom;
}

void qsort_s(void *base, size_t num, size_t width, int(__cdecl *comp)(void *, const void *, const void *), void *context)
{
    char *lo, *hi;       /* ends of sub-array currently sorting */
    char *mid;           /* points to middle of subarray */
    char *loguy, *higuy; /* traveling pointers for partition step */
    size_t size;         /* size of the sub-array */
    char *lostk[STKSIZ], *histk[STKSIZ];
    int stkptr; /* stack for saving sub-array to be processed */

    /* validation section */
    _VALIDATE_RETURN_VOID(base != NULL || num == 0, EINVAL);
    _VALIDATE_RETURN_VOID(width > 0, EINVAL);
    _VALIDATE_RETURN_VOID(comp != NULL, EINVAL);

    if (num < 2)
        return; /* nothing to do */

    stkptr = 0; /* initialize stack */

    lo = (char *)base;
    hi = (char *)base + width * (num - 1); /* initialize limits */

    /* this entry point is for pseudo-recursion calling: setting
       lo and hi and jumping to here is like recursion, but stkptr is
       preserved, locals aren't, so we preserve stuff on the stack */
recurse:

    size = (hi - lo) / width + 1; /* number of el's to sort */

    /* below a certain size, it is faster to use a O(n^2) sorting method */
    if (size <= CUTOFF)
    {
        __SHORTSORT(lo, hi, width, comp, context);
    }
    else
    {
        /* First we pick a partitioning element.  The efficiency of the
           algorithm demands that we find one that is approximately the median
           of the values, but also that we select one fast.  We choose the
           median of the first, middle, and last elements, to avoid bad
           performance in the face of already sorted data, or data that is made
           up of multiple sorted runs appended together.  Testing shows that a
           median-of-three algorithm provides better performance than simply
           picking the middle element for the latter case. */

        mid = lo + (size / 2) * width; /* find middle element */

        /* Sort the first, middle, last elements into order */
        if (__COMPARE(context, lo, mid) > 0)
        {
            swap(lo, mid, width);
        }
        if (__COMPARE(context, lo, hi) > 0)
        {
            swap(lo, hi, width);
        }
        if (__COMPARE(context, mid, hi) > 0)
        {
            swap(mid, hi, width);
        }

        /* We now wish to partition the array into three pieces, one consisting
           of elements <= partition element, one of elements equal to the
           partition element, and one of elements > than it.  This is done
           below; comments indicate conditions established at every step. */

        loguy = lo;
        higuy = hi;

        /* Note that higuy decreases and loguy increases on every iteration,
           so loop must terminate. */
        for (;;)
        {
            /* lo <= loguy < hi, lo < higuy <= hi,
               A[i] <= A[mid] for lo <= i <= loguy,
               A[i] > A[mid] for higuy <= i < hi,
               A[hi] >= A[mid] */

            /* The doubled loop is to avoid calling comp(mid,mid), since some
               existing comparison funcs don't work when passed the same
               value for both pointers. */

            if (mid > loguy)
            {
                do
                {
                    loguy += width;
                } while (loguy < mid && __COMPARE(context, loguy, mid) <= 0);
            }
            if (mid <= loguy)
            {
                do
                {
                    loguy += width;
                } while (loguy <= hi && __COMPARE(context, loguy, mid) <= 0);
            }

            /* lo < loguy <= hi+1, A[i] <= A[mid] for lo <= i < loguy,
               either loguy > hi or A[loguy] > A[mid] */

            do
            {
                higuy -= width;
            } while (higuy > mid && __COMPARE(context, higuy, mid) > 0);

            /* lo <= higuy < hi, A[i] > A[mid] for higuy < i < hi,
               either higuy == lo or A[higuy] <= A[mid] */

            if (higuy < loguy)
                break;

            /* if loguy > hi or higuy == lo, then we would have exited, so
               A[loguy] > A[mid], A[higuy] <= A[mid],
               loguy <= hi, higuy > lo */

            swap(loguy, higuy, width);

            /* If the partition element was moved, follow it.  Only need
               to check for mid == higuy, since before the swap,
               A[loguy] > A[mid] implies loguy != mid. */

            if (mid == higuy)
                mid = loguy;

            /* A[loguy] <= A[mid], A[higuy] > A[mid]; so condition at top
               of loop is re-established */
        }

        /*     A[i] <= A[mid] for lo <= i < loguy,
               A[i] > A[mid] for higuy < i < hi,
               A[hi] >= A[mid]
               higuy < loguy
           implying:
               higuy == loguy-1
               or higuy == hi - 1, loguy == hi + 1, A[hi] == A[mid] */

        /* Find adjacent elements equal to the partition element.  The
           doubled loop is to avoid calling comp(mid,mid), since some
           existing comparison funcs don't work when passed the same value
           for both pointers. */

        higuy += width;
        if (mid < higuy)
        {
            do
            {
                higuy -= width;
            } while (higuy > mid && __COMPARE(context, higuy, mid) == 0);
        }
        if (mid >= higuy)
        {
            do
            {
                higuy -= width;
            } while (higuy > lo && __COMPARE(context, higuy, mid) == 0);
        }

        /* OK, now we have the following:
              higuy < loguy
              lo <= higuy <= hi
              A[i]  <= A[mid] for lo <= i <= higuy
              A[i]  == A[mid] for higuy < i < loguy
              A[i]  >  A[mid] for loguy <= i < hi
              A[hi] >= A[mid] */

        /* We've finished the partition, now we want to sort the subarrays
           [lo, higuy] and [loguy, hi].
           We do the smaller one first to minimize stack usage.
           We only sort arrays of length 2 or more.*/

        if (higuy - lo >= hi - loguy)
        {
            if (lo < higuy)
            {
                lostk[stkptr] = lo;
                histk[stkptr] = higuy;
                ++stkptr;
            } /* save big recursion for later */

            if (loguy < hi)
            {
                lo = loguy;
                goto recurse; /* do small recursion */
            }
        }
        else
        {
            if (loguy < hi)
            {
                lostk[stkptr] = loguy;
                histk[stkptr] = hi;
                ++stkptr; /* save big recursion for later */
            }

            if (lo < higuy)
            {
                hi = higuy;
                goto recurse; /* do small recursion */
            }
        }
    }

    /* We have sorted the array, except for any pending sorts on the stack.
       Check if there are any, and do them. */

    --stkptr;
    if (stkptr >= 0)
    {
        lo = lostk[stkptr];
        hi = histk[stkptr];
        goto recurse; /* pop subarray from stack */
    }
    else
        return; /* all subarrays done */
}

/***
 *shortsort(hi, lo, width, comp) - insertion sort for sorting short arrays
 *shortsort_s(hi, lo, width, comp, context) - insertion sort for sorting short arrays
 *
 *Purpose:
 *       sorts the sub-array of elements between lo and hi (inclusive)
 *       side effects:  sorts in place
 *       assumes that lo < hi
 *
 *Entry:
 *       char *lo = pointer to low element to sort
 *       char *hi = pointer to high element to sort
 *       size_t width = width in bytes of each array element
 *       int (*comp)() = pointer to function returning analog of strcmp for
 *               strings, but supplied by user for comparing the array elements.
 *               it accepts 2 pointers to elements, together with a pointer to a context
 *               (if present). Returns neg if 1<2, 0 if 1=2, pos if 1>2.
 *       void *context - pointer to the context in which the function is
 *               called. This context is passed to the comparison function.
 *
 *Exit:
 *       returns void
 *
 *Exceptions:
 *
 *******************************************************************************/

static void shortsort_s(char *lo, char *hi, size_t width, int(__cdecl *comp)(void *, const void *, const void *), void *context)
{
    char *p, *max;

    /* Note: in assertions below, i and j are alway inside original bound of
       array to sort. */

    while (hi > lo)
    {
        /* A[i] <= A[j] for i <= j, j > hi */
        max = lo;
        for (p = lo + width; p <= hi; p += width)
        {
            /* A[i] <= A[max] for lo <= i < p */
            if (__COMPARE(context, p, max) > 0)
            {
                max = p;
            }
            /* A[i] <= A[max] for lo <= i <= p */
        }

        /* A[i] <= A[max] for lo <= i <= hi */

        swap(max, hi, width);

        /* A[i] <= A[hi] for i <= hi, so A[i] <= A[j] for i <= j, j >= hi */

        hi -= width;

        /* A[i] <= A[j] for i <= j, j > hi, loop top condition established */
    }
    /* A[i] <= A[j] for i <= j, j > lo, which implies A[i] <= A[j] for i < j,
       so array is sorted */
}

/***
 *swap(a, b, width) - swap two elements
 *
 *Purpose:
 *       swaps the two array elements of size width
 *
 *Entry:
 *       char *a, *b = pointer to two elements to swap
 *       size_t width = width in bytes of each array element
 *
 *Exit:
 *       returns void
 *
 *Exceptions:
 *
 *******************************************************************************/

static void swap(char *a, char *b, size_t width)
{
    char tmp;

    if (a != b)
        /* Do the swap one character at a time to avoid potential alignment
           problems. */
        while (width--)
        {
            tmp = *a;
            *a++ = *b;
            *b++ = tmp;
        }
}

#undef __COMPARE
#undef __SHORTSORT
#undef swap

void DestroyIcon(HICON hIcon)
{
}

void GetWindowRect(HWND hWnd, RECT *rc)
{
}

int GetWindowScale(HWND hWnd)
{
    return 100;
}

void PostThreadMessage(int tid, UINT msg, WPARAM wp, LPARAM lp)
{
}

BOOL WaitMessage()
{
    return FALSE;
}

BOOL IsWindow(HWND hWnd)
{
    return FALSE;
}
void DestroyWindow(HWND hwnd)
{
}

HDC GetDC(HWND hWnd)
{
    return 0;
}
void ReleaseDC(HWND hwnd, HDC hdc)
{
}

int GetSystemMetrics(int nIndex)
{
    return 0;
}

void SetCursor(HCURSOR hCursor)
{
}

BOOL ShellExecute(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd)
{
    return FALSE;
}

BOOL ClientToScreen(HWND hWnd, LPPOINT ppt)
{
    return 0;
}
BOOL ScreenToClient(HWND hWnd, LPPOINT ppt)
{
    return 0;
}

BOOL GetClipBox(HDC hdc, RECT *pRc)
{
    return 0;
}

HMONITOR
MonitorFromWindow(HWND hwnd, DWORD dwFlags)
{
    return 0;
}

BOOL GetMonitorInfo(HMONITOR hMonitor, LPMONITORINFO lpmi)
{
    return FALSE;
}

void DeleteObject(HGDIOBJ hObj)
{
}

SHORT GetKeyState(int nVirtKey)
{
    return 0;
}

HHOOK SetWindowsHookEx(int idHook, HOOKPROC lpfn, HINSTANCE hmod, DWORD dwThreadId)
{
    return 0;
}

BOOL UnhookWindowsHookEx(HHOOK hhk)
{
    return FALSE;
}

LRESULT
CallNextHookEx(HHOOK hhk, int nCode, WPARAM wParam, LPARAM lParam)
{
    return 0;
}

BOOL PostMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}

LRESULT SendMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return 0;
}

int MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
    return 0;
}

BOOL SetForegroundWindow(HWND hWnd)
{
    return 0;
}

HWND GetForegroundWindow()
{
    return 0;
}

BOOL ShowWindow(HWND hWnd, int nCmdShow)
{
    return 0;
}

BOOL SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
    return 0;
}

LONG SetWindowLong(HWND hWnd, int nIndex,LONG data){
    return 0;
}
LONG GetWindowLong(HWND hWnd, int nIndex)
{
    return 0;
}

UINT MapVirtualKey(UINT uCode, UINT uMapType)
{
    return 0;
}

BOOL CreateCaret(HWND hWnd, HBITMAP hBitmap, int nWidth, int nHeight)
{
    return 0;
}

BOOL DestroyCaret(VOID)
{
    return FALSE;
}
BOOL HideCaret(HWND hWnd)
{
    return 0;
}

BOOL ShowCaret(HWND hWnd)
{
    return 0;
}

BOOL SetCaretPos(int X, int Y)
{
    return 0;
}

BOOL GetCaretPos(LPPOINT lpPoint)
{
    return 0;
}

HWND GetActiveWindow()
{
    return 0;
}

HWND GetDesktopWindow()
{
    return 0;
}

BOOL IsWindowEnabled(HWND hWnd)
{
    return 0;
}

void EnableWindow(HWND hWnd, BOOL bEnable)
{
}

HWND SetActiveWindow(HWND hWnd)
{
    return 0;
}

HWND GetParent(HWND hwnd)
{
    return 0;
}

HWND SetParent(HWND hWnd, HWND hParent){
    return 0;
}

BOOL GetCursorPos(LPPOINT ppt)
{
    return 0;
}

HWND WindowFromPoint(POINT pt)
{
    return 0;
}

DWORD GetTickCount()
{
    return 0;
}

UINT_PTR SetTimer(HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc)
{
    return 0;
}

BOOL KillTimer(HWND hWnd, UINT_PTR uIDEvent)
{
    return 0;
}

HWND GetFocus()
{
    return 0;
}

HWND SetFocus(HWND hWnd)
{
    return 0;
}

HDC BeginPaint(HWND hWnd, PAINTSTRUCT *ps){
    return 0;
}
void EndPaint(HWND hWnd, PAINTSTRUCT *ps){}

BOOL UpdateWindow(HWND hWnd)
{
    return 0;
}

BOOL GetClientRect(HWND hWnd, RECT *pRc)
{
    return 0;
}

BOOL RegisterDragDrop(HWND, IDropTarget *pDrapTarget){
    return 0;
}

BOOL GetModuleFileName(HMODULE hModule, LPTSTR pszPath, int length)
{
    return 0;
}

BOOL CallMsgFilter(LPMSG lpMsg, int nCode)
{
    return 0;
}

__time64_t _mktime64(const tm *ptime)
{
    return 0;
}

__time64_t _time64(__time64_t *_Time)
{
    return 0;
}

HCURSOR LoadCursor(HINSTANCE hInstance, LPCSTR lpCursorName)
{
    return 0;
}

BOOL DestroyCursor(HCURSOR hCursor)
{
    return 0;
}


BOOL IsFilePathValid(const char* path) {
    return access(path, F_OK) == 0;
}