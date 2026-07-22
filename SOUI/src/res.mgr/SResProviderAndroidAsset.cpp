// ============================================================================
// SResProviderAndroidAsset - Android NDK AAssetManager backed IResProvider.
// ============================================================================
#ifdef __ANDROID__
#include "souistd.h"
#include <res.mgr/SResProviderAndroidAsset.h>
#include <android/asset_manager.h>
#include <res.mgr/SResProvider.h>  // SResLoadFromMemory

SNSBEGIN

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------


SStringT SResProviderAndroidAsset::NormalizeAssetPath(const SStringT &src) {
    SStringT out = src;
    out.ReplaceChar(_T('\\'), _T('/'));
    // Drop leading '/' (Android asset paths never start with '/')
    while (!out.IsEmpty() && out.GetAt(0) == _T('/')) {
        out = out.Mid(1);
    }
    return out;
}

AAsset *SResProviderAndroidAsset::OpenAsset(const SStringT &assetPath, int mode) const {
    if (!m_assetMgr || assetPath.IsEmpty()) return nullptr;
    SStringA assetPathU8 = S_CT2A(assetPath, CP_UTF8);
    return AAssetManager_open(m_assetMgr, assetPathU8.c_str(), mode);
}

SStringT SResProviderAndroidAsset::GetAssetPath(LPCTSTR strType, LPCTSTR pszResName) const {
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

SResProviderAndroidAsset::SResProviderAndroidAsset()
    : m_assetMgr(nullptr) {}

BOOL SResProviderAndroidAsset::Init(WPARAM wParam, LPARAM lParam) {
    m_assetMgr = reinterpret_cast<AAssetManager*>(wParam);
    if (!m_assetMgr) return FALSE;

    LPCTSTR prefix = reinterpret_cast<LPCTSTR>(lParam);
    if (!prefix || !*prefix) return FALSE;
    m_prefix = NormalizeAssetPath(prefix);

    // 1. 读 uires.idx 的完整 asset 路径
    SStringT idxPath = m_prefix + _T('/') + UIRES_INDEX;
    idxPath = NormalizeAssetPath(idxPath);

    AAsset *pIdxAsset = OpenAsset(idxPath, AASSET_MODE_BUFFER);
    if (!pIdxAsset) {
        return FALSE;
    }
    off64_t len = AAsset_getLength64(pIdxAsset);
    const void *data = AAsset_getBuffer(pIdxAsset);
    if (!data || len <= 0) {
        AAsset_close(pIdxAsset);
        return FALSE;
    }

    // 2. 用 pugi XML（SXmlDoc 封装）解析
    SXmlDoc xmlDoc;
    const bool bLoaded = xmlDoc.load_buffer(data, (size_t)len, xml_parse_default, enc_auto);
    AAsset_close(pIdxAsset);
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

    // 3. 遍历 type 节点（LAYOUT / values / ICON / IMG / UIDEF ...）
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
                //SSLOGI()<<"add res "<<strType.c_str()<<":"<<wsName<<" path="<<path.c_str();
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

size_t SResProviderAndroidAsset::GetRawBufferSize(LPCTSTR strType, LPCTSTR pszResName) {
    SStringT assetPath = GetAssetPath(strType, pszResName);
    if (assetPath.IsEmpty()) return 0;
    AAsset *a = OpenAsset(assetPath, AASSET_MODE_BUFFER);
    if (!a) return 0;
    off64_t len = AAsset_getLength64(a);
    AAsset_close(a);
    return len > 0 ? (size_t)len : 0;
}

BOOL SResProviderAndroidAsset::GetRawBuffer(
    LPCTSTR strType, LPCTSTR pszResName, LPVOID pBuf, size_t size) {
    SASSERT(pBuf || size == 0);
    SStringT assetPath = GetAssetPath(strType, pszResName);
    if (assetPath.IsEmpty()) return FALSE;
    AAsset *a = OpenAsset(assetPath, AASSET_MODE_BUFFER);
    if (!a) return FALSE;
    off64_t len = AAsset_getLength64(a);
    if (len <= 0) { AAsset_close(a); return FALSE; }
    if ((size_t)len > size) {
        AAsset_close(a);
        return FALSE;
    }
    const void *src = AAsset_getBuffer(a);
    if (!src) { AAsset_close(a); return FALSE; }
    memcpy(pBuf, src, (size_t)len);
    AAsset_close(a);
    return TRUE;
}

// ----------------------------------------------------------------------------
// Decode from memory buffer
// ----------------------------------------------------------------------------

static SAutoBuf *LoadToAutoBuf(SResProviderAndroidAsset *self,
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

IBitmapS *SResProviderAndroidAsset::LoadImage(LPCTSTR strType, LPCTSTR pszResName) {
    SAutoBuf *pBuf = LoadToAutoBuf(this, strType, pszResName);
    if (!pBuf) return nullptr;
    IBitmapS *ret = SResLoadFromMemory::LoadImage(
        (LPVOID)(char *)(*pBuf), (size_t)pBuf->size());
    delete pBuf;
    return ret;
}

IImgX *SResProviderAndroidAsset::LoadImgX(LPCTSTR strType, LPCTSTR pszResName) {
    SAutoBuf *pBuf = LoadToAutoBuf(this, strType, pszResName);
    if (!pBuf) return nullptr;
    IImgX *ret = SResLoadFromMemory::LoadImgX(
        (LPVOID)(char *)(*pBuf), (size_t)pBuf->size());
    delete pBuf;
    return ret;
}

// Android 平台没有 HBITMAP/HICON/HCURSOR 的 GDI handle，默认 SOUI 在非
// Windows 下也不走这些接口（通过 LoadImage + IBitmapS 展示即可）。
// 这里做一下兜底：尽量返回 0，避免误调用导致 NPE。

HBITMAP SResProviderAndroidAsset::LoadBitmap(LPCTSTR pszResName) {
    return NULL;
}

HICON SResProviderAndroidAsset::LoadIcon(LPCTSTR pszResName, int /*cx*/, int /*cy*/) {
    return NULL;
}

HCURSOR SResProviderAndroidAsset::LoadCursor(LPCTSTR pszResName) {
    return NULL;
}

// ----------------------------------------------------------------------------
// HasResource
// ----------------------------------------------------------------------------

BOOL SResProviderAndroidAsset::HasResource(LPCTSTR strType, LPCTSTR pszResName) {
    SStringT path = GetAssetPath(strType, pszResName);
    if (path.IsEmpty()) return FALSE;
    AAsset *a = OpenAsset(path, AASSET_MODE_UNKNOWN);
    if (!a) return FALSE;
    AAsset_close(a);
    return TRUE;
}

// ----------------------------------------------------------------------------
// EnumResource / EnumFile
// ----------------------------------------------------------------------------

void SResProviderAndroidAsset::EnumResource(EnumResCallback funEnumCB, LPARAM lp) {
    SPOSITION pos = m_mapFiles.GetStartPosition();
    while (pos) {
        SResID id = m_mapFiles.GetNextKey(pos);
        if (!funEnumCB(id.szName, id.szType, lp)) break;
    }
}

void SResProviderAndroidAsset::EnumFile(EnumFileCallback funEnumCB, LPARAM lp) {
    if (!funEnumCB || !m_assetMgr) return;
    _EnumFile(m_prefix, funEnumCB, lp);
}

void SResProviderAndroidAsset::_EnumFile(const SStringT &dirPath, EnumFileCallback funEnumCB, LPARAM lp) {
    if (!funEnumCB || !m_assetMgr) return;
    
    SStringA dirPathU8 = S_CT2A(dirPath, CP_UTF8);
    AAssetDir *assetDir = AAssetManager_openDir(m_assetMgr, dirPathU8.c_str());
    if (!assetDir) return;
    
    const char *fileName = nullptr;
    while ((fileName = AAssetDir_getNextFileName(assetDir)) != nullptr) {
        SStringT fileNameT = S_CA2T(fileName, CP_UTF8);
        SStringT fullPath = dirPath + _T('/') + fileNameT;
        
        SStringA fullPathU8 = S_CT2A(fullPath, CP_UTF8);
        AAsset *asset = AAssetManager_open(m_assetMgr, fullPathU8.c_str(), AASSET_MODE_UNKNOWN);
        if (asset) {
            AAsset_close(asset);
            if (!funEnumCB(fullPath.c_str(), lp)) {
                AAssetDir_close(assetDir);
                return;
            }
        } else {
            _EnumFile(fullPath, funEnumCB, lp);
        }
    }
    
    AAssetDir_close(assetDir);
}

SNSEND

#endif//__ANDROID__