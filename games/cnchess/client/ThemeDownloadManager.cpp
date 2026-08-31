// ThemeDownloadManager.cpp: 客户端主题资源下载管理器实现
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ThemeDownloadManager.h"
#include "WebSocketClient.h"
#include "md5.h"
#include <resprovider-zip/zipresprovider-param.h>
#include <helper/slog.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <shellapi.h>
#define kLogTag "ThemeDownloadMgr"

// 递归创建目录
static bool MakeDirs(const SStringT& strPath)
{
    if (strPath.IsEmpty()) return false;
    if (GetFileAttributes(strPath) != INVALID_FILE_ATTRIBUTES)
        return true;

    // 找到父目录
    int nPos = strPath.ReverseFind(PATH_SLASH);
    if (nPos > 0)
    {
        SStringT strParent = strPath.Left(nPos);
        if (!strParent.IsEmpty() && !MakeDirs(strParent))
            return false;
    }
    return CreateDirectory(strPath,NULL) || GetLastError() == ERROR_ALREADY_EXISTS;
}

// 从完整路径中提取目录部分并创建
static bool EnsureParentDir(const SStringT& strFilePath)
{
    int nPos = strFilePath.ReverseFind(PATH_SLASH);
    if (nPos <= 0) return true;
    SStringT strDir = strFilePath.Left(nPos);
    return MakeDirs(strDir);
}

// 规范化ZIP中的路径分隔符为当前平台分隔符
static SStringT NormalizeZipPath(const TCHAR* pszZipPath)
{
    SStringT strPath = pszZipPath;
    // ZIP内部使用'/'，统一替换为当前平台分隔符
    strPath.ReplaceChar(_T('/'), TPATH_SLASH);
    strPath.ReplaceChar(_T('\\'), TPATH_SLASH);
    return strPath;
}

// EnumFile 回调参数结构
struct ExtractContext
{
    IResProvider* pResProvider;
    const SStringT* pstrDestDir;
    bool bSuccess;
    int nFileCount;
};

// EnumFile 回调函数：提取单个文件
static BOOL CALLBACK ExtractOneFile(LPCTSTR pszFileName, LPARAM lp)
{
    ExtractContext* pCtx = (ExtractContext*)lp;
    if (!pCtx || !pCtx->pResProvider) return FALSE;

    // 跳过以目录分隔符结尾的项（纯目录项）
    size_t len = _tcslen(pszFileName);
    if (len > 0 && (pszFileName[len - 1] == _T('/') || pszFileName[len - 1] == _T('\\')))
        return TRUE;

    // 规范化路径
    SStringT strRelPath = NormalizeZipPath(pszFileName);
    SStringT strFullPath = *(pCtx->pstrDestDir) + TPATH_SLASH2 + strRelPath;

    // 确保父目录存在
    if (!EnsureParentDir(strFullPath))
    {
        SLOGW() << "ExtractZip: failed to create parent dir for: " << S_CT2A(strFullPath);
        pCtx->bSuccess = false;
        return TRUE;
    }

    // 获取文件大小
    size_t szFile = pCtx->pResProvider->GetRawBufferSize(NULL, pszFileName);
    if (szFile == 0)
    {
        // 空文件，直接创建
        FILE* fOut = _tfopen(strFullPath, _T("wb"));
        if (fOut)
        {
            fclose(fOut);
            pCtx->nFileCount++;
        }
        else
        {
            SLOGW() << "ExtractZip: failed to create empty file: " << S_CT2A(strFullPath);
            pCtx->bSuccess = false;
        }
        return TRUE;
    }

    // 分配缓冲区读取
    size_t szAlloc = szFile;
    BYTE* pBuf = new BYTE[szAlloc];
    if (!pBuf)
    {
        SLOGW() << "ExtractZip: out of memory for file: " << S_CT2A(strRelPath);
        pCtx->bSuccess = false;
        return TRUE;
    }

    BOOL bRead = pCtx->pResProvider->GetRawBuffer(NULL, pszFileName, pBuf, szAlloc);
    if (!bRead)
    {
        SLOGW() << "ExtractZip: failed to read file: " << S_CT2A(strRelPath);
        delete[] pBuf;
        pCtx->bSuccess = false;
        return TRUE;
    }

    // 写入文件
    FILE* fOut = _tfopen(strFullPath, _T("wb"));
    if (!fOut)
    {
        SLOGW() << "ExtractZip: failed to open output: " << S_CT2A(strFullPath);
        delete[] pBuf;
        pCtx->bSuccess = false;
        return TRUE;
    }

    fwrite(pBuf, 1, szFile, fOut);
    fclose(fOut);
    delete[] pBuf;
    pCtx->nFileCount++;
    return TRUE;
}

