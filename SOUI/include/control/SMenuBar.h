#ifndef __SMENUBAR__H__
#define __SMENUBAR__H__

#include <core/SWnd.h>
#include <helper/SMenu.h>
#include <helper/SMenuEx.h>
#include <helper/STls.h>

SNSBEGIN

/**
 * @class SMenuBarItem
 * @brief Menu Bar Item
 * @details Represents an item in the menu bar.
 */
class SMenuBarItem;

/**
 * @class SMenuBar
 * @brief Menu Bar Control
 * @details A control that displays a menu bar with multiple menu items.
 */
class SOUI_EXP SMenuBar : public SWindow {
    DEF_SOBJECT(SWindow, L"menubar")
    friend class SMenuBarItem;

  public:
    /**
     * @brief Constructor
     */
    SMenuBar();

    /**
     * @brief Destructor
     */
    ~SMenuBar();

    /**
     * @brief Insert a menu item by title and resource name
     * @param pszTitle Title of the menu item
     * @param pszResName Resource name of the menu item
     * @param iPos Position to insert the menu item, -1 for end
     * @return TRUE if successful, FALSE otherwise
     */
    BOOL Insert(LPCTSTR pszTitle, LPCTSTR pszResName, int iPos = -1);

    /**
     * @brief Insert a menu item from an XML node
     * @param pNode XML node containing the menu item configuration
     * @param iPos Position to insert the menu item, -1 for end
     * @return TRUE if successful, FALSE otherwise
     */
    BOOL Insert(IXmlNode *pNode, int iPos = -1);

    /**
     * @brief Get the menu at a specific position
     * @param dwPos Position of the menu
     * @return Pointer to the menu, NULL if not found
     */
    SMenu *GetMenu(DWORD dwPos);

    /**
     * @brief Get the menu ex at a specific position
     * @param dwPos Position of the menu
     * @return Pointer to the menu ex, NULL if not found
     */
    SMenuEx *GetMenuEx(DWORD dwPos);

    /**
     * @brief Hit test to determine the menu item under the mouse
     * @param pt Mouse coordinates in the host window's client coordinate system
     * @return Index of the menu item if found, -1 otherwise
     */
    int HitTest(CPoint pt);

    /**
     * @brief Check if using SMenuEx
     * @return TRUE if using SMenuEx, FALSE otherwise
     */
    BOOL IsUseMenuEx() const
    {
        return m_bUseMenuEx;
    }

  protected:
    /**
     * @brief Get the menu bar item at a specific position
     * @param dwPos Position of the menu item
     * @return Pointer to the menu bar item, NULL if not found
     */
    SMenuBarItem *GetMenuItem(DWORD dwPos);

    /**
     * @brief Create child items from XML configuration
     * @param xmlNode XML node for the child items
     * @return TRUE if successful, FALSE otherwise
     */
    BOOL CreateChildren(SXmlNode xmlNode) override;

    /**
     * @brief Callback function for menu switch
     * @param code Hook code
     * @param wParam Additional message-specific information
     * @param lParam Additional message-specific information
     * @return Result of the message processing
     */
    static LRESULT CALLBACK MenuSwitch(int code, WPARAM wParam, LPARAM lParam);

    SOUI_ATTRS_BEGIN()
        ATTR_BOOL(L"useMenuEx", m_bUseMenuEx, FALSE)
    SOUI_ATTRS_END()
  protected:
    SArray<SMenuBarItem *> m_lstMenuItem; /**< Array of menu bar items */
    HWND m_hWnd;                          /**< Handle to the window */
    SXmlDoc m_xmlStyle;                   /**< XML document for style configuration */
    BOOL m_bIsShow;                       /**< Flag indicating if the menu bar is shown */
    SMenuBarItem *m_pNowMenu;             /**< Pointer to the currently active menu item */
    int m_iNowMenu;                       /**< Index of the currently active menu item */
    CPoint m_ptMouse;                     /**< Mouse coordinates */
    BOOL m_bUseMenuEx;                    /**< Flag indicating whether to use SMenuEx */

    /**
     * @brief Get the message hook handle stored for the current thread
     * @details VS2008 无 thread_local 支持，经 STls 线程局部存储保存。
     */
    static HHOOK GetMsgHook();

    /**
     * @brief Set the message hook handle for the current thread
     * @param hMsgHook Hook handle, NULL to clear
     */
    static void SetMsgHook(HHOOK hMsgHook);

    /**
     * @brief Get the menu bar instance bound to the current thread
     */
    static SMenuBar *GetMenuBar();

    /**
     * @brief Bind the current thread's menu bar instance
     * @param pMenuBar Menu bar instance, NULL to clear
     */
    static void SetMenuBar(SMenuBar *pMenuBar);

  private:
    static STlsId s_tlsMsgHook; /**< TLS slot id for the message hook handle */
    static STlsId s_tlsMenuBar; /**< TLS slot id for the current menu bar instance */
};

SNSEND

#endif /**< __SMENUBAR__H__ */