#ifndef _STREE_LIST_H_
#define _STREE_LIST_H_

#include <control/STreeCtrl.h>

SNSBEGIN

    class SMCTreeCtrl : public STreeCtrl
    {
        DEF_SOBJECT(STreeCtrl,L"mctreectrl")

        friend class STreeList;
        struct MCITEM
        {
            SArray<SStringT> arrText;
            LPARAM   lParam;
        };
    public:
        SMCTreeCtrl();
        ~SMCTreeCtrl();
        
        int InsertColumn(int iCol,int nWid);
        BOOL SetColWidth(int iCol,int nWid);
        void SetTreeWidth(int nWid);
        BOOL DeleteColumn(int iCol);
        BOOL SetItemText(HSTREEITEM hItem,int iCol,const SStringT strText);
		CPoint GetViewOrigin() const;
		CSize GetViewSize() const;

		STDMETHOD_(LPARAM, GetItemData)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;
		STDMETHOD_(BOOL, SetItemData)(THIS_ HSTREEITEM hItem, LPARAM lParam) OVERRIDE;

        HSTREEITEM InsertItem(LPCTSTR pszText,int iImage,int iSelImage,LPARAM lParam,HSTREEITEM hParent=STVI_ROOT,HSTREEITEM hAfter = STVI_LAST,BOOL bEnsureVisible=FALSE);
    protected:
        virtual void DrawItem(IRenderTarget *pRT, const CRect & rc, HSTREEITEM hItem);
        virtual void DrawTreeItem(IRenderTarget *pRT, CRect & rc,HSTREEITEM hItem);
        virtual void DrawListItem(IRenderTarget *pRT, CRect & rc,HSTREEITEM hItem);
        
        virtual void OnNodeFree(LPTVITEM & pItemData) OVERRIDE;
        virtual void OnInsertItem(LPTVITEM & pItemData) OVERRIDE;
        virtual int CalcItemWidth(const LPTVITEM pItem) OVERRIDE;
		virtual void UpdateContentWidth() OVERRIDE;

        virtual void WINAPI OnFinalRelease();
    protected:
        SOUI_ATTRS_BEGIN()
            ATTR_INT(L"treeWidth",m_nTreeWidth,FALSE)
        SOUI_ATTRS_END()

        SArray<int> m_arrColWidth;
        int         m_nTreeWidth;
    };
    
    class STreeList : public SWindow
    {
    DEF_SOBJECT(SWindow,L"treelist")
    public:
        STreeList(void);
        ~STreeList(void);
        
        SMCTreeCtrl * GetMCTreeCtrl(){return m_pTreeCtrl;}
        
    protected:
            /**
        * SListCtrl::OnHeaderClick
        * @brief    列表头单击事件 -- 
        * @param    EventArgs *pEvt
        *
        * Describe  列表头单击事件
        */
        BOOL            OnHeaderClick(EventArgs *pEvt);
        /**
        * SListCtrl::OnHeaderSizeChanging
        * @brief    列表头大小改变
        * @param    EventArgs *pEvt -- 
        *
        * Describe  列表头大小改变
        */
        BOOL            OnHeaderSizeChanging(EventArgs *pEvt);
        /**
        * SListCtrl::OnHeaderSwap
        * @brief    列表头交换
        * @param    EventArgs *pEvt -- 
        *
        * Describe  列表头交换
        */
        BOOL            OnHeaderSwap(EventArgs *pEvt);

        BOOL            OnScrollEvent(EventArgs *pEvt);
    protected:
        virtual BOOL CreateChildren(SXmlNode pNode) OVERRIDE;
        virtual BOOL OnRelayout(const CRect &rcWnd) OVERRIDE;
        virtual SHeaderCtrl * CreateHeader();
        virtual SMCTreeCtrl * CreateMcTreeCtrl();

        SOUI_ATTRS_BEGIN()
            ATTR_INT(L"headerHeight",m_nHeaderHeight,FALSE)
            ATTR_I18NSTRT(L"treeLabel",m_strTreeLabel,FALSE)
        SOUI_ATTRS_END()
        
        SHeaderCtrl *m_pHeader;
        int          m_nHeaderHeight;
        STrText      m_strTreeLabel;
        
        SMCTreeCtrl *m_pTreeCtrl;
    };
SNSEND

#endif//_STREE_LIST_H_