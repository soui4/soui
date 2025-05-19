#include "stdafx.h"
#include "SFolderList.h"
#include <gdialpha.h>
#include <shellapi.h>
#pragma comment(lib,"comctl32.lib")

namespace SOUI
{
    SFolderTreeCtrl::SFolderTreeCtrl()
    :m_pSkinPerBkgnd(GETBUILTINSKIN(SKIN_SYS_PROG_BKGND))
    ,m_pSkinPerFrgnd(GETBUILTINSKIN(SKIN_SYS_PROG_BAR))
    ,m_hIconList(NULL)
    {
        #ifdef _WIN32
        SHFILEINFO    sfi={0};
        m_hIconList = (HIMAGELIST)SHGetFileInfo(_T(""),
            0,
            &sfi,
            sizeof(SHFILEINFO),
            SHGFI_SYSICONINDEX | SHGFI_SMALLICON ) ;
        #endif
    }

    SFolderTreeCtrl::~SFolderTreeCtrl()
    {
        #ifdef _WIN32
        ImageList_Destroy(m_hIconList);
        #endif//
    }

    void SFolderTreeCtrl::OnFinalRelease()
    {
        delete this;
    }

    void SFolderTreeCtrl::OnNodeFree(LPTVITEM & pItemData)
    {
        MCITEM *pMcItem = (MCITEM*)pItemData->lParam;
        delete (FILEITEMINFO*)pMcItem->lParam;
        delete pMcItem;
        STreeCtrl::OnNodeFree(pItemData);
    }