ThemeDownloadManager::ThemeDownloadManager()
    : m_pListener(NULL)
    , m_state(STATE_IDLE)
    , m_dwTotalSize(0)
    , m_dwReceivedBytes(0)
    , m_pWsClient(NULL)
{
    memset(m_serverMD5, 0, sizeof(m_serverMD5));
}

ThemeDownloadManager::~ThemeDownloadManager()
{
}

bool ThemeDownloadManager::Init(const SStringT& strCacheDir)
{
    m_strCacheDir = strCacheDir;
    m_strZipPath = strCacheDir + TPATH_SLASH2 + _T("def_theme.zip");
    m_strMD5Path = strCacheDir + TPATH_SLASH2 + _T("def_theme.md5");
    m_strThemeDir = strCacheDir + TPATH_SLASH2 + _T("def_theme");

    // 确保缓存目录存在
    if (!MakeDirs(m_strCacheDir))
    {
        SLOGE() << "Failed to create cache dir: " << S_CT2A(m_strCacheDir);
        return false;
    }
    return true;
}

bool ThemeDownloadManager::GetLocalMD5(unsigned char md5[16]) const
{
    memset(md5, 0, 16);

    // 优先读取保存的MD5文件
    FILE* f = _tfopen(m_strMD5Path, _T("rb"));
    if (f)
    {
        size_t nRead = fread(md5, 1, 16, f);
        fclose(f);
        if (nRead == 16)
        {
            // 验证zip文件是否存在
            if (GetFileAttributes(m_strZipPath) != INVALID_FILE_ATTRIBUTES)
                return true;
        }
    }

    // 如果没有MD5文件但zip存在，则计算zip的MD5
    if (GetFileAttributes(m_strZipPath) != INVALID_FILE_ATTRIBUTES)
    {
        SStringA strZipPathA = S_CT2A(m_strZipPath);
        if (MD5_File(strZipPathA.c_str(), md5))
            return true;
    }

    memset(md5, 0, 16);
    return false;
}

bool ThemeDownloadManager::IsThemeReady() const
{
    // 主题目录存在且包含config.xml
    SStringT strConfig = m_strThemeDir + TPATH_SLASH2 + _T("config.xml");
    return GetFileAttributes(strConfig) != INVALID_FILE_ATTRIBUTES;
}

bool ThemeDownloadManager::RequestTheme(WebSocketClient* pWs)
{
    if (!pWs)
    {
        SLOGE() << "RequestTheme: WebSocket client is null";
        return false;
    }

    m_pWsClient = pWs;
    m_state = STATE_WAITING_ACK;
    m_dwReceivedBytes = 0;
    m_zipChunks.clear();

    // 获取本地MD5
    unsigned char localMD5[16];
    GetLocalMD5(localMD5);

    // 发送主题请求
    THEME_REQ req;
    memset(&req, 0, sizeof(req));
    memcpy(req.md5, localMD5, 16);
    // 根据编译平台设置 OS ID，服务器据此下发对应主题资源包
#if defined(__MOBILE__)
    req.dwOSId = OS_ID_ANDROID;
#else
    req.dwOSId = OS_ID_WINDOWS;
#endif

    bool bRet = pWs->SendMsg(GMT_THEME_REQ, &req, sizeof(req)) != FALSE;
    SLOGI() << "RequestTheme: sent THEME_REQ, OSId=" << req.dwOSId << " bRet=" << bRet;
    return bRet;
}

void ThemeDownloadManager::Reset()
{
    m_state = STATE_IDLE;
    m_dwReceivedBytes = 0;
    m_zipChunks.clear();
    m_pWsClient = NULL;
}

bool ThemeDownloadManager::HandleMessage(DWORD dwType, const BYTE* pData, DWORD dwSize)
{
    switch (dwType)
    {
    case GMT_THEME_ACK:
        OnThemeAck(pData, dwSize);
        return true;
    case GMT_THEME_DATA:
        OnThemeData(pData, dwSize);
        return true;
    default:
        return false;
    }
}

