#ifndef _CLIPBOARDMANAGER_H_
#define _CLIPBOARDMANAGER_H_

#include <vector>
#include <control/STreeCtrl.h>
#include "FileTreeAdapter.h"

class CClipboardManager
{
public:
    CClipboardManager(HWND hWnd);
    ~CClipboardManager();

    // 剪贴板操作
    void SerializeItemsToClipboard(const std::vector<HSTREEITEM>& items, int nOperation, CFileTreeAdapter* pAdapter);
    BOOL DeserializeItemsFromClipboard(std::vector<SStringT>& vecItemPaths, int& nOperation);
    BOOL HasClipboardData();

    // 常量定义
    static const UINT GetClipboardFormat();

private:
    HWND m_hWnd;
    static UINT m_uClipboardFormat;

    // 辅助函数
    void InitializeClipboardFormat();
};

#endif // _CLIPBOARDMANAGER_H_