// ============================================================================
// SResProviderOhosRawFile - OHOS NativeResourceManager rawfile backed IResProvider.
// 与 SResProviderAndroidAsset 语义同构：Init 解析 prefix/uires.idx 建立
// SResID(type,name) -> 相对路径映射，其余接口基于该映射 + rawfile API 直读。
// ============================================================================
#ifdef __OHOS__
#include "souistd.h"
#include <res.mgr/SResProviderOhosRawFile.h>
#include <rawfile/raw_file.h>
#include <rawfile/raw_dir.h>
#include <res.mgr/SResProvider.h>  // SResLoadFromMemory

SNSBEGIN

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------

SStringT SResProviderOhosRawFile::NormalizeAssetPath(const SStringT &src) {
    SStringT out = src;
    out.ReplaceChar(_T('\\'), _T('/'));
    // Drop leading '/' (OHOS rawfile paths never start with '/')
    while (!out.IsEmpty() && out.GetAt(0) == _T('/')) {
        out = out.Mid(1);
    }
    return out;
}

RawFile *SResProviderOhosRawFile::OpenRawFile(const SStringT &filePath) const {
    if (!m_resMgr || filePath.IsEmpty()) return nullptr;
    SStringA filePathU8 = S_CT2A(filePath, CP_UTF8);
    return OH_ResourceManager_OpenRawFile(m_resMgr, filePathU8.c_str());
}

SStringT SResProviderOhosRawFile::GetAssetPath(LPCTSTR strType, LPCTSTR pszResName) const {
    if (!pszResName) return _T("");
    SStringT relative;
    if (!strType || !*strType) {
        // 直接当相对路径使用（外部传入 path:xxx 这类场景）
        relative = NormalizeAssetPath(pszResName);
    } else {
        SResID id(strType, pszResName);
        auto *p = m_mapFiles.Lookup(id);
        if (!p) return _T("");
        relative = p->m_value;  // 存入时已经 NormalizeAssetPath
    }
    if (relative.IsEmpty()) return _T("");
    SStringT full = m_prefix + _T('/') + relative;
    return NormalizeAssetPath(full);
}

// ----------------------------------------------------------------------------
// Init
// ----------------------------------------------------------------------------

SResProviderOhosRawFile::SResProviderOhosRawFile()
    : m_resMgr(nullptr) {}

BOOL SResProviderOhosRawFile::Init(WPARAM wParam, LPARAM lParam) {
    m_resMgr = reinterpret_cast<NativeResourceManager*>(wParam);
    if (!m_resMgr) return FALSE;

    LPCTSTR prefix = reinterpret_cast<LPCTSTR>(lParam);
    if (!prefix || !*prefix) return FALSE;
    m_prefix = NormalizeAssetPath(prefix);

    // 1. 读 uires.idx 的完整 rawfile 路径
    SStringT idxPath = m_prefix + _T('/') + UIRES_INDEX;
    idxPath = NormalizeAssetPath(idxPath);

    RawFile *pIdxFile = OpenRawFile(idxPath);
    if (!pIdxFile) {
        return FALSE;
    }
    long len = OH_ResourceManager_GetRawFileSize(pIdxFile);
    if (len <= 0) {
        OH_ResourceManager_CloseRawFile(pIdxFile);
        return FALSE;
    }

    // 2. 整读 uires.idx 到内存（rawfile API 需先 seek 到 0 再读）
    SAutoBuf idxBuf;
    idxBuf.Allocate((size_t)len);
    OH_ResourceManager_SeekRawFile(pIdxFile, 0, 0); // SEEK_SET
    int nRead = OH_ResourceManager_ReadRawFile(pIdxFile, (LPVOID)(char *)idxBuf, (size_t)len);
    OH_ResourceManager_CloseRawFile(pIdxFile);
    if (nRead != (int)len) {
        return FALSE;
    }

    // 3. 用 pugi XML（SXmlDoc 封装）解析
    SXmlDoc xmlDoc;
    const bool bLoaded = xmlDoc.load_buffer((const void *)(char *)idxBuf, (size_t)nRead, xml_parse_default, enc_auto);
    if (!bLoaded) {
        return FALSE;
    }

    SXmlNode xmlResource = xmlDoc.root().child(L"resource");
    if (!xmlResource) {
        // 老版本的 uires.idx 可能没有 <resource> 包装，允许 root 直接作为根
        xmlResource = xmlDoc.root().first_child();
    }
    if (!xmlResource) {
        return FALSE;
    }

    // 4. 遍历 type 节点（LAYOUT / values / ICON / IMG / UIDEF ...）
    SXmlNode xmlType = xmlResource.first_child();
    while (xmlType) {
        // pugi name 是 UTF-8，S_CW2T 在窄字符模式下会转换
        SStringT strType = S_CW2T(xmlType.name());
        SXmlNode xmlFile = xmlType.child(L"file");
        while (xmlFile) {
            const wchar_t *wsName = xmlFile.attribute(L"name").value();
            const wchar_t *wsPath = xmlFile.attribute(L"path").value();
            SStringT name = S_CW2T(wsName);
            SStringT path = NormalizeAssetPath(S_CW2T(wsPath));
            if (!name.IsEmpty() && !path.IsEmpty()) {
                SResID id(strType, name);
                m_mapFiles[id] = path;
            }
            xmlFile = xmlFile.next_sibling(L"file");
        }
        xmlType = xmlType.next_sibling();
    }
    return TRUE;
}

