/**
 * @file SCalendar.h
 * @brief Calendar Control
 * @version v1.0
 * @author soui
 * @date 2014-05-25
 *
 * @details This class implements a calendar control for date selection and display.
 */

#ifndef __SCALENDAR__H__
#define __SCALENDAR__H__

#include "core/SWnd.h"

SNSBEGIN

/**
 * @class SCalendarCore
 * @brief Calendar Core Class
 * @details This class is the core of the calendar, with most functions being static.
 */
class SOUI_EXP SCalendarCore {
  public:
    /**
     * @brief Determines if a year is a leap year.
     * @param wYear The year to check.
     * @param bLeapYear [output] TRUE if leap year, FALSE if common year.
     * @return TRUE -- Success, FALSE -- Failure
     * @details Checks if the given year is a leap year.
     *          Note: Valid range is (1600 to 6999).
     */
    static BOOL IsLeapYear(WORD wYear, BOOL &bLeapYear);

    /**
     * @brief Calculates the day number within the year (starting from 0, where 0 is January 1st).
     * @param wYear The year.
     * @param wMonth The month.
     * @param wDay The day.
     * @param wDays [output] The day number within the year.
     * @return TRUE -- Success, FALSE -- Failure
     * @details Given a date, calculates the day number within that year, starting from 0.
     *          Note: Valid range is (START_YEAR to END_YEAR-1).
     */
    static BOOL GetDaysNumInYear(WORD wYear, WORD wMonth, WORD wDay, WORD &wDays);

    /**
     * @brief Calculates the month and day from the day number within the year.
     * @param wYear The year.
     * @param wDays The day number within the year.
     * @param wMonth [output] The month.
     * @param wDay [output] The day.
     * @return TRUE -- Success, FALSE -- Failure
     * @details Given a year and day number, calculates the corresponding month and day.
     *          Note: Valid range is (START_YEAR to END_YEAR-1).
     */
    static BOOL GetDateFromDays(WORD wYear, WORD wDays, WORD &wMonth, WORD &wDay);

    /**
     * @brief Returns the day of the week.
     * @param wYear The year.
     * @param wMonth The month.
     * @param wDay The day.
     * @return Returns: 0, 1, 2, 3, 4, 5, 6 corresponding to Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday; -1 if date is invalid or out of range.
     * @details Given a date, returns the day of the week.
     *          Note: Valid range is (START_YEAR-01-01 to END_YEAR-12-31).
     */
    static short GetDayOfWeek(WORD wYear, WORD wMonth, WORD wDay);

    /**
     * @brief Returns the number of days in a specified month.
     * @param wYear The year.
     * @param wMonth The month.
     * @return Returns the number of days in the month, returns 0 if the month/year is invalid.
     * @details Given a year and month, returns the number of days in that month.
     *          Note: Valid range is (START_YEAR-01 to END_YEAR-12).
     */
    static WORD GetDaysOfMonth(WORD wYear, WORD wMonth);

    /**
     * @brief Validates the year, month, and day.
     * @param wYear The year.
     * @param wMonth The month.
     * @param wDay The day.
     * @return FALSE -- Failure, TRUE -- Success
     * @details Validates the given date.
     *          Note: Valid range is (START_YEAR-01-01 to END_YEAR-12-31).
     */
    static BOOL DateCheck(WORD wYear, WORD wMonth, WORD wDay);

    /**
     * @brief Formats the year display.
     * @param iYear The year.
     * @return The formatted year string.
     * @details Formats the year display, using the Chinese zodiac for the lunar year.
     */
    static SStringT FormatYear(WORD iYear);

    /**
     * @brief Formats the month display.
     * @param iMonth The month.
     * @return The formatted month string.
     */
    static SStringT FormatMonth(WORD iMonth);

    /**
     * @brief Formats the day display.
     * @param iDay The day.
     * @return The formatted day string.
     */
    static SStringT FormatDay(WORD iDay);
};

// Button macro definitions
#define HIT_NULL   -1  /**< No Hit */
#define HIT_LEFT   -10 /**< Left button (previous month) */
#define HIT_RIGHT  -11 /**< Right button (next month) */
#define HIT_YEAR   -12 /**< Year-month button (not used yet)*/
#define HIT_YEAR_1 -13 /**< Year-month button (not used yet)*/
#define HIT_YEAR_2 -14 /**< Year-month button (not used yet)*/
#define HIT_YEAR_3 -15 /**< Year-month button (not used yet)*/
#define HIT_TODAY  42  /**< Today button */

// Calendar display states
#define SHOW_MONTH        -101 /**< Show month */
#define SHOW_YEAR         -102 /**< Show year */
#define SHOW_YEAR_DECADE  -103 /**< Show decade */
#define SHOW_YEAR_CENTURY -104 /**< Show century */

/**
 * @class SCalendar
 * @brief Calendar Control Class
 * @details This class implements a calendar control for date selection and display.
 */
class SOUI_EXP SCalendar : public SWindow {
    DEF_SOBJECT(SWindow, L"calendar")

