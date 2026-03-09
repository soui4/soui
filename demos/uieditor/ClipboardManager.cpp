#include "stdafx.h"
#include "ClipboardManager.h"
#include <vector>
#include <sstream>
#include <helper/slog.h>
#include <windows.h>
#include <shlobj.h>
#define kLogTag "ClipboardManager"

// 静态成员初始化
UINT CClipboardManager::m_uClipboardFormat = 0;

CClipboardManager::CClipboardManager(HWND hWnd)
    : m_hWnd(hWnd)
{
    InitializeClipboardFormat();
}

CClipboardManager::~CClipboardManager()
{
}

void CClipboardManager::InitializeClipboardFormat()
{
    if (m_uClipboardFormat == 0)
    {
        m_uClipboardFormat = RegisterClipboardFormat(_T("SOUI_UIEDITOR_FILE_ITEMS"));
    }
}

const UINT CClipboardManager::GetClipboardFormat()
{
    if (m_uClipboardFormat == 0)
    {
        m_uClipboardFormat = RegisterClipboardFormat(_T("SOUI_UIEDITOR_FILE_ITEMS"));
    }
    return m_uClipboardFormat;
}

void CClipboardManager::SerializeItemsToClipboard(const std::vector<HSTREEITEM>& items, int nOperation, CFileTreeAdapter* pAdapter)
{
    if(items.empty())
        return;
    if(!OpenClipboard(m_hWnd))
    {
        SLOGW()<<"OpenClipboard failed";
        return;
    }
    EmptyClipboard();
    // 准备数据：操作类型 + 项目数量 + 项目路径列表（UTF-8）
    std::vector<SStringA> vecItemPathsUtf8;
    for(size_t i = 0; i < items.size(); i++)
    {
        const FileItemData& itemData = pAdapter->GetItemData(items[i]);
        vecItemPathsUtf8.push_back(S_CT2A(itemData.strPath));
    }

    // 计算所需缓冲区大小
    std::stringstream ss;
    int nCount = vecItemPathsUtf8.size();
    for(size_t i = 0; i < vecItemPathsUtf8.size(); i++)
    {
        ss<< vecItemPathsUtf8[i].c_str();
        ss<< '\0';
    }
    ss<< '\0';

    // 分配内存
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, ss.str().size() + sizeof(int) * 2);
    if(hGlobal)
    {
        LPVOID lpData = GlobalLock(hGlobal);
        if(lpData)
        {
            BYTE* pBuffer = (BYTE*)lpData;
            
            // 写入操作类型
            *((int*)pBuffer) = nOperation;
            pBuffer += sizeof(int);
            
            // 写入项目数量
            *((int*)pBuffer) = nCount;
            pBuffer += sizeof(int);
            // 写入项目路径（UTF-8）
            memcpy(pBuffer, ss.str().c_str(), ss.str().size());
            
            GlobalUnlock(hGlobal);
            SetClipboardData(m_uClipboardFormat, hGlobal);
        }
        else
        {
            GlobalFree(hGlobal);
        }
    }
    hGlobal = GlobalAlloc(GMEM_MOVEABLE, ss.str().size() + sizeof(DROPFILES));
    if(hGlobal)
    {
        LPVOID lpData = GlobalLock(hGlobal);
        if(lpData)
        {
            BYTE* pBuffer = (BYTE*)lpData;
            DROPFILES* pDropFiles = (DROPFILES*)pBuffer;
            pDropFiles->fNC = FALSE;
            pDropFiles->fWide = FALSE;
            pDropFiles->pFiles = sizeof(DROPFILES);
            pBuffer += sizeof(DROPFILES);
            memcpy(pBuffer, ss.str().c_str(), ss.str().size());
            GlobalUnlock(hGlobal);
            SetClipboardData(CF_HDROP, hGlobal);
        }
        else
        {
            GlobalFree(hGlobal);
        }
    }
    CloseClipboard();
}

BOOL CClipboardManager::HasClipboardData()
{
    BOOL bHasData = FALSE;
    
    if(IsClipboardFormatAvailable(m_uClipboardFormat))
    {
        if(OpenClipboard(m_hWnd))
        {
            HGLOBAL hGlobal = (HGLOBAL)GetClipboardData(m_uClipboardFormat);
            if(hGlobal)
            {
                LPVOID lpData = GlobalLock(hGlobal);
                if(lpData)
                {
                    BYTE* pBuffer = (BYTE*)lpData;
                    int nOperation = *((int*)pBuffer);
                    int nCount = *((int*)(pBuffer + sizeof(int)));
                    bHasData = (nOperation > 0 && nCount > 0);
                    GlobalUnlock(hGlobal);
                }
            }
            CloseClipboard();
        }
    }
    
    if(!bHasData && IsClipboardFormatAvailable(CF_HDROP))
    {
        bHasData = TRUE;
    }
    
    return bHasData;
}

BOOL CClipboardManager::DeserializeItemsFromClipboard(std::vector<SStringT>& vecItemPaths, int& nOperation)
{
    vecItemPaths.clear();

    // 首先尝试读取自定义格式
    if(IsClipboardFormatAvailable(m_uClipboardFormat))
    {
        if(OpenClipboard(m_hWnd))
        {
            HGLOBAL hGlobal = (HGLOBAL)GetClipboardData(m_uClipboardFormat);
            if(hGlobal)
            {
                LPVOID lpData = GlobalLock(hGlobal);
                if(lpData)
                {
                    BYTE* pBuffer = (BYTE*)lpData;
                    
                    // 读取操作类型
                    nOperation = *((int*)pBuffer);
                    pBuffer += sizeof(int);
                    
                    // 读取项目数量
                    int nCount = *((int*)pBuffer);
                    pBuffer += sizeof(int);
                    
                    // 读取项目路径（UTF-8）
                    for(int i = 0; i < nCount; i++)
                    {
                        // 读取 UTF-8 字符串
                        char* szUtf8Path = (char*)pBuffer;
                        int nLen = strlen(szUtf8Path) + 1;
                        // 转换为宽字符串
                        SStringT strPath = S_CA2T(szUtf8Path, CP_UTF8);
                        vecItemPaths.push_back(strPath);
                        pBuffer += nLen;
                    }
                    
                    GlobalUnlock(hGlobal);
                    CloseClipboard();
                    return !vecItemPaths.empty();
                }
            }
            CloseClipboard();
        }
    }

    // 如果自定义格式失败，尝试读取系统资源管理器的 CF_HDROP 格式
    if(IsClipboardFormatAvailable(CF_HDROP))
    {
        if(OpenClipboard(m_hWnd))
        {
            HGLOBAL hGlobal = (HGLOBAL)GetClipboardData(CF_HDROP);
            if(hGlobal)
            {
                HDROP hDrop = (HDROP)GlobalLock(hGlobal);
                if(hDrop)
                {
                    UINT nFileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
                    for(UINT i = 0; i < nFileCount; i++)
                    {
                        TCHAR szFilePath[MAX_PATH];
                        if(DragQueryFile(hDrop, i, szFilePath, MAX_PATH))
                        {
                            vecItemPaths.push_back(szFilePath);
                        }
                    }
                    GlobalUnlock(hGlobal);
                    CloseClipboard();
                    if(!vecItemPaths.empty())
                    {
                        nOperation = 1; // 资源管理器复制默认为复制操作
                        return TRUE;
                    }
                }
            }
            CloseClipboard();
        }
    }

    return FALSE;
}