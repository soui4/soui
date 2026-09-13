//============================================================================
/** SResProviderOhosRawFile - OHOS NativeResourceManager rawfile backed IResProvider. */
/** Structurally identical in semantics to SResProviderAndroidAsset: Init parses prefix/uires.idx and builds */
/** SResID(type,name) -> relative path mapping; other interfaces read directly via this mapping + rawfile API. */
//============================================================================
#ifdef __OHOS__
#include "souistd.h"
#include <res.mgr/SResProviderOhosRawFile.h>
#include <rawfile/raw_file.h>
#include <rawfile/raw_dir.h>
#include <res.mgr/SResProvider.h> /**< SResLoadFromMemory */

SNSBEGIN

//----------------------------------------------------------------------------
/** Helpers */
//----------------------------------------------------------------------------

SStringT SResProviderOhosRawFile::NormalizeAssetPath(const SStringT &src)
{
    SStringT out = src;
    out.ReplaceChar(_T('\\'), _T('/'));
    // Drop leading '/' (OHOS rawfile paths never start with '/')
    while (!out.IsEmpty() && out.GetAt(0) == _T('/'))
    {
        out = out.Mid(1);
    }
    return out;
}

RawFile *SResProviderOhosRawFile::OpenRawFile(const SStringT &filePath) const
{
    if (!m_resMgr || filePath.IsEmpty())
        return nullptr;
    SStringA filePathU8 = S_CT2A(filePath, CP_UTF8);
    return OH_ResourceManager_OpenRawFile(m_resMgr, filePathU8.c_str());
}

SStringT SResProviderOhosRawFile::GetAssetPath(LPCTSTR strType, LPCTSTR pszResName) const
{
    if (!pszResName)
        return _T("");
    SStringT relative;
    if (!strType || !*strType)
    {
        // Use directly as a relative path (e.g. cases where path:xxx is passed externally)
        relative = NormalizeAssetPath(pszResName);
    }
    else
    {
        SResID id(strType, pszResName);
        auto *p = m_mapFiles.Lookup(id);
        if (!p)
            return _T("");
        relative = p->m_value; // Already normalized via NormalizeAssetPath when stored
    }
    if (relative.IsEmpty())
        return _T("");
    SStringT full = m_prefix + _T('/') + relative;
    return NormalizeAssetPath(full);
}

//----------------------------------------------------------------------------
/** Init */
//----------------------------------------------------------------------------

SResProviderOhosRawFile::SResProviderOhosRawFile()
    : m_resMgr(nullptr)
{
}

