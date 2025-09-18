#ifndef __SHOTKEYCTRL__H__
#define __SHOTKEYCTRL__H__

#include <core/SWnd.h>
#include <core/SAccelerator.h>
#include <proxy/SWindowProxy.h>
#include <interface/SCtrls-i.h>

SNSBEGIN

/**
 * @class SHotKeyCtrl
 * @brief HotKey Control
 * @details A control for setting and displaying hotkeys.
 */
class SOUI_EXP SHotKeyCtrl
    : public TWindowProxy<IHotKeyCtrl>
    , public SAccelerator {
    DEF_SOBJECT(SWindow, L"hotkey")

    enum
    {
        Mod_SC = Mod_Shift | Mod_Ctrl,
        Mod_SA = Mod_Shift | Mod_Alt,
        Mod_CA = Mod_Ctrl | Mod_Alt,
        Mod_SCA = Mod_Shift | Mod_Ctrl | Mod_Alt,
    };

  public:
    /**
     * @brief Constructor
     */
    SHotKeyCtrl(void);

    /**
     * @brief Destructor
     */
    virtual ~SHotKeyCtrl(void);

  public:
    /**
     * @brief Set the rules for invalid combinations and modifiers
     * @param wInvalidComp Invalid combination flags
     * @param wModifier Modifier flags
     */
    STDMETHOD_(void, SetRule)(THIS_ WORD wInvalidComp, WORD wModifier) OVERRIDE;

    /**
     * @brief Set the hotkey
     * @param wKey Key code
     * @param wModifiers Modifier flags
     */
    STDMETHOD_(void, SetHotKey)(THIS_ WORD wKey, WORD wModifiers) OVERRIDE;

    /**
     * @brief Get the hotkey
     * @param wKey Output key code
     * @param wModifiers Output modifier flags
     */
    STDMETHOD_(void, GetHotKey)(THIS_ WORD *wKey, WORD *wModifiers) SCONST OVERRIDE;

  public:
    /**
     * @brief Get the window text
     * @param bRawText Whether to get raw text
     * @return Window text
     */
    virtual SStringT GetWindowText(BOOL bRawText = FALSE) SCONST OVERRIDE;

  protected:
    /**
     * @brief Get the dialog code
     * @return Dialog code
     * @details This function is a message handler.
     */
    virtual UINT WINAPI OnGetDlgCode() const
    {
        return (SC_WANTALLKEYS | SC_WANTSYSKEY) & (~SC_WANTTAB);
    }

    /**
     * @brief Handle left mouse button down event
     * @param nFlags Flags
     * @param pt Mouse coordinates
     * @details This function is a message handler.
     */
    void OnLButtonDown(UINT nFlags, CPoint pt);

    /**
     * @brief Paint the control
     * @param pRT Rendering target handle
     * @details This function is a message handler.
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief Handle set focus event
     * @param wndOld Previous focus window handle
     * @details This function is a message handler.
     */
    void OnSetFocus(SWND wndOld);

    /**
     * @brief Handle kill focus event
     * @param wndFocus New focus window handle
     * @details This function is a message handler.
     */
    void OnKillFocus(SWND wndFocus);

    /**
     * @brief Handle key down event
     * @param nChar Key code
     * @param nRepCnt Repeat count
     * @param nFlags Flags
     * @details This function is a message handler.
     */
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Handle key up event
     * @param nChar Key code
     * @param nRepCnt Repeat count
     * @param nFlags Flags
     * @details This function is a message handler.
     */
    void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Handle system key down event
     * @param nChar Key code
     * @param nRepCnt Repeat count
     * @param nFlags Flags
     * @details This function is a message handler.
     */
    void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Handle system key up event
     * @param nChar Key code
     * @param nRepCnt Repeat count
     * @param nFlags Flags
     * @details This function is a message handler.
     */
    void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Update the modifier keys
     */
    void UpdateModifier();

    /**
     * @brief Update the caret position
     * @param pRT Rendering target handle
     */
    void UpdateCaret(IRenderTarget *pRT);

    /**
     * @brief Get the text alignment
     * @return Text alignment
     */
    UINT GetTextAlign();

  protected:
    /**
     * @brief Handle custom attribute "invalidComb"
     * @param value Attribute value
     * @param bLoading Loading flag
     * @return HRESULT
     */
    HRESULT OnAttrInvalidComb(const SStringW &value, BOOL bLoading);

    /**
     * @brief Handle custom attribute "invalidSysKey"
     * @param value Attribute value
     * @param bLoading Loading flag
     * @return HRESULT
     */
    HRESULT OnAttrInvalidSysKey(const SStringW &value, BOOL bLoading);

    /**
     * @brief Handle custom attribute "invalidModifier"
     * @param value Attribute value
     * @param bLoading Loading flag
     * @return HRESULT
     */
    HRESULT OnAttrInvalidModifier(const SStringW &value, BOOL bLoading);

    /**
     * @brief Handle custom attribute "hotKey"
     * @param value Attribute value
     * @param bLoading Loading flag
     * @return HRESULT
     */
    HRESULT OnAttrHotKey(const SStringW &value, BOOL bLoading);

    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"invalidComb", OnAttrInvalidComb)
        ATTR_CUSTOM(L"invalidSysKey", OnAttrInvalidSysKey)
        ATTR_CUSTOM(L"invalidModifier", OnAttrInvalidModifier)
        ATTR_CUSTOM(L"hotKey", OnAttrHotKey)
    SOUI_ATTRS_END()

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_SETFOCUS_EX(OnSetFocus)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_KEYUP(OnKeyUp)
        MSG_WM_SYSKEYDOWN(OnSysKeyDown)
        MSG_WM_SYSKEYUP(OnSysKeyUp)
    SOUI_MSG_MAP_END()

  protected:
    WORD m_wInvalidComb;     /**< Flags for invalid key combinations */
    WORD m_wInvalidModifier; /**< Replacement modifiers for invalid combinations */
    BOOL m_bInSetting;       /**< Flag indicating if the control is being set */
};

SNSEND

#endif // __SHOTKEYCTRL__H__