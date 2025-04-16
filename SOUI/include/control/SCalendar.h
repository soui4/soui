/**
 * @file SCalendar.h
 * @brief 日历时间控件
 * @version v1.0
 * @author soui
 * @date 2014-05-25
 *
 * @details 此类完成日历控件的实现。
 */

#ifndef __SCALENDAR__H__
#define __SCALENDAR__H__

#include "core/SWnd.h"

SNSBEGIN

/**
 * @class SCalendarCore
 * @brief 日历核心类
 * @details 此类是日历的核心类，大部分函数都是静态函数。
 */
class SOUI_EXP SCalendarCore {
  public:
    /**
     * @brief 判断闰年
     * @param wYear 待判断的公历年份
     * @param bLeapYear 【输出参数】TRUE：闰年，FALSE：平年
     * @return TRUE -- 成功  FALSE -- 失败
     * @details 判断是否是闰年
     *          注意：有效范围是（1600年到6999年）
     */
    static BOOL IsLeapYear(WORD wYear, BOOL &bLeapYear);

    /**
     * @brief 计算日期在年内的序数（序数从0开始，即0为元旦）
     * @param wYear 公历年
     * @param wMonth 公历月
     * @param wDay 公历日
     * @param wDays 【输出参数】年内序数
     * @return TRUE -- 成功  FALSE -- 失败
     * @details 输入日期，计算该日期在这一年内的序数，序数从0开始
     *          注意：有效范围是（START_YEAR年到END_YEAR-1年）
     */
    static BOOL GetDaysNumInYear(WORD wYear, WORD wMonth, WORD wDay, WORD &wDays);

    /**
     * @brief 从年内序数计算月、日
     * @param wYear 公历年
     * @param wDays 年内序数
     * @param wMonth 【输出参数】公历月
     * @param wDay 【输出参数】公历日
     * @return TRUE -- 成功  FALSE -- 失败
     * @details 输入年份以及年内序数，计算出月和日
     *          注意：有效范围是（START_YEAR年到END_YEAR-1年）
     */
    static BOOL GetDateFromDays(WORD wYear, WORD wDays, WORD &wMonth, WORD &wDay);

    /**
     * @brief 返回星期几
     * @param wYear 公历年
     * @param wMonth 公历月
     * @param wDay 公历日
     * @return 返回:0,1,2,3,4,5,6分别对应日、一、二、三、四、五、六，-1表示日期错误或超出范围
     * @details 输入年月日返回星期几
     *          注意:有效范围是(START_YEAR年1月1日 --- END_YEAR-1年12月31日)
     */
    static short GetDayOfWeek(WORD wYear, WORD wMonth, WORD wDay);

    /**
     * @brief 返回指定月份的天数
     * @param wYear 公历年
     * @param wMonth 公历月
     * @return 返回该月的天数，返回0表示输入年月有误
     * @details 输入年月返回本月的天数
     *          注意:有效范围是(START_YEAR年1月 --- END_YEAR-1年12月)
     */
    static WORD GetDaysOfMonth(WORD wYear, WORD wMonth);

    /**
     * @brief 检验年、月、日的合法性
     * @param wYear 公历年
     * @param wMonth 公历月
     * @param wDay 公历日
     * @return FALSE-失败，TRUE-成功
     * @details 输入公历日期，检查日期的合法性
     *          注意:有效范围是(START_YEAR年1月1日---END_YEAR-1年12月31日)
     */
    static BOOL DateCheck(WORD wYear, WORD wMonth, WORD wDay);

    /**
     * @brief 格式化年份显示型式
     * @param iYear 年
     * @return 格式化后的年份字符串
     * @details 指定阴历年返回采用干支纪年法
     */
    static SStringT FormatYear(WORD iYear);

    /**
     * @brief 格式化月份显示型式
     * @param iMonth 月
     * @return 格式化后的月份字符串
     */
    static SStringT FormatMonth(WORD iMonth);

    /**
     * @brief 格式化日期显示型式
     * @param iDay 日
     * @return 格式化后的日期字符串
     */
    static SStringT FormatDay(WORD iDay);
};

// 按钮 宏定义
#define HIT_NULL   -1  // 无
#define HIT_LEFT   -10 // 上一个月 按钮
#define HIT_RIGHT  -11 // 下一个月 按钮
#define HIT_YEAR   -12 //  年月  还没用到
#define HIT_YEAR_1 -13
#define HIT_YEAR_2 -14
#define HIT_YEAR_3 -15
#define HIT_TODAY  42 //  今天

