#ifndef _CLASS_H_
#define _CLASS_H_

#define MAX_ATOM_LEN 255

typedef struct tagCLASS
{
    UINT         style;         /* Class style */
    WNDPROC      winproc;       /* Window procedure */
    INT          cbClsExtra;    /* Class extra bytes */
    INT          cbWndExtra;    /* Window extra bytes */
    UINT_PTR     instance;      /* Module that created the task */
    HICON        hIcon;         /* Default icon */
    HICON        hIconSm;       /* Default small icon */
    HICON        hIconSmIntern; /* Internal small icon, derived from hIcon */
    HCURSOR      hCursor;       /* Default cursor */
    HBRUSH       hbrBackground; /* Default background */
    ATOM         atomName;      /* Name of the class */
    char         name[MAX_ATOM_LEN + 1];
    char       * basename;      /* Base name for redirected classes, pointer within 'name'. */
} CLASS;

typedef struct tagWNDCLASSEX
{
    UINT      cbSize;
    UINT      style;
    WNDPROC   lpfnWndProc;
    INT       cbClsExtra;
    INT       cbWndExtra;
    HINSTANCE hInstance;
    HICON     hIcon;
    HCURSOR   hCursor;
    HBRUSH    hbrBackground;
    LPCSTR      lpszMenuName;
    LPCSTR      lpszClassName;
    HICON     hIconSm;
} WNDCLASSEX, *PWNDCLASSEX, *LPWNDCLASSEX;


ATOM WINAPI RegisterClassEx( const WNDCLASSEX* wc );
BOOL WINAPI UnregisterClass( LPCSTR className, HINSTANCE instance );

BOOL WINAPI GetClassInfoEx( HINSTANCE hInstance, LPCSTR name, WNDCLASSEX *wc );

BOOL WINAPI GetAtomName(ATOM atomName, LPSTR name, int cchLen);

#endif//_CLASS_H_