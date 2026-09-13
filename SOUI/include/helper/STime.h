#ifndef STime_h__
#define STime_h__

#include <time.h>

SNSBEGIN

/**
 * @class STimeSpan
 * @brief Time span class
 */
class SOUI_EXP STimeSpan {
  public:
    /**
     * @brief Default constructor
     */
    STimeSpan();

    /**
     * @brief Constructor, initialized from a time span in seconds
     * @param time Time span in seconds
     */
    STimeSpan(__time64_t time);

    /**
     * @brief Constructor, initialized from days, hours, minutes, seconds
     * @param lDays Number of days
     * @param nHours Number of hours
     * @param nMins Number of minutes
     * @param nSecs Number of seconds
     */
    STimeSpan(LONG lDays, int nHours, int nMins, int nSecs);

    /**
     * @brief Get the number of days
     * @return Number of days
     */
    LONGLONG GetDays() const;

    /**
     * @brief Get the total number of hours
     * @return Total number of hours
     */
    LONGLONG GetTotalHours() const;

    /**
     * @brief Get the number of hours
     * @return Number of hours
     */
    LONG GetHours() const;

    /**
     * @brief Get the total number of minutes
     * @return Total number of minutes
     */
    LONGLONG GetTotalMinutes() const;

    /**
     * @brief Get the number of minutes
     * @return Number of minutes
     */
    LONG GetMinutes() const;

    /**
     * @brief Get the total number of seconds
     * @return Total number of seconds
     */
    LONGLONG GetTotalSeconds() const;

    /**
     * @brief Get the number of seconds
     * @return Number of seconds
     */
    LONG GetSeconds() const;

    /**
     * @brief Get the time span in seconds
     * @return Time span in seconds
     */
    __time64_t GetTimeSpan() const;

    /**
     * @brief Addition operator overload
     * @param span Time span object
     * @return New time span object
     */
    STimeSpan operator+(STimeSpan span) const;

    /**
     * @brief Subtraction operator overload
     * @param span Time span object
     * @return New time span object
     */
    STimeSpan operator-(STimeSpan span) const;

    /**
     * @brief Addition-assignment operator overload
     * @param span Time span object
     * @return Reference to the current time span object
     */
    STimeSpan &operator+=(STimeSpan span);

    /**
     * @brief Subtraction-assignment operator overload
     * @param span Time span object
     * @return Reference to the current time span object
     */
    STimeSpan &operator-=(STimeSpan span);

    /**
     * @brief Equality operator overload
     * @param span Time span object
     * @return Returns true if equal, otherwise false
     */
    bool operator==(STimeSpan span) const;

    /**
     * @brief Inequality operator overload
     * @param span Time span object
     * @return Returns true if not equal, otherwise false
     */
    bool operator!=(STimeSpan span) const;

    /**
     * @brief Less-than operator overload
     * @param span Time span object
     * @return Returns true if less than, otherwise false
     */
    bool operator<(STimeSpan span) const;

    /**
     * @brief Greater-than operator overload
     * @param span Time span object
     * @return Returns true if greater than, otherwise false
     */
    bool operator>(STimeSpan span) const;

    /**
     * @brief Less-than-or-equal operator overload
     * @param span Time span object
     * @return Returns true if less than or equal, otherwise false
     */
    bool operator<=(STimeSpan span) const;

    /**
     * @brief Greater-than-or-equal operator overload
     * @param span Time span object
     * @return Returns true if greater than or equal, otherwise false
     */
    bool operator>=(STimeSpan span) const;

  private:
    __time64_t m_timeSpan; /**< Time span in seconds */
};

/**
 * @class STime
 * @brief Time class
 */
class SOUI_EXP STime {
  public:
    /**
     * @brief Get the current time
     * @return Current time object
     */
    static STime GetCurrentTime();

    /**
     * @brief Get the current time in milliseconds
     * @return Current time in milliseconds
     */
    static uint64_t GetCurrentTimeMs();

    /**
     * @brief Default constructor
     * @param tm Time in seconds (default is 0)
     */
    STime(__time64_t tm = 0)
        : m_time(tm)
    {
    }

