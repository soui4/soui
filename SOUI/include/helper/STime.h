#ifndef STime_h__
#define STime_h__

#include <time.h>

SNSBEGIN

/**
 * @class STimeSpan
 * @brief 时间间隔类
 */
class SOUI_EXP STimeSpan {
  public:
    /**
     * @brief 默认构造函数
     */
    STimeSpan();

    /**
     * @brief 构造函数，从时间间隔秒数初始化
     * @param time 时间间隔秒数
     */
    STimeSpan(__time64_t time);

    /**
     * @brief 构造函数，从天、小时、分钟、秒初始化
     * @param lDays 天数
     * @param nHours 小时数
     * @param nMins 分钟数
     * @param nSecs 秒数
     */
    STimeSpan(LONG lDays, int nHours, int nMins, int nSecs);

    /**
     * @brief 获取天数
     * @return 天数
     */
    LONGLONG GetDays() const;

    /**
     * @brief 获取总小时数
     * @return 总小时数
     */
    LONGLONG GetTotalHours() const;

    /**
     * @brief 获取小时数
     * @return 小时数
     */
    LONG GetHours() const;

    /**
     * @brief 获取总分钟数
     * @return 总分钟数
     */
    LONGLONG GetTotalMinutes() const;

    /**
     * @brief 获取分钟数
     * @return 分钟数
     */
    LONG GetMinutes() const;

    /**
     * @brief 获取总秒数
     * @return 总秒数
     */
    LONGLONG GetTotalSeconds() const;

    /**
     * @brief 获取秒数
     * @return 秒数
     */
    LONG GetSeconds() const;

    /**
     * @brief 获取时间间隔秒数
     * @return 时间间隔秒数
     */
    __time64_t GetTimeSpan() const;

    /**
     * @brief 加法运算符重载
     * @param span 时间间隔对象
     * @return 新的时间间隔对象
     */
    STimeSpan operator+(STimeSpan span) const;

    /**
     * @brief 减法运算符重载
     * @param span 时间间隔对象
     * @return 新的时间间隔对象
     */
    STimeSpan operator-(STimeSpan span) const;

    /**
     * @brief 加法赋值运算符重载
     * @param span 时间间隔对象
     * @return 当前时间间隔对象的引用
     */
    STimeSpan &operator+=(STimeSpan span);

    /**
     * @brief 减法赋值运算符重载
     * @param span 时间间隔对象
     * @return 当前时间间隔对象的引用
     */
    STimeSpan &operator-=(STimeSpan span);

    /**
     * @brief 等于运算符重载
     * @param span 时间间隔对象
     * @return 如果相等返回true，否则返回false
     */
    bool operator==(STimeSpan span) const;

    /**
     * @brief 不等于运算符重载
     * @param span 时间间隔对象
     * @return 如果不相等返回true，否则返回false
     */
    bool operator!=(STimeSpan span) const;

    /**
     * @brief 小于运算符重载
     * @param span 时间间隔对象
     * @return 如果小于返回true，否则返回false
     */
    bool operator<(STimeSpan span) const;

    /**
     * @brief 大于运算符重载
     * @param span 时间间隔对象
     * @return 如果大于返回true，否则返回false
     */
    bool operator>(STimeSpan span) const;

    /**
     * @brief 小于等于运算符重载
     * @param span 时间间隔对象
     * @return 如果小于等于返回true，否则返回false
     */
    bool operator<=(STimeSpan span) const;

    /**
     * @brief 大于等于运算符重载
     * @param span 时间间隔对象
     * @return 如果大于等于返回true，否则返回false
     */
    bool operator>=(STimeSpan span) const;

  private:
    __time64_t m_timeSpan; // 时间间隔秒数
};

/**
 * @class STime
 * @brief 时间类
 */
class SOUI_EXP STime {
  public:
    /**
     * @brief 获取当前时间
     * @return 当前时间对象
     */
    static STime GetCurrentTime();

    /**
     * @brief 获取当前时间的毫秒数
     * @return 当前时间的毫秒数
     */
    static uint64_t GetCurrentTimeMs();

    /**
     * @brief 默认构造函数
     * @param tm 时间秒数（默认为0）
     */
    STime(__time64_t tm = 0)
        : m_time(tm)
    {
    }

