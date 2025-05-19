#ifndef __SCOMBOBOX__H__
#define __SCOMBOBOX__H__

#include <control/SComboBase.h>
#include <control/SListbox.h>
#include <control/SCmnCtrl.h>

SNSBEGIN

/**
 * @class SComboBox
 * @brief Editable ComboBox Control
 * @details A ComboBox control that can be editable and includes a dropdown list.
 */
class SOUI_EXP SComboBox : public TComboBaseProxy<IComboBox> {
    DEF_SOBJECT(SComboBase, L"combobox")

  public:
    /**
     * @brief Constructor
     */
    SComboBox();

    /**
     * @brief Destructor
     */
    virtual ~SComboBox();

  public:
    /**
     * @brief Set the current selection index
     * @param iSel Index to set as the current selection
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, SetCurSel)(THIS_ int iSel) OVERRIDE;

    /**
     * @brief Get the current selection index
     * @return Current selection index
     */
    STDMETHOD_(int, GetCurSel)(THIS) SCONST OVERRIDE;

    /**
     * @brief Get the number of items in the list box
     * @return Number of items
     */
    STDMETHOD_(int, GetCount)(THIS) SCONST OVERRIDE;

    /**
     * @brief Get the data associated with a list box item
     * @param iItem Item index
     * @return Data associated with the item
     */
    STDMETHOD_(LPARAM, GetItemData)(THIS_ UINT iItem) SCONST OVERRIDE;

    /**
     * @brief Set the data associated with a list box item
     * @param iItem Item index
     * @param lParam Data to associate with the item
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, SetItemData)(THIS_ UINT iItem, LPARAM lParam) OVERRIDE;

    /**
     * @brief Insert an item into the list box
     * @param iPos Position to insert the item
     * @param pszText Text of the item
     * @param iIcon Icon index for the item
     * @param lParam Data to associate with the item
     * @return Index of the inserted item or -1 if failed
     */
    STDMETHOD_(int, InsertItem)(THIS_ int iPos, LPCTSTR pszText, int iIcon, LPARAM lParam) OVERRIDE;

    /**
     * @brief Insert an item into the list box (ANSI version)
     * @param iPos Position to insert the item
     * @param pszText Text of the item
     * @param iIcon Icon index for the item
     * @param lParam Data to associate with the item
     * @return Index of the inserted item or -1 if failed
     */
    STDMETHOD_(int, InsertItemA)(THIS_ int iPos, LPCSTR pszText, int iIcon, LPARAM lParam) OVERRIDE
    {
        SStringT str = S_CA2T(pszText, CP_UTF8);
        return InsertItem(iPos, str, iIcon, lParam);
    }

    /**
     * @brief Delete a string from the list box
     * @param iPos Index of the item to delete
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, DeleteString)(THIS_ int iPos) OVERRIDE;

    /**
     * @brief Reset the content of the list box
     */
    STDMETHOD_(void, ResetContent)(THIS) OVERRIDE;

    /**
     * @brief Get the text of a list box item
     * @param iItem Item index
     * @param bRawText Whether to get raw text
     * @param str Output string for the item text
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, GetItemText)(int iItem, BOOL bRawText, IStringT *str) SCONST OVERRIDE;

    /**
     * @brief Get the text of a list box item (ANSI version)
     * @param iItem Item index
     * @param bRawText Whether to get raw text
     * @param str Output string for the item text
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, GetItemTextA)(int iItem, BOOL bRawText, IStringA *str) SCONST OVERRIDE
    {
        SStringT strBuf;
        BOOL bRet = GetItemText(iItem, bRawText, &strBuf);
        SStringA strBufA = S_CT2A(strBuf, CP_UTF8);
        str->Copy(&strBufA);
        return bRet;
    }

    /**
     * @brief Get the list box interface
     * @return Pointer to the list box interface
     */
    STDMETHOD_(IListBox *, GetIListBox)(THIS) OVERRIDE;

  public:
    /**
     * @brief Get the list box pointer
     * @return Pointer to the list box
     */
    SListBox *GetListBox()
    {
        return m_pListBox;
    }

  protected:
    /**
     * @brief Notify event
     * @param evt Event object
     * @return TRUE if successful, FALSE otherwise
     * @details This function is a message handler.
     */
    STDMETHOD_(BOOL, FireEvent)(THIS_ IEvtArgs *evt) OVERRIDE;

    /**
     * @brief Handle scale change event
     * @param nScale Scale factor
     */
    virtual void OnScaleChanged(int nScale);

    /**
     * @brief Handle language change event
     * @return HRESULT
     */
    virtual HRESULT OnLanguageChanged();

    /**
     * @brief Create the list box
     * @param xmlNode XML node for the list box
     * @return TRUE if successful, FALSE otherwise
     */
    virtual BOOL CreateListBox(SXmlNode xmlNode);

    /**
     * @brief Get the height of the list box
     * @return Height of the list box
     */
    virtual int GetListBoxHeight();

    /**
     * @brief Handle creation of the dropdown window
     * @param pDropDown Dropdown window pointer
     */
    virtual void OnCreateDropDown(SDropDownWnd *pDropDown);

    /**
     * @brief Handle destruction of the dropdown window
     * @param pDropDown Dropdown window pointer
     */
    virtual void OnDestroyDropDown(SDropDownWnd *pDropDown);

    /**
     * @brief Handle selection change in the dropdown window
     */
    virtual void OnSelChanged();

  protected:
    SListBox *m_pListBox; /**< Pointer to the list box */
};

SNSEND

#endif // __SCOMBOBOX__H__