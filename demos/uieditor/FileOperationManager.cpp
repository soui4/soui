#include "stdafx.h"
#include "FileOperationManager.h"
#include "Dialog/DlgInput.h"
#include <shlwapi.h>
#include <atl.mini/SComCli.h>
#include "FileTreeDragdrop.h"
#include "MainDlg.h"
#include <helper/slog.h>
#define kLogTag "FileOperationManager"

enum {
    file_op_copy=1,
    file_op_cut=2,
};

CFileOperationManager::CFileOperationManager( CMainDlg *pMainDlg, STreeView* pTreeView, CFileTreeAdapter* pAdapter, HWND hWnd)
    : m_pTreeView(pTreeView)
    , m_pFileTreeAdapter(pAdapter)
    , m_pClipboardManager(new CClipboardManager(hWnd))
    , m_pMainDlg(pMainDlg)
{
}

CFileOperationManager::~CFileOperationManager()
{
    if (m_pClipboardManager)
    {
        delete m_pClipboardManager;
    }
}

std::vector<HSTREEITEM> CFileOperationManager::GetSelectedItems(HSTREEITEM hDefaultItem)
{
    std::vector<HSTREEITEM> selectedItems;
    int selCount = m_pTreeView->GetSelItemCount();
    if(selCount > 0)
    {
        HSTREEITEM *pItems = new HSTREEITEM[selCount];
        m_pTreeView->GetSelItems(pItems, selCount);
        for(int i = 0; i < selCount; i++)
        {
            selectedItems.push_back(pItems[i]);
        }
        delete []pItems;
    }
    else if(hDefaultItem)
    {
        selectedItems.push_back(hDefaultItem);
    }
    return selectedItems;
}