    /**
     * @brief 构造函数，从年、月、日、时、分、秒初始化
     * @param nYear 年份
     * @param nMonth 月份
     * @param nDay 日
     * @param nHour 小时
     * @param nMin 分钟
     * @param nSec 秒
     * @param nDST 夏令时标志（默认为-1，表示自动检测）
     */
    STime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, int nDST = -1);

    /**
     * @brief 设置日期
     * @param nYear 年份
     * @param nMonth 月份
     * @param nDay 日
     */
    void SetDate(int nYear, int nMonth, int nDay);

    /**
     * @brief 设置日期和时间
     * @param nYear 年份
     * @param nMonth 月份
     * @param nDay 日
     * @param nHour 小时
     * @param nMin 分钟
     * @param nSec 秒
     * @param nDST 夏令时标志（默认为-1，表示自动检测）
     */
    void SetDateTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, int nDST = -1);

    /**
     * @brief 获取时间秒数
     * @return 时间秒数
     */
    __time64_t GetTime() const;

    /**
     * @brief 获取本地时间结构
     * @param ptm 本地时间结构指针
     * @return 本地时间结构指针
     */
    struct tm *GetLocalTm(struct tm *ptm) const;

    /**
     * @brief 获取系统时间结构
     * @param timeDest 系统时间结构引用
     * @return 如果成功返回true，否则返回false
     */
    bool GetAsSystemTime(SYSTEMTIME &timeDest) const;

    /**
     * @brief 获取年份
     * @return 年份
     */
    int GetYear() const;

    /**
     * @brief 获取月份
     * @return 月份
     */
    int GetMonth() const;

    /**
     * @brief 获取日
     * @return 日
     */
    int GetDay() const;

    /**
     * @brief 获取小时
     * @return 小时
     */
    int GetHour() const;

    /**
     * @brief 获取分钟
     * @return 分钟
     */
    int GetMinute() const;

    /**
     * @brief 获取秒
     * @return 秒
     */
    int GetSecond() const;

    /**
     * @brief 获取星期几
     * @return 星期几（0=星期日，1=星期一，...，6=星期六）
     */
    int GetDayOfWeek() const;

    /**
     * @brief 格式化时间字符串
     * @param pszFormat 格式字符串
     * @return 格式化后的时间字符串
     */
    SStringT Format(LPCTSTR pszFormat) const;

    /**
     * @brief 赋值运算符重载
     * @param time 时间秒数
     * @return 当前时间对象的引用
     */
    STime &operator=(__time64_t time);

    /**
     * @brief 加法赋值运算符重载
     * @param span 时间间隔对象
     * @return 当前时间对象的引用
     */
    STime &operator+=(STimeSpan span);

    /**
     * @brief 减法赋值运算符重载
     * @param span 时间间隔对象
     * @return 当前时间对象的引用
     */
    STime &operator-=(STimeSpan span);

    /**
     * @brief 减法运算符重载
     * @param time 时间对象
     * @return 时间间隔对象
     */
    STimeSpan operator-(STime time) const;

    /**
     * @brief 减法运算符重载
     * @param span 时间间隔对象
     * @return 新的时间对象
     */
    STime operator-(STimeSpan span) const;

    /**
     * @brief 加法运算符重载
     * @param span 时间间隔对象
     * @return 新的时间对象
     */
    STime operator+(STimeSpan span) const;

    /**
     * @brief 等于运算符重载
     * @param time 时间对象
     * @return 如果相等返回true，否则返回false
     */
    bool operator==(STime time) const;

    /**
     * @brief 不等于运算符重载
     * @param time 时间对象
     * @return 如果不相等返回true，否则返回false
     */
    bool operator!=(STime time) const;

    /**
     * @brief 小于运算符重载
     * @param time 时间对象
     * @return 如果小于返回true，否则返回false
     */
    bool operator<(STime time) const;

    /**
     * @brief 大于运算符重载
     * @param time 时间对象
     * @return 如果大于返回true，否则返回false
     */
    bool operator>(STime time) const;

    /**
     * @brief 小于等于运算符重载
     * @param time 时间对象
     * @return 如果小于等于返回true，否则返回false
     */
    bool operator<=(STime time) const;

    /**
     * @brief 大于等于运算符重载
     * @param time 时间对象
     * @return 如果大于等于返回true，否则返回false
     */
    bool operator>=(STime time) const;

  protected:
    time_t m_time; // 时间秒数
};

SNSEND

#endif // STime_h__