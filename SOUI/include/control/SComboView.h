#ifndef __SCOMBOVIEW__H__
#define __SCOMBOVIEW__H__

#include "SComboBase.h"
#include "SListView.h"

SNSBEGIN

/**
 * @class SComboView
 * @brief ComboBox Control with ListView
 * @details A ComboBox control that uses a ListView for its dropdown content.
 */
class SOUI_EXP SComboView : public TComboBaseProxy<IComboView> {
    DEF_SOBJECT(SComboBase, L"comboview")

  public:
    /**
     * @brief Constructor
     */
    SComboView(void);

    /**
     * @brief Destructor
     */
    ~SComboView(void);

  public:
    /**
     * @brief Set the current selection index
     * @param iSel Index to set as the current selection
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, SetCurSel)(int iSel) OVERRIDE;

    /**
     * @brief Get the current selection index
     * @return Current selection index
     */
    STDMETHOD_(int, GetCurSel)() SCONST OVERRIDE;

    /**
     * @brief Get the number of items in the list view
     * @return Number of items
     */
    STDMETHOD_(int, GetCount)() SCONST OVERRIDE;

    /**
     * @brief Get the text of a list view item
     * @param iItem Item index
     * @param bRawText Whether to get raw text
     * @param str Output string for the item text
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, GetItemText)(int iItem, BOOL bRawText, IStringT *str) SCONST OVERRIDE;

    /**
     * @brief Get the text of a list view item (ANSI version)
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
     * @brief Get the list view interface
     * @return Pointer to the list view interface
     */
    STDMETHOD_(IListView *, GetIListView)(THIS) OVERRIDE;

  public:
    /**
     * @brief Get the list view pointer
     * @return Pointer to the list view
     */
    SListView *GetListView();

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
    virtual HRESULT OnLanguageChanged() override;

    /**
     * @brief Create the list view
     * @param xmlNode XML node for the list view
     * @return TRUE if successful, FALSE otherwise
     */
    virtual BOOL CreateListBox(SXmlNode xmlNode);

    /**
     * @brief Get the height of the list view
     * @return Height of the list view
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
    SListView *m_pListBox; /**< Pointer to the list view */
};

SNSEND

#endif // __SCOMBOVIEW__H__