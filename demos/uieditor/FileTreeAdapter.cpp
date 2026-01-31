#include "stdafx.h"
#include "FileTreeAdapter.h"
#include <shlwapi.h>
#include <helper/slog.h>
#define kLogTag "FileTreeAdapter"
SNSBEGIN

// 自动生成可用的文件名
SStringT CFileTreeAdapter::GenerateAvailableName(const SStringT& strDir, const SStringT& strPrefix, const SStringT& strExt)
{
    int nIndex = 1;
    SStringT strNewName;
    SStringT strFullPath;
    
    do
    {
        if (strExt.IsEmpty())
            strNewName.Format(_T("%s%d"), strPrefix.c_str(), nIndex);
        else
            strNewName.Format(_T("%s%d.%s"), strPrefix.c_str(), nIndex, strExt.c_str());
        strFullPath = strDir + _T("/") + strNewName;
        nIndex++;
    } while (PathFileExists(strFullPath));
    
    return strNewName;
}

CFileTreeAdapter::CFileTreeAdapter(ResManger * pResManger)
    : m_bRootInited(FALSE),m_resManger(pResManger)
{
}

CFileTreeAdapter::~CFileTreeAdapter()
{
}

void CFileTreeAdapter::Clear()
{
    m_tree.DeleteAllItems();
    m_bRootInited = FALSE;
    notifyBranchChanged(ITEM_ROOT);
}

void CFileTreeAdapter::SetRootPath(const SStringT& strRootPath)
{
    m_tree.DeleteAllItems();
    m_strRootPath = strRootPath;
    m_strRootPath.ReplaceChar(_T('\\'), _T('/'));
    // 创建根目录项
    FileItemData rootData;
    rootData.strPath = m_strRootPath;
    rootData.strName = m_strRootPath.Right(m_strRootPath.GetLength() - m_strRootPath.ReverseFind(_T('/')) - 1);
    rootData.bIsDir = TRUE;
    rootData.nChilds = -1;
    rootData.bIsCut = FALSE;
    rootData.nFileType = FT_DIR;
    
    SLOGI()<<"SetRootPath, root path:"<<m_strRootPath.c_str();
    HSTREEITEM hRootItem = InsertItem(rootData, ITEM_ROOT);
    SetItemExpanded(hRootItem, TRUE);
    
    // 枚举根目录下的文件和子目录
    EnumDirectory(m_strRootPath, hRootItem);
    // 通知数据变化
    notifyBranchChanged(ITEM_ROOT);
    SLOGI()<<"SetRootPath, notifyBranchChanged finished";
    m_bRootInited = TRUE;
}

void CFileTreeAdapter::EnumDirectory(const SStringT& strPath, HSTREEITEM hParentItem)
{
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile((strPath + _T("/*.*")).GetBuffer(), &findData);
    int nChilds = 0;
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (_tcscmp(findData.cFileName, _T(".")) == 0 || _tcscmp(findData.cFileName, _T("..")) == 0)
            {
                continue;
            }
            
            FileItemData itemData;
            itemData.strName = findData.cFileName;
            itemData.strPath = strPath + _T("/") + findData.cFileName;
            itemData.bIsDir = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            itemData.nChilds = -1;
            itemData.nFileType = FT_UNKNOWN;
            itemData.bIsCut = FALSE;
            HSTREEITEM hItem = InsertItem(itemData, hParentItem);
            nChilds++;
            // 如果是目录，设置默认展开状态
            if (itemData.bIsDir)
            {
                // 暂时不枚举子目录，等待用户展开时再枚举
                itemData.nChilds = -1;
                SetItemDataByIndex(hItem, DATA_INDEX_ITEM_EXPANDED, FALSE);
            }
            
        } while (FindNextFile(hFind, &findData));
        
        FindClose(hFind);
    }
    if (hParentItem != ITEM_ROOT)
    {
        FileItemData &itemData = GetItemData(hParentItem);
        itemData.nChilds = nChilds;
    }
}

