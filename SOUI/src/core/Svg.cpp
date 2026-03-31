#include <souistd.h>
#include "core/Svg.h"

#ifdef SOUI_ENABLE_SVG
#define NANOSVG_IMPLEMENTATION
#define NANOSVG_ALL_COLOR_KEYWORDS
#include <src/nanosvg.h>
#include <helper/obj-ref-impl.hpp>
SNSBEGIN

/**
 * @class SvgObj
 * @brief svg 对象
 */
class SvgObj : public TObjRefImpl<ISvgObj> {
    void *m_ptr;

  public:
    SvgObj()
        : m_ptr(NULL)
    {
    }
    virtual ~SvgObj();

  public:
    STDMETHOD_(BOOL, Load)(THIS_ char *strSvg) OVERRIDE;
    STDMETHOD_(void, Free)(THIS) OVERRIDE;
    STDMETHOD_(void *, GetPtr)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(int, GetWidth)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(int, GetHeight)(CTHIS) SCONST OVERRIDE;
};

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
    if (strLst.GetCount() == 1)
    {
        strLst.InsertAt(0, _T("file"));
    }
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

static ISvgObjPtr CreateSvgFromFILE(FILE *f)
{
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *pBuf = (char *)malloc(sz + 1);
    fread(pBuf, 1, sz, f);
    pBuf[sz] = 0; // add null terminator
    ISvgObj *pSvg = CreateSvgObj(pBuf);
    free(pBuf);
    return pSvg;
}

EXTERN_C ISvgObjPtr CreateSvgFromFileA(LPCSTR pszSvgFile)
{
    FILE *fp = fopen(pszSvgFile, "rb");
    if (!fp)
        return NULL;
    ISvgObjPtr pSvg = CreateSvgFromFILE(fp);
    fclose(fp);
    return pSvg;
}

EXTERN_C ISvgObjPtr CreateSvgFromFileW(LPCWSTR pszSvgFile)
{
    FILE *fp = _wfopen(pszSvgFile, L"rb");
    if (!fp)
        return NULL;
    ISvgObjPtr pSvg = CreateSvgFromFILE(fp);
    fclose(fp);
    return pSvg;
}

SNSEND

#else // !SOUI_ENABLE_SVG

// When SVG is disabled, provide stub implementations that return NULL
SNSBEGIN

EXTERN_C ISvgObjPtr CreateSvgObj(char *strSvg)
{
    (void)strSvg; // unused parameter
    return NULL;
}

EXTERN_C ISvgObjPtr CreateSvgFromResId(LPCTSTR resId)
{
    (void)resId; // unused parameter
    return NULL;
}

EXTERN_C ISvgObjPtr CreateSvgFromFileA(LPCSTR pszSvgFile)
{
    (void)pszSvgFile; // unused parameter
    return NULL;
}

EXTERN_C ISvgObjPtr CreateSvgFromFileW(LPCWSTR pszSvgFile)
{
    (void)pszSvgFile; // unused parameter
    return NULL;
}

SNSEND

#endif // SOUI_ENABLE_SVG
