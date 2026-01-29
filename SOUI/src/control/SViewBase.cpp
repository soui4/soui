#include "control/SViewBase.h"

SNSBEGIN

SViewBase::SViewBase(SWindow *pHost)
    : SHostProxy(pHost)
    , m_bPendingUpdate(false)
    , m_iPendingUpdateItem(-2)
    , m_iPendingViewItem(-1)
    , m_iFirstVisible(0)
    , m_itemCapture(NULL)
    , m_iSelItem(-1)
    , m_pHoverItem(NULL)
    , m_bDatasetInvalidated(FALSE)
    , m_bWantTab(FALSE)
    , m_bMultiSel(FALSE) {
}

SViewBase::~SViewBase() {
    // Clean up item recycle
    for (int i = 0; i < m_itemRecycle.GetCount(); i++) {
        SList<SItemPanel *> *pList = m_itemRecycle[i];
        if (pList) {
            while (!pList->IsEmpty()) {
                SItemPanel *pItem = pList->RemoveHead();
                if (pItem) {
                    pItem->Destroy();
                    delete pItem;
                }
            }
            delete pList;
        }
    }
    m_itemRecycle.RemoveAll();
}

void SViewBase::OnItemSetCapture(SOsrPanel *pItem, BOOL bCapture) {
    if (bCapture) {
        m_itemCapture = pItem;
    } else {
        m_itemCapture = NULL;
    }
}

BOOL SViewBase::OnItemGetRect(const SOsrPanel *pItem, CRect &rcItem) const {
    if (!pItem) return FALSE;
    rcItem = pItem->GetWindowRect();
    return TRUE;
}

BOOL SViewBase::IsItemRedrawDelay() const {
    return FALSE;
}

BOOL SViewBase::IsTimelineEnabled() const {
    return FALSE;
}

void SViewBase::onDataSetChanged() {
    m_bPendingUpdate = true;
    m_iPendingUpdateItem = -1;
    m_pHost->Invalidate();
}

void SViewBase::onDataSetInvalidated() {
    m_bPendingUpdate = true;
    m_iPendingUpdateItem = -1;
    m_bDatasetInvalidated = TRUE;
    m_pHost->Invalidate();
}

void SViewBase::onItemDataChanged(int iItem) {
    m_bPendingUpdate = true;
    m_iPendingUpdateItem = iItem;
    m_pHost->Invalidate();
}

void SViewBase::DispatchMessage2Items(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    for (SPOSITION  it = m_lstItems.GetHeadPosition(); it;) {
        ItemInfo &itInfo = m_lstItems.GetNext(it);
        itInfo.pItem->SDispatchMessage(uMsg, wParam, lParam);
    }
}

void SViewBase::SetMultiSel(BOOL bMultiSel) {
    m_bMultiSel = bMultiSel;
    if (!bMultiSel) {
        // Clear all selections except the current one
        ClearSelItems();
        if (m_iSelItem != -1) {
            AddSelItem(m_iSelItem);
        }
    }
}

BOOL SViewBase::GetMultiSel() const {
    return m_bMultiSel;
}

void SViewBase::AddSelItem(int iItem) {
    if (iItem < 0) return;
    
    m_mapSelItems[iItem] = TRUE;
    
    // Update the visible item's state
    SItemPanel *pItem = GetItemPanel(iItem);
    if (pItem) {
        pItem->ModifyItemState(WndState_Check, 0);
        RedrawItem(pItem);
    }
}

void SViewBase::RemoveSelItem(int iItem) {
    if (iItem < 0) return;
    
    m_mapSelItems.RemoveKey(iItem);
    
    // Update the visible item's state
    SItemPanel *pItem = GetItemPanel(iItem);
    if (pItem) {
        pItem->ModifyItemState(0, WndState_Check);
        RedrawItem(pItem);
    }
}

void SViewBase::ClearSelItems() {
    // Update all visible items' states
    for (SPOSITION pos = m_mapSelItems.GetStartPosition(); pos;) {
        int iItem;
        BOOL bVal;
        m_mapSelItems.GetNextAssoc(pos, iItem, bVal);
        
        SItemPanel *pItem = GetItemPanel(iItem);
        if (pItem) {
            pItem->ModifyItemState(0, WndState_Check);
            RedrawItem(pItem);
        }
    }
    
    m_mapSelItems.RemoveAll();
}

BOOL SViewBase::IsItemSelected(int iItem) const {
    if (iItem < 0) return FALSE;
    if(m_bMultiSel){
        const ItemSelectionMap::CPair *pVal = m_mapSelItems.Lookup(iItem);
        return pVal && pVal->m_value;
    }else{
        // 单选模式下，m_iSelItem保存了选中的项
        return m_iSelItem == iItem;
    }
}

int SViewBase::GetSelItemCount() const {
    if (m_bMultiSel) {
        return m_mapSelItems.GetCount();
    } else {
        // 单选模式下，如果有选中项，返回1，否则返回0
        return m_iSelItem != -1 ? 1 : 0;
    }
}

int SViewBase::GetSelItems(int *pItems, int nMaxCount) const {
    if (m_bMultiSel) {
        // 多选模式下，返回m_mapSelItems中的所有项
        int i=0;
        for (SPOSITION pos = m_mapSelItems.GetStartPosition(); pos && nMaxCount > 0;) {
            int iItem;
            BOOL bVal;
            m_mapSelItems.GetNextAssoc(pos, iItem, bVal);
            pItems[i++] = iItem;
            nMaxCount--;
        }
        return i;
    } else {
        // 单选模式下，如果有选中项，返回该项
        if (m_iSelItem != -1 && nMaxCount > 0) {
            pItems[0] = m_iSelItem;
            return 1;
        } else {
            return 0;
        }
    }
}

void SViewBase::HandleSelectionChange(int nOldSel, int nNewSel) {
    if (nOldSel == nNewSel) {
        return;
    }

    // Clear old selection if in multi-select mode
    if (m_bMultiSel) {
        ClearSelItems();
    }

    // Update old selected item
    SItemPanel *pItem = GetItemPanel(nOldSel);
    if (pItem) {
        pItem->GetFocusManager()->ClearFocus();
        pItem->ModifyItemState(0, WndState_Check);
        RedrawItem(pItem);
    }

    // Update new selected item
    m_iSelItem = nNewSel;
    pItem = GetItemPanel(nNewSel);
    if (pItem) {
        pItem->ModifyItemState(WndState_Check, 0);
        RedrawItem(pItem);
    }

    // Add new selection if in multi-select mode
    if (m_bMultiSel && nNewSel != -1) {
        AddSelItem(nNewSel);
    }
}

SNSEND