void ThemeDownloadManager::OnThemeAck(const BYTE* pData, DWORD dwSize)
{
    if (m_state != STATE_WAITING_ACK)
    {
        SLOGW() << "OnThemeAck: unexpected state=" << m_state;
        return;
    }

    if (dwSize < sizeof(THEME_ACK))
    {
        NotifyError("Invalid THEME_ACK size");
        return;
    }

    const THEME_ACK* pAck = (const THEME_ACK*)pData;
    memcpy(m_serverMD5, pAck->md5, 16);
    m_dwTotalSize = pAck->dwTotalSize;

    if (m_dwTotalSize == 0)
    {
        // MD5匹配，无需下载
        SLOGI() << "OnThemeAck: MD5 match, no download needed";

        // 确保本地已解压
        if (!IsThemeReady())
        {
            // 本地没有解压目录，从zip解压
            if (GetFileAttributes(m_strZipPath) != INVALID_FILE_ATTRIBUTES)
            {
                m_state = STATE_EXTRACTING;
                if (ExtractZip(m_strZipPath, m_strThemeDir))
                {
                    m_state = STATE_DONE;
                    NotifyReady(false);
                }
                else
                {
                    NotifyError("Failed to extract cached theme zip");
                }
            }
            else
            {
                // 服务器说MD5匹配但本地既没有解压目录也没有zip
                // 这种情况不应该发生（可能是第一次连接时本地MD5为0但服务器也为0）
                NotifyError("Theme not available: no local cache and server reports match");
            }
        }
        else
        {
            m_state = STATE_DONE;
            NotifyReady(false);
        }
        return;
    }

    // 需要下载
    SLOGI() << "OnThemeAck: need download, totalSize=" << m_dwTotalSize;
    m_state = STATE_DOWNLOADING;
    m_dwReceivedBytes = 0;
}

void ThemeDownloadManager::OnThemeData(const BYTE* pData, DWORD dwSize)
{
    if (m_state != STATE_DOWNLOADING)
    {
        SLOGW() << "OnThemeData: unexpected state=" << m_state;
        return;
    }

    if (dwSize < sizeof(DWORD) * 2)
    {
        SLOGE() << "OnThemeData: invalid data size=" << dwSize;
        return;
    }

    const THEME_DATA* pDataHdr = (const THEME_DATA*)pData;
    DWORD dwOffset = pDataHdr->dwOffset;
    DWORD dwDataLen = pDataHdr->dwDataLen;

    if (dwOffset + dwDataLen > m_dwTotalSize)
    {
        SLOGE() << "OnThemeData: data exceeds total size, offset=" << dwOffset
                << " len=" << dwDataLen << " total=" << m_dwTotalSize;
        NotifyError("Theme data exceeds expected size");
        return;
    }

    const BYTE* pChunk = pDataHdr->byData;
    // 以分块形式保存接收到的数据，避免一次性分配完整内存块
    m_zipChunks.emplace_back(dwOffset, dwDataLen);
    memcpy(m_zipChunks.back().data.data(), pChunk, dwDataLen);
    m_dwReceivedBytes += dwDataLen;

    // 通知进度
    if (m_pListener)
    {
        int nPercent = (int)((m_dwReceivedBytes * 100LL) / m_dwTotalSize);
        m_pListener->OnThemeProgress(nPercent);
    }

    SLOGI() << "OnThemeData: offset=" << dwOffset << " len=" << dwDataLen
            << " received=" << m_dwReceivedBytes << "/" << m_dwTotalSize;

    // 检查是否下载完成
    if (m_dwReceivedBytes >= m_dwTotalSize)
    {
        m_state = STATE_EXTRACTING;
        if (SaveDownloadedZip())
        {
            if (ExtractZip(m_strZipPath, m_strThemeDir))
            {
                m_state = STATE_DONE;
                NotifyReady(true);
            }
            else
            {
                NotifyError("Failed to extract downloaded theme zip");
            }
        }
    }
}