    //copy from treectrl.cpp
    void SFolderTreeCtrl::ItemLayout()
    {
        int nOffset = 0;
        CSize sizeSkin;

        m_uItemMask = 0;
        m_rcToggle.SetRect(0, 0, 0, 0);
        m_rcCheckBox.SetRect(0, 0, 0, 0);
        m_rcIcon.SetRect(0, 0, 0, 0);

        //计算位置    
        if (m_pToggleSkin)
        {
            m_uItemMask |= STVIMask_Toggle;
            sizeSkin = m_pToggleSkin->GetSkinSize();        
            m_rcToggle.SetRect( 
                nOffset, 
                (m_nItemHei - sizeSkin.cy) / 2,
                nOffset + sizeSkin.cx,
                m_nItemHei - (m_nItemHei - sizeSkin.cy) / 2);
            nOffset += sizeSkin.cx;
        }

        if (m_bCheckBox && m_pCheckSkin)
        {
            m_uItemMask |= STVIMask_CheckBox;
            sizeSkin = m_pCheckSkin->GetSkinSize();    
            m_rcCheckBox.SetRect( 
                nOffset, 
                (m_nItemHei - sizeSkin.cy) / 2,
                nOffset + sizeSkin.cx,
                m_nItemHei - (m_nItemHei - sizeSkin.cy) / 2);
            nOffset += sizeSkin.cx;
        }

        if (m_hIconList)
        {
            m_uItemMask |= STVIMask_Icon;
            sizeSkin = CSize(::GetSystemMetrics(SM_CXSMICON),::GetSystemMetrics(SM_CYSMICON));
            m_rcIcon.SetRect( 
                nOffset, 
                (m_nItemHei - sizeSkin.cy) / 2,
                nOffset + sizeSkin.cx,
                m_nItemHei - (m_nItemHei - sizeSkin.cy) / 2);
            nOffset += sizeSkin.cx;
        }

        m_nItemOffset = nOffset;
    }
    void SFolderTreeCtrl::DrawTreeItem(IRenderTarget *pRT, CRect & rc, HSTREEITEM hItem)
    {    
        BOOL     bTextColorChanged = FALSE;;
        COLORREF crOldText=RGBA(0xFF,0xFF,0xFF,0xFF);
        CRect rcItemBg;
        LPTVITEM pItem=CSTree<LPTVITEM>::GetItem(hItem);

        pRT->OffsetViewportOrg(rc.left + pItem->nLevel * m_nIndent,rc.top);

        rcItemBg.SetRect( m_nItemOffset + m_nItemMargin, 0, pItem->nContentWidth, m_nItemHei);
        if(rcItemBg.right > rc.Width() - pItem->nLevel * m_nIndent)
            rcItemBg.right = rc.Width() - pItem->nLevel * m_nIndent;
        //绘制背景
        if (hItem == m_hSelItem)
        {
            if (m_pItemSelSkin != NULL)
                m_pItemSelSkin->DrawByIndex(pRT, rcItemBg, 0);
            else if (CR_INVALID != m_crItemSelBg)
                pRT->FillSolidRect(rcItemBg, m_crItemSelBg);

            if (CR_INVALID != m_crItemSelText)
            {
                bTextColorChanged = TRUE;
                crOldText = pRT->SetTextColor(m_crItemSelText);
            }
        }
        else
        {
            if (CR_INVALID != m_crItemText)
            {
                bTextColorChanged = TRUE;
                crOldText = pRT->SetTextColor(m_crItemText);
            }
        }

        if (pItem->bHasChildren &&
            STVIMask_Toggle == (m_uItemMask & STVIMask_Toggle))
        {
            int nImage = IIF_STATE3(pItem->dwToggleState, 0, 1, 2);
            if (!pItem->bCollapsed) nImage += 3;
            m_pToggleSkin->DrawByIndex(pRT, m_rcToggle, nImage);
        }

        if (STVIMask_CheckBox == (m_uItemMask & STVIMask_CheckBox))
        {
            int nImage = IIF_STATE3(pItem->dwCheckBoxState, 0, 1, 2);
            if (pItem->nCheckBoxValue == STVICheckBox_Checked)
                nImage += 3;
            else if (pItem->nCheckBoxValue == STVICheckBox_PartChecked)
                nImage += 6;
            m_pCheckSkin->DrawByIndex(pRT, m_rcCheckBox, nImage);
        }

        #ifdef _WIN32
        if (STVIMask_Icon == (m_uItemMask & STVIMask_Icon) &&
            (pItem->nSelectedImage != -1 || pItem->nImage != -1))
        {
            int iImage = (pItem->nSelectedImage != -1 && hItem == m_hSelItem)?pItem->nSelectedImage:pItem->nImage;
            HDC hdc = pRT->GetDC(0);
            ALPHAINFO ai;
            CGdiAlpha::AlphaBackup(hdc,m_rcIcon,ai); 
            ImageList_Draw(m_hIconList,iImage,hdc,m_rcIcon.left,m_rcIcon.top,0);
            CGdiAlpha::AlphaRestore(ai);
            pRT->ReleaseDC(hdc);
        }
        #endif//_WIN32

        UINT align=DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS;
        rcItemBg.OffsetRect(m_nItemMargin, 0);
        pRT->DrawText(pItem->strText,-1,rcItemBg,align);    

        if (bTextColorChanged)
            pRT->SetTextColor(crOldText);

        pRT->OffsetViewportOrg(-(rc.left  + pItem->nLevel * m_nIndent),-rc.top);
    }

    
    void SFolderTreeCtrl::OnInsertItem( LPTVITEM & pItemData )
    {
        HSTREEITEM hItem = pItemData->hItem;
        FILEITEMINFO * pFi = GetFileInfo(hItem);
        
        //update parent item size
        HSTREEITEM hParent = GetParentItem(hItem);
        while(hParent)
        {
            FILEITEMINFO *pFiP = GetFileInfo(hParent);
            pFiP->nSize += pFi->nSize;
            hParent = GetParentItem(hParent);
        }
    }