// 日历显示的状态
#define SHOW_MONTH        -101 // 显示月份
#define SHOW_YEAR         -102 // 显示年份
#define SHOW_YEAR_DECADE  -103 // 显示近10年
#define SHOW_YEAR_CENTURY -104 // 显示近100年

/**
 * @class SCalendar
 * @brief 日历控件类
 * @details 此类实现了一个日历控件，支持日期选择和显示。
 */
class SOUI_EXP SCalendar : public SWindow {
    DEF_SOBJECT(SWindow, L"calendar")

  public:
    /**
     * @brief 构造函数
     * @param iYear 初始年份
     * @param iMonth 初始月份
     * @param iDay 初始日期
     */
    SCalendar(WORD iYear, WORD iMonth, WORD iDay);

    /**
     * @brief 默认构造函数
     */
    SCalendar();

    /**
     * @brief 析构函数
     */
    ~SCalendar();

    /**
     * @brief 获取当前年份
     * @return 当前年份
     */
    WORD GetYear();

    /**
     * @brief 获取当前月份
     * @return 当前月份
     */
    WORD GetMonth();

    /**
     * @brief 获取当前日期
     * @return 当前日期
     */
    WORD GetDay();

    /**
     * @brief 获取当前日期
     * @param iYear 【输出参数】年份
     * @param iMonth 【输出参数】月份
     * @param iDay 【输出参数】日期
     */
    void GetDate(WORD &iYear, WORD &iMonth, WORD &iDay);

    /**
     * @brief 设置日期
     * @param iYear 年份
     * @param iMonth 月份
     * @param iDay 日期
     * @param nBtnType 按钮类型
     * @param bNotify 是否通知事件
     * @return TRUE -- 成功  FALSE -- 失败
     */
    BOOL SetDate(WORD iYear, WORD iMonth, WORD iDay, int nBtnType = HIT_NULL, bool bNotify = false);

  protected:
    /**
     * @brief 语言改变时调用
     * @return HRESULT 表示操作结果
     */
    virtual HRESULT OnLanguageChanged();

  protected:
    /**
     * @brief 初始化控件
     */
    void Init();

    /**
     * @brief 绘制控件
     * @param pRT 渲染目标
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief 处理左键按下事件
     * @param nFlags 按键状态
     * @param point 鼠标位置
     */
    void OnLButtonDown(UINT nFlags, CPoint point);

    /**
     * @brief 处理左键释放事件
     * @param nFlags 按键状态
     * @param point 鼠标位置
     */
    void OnLButtonUp(UINT nFlags, CPoint point);

    /**
     * @brief 处理鼠标移动事件
     * @param nFlags 按键状态
     * @param pt 鼠标位置
     */
    void OnMouseMove(UINT nFlags, CPoint pt);

    /**
     * @brief 处理鼠标离开事件
     */
    void OnMouseLeave();

    /**
     * @brief 处理鼠标滚轮事件
     * @param nFlags 按键状态
     * @param zDelta 滚轮滚动值
     * @param pt 鼠标位置
     * @return TRUE -- 成功  FALSE -- 失败
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
     * @brief 创建控件时调用
     * @param lp 创建参数
     * @return HRESULT 表示操作结果
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
     * @brief 测试鼠标点击位置
     * @param pt 鼠标位置
     * @return 点击位置对应的项
     */
    int HitTest(const CPoint &pt);

    /**
     * @brief 绘制日历头（年月）
     * @param pRT 渲染目标
     * @param rect 绘制区域
     */
    void DrawYearMonth(IRenderTarget *pRT, const CRect &rect);

    /**
     * @brief 绘制星期
     * @param pRT 渲染目标
     * @param rect 绘制区域
     */
    void DrawWeek(IRenderTarget *pRT, const CRect &rect);

    /**
     * @brief 绘制日期
     * @param pRT 渲染目标
     * @param rcDay 日期区域
     * @param nItem 日期项
     */
    void DrawDay(IRenderTarget *pRT, CRect &rcDay, int nItem);

    /**
     * @brief 绘制今天按钮
     * @param pRT 渲染目标
     * @param rcDay 今天按钮区域
     */
    void DrawToday(IRenderTarget *pRT, CRect &rcDay);

    /**
     * @brief 获取项的绘制区域
     * @param nItem 项索引
     * @param rcItem 【输出参数】项区域
     */
    void GetItemRect(int nItem, CRect &rcItem);