BOOL SResProviderOhosRawFile::Init(WPARAM wParam, LPARAM lParam)
{
    m_resMgr = reinterpret_cast<NativeResourceManager *>(wParam);
    if (!m_resMgr)
        return FALSE;

    LPCTSTR prefix = reinterpret_cast<LPCTSTR>(lParam);
    if (!prefix || !*prefix)
        return FALSE;
    m_prefix = NormalizeAssetPath(prefix);

    // 1. Read the full rawfile path of uires.idx
    SStringT idxPath = m_prefix + _T('/') + UIRES_INDEX;
    idxPath = NormalizeAssetPath(idxPath);

    RawFile *pIdxFile = OpenRawFile(idxPath);
    if (!pIdxFile)
    {
        return FALSE;
    }
    long len = OH_ResourceManager_GetRawFileSize(pIdxFile);
    if (len <= 0)
    {
        OH_ResourceManager_CloseRawFile(pIdxFile);
        return FALSE;
    }

    // 2. Read the entire uires.idx into memory (rawfile API must seek to 0 before reading)
    SAutoBuf idxBuf;
    idxBuf.Allocate((size_t)len);
    OH_ResourceManager_SeekRawFile(pIdxFile, 0, 0); // SEEK_SET
    int nRead = OH_ResourceManager_ReadRawFile(pIdxFile, (LPVOID)(char *)idxBuf, (size_t)len);
    OH_ResourceManager_CloseRawFile(pIdxFile);
    if (nRead != (int)len)
    {
        return FALSE;
    }

    // 3. Parse using pugi XML (SXmlDoc wrapper)
    SXmlDoc xmlDoc;
    const bool bLoaded = xmlDoc.load_buffer((const void *)(char *)idxBuf, (size_t)nRead, xml_parse_default, enc_auto);
    if (!bLoaded)
    {
        return FALSE;
    }

    SXmlNode xmlResource = xmlDoc.root().child(L"resource");
    if (!xmlResource)
    {
        // Old versions of uires.idx may lack the <resource> wrapper; allow root to be used directly as the root
        xmlResource = xmlDoc.root().first_child();
    }
    if (!xmlResource)
    {
        return FALSE;
    }

    // 4. Iterate type nodes (LAYOUT / values / ICON / IMG / UIDEF ...)
    SXmlNode xmlType = xmlResource.first_child();
    while (xmlType)
    {
        // pugi name is UTF-8; S_CW2T converts under narrow-character mode
        SStringT strType = S_CW2T(xmlType.name());
        SXmlNode xmlFile = xmlType.child(L"file");
        while (xmlFile)
        {
            const wchar_t *wsName = xmlFile.attribute(L"name").value();
            const wchar_t *wsPath = xmlFile.attribute(L"path").value();
            SStringT name = S_CW2T(wsName);
            SStringT path = NormalizeAssetPath(S_CW2T(wsPath));
            if (!name.IsEmpty() && !path.IsEmpty())
            {
                SResID id(strType, name);
                m_mapFiles[id] = path;
            }
            xmlFile = xmlFile.next_sibling(L"file");
        }
        xmlType = xmlType.next_sibling();
    }
    return TRUE;
}

//----------------------------------------------------------------------------
/** Raw buffer */
//----------------------------------------------------------------------------

size_t SResProviderOhosRawFile::GetRawBufferSize(LPCTSTR strType, LPCTSTR pszResName)
{
    SStringT assetPath = GetAssetPath(strType, pszResName);
    if (assetPath.IsEmpty())
        return 0;
    RawFile *f = OpenRawFile(assetPath);
    if (!f)
        return 0;
    long len = OH_ResourceManager_GetRawFileSize(f);
    OH_ResourceManager_CloseRawFile(f);
    return len > 0 ? (size_t)len : 0;
}

BOOL SResProviderOhosRawFile::GetRawBuffer(LPCTSTR strType, LPCTSTR pszResName, LPVOID pBuf, size_t size)
{
    SASSERT(pBuf || size == 0);
    SStringT assetPath = GetAssetPath(strType, pszResName);
    if (assetPath.IsEmpty())
        return FALSE;
    RawFile *f = OpenRawFile(assetPath);
    if (!f)
        return FALSE;
    long len = OH_ResourceManager_GetRawFileSize(f);
    if (len <= 0)
    {
        OH_ResourceManager_CloseRawFile(f);
        return FALSE;
    }
    if ((size_t)len > size)
    {
        OH_ResourceManager_CloseRawFile(f);
        return FALSE;
    }
    // The rawfile API reads from the current offset; must seek to the file head first
    OH_ResourceManager_SeekRawFile(f, 0, 0); // SEEK_SET
    int nRead = OH_ResourceManager_ReadRawFile(f, pBuf, (size_t)len);
    OH_ResourceManager_CloseRawFile(f);
    return nRead == (int)len ? TRUE : FALSE;
}

//----------------------------------------------------------------------------
/** Decode from memory buffer */
//----------------------------------------------------------------------------

static SAutoBuf *LoadToAutoBuf(SResProviderOhosRawFile *self, LPCTSTR strType, LPCTSTR pszResName)
{
    size_t sz = self->GetRawBufferSize(strType, pszResName);
    if (sz == 0)
        return nullptr;
    SAutoBuf *pBuf = new SAutoBuf();
    pBuf->Allocate(sz);
    if (!self->GetRawBuffer(strType, pszResName, (LPVOID)(char *)(*pBuf), sz))
    {
        delete pBuf;
        return nullptr;
    }
    return pBuf;
}