void CFileTreeAdapter::getView(HSTREEITEM hItem, SItemPanel *pItem, SXmlNode xmlTemplate)
{
    if (hItem == ITEM_ROOT)
        return;
    
    if (pItem->GetChildrenCount() == 0)
    {
        pItem->InitFromXml(&xmlTemplate);
    }
    
    FileItemData& itemData = GetItemData(hItem);
    if(itemData.nFileType == FT_UNKNOWN){
        if(!itemData.bIsDir)
        {
            itemData.nFileType = m_resManger->GetFileType(itemData.strPath);
        }else{
            itemData.nFileType = FT_DIR;
        }
    }
    // 设置图标
    SImageWnd *pImgState = pItem->FindChildByName2<SImageWnd>(_T("img_state"));
    if (pImgState)
    {
        pImgState->SetIcon(itemData.nFileType);
        // 如果处于剪切状态，设置半透明效果
        if (itemData.bIsCut)
        {
            pImgState->SetAlpha(128); // 设置50%透明度
        }
        else
        {
            pImgState->SetAlpha(255); // 恢复完全不透明
        }
    }
    
    // 设置文本
    SWindow *pTxtLabel = pItem->FindChildByName(_T("txt_label"));
    if (pTxtLabel)
    {
        pTxtLabel->SetWindowText(itemData.strName);
        if (itemData.bIsCut)
        {
            pTxtLabel->SetAttributeA("colorText", "rgba(255,255,255,128)",FALSE);
        }
        else
        {
            pTxtLabel->SetAttributeA("colorText", "#ffffff", FALSE);
        }
    }
}

// 设置项的剪切状态
void CFileTreeAdapter::SetItemCutState(HSTREEITEM hItem, BOOL bIsCut)
{
    if (hItem == ITEM_NULL || hItem == ITEM_ROOT)
        return;
    
    FileItemData& itemData = GetItemData(hItem);
    itemData.bIsCut = bIsCut;
    
    // 通知该项需要重绘
    notifyBranchInvalidated(hItem,FALSE,FALSE);
}

// 清除所有项的剪切状态
void CFileTreeAdapter::ClearAllCutStates()
{
    // 遍历所有项，清除剪切状态
    std::vector<HSTREEITEM> items;
    GetAllItems(items, ITEM_ROOT);
    
	for (std::vector<HSTREEITEM>::iterator it = items.begin();it!=items.end(); it++)
    {
		HSTREEITEM hItem = *it;
        if (hItem != ITEM_ROOT)
        {
            FileItemData& itemData = GetItemData(hItem);
            if (itemData.bIsCut)
            {
                itemData.bIsCut = FALSE;
            }
        }
    }
    notifyBranchInvalidated(ITEM_ROOT);
}

// 检查项是否处于剪切状态
BOOL CFileTreeAdapter::IsItemCut(HSTREEITEM hItem)
{
    if (hItem == ITEM_NULL || hItem == ITEM_ROOT)
        return FALSE;
    
    const FileItemData& itemData = GetItemData(hItem);
    return itemData.bIsCut;
}

// 递归获取所有树项
void CFileTreeAdapter::GetAllItems(std::vector<HSTREEITEM>& items, HSTREEITEM hParentItem)
{
    HSTREEITEM hChildItem = GetFirstChildItem(hParentItem);
    while (hChildItem != ITEM_NULL)
    {
        items.push_back(hChildItem);
        // 递归获取子项
        GetAllItems(items, hChildItem);
        hChildItem = GetNextSiblingItem(hChildItem);
    }
}

BOOL CFileTreeAdapter::HasChildren(HSTREEITEM hItem) const
{
    if (hItem == ITEM_ROOT)
        return TRUE;
    const FileItemData& itemData = GetItemData(hItem);
    return itemData.bIsDir && itemData.nChilds != 0;
}

