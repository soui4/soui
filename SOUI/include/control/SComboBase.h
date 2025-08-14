#ifndef __SCOMBOBASE__H__
#define __SCOMBOBASE__H__

#include <core/SWnd.h>
#include <control/SEdit.h>
#include <control/SDropDown.h>

SNSBEGIN

#define IDC_CB_EDIT       -100
#define IDC_DROPDOWN_LIST -200

/**
 * @class SComboEdit
 * @brief Edit Control Embedded in ComboBox
 * @details An edit control embedded within a ComboBox.
 */
class SOUI_EXP SComboEdit : public SEdit {
    DEF_SOBJECT(SEdit, L"ComboEdit")

  public:
    /**
     * @brief Constructor
     */
    SComboEdit();

    /**
     * @brief Destructor
     */
    virtual ~SComboEdit();

  protected:
    /**
     * @brief Handle mouse hover event
     * @param wParam Extra parameters
     * @param ptPos Mouse position
     * @details This function is a message handler.
     */
    void OnMouseHover(WPARAM wParam, CPoint ptPos);

    /**
     * @brief Handle mouse leave event
     * @details This function is a message handler.
     */
    void OnMouseLeave();

    /**
     * @brief Handle key down event
     * @param nChar Key code
     * @param nRepCnt Repeat count
     * @param nFlags Flags
     * @details This function is a message handler.
     */
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Notify event
     * @param evt Event object
     * @return Success--TRUE Failure--FALSE
     * @details This function is a message handler.
     */
    STDMETHOD_(BOOL, FireEvent)(THIS_ IEvtArgs *evt) OVERRIDE;

    /**
     * @brief Handle kill focus event
     * @param wndFocus New focus window handle
     */
    void OnKillFocus(SWND wndFocus);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_MOUSEHOVER(OnMouseHover)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
    SOUI_MSG_MAP_END()
};

class SOUI_EXP SDropDownWnd_ComboBox : public SDropDownWnd {
  public:
    SDropDownWnd_ComboBox(ISDropDownOwner *pOwner)
        : SDropDownWnd(pOwner)
    {
    }

    virtual BOOL WINAPI PreTranslateMessage(MSG *pMsg);
};

/**
 * @class SComboBoxBase
 * @brief Base ComboBox Control
 * @details A base class for ComboBox controls that can be editable or dropdown-only.
 */
