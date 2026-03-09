#ifndef _FILEOPERATIONMANAGER_H_
#define _FILEOPERATIONMANAGER_H_

#include <vector>
#include <control/STreeCtrl.h>
#include "FileTreeAdapter.h"
#include "ClipboardManager.h"

class CFileOperationManager
{
public:
    CFileOperationManager(STreeView* pTreeView, CFileTreeAdapter* pAdapter, HWND hWnd);
    ~CFileOperationManager();

    // 文件操作
    void OnFileCopy();
    void OnFileCut();
    void OnFilePaste();
    void OnFilePaste(HSTREEITEM hItem);
    void OnFileDelete(HSTREEITEM hItem);
    void OnFileRename(HSTREEITEM hItem);

    // 辅助函数
    std::vector<HSTREEITEM> GetSelectedItems(HSTREEITEM hDefaultItem = ITEM_NULL);
    BOOL HandleTreeViewKeyboardShortcut(UINT nChar);

    // 拖拽相关
    BOOL StartDrag(HSTREEITEM hItem, const CPoint& pt);

    // 剪贴板相关
    BOOL HasClipboardData();

private:
    STreeView* m_pTreeView;
    CFileTreeAdapter* m_pFileTreeAdapter;
    CClipboardManager* m_pClipboardManager;

    // 拖拽相关
    BOOL CreateDragData(const std::vector<HSTREEITEM>& selectedItems, HGLOBAL& hGlobal, std::vector<SStringW>& filePaths);
};

#endif // _FILEOPERATIONMANAGER_H_