// ----------------------------------------------------------------------------
// Raw buffer
// ----------------------------------------------------------------------------

size_t SResProviderOhosRawFile::GetRawBufferSize(LPCTSTR strType, LPCTSTR pszResName) {
    SStringT assetPath = GetAssetPath(strType, pszResName);
    if (assetPath.IsEmpty()) return 0;
    RawFile *f = OpenRawFile(assetPath);
    if (!f) return 0;
    long len = OH_ResourceManager_GetRawFileSize(f);
    OH_ResourceManager_CloseRawFile(f);
    return len > 0 ? (size_t)len : 0;
}

BOOL SResProviderOhosRawFile::GetRawBuffer(
    LPCTSTR strType, LPCTSTR pszResName, LPVOID pBuf, size_t size) {
    SASSERT(pBuf || size == 0);
    SStringT assetPath = GetAssetPath(strType, pszResName);
    if (assetPath.IsEmpty()) return FALSE;
    RawFile *f = OpenRawFile(assetPath);
    if (!f) return FALSE;
    long len = OH_ResourceManager_GetRawFileSize(f);
    if (len <= 0) { OH_ResourceManager_CloseRawFile(f); return FALSE; }
    if ((size_t)len > size) {
        OH_ResourceManager_CloseRawFile(f);
        return FALSE;
    }
    // rawfile API 从当前偏移读取，需先 seek 到文件头
    OH_ResourceManager_SeekRawFile(f, 0, 0); // SEEK_SET
    int nRead = OH_ResourceManager_ReadRawFile(f, pBuf, (size_t)len);
    OH_ResourceManager_CloseRawFile(f);
    return nRead == (int)len ? TRUE : FALSE;
}

// ----------------------------------------------------------------------------
// Decode from memory buffer
// ----------------------------------------------------------------------------

static SAutoBuf *LoadToAutoBuf(SResProviderOhosRawFile *self,
                               LPCTSTR strType, LPCTSTR pszResName) {
    size_t sz = self->GetRawBufferSize(strType, pszResName);
    if (sz == 0) return nullptr;
    SAutoBuf *pBuf = new SAutoBuf();
    pBuf->Allocate(sz);
    if (!self->GetRawBuffer(strType, pszResName, (LPVOID)(char *)(*pBuf), sz)) {
        delete pBuf;
        return nullptr;
    }
    return pBuf;
}

