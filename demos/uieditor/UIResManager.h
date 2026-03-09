#ifndef _UIRESMANAGER_H_
#define _UIRESMANAGER_H_

#include <vector>
#include <control/STreeCtrl.h>
#include "FileTreeAdapter.h"
#include "ResManger.h"

class CUIResManager
{
public:
    CUIResManager(ResManger* pResManger);
    ~CUIResManager();

    // UI资源操作
    void AddUires(HSTREEITEM hItem, STreeView* pTreeView, CFileTreeAdapter* pAdapter);
    void AddSkin(HSTREEITEM hItem, STreeView* pTreeView, CFileTreeAdapter* pAdapter);
    void AddFilesInDirectoryToUIRes(const SStringT& dirPath);

private:
    ResManger* m_pResManger;

    // 辅助函数
    void AddFileToUIRes(const SStringT& filePath);
};

#endif // _UIRESMANAGER_H_