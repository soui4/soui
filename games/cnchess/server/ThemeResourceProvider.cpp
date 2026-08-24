// ThemeResourceProvider.cpp: 主题资源管理器实现
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ThemeResourceProvider.h"
#include <helper/slog.h>
#define kLogTag "ThemeResourceProvider"

ThemeResourceProvider* ThemeResourceProvider::GetInstance()
{
    static ThemeResourceProvider s_instance;
    return &s_instance;
}

ThemeResourceProvider::ThemeResourceProvider()
{
}

ThemeResourceProvider::~ThemeResourceProvider()
{
}

bool ThemeResourceProvider::Init(DWORD dwOSId, const char* pszThemeZipPath)
{
    FILE* f = fopen(pszThemeZipPath, "rb");
    if (!f) {
        SLOGE() << "Failed to open theme zip: " << pszThemeZipPath;
        return false;
    }

    fseek(f, 0, SEEK_END);
    long nSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (nSize <= 0) {
        SLOGE() << "Theme zip is empty or invalid: " << pszThemeZipPath;
        fclose(f);
        return false;
    }

    ThemeEntry& entry = m_themes[dwOSId];
    entry.zipData.resize(nSize);
    size_t nRead = fread(entry.zipData.data(), 1, nSize, f);
    fclose(f);

    if (nRead != (size_t)nSize) {
        SLOGE() << "Failed to read theme zip data";
        entry.zipData.clear();
        return false;
    }

    entry.dwTotalSize = (DWORD)nSize;
    MD5_Buffer(entry.zipData.data(), entry.zipData.size(), entry.md5);
    entry.bLoaded = true;
    return true;
}

const ThemeResourceProvider::ThemeEntry* ThemeResourceProvider::GetEntry(DWORD dwOSId) const
{
    // 未知平台(0)回退到桌面主题(OS_ID_WINDOWS)
    if (dwOSId == OS_ID_UNKNOWN)
        dwOSId = OS_ID_WINDOWS;

    auto it = m_themes.find(dwOSId);
    if (it == m_themes.end() || !it->second.bLoaded)
        return nullptr;
    return &it->second;
}

ThemeResourceProvider::ThemeEntry* ThemeResourceProvider::GetEntryMutable(DWORD dwOSId)
{
    if (dwOSId == OS_ID_UNKNOWN)
        dwOSId = OS_ID_WINDOWS;
    auto it = m_themes.find(dwOSId);
    if (it == m_themes.end() || !it->second.bLoaded)
        return nullptr;
    return &it->second;
}

bool ThemeResourceProvider::IsLoaded(DWORD dwOSId) const
{
    return GetEntry(dwOSId) != nullptr;
}

const unsigned char* ThemeResourceProvider::GetMD5(DWORD dwOSId) const
{
    const ThemeEntry* p = GetEntry(dwOSId);
    return p ? p->md5 : nullptr;
}

DWORD ThemeResourceProvider::GetTotalSize(DWORD dwOSId) const
{
    const ThemeEntry* p = GetEntry(dwOSId);
    return p ? p->dwTotalSize : 0;
}

bool ThemeResourceProvider::GetDataChunk(DWORD dwOSId, DWORD dwOffset, DWORD dwChunkSize, const BYTE** ppOutData, DWORD* pOutLen) const
{
    const ThemeEntry* p = GetEntry(dwOSId);
    if (!p || dwOffset >= p->dwTotalSize) {
        *pOutLen = 0;
        return false;
    }

    DWORD dwRemaining = p->dwTotalSize - dwOffset;
    DWORD dwActualLen = (dwChunkSize < dwRemaining) ? dwChunkSize : dwRemaining;

    *ppOutData = p->zipData.data() + dwOffset;
    *pOutLen = dwActualLen;
    return true;
}

bool ThemeResourceProvider::IsMD5Match(DWORD dwOSId, const unsigned char clientMD5[16]) const
{
    const ThemeEntry* p = GetEntry(dwOSId);
    if (!p) return false;
    return MD5_Compare(p->md5, clientMD5);
}