  public:
    /**
     * @brief Constructor
     * @param iYear Initial year
     * @param iMonth Initial month
     * @param iDay Initial day
     */
    SCalendar(WORD iYear, WORD iMonth, WORD iDay);

    /**
     * @brief Default constructor
     */
    SCalendar();

    /**
     * @brief Destructor
     */
    ~SCalendar();

    /**
     * @brief Gets the current year.
     * @return The current year.
     */
    WORD GetYear();

    /**
     * @brief Gets the current month.
     * @return The current month.
     */
    WORD GetMonth();

    /**
     * @brief Gets the current day.
     * @return The current day.
     */
    WORD GetDay();

    /**
     * @brief Gets the current date.
     * @param iYear [output] The year.
     * @param iMonth [output] The month.
     * @param iDay [output] The day.
     */
    void GetDate(WORD &iYear, WORD &iMonth, WORD &iDay);

    /**
     * @brief Sets the date.
     * @param iYear The year.
     * @param iMonth The month.
     * @param iDay The day.
     * @param nBtnType Button type.
     * @param bNotify Whether to notify events.
     * @return TRUE -- Success, FALSE -- Failure
     */
    BOOL SetDate(WORD iYear, WORD iMonth, WORD iDay, int nBtnType = HIT_NULL, bool bNotify = false);

  protected:
    /**
     * @brief Called when the language changes.
     * @return HRESULT indicating the result of the operation.
     */
    virtual HRESULT OnLanguageChanged();

  protected:
    /**
     * @brief Initializes the control.
     */
    void Init();

    /**
     * @brief Paints the control.
     * @param pRT The rendering target.
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief Handles left mouse button down event.
     * @param nFlags Key state.
     * @param point Mouse position.
     */
    void OnLButtonDown(UINT nFlags, CPoint point);

    /**
     * @brief Handles left mouse button up event.
     * @param nFlags Key state.
     * @param point Mouse position.
     */
    void OnLButtonUp(UINT nFlags, CPoint point);

    /**
     * @brief Handles mouse move event.
     * @param nFlags Key state.
     * @param pt Mouse position.
     */
    void OnMouseMove(UINT nFlags, CPoint pt);

    /**
     * @brief Handles mouse leave event.
     */
    void OnMouseLeave();

    /**
     * @brief Handles mouse wheel event.
     * @param nFlags Key state.
     * @param zDelta Wheel delta.
     * @param pt Mouse position.
     * @return TRUE -- Success, FALSE -- Failure
     */
    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    SOUI_ATTRS_BEGIN()
        ATTR_LAYOUTSIZE(L"yearHeight", m_nYearMonthHeight, TRUE)
        ATTR_LAYOUTSIZE(L"weekHeight", m_nWeekHeight, TRUE)
        ATTR_LAYOUTSIZE(L"todayHeight", m_nFooterHeight, TRUE)
        ATTR_COLOR(L"colorSelText", m_crSelText, TRUE)
        ATTR_COLOR(L"colorOtherText", m_crOtherDayText, TRUE)
        ATTR_COLOR(L"colorSelBg", m_crSelDayBack, TRUE)
        ATTR_COLOR(L"colorHoverText", m_crHoverText, TRUE)
        ATTR_SKIN(L"prevSkin", m_pSkinPrev, TRUE)
        ATTR_SKIN(L"nextSkin", m_pSkinNext, TRUE)
        ATTR_SKIN(L"daySkin", m_pSkinDay, TRUE)
        ATTR_SKIN(L"weekSkin", m_pSkinWeek, TRUE)
        ATTR_I18NSTRT(L"textSunday", m_strWeek[0], TRUE)
        ATTR_I18NSTRT(L"textMonday", m_strWeek[1], TRUE)
        ATTR_I18NSTRT(L"textTuesday", m_strWeek[2], TRUE)
        ATTR_I18NSTRT(L"textWednesday", m_strWeek[3], TRUE)
        ATTR_I18NSTRT(L"textThursday", m_strWeek[4], TRUE)
        ATTR_I18NSTRT(L"textFriday", m_strWeek[5], TRUE)
        ATTR_I18NSTRT(L"textSaturday", m_strWeek[6], TRUE)
    SOUI_ATTRS_END()

  protected:
    /**
     * @brief Called when the control is created.
     * @param lp Creation parameters.
     * @return HRESULT indicating the result of the operation.
     */
    int OnCreate(LPVOID lp);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_CREATE(OnCreate)
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
    SOUI_MSG_MAP_END()

  protected:
    /**
     * @brief Tests the mouse click position.
     * @param pt Mouse position.
     * @return The item corresponding to the click position.
     */
    int HitTest(const CPoint &pt);

    /**
     * @brief Draws the calendar header (year and month).
     * @param pRT The rendering target.
     * @param rect The drawing area.
     */
    void DrawYearMonth(IRenderTarget *pRT, const CRect &rect);

    /**
     * @brief Draws the week days.
     * @param pRT The rendering target.
     * @param rect The drawing area.
     */
    void DrawWeek(IRenderTarget *pRT, const CRect &rect);

