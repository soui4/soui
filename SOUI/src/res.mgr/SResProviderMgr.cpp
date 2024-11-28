#include "souistd.h"
#include "res.mgr/SResProviderMgr.h"
#include "res.mgr/SResProvider.h"
#include "helper/SplitString.h"
#include <res.mgr/SUiDef.h>

SNSBEGIN

const static TCHAR KTypeFile[] = _T("file"); //从文件加载资源时指定的类型

SResProviderMgr::SResProviderMgr()
{
}

SResProviderMgr::~SResProviderMgr(void)
{
    RemoveAll();
}

void SResProviderMgr::RemoveAll()
{
    SAutoLock lock(m_cs);
    SPOSITION pos = m_lstResPackage.GetHeadPosition();
    while (pos)
    {
        IResProvider *pResProvider = m_lstResPackage.GetNext(pos);
#ifdef _DEBUG //检查资源使用情况
        SSLOGD() << "++++begin of check resource usage";
        pResProvider->EnumResource(&SResProviderMgr::CheckUsage, (LPARAM)&m_mapResUsageCount);
        SSLOGD() << "----end of check resource usage";
#endif
        pResProvider->Release();
    }
    m_lstResPackage.RemoveAll();

    pos = m_mapCachedCursor.GetStartPosition();
    while (pos)
    {
        CURSORMAP::CPair *pPair = m_mapCachedCursor.GetNext(pos);
        DestroyCursor(pPair->m_value);
    }
    m_mapCachedCursor.RemoveAll();
}

IResProvider *SResProviderMgr::GetMatchResProvider(LPCTSTR pszType, LPCTSTR pszResName)
{
    SAutoLock lock(m_cs);
    SPOSITION pos = m_lstResPackage.GetTailPosition();
    while (pos)
    {
        IResProvider *pResProvider = m_lstResPackage.GetPrev(pos);
        if (pResProvider->HasResource(pszType, pszResName))
        {
#ifdef _DEBUG
            m_mapResUsageCount[SStringT().Format(_T("%s:%s"), pszType, pszResName).MakeLower()]++;
#endif
            return pResProvider;
        }
    }
    return NULL;
}

void SResProviderMgr::AddResProvider(IResProvider *pResProvider, LPCTSTR pszUidef)
{
    SAutoLock lock(m_cs);
    m_lstResPackage.AddTail(pResProvider);
    pResProvider->AddRef();
    if (pszUidef)
    {
        GETUIDEF->InitDefUiDef(pResProvider, pszUidef);
    }
}

void SResProviderMgr::AddResProviderA(IResProvider *pResProvider, LPCSTR pszUidef)
{
    if (!pszUidef)
        return AddResProvider(pResProvider, NULL);
    SStringT strUiDef = S_CA2T(pszUidef, CP_UTF8);
    return AddResProvider(pResProvider, strUiDef.c_str());
}

void SResProviderMgr::RemoveResProvider(IResProvider *pResProvider)
{
    SAutoLock lock(m_cs);
    SPOSITION pos = m_lstResPackage.GetTailPosition();
    while (pos)
    {
        SPOSITION posPrev = pos;
        IResProvider *pResProvierT = m_lstResPackage.GetPrev(pos);
        if (pResProvierT == pResProvider)
        {
            m_lstResPackage.RemoveAt(posPrev);
            pResProvierT->Release();
            break;
        }
    }
}

IResProvider *SResProviderMgr::GetHeadResProvider() const
{
    return m_lstResPackage.GetHead();
}

IResProvider *SResProviderMgr::GetTailResProvider() const
{
    return m_lstResPackage.GetTail();
}