class SOUI_EXP SComboBase
    : public TWindowProxy<IComboBase>
    , public ISDropDownOwner {
    DEF_SOBJECT(SWindow, L"combobase")

  public:
    /**
     * @brief Constructor
     */
    SComboBase(void);

    /**
     * @brief Destructor
     */
    virtual ~SComboBase(void);

  public:
    /**
     * @brief Find string in the list box
     * @param pszFind String to find
     * @param nAfter Index after which to start searching
     * @param bPartMatch Whether to allow partial matches
     * @return Index of the found string or -1 if not found
     */
    STDMETHOD_(int, FindString)
    (THIS_ LPCTSTR pszFind, int nAfter = -1, BOOL bPartMatch = TRUE) OVERRIDE;

    /**
     * @brief Find string in the list box (ANSI version)
     * @param pszFind String to find
     * @param nAfter Index after which to start searching
     * @param bPartMatch Whether to allow partial matches
     * @return Index of the found string or -1 if not found
     */
    STDMETHOD_(int, FindStringA)
    (THIS_ LPCSTR pszFind, int nAfter = -1, BOOL bPartMatch = TRUE) OVERRIDE
    {
        SStringT str = S_CA2T(pszFind, CP_UTF8);
        return FindString(str, nAfter, bPartMatch);
    }

    /**
     * @brief Drop down the list box
     */
    STDMETHOD_(void, DropDown)(THIS) OVERRIDE;

    /**
     * @brief Close the dropped list box
     */
    STDMETHOD_(void, CloseUp)(THIS) OVERRIDE;

    /**
     * @brief Check if the list box is dropped down
     * @return TRUE if dropped down, FALSE otherwise
     */
    STDMETHOD_(BOOL, IsDropdown)(THIS) SCONST OVERRIDE;

    /**
     * @brief Set the dropdown state
     * @param bDropdown TRUE to drop down, FALSE to close up
     */
    STDMETHOD_(void, SetDropdown)(THIS_ BOOL bDropdown) OVERRIDE;

  public:
    /**
     * @brief Set the window text
     * @param pszText Text to set
     */
    STDMETHOD_(void, SetWindowText)(LPCTSTR pszText) OVERRIDE;

    /**
     * @brief Get the desired size of the control
     * @param psz Output size
     * @param nParentWid Parent container width
     * @param nParentHei Parent container height
     */
    STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE *psz, int nParentWid, int nParentHei) OVERRIDE;

    /**
     * @brief Set focus to the control
     */
    STDMETHOD_(void, SetFocus)(THIS) OVERRIDE;

    /**
     * @brief Notify event
     * @param evt Event object
     * @return Success--TRUE Failure--FALSE
     */
    STDMETHOD_(BOOL, FireEvent)(THIS_ IEvtArgs *evt) OVERRIDE;

    /**
     * @brief Get dialog code
     * @return Dialog code
     */
    STDMETHOD_(UINT, OnGetDlgCode)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Check if the control is focusable
     * @return TRUE if focusable, FALSE otherwise
     */
    STDMETHOD_(BOOL, IsFocusable)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Update the position of child controls
     */
    STDMETHOD_(void, UpdateChildrenPosition)(THIS) OVERRIDE;

    /**
     * @brief Get the window text
     * @param bRawText Whether to get raw text
     * @return Window text
     */
    SStringT GetWindowText(BOOL bRawText = FALSE) SCONST OVERRIDE;

    /**
     * @brief Get the text of a list box item
     * @param iItem Item index
     * @param bRawText Whether to get raw text
     * @return Item text
     */
    SStringT GetLBText(int iItem, BOOL bRawText = FALSE) SCONST;

  protected:
    /**
     * @brief Get the owner window
     * @return Owner window pointer
     */
    virtual SWindow *GetDropDownOwner();

    /**
     * @brief Handle creation of the dropdown window
     * @param pDropDown Dropdown window pointer
     */
    virtual void OnCreateDropDown(SDropDownWnd *pDropDown);

    /**
     * @brief Handle destruction of the dropdown window
     * @param pDropDown Dropdown window pointer
     * @param uCode Message code
     */
    virtual void OnDestroyDropDown(SDropDownWnd *pDropDown);

    /**
     * @brief Handle selection change in the dropdown window
     */
    virtual void OnSelChanged();

  protected:
    /**
     * @brief Update the dropdown list based on input
     * @param strInput Input string
     */
    void UpdateDropdown(const SStringT &strInput);

    /**
     * @brief Calculate the popup rectangle
     * @param nHeight Height of the dropdown window
     * @param rcPopup Output rectangle for the popup
     * @return TRUE if successful, FALSE otherwise
     */
    BOOL CalcPopupRect(int nHeight, CRect &rcPopup);

    /**
     * @brief Create the list box
     * @param xmlNode XML node for the list box
     * @return TRUE if successful, FALSE otherwise
     */
    virtual BOOL CreateListBox(SXmlNode xmlNode) = 0;

    /**
     * @brief Get the height of the list box
     * @return Height of the list box
     */
    virtual int GetListBoxHeight() = 0;

    /**
     * @brief Get the rectangle of the dropdown button
     * @param prc Output rectangle for the button
     */
    void GetDropBtnRect(LPRECT prc);

    /**
     * @brief Create child controls
     * @param xmlNode XML node for the child controls
     * @return TRUE if successful, FALSE otherwise
     */
    virtual BOOL CreateChildren(SXmlNode xmlNode);

    /**
     * @brief Get the rectangle of the text
     * @param pRect Output rectangle for the text
     */
    virtual void GetTextRect(LPRECT pRect);

    /**
     * @brief Handle colorization event
     * @param cr Color
     */
    virtual void OnColorize(COLORREF cr);

    /**
     * @brief Handle language change event
     * @return HRESULT
     */
    virtual HRESULT OnLanguageChanged();

    /**
     * @brief Handle scale change event
     * @param nScale Scale factor
     */
    virtual void OnScaleChanged(int nScale);

    /**
     * @brief Paint the control
     * @param pRT Rendering target handle
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief Handle left mouse button down event
     * @param nFlags Flags
     * @param pt Mouse coordinates
     */
    void OnLButtonDown(UINT nFlags, CPoint pt);

    /**
     * @brief Handle mouse move event
     * @param nFlags Flags
     * @param pt Mouse coordinates
     */
    void OnMouseMove(UINT nFlags, CPoint pt);

    /**
     * @brief Handle mouse leave event
     */
    void OnMouseLeave();

    /**
     * @brief Handle key down event
     * @param nChar Key code
     * @param nRepCnt Repeat count
     * @param nFlags Flags
     */
    void OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Handle mouse wheel event
     * @param nFlags Flags
     * @param zDelta Wheel delta
     * @param pt Mouse coordinates
     * @return TRUE if handled, FALSE otherwise
     */
    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    /**
     * @brief Handle character input event
     * @param nChar Character code
     * @param nRepCnt Repeat count
     * @param nFlags Flags
     */
    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Handle destroy event
     */
    void OnDestroy();

    /**
     * @brief Handle create event
     * @param lp Creation parameters
     * @return 0 if successful
     */
    int OnCreate(LPVOID);

    /**
     * @brief Handle kill focus event
     * @param wndFocus New focus window handle
     */
    void OnKillFocus(SWND wndFocus);

    /**
     * @brief Get the cue text
     * @param bRawText Whether to get raw text
     * @return Cue text
     */
    SStringT GetCueText(BOOL bRawText = FALSE) const;

    /**
     * @brief Handle custom attribute "dropDown"
     * @param strValue Attribute value
     * @param bLoading Loading flag
     * @return HRESULT
     */
    HRESULT OnAttrDropDown(const SStringW &strValue, BOOL bLoading);

    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"dropDown", OnAttrDropDown)
        ATTR_LAYOUTSIZE(L"dropHeight", m_nDropHeight, TRUE)
        ATTR_INT(L"curSel", m_iInitSel, FALSE)
        ATTR_SKIN(L"btnSkin", m_pSkinBtn, TRUE)
        ATTR_INT(L"animateTime", m_nAnimTime, FALSE)
        ATTR_INT(L"autoFitDropBtn", m_bAutoFitDropBtn, TRUE)
        ATTR_COLOR(L"cueColor", m_crCue, TRUE)
        ATTR_I18NSTRT(L"cueText", m_strCue, TRUE)
        ATTR_BOOL(L"autoMatch", m_bAutoMatch, FALSE)
        ATTR_BOOL(L"autoDropdown", m_bAutoDropdown, FALSE)
    SOUI_ATTRS_END()

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_CHAR(OnChar)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
    SOUI_MSG_MAP_END()

  protected:
    /**
     * @brief Get the text from the edit control
     * @return Edit control text
     */
    SStringT GetEditText() const
    {
        if (!m_bDropdown)
        {
            return m_pEdit->GetWindowText();
        }
        else
        {
            return SStringT();
        }
    }

    SEdit *m_pEdit;                   /**< Edit control pointer */
    DWORD m_dwBtnState;               /**< Button state */
    SAutoRefPtr<ISkinObj> m_pSkinBtn; /**< Button skin object */

    BOOL m_bDropdown;             /**< Editable or dropdown-only */
    SLayoutSize m_nDropHeight;    /**< Dropdown height */
    int m_nAnimTime;              /**< Animation time */
    int m_iInitSel;               /**< Initial selection index */
    SDropDownWnd *m_pDropDownWnd; /**< Dropdown window pointer */
    SXmlDoc m_xmlDropdownStyle;   /**< Dropdown style XML document */
    BOOL m_bAutoFitDropBtn;       /**< Auto-fit dropdown button size */
    COLORREF m_crCue;             /**< Cue text color */
    STrText m_strCue;             /**< Cue text */

    SStringT m_strMatch;   /**< Quick input to locate items in the dropdown list */
    DWORD m_LastPressTime; /**< Last key press time */
    BOOL m_bAutoMatch;     /**< Automatically match from the dropdown list on edit input */
    BOOL m_bAutoDropdown;  /**< Automatically update the dropdown list on edit input */
    int m_nTextLength;
};

