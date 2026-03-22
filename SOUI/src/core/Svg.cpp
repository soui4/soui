#include <souistd.h>
#include "core/Svg.h"
#define NANOSVG_IMPLEMENTATION
#define NANOSVG_ALL_COLOR_KEYWORDS
#include <src/nanosvg.h>

SNSBEGIN
SvgObj::~SvgObj()
{
    Free();
}

BOOL SvgObj::Load(THIS_ char *strSvg)
{
    Free();
    m_ptr = (void *)nsvgParse(strSvg, "px", 96.0f);
    return m_ptr != NULL;
}

void *SvgObj::GetPtr(CTHIS) SCONST
{
    return m_ptr;
}

void SvgObj::Free(THIS)
{
    if (m_ptr)
    {
        nsvgDelete((NSVGimage *)m_ptr);
        m_ptr = NULL;
    }
}

int SvgObj::GetWidth() SCONST
{
    NSVGimage *nsvg = (NSVGimage *)m_ptr;
    return nsvg ? (int)nsvg->width : 0;
}
int SvgObj::GetHeight() SCONST
{
    NSVGimage *nsvg = (NSVGimage *)m_ptr;
    return nsvg ? (int)nsvg->height : 0;
}

//------------------------------------------------------------
EXTERN_C ISvgObjPtr CreateSvgObj(char *strSvg)
{
    ISvgObj *ret = new SvgObj;
    if (!ret->Load(strSvg))
    {
        ret->Release();
        return NULL;
    }
    return ret;
}

EXTERN_C ISvgObjPtr CreateSvgFromResId(LPCTSTR resId)
{
    SStringTList strLst = ParseResID(resId);
    if (strLst.GetCount() != 2)
        return NULL;
    SApplication *pApp = SApplication::getSingletonPtr();
    int sz = pApp->GetRawBufferSize(strLst[0], strLst[1]);
    if (sz == 0)
        return NULL;
    char *pBuf = (char *)malloc(sz + 1);
    pApp->GetRawBuffer(strLst[0], strLst[1], pBuf, sz);
    pBuf[sz] = 0; // add null terminator
    ISvgObj *pSvg = CreateSvgObj(pBuf);
    free(pBuf);
    return pSvg;
}

SNSEND