LPCTSTR SResProviderMgr::SysCursorName2ID(LPCTSTR pszCursorName)
{
    struct Cursor2ID
    {
        LPCTSTR szName;
        LPCTSTR szID;
    };
    static const Cursor2ID cursorMap[] = {
        { _T("arrow"), IDC_ARROW }, { _T("ibeam"), IDC_IBEAM }, { _T("wait"), IDC_WAIT }, { _T("cross"), IDC_CROSS }, { _T("uparrow"), IDC_UPARROW }, { _T("size"), IDC_SIZE }, { _T("sizenwse"), IDC_SIZENWSE }, { _T("sizenesw"), IDC_SIZENESW }, { _T("sizewe"), IDC_SIZEWE }, { _T("sizens"), IDC_SIZENS }, { _T("sizeall"), IDC_SIZEALL }, { _T("no"), IDC_NO }, { _T("hand"), IDC_HAND }, { _T("help"), IDC_HELP },
    };
    SAutoLock lock(m_cs);
    for (int i = 0; i < ARRAYSIZE(cursorMap); i++)
    {
        if (0 == _tcsicmp(cursorMap[i].szName, pszCursorName))
            return cursorMap[i].szID;
    }
    return NULL;
}

BOOL SResProviderMgr::GetRawBuffer(LPCTSTR strType, LPCTSTR pszResName, LPVOID pBuf, size_t size)
{
    SAutoLock lock(m_cs);
    if (!strType)
    { // support res src by resource path.
        SPOSITION pos = m_lstResPackage.GetHeadPosition();
        while (pos)
        {
            IResProvider *pRes = m_lstResPackage.GetNext(pos);
            if (pRes->HasResource(NULL, pszResName))
                return pRes->GetRawBuffer(strType, pszResName, pBuf, size);
        }
        return FALSE;
    }
    if (IsFileType(strType))
    {
        SStringT strPath = m_strFilePrefix + pszResName;
        return SResLoadFromFile::GetRawBuffer(strPath, pBuf, size);
    }
    else
    {
#ifdef _DEBUG
        m_mapResUsageCount[SStringT().Format(_T("%s:%s"), strType, pszResName).MakeLower()]++;
#endif
        IResProvider *pResProvider = GetMatchResProvider(strType, pszResName);
        if (!pResProvider)
            return FALSE;
        return pResProvider->GetRawBuffer(strType, pszResName, pBuf, size);
    }
}

size_t SResProviderMgr::GetRawBufferSize(LPCTSTR strType, LPCTSTR pszResName)
{
    SAutoLock lock(m_cs);
    if (!strType)
    { // support res src by resource path.
        SPOSITION pos = m_lstResPackage.GetHeadPosition();
        while (pos)
        {
            IResProvider *pRes = m_lstResPackage.GetNext(pos);
            if (pRes->HasResource(NULL, pszResName))
                return pRes->GetRawBufferSize(strType, pszResName);
        }
        return 0;
    }
    if (IsFileType(strType))
    {
        SStringT strPath = m_strFilePrefix + pszResName;
        return SResLoadFromFile::GetRawBufferSize(strPath);
    }
    else
    {
#ifdef _DEBUG
        m_mapResUsageCount[SStringT().Format(_T("%s:%s"), strType, pszResName).MakeLower()]++;
#endif

        IResProvider *pResProvider = GetMatchResProvider(strType, pszResName);
        if (!pResProvider)
            return 0;
        return pResProvider->GetRawBufferSize(strType, pszResName);
    }
}

IImgX *SResProviderMgr::LoadImgX(LPCTSTR strType, LPCTSTR pszResName)
{
    SAutoLock lock(m_cs);
    if (!strType)
    { // support res src by resource path.
        SPOSITION pos = m_lstResPackage.GetHeadPosition();
        while (pos)
        {
            IResProvider *pRes = m_lstResPackage.GetNext(pos);
            IImgX *img = pRes->LoadImgX(NULL, pszResName);
            if (img)
                return img;
        }
        return NULL;
    }

    if (IsFileType(strType))
    {
        SStringT strPath = m_strFilePrefix + pszResName;
        return SResLoadFromFile::LoadImgX(strPath);
    }
    else
    {
#ifdef _DEBUG
        m_mapResUsageCount[SStringT().Format(_T("%s:%s"), strType, pszResName).MakeLower()]++;
#endif

        IResProvider *pResProvider = GetMatchResProvider(strType, pszResName);
        if (!pResProvider)
            return NULL;
        return pResProvider->LoadImgX(strType, pszResName);
    }
}