BOOL CFileOperationManager::HandleTreeViewKeyboardShortcut(UINT nChar)
{
    if(!m_pTreeView)
        return FALSE;

#ifdef __APPLE__
    BOOL  bCtrlPressed  = (GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000 != 0;
#else
    BOOL bCtrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
#endif
    HSTREEITEM hItem = m_pTreeView->GetSel();
    if(bCtrlPressed)
    {
        if(nChar == _T('C') || nChar == _T('c'))
        {
            OnFileCopy();
            return TRUE;
        }
        else if(nChar == _T('V') || nChar == _T('v'))
        {
            if(hItem == ITEM_NULL)
                return FALSE;
            
            OnFilePaste(hItem);
            return TRUE;
        }
        else if(nChar == _T('X') || nChar == _T('x'))
        {
            OnFileCut();
            return TRUE;
        }
    }
    else if (nChar == VK_DELETE || nChar == VK_BACK)
    {
        OnFileDelete(ITEM_NULL);
        return TRUE;
    }
    else if (nChar == VK_F2)
    {
        if (hItem == ITEM_NULL)
            return FALSE;
        OnFileRename(hItem);
        return TRUE;
    }
    return FALSE;
}

void CFileOperationManager::OnFileCopy()
{
    std::vector<HSTREEITEM> selectedItems = GetSelectedItems();
    if(!selectedItems.empty())
    {
        // 清除之前的剪切状态
        m_pFileTreeAdapter->ClearAllCutStates();
        // 调用剪贴板管理器序列化项目
        m_pClipboardManager->SerializeItemsToClipboard(selectedItems, file_op_copy, m_pFileTreeAdapter);
    }
}

void CFileOperationManager::OnFileCut()
{
    std::vector<HSTREEITEM> selectedItems = GetSelectedItems();
    if(!selectedItems.empty())
    {
        // 清除之前的剪切状态
        m_pFileTreeAdapter->ClearAllCutStates();
        // 标记当前选中项为剪切状态
        for(std::vector<HSTREEITEM>::iterator it = selectedItems.begin();it!= selectedItems.end();it++)
        {
            m_pFileTreeAdapter->SetItemCutState(*it, TRUE);
        }
        // 调用剪贴板管理器序列化项目
        m_pClipboardManager->SerializeItemsToClipboard(selectedItems, file_op_cut, m_pFileTreeAdapter);
    }
}

void CFileOperationManager::OnFileDelete(HSTREEITEM hItem)
{
    std::vector<HSTREEITEM> itemsToDelete = GetSelectedItems(hItem);
    if (!itemsToDelete.empty())
    {
        SStringT strMsg = _T("确定要删除") + SStringT().Format(_T("%d"), itemsToDelete.size()) + _T("个项目吗？");
        if (SMessageBox(NULL, strMsg, _T("提示"), MB_YESNO | MB_ICONQUESTION) == IDYES)
        {
            for (size_t i = 0; i < itemsToDelete.size(); i++)
            {
                m_pFileTreeAdapter->RemoveItem(itemsToDelete[i]);
            }
        }
    }
}

void CFileOperationManager::OnFileRename(HSTREEITEM hItem)
{
    const FileItemData &itemData = m_pFileTreeAdapter->GetItemData(hItem);
    SDlgInput dlgInput;
    dlgInput.m_strTitle = _T("请输入新名称");
    dlgInput.m_strValue = itemData.strName;
    if (IDOK == dlgInput.DoModal(NULL) && !dlgInput.m_strValue.IsEmpty())
    {
        ::MoveFile(itemData.strPath, itemData.strPath.Left(itemData.strPath.ReverseFind(_T('/')) + 1) + dlgInput.m_strValue);
    }
}

void CFileOperationManager::OnFilePaste()
{
    HSTREEITEM hItem = m_pTreeView->GetSel();
    if(hItem)
    {
        OnFilePaste(hItem);
    }
}

void CFileOperationManager::OnFilePaste(HSTREEITEM hItem, BOOL bCopy, HGLOBAL hdrop)
{
    std::vector<SStringT> vecClipboardItems;
    int nClipboardOperation = 0;
    BOOL bFromClipboard = FALSE;
    if (hdrop)
    {
        bFromClipboard = m_pClipboardManager->DeserializeItemsFromHdrop(hdrop,vecClipboardItems, nClipboardOperation);
        if(bFromClipboard && vecClipboardItems.size() == 1 && vecClipboardItems[0].EndsWith(_T(".idx"))){
            m_pMainDlg->OnDrop(vecClipboardItems[0]);
            return;
        }
    }    
    else
        bFromClipboard = m_pClipboardManager->DeserializeItemsFromClipboard(vecClipboardItems, nClipboardOperation);

    if (bFromClipboard && !vecClipboardItems.empty() && nClipboardOperation > 0)
    {
        const FileItemData &destItemData = m_pFileTreeAdapter->GetItemData(hItem);
        SStringT strDestPath = destItemData.bIsDir ? destItemData.strPath : destItemData.strPath.Left(destItemData.strPath.ReverseFind(_T('/')));

        for (size_t i = 0; i < vecClipboardItems.size(); i++)
        {
            SStringT strSrcPath = vecClipboardItems[i];
            strSrcPath.ReplaceChar(_T('\\'), _T('/'));
            int nPos = strSrcPath.ReverseFind(_T('/'));
            SStringT strFileName = strSrcPath.Mid(nPos + 1);
            SStringT strDestFilePath = strDestPath + _T('/') + strFileName;

            if (nClipboardOperation == 1)
            {
                if (::PathFileExists(strDestFilePath))
                {
                    SStringT strBaseName = strFileName.Left(strFileName.ReverseFind(_T('.')));
                    SStringT strExt = strFileName.Mid(strFileName.ReverseFind(_T('.')));
                    int nIndex = 1;
                    while (::PathFileExists(strDestPath + _T('/') + strBaseName + _T('(') + SStringT().Format(_T("%d"), nIndex) + _T(')') + strExt))
                    {
                        nIndex++;
                    }
                    strDestFilePath = strDestPath + _T('/') + strBaseName + _T('(') + SStringT().Format(_T("%d"), nIndex) + _T(')') + strExt;
                }

                if (::PathIsDirectory(strSrcPath))
                {
                    SHFILEOPSTRUCT fos = { 0 };
                    fos.wFunc = bCopy?FO_COPY:FO_MOVE;
                    fos.pFrom = strSrcPath + _T('\0');
                    fos.pTo = strDestFilePath + _T('\0');
                    fos.fFlags = FOF_NOCONFIRMMKDIR | FOF_SILENT | FOF_NOERRORUI;
                    ::SHFileOperation(&fos);
                }
                else
                {
                    if (bCopy)
                        ::CopyFile(strSrcPath, strDestFilePath, FALSE);
                    else
                        ::MoveFile(strSrcPath, strDestFilePath);
                }
            }
            else if (nClipboardOperation == file_op_cut)
            {
                if (::PathIsDirectory(strSrcPath))
                {
                    SHFILEOPSTRUCT fos = { 0 };
                    fos.wFunc = FO_MOVE;
                    fos.pFrom = strSrcPath + _T('\0');
                    fos.pTo = strDestPath + _T('\0');
                    fos.fFlags = FOF_NOCONFIRMMKDIR | FOF_SILENT | FOF_NOERRORUI;
                    ::SHFileOperation(&fos);
                }
                else
                {
                    ::MoveFile(strSrcPath, strDestFilePath);
                }
            }
        }
        // 清除所有剪切状态
        m_pFileTreeAdapter->ClearAllCutStates();
    }
}

BOOL CFileOperationManager::CreateDragData(const std::vector<HSTREEITEM>& selectedItems, HGLOBAL& hGlobal, std::vector<SStringW>& filePaths)
{
    // 收集所有选中项目的文件路径信息
    int totalPathLength = 0;
    
    for(size_t i = 0; i < selectedItems.size(); i++)
    {
        const FileItemData &itemData = m_pFileTreeAdapter->GetItemData(selectedItems[i]);
        SStringW strPathW = S_CT2W(itemData.strPath);
        filePaths.push_back(strPathW);
        totalPathLength += strPathW.GetLength() + 1; // +1 用于宽字符终止符
    }

    // 创建 DROPFILES 格式的全局内存，用于支持标准文件拖放
    hGlobal = GlobalAlloc(GMEM_MOVEABLE, sizeof(DROPFILES) + totalPathLength * sizeof(wchar_t) + sizeof(wchar_t));
    if(!hGlobal)
    {
        return FALSE;
    }

    DROPFILES *pDropFiles = (DROPFILES *)GlobalLock(hGlobal);
    if(!pDropFiles)
    {
        GlobalFree(hGlobal);
        return FALSE;
    }

    // 初始化 DROPFILES 结构
    pDropFiles->pFiles = sizeof(DROPFILES);
    pDropFiles->pt.x = 0;
    pDropFiles->pt.y = 0;
    pDropFiles->fNC = FALSE;
    pDropFiles->fWide = TRUE;

    // 复制所有选中项目的路径到全局内存
    wchar_t *pPath = (wchar_t *)((BYTE *)pDropFiles + sizeof(DROPFILES));
    for(size_t i = 0; i < filePaths.size(); i++)
    {
        SStringW strPathW = filePaths[i];
        int pathLength = strPathW.GetLength() + 1;
        wcscpy_s(pPath, pathLength, strPathW.c_str());
        pPath += pathLength;
    }
    // 添加最终的空终止符
    *pPath = L'\0';

    GlobalUnlock(hGlobal);
    return TRUE;
}

BOOL CFileOperationManager::StartDrag(HSTREEITEM hItem, const CPoint& pt)
{
    std::vector<HSTREEITEM> selectedItems = GetSelectedItems(hItem);
    if(selectedItems.empty())
    {
        return FALSE;
    }

    HGLOBAL hGlobal = NULL;
    std::vector<SStringW> filePaths;
    if(!CreateDragData(selectedItems, hGlobal, filePaths))
    {
        return FALSE;
    }

    // 创建数据对象和拖放源
    SComPtr<IDataObject> pDataObj(new FileTreeDataSource(hGlobal, TRUE), FALSE);
    SComPtr<FileTreeDropSource> pDropSource(new FileTreeDropSource(), FALSE);
    
    if(!pDataObj || !pDropSource)
    {
        GlobalFree(hGlobal);
        return FALSE;
    }

    HBITMAP hDragBitmap = FileTreeDropSource::CreateDragBitmap(filePaths);
    pDropSource->SetDragImage(hDragBitmap, CPoint(-10, -10));

    // 执行拖放操作
    DWORD dwEffect = DROPEFFECT_MOVE;
    HRESULT hr = ::DoDragDrop(pDataObj, pDropSource, DROPEFFECT_MOVE, &dwEffect);
    SLOGI()<<"DoDragDrop hr="<<hr<<" dwEffect="<<dwEffect;

    // 拖放操作完成后才删除位图
    if (hDragBitmap)
    {
        DeleteObject(hDragBitmap);
    }

    return TRUE;
}

BOOL CFileOperationManager::HasClipboardData()
{
    if (m_pClipboardManager)
    {
        return m_pClipboardManager->HasClipboardData();
    }
    return FALSE;
}