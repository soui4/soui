#ifndef __FILETREEADAPTER_H__
#define __FILETREEADAPTER_H__

#include <helper/SAdapterBase.h>
#include "ResManger.h"

SNSBEGIN

// 文件项数据
struct FileItemData
{
    SStringT strPath;   // 文件/目录路径
    SStringT strName;   // 文件/目录名称
    BOOL     bIsDir;    // 是否为目录
    int      nChilds;   // 子项数量
    int      nFileType; // 文件类型
    BOOL     bIsCut;    // 是否处于剪切状态
};

class CFileTreeAdapter : public STreeAdapterBase<FileItemData>
{
public:
    CFileTreeAdapter(ResManger * pResManger);
    ~CFileTreeAdapter();

    void Clear();
    // 设置根目录
    void SetRootPath(const SStringT& strRootPath);
    
    // 枚举目录
    void EnumDirectory(const SStringT& strPath, HSTREEITEM hParentItem);
    
    // 实现getView方法
    STDMETHOD_(void, getView)(THIS_ HSTREEITEM hItem, SItemPanel *pItem, SXmlNode xmlTemplate) OVERRIDE;
    
    // 展开项时的处理
    STDMETHOD_(void, ExpandItem)(HSTREEITEM hItem, UINT uCode) OVERRIDE;

    STDMETHOD_(BOOL, HasChildren)(HSTREEITEM hItem) const OVERRIDE;
    
    	// 辅助函数
	static SStringT GenerateAvailableName(const SStringT& strDir, const SStringT& strPrefix, const SStringT& strExt);

    // 文件管理功能
    // 创建文件或目录
    BOOL CreateItem(HSTREEITEM hParentItem, const SStringT& strName, BOOL bIsDir);
    
    // 复制文件或目录
    BOOL CopyItem(HSTREEITEM hSrcItem, HSTREEITEM hDestParentItem, const SStringT& strNewName = _T(""));
    
    // 删除文件或目录
    BOOL RemoveItem(HSTREEITEM hItem);
    
    // 移动文件或目录
    BOOL MoveItem(HSTREEITEM hSrcItem, HSTREEITEM hDestParentItem, const SStringT& strNewName = _T(""));
    
    // 刷新目录
    void RefreshDirectory(HSTREEITEM hItem);
    
    // 根据路径查找树项
    HSTREEITEM FindItemByPath(const SStringT& strPath);
    
    // 根据路径删除树项
    BOOL RemoveItemByPath(const SStringT& strPath);
    
    // 根据路径添加树项
    BOOL AddItemByPath(const SStringT& strPath);
    
    // 剪切状态管理
    void SetItemCutState(HSTREEITEM hItem, BOOL bIsCut);
    void ClearAllCutStates();
    BOOL IsItemCut(HSTREEITEM hItem);
    
    // 辅助方法
    void GetAllItems(std::vector<HSTREEITEM>& items, HSTREEITEM hParentItem);
private:
    SStringT m_strRootPath; // 根目录路径
    BOOL     m_bRootInited; // 根目录是否已初始化
    ResManger * m_resManger; // 资源管理器对象指针
    // 辅助函数
    BOOL CreateDirectoryRecursive(const SStringT& strPath);
    BOOL CopyDirectory(const SStringT& strSrcPath, const SStringT& strDestPath);
    BOOL DeleteDirectory(const SStringT& strPath);
};

SNSEND

#endif // __FILETREEADAPTER_H__