IBitmapS *SResProviderMgr::LoadImage(LPCTSTR pszType, LPCTSTR pszResName)
{
    SAutoLock lock(m_cs);
    if (!pszType)
    { // support res src by resource path.
        SPOSITION pos = m_lstResPackage.GetHeadPosition();
        while (pos)
        {
            IResProvider *pRes = m_lstResPackage.GetNext(pos);
            IBitmapS *bmp = pRes->LoadImage(NULL, pszResName);
            if (bmp)
                return bmp;
        }
        return NULL;
    }
    if (IsFileType(pszType))
    {
        SStringT strPath = m_strFilePrefix + pszResName;
        return SResLoadFromFile::LoadImage(strPath);
    }
    else
    {
#ifdef _DEBUG
        m_mapResUsageCount[SStringT().Format(_T("%s:%s"), pszType, pszResName).MakeLower()]++;
#endif

        IResProvider *pResProvider = GetMatchResProvider(pszType, pszResName);
        if (!pResProvider)
        {
            SASSERT_FMT(false, _T("load image failed, resource index %s:%s not found!"), pszType, pszResName);
            return NULL;
        }
        IBitmapS *pRet = pResProvider->LoadImage(pszType, pszResName);
        SASSERT_FMT(pRet, _T("load image failed, resource content %s:%s not found!"), pszType, pszResName);
        return pRet;
    }
}

HBITMAP SResProviderMgr::LoadBitmap(LPCTSTR pszResName, BOOL bFromFile /*= FALSE*/)
{
    SAutoLock lock(m_cs);
    if (bFromFile)
    {
        SStringT strPath = m_strFilePrefix + pszResName;
        return SResLoadFromFile::LoadBitmap(strPath);
    }
    else
    {
#ifdef _DEBUG
        m_mapResUsageCount[SStringT().Format(_T("bitmap:%s"), pszResName).MakeLower()]++;
#endif

        IResProvider *pResProvider = GetMatchResProvider(KTypeBitmap, pszResName);
        if (!pResProvider)
        {
            SASSERT_FMT(false, _T("load bitmap failed, resource index %s not found!"), pszResName);
            return NULL;
        }
        HBITMAP hBmp = pResProvider->LoadBitmap(pszResName);
        SASSERT_FMT(hBmp, _T("load bitmap failed, resource content %s not found!"), pszResName);
        return hBmp;
    }
}

HCURSOR SResProviderMgr::LoadCursor(LPCTSTR pszResName, BOOL bFromFile /*= FALSE*/)
{
    SAutoLock lock(m_cs);
    if (IS_INTRESOURCE(pszResName))
    {
        return ::LoadCursor(NULL, pszResName);
    }
    else
    {
        LPCTSTR pszCursorID = SysCursorName2ID(pszResName);
        if (pszCursorID)
            return ::LoadCursor(NULL, pszCursorID);
    }
    const CURSORMAP::CPair *pPair = m_mapCachedCursor.Lookup(pszResName);
    if (pPair)
        return pPair->m_value;

    HCURSOR hRet = NULL;
    if (bFromFile)
    {
        SStringT strPath = m_strFilePrefix + pszResName;
        hRet = SResLoadFromFile::LoadCursor(strPath);
    }
    else
    {

#ifdef _DEBUG
        m_mapResUsageCount[SStringT().Format(_T("cursor:%s"), pszResName).MakeLower()]++;
#endif

        IResProvider *pResProvider = GetMatchResProvider(KTypeCursor, pszResName);
        if (!pResProvider)
        {
            SASSERT_FMT(false, _T("load cursor failed, resource index %s not found!"), pszResName);
        }
        else
        {
            hRet = pResProvider->LoadCursor(pszResName);
        }
    }
    if (hRet)
    {
        m_mapCachedCursor[pszResName] = hRet;
    }
    else
    {
        SASSERT_FMT(false, _T("load cursor failed, resource content %s not found!"), pszResName);
    }
    return hRet;
}

HICON SResProviderMgr::LoadIcon(LPCTSTR pszResName, int cx /*=0*/, int cy /*=0*/, BOOL bFromFile /*= FALSE*/)
{
    SAutoLock lock(m_cs);
    if (bFromFile)
    {
        SStringT strPath = m_strFilePrefix + pszResName;
        return SResLoadFromFile::LoadIcon(strPath, cx, cy);
    }
    else
    {
#ifdef _DEBUG
        m_mapResUsageCount[SStringT().Format(_T("icon:%s"), pszResName).MakeLower()]++;
#endif
        IResProvider *pResProvider = GetMatchResProvider(KTypeIcon, pszResName);
        if (!pResProvider)
        {
            SASSERT_FMT(false, _T("load icon failed, resource %s not found!"), pszResName);
            return NULL;
        }
        HICON hRet = pResProvider->LoadIcon(pszResName, cx, cy);
        SASSERT_FMT(hRet, _T("load icon failed, resource content %s not found!"), pszResName);
        return hRet;
    }
}