bool ThemeDownloadManager::SaveDownloadedZip()
{
    // 按偏移排序分块，确保写入顺序正确
    m_zipChunks.sort([](const Chunk& a, const Chunk& b) {
        return a.dwOffset < b.dwOffset;
    });

    // 校验分块连续性：无缺口、无重叠，且覆盖完整区间 [0, m_dwTotalSize)
    DWORD dwExpected = 0;
    for (const auto& chunk : m_zipChunks)
    {
        if (chunk.dwOffset != dwExpected)
        {
            SLOGE() << "SaveDownloadedZip: data gap/overlap at offset " << chunk.dwOffset
                    << " expected " << dwExpected;
            NotifyError("Downloaded theme data is incomplete or corrupted");
            return false;
        }
        dwExpected += (DWORD)chunk.data.size();
    }
    if (dwExpected != m_dwTotalSize)
    {
        SLOGE() << "SaveDownloadedZip: size mismatch, got=" << dwExpected
                << " expected=" << m_dwTotalSize;
        NotifyError("Downloaded theme size mismatch");
        return false;
    }

    // 增量计算MD5，无需合并为完整内存块
    MD5_CTX ctx;
    MD5_Init(&ctx);
    for (const auto& chunk : m_zipChunks)
    {
        if (!chunk.data.empty())
            MD5_Update(&ctx, chunk.data.data(), chunk.data.size());
    }
    unsigned char localMD5[16];
    MD5_Final(localMD5, &ctx);

    if (!MD5_Compare(localMD5, m_serverMD5))
    {
        SLOGE() << "SaveDownloadedZip: MD5 mismatch!";
        NotifyError("Downloaded theme MD5 mismatch");
        return false;
    }

    // 确保目录存在
    EnsureParentDir(m_strZipPath);

    // 顺序写入zip文件，逐块写入，无需合并为完整内存块
    FILE* f = _tfopen(m_strZipPath, _T("wb"));
    if (!f)
    {
        SLOGE() << "SaveDownloadedZip: failed to write zip: " << S_CT2A(m_strZipPath);
        NotifyError("Failed to write theme zip file");
        return false;
    }
    for (const auto& chunk : m_zipChunks)
    {
        if (!chunk.data.empty())
            fwrite(chunk.data.data(), 1, chunk.data.size(), f);
    }
    fclose(f);

    // 保存MD5文件
    WriteMD5File(m_serverMD5);

    SLOGI() << "SaveDownloadedZip: saved, size=" << m_dwTotalSize;
    m_zipChunks.clear();
    return true;
}

bool ThemeDownloadManager::WriteMD5File(const unsigned char md5[16])
{
    FILE* f = _tfopen(m_strMD5Path, _T("wb"));
    if (!f) return false;
    fwrite(md5, 1, 16, f);
    fclose(f);
    return true;
}

bool ThemeDownloadManager::ExtractZip(const SStringT& strZipPath, const SStringT& strDestDir)
{
    SLOGI() << "ExtractZip: zip=" << S_CT2A(strZipPath) << " dest=" << S_CT2A(strDestDir);
    SComMgr2 comMgr;
    // 创建 SResProviderZip 实例
    SAutoRefPtr<IResProvider> pResProvider;
    if (!comMgr.CreateResProvider_ZIP((IObjRef**)&pResProvider)) {
        SLOGE() << "ExtractZip: failed to create SResProviderZip instance";
        return false;
    }

    // 初始化 ResProvider（加载 ZIP 文件）
    ZIPRES_PARAM zipParam;
    ZipFile(&zipParam, NULL, strZipPath, NULL, NULL);
    if (!pResProvider->Init((WPARAM)&zipParam, 0))
    {
        SLOGE() << "ExtractZip: failed to init ResProvider with zip";
        return false;
    }
    if(GetFileAttributes(strDestDir)!=INVALID_FILE_ATTRIBUTES)
    {//remove old files.
        SHFILEOPSTRUCT op={0};
        TCHAR szBuf[MAX_PATH] = { 0 };
        _tcscpy(szBuf, strDestDir);
        op.pFrom = szBuf;
        op.wFunc = FO_DELETE;
        op.fFlags = FOF_NOCONFIRMATION|
            FOF_NOERRORUI |  
            FOF_SILENT;
        SHFileOperation(&op);
    }
    // 创建目标目录
    if (!MakeDirs(strDestDir))
    {
        SLOGE() << "ExtractZip: failed to create dest dir";
        return false;
    }

    // 枚举并提取文件
    ExtractContext ctx;
    ctx.pResProvider = pResProvider;
    ctx.pstrDestDir = &strDestDir;
    ctx.bSuccess = true;
    ctx.nFileCount = 0;

    pResProvider->EnumFile(ExtractOneFile, (LPARAM)&ctx);
    pResProvider = NULL;//release the resprovider before commgr deconstructor.
    SLOGI() << "ExtractZip: extracted " << ctx.nFileCount << " files";
    return ctx.bSuccess;
}

void ThemeDownloadManager::NotifyError(const SStringA& strErr)
{
    m_state = STATE_ERROR;
    SLOGE() << "ThemeDownload error: " << strErr;
    if (m_pListener)
        m_pListener->OnThemeError(strErr);
}

void ThemeDownloadManager::NotifyReady(bool bUpdated)
{
    if (m_pListener)
        m_pListener->OnThemeReady(m_strThemeDir, bUpdated);
}