void CFileTreeAdapter::ExpandItem(HSTREEITEM hItem, UINT uCode)
{
    BOOL bExpandedOld = IsItemExpanded(hItem);
    BOOL bExpandedNew = bExpandedOld;
    
    switch (uCode)
    {
    case TVC_COLLAPSE:
        bExpandedNew = FALSE;
        break;
    case TVC_EXPAND:
        bExpandedNew = TRUE;
        break;
    case TVC_TOGGLE:
        bExpandedNew = !bExpandedOld;
        break;
    }
    
    if (bExpandedOld == bExpandedNew)
        return;
    
    SetItemExpanded(hItem, bExpandedNew);
    
    // 如果是展开目录，并且还没有枚举子目录，就枚举子目录
    if (bExpandedNew)
    {
        const FileItemData& itemData = GetItemData(hItem);
        if (itemData.bIsDir && itemData.nChilds == -1)
        {
            EnumDirectory(itemData.strPath, hItem);
        }
    }
    
    notifyBranchExpandChanged(hItem, bExpandedOld, bExpandedNew);
}

// 创建文件或目录
BOOL CFileTreeAdapter::CreateItem(HSTREEITEM hParentItem, const SStringT& strName, BOOL bIsDir)
{
    if (hParentItem == ITEM_NULL)
        return FALSE;
    
    const FileItemData& parentData = GetItemData(hParentItem);
    if (!parentData.bIsDir)
        return FALSE;
    
    SStringT strNewPath = parentData.strPath + _T("/") + strName;
    
    // 检查是否已存在
    if (PathFileExists(strNewPath))
        return FALSE;
    
    BOOL bSuccess = FALSE;
    if (bIsDir)
    {
        // 创建目录
        bSuccess = CreateDirectory(strNewPath, NULL) ? TRUE : FALSE;
    }
    else
    {
        // 创建文件
        HANDLE hFile = CreateFile(strNewPath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            bSuccess = TRUE;
            CloseHandle(hFile);
        }
    }
 
    return bSuccess;
}

// 复制文件或目录
BOOL CFileTreeAdapter::CopyItem(HSTREEITEM hSrcItem, HSTREEITEM hDestParentItem, const SStringT& strNewName)
{
    if (hSrcItem == ITEM_NULL || hDestParentItem == ITEM_NULL)
        return FALSE;
    
    const FileItemData& srcData = GetItemData(hSrcItem);
    const FileItemData& destParentData = GetItemData(hDestParentItem);
    
    if (!destParentData.bIsDir)
        return FALSE;
    
    SStringT strNewNameFinal = strNewName.IsEmpty() ? srcData.strName : strNewName;
    SStringT strDestPath = destParentData.strPath + _T("/") + strNewNameFinal;
    
    // 检查目标是否已存在
    if (PathFileExists(strDestPath))
        return FALSE;
    
    BOOL bSuccess = FALSE;
    if (srcData.bIsDir)
    {
        // 复制目录
        bSuccess = CopyDirectory(srcData.strPath, strDestPath);
    }
    else
    {
        // 复制文件
        bSuccess = CopyFile(srcData.strPath, strDestPath, FALSE) ? TRUE : FALSE;
    }
 
    return bSuccess;
}

// 删除文件或目录
BOOL CFileTreeAdapter::RemoveItem(HSTREEITEM hItem)
{
    if (hItem == ITEM_NULL || hItem == ITEM_ROOT)
        return FALSE;
    
    const FileItemData& itemData = GetItemData(hItem);
    HSTREEITEM hParentItem = GetParentItem(hItem);
    
    BOOL bSuccess = FALSE;
    if (itemData.bIsDir)
    {
        // 删除目录
        bSuccess = DeleteDirectory(itemData.strPath);
    }
    else
    {
        // 删除文件
        bSuccess = DeleteFile(itemData.strPath) ? TRUE : FALSE;
    }

    return bSuccess;
}

// 移动文件或目录
BOOL CFileTreeAdapter::MoveItem(HSTREEITEM hSrcItem, HSTREEITEM hDestParentItem, const SStringT& strNewName)
{
    if (hSrcItem == ITEM_NULL || hDestParentItem == ITEM_NULL || hSrcItem == ITEM_ROOT)
        return FALSE;
    
    const FileItemData& srcData = GetItemData(hSrcItem);
    const FileItemData& destParentData = GetItemData(hDestParentItem);
    
    if (!destParentData.bIsDir)
        return FALSE;
    
    SStringT strNewNameFinal = strNewName.IsEmpty() ? srcData.strName : strNewName;
    SStringT strDestPath = destParentData.strPath + _T("/") + strNewNameFinal;
    
    // 检查目标是否已存在
    if (PathFileExists(strDestPath))
        return FALSE;
    
    // 移动文件或目录
    BOOL bSuccess = MoveFile(srcData.strPath, strDestPath) ? TRUE : FALSE;
 
    return bSuccess;
}