    /**
     * @brief Constructor, initialized from year, month, day, hour, minute, second
     * @param nYear Year
     * @param nMonth Month
     * @param nDay Day
     * @param nHour Hour
     * @param nMin Minute
     * @param nSec Second
     * @param nDST Daylight saving time flag (default is -1, meaning auto-detect)
     */
    STime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, int nDST = -1);

    /**
     * @brief Set the date
     * @param nYear Year
     * @param nMonth Month
     * @param nDay Day
     */
    void SetDate(int nYear, int nMonth, int nDay);

    /**
     * @brief Set the date and time
     * @param nYear Year
     * @param nMonth Month
     * @param nDay Day
     * @param nHour Hour
     * @param nMin Minute
     * @param nSec Second
     * @param nDST Daylight saving time flag (default is -1, meaning auto-detect)
     */
    void SetDateTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, int nDST = -1);

    /**
     * @brief Get the time in seconds
     * @return Time in seconds
     */
    __time64_t GetTime() const;

    /**
     * @brief Get the local time structure
     * @param ptm Pointer to local time structure
     * @return Pointer to local time structure
     */
    struct tm *GetLocalTm(struct tm *ptm) const;

    /**
     * @brief Get the system time structure
     * @param timeDest Reference to system time structure
     * @return Returns true on success, otherwise false
     */
    bool GetAsSystemTime(SYSTEMTIME &timeDest) const;

    /**
     * @brief Get the year
     * @return Year
     */
    int GetYear() const;

    /**
     * @brief Get the month
     * @return Month
     */
    int GetMonth() const;

    /**
     * @brief Get the day
     * @return Day
     */
    int GetDay() const;

    /**
     * @brief Get the hour
     * @return Hour
     */
    int GetHour() const;

    /**
     * @brief Get the minute
     * @return Minute
     */
    int GetMinute() const;

    /**
     * @brief Get the second
     * @return Second
     */
    int GetSecond() const;

    /**
     * @brief Get the day of the week
     * @return Day of the week (0=Sunday, 1=Monday, ..., 6=Saturday)
     */
    int GetDayOfWeek() const;

    /**
     * @brief Format the time string
     * @param pszFormat Format string
     * @return Formatted time string
     */
    SStringT Format(LPCTSTR pszFormat) const;

    /**
     * @brief Assignment operator overload
     * @param time Time in seconds
     * @return Reference to the current time object
     */
    STime &operator=(__time64_t time);

    /**
     * @brief Addition-assignment operator overload
     * @param span Time span object
     * @return Reference to the current time object
     */
    STime &operator+=(STimeSpan span);

    /**
     * @brief Subtraction-assignment operator overload
     * @param span Time span object
     * @return Reference to the current time object
     */
    STime &operator-=(STimeSpan span);

    /**
     * @brief Subtraction operator overload
     * @param time Time object
     * @return Time span object
     */
    STimeSpan operator-(STime time) const;

    /**
     * @brief Subtraction operator overload
     * @param span Time span object
     * @return New time object
     */
    STime operator-(STimeSpan span) const;

    /**
     * @brief Addition operator overload
     * @param span Time span object
     * @return New time object
     */
    STime operator+(STimeSpan span) const;

    /**
     * @brief Equality operator overload
     * @param time Time object
     * @return Returns true if equal, otherwise false
     */
    bool operator==(STime time) const;

    /**
     * @brief Inequality operator overload
     * @param time Time object
     * @return Returns true if not equal, otherwise false
     */
    bool operator!=(STime time) const;

    /**
     * @brief Less-than operator overload
     * @param time Time object
     * @return Returns true if less than, otherwise false
     */
    bool operator<(STime time) const;

    /**
     * @brief Greater-than operator overload
     * @param time Time object
     * @return Returns true if greater than, otherwise false
     */
    bool operator>(STime time) const;

    /**
     * @brief Less-than-or-equal operator overload
     * @param time Time object
     * @return Returns true if less than or equal, otherwise false
     */
    bool operator<=(STime time) const;

    /**
     * @brief Greater-than-or-equal operator overload
     * @param time Time object
     * @return Returns true if greater than or equal, otherwise false
     */
    bool operator>=(STime time) const;

  protected:
    time_t m_time; /**< Time in seconds */
};

SNSEND

#endif /**< STime_h__ */