    SStringT Size2String(__int64 nSize)
    {
        SStringT str;
        if((nSize>>40)!=0)
        {
            __int64 nUnit = (((__int64)1)<<40);
            __int64 nHei = nSize>>40;
            __int64 nLow = nSize - (nHei<<40);
                      
            __int64 nLowPer = nLow*100/nUnit;
            str.Format(_T("%d.%d T"),(int)nHei,(int)nLowPer);
        }else if((nSize>>30)!=0)
        {
            __int64 nUnit = (1<<30);
            __int64 nHei = nSize>>30;
            __int64 nLow = nSize - (nHei<<30);

            __int64 nLowPer = nLow*100/nUnit;
            str.Format(_T("%d.%d G"),(int)nHei,(int)nLowPer);
        }else if((nSize>>20)!=0)
        {
            __int64 nUnit = (1<<20);
            __int64 nHei = nSize>>20;
            __int64 nLow = nSize - (nHei<<20);

            __int64 nLowPer = nLow*100/nUnit;
            str.Format(_T("%d.%d M"),(int)nHei,(int)nLowPer);
        }else if((nSize>>10)!=0)
        {
            __int64 nUnit = (1<<10);
            __int64 nHei = nSize>>10;
            __int64 nLow = nSize - (nHei<<10);
            __int64 nLowPer = nLow*100/nUnit;
            str.Format(_T("%d.%d K"),(int)nHei,(int)nLowPer);
        }else
        {
            str.Format(_T("%d B"),(int)nSize);
        }
        return str;
    }
    
