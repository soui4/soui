#ifndef __SVIEWBASE__H__
#define __SVIEWBASE__H__

#include <core/SPanel.h>
#include <core/SItemPanel.h>
#include <interface/SAdapter-i.h>
#include <proxy/SPanelProxy.h>

SNSBEGIN

/**
 * @class SViewBase
 * @brief Base class for view controls that use adapters
 * @details This class provides common functionality for view controls that use adapters,
 *          such as SListView, STileView, and SMcListView.
 *          Drag scroll and fling animation are handled by SPanel.
 */
class SOUI_EXP SViewBase
    : protected SHostProxy
    , protected IItemContainer {

  public:
    /**
     * @brief Constructor
     */
    SViewBase(SPanel *pView);

    /**
     * @brief Destructor
     */
    ~SViewBase();

  protected:
    /**
     * @brief Handle item capture
     * @param pItem Pointer to the item panel
     * @param bCapture Capture flag
     */
    virtual void OnItemSetCapture(SOsrPanel *pItem, BOOL bCapture);

    /**
     * @brief Get the rectangle of an item
     * @param pItem Pointer to the item panel
     * @param rcItem Output rectangle
     * @return TRUE if successful, FALSE otherwise
     */
    virtual BOOL OnItemGetRect(const SOsrPanel *pItem, CRect &rcItem) const;

    /**
     * @brief Check if item redraw is delayed
     * @return TRUE if redraw is delayed, FALSE otherwise
     */
    virtual BOOL IsItemRedrawDelay() const;

    virtual BOOL IsTimelineEnabled() const;

    /**
     * @brief Redraw a specific item
     * @param pItem Pointer to the item panel
     */
    virtual void RedrawItem(SOsrPanel *pItem) = 0;

    virtual ILvAdapter *getAdapter() = 0;

  public:
    /**
     * @brief Handle item click event
     * @param pEvt Event arguments
     * @return TRUE if handled, FALSE otherwise
     */
    BOOL OnItemClick(IEvtArgs *pEvt);

    /**
     * @brief Handle item click up event
     * @param pEvt Event arguments
     * @return TRUE if handled, FALSE otherwise
     */
    BOOL OnItemClickUp(IEvtArgs *pEvt);

  protected:
    /**
     * @brief Handle data set changed event
     */
    virtual void onDataSetChanged();

    /**
     * @brief Handle data set invalidated event
     */
    virtual void onDataSetInvalidated();

    /**
     * @brief Handle item data changed event
     * @param iItem Index of the item
     */
    virtual void onItemDataChanged(int iItem);

  protected:
    /**
     * @brief Dispatch messages to items
     * @param uMsg Message identifier
     * @param wParam Additional message-specific information
     * @param lParam Additional message-specific information
     */
    void DispatchMessage2Items(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Update visible items
     */
    virtual void UpdateVisibleItems() = 0;

    /**
     * @brief Update a specific visible item
     * @param iItem Index of the item
     */
    virtual void UpdateVisibleItem(int iItem) = 0;

    /**
     * @brief Sets the multiple selection mode.
     * @param bMultiSel TRUE to enable multiple selection, FALSE otherwise.
     */
    void SetMultiSel(BOOL bMultiSel);

    /**
     * @brief Gets the multiple selection mode.
     * @return TRUE if multiple selection is enabled, FALSE otherwise.
     */
    BOOL GetMultiSel() const;

    /**
     * @brief Sets the selected item.
     * @param iItem Index of the item.
     * @note Virtual: derived controls (SListView/SMCListView/STileView)
     *       override this with their COM-interface SetSel. Item clicks and
     *       keyboard navigation therefore share ONE implementation path -
     *       do not reintroduce a second, non-virtual variant.
     */
    void SelectItem(int iItem, BOOL bNotify);

    /**
     * @brief Selects the contiguous range between two items (Explorer-style
     *        anchored shift-selection): the set becomes exactly
     *        [min..max](anchor, cursor), items outside are deselected and
     *        items inside are selected, each flip firing its per-item event.
     *        Multi-selection mode only.
     * @param iAnchor Selection anchor (fixed base of the range).
     * @param iCursor Keyboard cursor / clicked item (moving end).
     */
    void SetSelRange(int iAnchor, int iCursor);

    /**
     * @brief Adds an item to the selection.
     * @param iItem Index of the item.
     */
    void AddSelItem(int iItem);

    /**
     * @brief Removes an item from the selection.
     * @param iItem Index of the item.
     */
    void RemoveSelItem(int iItem);

    /**
     * @brief Clears all selected items.
     */
    void ClearSelItems();

    /**
     * @brief Prunes the selection after the dataset changes size.
     * @param nNewCount New number of items in the dataset.
     * @details Drops the cursor, the multi-selection anchor and any selected
     *          entry that falls beyond the new item count. The anchor can be
     *          independent of the cursor (a stable base for Shift ranges), so
     *          it is normalized here instead of relying on the cursor check.
     *          Removed rows no longer exist, so they are deselected silently
     *          (no per-item event), matching the tree views' pruning rule.
     */
    void PruneSelItems(int nNewCount);

    /**
     * @brief Checks if an item is selected.
     * @param iItem Index of the item.
     * @return TRUE if the item is selected, FALSE otherwise.
     */
    BOOL IsItemSelected(int iItem) const;

    /**
     * @brief Gets the count of selected items.
     * @return Number of selected items.
     */
    int GetSelItemCount() const;

    /**
     * @brief Gets all selected items.
     * @param pItems Output parameter to store the selected item indices.
     * @param nMaxCount Maximum number of items to retrieve.
     * @return Number of selected items.
     */
    int GetSelItems(int *pItems, int nMaxCount) const;

    /**
     * @brief Takes a snapshot of the current selection (for rubber band cancel).
     */
    void SnapshotSelItems();

    /**
     * @brief Restores the selection from the snapshot taken by SnapshotSelItems.
     */
    void RestoreSelItems();

    /**
     * @brief Handles the core selection logic.
     * @param nOldSel Index of the previously selected item.
     * @param nNewSel Index of the newly selected item.
     */
    void HandleSelectionChange(int nOldSel, int nNewSel);

  public:
    /**
     * @brief Get the item panel for a specific item
     * @param iItem Index of the item
     * @return Pointer to the item panel
     */
    virtual SItemPanel *GetItemPanel(int iItem) = 0;

    /**
     * @brief Number of Accessible children (currently visible items).
     * @return Current visible SItemPanel count
     */
    int GetVisibleAccChildCount() const
    {
        return static_cast<int>(m_lstItems.GetCount());
    }

    /**
     * @brief Get the currently visible SItemPanel as the Accessible child by 1-based index.
     * @param iChild Index (1-based)
     * @return Corresponding SItemPanel (SWindow*), returns NULL if out of range
     */
    SWindow *GetVisibleAccChild(int iChild) const;

    /**
     * @brief Compute the visible SItemPanel index (1-based, Accessible child index) for the currently selected item.
     * @return Selected item index (1-based), returns 0 if not matched
     */
    int GetVisibleAccSelIndex() const;

    SOUI_ATTRS_BEGIN()
        ATTR_BOOL(L"wantTab", m_bWantTab, FALSE)
        ATTR_BOOL(L"multiSel", m_bMultiSel, FALSE)
    SOUI_ATTRS_BREAK()
  protected:
    /**
     * @struct ItemInfo
     * @brief Information about a visible item
     */
    struct ItemInfo
    {
        SItemPanel *pItem; /**< Pointer to the item panel */
        int nType;         /**< Type of the item */
    };

  protected:
    bool m_bPendingUpdate;    /**< Flag indicating pending update */
    int m_iPendingUpdateItem; /**< Index of the item to update, -1 for all, -2 for nothing */
    int m_iPendingViewItem;   /**< Index of the item to view, -1 for init */

    int m_iFirstVisible;        /**< Index of the first visible item */
    SList<ItemInfo> m_lstItems; /**< List of currently visible items */
    SOsrPanel *m_itemCapture;   /**< Item panel that has been set capture */

    int m_iSelItem;             /**< Index of the selected item (single) / keyboard cursor (multi) */
    int m_iSelAnchor;           /**< Multi-selection range anchor: fixed base for Shift ranges; follows the cursor on every non-Shift change */
    SOsrPanel *m_pHoverItem;    /**< Item panel under the mouse */
    BOOL m_bDatasetInvalidated; /**< Flag indicating data set is invalidated */

    SArray<SList<SItemPanel *> *> m_itemRecycle; /**< Recycle bin for item panels */

    SXmlDoc m_xmlTemplate; /**< XML template for items */
    BOOL m_bWantTab;       /**< Flag indicating whether to want tab */
    BOOL m_bMultiSel;      /**< Flag indicating if multiple selection is enabled */

    typedef SMap<int, BOOL> ItemSelectionMap;
    ItemSelectionMap m_mapSelItems; /**< Map of selected items (index -> is_selected) */
    SArray<int> m_arrBandSnapshot;  /**< Selection snapshot taken when the rubber band starts */

    SPanel *m_pView; /**< Pointer to the view panel */
};

SNSEND

#endif /**< __SVIEWBASE__H__ */
