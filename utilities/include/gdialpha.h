#ifndef __GDIALPHA__H__
#define __GDIALPHA__H__
#ifdef _WIN32

#include "utilities-def.h"
#include <windows.h>

SNSBEGIN

#define  MAX_ALPHABUF    1<<16

typedef struct tagALPHAINFO
{
    BITMAP bm;
    LPBYTE lpBuf;
    RECT    rc;
    tagALPHAINFO()
    {
        lpBuf=NULL;
        rc.left=rc.top=rc.right=rc.bottom=0;
    }
} ALPHAINFO,* LPALPHAINFO;

class UTILITIES_API CGdiAlpha
{
private:
    static BYTE s_byAlphaBack[MAX_ALPHABUF];

    static LPBYTE ALPHABACKUP(BITMAP *pBitmap,int x,int y,int cx,int cy);
    //恢复位图的Alpha通道
    static void ALPHARESTORE(BITMAP *pBitmap,int x,int y,int cx,int cy,LPBYTE lpAlpha);
public:

    static BOOL AlphaBackup(HDC hdc,LPCRECT pRect,ALPHAINFO &alphaInfo);

    static void AlphaRestore(ALPHAINFO &alphaInfo);
};

SNSEND

#endif //_WIN32
#endif // __GDIALPHA__H__