IBitmapS *SResProviderOhosRawFile::LoadImage(LPCTSTR strType, LPCTSTR pszResName) {
    SAutoBuf *pBuf = LoadToAutoBuf(this, strType, pszResName);
    if (!pBuf) return nullptr;
    IBitmapS *ret = SResLoadFromMemory::LoadImage(
        (LPVOID)(char *)(*pBuf), (size_t)pBuf->size());
    delete pBuf;
    return ret;
}

IImgX *SResProviderOhosRawFile::LoadImgX(LPCTSTR strType, LPCTSTR pszResName) {
    SAutoBuf *pBuf = LoadToAutoBuf(this, strType, pszResName);
    if (!pBuf) return nullptr;
    IImgX *ret = SResLoadFromMemory::LoadImgX(
        (LPVOID)(char *)(*pBuf), (size_t)pBuf->size());
    delete pBuf;
    return ret;
}

// OHOS 平台没有 HBITMAP/HICON/HCURSOR 的 GDI handle，默认 SOUI 在非
// Windows 下也不走这些接口（通过 LoadImage + IBitmapS 展示即可）。
// 这里做一下兜底：尽量返回 0，避免误调用导致 NPE。

HBITMAP SResProviderOhosRawFile::LoadBitmap(LPCTSTR pszResName) {
    return NULL;
}

HICON SResProviderOhosRawFile::LoadIcon(LPCTSTR pszResName, int /*cx*/, int /*cy*/) {
    return NULL;
}

HCURSOR SResProviderOhosRawFile::LoadCursor(LPCTSTR pszResName) {
    return NULL;
}

// ----------------------------------------------------------------------------
// HasResource
// ----------------------------------------------------------------------------

BOOL SResProviderOhosRawFile::HasResource(LPCTSTR strType, LPCTSTR pszResName) {
    SStringT path = GetAssetPath(strType, pszResName);
    if (path.IsEmpty()) return FALSE;
    RawFile *f = OpenRawFile(path);
    if (!f) return FALSE;
    OH_ResourceManager_CloseRawFile(f);
    return TRUE;
}

// ----------------------------------------------------------------------------
// EnumResource / EnumFile
// ----------------------------------------------------------------------------

void SResProviderOhosRawFile::EnumResource(EnumResCallback funEnumCB, LPARAM lp) {
    SPOSITION pos = m_mapFiles.GetStartPosition();
    while (pos) {
        SResID id = m_mapFiles.GetNextKey(pos);
        if (!funEnumCB(id.szName, id.szType, lp)) break;
    }
}

void SResProviderOhosRawFile::EnumFile(EnumFileCallback funEnumCB, LPARAM lp) {
    if (!funEnumCB || !m_resMgr) return;
    _EnumFile(m_prefix, funEnumCB, lp);
}

void SResProviderOhosRawFile::_EnumFile(const SStringT &dirPath, EnumFileCallback funEnumCB, LPARAM lp) {
    if (!funEnumCB || !m_resMgr) return;

    SStringA dirPathU8 = S_CT2A(dirPath, CP_UTF8);
    RawDir *rawDir = OH_ResourceManager_OpenRawDir(m_resMgr, dirPathU8.c_str());
    if (!rawDir) return;

    int count = OH_ResourceManager_GetRawFileCount(rawDir);
    for (int i = 0; i < count; ++i) {
        const char *pszName = OH_ResourceManager_GetRawFileName(rawDir, i);
        if (!pszName) continue;
        SStringT nameT = S_CA2T(pszName, CP_UTF8);
        SStringT fullPath = dirPath + _T('/') + nameT;

        SStringA fullPathU8 = S_CT2A(fullPath, CP_UTF8);
        if (OH_ResourceManager_IsRawDir(m_resMgr, fullPathU8.c_str())) {
            _EnumFile(fullPath, funEnumCB, lp);
        } else {
            if (!funEnumCB(fullPath.c_str(), lp)) {
                OH_ResourceManager_CloseRawDir(rawDir);
                return;
            }
        }
    }

    OH_ResourceManager_CloseRawDir(rawDir);
}

SNSEND

#endif//__OHOS__