    void SFolderTreeCtrl::DrawListItem(IRenderTarget *pRT, CRect & rc,HSTREEITEM hItem)
    {
        FILEITEMINFO *pFi = GetFileInfo(hItem);
        CRect rcItem = rc;
        rcItem.right = rcItem.left + m_arrColWidth[0];
        SStringT strTxt = Size2String(pFi->nSize);        
        pRT->DrawText(strTxt,strTxt.GetLength(),&rcItem,DT_SINGLELINE|DT_VCENTER);

        rcItem.left =rcItem.right;
        rcItem.right = rcItem.left + m_arrColWidth[1];
        
        rcItem.DeflateRect(0,2,0,2);
        m_pSkinPerBkgnd->DrawByIndex(pRT,rcItem,0);
        if(pFi->percent<0.0)
        {
            strTxt = _T("...");
            pRT->DrawText(strTxt,strTxt.GetLength(),&rcItem,DT_SINGLELINE|DT_VCENTER|DT_CENTER);        
        }else
        {
            CRect rcPercent = rcItem;
            rcPercent.right = rcItem.left + rcItem.Width()*pFi->percent/100;
            m_pSkinPerFrgnd->DrawByIndex(pRT,rcPercent,0);
            strTxt.Format(_T("%.1f%%"),pFi->percent);
            pRT->DrawText(strTxt,strTxt.GetLength(),&rcItem,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
        }
    }

    HSTREEITEM SFolderTreeCtrl::InsertItem( LPCTSTR pszFileName,BOOL bFolder,__int64 fileSize,HSTREEITEM hParent )
    {
        FILEITEMINFO *pFi = new FILEITEMINFO;
        pFi->nSize = fileSize;
        pFi->percent=-1;
        pFi->bFolder = bFolder;
        #ifdef _WIN32
        SHFILEINFO sfi;
        SHGetFileInfo(pszFileName,bFolder?FILE_ATTRIBUTE_DIRECTORY:FILE_ATTRIBUTE_NORMAL,&sfi,sizeof(sfi),SHGFI_USEFILEATTRIBUTES|SHGFI_SYSICONINDEX);
        return SMCTreeCtrl::InsertItem(pszFileName,sfi.iIcon,sfi.iIcon,(LPARAM)pFi,hParent);
        #else
        return SMCTreeCtrl::InsertItem(pszFileName,0,0,(LPARAM)pFi,hParent);
        #endif//_WIN32
    }

    SFolderTreeCtrl::FILEITEMINFO * SFolderTreeCtrl::GetFileInfo( HSTREEITEM hItem )
    {
        return (FILEITEMINFO*)SMCTreeCtrl::GetItemData(hItem);
    }

    void SFolderTreeCtrl::UpdateTreeItemPercent(HSTREEITEM hItem)
    {
        FILEITEMINFO * pFi = GetFileInfo(hItem);
        HSTREEITEM hChild = GetChildItem(hItem);
        while(hChild)
        {
            FILEITEMINFO *pFi2 = GetFileInfo(hChild);
            pFi2->percent = ((float)pFi2->nSize)*100.0f/pFi->nSize;
            if(pFi2->bFolder)
            {
                UpdateTreeItemPercent(hChild);
            }
            hChild = GetNextSiblingItem(hChild);
        }
    }
    
    int SFolderTreeCtrl::SortChildrenCB(void* pCtx,const void *phItem1,const void *phItem2)
    {
        SORTCTX *pSortCtx = (SORTCTX*)pCtx;
        HSTREEITEM hItem1 = *(HSTREEITEM*)phItem1;
        HSTREEITEM hItem2 = *(HSTREEITEM*)phItem2;
        
        int nRet =0;
        if(pSortCtx->si.mode == SORTINFO::BYNAME)
        {
            SStringT str1,str2;
            pSortCtx->pTree->GetItemText(hItem1,str1);
            pSortCtx->pTree->GetItemText(hItem2,str2);
            nRet = str1.Compare(str2);
        }else
        {
            FILEITEMINFO *pfi1 = pSortCtx->pTree->GetFileInfo(hItem1);
            FILEITEMINFO *pfi2 = pSortCtx->pTree->GetFileInfo(hItem2);
            nRet = (int) (pfi1->nSize - pfi2->nSize);
        }
        if(!pSortCtx->si.bAscend)
        {
            nRet *= -1;
        }
        return nRet;
    }

    void SFolderTreeCtrl::SortChildren(HSTREEITEM hItem,SORTINFO si)
    {
        SORTCTX ctx = {this,si};
        STreeCtrl::SortChildren(hItem,SortChildrenCB,&ctx);
        Invalidate();
    }



    //////////////////////////////////////////////////////////////////////////
    SFolderTreeList::SFolderTreeList(void):m_pFolderTreectrl(NULL)
    {
        m_si.mode = SFolderTreeCtrl::SORTINFO::BYNAME;
        m_si.bAscend = FALSE;
    }

    SFolderTreeList::~SFolderTreeList(void)
    {
    }

    BOOL SFolderTreeList::CreateChildren(SXmlNode pNode)
    {
        BOOL bRet = STreeList::CreateChildren(pNode);
        if(!bRet)
            return FALSE;
        
        m_pFolderTreectrl = sobj_cast<SFolderTreeCtrl>(GetMCTreeCtrl());
        SASSERT(m_pFolderTreectrl);
        SASSERT(m_pHeader->GetItemCount() == 3);
        
        m_pHeader->GetEventSet()->subscribeEvent(EventHeaderClick::EventID,Subscriber(&SFolderTreeList::OnHeaderClick,this));
        return TRUE;
    }

    BOOL SFolderTreeList::OnHeaderClick(EventArgs *pEvt)
    {
        EventHeaderClick *pEvt2 = sobj_cast<EventHeaderClick>(pEvt);
        
        SFolderTreeCtrl::SORTINFO si;
        si.mode = pEvt2->iItem==0?SFolderTreeCtrl::SORTINFO::BYNAME:SFolderTreeCtrl::SORTINFO::BYSIZE;
        if(si.mode != m_si.mode)
        {
            m_si.mode = si.mode;
            m_si.bAscend = FALSE;
        }
        else
        {
            m_si.bAscend = !m_si.bAscend;
        }
        
        m_pFolderTreectrl->SortChildren(STVI_ROOT,m_si);
        return true;
    }

    SMCTreeCtrl * SFolderTreeList::CreateMcTreeCtrl()
    {
        return new SFolderTreeCtrl;
    }
}