    /**
     * @brief Draws the day.
     * @param pRT The rendering target.
     * @param rcDay The day area.
     * @param nItem The day item.
     */
    void DrawDay(IRenderTarget *pRT, CRect &rcDay, int nItem);

    /**
     * @brief Draws the "Today" button.
     * @param pRT The rendering target.
     * @param rcDay The "Today" button area.
     */
    void DrawToday(IRenderTarget *pRT, CRect &rcDay);

    /**
     * @brief Gets the item drawing area.
     * @param nItem The item index.
     * @param rcItem [output] The item area.
     */
    void GetItemRect(int nItem, CRect &rcItem);

    /**
     * @brief Sets the previous month.
     */
    void SetLastMonth();

    /**
     * @brief Sets the next month.
     */
    void SetNextMonth();

    /**
     * @brief Sets the previous year.
     */
    void SetLastYear();

    /**
     * @brief Sets the next year.
     */
    void SetNextYear();

    /**
     * @brief Sets the previous decade.
     */
    void SetLastYearDecade();

    /**
     * @brief Sets the next decade.
     */
    void SetNextYearDecade();

    /**
     * @brief Sets the previous century.
     */
    void SetLastYearCentury();

    /**
     * @brief Sets the next century.
     */
    void SetNextYearCentury();

    /**
     * @brief Sets the year and month.
     * @param iYear The year.
     * @param iMonth The month.
     */
    void SetYearMonth(int iYear, int iMonth);

    /**
     * @brief Paints the month view.
     * @param pRT The rendering target.
     */
    void OnPaintMonth(IRenderTarget *pRT);

    /**
     * @brief Paints the year, decade, and century views.
     * @param pRT The rendering target.
     */
    void OnPaintYearDecadeCentury(IRenderTarget *pRT);

    /**
     * @brief Draws the year, decade, and century items.
     * @param pRT The rendering target.
     * @param rect The drawing area.
     * @param nItem The item index.
     */
    void DrawYearDecadeCentury(IRenderTarget *pRT, const CRect &rect, int nItem);

  public:
    /**
     * @brief Sets the calendar display type.
     * @param showType The display type.
     */
    void SetShowType(int showType);

    /**
     * @brief Sets the year, decade, and century views.
     */
    void SetYearDecadeCentury();

  protected:
    /**
     * @brief Height of the year and month.
     */
    SLayoutSize m_nYearMonthHeight;

    /**
     * @brief Height of the week.
     */
    SLayoutSize m_nWeekHeight;

    /**
     * @brief Height of the "Today" button.
     */
    SLayoutSize m_nFooterHeight;

    /**
     * @brief Selected text color.
     */
    COLORREF m_crSelText;

    /**
     * @brief Color of other days' text.
     */
    COLORREF m_crOtherDayText;

    /**
     * @brief Background color of the selected day.
     */
    COLORREF m_crSelDayBack;

    /**
     * @brief Color of the hover text.
     */
    COLORREF m_crHoverText;

    /**
     * @brief Skin for the previous button.
     */
    SAutoRefPtr<ISkinObj> m_pSkinPrev;

    /**
     * @brief Skin for the next button.
     */
    SAutoRefPtr<ISkinObj> m_pSkinNext;

    /**
     * @brief Skin for the day.
     */
    SAutoRefPtr<ISkinObj> m_pSkinDay;

    /**
     * @brief Skin for the week.
     */
    SAutoRefPtr<ISkinObj> m_pSkinWeek;

    /**
     * @brief Header text.
     */
    STrText m_strWeek[7];

    /**
     * @brief Date information structure.
     */
    struct wDayInfo
    {
        WORD iDay; // Calendar day
        int nType; // -1 previous month, 0 current month, 1 next month
    };

    /**
     * @brief Date information array.
     */
    wDayInfo m_arrDays[42];

    /**
     * @brief Date area.
     */
    CRect m_rcDays;

    /**
     * @brief "Today" button area.
     */
    CRect m_rcToday;

    /**
     * @brief Selected item.
     */
    int m_nSelItem;

    /**
     * @brief Hover item.
     */
    int m_nHoverItem;

    /**
     * @brief Current year.
     */
    WORD m_iYear;

    /**
     * @brief Current month.
     */
    WORD m_iMonth;

    /**
     * @brief Current date.
     */
    SYSTEMTIME m_Today;

    /**
     * @brief Calendar display state.
     */
    int m_showType;

    /**
     * @brief Calendar display state when mouse button is down.
     */
    int m_showTypeLbdown;

    /**
     * @brief Month or year information structure.
     */
    struct wMonthOrYearInfo
    {
        WORD iMonthOrYear; // Calendar month-year-decade-century
        int nType;         // -1 previous, 0 current, 1 next
    };

    /**
     * @brief Month or year information array.
     */
    wMonthOrYearInfo m_arrMonthOrYear[12];
};

SNSEND

#endif // __SCALENDAR__H__