template <class T>
class TComboBaseProxy
    : public T
    , public SComboBase {
  public:
    STDMETHOD_(long, AddRef)(THIS) OVERRIDE
    {
        return SComboBase::AddRef();
    }

    STDMETHOD_(long, Release)(THIS) OVERRIDE
    {
        return SComboBase::Release();
    }

    STDMETHOD_(void, OnFinalRelease)(THIS) OVERRIDE
    {
        SComboBase::OnFinalRelease();
    }

    STDMETHOD_(IWindow *, ToIWindow)(THIS) OVERRIDE
    {
        return (SComboBase *)this;
    }

    STDMETHOD_(HRESULT, QueryInterface)(REFGUID id, IObjRef **ppRet) OVERRIDE
    {
        if (id == T::GetIID())
        {
            *ppRet = (T *)this;
            AddRef();
            return S_OK;
        }
        else
        {
            return SComboBase::QueryInterface(id, ppRet);
        }
    }

    STDMETHOD_(int, FindString)
    (THIS_ LPCTSTR pszFind, int nAfter = -1, BOOL bPartMatch = TRUE) OVERRIDE
    {
        return SComboBase::FindString(pszFind, nAfter, bPartMatch);
    }

    STDMETHOD_(int, FindStringA)
    (THIS_ LPCSTR pszFind, int nAfter = -1, BOOL bPartMatch = TRUE) OVERRIDE
    {
        return SComboBase::FindStringA(pszFind, nAfter, bPartMatch);
    }

    STDMETHOD_(void, DropDown)(THIS) OVERRIDE
    {
        return SComboBase::DropDown();
    }

    STDMETHOD_(void, CloseUp)(THIS) OVERRIDE
    {
        return SComboBase::CloseUp();
    }

    STDMETHOD_(BOOL, IsDropdown)(THIS) SCONST OVERRIDE
    {
        return SComboBase::IsDropdown();
    }

    STDMETHOD_(void, SetDropdown)(THIS_ BOOL bDropdown) OVERRIDE
    {
        return SComboBase::SetDropdown(bDropdown);
    }
};

SNSEND
#endif // __SCOMBOBASE__H__