    /**
     * @brief 设置上一个月
     */
    void SetLastMonth();

    /**
     * @brief 设置下一个月
     */
    void SetNextMonth();

    /**
     * @brief 设置上一年
     */
    void SetLastYear();

    /**
     * @brief 设置下一年
     */
    void SetNextYear();

    /**
     * @brief 设置上一个十年
     */
    void SetLastYearDecade();

    /**
     * @brief 设置下一个十年
     */
    void SetNextYearDecade();

    /**
     * @brief 设置上一个世纪
     */
    void SetLastYearCentury();

    /**
     * @brief 设置下一个世纪
     */
    void SetNextYearCentury();

    /**
     * @brief 设置年月
     * @param iYear 年份
     * @param iMonth 月份
     */
    void SetYearMonth(int iYear, int iMonth);

    /**
     * @brief 绘制月份视图
     * @param pRT 渲染目标
     */
    void OnPaintMonth(IRenderTarget *pRT);

    /**
     * @brief 绘制年份、十年、世纪视图
     * @param pRT 渲染目标
     */
    void OnPaintYearDecadeCentury(IRenderTarget *pRT);

    /**
     * @brief 绘制年份、十年、世纪项
     * @param pRT 渲染目标
     * @param rect 绘制区域
     * @param nItem 项索引
     */
    void DrawYearDecadeCentury(IRenderTarget *pRT, const CRect &rect, int nItem);

  public:
    /**
     * @brief 设置日历显示类型
     * @param showType 显示类型
     */
    void SetShowType(int showType);

    /**
     * @brief 设置年份、十年、世纪视图
     */
    void SetYearDecadeCentury();

  protected:
    /**
     * @brief 年月高度
     */
    SLayoutSize m_nYearMonthHeight;

    /**
     * @brief 星期高度
     */
    SLayoutSize m_nWeekHeight;

    /**
     * @brief 今天按钮高度
     */
    SLayoutSize m_nFooterHeight;

    /**
     * @brief 选中文本颜色
     */
    COLORREF m_crSelText;

    /**
     * @brief 其他天的文本颜色
     */
    COLORREF m_crOtherDayText;

    /**
     * @brief 选中天的背景颜色
     */
    COLORREF m_crSelDayBack;

    /**
     * @brief 悬停文本颜色
     */
    COLORREF m_crHoverText;

    /**
     * @brief 上一个按钮皮肤
     */
    SAutoRefPtr<ISkinObj> m_pSkinPrev;

    /**
     * @brief 下一个按钮皮肤
     */
    SAutoRefPtr<ISkinObj> m_pSkinNext;

    /**
     * @brief 天皮肤
     */
    SAutoRefPtr<ISkinObj> m_pSkinDay;

    /**
     * @brief 星期皮肤
     */
    SAutoRefPtr<ISkinObj> m_pSkinWeek;

    /**
     * @brief 表头文本
     */
    STrText m_strWeek[7];

    /**
     * @brief 日期信息结构
     */
    struct wDayInfo
    {
        WORD iDay; // 日历天
        int nType; // -1 前一个月 0 当月 1 下一个月
    };

    /**
     * @brief 日期信息数组
     */
    wDayInfo m_arrDays[42];

    /**
     * @brief 日期区域
     */
    CRect m_rcDays;

    /**
     * @brief 今天按钮区域
     */
    CRect m_rcToday;

    /**
     * @brief 选中项
     */
    int m_nSelItem;

    /**
     * @brief 悬停项
     */
    int m_nHoverItem;

    /**
     * @brief 当前年份
     */
    WORD m_iYear;

    /**
     * @brief 当前月份
     */
    WORD m_iMonth;

    /**
     * @brief 当前日期
     */
    SYSTEMTIME m_Today;

    /**
     * @brief 日历显示状态
     */
    int m_showType;

    /**
     * @brief 鼠标按下时的日历显示状态
     */
    int m_showTypeLbdown;

    /**
     * @brief 月份或年份信息结构
     */
    struct wMonthOrYearInfo
    {
        WORD iMonthOrYear; // 日历 月-年-年代-世纪
        int nType;         // -1前一 0 当前 1后一
    };

    /**
     * @brief 月份或年份信息数组
     */
    wMonthOrYearInfo m_arrMonthOrYear[12];
};

SNSEND

#endif // __SCALENDAR__H__