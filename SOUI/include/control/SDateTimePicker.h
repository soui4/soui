#ifndef __SDATETIMEPICKER__H__
#define __SDATETIMEPICKER__H__

#include <core/SWnd.h>
#include <proxy/SWindowProxy.h>
#include <interface/SCtrl-i.h>
#include <control/SDropDown.h>
#include <control/SCalendar.h>

SNSBEGIN

/**
 * @class SDateTimePicker
 * @brief DateTime Picker Control
 * @details A DateTime picker control that allows users to select a date and optionally time.
 */
class SOUI_EXP SDateTimePicker
    : public TWindowProxy<IDateTimePicker>
    , public ISDropDownOwner {
    DEF_SOBJECT(SWindow, L"dateTimePicker")

  public:
    /**
     * @enum EnDateType
     * @brief Enumeration for date types
     */
    enum EnDateType
    {
        eDT_NULL = 0,
        eDT_Year,
        eDT_Month,
        eDT_Day,
        eDT_Hour,
        eDT_Minute,
        eDT_Second,
    };

    /**
     * @brief Constructor
     */
    SDateTimePicker();

    /**
     * @brief Destructor
     */
    ~SDateTimePicker();

  public:
    /**
     * @brief Set the time
     * @param wYear Year
     * @param wMonth Month
     * @param wDay Day
     * @param wHour Hour
     * @param wMinute Minute
     * @param wSecond Second
     */
    STDMETHOD_(void, SetTime)
    (THIS_ WORD wYear, WORD wMonth, WORD wDay, WORD wHour, WORD wMinute, WORD wSecond) OVERRIDE;

    /**
     * @brief Get the time
     * @param wYear Output year
     * @param wMonth Output month
     * @param wDay Output day
     * @param wHour Output hour
     * @param wMinute Output minute
     * @param wSecond Output second
     */
    STDMETHOD_(void, GetTime)
    (THIS_ WORD *wYear, WORD *wMonth, WORD *wDay, WORD *wHour, WORD *wMinute, WORD *wSecond) SCONST OVERRIDE;

    /**
     * @brief Close the dropdown
     */
    STDMETHOD_(void, CloseUp)(THIS) OVERRIDE;

    /**
     * @brief Drop down the list
     */
    STDMETHOD_(void, DropDown)(THIS) OVERRIDE;

    /**
     * @brief Clear the selected date and time
     */
    STDMETHOD_(void, Clear)(THIS) OVERRIDE;

  public:
    /**
     * @brief Get the window text
     * @param bRawText Whether to get raw text
     * @return Window text
     */
    SStringT GetWindowText(BOOL bRawText = FALSE);

    /**
     * @brief Set the time using SYSTEMTIME structure
     * @param sysTime SYSTEMTIME structure
     */
    void SetTime(const SYSTEMTIME &sysTime);

    /**
     * @brief Get the time using SYSTEMTIME structure
     * @param sysTime SYSTEMTIME structure
     */
    void GetTime(SYSTEMTIME &sysTime);

  protected:
    /**
     * @brief Get the owner window for the dropdown
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
     */
    virtual void OnDestroyDropDown(SDropDownWnd *pDropDown);

  protected:
    /**
     * @brief Hit test to determine the date type at a given point
     * @param pt Mouse coordinates
     * @return Date type at the point
     */
    EnDateType HitTest(CPoint pt);

    /**
     * @brief Create child controls
     * @param xmlNode XML node for the child controls
     * @return TRUE if successful, FALSE otherwise
     */
    virtual BOOL CreateChildren(SXmlNode xmlNode);

    /**
     * @brief Called when the scale of the window changes.
     * @param scale The new scale value.
     */
    virtual void OnScaleChanged(int scale);

    /**
     * @brief Handle date change event
     * @param pEvt Event object
     * @return TRUE if handled, FALSE otherwise
     */
    BOOL OnDateChanged(EventCalendarExChanged *pEvt);

    /**
     * @brief Handle date command event
     * @param pEvt Event object
     * @return TRUE if handled, FALSE otherwise
     */
    BOOL OnDateCmd(EventCmd *pEvt);

    /**
     * @brief Get the rectangle of the dropdown button
     * @param pBtnRc Output rectangle for the button
     * @param pSkinRc Output rectangle for the skin
     */
    void GetDropBtnRect(LPRECT pBtnRc, LPRECT pSkinRc = NULL);

    /**
     * @brief Convert a number to formatted text based on the date type
     * @param eType Date type
     * @param wNum Number to format
     * @return Formatted text
     */
    SStringT ToFormatText(EnDateType eType, WORD wNum);

    /**
     * @brief Calculate the popup rectangle
     * @param nHeight Height of the popup
     * @param rcPopup Output rectangle for the popup
     * @return TRUE if successful, FALSE otherwise
     */
    bool CalcPopupRect(int nHeight, CRect &rcPopup);

    /**
     * @brief Draw the date/time component
     * @param eType Date type
     * @param pRT Rendering target handle
     * @param wNum Number to draw
     * @param rcText Rectangle for the text
     */
    void Draw(EnDateType eType, IRenderTarget *pRT, WORD wNum, CRect &rcText);

  protected:
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
     * @brief Handle mouse wheel event
     * @param nFlags Flags
     * @param zDelta Wheel delta
     * @param pt Mouse coordinates
     * @return TRUE if handled, FALSE otherwise
     */
    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    /**
     * @brief Handle key down event
     * @param nChar Key code
     * @param nRepCnt Repeat count
     * @param nFlags Flags
     */
    void OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags);

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
     * @brief Handle set focus event
     * @param wndOld Previous focus window handle
     */
    void OnSetFocus(SWND wndOld);

    /**
     * @brief Handle kill focus event
     * @param wndFocus New focus window handle
     */
    void OnKillFocus(SWND wndFocus);

    /**
     * @brief Handle time wheel event
     * @param bUp Whether the wheel is scrolled up
     */
    void TimeWheel(bool bUp);

    /**
     * @brief Handle circular number increment/decrement
     * @param bUp Whether to increment
     * @param wNum Current number
     * @param wMin Minimum number
     * @param wMax Maximum number
     */
    void CircluNum(bool bUp, WORD &wNum, WORD wMin, WORD wMax);

    /**
     * @brief Handle custom attribute "cueText"
     * @param strValue Attribute value
     * @param bLoading Loading flag
     * @return HRESULT
     */
    HRESULT OnAttrCueText(const SStringW &strValue, BOOL bLoading);

    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"btnSkin", m_pSkinBtn, TRUE)
        ATTR_BOOL(L"timeEnable", m_bTimeEnable, TRUE)
        ATTR_INT(L"dropWidth", m_nDropWidth, TRUE)
        ATTR_COLOR(L"cueColor", m_crCue, TRUE)
        ATTR_CUSTOM(L"cueText", OnAttrCueText)
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
        MSG_WM_SETFOCUS_EX(OnSetFocus)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
    SOUI_MSG_MAP_END()

  protected:
    DWORD m_dwBtnState;               /**< Button state */
    SAutoRefPtr<ISkinObj> m_pSkinBtn; /**< Button skin object */
    SDropDownWnd *m_pDropDownWnd;     /**< Dropdown window pointer */
    EnDateType m_eSelDateType;        /**< Selected date type */
    int m_nNumWidth;                  /**< Width of number display */
    int m_nNumHeight;                 /**< Height of number display */
    int m_nCharWidth;                 /**< Width of character display */
    COLORREF m_crSelBg;               /**< Selection background color */
    COLORREF m_crSelText;             /**< Selection text color */
    SYSTEMTIME m_sysTime;             /**< System time */
    SStringT m_sKey;                  /**< Key string */

    int m_nDropWidth;       /**< Dropdown width */
    bool m_bTimeEnable;     /**< Whether time (hour, minute, second) is enabled */
    SCalendar *m_pCalendar; /**< Calendar pointer */
    WORD m_wCharNum;        /**< Character number */
    STrText m_strCue;       /**< Cue text */
    COLORREF m_crCue;       /**< Cue text color */
};

SNSEND

#endif // __SDATETIMEPICKER__H__