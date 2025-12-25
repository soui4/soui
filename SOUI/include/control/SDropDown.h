#ifndef __SDROPDOWN__H__
#define __SDROPDOWN__H__

#include <core/SHostWnd.h>

SNSBEGIN

#define IDC_DROPDOWN_LIST -200

class SDropDownWnd;

/**
 * @struct ISDropDownOwner
 * @brief Interface for Dropdown Owner
 * @details This interface defines the methods that a dropdown owner must implement.
 */
struct ISDropDownOwner
{
    /**
     * @brief Get the dropdown owner window
     * @return Pointer to the dropdown owner window
     */
    virtual SWindow *GetDropDownOwner() = 0;

    /**
     * @brief Notify that the dropdown window has been created
     * @param pDropDown Pointer to the dropdown window
     */
    virtual void OnCreateDropDown(SDropDownWnd *pDropDown) = 0;

    /**
     * @brief Notify that the dropdown window is being destroyed
     * @param pDropDown Pointer to the dropdown window
     */
    virtual void OnDestroyDropDown(SDropDownWnd *pDropDown) = 0;
};

/**
 * @class SDropDownWnd
 * @brief Dropdown Window Class
 * @details A base class for dropdown windows. Derive from this class to create custom dropdown windows.
 */
class SOUI_EXP SDropDownWnd
    : public SHostWnd
    , public IMsgFilter {
    typedef SHostWnd __baseCls;

  public:
    /**
     * @brief Constructor
     * @param pOwner Pointer to the dropdown owner
     */
    SDropDownWnd(ISDropDownOwner *pOwner);

    /**
     * @brief Destructor
     */
    virtual ~SDropDownWnd();

    /**
     * @brief Get the dropdown owner window
     * @return Pointer to the dropdown owner window
     */
    SWindow *GetDropDownOwner();

    /**
     * @brief Create the dropdown window
     * @param lpRect Rectangle for the dropdown window
     * @param pInitXml initialization XML node
     * @param dwStyle Window style
     * @param dwExStyle Extended window style
     * @return TRUE if successful, FALSE otherwise
     */
    BOOL Create(LPCRECT lpRect, IXmlNode *pInitXml, DWORD dwStyle = WS_POPUP, DWORD dwExStyle = WS_EX_TOOLWINDOW | WS_EX_TOPMOST);

    /**
     * @brief End the dropdown window
     * @param uCode Exit code for the dropdown window
     */
    void EndDropDown(UINT uCode = IDCANCEL);

    /**
     * @brief Get the exit code of the dropdown window
     * @return Exit code
     */
    UINT GetExitCode() const
    {
        return m_uExitCode;
    }

    /**
     * @brief Get the value of the dropdown window
     * @return Value of the dropdown window (default is 0)
     */
    virtual int GetValue() const
    {
        return 0;
    }

  protected:
    /**
     * @brief Get the message loop
     * @return Pointer to the message loop
     */
    STDMETHOD_(IMessageLoop *, GetMsgLoop)();

    /**
     * @brief Prevent the window from releasing capture
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, OnReleaseSwndCapture)();

    /**
     * @brief Pre-translate messages
     * @param pMsg Pointer to the message
     * @return TRUE if the message is handled, FALSE otherwise
     */
    virtual BOOL WINAPI PreTranslateMessage(MSG *pMsg);

    /**
     * @brief Handle left mouse button down event
     * @param nFlags Flags
     * @param point Mouse coordinates
     */
    void OnLButtonDown(UINT nFlags, CPoint point);

    /**
     * @brief Handle left mouse button up event
     * @param nFlags Flags
     * @param point Mouse coordinates
     */
    void OnLButtonUp(UINT nFlags, CPoint point);

    /**
     * @brief Handle key down event
     * @param nChar Key code
     * @param nRepCnt Repeat count
     * @param nFlags Flags
     */
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Handle kill focus event
     * @param wndFocus Handle to the window receiving focus
     */
    void OnKillFocus(HWND wndFocus);

    /**
     * @brief Handle destroy event
     */
    void OnDestroy();

    /**
     * @brief Handle activate application event
     * @param bActive Activation flag
     * @param dwThreadID Thread ID
     */
    void OnActivateApp(BOOL bActive, DWORD dwThreadID);

    /**
     * @brief Handle mouse activate event
     * @param wndTopLevel Handle to the top-level window
     * @param nHitTest Hit test code
     * @param message Mouse message code
     * @return Activation code
     */
    int OnMouseActivate(HWND wndTopLevel, UINT nHitTest, UINT message);

    /**
     * @brief Handle activate event
     * @param nState Activation state
     * @param bMinimized Minimized flag
     * @param wndOther Handle to the previous active window
     */
    void OnActivate(UINT nState, BOOL bMinimized, HWND wndOther); // Interrupt message processing to prevent setting focus

    /**
     * @brief Handle final message
     * @param hwnd Handle to the window
     */
    virtual void OnFinalMessage(HWND hwnd);

  protected:
    ISDropDownOwner *m_pOwner; /**< Pointer to the dropdown owner */
    BOOL m_bClick;             /**< Click state */
    UINT m_uExitCode;          /**< Exit code */

    BEGIN_MSG_MAP_EX(SDropDownWnd)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_KILLFOCUS(OnKillFocus)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_ACTIVATE(OnActivate)
        MSG_WM_ACTIVATEAPP(OnActivateApp)
        MSG_WM_MOUSEACTIVATE(OnMouseActivate)
        CHAIN_MSG_MAP(SHostWnd)
    END_MSG_MAP()
};

SNSEND

#endif // __SDROPDOWN__H__