BOOL SResProviderMgr::HasResource(LPCTSTR pszType, LPCTSTR pszResName)
{
    SAutoLock lock(m_cs);
    if (IsFileType(pszType))
    {
        SStringT strPath = m_strFilePrefix + pszResName;
        return IsFilePathValid(strPath);
    }
    else
    {
        return NULL != GetMatchResProvider(pszType, pszResName);
    }
}

IBitmapS *SResProviderMgr::LoadImage2(const SStringW &strImgID)
{
    SStringT strImgID2 = S_CW2T(strImgID);
    SStringTList strLst;
    int nSegs = ParseResID(strImgID2, strLst);
    if (nSegs == 2)
        return LoadImage(strLst[0], strLst[1]);
    else
        return LoadImage(NULL, strLst[0]);
}

HICON SResProviderMgr::LoadIcon2(const SStringW &strIconID)
{
    SStringT strIconID2 = S_CW2T(strIconID);
    SStringTList strLst;
    int nSegs = ParseResID(strIconID2, strLst);
    if (nSegs == 2)
    {
        int cx = _ttoi(strLst[1]);
        return LoadIcon(strLst[0], cx, cx);
    }
    else
    {
        return LoadIcon(strLst[0]);
    }
}

BOOL SResProviderMgr::IsFileType(LPCTSTR pszType)
{
    if (!pszType)
        return FALSE;
    return _tcsicmp(pszType, KTypeFile) == 0;
}

#ifdef _DEBUG
BOOL SResProviderMgr::CheckUsage(LPCTSTR pszName, LPCTSTR pszType, LPARAM lp)
{
    SMap<SStringT, int> *mapResUsageCount = (SMap<SStringT, int> *)lp;
    if (IS_INTRESOURCE(pszName))
        return TRUE;
    SStringT key = SStringT().Format(_T("%s:%s"), pszType, pszName);
    key.MakeLower();
    if (!mapResUsageCount->Lookup(key))
    { //发现未使用资源
        SSLOGFMTD(_T("resource of [%s] was not used."), (LPCTSTR)key);
    }
    return TRUE;
}
#endif
void SResProviderMgr::SetFilePrefix(LPCTSTR pszFilePrefix)
{
    SAutoLock lock(m_cs);
    m_strFilePrefix = pszFilePrefix;
    if (!m_strFilePrefix.EndsWith(_T("\\")))
        m_strFilePrefix.Append(_T("\\"));
}

BOOL SResProviderMgr::LoadRawBuffer(LPCTSTR pszType, LPCTSTR pszResName, IResProvider *pResProvider, SAutoBuf &buf)
{
    SAutoLock lock(m_cs);
    if (IsFileType(pszType))
    {
        size_t dwSize = SResLoadFromFile::GetRawBufferSize(pszResName);
        if (dwSize == 0)
            return FALSE;
        buf.Allocate(dwSize);
        SResLoadFromFile::GetRawBuffer(pszResName, buf, dwSize);
        return TRUE;
    }
    if (!pResProvider)
    {
        pResProvider = GetMatchResProvider(pszType, pszResName);
    }
    if (!pResProvider)
    {
        SSLOGW() << "GetMatchResProvider failed: " << pszType << ":" << pszResName << " not found in respovider list";
        return FALSE;
    }
    size_t dwSize = pResProvider->GetRawBufferSize(pszType, pszResName);
    if (dwSize == 0)
    {
        SSLOGW() << "GetMatchResProvider failed: " << pszType << ":" << pszResName << " pResProvider->GetRawBufferSize return 0";
        return FALSE;
    }
    buf.Allocate(dwSize);
    return pResProvider->GetRawBuffer(pszType, pszResName, buf, dwSize);
}

SNSEND