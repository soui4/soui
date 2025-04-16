#ifndef __SMESSAGEBOX__H__
#define __SMESSAGEBOX__H__

#include <core/SHostDialog.h>
#include <helper/SDpiHelper.hpp>
#include <interface/SMessageBox-i.h>

SNSBEGIN

// IDs that must be specified in the message box template.
#define NAME_MSGBOX_BTNSWITCH L"btnSwitch" // Tab control containing 3 pages
#define NAME_MSGBOX_TEXT      L"msgtext"   // Text control, only coordinates need to be specified
#define NAME_MSGBOX_TITLE     L"msgtitle"  // Title control ID
#define NAME_MSGBOX_ICON      L"msgicon"   // Icon display control ID
#define NAME_MSGBOX_BTN1      L"button1st" // First button ID, will be automatically modified to IDs like IDOK, IDCANCEL, etc.
#define NAME_MSGBOX_BTN2      L"button2nd" // Second button ID
#define NAME_MSGBOX_BTN3      L"button3rd" // Third button ID

/**
 * @class SMessageBoxImpl
 * @brief Message Box Implementation
 * @details Implementation class for message boxes. If more personalized message boxes are needed, derive from this class.
 */
class SOUI_EXP SMessageBoxImpl
    : public SHostDialog
    , public SDpiHandler<SMessageBoxImpl> {
  public:
    /**
     * @brief Constructor
     */
    SMessageBoxImpl();

    /**
     * @brief Display a message box
     * @param hWnd Parent window handle
     * @param lpText Message content
     * @param lpCaption Window title
     * @param uType Window type
     * @return Result of the message box
     */
    INT_PTR MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);

  protected:
    /**
     * @brief Get the initialization XML node
     * @param xmlDoc XML document
     * @return XML node for initialization
     */
    SXmlNode OnGetInitXmlNode(SXmlDoc &xmlDoc) override;

    /**
     * @brief Initialize the dialog
     * @param wnd Host window handle
     * @param lInitParam Initialization parameter (not used)
     * @return TRUE if successful, FALSE otherwise
     */
    BOOL OnInitDialog(HWND wnd, LPARAM lInitParam);

    /**
     * @brief Set the icon
     * @param uType Icon type
     * @return TRUE if successful, FALSE otherwise
     * @details Set the icon. This method can be overridden to display custom icons.
     */
    virtual BOOL OnSetIcon(UINT uType);

    /**
     * @brief Get the button text
     * @param nBtnID Button ID
     * @return Button text
     */
    virtual SStringT OnGetButtonText(int nBtnID) const;

    /**
     * @brief Handle button click event
     * @param uID Control ID
     * @details Message handler for button clicks.
     */
    void OnBtnClick(int uID)
    {
        EndDialog(uID);
    }

    EVENT_MAP_BEGIN()
        EVENT_ID_COMMAND_RANGE(IDOK, IDNO, OnBtnClick)
    EVENT_MAP_END2(SHostDialog)

    BEGIN_MSG_MAP_EX(SMessageBoxImpl)
        MSG_WM_INITDIALOG(OnInitDialog)
        CHAIN_MSG_MAP(SDpiHandler<SMessageBoxImpl>)
        CHAIN_MSG_MAP(SHostDialog)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()
};

SNSEND

#endif // __SMESSAGEBOX__H__