#ifndef _SFOLDER_LIST_H_
#define _SFOLDER_LIST_H_

#include "STreeList.h"

SNSBEGIN

    class SFolderTreeCtrl : public SMCTreeCtrl
    {
        DEF_SOBJECT(SMCTreeCtrl,L"foldertreectrl")
        
        struct FILEITEMINFO
        {
            BOOL    bFolder;//文件夹标志
            __int64 nSize;  //总文件夹大小
            float   percent;//占上级文件夹的百分比
        };
        
        struct SORTINFO
        {
            enum{
                BYNAME,
                BYSIZE,
            } mode;
            BOOL bAscend;   //升序
        };
        
        struct SORTCTX
        {
            SFolderTreeCtrl *pTree;
            SORTINFO si;
        };
        friend class SFolderTreeList;
    public:
        SFolderTreeCtrl();
        ~SFolderTreeCtrl();
        
        HSTREEITEM InsertItem(LPCTSTR pszFileName,BOOL bFolder,__int64 fileSize,HSTREEITEM hParent);
        void UpdateTreeItemPercent(HSTREEITEM hItem);
        FILEITEMINFO * GetFileInfo(HSTREEITEM hItem);
        void SortChildren(HSTREEITEM hItem,SORTINFO si);
        
    protected:
        static int  SortChildrenCB(void *pCtx, const void *phItem1, const void *phItem2);
        
        virtual void DrawTreeItem(IRenderTarget *pRT, CRect & rc,HSTREEITEM hItem);
        virtual void DrawListItem(IRenderTarget *pRT, CRect & rc,HSTREEITEM hItem);
        virtual void OnNodeFree(LPTVITEM & pItemData) override;
        virtual void OnInsertItem(LPTVITEM & pItemData) override;
        virtual void ItemLayout();

        virtual void WINAPI OnFinalRelease();
        
    protected:
        SOUI_ATTRS_BEGIN()
            ATTR_SKIN(L"percentBkgnd",m_pSkinPerBkgnd,FALSE)
            ATTR_SKIN(L"percentFrgnd",m_pSkinPerFrgnd,FALSE)
        SOUI_ATTRS_END()
        
        ISkinObj    *m_pSkinPerBkgnd,*m_pSkinPerFrgnd;
        HIMAGELIST   m_hIconList;
    };
    
    class SFolderTreeList : public STreeList
    {
    DEF_SOBJECT(STreeList,L"foldertreelist")
    public:
        SFolderTreeList(void);
        ~SFolderTreeList(void);
                
        SFolderTreeCtrl * GetFolderTreeCtrl() { return m_pFolderTreectrl;}
    protected:
            /**
        * SListCtrl::OnHeaderClick
        * @brief    列表头单击事件 -- 
        * @param    EventArgs *pEvt
        *
        * Describe  列表头单击事件
        */
        BOOL            OnHeaderClick(EventArgs *pEvt);

    protected:
        virtual BOOL CreateChildren(SXmlNode xmlNode) OVERRIDE;
        virtual SMCTreeCtrl * CreateMcTreeCtrl();

        SFolderTreeCtrl::SORTINFO m_si;
        SFolderTreeCtrl * m_pFolderTreectrl;
    };
SNSEND

#endif//_SFOLDER_LIST_H_