// 刷新目录
void CFileTreeAdapter::RefreshDirectory(HSTREEITEM hItem)
{
    if (hItem == ITEM_NULL)
        return;
    SStringT strPath;
    BOOL bExpanded = TRUE;
    if (hItem == ITEM_ROOT)
    {
        strPath = m_strRootPath;
    }
    else
    {
        const FileItemData &itemData = GetItemData(hItem);
        if (!itemData.bIsDir)
            return;
        strPath = itemData.strPath;
    }
    
    // 删除所有子项
    HSTREEITEM hChildItem = GetFirstChildItem(hItem);
    while (hChildItem != ITEM_NULL)
    {
        HSTREEITEM hNextItem = GetNextSiblingItem(hChildItem);
        DeleteItem(hChildItem);
        hChildItem = hNextItem;
    }
    
    // 重新枚举目录
    EnumDirectory(strPath, hItem);
    // 恢复展开状态
    if (bExpanded)
    {
        SetItemExpanded(hItem, TRUE);
        notifyBranchChanged(hItem);
    }
}

// 根据路径查找树项
HSTREEITEM CFileTreeAdapter::FindItemByPath(const SStringT& strPath)
{
    // 从根目录开始查找
    HSTREEITEM hItem = GetFirstChildItem(ITEM_ROOT);
    if (hItem == ITEM_NULL)
        return ITEM_NULL;
    
    // 规范化路径
    SStringT strNormalizedPath = strPath;
    strNormalizedPath.ReplaceChar(_T('\\'), _T('/'));
    
    // 如果路径是根目录
    if (strNormalizedPath == m_strRootPath)
        return hItem;
    
    // 确保路径以根目录开始
    if (!strNormalizedPath.StartsWith(m_strRootPath))
        return ITEM_NULL;
    
    // 从根目录的子项开始查找
    HSTREEITEM hChildItem = GetFirstChildItem(hItem);
    while (hChildItem != ITEM_NULL)
    {
        const FileItemData& itemData = GetItemData(hChildItem);
        SStringT itemPath = itemData.strPath;
            itemPath.ReplaceChar(_T('\\'), _T('/'));
        
        if (itemPath == strNormalizedPath)
        {
            // 找到匹配的项
            return hChildItem;
        }
        else if (itemData.bIsDir && strNormalizedPath.StartsWith(itemPath + _T('/')))
        {
            // 进入子目录继续查找
            hItem = hChildItem;
            hChildItem = GetFirstChildItem(hItem);
        }
        else
        {
            // 继续查找兄弟项
            hChildItem = GetNextSiblingItem(hChildItem);
        }
    }
    
    return ITEM_NULL;
}

// 根据路径删除树项
BOOL CFileTreeAdapter::RemoveItemByPath(const SStringT& strPath)
{
    // 查找对应的树项
    HSTREEITEM hItem = FindItemByPath(strPath);
    if (hItem == ITEM_NULL)
        return FALSE; // 文件不在树中，不处理
    
    // 获取父项
    HSTREEITEM hParentItem = GetParentItem(hItem);
    if (hParentItem == ITEM_NULL)
        return FALSE; // 不能删除根项
    
    // 删除项
    DeleteItem(hItem);
    
    // 通知父项变化
    notifyBranchChanged(hParentItem);
    return TRUE;
}