IBitmapS *SResProviderOhosRawFile::LoadImage(LPCTSTR strType, LPCTSTR pszResName)
{
    SAutoBuf *pBuf = LoadToAutoBuf(this, strType, pszResName);
    if (!pBuf)
        return nullptr;
    IBitmapS *ret = SResLoadFromMemory::LoadImage((LPVOID)(char *)(*pBuf), (size_t)pBuf->size());
    delete pBuf;
    return ret;
}

IImgX *SResProviderOhosRawFile::LoadImgX(LPCTSTR strType, LPCTSTR pszResName)
{
    SAutoBuf *pBuf = LoadToAutoBuf(this, strType, pszResName);
    if (!pBuf)
        return nullptr;
    IImgX *ret = SResLoadFromMemory::LoadImgX((LPVOID)(char *)(*pBuf), (size_t)pBuf->size());
    delete pBuf;
    return ret;
}

/** The OHOS platform has no GDI handles for HBITMAP/HICON/HCURSOR; by default SOUI on non- */
/** Windows also does not use these interfaces (LoadImage + IBitmapS is sufficient for display). */
/** Add a fallback here: return 0 when possible to avoid NPE from mistaken calls. */

HBITMAP SResProviderOhosRawFile::LoadBitmap(LPCTSTR pszResName)
{
    return NULL;
}

HICON SResProviderOhosRawFile::LoadIcon(LPCTSTR pszResName, int /**< cx */, int /**< cy */)
{
    return NULL;
}

HCURSOR SResProviderOhosRawFile::LoadCursor(LPCTSTR pszResName)
{
    return NULL;
}

//----------------------------------------------------------------------------
/** HasResource */
//----------------------------------------------------------------------------

BOOL SResProviderOhosRawFile::HasResource(LPCTSTR strType, LPCTSTR pszResName)
{
    SStringT path = GetAssetPath(strType, pszResName);
    if (path.IsEmpty())
        return FALSE;
    RawFile *f = OpenRawFile(path);
    if (!f)
        return FALSE;
    OH_ResourceManager_CloseRawFile(f);
    return TRUE;
}

//----------------------------------------------------------------------------
/** EnumResource / EnumFile */
//----------------------------------------------------------------------------

void SResProviderOhosRawFile::EnumResource(EnumResCallback funEnumCB, LPARAM lp)
{
    SPOSITION pos = m_mapFiles.GetStartPosition();
    while (pos)
    {
        SResID id = m_mapFiles.GetNextKey(pos);
        if (!funEnumCB(id.szName, id.szType, lp))
            break;
    }
}

void SResProviderOhosRawFile::EnumFile(EnumFileCallback funEnumCB, LPARAM lp)
{
    if (!funEnumCB || !m_resMgr)
        return;
    _EnumFile(m_prefix, funEnumCB, lp);
}

void SResProviderOhosRawFile::_EnumFile(const SStringT &dirPath, EnumFileCallback funEnumCB, LPARAM lp)
{
    if (!funEnumCB || !m_resMgr)
        return;

    SStringA dirPathU8 = S_CT2A(dirPath, CP_UTF8);
    RawDir *rawDir = OH_ResourceManager_OpenRawDir(m_resMgr, dirPathU8.c_str());
    if (!rawDir)
        return;

    int count = OH_ResourceManager_GetRawFileCount(rawDir);
    for (int i = 0; i < count; ++i)
    {
        const char *pszName = OH_ResourceManager_GetRawFileName(rawDir, i);
        if (!pszName)
            continue;
        SStringT nameT = S_CA2T(pszName, CP_UTF8);
        SStringT fullPath = dirPath + _T('/') + nameT;

        SStringA fullPathU8 = S_CT2A(fullPath, CP_UTF8);
        if (OH_ResourceManager_IsRawDir(m_resMgr, fullPathU8.c_str()))
        {
            _EnumFile(fullPath, funEnumCB, lp);
        }
        else
        {
            if (!funEnumCB(fullPath.c_str(), lp))
            {
                OH_ResourceManager_CloseRawDir(rawDir);
                return;
            }
        }
    }

    OH_ResourceManager_CloseRawDir(rawDir);
}

SNSEND

#endif /**< __OHOS__ */
