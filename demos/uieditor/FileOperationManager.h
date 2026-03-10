#ifndef _FILEOPERATIONMANAGER_H_
#define _FILEOPERATIONMANAGER_H_

#include <vector>
#include <control/STreeCtrl.h>
#include "FileTreeAdapter.h"
#include "ClipboardManager.h"
class CMainDlg;
class CFileOperationManager
{
public:
    CFileOperationManager( CMainDlg *pMainDlg, STreeView* pTreeView, CFileTreeAdapter* pAdapter, HWND hWnd);
    ~CFileOperationManager();

    // 文件操作
    void OnFileCopy();
    void OnFileCut();
    void OnFilePaste();
    void OnFilePaste(HSTREEITEM hItem,BOOL bCopy=TRUE, HGLOBAL hdrop=NULL);
    void OnFileDelete(HSTREEITEM hItem);
    void OnFileRename(HSTREEITEM hItem);

    // 辅助函数
    std::vector<HSTREEITEM> GetSelectedItems(HSTREEITEM hDefaultItem = ITEM_NULL);
    BOOL HandleTreeViewKeyboardShortcut(UINT nChar);

    // 拖拽相关
    BOOL StartDrag(HSTREEITEM hItem, const CPoint& pt);

    // 剪贴板相关
    BOOL HasClipboardData();

    CClipboardManager * GetClipboardManager(){return m_pClipboardManager;}
private:
    STreeView* m_pTreeView;
    CFileTreeAdapter* m_pFileTreeAdapter;
    CClipboardManager* m_pClipboardManager;
    CMainDlg *m_pMainDlg;

    // 拖拽相关
    BOOL CreateDragData(const std::vector<HSTREEITEM>& selectedItems, HGLOBAL& hGlobal, std::vector<SStringW>& filePaths);
};

#endif // _FILEOPERATIONMANAGER_H_