// 根据路径添加树项
BOOL CFileTreeAdapter::AddItemByPath(const SStringT& strPath)
{
    // 规范化路径
    SStringT strNormalizedPath = strPath;
    strNormalizedPath.ReplaceChar(_T('\\'), _T('/'));
    
    // 确保路径以根目录开始
    if (!strNormalizedPath.StartsWith(m_strRootPath))
        return FALSE;
    
    // 获取父目录路径
    int nLastSlash = strNormalizedPath.ReverseFind(_T('/'));
    if (nLastSlash <= 0)
        return FALSE;
    
    SStringT strParentPath = strNormalizedPath.Left(nLastSlash);
    SStringT strItemName = strNormalizedPath.Mid(nLastSlash + 1);
    
    // 查找父目录对应的树项
    HSTREEITEM hParentItem = FindItemByPath(strParentPath);
    if (hParentItem == ITEM_NULL)
        return FALSE; // 父目录不在树中
    
    // 检查父目录是否已展开
    const FileItemData& parentData = GetItemData(hParentItem);
    if (!parentData.bIsDir)
        return FALSE;
    if (parentData.nChilds==-1)
        return TRUE;
    // 检查文件是否已存在
    if (FindItemByPath(strNormalizedPath) != ITEM_NULL)
        return FALSE; // 文件已存在
    
    // 检查路径是否存在
    if (!PathFileExists(strPath))
        return FALSE;
    
    // 确定是文件还是目录
    DWORD dwAttrib = GetFileAttributes(strPath);
    BOOL bIsDir = (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
    
    // 创建新项数据
    FileItemData newData;
    newData.strPath = strPath;
    newData.strName = strItemName;
    newData.bIsDir = bIsDir;
    newData.bIsCut = FALSE;
    newData.nChilds = bIsDir ? -1 : 0;
    newData.nFileType = FT_UNKNOWN;
    
    // 插入新项
    InsertItem(newData, hParentItem);
    
    // 通知父项变化
    notifyBranchChanged(hParentItem);
    return TRUE;
}

// 递归创建目录
BOOL CFileTreeAdapter::CreateDirectoryRecursive(const SStringT& strPath)
{
    if (PathFileExists(strPath))
        return TRUE;
    
    // 创建父目录
    SStringT strParentPath = strPath.Left(strPath.ReverseFind(_T('/')));
    if (!CreateDirectoryRecursive(strParentPath))
        return FALSE;
    
    // 创建当前目录
    return CreateDirectory(strPath, NULL) ? TRUE : FALSE;
}

// 复制目录
BOOL CFileTreeAdapter::CopyDirectory(const SStringT& strSrcPath, const SStringT& strDestPath)
{
    // 创建目标目录
    if (!CreateDirectory(strDestPath, NULL))
    {
        if (GetLastError() != ERROR_ALREADY_EXISTS)
            return FALSE;
    }
    
    // 枚举源目录中的文件和子目录
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile((strSrcPath + _T("/*.*")).GetBuffer(), &findData);
    
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (_tcscmp(findData.cFileName, _T(".")) == 0 || _tcscmp(findData.cFileName, _T("..")) == 0)
                continue;
            
            SStringT strSrcItem = strSrcPath + _T("/") + findData.cFileName;
            SStringT strDestItem = strDestPath + _T("/") + findData.cFileName;
            
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // 递归复制子目录
                if (!CopyDirectory(strSrcItem, strDestItem))
                {
                    FindClose(hFind);
                    return FALSE;
                }
            }
            else
            {
                // 复制文件
                if (!CopyFile(strSrcItem, strDestItem, FALSE))
                {
                    FindClose(hFind);
                    return FALSE;
                }
            }
            
        } while (FindNextFile(hFind, &findData));
        
        FindClose(hFind);
    }
    
    return TRUE;
}

// 删除目录
BOOL CFileTreeAdapter::DeleteDirectory(const SStringT& strPath)
{
    // 枚举目录中的文件和子目录
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile((strPath + _T("/*.*")).GetBuffer(), &findData);
    
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (_tcscmp(findData.cFileName, _T(".")) == 0 || _tcscmp(findData.cFileName, _T("..")) == 0)
                continue;
            
            SStringT strItem = strPath + _T("/") + findData.cFileName;
            
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // 递归删除子目录
                if (!DeleteDirectory(strItem))
                {
                    FindClose(hFind);
                    return FALSE;
                }
            }
            else
            {
                // 删除文件
                if (!DeleteFile(strItem))
                {
                    FindClose(hFind);
                    return FALSE;
                }
            }
            
        } while (FindNextFile(hFind, &findData));
        
        FindClose(hFind);
    }
    
    // 删除空目录
    return RemoveDirectory(strPath) ? TRUE : FALSE;
}

SNSEND