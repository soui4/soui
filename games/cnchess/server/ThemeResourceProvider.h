// ThemeResourceProvider.h: 主题资源管理器
// 负责加载多平台主题zip包、计算MD5、提供分块数据
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>
#include <map>
#include "md5.h"
#include <protocol.h>

class ThemeResourceProvider
{
public:
    static ThemeResourceProvider* GetInstance();

    // 初始化，加载指定 OS ID 对应的主题 zip 文件
    bool Init(DWORD dwOSId, const char* pszThemeZipPath);

    // 指定 OS ID 的主题是否已加载
    bool IsLoaded(DWORD dwOSId) const;

    // 获取指定 OS ID 的主题 MD5。未加载返回 nullptr
    const unsigned char* GetMD5(DWORD dwOSId) const;

    // 获取指定 OS ID 的主题 zip 总大小。未加载返回 0
    DWORD GetTotalSize(DWORD dwOSId) const;

    // 获取指定 OS ID 主题在指定偏移处的数据块
    bool GetDataChunk(DWORD dwOSId, DWORD dwOffset, DWORD dwChunkSize, const BYTE** ppOutData, DWORD* pOutLen) const;

    // 比较指定 OS ID 主题的 MD5 与客户端 MD5
    bool IsMD5Match(DWORD dwOSId, const unsigned char clientMD5[16]) const;

private:
    ThemeResourceProvider();
    ~ThemeResourceProvider();

    struct ThemeEntry {
        std::vector<BYTE> zipData;   // 主题 zip 数据
        unsigned char    md5[16];   // 主题 zip 的 MD5
        DWORD            dwTotalSize; // 主题 zip 总大小
        bool             bLoaded;   // 是否已加载
        ThemeEntry() : dwTotalSize(0), bLoaded(false) { memset(md5, 0, sizeof(md5)); }
    };

    // OS_ID -> ThemeEntry。未知平台(0)回退到桌面主题(OS_ID_WINDOWS)
    const ThemeEntry* GetEntry(DWORD dwOSId) const;
    ThemeEntry* GetEntryMutable(DWORD dwOSId);

    std::map<DWORD, ThemeEntry> m_themes;
};
